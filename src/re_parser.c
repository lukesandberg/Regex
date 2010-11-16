#include "re_parser.h"
#include "re_lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <util/fat_stack.h>
#include <util/util.h>
/*
This will parse our simple grammar

re: 
	  empty
	| reg

reg:
	  reg ALT_TOK simple-re
	| simple-re

simple-re:
	  simple-re basic-re
	| basic-re

basic-re:
	  basic-re STAR_TOK
	| basic-re PLUS_TOK
	| basic-re QMARK_TOK
	| basic-re LCR_TOK num RCR_TOK
	| basic-re LCR_TOK num1 COMMA_TOK num2 RCR_TOK
	| (reg)
	| CHAR_TOK

*/

#define parse_error(en, pos) do {\
	if(er != NULL){\
		er->errno = en;\
		er->position = pos;\
	}\
	}while(0)

static ast_node* parse_basic_re(fat_stack* tok_stk, re_error *er);
static ast_node* parse_simple_re(fat_stack* tok_stk, re_error*er);
static ast_node* parse_reg(fat_stack* tok_stk, re_error* er);
static ast_node* parse_simple_re(fat_stack* tok_stk, re_error*er);
static ast_node* handle_group(fat_stack* tok_stk, re_error *er);
static int read_num(fat_stack* tok_stk, re_error *er, unsigned int* val);
//0 is failure, 1 is comma, 2 is lbracket
static int read_comma_or_lbracket(fat_stack* tok_stk, re_error * er);
static inline fat_stack* read_all_tokens(lexer* lxr, re_error* er);

struct parse_state
{
	lexer lxr;
	fat_stack* tokens;
};

enum stack_token_type
{
	NODE,
	TOKEN
};
typedef struct
{
	enum stack_token_type type;
	union
	{
		ast_node* node;
		token tok;
	} v;
} stack_token;


static ast_node* get_expected_re(struct parse_state* state, re_error* er)
{
	ast_node* single_node = NULL;
	if(fat_stack_size(state->tokens) == 0)
	{
		parse_error(E_EXPECTED_TOKEN, -1);
		return NULL;
	}
	
	stack_token* tok_ptr = (stack_token*)fat_stack_peek(state->tokens);
	stack_token tok = *tok_ptr;
	
	if(tok.type == NODE)
		single_node = tok.v.node;
	else
		parse_error(E_EXPECTED_TOKEN, tok.v.tok.position);
	return single_node;
}

static token get_expected_tok(struct parse_state* state, re_error* er)
{
	token invalid_tok = {.type=INVALID_TOK, .position=-1};
	if(fat_stack_size(state->tokens) == 0)
	{
		parse_error(E_EXPECTED_TOKEN, -1);
		return invalid_tok;
	}
	stack_token* tok_ptr = (stack_token*)fat_stack_peek(state->tokens);
	stack_token tok = *tok_ptr;
	
	if(tok.type == TOKEN)
		return tok.v.tok;
	else
	{
		parse_error(E_INVALID_TOKEN, -1);
		return invalid_tok;
	}
}

static int get_expected_num(struct parse_state* state, unsigned int *num, re_error* er)
{
	token tok = get_expected_tok(state, er);
	if(tok.type == INVALID_TOK)
		return 0;
	if(tok.type != NUM_TOK)
	{
		parse_error(E_EXPECTED_TOKEN, tok.position);
		return 0;
	}
	*num = tok.v.num_value;
	return 1;
}

static int handle_counted_rep(struct parse_state* state, re_error* er)
{
	unsigned int max;
	unsigned int min;
	if(!get_expected_num(state, &max, er))
		return 0;
	fat_stack_pop(state->tokens);
	min = max;//default to the same
	token tok = get_expected_tok(state, er);
	fat_stack_pop(state->tokens);
	if(tok.type == INVALID_TOK)
		return 0;
	if(tok.type != COMMA_TOK && tok.type != LCR_TOK)
	{
		parse_error(E_UNEXPECTED_TOKEN, tok.position);
		return 0;
	}
	if(tok.type == COMMA_TOK)
	{
		if(!get_expected_num(state, &min, er))
			return 0;
		fat_stack_pop(state->tokens);
		tok = get_expected_tok(state, er);
		if(tok.type == INVALID_TOK)
			return 0;
		if(tok.type != LCR_TOK)
		{
			parse_error(E_UNEXPECTED_TOKEN, tok.position);
			return 0;
		}
		fat_stack_pop(state->tokens);
	}
	//we have the min and the max and its all off the stack
	ast_node* node = get_expected_re(state, er);
	if(node == NULL)
		return 0;
	loop_node* ln = make_loop(node, min, max);
	if(ln == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return 0;
	}
	//put our new node on top of the stack
	((stack_token*) fat_stack_peek(state->tokens))->v.node = (ast_node*) ln;
	return 1;
}

static int handle_unary_op(struct parse_state* state, node_type type, re_error* er)
{
	stack_token* tok_ptr = (stack_token*) fat_stack_peek(state->tokens);
	stack_token tok = *tok_ptr;
	if(tok.type == NODE)
	{
		ast_node* node = tok.v.node;
		unary_node* un = make_unary(node, type);
		if(un == NULL)
		{
			parse_error(E_OUT_OF_MEMORY, -1);
			return 0;
		}
		tok_ptr->v.node = (ast_node*) un;
		return 1;
	}
	else
	{
		parse_error(E_MISSING_OP_ARGUMENT, tok.v.tok.position);
		return 0;
	}
}

static int multi_node_add(multi_node* m, ast_node* n)
{
	if(!linked_list_add_last(m->list, n))
	{
		free_node((ast_node*)m);
		free_node(n);
		return 0;
	}
	return 1;
}

static multi_node* make_multi_and_push(node_type multi_type, unsigned int count, ...)
{
	multi_node* m = make_multi(multi_type);
	if(m == NULL)
		return NULL;
	
	va_list args;
	va_start(args, count);
	ast_node* node;
	int error = 0;
	for(unsigned int i = 0; i < count; i++)
	{
		node = va_arg(args, ast_node*);
		if(error)
		{
			free_node(node);
		}
		else if(!multi_node_add(m, node))
		{
			m = NULL;
			error = 1;
		}
	}
	va_end(args);
	return m;
}

static int push_node(struct parse_state* state, ast_node* node, re_error* er)
{
	if(node == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return 0;
	}

	stack_token tok;
	tok.type = NODE;
	tok.v.node = node;

	if(!fat_stack_push(state->tokens, &tok))
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return 0;
	}
	return 1;
}

//this will handle a stack that looks like
//(RE | RE |RE and turn it into a multinode of alternations or a single node on the stack
//in either case the top of the stack will be a single RE 
static int do_alternation(struct parse_state* state, re_error *er, int should_stop_at_lparen)
{
	stack_token* tok_ptr;
	multi_node* alt = NULL;
	ast_node* single_node = get_expected_re(state, er);
	if(single_node == NULL)
		return 0;

	int expect_re = 0; //0 -> expect an ALT or end; 1 -> expect an RE
	while(fat_stack_size(state->tokens) > 1)
	{
		fat_stack_pop(state->tokens);
		if(expect_re)
		{
			ast_node* node = get_expected_re(state, er);
			if(node == NULL)
				goto error;

			if(alt == NULL)
			{
				alt = make_multi_and_push(ALT, 1, single_node);
				if(alt == NULL)
				{
					parse_error(E_OUT_OF_MEMORY, -1);
					goto error;
				}
				single_node = NULL;
			}
			if(!multi_node_add(alt, node))
			{
				parse_error(E_OUT_OF_MEMORY, -1);
				goto error;
			}
			expect_re = 0;
		}
		else
		{
			token tok = get_expected_tok(state, er);
			if(tok.type == INVALID_TOK)
				goto error;
			if(tok.type == ALT_TOK)
			{
				//this is what we expected
				expect_re = 1;
			}//we must be at the end or an error
			else if(tok.type == LPAREN_TOK && should_stop_at_lparen)
			{
				goto end;
			}
			else
			{
				parse_error(E_INVALID_TOKEN, tok.position);
				goto error;
			}
		}
	}
//if we get here then we ran out of tokens
//this is only ok if we were not supposed to stop at an lparen
	if(should_stop_at_lparen)
	{
		parse_error(E_UNMATCHED_PAREN, -1);
		goto error;
	}
	
end:
	tok_ptr = (stack_token*) fat_stack_peek(state->tokens);
	tok_ptr->type = NODE;
	//put the alternation or single re on top of the stack
	if(single_node != NULL)
	{
		tok_ptr->v.node= single_node;
	}
	else if(alt != NULL)
	{
		tok_ptr->v.node= (ast_node*) alt;
	}
	
	//we have the first re
	return 1;
error:
	if(alt != NULL) free_node((ast_node*) alt);
	if(single_node != NULL) free_node(single_node);
	return 0;
}

static int do_concat(struct parse_state* state, re_error *er)
{
	ast_node* single_node = NULL;
	multi_node* cat = NULL;
	
	stack_token* tok_ptr = NULL;
	stack_token tok;
	
	single_node = get_expected_re(state, er);
	if(single_node == NULL)
		return 0;
	
	while(fat_stack_size(state->tokens) > 1)
	{
		fat_stack_pop(state->tokens);//pop the previous item
		tok_ptr = (stack_token*) fat_stack_peek(state->tokens);
		tok = *tok_ptr;
		if(tok.type == NODE)
		{
			ast_node* node = tok.v.node;
			if(cat == NULL)
			{
				cat = make_multi_and_push(CONCAT, 1, single_node);
				if(cat == NULL)
				{
					parse_error(E_OUT_OF_MEMORY, -1);
					return 0;
				}
			}
			if(!multi_node_add(cat, node))
			{
				parse_error(E_OUT_OF_MEMORY, -1);
				return 0;
			}
		}
		else
		{
			token_type tt = tok.v.tok.type;
			if(tt == LPAREN_TOK || tt == ALT_TOK)
			{
				goto end;
			}
			else
			{
				parse_error(E_UNEXPECTED_TOKEN, tok.v.tok.position);
				if(cat != NULL) free_node((ast_node*) cat);
				if(single_node != NULL) free_node(single_node);
				return 0;
			}
		}
	}
end:
//the previous item is still on the stack so instead of popping and pushing a new one
//we will just edit the top, this will remove a free/malloc pair which elimiates one more
//error location
	if(cat != NULL)
	{
		tok_ptr->type = NODE;
		tok_ptr->v.node =  (ast_node*) cat;
	}
	else if(single_node != NULL)
	{
		tok_ptr->type = NODE;
		tok_ptr->v.node =  single_node;
	}
	else
	{
		//this was an empty concatentation
		parse_error(E_EXPECTED_TOKEN, tok.v.tok.position);
		return 0;
	}
	
	return 1;
}
static int push_token(struct parse_state *state, token tok, re_error* er)
{
	stack_token st;
	st.type = TOKEN;
	st.v.tok = tok;
	if(!fat_stack_push(state->tokens, &st))
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return 0;
	}
	return 1;
}

static int handle_end(struct parse_state *state, ast_node** rval, re_error* er)
{
	if(!do_concat(state, er))
		return 0;
	if(!do_alternation(state, er, 0))
		return 0;
	unsigned int sz = fat_stack_size(state->tokens);
	if(sz != 1)
	{
		if(sz > 1) parse_error(E_UNEXPECTED_TOKEN, -1);
		else parse_error(E_EXPECTED_TOKEN, -1);
		return 0;
	}
	*rval = get_expected_re(state, er);
	if(*rval == NULL)
	{
		parse_error(E_UNEXPECTED_TOKEN, -1);
		return 0;
	}
	return 1;
}

//0 implies an error, 1 implies continue, 2 implies that we are done
static int token_dispatch(struct parse_state* state, token* tok, ast_node** rval, re_error* er)
{
	int v;
	switch(tok->type)
	{
		case CHAR_TOK:
			v = push_node(state, (ast_node*) make_char(tok->v.char_value), er);
			break;
		case PLUS_TOK:
			v = handle_unary_op(state, PLUS, er);
			break;
		case QMARK_TOK:
			v = handle_unary_op(state, QMARK, er);
			break;
		case STAR_TOK:
			v = handle_unary_op(state, STAR, er);
			break;
		case NG_STAR_TOK:
			v = handle_unary_op(state, NG_STAR, er);
			break;
		case NG_PLUS_TOK:
			v = handle_unary_op(state, NG_PLUS, er);
			break;
		case NG_QMARK_TOK:
			v = handle_unary_op(state, NG_QMARK, er);
			break;
		case WILDCARD_TOK:
			v = push_node(state, (ast_node*) make_node(WILDCARD), er);
			break;
		case DIGIT_TOK:
			v = push_node(state, make_node(DIGIT), er);
			break;
		case WHITESPACE_TOK:
			v = push_node(state, make_node(WHITESPACE), er);
			break;
		case ALPHA_TOK:
			v = push_node(state, make_node(ALPHA), er);
			break;
		case LCR_TOK:
		case LPAREN_TOK:
		case COMMA_TOK:
		case NUM_TOK:
			v = push_token(state, *tok, er);
			break;
		case RCR_TOK:
			v = handle_counted_rep(state, er);
			break;
		case ALT_TOK:
			v = do_concat(state, er);
			if(v)
				v = push_token(state, *tok, er);
			break;
		case RPAREN_TOK:
			v = do_concat(state, er);
			if(v)
				v = do_alternation(state, er, 1);
			break;
		case INVALID_TOK:
			parse_error(E_INVALID_TOKEN, tok->position);
			v = 0;
			break;
		case END_TOK:
			v = handle_end(state, rval, er);
			if(v) v =2;//
			break;
	}
	return v;
}

static void destroy_stack(fat_stack* stack)
{
	while(fat_stack_size(stack) >0)
	{
		stack_token* t = (stack_token*) fat_stack_peek(stack);
		if(t->type == NODE)
		{
			free_node(t->v.node);
		}
		fat_stack_pop(stack);
	}
	fat_stack_destroy(stack);
}

ast_node* re_parse_new(char* regex, re_error* er)
{
	//default to success
	parse_error(E_SUCCESS, -1);
	ast_node* tree = NULL;
	struct parse_state state;
	init_lexer(&state.lxr, regex);
	state.tokens = fat_stack_create(sizeof(stack_token));
	if(state.tokens == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return NULL;
	}
	token tok;
	int continu = 1;//continue is a keyword :(
	while(continu)
	{
		tok = read_token(&state.lxr);
		continu = token_dispatch(&state, &tok, &tree, er);
		if(continu == 2)//we are done
			break;	
	}

	destroy_stack(state.tokens);
	return tree;
}



static ast_node* handle_group(fat_stack* tok_stk, re_error *er)
{
	//we already saw the rparen
	//since the matching lparen is not part of the reg
	//we need to find it and pass a token stack that doesnt include it
	int c = 1;
	unsigned int position = -1;
	fat_stack_entry* prev_e = NULL;
	fat_stack_entry* e = tok_stk->top;
	int subex_size = 0;
	while(e != NULL)
	{
		token* tok = (token*) &e->val[0];
		position = tok->position;
		if(tok->type == LPAREN_TOK)
			c--;
		else if(tok->type == RPAREN_TOK)
			c++;
		if(c==0)
			break;
		prev_e = e;
		subex_size++;
		e = e->next;
	}
	if(c >0)
	{
		parse_error(E_UNMATCHED_PAREN, position);
		return NULL;
	}
	//e is pointing at the LPAREN
	if(prev_e == NULL)
	{//we had an empty group which is invalid
		parse_error(E_UNEXPECTED_TOKEN, position);
		return NULL;
	}
	//now we know we have a non empty sub expression
	//so chop up the stack to pop up the whole
	fat_stack* sub_stk = fat_stack_create(sizeof(token));
	if(sub_stk == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return NULL;
	}
	sub_stk->top = tok_stk->top;
	sub_stk->entries = subex_size;
	prev_e->next = NULL;//sever the linked list

	tok_stk->top = e->next;//we don't want the LPAREN
	free(e);//free the LPAREN
	tok_stk->entries -= subex_size + 1;

	ast_node* sub = parse_reg(sub_stk, er);
	fat_stack_destroy(sub_stk);
	return sub;
}

static int read_num(fat_stack* tok_stk, re_error *er, unsigned int* val)
{
	if(fat_stack_size(tok_stk) == 0)
	{
		parse_error(E_EXPECTED_TOKEN, -1);
		return 0;
	}
	token* tokptr = (token*) fat_stack_peek(tok_stk);
	token tok = *tokptr;//copy to local mem
	fat_stack_pop(tok_stk);
	if(tok.type == NUM_TOK)
	{
		*val = tok.v.num_value;
		return 1;
	}
	parse_error(E_INVALID_TOKEN, tok.position);
	return 0;
}
//0 is failure, 1 is comma, 2 is lbracket
static int read_comma_or_lbracket(fat_stack* tok_stk, re_error * er)
{
	if(fat_stack_size(tok_stk) == 0)
	{
		parse_error(E_EXPECTED_TOKEN, -1);
		return 0;
	}
	token* tokptr = (token*) fat_stack_peek(tok_stk);
	token tok = *tokptr;//copy to local mem
	fat_stack_pop(tok_stk);
	if(tok.type == COMMA_TOK)
	{
		return 1;
	}
	if(tok.type == LCR_TOK)
	{
		return 2;
	}
	
	parse_error(E_INVALID_TOKEN, tok.position);
	return 0;
}

static ast_node* parse_counted_rep(fat_stack* tok_stk, re_error*er)
{
	unsigned int max = 0;
	unsigned int min = 0;
	if(read_num(tok_stk, er, &max))
	{
		min = max;
		switch(read_comma_or_lbracket(tok_stk, er))
		{
			case 1://comma
				if(!read_num(tok_stk, er, &min))
				{
					return  NULL;
				}
				if(read_comma_or_lbracket(tok_stk, er) != 2)
				{
					return NULL;
				}
				break;
			case 2://lbracket
				break;
			default:
				return NULL;
		}
	}
	else
	{
		return NULL;
	}
	
	ast_node* sub = parse_basic_re(tok_stk, er);
	if(sub == NULL)
	{
		//fix our error message
		parse_error(E_MISSING_OP_ARGUMENT, er->position);
		return NULL;
	}
	ast_node* n = (ast_node*)  make_loop(sub, min, max);
	if(n == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		free_node(sub);
	}
	return n;

}

static ast_node* oom_check(ast_node* node, re_error *er)
{
	if(node == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return NULL;
	}
	return node;
}

static ast_node* parse_basic_re(fat_stack* tok_stk, re_error *er)
{
	if(fat_stack_size(tok_stk) == 0)
	{
		parse_error(E_EXPECTED_TOKEN, -1);
		return NULL;
	}
	token* tokptr = (token*) fat_stack_peek(tok_stk);
	token tok = *tokptr;//copy to local mem
	fat_stack_pop(tok_stk);
	node_type unary_type;
	switch(tok.type)
	{
		case CHAR_TOK:
			return oom_check((ast_node*) make_char(tok.v.char_value), er);
		case WILDCARD_TOK:
			return oom_check((ast_node*) make_node(WILDCARD), er);
		case ALPHA_TOK:
			return oom_check((ast_node*) make_node(ALPHA), er);
		case WHITESPACE_TOK:
			return oom_check((ast_node*) make_node(WHITESPACE), er);
		case DIGIT_TOK:
			return oom_check((ast_node*) make_node(DIGIT), er);
		/*unary ops all have the same precedence*/
		case PLUS_TOK:
			unary_type = PLUS;
			break;
		case QMARK_TOK:
			unary_type = QMARK;
			break;
		case STAR_TOK:
			unary_type = STAR;
			break;
		case NG_STAR_TOK:
			unary_type = NG_STAR;
			break;
		case NG_PLUS_TOK:
			unary_type = NG_PLUS;
			break;
		case NG_QMARK_TOK:
			unary_type = NG_QMARK;
			break;
		case RCR_TOK:
			//this is a special case because we now have some
			//special stuff to parse
			return parse_counted_rep(tok_stk, er);
		case RPAREN_TOK:
			;
			ast_node* reg = handle_group(tok_stk, er);
			if(reg == NULL)
				return NULL;
			unary_node* capture = make_unary(reg, CAPTURE);
			if(capture == NULL)
			{
				parse_error(E_OUT_OF_MEMORY, -1);
				return NULL;
			}
			return (ast_node*) capture;
		default:
			//error
			parse_error(E_UNEXPECTED_TOKEN, tok.position);
			return NULL;
	}
	//if we get here then we need to recurse for a unary op
	ast_node* sub = parse_basic_re(tok_stk, er);
	if(sub == NULL)
	{
		//fix our error message
		parse_error(E_MISSING_OP_ARGUMENT, tok.position);
		return NULL;
	}
	unary_node* n = make_unary(sub, unary_type);
	if(n == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		free_node((ast_node*) sub);
	}
	return (ast_node*) n;
}

static ast_node* parse_simple_re(fat_stack* tok_stk, re_error* er)
{
	multi_node* cat = NULL;
	//simple is basic or a cat of basic and simple
	while(1)
	{
		//very similar to parse_reg, just that alt has lower precedence than cat
		ast_node* basic_re = parse_basic_re(tok_stk, er);
		if(basic_re == NULL)
			return NULL;//there was an error, break out
		//now we are done if
		//there is no next token
		//or if the next token is an ALT
		int is_done = 0;
		if(fat_stack_size(tok_stk) == 0 || ((token*)fat_stack_peek(tok_stk))->type == ALT_TOK)
			is_done = 1;
		if(cat == NULL)
		{
			//we are not part of an existing cat
			if(is_done)
				return basic_re;
			cat = make_multi(CONCAT);
			if(cat == NULL)
			{
				parse_error(E_OUT_OF_MEMORY, -1);
				free_node(basic_re);
				return NULL;
			}
		}
		int s = linked_list_add_first(cat->list, basic_re);
		if(!s)
		{
			parse_error(E_OUT_OF_MEMORY, -1);
			free_node(basic_re);
			free_node((ast_node*) cat);
			return NULL;
		}
		if(is_done)
			return (ast_node*) cat;
		//at this point we need to recurse, but to prevent blowing the stack for long inputs 
		//we just do a loop and keep accumulating in the cat arg
		//e.g. a long sequence of characters "aaaaaaa..." will result in a recursion
		//for each char which could overflow the stack
	}
}

static ast_node* parse_reg(fat_stack* tok_stk, re_error* er)
{
	//given our grammar its either
	//simple-re or reg | simple-re
	//so lets try to parse a simple test for a pipe or end and recurse
	ast_node* simple_re = parse_simple_re(tok_stk, er);
	if(simple_re == NULL)
		return NULL;//there was an error, break out
	if(fat_stack_size(tok_stk) == 0)
		return simple_re;
	//o.w. we must have an alt
	token* topptr = (token*) fat_stack_peek(tok_stk);
	token top = *topptr;
	fat_stack_pop(tok_stk);
	if(top.type == ALT_TOK)
	{
		ast_node* reg_node = parse_reg(tok_stk, er);
		if(reg_node == NULL)
		{
			free_node(simple_re);
			return NULL;
		}
		ast_node* alt = (ast_node*) make_binary(reg_node, simple_re, ALT);
		if(alt == NULL)
		{
			parse_error(E_OUT_OF_MEMORY, -1);
			free_node(simple_re);
			free_node(reg_node);
			return NULL;	
		}
		return alt;
	}
	else
	{
		parse_error(E_UNEXPECTED_TOKEN, top.position);
		free_node(simple_re);
		return NULL;
	}
}

static inline fat_stack* read_all_tokens(lexer* lxr, re_error* er)
{
	//we need to read all of our tokens onto the stack
	fat_stack* stk = fat_stack_create(sizeof(token));
	if(stk == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return NULL;
	}
	token tok;
	while((tok = read_token(lxr)).type != END_TOK)
	{
		if(tok.type == INVALID_TOK)
		{
			parse_error(E_INVALID_TOKEN, tok.position);
			fat_stack_destroy(stk);
			return NULL;
		}
		int s = fat_stack_push(stk, &tok);
		if(!s)
		{
			parse_error(E_OUT_OF_MEMORY, -1);
			fat_stack_destroy(stk);
			return NULL;
		}
	}
	return stk;
}

ast_node* re_parse(char *regex, re_error* er)
{
	//default to success
	parse_error(E_SUCCESS, -1);
	
	if(*regex == '\0')
	{
		ast_node* n = make_node(EMPTY);
		if(n == NULL)
		{
			parse_error(E_OUT_OF_MEMORY, -1);
			return NULL;
		}
		return n;
	}
	
	lexer lxr;
	init_lexer(&lxr, regex);
	
	fat_stack* stk = read_all_tokens(&lxr, er);
	if(stk == NULL)
		return NULL;
	ast_node* tree = parse_reg(stk, er);
	fat_stack_destroy(stk);
	return tree;

}

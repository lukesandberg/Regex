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


struct parse_state
{
	lexer lxr;
	fat_stack* tokens;
};

enum stop_criteria
{
	SHOULD_STOP_AT_LPAREN = 0x1,
	SHOULD_STOP_AT_END = 0x2,
	SHOULD_STOP_AT_ALT = 0x4,
	STOP_AT_ANY = 0x7
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
	token invalid_tok;
	invalid_tok.type=INVALID_TOK;
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

static int read_num(struct parse_state* state, unsigned int *rval, re_error* er)
{
	token tok = read_token(&state->lxr);
	if(tok.type != NUM_TOK)
	{
		parse_error(E_EXPECTED_NUM, tok.position);
		return 0;
	}
	*rval = tok.v.num_value;
	return 1;
}

static int handle_counted_rep(struct parse_state* state, re_error* er)
{
	//we have already read the lcr_tok now read the rest
	unsigned int max;
	unsigned int min;

	if(!read_num(state, &min, er))
		return 0;
	
	max = min;//default
	token delim = read_token(&state->lxr);
	if(delim.type != COMMA_TOK && delim.type != RCR_TOK)
	{
		parse_error(E_UNEXPECTED_TOKEN, delim.position);
		return 0;
	}
	if(delim.type == COMMA_TOK)
	{
		if(!read_num(state, &max, er))
			return 0;
		
		delim = read_token(&state->lxr);
		if(delim.type != RCR_TOK)
		{
			parse_error(E_UNEXPECTED_TOKEN, delim.position);
			return 0;
		}
	}
	//we have the min and the max
	ast_node* node = get_expected_re(state, er);
	if(node == NULL)
		return 0;
	loop_node* ln = make_loop(node, min, max);
	if(ln == NULL)
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return 0;
	}
	//replace the re on top of the stack with our expression
	((stack_token*) fat_stack_peek(state->tokens))->v.node = (ast_node*) ln;
	return 1;
}

static int handle_unary_op(struct parse_state* state, node_type type, re_error* er)
{
	if(fat_stack_size(state->tokens) == 0)
	{
		parse_error(E_MISSING_OP_ARGUMENT, -1);
		return 0;
	}
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
	if(!linked_list_add_first(m->list, n))
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
static int do_alternation(struct parse_state* state, re_error *er, enum stop_criteria stop)
{
	stack_token* tok_ptr;
	multi_node* alt = NULL;
	ast_node* single_node = NULL;

	int expect_re = 1; //0 -> expect an ALT or end; 1 -> expect an RE
	while(fat_stack_size(state->tokens) > 0)
	{
		if(expect_re)
		{
			ast_node* node = get_expected_re(state, er);
			if(node == NULL)
				goto error;
			if(single_node == NULL && alt == NULL)
			{
				single_node = node;
			}
			else
			{
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
			else if((tok.type == LPAREN_TOK) && (stop & SHOULD_STOP_AT_LPAREN))
			{
				goto end;
			}
			else
			{
				parse_error(E_INVALID_TOKEN, tok.position);
				goto error;
			}
		}
		if(fat_stack_size(state->tokens) == 1)
			break;
		fat_stack_pop(state->tokens);
	}
	if(expect_re)
	{
		parse_error(E_MISSING_OP_ARGUMENT, -1);
		goto error;
	}
//if we get here then we ran out of tokens
//this is only ok if we were not supposed to stop at an lparen
	if(!(stop & SHOULD_STOP_AT_END))
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
	else
	{
		parse_error(E_EXPECTED_TOKEN, -1);
		return 0;
	}
	
	//we have the first re
	return 1;
error:
	if(alt != NULL) free_node((ast_node*) alt);
	if(single_node != NULL) free_node(single_node);
	return 0;
}

static int do_concat(struct parse_state* state, re_error *er, enum stop_criteria stop)
{
	ast_node* single_node = NULL;
	multi_node* cat = NULL;
	stack_token* tok_ptr = NULL;
	stack_token tok;

	while(fat_stack_size(state->tokens) > 0)
	{
		tok_ptr = (stack_token*) fat_stack_peek(state->tokens);
		tok = *tok_ptr;
		if(tok.type == NODE)
		{
			ast_node* node = tok.v.node;
			if(single_node == NULL && cat == NULL)
			{
				single_node = tok.v.node;
			}
			else
			{
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
		}
		else
		{
			token_type tt = tok.v.tok.type;
			if((tt == LPAREN_TOK && (stop & SHOULD_STOP_AT_LPAREN))
				|| (tt == ALT_TOK && (stop & SHOULD_STOP_AT_ALT)))
			{
				goto end;
			}
			else
			{
				parse_error(E_UNEXPECTED_TOKEN, tok.v.tok.position);
				goto error;
			}
		}
		fat_stack_pop(state->tokens);//pop the previous item
	}

	if(!(stop & SHOULD_STOP_AT_END))
	{
		parse_error(E_UNMATCHED_PAREN, -1);
		goto error;
	}
end:
//the previous item is still on the stack so instead of popping and pushing a new one
//we will just edit the top, this will remove a free/malloc pair which elimiates one more
//error location

	tok.type = NODE;
	if(!fat_stack_push(state->tokens, &tok))
	{
		parse_error(E_OUT_OF_MEMORY, -1);
		return 0;
	}
	tok_ptr = (stack_token*) fat_stack_peek(state->tokens);
	
	if(cat != NULL)
	{
		tok_ptr->v.node =  (ast_node*) cat;
	}
	else if(single_node != NULL)
	{
		tok_ptr->v.node =  single_node;
	}
	else
	{
		ast_node* empty = make_node(EMPTY);
		if(empty == NULL)
		{
			parse_error(E_OUT_OF_MEMORY, -1);
			return 0;
		}
		tok_ptr->v.node = empty;
	}
	
	return 1;

error:
	if(cat != NULL) free_node((ast_node*) cat);
	if(single_node != NULL) free_node(single_node);
	return 0;
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
	if(!do_concat(state, er, SHOULD_STOP_AT_END | SHOULD_STOP_AT_ALT))
		return 0;
	if(!do_alternation(state, er, SHOULD_STOP_AT_END))
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
	fat_stack_pop(state->tokens);
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
		case LPAREN_TOK:
			v = push_token(state, *tok, er);
			break;
		case LCR_TOK:
			v = handle_counted_rep(state, er);
			break;
		case ALT_TOK:
			v = do_concat(state, er, STOP_AT_ANY);
			if(v)
				v = push_token(state, *tok, er);
			break;
		case RPAREN_TOK:
			v = do_concat(state, er, SHOULD_STOP_AT_ALT | SHOULD_STOP_AT_LPAREN);
			if(v)
				v = do_alternation(state, er, SHOULD_STOP_AT_LPAREN);
			if(v)
				v = handle_unary_op(state, CAPTURE, er);
			break;
		case INVALID_TOK:
		/*commas nums and rcrs are should be handled automatically be the handle_counted_rep procedure
		 if they appear elsewhere then there is an error*/
		case COMMA_TOK:
		case NUM_TOK:
		case RCR_TOK:
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

ast_node* re_parse(char* regex, re_error* er)
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
#include "re_parser.h"
#include "re_lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <util/fat_stack.h>
#include <util/util.h>
/*
This will parse our simple grammar

re: 
	  empty
	| reg

reg:
	  reg | simple-re
	| simple-re

simple-re:
	  simple-re basic-re
	| basic-re

basic-re:
	  basic-re STAR
	| basic-re PLUS
	| basic-re QMARK
	| (reg)
	| CHAR		

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
			return (ast_node*) make_char(tok.v.char_value);
		case WILDCARD_TOK:
			return (ast_node*) make_node(WILDCARD);
		case ALPHA_TOK:
			return (ast_node*) make_node(ALPHA);
		case WHITESPACE_TOK:
			return (ast_node*) make_node(WHITESPACE);
		case DIGIT_TOK:
			return (ast_node*) make_node(DIGIT);
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
		case RPAREN_TOK:
			return handle_group(tok_stk, er);
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

static ast_node* parse_re(lexer* lxr, re_error* er)
{
	token tok = read_token(lxr);
	if(tok.type == END_TOK)
	{
		ast_node* n = make_node(EMPTY);
		if(n == NULL)
		{
			parse_error(E_OUT_OF_MEMORY, -1);
			return NULL;
		}
		return n;
	}
	unread_token(lxr, tok);
	fat_stack* stk = read_all_tokens(lxr, er);
	if(stk == NULL)
		return NULL;
	ast_node* tree = parse_reg(stk, er);
	fat_stack_destroy(stk);
	return tree;
}

ast_node* re_parse(char *regex, re_error* er)
{
	//default to success
	parse_error(E_SUCCESS, -1);
	
	lexer l;
	init_lexer(&l, regex);
	return parse_re(&l, er);
}

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

static ast_node* parse_basic_re(fat_stack* tok_stk, re_error *er);
static ast_node* parse_simple_re(fat_stack* tok_stk, re_error*er);
static ast_node* parse_reg(fat_stack* tok_stk, re_error* er);
static ast_node* parse_simple_re(fat_stack* tok_stk, re_error*er);

static ast_node* handle_group(fat_stack* tok_stk, re_error *er)
{
	//we already saw the rparen
	ast_node* reg = parse_reg(tok_stk, er);
	if(reg == NULL)
		return NULL;
	token* top = fat_stack_peek(tok_stk);
	if(top->type == LPAREN_TOK)
	{
		fat_stack_pop(tok_stk);
		return reg;
	}
	free_node(reg);
	if(er != NULL)
	{
		er->errno = E_UNMATCHED_PAREN;
		er->position = top->position;
	}
	return NULL;
}

static ast_node* parse_basic_re(fat_stack* tok_stk, re_error *er)
{
	if(fat_stack_size(tok_stk) == 0)
	{
		if(er != NULL)
		{
			er->errno = E_EXPECTED_TOKEN;
			er->position = -1;
		}
		return NULL;
	}
	token* tokptr = (token*) fat_stack_peek(tok_stk);
	token tok = *tokptr;//copy to local mem
	fat_stack_pop(tok_stk);
	unary_node* n = NULL;
	switch(tok.type)
	{
		case CHAR_TOK:
			return (ast_node*) make_rule(tok.v.char_value);
		case WILDCARD_TOK:
			return (ast_node*) make_rule(RWILDCARD);
		case ALPHA_TOK:
			return (ast_node*) make_rule(RALPHA);
		case WHITESPACE_TOK:
			return (ast_node*) make_rule(RWHITESPACE);
		case DIGIT_TOK:
			return (ast_node*) make_rule(RDIGIT);
		/*unary ops all have the same precedence*/
		case PLUS_TOK:
			n = make_unary(NULL, PLUS);
			break;
		case QMARK_TOK:
			n = make_unary(NULL, QMARK);
			break;
		case STAR_TOK:
			n = make_unary(NULL, STAR);
			break;
		case RPAREN_TOK:
			return handle_group(tok_stk, er);
		default:
			//error
			if(er != NULL)
			{
				er->errno = E_UNEXPECTED_TOKEN;
				er->position = tok.position;
			}
			return NULL;
	}
	//if we get here then we need to recurse for a unary op
	ast_node* sub = parse_basic_re(tok_stk, er);
	if(sub == NULL)
	{
		//fix our error message
		if(er != NULL)
		{
			er->errno = E_MISSING_OP_ARGUMENT;
			er->position = tok.position;
		}
		free_node((ast_node*)n);
		return NULL;
	}
	n->expr = sub;
	return (ast_node*) n;
}

static ast_node* parse_simple_re(fat_stack* tok_stk, re_error*er)
{
	//simple is basic or a cat of basic and simple
	//very similar to parse_reg, just that alt has lower precedence than cat
	ast_node* basic_re = parse_basic_re(tok_stk, er);
	if(basic_re == NULL)
		return NULL;//there was an error, break out
	//now we are done if
	//there is no next token
	//or if the next token is an ALT
	if(fat_stack_size(tok_stk) == 0 || ((token*)fat_stack_peek(tok_stk))->type == ALT_TOK)
		return basic_re;
	//o.w. we must have a cat
	//so recurse, check and then cat
	ast_node* simple_node = parse_simple_re(tok_stk, er);
	if(simple_node == NULL)
	{
		free_node(basic_re);
		return NULL;
	}
	return (ast_node*) make_binary(simple_node, basic_re, CONCAT);	
}

static ast_node* parse_reg(fat_stack* tok_stk, re_error* er)
{
	//given our grammar its either
	//simple-re or reg | simple-re
	//so lets try to parse a simple test for a pipe or end and recurse
	ast_node* simple_re = parse_simple_re(tok_stk, er);
	if(simple_re == NULL)
		return NULL;//there was an error, break out
	if(fat_stack_size(tok_stk) == 0 || ((token*)fat_stack_peek(tok_stk))->type == LPAREN_TOK)
		return simple_re;
	//o.w. we must have an alt
	token* topptr = (token*) fat_stack_peek(tok_stk);
	token top = *topptr;
	fat_stack_pop(tok_stk);
	switch(top.type)
	{
		case ALT_TOK:
			;
			ast_node* reg_node = parse_reg(tok_stk, er);
			if(reg_node == NULL)
			{
				free_node(simple_re);
				return NULL;
			}
			ast_node* alt = (ast_node*) make_binary(reg_node, simple_re, ALT);
			return alt;
		case LPAREN_TOK:
			return simple_re;
		default:
			free_node(simple_re);
			return NULL;
	}
}
static inline fat_stack* read_all_tokens(lexer* lxr, re_error* er)
{
	//we need to read all of our tokens onto the stack
	fat_stack* stk = fat_stack_create(sizeof(token));
	token tok;
	while((tok = read_token(lxr)).type != END_TOK)
	{
		if(tok.type == INVALID_TOK)
		{
			if(er != NULL)
			{
				er->errno = E_INVALID_TOKEN;
				er->position = tok.position;
			}
			goto error;
		}
		fat_stack_push(stk, &tok);
	}
	return stk;
error:
	fat_stack_destroy(stk);
	return NULL;
	
}

static ast_node* parse_re(lexer* lxr, re_error* er)
{
	token tok = read_token(lxr);
	if(tok.type == END_TOK)
	{
		return make_empty();
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
	if(er != NULL)
	{
		er->errno = E_SUCCESS;
		er->position = -1;
	}
	lexer l;
	init_lexer(&l, regex);
	return parse_re(&l, er);
}

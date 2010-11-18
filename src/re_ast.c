#include "re_ast.h"
#include <stdlib.h>
#include <util/util.h>
#include <assert.h>

ast_node* make_node(node_type t)
{
	ast_node* node = (ast_node*) malloc(sizeof(ast_node));
	if(node == NULL) return NULL;
	node->type = t;
	return node;
}
unary_node* make_unary(ast_node* sub, node_type t)
{
	unary_node* node = (unary_node*) malloc(sizeof(unary_node));
	if(node == NULL) return NULL;
	node->base.type = t;
	node->expr = sub;
	return node;
}

char_node* make_char(char rl)
{
	char_node* node = (char_node*) malloc(sizeof(char_node));
	if(node == NULL) return NULL;
	node->base.type = CHAR;
	node->c = rl;
	return node;
}
binary_node* make_binary(ast_node* left, ast_node* right, node_type t)
{
	binary_node* node = (binary_node*) malloc(sizeof(binary_node));
	if(node == NULL) return NULL;
	node->base.type = t;
	node->left = left;
	node->right = right;
	return node;
}
multi_node* make_multi(node_type t)
{
	multi_node* node = (multi_node*) malloc(sizeof(multi_node));
	if(node == NULL) return NULL;
	node->base.type = t;
	node->list = make_linked_list();
	if(node->list == NULL)
	{
		free(node);
		return NULL;
	}
	return node;
}

loop_node* make_loop(ast_node* sub, unsigned int min, unsigned int max)
{
	loop_node* node = (loop_node*) malloc(sizeof(loop_node));
	if(node == NULL) return NULL;
	node->base.base.type = CREP;
	node->base.expr = sub;
	node->min = min;
	node->max = max;
	return node;
}

void free_node(ast_node* n)
{
	switch(n->type)
	{
		case QMARK:
		case PLUS:
		case STAR:
		case NG_QMARK:
		case NG_PLUS:
		case NG_STAR:
		case CAPTURE:
		case CREP:
			;//to avoid the gcc declarations after labels issue
			unary_node* sn = (unary_node*) n;
			if(sn->expr != NULL) free_node(sn->expr);
			break;
		case CONCAT:
		case ALT:
			;
			multi_node* mn = (multi_node*) n;
			while(!linked_list_is_empty(mn->list))
			{
				free_node((ast_node*) linked_list_remove_first(mn->list));
			}
			linked_list_destroy(mn->list);
			break;
		case CHAR:
		case WHITESPACE:
		case ALPHA:
		case DIGIT:
		case WILDCARD:
		case EMPTY:
			//do nothing
			break;
	}
	free(n);
}

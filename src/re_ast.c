#include "re_ast.h"
#include <stdlib.h>
#include <util/util.h>
#include <assert.h>

ast_node* make_node(node_type t)
{
	ast_node* node = NEW(ast_node);
	node->type = t;
	return node;
}
unary_node* make_unary(ast_node* sub, node_type t)
{
	unary_node* node = NEW(unary_node);
	node->base.type = t;
	node->expr = sub;
	return node;
}

char_node* make_char(char rl)
{
	char_node* node = NEW(char_node);
	node->base.type = CHAR;
	node->c = rl;
	return node;
}
binary_node* make_binary(ast_node* left, ast_node* right, node_type t)
{
	binary_node* node = NEW(binary_node);
	node->base.type = t;
	node->left = left;
	node->right = right;
	return node;
}
multi_node* make_multi(node_type t)
{
	multi_node* node = NEW(multi_node);
	node->base.type = t;
	node->list = make_linked_list();
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
			;//to avoid the gcc declarations after labels issue
			unary_node* sn = (unary_node*) n;
			if(sn->expr != NULL) free_node(sn->expr);
			break;
		case CONCAT:
			;
			multi_node* mn = (multi_node*) n;
			while(!linked_list_is_empty(mn->list))
			{
				free_node((ast_node*) linked_list_remove_first(mn->list));
			}
			linked_list_destroy(mn->list);
			break;
		case ALT:
			;//to avoid the gcc declarations after labels issue
			binary_node* cn = (binary_node*) n;
			if(cn->left != NULL) free_node(cn->left);
			if(cn->right != NULL) free_node(cn->right);
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

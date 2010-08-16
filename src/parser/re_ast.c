#include "re_ast.h"
#include <stdlib.h>
#include <util/util.h>
#include <assert.h>

ast_node* make_empty()
{
	ast_node* node = NEW(ast_node);
	node->type = EMPTY;
	return node;
}
unary_node* make_unary(ast_node* sub, node_type t)
{
	unary_node* node = NEW(unary_node);
	node->base.type = t;
	node->expr = sub;
	return node;
}

rule_node* make_rule(char_rule rl)
{
	rule_node* node = NEW(rule_node);
	node->base.type = MATCH;
	node->rule = rl;
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

void free_node(ast_node* n)
{
	switch(n->type)
	{
		case QMARK:
		case PLUS:
		case STAR:
			;//to avoid the gcc declarations after labels issue
			unary_node* sn = (unary_node*) n;
			if(sn->expr != NULL) free_node(sn->expr);
			break;
		case CONCAT:
		case ALT:
			;//to avoid the gcc declarations after labels issue
			binary_node* cn = (binary_node*) n;
			if(cn->left != NULL) free_node(cn->left);
			if(cn->right != NULL) free_node(cn->right);
			break;
		case MATCH:
		case EMPTY:
			//do nothing
			break;
	}
	free(n);
}

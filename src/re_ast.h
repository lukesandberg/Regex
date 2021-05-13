#ifndef __RE_AST_H__
#define __RE_AST_H__

#include "util/linked_list.h"
#include <stddef.h>

typedef enum
{
	CONCAT,
	ALT,
	STAR,
	QMARK,
	PLUS,
	NG_STAR,
	NG_QMARK,
	NG_PLUS,
	CHAR,
	WILDCARD,
	WHITESPACE,
	ALPHA,
	DIGIT,
	EMPTY,
	CAPTURE,
	CREP,
} node_type;

/*simple 'base' struct*/
typedef struct
{
	node_type type;
	unsigned int start_char;
	unsigned int end_char;
	size_t sub_prog_size;
} ast_node;

typedef struct
{
	ast_node base;
	char c;
} char_node;

typedef struct
{
	ast_node base;
	ast_node *expr;
} unary_node;

typedef struct
{
	unary_node base;
	unsigned int min;
	unsigned int max;
} loop_node;

typedef struct
{
	ast_node base;
	ast_node *left;
	ast_node *right;
} binary_node;

typedef struct
{
	ast_node base;
	linked_list* list;
} multi_node;

void free_node(ast_node* n);
ast_node* make_node(node_type t);
unary_node* make_unary(ast_node* sub, node_type t);
char_node* make_char(char c);
binary_node* make_binary(ast_node* left, ast_node* right, node_type t);
multi_node* make_multi(node_type t);
loop_node* make_loop(ast_node* sub, unsigned int min, unsigned int max);

#endif

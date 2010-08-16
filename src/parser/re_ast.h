#ifndef __RE_AST_H__
#define __RE_AST_H__

typedef enum
{
	RWILDCARD = 256,
	RWHITESPACE = 257,
	RALPHA = 258,
	RDIGIT = 259
} char_rule;

typedef enum
{
	CONCAT,
	STAR,
	ALT,
	QMARK,
	PLUS,
	MATCH,
	EMPTY
} node_type;

/*simple 'base' struct*/
typedef struct
{
	node_type type;
} ast_node;

typedef struct
{
	ast_node base;
	char_rule rule;
} rule_node;

typedef struct
{
	ast_node base;
	ast_node *expr;
} unary_node;

typedef struct
{
	ast_node base;
	ast_node *left;
	ast_node *right;
} binary_node;

void free_node(ast_node* n);
ast_node* make_empty();
unary_node* make_unary(ast_node* sub, node_type t);
rule_node* make_rule(char_rule rl);
binary_node* make_binary(ast_node* left, ast_node* right, node_type t);

#endif

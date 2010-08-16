#include <re_compiler.h>
#include <parser/re_parser.h>
#include <util/util.h>

#include <stdlib.h>
#include <assert.h>


static inline void compile_concat(instruction* inst, unsigned int* loc, binary_node* n);
static inline void compile_alt(instruction* inst, unsigned int* loc, binary_node* n);
static inline void compile_plus(instruction* inst, unsigned int* loc, unary_node* n);
static inline void compile_qmark(instruction* inst, unsigned int* loc, unary_node* n);
static inline void compile_star(instruction* inst, unsigned int* loc, unary_node* n);
static inline void compile_match(instruction* inst, unsigned int* loc, rule_node* n);

static void compile_recursive(instruction* inst, unsigned int* loc, ast_node* n)
{
	switch(n->type)
	{
		case MATCH:
			compile_match(inst, loc, (rule_node*) n);
			break;
		case STAR:
			compile_star(inst, loc, (unary_node*) n);
			break;
		case PLUS:
			compile_plus(inst, loc, (unary_node*) n);
			break;
		case QMARK:
			compile_qmark(inst, loc, (unary_node*) n);
			break;
		case CONCAT:	
			compile_concat(inst, loc,  (binary_node*) n);
			break;
		case ALT:
			compile_alt(inst, loc, (binary_node*) n);
			break;
		case EMPTY:
			//do nothing
			break;
	}
}

/*
e?:
    split L1, L2
L1: codes for e
L2:
 */
static inline void compile_qmark(instruction* inst, unsigned int* loc, unary_node* n)
{
	unsigned int split_location = *loc;
	instruction* split = inst + split_location;
	split->op = I_SPLIT;
	split->v.split.left = split_location + 1;//L2
	compile_recursive(inst, loc, n->expr);
	split->v.split.right = *loc;
}

/*
e+:
L1: codes for e
    split L1, L3
L3:
*/
static inline void compile_plus(instruction* inst, unsigned int* loc, unary_node* n)
{
	unsigned int L1 = *loc;
	compile_recursive(inst, loc, n->expr);

	unsigned int split_location = *loc;
	instruction* split = inst + split_location;
	split->op = I_SPLIT;
	split->v.split.left = L1;
	split->v.split.right = split_location +1;//L2
	(*loc)++;
}


/*
e1|e2:
    split L1, L2
L1: codes for e1
    jmp L3
L2: codes for e2
L3:
   */
static void compile_alt(instruction* inst, unsigned int* loc, binary_node* n)
{
	unsigned int split_location = *loc;
	instruction* split = inst + split_location;
	split->op = I_SPLIT;
	split->v.split.left = split_location + 1;//L1
	(*loc)++;
	compile_recursive(inst, loc, n->left);//codes for e1
	
	unsigned int jmp_location = *loc;
	split->v.split.right = jmp_location +1;//L2

	instruction* jmp = inst + jmp_location;//jmp L3
	jmp->op = I_JMP;
	(*loc)++;

	compile_recursive(inst, loc, n->right);//codes for e2
	jmp->v.jump = *loc;
}

/*
e1e2:
   codes for e1
   codes for e2
*/
static inline void compile_concat(instruction* inst, unsigned int* loc, binary_node* n)
{
	compile_recursive(inst, loc, n->left);
	compile_recursive(inst, loc, n->right);
}
/*

e*:
L1: split L2, L3
L2: codes for e
    jmp L1
L3:
*/
static inline void compile_star(instruction* inst, unsigned int* loc, unary_node* n)
{
	unsigned int split_location = *loc;//L1
	instruction* split = inst + split_location; //split L2 L3
	split->op = I_SPLIT;
	split->v.split.left = split_location + 1;//L2	
	(*loc)++;
	compile_recursive(inst, loc, n->expr);
	
	unsigned int jmp_location = *loc;
	split->v.split.right = jmp_location + 1;//L3

	instruction* jmp = inst + jmp_location;//jmp L1
	jmp->op = I_JMP;
	jmp->v.jump = split_location;
	(*loc)++;
}
/*
a:
char a
*/
static inline void compile_match(instruction* inst, unsigned int* loc, rule_node* n)
{
	instruction* rule = inst + (*loc);
	rule->op = I_RULE;
	rule->v.rule = n->rule;
	(*loc)++;
}

static ast_node* optimize(ast_node* n)
{
	//default null optimization
	return n;
}

//our algorithm is deterministic, we can determine exactly how many 
//instructions we will generate from the syntax tree
static size_t tree_size(ast_node* n)
{	
	switch(n->type)
	{
		case PLUS:
			//plus's are the sub exp plus a split
			return 1 + tree_size(((unary_node*)n)->expr);
		case QMARK:
			//qmarks are the sub exp plus a jmp
			return 1 + tree_size(((unary_node*)n)->expr);
		case ALT:
			//alts are a jmp and a split plus both subs
			return 2 + tree_size(((binary_node*) n)->left) + tree_size(((binary_node*)n)->right);
		case CONCAT:
			//cats are just both sequences smashed together
			return tree_size(((binary_node*) n)->left) + tree_size(((binary_node*)n)->right);
		case STAR:
			//stars are the sub exp plus a jmp and a split
			return 2 + tree_size(((unary_node*)n)->expr);
		case MATCH:
			return 1;
		case EMPTY:
			return 0;
	}
	dassert(0, "unexpected case");
	return 0;
}

program* compile_regex(char* str, re_error* error)
{
	ast_node* tree = re_parse(str, error);
	if(tree == NULL) return NULL;//there was an error during parsing

	tree = optimize(tree);

	size_t sz = tree_size(tree) + 1;//we add one for the final match inst

	program* prog = checked_malloc(sizeof(program) + sz*sizeof(instruction));
	unsigned int loc = 0;
	prog->size = sz;
	compile_recursive(&(prog->code[0]), &loc, tree);
	prog->code[loc].op = I_MATCH;
	free_node(tree);
	return prog;
}



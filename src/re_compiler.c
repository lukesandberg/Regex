#include <re_compiler.h>
#include <re_parser.h>
#include <util/util.h>

#include <stdlib.h>
#include <assert.h>


static inline void compile_concat(instruction* inst, size_t* loc, multi_node* n);
static inline void compile_alt(instruction* inst, size_t* loc, binary_node* n);
static inline void compile_plus(instruction* inst, size_t* loc, unary_node* n, int ng);
static inline void compile_qmark(instruction* inst, size_t* loc, unary_node* n, int ng);
static inline void compile_star(instruction* inst, size_t* loc, unary_node* n, int ng);
static inline void compile_char(instruction* inst, size_t* loc, char_node* n);

static void compile_recursive(instruction* inst, size_t* loc, ast_node* n)
{
	switch(n->type)
	{
		case CHAR:
			compile_char(inst, loc, (char_node*) n);
			break;
		case STAR:
		case NG_STAR:
			compile_star(inst, loc, (unary_node*) n, n->type == NG_STAR);
			break;
		case PLUS:
		case NG_PLUS:
			compile_plus(inst, loc, (unary_node*) n, n->type == NG_PLUS);
			break;
		case QMARK:
		case NG_QMARK:
			compile_qmark(inst, loc, (unary_node*) n, n->type == NG_QMARK);
			break;
		case CONCAT:	
			compile_concat(inst, loc,  (multi_node*) n);
			break;
		case ALT:
			compile_alt(inst, loc, (binary_node*) n);
			break;
		case WILDCARD:
			(inst + (*loc))->op = I_WILDCARD;
			(*loc)++;
			break;
		case ALPHA:
			(inst + (*loc))->op = I_ALPHA;
			(*loc)++;
			break;
		case DIGIT:
			(inst + (*loc))->op = I_DIGIT;
			(*loc)++;
			break;
		case WHITESPACE:
			(inst + (*loc))->op = I_WHITESPACE;
			(*loc)++;
			break;
		case EMPTY:
			//do nothing
			break;
	}
}

static inline void flip_split(instruction* split)
{
	instruction*tmp = split->v.split.left;
	split->v.split.left = split->v.split.right;
	split->v.split.right = tmp;
}

/*
e?:
    split L1, L2
L1: codes for e
L2:
 */
static inline void compile_qmark(instruction* inst, size_t* loc, unary_node* n, int ng)
{
	size_t split_location = *loc;
	instruction* split = inst + split_location;
	split->op = I_SPLIT;
	split->v.split.left = inst + (split_location + 1);//L1
	(*loc)++;
	compile_recursive(inst, loc, n->expr);
	split->v.split.right = inst + (*loc);//L2
	if(ng) flip_split(split);
}

/*
e+:
L1: codes for e
    split L1, L3
L3:
*/
static inline void compile_plus(instruction* inst, size_t* loc, unary_node* n, int ng)
{
	size_t L1 = *loc;
	compile_recursive(inst, loc, n->expr);

	size_t split_location = *loc;
	size_t L3 = split_location + 1;
	instruction* split = inst + split_location;
	split->op = I_SPLIT;
	split->v.split.left = inst + L1;
	split->v.split.right = inst + L3;
	(*loc)++;
	if(ng) flip_split(split);
}

/*
e1|e2:
    split L1, L2
L1: codes for e1
    jmp L3
L2: codes for e2
L3:
   */
static void compile_alt(instruction* inst, size_t* loc, binary_node* n)
{
	size_t split_location = *loc;
	instruction* split = inst + split_location;
	split->op = I_SPLIT;
	split->v.split.left = inst + (split_location + 1);//L1
	(*loc)++;
	compile_recursive(inst, loc, n->left);//codes for e1
	
	size_t jmp_location = *loc;
	split->v.split.right = inst + (jmp_location +1);//L2

	instruction* jmp = inst + jmp_location;//jmp L3
	jmp->op = I_JMP;
	(*loc)++;

	compile_recursive(inst, loc, n->right);//codes for e2
	jmp->v.jump = inst + (*loc);//L3
}

/*
e1e2:
   codes for e1
   codes for e2
*/
static inline void compile_concat(instruction* inst, size_t* loc, multi_node* n)
{
	//technically the concat operator only strings two expressions together
	//but in order to limit the depth of the tree we treat it as an operation
	//over an arbitrary number of nodes
	linked_list_node* c = linked_list_first(n->list);
	while(c != NULL)
	{
		compile_recursive(inst, loc, (ast_node*) linked_list_value(c));
		c = linked_list_next(c);
	}
}
/*

e*:
L1: split L2, L3
L2: codes for e
    jmp L1
L3:
*/
static inline void compile_star(instruction* inst, size_t* loc, unary_node* n, int ng)
{
	instruction* split = inst + (*loc); //split L2 L3
	split->op = I_SPLIT;
	split->v.split.left = split + 1;//L2	
	(*loc)++;
	compile_recursive(inst, loc, n->expr);
	
	instruction* jmp = inst + (*loc);//jmp L1
	split->v.split.right = jmp + 1;//L3
	jmp->op = I_JMP;
	jmp->v.jump = split;
	(*loc)++;
	if(ng) flip_split(split);
}
/*
a:
char a
*/
static inline void compile_char(instruction* inst, size_t* loc, char_node* n)
{
	instruction* rule = inst + (*loc);
	rule->op = I_CHAR;
	rule->v.c = n->c;
	(*loc)++;
}

static ast_node* optimize(ast_node* n)
{
	//default null optimization
	return n;
}

//our algorithm is deterministic, we can determine exactly how many 
//instructions we will generate from the syntax tree
static size_t program_size(ast_node* n)
{	
	switch(n->type)
	{
		case NG_PLUS:
		case PLUS:
			//plus's are the sub exp plus a split
			return 1 + program_size(((unary_node*)n)->expr);
		case QMARK:
		case NG_QMARK:
			//qmarks are the sub exp plus a jmp
			return 1 + program_size(((unary_node*)n)->expr);
		case ALT:
			//alts are a jmp and a split plus both subs
			return 2 + program_size(((binary_node*) n)->left) + program_size(((binary_node*)n)->right);
		case CONCAT:
			;
			//cats are just the sum of all the sub_sequences
			size_t sz = 0;
			multi_node* mn = (multi_node*) n;
			linked_list_node* ln = linked_list_first(mn->list);
			while(ln != NULL)
			{
				sz += program_size((ast_node*) linked_list_value(ln));
				ln = linked_list_next(ln);
			}
			return sz;
		case STAR:
		case NG_STAR:
			//stars are the sub exp plus a jmp and a split
			return 2 + program_size(((unary_node*)n)->expr);
		case CHAR:
		case WILDCARD:
		case ALPHA:
		case DIGIT:
		case WHITESPACE:
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
	if(tree == NULL)
	       	return NULL;//there was an error during parsing

	tree = optimize(tree);

	size_t sz = program_size(tree) + 1;//we add one for the final match inst

	program* prog = malloc(sizeof(program) + sz*sizeof(instruction));
	if(prog == NULL)
	{
		if(error != NULL)
		{
			error->errno = E_OUT_OF_MEMORY;
			error->position = -1;
		}
		goto end;
	}
	size_t loc = 0;
	prog->size = sz;
	compile_recursive(&(prog->code[0]), &loc, tree);
	prog->code[loc].op = I_MATCH;
end:
	free_node(tree);
	return prog;
}



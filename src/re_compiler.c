#include <re_compiler.h>
#include <re_parser.h>
#include <util/util.h>

#include <stdlib.h>
#include <assert.h>

struct compile_state
{
	instruction* inst;
	size_t next_save_reg;
};

static inline void compile_concat(struct compile_state *state, multi_node* n);
static inline void compile_alt(struct compile_state *state, binary_node* n);
static inline void compile_plus(struct compile_state *state, unary_node* n, int ng);
static inline void compile_qmark(struct compile_state *state, unary_node* n, int ng);
static inline void compile_capture(struct compile_state *state, unary_node* n);
static inline void compile_star(struct compile_state *state, unary_node* n, int ng);
static inline void compile_char(struct compile_state *state, char_node* n);
static inline void compile_op(struct compile_state *state, op_code op);
static void compile_recursive(struct compile_state *state, ast_node* n)
{
	switch(n->type)
	{
		case CHAR:
			compile_char(state, (char_node*) n);
			break;
		case STAR:
		case NG_STAR:
			compile_star(state, (unary_node*) n, n->type == NG_STAR);
			break;
		case PLUS:
		case NG_PLUS:
			compile_plus(state, (unary_node*) n, n->type == NG_PLUS);
			break;
		case QMARK:
		case NG_QMARK:
			compile_qmark(state, (unary_node*) n, n->type == NG_QMARK);
			break;
		case CONCAT:	
			compile_concat(state,  (multi_node*) n);
			break;
		case ALT:
			compile_alt(state, (binary_node*) n);
			break;
		case WILDCARD:
			compile_op(state, I_WILDCARD);
			break;
		case ALPHA:
			compile_op(state, I_ALPHA);
			break;
		case DIGIT:
			compile_op(state, I_DIGIT);
			break;
		case WHITESPACE:
			compile_op(state, I_WHITESPACE);
			break;
		case CAPTURE:
			compile_capture(state, (unary_node*) n);
			break;
		case EMPTY:
			//do nothing
			break;
	}
}

static inline void compile_capture(struct compile_state *state, unary_node* n)
{
	instruction* save_1 = state->inst;
	save_1->op = I_SAVE;
	save_1->v.save_register = state->next_save_reg;
	state->next_save_reg++;
	state->inst++;	
	
	compile_recursive(state, n->expr);

	instruction* save_2 = state->inst;
	save_2->op = I_SAVE;
	save_2->v.save_register = state->next_save_reg;
	state->next_save_reg++;
	state->inst++;	
}

static inline void compile_op(struct compile_state *state, op_code op)
{
	state->inst->op = op;
	state->inst++;
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
static inline void compile_qmark(struct compile_state *state, unary_node* n, int ng)
{
	instruction* split = state->inst;
	split->op = I_SPLIT;
	state->inst++;
	split->v.split.left = state->inst;//L1
	compile_recursive(state, n->expr);
	split->v.split.right = state->inst;//L2
	if(ng) flip_split(split);
}

/*
e+:
L1: codes for e
    split L1, L3
L3:
*/
static inline void compile_plus(struct compile_state *state, unary_node* n, int ng)
{
	instruction* L1 = state->inst;

	compile_recursive(state, n->expr);
	
	instruction* split = state->inst;
	split->op = I_SPLIT;
	split->v.split.left = L1;
	state->inst++;
	split->v.split.right = state->inst;//L3
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
static void compile_alt(struct compile_state *state, binary_node* n)
{
	instruction* split = state->inst;
	split->op = I_SPLIT;
	state->inst++;
	split->v.split.left = state->inst;//L1
	compile_recursive(state, n->left);//codes for e1
	
	instruction* jmp = state->inst;//jmp L3
	jmp->op = I_JMP;
	state->inst++;
	split->v.split.right = state->inst;

	compile_recursive(state, n->right);//codes for e2
	jmp->v.jump = state->inst;//L3
}

/*
e1e2:
   codes for e1
   codes for e2
*/
static inline void compile_concat(struct compile_state *state, multi_node* n)
{
	//technically the concat operator only strings two expressions together
	//but in order to limit the depth of the tree we treat it as an operation
	//over an arbitrary number of nodes
	linked_list_node* c = linked_list_first(n->list);
	while(c != NULL)
	{
		compile_recursive(state, (ast_node*) linked_list_value(c));
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
static inline void compile_star(struct compile_state* state, unary_node* n, int ng)
{
	instruction* split = state->inst; //split L2 L3
	split->op = I_SPLIT;
	state->inst++;
	split->v.split.left = state->inst;//L2	
	compile_recursive(state, n->expr);
	
	instruction* jmp = state->inst;//jmp L1
	state->inst++;
	split->v.split.right = state->inst;//L3
	jmp->op = I_JMP;
	jmp->v.jump = split;
	if(ng) flip_split(split);
}
/*
a:
char a
*/
static inline void compile_char(struct compile_state *state, char_node* n)
{
	instruction* rule = state->inst;
	rule->op = I_CHAR;
	rule->v.c = n->c;
	state->inst++;
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
		case CAPTURE:
			return 2 + program_size(((unary_node*)n)->expr);
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

program* compile_regex(char* str, re_error* error, size_t *num_save_regs)
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
	prog->size = sz;
	
	struct compile_state state;
	state.inst = &(prog->code[0]);
	state.next_save_reg = 0;

	compile_recursive(&state, tree);
	state.inst->op = I_MATCH;//last instruction
	*num_save_regs = state.next_save_reg;
end:
	free_node(tree);
	return prog;
}



#include <re_compiler.h>
#include <re_parser.h>
#include <util/util.h>

#include <stdlib.h>
#include <string.h>

struct compile_state
{
	instruction* first;
	instruction* inst;
	size_t next_save_reg;
	size_t max_loop_vars;
	size_t next_loop_var;
};
static size_t program_size(ast_node* n);
static inline void compile_concat(struct compile_state *state, multi_node* n);
static inline void compile_alt(struct compile_state *state, multi_node* n);
static inline void compile_plus(struct compile_state *state, unary_node* n, int ng);
static inline void compile_qmark(struct compile_state *state, unary_node* n, int ng);
static inline void compile_capture(struct compile_state *state, unary_node* n);
static inline void compile_star(struct compile_state *state, unary_node* n, int ng);
static inline void compile_char(struct compile_state *state, char_node* n);
static inline void compile_op(struct compile_state *state, op_code op);
static inline void compile_crep(struct compile_state *state, loop_node* n);
static void compile_recursive(struct compile_state *state, ast_node* n)
{
	switch(n->type)
	{
		case CHAR:
			compile_char(state, (char_node*) n);
			break;
		case CREP:
			compile_crep(state, (loop_node*) n);
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
			compile_alt(state, (multi_node*) n);
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
	state->next_save_reg += 2;
	size_t nsr = state->next_save_reg -1;//we need to save it now in case ther are nested captures
	state->inst++;
	
	compile_recursive(state, n->expr);

	instruction* save_2 = state->inst;
	save_2->op = I_SAVE;
	save_2->v.save_register = nsr;
	state->inst++;	
}

static inline void compile_op(struct compile_state *state, op_code op)
{
	state->inst->op = op;
	state->inst++;
}

static inline void flip_split(instruction* split)
{
	unsigned int tmp = split->v.split.left;
	split->v.split.left = split->v.split.right;
	split->v.split.right = tmp;
}

/*
e{n,m}:
    SETZ idx
L1: SPLIT L2 L3
L2: DGT idx M
    codes for e
    INCR idx
    JMP L1
L3: DLT idx N 
*/
static inline void compile_crep(struct compile_state *state, loop_node* n)
{
	unsigned int reg = state->next_loop_var;
	state->next_loop_var++;
	if(state->next_loop_var > state->max_loop_vars)
	{
		state->max_loop_vars = state->next_loop_var;
	}
	instruction* setz = state->inst;
	setz->op = I_SETZ;
	setz->v.idx = reg;
	state->inst++;
	
	unsigned int L1 = state->inst - state->first;
	instruction* split = state->inst;
	split->op = I_SPLIT;
	state->inst++;
	
	unsigned int L2 = state->inst - state->first;
	split->v.split.left = L2;

	instruction* dgt = state->inst;
	dgt->op = I_DGTEQ;
	dgt->v.comparison.idx = reg;
	dgt->v.comparison.comp = n->max;
	state->inst++;

	compile_recursive(state, n->base.expr);

	instruction *incr = state->inst;
	incr->op = I_INCR;
	incr->v.idx = reg;
	state->inst++;

	instruction *jmp = state->inst;
	jmp->op = I_JMP;
	jmp->v.jump = L1;
	state->inst++;

	unsigned int L3 = state->inst - state->first;
	split->v.split.right = L3;

	instruction* dlt = state->inst;
	dlt->op = I_DLT;
	dlt->v.comparison.idx = reg;
	dlt->v.comparison.comp = n->min;
	state->inst++;

	state->next_loop_var--;
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
	split->v.split.left = state->inst - state->first;//L1
	compile_recursive(state, n->expr);
	split->v.split.right = state->inst - state->first;//L2
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
	unsigned int L1 = state->inst - state->first;

	compile_recursive(state, n->expr);
	
	instruction* split = state->inst;
	split->op = I_SPLIT;
	split->v.split.left = L1;
	state->inst++;
	split->v.split.right = state->inst - state->first;//L3
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
/*
 * e1|e2|e3
 *
 * 	split L1 LD1
 * LD1:	split L2 L3
 * L1:	codes for e1
 * 	jmp end
 * L2: 	codes for e2
 * 	jmp end
 * L3:	codes for e3
 * end:
 */
static void compile_alt(struct compile_state *state, multi_node* n)
{//we know that there are at least two items on this list

	unsigned int end = program_size((ast_node*)n) + state->inst - state->first;
	
	linked_list_node* last = linked_list_last(n->list);
	linked_list_node* first = linked_list_first(n->list);
	linked_list_node* current = first;
	linked_list_node* next = linked_list_next(current);
	instruction* first_split = state->inst;
	while(next != last)
	{
		instruction* split = state->inst;
		split->op = I_SPLIT;
		state->inst++;
		split->v.split.right = state->inst - state->first;
		current = next;
		next = linked_list_next(next);
	}
	instruction* last_split = state->inst;
	last_split->op = I_SPLIT;
	state->inst++;
	
	current = first;
	next = linked_list_next(current);
	instruction* cur_split = first_split;
	while(current != NULL)
	{
		unsigned int li = state->inst - state->first;
		if(current == last)
		{
			cur_split->v.split.right = li;
		}
		else
		{
			cur_split->v.split.left = li;
		}
		compile_recursive(state, (ast_node*) linked_list_value(first));
		
		if(current != last)
		{
			//the last one doesnt get a jmp
			state->inst->op = I_JMP;
			state->inst->v.jump = end;
			state->inst++;
		}
		current = next;
		next = linked_list_next(next);
		if(next != last)
			cur_split++;
	}
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
	split->v.split.left = state->inst - state->first;//L2	
	compile_recursive(state, n->expr);
	
	instruction* jmp = state->inst;//jmp L1
	state->inst++;
	split->v.split.right = state->inst - state->first;//L3
	jmp->op = I_JMP;
	jmp->v.jump = split - state->first;
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
		case CREP:
			return 6 + program_size(((unary_node*)n)->expr);
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
static inline void modify_indices(instruction* inst, size_t len, size_t offset)
{
	for(unsigned int i = 0; i < len; i++)
	{
		op_code op = inst->op;
		if(op == I_DGTEQ || op == I_DLT)
		{
			inst->v.comparison.idx += offset;
		}
		else if(op == I_SETZ || op == I_INCR)
		{
			inst->v.idx += offset;
		}
		inst++;
	}
}


program* compile_regex(char* str, re_error* error, size_t *num_regs, size_t* num_capture_regs)
{
	ast_node* tree = re_parse(str, error);
	if(tree == NULL)
	       	return NULL;//there was an error during parsing

	tree = optimize(tree);

	size_t sz = program_size(tree) + 1;//we add one for the final match inst
	program* prog = malloc(sizeof(program) + sz * sizeof(instruction));
	
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
	state.first = &(prog->code[0]);
	state.inst = state.first;
	state.next_save_reg = 0;
	state.max_loop_vars = 0;
	state.next_loop_var = 0;

	compile_recursive(&state, tree);
	compile_op(&state, I_MATCH);//last instruction should always be a match
	*num_regs = state.next_save_reg  + state.max_loop_vars;
	*num_capture_regs = state.next_save_reg;
	
	modify_indices(state.first, sz, state.next_save_reg);

end:
	free_node(tree);
	return prog;
}



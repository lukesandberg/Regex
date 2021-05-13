#include "re_compiler.h"
#include "re_parser.h"
#include "util/util.h"

#include <stdlib.h>
#include <string.h>
#include "regex.h"

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
static inline void compile_crep(struct compile_state *state, loop_node* n);

static inline unsigned int current_index(struct compile_state* state);
static inline void op(struct compile_state *state, op_code op);
static inline void jmp(struct compile_state * state, unsigned int target);
static inline void incr(struct compile_state * state, unsigned int reg);
static inline void setz(struct compile_state * state, unsigned int reg);
static inline void save(struct compile_state * state, unsigned int sr);
static inline void comp(struct compile_state* state,  op_code op, unsigned int reg, unsigned int comp);
static inline void split(struct compile_state* state, unsigned int left, unsigned int right, int ng);
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
			op(state, I_WILDCARD);
			break;
		case ALPHA:
			op(state, I_ALPHA);
			break;
		case DIGIT:
			op(state, I_DIGIT);
			break;
		case WHITESPACE:
			op(state, I_WHITESPACE);
			break;
		case CAPTURE:
			compile_capture(state, (unary_node*) n);
			break;
		case EMPTY:
			//do nothing
			break;
	}
}

static inline unsigned int current_index(struct compile_state* state)
{
	return state->inst - state->first;
}

static inline void op(struct compile_state *state, op_code op)
{
	state->inst->op = op;
	state->inst++;
}

static inline void save(struct compile_state * state, unsigned int sr)
{
	state->inst->op = I_SAVE;
	state->inst->v.save_register = sr;
	state->inst++;
}

static inline void jmp(struct compile_state * state, unsigned int target)
{
	state->inst->op = I_JMP;
	state->inst->v.jump = target;
	state->inst++;
}

static inline void incr(struct compile_state * state, unsigned int reg)
{
	state->inst->op = I_INCR;
	state->inst->v.idx = reg;
	state->inst++;
}
static inline void setz(struct compile_state * state, unsigned int reg)
{
	state->inst->op = I_SETZ;
	state->inst->v.idx = reg;
	state->inst++;
}

static inline void comp(struct compile_state* state,  op_code op, unsigned int reg, unsigned int comp)
{
	instruction* dgt = state->inst;
	dgt->op = op;
	dgt->v.comparison.idx = reg;
	dgt->v.comparison.comp = comp;
	state->inst++;
}
static inline void split(struct compile_state* state, unsigned int left, unsigned int right, int ng)
{
	instruction* s = state->inst;
	s->op = I_SPLIT;
	s->v.split.left = ng ? right : left;
	s->v.split.right = ng ? left : right;
	state->inst++;
}

static inline void compile_capture(struct compile_state *state, unary_node* n)
{
	save(state, state->next_save_reg);
	
	state->next_save_reg += 2;
	size_t nsr = state->next_save_reg -1;//we need to save it now in case there are nested captures
	
	compile_recursive(state, n->expr);
	
	save(state, nsr);
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
	setz(state, reg);
	
	unsigned int L1 = current_index(state);
	unsigned int L2 = L1 + 1;
	unsigned int L3 = L2 + n->base.expr->sub_prog_size + 3;//plus 3 for the incr, jmp and dlt
	split(state, L2, L3, 0);
	comp(state, I_DGTEQ, reg, n->max);
	compile_recursive(state, n->base.expr);
	incr(state, reg);
	jmp(state, L1);
	comp(state, I_DLT, reg, n->min);
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
	unsigned int L1 = current_index(state) + 1;
	unsigned int L2 = L1 + n->expr->sub_prog_size;
	split(state, L1, L2, ng);
	compile_recursive(state, n->expr);
}

/*
e+:
L1: codes for e
    split L1, L2
L2:
*/
static inline void compile_plus(struct compile_state *state, unary_node* n, int ng)
{
	unsigned int L1 = current_index(state);

	compile_recursive(state, n->expr);
	unsigned int L2 = current_index(state) + 1;
	split(state, L1, L2, ng);
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
	unsigned int start = current_index(state);
	unsigned int end = n->base.sub_prog_size + start;
	
	linked_list_node* last = linked_list_last(n->list);
	linked_list_node* first = linked_list_first(n->list);
	linked_list_node* current = first;
	
	unsigned int Li = start + linked_list_size(n->list) - 1;
	unsigned int LDi = start + 1;
	for(linked_list_node* next = linked_list_next(current); next != last; current = next, next = linked_list_next(next))
	{
		split(state, Li, LDi, 0);
		LDi++;
		Li += ((ast_node*) linked_list_value(current))->sub_prog_size + 1;
	}
	unsigned int Ln = Li + ((ast_node*) linked_list_value(current))->sub_prog_size + 1;
	split(state, Li, Ln, 0);
	
	current = first;
	for(current = first; current != last; current = linked_list_next(current))
	{
		compile_recursive(state, (ast_node*) linked_list_value(current));
		//every one but the last gets a jmp
		jmp(state, end);
	}
	compile_recursive(state, (ast_node*) linked_list_value(current));
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
	unsigned int L1 = current_index(state);
	unsigned int L2 = L1 + 1;
	unsigned int L3 = L2 + n->expr->sub_prog_size + 1;

	split(state, L2, L3, ng);
	compile_recursive(state, n->expr);
	jmp(state, L1);
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
	size_t sz = 0;
	switch(n->type)
	{
		case CREP:
			sz = 6 + program_size(((unary_node*)n)->expr);
			break;
		case NG_PLUS:
		case PLUS:
			//plus's are the sub exp plus a split
			sz = 1 + program_size(((unary_node*)n)->expr);
			break;
		case QMARK:
		case NG_QMARK:
			//qmarks are the sub exp plus a jmp
			sz = 1 + program_size(((unary_node*)n)->expr);
			break;
		case CAPTURE:
			sz = 2 + program_size(((unary_node*)n)->expr);
			break;
		case ALT:
			;
			//each alt part has a split and a jmp except the last
			//so add up each part + 2 then subtract 2
			sz = -2;
			linked_list_node* aln = linked_list_first(((multi_node*) n)->list);
			while(aln  != NULL)
			{
				sz += program_size((ast_node*) linked_list_value(aln)) + 2;
				aln = linked_list_next(aln);
			}
			break;
		case CONCAT:
			;
			//cats are just the sum of all the sub_sequences
			linked_list_node* cln = linked_list_first(((multi_node*) n)->list);
			while(cln != NULL)
			{
				sz += program_size((ast_node*) linked_list_value(cln));
				cln = linked_list_next(cln);
			}
			break;
		case STAR:
		case NG_STAR:
			//stars are the sub exp plus a jmp and a split
			sz = 2 + program_size(((unary_node*)n)->expr);
			break;
		case CHAR:
		case WILDCARD:
		case ALPHA:
		case DIGIT:
		case WHITESPACE:
			sz = 1;
			break;
		case EMPTY:
			sz = 0;
			break;
	}
	n->sub_prog_size = sz;
	return sz;
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

regex* compile_regex(ast_node* tree)
{
	tree = optimize(tree);
	
	size_t sz = program_size(tree) + 1;//we add one for the final match inst
	regex* re = NEWE(regex, sz * sizeof(instruction));
	if(re == NULL)
	{
		return NULL;
	}
	re->prog.size = sz;
	
	struct compile_state state;
	state.first = &(re->prog.code[0]);
	state.inst = state.first;
	state.next_save_reg = 0;
	state.max_loop_vars = 0;
	state.next_loop_var = 0;

	compile_recursive(&state, tree);
	op(&state, I_MATCH);//last instruction should always be a match
	re->num_registers = state.next_save_reg  + state.max_loop_vars;
	re->num_capture_regs = state.next_save_reg;
	
	modify_indices(state.first, sz, state.next_save_reg);

	return re;
}



#include <re_compiler.h>
#include <re_ast.h>
#include <regex.h>
#include <util/util.h>

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

struct regex_s
{
	char* src;
	size_t num_registers;
	program *prog;
};

typedef struct
{
	instruction *pc;
	char* save_regs[];
} thread_t;

typedef struct 
{
	size_t size;
	size_t cap;
	thread_t* threads[];
} thread_list;

struct re_run_state
{
	thread_list *cache;
	regex* re;
};

static thread_list* make_list(size_t sz)
{
	thread_list* lst = (thread_list*) malloc(sizeof(thread_list) + sizeof(thread_t*) * sz);
	if(lst == NULL) return NULL;
	lst->size = 0;
	lst->cap = sz;
	return lst;
}

static thread_t* thread(struct re_run_state *state, instruction* pc, char** regs)
{
	size_t regsz = state->re->num_registers * sizeof(char*);
	thread_t* rval = NULL;
	if(state->cache->size > 0)
	{
		state->cache->size--;
		rval = state->cache->threads[state->cache->size]
	}
	else
	{
		rval = (thread_t*) malloc(sizeof(thread_t) + regsz);
		if(rval == NULL) return NULL;
	}
	rval->pc = pc;
	memcpy(&(rval->save_regs[0]), regs, regsz);
	return rval;
}

static inline void add_to_list(struct re_run_state *state, thread_list* l, thread_t* t, char* v)
{
tailcall:
	if(t->pc->tag != v)
	{
		l->threads[l->size] = t;
		l->size++;
		t->pc->tag = v;
		switch(t->pc->op)
		{
			case I_JMP:
				t = thread(state, t.pc->v.jump, &(t->save_regs[0]));
				goto tailcall;
			case I_SPLIT:
				add_to_list(l, thread(state, t.pc->v.split.left, &(t->save_regs[0])), v);
				t = thread(state, t.pc->v.split.right, &(t->save_regs[0]));
				goto tailcall;
			case I_SAVE:
				t->save_regs[t->pc->v.save_register] = v;
				t= thread(state, t->pc + 1, &(t->save_regs[0]));
				goto tailcall;
			case I_CHAR:
			case I_WHITESPACE:
			case I_WILDCARD:
			case I_ALPHA:
			case I_DIGIT:
			case I_MATCH:
				//fallthrough, we could just give an empty default case
				//but this will give us warnings if we add new instructions
				//to make sure it is properly handled here
				;
		}
	}
}


regex* regex_create(char* re_str, re_error* er)
{
	size_t num_regs;
	program *prog = compile_regex(re_str, er, &num_regs);
	if(prog == NULL) 
		return NULL;
	regex * re = (regex*) malloc(sizeof(regex) + (num_regs * sizeof(char*)));
	if(re == NULL)
	{
		if(er != NULL)
		{
			er->errno = E_OUT_OF_MEMORY;
			er->position = -1;
		}
		free(prog);
		return NULL;
	}
	re->num_registers = num_regs;
	re->src = re_str;
	re->prog = prog;
	return re;
}

void regex_destroy(regex* re)
{
	free(re->prog);
	free(re);
}

static inline void reset_regex(regex* re)
{
	program *prog = re->prog;
	size_t len = prog->size;
	instruction* code = &(prog->code[0]);
	for(unsigned int i = 0; i < len; i++)
		code[i].tag = NULL;
}

int regex_matches(regex* re, char*str)
{
	program *prog = re->prog;
	size_t len = prog->size;
	struct re_run_state state;
	state.re = re;
	state.cache = make_list(len);
	if(state.cache == NULL)
		return -1;
	instruction* code = &(prog->code[0]);
	thread_list *clst, *nlst;
	clst = make_list(len);
	if(clst == NULL)
	{
		free(state.cache);
		return -1;
	}
	nlst = make_list(len);
	if(nlst == NULL)
	{
		free(state.cache);
		free(clst);
		return -1;
	}

	char* c = str;
	int rval;
	add_to_list(clst, thread(code),c);
	do
	{
		rval = 0;
		for(int ti = 0; ti < clst->size; ti++)
		{
			instruction* pc = clst->threads[ti]->pc;
			switch(pc->op)
			{
				case I_CHAR:
					if(pc->v.c == *c)
						add_to_list(nlst, thread(pc + 1), c + 1);
					break;
				case I_ALPHA:
					if(isalpha(*c))
						add_to_list(nlst, thread(pc + 1), c + 1);
					break;
				case I_WHITESPACE:
					if(isspace(*c))
						add_to_list(nlst, thread(pc + 1), c + 1);
					break;
				case I_DIGIT:
					if(isdigit(*c))
						add_to_list(nlst, thread(pc + 1), c + 1);
					break;
				case I_WILDCARD:
					add_to_list(nlst, thread(pc + 1), c + 1);
					break;
				case I_MATCH:
					rval = 1;
					break;
				case I_JMP:
				case I_SPLIT:
					;
					//skip over control flow
			}
		}
		thread_list* tmp = nlst;
		nlst = clst;
		nlst->size = 0;
		clst = tmp;
	}while(*c++ != '\0');
	free(nlst);
	free(clst);
	reset_regex(re);
	return rval;
}

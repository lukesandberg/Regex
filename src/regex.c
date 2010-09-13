#include <re_compiler.h>
#include <re_ast.h>
#include <regex.h>
#include <util/util.h>

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct regex_s
{
	char* src;
	size_t num_registers;
	program *prog;
};

typedef struct
{
	size_t nref;
	char* regs[];
} cap;

typedef struct
{
	instruction *pc;
	cap* caps;
} thread_t;

typedef struct 
{
	size_t size;
	size_t cap;
	thread_t threads[];
} thread_list;

struct re_run_state
{
	regex* re;
	size_t size;
	cap* cap_cache[];
};

static inline cap* make_cap(struct re_run_state* state)
{
	cap* c;
	if(state->size>0)
	{
		state->size--;
		c = state->cap_cache[state->size];
	}
	else
	{
		c = (cap*) malloc(sizeof(cap) + sizeof(char*) * (state->re->num_registers));
		if(c == NULL) return NULL;
	}
	c->nref = 1;
	return c;
}

static inline void incref(cap* c)
{
	if(c != NULL) c->nref++;
}

static inline void decref(struct re_run_state* state, cap* c)
{
	if(c != NULL)
	{
		c->nref--;
		if(c->nref == 0)
		{
			state->cap_cache[state->size] = c;
			state->size++;
		}
	}
}

static inline cap* update(struct re_run_state* state, cap* c, unsigned int i, char* v)
{
	cap* r = c;
	if(c != NULL)
	{
		if(c->nref > 1)
		{
			c->nref--;
			r = make_cap(state);
			if(r == NULL) return NULL;
			memcpy(&(r->regs[0]), &(c->regs[0]), sizeof(char*)*(state->re->num_registers));
		}
		r->regs[i] = v;
	}
	return r;
}

static thread_list* make_list(size_t sz)
{
	thread_list* lst = (thread_list*) malloc(sizeof(thread_list) + sizeof(thread_t) * sz);
	if(lst == NULL) return NULL;
	lst->size = 0;
	lst->cap = sz;
	return lst;
}

static inline thread_t thread(instruction* pc, cap* caps)
{
	return (thread_t) {.pc = pc, .caps = caps};
}

static int add_to_list(struct re_run_state *state, thread_list* l, thread_t t, char* v)
{
tailcall:
	if(t.pc->tag != v)
	{
		l->threads[l->size] = t;
		l->size++;
		t.pc->tag = v;
		switch(t.pc->op)
		{
			case I_JMP:
				t = thread(t.pc->v.jump, t.caps);
				goto tailcall;
			case I_SPLIT:
				incref(t.caps);
				//can't avoid recursion here
				if(add_to_list(state, l, thread(t.pc->v.split.left, t.caps), v) == 0)
					return 0;//propgate error
				t = thread(t.pc->v.split.right, t.caps);
				goto tailcall;
			case I_SAVE:
				;
				cap* c = update(state, t.caps, t.pc->v.save_register, v);
				if(t.caps != NULL && c == NULL)
					return 0;
				t = thread(t.pc + 1, c);
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
	return 1;
}


regex* regex_create(char* re_str, re_error* er)
{
	size_t num_regs;
	program *prog = compile_regex(re_str, er, &num_regs);
	if(prog == NULL) 
		return NULL;
	regex * re = (regex*) malloc(sizeof(regex));
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
	instruction* end = code + len;
	while(code < end)
	{
		code->tag = NULL;
		code++;
	}
}

int regex_matches(regex* re, char*str, char** save_regs)
{
	int rval = -1;
	char* c = str;
	thread_list *clst = NULL;
	thread_list *nlst = NULL;
	cap* caps = NULL;
	program *prog = re->prog;
	instruction* code = &(prog->code[0]);
	size_t len = prog->size;
	
	clst = make_list(len);
	if(clst == NULL)
		goto end;
	nlst = make_list(len);
	if(nlst == NULL)
		goto end;
	
	struct re_run_state *state = state;
	state = NULL;//this hacks around the gcc variable uninitialized warning
	if(save_regs != NULL)
	{
		state = (struct re_run_state*) malloc(sizeof(struct re_run_state) + len * sizeof(cap*));
		if(state == NULL)
			goto end;
		state->re = re;
		state->size = 0;
		caps = make_cap(state);
		if(caps == NULL)
			goto end;
		memset(&(caps->regs[0]), 0, sizeof(char*) * (re->num_registers));
	}
	if(!add_to_list(state, clst, thread(code, caps), c))
		goto end;
	rval = 0;
	do
	{
		for(unsigned int ti = 0; ti < clst->size; ti++)
		{
			thread_t t = clst->threads[ti];
			instruction* pc = t.pc;
			int v = 0;
			switch(pc->op)
			{
				case I_CHAR:
					v = (pc->v.c == *c);
					break;
				case I_ALPHA:
					v = isalpha(*c);
					break;
				case I_WHITESPACE:
					v = isspace(*c);
					break;
				case I_DIGIT:
					v = isdigit(*c);
					break;
				case I_WILDCARD:
					v = (*c != '\0');
					break;
				case I_MATCH:
					v = 0;
					rval = (*c == '\0');
					if(rval)
					{
						if(save_regs != NULL)
						{
							memcpy(save_regs, &(t.caps->regs[0]), (re->num_registers) * sizeof(char*));
						}
						//we want to use the earliest valid capture due
						//to the way that our matching priorities work
						//out, so decref the rest
						for(; ti < clst->size; ti++)
							decref(state, clst->threads[ti].caps);
						goto end; 
					}
					break;
				case I_JMP:
				case I_SPLIT:
				case I_SAVE:
					v = -1;
					//skip over control flow because we already processed it
			}
			if(v > 0)//did we pass the test
			{
				add_to_list(state, nlst, thread(pc + 1, t.caps), c + 1);
			}
			else if(v == 0)
			{
				//thread death
				decref(state, t.caps);
			}
		}
		thread_list* tmp = nlst;
		nlst = clst;
		nlst->size = 0;
		clst = tmp;
	} while(*c++ != '\0');


end:
	if(nlst != NULL)
		free(nlst);
	if(clst != NULL)
		free(clst);
	if(state != NULL)
	{
		for(size_t i = 0; i < state->size; i++)
		{
			free(state->cap_cache[i]);
		}
		free(state);
	}
	reset_regex(re);
	return rval;

}

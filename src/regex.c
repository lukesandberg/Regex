#include <re_compiler.h>
#include <re_ast.h>
#include <regex.h>
#include <util/util.h>

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
typedef struct thread_list_s thread_list;

typedef struct
{
	instruction *pc;
} thread_t;

struct thread_list_s
{
	size_t size;
	size_t cap;
	thread_t threads[];
};

static thread_list* make_list(size_t sz)
{
	thread_list* lst = (thread_list*) malloc(sizeof(thread_list) + sizeof(thread_t) *sz);
	if(lst == NULL) return NULL;
	lst->size = 0;
	lst->cap = sz;
	return lst;
}
static inline thread_t thread(instruction* pc)
{
	return (thread_t) {.pc = pc};
}

static inline void add_to_list(thread_list* l, thread_t t, void* v)
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
				t = thread(t.pc->v.jump);
				goto tailcall;
			case I_SPLIT:
				add_to_list(l, thread(t.pc->v.split.left), v);
				t = thread(t.pc->v.split.right);
				goto tailcall;
			case I_CHAR:
			case I_WHITESPACE:
			case I_WILDCARD:
			case I_ALPHA:
			case I_DIGIT:
			case I_MATCH:
				;
		}
	}
}

struct regex_s
{
	char* src;
	program *prog;
};

regex* regex_create(char* re_str, re_error* er)
{
	program *prog = compile_regex(re_str, er);
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
	instruction* code = &(prog->code[0]);
	size_t len = prog->size;
	thread_list *clst, *nlst;
	clst = make_list(len);
	if(clst == NULL)
		return -1;
	nlst = make_list(len);
	if(nlst == NULL)
	{
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
			instruction* pc = clst->threads[ti].pc;
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

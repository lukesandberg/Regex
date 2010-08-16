#include "re_compiler.h"
#include "regex.h"
#include <stdlib.h>
#include <ctype.h>
#include <util/util.h>
#include <parser/re_ast.h>
typedef struct thread_list_s thread_list;

typedef struct
{
	instruction *pc;
	thread_list* lst;
} thread_t;

struct thread_list_s
{
	size_t size;
	thread_t threads[];
};

static thread_list* make_list(size_t sz)
{
	thread_list* lst = (thread_list*) malloc(sizeof(thread_list) + sizeof(thread_t) *sz);
	lst->size = 0;
	return lst;
}

static inline void add_to_list(thread_list* l, thread_t t)
{
	if(t.lst != l)
	{
		l->threads[l->size] = t;
		l->size++;
		t.lst = l;
	}
}
static inline thread_t thread(instruction* pc)
{
	return (thread_t) {.pc = pc, .lst = NULL};
}

static inline int matches_rule(char c, unsigned int rule)
{
	switch(rule)
	{
		case RWILDCARD:
		       return 1;
		case RWHITESPACE:
			return isspace(c);
		case RALPHA:
			return isalpha(c);
		case RDIGIT:
			return isdigit(c);
		default:
		if(rule < 256)
		{
			return c == (char) rule;
		}
		dassert(0, "unknown rule");
	}
	return 0;
}

struct regex_s
{
	char* src;
	program *prog;
};

regex* regex_create(char* re_str, re_error* er)
{
	program *prog = compile_regex(re_str, er);
	if(prog == NULL) return NULL;
	regex * re = (regex*) malloc(sizeof(regex));
	re->src = re_str;
	re->prog = prog;
	return re;
}

void regex_destroy(regex* re)
{
	free(re->prog);
	free(re);
}

int regex_matches(regex* re, char*str)
{
	program *prog = re->prog;
	instruction* code = &(prog->code[0]);
	
	size_t len = re->prog->size;
	thread_list *clst, *nlst;
	clst = make_list(len);
	nlst = make_list(len);
	add_to_list(clst, thread(code));
	int rval = 1;
	char*c = str;
	do
	{
		rval = 0;
		for(int ti = 0; ti < clst->size; ti++)
		{
			instruction* pc = clst->threads[ti].pc;
			switch(pc->op)
			{
				case I_RULE:
					if(matches_rule(*c, pc->v.rule))
						add_to_list(nlst, thread(pc + 1));
					break;
				case I_JMP:
					add_to_list(clst, thread(code + pc->v.jump));
					break;
				case I_SPLIT:
					add_to_list(clst, thread(code + pc->v.split.left));
					add_to_list(clst, thread(code + pc->v.split.right));
					break;
				case I_MATCH:
					rval = 1;
					break;
				default:
					dassert(0, "unexpected op code");
			}
		}
		thread_list* tmp = nlst;
		nlst = clst;
		nlst->size = 0;
		clst = tmp;
	}while(*c++ != '\0');
	
	free(nlst);
	free(clst);
	return rval;
}

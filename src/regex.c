#include <re_compiler.h>
#include <re_ast.h>
#include <regex.h>
#include <util/util.h>
#include <util/sparse_map.h>
#include <capture_group.h>

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

struct re_run_state
{
	regex* re;
	char *c;
	cg_cache* cache;
};

static int add_to_list(struct re_run_state *state, sparse_map* map, unsigned int pc_index, capture_group* cap)
{
tailcall:
	if(!sparse_map_contains(map, pc_index))
	{
		sparse_map_set(map, pc_index, cap);
		instruction *pc = state->re->prog->code + pc_index;
		switch(pc->op)
		{
			case I_JMP:
				pc_index = pc->v.jump;
				goto tailcall;
			case I_SPLIT:
				if(cap != NULL) cg_incref(cap);
				//can't avoid recursion here
				if(add_to_list(state, map, pc->v.split.left, cap) == 0)
					return 0;//propgate error
				pc_index = pc->v.split.right;
				goto tailcall;
			case I_SAVE:
				;
				if(cap != NULL)
				{
					cap = cg_update(state->cache, cap, pc->v.save_register, state->c);
					if(cap == NULL)
						return 0;
				}
				pc_index++;
				goto tailcall;
			case I_DGT:
				
				break;
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

static void free_list(struct re_run_state* state, sparse_map* lst)
{
	for(unsigned int i = 0; i < sparse_map_num_entries(lst); i++)
	{
		void* val = NULL;
		sparse_map_get_entry(lst, i, &val);
		if(val != NULL)
		{
			cg_decref(state->cache, (capture_group*) val);
		}
	}
	free_sparse_map(lst);
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

int regex_matches(regex* re, char*str, capture_group** r_caps)
{
	int rval = -1;
	sparse_map *clst = NULL;
	sparse_map *nlst = NULL;
	capture_group* caps = NULL;
	program *prog = re->prog;
	size_t len = prog->size;
	
	clst = make_sparse_map(len);
	if(clst == NULL)
		goto end;
	nlst = make_sparse_map(len);
	if(nlst == NULL)
		goto end;
	struct re_run_state state;
	state.re = re;
	state.c = str;
	state.cache = NULL;
	if(r_caps != NULL)
	{
		state.cache = make_cg_cache(len);
		if(state.cache == NULL)
			goto end;
		caps = make_capture_group(state.cache, re->num_registers);
		if(caps == NULL)
			goto end;
	}
	if(!add_to_list(&state, clst, 0, caps))
		goto end;
	caps = NULL;
	rval = 0;
	do
	{
		for(unsigned int i = 0; i < sparse_map_num_entries(clst); i++)
		{
			void* val = NULL;
			unsigned int pc_index = sparse_map_get_entry(clst, i, &val);

			instruction* pc = prog->code + pc_index;
			caps = (capture_group*) val;
			int v = 0;
			switch(pc->op)
			{
				case I_CHAR:
					v = (pc->v.c == *state.c);
					break;
				case I_ALPHA:
					v = isalpha(*state.c);
					break;
				case I_WHITESPACE:
					v = isspace(*state.c);
					break;
				case I_DIGIT:
					v = isdigit(*state.c);
					break;
				case I_WILDCARD:
					v = (*state.c != '\0');
					break;
				case I_MATCH:
					v = 0;//we never actually go past this
					rval = (*state.c == '\0');
					if(rval)//we are at the end (all matches are anchored)
						//by default
					{
						if(r_caps != NULL)
						{
							*r_caps = caps;
							cg_incref(caps);
							caps = NULL;
						}
						goto end; 
					}
					break;
				case I_JMP:
				case I_SPLIT:
				case I_SAVE:
					v = -1;
					//skip over control flow because we already processed it in add_to_list
			}
			if(v > 0)//we did pass the test
			{
				if(add_to_list(&state, nlst, pc_index + 1, caps) == 0)
					//we ran out of memory... darn it
					goto end;
			}
			else if(v == 0 && state.cache != NULL)
			{
				//thread death
				cg_decref(state.cache, caps);
			}
		}
		sparse_map* tmp = nlst;
		nlst = clst;
		sparse_map_clear(nlst);
		clst = tmp;
	} while(*state.c++ != '\0');


end:
	if(nlst != NULL)
		free_list(&state, nlst);
	if(clst != NULL)
		free_list(&state, clst);
	if(state.cache != NULL)
		free_cg_cache(state.cache);
	if(caps != NULL)
		free(caps);
	return rval;

}

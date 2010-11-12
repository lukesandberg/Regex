#include <re_compiler.h>
#include <re_ast.h>

#include <util/util.h>
#include <util/sparse_map.h>
#include <thread_state.h>
#include <re.h>
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
	size_t num_capture_regs;
};

struct re_run_state
{
	regex* re;
	char *c;
	ts_cache* cache;
};


static int add_to_list(struct re_run_state *state, sparse_map* map, unsigned int pc_index, thread_state* ts)
{
	unsigned int reg_val = 0;
tailcall:
	if(!sparse_map_contains(map, pc_index))
	{
		sparse_map_set(map, pc_index, ts);
		instruction *pc = state->re->prog->code + pc_index;
		switch(pc->op)
		{
			case I_JMP:
				pc_index = pc->v.jump;
				goto tailcall;
			case I_SPLIT:
				ts_incref(ts);
				//can't avoid recursion here
				if(add_to_list(state, map, pc->v.split.left, ts) == 0)
					return 0;//propgate error
				pc_index = pc->v.split.right;
				goto tailcall;
			case I_SAVE:
				ts = ts_update(state->cache, ts, pc->v.save_register, (unsigned int) state->c);
				if(ts == NULL)
					return 0;
				pc_index++;
				goto tailcall;
			case I_DGT:
				reg_val = ts->regs[pc->v.comparison.idx];
				if(reg_val > pc->v.comparison.comp)
				{
					ts_decref(state->cache, ts);
				}
				else
				{
					pc_index++;
					goto tailcall;
				}
				break;
			case I_DLT:
				reg_val = ts->regs[pc->v.comparison.idx];
				if(reg_val < pc->v.comparison.comp)
				{
					ts_decref(state->cache, ts);
				}
				else
				{
					pc_index++;
					goto tailcall;
				}
				break;
			case I_SETZ:
				ts = ts_update(state->cache, ts, pc->v.idx, 0);
				pc_index++;
				goto tailcall;
			case I_INCR:
				reg_val = ts->regs[pc->v.idx];
				ts = ts_update(state->cache, ts, pc->v.idx, reg_val);
				pc_index++;
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

static void free_list(struct re_run_state* state, sparse_map* lst)
{
	for(unsigned int i = 0; i < sparse_map_num_entries(lst); i++)
	{
		void* val = NULL;
		sparse_map_get_entry(lst, i, &val);
		if(val != NULL)
		{
			ts_decref(state->cache, (thread_state*) val);
		}
	}
	free_sparse_map(lst);
}



static inline capture_group* extract_capture_groups(regex* re, thread_state* ts)
{
	capture_group* cg = (capture_group*) malloc(sizeof(capture_group) + sizeof(char*) * re->num_capture_regs);
	memcpy(cg->regs, ts->regs, re->num_capture_regs*sizeof(char*));
	return cg;
}

int regex_matches(regex* re, char*str, capture_group** r_caps)
{
	int rval = -1;
	sparse_map *clst = NULL;
	sparse_map *nlst = NULL;
	thread_state* ts = NULL;
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
	state.cache = make_ts_cache(len);
	if(state.cache == NULL)
		goto end;
	ts = make_thread_state(state.cache, re->num_registers);
	if(ts == NULL)
		goto end;
	if(!add_to_list(&state, clst, 0, ts))
		goto end;
	rval = 0;
	do
	{
		for(unsigned int i = 0; i < sparse_map_num_entries(clst); i++)
		{
			void* val = NULL;
			unsigned int pc_index = sparse_map_get_entry(clst, i, &val);

			instruction* pc = prog->code + pc_index;
			ts = (thread_state*) val;
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
							*r_caps = extract_capture_groups(re, ts);
							//our thread is essentially dead so decref to return to the cache
							ts_decref(state.cache, ts);
							ts = NULL;
						}
						
						goto end; 
					}
					break;
				case I_JMP:
				case I_SPLIT:
				case I_SAVE:
				case I_DGT:
				case I_DLT:
				case I_SETZ:
				case I_INCR:
					v = -1;
					//skip over control flow because we already processed it in add_to_list
			}
			if(v > 0)//we did pass the test
			{
				if(add_to_list(&state, nlst, pc_index + 1, ts) == 0)
					//we ran out of memory... darn it
					goto end;
			}
			else if(v == 0)
			{
				//thread death
				ts_decref(state.cache, ts);
			}
		}
		sparse_map* tmp = nlst;
		nlst = clst;
		sparse_map_clear(nlst);
		clst = tmp;
	} while(*state.c++ != '\0');


end:
	if(nlst != NULL)//kill all pending threads
		free_list(&state, nlst);
	if(clst != NULL)
		free_list(&state, clst);
	if(state.cache != NULL)
		free_ts_cache(state.cache);
	if(ts != NULL)
		free(ts);
	return rval;
}

regex* regex_create(char* re_str, re_error* er)
{
	size_t num_regs, num_capture_regs;
	program *prog = compile_regex(re_str, er, &num_regs, &num_capture_regs);
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
	re->num_capture_regs = num_capture_regs;
	return re;
}

void regex_destroy(regex* re)
{
	free(re->prog);
	free(re);
}
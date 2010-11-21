#include <re_compiler.h>
#include <re_ast.h>

#include <util/util.h>
#include <util/sparse_map.h>
#include <thread_state.h>
#include <re_parser.h>
#include <capture_group.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

struct re_run_state
{
	regex* re;
	char* str;
	ts_cache* cache;
	sparse_map* clst;
	sparse_map* nlst;
    capture_group** r_caps;
};

static int add_to_list(struct re_run_state *state, sparse_map* map, unsigned int pc_index, thread_state* ts, char* c)
{
	unsigned int reg_val = 0;
tailcall:
	if(!sparse_map_contains(map, pc_index))
	{
		sparse_map_set(map, pc_index, ts);
		instruction *pc = state->re->prog.code + pc_index;
		switch(pc->op)
		{
			case I_JMP:
				pc_index = pc->v.jump;
				goto tailcall;
			case I_SPLIT:
				ts_incref(ts);
				//can't avoid recursion here
				if(add_to_list(state, map, pc->v.split.left, ts, c) == 0)
					return 0;//propgate error
				pc_index = pc->v.split.right;
				goto tailcall;
			case I_SAVE:
				ts = ts_update(state->cache, ts, pc->v.save_register, (unsigned int) (c - state->str));
				if(ts == NULL)
					return 0;
				pc_index++;
				goto tailcall;
			case I_DGTEQ:
				reg_val = ts->regs[pc->v.comparison.idx];
				if(reg_val >= pc->v.comparison.comp)
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
				ts = ts_update(state->cache, ts, pc->v.idx, reg_val + 1);
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

static inline capture_group* extract_capture_groups(struct re_run_state* state, thread_state* ts)
{
	capture_group* cg = NEWE(capture_group, sizeof(char*) * state->re->num_capture_regs);
	if(cg == NULL)
		return NULL;
	unsigned int len = state->re->num_capture_regs;
	char* str = state->str;
	for(unsigned int i = 0; i < len; i++)
	{
		cg->regs[i] = str + ts->regs[i];
	}
	cg->sz = len;
	return cg;
}

static int init_state(regex* re, char* str, capture_group** r_caps, struct re_run_state* state)
{
	size_t len = re->prog.size;
	state->r_caps = r_caps;
	state->re = re;
	state->str = str;
	state->cache = make_ts_cache(len);
	if(state->cache == NULL)
		return 0;
	state->clst = make_sparse_map(len);
	if(state->clst == NULL)
	{
		free_ts_cache(state->cache);
		return 0;
	}
	state->nlst = make_sparse_map(len);
	if(state->nlst == NULL)
	{
		free_ts_cache(state->cache);
		free_list(state, state->clst);
		return 0;
	}
	return 1;	
}
static void swap_lists(struct re_run_state* state)
{
	sparse_map* tmp = state->nlst;
	state->nlst = state->clst;
	sparse_map_clear(state->nlst);
	state->clst = tmp;
}
static void free_state(struct re_run_state* state)
{
	free_list(state, state->nlst);
	free_list(state, state->clst);
	free_ts_cache(state->cache);
}


static int process_char(struct re_run_state* state, char* c)
{
	for(unsigned int i = 0; i < sparse_map_num_entries(state->clst); i++)
	{
		void* val = NULL;
		unsigned int pc_index = sparse_map_get_entry(state->clst, i, &val);
		instruction* pc = state->re->prog.code + pc_index;
		thread_state* ts = (thread_state*) val;
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
				v = 0;//we never actually go past this
				if(*c == '\0')//we are at the end (all matches are anchored)
					//by default
				{
					if(state->r_caps != NULL)
					{
						*(state->r_caps) = extract_capture_groups(state, ts);
						if(*(state->r_caps) == NULL) //extraction failed
						{
							return -1;
						}
					}
					return 1;
				}
				break;
			case I_JMP:
			case I_SPLIT:
			case I_SAVE:
			case I_DGTEQ:
			case I_DLT:
			case I_SETZ:
			case I_INCR:
				v = -1;
				//skip over control flow because we already processed it in add_to_list
		}
		if(v > 0)//we did pass the test
		{
			if(!add_to_list(state, state->nlst, pc_index + 1, ts, c + 1))
			{
				return -1;
			}
		}
		else if(v == 0)
		{
			//thread death
			ts_decref(state->cache, ts);
		}
	}
	return 0;
}

int regex_matches(regex* re, char*str, capture_group** r_caps)
{
	int rval = -1;
	char* c = str;
	thread_state* ts;
	struct re_run_state state;
	if(!init_state(re, str, r_caps, &state))
		return -1;
	ts = make_thread_state(state.cache, re->num_registers);
	if(ts == NULL)
		goto end;
	if(!add_to_list(&state, state.clst, 0, ts, c))
		goto end;
	rval = 0;
	do
	{
		int v = process_char(&state, c);
		if(v != 0)
		{
			rval = v;
			break;
		}
		swap_lists(&state);
	} while(*c++ != '\0');

end:
	free_state(&state);
	return rval;
}

regex* regex_create(char* re_str, re_error* er)
{
	ast_node* tree = re_parse(re_str, er);
	if(tree == NULL)
	       	return NULL;//there was an error during parsing
	
	regex * re = compile_regex(tree);
	free_node(tree);
	if(re == NULL)
	{
		if(er != NULL)
		{
			er->errno = E_OUT_OF_MEMORY;
			er->position = -1;
		}
		return NULL;
	}	
	re->src = re_str;
	return re;
}

void regex_destroy(regex* re)
{
	rfree(re);
}
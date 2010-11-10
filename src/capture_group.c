#include <thread_state.h>
#include <stdlib.h>
#include <stdint.h>

struct _ts
{
	size_t nref;
	size_t sz;
	char* regs[];
};

struct _tsc_s
{
	size_t n;
	thread_state* ts_cache[];
};

ts_cache* make_ts_cache(size_t sz)
{
	ts_cache* cache = (ts_cache*) malloc(sizeof(ts_cache) + sizeof(thread_state*) * sz);
	if(cache == NULL)
		return NULL;
	cache->n = 0;
	return cache;
}
void free_ts_cache(ts_cache* c)
{
	for(unsigned int i = 0; i < c->n; i++)
	{
		free(c->ts_cache[i]);
	}
	free(c);
}

thread_state* make_thread_state(ts_cache* cache, size_t sz)
{
	thread_state* c;
	if(cache->n > 0)
	{
		cache->n--;
		c = cache->ts_cache[cache->n];
	}
	else
	{
		c = (thread_state*) malloc(sizeof(thread_state) + sizeof(char*) * sz);
		if(c == NULL) return NULL;
		c->sz = sz;
		c->nref = 0;
	}
	c->nref = 1;
	return c;
}

size_t ts_num_captures(thread_state * ts)
{
	return ts->sz / 2;
}

char* ts_get_cap(thread_state *ts, unsigned int i, char** end)
{
	*end = ts->regs[2*i + 1];
	return ts->regs[2*i];
}

void ts_incref(thread_state* c)
{
	c->nref++;
}
void ts_decref(ts_cache* cache, thread_state* c)
{
	c->nref--;
	if(c->nref == 0)
	{
		cache->ts_cache[cache->n] = c;
		cache->n++;
	}
}

thread_state* ts_update(ts_cache* cache, thread_state* c, unsigned int i, char* v)
{
	thread_state* r = c;
	if(c->nref > 1)
	{
		c->nref--;
		r = make_thread_state(cache, c->sz);
		if(r == NULL) return NULL;
		memcpy(&(r->regs[0]), &(c->regs[0]), sizeof(char*) * c->sz);
	}
	r->regs[i] = v;
	return r;
}


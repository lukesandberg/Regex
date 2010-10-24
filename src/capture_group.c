#include <capture_group.h>
#include <stdlib.h>

struct _cg_s
{
	size_t nref;
	size_t sz;
	char* regs[];
};

struct _cgc_s
{
	size_t n;
	capture_group* cap_cache[];
};

cg_cache* make_cg_cache(size_t sz)
{
	cg_cache* cache = (cg_cache*) malloc(sizeof(cg_cache) + sizeof(capture_group*) * sz);
	if(cache == NULL)
		return NULL;
	cache->n = 0;
	return cache;
}
void free_cg_cache(cg_cache* c)
{
	for(unsigned int i = 0; i < c->n; i++)
	{
		free(c->cap_cache[i]);
	}
	free(c);
}

capture_group* make_capture_group(cg_cache* cache, size_t sz)
{
	capture_group* c;
	if(cache->n > 0)
	{
		cache->n--;
		c = cache->cap_cache[cache->n];
	}
	else
	{
		c = (capture_group*) malloc(sizeof(capture_group) + sizeof(char*) * sz);
		if(c == NULL) return NULL;
		c->sz = sz;
		c->nref = 0;
	}
	c->nref = 1;
	return c;
}

size_t cg_num_captures(capture_group * cg)
{
	return cg->sz / 2;
}

char* cg_get_cap(capture_group *cg, unsigned int i, char** end)
{
	*end = cg->regs[2*i + 1];
	return cg->regs[2*i];
}

void cg_incref(capture_group* c)
{
	c->nref++;
}
void cg_decref(cg_cache* cache, capture_group* c)
{
	c->nref--;
	if(c->nref == 0)
	{
		cache->cap_cache[cache->n] = c;
		cache->n++;
	}
}

capture_group* cg_update(cg_cache* cache, capture_group* c, unsigned int i, char* v)
{
	capture_group* r = c;
	if(c->nref > 1)
	{
		c->nref--;
		r = make_capture_group(cache, c->sz);
		if(r == NULL) return NULL;
		memcpy(&(r->regs[0]), &(c->regs[0]), sizeof(char*) * c->sz);
	}
	r->regs[i] = v;
	return r;
}


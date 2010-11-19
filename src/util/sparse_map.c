#include "sparse_map.h"
#include <stdlib.h>
#include <util/util.h>
struct _sparse_map_s
{
	void** values;
	unsigned int n;
	size_t max;
	unsigned int indices[];
};

sparse_map* make_sparse_map(size_t max)
{
	sparse_map* map = NEWE(sparse_map, sizeof(unsigned int) * max * 2);
	if(map == NULL) 
		return NULL;
	void** vals = rmalloc(sizeof(void*)*max);
	if(vals == NULL) 
	{
		rfree(map);
		return NULL;
	}
	
	map->max = max;
	map->n = 0;
	map->values = vals;
	return map;
}

static inline void set_dense(sparse_map* map, unsigned int n, unsigned int v)
{
    map->indices[n] = v;
}

static inline void set_sparse(sparse_map* map, unsigned int n, unsigned int v)
{
    map->indices[map->max + n] = v;
}

static inline unsigned int get_dense(sparse_map* map, unsigned int n)
{
    return map->indices[n];
}

static inline unsigned int get_sparse(sparse_map* map, unsigned int n)
{
    return map->indices[map->max + n];
}

void free_sparse_map(sparse_map* map)
{
	rfree(map->values);
	rfree(map);
}

void sparse_map_set(sparse_map* map, unsigned int i, void* v)
{
	unsigned int n = map->n;
	set_dense(map, n, i);
	set_sparse(map, i, n);
	map->values[n] = v;
	map->n = n+1;
}

int sparse_map_contains(sparse_map* map, unsigned int i)
{
	unsigned int s = get_sparse(map, i);
	return s < map->n && get_dense(map,s) == i;
}
void* sparse_map_get(sparse_map* map, unsigned int i)
{
	return map->values[get_sparse(map, i)];
}

void sparse_map_clear(sparse_map* map)
{
	map->n = 0;
}

size_t sparse_map_num_entries(sparse_map* map)
{
	return map->n;
}
unsigned int sparse_map_get_entry(sparse_map* map, unsigned int ind, void** val)
{
	*val = map->values[ind];
	return get_dense(map, ind);
}


#include "sparse_map.h"
#include <stdlib.h>

struct _sparse_map_s
{
	unsigned int* dense;
	unsigned int* sparse;
	void** values;
	unsigned int n;
	size_t max;
};

sparse_map* make_sparse_map(size_t max)
{
	sparse_map* map = (sparse_map*) malloc(sizeof(sparse_map));
	if(map == NULL) 
		return NULL;
	unsigned int *arrs = (unsigned int*) malloc(sizeof(unsigned int)* 2 * max);
	if(arrs == NULL) 
	{
		free(map);
		return NULL;
	}
	void** vals = (void**) malloc(sizeof(void*)*max);
	if(vals == NULL) 
	{
		free(arrs);
		free(map);
		return NULL;
	}
	
	map->max = max;
	map->n = 0;
	map->dense = arrs;
	map->sparse = arrs+max;
	map->values = vals;
	return map;
}

void free_sparse_map(sparse_map* map)
{
	free(map->dense);
	free(map->values);
	free(map);
}

void sparse_map_set(sparse_map* map, unsigned int i, void* v)
{
	unsigned int n = map->n;
	map->dense[n] = i;
	map->values[n] = v;
	map->sparse[i] = n;
	map->n = n+1;
}

int sparse_map_contains(sparse_map* map, unsigned int i)
{
	unsigned int s = map->sparse[i];
	return s < map->n && map->dense[s] == i;
}
void* sparse_map_get(sparse_map* map, unsigned int i)
{
	unsigned int ind = map->sparse[i];
	return map->values[ind];
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
	return map->dense[ind];
}


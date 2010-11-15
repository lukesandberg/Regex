#ifndef _SPARSE_MAP_H
#define _SPARSE_MAP_H
#include <stdlib.h>
//sparse map implementation based on 
//http://research.swtch.com/2008/03/using-uninitialized-memory-for-fun-and.html
//this should be useful as long as the max number of items in the map is known
//and they can be represented as integers between 0....MAX_NUM

/*
   This map allow O(1) insertion, O(N) iteration in the same
   order as insertion. O(1) clear time and O(1) containment test time,
   it uses O(M) memory where M is the max number of elements in the map.

   It provides no capability to remove (single) items.  This functionality
   could be added but it would be O(N) in the worst and average cases, so
   it is not provided.
*/

typedef struct _sparse_map_s sparse_map;

sparse_map* make_sparse_map(size_t max);

void free_sparse_map(sparse_map* map);

void sparse_map_set(sparse_map* map, unsigned int i, void* v);

int sparse_map_contains(sparse_map* map, unsigned int i);

void* sparse_map_get(sparse_map* map, unsigned int i);

void sparse_map_clear(sparse_map* map);

size_t sparse_map_num_entries(sparse_map* map);

unsigned int sparse_map_get_entry(sparse_map* map, unsigned int ind, void** val);

#endif

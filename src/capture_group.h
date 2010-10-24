#ifndef _CAPTURE_GROUP_H_
#define _CAPTURE_GROUP_H_

#include <stdlib.h>
#include <string.h>
typedef struct _cg_s capture_group;
typedef struct _cgc_s cg_cache;

capture_group* make_capture_group(cg_cache* cache, size_t sz);
cg_cache* make_cg_cache(size_t sz);
void free_cg_cache(cg_cache* c);

void cg_incref(capture_group* c);

void cg_decref(cg_cache* cache, capture_group* c);

capture_group* cg_update(cg_cache* cache, capture_group* c, unsigned int i, char* v);

size_t cg_num_captures(capture_group * cg);

char* cg_get_cap(capture_group *cg, unsigned int i, char** end);

#endif

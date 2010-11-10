#ifndef _THREAD_STATE_H_
#define _THREAD_STATE_H_

#include <stdlib.h>
#include <string.h>
typedef struct _ts_s thread_state;
typedef struct _tsc_s ts_cache;

thread_state* make_thread_state(ts_cache* cache, size_t sz);

ts_cache* make_ts_cache(size_t sz);
void free_ts_cache(ts_cache* c);

void ts_incref(thread_state* c);

void ts_decref(ts_cache* cache, thread_state* c);

thread_state* ts_update(ts_cache* cache, thread_state* c, unsigned int i, char* v);

size_t ts_num_captures(thread_state * ts);

char* ts_get_cap(thread_state *ts, unsigned int i, char** end);

#endif

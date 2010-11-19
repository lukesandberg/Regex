#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdlib.h>

#define CRASH_MACRO (*((int*)0))++;
#define NEW(type) ((type*) rmalloc(sizeof(type)))
#define NEWE(type, extra) ((type*) rmalloc(sizeof(type) + extra))

#define DEBUG_LEVEL TRACE
#define DEBUG
#define MEM_TEST
extern unsigned int mem_fail_count;
extern unsigned int mem_usage;
typedef enum
{
	TRACE,
	WARN,
	ERROR,
} debug_level;


void dassert(int check, const char* msg);
void debug(debug_level level, const char* fmt, ...);
void* rmalloc(const size_t sz);
void rfree(void* ptr);
char* copy_cstring(const char*);

#endif

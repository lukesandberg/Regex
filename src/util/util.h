#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdlib.h>

#define CRASH_MACRO (*((int*)0))++;
#define NEW(type) ((type*) checked_malloc(sizeof(type)))
#define DEBUG
typedef enum
{
	TRACE,
	WARN,
	ERROR,
} debug_level;

#define DEBUG_TYPE TRACE
void dassert(int check, const char* msg);
void debug(debug_level level, const char* fmt, ...);
void* checked_malloc(const size_t sz);
void checked_free(void* ptr);
char* copy_cstring(const char*);

#endif

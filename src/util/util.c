#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

unsigned int mem_fail_count = 0xffffffu;
unsigned int mem_usage = 0;
void debug(debug_level level, const char* fmt, ...)
{
#ifdef DEBUG
	if(level == DEBUG_LEVEL)
	{
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}
#else
	//empty func in non debug mode
#endif
}


void dassert(int check, const char* msg)
{
#ifdef DEBUG

	if(check)
	{
		debug(ERROR, msg);
		exit(1);
	}
#else
#endif
}

void* rmalloc(size_t sz)
{
#ifdef MEM_TEST
	if(mem_fail_count > 0)
	{
		mem_fail_count--;
		
		void* mem = malloc(sz + sizeof(size_t));
		if(mem == NULL)
			return NULL;
		mem_usage += sz;
		*((size_t*) mem) = sz;//store the size of the block
		return mem + sizeof(size_t);//return the block just past the size
	}
	else
	{
		return NULL;
	}	
#else
	return malloc(sz);
#endif
}

void rfree(void *ptr)
{
#ifdef MEM_TEST	
	ptr = ptr - sizeof(size_t);
	mem_usage -= *((size_t*) ptr);
#endif
//	free(ptr);
}

char* copy_cstring(const char* s)
{
	int len = strlen(s) + 1;//+1 for the '\0' byte
	char* ns = (char*) rmalloc(len);
	memcpy(ns, s, len);
	return ns;
}

#include <util/util.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

static int mem_mix = 0xdeadbeef;

void debug(debug_level level, const char* fmt, ...)
{
#ifdef DEBUG
	if(level == DEBUG_TYPE)
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

void* checked_malloc(size_t sz)
{
	void* rval = malloc(sz);
	if(rval == NULL)
	{
		fprintf(stderr, "out of memory!\n");
		exit(1);
	}
	memset(rval, 0, sz);
	return rval;
}

void checked_free(void *ptr)
{
	if(ptr == NULL)
	{
		fprintf(stderr, "attempting to free a null object!\n");
		exit(1);
	}
	int *iptr = ptr;
	*iptr = mem_mix;//trash the first word of the region
	free(ptr);
}

char* copy_cstring(const char* s)
{
	int len = strlen(s);
	char* ns = (char*) checked_malloc(len + 1);
	memcpy(ns, s, len);
	ns[len] = '\0';
	return ns;
}

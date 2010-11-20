#ifndef __STRING_UTIL__H__
#define __STRING_UTIL__H__

#include <stdlib.h>
#include <string.h>

static inline char* repeat(char* pattern, long long times)
{
	size_t len = strlen(pattern);
	char* buf = malloc(times*len + 1);
	char* copy_loc = buf;
	for(int i = 0; i <times; i++)
	{
		memcpy(copy_loc, pattern, len);
		copy_loc += len;
	}
	*copy_loc = '\0';
	return buf;
}

static inline char* acat(char* left, char* right)
{
	size_t ll = strlen(left);
	size_t rl = strlen(right);
	char* buf = malloc(ll+rl+1);
	memcpy(buf, left, ll);
	memcpy(buf + ll, right, rl);
	buf[ll+rl] = '\0';
	return buf;
}

#endif
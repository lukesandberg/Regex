#include "matcher_tests.h"
#include "minunit.h"
#include <re.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define __USE_BSD
#include <sys/time.h>

static struct timeval start_time;
void start_timer()
{
	gettimeofday(&start_time, NULL);
}
static char* stop_and_print_timer()
{
	static char buf[100];
	struct timeval end_time;
	struct timeval diff;
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &diff);
	snprintf(buf,100, "%ld.%.6ld", diff.tv_sec, diff.tv_usec);
	return buf;
}

static char* repeat(char* pattern, long long times)
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

static char* acat(char* left, char* right)
{
	size_t ll = strlen(left);
	size_t rl = strlen(right);
	char* buf = malloc(ll+rl+1);
	memcpy(buf, left, ll);
	memcpy(buf + ll, right, rl);
	buf[ll+rl] = '\0';
	return buf;
}

static char* TestLongExp(long long rep)
{
	char* l = repeat("a?", rep);
	char* r = repeat("a", rep);

	char* re_str = acat(l, r);
	free(l);
	regex* re = regex_create(re_str, NULL);
	start_timer();
	regex_matches(re, r, NULL);
	
	char* time = stop_and_print_timer();
	printf("Long (a?)^%lld(a)^%lld:\t%s\n", rep, rep, time);
	regex_destroy(re);
	free(r);
	free(re_str);
	return NULL;
}

void test_performance(long long n)
{
	TestLongExp(n);
}

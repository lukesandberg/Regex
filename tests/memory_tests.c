#include "memory_tests.h"
#include "minunit.h"
#include <stdio.h>
#include "string_util.h"
#include <re.h>

static int re_stressor(char* re, char* str)
{
	unsigned int nc = 146;
	mem_fail_count = nc;
	unsigned int mem_usage_before = mem_usage;
	unsigned int mem_usage_after = mem_usage;
	while(1)
	{
		mem_usage_before = mem_usage;
		
		re_error er;
		regex* r = regex_create(re, &er);
		if(er.errno == E_SUCCESS)
		{
			int rv = regex_matches(r, str, NULL);
			regex_destroy(r);
			if(rv == 0)
				mu_assert("regex should match!", 0);
			else if(rv == 1)
				break;
		}
		else if(er.errno != E_OUT_OF_MEMORY)
			mu_assert("unexpected error", 0);
		
		mem_usage_after = mem_usage;
		mu_assert("memory should not have leaked", mem_usage_after == mem_usage_before);
		mem_fail_count = nc;
		nc++;
	}

	return 1;
}

static int TestBasicLong()
{
	char* re = repeat("a", 1000);
	char* str = repeat("a", 1000);
	int v = re_stressor(re, str);
	free(re);
	free(str);
	return v;	
}
static int TestComplex()
{
	return re_stressor("(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)*", "asdfgadfg");
}
static int TestRepetitions()
{
	return re_stressor("(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z){3,4}", "asd");
}

void test_memory()
{
	printf("Testing Memory\n");
	mu_run_test(TestBasicLong);
	mu_run_test(TestComplex);
	mu_run_test(TestRepetitions);
}

#include "memory_tests.h"
#include "minunit.h"
#include <stdio.h>
#include "string_util.h"
#include <re.h>
static int TestMemory1()
{
	char* re = repeat("a", 1000);
	char* str = repeat("a", 1000);
	unsigned int nc = 2;
	mem_fail_count = 1;
	while(1)
	{
		printf("Fail after %i allocs\n", mem_fail_count);
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
		mem_fail_count = nc;
		nc++;
	}
	return 1;
}

void test_memory()
{
	printf("Testing Memory\n");
#ifndef MEM_TEST
	mu_start_test("TestMemory");
	mu_fail("You must define MEM_TEST to run the memory tests", __FILE__, __LINE__);
#else
	mu_run_test(TestMemory1);
#endif
}

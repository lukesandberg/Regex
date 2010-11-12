#include "capture_tests.h"
#include "minunit.h"
#include <stdlib.h>
#include <stdio.h>

int capture(char* re_str, char* str, capture_group** cg)
{
	re_error er;
	regex* re = regex_create(re_str, &er);
	if(re == NULL) return 0;
	int m = regex_matches(re, str, cg);
	regex_destroy(re);
	return m;
}

char* TestCaptureEverything()
{
	capture_group* cg;
	mu_assert("should match", capture("(.*)", "adsfasdf", &cg));
	char* start;
	char* end;
	start = cg_get_capture(cg, 0, &end);
	mu_assert("first char should be a", *start == 'a');
	mu_assert("last char should be f", *end == 'f');
	free(cg);
	return NULL;
}

void test_captures()
{
	printf("Testing Captures\n");
	mu_run_test(TestCaptureEverything);
}


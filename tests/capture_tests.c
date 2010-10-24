#include "matcher_tests.h"
#include "minunit.h"
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>

int capture(char* re_str, char* str, capture_group** caps)
{
	re_error er;
	regex* re = regex_create(re_str, &er);
	if(re == NULL) return 0;
	int m = regex_matches(re, str, caps);
	regex_destroy(re);
	return m;
}



char* TestCaptureEverything()
{
	capture_group* caps;
	mu_assert("should match", capture("(.*)", "adsfasdf", &caps));
	char* start;
	char* end;
	start = cg_get_cap(caps, 0, &end);
	mu_assert("first char should be a", *start == 'a');
	mu_assert("last char should be f", *end == 'f');
	return NULL;
}



void test_captures()
{
	printf("Testing Captures\n");
	mu_run_test(TestCaptureEverything);
}



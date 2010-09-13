#include "matcher_tests.h"
#include "minunit.h"
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>

int capture(char* re_str, char* str, char** caps)
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
	char* caps[2];
	mu_assert("should match", capture("(.*)", "adsfasdf", caps));
	mu_assert("first char should be a", *(caps[0])=='a');
	mu_assert("first char should be \\0", *(caps[1])=='\0');
	return NULL;
}



void test_captures()
{
	printf("Testing Captures\n");
	mu_run_test(TestCaptureEverything);
}



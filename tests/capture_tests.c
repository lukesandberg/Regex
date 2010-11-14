#include "capture_tests.h"
#include "minunit.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vm.h>
#include <regex.h>

int capture(char* re_str, char* str, capture_group** cg)
{
	re_error er;
	regex* re = regex_create(re_str, &er);
	if(re == NULL) return 0;
	int m = regex_matches(re, str, cg);
	regex_destroy(re);
	return m;
}

static char* TestCaptureEverything()
{
	capture_group* cg;
	mu_assert("should match", capture("(.*)", "adsfasdf", &cg));
	char* start;
	char* end;
	start = cg_get_capture(cg, 0, &end);
	mu_assert("first char should be a", *start == 'a');
	mu_assert("last char should be f", *end == '\0');
	free(cg);
	return NULL;
}

static char* TestCaptureGreediness()
{
	capture_group* cg;
	mu_assert("should match", capture("(.*)df", "asdfasdf", &cg));
	char* start;
	char* end;
	start = cg_get_capture(cg, 0, &end);
	mu_assert("greedy capture should match asdfas", strncmp(start, "asdfas", 6) == 0);
	free(cg);

	mu_assert("should match", capture("(.*?)df", "asdfasdf", &cg));
	start = cg_get_capture(cg, 0, &end);
	mu_assert("non greedy capture should match asdfas", strncmp(start, "as", 2) == 0);
	free(cg);
	return NULL;
}

static char* TestCountedRepCapture()
{
	capture_group* cg;
	char *start, *end;
	mu_assert("should match", capture("(ab){2}.*", "ababghjk", &cg));
	start = cg_get_capture(cg, 0, &end);
	mu_assert("capture should match ab", strncmp(start, "ab", 2) == 0);
	free(cg);
	return NULL;
}
static char* TestStarCaptureInteraction()
{
	capture_group* cg;
	char *start, *end;
	mu_assert("should match", capture("(ab)*", "abab", &cg));
	start = cg_get_capture(cg, 0, &end);
	mu_assert("capture should match ab", strncmp(start, "ab", 2) == 0);
	free(cg);
	return NULL;
}
void test_captures()
{
	printf("Testing Captures\n");
	mu_run_test(TestCaptureEverything);
	mu_run_test(TestCaptureGreediness);
	mu_run_test(TestCountedRepCapture);
	mu_run_test(TestStarCaptureInteraction);
}


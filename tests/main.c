#include "minunit.h"
#include "regex.h"
#include "parse.h"
#include <stdlib.h>

int match(char* re_str, char* str)
{
	regex* re = regex_create(re_str);
	int m = regex_matches(re, str);
	regex_destroy(re);
	return m;
}

char* TestWildCard()
{
	mu_assert("should match using wildcard", match("asd.", "asdf"));
	mu_assert("escaped wildcard should not match", !match("asd\\.", "asdf"));
	mu_assert("escaped wildcard should match", match("asd\\.", "asd."));
	mu_assert("wildcard should not match empty string", !match(".", ""));
	return NULL;
}
char* TestEmptyRegex()
{
	mu_assert("empty regex should match empty string", match("", ""));
	mu_assert("empty regex should not match non empty string", !match("", "a"));
	return NULL;
}
int main(int argc, char**argv)
{
	mu_run_test(TestWildCard);
	mu_run_test(TestEmptyRegex);
	mu_print_summary();
}

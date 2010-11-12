#include "matcher_tests.h"
#include "minunit.h"
#include <re.h>
#include <stdlib.h>
#include <stdio.h>

int match(char* re_str, char* str)
{
	re_error er;
	regex* re = regex_create(re_str, &er);
	if(re == NULL) return 0;
	int m = regex_matches(re, str, NULL);
	regex_destroy(re);
	return m;
}

char* TestBasic()
{
	mu_assert("should simple match", match("a", "a"));
	return NULL;
}
char* TestWildCard()
{
	mu_assert("should match using wildcard", match("asd.", "asdf"));
	mu_assert("escaped wildcard should not match", !match("asd\\.", "asdf"));
	mu_assert("escaped wildcard should match", match("asd\\.", "asd."));
	mu_assert("wildcard should not match empty string", !match(".", ""));
	mu_assert("double wildcard", match("..", "zg"));
	mu_assert("double wildcard, with prefix", match("a..", "azg"));
	return NULL;
}
char* TestEmptyRegex()
{
	mu_assert("empty regex should match empty string", match("", ""));
	mu_assert("empty regex should not match non empty string", !match("", "a"));
	return NULL;
}
char* TestReuseRegex()
{
	regex* re = regex_create("asd.", NULL);
	mu_assert("should match", regex_matches(re, "asdf", NULL));
	mu_assert("should match again", regex_matches(re, "asdf", NULL));
	mu_assert("should not", !regex_matches(re, "", NULL));
	regex_destroy(re);
	return NULL;
}
char* TestCharacterClass()
{
	mu_assert("space should match", match("\\s", " "));
	mu_assert("tab should match", match("\\s", "\t"));
	mu_assert("alpha should match", match("\\w", "b"));
	mu_assert("match multiple digit", match("\\d\\d", "12"));
	return NULL;
}
char* TestStar()
{
	mu_assert("star should match zero times", match("a*",""));
	mu_assert("star should match one times", match("a*","a"));
	mu_assert("star should match two times", match("a*","aa"));
	mu_assert("star should match many times", match("a*","aaaaaaaaaa"));
	return NULL;
}
char* TestQMark()
{
	mu_assert("qmark should match zero times", match("a?", ""));
	mu_assert("qmark should match one times", match("a?", "a"));
	mu_assert("qmark should not match more than once", !match("a?", "aa"));
	return NULL;
}
char* TestPlus()
{
	mu_assert("plus should not match zero times", !match("a+", ""));
	mu_assert("plus should match one times", match("a+", "a"));
	mu_assert("plus shuold match many times",match("a+", "aaaaaaaaaaaaaaaaa"));
	return NULL;
}
char* TestAlternation()
{
	mu_assert("alt basic test: match left", match("ab|cd", "ab"));
	mu_assert("alt basic test: match right", match("ab|cd", "cd"));
	mu_assert("alt sanity check", !match("ab|cd", "ad"));
	return NULL;
}
char* TestSubExpression()
{
	mu_assert("basic sub expression", match("a(bc)d", "abcd"));
	mu_assert("repeated group", match("(ab)*", "ababab"));
	mu_assert("nested groups", match("(ab(cd)*ef)+", "abcdcdefabcdcdcdef"));
	mu_assert("sub alternation 1", match("a(b|c)d", "acd"));
	mu_assert("sub alternation 2", match("a(b|c)d", "abd"));
	return NULL;
}


void  test_matcher()
{
	printf("Testing Matcher\n");
	mu_run_test(TestEmptyRegex);
	mu_run_test(TestBasic);
	mu_run_test(TestWildCard);
	mu_run_test(TestReuseRegex);
	mu_run_test(TestCharacterClass);
	mu_run_test(TestStar);
	mu_run_test(TestQMark);
	mu_run_test(TestPlus);
	mu_run_test(TestAlternation);
	mu_run_test(TestSubExpression);
}

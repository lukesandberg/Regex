#include "matcher_tests.h"
#include "minunit.h"
#include <re.h>
#include <stdlib.h>
#include <stdio.h>
#include <vm.h>
#include <regex.h>

int match(char* re_str, char* str)
{
	re_error er;
	regex* re = regex_create(re_str, &er);
	//print_program(&(re->prog));
	if(re == NULL) return 0;
	int m = regex_matches(re, str, NULL);
	regex_destroy(re);
	return m;
}

static int TestBasic()
{
	mu_assert("should simple match", match("a", "a"));
	return 1;
}
static int TestWildCard()
{
	mu_assert("should match using wildcard", match("asd.", "asdf"));
	mu_assert("escaped wildcard should not match", !match("asd\\.", "asdf"));
	mu_assert("escaped wildcard should match", match("asd\\.", "asd."));
	mu_assert("wildcard should not match empty string", !match(".", ""));
	mu_assert("double wildcard", match("..", "zg"));
	mu_assert("double wildcard, with prefix", match("a..", "azg"));
	return 1;
}
static int TestEmptyRegex()
{
	mu_assert("empty regex should match empty string", match("", ""));
	mu_assert("empty regex should not match non empty string", !match("", "a"));
	return 1;
}
static int TestReuseRegex()
{
	regex* re = regex_create("asd.", NULL);
	mu_assert("should match", regex_matches(re, "asdf", NULL));
	mu_assert("should match again", regex_matches(re, "asdf", NULL));
	mu_assert("should not", !regex_matches(re, "", NULL));
	regex_destroy(re);
	return 1;
}
static int TestCharacterClass()
{
	mu_assert("space should match", match("\\s", " "));
	mu_assert("tab should match", match("\\s", "\t"));
	mu_assert("alpha should match", match("\\w", "b"));
	mu_assert("match multiple digit", match("\\d\\d", "12"));
	return 1;
}
static int TestStar()
{
	mu_assert("star should match zero times", match("a*",""));
	mu_assert("star should match one times", match("a*","a"));
	mu_assert("star should match two times", match("a*","aa"));
	mu_assert("star should match many times", match("a*","aaaaaaaaaa"));
	return 1;
}
static int TestQMark()
{
	mu_assert("qmark should match zero times", match("a?", ""));
	mu_assert("qmark should match one times", match("a?", "a"));
	mu_assert("qmark should not match more than once", !match("a?", "aa"));
	return 1;
}
static int TestPlus()
{
	mu_assert("plus should not match zero times", !match("a+", ""));
	mu_assert("plus should match one times", match("a+", "a"));
	mu_assert("plus shuold match many times",match("a+", "aaaaaaaaaaaaaaaaa"));
	return 1;
}
static int TestAlternation()
{
	mu_assert("alt basic test: match left", match("ab|cd", "ab"));
	mu_assert("alt basic test: match right", match("ab|cd", "cd"));
	mu_assert("alt sanity check", !match("ab|cd", "ad"));
	mu_assert("alt with empty 1", match("a(|b)c", "abc"));
	mu_assert("alt with empty 2", match("a(|b)c", "ac"));
	char* large_alt = "a|b|c|d|e|f|g|h|i|j|k|l||m|n|o|p|q|r|s|t|u|v|w|x|y|z";
	char* letters[26] = {"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"};
	char buf[28];
	for(int i = 0; i < 26; i++)
	{
		char* msg = "large alternation test %s";
		sprintf(buf, msg, letters[i]);
		mu_assert(buf, match(large_alt, letters[i]));
	}

	return 1;
}
static int TestSubExpression()
{
	mu_assert("basic sub expression", match("a(bc)d", "abcd"));
	mu_assert("repeated group", match("(ab)*", "ababab"));
	mu_assert("nested groups", match("(ab(cd)*ef)+", "abcdcdefabcdcdcdef"));
	mu_assert("sub alternation 1", match("a(b|c)d", "acd"));
	mu_assert("sub alternation 2", match("a(b|c)d", "abd"));
	return 1;
}

static int TestCountedRep()
{
	mu_assert("basic counted rep match: \"a{2}\", \"aa\"", match("a{2}", "aa"));
	mu_assert("range counted rep no match: \"a{2,4}\", \"a\"", !match("a{2,4}", "a"));
	mu_assert("range counted rep match: \"a{2,4}\", \"aa\"", match("a{2,4}", "aa"));
	mu_assert("range counted rep match: \"a{2,4}\", \"aaa\"", match("a{2,4}", "aaa"));
	mu_assert("range counted rep match: \"a{2,4}\", \"aaaa\"", match("a{2,4}", "aaaa"));
	mu_assert("range counted rep no match: \"a{2,4}\", \"aaaaa\"", !match("a{2,4}", "aaaaa"));
	return 1;
}
static int TestGroupRepetitions()
{
	mu_assert("basic group counted rep match: \"(abc){2}\", \"aa\"", match("(abc){2}", "abcabc"));
	mu_assert("range group counted rep no match: \"(abc){2,4}\", \"abc\"", !match("(abc){2,4}", "abc"));
	mu_assert("range group counted rep match: \"(abc){2,4}\", \"abcabc\"", match("(abc){2,4}", "abcabc"));
	mu_assert("range group counted rep match: \"(abc){2,4}\", \"abcabcabc\"", match("(abc){2,4}", "abcabcabc"));
	mu_assert("range group counted rep match: \"(abc){2,4}\", \"abcabcabcabc\"", match("(abc){2,4}", "abcabcabcabc"));
	mu_assert("range group counted rep no match: \"(abc){2,4}\", \"abcabcabcabcabc\"", !match("(abc){2,4}", "abcabcabcabcabc"));
	return 1;
}
void test_matcher()
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
	mu_run_test(TestCountedRep);
	mu_run_test(TestGroupRepetitions);
}

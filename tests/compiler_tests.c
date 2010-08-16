#include "compiler_tests.h"
#include "minunit.h"
#include <re_compiler.h>
#include <stdio.h>

static char* TestSingleMatch()
{
	program* prog = compile_regex("a", NULL);
	mu_assert("program length", prog->size ==  2);
	mu_assert("first inst is rule", prog->code[0].op == I_RULE);
	mu_assert("second inst is match", prog->code[1].op == I_MATCH);
	free(prog);
	return NULL;
}

static char* TestStar()
{
	program* prog = compile_regex("a*", NULL);
	mu_assert("program length", prog->size ==  4);
	mu_assert("first inst is split", prog->code[0].op == I_SPLIT);
	mu_assert("second inst is rule", prog->code[1].op == I_RULE);
	mu_assert("third inst is jmp", prog->code[2].op == I_JMP);
	mu_assert("final inst is match", prog->code[3].op == I_MATCH);
	free(prog);
	return NULL;
}

static char* TestConcat()
{
	program* prog = compile_regex("ab", NULL);
	mu_assert("program length", prog->size ==  3);
	mu_assert("first inst is split", prog->code[0].op == I_RULE);
	mu_assert("second inst is rule", prog->code[1].op == I_RULE);
	mu_assert("third inst is match", prog->code[2].op == I_MATCH);
	free(prog);
	return NULL;
}
static char* TestInvalid()
{
	re_error er;
	program* prog = compile_regex("*", &er);
	mu_assert("program is NULL", prog == NULL);
	mu_assert("error is unexpected star", er.errno = E_UNEXPECTED_TOKEN);
	return NULL;
}

void test_compiler()
{
	printf("Testing compiler\n");
	mu_run_test(TestSingleMatch);
	mu_run_test(TestConcat);
	mu_run_test(TestStar);
	mu_run_test(TestInvalid);
}

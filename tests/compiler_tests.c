#include "compiler_tests.h"
#include "minunit.h"
#include <re_compiler.h>
#include <stdio.h>

static char* TestSingleMatch()
{
	size_t nr, nc;
	program* prog = compile_regex("a", NULL, &nr, &nc);
	mu_assert("program length", prog->size ==  2);
	mu_assert("first inst is rule", prog->code[0].op == I_CHAR);
	mu_assert("second inst is match", prog->code[1].op == I_MATCH);
	free(prog);
	return NULL;
}

static char* TestStar()
{
	size_t nr, nc;
	program* prog = compile_regex("a*", NULL, &nr, &nc);
	mu_assert("program length", prog->size ==  4);
	mu_assert("first inst is split", prog->code[0].op == I_SPLIT);
	mu_assert("second inst is rule", prog->code[1].op == I_CHAR);
	mu_assert("third inst is jmp", prog->code[2].op == I_JMP);
	mu_assert("final inst is match", prog->code[3].op == I_MATCH);
	free(prog);
	return NULL;
}

static char* TestConcat()
{
	size_t nr, nc;
	program* prog = compile_regex("ab", NULL, &nr, &nc);
	mu_assert("program length", prog->size ==  3);
	mu_assert("first inst is split", prog->code[0].op == I_CHAR);
	mu_assert("second inst is rule", prog->code[1].op == I_CHAR);
	mu_assert("third inst is match", prog->code[2].op == I_MATCH);
	free(prog);
	return NULL;
}
static char* TestInvalid()
{
	size_t nr, nc;
	re_error er;
	program* prog = compile_regex("*", &er, &nr, &nc);
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

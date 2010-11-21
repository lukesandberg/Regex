#include "compiler_tests.h"
#include "minunit.h"
#include <re_compiler.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/util.h>
#include <re_parser.h>
#include <regex.h>

static int TestSingleMatch()
{
	regex* re = regex_create("a", NULL);
	program* prog = &(re->prog);
	mu_assert("program length", prog->size ==  2);
	mu_assert("first inst is rule", prog->code[0].op == I_CHAR);
	mu_assert("second inst is match", prog->code[1].op == I_MATCH);
	rfree(re);
	return 1;
}

static int TestStar()
{
	regex* re = regex_create("a*", NULL);
	program* prog = &(re->prog);
	mu_assert("program length", prog->size ==  4);
	mu_assert("first inst is split", prog->code[0].op == I_SPLIT);
	mu_assert("second inst is rule", prog->code[1].op == I_CHAR);
	mu_assert("third inst is jmp", prog->code[2].op == I_JMP);
	mu_assert("final inst is match", prog->code[3].op == I_MATCH);
	rfree(re);
	return 1;
}

static int TestConcat()
{
	regex* re = regex_create("ab", NULL);
	program* prog = &(re->prog);
	mu_assert("program length", prog->size ==  3);
	mu_assert("first inst is split", prog->code[0].op == I_CHAR);
	mu_assert("second inst is rule", prog->code[1].op == I_CHAR);
	mu_assert("third inst is match", prog->code[2].op == I_MATCH);
	rfree(re);
	return 1;
}
static int TestInvalid()
{
	re_error er;
	regex* re = regex_create("*", &er);
	mu_assert("program is NULL", re == NULL);
	mu_assert("error is unexpected star", er.errno = E_UNEXPECTED_TOKEN);
	return 1;
}

static int TestAlternation()
{
	regex* re = regex_create("a|b|c", NULL);
	program* prog = &(re->prog);
	mu_assert("program length", prog->size ==  8);
	
	mu_assert("first inst is split", prog->code[0].op == I_SPLIT);
	mu_assert("split ops are 2 and 1", prog->code[0].v.split.left == 2 && prog->code[0].v.split.right == 1);
	mu_assert("second inst is split", prog->code[1].op == I_SPLIT);
	mu_assert("split ops are 4 and 6", prog->code[1].v.split.left == 4 && prog->code[1].v.split.right == 6);
	mu_assert("third inst is CHAR", prog->code[2].op == I_CHAR);
	mu_assert("fourth inst is JMP", prog->code[3].op == I_JMP);
	mu_assert("jmp val is 7", prog->code[3].v.jump == 7);
	mu_assert("fifth inst is CHAR", prog->code[4].op == I_CHAR);
	mu_assert("sixth inst is JMP", prog->code[5].op == I_JMP);
	mu_assert("seventh inst is CHAR", prog->code[6].op == I_CHAR);
	mu_assert("eighth inst is match", prog->code[7].op == I_MATCH);
	
	rfree(re);
	return 1;
}
static int TestLoop()
{
	regex* re = regex_create("a{2,3}", NULL);
	program* prog = &(re->prog);
	mu_assert("program size should be 8", prog->size == 8);
	mu_assert("first inst is setz", prog->code[0].op == I_SETZ);
	mu_assert("setz reg should be 0", prog->code[0].v.idx == 0);
	mu_assert("second inst is split", prog->code[1].op == I_SPLIT);
	mu_assert("third inst is dgt", prog->code[2].op == I_DGTEQ);
	mu_assert("dgt comparison should be reg 0 <= 3", prog->code[2].v.comparison.idx == 0 && prog->code[2].v.comparison.comp == 3);
	mu_assert("fourth inst is char", prog->code[3].op == I_CHAR);
	mu_assert("fifth inst is incr", prog->code[4].op == I_INCR);
	mu_assert("sixth inst is jmp", prog->code[5].op == I_JMP);
	mu_assert("seventh inst is dlt", prog->code[6].op == I_DLT);
	mu_assert("dlt comparison should be reg 0 >= 2", prog->code[6].v.comparison.idx == 0 && prog->code[6].v.comparison.comp == 2);
	mu_assert("eigth inst is match", prog->code[7].op == I_MATCH);
	rfree(re);
	return 1;
}

void test_compiler()
{
	printf("Testing Compiler\n");
	mu_run_test(TestSingleMatch);
	mu_run_test(TestConcat);
	mu_run_test(TestStar);
	mu_run_test(TestInvalid);
	mu_run_test(TestLoop);
	mu_run_test(TestAlternation);
}

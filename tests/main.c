#include "minunit.h"
#include "lexer_tests.h"
#include "parser_tests.h"
#include "matcher_tests.h"
#include "compiler_tests.h"
#include "perf_tests.h"
#include "fuzz_tests.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char**argv)
{
	if(argc == 2 && argv[1][0] == 'p')
	{
		test_performance();
	}
	else if(argc == 1 || (argc == 2 && argv[1][0] == 't'))
	{
		test_lexer();
		test_parser();
		test_compiler();
		test_matcher();
		mu_print_summary();
	}
	else if(argc == 4 && argv[1][0] == 'r')
	{
		char* re = argv[2];
		char* str = argv[3];
		int m = match(re, str);
		printf("%s matches %s:\t%s\n", str, re, m?"true": "false"); 
	}
	else if(argc >= 2 && argv[1][0] == 'f')//fuzz
	{
		unsigned int count = 0;
		if(argc == 3)
		{
			count = (unsigned int) atoll(argv[2]);
		}
		fuzz_test(count);
	}
	else
	{
		printf("usage:\n\t%s p\tperformance tests\n\t%s t\tunit tests\n", argv[0], argv[0]);
	}
}

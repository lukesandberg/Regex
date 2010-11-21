#include "minunit.h"
#include "lexer_tests.h"
#include "parser_tests.h"
#include "matcher_tests.h"
#include "capture_tests.h"
#include "compiler_tests.h"
#include "perf_tests.h"
#include "fuzz_tests.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_tests.h"
#include <util/util.h>

void run_tests()
{
	test_lexer();
	test_parser();
	test_compiler();
	test_matcher();
	test_captures();
#ifdef MEM_TEST
	test_memory();
#endif
	mu_print_summary();
}

static void print_usage(char* prog_name)
{
	char* usage_format = "\t%s %-20s\t%s\n";
	printf("usage:\n");
	printf(usage_format, prog_name, "[t]", "unit tests");
	printf(usage_format, prog_name, "f [count=inf]", "fuzz test");
	printf(usage_format, prog_name, "p [count=1000]","performance tests");
	printf(usage_format, prog_name, "m ","memory tests, for proper handling of low memory situations");
	printf(usage_format, prog_name, "r <regex> <str>", "test if regex matches");
	printf(usage_format, prog_name, "c <regex> <str>", "test for match and extract captures");
}

int main(int argc, char**argv)
{
	if(argc == 2 && argv[1][0] == 'p')
	{
		test_performance(1000l);
	}
	if(argc == 2 && argv[1][0] == 'm')
	{
#ifndef MEM_TEST
		printf("You must define MEM_TEST to run the memory tests\n");
#else
		test_memory();
		mu_print_summary();;
#endif
	}
	else if(argc == 3 && argv[1][0] == 'p')
	{
		long long n = atoll(argv[2]);
		test_performance(n);
	}
	else if(argc == 1 || (argc == 2 && argv[1][0] == 't'))
	{
		run_tests();
	}
	else if(argc == 4 && argv[1][0] == 'r')
	{
		char* re = argv[2];
		char* str = argv[3];
		int m = match(re, str);
		printf("%s matches %s:\t%s\n", str, re, m?"true": "false"); 
	}
	else if(argc == 4 && argv[1][0] == 'c')
	{
		char* re = argv[2];
		char* str = argv[3];
		capture_group* cg;
		int m = capture(re, str, &cg);
		printf("%s matches %s:\t%s\n", str, re, m ? "true": "false");
		if(m)
		{
			for(unsigned int i = 0 ; i < cg_get_num_captures(cg); i++)
			{
				char *end = NULL;
				char* start = cg_get_capture(cg, i, &end);
				printf("%i:\t%.*s\n", i/2, (end - start), start);
			}
			cg_destroy(cg);
		}
	}
	else if(argc == 2 && argv[1][0] == 'f')//fuzz
	{
		fuzz_test(0);
	}
	else if(argc == 3 && argv[1][0] == 'f')//fuzz
	{
		fuzz_test((unsigned int) atoll(argv[2]));
	}
	else
	{
		print_usage(argv[0]);
	}
}

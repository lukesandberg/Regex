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


void run_tests()
{
	test_lexer();
	test_parser();
	test_compiler();
	test_matcher();
	test_captures();
	mu_print_summary();
}

int main(int argc, char**argv)
{
	if(argc == 2 && argv[1][0] == 'p')
	{
		test_performance();
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
	else if(argc == 5 && argv[1][0] == 'c')
	{
		char* re = argv[2];
		char* str = argv[3];
		int n = atoi(argv[4]);
		char** caps = (char**) calloc(2*n, sizeof(char*));
		int m = capture(re, str, caps);	
		printf("%s matches %s:\t%s\n", str, re, m ? "true": "false");
		if(m)
		{
			for(int i = 0 ; i < n*2; i +=2)
			{
				char* sub = strndup(caps[i], caps[i+1] - caps[i]);
				printf("%i:\t%s\n", i/2, sub);
				free(sub);
			}
		}
		free(caps);
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

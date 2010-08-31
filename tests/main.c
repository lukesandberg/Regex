#include "minunit.h"
#include "lexer_tests.h"
#include "parser_tests.h"
#include "matcher_tests.h"
#include "compiler_tests.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include "perf_tests.h"
#include <unistd.h>
#define MAX_RANDOM_STRING 10000
char* rand_string()
{
	int len = random()%MAX_RANDOM_STRING + 1;
	char* str = (char*) malloc(len +1);
	for(int i = 0; i<len;i++)
	{
		str[i] =random()%128;// chars[random()%cl];
	}
	str[len] = '\0';
	return str;
}


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
	else if(argc == 2 && argv[1][0] == 'f')//fuzz
	{
		unsigned long count = 0;
		unsigned long fail = 0;
		while(1)
		{
			char* re = rand_string();
			char* m = rand_string();
			char* command;
			asprintf(&command, "%s r \"%s\" \"%s\" >  /dev/null", argv[0], re, m);
			//we execute in a sub proc 
			count++;
			int ret = system(command);
			if(WEXITSTATUS(ret) != 0)
			{
				fail++;
				printf("%s returned %i\n", command, WEXITSTATUS(ret));
			}
			free(command);
			free(re);
			free(m);
			if(WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
				break;
		}
		printf("%li out of %li failed\n", fail, count);
	}
	else
	{
		printf("usage:\n\t%s p\tperformance tests\n\t%s t\tunit tests\n", argv[0], argv[0]);
	}
}

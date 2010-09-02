#include "fuzz_tests.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include "matcher_tests.h"

#define MAX_RANDOM_STRING 10000
char* rand_string()
{
	int len = random()%MAX_RANDOM_STRING + 1;
	char* str = (char*) malloc(len +1);
	for(int i = 0; i<len;i++)
	{
		str[i] =random()%127 + 1;// chars[random()%cl];
	}
	str[len] = '\0';
	return str;
}

static int fuzz()
{
	//we execute in a sub proc so that crashes don't bring us down 
	pid_t child;
	child = fork();
	if(child == 0)
	{
		char* re = rand_string();
		char* m = rand_string();
		//child
		match(re, m);
		free(re);
		free(m);
		exit(0);
	}
	else if(child > 0)
	{
		int status;
		do
		{
			if( waitpid(child, &status, 0) == -1)
			{
				perror("waitpid");
				exit(EXIT_FAILURE);
			}
			
			if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGINT || WTERMSIG(status) == SIGQUIT))
        	               return -1;

		} while (!WIFEXITED(status));
		if(WEXITSTATUS(status) != 0)
			return 1;
		return 0;
	}
	else
	{
		perror("fork failed");
		exit(1);
	}
}


void fuzz_test(unsigned int count)
{
	int flag = !count;
	unsigned int failures = 0;
	unsigned int total = 0;
	for(; total < count || flag; total++)
	{
		int v = fuzz();
		if(v >0) failures ++;
		else if(v <0)
		{
			goto end;
		}
	}
	
end:
	printf("%i out of %i failed\n", failures, total);
}


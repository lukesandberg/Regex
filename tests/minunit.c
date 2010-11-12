#include "minunit.h"
#include <stdlib.h>
#include <stdio.h>

int tests_run = 0;
int failures = 0;
int exit_early = 0;
int verbose = 1;
void mu_process_result(char * tn, char* result)
{
	tests_run++;
	if(result == NULL)
	{
		if(verbose)
		{
			printf("%i\t%s:\tSuccess\n", tests_run, tn);
		}
	}
	else
	{
		failures++;
		printf("%i\t%s:\tFailure\n\t\t\t%s\n", tests_run, tn, result);
	}
}

void mu_print_summary()
{
	printf("%i out of %i tests passed\n", (tests_run - failures), tests_run);
}

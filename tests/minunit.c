#include "minunit.h"
#include <stdlib.h>
#include <stdio.h>

int tests_run = 0;
int failures = 0;
int exit_early = 0;
int verbose = 1;
static char* prefix_fmt = "  %-5i\t";
static char* success_fmt = "%-30s\tSuccess\n";
static char* failure_fmt = "%-30s\tFailure\t%20s:%i\t%s\n";

void mu_process_result(char * tn, char* result, char* filename, unsigned int ln)
{
	tests_run++;
	printf(prefix_fmt, tests_run);
	if(result == NULL)
	{
		if(verbose)
		{
			printf(success_fmt, tn);
		}
	}
	else
	{
		failures++;
		printf(failure_fmt, tn, filename, ln, result);
	}
}

void mu_print_summary()
{
	printf("%i out of %i tests passed\n", (tests_run - failures), tests_run);
}

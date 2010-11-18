#include "minunit.h"
#include <stdlib.h>
#include <stdio.h>

int tests_run = 0;
int failures = 0;
int verbose = 1;
static char* prefix_fmt = "  %-5i\t";
static char* success_fmt = "%-30s\tSuccess\n";
static char* failure_fmt = "%-30s\tFailure\t%20s:%i\t%s\n";
static char* current_test;

void mu_start_test(char* test)
{
	current_test = test;
	tests_run++;
}

void mu_fail(char* msg, char* filename, unsigned int ln)
{
	failures++;
	printf(prefix_fmt, tests_run);
	printf(failure_fmt, current_test, filename, ln, msg);
}

void mu_succeed()
{
	if(verbose)
	{
		printf(prefix_fmt, tests_run);
		printf(success_fmt, current_test);
	}
}

void mu_print_summary()
{
	printf("%i out of %i tests passed\n", (tests_run - failures), tests_run);
}

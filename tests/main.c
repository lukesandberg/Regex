#include "minunit.h"
#include "lexer_tests.h"
#include "parser_tests.h"
#include "matcher_tests.h"
#include "compiler_tests.h"

int main(int argc, char**argv)
{
	test_lexer();
	test_parser();
	test_compiler();
	test_matcher();
	mu_print_summary();
}

#include "parser_tests.h"
#include "minunit.h"
#include <parser/re_parser.h>
#include <stdio.h>

static char* TestEmpty()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be an empty node", tree->type == EMPTY);
	free_node(tree);
	return NULL;
}
static char* TestSingleMatch()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("a", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a match node", tree->type == MATCH);
	free_node(tree);

	return NULL;
}
static char* TestSingleEscape()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("\\\\", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a match node", tree->type == MATCH);
	free_node(tree);

	return NULL;
}
static char* TestSingleStar()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("a*", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a star node", tree->type == STAR);
	unary_node* sn = (unary_node*) tree;
	mu_assert("should be a match inside the star", sn->expr->type == MATCH);
	free_node(tree);

	return NULL;
}
static char* TestConcat()
{
	re_error er;
	ast_node* tree;
		
	tree = re_parse("ab", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a concat node", tree->type == CONCAT);
	binary_node* cn = (binary_node*) tree;
	mu_assert("left should be a match", cn->left->type == MATCH);
	mu_assert("right should be a match", cn->right->type == MATCH);
	free_node(tree);

	return NULL;
}

static char* TestConcatStarPrecedence()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("ab*", &er);	
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a concat node", tree->type == CONCAT);
	binary_node* cn = (binary_node*) tree;
	mu_assert("left should be a match", cn->left->type == MATCH);
	mu_assert("right should be a star", cn->right->type == STAR);
	unary_node* sn = (unary_node*) cn->right;
	mu_assert("should be a match inside the star inside the concat", sn->expr->type == MATCH);
	free_node(tree);

	return NULL;
}


static char* TestInvalid()
{
	re_error er;
	ast_node* tree;
	tree = re_parse("*", &er);
	mu_assert("should be missing argument", er.errno == E_MISSING_OP_ARGUMENT);
	mu_assert("invalid parse should give NULL output", tree == NULL);	
	return NULL;
}

static char* TestNullError()
{
	ast_node* tree = re_parse("a*", NULL);
	mu_assert("should be non null", tree != NULL);
	tree = re_parse("*", NULL);
	mu_assert("should be null", tree == NULL);
	return NULL;
}

void test_parser()
{
	printf("Testing parser\n");
	mu_run_test(TestEmpty);
	mu_run_test(TestSingleMatch);
	mu_run_test(TestSingleEscape);
	mu_run_test(TestSingleStar);
	mu_run_test(TestConcat);

	mu_run_test(TestConcatStarPrecedence);
	mu_run_test(TestInvalid);
	mu_run_test(TestNullError);
}

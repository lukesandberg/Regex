#include <parser_tests.h>
#include <minunit.h>
#include <re_parser.h>
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
	mu_assert("should be a char node", tree->type == CHAR);
	free_node(tree);

	return NULL;
}
static char* TestSingleEscape()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("\\\\", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a char node", tree->type == CHAR);
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
	mu_assert("should be a char inside the star", sn->expr->type == CHAR);
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
	multi_node* cn = (multi_node*) tree;
	linked_list_node* n = linked_list_first(cn->list);
	mu_assert("first should be a match", ((ast_node*)linked_list_value(n))->type == CHAR);
	n = linked_list_next(n);
	mu_assert("second should be a match", ((ast_node*)linked_list_value(n))->type == CHAR);
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
	multi_node* cn = (multi_node*) tree;
	linked_list_node* n = linked_list_first(cn->list);
	mu_assert("first should be a match", ((ast_node*) linked_list_value(n))->type == CHAR);
	n = linked_list_next(n);
	unary_node* sn = (unary_node*) linked_list_value(n);
	mu_assert("second should be a star", sn->base.type == STAR);
	mu_assert("should be a match inside the star inside the concat", sn->expr->type == CHAR);
	free_node(tree);

	return NULL;
}

static char* TestSubExpression()
{
	re_error er;
	ast_node* tree;
	tree = re_parse("a(bc)d", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a concat node", tree->type == CONCAT);
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
	free_node(tree);
	tree = re_parse("*", NULL);
	mu_assert("should be null", tree == NULL);
	return NULL;
}


static char* TestCountedRepetition()
{
    	re_error er;
	ast_node* tree = re_parse("a{2}", &er);
	if(tree == NULL)
	{
	    return (char*) re_error_description(er);
	}
	mu_assert("should be loop node", tree->type==CREP);
	loop_node* ln = (loop_node*) tree;
	mu_assert("min loop range should be 2", ln->min == 2);
	mu_assert("max loop range should be 2", ln->max == 2);
	mu_assert("expr should be CHAR expr", ln->base.expr->type==CHAR);
	mu_assert("expr should be a value", ((char_node*)(ln->base.expr))->c=='a');
	free_node(tree);

	tree = re_parse("a{2, 4}", &er);
	if(tree == NULL)
	{
	    return (char*) re_error_description(er);
	}
	mu_assert("should be loop node", tree->type==CREP);
	ln = (loop_node*) tree;
	mu_assert("min loop range should be 2", ln->min == 2);
	mu_assert("max loop range should be 4", ln->max == 4);
	mu_assert("expr should be CHAR expr", ln->base.expr->type==CHAR);
	mu_assert("expr should be a value", ((char_node*)(ln->base.expr))->c=='a');
	free_node(tree);
	return NULL;
}
static char* TestCountedRepetitionGroup()
{
    	re_error er;
	ast_node* tree = re_parse("(abc){2}", &er);
	if(tree == NULL)
	{
	    return (char*) re_error_description(er);
	}
	mu_assert("should be loop node", tree->type==CREP);
	loop_node* ln = (loop_node*) tree;
	mu_assert("min loop range should be 2", ln->min == 2);
	mu_assert("max loop range should be 2", ln->max == 2);
	mu_assert("expr should be CAPTURE expr", ln->base.expr->type==CAPTURE);
	unary_node* cg = (unary_node*) ln->base.expr;
	mu_assert("capture should be concat group", cg->expr->type == CONCAT);
	free_node(tree);
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
	mu_run_test(TestSubExpression);

	mu_run_test(TestConcatStarPrecedence);
	mu_run_test(TestInvalid);
	mu_run_test(TestNullError);
	mu_run_test(TestCountedRepetition);
	mu_run_test(TestCountedRepetitionGroup);
}

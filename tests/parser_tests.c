#include <parser_tests.h>
#include <minunit.h>
#include <re_parser.h>
#include <stdio.h>

static int TestEmpty()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be an empty node", tree->type == EMPTY);
	free_node(tree);
	return 1;
}
static int TestSingleMatch()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("a", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a char node", tree->type == CHAR);
	free_node(tree);

	return 1;
}
static int TestSingleEscape()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("\\\\", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a char node", tree->type == CHAR);
	free_node(tree);

	return 1;
}
static int TestSingleStar()
{
	re_error er;
	ast_node* tree;
	
	tree = re_parse("a*", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a star node", tree->type == STAR);
	unary_node* sn = (unary_node*) tree;
	mu_assert("should be a char inside the star", sn->expr->type == CHAR);
	free_node(tree);

	return 1;
}
static int TestConcat()
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

	return 1;
}

static int TestConcatStarPrecedence()
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

	return 1;
}

static int TestSubExpression()
{
	re_error er;
	ast_node* tree;
	tree = re_parse("a(bc)d", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a concat node", tree->type == CONCAT);
	free_node(tree);
	return 1;
}

static int TestInvalid()
{
	re_error er;
	ast_node* tree;
	tree = re_parse("*", &er);
	mu_assert("should be missing argument", er.errno == E_MISSING_OP_ARGUMENT);
	mu_assert("invalid parse should give NULL output", tree == NULL);	
	return 1;
}

static int TestAlteration()
{
	re_error er;
	ast_node* tree;
	tree = re_parse("a|b", &er);
	mu_assert("should be a valid parse", er.errno == E_SUCCESS);
	mu_assert("should be a alternation node", tree->type == ALT);
	linked_list_node* node = linked_list_first(((multi_node*) tree)->list);
	mu_assert("should be a CHAR", ((ast_node*)linked_list_value(node))->type == CHAR);
	node = linked_list_next(node);
	mu_assert("should be a CHAR", ((ast_node*)linked_list_value(node))->type == CHAR);
	free_node(tree);
	return 1;
}

static int TestNullError()
{
	ast_node* tree = re_parse("a*", NULL);
	mu_assert("should be non null", tree != NULL);
	free_node(tree);
	tree = re_parse("*", NULL);
	mu_assert("should be null", tree == NULL);
	return 1;
}


static int TestCountedRepetition()
{
	re_error er;
	ast_node* tree = re_parse("a{2}", &er);
	mu_assert((char*) re_error_description(er), tree != NULL);

	mu_assert("should be loop node", tree->type==CREP);
	loop_node* ln = (loop_node*) tree;
	mu_assert("min loop range should be 2", ln->min == 2);
	mu_assert("max loop range should be 2", ln->max == 2);
	mu_assert("expr should be CHAR expr", ln->base.expr->type==CHAR);
	mu_assert("expr should be a value", ((char_node*)(ln->base.expr))->c=='a');
	free_node(tree);

	tree = re_parse("a{2, 4}", &er);
 	mu_assert((char*) re_error_description(er), tree != NULL);
	
	mu_assert("should be loop node", tree->type==CREP);
	ln = (loop_node*) tree;
	mu_assert("min loop range should be 2", ln->min == 2);
	mu_assert("max loop range should be 4", ln->max == 4);
	mu_assert("expr should be CHAR expr", ln->base.expr->type==CHAR);
	mu_assert("expr should be a value", ((char_node*)(ln->base.expr))->c=='a');
	free_node(tree);
	return 1;
}
static int TestCountedRepetitionGroup()
{
	re_error er;
	ast_node* tree = re_parse("(abc){2}", &er);
 	mu_assert((char*) re_error_description(er), tree != NULL);
	mu_assert("should be loop node", tree->type==CREP);
	loop_node* ln = (loop_node*) tree;
	mu_assert("min loop range should be 2", ln->min == 2);
	mu_assert("max loop range should be 2", ln->max == 2);
	mu_assert("expr should be CAPTURE expr", ln->base.expr->type==CAPTURE);
	unary_node* cg = (unary_node*) ln->base.expr;
	mu_assert("capture should be concat group", cg->expr->type == CONCAT);
	free_node(tree);
	return 1;
}

static int TestCountedRepetitionSequence()
{
	re_error er;
	ast_node* tree = re_parse("a{2}.*", &er);
 	mu_assert((char*) re_error_description(er), tree != NULL);
	mu_assert("should be a concat node", tree->type == CONCAT);
	multi_node* cn = (multi_node*) tree;
	linked_list_node* n = linked_list_first(cn->list);
	mu_assert("first should be a loop", ((ast_node*) linked_list_value(n))->type == CREP);

	loop_node* ln = (loop_node*) linked_list_value(n);
	mu_assert("min loop range should be 2", ln->min == 2);
	mu_assert("max loop range should be 2", ln->max == 2);
	mu_assert("expr should be CHAR expr", ln->base.expr->type==CHAR);
	mu_assert("expr should be a value", ((char_node*)(ln->base.expr))->c=='a');

	n = linked_list_next(n);
	mu_assert("next should be a STAR", ((ast_node*) linked_list_value(n))->type == STAR);
	unary_node* un = (unary_node*) linked_list_value(n);
	mu_assert("should be a match inside the star", un->expr->type == WILDCARD);
	
	free_node(tree);
	return 1;
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
	mu_run_test(TestAlteration);
	mu_run_test(TestCountedRepetition);
	mu_run_test(TestCountedRepetitionGroup);
	mu_run_test(TestCountedRepetitionSequence);
}

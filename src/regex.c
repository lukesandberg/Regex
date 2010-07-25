#include "parse.h"
#include "regex.h"
#include <stdlib.h>

struct regex_s
{
	node* graph;//the whole regex graph
	node* current;//the current point 
};

regex* regex_create(char* re_str)
{
	regex* re = (regex*) malloc(sizeof(regex));
	re->graph = node_graph_create(re_str);
	re->current = NULL;
	return re;
}

void regex_destroy(regex* re)
{
	node_graph_destroy(re->graph);
	free(re);
}

static int regex_advance(regex* re)
{
	if(re->current == NULL)//first step
		re->current = re->graph;
	else
		re->current = re->current->next;
	return re->current != NULL;
}

static int regex_matches_char(regex* re, char c)
{
	if(re->current == NULL)
		return 0;
	return node_matches_char(re->current, c);
}

int regex_matches(regex *re, char*str)
{
	while(regex_advance(re))
	{
		if(!regex_matches_char(re, *str))
			return 0;
		str++;
	}
	//we exit when we have run out of nodes to advance to
	if(*str == '\0')
		return 1;
	return 0;
}

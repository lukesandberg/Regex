#include "parse.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

static node* make_char_match_node(char c)
{
	node* nn = (node*) malloc(sizeof(node));
	nn->value = c;
	nn->rule = CHAR_MATCH;
	return nn;
}

static node* make_wildcard_node()
{
	node* nn = (node*) malloc(sizeof(node));
	nn->value = '\0';
	nn->rule = WILDCARD;
	return nn;
}

void node_graph_destroy(node* n)
{
	if(n == NULL) return;
	if(n->next != NULL)
	{
		node_graph_destroy(n->next);
	}
	free(n);
}

int node_matches_char(node* n, char c)
{
	switch(n->rule)
	{
		case CHAR_MATCH:
			return c == n->value;
		case WILDCARD:
			return c != '\0';//the only invalid character for the
					//wildcard is the null char
	}
	return 0;
}


//start simple
node* node_graph_create(char* re)
{
	node* first = NULL;
	node* prev = NULL;
	char c = '\0';
	while((c = *re++))
	{
		node* cur = NULL;
		if(c == '\\')//begin escape sequence
		{
			c = *re++;
			if(c == '\\' || c == '.')
			{
				cur = make_char_match_node(c);
			}
			else
			{
				fprintf(stderr, "unexpected escape sequence: '\\%c\n", c);
			       exit(1);	
			}
		}
		else if( c == '.')//wildcard
		{
			cur = make_wildcard_node();
		}
		else
		{
			cur = make_char_match_node(c);
		}

		//at this point cur is non null
		if(first == NULL)
		{
			first = prev = cur;
		}
		else
		{
			prev->next = cur;
			prev = cur;
		}
	}
	if(first == NULL)
	{
		//the re string was empty
		//return our special empty re
		first = make_char_match_node('\0');
	}
	return first;
}

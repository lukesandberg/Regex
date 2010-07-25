#ifndef __PARSE_H__
#define __PARSE_H__

enum rule_e
{
	START,//default starting state
	END,//ending state
	CHAR_MATCH,
	WILDCARD// DOT character
};

//for now our regexes are really simple
//just match a character or a wildcard
struct node_s
{
	enum rule_e rule;
	char value;
	struct node_s* next;
};

typedef struct node_s node;

node* node_graph_create(char* ng);
void node_graph_destroy(node* ng);
int node_matches_char(node* n, char c);
#endif

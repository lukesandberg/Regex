#ifndef __RE_LEX_H_
#define __RE_LEX_H_

typedef enum
{
	END_TOK,
	INVALID_TOK,
	CHAR_TOK,
	PLUS_TOK,//+
	QMARK_TOK,//?
	ALT_TOK,//|
	STAR_TOK,//*
	WILDCARD_TOK,//.
	DIGIT_TOK,//\d
	WHITESPACE_TOK,//\s
	ALPHA_TOK,//\w
	LPAREN_TOK,//(
	RPAREN_TOK,//)
} token_type;

typedef struct
{
	token_type type;
	unsigned int position;
	union
	{
		char char_value;	
	} v;
} token;


typedef struct
{
	char* str;
	int pos;
} lexer;


void init_lexer(lexer* lexer, char* str);
//behavior is undefined if you call read_token after receiving an END
token read_token(lexer* lexer);
void unread_token(lexer* l, token t);

#endif

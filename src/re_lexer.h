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
	NG_STAR_TOK,//*?
	NG_PLUS_TOK,//+?
	NG_QMARK_TOK,//??
	LCR_TOK,//{
	RCR_TOK,//}
	COMMA_TOK,//, only a special tok inside {}'s
	NUM_TOK,//a number only inside {}'s
} token_type;

typedef struct
{
	token_type type;
	unsigned int position;
	union
	{
		char char_value;
		unsigned int num_value;	
	} v;
} token;


typedef struct
{
	char* str;
	unsigned int:1 in_cr;
	unsigned int:1 has_num1;
	unsigned int:1 has_num2;
	unsigned int:1 past_comma;
	unsigned int pos;
} lexer;


void init_lexer(lexer* lexer, char* str);
//behavior is undefined if you call read_token after receiving an END
token read_token(lexer* lexer);

#endif

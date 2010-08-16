#include <parser/re_lexer.h>
#include <stdio.h>
//tests if c is a special char
static inline int is_special(char c)
{
	return 	   c == '*' 
		|| c == '\\' 
		|| c == '|' 
		|| c == '+'
		|| c == '?'
		|| c == '.'
		|| c == '('
		|| c == ')';
}

struct lexer_s
{
	char* str;
	int pos;
};

void init_lexer(lexer* l, char* str)
{
	l->str = str;
	l->pos = 0;
}
void unread_token(lexer* l, token t)
{
	l->pos = t.position;
}
token read_token(lexer* l)
{
	int start_pos = l->pos;
	char c = l->str[start_pos];
	l->pos++;
	token tok;
	if(c != '\0')
	{
		tok.type = INVALID_TOK;
		tok.position = start_pos;
		switch(c)
		{
			case '*':
				tok.type = STAR_TOK;
				break;
			case '+':
				tok.type = PLUS_TOK;
				break;
			case '.':
				tok.type = WILDCARD_TOK;
				break;
			case '?':
				tok.type = QMARK_TOK;
				break;
			case '|':
				tok.type = ALT_TOK;
				break;
			case '(':
				tok.type = LPAREN_TOK;
				break;
			case ')':
				tok.type = RPAREN_TOK;
				break;
			case '\\':
				c = l->str[l->pos];
				l->pos++;
				if(is_special(c))
				{//this is an escape sequence
					tok.type = CHAR_TOK;
					tok.v.char_value = c;
				}
				else
				{
					//we need to test for character classes
					switch(c)
					{
						case 'w':
							tok.type = ALPHA_TOK;
							break;
						case 's':
							tok.type = WHITESPACE_TOK;
							break;
						case 'd':
							tok.type = DIGIT_TOK;
							break;
					}
					//o.w. we fall through to invalid
				}
				break;
			default://everything else is just a char match
				tok.type = CHAR_TOK;
				tok.v.char_value = c;
		}
		return tok;
	}
	tok.type = END_TOK;
	return tok;
}

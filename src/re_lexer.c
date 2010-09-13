#include <re_lexer.h>
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
	tok.type = INVALID_TOK;
	tok.position = start_pos;
	if(c != '\0')
	{
		char nc = l->str[start_pos + 1];//one look ahead char
		switch(c)
		{
			case '*':
				if(nc == '?')
				{
					l->pos++;
					tok.type = NG_STAR_TOK;
				}
				else
					tok.type = STAR_TOK;
				break;
			case '+':
				if(nc == '?')
				{
					l->pos++;
					tok.type = NG_PLUS_TOK;
				}
				else
					tok.type = PLUS_TOK;
				break;
			case '?':
				if(nc == '?')
				{
					l->pos++;
					tok.type = NG_QMARK_TOK;
				}
				else
					tok.type = QMARK_TOK;
				break;
			case '.':
				tok.type = WILDCARD_TOK;
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
				l->pos++;//advance one tok
				if(is_special(nc))
				{//this is an escape sequence
					tok.type = CHAR_TOK;
					tok.v.char_value = nc;
				}
				else
				{
					//we need to test for character classes
					switch(nc)
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

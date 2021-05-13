#include "re_lexer.h"
#include <stdio.h>
#include <ctype.h>

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
		|| c == ')'
		|| c == '{'
		|| c == '}';
}

void init_lexer(lexer* l, char* str)
{
	l->str = str;
	l->pos = 0;
	l->in_cr = 0;
	l->past_comma = 0;
	l->has_num1 = 0;
	l->has_num2 = 0;
}

token read_token(lexer* l)
{
	int start_pos = l->pos;
	char c = l->str[start_pos];
	l->pos++;
	token tok;
	tok.type = INVALID_TOK;
	tok.position = start_pos;
	if(l->in_cr)
	{
		//now we are parsing either a number or a comma
		//numbers can be proceeded or followed by whitespace
		//which is uninteresting
		//if we had an re engine we could capture with
		//\s*(\d+)\s*(,\s*(\d+)\s*)} and pull out capture
		//groups 1 and 3
		while(isspace(c))
		{
			c = l->str[l->pos];
			l->pos++;
		}
		//at this point it is either a digit a comma or a }
		if(c == ',')
		{
			if(l->has_num1 && !l->past_comma)
			{
				l->past_comma = 1;
				tok.type = COMMA_TOK;
			}
		}
		else if(c == '}')
		{
			if((l->has_num1 && !l->past_comma && !l->has_num2)
				|| (l->has_num1 && l->past_comma && l->has_num2))
			{
				l->in_cr =0;
				l->past_comma = 0;
				l->has_num1 = 0;
				l->has_num2 = 0;
				tok.type = RCR_TOK;
			}
		}
		else if(isdigit(c))
		{
			unsigned int num = (c -'0');
			c = l->str[l->pos];
			while(isdigit(c))
			{
				num = num * 10 + (c -'0');
				l->pos++;
				c = l->str[l->pos];
			}
			if(!l->has_num1 || (l->has_num1 && l->past_comma && !l->has_num2))
			{
				tok.type = NUM_TOK;
				tok.v.num_value = num;
				if(!l->has_num1)
				{
					l->has_num1 = 1;
				}
				else
				{
					l->has_num2 = 1;
				}
			}
		}
	}
	else if(c != '\0')
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
			case '{':
				if(l->in_cr)
				{
					//nested {}'s are invalid
					break;
				}
				tok.type = LCR_TOK;
				l->in_cr = 1;
				break;
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
	else
	{
		tok.type = END_TOK;
	}
	return tok;
}

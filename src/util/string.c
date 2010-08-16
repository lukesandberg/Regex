#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <util/string.h>
#include <util/util.h>

#define DEFAULT_STRING_INCREMENT 1024

struct string_s
{
	char* s;
	int len;
	int cap;
};

string_t* string_create()
{
	string_t* s = checked_malloc(sizeof(struct string_s));
	s->s = checked_malloc(sizeof(char)*DEFAULT_STRING_INCREMENT);
	s->len = 0;
	s->cap = DEFAULT_STRING_INCREMENT;
	return s;
}

void string_free(string_t* str)
{
	checked_free(str->s);
	checked_free(str);
}

static void string_copy_to_new_char_array(string_t* str, size_t new_size)
{
	char* ns = checked_malloc(new_size);
	memcpy(ns, str->s, str->len+1);
	checked_free(str->s);
	str->s = ns;
	str->cap = new_size;
}

void string_append_char(string_t* str, char c)
{
	if(str->len >= str->cap - 1)
	{
		string_copy_to_new_char_array(str, str->cap + DEFAULT_STRING_INCREMENT);
	}
	str->s[str->len] = c;
	str->len++;
}

void string_append(string_t* str, char* s)
{
	int slen = strlen(s);
	while(str->len + slen >= str->cap - 1)
	{
		string_copy_to_new_char_array(str, str->cap + DEFAULT_STRING_INCREMENT);
	}
	memcpy(str->s + str->len, s, slen);
	str->len += slen;
}
string_t* string_concat(string_t* str1, string_t* str2)
{
	string_t* ns = string_create();
	string_append(ns, string_cstring(str1));
	string_append(ns, string_cstring(str2));
	return ns;
}

char* string_cstring(string_t* str)
{
	assert(str->s[str->len] == '\0');
	return str->s;
}
void string_trim(string_t* str)
{
	string_copy_to_new_char_array(str, str->len + 1);
}
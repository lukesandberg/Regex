#ifndef __string_h__
#define __string_h__

typedef struct string_s string_t;

string_t* string_create ();
void string_free(string_t* str);
void string_append(string_t* str, char* s);
void string_append_char(string_t* str, char c);
string_t* string_concat(string_t* str1, string_t* str2);
char* string_cstring(string_t* str);
void string_trim();

#endif
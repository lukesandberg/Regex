#ifndef __REGEX_H__
#define __REGEX_H__

#include <re_error.h>
#include <capture_group.h>
typedef struct regex_s regex;

regex* regex_create(char* re, re_error* er);
void regex_destroy(regex* re);
//1 means it matched
//0 means it didnt match
//-1 means an error ocurred (usually an OOM)
int regex_matches(regex* re, char *str, capture_group** caps);

#endif

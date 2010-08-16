#ifndef __REGEX_H__
#define __REGEX_H__
#include "re_error.h"

typedef struct regex_s regex;

regex* regex_create(char* re, re_error* er);
void regex_destroy(regex* re);
int regex_matches(regex* re, char *str);

#endif

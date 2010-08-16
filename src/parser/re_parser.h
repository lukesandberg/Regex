#ifndef __RE_PARSER_H__
#define __RE_PARSER_H__
#include "re_ast.h"
#include <re_error.h>

ast_node* re_parse(char* regex, re_error* er);

#endif

#include "re_error.h"
#include <stdio.h>

static const char *error_messages[NUM_ERROR_CODES] = 
{
	"success",
	"unexpected character token",
	"unexpected token",
	"unmatched paren",
	"expected a token",
	"missing operator argument",
	"system out of memory",
	"unknown token"
};
static const char* unknown_error_msg = "unknown error code";
const char* re_error_description(re_error er)
{
	if(er.errno < 0 || er.errno >= NUM_ERROR_CODES)
		return unknown_error_msg;
	return error_messages[er.errno];
}

void re_error_print(re_error er)
{
	printf("regex error: %s at character %i\n", re_error_description(er), er.position);
}



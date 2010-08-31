#ifndef _RE_ERROR_H_
#define _RE_ERROR_H_

typedef enum
{
	E_SUCCESS,
	E_UNEXPECTED_CHAR,
	E_INVALID_TOKEN,
	E_UNMATCHED_PAREN,
	E_UNEXPECTED_TOKEN,
	E_EXPECTED_TOKEN,
	E_MISSING_OP_ARGUMENT,
	E_OUT_OF_MEMORY,
	NUM_ERROR_CODES
} re_error_code;

typedef struct
{
	re_error_code errno;
	int position;
}re_error;

const char* re_error_description(re_error er);
void re_error_print(re_error er);

#endif

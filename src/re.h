#ifndef __RE_H__
#define __RE_H__
/*this is the main header file for the regex project
  it contains all the symbols and procedures neccesary to use the regex
  library*/

/*forward type declarations */
typedef struct _cg_s capture_group;
typedef struct regex_s regex;

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

regex* regex_create(char* re_str, re_error* er);
void regex_destroy(regex* re);
//1 means it matched
//0 means it didnt match
//-1 means an error ocurred (usually an OOM)
int regex_matches(regex* re, char *str, capture_group** caps);

const char* re_error_description(re_error er);
void re_error_print(re_error er);


char* cg_get_capture(capture_group* cg, unsigned int n, char**end);
unsigned int cg_get_num_captures(capture_group* cg);
#endif
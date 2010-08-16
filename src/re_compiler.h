#ifndef __RE_COMPILER_H__
#define __RE_COMPILER_H__
#include <vm.h>
#include <re_error.h>

program* compile_regex(char* str, re_error* er);

#endif


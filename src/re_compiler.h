#ifndef __RE_COMPILER_H__
#define __RE_COMPILER_H__
#include <vm.h>
#include <re_error.h>

program* compile_regex(char* str, re_error* er, size_t* num_save_regs, size_t* num_loop_vars);

#endif


#ifndef __RE_COMPILER_H__
#define __RE_COMPILER_H__

#include <vm.h>
#include <re.h>

program* compile_regex(char* str, re_error* er, size_t* num_regs, size_t* num_capture_regs);

#endif


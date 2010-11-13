#ifndef __REGEX_H__
#define __REGEX_H__
#include <stdlib.h>
#include <re.h>
#include <vm.h>

struct regex_s
{
	char* src;
	size_t num_registers;
	program *prog;
	size_t num_capture_regs;
};
#endif

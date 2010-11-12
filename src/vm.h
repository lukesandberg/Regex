#ifndef __RE_VM_H_
#define __RE_VM_H_
#include <stdlib.h>

typedef enum
{
	I_CHAR,
	I_WILDCARD,
	I_ALPHA,
	I_DIGIT,
	I_WHITESPACE,
	I_JMP,
	I_SPLIT,
	I_MATCH,
	I_SAVE,
	I_DGTEQ,
	I_DLT,
	I_SETZ,
	I_INCR
}op_code;

typedef struct _inst_s
{
	op_code op;
	union
	{
		char c;
		unsigned int save_register;
		unsigned int idx;//for setz incr instructions
		unsigned int jump;
		struct s_s
		{
			unsigned int left;
			unsigned int right;
		} split;
		struct c_s
		{
			unsigned int idx;
			unsigned int comp;
		} comparison;
	} v;
} instruction;

typedef struct
{
	size_t size;
	instruction code[];
}program;

void print_program(program* prog);
void print_instruction(instruction *pc, size_t index);



#endif

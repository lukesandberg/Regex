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
	I_SAVE
}op_code;

typedef struct _inst_s
{
	op_code op;
	union
	{
		char c;
		size_t save_register;
		unsigned int jump;
		struct
		{
			unsigned int left;
			unsigned int right;
		} split;
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

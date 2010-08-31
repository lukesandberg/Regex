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
	I_MATCH
}op_code;

typedef struct _inst_s
{
	op_code op;
	union
	{
		char c;
		struct _inst_s* jump;
		struct
		{
			struct _inst_s* left;
			struct _inst_s* right;
		}split;
	} v;
	void* tag;
	//space to hold a value for additional data needed by the runtime
	//this kind of violates the standard vm abstraction
	//because we expect this member to be modified at runtime
	//but generally breaking an abstraction to switch from O(N) to O(1) is
	//worth it
} instruction;

typedef struct
{
	size_t size;
	instruction code[];
}program;

void print_program(program* prog);
void print_instruction(instruction *pc, size_t index);



#endif

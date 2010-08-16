#ifndef __RE_VM_H_
#define __RE_VM_H_
#include <stdlib.h>

typedef enum
{
	I_RULE,
	I_JMP,
	I_SPLIT,
	I_MATCH
}op_code;

typedef struct
{
	op_code op;
	union
	{
		unsigned int rule;
		unsigned int jump;
		struct
		{
			unsigned int left;
			unsigned int right;
		}split;
	} v;
} instruction;

typedef struct
{
	size_t size;
	instruction code[];
}program;




#endif

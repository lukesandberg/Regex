#include <vm.h>
#include <stdio.h>

void print_instruction(instruction *pc, size_t ind )
{
	switch(pc->op)
	{
		case I_SAVE:
			printf("CHAR: %c", pc->v.save_register);
			break;
		case I_CHAR:
			printf("CHAR: %c", pc->v.c);
			break;
		case I_JMP:
			printf("JMP: %i", ind + (int)(pc - pc->v.jump));
			break;
		case I_SPLIT:
			printf("SPLIT: %i %i", ind + (int)(pc - pc->v.split.left),ind + (int)(pc - pc->v.split.right));
			break;
		case I_ALPHA:
			printf("ALPHA");
			break;
		case I_DIGIT:
			printf("DIGIT");
			break;
		case I_WILDCARD:
			printf("WILDCARD");
			break;
		case I_WHITESPACE:
			printf("WHITESPACE");
			break;
		case I_MATCH:
			printf("MATCH");
			break;
		case I_SETZ:
			printf("SETZ: %i", pc->v.idx);
			break;
		case I_INCR:
			printf("INCR: %i", pc->v.idx);
			break;
		case I_DGTEQ:
			printf("DGT: %i %i", pc->v.comparison.idx, pc->v.comparison.comp);
			break;
		case I_DLT:
			printf("DLT: %i %i", pc->v.comparison.idx, pc->v.comparison.comp);
			break;
	}
}

void print_program(program* prog)
{
	size_t sz = prog->size;
	printf("program: %i\n", sz);
	for(size_t i = 0; i < sz; i++)
	{
		printf("%i:\t", i);
		instruction *pc = &(prog->code[i]);
		print_instruction(pc, i);
		printf("\n");
	}
}

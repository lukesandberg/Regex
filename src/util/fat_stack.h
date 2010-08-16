#ifndef __FAT_STACK_H__
#define __FAT_STACK_H__

typedef struct fat_stack_s fat_stack;
fat_stack* fat_stack_create();
void fat_stack_push(fat_stack* stk, void* val);
void* fat_stack_peek(fat_stack* stk);
void fat_stack_pop(fat_stack* stk);
int fat_stack_size(fat_stack* stk);
void fat_stack_destroy(fat_stack* stk);
char fat_stack_is_empty(fat_stack* stk);
#endif

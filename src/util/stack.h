#ifndef __STACK_H__
#define __STACK_H__

typedef struct stack_s stack;
stack* stack_create();
void stack_push(stack* stk, void* val);
void* stack_pop(stack* stk);
int stack_size(stack* stk);
void stack_destroy(stack* stk);
char stack_is_empty(stack* stk);
stack* stack_reverse(stack* stk);
void stack_reverse_in_place(stack* stk);
void stack_foreach(stack* stk, void (*cb)(void*, void*), void* data);
#endif

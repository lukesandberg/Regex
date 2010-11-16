#include <stdlib.h>
#include <string.h>

#include "fat_stack.h"

fat_stack* fat_stack_create(size_t obj_size)
{
	fat_stack* stk = (fat_stack*) malloc(sizeof(fat_stack));
	if(stk == NULL) return NULL;
	stk->obj_size = obj_size;
	stk->top = NULL;
	stk->entries = 0;
	return stk;
}
char fat_stack_is_empty(fat_stack* stk)
{
	return stk->entries == 0;
}

unsigned int fat_stack_size(fat_stack* stk)
{
	return stk->entries;
}

int fat_stack_push(fat_stack* stk, void* val)
{
	fat_stack_entry* entry = (fat_stack_entry*) malloc(sizeof(fat_stack_entry) + stk->obj_size);
	if(entry == NULL)
		return 0;
	entry->next=stk->top;
	memcpy(&(entry->val[0]), val, stk->obj_size);
	stk->top = entry;
	stk->entries++;
	return 1;
}

void fat_stack_pop(fat_stack* stk)
{
	fat_stack_entry* tmp = stk->top;
	stk->top = tmp->next;
	stk->entries--;
	free(tmp);
}
void* fat_stack_peek(fat_stack* stk)
{
	return &(stk->top->val[0]);
}
void fat_stack_destroy(fat_stack* stk)
{
	while(stk->entries > 0) fat_stack_pop(stk);
	free(stk);
}

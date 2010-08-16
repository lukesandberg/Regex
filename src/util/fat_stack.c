#include <stdlib.h>
#include <string.h>

#include "fat_stack.h"
#include "util.h"

typedef struct list_entry_s
{
	struct list_entry_s* next;
	unsigned char val[];
} list_entry;

struct fat_stack_s
{
	size_t obj_size;
	list_entry* top;
	unsigned int entries;
};

fat_stack* fat_stack_create(size_t obj_size)
{
	fat_stack* stk = (fat_stack*) checked_malloc(sizeof(fat_stack));
	stk->obj_size = obj_size;
	stk->top = NULL;
	stk->entries = 0;
	return stk;
}
char fat_stack_is_empty(fat_stack* stk)
{
	return stk->entries == 0;
}
int fat_stack_size(fat_stack* stk)
{
	return stk->entries;
}

void fat_stack_push(fat_stack* stk, void* val)
{
	list_entry* entry = (list_entry*) checked_malloc(sizeof(list_entry) + stk->obj_size);
	entry->next=stk->top;
	memcpy(&(entry->val[0]), val, stk->obj_size);
	stk->top = entry;
	stk->entries++;
}

void fat_stack_pop(fat_stack* stk)
{
	list_entry* tmp = stk->top;
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
	checked_free(stk);
}

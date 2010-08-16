#include <stdlib.h>

#include <util/stack.h>
#include <util/util.h>

typedef struct list_entry_s
{
	struct list_entry_s* next;
	void* val;
} list_entry;

struct stack_s
{
	list_entry* top;
	int entries;
};

stack* stack_create()
{
	stack* stk = (stack*) checked_malloc(sizeof(stack));
	stk->top = NULL;
	stk->entries = 0;
	return stk;
}
char stack_is_empty(stack* stk)
{
	return stk->entries == 0;
}
int stack_size(stack* stk)
{
	return stk->entries;
}
static list_entry* create_entry(list_entry* next, void* val)
{
	list_entry* entry = (list_entry*) checked_malloc(sizeof(list_entry));
	entry->next=next;
	entry->val = val;
	return entry;
}

static void* destroy_entry(list_entry* entry)
{
	void* val = entry->val;
	checked_free(entry);
	return val;
}

void stack_push(stack* stk, void* val)
{
	stk->top = create_entry(stk->top, val);
	stk->entries++;
}

void* stack_pop(stack* stk)
{
	list_entry* tmp = stk->top;
	stk->top = tmp->next;
	stk->entries--;
	return destroy_entry(tmp);
}

void stack_destroy(stack* stk)
{
	while(stk->entries > 0) stack_pop(stk);
	checked_free(stk);
}

stack* stack_reverse(stack* stk)
{
	stack* nstk = stack_create();
	list_entry* e = stk->top;
	while(e != NULL)
	{
		stack_push(nstk, e->val);
		e = e->next;
	}
	return nstk;
}

void stack_reverse_in_place(stack* stk)
{
	list_entry* nt = NULL;
	list_entry* ot = stk->top;
	while(ot != NULL)
	{
		list_entry* not = ot->next;
		ot->next = nt;
		nt = ot;
		ot = not;
	}
	stk->top = nt;	
}

void stack_foreach(stack* stk, void (*cb)(void*, void*), void* data)
{
		list_entry *le = stk->top;
		while(le != NULL)
		{
			(*cb)(le->val, data);
			le = le->next;
		}
}

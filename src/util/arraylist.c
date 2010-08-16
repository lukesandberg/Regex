
#include <string.h>
#include <util/arraylist.h>
#include <util/util.h>
/*
  constants
*/
#define ARRAYLIST_INITIAL_CAPACITY 10
#define ARRAYLIST_CAPACITY_DELTA 10

static const size_t object_size = sizeof(void*);

/*
  structures
*/
struct arraylist_s
{
    int _current_capacity;
    void**_data;
    int _size;
};


void arraylist_free(arraylist* list)
{
    free(list->_data);
    free(list);
}

arraylist* arraylist_create()
{
	arraylist* list;
	list = (arraylist*) checked_malloc(sizeof(arraylist));
	list->_current_capacity = ARRAYLIST_INITIAL_CAPACITY;
	list->_data = (void**) checked_malloc(object_size * list->_current_capacity);
	list->_size = 0;

    return list;
}

int arraylist_add(arraylist* list, void* object)
{
    int old_size = arraylist_size(list);
    int new_capacity;
    void** new_data;

    (list->_size)++;
    if (old_size == list->_current_capacity)
    {
        new_capacity = list->_current_capacity + ARRAYLIST_CAPACITY_DELTA;

        new_data = (void**) checked_malloc(object_size * new_capacity);

        memcpy(new_data, list->_data, object_size * old_size);
        free(list->_data);
        (list->_data) = new_data;
        list->_current_capacity = new_capacity;
    }
    (list->_data)[old_size] = object;
    return 1;
}

int arraylist_remove(arraylist* list, const void* object)
{
    int length = arraylist_size(list);
    int last_index = length - 1;
    int new_size, new_capacity;
    int index;

    for (index = 0; index < length; index++)
    {
        if (arraylist_get(list, index) == object)//equality is simply identical pointers
        {
            (list->_size)--;
            if (index < last_index)
            {
                memmove(list->_data + index, list->_data + index + 1, object_size * (last_index - index));
                new_size = list->_size;
                new_capacity = list->_current_capacity - ARRAYLIST_CAPACITY_DELTA;
                if (new_capacity > new_size)
                {
                    list->_data = (void**) realloc(list->_data, object_size * new_capacity);
                    list->_current_capacity = new_capacity;
                }
            }
            return 1;
        }
    }
    return 0;
}

int arraylist_contains(arraylist* list, const void* object)
{
    return (arraylist_index_of(list, object) > -1);
}

int arraylist_index_of(arraylist* list, const void* object)
{
    int length = arraylist_size(list);
    int index;

    for (index = 0; index < length; index++)
    {
        if (arraylist_get(list, index) ==  object)
        {
            return index;
        }
    }
    return -1;
}

int arraylist_is_empty(arraylist* list)
{
    return (0 == arraylist_size(list));
}

int arraylist_size(arraylist* list)
{
    return list->_size;
}

void* arraylist_get(arraylist* list, const int index)
{
    return list->_data[index];
}
void arraylist_trim(arraylist* list)
{
	unsigned int new_cap = list->_size < ARRAYLIST_INITIAL_CAPACITY ? ARRAYLIST_INITIAL_CAPACITY : list->_size;
	list->_data = (void**) realloc(list->_data, object_size * new_cap);
	list->_current_capacity = new_cap;
}
void arraylist_clear(arraylist* list)
{
	list->_size = 0;
}
void arraylist_clear_and_trim(arraylist* list)
{
    list->_data = (void**) realloc(list->_data, object_size * ARRAYLIST_INITIAL_CAPACITY);
    list->_current_capacity = ARRAYLIST_INITIAL_CAPACITY;
    list->_size = 0;
}

void arraylist_sort(arraylist* list, int (*compare)(const void*, const void*))
{
    qsort(list->_data,
          arraylist_size(list),
          sizeof(void*),
          compare);
}

void arraylist_iter(arraylist* list, void (*each)(void*, void*), void* d)
{
	int length = list->_size;;
	for(int i = 0; i < length; i++)
	{
		each(list->_data[i], d);
	}
}

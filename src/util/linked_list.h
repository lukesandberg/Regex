#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H


typedef struct _ll_list linked_list;
typedef struct _ll_node linked_list_node;

linked_list* make_linked_list();
void linked_list_destroy(linked_list *ll);
void* linked_list_remove_first(linked_list* ll);
void* linked_list_remove_last(linked_list* ll);
int linked_list_add_first(linked_list* ll, void* d);
int linked_list_add_last(linked_list* ll, void* d);
int linked_list_is_empty(linked_list* ll);
unsigned int linked_list_size(linked_list* ll);

linked_list_node* linked_list_next(linked_list_node* n);
linked_list_node* linked_list_prev(linked_list_node* n);
void* linked_list_value(linked_list_node* n);
linked_list_node* linked_list_first(linked_list* ll);
linked_list_node* linked_list_last(linked_list* ll);

#endif

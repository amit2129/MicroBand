#include <stdlib.h>

typedef struct ll_node {
		struct ll_node *next;
		void *data;

} ll_node;

typedef struct linked_list
{
	    size_t count;
	    ll_node *head;
} linked_list;


void ll_init(linked_list *ll);


void ll_insert(linked_list *ll, ll_node *new_node);


void ll_insert_data(linked_list *ll, void *data);


void *get_object_with_data(linked_list *, int (*comp_func)(void *, void *), void *comparison_data);


int ll_remove(linked_list *ll, ll_node *node);


int ll_remove_data(linked_list *ll, void *data);


void ll_print(linked_list *ll, void (*print_func)(void *));


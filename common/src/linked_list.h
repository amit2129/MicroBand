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


void *get_object_with_data(linked_list, int (*comp_func)(void *, void *));


int remove_node(linked_list *ll, ll_node *node);







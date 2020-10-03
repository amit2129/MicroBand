#include "linked_list.h"
#include <stdio.h>

void ll_init(linked_list *ll) {
		ll->count = 0;
		ll->head = NULL;
}


void ll_insert(linked_list *ll, ll_node *new_node) {
		new_node->next = ll->head;
		ll->head = new_node;
		ll->count++;
}


void *get_object_with_data(linked_list *ll, int (*comp_func)(void *, void *), void *comparison_data){
		ll_node *cursor = ll->head;
		while (cursor && !comp_func(cursor->data, comparison_data)) {
				cursor = cursor->next;
		}
		if (cursor)
			return cursor->data;
		return NULL;
}


int remove_node(linked_list *ll, ll_node *node) {
		if (!ll->head){
				return - 1;
		}

		ll_node *prev = NULL;
		ll_node *cursor = ll->head;
		while (cursor && cursor != node) {
				prev = cursor;
				cursor = cursor->next;
		}

		if (cursor) {
			if (prev) {
				prev->next = cursor->next;
			}
			else {
				ll->head = node->next;
			}
			ll->count--;
			return 0;
		}
		else {
				return -2;
		}
}






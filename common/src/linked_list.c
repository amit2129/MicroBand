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


void ll_insert_data(linked_list *ll, void *data) {
		ll_node *new_node = (ll_node *) malloc(sizeof(ll_node));
		new_node->data = data;
		ll_insert(ll, new_node);
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


int ll_remove(linked_list *ll, ll_node *node) {
		return ll_remove_data(ll, node->data);
}


int ll_remove_data(linked_list *ll, void *data){
	if (!ll->head){
			return - 1;
	}

	ll_node *prev = NULL;
	ll_node *cursor = ll->head;
	while (cursor && cursor->data != data) {
			prev = cursor;
			cursor = cursor->next;
	}

	if (cursor) {
			if (prev)
				prev->next = cursor->next;
			else
				ll->head = cursor->next;
			ll->count--;
			return 0;
	}
	else
			return -2;
}



void ll_print(linked_list *ll, void (*print_func)(void *)){
	ll_node *cursor = ll->head;
	while (cursor) {
			print_func(cursor->data);
			cursor = cursor->next;
	}
}

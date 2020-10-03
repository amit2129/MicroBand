#include <stdio.h>
#include <stdlib.h>
#include "utils.h"


int main() {
		linked_list ll;
		ll_init(&ll);
		int array[5];
		for (int i = 0; i < 5; i++) {
			ll_node *newNode = (ll_node *) malloc(sizeof(ll_node));
			array[i] = i;
			newNode->data = (void*)(array + i);
			ll_insert(&ll, newNode);
		}

		ll_node *nodeArray[5];
		ll_node *cursor = ll.head;
		for (int i = 0; i < 5; i++) {
			printf("data at %d:%d\n", i, *(int *)cursor->data);
			nodeArray[i] = cursor;
			cursor = cursor->next;
		}

		cursor = ll.head;
		while (cursor) {
			printf("data:%d\n", *(int *)cursor->data);
			cursor = cursor->next;

		}
		for (int i = 0; i< 5; i++) {
				printf("ret: %d ", remove_node(&ll, nodeArray[i]));
				printf("head is: %p\n", (void *)ll.head);
		}

		cursor = ll.head;
		while (cursor) {
				printf("data:%d\n", *(int *)cursor->data);
				cursor = cursor->next;
		}
		return 0;
}

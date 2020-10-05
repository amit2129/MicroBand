#include <stdlib.h>
#include "MR.h"

void init_mr(MR *mr, uint16_t sz){
  mr->sz = sz;
  mr->buffer = (uint8_t * )malloc(sz);
  for (uint8_t i = 0; i < sz; i++) {
    mr->buffer[i] = 0;
  }
}

void free_mr(MR *mr) {
  free(mr->buffer);
}

void clear_mr(MR *mr) {
  int sz = mr->sz;
  for (uint8_t i = 0; i < sz; i++) {
    *(mr->buffer+i) = 0;
  }
}

int write_to_mr(MR *mr, uint32_t offset, uint8_t *src_buffer, uint32_t length) {
	if (offset + length > mr->sz)
		return 1;

	pthread_mutex_lock(&mr->lock);
	memcpy(mr->buffer + offset, src_buffer, length);
	pthread_mutex_unlock(&mr->lock);
}

int read_from_mr(MR *mr, uint32_t offset, uint8_t *dest_buffer, uint32_t length) {
	if (offset + length > mr->sz)
		return 1;

	pthread_mutex_lock(&mr->lock);
	memcpy(dest_buffer, mr->buffer + offset, length);
	pthread_mutex_unlock(&mr->lock);
}


#ifndef MR_H
#define MR_H

#include "../../common/src/queue.h"
#include <string.h>
#include <stdint.h>
#include <pthread.h>

typedef struct mr {
  uint8_t* buffer;
  uint16_t sz;
  pthread_mutex_t lock;
} MR;

void init_mr(MR *mr, uint16_t sz);

void free_mr(MR *mr);

int write_to_mr(MR *mr, uint32_t offset, uint8_t *src_buffer, uint32_t length);

int read_from_mr(MR *mr, uint32_t offset, uint8_t *dest_buffer, uint32_t length);


#endif

#ifndef MR_H
#define MR_H

#include "Queue.h"

#include <stdint.h>

typedef struct mr {
  uint8_t* buffer;
  uint16_t sz;
} MR;

void init_mr(MR *mr, uint16_t sz);

void free_mr(MR *mr);


#endif

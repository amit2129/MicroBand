#ifndef MR_H
#define MR_H

#include "Queue.h"

#include <stdint.h>

typedef struct mr {
  uint8_t* buffer;
  uint16_t sz;
  uint8_t rkey;
} MR;

void init_mr(MR *mr, uint16_t sz);


#endif

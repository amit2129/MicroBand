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

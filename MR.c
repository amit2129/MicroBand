#include <stdlib.h>
#include "MR.h"


void init_mr(MR *mr, uint16_t sz){
  mr->sz = sz;
  mr->buffer = malloc(sz * sizeof(uint8_t));
  for (uint8_t i = 0; i < sz; i++) {
    *(mr->buffer+i) = 0;
  }
}

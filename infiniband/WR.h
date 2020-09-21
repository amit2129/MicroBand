#ifndef WR_H
#define WR_H
#include "SGE.h"
#include <stdint.h>


typedef struct wqe {
  SGE sge;
  uint8_t wr_id;
} WQE;

#endif

#ifndef WR_H
#define WR_H
#include "SGE.h"
#include <stdint.h>
 

typedef struct wqe {
  SGE sge;
  uint16_t wr_id;
} WQE;


uint16_t get_wr_id();

#endif

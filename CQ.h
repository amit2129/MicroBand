#ifndef CQ_H
#define CQ_H
#include "MR.h"
#include "Queue.h"




typedef struct cqe {
  int offset;
  int sz;
  MR *mr;
} CQE;



typedef struct cq {
  int cq_num;
  circular_buffer* queue;
} CQ;

#endif

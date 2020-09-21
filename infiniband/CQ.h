#ifndef CQ_H
#define CQ_H
#include "MR.h"
#include "Queue.h"


typedef struct cqe {
  uint8_t wr_id;
  uint8_t byte_len;
  uint8_t qp_num;
  uint8_t remote_qp_num;
} CQE;


typedef struct cq {
  uint8_t cq_num;
  circular_buffer queue;
} CQ;


void init_cq(CQ *cq, uint8_t sz);

void free_cq(CQ *cq);

int cq_push_back(CQ *cq, CQE *cqe);

int cq_pop_front(CQ *cq, CQE *cqe);

#endif

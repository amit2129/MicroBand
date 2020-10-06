#ifndef CQ_H
#define CQ_H
#include "MR.h"
#include "../../common/src/queue.h"


typedef struct cqe {
  uint16_t wr_id;
  uint16_t byte_len;
  uint8_t status;
  uint8_t qp_num;
  uint8_t remote_qp_num;
} CQE;


typedef struct cq {
  uint8_t cq_num;
  circular_buffer queue;
} CQ;


void init_cq(CQ *cq, uint16_t sz);

void free_cq(CQ *cq);

int cq_push_back(CQ *cq, CQE *cqe);

int cq_pop_front(CQ *cq, CQE *cqe);

#endif

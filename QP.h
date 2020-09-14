#ifndef QP_H
#define QP_H
#include "CQ.h"




typedef struct qp {
  int qp_num;
  circular_buffer* send_queue;
  circular_buffer* recv_queue;
  CQ *completion_queue;
  MR *mem_reg;
} QP;


void init_qp(QP *qp, MR *mr, CQ *cq, uint16_t queue_size);

#endif

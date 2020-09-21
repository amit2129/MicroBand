#ifndef QP_H
#define QP_H
#include "CQ.h"
#include "WR.h"


typedef struct qp {
  uint8_t qp_num;
  uint8_t remote_qp_num;
  circular_buffer *send_queue;
  circular_buffer *recv_queue;
  CQ *completion_queue;
  MR *mem_reg;
} QP;


void init_qp(QP *qp, MR *mr, CQ *cq, uint8_t queue_size);

void free_qp(QP *qp);

int post_send(QP *qp, WQE *wr_s);

int post_recv(QP *qp, WQE *wr_r);

void process_send(QP *qp);

void process_recv(QP *qp, uint8_t *data, uint8_t data_len);

#endif

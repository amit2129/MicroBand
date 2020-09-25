#include "QP.h"
#include "WR.h"
#include "CQ.h"
#include "MR.h"
#include "utils.h"
#include <string.h>


static int qp_counter = 0;
 
void init_qp(QP *qp, MR *mr, CQ *cq, uint8_t queue_size) {
  qp->qp_num = ++qp_counter;
  qp->send_queue = (circular_buffer *) malloc(sizeof(circular_buffer));
  qp->recv_queue = (circular_buffer *) malloc(sizeof(circular_buffer));
  cb_init(qp->send_queue, queue_size, sizeof(WQE));
  cb_init(qp->recv_queue, queue_size, sizeof(WQE));
  qp->completion_queue = cq;
  qp->mem_reg = mr;
}

void free_qp(QP *qp) {
  cb_free(qp->send_queue);
  cb_free(qp->recv_queue);
  free(qp->send_queue);
  free(qp->recv_queue);
  
  free_cq(qp->completion_queue);
  free_mr(qp->mem_reg);
}


int post_send(QP *qp, WQE *wr_s){
  int ret = cb_push_back(qp->send_queue, wr_s);
  return ret;
}

int post_recv(QP *qp, WQE *wr_r){
  int ret = cb_push_back(qp->recv_queue, wr_r);
  return ret;
}


void process_send(QP *qp, void *send_util, uint8_t (*send)(QP *, WQE *, void *)) {
  // do_some_sending
  WQE wr_s;
  uint8_t ret_pop_front = cb_pop_front(qp->send_queue, &wr_s);
  if (ret_pop_front){
    #if defined(DEBUG) 
      print_str("ret_pop_front non-0");
    #endif
    return;
  }

  uint8_t ret = (*send)(qp, &wr_s, send_util);

  CQE cqe;

  if (ret) {
    cqe.wr_id = wr_s.wr_id;
    cqe.status = 1;
    cq_push_back(qp->completion_queue, &cqe);
  }
  else {
    cqe.byte_len = wr_s.sge.length;
    cqe.wr_id = wr_s.wr_id;
    cqe.qp_num = qp->qp_num;
    cqe.remote_qp_num = qp->remote_qp_num;
    cqe.status = 0;
    cq_push_back(qp->completion_queue, &cqe);
  }

  
  // wr_s is now initialized and the physical execution of the send_pipeline should be called.
  // lets assume the send happened and we will report a completion to the CQ
  // TODO: implement actual sending
 }


void process_recv(QP *qp, uint8_t *data, uint8_t data_len) {
  WQE wr_r;
  uint8_t ret_pop_front = cb_pop_front(qp->recv_queue, &wr_r);
  if (ret_pop_front){
    #if defined(DEBUG)    
      print_str("ret_pop_front non-0");
    #endif
    return;
  }
  memcpy(wr_r.sge.addr, data, data_len);
  // lets assume the receiver pipeline delivers data to this function
  CQE cqe;
  cqe.byte_len = data_len;
  cqe.wr_id = wr_r.wr_id;
  cqe.qp_num = qp->qp_num;
  cqe.remote_qp_num = qp->remote_qp_num;
  cq_push_back(qp->completion_queue, &cqe);
}


void process_recv_handle(QP *qp, void *recv_util, CQE (*recv)(QP *, WQE *, void *)) {
  WQE wr_r;
  uint8_t ret_pop_front = cb_pop_front(qp->recv_queue, &wr_r);
  if (ret_pop_front){
    #if defined(DEBUG)    
      print_str("ret_pop_front non-0");
    #endif
    return;
  }

  CQE cqe = (*recv)(qp, &wr_r, recv_util);
  cq_push_back(qp->completion_queue, &cqe);
 
//  CQE cqe;
//  cqe.byte_len = data_len;
//  cqe.wr_id = wr_r.wr_id;
//  cqe.qp_num = qp->qp_num;
//  cqe.remote_qp_num = qp->remote_qp_num;
}

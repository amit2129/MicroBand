#include "QP.h"
#include "WR.h"
#include "CQ.h"
#include "MR.h"
#include "utils.h"
#include <string.h>
#ifdef QP_LINKED_LIST
	#include <stdlib.h>
	#include "../../common/src/utils.h"
	linked_list qp_ll = {0, NULL};
#endif


uint8_t (*qp_send_func)(QP *, WQE *, void *) = NULL;
CQE 	(*qp_recv_func)(QP *, WQE *, void *) = NULL;

static int qp_counter = 0;
 
void init_qp(QP *qp, MR *mr, CQ *cq, uint8_t queue_size) {
  qp->qp_num = ++qp_counter;
  qp->send_queue = (circular_buffer *) malloc(sizeof(circular_buffer));
  qp->recv_queue = (circular_buffer *) malloc(sizeof(circular_buffer));
  cb_init(qp->send_queue, queue_size, sizeof(WQE));
  cb_init(qp->recv_queue, queue_size, sizeof(WQE));
  qp->completion_queue = cq;
  qp->mem_reg = mr;


  #ifdef QP_LINKED_LIST
  	ll_insert_data(&qp_ll, (void *)qp);
  #endif
}

void free_qp(QP *qp) {
  cb_free(qp->send_queue);
  cb_free(qp->recv_queue);
  free(qp->send_queue);
  free(qp->recv_queue);
  
  free_cq(qp->completion_queue);
  free_mr(qp->mem_reg);

  #ifdef QP_LINKED_LIST
  	ll_remove_data(&qp_ll, (void *)qp);
  #endif

}


int post_send(QP *qp, WQE *wr_s){
  int ret = cb_push_back(qp->send_queue, wr_s);
  return ret;
}

int post_recv(QP *qp, WQE *wr_r){
  int ret = cb_push_back(qp->recv_queue, wr_r);
  return ret;
}


int process_send_handle(QP *qp, void *send_util) {
  // do_some_sending
  WQE wr_s;
  uint8_t ret_pop_front = cb_pop_front(qp->send_queue, &wr_s);
  if (ret_pop_front){
    return 1;
  }

  printf("passed ret_pop\n");
  printf("qp_send_func is: %p\n", qp_send_func);

  uint8_t ret = qp_send_func(qp, &wr_s, send_util);

  printf("ret is %d\n", ret);
  CQE cqe;

  if (ret) {
    cqe.wr_id = wr_s.wr_id;
    cqe.status = ret;
    cq_push_back(qp->completion_queue, &cqe);
	return 2;
  }
  cqe.byte_len = wr_s.sge.length;
  cqe.wr_id = wr_s.wr_id;
  cqe.qp_num = qp->qp_num;
  cqe.remote_qp_num = qp->remote_qp_num;
  cqe.status = 0;
  cq_push_back(qp->completion_queue, &cqe);
  return 0;
 }


void process_recv(QP *qp, uint8_t *data, uint8_t data_len) {
  WQE wr_r;
  uint8_t ret_pop_front = cb_pop_front(qp->recv_queue, &wr_r);
  if (ret_pop_front){
    return;
  }
	
  write_to_mr(qp->mem_reg, wr_r.sge.addr - qp->mem_reg->buffer, data, data_len);
  CQE cqe;
  cqe.byte_len = data_len;
  cqe.wr_id = wr_r.wr_id;
  cqe.qp_num = qp->qp_num;
  cqe.remote_qp_num = qp->remote_qp_num;
  cq_push_back(qp->completion_queue, &cqe);
}


void process_recv_handle(QP *qp, void *recv_util) {
  WQE wr_r;
  uint8_t ret_pop_front = cb_pop_front(qp->recv_queue, &wr_r);
  if (ret_pop_front){
    return;
  }

  CQE cqe = qp_recv_func(qp, &wr_r, recv_util);
  cq_push_back(qp->completion_queue, &cqe);
}

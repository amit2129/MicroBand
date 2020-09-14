#include "QP.h"
#include "WR.h"
#include "CQ.h"
#include "MR.h"


static int qp_counter = 0;
 
void init_qp(QP *qp, MR *mr, CQ *cq, uint16_t queue_size) {

  qp->qp_num = qp_counter++;
  cb_init(qp->send_queue, queue_size, sizeof(WR_S));
  cb_init(qp->recv_queue, queue_size, sizeof(WR_R));
  qp->completion_queue = cq;
  qp->mem_reg = mr;
}

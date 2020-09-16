#include "CQ.h"
#include "Queue.h"

static int cq_counter = 0;

void init_cq(CQ *cq, uint8_t sz) {
  cq->cq_num = cq_counter++;
  cq->queue = malloc(sizeof(circular_buffer));
  cb_init(cq->queue, sz, sizeof(CQE));
}

int cq_push_back(CQ *cq, CQE *cqe){
    return cb_push_back(cq->queue, (void *)cqe);
}

int cq_pop_front(CQ *cq, CQE *cqe){
    return cb_pop_front(cq->queue, (void *)cqe);
}

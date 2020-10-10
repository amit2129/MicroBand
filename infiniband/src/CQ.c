#include "CQ.h"

static int cq_counter = 0;

void init_cq(CQ *cq, uint16_t sz) {
  cq->cq_num = ++cq_counter;
  cb_init(&cq->queue, sz, sizeof(CQE));
}

void free_cq(CQ *cq) {
  cb_free(&cq->queue);
}

int cqe_count(CQ *cq) {
    return cq->queue.count;
}

int cq_push_back(CQ *cq, CQE *cqe){
    return cb_push_back(&cq->queue, cqe);
}

int cq_pop_front(CQ *cq, CQE *cqe){
    return cb_pop_front(&cq->queue, cqe);
}

void flush_cq(CQ *cq) {
    cb_flush(&cq->queue);
}

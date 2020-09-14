#include "CQ.h"
#include "Queue.h"

static int cq_counter = 0;

void init_cq(CQ *cq, uint16_t sz) {
  cq->cq_num = cq_counter++;
  cq->queue = malloc(sizeof(circular_buffer));
  cb_init(cq->queue, sz, sizeof(CQE));
}

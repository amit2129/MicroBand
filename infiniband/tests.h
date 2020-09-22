#if defined(DEBUG)

#ifndef TESTS_H
#define TESTS_H

#include <string.h>
#include "MR.h"
#include "CQ.h"
#include "QP.h"
#include "Queue.h"
#include "utils.h"

uint8_t cq_test();

uint8_t check_mr_data(MR *mr, uint8_t value, uint8_t data_len);

uint8_t test_qp_recv_over_queue_size();

uint8_t test_qp_single_post_send(QP *qp, WQE *wqe);

uint8_t test_qp_send_over_queue_size();

uint8_t create_objects_test();

uint8_t cb_test();

uint8_t mr_test();

uint8_t run_tests();

#endif

#endif

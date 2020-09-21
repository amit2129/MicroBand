#ifndef TESTS_H
#define TESTS_H

#include <string.h>
#include "MR.h"
#include "CQ.h"
#include "QP.h"
#include "Queue.h"
#include "utils.h"

void cq_test();

uint8_t check_mr_data(MR *mr, uint8_t value, uint8_t data_len);

void test_qp_single_post_recv(QP *qp, uint8_t value, uint8_t data_len);

void test_qp_recv_over_queue_size();

uint8_t test_qp_single_post_send(QP *qp, WQE *wqe);

void test_qp_send_over_queue_size();

void create_objects_test();

void cb_test();

void mr_test();

void run_tests();

#endif

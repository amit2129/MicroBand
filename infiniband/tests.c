#if defined(DEBUG)

#include "tests.h"

#define ELEM_NUM 10

uint8_t cq_test() {
  uint8_t status = 0;
  CQ cq;
  init_cq(&cq, ELEM_NUM);
  for (int i = 0; i < ELEM_NUM; i++) {
    CQE cqe;
    cqe.wr_id = i;
    cqe.byte_len = ELEM_NUM - i - 1;
    cqe.qp_num = i;
    cqe.remote_qp_num = ELEM_NUM - i - 1;
    status += print_if_noeq("cq_push_back", cq_push_back(&cq, &cqe), 0);
  }
  for (int i = 0; i < ELEM_NUM; i++) {
    CQE cqe;
    status += print_if_noeq("cq_pop_front", cq_pop_front(&cq, &cqe), 0);
    status += print_if_noeq("cqe.wr_id", cqe.wr_id, i);
    status += print_if_noeq("cqe.byte_len", cqe.byte_len, ELEM_NUM - i - 1);
    status += print_if_noeq("cqe.qp_num", cqe.qp_num, i);
    status += print_if_noeq("cqe.remote_qp_num", cqe.remote_qp_num, ELEM_NUM - i - 1);
  }
  free_cq(&cq);
  print_str(" - cq_test passed\n");

  return status;
}


uint8_t check_mr_data(MR *mr, uint8_t value, uint8_t data_len) {
  uint8_t status =0;
  for (int i = 0; i < data_len; i++) {
    status += print_if_noeq("mr->buffer[i] == value", mr->buffer[i], value);
  }
  for (int i = data_len; i < mr->sz; i++) {
    status += print_if_noeq("mr->buffer[i] == 0", mr->buffer[i], 0);
  }
  return status;
}


uint8_t test_qp_recv_over_queue_size() {
  uint8_t status = 0;
  CQ cq;
  init_cq(&cq, ELEM_NUM);
  MR mr;
  init_mr(&mr, ELEM_NUM * 2);
  QP qp;
  init_qp(&qp, &mr, &cq, ELEM_NUM);
  uint8_t input_data[ELEM_NUM];
  // pushing 1 more than the Queue size, expecting it to succeed
  // because the queue is "popped" every iteration
  for (uint8_t i = 0; i < ELEM_NUM + 1; i++) {
    uint8_t *recv_buffer = qp.mem_reg->buffer;
    uint8_t recv_len = i;

    WQE wqe;
    wqe.sge.addr = recv_buffer;
    wqe.sge.length = recv_len;
    wqe.wr_id = i;
    uint8_t ret = post_recv(&qp, &wqe);
    status += print_if_noeq("ret_post_recv", ret, 0);    
    memset(input_data, i, i * sizeof(uint8_t));
    
    process_recv(&qp, input_data, i);
    
    CQE cqe;
    ret = cq_pop_front(&cq, &cqe);
    status += print_if_noeq("ret_cq_pop", ret, 0);
    status += check_mr_data(&mr, i, i);
  }

  if (status)
    print_str(" - qp_test_over_recv_count failed\n");
  else
    print_str(" - qp_test_over_recv_count passed\n");

  return status;
}


uint8_t test_qp_single_post_send(QP *qp, WQE *wqe) {
  uint8_t status = 0;
  int ret = post_send(qp, wqe);
  status += print_if_noeq("ret test_qp", ret, 0);
  process_send(qp);
  if (status)
    print_str(" - test_qp_single_post_send failed\n");
  return status;
}


uint8_t test_qp_send_over_queue_size() {
  uint8_t status = 0;
  CQ cq;
  init_cq(&cq, ELEM_NUM);
  MR mr;
  init_mr(&mr, 2 * ELEM_NUM);
  QP qp;
  init_qp(&qp, &mr, &cq, ELEM_NUM);

  // pushing 1 more than the Queue size, expecting it to succeed
  // because the queue is "popped" every iteration
  for (uint8_t i = 0; i < ELEM_NUM + 1; i++) {

    uint8_t *send_buffer = qp.mem_reg->buffer;
    uint8_t send_len = i;

    memset(send_buffer, i, send_len * sizeof(uint8_t));

    WQE wqe;
    wqe.sge.addr = send_buffer;
    wqe.sge.length = send_len;
    wqe.wr_id = i;
    qp.remote_qp_num = i;

    status += test_qp_single_post_send(&qp, &wqe);

    CQE cqe;
    uint8_t ret = cq_pop_front(qp.completion_queue, &cqe);
    status += print_if_noeq("ret: cq_pop_front", ret, 0);
    status += print_if_noeq("cqe.wr_id", cqe.wr_id, i);
    status += print_if_noeq("cqe.byte_len", cqe.byte_len, i);
    status += print_if_noeq("cqe.remote_qp_num", cqe.remote_qp_num, i);
  }
  
  if (status)
    print_str(" - qp_test_over_send_count failed\n");
  else
    print_str(" - qp_test_over_send_count passed\n"); 

  return status;
}


uint8_t create_objects_test() {
  for (int i = 0; i < ELEM_NUM; i++) {
    CQ cq;
    init_cq(&cq, ELEM_NUM);

    MR mr;
    init_mr(&mr, ELEM_NUM);
    QP qp;
    init_qp(&qp, &mr, &cq, ELEM_NUM);
    free_qp(&qp);
  }

  print_str(" - create_objects_test passed\n");
  return 0;
}



uint8_t cb_test() {
  uint8_t status = 0;
  circular_buffer cb;
  cb_init(&cb, ELEM_NUM, sizeof(uint8_t));
  uint8_t arr[ELEM_NUM];
  for (uint8_t i = 0; i < ELEM_NUM; i++) {
    uint8_t data = i;
    if (!cb_push_back(&cb, &data)) {
      arr[i] = i;
    }
  }

  for (uint8_t i = 0; i < ELEM_NUM; i++) {
    uint8_t data2;
    if (!cb_pop_front(&cb, &data2)) {
      status += print_if_noeq("arr[i] == data2", arr[i], data2);
    }
  }

  cb_free(&cb);

  if (status)
    print_str(" - cb_test failed\n");
  else
    print_str(" - cb_test passed\n");

  return status;
}


uint8_t mr_test() {
  uint8_t status = 0;
  MR mr;
  init_mr(&mr, ELEM_NUM);

  for (uint8_t i = 0; i < ELEM_NUM; i++) {
    status += print_if_noeq("mr_buffer", mr.buffer[i], 0);
  }

  free_mr(&mr);

  if (status)
    print_str(" - mr_test failed\n");
  else
    print_str(" - mr_test passed\n");

  return status;
}



uint8_t run_tests() {
  uint8_t status = 0;
  status += create_objects_test();
  status += mr_test();
  status += cb_test();
  status += cq_test();
  status += test_qp_send_over_queue_size();
  status += test_qp_recv_over_queue_size();
  return status;
}

#endif

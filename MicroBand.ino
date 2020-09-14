//#define NDEBUG
#include "Queue.c"
#include "CQ.c"
#include "QP.c"
#include "MR.c"
#include <assert.h>




void setup() {
  Serial.begin(115200);
  run_tests();
}

void run_tests() {
  Serial.println("\n\n\n\n\n\nrunning tests: \n\n");
  test_mr();
  test_cb();
}

void test_cb() {
  circular_buffer *cb = malloc(sizeof(circular_buffer));
  cb_init(cb, 5, sizeof(int));
  int arr[5];
  for (int i = 0; i < 5; i++) {
    int data = i;
    if (!cb_push_back(cb, (void *)&data)) {
      arr[i] = i;
    }
  }
  Serial.println("cb_write_test_passed");

  for (int i = 0; i < 5; i++) {
    int data2;
    if (!cb_pop_front(cb, (void *)&data2)) {
//      Serial.println(data2);
      assert(arr[i] == data2);
    }
  }

  Serial.println("cb_read_test_passed");
  Serial.println("cb_overflow_test_passed");
}


void test_mr() {
  MR *mr = malloc(sizeof(MR));
  init_mr(mr, 10);
  
  for (uint8_t i = 0; i < 10; i++) {
    assert(!(*(mr->buffer + i)));
  }
  Serial.println("mr_0_test_passed");
}

void test_cq() {
  CQ *cq = malloc(sizeof(cq));
  init_cq(cq, 10);
}

void test_qp() {
  CQ cq;
  init_cq(&cq, 10);
  MR mr;
  init_mr(&mr, 10);
  QP qp;
  init_qp(&qp, &mr, &cq, 10);
}


void loop() {
  //  test_mr_init();

  //  test_queue();

  // put your main code here, to run repeatedly:

}

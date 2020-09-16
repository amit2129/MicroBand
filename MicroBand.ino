#include <LibPrintf.h>

#include "Queue.c"
#include "CQ.c"
#include "QP.c"
#include "MR.c"
#include "tests.c"
#include <assert.h>

void print_str(char *s) {
  printf("%s", s);
}

void print_int(char *s, uint16_t a) {
  Serial.print(s);
  Serial.println(a);
}

uint8_t print_if_noeq(char *s, uint16_t value, uint16_t expected) {
  if (value != expected){
    printf("%s actual:%d no-eq expected:%d\n", s, value, expected);
    return 1;
  }
  return 0;
}

void setup() {
  Serial.begin(115200);
  printf("\nrunning tests\n");
  run_tests();
  printf("done running tests");
  delay(2000);
  exit(0);
}

void loop() {
}

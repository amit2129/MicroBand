#if defined(ARDUINO_AVR_UNO)
  #include <LibPrintf.h>
#else
  #include <stdio.h>
#endif

#include "Queue.h"
#include "CQ.h"
#include "QP.h"
#include "MR.h"
#include "tests.h"
#include <assert.h>

void print_str(char *s) {
  printf("%s", s);
}

void print_int(char *s, uint16_t a) {
  printf("%s%d", s, a);
}

uint8_t print_if_noeq(char *s, uint16_t value, uint16_t expected) {
  if (value != expected){
    printf("%s actual:%d no-eq expected:%d\n", s, value, expected);
    return 1;
  }
  return 0;
}

#if defined(ARDUINO_AVR_UNO)
  void setup() {
    Serial.begin(115200);
    printf("\nrunning tests\n");
    run_tests();
    printf("done running tests");
    delay(2000);
    exit(0);
  }
  
  void loop() {}

#else
  int main(void) {
    printf("running tests\n");
    run_tests();
    printf("done running tests\n");
  }
#endif


#include "WR.h"


static int wr_counter = 0;


uint16_t get_wr_id(){
  return ++wr_counter;
}

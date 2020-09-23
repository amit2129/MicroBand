#ifndef SGE_H
#define SGE_H
#include <stdint.h>


typedef struct sge {
    uint8_t *addr;
    uint8_t length;
} SGE;


#endif

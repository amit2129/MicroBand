#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "MR.h"

void print_str(char *s);

void print_int(char *s, uint16_t a);

uint8_t print_if_noeq(char *s, uint16_t value, uint16_t expected);

void print_mr(MR *mr);

#endif

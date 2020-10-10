#ifndef _MB_TRANSPORT_SHARED
#define _MB_TRANSPORT_SHARED

#define ETH_P_MB 0x2129
#include <stdint.h>
#include "../../infiniband/src/infiniband.h"


typedef struct microband_transport_header {
	uint32_t source_qp;
	uint32_t dest_qp;
	uint32_t sequence_number;
	uint32_t ack_number;
	uint16_t data_len;
	uint32_t reserved_1;
	uint16_t reserved_2; // using for connection establishment atm
} mb_transport;

int get_by_qp_num(void *vp_qp, void *vp_qp_num);
#endif

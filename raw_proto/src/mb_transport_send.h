#include "mb_transport_shared.h"

struct send_util {
		int socket;
		uint8_t *buffer;
		uint16_t send_len;
		struct sockaddr_ll *sadr_ll;
};

uint16_t write_mb_header(QP *qp, uint8_t data_len, uint8_t *send_buffer)
{
	mb_transport *mbh = (mb_transport *)send_buffer;
	mbh->source_qp 	= htons(qp->qp_num);
	mbh->dest_qp	= htons(qp->remote_qp_num);
	mbh->data_len = htons(data_len);
	return sizeof(mb_transport);
}



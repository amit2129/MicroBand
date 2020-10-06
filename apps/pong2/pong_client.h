#ifndef _PONG_CLIENT_H
#define _PONG_CLIENT_H

#include "../../infiniband/src/infiniband.h"
#include "../../raw_proto/src/mb_transport_send.h"
typedef struct player_location {
	char *player_name;
	float height;
} player_location;

typedef struct pong_location {
	float x;
	float y;
} pong_location;


#define WR_ID_RECV 0
#define WR_ID_SEND 1

void send_self_location(QP *qp, struct send_util *su, player_location *loc);

void recv_pong_and_remote_location(QP *qp, uint32_t remote_data_address_offset, pong_location *loc, player_location *remote);

struct send_thread_arg {
		QP *qp;
		struct send_util *su;
};

void *send_thread_work(void *arg_ptr);

struct recv_thread_arg {
	QP *qp;
	struct send_util *su;
};


void *recv_thread_work(void *arg_ptr);


void game_thread(char *player_name, QP *qp, struct send_util *su); 

#endif

#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<errno.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ioctl.h>

#include<net/if.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/if_ether.h>
#include<netinet/udp.h>

#include<linux/if_packet.h>

#include<arpa/inet.h>

#define ETH_P_MB 0x2129
#include <stdlib.h>
#define QP_LINKED_LIST
#include "../../infiniband/src/infiniband.h"
#include "../../common/src/utils.h"
#include "../../raw_proto/src/mb_transport_send.h"


struct ifreq ifreq_c,ifreq_i,ifreq_ip;


typedef struct player_location {
	char *player_name;
	float height;
} player_location;

typedef struct pong_location {
	float x;
	float y;
} pong_location;



void send_self_location(QP *qp, struct send_util *su, player_location *loc, uint8_t *buffer){
	printf("sending self location\n");
	int send_len = 0;
	MR *mem_reg = qp->mem_reg;
	printf("strlen: %ld\n", strlen(loc->player_name) + 1);
	memcpy(mem_reg, loc->player_name, strlen(loc->player_name) + 1);
	send_len += strlen(loc->player_name) + 1;

	printf("fault?\n");
	uint8_t *float_uint8_ptr = (uint8_t *)&loc->height;
	memcpy(mem_reg, float_uint8_ptr, sizeof(float));
	send_len += sizeof(float);

	printf("mr: %p\n mr_buffer:%p\n", qp->mem_reg->buffer);
	
	WQE send_wqe;
	send_wqe.sge.addr = buffer;
	send_wqe.sge.length = send_len;
	printf("before send, length: %d\n", send_wqe.sge.length);
	post_send(qp, &send_wqe);
	process_send_handle(qp, (void *)su);
	printf("after send\n");
}


void recv_pong_and_remote_location(QP *qp, pong_location *loc, player_location *remote) {

}



int main(int argc, char *argv[]) {
	qp_send_func = &send_data;
	int raw_socket;
	uint16_t send_len = 0;
	uint8_t *send_buffer;
	if (argc < 3) {
			printf("please supply an interface name and dest mac\n");
			return 1;
	}

	raw_socket=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
	if(raw_socket == -1)
		printf("error in socket");

	char *eth_interface = argv[1];
	uint8_t dest_mac[6];
	parse_mac(argv[2], dest_mac);

	send_buffer=(unsigned char*)malloc(1500);
	memset(send_buffer,0,1500);

	send_len += set_mandatory_values(raw_socket, send_buffer, eth_interface, dest_mac);
	printf("send_len is: %d\n", send_len);

	struct sockaddr_ll sadr_ll;
	sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
	sadr_ll.sll_halen   = ETH_ALEN;
	sadr_ll.sll_addr[0]  = dest_mac[0];
	sadr_ll.sll_addr[1]  = dest_mac[1];
	sadr_ll.sll_addr[2]  = dest_mac[2];
	sadr_ll.sll_addr[3]  = dest_mac[3];
	sadr_ll.sll_addr[4]  = dest_mac[4];
	sadr_ll.sll_addr[5]  = dest_mac[5];
	
	
	int send_num = atoi(argv[3]);
	CQ cq;
	init_cq(&cq, 10);
	MR mr;
	init_mr(&mr, 90);
	QP qp;
	init_qp(&qp, &mr, &cq, 10);
	qp.qp_num = 21;
	qp.remote_qp_num = 1;

	mr.buffer = (uint8_t *) malloc(90);
	WQE send_wqe;
	send_wqe.sge.addr = mr.buffer;
	send_wqe.sge.length =  25;

	char *player_name = "hello";
	player_location self_location;
//	player_location remote_location;
	self_location.player_name = player_name;
	self_location.height = 0.5;
	pong_location loc;


	struct send_util su = {raw_socket, send_buffer, send_len, &sadr_ll};
	struct send_util su_backup = su;
	send_wqe.wr_id = 0;
	post_send(&qp, &send_wqe);
	process_send_handle(&qp, (void *)&su);
	post_send(&qp, &send_wqe);
	process_send_handle(&qp, (void *)&su);
//	printf("sent_data from a qp\n");
	while (1) {
			sleep(1);
			printf("in loop\n");

			send_self_location(&qp, &su, &self_location, mr.buffer);
			su = su_backup;

	}
	return 0;
}

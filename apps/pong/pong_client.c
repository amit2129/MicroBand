#include "pong_shared.h"
#include <stdio.h>
//#define QP_LINKED_LIST
#include "../../infiniband/src/infiniband.h"
#include "../../common/src/utils.h"
#include "../../raw_proto/src/mb_transport_send.h"


//struct ifreq ifreq_c,ifreq_i,ifreq_ip;
//
//
//void send_self_location(QP *qp, struct send_util su, player_location *loc){
//	printf("sending self location");
//	int send_len = 0;
//	MR *mem_reg = qp->mem_reg;
//	printf("strlen: %ld\n", strlen(loc->player_name) + 1);
//	memcpy(mem_reg, loc->player_name, strlen(loc->player_name) + 1);
//	send_len += strlen(loc->player_name) + 1;
//
//	uint8_t *float_uint8_ptr = (uint8_t *)&loc->height;
//	memcpy(mem_reg, float_uint8_ptr, sizeof(float));
//	send_len += sizeof(float);
//	
//	WQE send_wqe;
//	send_wqe.sge.addr = qp->mem_reg->buffer;
//	send_wqe.sge.length = send_len;
//
//	post_send(qp, &send_wqe);
//	process_send_handle(qp, (void *)&su);
//}
//
//
//void recv_pong_and_remote_location(QP *qp, pong_location *loc, player_location *remote) {
//
//}

int main(int argc, char *argv[]) {
//	qp_send_func = &send_data;
//	int raw_socket;
//	uint16_t send_len = 0;
//	uint8_t *send_buffer;
//	if (argc < 3) {
//			printf("please supply an interface name and dest mac\n");
//			return 1;
//	}

//	raw_socket=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
//	if(raw_socket == -1)
//		perror("error in socket");
//
//	char *eth_interface = argv[1];
//	uint8_t dest_mac[6];
//	parse_mac(argv[2], dest_mac);

//	send_buffer=(unsigned char*)malloc(1500);
//	memset(send_buffer,0,1500);
//
//	printf("raw_socket: %d\n", raw_socket);
//	printf("send_buffer: %p\n", (void *)send_buffer);
//	printf("eth_interface: %s\n", eth_interface);
//	printf("dest_mac: %s\n", argv[2]);
//	send_len += set_mandatory_values(raw_socket, send_buffer, eth_interface, dest_mac);
//	//printf("send_len is: %d\n", send_len);
//	printf("before sockaddr");
//
//	struct sockaddr_ll sadr_ll;
//	sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
//	sadr_ll.sll_halen   = ETH_ALEN;
//	sadr_ll.sll_addr[0]  = dest_mac[0];
//	sadr_ll.sll_addr[1]  = dest_mac[1];
//	sadr_ll.sll_addr[2]  = dest_mac[2];
//	sadr_ll.sll_addr[3]  = dest_mac[3];
//	sadr_ll.sll_addr[4]  = dest_mac[4];
//	sadr_ll.sll_addr[5]  = dest_mac[5];

//	CQ cq;
//	init_cq(&cq, 5);
//	MR mr;
//	init_mr(&mr, 10);
//	QP qp;
//	init_qp(&qp, &mr, &cq, 5);
//	qp.qp_num = 1;
//	qp.remote_qp_num = 0;
//
//
//	char *player_name = argv[1];
//	player_location self_location;
//	player_location remote_location;
//	self_location.player_name = player_name;
//	self_location.height = 0.5;
//	pong_location loc;
//	loc.x = 0.5;
//	loc.y = 0.5;
int a = 0;
//	struct send_util su = {raw_socket, send_buffer, send_len, &sadr_ll};
	printf("a is: %d", a);
	while(1) {
		sleep(1);
		printf("in loop");
		a = a + 1;

		if (a == 4312124)
				break;
		//send_self_location(&qp, su, &self_location);
		//recv_pong_location(&pong_location);
		//display_gui(pong_location, self_location, remote_location)
	}
	printf("a is: %d", a);
}



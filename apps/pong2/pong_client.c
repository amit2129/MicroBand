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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<arpa/inet.h>

#define ETH_P_MB 0x2129
#include <stdlib.h>
#define QP_LINKED_LIST

#include "pong_client.h"
#include "../../common/src/utils.h"


struct ifreq ifreq_c,ifreq_i,ifreq_ip;


int get_by_qp_num(void *vp_qp, void *vp_qp_num) {
	return (((QP *)vp_qp)->qp_num == *(uint32_t *)vp_qp_num);
}


void dispatch_to_qp(mb_transport *mb_trns, uint8_t *data, FILE *log_txt){
	uint32_t dest_qp_num = ntohs(mb_trns->dest_qp);
	fprintf(log_txt, "attempting to dispatch to QP num: %d\n", dest_qp_num);
	QP *qp = get_object_with_data(&qp_ll, &get_by_qp_num, (void *)&dest_qp_num);
	if (qp) {
		process_recv(qp, data, ntohs(mb_trns->data_len));
		CQE cqe;
		int ret_poll = cq_pop_front(qp->completion_queue, &cqe);
		if (ret_poll)
			fprintf(log_txt, "No cqe, receive failed for QP\n");
		else {
			fprintf(log_txt, "Successfully received at QP with byte_len: %d\n", cqe.byte_len);
		}
		return;

	}
	else {
		printf("QP with dest_qp_num: %d not found\n", dest_qp_num);
		exit(1);
	}
	
	fprintf(log_txt, "no QP with num: %d\n", dest_qp_num);
}



void send_self_location(QP *qp, struct send_util *su, player_location *loc){
	int send_len = 0;

	MR *mem_reg = qp->mem_reg;
	write_to_mr(mem_reg, send_len, loc->player_name, strlen(loc->player_name) + 1);	
	send_len += strlen(loc->player_name) + 1;

	uint8_t *float_uint8_ptr = (uint8_t *)&loc->height;

	write_to_mr(mem_reg, send_len, float_uint8_ptr, sizeof(float));
	send_len += sizeof(float);
	
	WQE send_wqe;
	send_wqe.sge.addr = mem_reg->buffer;
	send_wqe.sge.length = send_len;
	send_wqe.wr_id = WR_ID_SEND;
	printf("send length is: %d\n", send_len);
	post_send(qp, &send_wqe);	
}


void recv_pong_and_remote_location(QP *qp, uint32_t remote_data_address_offset, pong_location *loc, player_location *remote) {
	WQE recv_wqe;
	recv_wqe.sge.addr = qp->mem_reg->buffer + remote_data_address_offset;
	recv_wqe.sge.length = qp->mem_reg->sz - remote_data_address_offset;
	recv_wqe.wr_id = WR_ID_RECV;
	post_recv(qp, &recv_wqe);
}


void *send_thread_work(void *arg_ptr) {
	struct send_thread_arg *sta = (struct send_thread_arg *)arg_ptr;
	QP *qp = sta->qp;
	struct send_util *su = sta->su;
	while(1) {
		sleep(1);
		printf("handling send\n");
		process_send_handle(qp, (void *)su);
	}
	return NULL;

}



void *recv_thread_work(void *arg_ptr) {

	while (1) {
		sleep(1);
		printf("handling recv\n");
	}
	return NULL;
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
		printf("error creating socket");

	char *eth_interface = argv[1];
	uint8_t dest_mac[6];
	parse_mac(argv[2], dest_mac);

	send_buffer=(unsigned char*)malloc(1500);
	memset(send_buffer,0,1500);

	send_len += set_mandatory_values(raw_socket, send_buffer, eth_interface, dest_mac);
	struct sockaddr_ll sadr_ll;
	sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
	sadr_ll.sll_halen   = ETH_ALEN;
	sadr_ll.sll_addr[0]  = dest_mac[0];
	sadr_ll.sll_addr[1]  = dest_mac[1];
	sadr_ll.sll_addr[2]  = dest_mac[2];
	sadr_ll.sll_addr[3]  = dest_mac[3];
	sadr_ll.sll_addr[4]  = dest_mac[4];
	sadr_ll.sll_addr[5]  = dest_mac[5];

	// CAN WE REPLACE THESE LINES WITH memcpy?	
	CQ cq;
	init_cq(&cq, 10);
	MR mr;
	init_mr(&mr, 90);
	QP qp;
	init_qp(&qp, &mr, &cq, 10);
	qp.qp_num = 1;
	qp.remote_qp_num = 0;
	struct send_util su = {raw_socket, send_buffer, send_len, &sadr_ll};
	struct send_thread_arg sta = {&qp, &su};
	struct recv_thread_arg rta = {&qp, &su};

	pthread_t send_thread, recv_thread;
	int iret1 = pthread_create(&send_thread, NULL, send_thread_work, (void*)&sta);
	if (iret1) {
		perror("thread send creation failed\n");
		exit(1);
	}

	int iret2 = pthread_create( &recv_thread, NULL, recv_thread_work, (void*)&rta);
	if (iret2) {
		perror("thread recv creation failed\n");
		exit(1);
	}
	game_thread(argv[2], &qp, &su);
   
     pthread_join(send_thread, NULL);
//
}


void game_thread(char *player_name, QP *qp, struct send_util *su) {

	player_location self_location;
	player_location other_location;
	self_location.player_name = player_name;
	self_location.height = 0.5;
	pong_location pong_loc;



	send_self_location(qp, su, &self_location);

	recv_pong_and_remote_location(qp, 5, &pong_loc, &other_location);

	CQE cqe;
	while (1) {
		sleep(1);
		printf("in loop\n");
		int ret =  cq_pop_front(qp->completion_queue, &cqe);
		printf("cq_pop return is: %d\n", ret);
		//	if (cqe.wr_id == WR_ID_SEND)
		send_self_location(qp, su, &self_location);
		//	else
		//		recv_pong_and_remote_location(qp, 5, &pong_loc, &other_location);
		//update_gui();
	}

}

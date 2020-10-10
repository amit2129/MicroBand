#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<errno.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include <time.h>
#include<net/if.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/if_ether.h>
#include<netinet/udp.h>

#include<linux/if_packet.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ETH_P_MB 0x2129
#include <stdlib.h>
#define QP_LINKED_LIST

#include "pong_client.h"
#include "../../common/src/utils.h"
#include "../../raw_proto/src/mb_transport_recv.h"
#include "../../raw_proto/src/conn_est.h"



struct ifreq ifreq_c,ifreq_i,ifreq_ip;



void dispatch_to_qp(mb_transport *mb_trns, uint8_t *data, FILE *log_txt){

	uint32_t dest_qp_num = ntohs(mb_trns->dest_qp);
//	printf("attempting to dispatch to QP num: %d\n", dest_qp_num);
	printf("from qp_num: %d\n", ntohs(mb_trns->source_qp));
	QP *qp = get_object_with_data(&qp_ll, &get_by_qp_num, (void *)&dest_qp_num);
	if (qp) {
		process_recv(qp, data, ntohs(mb_trns->data_len));
		if (qp->qp_num == 0){
			CQE cqe;
			int ret_poll = cq_pop_front(qp->completion_queue, &cqe);
			//		if (ret_poll)
			//			fprintf(log_txt, "No cqe, receive failed for QP\n");
			//		else {
			//			fprintf(log_txt, "Successfully received at QP with byte_len: %d\n", cqe.byte_len);
			//		}
			//
			WQE recv_wqe;
			recv_wqe.sge.addr = qp->mem_reg->buffer;
			recv_wqe.sge.length = qp->mem_reg->sz;
			recv_wqe.wr_id = WR_ID_RECV;
			post_recv(qp, &recv_wqe);

		}
		return;

	}
	else {
//		printf("QP with dest_qp_num: %d not found (sent from qp_num:%d)\n", dest_qp_num, ntohs(mb_trns->source_qp));
	}

	//	fprintf(log_txt, "no QP with num: %d\n", dest_qp_num);
}




void *send_thread_work(void *arg_ptr) {
	struct send_thread_arg *sta = (struct send_thread_arg *)arg_ptr;
	QP *qp = sta->qp;
	struct send_util *su = sta->su;
	uint64_t packet_counter = 0;
	while(1) {
		//			usleep(2000000);
		//			printf("handling send\n");
		
		if (!process_send_handle(qp, (void *)su)) {
//			printf("packets sent: %ld\n", ++packet_counter);
		}
	}
	return NULL;
}


void *recv_thread_work(void *arg_ptr) {
	FILE *log_txt = ((struct recv_thread_arg *)arg_ptr)->log_txt;	
	uint8_t *local_mac = ((struct recv_thread_arg *)arg_ptr)->local_mac;


	unsigned char* buffer = (unsigned char *)malloc(65536); 
	memset(buffer,0,65536);
	struct sockaddr saddr;
	int sock_r,saddr_len,buflen;

	printf("starting recv_thread .... \n");

	sock_r=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_MB)); 
	if(sock_r<0){
		printf("error in socket\n");
	}

	int packet_count = 0;
	while(1){
		saddr_len=sizeof (saddr);
		buflen=recvfrom(sock_r,buffer,65536,0,&saddr,(socklen_t *)&saddr_len);

		if (buflen<0){
			printf("error in reading recvfrom function\n");
		}

		fflush(log_txt);

		if(!process_packet(buffer, log_txt, local_mac, NULL)){
			packet_count++;
			//printf("packet_count: %d\n", packet_count);
		}
	}
	close(sock_r);	
	return NULL;
}


double time_rtt(QP *qp);
int main(int argc, char *argv[]) {

	FILE *log_txt=fopen("log.txt","w");
	if(!log_txt)
	{
		printf("unable to open log.txt\n");
		return -1;

	}
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

	// CAN WE REPLACE THESE LINES WITH memcpy?	
	printf("passed sadr_ll\n");
	CQ cq;
	init_cq(&cq, 50);
	MR mr;
	init_mr(&mr, 1400);
	if (!mr.buffer) {
		printf("couldn't allocate buffer of size 600\n");
		exit(1);
	}
	printf("allocated mr buffer\n");
	QP qp;
	init_qp(&qp, &mr, &cq, 50);
	qp.qp_num = atoi(argv[3]);
	qp.remote_qp_num = 0;

	struct send_util su = {raw_socket, send_buffer, send_len, &sadr_ll};



	game_data gd = {0.5, 0.5, 0.5, 0.5};
	struct send_thread_arg sta = {&qp, &su};
	struct recv_thread_arg rta = {log_txt, ifreq_c.ifr_hwaddr.sa_data};



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

	printf("created two threads\n");

	clock_t t; 
	t = clock(); 
	establish_conn(&qp, ifreq_c.ifr_hwaddr.sa_data, INITIATOR);
	t = clock() - t; 
	double time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("time_taken to connect: %lf\n",time_taken);

	clock_t prev_time = clock();
	double ret = 0;
	double sum = 0;
	int count = 0;
	sleep(3);
	while(1) {	
		t = clock() - prev_time; 
		double time_taken = ((double)t)/CLOCKS_PER_SEC;
		
		sum += time_rtt(&qp);
		count++;
		
	//	if (time_taken > 2) {	
	//		prev_time = clock();
	//		printf("Average RTT: %f\n", sum / count);
	//	}


//		printf("connected with qp %d to remote qp %d at HWADDR: ", qp.qp_num, qp.remote_qp_num);
//		for (int i = 0; i < 6; i++ ) {
//			if (i != 5)
//				printf("%.02X-", dest_mac[i]);
//			else
//				printf("%.02X", dest_mac[i]);
//		}
//
//		printf("\n");
//		sleep(1);
	}

	//game_thread(argv[2], &qp);

}


double time_rtt(QP *qp) {
//	usleep(50);
	CQE cqe;
	cqe.wr_id = 0;
	WQE minimal_wqe_send;
	minimal_wqe_send.sge.addr = qp->mem_reg->buffer;
	minimal_wqe_send.sge.length = qp->mem_reg->sz;
	minimal_wqe_send.wr_id = 1;

//	WQE minimal_wqe_recv;
//	minimal_wqe_recv.sge.addr = qp->mem_reg->buffer + sizeof(clock_t);
//	minimal_wqe_recv.sge.length = qp->mem_reg->sz;
//	minimal_wqe_recv.wr_id = 6;
	sleep(5);
	//flush_qp(qp);
	clock_t t, t2; 
	t = clock();
	for (int i = 0; i < 50; i++) {
	    cq_pop_front(qp->completion_queue, &cqe);
	    process_recv(qp, NULL, 0);
	}

	write_to_mr(qp->mem_reg, 0, (uint8_t *)&t, sizeof(clock_t));	

	post_recv(qp, &minimal_wqe_send);	

	int post_num = 1;
	for (int i = 0; i < post_num; i++) {
		post_send(qp, &minimal_wqe_send);
	}
//	post_send(qp, &minimal_wqe_send);
	

	int counter = 0;
	while (counter < post_num + 1) {
		if (!cq_pop_front(qp->completion_queue, &cqe)) {
			counter++;
		}
	}
	read_from_mr(qp->mem_reg,0 ,(uint8_t *)&t, sizeof(clock_t));
	t = clock() - t;			
	double time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("RTT: %f\n", time_taken);
	return time_taken;

//	while (1){
//		post_recv(qp, &minimal_wqe_recv);
//		post_send(qp, &minimal_wqe_send);
//
//		//post_send(qp, &minimal_wqe_send);
//		while(!cq_pop_front(qp->completion_queue, &cqe)) {
//			if (cqe.wr_id == 6) {
//				read_from_mr(qp->mem_reg, sizeof(clock_t),(uint8_t *)&t, sizeof(clock_t));
//				t = clock() - t;			
//				double time_taken = ((double)t)/CLOCKS_PER_SEC;
//				printf("RTT: %f\n", time_taken);
//				return time_taken;
//				//double time_taken = ((double)t)/CLOCKS_PER_SEC;
//				//printf("RTT: %f\n",time_taken);
//				//	flush_qp(qp);	
//			}
//		}
//		//printf("SENDING TIME\n");
//		//usleep(500);
//	}
}
//void send_self_location(QP *qp, struct send_util *su, player_location *loc){
//	int send_len = 0;
//
//	MR *mem_reg = qp->mem_reg;
//write_to_mr(mem_reg, send_len, loc->player_name, strlen(loc->player_name) + 1);	
//	send_len += strlen(loc->player_name) + 1;
//
//	uint8_t *float_uint8_ptr = (uint8_t *)&loc->height;
//
//	write_to_mr(mem_reg, send_len, float_uint8_ptr, sizeof(float));
//	send_len += sizeof(float);
//	
//	WQE send_wqe;
//	send_wqe.sge.addr = mem_reg->buffer;
//	send_wqe.sge.length = send_len;
//	send_wqe.wr_id = WR_ID_SEND;
//	printf("send length is: %d\n", send_len);
//	post_send(qp, &send_wqe);	
//}
//
//
//void recv_pong_and_remote_location(QP *qp, uint32_t remote_data_address_offset, pong_location *loc, player_location *remote) {
//	WQE recv_wqe;
//	recv_wqe.sge.addr = qp->mem_reg->buffer + remote_data_address_offset;
//	recv_wqe.sge.length = qp->mem_reg->sz - remote_data_address_offset;
//	recv_wqe.wr_id = WR_ID_RECV;
//	post_recv(qp, &recv_wqe);
//}

void update_gui() {
	// do something to update QT gui
}

void update_user_location_gui(float *user_location){
}



void game_thread(char *player_name, QP *qp) {

	WQE send_wr;
	send_wr.sge.addr = qp->mem_reg->buffer;
	send_wr.sge.length = sizeof(game_data);
	send_wr.wr_id = WR_ID_SEND;

	WQE recv_wr;
	recv_wr.sge.addr = qp->mem_reg->buffer + sizeof(game_data);
	recv_wr.sge.length = sizeof(game_data);
	recv_wr.wr_id = WR_ID_RECV;

	game_data actual_game_data;
	float game_user_location = 0.5;
	qp->state = QPS_INIT;

	post_send(qp, &send_wr);
	post_recv(qp, &recv_wr);
	CQE cqe;
	while (1) {
		// while there are completions, read them and update the local/remote data.
		while (!cq_pop_front(qp->completion_queue, &cqe)) {
			// got send completion, writing new user_location into player_loc
			if (cqe.wr_id == WR_ID_SEND){
				printf("got send_comp\n");
				write_to_mr(qp->mem_reg, offsetof(game_data, player_loc), 
						(uint8_t *)&game_user_location, sizeof(float));
			}
			// received recv completion (remote data is up to date, reading it)
			else if (cqe.wr_id == WR_ID_RECV) {
				printf("got recv_comp\n");
				read_from_mr(qp->mem_reg, recv_wr.sge.addr - qp->mem_reg->buffer,
						(uint8_t *)&actual_game_data, sizeof(game_data));
				actual_game_data.player_loc = game_user_location;
				post_recv(qp, &recv_wr);

			}
			else {
				printf("got error cqe: status:%d\n", cqe.status);
			}

		}
		update_gui();
		update_user_location_gui(&game_user_location);

		// send local data and recv remote data again
		post_send(qp, &send_wr);
	}
}

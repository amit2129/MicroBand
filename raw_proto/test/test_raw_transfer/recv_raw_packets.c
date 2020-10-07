#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include<signal.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<sys/types.h>

#include<linux/if_packet.h>
#include<netinet/in.h>		 
#include<netinet/if_ether.h>    // for ethernet header
#include<netinet/ip.h>		// for ip header
#include<netinet/udp.h>		// for udp header
#include<netinet/tcp.h>
#include<arpa/inet.h>           // to avoid warning at inet_ntoa

#include <pthread.h>
// keep QPs in a linked list named qp_ll
#define QP_LINKED_LIST

#include "../../../infiniband/src/infiniband.h"
#include "../../../common/src/utils.h"
#include "../../src/mb_transport_recv.h"

#define SEND_WR_ID 1
#define RECV_WR_ID 0

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
		if (qp->qp_num == 0) {
			WQE recv_wqe;
			recv_wqe.sge.addr = qp->mem_reg->buffer;
			recv_wqe.sge.length = qp->mem_reg->sz;
			recv_wqe.wr_id = RECV_WR_ID;
			post_recv(qp, &recv_wqe);
		}
		return;

	}
	else {
		printf("QP with dest_qp_num: %d not found\n", dest_qp_num);
		exit(1);
	}
	
	fprintf(log_txt, "no QP with num: %d\n", dest_qp_num);
}

void print_qp(void *data) {
	QP *qp = (QP *)data;
	printf("\n\n_________QP________\n\n");
	printf("QP NUM		:%d\n", qp->qp_num);
	printf("REMOTE QP NUM 	:%d\n", qp->remote_qp_num);
	printf("___________________\n");
}


typedef struct dispatch_thread_args {
	FILE *log_txt;
} dispatch_thread_args;


void *dispatch_function(void *recv_args) {
    FILE *log_txt = ((dispatch_thread_args *)(recv_args))->log_txt;
    unsigned char* buffer = (unsigned char *)malloc(65536); 
    memset(buffer,0,65536);
    struct sockaddr saddr;
    int sock_r,saddr_len,buflen;

    printf("starting dispatch_thread .... \n");
    sock_r=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL)); 
    if(sock_r<0){
        printf("error in socket\n");
    }

    int packet_count = 0;
    while(1){
	saddr_len=sizeof (saddr);
	buflen=recvfrom(sock_r,buffer,65536,0,&saddr,(socklen_t *)&saddr_len);

	if(buflen<0){
	    printf("error in reading recvfrom function\n");
	}
	fflush(log_txt);

	if(!process_packet(buffer, log_txt)){
		packet_count++;
		printf("packet_count: %d\n", packet_count);
	}
    }
    close(sock_r);
}

//typedef struct connection_data {
//	int player_num;
//	player_location client1_loc;
//	player_location client2_loc;
//	pong_location pong_loc;
//	pthread_mutex_t lock;
//} conn_data;
//
//
//void copy_data(conn_data *src, conn_data *dst) {
//    pthread_mutex_lock(&src.lock);
//
//    dst->client1_loc = src->client1_loc;
//    dst->client2_loc = src->client2_loc;
//    dst->pong_loc = src->pong_loc;
//
//    pthread_mutex_unlock(&src.lock);
//}
//
//
//typedef struct client_connection_args {
//	float updated_location;
//	float *access_ptr;
//	conn_data *shared_data;
//	uint32_t client_qp_num;
//	QP *new_qp;
//} client_connection_args;
//
//
//void write_shared_location(player_location *ptr, player_location loc) {
//    pthread_mutex_lock(&src.lock);
//    *ptr = loc
//    pthread_mutex_unlock(&src.lock);
//}
//
//void *get_attribute(player_location *ptr, ) {
//
//}
//


int main()
{
	
	FILE *log_txt=fopen("log.txt","w");
	if(!log_txt)
	{
		printf("unable to open log.txt\n");
		return -1;

	}

	ll_init(&qp_ll);
	
	// init objects:
	const int OBJECT_NUM = 1;
	const int OBJECT_SIZE = 20;


	QP qps[OBJECT_NUM];
	CQ cqs[OBJECT_NUM];
	MR mrs[OBJECT_NUM];
	WQE wqes[OBJECT_NUM];
	
	for (int i = 0; i < OBJECT_NUM; i++) {
		init_cq(cqs + i, OBJECT_SIZE);
		init_mr(mrs + i, OBJECT_SIZE * 4);
		init_qp(qps + i, mrs + i, cqs + i, OBJECT_SIZE);
		qps[i].remote_qp_num = 1;
		qps[i].qp_num = 0;
		printf("qp_num is: %d\n", qps[i].qp_num);

		wqes[i].sge.addr = mrs[i].buffer;
		wqes[i].sge.length = mrs[i].sz;
		wqes[i].wr_id = get_wr_id();

		post_recv(qps + i, wqes + i);
		
		//ll_insert(&qp_ll, qp_nodes + i);
	}
	ll_print(&qp_ll, &print_qp);

	int recv_num = 2;
	printf("finished\n");

	pthread_t dispatch_thread;
	dispatch_thread_args dta = {log_txt};
	int iret1 = pthread_create(&dispatch_thread, NULL, dispatch_function, (void *)&log_txt);
	if (iret1) {
		perror("thread send creation failed\n");
		exit(1);
	}

	pthread_join(dispatch_thread, NULL);


}


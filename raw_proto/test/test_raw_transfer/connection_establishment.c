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
#include <unistd.h>
#include <pthread.h>
// keep QPs in a linked list named qp_ll
#define QP_LINKED_LIST

#include "../../../infiniband/src/infiniband.h"
#include "../../../common/src/utils.h"
#include "../../src/mb_transport_recv.h"
#include "../../src/mb_transport_send.h"
#include "../../src/mb_transport_shared.h"
#include "../../src/conn_est.h"

#define WR_ID_SEND 1
#define WR_ID_RECV 2


int building_connection = 0;
struct ifreq ifreq_c,ifreq_i,ifreq_ip;
char *eth_interface;
char *source_mac;


typedef struct send_thread_work_args {
	uint8_t *dest_mac;
	QP **qp_array;
	uint32_t num_qps;
	int *done_init;
}stwa;

void *send_thread_work(void *arg_ptr) {
	stwa *thread_args = (stwa *) arg_ptr;
	int num_qps = thread_args->num_qps;

	QP *qps[num_qps];
	for (int i = 0; i < num_qps; i++) {
		qps[i] = thread_args->qp_array[i];
	}
	uint8_t dest_mac[6];
	memcpy(dest_mac, thread_args->dest_mac, 6);
	*thread_args->done_init = 1;

	int raw_socket;
	uint16_t send_len = 0;
	uint8_t *send_buffer;

	raw_socket=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
	if(raw_socket == -1)
		printf("error creating socket");


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

	struct send_util su = {raw_socket, send_buffer, send_len, &sadr_ll};

	while(1) {
		usleep(1000);
		//		printf("handling send ");
		for (int i = 0; i < num_qps; i++) {
			//			printf("from qp: %d ",  qps[i]->qp_num);
			process_send_handle(qps[i], (void *)&su);
		}
		//		printf("\n");
	}
	return NULL;
}


typedef struct new_qp_thread_args {
	mb_transport mb_trns;
	uint8_t *data;
} nqta;

void *new_qp_thread(void *vp_args) {

	pthread_t sending_thread;



	nqta *args = (nqta *)vp_args;
	QP qp;
	CQ cq;
	MR mr;
	init_cq(&cq, 50);
	init_mr(&mr, 1400);
	init_qp(&qp, &mr, &cq, 50);
	write_to_mr(qp.mem_reg, 0, args->data, sizeof(cned));

	cned temp_cned;
	read_from_mr(qp.mem_reg, 0, (uint8_t *)&temp_cned, sizeof(cned));

	printf("remote mac is: ");
	print_mac(temp_cned.remote_mac_address);	

	printf("remote qp num ntohs is: %d\n", ntohs(args->mb_trns.source_qp));
	qp.remote_qp_num = ntohs(args->mb_trns.source_qp);

	free(args->data);
	free(args);



	// launch send thread
	int done_init = 0;
	QP *qp_p = &qp;
	stwa thread_args = {temp_cned.remote_mac_address, &qp_p, 1, &done_init};

	int thread_ret = pthread_create(&sending_thread, NULL, send_thread_work, (void *)&thread_args);

	if (thread_ret) {
		perror("thread sending_thread creation failed\n");
		exit(1);
	}


	while(!done_init) {
		//	printf("waiting for sending thread to finish_init\n");
		//	sleep(1); 
	}

	printf("done init\n");


	clock_t t; 
	t = clock(); 
	establish_conn(&qp, NULL, RECIPIENT);
	building_connection = 0;
	t = clock() - t; 
	double time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("time_taken to connect: %lf\n",time_taken);
	uint8_t * dest_mac = temp_cned.remote_mac_address;


	while(1) {

		time_rtt(&qp);
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


	pthread_join(sending_thread, NULL);
}

void time_rtt(QP *qp) {
	CQE cqe;
	cqe.wr_id = 9;
	static uint16_t wr_id = 9;
	WQE minimal_wqe_send;
	minimal_wqe_send.sge.addr = qp->mem_reg->buffer;
	minimal_wqe_send.sge.length = qp->mem_reg->sz;
	minimal_wqe_send.wr_id = wr_id;


	WQE minimal_wqe_recv;
	minimal_wqe_recv.sge.addr = qp->mem_reg->buffer;
	minimal_wqe_recv.sge.length = qp->mem_reg->sz;
	minimal_wqe_recv.wr_id = 1;
	int post_num = 10;

//	printf("recv_count: %d\n", qp->recv_queue->count);
	if (wr_id == 9) {

		for (int i = 0; i < 50; i++) {
			cq_pop_front(qp->completion_queue, &cqe);
			process_recv(qp, NULL, 0);
		}

		for (int i = 0; i < post_num; i++) {
			post_recv(qp, &minimal_wqe_recv);
		}
		
	}
	else {
	for (int i = 0; i < post_num; i++) {
			post_recv(qp, &minimal_wqe_recv);
		}
	}

	//	flush_qp(qp);
	clock_t t = 0; 
	//post_send(qp, &minimal_wqe_send);
	int counter = 0;
	while (1) {
		while (!cq_pop_front(qp->completion_queue, &cqe)){
			if (cqe.wr_id < 2 && ++counter == post_num) {			
				post_send(qp, &minimal_wqe_send);
				wr_id = 10;
				return;
			}
		}
		//		printf("WAITING TIME\n");
		//		usleep(1000000);
	}

}


// recipient



//enum ConnState {RESET, INIT, RTR, RTS};
//
//typedef struct connection_establishment_data {
//    uint32_t qp_num_establisher;
//    uint32_t qp_num_recipient;
//    enum ConnState initiator_state;
//    enum ConnState recipient_state;
//    uint8_t remote_mac_address[6];
//} cned;









void create_new_thread(mb_transport *mb_trns, uint8_t *data) {
	pthread_t new_thread_handle;


	// new_args will be free inside the new thread 
	// once they are copied to stack memory
	nqta *new_args = (nqta *) malloc(sizeof(nqta));
	memcpy(&new_args->mb_trns, mb_trns, sizeof(mb_transport));
	new_args->data = (uint8_t *) malloc(sizeof(uint8_t) * ntohs(mb_trns->data_len));
	memcpy(new_args->data, data, ntohs(mb_trns->data_len));

	int iret1 = pthread_create(&new_thread_handle, NULL, new_qp_thread, (void *)new_args);
	if (iret1) {
		perror("thread new_qp_thread creation failed\n");
		exit(1);
	}
	printf("succesfully created new thread for qp\n");


}


void dispatch_to_qp(mb_transport *mb_trns, uint8_t *data, FILE *log_txt){

	uint16_t received_state = ntohs(mb_trns->reserved_2);
	//	printf("got mb_packet! with state: %d\n", ntohs(mb_trns->reserved_2));
	uint32_t dest_qp_num = ntohs(mb_trns->dest_qp);

//		printf("attempting to dispatch to QP num: %d\n", dest_qp_num);
	//	fprintf(log_txt, "attempting to dispatch to QP num: %d\n", dest_qp_num);
	QP *qp = get_object_with_data(&qp_ll, &get_by_qp_num, (void *)&dest_qp_num);
	if (qp) {
//		printf("received_state:%d, building_connection:%d\n", received_state, building_connection);
		if (qp->qp_num == 0 && received_state > 0 && received_state < 3 && !building_connection) {
			building_connection = 1;
			create_new_thread(mb_trns, data);
			printf("created new thread~!\n");
			return;
		}
	
		if (qp->qp_num != 0 && received_state > 1 && qp->remote_qp_num != ntohs(mb_trns->source_qp)) {
			printf("discarding for qp:%d for state:%d\n",qp->qp_num, received_state);
			exit(1);
			return;
		}

//		printf("processed recv\n");
		process_recv(qp, data, ntohs(mb_trns->data_len));
		if (qp->qp_num == 0) {
			CQE cqe;
			int ret_poll = cq_pop_front(qp->completion_queue, &cqe);
			//			if (ret_poll)
			//				fprintf(log_txt, "No cqe, receive failed for QP\n");
			//			else {
			//				fprintf(log_txt, "Successfully received at QP with byte_len: %d\n", cqe.byte_len);
			//			}
			// post another recv work request for the whole size
			WQE recv_wqe;
			recv_wqe.sge.addr = qp->mem_reg->buffer;
			recv_wqe.sge.length = qp->mem_reg->sz;
			recv_wqe.wr_id = WR_ID_RECV;
			post_recv(qp, &recv_wqe);
		}
		return;

	}
	else {
//		printf("QP with dest_qp_num: %d not found\n", dest_qp_num);
//		exit(1);
	}

	//fprintf(log_txt, "no QP with num: %d\n", dest_qp_num);
}


typedef struct dispatch_thread_args {
	FILE *log_txt;
	uint8_t *smac;
	uint8_t *dmac;
} dispatch_thread_args;


void *dispatch_function(void *dispatch_args) {
	FILE *log_txt = ((dispatch_thread_args *)(dispatch_args))->log_txt;
	uint8_t *smac = ((dispatch_thread_args *)(dispatch_args))->smac;
	uint8_t *dmac = ((dispatch_thread_args *)(dispatch_args))->dmac;

	unsigned char* buffer = (unsigned char *)malloc(65536); 
	memset(buffer,0,65536);
	struct sockaddr saddr;
	int sock_r,saddr_len,buflen;

	printf("starting dispatch_thread .... \n");
	print_mac(smac);	
	printf("\n");
	sock_r=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_MB)); 
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

		if(!process_packet(buffer, log_txt, smac, dmac)){
			packet_count++;
	//		printf("packet_count: %d\n", packet_count);
		}
	}
	close(sock_r);
}
void print_qp(void *data) {
	QP *qp = (QP *)data;
	printf("\n\n_________QP________\n\n");
	printf("QP NUM		:%d\n", qp->qp_num);
	printf("REMOTE QP NUM 	:%d\n", qp->remote_qp_num);
	printf("___________________\n");
}



int main(int argc, char *argv[])
{

	qp_send_func = &send_data;
	char *ifname = argv[1];
	eth_interface = argv[1];
	int raw_socket;
	uint16_t send_len = 0;
	uint8_t *send_buffer;
	if (argc < 2) {
		printf("please supply an interface name and dest mac\n");
		return 1;
	}

	raw_socket=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
	if(raw_socket == -1)
		printf("error creating socket");


	memset(&ifreq_c,0,sizeof(ifreq_c));
	strncpy(ifreq_c.ifr_name,ifname,IFNAMSIZ-1);

	if((ioctl(raw_socket,SIOCGIFHWADDR,&ifreq_c))<0)
		printf("error in SIOCGIFHWADDR ioctl reading");

	printf("Source Mac= %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]));



	FILE *log_txt=fopen("log.txt","w");
//	if(!log_txt)
//	{
//		printf("unable to open log.txt\n");
//		return -1;

//	}

	building_connection = 0;
	// initializing automatic linked list qp management.
	ll_init(&qp_ll);

	QP qp_zero;
	CQ cq;
	MR mr;

	init_cq(&cq, 20);
	init_mr(&mr, 100);
	init_qp(&qp_zero, &mr, &cq, 20);
	qp_zero.qp_num = 0;
	qp_zero.remote_qp_num = -1;
	ll_print(&qp_ll, &print_qp);

	printf("finished\n");
	uint8_t smac[6];
	memcpy(smac, ifreq_c.ifr_hwaddr.sa_data, 6);

	source_mac = ifreq_c.ifr_hwaddr.sa_data;

	pthread_t dispatch_thread;
	dispatch_thread_args dta = {log_txt, smac, NULL};
	int iret1 = pthread_create(&dispatch_thread, NULL, dispatch_function, (void *)&dta);
	if (iret1) {
		perror("thread send creation failed\n");
		exit(1);
	}

	pthread_join(dispatch_thread, NULL);

}



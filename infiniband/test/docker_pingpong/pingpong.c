// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include "../../src/infiniband.h"
#define PORT 1024


uint8_t send_data(QP *qp, WQE *wr_s, void *send_util) {

	int sock = *(int *)send_util;
	send(sock , wr_s->sge.addr , wr_s->sge.length , 0);
	return 0;
}

CQE recv_data(QP *qp, WQE *wr_r, void *recv_util) {
	int sock = *(int *)recv_util;
	int valread = read(sock, wr_r->sge.addr, wr_r->sge.length);
	CQE cqe;
	cqe.status = (valread == 0);
	cqe.byte_len = valread;
	cqe.wr_id = wr_r->wr_id;
	cqe.qp_num = qp->qp_num;
	cqe.remote_qp_num = qp->remote_qp_num;
	return cqe;
	
}

void print_mr(MR *mr) {
	int i;
	for (i = 0; i < mr->sz; i++)
	{
	    if (i > 0)
		    printf(":");
	    else
		    printf("\n");
	    printf("%02X", mr->buffer[i]);
	}
	printf("\n");
}

uint32_t buffer_to_int(uint8_t *buffer) {
	uint32_t ret = 0;
	ret |= (buffer[0] << 24);
	ret |= (buffer[0] << 16);
	ret |= (buffer[0] << 8);
	ret |= (buffer[3]);
	return ret;
}

void int_to_buffer(uint32_t n, uint8_t *buffer ) {
	buffer[0] = (n >> 24) & 0xFF;
	buffer[1] = (n >> 16) & 0xFF;
	buffer[2] = (n >> 8) & 0xFF;
	buffer[3] = n & 0xFF;
}


int run_server() 
{ 
	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	uint8_t buffer[1024] = {0}; 
	char *hello = "Hello from server";

	int iters = 10;
	uint32_t num = 0;
	CQ cq;
	init_cq(&cq, 10);
	MR mr;
	init_mr(&mr, 16);
	QP qp;
	init_qp(&qp, &mr, &cq, 10);
	memset(mr.buffer, 0, 10);

	WQE recv_wqe;
	recv_wqe.sge.addr = mr.buffer;
	recv_wqe.sge.length = sizeof(uint32_t);

	for (int i = 0; i < iters; i++) {
		recv_wqe.wr_id = get_wr_id();
		post_recv(&qp, &recv_wqe);
	}

	WQE send_wqe;
	send_wqe.sge.addr = mr.buffer;
	send_wqe.sge.length = sizeof(uint32_t);

	for (int i = 0; i < iters; i++) {
		send_wqe.wr_id = get_wr_id();
		post_send(&qp, &send_wqe);
	}	

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	printf("waiting\n");

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	}

	printf("\nconnection success\n");
	uint32_t ret_num = 0;
	uint32_t rc = 0;
	for (int i =0; i < iters; i++) {
		printf("iter is: %d, ret_num:%d\n", i, ret_num);
		rc += !(ret_num == i);
		process_send(&qp,(void *)&new_socket, &send_data);
		process_recv_handle(&qp, (void *)&new_socket, &recv_data);
		ret_num = buffer_to_int(mr.buffer);
		int_to_buffer(ret_num, mr.buffer);

	}
	return rc;
}




int run_client(char const *host_address) 
{ 
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	
	int iters = 10;
	uint32_t num = 0;
	CQ cq;
	init_cq(&cq, 10);
	MR mr;
	init_mr(&mr, 10);
	QP qp;
	init_qp(&qp, &mr, &cq, 10);


	WQE recv_wqe;
	recv_wqe.sge.addr = mr.buffer;
	recv_wqe.sge.length = sizeof(uint32_t);

	for (int i = 0; i < iters; i++) {
		recv_wqe.wr_id = get_wr_id();
		post_recv(&qp, &recv_wqe);
	}

	WQE send_wqe;
	send_wqe.sge.addr = mr.buffer;
	send_wqe.sge.length = sizeof(uint32_t);

	for (int i = 0; i < iters; i++) {
		send_wqe.wr_id = get_wr_id();
		post_send(&qp, &send_wqe);
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, host_address, &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}
	
	printf("\nconnection success\n");
	sleep(1);
	uint32_t ret_num = 0;
	int_to_buffer(ret_num, mr.buffer);
	uint32_t rc = 0;
	for (int i =0; i < iters; i++) {
		process_recv_handle(&qp, (void *)&sock, &recv_data);
		ret_num = buffer_to_int(mr.buffer);
		printf("iter is: %d, ret_num:%d\n", i, ret_num);
		rc += !(ret_num == i);
		int_to_buffer(ret_num+1, mr.buffer);
		process_send(&qp,(void *)&sock, &send_data);

	}


	return rc; 
}

int main(int argc, char const *argv[]) {
	if (argc >=2 && strcmp(argv[1], "-s") == 0) {
		printf("running client\n");
		return run_client(argv[2]);
	}
	else {
		printf("running server\n");
		return run_server();
	}
}



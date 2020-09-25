// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include "../../src/infiniband.h"
#define PORT 1024


uint8_t send_data(QP *qp, WQE *wr_s, void *send_util) {

	int sock = *(int *)send_util;
	printf("sending data: %s\n", wr_s->sge.addr);
	send(sock , wr_s->sge.addr , wr_s->sge.length , 0);
	//printf("sock in send is: %d", sock);
	//send(sock , "hello" , strlen("hello") , 0);
	return 0;
}


void print_mr(MR *mr) {
	int i;
	for (i = 0; i < mr->sz; i++)
	{
	    if (i > 0) printf(":");
	    printf("%02X", mr->buffer[i]);
	}
	printf("\n");
}

int main(int argc, char const *argv[]) 
{ 
	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	uint8_t buffer[1024] = {0}; 
	char *hello = "Hello from server";
	CQ cq;
	init_cq(&cq, 20);
	MR mr;
	init_mr(&mr, 40);
	QP qp;
	init_qp(&qp, &mr, &cq, 20);

	memset(mr.buffer, 0, 40);

	WQE wqe1;
	wqe1.sge.addr = mr.buffer;
	wqe1.sge.length = mr.sz/2;
	wqe1.wr_id = get_wr_id();
	printf("recv_wr_id: %d\n", wqe1.wr_id);
	post_recv(&qp, &wqe1);
	
	WQE wqe2;
	wqe2.sge.addr = mr.buffer + mr.sz / 2;
	wqe2.sge.length = mr.sz / 2;
	wqe2.wr_id = get_wr_id();
	post_recv(&qp, &wqe2);


//	WQE wqe3;
//	wqe3.sge.addr = mr.buffer;
//	wqe3.sge.length = mr.sz;
//	wqe3.wr_id = get_wr_id();
//	post_send(&qp, &wqe3);



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
	// Forcefully attaching socket to the port 8080 
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

//	global_socket = new_socket;

	valread = read(new_socket , buffer, 1024);
	printf("buffer is: %s\n", buffer);
	process_recv(&qp, buffer, 20);
	print_mr(&mr);

//	CQE cqe;
    
//	printf("cq_pop_front_ret: %d\n", cq_pop_front(&cq, &cqe));
//	printf("cqe byte_len: %d\n", cqe.byte_len);
//	printf("cqe.wr_id: %d\n", cqe.wr_id);


	valread = read(new_socket, buffer, 1024);
	process_recv(&qp, buffer, 20);

	printf("buffer is: %s\n", buffer);
	print_mr(&mr);
	printf("mr is: %s\n", mr.buffer);

//	valread = read(new_socket , buffer, 20);
//	process_recv(&qp, buffer, 20);

//	printf("cq_pop_front_ret: %d\n", cq_pop_front(&cq, &cqe));
//	printf("cqe byte_len: %d\n", cqe.byte_len);
//	printf("mr: %s\n", mr.buffer);


//	process_send(&qp, &send_data);	
	return 0; 
} 


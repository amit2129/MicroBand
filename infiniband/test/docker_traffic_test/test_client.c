// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include "../../src/infiniband.h"
#define PORT 1024


uint8_t send_data(QP *qp, WQE *wr_s, void *sock_ptr) {

	int sock = *(int *)sock_ptr;
	send(sock , wr_s->sge.addr , wr_s->sge.length , 0);
	return 0;
}


void print_mr(uint8_t *buffer, uint16_t size) {
	int i;
	for (i = 0; i < size; i++)
	{
	    if (i > 0) printf(":");
	    printf("%02X", buffer[i]);
	}
	printf("\n");
}

int main(int argc, char const *argv[]) 
{ 
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *data = "Amit sending data"; 
	char buffer[1024] = {0};
        printf("host is: %s\n", argv[1]);
	CQ cq;
	init_cq(&cq, 20);
	MR mr;
	init_mr(&mr, 40);
	QP qp;
	init_qp(&qp, &mr, &cq, 20);
	sleep(3);

	strcpy(mr.buffer, data);
	printf("data written to mr: %s\n", mr.buffer);
	WQE wqe1;
	wqe1.sge.addr = mr.buffer;
	wqe1.sge.length = mr.sz / 2;
	wqe1.wr_id = get_wr_id();

	uint8_t ret;
	ret = post_send(&qp, &wqe1);

	WQE wqe2;
	wqe2.sge.addr = mr.buffer;
	wqe2.sge.length = mr.sz / 2;
	wqe2.wr_id = get_wr_id();
	ret = post_send(&qp, &wqe2);

	post_send(&qp, &wqe1);
//	post_send(&qp, &wqe2);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}
//	sleep(1);

	process_send(&qp,(void *)&sock, &send_data);
	//process_send(&qp,(void *)&sock , &send_data);

	CQE cqe;
	printf("cq_pop1: %d\n", cq_pop_front(qp.completion_queue, &cqe));
	printf("cqe.status: %d\n", cqe.status);
	printf("cqe.byte_len: %d\n", cqe.byte_len);
	printf("cqe.qp_num: %d\n", cqe.qp_num);
	printf("completion received for wr_id: %d\n", cqe.wr_id);


	process_send(&qp,(void *)&sock, &send_data);

	printf("cq_pop2: %d\n", cq_pop_front(qp.completion_queue, &cqe));
	printf("cqe.status: %d\n", cqe.status);
	printf("cqe.byte_len: %d\n", cqe.byte_len);
	printf("cqe.qp_num: %d\n", cqe.qp_num);
	printf("completion received for wr_id: %d\n", cqe.wr_id);

	process_send(&qp,(void *)&sock, &send_data);

	printf("cq_pop3: %d\n", cq_pop_front(qp.completion_queue, &cqe));
	printf("cqe.status: %d\n", cqe.status);
	printf("cqe.byte_len: %d\n", cqe.byte_len);
	printf("cqe.qp_num: %d\n", cqe.qp_num);
	printf("completion received for wr_id: %d\n", cqe.wr_id);


	//valread = read(sock , buffer, 1024);
	//process_recv(&qp, buffer, 40);
	
	//print_mr(qp.mem_reg);

	printf("success\n");
	return 0; 
} 


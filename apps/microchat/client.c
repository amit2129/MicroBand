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

// keep QPs in a linked list named qp_ll
#define QP_LINKED_LIST

#include "../../../infiniband/src/infiniband.h"
#include "../../../common/src/utils.h"
#include "../../src/mb_transport_recv.h"

int main()
{

	struct sockaddr saddr;
	int sock_r,saddr_len,buflen;

	unsigned char* buffer = (unsigned char *)malloc(65536); 
	memset(buffer,0,65536);

	FILE *log_txt=fopen("log.txt","w");
	if(!log_txt)
	{
		printf("unable to open log.txt\n");
		return -1;

	}

	printf("starting .... \n");

	sock_r=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL)); 
	if(sock_r<0)
	{
		printf("error in socket\n");
		return -1;
	}
	ll_init(&qp_ll);
	
	// init objects:
	const int OBJECT_NUM = 2;
	const int OBJECT_SIZE = 10;


	QP qps[OBJECT_NUM];
	CQ cqs[OBJECT_NUM];
	MR mrs[OBJECT_NUM];
	WQE wqes[OBJECT_NUM];
	
	for (int i = 0; i < OBJECT_NUM; i++) {
		init_cq(cqs + i, OBJECT_SIZE);
		init_mr(mrs + i, OBJECT_SIZE * 4);
		init_qp(qps + i, mrs + i, cqs + i, OBJECT_SIZE);
		qps[i].remote_qp_num = OBJECT_NUM-i;
		printf("qp_num is: %d\n", qps[i].qp_num);

		wqes[i].sge.addr = mrs[i].buffer;
		wqes[i].sge.length = sizeof(uint32_t);
		wqes[i].wr_id = get_wr_id();

		post_recv(qps + i, wqes + i);
		//ll_insert(&qp_ll, qp_nodes + i);
	}
	ll_print(&qp_ll, &print_qp);

	while(1)
	{
		saddr_len=sizeof saddr;
		buflen=recvfrom(sock_r,buffer,65536,0,&saddr,(socklen_t *)&saddr_len);


		if(buflen<0)
		{
			printf("error in reading recvfrom function\n");
			return -1;
		}
		fflush(log_txt);
		if (!process_packet(buffer, log_txt)) {
			recv_num--;
		}

	}

	close(sock_r);
	printf("finished\n");

}


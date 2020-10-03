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
#include "../../../infiniband/src/infiniband.h"
#include "../../../common/src/utils.h"
#include "../../src/mb_transport_send.h"


struct ifreq ifreq_c,ifreq_i,ifreq_ip; /// for each ioctl keep diffrent ifreq structure otherwise error may come in sending(sendto )


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
	init_mr(&mr, 10);
	QP qp;
	init_qp(&qp, &mr, &cq, 10);
	qp.qp_num = 21;
	qp.remote_qp_num = 1;
	mr.buffer[0] = 0x21;
	mr.buffer[1] = 0x29;
	mr.buffer[2] = 0x22;
	mr.buffer[3] = 0x30;

	WQE send_wqe;
	send_wqe.sge.addr = mr.buffer;
	send_wqe.sge.length = sizeof(uint32_t);

	struct send_util su = {raw_socket, send_buffer, send_len, &sadr_ll};
	send_wqe.wr_id = 0;
	post_send(&qp, &send_wqe);
	process_send_handle(&qp, (void *)&su);
	printf("sent_data from a qp\n");
	return 0;
}

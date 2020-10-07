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


void send_packet_util(struct send_util *send_util){
	int sent_bytes = sendto(send_util->socket,
				send_util->buffer,
				send_util->send_len, 
				0, 
				(const struct sockaddr*)(send_util->sadr_ll), 
				sizeof(struct sockaddr_ll));
	
	if(sent_bytes<0)
	{
		printf("error in sending....sendlen=%d....errno=%d\n",sent_bytes,errno);
		exit(-1);
	}

}


uint8_t send_data(QP *qp, WQE *wr_s, void *send_util) {

	struct send_util util = *(struct send_util *)send_util;
	uint16_t data_len = wr_s->sge.length;

	// wrote header to buffer
	util.send_len += write_mb_header(qp, data_len, util.buffer + util.send_len);
	// wrote data to buffer
	memcpy(util.buffer + util.send_len, wr_s->sge.addr, wr_s->sge.length);
	util.send_len += wr_s->sge.length;

	// util fields
	// socket was preassigned
	// buffer was preassigned
	// send_len was increased
	// sadr was preassigned
	send_packet_util(&util);
	return 0;
}

uint8_t (*qp_send_func)(QP *, WQE *, void *); // =&send_data


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


unsigned short checksum(unsigned short* buff, int _16bitword)
{
	unsigned long sum;
	for(sum=0;_16bitword>0;_16bitword--)
		sum+=htons(*(buff)++);
	do
	{
		sum = ((sum >> 16) + (sum & 0xFFFF));
	}
	while(sum & 0xFFFF0000);

	return (~sum);


	
}
 
uint16_t write_eth_header(uint8_t *packet_buffer,uint8_t *dest_mac){
	struct ethhdr *eth = (struct ethhdr *)(packet_buffer);
  	eth->h_source[0] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]);
  	eth->h_source[1] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]);
   	eth->h_source[2] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]);
   	eth->h_source[3] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]);
   	eth->h_source[4] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]);
   	eth->h_source[5] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]);

   	eth->h_dest[0]    =  dest_mac[0];
   	eth->h_dest[1]    =  dest_mac[1];
   	eth->h_dest[2]    =  dest_mac[2];
  	eth->h_dest[3]    =  dest_mac[3];
   	eth->h_dest[4]    =  dest_mac[4];
   	eth->h_dest[5]    =  dest_mac[5];

   	eth->h_proto = htons(ETH_P_MB); 

	return sizeof(struct ethhdr);



}

void parse_mac(char *mac_string, uint8_t *bytes) {

	int values[6];
	int i;

	if( 6 == sscanf( mac_string, "%x:%x:%x:%x:%x:%x%*c",
				    &values[0], &values[1], &values[2],
				        &values[3], &values[4], &values[5] ) )
	{
		    /* convert to uint8_t */
		    for( i = 0; i < 6; ++i )
			            bytes[i] = (uint8_t) values[i];
	}

	else
	{
		printf("illegal mac\n");
		exit(1);
	}
}


uint16_t set_mandatory_values(int raw_socket, uint8_t *send_buffer, char *ifname, uint8_t *destination_mac) {
	printf("ifname is: %s\n", ifname);
	memset(&ifreq_i,0,sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name, ifname, IFNAMSIZ-1);

	if((ioctl(raw_socket, SIOCGIFINDEX,&ifreq_i))<0)
		printf("error in index ioctl reading");

	printf("index=%d\n",ifreq_i.ifr_ifindex);

	memset(&ifreq_c,0,sizeof(ifreq_c));
	strncpy(ifreq_c.ifr_name,ifname,IFNAMSIZ-1);

	if((ioctl(raw_socket,SIOCGIFHWADDR,&ifreq_c))<0)
		printf("error in SIOCGIFHWADDR ioctl reading");

	printf("Source Mac= %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]));
	printf("Dest Mac= %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", destination_mac[0], destination_mac[1], destination_mac[2], destination_mac[3], destination_mac[4], destination_mac[5]);

	return write_eth_header(send_buffer, destination_mac);	
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

	send_wqe.wr_id = 0;
	post_send(&qp, &send_wqe);
	struct send_util su = {raw_socket, send_buffer, send_len, &sadr_ll};
	process_send_handle(&qp, (void *)&su);
	printf("sent_data from a qp\n");
	return 0;
}

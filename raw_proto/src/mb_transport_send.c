#include "mb_transport_send.h"
#include <stdio.h>

uint16_t write_mb_header(QP *qp, uint8_t data_len, uint8_t *send_buffer)
{
	mb_transport *mbh = (mb_transport *)send_buffer;
	mbh->source_qp 	= htons(qp->qp_num);
	mbh->dest_qp	= htons(qp->remote_qp_num);
	mbh->data_len = htons(data_len);
    mbh->reserved_2 = htons(qp->state);
	printf("send reserved_2 as: %d\n", qp->state);
	return sizeof(mb_transport);
}

struct ifreq ifreq_c,ifreq_i,ifreq_ip;
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
	printf("data_len is: %d", data_len);

	//	printf("send")

	// wrote header to buffer
	util.send_len += write_mb_header(qp, data_len, util.buffer + util.send_len);
	// wrote data to buffer
	read_from_mr(qp->mem_reg, wr_s->sge.addr - qp->mem_reg->buffer, util.buffer + util.send_len, wr_s->sge.length);
	util.send_len += wr_s->sge.length;

	// util fields
	// socket was preassigned
	// buffer was preassigned
	// send_len was increased
	// sadr was preassigned
	//
	printf("52 before send, util data_len is: %d\n", util.send_len);
	send_packet_util(&util);
	return 0;
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
	printf("prot is: %d\n", htons(ETH_P_MB));

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

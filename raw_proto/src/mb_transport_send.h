#ifndef _MB_TRANSPORT_SEND_H
#define _MB_TRANSPORT_SEND_H

#include "mb_transport_shared.h"
#include <stdint.h>
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

#include "../../infiniband/src/infiniband.h"

struct send_util {
		int socket;
		uint8_t *buffer;
		uint16_t send_len;
		struct sockaddr_ll *sadr_ll;
};

uint16_t write_mb_header(QP *qp, uint8_t data_len, uint8_t *send_buffer);


void send_packet_util(struct send_util *send_util);


uint8_t send_data(QP *qp, WQE *wr_s, void *send_util); 


unsigned short checksum(unsigned short* buff, int _16bitword);


uint16_t write_eth_header(uint8_t *packet_buffer,uint8_t *dest_mac);


void parse_mac(char *mac_string, uint8_t *bytes);


uint16_t set_mandatory_values(int raw_socket, uint8_t *send_buffer, char *ifname, uint8_t *destination_mac);

#endif

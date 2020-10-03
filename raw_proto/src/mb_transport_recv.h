#ifndef _MB_TRANSPORT_RECV
#define _MB_TRANSPORT_RECV

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

#include "mb_transport_shared.h"
#include "../../infiniband/src/infiniband.h"
#include "../../common/src/utils.h"


int get_by_qp_num(void *vp_qp, void *vp_qp_num);


void dispatch_to_qp(mb_transport *mb_trns, uint8_t *data, FILE *log_txt);


void parse_payload(unsigned char* buffer,int data_len, mb_transport* mb_trns, FILE* log_txt);


void parse_mb_transport_header(unsigned char* buffer, FILE *log_txt);


void parse_ethernet_header(unsigned char* buffer, FILE *log_txt, uint8_t *dest_mac, uint8_t* source_mac);


int process_packet(unsigned char* buffer, FILE *log_txt, uint8_t *dest_mac, uint8_t *source_mac);

#endif

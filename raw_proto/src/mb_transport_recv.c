#define QP_LINKED_LIST
#include "mb_transport_recv.h"
#include "../../infiniband/src/infiniband.h"
#include <stdio.h>




void parse_payload(unsigned char* buffer,int data_len, mb_transport* mb_trns, FILE* log_txt){
	int i=0;
//	unsigned char * data = buffer;
	//fprintf(log_txt,"\nData\n");
//	//fprintf(log_txt,"data as str: %s\n", data);
//	float *float_ptr = (float*) (buffer + strlen(buffer) + 1);
//	//fprintf(log_txt, "float_data: %f\n", *float_ptr);
	
	for(i=0;i<data_len;i++)
	{
	//	if(i!=0 && i%16==0)
			//fprintf(log_txt,"\n");
		//fprintf(log_txt," %.2X ",data[i]);
	}

	//fprintf(log_txt,"\n");
	dispatch_to_qp(mb_trns, buffer, log_txt);
}


void parse_mb_transport_header(unsigned char* buffer, FILE *log_txt){
	//fprintf(log_txt,"\nMicroband Transport Header\n");

	mb_transport *mb_trns = (mb_transport*)buffer;
	//fprintf(log_txt , "\t|-Source QP		: %d\n" , ntohs(mb_trns->source_qp));
	//fprintf(log_txt , "\t|-Destination QP	: %d\n" , ntohs(mb_trns->dest_qp));
	//fprintf(log_txt , "\t|-Sequence Number	: %d\n" , ntohs(mb_trns->sequence_number));
	//fprintf(log_txt , "\t|-Ack Number		: %d\n" , ntohs(mb_trns->ack_number));
	//fprintf(log_txt , "\t|-Data len		: %d\n" , ntohs(mb_trns->data_len));
	//fprintf(log_txt , "\t|-Reserved2		: %d\n" , ntohs(mb_trns->reserved_2));
         
	parse_payload(buffer + sizeof(mb_transport) ,ntohs(mb_trns->data_len), mb_trns, log_txt);

	//fprintf(log_txt,"*****************************************************************\n\n\n");
}

int validate_mac(uint8_t *mac_a, uint8_t *mac_b) {

	for (uint8_t i = 0; i < 6; i++) {
		if (mac_a[i] != mac_b[i]) {
			printf("\t|-MAC-A	: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",mac_a[0],mac_a[1],mac_a[2],mac_a[3],mac_a[4],mac_a[5]);
			printf("\t|-MAC-B	: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",mac_b[0],mac_b[1],mac_b[2],mac_b[3],mac_b[4],mac_b[5]);
			return 1;
		}
	}
	return 0;
}


void parse_ethernet_header(unsigned char* buffer, FILE *log_txt, uint8_t* dest_mac, uint8_t *source_mac)
{
	struct ethhdr *eth = (struct ethhdr *)buffer;
	//fprintf(log_txt,"\nEthernet Header\n");
	//fprintf(log_txt,"\t|-Source Address	: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
	//fprintf(log_txt,"\t|-Destination Address	: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
	//fprintf(log_txt,"\t|-Protocol		: %04x\n",ntohs(eth->h_proto));
	if (validate_mac(eth->h_dest, dest_mac)){
			printf("packet dropped, wrong destination\n");
			return;
	}
        if (source_mac && validate_mac(eth->h_source, source_mac)){
			printf("packet dropped, wrong source\n");
			return;
	}
	parse_mb_transport_header(buffer + sizeof(struct ethhdr), log_txt);
}



int process_packet(unsigned char* buffer, FILE *log_txt, uint8_t *dest_mac, uint8_t *source_mac)
{

	struct ethhdr *eth = (struct ethhdr *)(buffer);
	if (eth->h_proto == htons(ETH_P_MB)) {

	//fprintf(log_txt,"\n*************************MicroBand Packet******************************");
		parse_ethernet_header(buffer, log_txt, dest_mac, source_mac);
		return 0;
	}
	else {
		return -1;
	}	

}

#define QP_LINKED_LIST
#include "mb_transport_recv.h"
#include "../../infiniband/src/infiniband.h"

int get_by_qp_num(void *vp_qp, void *vp_qp_num) {
	return (((QP *)vp_qp)->qp_num == *(uint32_t *)vp_qp_num);
}


void dispatch_to_qp(mb_transport *mb_trns, uint8_t *data, FILE *log_txt){
	uint32_t dest_qp_num = ntohs(mb_trns->dest_qp);
	fprintf(log_txt, "attempting to dispatch to QP num: %d\n", dest_qp_num);
	QP *qp = get_object_with_data(&qp_ll, &get_by_qp_num, (void *)&dest_qp_num);
	if (qp) {
		process_recv(qp, data, ntohs(mb_trns->data_len));
		CQE cqe;
		int ret_poll = cq_pop_front(qp->completion_queue, &cqe);
		if (ret_poll)
			fprintf(log_txt, "No cqe, receive failed for QP\n");
		else 
			fprintf(log_txt, "Successfully received at QP with byte_len: %d\n", cqe.byte_len);
		return;

	}
	else {
		printf("QP with dest_qp_num: %d not found\n", dest_qp_num);
		exit(1);
	}
	
	fprintf(log_txt, "no QP with num: %d\n", dest_qp_num);
}



void parse_payload(unsigned char* buffer,int data_len, mb_transport* mb_trns, FILE* log_txt){
	int i=0;
	unsigned char * data = (buffer);
	fprintf(log_txt,"\nData\n");
	
	for(i=0;i<data_len;i++)
	{
		if(i!=0 && i%16==0)
			fprintf(log_txt,"\n");
		fprintf(log_txt," %.2X ",data[i]);
	}

	fprintf(log_txt,"\n");
	dispatch_to_qp(mb_trns, buffer, log_txt);
}


void parse_mb_transport_header(unsigned char* buffer, FILE *log_txt){
	fprintf(log_txt,"\nMicroband Transport Header\n");

	mb_transport *mb_trns = (mb_transport*)buffer;
	fprintf(log_txt , "\t|-Source QP		: %d\n" , ntohs(mb_trns->source_qp));
	fprintf(log_txt , "\t|-Destination QP	: %d\n" , ntohs(mb_trns->dest_qp));
	fprintf(log_txt , "\t|-Sequence Number	: %d\n" , ntohs(mb_trns->sequence_number));
	fprintf(log_txt , "\t|-Ack Number		: %d\n" , ntohs(mb_trns->ack_number));
	fprintf(log_txt , "\t|-Data len		: %d\n" , ntohs(mb_trns->data_len));
	parse_payload(buffer + sizeof(mb_transport) ,ntohs(mb_trns->data_len), mb_trns, log_txt);

	fprintf(log_txt,"*****************************************************************\n\n\n");
}


void parse_ethernet_header(unsigned char* buffer, FILE *log_txt)
{
	struct ethhdr *eth = (struct ethhdr *)buffer;
	fprintf(log_txt,"\nEthernet Header\n");
	fprintf(log_txt,"\t|-Source Address	: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
	fprintf(log_txt,"\t|-Destination Address	: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
	fprintf(log_txt,"\t|-Protocol		: %04x\n",ntohs(eth->h_proto));
	parse_mb_transport_header(buffer + sizeof(struct ethhdr), log_txt);
}


int process_packet(unsigned char* buffer, FILE *log_txt)
{

	struct ethhdr *eth = (struct ethhdr *)(buffer);
	if (ntohs(eth->h_proto) == ETH_P_MB) {

	fprintf(log_txt,"\n*************************MicroBand Packet******************************");
		parse_ethernet_header(buffer, log_txt);
		return 0;
	}
	else {
		return -1;
	}	

}

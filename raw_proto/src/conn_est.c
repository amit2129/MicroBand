#include <stdio.h>
#include <unistd.h>

#include "conn_est.h"
#include "mb_transport_shared.h"
#include "../../infiniband/src/infiniband.h"

#define WR_ID_SEND 1
#define WR_ID_RECV 2


void print_mac(uint8_t *mac){
	printf("mac: ");
	for (int i = 0; i < 6; i++) {
		if (i == 5)
			printf("%.02X\n", mac[i]);
		else
			printf("%.02X-", mac[i]);
	}
}




void init_cned(cned *conn_est_data, uint32_t local_qp_num, size_t qp_num_offset) {
	*(uint32_t *)(conn_est_data + qp_num_offset) = local_qp_num;
	conn_est_data->initiator_state = QPS_RESET;
	conn_est_data->recipient_state = QPS_RESET;
}


void establish_conn(QP *qp, uint8_t *smac, enum EstablishmentRole role){


	switch (role) {
		case INITIATOR:
			establish_conn_initiator(qp, smac);
			break;
		case RECIPIENT:
			establish_conn_recipient(qp);
			break;	
		default:
			printf("role is incorrect");
	}
}

void print_cned(cned *conn_est_data) {
	printf("****************CONN EST DATA*******************\n");
	printf("qp_num_initiator	:%d\n", conn_est_data->qp_num_initiator);
	printf("qp_num_recipient	:%d\n", conn_est_data->qp_num_recipient);
	printf("initiator_state		:%d\n", conn_est_data->initiator_state);
	printf("recipient_state		:%d\n", conn_est_data->recipient_state);
	printf("remote_mac_address	:");
	for (int i = 0; i < 6; i++ ) {
		if (i != 5)
			printf("%.02X-", conn_est_data->remote_mac_address[i]);
		else
			printf("%.02X", conn_est_data->remote_mac_address[i]);
	}
	printf("\n**********************************************\n");

}

void establish_conn_initiator(QP *qp, uint8_t *smac) {
	cned conn_est_data;
	WQE send_wqe;
	send_wqe.sge.addr = qp->mem_reg->buffer;
	send_wqe.sge.length = sizeof(cned);
	send_wqe.wr_id = WR_ID_SEND;

	// to initiate a connection we move our QP state to INIT
	// populate the qp_num_initiator, initiator_state and mac_address 
	qp->state = QPS_INIT;
	conn_est_data.qp_num_initiator = qp->qp_num;
	memcpy(conn_est_data.remote_mac_address, smac, 6);
	conn_est_data.initiator_state = qp->state;
	write_to_mr(qp->mem_reg, 0, (uint8_t *)&conn_est_data, sizeof(cned));

	read_from_mr(qp->mem_reg, 0, (uint8_t *)&conn_est_data, sizeof(cned));

	print_cned(&conn_est_data);

	//printf("posting send on qp number %d\n", qp->qp_num);
	//post_send(qp, &send_wqe);


	WQE recv_wqe;
	recv_wqe.sge.addr = qp->mem_reg->buffer + sizeof(cned);
	recv_wqe.sge.length = sizeof(cned);
	recv_wqe.wr_id = WR_ID_RECV;

	CQE cqe;
	//	post_recv(qp, &recv_wqe);
	//	post_send(qp, &send_wqe);
	//	
	//	sleep(5);
	//	int ret = cq_pop_front(qp->completion_queue, &cqe);
	//	printf("pop_ret is: %d\n", ret);
	//	printf("cqe_status is: %d\n", cqe.status);
	//	printf("cqe.wr_id = %d\n", cqe.wr_id);
	//
	//	
	//	ret = cq_pop_front(qp->completion_queue, &cqe);
	//	printf("pop_ret is: %d\n", ret);
	//	printf("cqe_status is: %d\n", cqe.status);
	//	printf("cqe.wr_id = %d\n", cqe.wr_id);
	//
	//	read_from_mr(qp->mem_reg, sizeof(cned), (uint8_t *)&conn_est_data, sizeof(cned));
	//
	//	print_cned(&conn_est_data);
	//	
	//	while (1) {
	//		sleep(1);
	//		cq_pop_front(qp->completion_queue, &cqe);
	//		
	//		printf("cqe_status is: %d\n", cqe.status);
	//		printf("cqe.wr_id = %d\n", cqe.wr_id);
	//
	//	}
	//
	for (int i = 0; i < qp->completion_queue->queue.capacity; i++) {
		cq_pop_front(qp->completion_queue, &cqe);
	}
	
	printf("POSTED RECEIVE\n");
	post_recv(qp, &recv_wqe);


	printf("POSTED SEND\n");
	post_send(qp, &send_wqe);


	int counter = 0;
	while(counter < 2) {
		if(!cq_pop_front(qp->completion_queue, &cqe))
				counter++;
	}

	printf("received two completions 132\n");
	read_from_mr(qp->mem_reg, sizeof(cned), (uint8_t *)&conn_est_data, sizeof(cned));
	printf("received\n");
	print_cned(&conn_est_data);

	qp->remote_qp_num = conn_est_data.qp_num_recipient;
	qp->state = QPS_RTS;
	conn_est_data.initiator_state = qp->state;
	write_to_mr(qp->mem_reg, 0, (uint8_t *)&conn_est_data, sizeof(cned));
	printf("sent");
	print_cned(&conn_est_data);

	printf("ABOUT TO POST SEND\n");
	post_send(qp, &send_wqe);
	post_send(qp, &send_wqe);
	printf("FINISHED_CONNECTING\n");
}

void establish_conn_recipient(QP *qp) {
	qp->state = QPS_RTR;
	cned conn_est_data;

	WQE recv_wqe;
	recv_wqe.sge.addr = qp->mem_reg->buffer;
	recv_wqe.sge.length = sizeof(cned);
	recv_wqe.wr_id = WR_ID_RECV;


	// received connection request, conn_est_data can be read from MR and 
	// in it the qp_num_establisher field should be valid.
	read_from_mr(qp->mem_reg, 0, (void *)&conn_est_data, sizeof(conn_est_data));

	print_cned(&conn_est_data);


	//qp->remote_qp_num = conn_est_data.qp_num_establisher;

	conn_est_data.qp_num_recipient = qp->qp_num;
	conn_est_data.recipient_state = qp->state;

	write_to_mr(qp->mem_reg, sizeof(cned), (void *)&conn_est_data, sizeof(conn_est_data));

	WQE send_wqe;
	send_wqe.sge.addr = qp->mem_reg->buffer + sizeof(cned);
	send_wqe.sge.length = sizeof(cned);
	send_wqe.wr_id = WR_ID_SEND;

	CQE cqe;
	
	post_recv(qp, &recv_wqe);
	post_recv(qp, &recv_wqe);

	post_send(qp, &send_wqe);

	int counter = 0;
	while(counter <= 2) {
		if(!cq_pop_front(qp->completion_queue, &cqe))
				counter++;
	}

	read_from_mr(qp->mem_reg, sizeof(cned), (uint8_t *)&conn_est_data, sizeof(cned));
	print_cned(&conn_est_data);
	qp->state = QPS_RTS;

}

//enum ConnStateSignal {CONN_REQ, REQ_ACK};
//
//
//
//enum ConnStateInitiator {
//			 I_INIT, // ignoring all 
//			 I_SENDING_CONN_REQ,// WAITING REQ_ACK
//			 // during: nothing
//			 // after : remote + know that remote has local, at this point we can start sending and the remote will receive our messages
//
//			 I_CONNECTED// once we receive any messaged addressed to us from the correct qp we know we are connected
//			};
//
//enum ConnStateRecipient {			    
//   			 R_INIT, // ignoring all
//    			 R_WAITING_FOR_REQ,// WAITING CONN_REQ
//			 // during: nothing (accepting requests from all QPs, discarding until CONN_REQ arrives)
//			 // after: remote, can start receiving data
//
//			 R_SENDING_REQ_ACK,// waiting for any packet from the remote targeted at the local QP
//			 // during: remote
//			 // after: remote + know that remote has local // can star
//
//    			 R_CONNECTED
//			};




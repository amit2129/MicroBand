#ifndef _CONN_EST_H
#define _CONN_EST_H
#include "../../infiniband/src/infiniband.h"

//enum ConnState {RESET, INIT, RTR, RTS};
enum EstablishmentRole {INITIATOR, RECIPIENT};

typedef struct connection_establishment_data {
	uint32_t qp_num_initiator;
	uint32_t qp_num_recipient;
	enum QP_State initiator_state;
	enum QP_State recipient_state;
	uint8_t remote_mac_address[6];
} cned;



void init_cned(cned *conn_est_data, uint32_t local_qp_num, size_t qp_num_offset);


void establish_conn(QP *qp, uint8_t *smac, enum EstablishmentRole role);


void establish_conn_initiator(QP *qp, uint8_t *smac);
 
void establish_conn_recipient(QP *qp); 

void print_mac(uint8_t *mac);
		
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


#endif




void poll_and_update_location(QP *qp, WQE *recv_wqe, player_location *shared_data_ptr){
    CQE cqe;
    while (cq_pop_front(qp->completion_queue, &cqe)) {
        if cqe.wr_id == recv_wqe->wr_id {
	    float received_location;
	    uint8_t *reinterpret_ptr = (uint8_t *)received_location;
	    uint32_t offset = recv_wqe->sge.addr - qp->mem_reg.buffer;
	    read_from_mr(qp->mem_reg, offset, reinterpret_ptr, sizeof(float));
	    printf("data received from client is: %d", shared_location);
	    write_shared_location(shared_data_ptr, received_location);
        }
	else {
	    // polled a send completion
	}
    }
}

typedef struct client_connection_args {
	uint8_t mr_offset;
	QP *new_qp;
} client_connection_args;


void *client_connection(void *cc_args) {
    client_connection_args *args = (client_connection_args *)cc_args;

    player_location local;
    player_location *access_ptr = cc_args->access_ptr;

    conn_data *shared_data_ptr = cc_args->shared_data;
    QP *qp = args->new_qp;
    qp->remote_qp_num = args->client_qp_num;

    // send is of exact size
    WQE send_wqe;
    send_wqe.sge.addr = qp->mem_reg.buffer;
    send_wqe.sge.length = sizeof(player_location) + sizeof(pong_location);
    send_wqe.wr_id = SEND_WR_ID


    // recv is the rest of the mr_size
    WQE recv_wqe;
    recv_wqe.sge.addr = qp->mem_reg.buffer + send_wqe.sge.length;
    recv_wqe.sge.length = qp->mem_reg.sz - send_wqe.sge.length;
    recv_wqe.wr_id = RECV_WR_ID;

    while(1) {
	poll_and_update_objects(qp, recv_wqe, access_ptr);

	post_recv(qp, &recv_wqe); // posting recv wqe to later receive

	write_data_to_mr(qp->mem_reg, shared_data_ptr)
	write_to_mr(qp->mem_reg, 0, )
	post_send(&qp, &send_wqe);
    }
}



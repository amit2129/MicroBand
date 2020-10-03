

struct chat_message {
	char *sender;
	char *receiver;
	char *message;
	uint64_t timestamp;
}


uint32_t send_message(uint32_t buffer*, struct chat_message *msg) {
	uint32_t send_len = 0;
	memcpy(buffer + send_len, sender, strlen(sender) + 1);
	send_len += strlen(sender) + 1;
	
	memcpy(buffer + send_len, receiver, strlen(receiver) + 1);
	send_len += strlen(receiver) + 1;
	
	memcpy(buffer + send_len, message, strlen(message) + 1);
	send_len += strlen(message) + 1;

	uint8_t timestamp_ptr = (uint8_t *) &timestamp;
	
	memcpy(buffer + send_len, timestamp_ptr, sizeof(uint64_t));
	send_len += sizeof(uint64_t);

	return send_len;
}


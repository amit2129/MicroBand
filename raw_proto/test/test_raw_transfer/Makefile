# the compiler: gcc for C program, define as g++ for C++
CC = gcc
BUILD_DIR=build
SRC=src
TEST=test
SERV=docker_server
CLNT=docker_client

INFINIBAND_DIR=../../../infiniband
COMMON_DIR=../../../common
TRANSPORT_PROTO_DIR=../..

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:
TARGET = raw_proto_test
TARGET1 = send_raw
TARGET2 = recv_raw

all: $(TARGET)_dockers

$(TARGET): $(TARGET1) $(TARGET2)

$(INFINIBAND_DIR)/%.o:
	make -C $(INFINIBAND_DIR) build_run_ll

$(COMMON_DIR)/%.o:
	make -C $(COMMON_DIR)

$(TRANSPORT_PROTO_DIR)/%.o:
	make -C $(TRANSPORT_PROTO_DIR) mb_transport_recv.o

$(TARGET1): $(INFINIBAND_DIR)/%.o $(COMMON_DIR)/%.o
	gcc $(INFINIBAND_DIR)/*.o $(COMMON_DIR)/*.o -o $(TARGET1) send_raw_packets.c

$(TARGET2): $(INFINIBAND_DIR)/%.o $(COMMON_DIR)/%.o $(TRANSPORT_PROTO_DIR)/%.o
	gcc $(INFINIBAND_DIR)/*.o $(COMMON_DIR)/*.o $(TRANSPORT_PROTO_DIR)/*.o -o $(TARGET2) recv_raw_packets.c

$(TARGET)_dockers: $(TARGET)
	docker build . -t $(SERV) -f server_dockerfile
	docker build . -t $(CLNT) -f client_dockerfile
	touch $(TARGET)_dockers

run_client:
	docker run -t --rm  --network=test_network docker_client

run_server:
	docker run -it --rm -v ~/git/MicroBand/raw_proto/test/docker_test_raw:/share --network=test_network --name=test_server docker_server

clean:
	$(RM) $(TARGET1) $(TARGET2)
	$(RM) log.txt


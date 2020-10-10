# the compiler: gcc for C program, define as g++ for C++
CC = gcc
BUILD_DIR=build
SRC=src
TEST=test
SERV=docker_server
CLNT=docker_client

INFINIBAND_DIR=../../infiniband
COMMON_DIR=../../common
TRANSPORT_PROTO_DIR=../../raw_proto

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:
TARGET = raw_proto_test
TARGET1 = pong_client


$(TARGET): $(TARGET1) 

$(INFINIBAND_DIR)/%.o:
	@+$(MAKE) -C $(INFINIBAND_DIR) build_run_ll

$(COMMON_DIR)/%.o:
	@make -C $(COMMON_DIR)

$(TRANSPORT_PROTO_DIR)/%.o:
	@make -C $(TRANSPORT_PROTO_DIR) all

$(TARGET1): $(INFINIBAND_DIR)/%.o $(COMMON_DIR)/%.o $(TRANSPORT_PROTO_DIR)/%.o
	@echo $(INFINIBAND_DIR)/*.o $(COMMON_DIR)/*.o
	@gcc -pthread  $(INFINIBAND_DIR)/*.o $(COMMON_DIR)/*.o $(TRANSPORT_PROTO_DIR)/*.o -o $(TARGET1) $(TARGET1).c


clean:
	$(RM) $(TARGET1) $(TARGET2)
	$(RM) log.txt


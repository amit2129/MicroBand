# the compiler: gcc for C program, define as g++ for C++
CC = gcc
BUILD_DIR=build
COMMON_DIR=../common
INFINIBAND_DIR=../infiniband

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

recv_send: mb_transport_recv.i mb_transport_send.i 

all:common_utils infiniband
	$(CC) $(CFLAGS) -c src/*.c


common_utils:
	make -C $(COMMON_DIR)

infiniband:
	make -C $(INFINIBAND_DIR) build_run_ll

mb_transport_recv.i: common_utils infiniband
	$(CC) $(CFLAGS)  -c src/mb_transport_recv.c

mb_transport_send.i: common_utils infiniband
	$(CC) $(CFLAGS)  -c src/mb_transport_send.c

clean:
	$(RM) *.o


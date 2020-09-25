# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall
COMMON_DIR=../common

# the build target executable:

all: build_test

build_test:
	$(RM) -r *.o
	$(CC) $(CFLAGS) -c -DDEBUG src/*.c
	$(CC) $(CFLAGS) -c -DDEBUG test/*.c

$(COMMON_DIR)/%.o:
	make -C $(COMMON_DIR)

build_run:
	$(RM) -r *.o
	$(CC) -c src/*.c

build_run_ll:
	$(RM) -r *.o
	$(CC) -DQP_LINKED_LIST -c src/*.c

test_traffic: build_run
	cp src/infiniband.h test/docker_traffic_test/
	@+$(MAKE) test_cl -C test/docker_traffic_test

test_pingpong: build_run
	@+$(MAKE) test_all -C test/docker_pingpong

clean:
	@+$(MAKE) clean -C test/docker_traffic_test
	@+$(MAKE) clean -C test/docker_pingpong
	$(RM) *.o
	$(RM) src/*.gch
	$(RM) test/*.gch


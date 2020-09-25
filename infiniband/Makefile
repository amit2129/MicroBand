# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:

all: build_test

build_test:
	$(RM) -r *.o
	$(CC) $(CFLAGS) -c -DDEBUG src/*.c
	$(CC) $(CFLAGS) -c -DDEBUG test/*.c

build_run:
	$(RM) -r *.o
	$(CC) -c src/*.c

test_traffic: build_run
	cp src/infiniband.h test/docker_traffic_test/
	@+$(MAKE) test_cl -C test/docker_traffic_test
	@+$(MAKE) test_sr -C test/docker_traffic_test
clean:
	@+$(MAKE) clean -C test/docker_traffic_test
	$(RM) *.o
	$(RM) src/*.gch
	$(RM) test/*.gch


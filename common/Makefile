# the compiler: gcc for C program, define as g++ for C++
CC = gcc
BUILD_DIR=build
INFINIBAND_DIR=$(SRC)/infiniband

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:

all: build

build:
	touch build
	$(CC) $(CFLAGS) -c src/*.c


build_test: build
	touch build_test
	$(CC) $(CFLAGS) -Isrc/ *.o test/main.c -o main

test: build_test
	./main

clean:
	$(RM) *.o
	$(RM) compiled
	$(RM) build
	$(RM) build_test


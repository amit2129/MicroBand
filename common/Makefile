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
	$(CC) $(CFLAGS) -c src/*.c

clean:
	$(RM) *.o
	$(RM) compiled


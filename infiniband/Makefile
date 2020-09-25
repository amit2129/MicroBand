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

clean:
	$(RM) *.o
	$(RM) src/*.gch
	$(RM) test/*.gch


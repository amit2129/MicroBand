# the compiler: gcc for C program, define as g++ for C++
CC = gcc
BUILD_DIR=build
SRC=src
TEST=test
INFINIBAND_DIR=$(SRC)/infiniband

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:
TARGET = microband

all: $(TARGET)_test

$(TARGET)_run:
	$(RM) $(TARGET)
	$(RM) $(TARGET)_test
	+$(MAKE) build_run -C infiniband
	cp $(TARGET).ino $(TARGET).c
	$(CC) $(CFLAGS) -c $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET)_run $$(find . -name "*.o")
	ln -s $(TARGET)_run $(TARGET)

$(TARGET)_test:
	$(RM) $(TARGET)
	$(RM) $(TARGET)_run
	+$(MAKE) build_test -C infiniband
	cp $(TARGET).ino $(TARGET).c
	$(CC) $(CFLAGS) -DDEBUG -c $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET)_test $$(find . -name "*.o")
	ln -s $(TARGET)_test $(TARGET)


build_run: $(TARGET)_run

build_test: $(TARGET)_test

test: $(TARGET)_test
	@./$(TARGET)

test_traffic:
	+$(MAKE) test_traffic -C infiniband

run: $(TARGET)_run
	@./$(TARGET)
clean:
	$(RM) $(TARGET)
	$(RM) $(TARGET)_run
	$(RM) $(TARGET)_test
	$(RM) $(TARGET).c
	$$(find . -name "*.o" -type f -delete)
	@+$(MAKE) clean -C infiniband

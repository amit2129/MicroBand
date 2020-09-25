# the compiler: gcc for C program, define as g++ for C++
CC = gcc
BUILD_DIR=build
SRC=src
TEST=test
INFINIBAND_DIR=$(SRC)/infiniband
COMMON_DIR=common

# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:
TARGET = microband

all: $(TARGET)_test

$(COMMON_DIR)/compiled:
	+$(MAKE) -C $(COMMON_DIR)
	touch $(COMMON_DIR)/compiled

$(TARGET)_run: $(COMMON_DIR)/compiled
	$(RM) $(TARGET)
	$(RM) $(TARGET)_test
	+$(MAKE) build_run -C infiniband
	cp $(TARGET).ino $(TARGET).c
	$(CC) $(CFLAGS) -c $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET)_run $$(find . -name "*.o")
	ln -s $(TARGET)_run $(TARGET)

$(TARGET)_test: $(COMMON_DIR)/compiled
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
	+$(MAKE) -C $(COMMON_DIR) test

test_traffic:
	+$(MAKE) test_traffic -C infiniband

test_pingpong:
	+$(MAKE) test_pingpong -C infiniband

run: $(TARGET)_run
	@./$(TARGET)

clean:
	$(RM) $(TARGET)
	$(RM) $(TARGET)_run
	$(RM) $(TARGET)_test
	$(RM) $(TARGET).c
	$(RM) $(TARGET).o
	+$(MAKE) clean -C infiniband
	+$(MAKE) clean -C $(COMMON_DIR)


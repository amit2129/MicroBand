# the compiler: gcc for C program, define as g++ for C++
CC = gcc
BUILD_DIR=/tmp/microband/
INFINIBAND_DIR=infiniband/

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:
TARGET = microband

all: build_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	cp $(INFINIBAND_DIR)* $(BUILD_DIR)
	cp $(TARGET).ino $(BUILD_DIR)$(TARGET).c

$(BUILD_DIR)$(TARGET)_test: $(BUILD_DIR)
	$(RM) $(BUILD_DIR)$(TARGET)_*
	$(CC) $(CFLAGS) -DDEBUG -o $(BUILD_DIR)$(TARGET)_test $(BUILD_DIR)*.c
	ln -sf $(BUILD_DIR)$(TARGET)_test microband

$(BUILD_DIR)$(TARGET)_run: $(BUILD_DIR)
	$(RM) $(BUILD_DIR)$(TARGET)_*
	$(CC) $(CFLAGS) -o $(BUILD_DIR)$(TARGET)_run $(BUILD_DIR)*.c
	ln -sf $(BUILD_DIR)$(TARGET)_run microband


build_test: $(BUILD_DIR)$(TARGET)_test

build_run: $(BUILD_DIR)$(TARGET)_run

test: build_test
	./$(TARGET)

run: build_run
	./$(TARGET)
clean:
	$(RM) $(TARGET)
	$(RM) -r $(BUILD_DIR)

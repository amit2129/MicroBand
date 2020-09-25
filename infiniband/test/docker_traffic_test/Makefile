# the compiler: gcc for C program, define as g++ for C++
CC = gcc
S_DIR=docker_server
C_DIR=docker_client
# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:

TARGET1=test_server
TARGET2=test_client

all: $(TARGET1) $(TARGET2)

test_sr: $(TARGET1)_docker $(TARGET2)_docker
	sudo docker run -d --rm --network=test_network --name=test_server docker_server
	sudo docker run -it --rm --network=test_network docker_client

test_cl: $(TARGET1)_docker $(TARGET2)_docker
	sudo docker run -itd --rm --network=test_network --name=test_server docker_server
	sudo docker run -it --rm --network=test_network docker_client

$(TARGET1):
	$(CC) ../../*.o -o $(TARGET1) $(TARGET1).c

$(TARGET2):
	$(CC) ../../*.o -o $(TARGET2) $(TARGET2).c

$(TARGET1)_docker: $(TARGET1)
	sudo docker build . -t $(S_DIR) -f server_dockerfile
	touch $(TARGET1)_docker

$(TARGET2)_docker: $(TARGET2)
	sudo docker build . -t $(C_DIR) -f client_dockerfile
	touch $(TARGET2)_docker

clean:
	$(RM) $(TARGET1)_docker
	$(RM) $(TARGET2)_docker
	$(RM) $(TARGET1)
	$(RM) $(TARGET2)

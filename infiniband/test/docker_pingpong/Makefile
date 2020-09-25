# the compiler: gcc for C program, define as g++ for C++
CC = gcc
SERV=docker_server
CLNT=docker_client
# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:

TARGET=pingpong

all: $(TARGET)

test_sr: $(TARGET)_dockers 
	sudo docker run -d --rm --network=test_network --name=test_server docker_server
	sudo docker run -it --rm --network=test_network docker_client

test_cl: $(TARGET)_dockers 
	sudo docker run -itd --rm --network=test_network --name=test_server docker_server
	sudo docker run -it --rm --network=test_network docker_client



test_all: test_sr test_cl


$(TARGET):
	$(CC) ../../*.o -o $(TARGET) $(TARGET).c

$(TARGET)_dockers: $(TARGET)
	sudo docker build . -t $(SERV) -f server_dockerfile
	sudo docker build . -t $(CLNT) -f client_dockerfile
	touch $(TARGET)_dockers


clean:
	$(RM) $(TARGET)_dockers
	$(RM) $(TARGET)

clean_cont:
	sudo docker kill $$(sudo docker ps -a -q --filter="name=test_server")

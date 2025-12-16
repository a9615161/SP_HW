# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g

all: isock_client isock_server

isock_client: main.o lookup8.o
	$(CC) $(CFLAGS) -o isock_client main.o lookup8.o

isock_server: isock_server.o lookup5.o
	$(CC) $(CFLAGS) -o isock_server isock_server.o lookup5.o

lookup8.o: lookup8.c dict.h
	$(CC) $(CFLAGS) -c lookup8.c

isock_server.o: isock_server.c dict.h
	$(CC) $(CFLAGS) -c isock_server.c
# Clean rule for convenience
clean:
	rm -f *.o usock_client usock_server
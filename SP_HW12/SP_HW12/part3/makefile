# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g

# Targets
# 根據作業要求，輸入 make 時要編譯出兩個程式 
all: udp_client udp_server

udp_client: main.o lookup9.o
	$(CC) $(CFLAGS) -o udp_client main.o lookup9.o

udp_server: udp_server.o lookup5.o
	$(CC) $(CFLAGS) -o udp_server udp_server.o lookup5.o

lookup9.o: lookup9.c dict.h
	$(CC) $(CFLAGS) -c lookup9.c

udp_server.o: udp_server.c dict.h
	$(CC) $(CFLAGS) -c udp_server.c
# Object file dependencies
# 所有 .c 檔都依賴 dict.h [cite: 1]
main.o: main.c dict.h
	$(CC) $(CFLAGS) -c main.c
lookup5.o: lookup5.c dict.h
	$(CC) $(CFLAGS) -c lookup5.c

# Clean rule for convenience
clean:
	rm -f *.o udp_client udp_server
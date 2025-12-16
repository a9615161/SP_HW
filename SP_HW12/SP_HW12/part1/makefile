# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g

# Targets
# 根據作業要求，輸入 make 時要編譯出兩個程式 
all: usock_client usock_server

# Client: 結合 main.o (介面) 與 lookup7.o (Unix Socket Client 實作) 
usock_client: main.o lookup7.o
	$(CC) $(CFLAGS) -o usock_client main.o lookup7.o

# Server: 結合 usock_server.o (Server 邏輯) 與 lookup5.o (本地搜尋實作) 
usock_server: usock_server.o lookup5.o
	$(CC) $(CFLAGS) -o usock_server usock_server.o lookup5.o

# Object file dependencies
# 所有 .c 檔都依賴 dict.h [cite: 1]
main.o: main.c dict.h
	$(CC) $(CFLAGS) -c main.c

lookup7.o: lookup7.c dict.h
	$(CC) $(CFLAGS) -c lookup7.c

usock_server.o: usock_server.c dict.h
	$(CC) $(CFLAGS) -c usock_server.c

lookup5.o: lookup5.c dict.h
	$(CC) $(CFLAGS) -c lookup5.c

# Clean rule for convenience
clean:
	rm -f *.o usock_client usock_server
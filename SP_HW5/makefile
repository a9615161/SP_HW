
CC = gcc

CFLAGS = -Wall -Wextra -g

TARGETS = pstree findHolder listPid test_process test_file

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<


# --- 清理規則 ---

# 移除所有編譯出來的檔案
clean:
	rm -f $(TARGETS) *.o


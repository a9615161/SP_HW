/*
 * 44-7. Write programs to verify the operation of nonblocking opens 
 * and nonblocking I/O on FIFOs.
 *
 * Usage: ./fifo_test [options]
 * Options:
 * -r          Act as a reader (default is writer if -r not specified)
 * -w          Act as a writer
 * -n          Use O_NONBLOCK for open()
 * -B          Clear O_NONBLOCK after open() (force blocking I/O)
 * -s <sec>    Sleep <sec> seconds between I/O operations (simulate slow I/O)
 * -f <name>   FIFO name (default: test_fifo)
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define DEFAULT_FIFO "test_fifo"
#define BUF_SIZE 1024

// 取得當前時間字串，用於觀察阻塞情況
char* currTime() {
    static char buf[100];
    time_t t;
    struct tm *tm;
    
    t = time(NULL);
    tm = localtime(&t);
    strftime(buf, sizeof(buf), "%T", tm);
    return buf;
}

void usageExit(const char *progName) {
    fprintf(stderr, "Usage: %s [-r|-w] [-n] [-B] [-s seconds] [-f fifo_name]\n", progName);
    fprintf(stderr, "  -r: Reader mode\n");
    fprintf(stderr, "  -w: Writer mode\n");
    fprintf(stderr, "  -n: Use O_NONBLOCK for open()\n");
    fprintf(stderr, "  -B: Switch to Blocking I/O after open (via fcntl)\n");
    fprintf(stderr, "  -s: Sleep seconds between I/O\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int opt;
    int isReader = 0;
    int isWriter = 0;
    int useNonBlock = 0;
    int switchToBlocking = 0;
    int sleepTime = 0;
    char *fifoName = DEFAULT_FIFO;
    int fd, flags;
    char buffer[BUF_SIZE];
    ssize_t numTransferred;

    // 解析參數
    while ((opt = getopt(argc, argv, "rwnBs:f:")) != -1) {
        switch (opt) {
        case 'r': isReader = 1; break;
        case 'w': isWriter = 1; break;
        case 'n': useNonBlock = 1; break;
        case 'B': switchToBlocking = 1; break;
        case 's': sleepTime = atoi(optarg); break;
        case 'f': fifoName = optarg; break;
        default: usageExit(argv[0]);
        }
    }

    if (isReader && isWriter) {
        fprintf(stderr, "Error: Cannot be both reader and writer\n");
        exit(EXIT_FAILURE);
    }
    if (!isReader && !isWriter) {
        usageExit(argv[0]);
    }

    // 忽略 SIGPIPE，避免 Writer 寫入關閉的 Pipe 時直接 Crash
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    // 1. 建立 FIFO
    if (mkfifo(fifoName, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    printf("[%s] Opening FIFO '%s' in %s mode...\n", currTime(), fifoName, 
           useNonBlock ? "NON-BLOCKING" : "BLOCKING");

    // 2. 開啟 FIFO
    flags = (isReader ? O_RDONLY : O_WRONLY);
    if (useNonBlock) flags |= O_NONBLOCK;

    fd = open(fifoName, flags);
    
    // 檢查 open 是否成功
    if (fd == -1) {
        printf("[%s] Open FAILED: %s (errno=%d)\n", currTime(), strerror(errno), errno);
        if (errno == ENXIO) {
            printf(">> Verification: O_NONBLOCK writer failed because no reader exists.\n");
        }
        exit(EXIT_FAILURE);
    }

    printf("[%s] Open SUCCEEDED.\n", currTime());

    // 3. (選擇性) 切換回阻塞 I/O
    if (switchToBlocking) {
        printf("[%s] Switching file descriptor to BLOCKING mode via fcntl...\n", currTime());
        int currentFlags = fcntl(fd, F_GETFL);
        if (fcntl(fd, F_SETFL, currentFlags & ~O_NONBLOCK) == -1) {
            perror("fcntl");
            exit(EXIT_FAILURE);
        }
    }

    // 4. I/O 迴圈
    if (isReader) {
        printf("--- Starting READ Loop ---\n");
        while (1) {
            if (sleepTime > 0) sleep(sleepTime);

            numTransferred = read(fd, buffer, BUF_SIZE);

            if (numTransferred == -1) {
                if (errno == EAGAIN) {
                    printf("[%s] read() returned EAGAIN (No data, writer open)\n", currTime());
                    sleep(1); // 避免洗版
                } else {
                    perror("read");
                    break;
                }
            } else if (numTransferred == 0) {
                printf("[%s] read() returned 0 (EOF - Writer closed)\n", currTime());
                break;
            } else {
                printf("[%s] Read %ld bytes\n", currTime(), (long)numTransferred);
            }
        }
    } else {
        printf("--- Starting WRITE Loop ---\n");
        memset(buffer, 'A', BUF_SIZE); // 填入假資料
        while (1) {
            if (sleepTime > 0) sleep(sleepTime);

            numTransferred = write(fd, buffer, BUF_SIZE);

            if (numTransferred == -1) {
                if (errno == EAGAIN) {
                    printf("[%s] write() returned EAGAIN (Pipe full)\n", currTime());
                    sleep(1); // 避免洗版
                } else if (errno == EPIPE) {
                    printf("[%s] write() returned EPIPE (Reader closed)\n", currTime());
                    break;
                } else {
                    perror("write");
                    break;
                }
            } else {
                printf("[%s] Wrote %ld bytes\n", currTime(), (long)numTransferred);
            }
        }
    }

    close(fd);
    return 0;
}
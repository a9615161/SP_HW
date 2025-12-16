#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>

#define DEFAULT_BUFSIZE 4096

int main(int argc, char *argv[]) {
    int nlines = 10;                  // 預設輸出最後 10 行
    size_t bufsize = DEFAULT_BUFSIZE; // 預設 buffer 大小
    int opt;

    // 解析選項
    while ((opt = getopt(argc, argv, "n:b:")) != -1) {
        switch (opt) {
            case 'n':
                nlines = atoi(optarg);
                break;
            case 'b':
                bufsize = (size_t)atol(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-n lines] [-b bufsize] file\n", argv[0]);
                exit(1);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected file argument\n");
        exit(1);
    }

    const char *filename = argv[optind];

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    off_t filesize = lseek(fd, 0, SEEK_END);
    if (filesize == -1) {
        perror("lseek");
        close(fd);
        exit(1);
    }

    char *buf = malloc(bufsize);
    if (!buf) {
        perror("malloc");
        close(fd);
        exit(1);
    }

    off_t pos = filesize;
    int newlines = 0;
    off_t start = 0;

    // 從檔案尾巴往前找 nlines+1 個 \n
    while (pos > 0) {
        size_t toread = (pos >= (off_t)bufsize) ? bufsize : pos;
        pos -= toread;

        if (lseek(fd, pos, SEEK_SET) == -1) {
            perror("lseek");
            free(buf);
            close(fd);
            exit(1);
        }

        ssize_t bytes = read(fd, buf, toread);
        if (bytes < 0) {
            perror("read");
            free(buf);
            close(fd);
            exit(1);
        }

        for (ssize_t i = bytes - 1; i >= 0; i--) {
            if (buf[i] == '\n') {
                newlines++;
                if (newlines == nlines + 1) {
                    start = pos + i + 1;
                    pos = 0; // 強迫跳出 while
                    break;
                }
            }
        }
    }

    // 從找到的位置開始輸出
    if (lseek(fd, start, SEEK_SET) == -1) {
        perror("lseek");
        free(buf);
        close(fd);
        exit(1);
    }

    ssize_t bytes;
    while ((bytes = read(fd, buf, bufsize)) > 0) {
        if (write(STDOUT_FILENO, buf, bytes) != bytes) {
            perror("write");
            free(buf);
            close(fd);
            exit(1);
        }
    }

    free(buf);
    close(fd);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DEFAULT_INIT_BUF 4096
#define DEFAULT_READ_SIZE 4096
#define DEFAULT_N 10
#define MAX_READ_BLOCKS 4096

int main(int argc, char *argv[]) {
    size_t buf_size = DEFAULT_INIT_BUF;
    int n_lines = DEFAULT_N;
    char *filename = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "b:n:")) != -1) {
        switch (opt) {
            case 'b':
                buf_size = atoi(optarg);
                if (buf_size == 0) buf_size = DEFAULT_INIT_BUF;
                break;
            case 'n':
                n_lines = atoi(optarg);
                if (n_lines <= 0) n_lines = DEFAULT_N;
                break;
            default:
                fprintf(stderr, "Usage: %s [-b buffer_size] [-n num_lines] <filename>\n", argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Usage: %s [-b buffer_size] [-n num_lines] <filename>\n", argv[0]);
        return 1;
    }

    filename = argv[optind];

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    char *buffer = malloc(buf_size);
    if (!buffer) {
        perror("malloc");
        close(fd);
        return 1;
    }

    size_t data_len = 0;
    int newline_count = 0;
    off_t offset = file_size;
    ssize_t nread;

    // 紀錄每次 read 的開始位置
    size_t read_starts[MAX_READ_BLOCKS];
    int read_count = 0;

    while (offset > 0) {
        size_t try_read = DEFAULT_READ_SIZE;
        if (try_read > offset) try_read = offset;

        offset -= try_read;

        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("lseek");
            free(buffer);
            close(fd);
            return 1;
        }

        // 動態擴充 buffer
        while (data_len + try_read > buf_size) {
            buf_size *= 2;
            char *newbuf = realloc(buffer, buf_size);
            if (!newbuf) {
                perror("realloc");
                free(buffer);
                close(fd);
                return 1;
            }
            buffer = newbuf;
        }

        read_starts[read_count++] = data_len;

        nread = read(fd, buffer + data_len, try_read);
        if (nread <= 0) break;
        data_len += nread;

        // 從這次 read 往前掃描換行
        for (ssize_t i = nread - 1; i >= 0; i--) {
            if (buffer[data_len - nread + i] == '\n') {
                newline_count++;
                if (newline_count == n_lines + 1) {
                    // 找到倒數第 n+1 個換行
                    size_t write_start = data_len - nread + i + 1;

                    // 從每個 read 的 start 寫到結尾
                    for (int j = 0; j < read_count; j++) {
                        size_t rs = read_starts[j];
                        size_t re = (j == read_count - 1) ? data_len : read_starts[j+1];
                        if (re <= write_start) continue; // 這塊不用寫
                        size_t ws = (rs < write_start) ? write_start : rs;
                        write(STDOUT_FILENO, buffer + ws, re - ws);
                    }

                    free(buffer);
                    close(fd);
                    return 0;
                }
            }
        }
    }

    // 如果行數不足 n，直接輸出整個 buffer
    write(STDOUT_FILENO, buffer, data_len);
    free(buffer);
    close(fd);
    return 0;
}

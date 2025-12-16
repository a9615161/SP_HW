#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define HOLE_SIZE 10240   // 10KB hole
#define DATA_SIZE 1024    // 1KB data

void write_data(int fd, const char *pattern, size_t len) {
    write(fd, pattern, len);
}

int main(void) {
    char buf[DATA_SIZE];
    for (int i = 0; i < DATA_SIZE; i++) buf[i] = 'A' + (i % 26);

    // 1. no_hole.bin (只有資料)
    int fd = open("no_hole.bin", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    write_data(fd, buf, DATA_SIZE);
    close(fd);

    // 2. hole_head.bin (前面是 10KB hole，後面是 1KB data)
    fd = open("hole_head.bin", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    lseek(fd, HOLE_SIZE, SEEK_SET);
    write_data(fd, buf, DATA_SIZE);
    close(fd);

    // 3. hole_tail.bin (先寫 1KB data，後面是 10KB hole)
    fd = open("hole_tail.bin", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    write_data(fd, buf, DATA_SIZE);
    lseek(fd, HOLE_SIZE, SEEK_CUR);
    close(fd);

    // 4. multi_hole.bin (data → hole → data → hole → data)
    fd = open("multi_hole.bin", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    write_data(fd, buf, DATA_SIZE);
    lseek(fd, HOLE_SIZE, SEEK_CUR);
    write_data(fd, buf, DATA_SIZE);
    lseek(fd, HOLE_SIZE, SEEK_CUR);
    write_data(fd, buf, DATA_SIZE);
    close(fd);

    //printf("Test files generated with 10KB holes.\n");
    return 0;
}

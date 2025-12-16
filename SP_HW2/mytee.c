#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    int opt;
    int append = 0;
    int fd;
    char buf[BUF_SIZE];
    ssize_t numRead;

    while ((opt = getopt(argc, argv, "a")) != -1) {
        if (opt == 'a')
            append = 1;
    }

    int flags = O_WRONLY | O_CREAT;
    if (append)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;

    fd = open(argv[optind], flags, 0644);

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        write(STDOUT_FILENO, buf, numRead);
        write(fd, buf, numRead);
    }

    close(fd);
    return 0;
}

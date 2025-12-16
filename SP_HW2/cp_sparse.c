#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef SEEK_DATA
#define SEEK_DATA 3
#endif

#ifndef SEEK_HOLE
#define SEEK_HOLE 4
#endif

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif

#define min(a,b) ((a) > (b) ? (b) : (a))

int main(int argc, char *argv[])
{
    int inFd, outFd;
    char buf[BUF_SIZE];
    ssize_t numRead = 0;
    off_t off = 0;

    inFd = open(argv[1], O_RDONLY);
    outFd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0666);

    do {
        off_t dataOff = lseek(inFd, off, SEEK_DATA);
        if (dataOff == -1)           // assume this is EOF
            break;

        off_t holeOff = lseek(inFd, dataOff, SEEK_HOLE);

        lseek(inFd, dataOff, SEEK_SET);
        lseek(outFd, dataOff, SEEK_SET);   // output also seek to  dataOff

        while (dataOff < holeOff) {
            size_t Rlen = holeOff - dataOff;
            size_t toRead = min(Rlen, BUF_SIZE);
            numRead = read(inFd, buf, toRead);
            if (numRead <= 0)        // EOF
                break;

            write(outFd, buf, numRead);
            dataOff += numRead;
        }

        off = holeOff;               
    } while (numRead != 0);
    //no data ,truncate to the end
    off_t end = lseek(inFd, 0, SEEK_END);
    ftruncate(outFd, end);

    close(inFd);
    close(outFd);
    return 0;
}

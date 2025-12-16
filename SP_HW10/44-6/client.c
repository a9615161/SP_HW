/* client.c */
#include "fifo_seqnum.h"

static char clientFifo[CLIENT_FIFO_NAME_LEN];

static void removeFifo(void) {
    unlink(clientFifo);
}

int main(int argc, char *argv[])
{
    int serverFd, clientFd;
    struct request req;
    struct response resp;
    /* 移除 flags 變數，不需要 fcntl 操作 */

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        printf("%s [seq-len...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    umask(0);
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) getpid());

    if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
        errExit("mkfifo");

    if (atexit(removeFifo) != 0) errExit("atexit");

    /* 【修正點】: 使用 O_RDWR 開啟 Client FIFO */
    /* 1. O_RDWR 開啟時不會阻塞 (解決 Deadlock 問題) */
    /* 2. 因為自己持有寫入端，read() 時若 Server 尚未連上，會進入阻塞等待資料，而不是回傳 EOF (解決 Race Condition) */
    clientFd = open(clientFifo, O_RDWR);
    if (clientFd == -1) errExit("open client fifo");

    /* 建構請求 */
    req.pid = getpid();
    req.seqLen = (argc > 1) ? atoi(argv[1]) : 1;

    /* 開啟 Server FIFO 並發送請求 */
    serverFd = open(SERVER_FIFO, O_WRONLY);
    if (serverFd == -1) errExit("open server fifo");

    if (write(serverFd, &req, sizeof(struct request)) != sizeof(struct request))
        errExit("Can't write to server");

    /* 讀取回應 */
    /* 這裡不再需要 fcntl 切換旗標，因為 open 時沒有設 O_NONBLOCK */
    ssize_t numRead = read(clientFd, &resp, sizeof(struct response));
    
    if (numRead != sizeof(struct response)) {
        fprintf(stderr, "Could not read response from server. numRead=%ld\n", (long)numRead);
        exit(EXIT_FAILURE);
    }

    printf("Assigned sequence number: %d\n", resp.seqNum);

    exit(EXIT_SUCCESS);
}
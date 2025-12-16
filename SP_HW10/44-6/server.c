/* server.c */
#include <signal.h>
#include "fifo_seqnum.h"

int main(int argc, char *argv[])
{
    int serverFd, dummyFd, clientFd;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    struct request req;
    struct response resp;
    int seqNum = 0;

    umask(0);

    /* 建立並開啟 Server FIFO */
    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
        errExit("mkfifo");

    serverFd = open(SERVER_FIFO, O_RDONLY);
    if (serverFd == -1) errExit("open");

    /* 開啟一個 dummy write fd，確保伺服器不會讀到 EOF */
    dummyFd = open(SERVER_FIFO, O_WRONLY);
    if (dummyFd == -1) errExit("open");

    /* 忽略 SIGPIPE，防止寫入關閉的 pipe 時導致 crash */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) errExit("signal");

    for (;;) {
        /* 讀取請求 */
        if (read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
            fprintf(stderr, "Error reading request; discarding\n");
            continue;
        }

        /* 建構 Client FIFO 名稱 */
        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) req.pid);

        /* 【關鍵修改】: 使用 O_NONBLOCK 開啟 Client FIFO */
        /* 根據 Table 44-1 [cite: 949]，若無讀取者，此操作將立即失敗並返回 -1 (ENXIO) */
        clientFd = open(clientFifo, O_WRONLY | O_NONBLOCK);
        if (clientFd == -1) {
            if (errno == ENXIO) {
                printf("Client %ld not open for reading. Request ignored (Anti-DoS).\n", (long)req.pid);
            } else {
                perror("open client fifo");
            }
            continue; /* 放棄此請求，繼續處理下一個 */
        }

        /* 傳送回應 */
        resp.seqNum = seqNum;
        if (write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
            fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);

        /* 關閉連線 */
        if (close(clientFd) == -1) perror("close");

        seqNum += req.seqLen;
    }
}
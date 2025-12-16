#include "seq.h"

int main(int argc, char *argv[]) {
    int msqid;
    struct reqMsg req;
    struct respMsg resp;
    int reqLen = 1; // 預設請求長度

    if (argc > 1) {
        reqLen = atoi(argv[1]);
    }

    /* 1. 取得現有的 Message Queue ID [cite: 17] 
       不需要 IPC_CREAT，因為假設 Server 已經建立了 */
    msqid = msgget(SEQ_KEY, 0);
    if (msqid == -1) {
        fprintf(stderr, "Error: Could not access queue. Is the server running?\n");
        exit(EXIT_FAILURE);
    }

    /* 2. 準備請求訊息 */
    req.mtype = 1;              // 發送給 Server 的訊息 type 必須是 1 
    req.clientId = getpid();    // 放入自己的 PID [cite: 627]
    req.seqLen = reqLen;

    /* 3. 發送請求 [cite: 140] */
    if (msgsnd(msqid, &req, REQ_MSG_SIZE, 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    /* 4. 等待回應 [cite: 222]
       type = getpid(): 只接收 mtype 等於自己 PID 的訊息 
       這樣可以確保多個 Client 同時操作時不會拿到別人的回應 */
    if (msgrcv(msqid, &resp, RESP_MSG_SIZE, getpid(), 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    printf("Client PID %d: Request Len %d -> Assigned Start Seq %d\n", 
           getpid(), reqLen, resp.startSeq);

    return 0;
}
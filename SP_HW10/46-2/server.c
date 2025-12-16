#include "seq.h"

int main(int argc, char *argv[]) {
    int msqid;
    struct reqMsg req;
    struct respMsg resp;
    int seqNum = 0; // 當前的序列號計數器

    /* 1. 建立 Message Queue [cite: 17, 27] 
       IPC_CREAT: 如果不存在則建立
       0666: 讀寫權限 */
    msqid = msgget(SEQ_KEY, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Queue ID: %d. Waiting for requests...\n", msqid);

    /* 2. 無窮迴圈處理請求 */
    for (;;) {
        /* 接收請求 [cite: 222]
           type = 1: 根據慣例，Server 讀取 type 為 1 的訊息  */
        ssize_t msgLen = msgrcv(msqid, &req, REQ_MSG_SIZE, 1, 0);
        if (msgLen == -1) {
            perror("msgrcv");
            break;
        }

        printf("Received request from Client PID %d for length %d\n", 
               req.clientId, req.seqLen);

        /* 3. 處理業務邏輯 (分配序列號) */
        resp.mtype = req.clientId;  // 設定回傳的 type 為 Client 的 PID 
        resp.startSeq = seqNum;
        seqNum += req.seqLen;       // 更新 Server 端的計數器

        /* 4. 發送回應回同一個 Queue [cite: 140] */
        if (msgsnd(msqid, &resp, RESP_MSG_SIZE, 0) == -1) {
            perror("msgsnd");
            break;
        }
        printf("Sent response to Client PID %ld: Start Seq %d\n", 
               (long)resp.mtype, resp.startSeq);
    }

    return 0;
}
#ifndef SEQ_H
#define SEQ_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h> /* For offsetof */

/* 定義一個固定的 Key，讓 Server 和 Client 都能找到同一個 Queue */
#define SEQ_KEY 0x1AAAAAA1

/* 請求訊息結構 (Client -> Server) */
struct reqMsg {
    long mtype;          /* 必須是 1  */
    pid_t clientId;      /* Client 的 PID，讓 Server 知道回傳給誰 [cite: 627] */
    int seqLen;          /* 請求的序列號長度 */
};

/* 回應訊息結構 (Server -> Client) */
struct respMsg {
    long mtype;          /* 必須是 Client 的 PID  */
    int startSeq;        /* 分配到的起始序列號 */
};

/* 計算訊息內容(mtext)的大小，不包含 mtype */
#define REQ_MSG_SIZE (sizeof(struct reqMsg) - sizeof(long))
#define RESP_MSG_SIZE (sizeof(struct respMsg) - sizeof(long))

#endif
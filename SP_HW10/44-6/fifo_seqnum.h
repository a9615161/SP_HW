/* fifo_seqnum.h */
#ifndef FIFO_SEQNUM_H
#define FIFO_SEQNUM_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* 定義 Server 與 Client 的 FIFO 路徑 */
#define SERVER_FIFO "/tmp/seqnum_sv"
#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)

/* * 錯誤處理巨集修正 
 * 使用 __VA_ARGS__ 支援格式化字串 (如 printf)
 */

// errExit: 印出錯誤訊息與系統錯誤碼(errno)，然後退出
#define errExit(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, ": %s\n", strerror(errno)); \
    exit(EXIT_FAILURE); \
} while (0)

// errMsg: 印出錯誤訊息與系統錯誤碼(errno)，但不退出
#define errMsg(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, ": %s\n", strerror(errno)); \
} while (0)

// usageErr: 印出用法錯誤訊息並退出
#define usageErr(...) do { \
    fprintf(stderr, "Usage: "); \
    fprintf(stderr, __VA_ARGS__); \
    exit(EXIT_FAILURE); \
} while (0)

// fatal: 印出嚴重錯誤訊息並退出 (無 errno)
#define fatal(msg) do { \
    fprintf(stderr, "%s\n", msg); \
    exit(EXIT_FAILURE); \
} while (0)

/* Request 結構 (Client --> Server) */
struct request {
    pid_t pid;      /* PID of client */
    int seqLen;     /* Length of desired sequence */
};

/* Response 結構 (Server --> Client) */
struct response {
    int seqNum;     /* Start of sequence */
};

#endif
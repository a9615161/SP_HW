#ifndef SHM_STRUCTURE_H
#define SHM_STRUCTURE_H

#include <pthread.h>
#include <unistd.h>
#include <signal.h>

// --- 可調整參數 ---
#define MAX_BUFFER_SIZE 10 // 緩衝區大小 B
#define NUM_CONSUMERS   3  // Consumer 數量 C
#define MESSAGE_SIZE    64 // 每個訊息的位元組大小

// --- 共享記憶體結構 ---
typedef struct {
    char data[MESSAGE_SIZE]; // 訊息內容
    int sequence_number;     // 訊息的序號
} Message;

typedef struct {
    // 共享資料區塊
    Message buffer[MAX_BUFFER_SIZE]; 
    
    // 狀態變數
    long current_seq; // Producer 生產的最新序號
    int write_index;  // Producer 寫入緩衝區的索引
    long consumer_read_cursor[NUM_CONSUMERS]; // 每個 Consumer 已讀取的最新序號
    
    // 同步機制
    pthread_mutex_t mutex; // 用於保護共享變數
    
    // Consumer Group ID
    pid_t consumer_group_id; 

} SharedMemory;

#endif // SHM_STRUCTURE_H
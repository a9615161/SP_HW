#define _POSIX_C_SOURCE 199309L // 確保 POSIX 函式（如 sigaction, usleep）的宣告可見
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // <--- 必須包含，提供 usleep() 和 setpgid()
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>     // <--- 必須包含，提供 struct sigaction, sigaction(), sigemptyset()
#include "shm_structure.h"
// ... 程式碼其餘部分不變 ...
#define SHM_NAME "/producer_consumer_shm"

SharedMemory *shm_ptr_global;
int consumer_id;
long total_received_messages = 0;
int read_delay_ms = 50; // 實驗用的 Consumer 讀取延遲

// 信號處理函式：Consumer 被動收到通知
void signal_handler(int sig) {
    long latest_seq, last_read_seq;
    
    pthread_mutex_lock(&shm_ptr_global->mutex);
    
    latest_seq = shm_ptr_global->current_seq;
    last_read_seq = shm_ptr_global->consumer_read_cursor[consumer_id];
    
    // 如果 Producer 序號沒有更新，可能是重複信號或已被其他 Consumer 處理
    if (latest_seq <= last_read_seq) {
        pthread_mutex_unlock(&shm_ptr_global->mutex);
        return; 
    }

    // 讀取從上次序號+1 到 最新序號 的所有資料
    long start_seq = last_read_seq + 1;
    
    // 計算 Loss Rate: 如果 start_seq 跳號了，則代表有資料在 buffer 被覆蓋而遺失。
    long lost_count = latest_seq - start_seq;
    if (lost_count > MAX_BUFFER_SIZE) {
        // 最多只能讀取緩衝區大小的資料，其餘為 Loss
        lost_count = lost_count - MAX_BUFFER_SIZE;
        // 實際開始讀取的序號，必須考慮 Buffer Overrun
        start_seq = latest_seq - MAX_BUFFER_SIZE + 1;
        if (start_seq < 1) start_seq = 1;
    }
    
    for (long seq = start_seq; seq <= latest_seq; ++seq) {
        int index = seq % MAX_BUFFER_SIZE;
        
        // 驗證讀取的資料序號是否正確 (雖然 Group Signal 無法傳遞序號，但共享記憶體中有)
        if (shm_ptr_global->buffer[index].sequence_number == seq) {
            // printf("C%d read seq %ld: %s\n", consumer_id, seq, shm_ptr_global->buffer[index].data);
            total_received_messages++;
        } else {
             // 這個錯誤通常表示 Buffer Overrun 發生，且讀到了舊資料
             // printf("C%d potential loss/old data at seq %ld, buffer seq %d\n", consumer_id, seq, shm_ptr_global->buffer[index].sequence_number);
        }
    }
    
    // 更新已讀取的最新序號
    shm_ptr_global->consumer_read_cursor[consumer_id] = latest_seq;
    
    pthread_mutex_unlock(&shm_ptr_global->mutex);

    // 模擬 Consumer 處理資料的成本 (如原報告所述)
    if (read_delay_ms > 0) {
        usleep(read_delay_ms * 1000);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <consumer_index_0_to_%d>\n", argv[0], NUM_CONSUMERS - 1);
        return 1;
    }
    consumer_id = atoi(argv[1]);
    if (consumer_id < 0 || consumer_id >= NUM_CONSUMERS) {
        fprintf(stderr, "Consumer index out of range.\n");
        return 1;
    }
    
    int shm_fd;
    
    // 1. 開啟共享記憶體
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open failed"); return 1; }
    
    // 2. 映射共享記憶體
    shm_ptr_global = mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr_global == MAP_FAILED) { perror("mmap failed"); return 1; }
    close(shm_fd);

    // 3. 設定 Consumer 群組 ID (僅第一個 Consumer 設定即可)
    pid_t pgid = getpgrp();
    if (shm_ptr_global->consumer_group_id == 0) {
        shm_ptr_global->consumer_group_id = pgid;
    }
    // 確保所有 Consumer 加入同一個群組
    if (setpgid(0, shm_ptr_global->consumer_group_id) == -1) {
        perror("setpgid failed");
    }

    // 4. 設定信號處理
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler; // 使用簡單的處理函式
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction failed");
        return 1;
    }

    printf("Consumer %d (PID: %d, PGID: %d) is running...\n", consumer_id, getpid(), getpgrp());

    // 5. 進入等待狀態，等待 Producer 發送信號
    while (shm_ptr_global->current_seq < 1000) { 
        // 這裡可以讓 Consumer 進入 sleep 或 pause/sigsuspend，以節省 CPU
        pause(); 
    }
    
    // 等待所有 Producer 流程結束後的結尾處理
    sleep(1);
    
    printf("\nConsumer %d finished. Total received: %ld\n", consumer_id, total_received_messages);

    munmap(shm_ptr_global, sizeof(SharedMemory));
    return 0;
}
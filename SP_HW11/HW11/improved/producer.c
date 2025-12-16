#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // for ftruncate, usleep
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>     // for kill
#include "shm_structure.h"
#define SHM_NAME "/producer_consumer_shm"

void cleanup(SharedMemory *shm_ptr, int shm_fd) {
    if (shm_ptr != MAP_FAILED) {
        pthread_mutex_destroy(&shm_ptr->mutex);
        munmap(shm_ptr, sizeof(SharedMemory));
    }
    close(shm_fd);
    shm_unlink(SHM_NAME);
    printf("Producer: Shared Memory cleaned up.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <message_interval_ms>\n", argv[0]);
        return 1;
    }
    int interval_ms = atoi(argv[1]);
    int shm_fd;
    SharedMemory *shm_ptr;

    // 1. 建立並配置共享記憶體
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open failed"); return 1; }
    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) { perror("ftruncate failed"); return 1; }
    
    // 2. 映射共享記憶體
    shm_ptr = mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) { perror("mmap failed"); return 1; }

    // 3. 初始化共享記憶體內容
    memset(shm_ptr, 0, sizeof(SharedMemory));
    // 初始化 Mutex
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm_ptr->mutex, &attr);
    
    printf("Producer PID: %d. Waiting for Consumers to set up...\n", getpid());
    // 等待 Consumer 建立群組 ID (PGID)
    sleep(2); 
    
    if (shm_ptr->consumer_group_id <= 0) {
        fprintf(stderr, "Consumer Group ID not set. Please run consumers first.\n");
        cleanup(shm_ptr, shm_fd);
        return 1;
    }

    printf("Producer starts. Sending to PGID: %d\n", shm_ptr->consumer_group_id);
    
    long total_messages = 1000;
    for (long i = 1; i <= total_messages; ++i) {
        
        pthread_mutex_lock(&shm_ptr->mutex);
        
        // 4. 更新狀態
        shm_ptr->current_seq = i;
        shm_ptr->write_index = i % MAX_BUFFER_SIZE;
        
        // 5. 寫入資料
        snprintf(shm_ptr->buffer[shm_ptr->write_index].data, MESSAGE_SIZE, "Message %ld", i);
        shm_ptr->buffer[shm_ptr->write_index].sequence_number = i;
        
        pthread_mutex_unlock(&shm_ptr->mutex);
        
        // 6. 發送 Group Signal
        // kill(PGID, SIGNAL)
        // 注意：Group Signal 不包含資料，Consumer 需自行讀取狀態變數
        if (kill(-shm_ptr->consumer_group_id, SIGUSR1) == -1) { 
             perror("kill group signal failed");
        }
        
        // 7. 間隔延遲
        if (interval_ms > 0) {
            usleep(interval_ms * 1000);
        }
    }
    
    printf("Producer finished sending %ld messages.\n", total_messages);
    
    // 清理資源
    // 注意：實際應用中，通常會讓 producer 保持執行或發送終止信號。
    // 這裡為簡單範例，直接清理並結束。
    cleanup(shm_ptr, shm_fd);

    return 0;
}
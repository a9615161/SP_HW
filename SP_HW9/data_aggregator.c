#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

// 定義資料結構
typedef struct {
    int pid;
    int data;
} DataItem;

// 共享記憶體頭部
typedef struct {
    int top;      // 目前 Stack 的索引 (Write Index)
    int m_size;   // Stack 的總容量 (M)
    int n_per_prod; // 每個 Producer 要寫入的次數 (N)
} SharedHeader;

// System V Semaphore 索引定義
#define SEM_MUTEX 0
#define SEM_FULL  1

// 定義 semun (部分系統需要自行定義)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// 全域變數
int shm_id;
int sem_id;
SharedHeader *shared_hdr;
DataItem *shared_stack; 

// =============================================================
// 修正重點：移除了 SEM_UNDO
// =============================================================
void sem_p(int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = -1;
    sb.sem_flg = 0; // <--- 移除 SEM_UNDO，確保 Producer 退出後信號保留
    if (semop(sem_id, &sb, 1) == -1) {
        perror("sem_p");
        exit(1);
    }
}

void sem_v(int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = 1;
    sb.sem_flg = 0; // <--- 移除 SEM_UNDO
    if (semop(sem_id, &sb, 1) == -1) {
        perror("sem_v");
        exit(1);
    }
}

void producer(int n_writes) {
    srand(getpid() + time(NULL)); 
    int pid = getpid();
    int ppid = getppid();
    printf("# Producer P%d (PID %d) starts writing...\n", pid - ppid  , pid);
    for (int i = 0; i < n_writes; ++i) {
        int random_num = (rand() % 10) + 1;

        while (1) {
            // 1. 取得 Mutex
            sem_p(SEM_MUTEX);

            // 2. 檢查 Stack 是否有空間
            if (shared_hdr->top < shared_hdr->m_size) {
                // === CRITICAL SECTION (Write) ===
                int idx = shared_hdr->top;
                shared_stack[idx].pid = pid;
                shared_stack[idx].data = random_num;
                shared_hdr->top++; // 更新 Write Index
                
                printf("# P%d writes (%d, %d),increments Sem 1.\n", 
                       pid-ppid, pid, random_num);
                
                // 3. 釋放 Mutex
                sem_v(SEM_MUTEX);
                
                // 4. 通知 Consumer (Full + 1)
                // 注意：這裡已經移除了 SEM_UNDO，所以即使 Producer exit，計數也會保留
                sem_v(SEM_FULL);
                
                break; // 寫入成功，跳出 while，準備下一筆
            } else {
                // === Stack Full ===
                // 釋放 Mutex
                sem_v(SEM_MUTEX);
                
                // Busy Waiting with sleep
                usleep(1000); 
            }
        }
    }
    exit(0);
}

void consumer(int total_items) {
    int processed_count = 0;
    long long total_sum = 0;
    printf("# Consumer (PID %d) waits on Semaphore 1 (Full Count)\n", getpid());
    while (processed_count < total_items) {
        // 1. 等待資料 (Wait Full)
        sem_p(SEM_FULL);

        // 2. 取得 Mutex
        sem_p(SEM_MUTEX);

        // === CRITICAL SECTION (Read) ===
        if (shared_hdr->top > 0) {
            shared_hdr->top--; // Pop
            int idx = shared_hdr->top;
            int data = shared_stack[idx].data;
            int pid = shared_stack[idx].pid;
            
            total_sum += data;
            processed_count++;

            printf("# Consumer reads (%d, %d), Sum = %lld\n", 
                   pid, data, total_sum);
        } else {
            // 這裡通常不應該發生，除非邏輯有誤
             // 若發生，為了避免死鎖，還是要釋放 mutex
            printf("Warning: Stack underflow detected but Semaphore signaled.\n");
        }

        // 3. 釋放 Mutex
        sem_v(SEM_MUTEX);
    }
    printf("# Output:\n");
    printf("Total Data Points Processed: %d\n", processed_count);
    printf("Final Cumulative Sum: %lld\n", total_sum);
    exit(0);
}

int main(int argc, char *argv[]) {
    int M = 0; 
    int N = 0; 

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <M_producers> <N_writes>\n", argv[0]);
        exit(1);
    }

    M = atoi(argv[1]);
    N = atoi(argv[2]);

    if (M <= 0 || N <= 0) {
        fprintf(stderr, "M and N must be positive integers.\n");
        exit(1);
    }

    // === 1. System V Shared Memory ===
    size_t shm_size = sizeof(SharedHeader) + (M * sizeof(DataItem));
    
    shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    void *shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    shared_hdr = (SharedHeader *)shm_ptr;
    shared_stack = (DataItem *)(shm_ptr + sizeof(SharedHeader));

    shared_hdr->top = 0; 
    shared_hdr->m_size = M;
    shared_hdr->n_per_prod = N;

    // === 2. System V Semaphores ===
    sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget");
        exit(1);
    }

    union semun arg;
    
    // Mutex = 1
    arg.val = 1;
    if (semctl(sem_id, SEM_MUTEX, SETVAL, arg) == -1) {
        perror("semctl mutex");
        exit(1);
    }

    // Full Count = 0
    arg.val = 0;
    if (semctl(sem_id, SEM_FULL, SETVAL, arg) == -1) {
        perror("semctl full");
        exit(1);
    }

    // === 3. Fork Producers ===
    for (int i = 0; i < M; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            producer(N); 
        } else if (pid < 0) {
            perror("fork producer");
            exit(1);
        }
    }

    // === 4. Fork Consumer ===
    pid_t cons_pid = fork();
    if (cons_pid == 0) {
        consumer(M * N); 
    } else if (cons_pid < 0) {
        perror("fork consumer");
        exit(1);
    }

    // === 5. Parent Clean up ===
    // 等待 M 個 Producer + 1 個 Consumer
    for (int i = 0; i < M + 1; ++i) {
        wait(NULL);
    }

    // 移除資源
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    return 0;
}
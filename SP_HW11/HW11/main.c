#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

// Shared Memory 名稱
#define SHM_NAME "/sp_hw11_shm_nano"

// 定義封包結構 (80 bytes) [cite: 36]
typedef struct {
    char msg[80];
} Packet;

// 全域變數 (供 Signal Handler 使用)
volatile sig_atomic_t wake_up = 0;
volatile int signal_seq_num = -1;

// 封裝 nanosleep：堅持睡滿指定的奈秒數，即使被 Signal 打斷也會繼續睡
// 這是製造 Loss 的關鍵，模擬 CPU Bound 的任務
void force_nanosleep(long nsec) {
    struct timespec req, rem;
    req.tv_sec = 0;
    req.tv_nsec = nsec; // 設定奈秒 (1ms = 1,000,000 ns)

    // 如果 nanosleep 被信號中斷 (回傳 -1 且 errno == EINTR)，
    // rem 會包含剩餘未睡的時間，我們就用 rem 當作新的 req 繼續睡
    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem;
    }
}

// Signal Handler: 必須使用 SA_SIGINFO 才能接收 sigqueue 帶來的參數 [cite: 37]
void sig_handler(int sig, siginfo_t *si, void *ucontext) {
    if (si->si_code == SI_QUEUE) {
        signal_seq_num = si->si_value.sival_int; // 取得資料序號
        wake_up = 1;
    }
}

int main(int argc, char *argv[]) {
    // 檢查參數 [cite: 20]
    if (argc != 5) {
        fprintf(stderr, "Usage: %s [D] [R] [C] [B]\n", argv[0]);
        return 1;
    }

    int D = atoi(argv[1]); // 資料數量
    int R = atoi(argv[2]); // 傳送速率 (ms)
    int C = atoi(argv[3]); // Consumer 數量
    int B = atoi(argv[4]); // Buffer Size

    // 計算 Shared Memory 大小
    size_t shm_size = (B * sizeof(Packet)) + (C * sizeof(int));

    // 建立 Shared Memory [cite: 37]
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }
    ftruncate(fd, shm_size);

    void *shm_ptr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    Packet *packet_buffer = (Packet *)shm_ptr;
    int *consumer_results = (int *)(shm_ptr + (B * sizeof(Packet)));
    memset(consumer_results, 0, C * sizeof(int));

    // 建立 Consumer Processes [cite: 22]
    pid_t pids[C];
    int consumer_id = -1;

    for (int i = 0; i < C; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            consumer_id = i;
            break;
        } else {
            pids[i] = pid;
        }
    }

    if (consumer_id != -1) {
        // ================= Consumer 邏輯 =================
        
        struct sigaction sa;
        sa.sa_sigaction = sig_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO; // 啟用參數傳遞功能 [cite: 37]

        if (sigaction(SIGUSR1, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }

        int my_count = 0;
        int current_shm_seq = -1;

        while (1) {
            // 等待 Signal 通知
            while (!wake_up) {
                pause();
            }
            wake_up = 0; 

            // 計算 Buffer 位置
            int target_idx = signal_seq_num % B;
            char *msg_content = packet_buffer[target_idx].msg;

            // 讀取並驗證資料
            if (sscanf(msg_content, "This is message %d", &current_shm_seq) == 1) {
                // 如果 Shared Memory 的序號等於 Signal 通知的序號，代表資料有效
                if (current_shm_seq == signal_seq_num) {
                    my_count++;
                }
            }

            // 檢查是否結束 (收到最後一筆資料 D-1)
            // 注意：即使要結束了，這裡通常也會執行 sleep，模擬處理這最後一筆資料
            int should_break = (signal_seq_num >= D - 1);

            // ============================================
            // 使用 nanosleep 模擬處理負載
            // 參數建議：2000000 ns = 2ms
            // 如果您的電腦很快，導致 Loss 還是 0，請增加此數值
            // ============================================
            force_nanosleep(50000000L); 

            if (should_break) {
                break;
            }
        }

        consumer_results[consumer_id] = my_count;
        munmap(shm_ptr, shm_size);
        exit(0);

    } else {
        // ================= Producer 邏輯 =================
        
        sleep(1); // 讓 Consumer 準備好 Handler

        for (int i = 0; i < D; i++) {
            // 1. 寫入 Shared Memory (Buffer) [cite: 28]
            int buf_idx = i % B;
            snprintf(packet_buffer[buf_idx].msg, 80, "This is message %d", i);

            // 2. 準備 Signal 參數 (序號) [cite: 37]
            union sigval value;
            value.sival_int = i;

            // 3. 發送 Signal 給所有 Consumer
            // 因為要帶參數，必須用 sigqueue 逐一發送 (效能瓶頸點)
            for (int j = 0; j < C; j++) {
                sigqueue(pids[j], SIGUSR1, value);
            }

            // 4. 等待 R ms (R=0 時全速發送) [cite: 24]
            if (R > 0) {
                usleep(R * 1000);
            }
        }

        // 等待所有 Consumer 結束
        for (int i = 0; i < C; i++) {
            waitpid(pids[i], NULL, 0);
        }

        // 統計結果 [cite: 38-43]
        long total_received = 0;
        for (int i = 0; i < C; i++) {
            total_received += consumer_results[i];
        }

        long total_expected = (long)D * C;
        double loss_rate = 1.0 - ((double)total_received / total_expected);

        printf("D=%d R=%d ms C=%d B=%d\n", D, R, C, B);
        printf("Total messages: %ld\n", total_expected);
        printf("Sum of received messages by all consumers: %ld\n", total_received);
        printf("Loss rate: %f\n", loss_rate);

        munmap(shm_ptr, shm_size);
        shm_unlink(SHM_NAME);
    }

    return 0;
}
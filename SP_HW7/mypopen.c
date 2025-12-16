/*
 * mypopen.c
 *
 * 實作 popen() 和 pclose() 的簡化版本。
 *
 * popen(): 建立一個子行程執行命令，並返回一個 FILE* 串流，
 * 用於讀取子行程的 stdout (type="r") 或
 * 寫入子行程的 stdin (type="w")。
 *
 * pclose(): 關閉 popen() 返回的串流，並等待子行程結束。
 *

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // For pipe, fork, dup2, exec, _exit, sysconf
#include <string.h>     // For strcmp
#include <sys/wait.h>   // For waitpid
#include <errno.h>      // For errno

#define BUF_SIZE 512

static pid_t *child_pids = NULL;
static int max_fds;

/**
 * @brief 實作 popen()。
 * @param command 要執行的 shell 命令字串。
 * @param type "r" (讀取子行程 stdout) 或 "w" (寫入子行程 stdin)。
 * @return 成功時返回一個 FILE* 串流，失敗時返回 NULL。
 */
FILE *mypopen(const char *command, const char *type) {
    int pfd[2]; // Pipe file descriptors
    pid_t pid;
    FILE *stream;
    int parent_fd, child_fd; // 用於區分父子行程各自要操作的 fd

    // 1. 第一次呼叫時，初始化 child_pids 陣列
    if (child_pids == NULL) {
        // 取得系統支援的最大 file descriptors 數量
        max_fds = sysconf(_SC_OPEN_MAX);
        if (max_fds == -1) {
            errno = ENOMEM; // (近似)
            return NULL;
        }

        // 配置並清零
        child_pids = calloc(max_fds, sizeof(pid_t));
        if (child_pids == NULL) {
            errno = ENOMEM;
            return NULL;
        }
    }

    // 2. 檢查 type 參數
    if (strcmp(type, "r") != 0 && strcmp(type, "w") != 0) {
        errno = EINVAL; // 無效的參數
        return NULL;
    }

    // 3. 建立 pipe
    if (pipe(pfd) == -1) {
        return NULL; // pipe() 會設定 errno
    }

    // 4. 建立子行程
    pid = fork();
    if (pid == -1) {
        // fork 失敗，關閉 pipe 並返回
        close(pfd[0]);
        close(pfd[1]);
        return NULL; // fork() 會設定 errno
    }

    /* ================================= */
    /* 子行程 (CHILD PROCESS)         */
    /* ================================= */
    if (pid == 0) {
        if (strcmp(type, "r") == 0) {
            // 父行程要 "r" (讀)，所以子行程要 "w" (寫)
            // 關閉子行程用不到的 pipe 讀取端
            close(pfd[0]);
            // 將子行程的 stdout 重定向到 pipe 的寫入端
            if (dup2(pfd[1], STDOUT_FILENO) == -1) {
                perror("child: dup2(stdout)");
                _exit(127); // 使用 _exit 避免刷新 stdio 緩衝區
            }
            // 關閉已重定向的原始 fd
            close(pfd[1]);
        } else {
            // 父行程要 "w" (寫)，所以子行程要 "r" (讀)
            // 關閉子行程用不到的 pipe 寫入端
            close(pfd[1]);
            // 將子行程的 stdin 重定向到 pipe 的讀取端
            if (dup2(pfd[0], STDIN_FILENO) == -1) {
                perror("child: dup2(stdin)");
                _exit(127);
            }
            // 關閉已重定向的原始 fd
            close(pfd[0]);
        }

        // 核心要求：關閉所有從父行程繼承來的、由其他 popen() 呼叫所建立的 FDs
        for (int i = 0; i < max_fds; i++) {
            if (child_pids[i] > 0) { // > 0 表示這是一個活躍的 popen fd
                close(i);
            }
        }

        // 執行命令
        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        
        // 如果 execl 返回，表示出錯
        perror("child: execl");
        _exit(127); // 127 是 shell 找不到命令的標準退出碼
    }

    /* ================================= */
    /* 父行程 (PARENT PROCESS)        */
    /* ================================= */
    
    // 根據 type 決定父行程要用的 fd，並關閉另一個
    if (strcmp(type, "r") == 0) {
        // 父行程要 "r" (讀)，使用 pipe 的讀取端 pfd[0]
        parent_fd = pfd[0];
        child_fd = pfd[1]; // 子行程的寫入端
    } else {
        // 父行程要 "w" (寫)，使用 pipe 的寫入端 pfd[1]
        parent_fd = pfd[1];
        child_fd = pfd[0]; // 子行程的讀取端
    }

    // 父行程關閉 pipe 中自己用不到的那一端
    close(child_fd);

    // 將 file descriptor 轉換為 FILE* 串流
    stream = fdopen(parent_fd, type);
    if (stream == NULL) {
        // fdopen 失敗，關閉 fd 並回收子行程
        close(parent_fd);
        waitpid(pid, NULL, 0); // 避免殭屍行程
        return NULL;
    }

    // 成功！在我們的陣列中儲存子行程 PID
    // 索引是父行程持有的 pipe file descriptor
    child_pids[parent_fd] = pid;

    return stream;
}

/**
 * @brief 實作 pclose()。
 * @param stream 由 mypopen() 返回的 FILE* 串流。
 * @return 成功時返回子行程的退出狀態，失敗時返回 -1。
 */
int mypclose(FILE *stream) {
    int fd, status;
    pid_t pid;

    if (stream == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (child_pids == NULL) {
        // 理論上，如果 stream 有效，child_pids 應該已被初始化
        // 但作為防禦性程式設計，還是檢查一下
        errno = EINVAL;
        return -1;
    }

    // 1. 根據 FILE* 取得 file descriptor
    fd = fileno(stream);
    if (fd == -1) {
        return -1; // fileno() 會設定 errno
    }

    // 2. 從我們的陣列中取出 PID
    pid = child_pids[fd];
    if (pid <= 0) { // 0 或 -1 表示這不是一個由 mypopen 管理的 fd
        errno = EINVAL;
        return -1;
    }

    // 3. 標記此 fd 已關閉
    child_pids[fd] = 0; // 或 -1

    // 4. 關閉串流 (這也會關閉底層的 fd)
    if (fclose(stream) == EOF) {
        return -1; // fclose() 會設定 errno
    }

    // 5. 等待指定的子行程結束
    // 使用迴圈來處理 EINTR (被訊號中斷) 的情況
    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) {
            status = -1; // waitpid() 失敗
            break;
        }
    }

    return status;
}


/* ========================================================================= */
/* 測試程式                                   */
/* ========================================================================= */
int main() {
    FILE *stream_in, *stream_out;
    char buf[BUF_SIZE];

    printf("test write : to upper\n");
    stream_out = mypopen("tr 'a-z' 'A-Z'", "w");
    if (stream_out == NULL) {
        perror("sh write");
        exit(EXIT_FAILURE);
    }

    fprintf(stream_out, "The quick brown fox jumps over the lazy dog\n");

    int status_w = mypclose(stream_out);
    printf("\nmypclose() return with code : %d\n", status_w);
    if (WIFEXITED(status_w)) {
        printf("child exit noramlly with code : %d\n", WEXITSTATUS(status_w));
    }
    printf("------------------------------------------\n");

    printf("------------------------------------------\n\n");


    printf("test read : show date\n");
    stream_in = mypopen("date", "r"); 
    if (stream_in == NULL) {
        perror("date");
        exit(EXIT_FAILURE);
    }

    // 讀取子行程的 stdout 直到 EOF
    while (fgets(buf, sizeof(buf), stream_in) != NULL) {
        printf("date: %s", buf); // buf 中已包含換行符
    }

    // 關閉串流並取得退出狀態
    int status_r = mypclose(stream_in);
    printf("\nmypclose() return with code : %d\n", status_r);
    if (WIFEXITED(status_r)) {
        printf("child exit noramlly with code : %d\n", WEXITSTATUS(status_r));
    }
    printf("------------------------------------------\n");

    // 釋放靜態陣列 (在實際的函式庫中通常不會這樣做，但
    // 為了 valgrind 檢查，在 main 結束前釋放是個好習慣)
    if (child_pids != NULL) {
        free(child_pids);
    }

    return 0;
}
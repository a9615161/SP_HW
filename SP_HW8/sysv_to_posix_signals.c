#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void errExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int my_sighold(int sig) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, sig);
    return sigprocmask(SIG_BLOCK, &set, NULL);
}

int my_sigrelse(int sig) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, sig);
    return sigprocmask(SIG_UNBLOCK, &set, NULL);
}

int my_sigignore(int sig) {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    return sigaction(sig, &sa, NULL);
}

int my_sigpause(int sig) {
    sigset_t currentMask, newMask;

    // 1. 取得目前的訊號遮罩
    if (sigprocmask(SIG_SETMASK, NULL, &currentMask) == -1) {
        return -1;
    }

    newMask = currentMask;
    sigdelset(&newMask, sig);

    return sigsuspend(&newMask);
}

typedef void (*SysVHandler)(int);

SysVHandler my_sigset(int sig, SysVHandler handler) {
    struct sigaction sa, old_sa;
    sigset_t set, old_mask;
    SysVHandler prevHandler;

    if (sigprocmask(SIG_SETMASK, NULL, &old_mask) == -1) {
        return SIG_ERR;
    }

    if (sigaction(sig, NULL, &old_sa) == -1) {
        return SIG_ERR;
    }

    if (sigismember(&old_mask, sig)) {
        prevHandler = SIG_HOLD;
    } else {
        prevHandler = old_sa.sa_handler;
    }

    sigemptyset(&set);
    sigaddset(&set, sig);

    if (handler == SIG_HOLD) {
        if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
            return SIG_ERR;
        }
    } else {
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART; 

        if (sigaction(sig, &sa, NULL) == -1) {
            return SIG_ERR;
        }

        if (sigprocmask(SIG_UNBLOCK, &set, NULL) == -1) {
            return SIG_ERR;
        }
    }

    return prevHandler;
}

static void usr1_handler(int sig) {
    printf(">> 捕捉到 SIGUSR1! <<\n");
}

int main() {
    printf("我的 PID 是: %ld\n\n", (long)getpid());

    // --- 測試 1: my_sigignore ---
    printf("--- 測試 my_sigignore(SIGINT) ---\n");
    if (my_sigignore(SIGINT) == -1) errExit("my_sigignore");
    printf("SIGINT (Ctrl-C) 現在應該被忽略。請在 3 秒內嘗試...\n");
    sleep(3);
    printf("... 測試結束。\n\n");

    // --- 測試 2: my_sigset (還原) ---
    printf("--- 測試 my_sigset(SIGINT, SIG_DFL) ---\n");
    if (my_sigset(SIGINT, SIG_DFL) == SIG_ERR) errExit("my_sigset DFL");
    printf("SIGINT (Ctrl-C) 已還原為預設動作 (終止)。\n\n");

    // --- 測試 3: my_sighold ---
    printf("--- 測試 my_sighold(SIGUSR1) ---\n");
    if (my_sighold(SIGUSR1) == -1) errExit("my_sighold");
    printf("SIGUSR1 已被阻擋 (held)。\n\n");

    // --- 測試 4: my_sigset (設定 handler) ---
    printf("--- 測試 my_sigset(SIGUSR1, handler) ---\n");
    SysVHandler prev = my_sigset(SIGUSR1, usr1_handler);
    if (prev == SIG_HOLD) {
        printf("my_sigset 成功回傳 SIG_HOLD。\n");
    } else {
        printf("my_sigset 測試失敗 (未回傳 SIG_HOLD)。\n");
    }
    printf("SIGUSR1 處理常式已設定，且訊號已解除阻擋。\n");
    printf("傳送 SIGUSR1 給自己...\n");
    kill(getpid(), SIGUSR1);
    sleep(1); // 等待 handler 處理完
    printf("\n");

    // --- 測試 5: my_sigpause ---
    printf("--- 測試 my_sigpause(SIGUSR1) ---\n");
    printf("首先，再次阻擋 SIGUSR1...\n");
    if (my_sighold(SIGUSR1) == -1) errExit("my_sighold 2");
    printf("SIGUSR1 已被阻擋。現在呼叫 my_sigpause() 等待它...\n");
    printf("請在另一個終端機執行: kill -USR1 %ld\n", (long)getpid());
    
    int result = my_sigpause(SIGUSR1); // 會解除阻擋並等待
    
    if (result == -1 && errno == EINTR) {
        printf("my_sigpause() 被訊號中斷並返回 (正確)。\n");
    } else {
        printf("my_sigpause() 返回 %d (errno: %d)\n", result, errno);
    }
    printf("\n測試完畢。\n");
    
    return 0;
}
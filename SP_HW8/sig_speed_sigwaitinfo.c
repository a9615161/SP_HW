#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

void errExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "Usage: %s num-sigs\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int numSigs = atoi(argv[1]);
    pid_t childPid;
    sigset_t blockMask, waitMaskParent, waitMaskChild;

    printf("PID: %ld; exchanging %d signals using sigwaitinfo()\n", (long)getpid(), numSigs);

    /* 1. 阻擋這兩個訊號，這樣才能被 sigwaitinfo 捕捉 */
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGUSR1);
    sigaddset(&blockMask, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
        errExit("sigprocmask");
    }

    /* 2. 準備 sigwaitinfo 要等待的遮罩 */
    sigemptyset(&waitMaskChild);
    sigaddset(&waitMaskChild, SIGUSR1); /* 子行程等待 SIGUSR1 */

    sigemptyset(&waitMaskParent);
    sigaddset(&waitMaskParent, SIGUSR2); /* 父行程等待 SIGUSR2 */

    childPid = fork();
    if (childPid == -1) {
        errExit("fork");
    }

    if (childPid == 0) { /* 子行程 */
        pid_t parentPid = getppid();
        for (int i = 0; i < numSigs; i++) {
            /* 同步等待 SIGUSR1 */
            if (sigwaitinfo(&waitMaskChild, NULL) == -1) errExit("sigwaitinfo (child)");

            /* 傳送 SIGUSR2 回給父行程 */
            if (kill(parentPid, SIGUSR2) == -1) errExit("kill (child)");
        }
        exit(EXIT_SUCCESS);

    } else { /* 父行程 */
        for (int i = 0; i < numSigs; i++) {
            /* 傳送 SIGUSR1 給子行程 */
            if (kill(childPid, SIGUSR1) == -1) errExit("kill (parent)");

            /* 同步等待 SIGUSR2 */
            if (sigwaitinfo(&waitMaskParent, NULL) == -1) errExit("sigwaitinfo (parent)");
        }
        printf("Parent finished.\n");
        exit(EXIT_SUCCESS);
    }
}
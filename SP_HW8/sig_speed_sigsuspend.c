#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

static volatile sig_atomic_t sigReceived = 0;

void errExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void handler(int sig) {
    sigReceived = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "Usage: %s num-sigs\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int numSigs = atoi(argv[1]);
    pid_t childPid;
    struct sigaction sa;
    sigset_t blockMask, emptyMask;

    printf("PID: %ld; exchanging %d signals using sigsuspend()\n", (long)getpid(), numSigs);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) errExit("sigaction SIGUSR1");
    if (sigaction(SIGUSR2, &sa, NULL) == -1) errExit("sigaction SIGUSR2");

    //block sig whlie setting the mask
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGUSR1);
    sigaddset(&blockMask, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
        errExit("sigprocmask");
    }

    sigemptyset(&emptyMask);

    childPid = fork();
    if (childPid == -1) {
        errExit("fork");
    }

    if (childPid == 0) { 
        pid_t parentPid = getppid();
        for (int i = 0; i < numSigs; i++) {
            sigReceived = 0;
            while (sigReceived == 0) {
                sigsuspend(&emptyMask);
            }

            if (kill(parentPid, SIGUSR2) == -1) errExit("kill (child)");
        }
        exit(EXIT_SUCCESS);

    } else { /* 父行程 */
        for (int i = 0; i < numSigs; i++) {
            /* 傳送 SIGUSR1 給子行程 */
            if (kill(childPid, SIGUSR1) == -1) errExit("kill (parent)");

            /* 等待來自子行程的 SIGUSR2 */
            sigReceived = 0;
            while (sigReceived == 0) {
                sigsuspend(&emptyMask);
            }
        }
        printf("Parent finished.\n");
        exit(EXIT_SUCCESS);
    }
}
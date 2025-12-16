/*
 * run_command.c :    do the fork, exec stuff, call other functions
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include "shell.h"
#include <unistd.h>   // for fork(), execvp(), _exit()
void run_command(char **myArgv) {
    pid_t pid;
    int status;
    int is_backg = 0;
    if(myArgv == NULL )
        return;
    else
        is_backg = is_background(myArgv);
    /* Create a new child process.
     * Fill in code.
	 */
    pid = fork();

    switch (pid) {

        /* Error. */
        case -1 :
            perror("fork");
            exit(errno);

        /* Parent. */
        default :
            /* Wait for child to terminate.
             * Fill in code.
			 */
            if (is_backg) {
                printf("[Background PID %d]\n", pid);
                return;
            }
            
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                return;
            }


            /* Optional: display exit status.  (See wstat(5).)
             * Fill in code.
			 */
            if (WIFEXITED(status)) {//if wait exited normally
                printf("Process exited with status %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {//if wait ended by signal
                printf("Process terminated by signal %d\n", WTERMSIG(status));
            }
            return;
        /* Child. */
        case 0 :
            /* Run command in child process.
             * Fill in code.
			 */
            execvp(myArgv[0], myArgv);
            perror("execvp");
            /* Handle error return from exec */
			exit(errno);
    }
}

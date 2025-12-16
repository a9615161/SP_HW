

#include <stdio.h>      // For perror, fprintf, stderr
#include <stdlib.h>     // For exit, EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>     // For pipe, fork, read, write, close, STDIN_FILENO, STDOUT_FILENO, dup2
#include <string.h>     // For memset
#include <ctype.h>      // For toupper
#include <sys/wait.h>   // For wait
#define BUF_SIZE 512   

int main() {
    int pipe_to_child[2];   // Pipe: Parent -> Child
    int pipe_to_parent[2];  // Pipe: Child -> Parent
    pid_t pid;
    char buf[BUF_SIZE];
    ssize_t numRead;

    if (pipe(pipe_to_child) == -1) {
        perror("pipe (parent to child) failed");
        exit(EXIT_FAILURE);
    }

    if (pipe(pipe_to_parent) == -1) {
        perror("pipe (child to parent) failed");
        exit(EXIT_FAILURE);
    }


    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        close(pipe_to_child[1]);   
        close(pipe_to_parent[0]);  

        if (dup2(pipe_to_child[0], STDIN_FILENO) == -1) {
            perror("child: dup2 (stdin)");
            exit(EXIT_FAILURE);
        }

        if (dup2(pipe_to_parent[1], STDOUT_FILENO) == -1) {
            perror("child: dup2 (stdout)");
            exit(EXIT_FAILURE);
        }

        if (close(pipe_to_child[0]) == -1) {
             perror("child: close(pipe_to_child[0])");
             exit(EXIT_FAILURE);
        }
        if (close(pipe_to_parent[1]) == -1) {
             perror("child: close(pipe_to_parent[1])");
             exit(EXIT_FAILURE);
        }

        while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
            
            for (int i = 0; i < numRead; i++) {
                buf[i] = toupper((unsigned char) buf[i]);
            }

            if (write(STDOUT_FILENO, buf, numRead) != numRead) {
                perror("child: write to stdout failed");
                exit(EXIT_FAILURE);
            }
        }

        if (numRead == -1) {
            perror("child: read from stdin failed");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS); 
    }



    close(pipe_to_child[0]);   
    close(pipe_to_parent[1]);  

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        
        if (write(pipe_to_child[1], buf, numRead) != numRead) {
            perror("parent: write to child failed");
            exit(EXIT_FAILURE);
        }

        ssize_t numReadFromChild = read(pipe_to_parent[0], buf, BUF_SIZE);

        if (numReadFromChild == -1) {
            perror("parent: read from child failed");
            exit(EXIT_FAILURE);
        } else if (numReadFromChild == 0) {
            fprintf(stderr, "parent: child terminated unexpectedly\n");
            break; 
        }

        // 將回應 (大寫文字) 寫入 stdout
        if (write(STDOUT_FILENO, buf, numReadFromChild) != numReadFromChild) {
            perror("parent: write to stdout failed");
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        perror("parent: read from stdin failed");
        exit(EXIT_FAILURE);
    }

    // 關閉 P->C pipe 的寫入端 (發送 EOF 給子行程)
    if (close(pipe_to_child[1]) == -1) {
        perror("parent: close(pipe_to_child[1])");
        exit(EXIT_FAILURE);
    }

    // 關閉 C->P pipe 的讀取端
    if (close(pipe_to_parent[0]) == -1) {
        perror("parent: close(pipe_to_parent[0])");
        exit(EXIT_FAILURE);
    }

    // 等待子行程結束
    if (wait(NULL) == -1) {
        perror("parent: wait() failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
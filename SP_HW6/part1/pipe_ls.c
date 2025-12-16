#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#define READ_END 0
#define WRITE_END 1
#define SIZE_MAX 1024
int main(){
    int pd[2];
    int pid ;

    if(pipe(pd) == -1){
        perror("pipe creation failed");
        return 1;
    }
    pid = fork();

    if(pid == -1){
        perror("fork failed");
        return 1;
    }
    else if(pid == 0){
        close(pd[READ_END]);
        dup2(pd[WRITE_END] , STDOUT_FILENO);
        //now pd[WRITE_END] was opened by 2 person
        close(pd[WRITE_END]);
        execlp("ls","ls" , "-l" , NULL);
        //should not return
        perror("execlp failed");
        exit(EXIT_FAILURE);
    }
    else{
        char buffer[SIZE_MAX];
        ssize_t Nread;
        int stat;
        
        close(pd[WRITE_END]);

        while((Nread = read(pd[READ_END] ,buffer, SIZE_MAX-1)) > 0 ){

            write(STDOUT_FILENO,buffer,Nread);
        }
        if(Nread == -1){
            perror("Failed to read from pipe");
        }
        
        close(pd[READ_END]);

        waitpid(pid,&stat,0);
    }

    return 0;
}
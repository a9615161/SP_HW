#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

/*
       As an extension to the POSIX.1-2001 standard, glibc's getcwd() allocates the buffer dynam‐
       ically using malloc(3) if buf is NULL.  In this case, the allocated buffer has the  length
       size  unless  size  is zero, when buf is allocated as big as necessary.  The caller should
       free(3) the returned buffer.
*/
int main(){
    char * dirPath;
    dirPath = getcwd(NULL,0);

    printf("%s\n",dirPath);
}

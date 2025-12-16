/*
 * redirect_in.c  :  check for <
 */

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define READ_END 0
#define WRITE_END 1

/*
 * Look for "<" in myArgv, then redirect input to the file.
 * Returns 0 on success, sets errno and returns -1 on error.
 */
int redirect_in(char ** myArgv) {
  	int i = 0;
  	int fd;

  	/* search forward for <
  	 *
	 * Fill in code. */
	for(int i = 0;myArgv[i] != NULL;++i){
		if(strcmp(myArgv[i] , "<") == 0 ){

			char * fileName = myArgv[i+1];
			if(fileName == NULL){
				fprintf(stderr, "expected file name to read\n");
    			errno = EINVAL; // Set errno for consistency
				return -1;
			}
			fd = open(fileName,O_RDONLY);
			if( fd == -1){
				perror(fileName);
				return -1;
			}

			if( dup2(fd , STDIN_FILENO) == -1 ){
				perror("dup2");
				return -1;
			}

			close(fd);

			free(myArgv[i]);
			myArgv[i] = NULL;
			free(myArgv[i+1]);
			myArgv[i+1] = NULL;
			// Shift all subsequent pointers left by 2
            int j = i;
            while (myArgv[j+2] != NULL) {
                myArgv[j] = myArgv[j+2];
                j++;
            }
            myArgv[j] = NULL; // Set the new end of the argument list

			break;
		}

	}

    	/* 1) Open file.
     	 * 2) Redirect stdin to use file for input.
   		 * 3) Cleanup / close unneeded file descriptors.
   		 * 4) Remove the "<" and the filename from myArgv.
		 *
   		 * Fill in code. */
  	return 0;
}

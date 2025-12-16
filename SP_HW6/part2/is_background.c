/*
 * is_background.c :  check for & at end
 */

#include <stdio.h>
#include "shell.h"
#include <string.h>
#include <stdlib.h>

int is_background(char ** myArgv) {

    if (myArgv == NULL || myArgv[0] == NULL) {
        return 0; // FALSE
    }

    int i = 0;

    while (myArgv[i+1] != NULL) {
        i++;
    }
    

    if (strcmp(myArgv[i], "&") == 0) {
        
        free(myArgv[i]);
        myArgv[i] = NULL;
        
        return 1; 
    }

    return 0; // FALSE
}
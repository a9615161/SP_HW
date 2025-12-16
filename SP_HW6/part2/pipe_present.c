/*
 *  pipe_present.c :  check for |
 */

#include <stdio.h>
#include "shell.h"
#include<string.h>
/*
 * Return index offset into argv of where "|" is,
 * -1 if in an illegal position (first or last index in the array),
 * or 0 if not present.
 */
int pipe_present(char ** myCurrentArgv) {
	int index = -1;

  	/* Search through myCurrentArgv for a match on "|". */
	int cnt = 0 ;
	for (cnt = 0; myCurrentArgv[cnt] != NULL; ++cnt);

	for( int i =0;i<cnt; ++i){
		if(strcmp( myCurrentArgv[i] , "|" ) == 0){
			index = i;
			break;
		}
	}

		
  	if (index == 0 || index == cnt-1) 
    	return -1;
  	else if (index == -1)	/* Off the end. */ 
    	return 0;

  	else 					/* In the middle. */
    	return index;
}

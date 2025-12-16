/*
 * parse.c : use whitespace to tokenise a line
 * Initialise a vector big enough
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"

/* Parse a commandline string into an argv array. */
char ** parse(char *line) {

  	static char delim[] = " \t\n"; /* SPACE or TAB or NL */
  	int count = 0;
  	char * token;
  	char **newArgv;

  	/* Nothing entered. */
  	if (line == NULL) {
    	return NULL;
  	}

  	/* Init strtok with commandline, then get first token.
     * Return NULL if no tokens in line.
	 *
	 * Fill in code.
     */
	//also handle empty input 
	if((token = strtok(line , delim)) == NULL)
		return NULL;
	
  	/* Create array with room for first token.
  	 *
	 * 
	 * While there are more tokens...
	 *
	 *  - Get next token.
	 *	- Resize array.
	 *  - Give token its own memory, then install it.
	 * 
  	 * Fill in code.
	 */
	newArgv = NULL;
	do {
        /* Resize array for one more element */
        char **tmp = realloc(newArgv, (count + 1) * sizeof(char *));
        if (tmp == NULL) {
            perror("realloc");
            exit(1);
        }
        newArgv = tmp;

        /* Copy token to its own memory */
        newArgv[count] = strdup(token);
        if (newArgv[count] == NULL) {
            perror("strdup");
            exit(1);
        }
		printf("[%d]:%s\n",count, newArgv[count]);
        count++;
    } while ((token = strtok(NULL, delim)) != NULL);


  	/* Null terminate the array and return it.
	 *
  	 * Fill in code.
	 */
	newArgv = realloc(newArgv, (count + 1) * sizeof(char *));
	newArgv [count] = NULL;
		
  	return newArgv;
}


/*
 * Free memory associated with argv array passed in.
 * Argv array is assumed created with parse() above.
 */
void free_argv(char **oldArgv) {

	int i = 0;

	/* Free each string hanging off the array.
	 * Free the oldArgv array itself.
	 *
	 * Fill in code.
	 */
	if(oldArgv == NULL)
		return;
	while (oldArgv[i] != NULL)
		free(oldArgv[i++]);
	free(oldArgv);
}

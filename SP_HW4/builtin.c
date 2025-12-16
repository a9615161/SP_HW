/*
 * builtin.c : check for shell built-in commands
 * structure of file is
 * 1. definition of builtin functions
 * 2. lookup-table
 * 3. definition of is_builtin and do_builtin
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shell.h"
#include <string.h>



/****************************************************************************/
/* builtin function definitions                                             */
/****************************************************************************/

/* "echo" command.  Does not print final <CR> if "-n" encountered. */
static void bi_echo(char **argv) {
    int opt;
    int n = 0;     // 用來記錄 -n 的值（第幾個字串）
    int argc = 0;

    // 計算 argv 有幾個參數
    while (argv[argc] != NULL)
        argc++;

    opterr = 0; // 關閉 getopt 的自動錯誤訊息

    // 解析選項 -n <number>
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Usage: echo [-n number] [strings...]\n");
                return;
        }
    }

    // 沒有 -n，印出全部字串（每個換行）
    if (n <= 0) {
        for (int i = optind; i < argc-1; i++)
            printf("%s ",argv[i]);
        printf("%s\n",argv[argc-1]);
    }
    // 有指定 -n，只印第 n 個字串（換行）
    else {
        int target = optind + n - 1;
        if (target < argc)
            puts(argv[target]);
        else
            fprintf(stderr, "echo: n = %d > number of argc = %d\n", n ,argc);
    }
}

/* Fill in code. */
static void myExit(char **argv) {
    // 先印出 Bye 訊息
    puts(argv[0]);

    // flush stdout/stderr，確保所有輸出都送出
    fflush(stdout);
    fflush(stderr);
	free_argv(argv);

	_exit(0);
}




/****************************************************************************/
/* lookup table                                                             */
/****************************************************************************/

static struct cmd {
	char * keyword;				/* When this field is argv[0] ... */
	void (* do_it)(char **);	/* ... this function is executed. */
} inbuilts[] = {

	/* Fill in code. */

	{ "echo", bi_echo },		/* When "echo" is typed, bi_echo() executes.  */
	{ "exit", myExit },
	{ "quit", myExit },
	{ "logout", myExit },
	{ "bye", myExit },
	{ NULL, NULL }				/* NULL terminated. */
};
//echo print all strings echo -n N: print the specified string
//quit example: exit, quit, logout and bye terminate the program.




/****************************************************************************/
/* is_builtin and do_builtin                                                */
/****************************************************************************/

static struct cmd * this; 		/* close coupling between is_builtin & do_builtin */

/* Check to see if command is in the inbuilts table above.
Hold handle to it if it is. */
int is_builtin(char *cmd) {
  	struct cmd *tableCommand;

  	for (tableCommand = inbuilts ; tableCommand->keyword != NULL; tableCommand++)
    	if (strcmp(tableCommand->keyword,cmd) == 0) {
			this = tableCommand;
			return 1;
		}
  	return 0;
}


/* Execute the function corresponding to the builtin cmd found by is_builtin. */
int do_builtin(char **argv) {
  	this->do_it(argv);
}

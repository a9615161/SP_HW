/*
 * builtin.c : check for shell built-in commands
 * structure of file is
 * 1. definition of builtin functions
 * 2. lookup-table
 * 3. definition of is_builtin and do_builtin
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/utsname.h>
#include "shell.h"
#include<string.h>
#include <errno.h>
/****************************************************************************/
/* builtin function definitions                                             */
/****************************************************************************/
static void bi_builtin(char ** argv);	/* "builtin" command tells whether a command is builtin or not. */
static void bi_cd(char **argv) ;		/* "cd" command. */
static void bi_echo(char **argv);		/* "echo" command.  Does not print final <CR> if "-n" encountered. */
static void bi_hostname(char ** argv);	/* "hostname" command. */
static void bi_id(char ** argv);		/* "id" command shows user and group of this process. */
static void bi_pwd(char ** argv);		/* "pwd" command. */
static void bi_quit(char **argv);		/* quit/exit/logout/bye command. */




/****************************************************************************/
/* lookup table                                                             */
/****************************************************************************/

static struct cmd {
  	char * keyword;					/* When this field is argv[0] ... */
  	void (* do_it)(char **);		/* ... this function is executed. */
} inbuilts[] = {
  	{ "builtin",    bi_builtin },   /* List of (argv[0], function) pairs. */

    /* Fill in code. */
    { "echo",       bi_echo },
    { "quit",       bi_quit },
    { "exit",       bi_quit },
    { "bye",        bi_quit },
    { "logout",     bi_quit },
    { "cd",         bi_cd },
    { "pwd",        bi_pwd },
    { "id",         bi_id },
    { "hostname",   bi_hostname },
    {  NULL,        NULL }          /* NULL terminated. */
};


static void bi_builtin(char ** argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "builtin: usage: builtin [command]\n");
        return;
    }
    
    if (is_builtin(argv[1])) {
        printf("%s is a shell builtin\n", argv[1]);
    } else {
        printf("%s is not a shell builtin\n", argv[1]);
    }
}

static void bi_cd(char **argv) {
    char *path;

    // If no argument (just "cd"), go to HOME directory
    if (argv[1] == NULL) {
        path = getenv("HOME");
        if (path == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    } else {
        // Go to the specified directory
        path = argv[1];
    }

    // Perform the directory change
    if (chdir(path) == -1) {
        // chdir failed, print error
        // perror("cd"); // This will print "cd: No such file or directory" etc.
        fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
    }
}

/* "echo" command: print arguments */
static void bi_echo(char **argv) {
    int i = 1;
    int print_newline = 1; // Flag to print newline at the end

    // Check for "-n" option
    if (argv[1] != NULL && strcmp(argv[1], "-n") == 0) {
        print_newline = 0;
        i++; // Start printing from the next argument
    }

    // Loop through all arguments
    while (argv[i] != NULL) {
        printf("%s", argv[i]); // Print the argument
        
        // Print a space if this is not the last argument
        if (argv[i+1] != NULL) {
            printf(" ");
        }
        i++;
    }

    if (print_newline) {
        printf("\n");
    }
}
static void bi_hostname(char ** argv) {
    struct utsname uts;

    if (uname(&uts) == -1) {
        perror("hostname");
    } else {
        printf("%s\n", uts.nodename);
    }
}

static void bi_id(char ** argv) {
    uid_t uid = getuid();
    gid_t gid = getgid();
    uid_t euid = geteuid();
    gid_t egid = getegid();

    struct passwd *pw;
    struct group *gr;

    // Print Real UID
    printf("uid=%d", uid);
    pw = getpwuid(uid);
    if (pw) printf("(%s)", pw->pw_name);

    // Print Real GID
    printf(" gid=%d", gid);
    gr = getgrgid(gid);
    if (gr) printf("(%s)", gr->gr_name);

    // Print Effective UID, if different
    if (uid != euid) {
        printf(" euid=%d", euid);
        pw = getpwuid(euid);
        if (pw) printf("(%s)", pw->pw_name);
    }

    // Print Effective GID, if different
    if (gid != egid) {
        printf(" egid=%d", egid);
        gr = getgrgid(egid);
        if (gr) printf("(%s)", gr->gr_name);
    }

    // We could also print supplementary groups here, but this is a good start
    
    printf("\n");
}

static void bi_pwd(char ** argv) {
    char path[PATH_MAX]; // PATH_MAX is from <limits.h>

    if (getcwd(path, sizeof(path)) != NULL) {
        printf("%s\n", path);
    } else {
        perror("pwd");
    }
}

static void bi_quit(char **argv) {
	exit(0);
}


/****************************************************************************/
/* is_builtin and do_builtin                                                */
/****************************************************************************/

static struct cmd * this; /* close coupling between is_builtin & do_builtin */

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

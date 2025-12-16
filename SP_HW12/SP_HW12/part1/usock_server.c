/*
 * usock_server : listen on a Unix socket ; fork ;
 *                child does lookup ; replies down same socket
 * argv[1] is the name of the local datafile
 * PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "dict.h"
#include<unistd.h>
int main(int argc, char **argv) {
    struct sockaddr_un server;
    int sd,cd,n;
    Dictrec tryit;

    if (argc != 3) {
      fprintf(stderr,"Usage : %s <dictionary source>"
          "<Socket name>\n",argv[0]);
      exit(errno);
    }

    /* Setup socket.
     * Fill in code. */
	sd = socket(AF_UNIX, SOCK_STREAM, 0);    
	/* Initialize address.
     * Fill in code. */
	memset(&server, 0, sizeof(server));
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, argv[2]);
    /* Name and activate the socket.
     * Fill in code. */
	unlink(argv[2]);	//remove if exist 
	bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_un));
	listen(sd, 5);

    /* main loop : accept connection; fork a child to have dialogue */
    for (;;) {
		/* Wait for a connection.
		 * Fill in code. */
		int addr_len = sizeof(server);
		//c for client dir?
		cd = accept(sd, (struct sockaddr *)&server, (socklen_t *)&addr_len);
		/* Handle new client in a subprocess. */
		switch (fork()) {
			case -1 : 
				DIE("fork");
			case 0 :
				close (sd);	/* Rendezvous socket is for parent only. */
				/* Get next request.
				 * Fill in code. */
				while ((n = read(cd, &tryit, sizeof(Dictrec))) > 0) {

					/* Lookup request. */
					switch(lookup(&tryit,argv[1]) ) {
						/* Write response back to client. */
						case FOUND: 
							/* Fill in code. */
							write(cd, &tryit, sizeof(Dictrec));
							break;
						case NOTFOUND: 
							/* Fill in code. */
							strcpy(tryit.text, "XXXX");
							write(cd, &tryit, sizeof(Dictrec));
							break;
						case UNAVAIL:
							DIE(argv[1]);
					} /* end lookup switch */

				} /* end of client dialog */

				/* Terminate child process.  It is done. */
				exit(0);

			/* Parent continues here. */
			default :
				close(cd);
				break;
		} /* end fork switch */
    } /* end forever loop */
} /* end main */

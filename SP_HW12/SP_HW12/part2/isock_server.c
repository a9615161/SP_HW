/*
 * isock_server : listen on an internet socket ; fork ;
 *                child does lookup ; replies down same socket
 * argv[1] is the name of the local datafile
 * PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include<string.h>
#include<unistd.h>
#include "dict.h"

int main(int argc, char **argv) {
	static struct sockaddr_in server;
	int sd,cd,n;
	Dictrec tryit;

	if (argc != 2) {
		fprintf(stderr,"Usage : %s <datafile>\n",argv[0]);
		exit(1);
	}

	/* Create the socket.
	 * Fill in code. */
	sd = socket(AF_INET, SOCK_STREAM, 0);    
	if (sd < 0) { DIE("socket"); }

	/* Initialize address.
	 * Fill in code. */
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	//server.address = htons('127.0.0.1');
	/* INADDR_ANY 代表「監聽這台電腦所有的網卡」，比寫死 127.0.0.1 好用 */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Name and activate the socket.
	 * Fill in code. */
	/* 建議加上這行，避免重啟 Server 時顯示 Address already in use */
    int opt = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if( bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0 ){
		DIE("bind");
	}
	if(listen(sd, 5) < 0){
		DIE("listen");
	}
	
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
					/* Lookup the word , handling the different cases appropriately */
					switch(lookup(&tryit,argv[1]) ) {
						/* Write response back to the client. */
						case FOUND:
							write(cd, &tryit, sizeof(Dictrec));
							break;
						case NOTFOUND:
							strcpy(tryit.text, "XXXX");
							write(cd, &tryit, sizeof(Dictrec));							 break;
						case UNAVAIL:
							DIE(argv[1]);
					} /* end lookup switch */
				} /* end of client dialog */
				exit(0); /* child does not want to be a parent */

			default :
				close(cd);
				break;
		} /* end fork switch */
	} /* end forever loop */
} /* end main */

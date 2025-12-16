/*
 * udp_server : listen on a UDP socket ;reply immediately
 * argv[1] is the name of the local datafile
 * PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

#include "dict.h"

int main(int argc, char **argv) {
	static struct sockaddr_in server,client;
	int sockfd,siz;
	Dictrec dr, *tryit = &dr;

	if (argc != 2) {
		fprintf(stderr,"Usage : %s <datafile>\n",argv[0]);
		exit(errno);
	}

	/* Create a UDP socket.
	 * Fill in code. */
	/* 使用 UDP (SOCK_DGRAM) */
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) { DIE("socket"); }
	/* Initialize address.
	 * Fill in code. */
	memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);	
	/* Name and activate the socket.
	 * Fill in code. */
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        DIE("bind");
    }
	for (;;) { /* await client packet; respond immediately */

		siz = sizeof(client); /* siz must be non-zero */

		/* Wait for a request.
		 * Fill in code. */
		if (recvfrom(sd, tryit, sizeof(Dictrec), 0, \
                    (struct sockaddr *)&client, &siz) \
					< 0) {
            DIE("recvfrom");
        }
		//one recvfrom is all "connection" ,not like TCP
		//while (read(cd, &tryit, sizeof(Dictrec)) > 0) {
		/* Lookup request and respond to user. */
		switch(lookup(tryit,argv[1]) ) {
			case FOUND: 
				/* Send response.
				 * Fill in code. */
				break;
			case NOTFOUND : 
				/* Send response.
				 * Fill in code. */
				strcpy(tryit->text, "XXXX");
				break;
			case UNAVAIL:
				DIE(argv[1]);
		} /* end lookup switch */
		if (sendto(sd, tryit, sizeof(Dictrec), 0, \
              (struct sockaddr *)&client, siz) \
			  != sizeof(Dictrec)) {
        DIE("sendto");
        }
	} /* end forever loop */
} /* end main */

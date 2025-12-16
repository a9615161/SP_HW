/*
 * lookup9 : does no looking up locally, but instead asks
 * a server for the answer. Communication is by Internet UDP Sockets
 * The name of the server is passed as resource. PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "dict.h"
#include <string.h>
int lookup(Dictrec * sought, const char * resource) {
	static int sockfd;
	static struct sockaddr_in server;
	struct hostent *host;
	static int first_time = 1;

	if (first_time) {  /* Set up server address & create local UDP socket */
		first_time = 0;

		/* Set up destination address.
		 * Fill in code. */
		if( (host = gethostbyname(resource)) == NULL ){
			DIE("gethostbyname");
		}

		memset(&server,0,sizeof(server));
		server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        memcpy(&server.sin_addr, host->h_addr, host->h_length);

		/* Allocate a socket.
		 * Fill in code. */
        /* 注意：這裡是 SOCK_DGRAM */
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            DIE("client: socket");
        }
		/* UDP not need connect */
	}

	/* Send a datagram & await reply
	 * Fill in code. */
	if(sendto(sockfd , sought , sizeof(Dictrec) , 0\
		,(struct sockaddr * )&server ,sizeof(server)) != sizeof(Dictrec) ) {
		DIE("c sendto");
	}
	//recv
	struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
	if(recvfrom(sockfd , sought , sizeof(Dictrec) , 0\
		,(struct sockaddr * )&from_addr ,&from_len) \
		!= sizeof(Dictrec) ) {
		DIE("c recvfrom");
	}

	if (strcmp(sought->text,"XXXX") != 0) {
		return FOUND;
	}

	return NOTFOUND;
}

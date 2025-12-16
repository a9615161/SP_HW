/*
 * lookup8 : does no looking up locally, but instead asks
 * a server for the answer. Communication is by Internet TCP Sockets
 * The name of the server is passed as resource. PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "dict.h"

int lookup(Dictrec * sought, const char * resource) {
	static int sockfd;
	static struct sockaddr_in server;
	struct hostent *host;
	static int first_time = 1;

	if (first_time) {        /* connect to socket ; resource is server name */
		first_time = 0;
		/* resource 傳進來的是 "localhost" 或 IP 字串 */
        if ((host = gethostbyname(resource)) == NULL) {
            DIE("client: gethostbyname");
        }
		/* Set up destination address. */
		memset(&server, 0, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port =htons(PORT);
		memcpy(&server.sin_addr, host->h_addr, host->h_length);
		/* Fill in code. */
		/* Allocate socket. */
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            DIE("client: socket");
        }

		/* Connect to the server.
		 * Fill in code. */
		if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
            DIE("client: connect");
        }
	}

	/* write query on socket ; await reply
	 * Fill in code. */
	if (write(sockfd, sought, sizeof(Dictrec)) != sizeof(Dictrec)){
		perror("client write");
        return UNAVAIL;
	}
	//from 
	if (read(sockfd, sought, sizeof(Dictrec)) != sizeof(Dictrec)){
		perror("client read");
        return UNAVAIL;
	}
	if (strcmp(sought->text,"XXXX") != 0) {
		return FOUND;
	}

	return NOTFOUND;
}

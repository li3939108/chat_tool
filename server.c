#include <sys/types.h>	  
#include "commons.h"
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdlib.h>
#include <sys/select.h>
#include <arpa/inet.h>

extern void Writen(int fd, void *ptr, size_t n) ;
extern void err_quit(const char *fmt, ...);

int main(int argc, char **argv){
	int i, maxi, maxfd, listenfd, connfd, sockfd, max_number_of_clients;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	fd_set rset, allset;
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;


	if(argc == 4){
		listenfd = socket(AF_INET, SOCK_STREAM, 0);
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		int return_value = inet_pton(servaddr.sin_family, argv[1], &servaddr.sin_addr), port ; 
		if(return_value == 1){
//			servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		}else if(return_value == 0){ err_quit("Illegal IP address: %s", argv[1]) ;
		}else{ err_quit("Unknow error : IP address") ; }
		port = strtol (argv[2], NULL, 0)  ;
		if(port < 1024){ err_quit("%d is too small, Please assign a larger port number", port) ; 
		}else{ servaddr.sin_port = htons(port);} 
		max_number_of_clients = strtol(argv[3], NULL, 0) ;
		if(max_number_of_clients <= 0){ err_quit("Max number of clients should be a positive integer.") ;}
	}else{
		err_quit("./server server_ip server_port max_clients") ;
	}

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++){
		client[i] = -1;		
	}	/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; ) {
		rset = allset;		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (SA *) &cliaddr, &clilen);
#ifdef	NOTDEF
			printf("new client: %s, port %d\n",
					inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
					ntohs(cliaddr.sin_port));
#endif

			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE)
				err_quit("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
					/*4connection closed by client */
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} else
					Writen(sockfd, buf, n);

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}

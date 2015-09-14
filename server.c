#include <sys/types.h>	  
#include <sys/socket.h>
#include <strings.h>

#define SERV_PORT 28376

typedef struct sockaddr SA ;

void main(int argc, char **argv){
	int listenfd, connfd;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in      cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (SA *) &cliaddr, &clilen);

		if ( (childpid = Fork()) == 0) {        /* child process */
			Close(listenfd);        /* close listening socket */
			str_echo(connfd);       /* process the request */
			exit(0);
		}
		Close(connfd);  /* parent closes connected socket */
	}


}

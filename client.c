#include "commons.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>	  
#include <strings.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"

extern void str_cli(FILE *fp, int sockfd) ;
extern void err_quit(const char *fmt, ...);

int main(int argc, char **argv){
	int sockfd;
	struct sockaddr_in  servaddr;
	char *username ;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	if(argc == 4){	
		int return_value = inet_pton(servaddr.sin_family, argv[2], &servaddr.sin_addr), port ; 
		username = argv[1] ;
		if(return_value == 1){
			/*servaddr.sin_addr.s_addr = htonl(INADDR_ANY);*/
		}else if(return_value == 0){ err_quit("Illegal IP address: %s", argv[2]) ;
		}else{ err_quit("Unknow error : IP address") ; }
		port  = strtol (argv[3], NULL, 0)  ;
		if(port < 1024){ err_quit("%d is too small, Please assign a larger port number", port) ; 
		}else{ servaddr.sin_port = htons(port);} 
	}else{
		err_quit("Usage: ./client username server_ip server_port") ;
	}


	if( 0 == connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) ){
		msg_JOIN(username, sockfd);
		str_cli(stdin, sockfd);	 /* do it all */
	}else{
		err_quit("connection error") ;
	}

	exit(0);
}

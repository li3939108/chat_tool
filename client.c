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

extern void str_cli(FILE *fp, int sockfd) ;
extern void err_quit(const char *fmt, ...);

int main(int argc, char **argv){
	int sockfd;
	struct sockaddr_in  servaddr;

	if (argc != 2){
		err_quit("usage: tcpcli <IPaddress>");
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);


	if( 0 == connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) ){
		char *username = "chaofan" ;
		int username_len = strlen(username) , i = 0;
		int32_t join_head[2] = {HEADER(HEADER_JOIN, 4+strlen(username)) , 
			ATTRIBUTE(ATTR_USERNAME, strlen(username))};
		char JOIN_buf[24];
		memcpy(JOIN_buf, (char *)join_head, sizeof join_head) ;
		memcpy(JOIN_buf+8, username, strlen(username) );
		send(sockfd, JOIN_buf, 8 + strlen(username), 0) ;
			
		str_cli(stdin, sockfd);	 /* do it all */
	}else{
		err_quit("connection error") ;
	}

	exit(0);
}


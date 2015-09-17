#include "commons.h"
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>


extern void Writen(int fd, void *ptr, size_t nbytes);
extern void err_quit(const char *fmt, ...);
ssize_t Readline(int fd, void *ptr, size_t maxlen) ;

void str_cli(FILE *fp, int sockfd){
	char	sendline[MAXLINE], recvline[MAXLINE], MSG_buf[MAXLINE+8];
	fd_set rset;
	int nready, fpfd = fileno(fp)  ;
	
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset) ;
	FD_SET(fpfd, &rset) ;
	

	for(;;){
		nready = select(sockfd + 1, &rset, NULL , NULL, NULL);
		if(FD_ISSET(sockfd, &rset)){
			/*read something from server */
			if (Readline(sockfd, recvline, MAXLINE) == 0){
				err_quit("str_cli: server terminated prematurely");
			}
			fputs(recvline, stdout);
		}
	
		if(FD_ISSET(fpfd, &rset) ){
			/*read something to send to server */
			if (fgets(sendline, MAXLINE, fp) != NULL) {
				int32_t send_head[2] = {HEADER(HEADER_SEND, 4 + strlen(sendline)),
					ATTRIBUTE(ATTR_MESSAGE, strlen(sendline))}; 
				memcpy(MSG_buf, send_head, 8) ;
				memcpy(MSG_buf+8, sendline, strlen(sendline)) ;
				Writen(sockfd, MSG_buf, 8+strlen(sendline));
			}else{
				err_quit("Error read from file %d", fpfd ) ;
			}	
		}
	
		
	}

}

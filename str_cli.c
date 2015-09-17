#include "commons.h"
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>



extern void Writen(int fd, void *ptr, size_t nbytes);
extern void err_quit(const char *fmt, ...);
ssize_t Readline(int fd, void *ptr, size_t maxlen) ;

void str_cli(FILE *fp, int sockfd){
	char	sendline[MAXLINE], recvline[MAXLINE], MSG_buf[MAXLINE+8];
	fd_set rset;
	int nready, fpfd = fileno(fp)  ;
	int maxfd = (fpfd > sockfd ? fpfd : sockfd);

	
	
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset) ;
	FD_SET(fpfd, &rset) ;
	

	for(;;){
		fd_set rrset = rset ;
		nready = select( maxfd + 1, &rrset, NULL , NULL, NULL);
		fprintf(stderr,"str_cli: %dready\n", nready);
		if(FD_ISSET(sockfd, &rrset)){
			/*read something from server */
			int n;
			if ((n = recv(sockfd, recvline, MAXLINE, 0)) == 0){
					err_quit("Server down");
			}else{

			int32_t *int_buf = (int32_t *)recvline ;
			int32_t header = ntohl(int_buf[0]) ;
			int version =  (int)( (header & 0xff800000) >> 25 ) ,
			type = (int)( (header & 0x007f0000) >> 16 ) ,
			length = (int)  (header & 0x0000ffff);

			int32_t attr = ntohl(int_buf[1] ) ;
			int first_attr_type = (attr & 0xffff0000) >> 16 ,
				first_attr_length = (attr & 0x0000ffff) ;

			fprintf(stderr, " Recv %d bytes\n", n);
			switch(type){

			case HEADER_FWD:{
			}break;
	
			case HEADER_ACK:{
			
			fprintf(stderr, "  ACK recv\n");
			if(n >= 10 && first_attr_type == ATTR_CLIENT_COUNT){
				int16_t  client_count = ntohs(  *(int16_t *)(int_buf + 2) ) ;
				fprintf(stdout, "client_count: %d\n",client_count);
			}else{

			}
			}break ;

			default:
			fputs(recvline, stdout);
			break ;

			}
			if(--nready == 0){continue;}
			}
		}
	
		if(FD_ISSET(fpfd, &rrset) ){
			/*read something to send to server */
			if (fgets(sendline, MAXLINE, fp) != NULL) {
				int32_t send_head[2] = {HEADER(HEADER_SEND, 4 + strlen(sendline)),
					ATTRIBUTE(ATTR_MESSAGE, strlen(sendline))}; 
				fprintf(stderr, "Got something from stdin\n"); 
				memcpy(MSG_buf, send_head, 8) ;
				memcpy(MSG_buf+8, sendline, strlen(sendline)) ;
				Writen(sockfd, MSG_buf, 8+strlen(sendline));
			}else{
				err_quit("Error read from file %d", fpfd ) ;
			}	
			if(--nready == 0){continue;}
		}
	
		
	}

}

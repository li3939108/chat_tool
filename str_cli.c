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

			switch(type){

			case HEADER_OFFLINE:
			case HEADER_ONLINE:{

			if(n >= 9 && first_attr_type == ATTR_USERNAME){
				int i = 0, position = 8 ;
				char username[SIZE_ATTR_USERNAME + 1] ;
				memcpy(username, recvline + position, first_attr_length) ;
				username[first_attr_length] = '\0' ;
				if(type == HEADER_ONLINE){
					fprintf( stdout, "\e[1;32m%s\e[0m is ONLINE\n", username) ;
				}else if(type == HEADER_OFFLINE){
					fprintf( stdout, "\e[0;31m%s\e[0m is OFFLINE\n", username) ;
				}
				position += first_attr_length ;
			}else{ 
				fprintf(stderr, "Incomplete OFFLINE or ONLINE\n") ;
				break ;
			}

			}
			break;

			case HEADER_FWD:{
			
			if(n >= 9 && first_attr_type == ATTR_MESSAGE){
				int i = 0, position = 8;
				char msg[SIZE_ATTR_MESSAGE + 1] ;
				char username[SIZE_ATTR_USERNAME + 1] ;

				int32_t attr  ;
				int attr_type, attr_length ;
				memcpy(msg, recvline + position, first_attr_length) ;
				msg[first_attr_length] = '\0';
				position += first_attr_length ;
				attr = ntohl( *(int32_t *)(recvline + position) ) ;
				attr_type = (attr & 0xffff0000) >> 16; 
				attr_length = (attr & 0x0000ffff) ;
				if(n >= position + 4 && attr_type == ATTR_USERNAME){
					position += 4;
					memcpy(username, recvline + position, attr_length) ;
					username[attr_length] = '\0' ;
					fprintf( stdout, "\e[1;97m%s\e[0m: %s", username, msg) ;
					position += attr_length ;
				}
			}

			}break;
	
			case HEADER_ACK:{
			
			if(n >= 10 && first_attr_type == ATTR_CLIENT_COUNT){
				int16_t  client_count = ntohs(  *(int16_t *)(int_buf + 2) ) ;
				int i = 0, position = 10;
				if( client_count > 1){
					fprintf(stdout, "Talk with %d people:\n", client_count - 1) ;
				}else{
					fprintf(stdout, "No one else is online\n") ;
				}
				for(i = 1; i < client_count ; i++){
					int32_t attr = ntohl( *(int32_t*)(recvline + position) ) ;
					int attr_type = (attr & 0xffff0000) >> 16 ,
						attr_length = (attr & 0x0000ffff) ;
					if(n >= position + 4 && attr_type == ATTR_USERNAME){
						char username[SIZE_ATTR_USERNAME + 1] ;
						position += 4;
						memcpy(username, recvline + position, attr_length) ;
						username[attr_length] = '\0' ;
						fprintf( stdout, "\e[0;97m%s\e[0m\n", username) ;
						position += attr_length ;
					}else{ 
						fprintf(stderr, "Incomplete ACK\n") ;
						break ;
					}
				}
				fprintf(stdout, "Start chattng\n-------------\n");
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
				memcpy(MSG_buf, send_head, 8) ;
				memcpy(MSG_buf+8, sendline, strlen(sendline)) ;
				send(sockfd, MSG_buf, 8+strlen(sendline), 0);
			}else{
				err_quit("Error read from file %d", fpfd ) ;
			}	
			if(--nready == 0){continue;}
		}
		
		fflush(stdout);
		fflush(stderr);
		
	}

}

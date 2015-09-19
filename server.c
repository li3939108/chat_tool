#include <sys/types.h>	  
#include "commons.h"
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdlib.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <search.h>

#define NOTDEF

extern void Writen(int fd, void *ptr, size_t n) ;
extern void err_quit(const char *fmt, ...);
char buf[MAXLINE], wbuf[MAXLINE];
void msg_FWD(
		int maxi,
		int client_count, 
		int sender_index,
		char client_username[][SIZE_ATTR_USERNAME + 1],
		int client[],
		int client_status[],
		char msgbuf[]){
	
	int32_t head = HEADER(HEADER_FWD, 4+strlen(msgbuf) +4+strlen( client_username[sender_index] ) ) ;
	int32_t attr_msg =  ATTRIBUTE(ATTR_MESSAGE, strlen(msgbuf) ) ;
	int32_t attr_username = ATTRIBUTE(ATTR_USERNAME, strlen( client_username[sender_index] ) ) ;
	int position = 0, i = 0;
	memcpy(wbuf + position, &head, 4) ;position += 4;
	memcpy(wbuf+position, &attr_msg, 4);position += 4;
	memcpy(wbuf+position, msgbuf, strlen(msgbuf) );position += strlen(msgbuf) ;
	memcpy(wbuf+position, &attr_username, 4 ); position += 4; 
	memcpy(wbuf+position, client_username[sender_index], strlen(client_username[sender_index] ) ) ;
	position += strlen(client_username[sender_index] ) ;
	for(i = 0;i<=maxi; i++){
		if( i != sender_index && client_status[i] > 0){
			if(0 == send(client[i], wbuf, position, 0) ){
				fprintf(stderr, "Error sending msg %s to client %d\n", msgbuf, i) ;
			}
		}
	}
}

int msg_ACK(
		int fd, 
		int maxi, 
		int client_count, 
		int requestor_index, 
		char client_username[][SIZE_ATTR_USERNAME + 1], 
		int client_status[]){
	int32_t head, attr_client_count = ( ATTRIBUTE(ATTR_CLIENT_COUNT, 2)) ;
	int32_t attrs_username[client_count] ;
	char usernames[client_count][16] ;
	int i, ct=0, position ;
	short network_client_count = htons( (short) client_count) ;
	for(i =0 ;i <= maxi; i++){
		if(ct == client_count){break;}
		if(client_status[i] > 0 && i != requestor_index){
			attrs_username[ct] = ( ATTRIBUTE(ATTR_USERNAME, strlen(client_username[i] )) );
			strcpy(usernames[ct], client_username[i] ) ;
			ct ++ ;
		}
	}
	
	memcpy(wbuf+4, &attr_client_count, 4) ;
	memcpy(wbuf+8, &network_client_count, 2) ;
	position = 10 ;
	for(i = 0; i < ct;i++){
		memcpy(wbuf+position, attrs_username+i, 4);
		position += 4; 
		memcpy(wbuf+position, usernames[i], strlen(usernames[i] ) ) ;
		position += strlen(usernames[i] ) ;
	}
	head = ( HEADER( HEADER_ACK, position - 4) ) ;
	memcpy(wbuf, &head, 4) ;
	fprintf(stderr, "%d bytes sent\n", position); 
	return send(fd, wbuf, position, 0) ;
}

int main(int argc, char **argv){
	int i, maxi, maxfd, listenfd, connfd, sockfd, max_number_of_clients, client_count = 0;
	int nready, client[FD_SETSIZE], client_status[FD_SETSIZE];
	ssize_t n;
	fd_set rset, allset;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	char response_buf[MAXLINE];
	char client_username[FD_SETSIZE][1+SIZE_ATTR_USERNAME] ;


	if(argc == 4){
		listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		int return_value = inet_pton(servaddr.sin_family, argv[1], &servaddr.sin_addr), port ; 
		if(return_value == 1){
			/*servaddr.sin_addr.s_addr = htonl(INADDR_ANY);*/
		}else if(return_value == 0){ err_quit("Illegal IP address: %s", argv[1]) ;
		}else{ err_quit("Unknow error : IP address") ; }
		port = strtol (argv[2], NULL, 0)  ;
		if(port < 1024){ err_quit("%d is too small, Please assign a larger port number", port) ; 
		}else{ servaddr.sin_port = htons(port);} 
		max_number_of_clients = strtol(argv[3], NULL, 0) ;
		if(max_number_of_clients <= 0){ err_quit("Max number of clients should be a positive integer.") ;
		}else{
			hcreate(max_number_of_clients) ;
		}
	}else{
		err_quit("Usage: ./server server_ip server_port max_clients") ;
	}

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;				/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++){
		client[i] = -1;		
		client_status[i] = -1 ;
	}	/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; ) {
		rset = allset;		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			if(client_count == max_number_of_clients){
				/* max clients */
				fprintf(stderr, "Number of clients reaches limit\n" );
				continue ;
			}

			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (SA *) &cliaddr, &clilen);
#ifdef	NOTDEF
			printf("new client port %d\n", ntohs(cliaddr.sin_port));
#endif

			for (i = 0; i < FD_SETSIZE; i++){
				if (client_status[i] <= 0) {
					if( (sockfd = client[i]) > 0) {
						close(sockfd);
						FD_CLR(sockfd, &allset);
						client[i] = -1;
						client_status[i] = CLIENT_STATUS_OFFLINE ;
						fprintf(stderr, "client didn't JOIN, %d closed\n", sockfd) ;
					}
					client[i] = connfd;	/* save descriptor */
					client_status[i] = CLIENT_STATUS_CONNECTED ;     /* connected but not joined yet */
					break;
				}
			}
			if (i == FD_SETSIZE  ){
				err_quit("too many clients");
			}
			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd){
				maxfd = connfd;			/* for select */
			}
			if (i > maxi){
				maxi = i;				/* max index in client[] array */
			}
			if (--nready <= 0){
				continue;				/* no more readable descriptors */
			}
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0){
				continue;}
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = recv(sockfd, buf, MAXLINE, 0)) < 4) {
					/*4connection closed by client */
					/* client exits */
					close(sockfd); FD_CLR(sockfd, &allset);
					if(client_status[i] > 0){ client_count -= 1;}
					client[i] = -1;	client_status[i] = -1 ;
					fprintf(stderr, "Client %d closed\n", i) ;
				} else{
					int32_t *int_buf = (int32_t *)buf ;
					int32_t header = ntohl(int_buf[0]) ;
					int version =  (int)( (header & 0xff800000) >> 25 ) ,
					type = (int)( (header & 0x007f0000) >> 16 ) ,
					length = (int)  (header & 0x0000ffff);
					int32_t attr = ntohl(int_buf[1] ) ;
					int first_attr_type = (attr & 0xffff0000) >> 16 ,
						first_attr_length = (attr & 0x0000ffff) ;

					switch(type ){

					case HEADER_JOIN:{

					if( n >= 9 &&  first_attr_type == ATTR_USERNAME ){
						if(client_status[i] <= 0) {
							ENTRY et={NULL, NULL} , *e;
							strncpy(client_username[i], (char *)(int_buf+2), first_attr_length) ;
							client_username[i][first_attr_length] = '\0';
							et.key = client_username[i] ;
							if(NULL == hsearch(et, FIND) ){
								if(client_count == max_number_of_clients||NULL == hsearch(et, ENTER)){
									err_quit("clients full");
								}else{
									client_count += 1;
									client_status[i] = CLIENT_STATUS_JOINED ;
									fprintf(stdout, "Client %d JOIN\n", i) ;
									msg_ACK(sockfd, maxi, client_count, i, client_username, client_status) ;
								}
							}else{
								close(sockfd);FD_CLR(sockfd, &allset);
								if(client_status[i] > 0){ client_count -= 1;}
								client[i] = -1;client_status[i] = CLIENT_STATUS_OFFLINE ;
								fprintf(stdout, "Duplicate username: \
									%s has joined chat; client %d closed\n",client_username[i],i );
							}
						}else{
							fprintf(stderr, "Duplicate JOIN message from %s\n", client_username[i]) ;
						}
					}else{
						close(sockfd);FD_CLR(sockfd, &allset);
						if(client_status[i] > 0){ client_count -= 1;}
						client[i] = -1;client_status[i] = CLIENT_STATUS_OFFLINE ;
						fprintf(stderr, "Incomplete JOIN message\n" ) ;
						fprintf(stderr, "Client %d closed\n", i) ;
					}

					}
					break;
					
					case HEADER_SEND:{

					if(n >= 9 && first_attr_type == ATTR_MESSAGE){
						buf[8+first_attr_length] = '\0' ;
						fprintf(stdout, "Reved msg from client %d: %s", i, buf + 8) ;
						msg_FWD(maxi, client_count, i, client_username, client, client_status, buf + 8) ;
					}else{
						close(sockfd);FD_CLR(sockfd, &allset);
						if(client_status[i] > 0){ client_count -= 1;}
						client[i] = -1;client_status[i] = CLIENT_STATUS_OFFLINE ;
						fprintf(stderr, "Incomplete SEND message\n" ) ;
						fprintf(stderr, "client %d closed\n", i) ;
					}
					
					}
					break ;

					case HEADER_IDLE:
					break ;

					default:
					Writen(sockfd, buf, n);
					break ;
					}
				}
				if (--nready <= 0){
					break;				/* no more readable descriptors */
				}
			}
		}
	}
}

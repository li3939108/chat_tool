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

extern void err_quit(const char *fmt, ...);
extern void err_msg(const char *fmt, ...);

#include "msg.h"

static char buf[MAXLINE];

ENTRY *nullify_key(const char *key){
	ENTRY et={(char *)key, NULL}, *er ;
	er = hsearch(et, FIND) ;
	if(er == NULL ){return NULL;
	}else{
		er->data = NULL ;
		return er ;
	}
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
			hcreate(MAXLINE) ;
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
						fprintf(stderr, "socket client didn't JOIN, %d closed\n", sockfd) ;
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
check_for_data:
		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0){
				continue;}
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = recv(sockfd, buf, MAXLINE, 0)) < 4) {
					/*4connection closed by client */
					/* client exits */
					close(sockfd); FD_CLR(sockfd, &allset);
					if(client_status[i] > 0){ 
						client_count -= 1;
						nullify_key(client_username[i]);
						msg_ON_OFF_LINE(maxi, client_count, i, client_username, client, client_status, HEADER_OFFLINE) ;
					}
					client[i] = -1;	client_status[i] = CLIENT_STATUS_OFFLINE ;
					
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
							ENTRY et={NULL, (void *)-1} , *er;
							strncpy(client_username[i], (char *)(int_buf+2), first_attr_length) ;
							client_username[i][first_attr_length] = '\0'; et.key = client_username[i] ;
							if(NULL == (er = hsearch(et, FIND) ) || er->data == NULL){
								if(client_count == max_number_of_clients){
									msg_NAK(i, client, "Clients full") ;
									err_msg("clients full");
								}else{
									er = hsearch(et, ENTER);er->data = (void*)-1;
									client_count += 1;
									client_status[i] = CLIENT_STATUS_JOINED ;
									msg_ON_OFF_LINE(maxi, client_count, i, client_username, client, client_status, HEADER_ONLINE) ;
									fprintf(stdout, "Client %d JOIN\n", i) ;
									msg_ACK(sockfd, maxi, client_count, i, client_username, client_status) ;
								}
							}else{
								msg_NAK(i, client, "Duplicate username") ;
								close(sockfd);FD_CLR(sockfd, &allset);
								if(client_status[i] > 0){ client_count -= 1;nullify_key(client_username[i]);}
								client[i] = -1;client_status[i] = CLIENT_STATUS_OFFLINE ;
								fprintf(stdout, "Duplicate username: \
									%s has joined chat; client %d closed\n",client_username[i],i );
							}
						}else{
							fprintf(stderr, "Duplicate JOIN message from %s\n", client_username[i]) ;
						}
					}else{
						close(sockfd);FD_CLR(sockfd, &allset);
						if(client_status[i] > 0){ client_count -= 1;nullify_key(client_username[i]);}
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
						if(client_status[i] > 0){ client_count -= 1;nullify_key(client_username[i]);}
						client[i] = -1;client_status[i] = CLIENT_STATUS_OFFLINE ;
						fprintf(stderr, "Incomplete SEND message\n" ) ;
						fprintf(stderr, "client %d closed\n", i) ;
					}
					
					}
					break ;

					case HEADER_IDLE:
					break ;

					default:
					write(sockfd, buf, n);
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

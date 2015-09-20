#include "commons.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

static char wbuf[MAXLINE] ;
int msg_JOIN(char username[SIZE_ATTR_USERNAME+1], int sockfd){
	int username_len = strlen(username) , i = 0;
	int32_t join_head[2] = {HEADER(HEADER_JOIN, 4+strlen(username)) , 
		ATTRIBUTE(ATTR_USERNAME, strlen(username))};
	char JOIN_buf[24];
	memcpy(JOIN_buf, (char *)join_head, sizeof join_head) ;
	memcpy(JOIN_buf+8, username, strlen(username) );
	return send(sockfd, JOIN_buf, 8 + strlen(username), 0) ;
}
			
void msg_NAK(
		int index,
		int client[], 
		char reason[]){
	int32_t head = HEADER(HEADER_NAK,  4+strlen( reason ) ) ;
	int32_t attr = ATTRIBUTE(ATTR_REASON, strlen(reason)) ;
	memcpy(wbuf, &head, 4) ;memcpy(wbuf+4, &attr, 4) ;
	memcpy(wbuf+8, reason, strlen(reason) ) ;
	if(0 == send(client[index], wbuf, 8+strlen(reason), 0) ){
		fprintf(stderr, "Error sending to client %d\n", index) ;
	}
	
}
void msg_ON_OFF_LINE(
		int maxi,
		int client_count, 
		int index,
		char client_username[][SIZE_ATTR_USERNAME + 1],
		int client[],
		int client_status[],
		int HEADER_ON_OFF_LINE){
	int32_t head = HEADER(HEADER_ON_OFF_LINE,  4+strlen( client_username[index] ) ) ;
	int32_t attr_username = ATTRIBUTE(ATTR_USERNAME, strlen( client_username[index] ) ) ;
	int position = 0, i = 0;
	memcpy(wbuf + position, &head, 4) ;position += 4;
	memcpy(wbuf+position, &attr_username, 4 ); position += 4; 
	memcpy(wbuf+position, client_username[index], strlen(client_username[index] ) ) ;
	position += strlen(client_username[index] ) ;
	
	for(i = 0;i<=maxi; i++){
		if( i != index && client_status[i] > 0){
			if(0 == send(client[i], wbuf, position, 0) ){
				fprintf(stderr, "Error sending to client %d\n", i) ;
			}
		}
	}
}
void msg_FWD(
		int maxi,
		int client_count, 
		int index,
		char client_username[][SIZE_ATTR_USERNAME + 1],
		int client[],
		int client_status[],
		char msgbuf[]){
	
	int32_t head = HEADER(HEADER_FWD, 4+strlen(msgbuf) +4+strlen( client_username[index] ) ) ;
	int32_t attr_msg =  ATTRIBUTE(ATTR_MESSAGE, strlen(msgbuf) ) ;
	int32_t attr_username = ATTRIBUTE(ATTR_USERNAME, strlen( client_username[index] ) ) ;
	int position = 0, i = 0;
	memcpy(wbuf + position, &head, 4) ;position += 4;
	memcpy(wbuf+position, &attr_msg, 4);position += 4;
	memcpy(wbuf+position, msgbuf, strlen(msgbuf) );position += strlen(msgbuf) ;
	memcpy(wbuf+position, &attr_username, 4 ); position += 4; 
	memcpy(wbuf+position, client_username[index], strlen(client_username[index] ) ) ;
	position += strlen(client_username[index] ) ;
	for(i = 0;i<=maxi; i++){
		if( i != index && client_status[i] > 0){
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

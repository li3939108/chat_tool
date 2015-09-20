#ifndef __MSG_H__
#define __MSG_H__


#include "commons.h"
extern void msg_JOIN(char username[SIZE_ATTR_USERNAME+1], int sockfd);
extern void msg_NAK(	int index,int client[], char reason[]);
extern void msg_ON_OFF_LINE(
		int maxi,
		int client_count, 
		int index,
		char client_username[][SIZE_ATTR_USERNAME + 1],
		int client[],
		int client_status[],
		int HEADER_ON_OFF_LINE);
void msg_FWD(
		int maxi,
		int client_count, 
		int index,
		char client_username[][SIZE_ATTR_USERNAME + 1],
		int client[],
		int client_status[],
		char msgbuf[]);
int msg_ACK(
		int fd, 
		int maxi, 
		int client_count, 
		int requestor_index, 
		char client_username[][SIZE_ATTR_USERNAME + 1], 
		int client_status[]);
#endif

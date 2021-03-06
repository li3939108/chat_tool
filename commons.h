#ifndef __COMMONS_H__
#define __COMMONS_H__

#define SERV_PORT 9877
#define MAXLINE 1024
#define LISTENQ 1024

#define VERSION 3

#define HEADER_JOIN 2
#define HEADER_FWD 3
#define HEADER_SEND 4
#define HEADER_NAK 5
#define HEADER_OFFLINE 6
#define HEADER_ACK 7
#define HEADER_ONLINE 8
#define HEADER_IDLE 9


#define ATTR_REASON 1
#define ATTR_USERNAME 2
#define ATTR_CLIENT_COUNT 3
#define ATTR_MESSAGE 4

#include <arpa/inet.h>
#define HEADER(TYPE, LENGTH) htonl( ((VERSION)  << 25) | ((TYPE) << 16) | (LENGTH) )
#define ATTRIBUTE(TYPE, LENGTH) htonl( ((TYPE) << 16 )| (LENGTH) )

#define SIZE_ATTR_REASON 32
#define SIZE_ATTR_USERNAME 16
#define SIZE_ATTR_CLIENT_COUNT 2
#define SIZE_ATTR_MESSAGE 512

#define CLIENT_STATUS_OFFLINE -1
#define CLIENT_STATUS_CONNECTED 0
#define CLIENT_STATUS_JOINED 1
#define CLIENT_STATUS_IDLE 2

typedef struct sockaddr SA ;
#endif

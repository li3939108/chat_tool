#include <unistd.h>
#include <errno.h>
#include "commons.h"

extern void  err_sys(const char *fmt, ...);
extern void Writen(int fd, void *ptr, size_t n) ;

void str_echo(int sockfd){
	ssize_t		n;
	char		buf[MAXLINE];

again:
	while ( (n = read(sockfd, buf, MAXLINE)) > 0){
		Writen(sockfd, buf, n); 
	}
	if (n < 0 && errno == EINTR){
		goto again;
	}else if (n < 0){
		err_sys("str_echo: read error");
	}
}

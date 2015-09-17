#include "commons.h"
#include <stdio.h>
#include <string.h>

extern void Writen(int fd, void *ptr, size_t nbytes);
extern void err_quit(const char *fmt, ...);
ssize_t Readline(int fd, void *ptr, size_t maxlen) ;

void str_cli(FILE *fp, int sockfd){
	char	sendline[MAXLINE], recvline[MAXLINE];
	while (fgets(sendline, MAXLINE, fp) != NULL) {
		Writen(sockfd, sendline, strlen(sendline));
		if (Readline(sockfd, recvline, MAXLINE) == 0){
			err_quit("str_cli: server terminated prematurely");
		}
		fputs(recvline, stdout);
	}
}

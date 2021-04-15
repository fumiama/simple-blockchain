#include "client.h"

int sockfd;
char buf[CLISTATSZ];
struct sockaddr_in their_addr;
pthread_t thread;

int main(int argc,char *argv[]) {   //usage: ./client host port
	ssize_t numbytes;
	puts("break!");
	while((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1);
	puts("Get sockfd");
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(atoi(argv[2]));
	their_addr.sin_addr.s_addr=inet_addr(argv[1]);
	bzero(&(their_addr.sin_zero), 8);
	
	while(connect(sockfd,(struct sockaddr*)&their_addr,sizeof(struct sockaddr)) == -1);
	puts("Connected to server");
	numbytes = recv(sockfd, buf, BUFSIZ,0);
	buf[numbytes]='\0';  
    buf[numbytes]='\0';  
	buf[numbytes]='\0';
	puts(buf);
	BLOCK* blk = buf;
	while(1) {
		numbytes = recv(sockfd, buf, BUFSIZ, 0);
		printf("Get block data, n1: %d.\n", blk->n1);
		scan_n2(blk);
		sleep(1);
	}
	close(sockfd);
	return 0;
}

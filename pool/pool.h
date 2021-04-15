#ifndef _POOL_H
#define _POOL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "../bitblock/bitblock.h"
#include "../chain/chain.h"

#define CLISZ 64

#define CMD_CLI_WAIT	0	//等待任务
#define CMD_CLI_START	1	//开始任务
#define CMD_CLI_ENDCON	2	//终止连接
#define CMD_CLI_FINTSK	3	//任务完成

struct CLISTAT {
	int accept_fd;
	pthread_t thread;
	uint64_t n1, n2;
	uint16_t zerobit_cnt;
	int cmd;
	char* data;
	ssize_t numbytes;
	BLOCK blk;
};
typedef struct CLISTAT CLISTAT;
#define CLISTATSZ (BLKSZ+2*sizeof(int)+sizeof(pthread_t)+2*sizeof(uint64_t)+sizeof(uint16_t)+sizeof(char*)+sizeof(ssize_t))

socklen_t struct_len = sizeof(struct sockaddr_in);
struct sockaddr_in server_addr;

#endif
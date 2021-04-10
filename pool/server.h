#ifndef _POOL_SERVER_H
#define _POOL_SERVER_H

#include "pool.h"
#include "../bitblock/bitblock.h"
#include "../chain/chain.h"

#define CLISZ 64

#define CMD_CLI_WAIT	0	//等待任务
#define CMD_CLI_START	1	//开始任务
#define CMD_CLI_ENDCON	2	//终止连接
#define CMD_CLI_FINTSK	3	//任务完成

struct BLKCHN {
	BLOCK* blk;
	struct BLKCHN* next;
};
typedef struct BLKCHN BLKCHN;

struct BLKCHNHEAD {
	BLKCHN* head;
	uint32_t len;
};
typedef struct BLKCHNHEAD BLKCHNHEAD;

struct CLISTAT {
	int accept_fd;
	pthread_t thread;
	uint64_t n1, n2;
	uint16_t zerobit_cnt;
	int cmd;
	BLOCK* blk;
	char* data;
	ssize_t numbytes;
};
typedef struct CLISTAT CLISTAT;

socklen_t struct_len = sizeof(struct sockaddr_in);
struct sockaddr_in server_addr;

#endif
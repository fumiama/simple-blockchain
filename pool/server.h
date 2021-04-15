#ifndef _POOL_SERVER_H
#define _POOL_SERVER_H

#include "pool.h"

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

#endif
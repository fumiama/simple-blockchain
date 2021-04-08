#include <stdint.h>
#include "../bitblock/bitblock.h"

//记录转账交易 73bytes
struct TRANS {
	uint8_t from_addr[256/8+1];
	uint8_t to_addr[256/8];
	uint64_t amount;
};
typedef struct TRANS TRANS;
union TRANSDAT {
	TRANS t;
	uint8_t data[sizeof(TRANS)];
};
typedef union TRANSDAT TRANSDAT;
//经过签名的转账交易数据块 137bytes
struct TRANSV {
	uint8_t ecc[512/8];
	TRANSDAT td;
};
typedef struct TRANSV TRANSV;
#define MAXTRANCNT (DATSZ/sizeof(TRANSV))	//data区域最大容纳交易数 6
//总的转账数据区块
struct COINDAT {
	TRANSV tv[MAXTRANCNT];
	uint32_t trans_cnt;		//现有交易数
	uint8_t remain[];		//剩余空间 54bytes
};
typedef struct COINDAT COINDAT;
#define MINEAMT 1024		//挖矿可获得的coin数
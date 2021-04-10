#include <stdint.h>
#include "../bitblock/bitblock.h"

//记录转账交易 74bytes
struct TRANS {
	uint8_t from_addr[256/8+1];
	uint8_t to_addr[256/8+1];
	uint64_t amount;
};
typedef struct TRANS TRANS;

//经过签名的转账交易数据块 138bytes
struct TRANSV {
	uint8_t ecc[512/8];
	TRANS ts;
};
typedef struct TRANSV TRANSV;
#define MAXTRANCNT (DATSZ/sizeof(TRANSV))	//data区域最大容纳交易数 6
//总的转账数据区块
struct COINDAT {
	TRANSV tv[MAXTRANCNT];
	uint32_t trans_cnt;		//现有交易数
	uint8_t remain[];		//剩余空间 53bytes
};
typedef struct COINDAT COINDAT;
#define MINEAMT 1024		//挖矿可获得的coin数

//进行转账交易
TRANSV* transact(const uint8_t* from_addr, const uint8_t* to_addr, const uint64_t amount, const uint8_t *p_privateKey);
//生成一笔挖矿收益转账
TRANSV* mine(const uint8_t* miner_pub_key, const uint8_t* miner_priv_key);
//创建一个转账data块，可存放多条转账
COINDAT* create_data();
//创建一个挖好矿(已添加全0转账)的data块
COINDAT* new_mined_data(const uint8_t* miner_pub_key, const uint8_t* miner_priv_key);
//向data添加一个转账交易数据块 成功返回当前交易数 错误返回EOF
int add_trans(COINDAT* cd, const TRANSV* tv);
//data块签名校验+挖矿收益验证
int check_ecc(const COINDAT* data);
//将转账data块加入区块链
void add_trans_chain(COINDAT** data, const uint8_t* miner_pub_key, const uint8_t* miner_priv_key, uint8_t* prev_hash);
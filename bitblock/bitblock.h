#ifndef _BITBLOCK_H
#define _BITBLOCK_H

#include <stdint.h>

struct BLOCK {
	uint64_t version;
	uint8_t prev_hash[256/8];		//前一个区块的hash
	uint8_t log_addr[256/8+1];		//记录者地址
	uint64_t n1;
	uint64_t n2;
	uint8_t dataecc[512/8];			//数据hash的签名
	uint8_t data[];					//任意数据
};									//块大小 1k
typedef struct BLOCK BLOCK;
#define BLKSZ 1024
#define DATSZ (BLKSZ-(64+256+256+64+64+512)/8)  //数据区880字节

uint8_t public_key_all_zero[33];

//包装并计算哈希
BLOCK* wrap_block(const uint64_t version, const uint16_t zerobit_cnt, const uint8_t* log_addr, const uint8_t* prev_hash, const uint8_t* p_privateKey, const char* data, uint32_t data_len);
//包装但不计算哈希
BLOCK* wrap_block_no_nounce(const uint64_t version, const uint8_t* log_addr, const uint8_t* prev_hash, const uint8_t* p_privateKey, const char* data, uint32_t data_len);

#endif
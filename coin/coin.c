#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coin.h"
#include "../SHA256/mark2/sha256.h"
#include "../ecc/ecc.h"

//进行转账交易
TRANSV* transact(const uint8_t* from_addr, const uint8_t* to_addr, const uint64_t amount, const uint8_t *p_privateKey) {
	TRANSV* t = malloc(sizeof(TRANSV));
	memcpy(t->t.t.from_addr, from_addr, 256/8+1);
	memcpy(t->t.t.to_addr, to_addr, 256/8);
	uint8_t *digest = malloc(256/8);
	sha256(t->t.data, sizeof(TRANSDAT), digest);
	ecdsa_sign(p_privateKey, digest, t->ecc);
	t->t.t.amount = amount;
	free(digest);
	return t;
}

//创建一个转账data块，可存放多条转账
COINDAT* create_data() {
	COINDAT* c = malloc(DATSZ);
	c->trans_cnt = 0;
	return c;
}

//向data添加一个转账交易数据块 成功返回当前交易数 错误返回EOF
int add_trans(COINDAT* cd, const TRANSV* tv) {
	if(cd->trans_cnt < MAXTRANCNT) {
		memcpy(&(cd->t[cd->trans_cnt]), tv, sizeof(TRANSV));
		return cd->trans_cnt++;
	} else return EOF;
}


#include "../chain/chain.h"
#include "../bitblock/bitblock.h"
#define printhash(x, bytes) for(int i = 0; i < (bytes); i++) printf("%02x", (x)[i])
uint8_t bitpool[ECC_BYTES];
uint8_t priv_key[ECC_BYTES];
uint8_t pub_key[ECC_BYTES+1];
int main() {
	COINDAT* data = NULL;
	if(access("block", 0)) {
        mkdir("block", 0755);
        ecc_make_key(pub_key, priv_key);
        printf("Generated miner pub key: ");
        printhash(pub_key, ECC_BYTES+1);
        putchar('\n');
        printf("Generated miner priv key: ");
        printhash(priv_key, ECC_BYTES);
        putchar('\n');
        make_new_chain("block", priv_key, 8);
		data = create_data();
		
        BLOCK* blk = wrap_block(0, 12, pub_key, bitpool, priv_key, data, DATSZ);
        append_chain("block", blk);
        free(blk);
    }
}
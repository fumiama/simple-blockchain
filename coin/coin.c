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

#ifdef SELF_TEST_COIN
#include <unistd.h>
#include "../chain/chain.h"
#define printhash(x, bytes) for(int i = 0; i < (bytes); i++) printf("%02x", (x)[i])
uint8_t prev_hash[ECC_BYTES];	//前一个块的hash
uint8_t priv_key[ECC_BYTES];	//矿机私钥
uint8_t pub_key[ECC_BYTES+1];	//矿机地址
uint8_t zero_pub[ECC_BYTES+1];	//全0地址
COINDAT* data = NULL;			//数据块指针
TRANSV* mine() {
	return transact(zero_pub, pub_key, MINEAMT, priv_key);		//挖矿奖励
}
COINDAT* new_mined_data() {
	COINDAT* data = NULL;
	TRANSV* tv = NULL;
	data = create_data();
	tv = mine();
	add_trans(data, tv);	//增加挖矿奖励
	free(tv);
	return data;
}
void miner_trans_to(const char* name, const uint64_t amount) {
	TRANSV* tv = NULL;
	uint8_t priv_key_acc[ECC_BYTES];	//私钥
	uint8_t pub_key_acc[ECC_BYTES+1];	//地址
	ecc_make_key(pub_key_acc, priv_key_acc);
	printf("Generated %s's pub key: ", name);
    printhash(pub_key_acc, ECC_BYTES+1);
    putchar('\n');
    printf("Generated %s's priv key: ", name);
    printhash(priv_key_acc, ECC_BYTES);
    putchar('\n');
	transact(pub_key, pub_key_acc, amount, priv_key);	//转amount个coin
	if(!~add_trans(data, tv)) {		//如果data已满
		BLOCK* blk = wrap_block(0, 8, pub_key, prev_hash, priv_key, data, DATSZ);
		sha256(blk, BLKSZ, prev_hash);
		append_chain("block", blk);
		free(blk);
		free(data);
		data = new_mined_data();
	}
	putchar('\n');
}
int main() {
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
    }
	data = new_mined_data();	//新建一个记录块
	miner_trans_to("aska", 16);
	miner_trans_to("boy", 1);
	miner_trans_to("chika", 3);
	miner_trans_to("doll", 7);
	miner_trans_to("emily", 4);
	miner_trans_to("fault", 23);
	miner_trans_to("greet", 99);
	miner_trans_to("haru", 233);
	miner_trans_to("izumi", 11);
	miner_trans_to("juice", 6);
	miner_trans_to("kato", 39);
	miner_trans_to("lumia", 2);
	miner_trans_to("minazuki", 137);
	miner_trans_to("node", 46);
	miner_trans_to("ono", 83);
}
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coin.h"
#include "../SHA256/mark2/sha256.h"
#include "../ecc/ecc.h"

//#define SELF_TEST_COIN

uint8_t zero_pub[ECC_BYTES+1];	//全0地址
#define printhash(x, bytes) for(int i = 0; i < (bytes); i++) printf("%02x", (x)[i])

//进行转账交易
TRANSV* transact(const uint8_t* from_addr, const uint8_t* to_addr, const uint64_t amount, const uint8_t *p_privateKey) {
	TRANSV* t = malloc(sizeof(TRANSV));
	memcpy(t->ts.from_addr, from_addr, 256/8+1);
	memcpy(t->ts.to_addr, to_addr, 256/8+1);
	t->ts.amount = amount;
	#ifdef SELF_TEST_COIN
		printf("Trans from: ");
		printhash(t->ts.from_addr, 256/8+1);
		putchar('\n');
		printf("Trans to: ");
		printhash(t->ts.to_addr, 256/8+1);
		putchar('\n');
	#endif
	uint8_t *digest = malloc(256/8);
	sha256(&(t->ts), sizeof(TRANS), digest);
	#ifdef SELF_TEST_COIN
		printf("Digest: ");
		printhash(digest, 256/8);
		putchar('\n');
	#endif
	if(!ecdsa_sign(p_privateKey, digest, t->ecc)) puts("ECC sign error.");
	free(digest);
	#ifdef SELF_TEST_COIN
		printf("ECC: ");
		printhash(t->ecc, 512/8);
		putchar('\n');
		puts("Transaction completed.\n");
	#endif
	return t;
}

TRANSV* mine(const uint8_t* miner_pub_key, const uint8_t* miner_priv_key) {
	return transact(zero_pub, miner_pub_key, MINEAMT, miner_priv_key);		//挖矿奖励
}

//创建一个转账data块，可存放多条转账
COINDAT* create_data() {
	COINDAT* c = malloc(DATSZ);
	c->trans_cnt = 0;
	return c;
}

//创建一个挖好矿(已添加全0转账)的data块
COINDAT* new_mined_data(const uint8_t* miner_pub_key, const uint8_t* miner_priv_key) {
	COINDAT* data = NULL;
	TRANSV* tv = NULL;
	data = create_data();
	tv = mine(miner_pub_key, miner_priv_key);
	add_trans(data, tv);	//增加挖矿奖励
	free(tv);
	return data;
}

//向data添加一个转账交易数据块 成功返回当前交易数 错误返回EOF
int add_trans(COINDAT* cd, const TRANSV* tv) {
	if(cd->trans_cnt < MAXTRANCNT) {
		memcpy(&(cd->tv[cd->trans_cnt]), tv, sizeof(TRANSV));
		#ifdef SELF_TEST_COIN
			printf("ECC when add trans: ");
			printhash(tv->ecc, 512/8);
			putchar('\n');
			printf("ECC after add trans: ");
			printhash(cd->tv[cd->trans_cnt].ecc, 512/8);
			putchar('\n');
			putchar('\n');
		#endif
		return cd->trans_cnt++;
	} else return EOF;
}

int check_ecc(const COINDAT* data) {
	int pass = 1;		//验证通过标志
	uint8_t* digest = malloc(256/8);
	TRANSV* tv;
	for(int i = 1; i < data->trans_cnt; i++) {	//第一个记录永远都是矿机收益，因此需要用to_addr验证，不在此验证
		tv = &(data->tv[i]);
		sha256(&(tv->ts), sizeof(TRANS), digest);
		#ifdef SELF_TEST_COIN
			printf("Digest of No.%d: ", i);
			printhash(digest, 256/8);
			putchar('\n');
			printf("ECC of No.%d: ", i);
			printhash(tv->ecc, 512/8);
			putchar('\n');
			printf("from_addr of No.%d: ", i);
			printhash(tv->ts.from_addr, 256/8);
			putchar('\n');
		#endif
		if(!ecdsa_verify(tv->ts.from_addr, digest, tv->ecc)) {
			printf("Verify data No.%d error.\n", i);
			pass = 0;
			break;
		}
	}
	sha256(&(data->tv[0].ts), sizeof(TRANS), digest);
	if(!ecdsa_verify(data->tv[0].ts.to_addr, digest, data->tv[0].ecc) || data->tv[0].ts.amount != MINEAMT) pass = 0;	//验证矿机收益
	free(digest);
	return pass;
}

void add_trans_chain(COINDAT** data, const uint8_t* miner_pub_key, const uint8_t* miner_priv_key, uint8_t* prev_hash) {
	if(check_ecc(*data)) {
		puts("ECC verify passed.");
		BLOCK* blk = wrap_block(0, 8, miner_pub_key, prev_hash, miner_priv_key, *data, DATSZ);
		sha256(blk, BLKSZ, prev_hash);
		append_chain("block", blk);
		free(blk);
		free(*data);
	} else puts("ECC verify failed.");
	*data = new_mined_data(miner_pub_key, miner_priv_key);
	puts("Add new block.");
}

#ifdef SELF_TEST_COIN
#include <unistd.h>
#include "../chain/chain.h"
uint8_t prev_hash[ECC_BYTES];	//前一个块的hash
uint8_t miner_priv_key[ECC_BYTES];	//矿机私钥
uint8_t miner_pub_key[ECC_BYTES+1];	//矿机地址
uint8_t priv_key_acc[ECC_BYTES];	//私钥
uint8_t pub_key_acc[ECC_BYTES+1];	//地址
COINDAT* data = NULL;			//数据块指针
void miner_trans_to(const char* name, const uint64_t amount) {
	TRANSV* tv = NULL;
	ecc_make_key(pub_key_acc, priv_key_acc);
	printf("Generated %s's pub key: ", name);
    printhash(pub_key_acc, ECC_BYTES+1);
    putchar('\n');
    printf("Generated %s's priv key: ", name);
    printhash(priv_key_acc, ECC_BYTES);
    putchar('\n');
	tv = transact(miner_pub_key, pub_key_acc, amount, miner_priv_key);	//转amount个coin
	if(!~add_trans(data, tv)) {		//如果data已满
		add_trans_chain(&data, miner_pub_key, miner_priv_key, prev_hash);
		add_trans(data, tv);
	}
	printf("ECC after add trans fun: ");
	printhash(data->tv[data->trans_cnt-1].ecc, 512/8);
	putchar('\n');
}
int main() {
	if(access("block", 0)) {
        mkdir("block", 0755);
        ecc_make_key(miner_pub_key, miner_priv_key);
        printf("Generated miner pub key: ");
        printhash(miner_pub_key, ECC_BYTES+1);
        putchar('\n');
        printf("Generated miner priv key: ");
        printhash(miner_priv_key, ECC_BYTES);
        putchar('\n');
        make_new_chain("block", miner_priv_key, 8);
		data = new_mined_data(miner_pub_key, miner_priv_key);	//新建一个记录块
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
		add_trans_chain(&data, miner_pub_key, miner_priv_key, prev_hash);
    }
}
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bitblock.h"
#include "../SHA256/mark2/sha256.h"
#include "../ecc/ecc.h"

static void gen_data_hash(BLOCK *blk, const uint8_t *p_privateKey) {
    uint8_t *digest = malloc(256/8);
    sha256(blk->data, DATSZ, digest);
    ecdsa_sign(p_privateKey, digest, blk->dataecc);
    free(digest);
}

static uint16_t zerobitsof(uint8_t *hash) {     //hash前多少比特为0
    uint16_t i;
    for(i = 0; i < 32; i++) if(hash[i]) break;
    uint8_t x = 8;
    uint8_t c = hash[i];
    while(x--) {
        c >>= 1;
        if(!c) break;
    }
    return i*8 + x;
}

int scan_n2(BLOCK *blk, uint16_t zerobit_cnt) {
    uint64_t i = 0;
    uint8_t *digest = malloc(256/8);
    int flag = 1;
    while(flag) {
        blk->n2 = i;
        sha256(blk, BLKSZ, digest);
        if(zerobit_cnt == zerobitsof(digest)) break;
        else if(i++ == UINT64_MAX) flag = 0;
    }
    free(digest);
    return flag;
}

static void scan_nounce(BLOCK *blk, uint16_t zerobit_cnt) {
    uint64_t i = 0;
    uint8_t *digest = malloc(256/8);
    while(1) {
        blk->n1 = i;
        if(scan_n2(blk, zerobit_cnt) || i++ == UINT64_MAX) break;
    }
    free(digest);
}

BLOCK* wrap_block_no_nounce(const uint64_t version, const uint8_t* log_addr, const uint8_t* prev_hash, const uint8_t* p_privateKey, const char* data, uint32_t data_len) {
    BLOCK* blk = (BLOCK*)malloc(BLKSZ);
    if(data_len > DATSZ) data_len = DATSZ;
    memset(blk->data, 0, DATSZ);
    memcpy(blk->data, data, data_len);
    gen_data_hash(blk, p_privateKey);
    blk->version = version;
    memcpy(blk->log_addr, log_addr, 256/8+1);
    memcpy(blk->prev_hash, prev_hash, 256/8);
    return blk;
}

BLOCK* wrap_block(const uint64_t version, const uint16_t zerobit_cnt, const uint8_t* log_addr, const uint8_t* prev_hash, const uint8_t* p_privateKey, const char* data, uint32_t data_len) {
    BLOCK* blk = wrap_block_no_nounce(version, log_addr, prev_hash, p_privateKey, data, data_len);
    scan_nounce(blk, zerobit_cnt);
    return blk;
}

#ifdef SELF_TEST_BITBLK
uint8_t digest[256/8];
static int save_blk(const char *__restrict__ __filename, BLOCK *blk) {
    FILE *fp = NULL;
    fp = fopen(__filename, "wb");
    if(fp) {
        if(fwrite(blk, BLKSZ, 1, fp) > 0) return !fclose(fp);
        else fclose(fp);
    }
    return 0;
}

static int read_blk(const char *__restrict__ __filename, BLOCK *blk) {
    FILE *fp = NULL;
    fp = fopen(__filename, "rb");
    if(fp) {
        if(fread(blk, BLKSZ, 1, fp) > 0) return !fclose(fp);
        else fclose(fp);
    }
    return 0;
}

#define printhash(x, bit) for(int i = 0; i < bit/8; i++) printf("%02x", x[i])
static BLOCK* blk;
static uint8_t priv_key_all_zero[ECC_BYTES+1];
int main() {
    blk = malloc(BLKSZ);
    printf("Size of a blk: %u\n", BLKSZ);
    sha256(blk, BLKSZ, digest);
    printf("Init hash: ");
    printhash(digest,256);
    putchar('\n');
    uint8_t count = 0;
    printf("How many bits do you want to be 0: ");
    scanf("%hhu", &count);
    gen_data_hash(blk, priv_key_all_zero);
    scan_nounce(blk, count);
    printf("When n1 is %llu, n2 is %llu, hash is ", blk->n1, blk->n2);
    sha256(blk, BLKSZ, digest);
    printhash(digest,256);
    putchar('\n');
    printf("Data ecc: ");
    printhash(blk->dataecc,512);
    putchar('\n');
    if(save_blk("blk0", blk)) puts("Save block success.");
    if(read_blk("blk0", blk)) puts("Read block success.");
    sha256(blk, BLKSZ, digest);
    printf("Hash from saved blk: ");
    printhash(digest,256);
    putchar('\n');
    printf("Data ecc: ");
    printhash(blk->dataecc,512);
    putchar('\n');
}
#endif

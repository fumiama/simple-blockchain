#include <stdio.h>
#include <string.h>
#include "bitblock.h"
#include "../SHA256/mark2/sha256.h"
#include "../ecc/ecc.h"

static uint8_t eccsignature[ECC_BYTES*2];

void gen_data_hash(BLOCK *blk, const uint8_t *p_privateKey) {
    sha256(blk->data, DATSZ, digest);
    ecdsa_sign(p_privateKey, digest, eccsignature);
    memcpy(blk->dataecc, eccsignature, ECC_BYTES*2);
}

uint8_t zerobyteof(uint8_t *hash) {     //hash前多少字节为0
    uint8_t i;
    for(i = 0; i < 32; i++) if(hash[i]) break;
    return i;
}

uint64_t scan_n1(BLOCK *blk, uint8_t zerobyte_cnt) {
    uint64_t i;
    for(i = 0; i <= UINT64_MAX; i++) {
        blk->n1 = i;
        sha256(blk, BLKSZ, digest);
        if(zerobyte_cnt == zerobyteof(digest)) break;
    }
    return i;
}

uint64_t scan_n2(BLOCK *blk, uint8_t zerobyte_cnt) {
    uint64_t i;
    for(i = 0; i <= UINT64_MAX; i++) {
        blk->n2 = i;
        sha256(blk, BLKSZ, digest);
        if(zerobyte_cnt == zerobyteof(digest)) break;
    }
    return i;
}

int save_blk(const char *__restrict__ __filename, BLOCK *blk) {
    FILE *fp = NULL;
    fp = fopen(__filename, "wb");
    if(fp && fwrite(blk, BLKSZ, 1, fp) > 0) {
        return !fclose(fp);
    } else return 0;
}

int read_blk(const char *__restrict__ __filename, BLOCK *blk) {
    FILE *fp = NULL;
    fp = fopen(__filename, "rb");
    if(fp && fread(blk, BLKSZ, 1, fp) > 0) {
        return !fclose(fp);
    } else return 0;
}

#ifdef SELF_TEST_BITBLK
#define printhash(x, bit) for(int i = 0; i < bit/8; i++) printf("%02x", x[i])
static BLOCK blk;
static uint8_t priv_key_all_zero[ECC_BYTES];
int main() {
    blk.front_blk[0] = 1;
    printf("Size of a blk: %tu\n", BLKSZ);
    sha256(&blk, BLKSZ, digest);
    printf("Init hash: ");
    printhash(digest,256);
    putchar('\n');
    uint8_t count = 0;
    printf("How many bytes do you want to be 0: ");
    scanf("%hhu", &count);
    gen_data_hash(&blk, priv_key_all_zero);
    printf("When n1 is %llu, hash is ", scan_n1(&blk, count));
    //sha256(&blk, BLKSZ, digest);
    printhash(digest,256);
    putchar('\n');
    printf("Data ecc: ");
    printhash(blk.dataecc,512);
    putchar('\n');
    if(save_blk("blk0", &blk)) puts("Save block success.");
    if(read_blk("blk0", &blk)) puts("Read block success.");
    sha256(&blk,  BLKSZ, digest);
    printf("Hash from saved blk: ");
    printhash(digest,256);
    putchar('\n');
    printf("Data ecc: ");
    printhash(blk.dataecc,512);
    putchar('\n');
}
#endif

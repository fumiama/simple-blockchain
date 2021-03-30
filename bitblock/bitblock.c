#include <stdio.h>
#include "bitblock.h"
#include "../SHA256/mark2/sha256.h"

uint8_t zerobyteof(uint8_t *hash) {
    uint8_t i;
    for(i = 0; i < 32; i++) if(hash[i]) break;
    return i;
}

uint64_t scan_n1(BLOCK *blk, uint8_t zerobyte_cnt) {
    uint64_t i;
    for(i = 0; i <= UINT64_MAX; i++) {
        blk->n1 = i;
        sha256(blk, sizeof(BLOCK), digest);
        if(zerobyte_cnt == zerobyteof(digest)) break;
    }
    return i;
}

uint64_t scan_n2(BLOCK *blk, uint8_t zerobyte_cnt) {
    uint64_t i;
    for(i = 0; i <= UINT64_MAX; i++) {
        blk->n2 = i;
        sha256(blk, sizeof(BLOCK), digest);
        if(zerobyte_cnt == zerobyteof(digest)) break;
    }
    return i;
}

int save_blk(const char *__restrict__ __filename, BLOCK *blk) {
    FILE *fp = NULL;
    fp = fopen(__filename, "wb");
    if(fp) {
        fwrite(blk, sizeof(BLOCK), 1, fp);
        fclose(fp);
        return 1;
    } else return 0;
}

int read_blk(const char *__restrict__ __filename, BLOCK *blk) {
    FILE *fp = NULL;
    fp = fopen(__filename, "rb");
    if(fp) {
        fread(blk, sizeof(BLOCK), 1, fp);
        fclose(fp);
        return 1;
    } else return 0;
}

#ifdef SELF_TEST_BITBLK
#define printhash(x) for(int i = 0; i < 256/8; i++) printf("%0x", x[i])
BLOCK blk;
int main() {
    blk.front_blk[0] = 1;
    printf("Size of a blk: %tu\n", sizeof(BLOCK));
    sha256(&blk, sizeof(blk), digest);
    printf("Init hash: ");
    printhash(digest);
    putchar('\n');
    uint8_t count = 0;
    printf("How many bytes do you want to be 0: ");
    scanf("%hhu", &count);
    printf("When n1 is %llu, hash is ", scan_n1(&blk, count));
    //sha256(&blk, sizeof(blk), digest);
    printhash(digest);
    putchar('\n');
    if(save_blk("blk0", &blk)) puts("Save block success.");
    if(read_blk("blk0", &blk)) puts("Read block success.");
    sha256(&blk,  sizeof(BLOCK), digest);
    printf("Hash from saved blk: ");
    printhash(digest);
    putchar('\n');
}
#endif

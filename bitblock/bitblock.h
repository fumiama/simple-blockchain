#include <stdint.h>
struct BLOCK {
    uint8_t front_blk[256/8];
    uint8_t log_addr[256/8];
    uint64_t n1;
    uint64_t n2;
    uint8_t data[1024-80];
};//Totol size 1k
typedef struct BLOCK BLOCK;
uint8_t digest[256/8];

uint8_t zerobyteof(uint8_t *hash);
uint64_t scan_n1(BLOCK *blk, uint8_t zerobyte_cnt);
uint64_t scan_n2(BLOCK *blk, uint8_t zerobyte_cnt);

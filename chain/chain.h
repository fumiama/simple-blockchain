#include <stdint.h>

struct CHAININF {
    uint32_t blk_cnt;       //现在最新区块数
    uint32_t blk_in_one;    //多少区块合并存入一个文件
};
typedef struct CHAININF CHAININF;

int make_new_chain(const char *folder, const uint8_t *p_privateKey, uint32_t blk_in_one);
int append_chain(const char *folder, const BLOCK *blk);
BLOCK* read_blk_in_chain(const char *folder, uint32_t index);

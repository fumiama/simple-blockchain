#include <stdint.h>

struct CHAININF {
    uint32_t blk_cnt;       //现在最新区块数
    uint32_t blk_in_one;    //多少区块合并存入一个文件
};
typedef struct CHAININF CHAININF;

#include <stdio.h>
#include "bitblock/bitblock.h"
#include "chain.h"

char* dir_buf[1024];
CHAININF cinf;

int make_new_chain(char *folder, uint8_t* log_addr, const uint8_t *p_privateKey, uint32_t blk_in_one) {
    sprintf(dir_buf, "%s/info", folder);
    FILE* fpinf = NULL;
    fpinf = fopen(dir_buf, "wb");
    if(fpinf) {
        cinf.blk_cnt = 0;
        cinf.blk_in_one = blk_in_one;
        fwrite(&cinf, sizeof(CHAININF), 1, fpinf);
        fclose(fpinf);
        return 1;
    } else return 0;
}

int append_chain(char *folder, uint8_t* log_addr, const uint8_t *p_privateKey, BLOCK *blk) {
    
}

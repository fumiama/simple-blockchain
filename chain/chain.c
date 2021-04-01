#include <stdio.h>
#include <sys/stat.h>
#include "bitblock/bitblock.h"
#include "chain.h"

char* dir_buf[1024];
CHAININF cinf;

off_t file_size(const char* fname) {
    struct stat statbuf;
    if(stat(fname, &statbuf)==0) return statbuf.st_size;
    else return -1;
}

FILE* open_info(char *folder, char* type) {
    sprintf(dir_buf, "%s/info", folder);
    FILE* fpinf = NULL;
    fpinf = fopen(dir_buf, type);
    return fpinf;
}

int make_new_chain(char *folder, uint8_t* log_addr, const uint8_t *p_privateKey, uint32_t blk_in_one) {
    FILE* fpinf = open_info(folder, "wb");
    if(fpinf) {
        cinf.blk_cnt = 0;
        cinf.blk_in_one = blk_in_one;
        if(fwrite(&cinf, sizeof(CHAININF), 1, fpinf) > 0) {
            return !fclose(fpinf);
        }
    }
    return 0;
}

int append_chain(char *folder, uint8_t* log_addr, const uint8_t *p_privateKey, BLOCK *blk) {
    FILE* fpinf = open_info(folder, "ab+");
    if(fpinf && fread(&cinf, sizeof(CHAININF), 1, fpinf) > 0) {
        
    }
}

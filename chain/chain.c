#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "../bitblock/bitblock.h"
#include "../SHA256/mark2/sha256.h"
#include "../ecc/ecc.h"
#include "chain.h"

static off_t file_size(const char* fname) {
    struct stat* statbuf = malloc(sizeof(struct stat));
    int stat_ret = stat(fname, statbuf);
    off_t sz = statbuf->st_size;
    free(statbuf);
    return stat_ret?EOF:statbuf->st_size;
}

static FILE* open_info(const char *folder, const char* type) {
    char* dir_buf = malloc(1024);
    sprintf(dir_buf, "%s/info", folder);
    FILE* fpinf = NULL;
    fpinf = fopen(dir_buf, type);
    free(dir_buf);
    return fpinf;
}

int make_new_chain(const char *folder, const uint8_t* log_addr, const uint8_t *p_privateKey, uint32_t blk_in_one) {
    CHAININF* cinf = malloc(sizeof(CHAININF));
    int flag = 0;
    if(!access(folder, 0)) {
        FILE* fpinf = open_info(folder, "wb");
        if(fpinf) {
            cinf->blk_cnt = 0;
            if(blk_in_one <= 0) blk_in_one = 1;
            cinf->blk_in_one = blk_in_one;
            if(fwrite(cinf, sizeof(CHAININF), 1, fpinf) > 0) flag = !fclose(fpinf);
            else fclose(fpinf);
        }
    }
    free(cinf);
    return flag;
}

#define set_blk_dir(folder, name) sprintf(dir_buf, "%s/%010u", folder, blkfilename)

int append_chain(const char *folder, const BLOCK *blk) {
    int success = 0;
    char* dir_buf = malloc(1024);
    CHAININF* cinf = malloc(sizeof(CHAININF));
    FILE* fpinf = open_info(folder, "rb+");
    if(fpinf) {
        if(fread(cinf, sizeof(CHAININF), 1, fpinf) > 0) {
            cinf->blk_cnt++;
            uint32_t blkfilename = (cinf->blk_cnt - 1) / cinf->blk_in_one;
            set_blk_dir(folder, blkfilename);
            rewind(fpinf);
            fwrite(cinf, sizeof(CHAININF), 1, fpinf);
            FILE* fp = NULL;
            fp = fopen(dir_buf, "ab");
            if(fp) {
                if(fwrite(blk, BLKSZ, 1, fp) > 0) success = !fclose(fp);
                else fclose(fp);
            }
        }
        fclose(fpinf);
    }
    free(dir_buf);
    free(cinf);
    return success;
}

BLOCK* read_blk_in_chain(const char *folder, uint32_t index) {
    BLOCK *blk = NULL;
    char* dir_buf = malloc(1024);
    CHAININF* cinf = malloc(sizeof(CHAININF));
    FILE* fpinf = open_info(folder, "rb");
    if(fpinf) {
        if(fread(cinf, sizeof(CHAININF), 1, fpinf) > 0) {
            if(index < cinf->blk_cnt) {
                uint32_t blkfilename = index / cinf->blk_in_one;
                uint32_t offset = index % cinf->blk_in_one;
                set_blk_dir(folder, blkfilename);
                FILE* fp = NULL;
                fp = fopen(dir_buf, "rb");
                if(fp) {
                    if(offset) fseek(fp, offset * BLKSZ, SEEK_SET);
                    blk = (BLOCK*)malloc(BLKSZ);
                    fread(blk, BLKSZ, 1, fp);
                    fclose(fp);
                }
            }
        }
        fclose(fpinf);
    }
    free(dir_buf);
    free(cinf);
    return blk;
}


uint8_t bitpool[ECC_BYTES];
int main() {
    if(access("block", 0)) {
        mkdir("block", 0755);
        make_new_chain("block", bitpool, bitpool, 8);
        BLOCK* blk = wrap_block(0, 1, bitpool, bitpool, bitpool, "init", 5);
        append_chain("block", blk);
        free(blk);
    }
    char* data = malloc(1);
    for(int i = 1; i < 32; i++) {
        *data = i;
        BLOCK* prev = read_blk_in_chain("block", i - 1);
        if(prev) {
            sha256(prev, BLKSZ, bitpool);
            BLOCK* blk = wrap_block(0, 1, bitpool, bitpool, bitpool, data, 1);
            append_chain("block", blk);
            free(blk);
            free(prev);
        }
    }
    free(data);
}

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "bitblock/bitblock.h"
#include "chain.h"

static char* dir_buf[1024];
static CHAININF cinf;
static BLOCK this, prev;

static off_t file_size(const char* fname) {
    struct stat statbuf;
    if(stat(fname, &statbuf)==0) return statbuf.st_size;
    else return -1;
}

static FILE* open_info(const char *folder, const char* type) {
    sprintf(dir_buf, "%s/info", folder);
    FILE* fpinf = NULL;
    fpinf = fopen(dir_buf, type);
    return fpinf;
}

int make_new_chain(const char *folder, const uint8_t* log_addr, const uint8_t *p_privateKey, uint32_t blk_in_one) {
    FILE* fpinf = open_info(folder, "wb");
    if(fpinf) {
        cinf.blk_cnt = 1;
        if(blk_in_one <= 0) blk_in_one = 1;
        cinf.blk_in_one = blk_in_one;
        if(fwrite(&cinf, sizeof(CHAININF), 1, fpinf) > 0) {
            return !fclose(fpinf);
        }
    }
    return 0;
}

int append_chain(const char *folder, const BLOCK *blk) {
    FILE* fpinf = open_info(folder, "rb+");
    if(fpinf && fread(&cinf, sizeof(CHAININF), 1, fpinf) > 0) {
        uint32_t blkfilename = cinf.blk_cnt / cinf.blk_in_one;
        sprintf(dir_buf, "%s/%010u", folder, blkfilename);
        if(file_size(dir_buf) / BLKSZ >= cinf.blk_in_one) {
            cinf.blk_cnt++;
            sprintf(dir_buf, "%s/%010u", folder, ++blkfilename);
            rewind(fpinf);
            fwrite(&cinf, sizeof(CHAININF), 1, fpinf);
        }
        fclose(fpinf);
        FILE* fp = NULL;
        fp = fopen(dir_buf, "ab");
        if(fp && fwrite(blk, BLKSZ, 1, fp) > 0) {
            return !fclose(fp);
        }
    }
    return 0;
}

int read_blk_in_chain(const char *folder, uint32_t index) {
    FILE* fpinf = open_info(folder, "rb+");
    if(fpinf && fread(&cinf, sizeof(CHAININF), 1, fpinf) > 0) {
        uint32_t blkfilename = index / cinf.blk_in_one;
        
    }
}

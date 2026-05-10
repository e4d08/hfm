#ifndef HFM_H
#define HFM_H

#include "block.h"
#include <stdint.h>
#include <stdio.h>

#define BYTE_LENGTH 8

#define ERROR_INVALID_OPTIONS 2
#define ERROR_ALLOC 3

enum HUFFMAN_MODE { MODE_COMPRESS, MODE_DECOMPRESS };

typedef struct HfmCode_s {
    uint16_t code;
    uint8_t length;
} HfmCode;

typedef unsigned char HfmWord;

extern int hfm_compress(FILE *source, FILE *output);
extern int hfm_decompress(FILE *source, FILE *output);

extern uint32_t hfm_compress_block(BlockHeader *header, uint8_t *source, uint8_t *dest, uint32_t n);
extern uint32_t hfm_decompress_block(BlockHeader *header, uint8_t *source, uint8_t *dest);

#endif // HFM_H

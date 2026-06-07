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
    uint32_t code;
    uint8_t length;
} HfmCode;

typedef unsigned char HfmWord;

extern uint16_t hfm_compress_block(CodeTable table,
                                   BlockHeader *header,
                                   uint8_t *source,
                                   uint8_t *dest,
                                   uint16_t n);
extern uint16_t hfm_decompress_block(CodeTable table,
                                     BlockHeader *header,
                                     uint8_t *source,
                                     uint8_t *dest);

#endif // HFM_H

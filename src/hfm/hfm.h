#ifndef HFM_H
#define HFM_H

#include "block.h"
#include <stdint.h>
#include <stdio.h>

enum HUFFMAN_MODE { MODE_COMPRESS, MODE_DECOMPRESS };

/**
 * Struct for code. `Code` is an actual code which is stored
 * as least significant bits of an uint32, and `length` is
 * the size of code which is amount of those bits. The limit
 * of code length is therefore 32, which is sufficent for example
 * for blocks of size 65535 and alphabet of size 256.
 */
typedef struct HfmCode_s {
    uint32_t code;
    uint8_t length;
} HfmCode;

typedef unsigned char HfmWord;

/**
 * Compress the block of length `n` located in `source`.
 * Writes header data to `header`.
 * Writes weights table to `table`.
 * Writes result to `dest`.
 * Returns compressed data size.
 */
extern uint16_t hfm_compress_block(CodeTable table,
                                   BlockHeader *header,
                                   uint8_t *source,
                                   uint8_t *dest,
                                   uint16_t n);

/**
 * Decompress the block located in `source` using `table`
 * as a table of weights and `header` as the source of header data.
 * Writes result to `dest`.
 */
extern uint16_t hfm_decompress_block(CodeTable table,
                                     BlockHeader *header,
                                     uint8_t *source,
                                     uint8_t *dest);

#endif // HFM_H

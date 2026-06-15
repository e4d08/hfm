#ifndef HFM_H
#define HFM_H

#include "block.h"
#include <stdint.h>
#include <stdio.h>

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

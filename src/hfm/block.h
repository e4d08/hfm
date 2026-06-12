#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

#define BLOCK_SIZE (65535) // Size of blocks which are operated by hfm (64 KiB - 1)
#define ALPHABET_SIZE (256) // 8-bit compression (1-byte)
#define ERR_BLOCK_END (1)
#define BLOCK_HEADER_SIZE (5)

enum BLOCK_FLAGS { BLOCK_UNCOMPRESSED = 1 };

typedef uint16_t CodeTable[ALPHABET_SIZE];

/**
 * Structure for blocks. It implements the basic brick for compressed data.
 * All raw data is read by `BLOCK_SIZE` bytes chunks and is compressed into 
 * blocks of no more than 64 KiBs each.
 * `buf` is the buffer for writing result.
 * `pos` is the current position of block (count of written bytes).
 * `cur_byte` is a buffer to represent current byte in memory.
 * `cur_len` is length of `cur_byte` (amount of written bits, up to 7).
 */
typedef struct Block_s {
    uint8_t *buf;
    uint16_t pos;
    uint8_t cur_byte;
    uint8_t cur_len;
} Block;

/**
 * Header of block. Its members are written to output file and read from input file.
 * `flags` currently can be 0 or 1, where the latter means an uncompressed block.
 * `block_size` is the size of the compressed block.
 * `data_size` is the size of the original block data.
 */
typedef struct BlockHeader_s {
    uint8_t flags;
    uint16_t block_size;
    uint16_t data_size;
} BlockHeader;

extern void Block_start_stream(Block *block, uint8_t *buf);
extern int Block_write_bit(Block *block, uint8_t bit);
extern int Block_write_byte(Block *block, uint8_t byte);
extern int Block_end_stream(Block *block);

/**
 * Serialize `header` to buffer `dest`.
 */
extern void BlockHeader_write(BlockHeader *header, uint8_t *dest);

/**
 * Read from buffer `src` to `header`.
 */
extern void BlockHeader_read(uint8_t *src, BlockHeader *header);

#endif // BLOCK_H

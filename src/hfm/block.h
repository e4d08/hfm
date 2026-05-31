#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

#define BLOCK_SIZE (65536) // 1024 * 64
#define ALPHABET_SIZE (256)
#define ERR_BLOCK_END (1)
#define BLOCK_HEADER_SIZE (5)

typedef uint16_t CodeTable[ALPHABET_SIZE];

typedef struct Block_s {
    uint8_t *buf;
    uint32_t pos;
    uint8_t cur_byte;
    uint8_t cur_len;
} Block;

typedef struct BlockHeader_s {
    uint8_t flags;
    uint32_t block_size;
    uint32_t data_size;
} BlockHeader;

extern void Block_start_stream(Block *block, uint8_t *buf);
extern int Block_write_bit(Block *block, uint8_t bit);
extern int Block_write_byte(Block *block, uint8_t byte);
extern int Block_end_stream(Block *block);
extern void BlockHeader_write(BlockHeader *header, uint8_t *dest);
extern void BlockHeader_read(uint8_t *src, BlockHeader *header);

#endif // BLOCK_H

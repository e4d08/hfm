#include "block.h"
#include <string.h>

static int Block_flush_byte(Block *block)
{
    if (block->pos >= BLOCK_SIZE) {
        return ERR_BLOCK_END;
    }

    block->buf[block->pos] = block->cur_byte;
    block->cur_byte = 0x0;
    block->pos += 1;
    block->cur_len = 0;

    return 0;
}

void Block_start_stream(Block *block, uint8_t *buf)
{
    block->buf = buf;
    memset(buf, 0, BLOCK_SIZE);
    block->pos = 0;
    block->cur_byte = 0x0;
    block->cur_len = 0;
}

int Block_write_byte(Block *block, uint8_t byte)
{
    block->cur_byte = byte;
    return Block_flush_byte(block);
}

int Block_write_bit(Block *block, uint8_t bit)
{
    block->cur_byte |= (bit << block->cur_len);
    block->cur_len += 1;

    if (block->cur_len == 8) {
        return Block_flush_byte(block);
    }

    return 0;
}

int Block_end_stream(Block *block)
{
    if (block->cur_len != 0) {
        return Block_flush_byte(block);
    }

    return 0;
}

void BlockHeader_read(uint8_t *src, BlockHeader *header) {
    header->flags = src[0];
    header->block_size = (src[1] << 8) | src[2];
    header->data_size = (src[3] << 8) | src[4];
}

void BlockHeader_write(BlockHeader *header, uint8_t *dest) {
    dest[0] = header->flags;
    dest[1] = (uint8_t)(header->block_size >> 8);
    dest[2] = (uint8_t)(header->block_size);
    dest[3] = (uint8_t)(header->data_size >> 8);
    dest[4] = (uint8_t)(header->data_size);
}

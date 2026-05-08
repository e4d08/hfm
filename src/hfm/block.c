#include "block.h"
#include <string.h>

static int Block_flush_byte(Block *block)
{
    if (block->pos >= BLOCK_SIZE) {
        return ERR_BLOCK_END;
    }

    block->buf[block->pos] = block->cur_byte;
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

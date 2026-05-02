#include "hfm.h"
#include "block.h"
#include "huffman_tree.h"
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static void
get_codes_from_tree(HuffmanTree *tree, HfmCode *codes, HfmCode cur_code)
{
    if (tree == NULL) {
        return;
    }

    if (huffman_tree_is_leaf(tree)) {
        codes[tree->value] = cur_code;
    }

    HfmCode left_code = {.code = cur_code.code << 1,
                         .length = cur_code.length + 1};
    HfmCode right_code = {.code = (cur_code.code << 1) | 1,
                          .length = cur_code.length + 1};
    get_codes_from_tree(tree->left_child, codes, left_code);
    get_codes_from_tree(tree->right_child, codes, right_code);
}

uint32_t hfm_compress_block(const uint8_t *source, uint8_t *dest, uint32_t n)
{
    tree_weight_t weights[ALPHABET_SIZE];

    for (uint32_t i = 0; i < n; ++i) {
        weights[source[i]] += 1;
    }

    HuffmanTree *huffman_tree = huffman_tree_create();
    huffman_tree_build(weights, huffman_tree);

    HfmCode codes[ALPHABET_SIZE];
    HfmCode zero_code = {0, 0};
    get_codes_from_tree(huffman_tree, codes, zero_code);

    Block block;
    const uint16_t block_start = BLOCK_HEADER_SIZE + TABLE_SIZE;

    Block_start_stream(&block, dest + block_start);
    for (uint32_t i = 0; i < n; ++i) {
        HfmCode code = codes[source[i]];
        for (int j = 0; j < code.length; ++j) {
            const uint8_t bit = (code.code >> j) & 1;
            Block_write_bit(&block, bit);
        }
    }
    Block_end_stream(&block);

    BlockHeader header = {.flags = 0x0, .block_size = (uint16_t)block.pos};
    memcpy(dest, &header.flags, 1);
    memcpy(dest + 1, &header.block_size, 2);
    memcpy(dest + BLOCK_HEADER_SIZE, &weights, sizeof(weights));

    return BLOCK_HEADER_SIZE + TABLE_SIZE + block.pos;
}

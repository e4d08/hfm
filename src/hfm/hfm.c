#include "hfm.h"
#include "block.h"
#include "huffman_tree.h"
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

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

static void
get_codes_from_tree(HuffmanTree *tree, HfmCode *codes, HfmCode cur_code) {
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

uint16_t hfm_compress_block(CodeTable table,
    BlockHeader *header,
    uint8_t *source,
    uint8_t *dest,
    uint16_t n) {
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        table[i] = 0;
    }

    for (uint16_t i = 0; i < n; ++i) {
        table[source[i]] += 1;
    }

    HuffmanTree *huffman_tree = huffman_tree_create();
    huffman_tree_build(table, huffman_tree);

    HfmCode codes[ALPHABET_SIZE];
    HfmCode zero_code = {.code = 0, .length = 0};
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        codes[i] = zero_code;
    }
    get_codes_from_tree(huffman_tree, codes, zero_code);

    Block block;

    block_start_stream(&block, dest);
    for (uint16_t i = 0; i < n; ++i) {
        HfmCode code = codes[source[i]];
        for (int j = code.length - 1; j >= 0; --j) {
            const uint8_t bit = (code.code >> j) & 1;
            block_write_bit(&block, bit);
        }
    }
    block_end_stream(&block);

    header->flags = 0x0;
    header->block_size = block.pos;
    header->data_size = n;

    if (block.pos >= n) {
        header->flags |= BLOCK_UNCOMPRESSED;
    }

    huffman_tree_free_rec(huffman_tree);
    return block.pos;
}

uint16_t hfm_decompress_block(CodeTable table,
    BlockHeader *header,
    uint8_t *source,
    uint8_t *dest) {
    HuffmanTree *huffman_tree = huffman_tree_create();
    huffman_tree_build(table, huffman_tree);

    uint16_t pos = 0;
    HuffmanTree *cur = huffman_tree;
    for (uint16_t i = 0; i < header->block_size; ++i) {
        tree_value_t byte = source[i];
        for (int j = 0; j < 8; ++j) {
            uint8_t bit = (byte >> j) & 1;
            if (bit == 0) {
                cur = cur->left_child;
            } else {
                cur = cur->right_child;
            }
            if (huffman_tree_is_leaf(cur)) {
                dest[pos] = cur->value;
                pos += 1;
                cur = huffman_tree;
                if (pos == header->data_size) {
                    huffman_tree_free_rec(huffman_tree);
                    return pos;
                }
            }
        }
    }

    huffman_tree_free_rec(huffman_tree);
    return pos;
}

#include "hfm.h"
#include "huffman_tree.h"
#include "unity.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

/* ─────────────── HuffmanTree lifecycle ─────────────── */

void test_tree_create_returns_zeroed_node(void)
{
    HuffmanTree *tree = huffman_tree_create();
    TEST_ASSERT_NOT_NULL(tree);
    TEST_ASSERT_EQUAL(0, tree->value);
    TEST_ASSERT_EQUAL(0, tree->weight);
    TEST_ASSERT_NULL(tree->left_child);
    TEST_ASSERT_NULL(tree->right_child);
    huffman_tree_free_rec(tree);
}

void test_tree_is_leaf_true_for_fresh_node(void)
{
    HuffmanTree *tree = huffman_tree_create();
    TEST_ASSERT_TRUE(huffman_tree_is_leaf(tree));
    huffman_tree_free_rec(tree);
}

void test_tree_is_leaf_false_when_children_exist(void)
{
    HuffmanTree *tree  = huffman_tree_create();
    HuffmanTree *left  = huffman_tree_create();
    HuffmanTree *right = huffman_tree_create();

    tree->left_child  = left;
    tree->right_child = right;

    TEST_ASSERT_FALSE(huffman_tree_is_leaf(tree));
    TEST_ASSERT_TRUE(huffman_tree_is_leaf(left));
    TEST_ASSERT_TRUE(huffman_tree_is_leaf(right));

    huffman_tree_free_rec(tree);
}

/* ─────────────── HuffmanTree build ─────────────── */

void test_tree_build_single_character(void)
{
    tree_weight_t weights[ALPHABET_SIZE] = { 0 };
    weights['A'] = 5;

    HuffmanTree *tree = huffman_tree_create();
    huffman_tree_build(weights, tree);

    TEST_ASSERT_FALSE(huffman_tree_is_leaf(tree));
    TEST_ASSERT_EQUAL(5, tree->weight);

    huffman_tree_free_rec(tree);
}

void test_tree_build_two_characters(void)
{
    tree_weight_t weights[ALPHABET_SIZE] = { 0 };
    weights['A'] = 3;
    weights['B'] = 7;

    HuffmanTree *tree = huffman_tree_create();
    huffman_tree_build(weights, tree);

    TEST_ASSERT_FALSE(huffman_tree_is_leaf(tree));
    TEST_ASSERT_EQUAL(10, tree->weight);
    TEST_ASSERT_NOT_NULL(tree->left_child);
    TEST_ASSERT_NOT_NULL(tree->right_child);

    huffman_tree_free_rec(tree);
}

void test_tree_build_weight_is_sum_of_all_frequencies(void)
{
    tree_weight_t weights[ALPHABET_SIZE] = { 0 };
    weights['X'] = 10;
    weights['Y'] = 20;
    weights['Z'] = 30;

    HuffmanTree *tree = huffman_tree_create();
    huffman_tree_build(weights, tree);

    TEST_ASSERT_EQUAL(60, tree->weight);

    huffman_tree_free_rec(tree);
}

/* ─────────────── Compress / Decompress round-trip ─────────────── */

void test_compress_decompress_roundtrip_simple(void)
{
    const char *msg     = "AAAAABBBCCD";
    uint16_t    n       = (uint16_t)strlen(msg);
    uint8_t     source[256];
    uint8_t     compressed[BLOCK_SIZE];
    uint8_t     decompressed[256];
    CodeTable   table   = { 0 };
    BlockHeader header  = { 0 };

    memcpy(source, msg, n);

    uint16_t comp_size = hfm_compress_block(table, &header, source, compressed, n);

    TEST_ASSERT_GREATER_THAN(0, comp_size);
    TEST_ASSERT_EQUAL(n, header.data_size);
    TEST_ASSERT_EQUAL(comp_size, header.block_size);

    uint16_t decomp_size = hfm_decompress_block(table, &header, compressed, decompressed);

    TEST_ASSERT_EQUAL(n, decomp_size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(source, decompressed, n);
}

void test_compress_decompress_roundtrip_all_bytes(void)
{
    uint8_t   source[ALPHABET_SIZE];
    uint8_t   compressed[BLOCK_SIZE];
    uint8_t   decompressed[ALPHABET_SIZE];
    CodeTable table  = { 0 };
    BlockHeader header = { 0 };

    for (int i = 0; i < ALPHABET_SIZE; ++i)
        source[i] = (uint8_t)i;

    uint16_t comp_size =
        hfm_compress_block(table, &header, source, compressed, ALPHABET_SIZE);

    TEST_ASSERT_GREATER_THAN(0, comp_size);

    uint16_t decomp_size =
        hfm_decompress_block(table, &header, compressed, decompressed);

    TEST_ASSERT_EQUAL(ALPHABET_SIZE, decomp_size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(source, decompressed, ALPHABET_SIZE);
}

void test_compress_decompress_repetitive_data(void)
{
    uint8_t   source[1000];
    uint8_t   compressed[BLOCK_SIZE];
    uint8_t   decompressed[1000];
    CodeTable table  = { 0 };
    BlockHeader header = { 0 };

    memset(source, 0x00, sizeof(source));

    uint16_t comp_size =
        hfm_compress_block(table, &header, source, compressed, 1000);

    TEST_ASSERT_LESS_THAN(1000, comp_size);

    uint16_t decomp_size =
        hfm_decompress_block(table, &header, compressed, decompressed);

    TEST_ASSERT_EQUAL(1000, decomp_size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(source, decompressed, 1000);
}

void test_compress_sets_uncompressed_flag_when_inflated(void)
{
    uint8_t   source[256];
    uint8_t   compressed[BLOCK_SIZE];
    CodeTable table  = { 0 };
    BlockHeader header = { 0 };

    for (int i = 0; i < 256; ++i)
        source[i] = (uint8_t)i;

    hfm_compress_block(table, &header, source, compressed, 256);

    TEST_ASSERT_BITS(BLOCK_UNCOMPRESSED, BLOCK_UNCOMPRESSED, header.flags);
}

void test_compress_clears_uncompressed_flag_for_compressible_data(void)
{
    uint8_t   source[200];
    uint8_t   compressed[BLOCK_SIZE];
    CodeTable table  = { 0 };
    BlockHeader header = { 0 };

    memset(source, 'A', sizeof(source));

    hfm_compress_block(table, &header, source, compressed, 200);

    /* For highly compressible data the flag should NOT be set */
    TEST_ASSERT_BITS_LOW(BLOCK_UNCOMPRESSED, header.flags);
    TEST_ASSERT_LESS_THAN(200, header.block_size);
    TEST_ASSERT_EQUAL(0, header.flags & BLOCK_UNCOMPRESSED);
}

void test_compress_decompress_single_byte(void)
{
    uint8_t   source[1] = { 0x42 };
    uint8_t   compressed[BLOCK_SIZE];
    uint8_t   decompressed[1];
    CodeTable table  = { 0 };
    BlockHeader header = { 0 };

    uint16_t comp_size = hfm_compress_block(table, &header, source, compressed, 1);
    TEST_ASSERT_GREATER_THAN(0, comp_size);

    uint16_t decomp_size = hfm_decompress_block(table, &header, compressed, decompressed);
    TEST_ASSERT_EQUAL(1, decomp_size);
    TEST_ASSERT_EQUAL_HEX8(0x42, decompressed[0]);
}

void test_compress_header_data_size_matches_input(void)
{
    uint8_t   source[] = "Hello, Huffman!";
    uint16_t  n        = (uint16_t)strlen((char *)source);
    uint8_t   compressed[BLOCK_SIZE];
    CodeTable table    = { 0 };
    BlockHeader header = { 0 };

    hfm_compress_block(table, &header, source, compressed, n);

    TEST_ASSERT_EQUAL(n, header.data_size);
}

void test_compress_decompress_binary_data(void)
{
    uint8_t source[] = {
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0x01, 0x02, 0x03,
        0xFE, 0xFD, 0xFC, 0x80, 0x7F, 0x55, 0xAA, 0x00,
    };
    uint16_t  n          = sizeof(source);
    uint8_t   compressed[BLOCK_SIZE];
    uint8_t   decompressed[sizeof(source)];
    CodeTable table      = { 0 };
    BlockHeader header   = { 0 };

    uint16_t comp_size = hfm_compress_block(table, &header, source, compressed, n);
    TEST_ASSERT_GREATER_THAN(0, comp_size);

    uint16_t decomp_size = hfm_decompress_block(table, &header, compressed, decompressed);
    TEST_ASSERT_EQUAL(n, decomp_size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(source, decompressed, n);
}

void test_compress_decompress_inflated(void)
{
    uint8_t   source[256];
    uint8_t   compressed[BLOCK_SIZE];
    uint8_t   decompressed[256];
    CodeTable table  = { 0 };
    BlockHeader header = { 0 };

    for (int i = 0; i < 256; ++i)
        source[i] = (uint8_t)i;

    uint16_t comp_size = hfm_compress_block(table, &header, source, compressed, 256);
    TEST_ASSERT_GREATER_THAN(0, comp_size);
    TEST_ASSERT_BITS(BLOCK_UNCOMPRESSED, BLOCK_UNCOMPRESSED, header.flags);

    uint16_t decomp_size = hfm_decompress_block(table, &header, compressed, decompressed);
    TEST_ASSERT_EQUAL(256, decomp_size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(source, decompressed, 256);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_tree_create_returns_zeroed_node);
    RUN_TEST(test_tree_is_leaf_true_for_fresh_node);
    RUN_TEST(test_tree_is_leaf_false_when_children_exist);

    RUN_TEST(test_tree_build_single_character);
    RUN_TEST(test_tree_build_two_characters);
    RUN_TEST(test_tree_build_weight_is_sum_of_all_frequencies);

    RUN_TEST(test_compress_decompress_roundtrip_simple);
    RUN_TEST(test_compress_decompress_roundtrip_all_bytes);
    RUN_TEST(test_compress_decompress_repetitive_data);
    RUN_TEST(test_compress_sets_uncompressed_flag_when_inflated);
    RUN_TEST(test_compress_clears_uncompressed_flag_for_compressible_data);
    RUN_TEST(test_compress_decompress_single_byte);
    RUN_TEST(test_compress_header_data_size_matches_input);
    RUN_TEST(test_compress_decompress_binary_data);
    RUN_TEST(test_compress_decompress_inflated);

    return UNITY_END();
}

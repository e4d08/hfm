#include "huffman_tree.h"
#include "unity.h"
#include "hfm.h"

void setUp() {}
void tearDown() {}

void test_huffman_tree_create() {
    HuffmanTree *tree = huffman_tree_create();
    TEST_ASSERT_NOT_NULL(tree);
    TEST_ASSERT_EQUAL(0, tree->value);
    TEST_ASSERT_EQUAL(0, tree->weight);
    TEST_ASSERT_NULL(tree->left_child);
    TEST_ASSERT_NULL(tree->right_child);
    huffman_tree_free_rec(tree);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_huffman_tree_create);
    return UNITY_END();
}

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t tree_value_t;
typedef uint32_t tree_weight_t;

#define TREE_WEIGHT_MAX UINT32_MAX

typedef struct HuffmanTree_s {
    tree_weight_t weight;
    tree_value_t value;
    struct HuffmanTree_s* left_child;
    struct HuffmanTree_s* right_child;
} HuffmanTree;

extern HuffmanTree* huffman_tree_create();
extern void huffman_tree_free_rec(HuffmanTree* tree);
extern void huffman_tree_build(tree_weight_t* weights, HuffmanTree* dest);
extern bool huffman_tree_is_leaf(HuffmanTree* tree);
extern void huffman_tree_print(FILE* stream, HuffmanTree* tree);
extern void huffman_tree_copy(HuffmanTree* dest, HuffmanTree* tree);

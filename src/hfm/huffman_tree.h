#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t tree_value_t; // type for tree values (characters)
typedef uint16_t tree_weight_t; // type for tree weights (frequencies)

#define TREE_WEIGHT_MAX UINT16_MAX // max weight of a tree (aka frequency of a character)

/**
 * Structure for Huffman Tree (and tree node). If it is a leaf,
 * HuffmanTree contains info about coded character, in particular
 * `weight` is for frequency, `value` is actual coded character,
 * `left_child` and `right_child` are pointers to children.
 */
typedef struct HuffmanTree_s {
    tree_weight_t weight;
    tree_value_t value;
    struct HuffmanTree_s *left_child;
    struct HuffmanTree_s *right_child;
} HuffmanTree;

/**
 * Do calloc and return pointer to a HuffmanTree (zeroed).
 */
extern HuffmanTree *huffman_tree_create();

/**
 * Free HuffmanTree recursively. Frees left subtree, then right
 * subtree, then frees `tree`.
 */
extern void huffman_tree_free_rec(HuffmanTree *tree);

/**
 * Build huffman tree at `dest` for provided `weights` (a table of 
 * frequencies of all characters).
 * After building `dest` needs to be freed.
 */
extern void huffman_tree_build(tree_weight_t *weights, HuffmanTree *dest);

/**
 * Check whether `tree` is a leaf (has no children). Returns
 * `true` is yes and `false` is no.
 */
extern bool huffman_tree_is_leaf(HuffmanTree *tree);

#endif // HUFFMAN_TREE_H

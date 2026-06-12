#include "huffman_tree.h"
#include "hfm.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void huffman_tree_copy(HuffmanTree *dest, HuffmanTree *tree) {
    if (tree == NULL) {
        return;
    }
    memcpy(dest, tree, sizeof(*dest));
    if (tree->left_child != NULL) {
        dest->left_child = huffman_tree_create();
        huffman_tree_copy(dest->left_child, tree->left_child);
    }
    if (tree->right_child != NULL) {
        dest->right_child = huffman_tree_create();
        huffman_tree_copy(dest->right_child, tree->right_child);
    }
}

static int huffman_tree_compare(const void *a, const void *b) {
    const tree_weight_t left_w = (**(HuffmanTree **)a).weight;
    const tree_weight_t right_w = (**(HuffmanTree **)b).weight;
    if (left_w == right_w) {
        return 0;
    }
    return left_w < right_w ? -1 : 1;
}

HuffmanTree *huffman_tree_create() {
    HuffmanTree *tree_ptr = calloc(1, sizeof(HuffmanTree));
    if (tree_ptr == NULL) {
        errno = ERROR_ALLOC;
        return NULL;
    }

    return tree_ptr;
}

void huffman_tree_free_rec(HuffmanTree *tree) {
    if (tree != NULL) {
        huffman_tree_free_rec(tree->left_child);
        huffman_tree_free_rec(tree->right_child);
        free(tree);
    }
}

void huffman_tree_build(tree_weight_t *weights, HuffmanTree *dest) {
    // ALPHABET_SIZE * 2 to prevent out-of-bounds
    // TODO
    const int K = ALPHABET_SIZE * 2;
    HuffmanTree *tree_array1[K], *tree_array2[K];
    for (int i = 0; i < K; ++i) {
        tree_array1[i] = huffman_tree_create();
        tree_array2[i] = huffman_tree_create();
        tree_array2[i]->weight = tree_array1[i]->weight = TREE_WEIGHT_MAX;
    }
    uint16_t n = 0;
    for (int ch = 0; ch < ALPHABET_SIZE; ++ch) {
        if (weights[ch] > 0) {
            tree_array1[n]->value = (tree_value_t)ch;
            tree_array1[n]->weight = weights[ch];
            ++n;
        }
    }

    qsort(tree_array1, n, sizeof(tree_array1[0]), huffman_tree_compare);

    int i = 0, j = 0;
    for (int k = 0; k < n - 1; ++k) {
        uint32_t sum11 =
            (uint32_t)tree_array1[i]->weight + tree_array1[i + 1]->weight;
        uint32_t sum12 =
            (uint32_t)tree_array1[i]->weight + tree_array2[j]->weight;
        uint32_t sum22 =
            (uint32_t)tree_array2[j]->weight + tree_array2[j + 1]->weight;
        free(tree_array2[k]);

        if (sum11 <= sum22 && sum11 <= sum12) {
            tree_array2[k] = huffman_tree_create();
            tree_array2[k]->left_child = tree_array1[i];
            tree_array2[k]->right_child = tree_array1[i + 1];
            tree_array2[k]->weight = (tree_weight_t)sum11;
            i += 2;
        } else if (sum12 <= sum22 && sum12 <= sum11) {
            tree_array2[k] = huffman_tree_create();
            tree_array2[k]->left_child = tree_array1[i];
            tree_array2[k]->right_child = tree_array2[j];
            tree_array2[k]->weight = (tree_weight_t)sum12;
            i += 1;
            j += 1;
        } else if (sum22 <= sum11 && sum22 <= sum12) {
            tree_array2[k] = huffman_tree_create();
            tree_array2[k]->left_child = tree_array2[j];
            tree_array2[k]->right_child = tree_array2[j + 1];
            tree_array2[k]->weight = (tree_weight_t)sum22;
            j += 2;
        }
    }

    if (n == 1) {
        dest->value = 0;
        dest->weight = tree_array1[0]->weight;
        dest->left_child = huffman_tree_create();
        dest->right_child = huffman_tree_create();
        huffman_tree_copy(dest->left_child, tree_array1[0]);
        huffman_tree_copy(dest->right_child, tree_array1[1]);
    } else {
        huffman_tree_copy(dest, tree_array2[n - 2]);
    }

    for (int k = 0; k < K; ++k) {
        free(tree_array1[k]);
        free(tree_array2[k]);
    }
}

inline bool huffman_tree_is_leaf(HuffmanTree *tree) {
    if (tree->left_child == NULL && tree->right_child == NULL) {
        return true;
    }
    return false;
}

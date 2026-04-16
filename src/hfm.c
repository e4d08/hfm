#include "hfm.h"
#include "huffman_tree.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static size_t read_file_to_buffer(FILE* stream, HfmWord** buffer)
{
    size_t file_length;
    fseek(stream, 0, SEEK_END);
    file_length = (size_t)ftell(stream);
    rewind(stream);
    *buffer = malloc(file_length);
    if (*buffer == NULL) {
        return 0;
    }
    fread(*buffer, file_length, 1, stream);
    return file_length;
}

static void get_codes_from_tree(HuffmanTree* tree, HfmCode* codes, HfmCode cur_code)
{
    if (tree == NULL) {
        return;
    }

    if (huffman_tree_is_leaf(tree)) {
        codes[tree->value] = cur_code;
    }

    HfmCode left_code = { .code = cur_code.code << 1, .length = cur_code.length + 1 };
    HfmCode right_code = { .code = (cur_code.code << 1) | 1, .length = cur_code.length + 1 };
    get_codes_from_tree(tree->left_child, codes, left_code);
    get_codes_from_tree(tree->right_child, codes, right_code);
}

int hfm_compress(FILE* source, FILE* output)
{
    HfmWord* input_buffer = NULL;
    uint64_t file_length = read_file_to_buffer(source, &input_buffer);
    if (file_length == 0) {
        fprintf(stderr, "Error while reading bytes from file.\n");
        return ERROR_ALLOC;
    }

    fwrite(&file_length, sizeof(file_length), 1, output);

    tree_weight_t cnt[ALPHABET_SIZE];
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        cnt[i] = 0;
    }

    for (uint64_t i = 0; i < file_length; ++i) {
        HfmWord cur_char = input_buffer[i];
        cnt[cur_char] += 1;
    }

    fwrite(cnt, sizeof(tree_weight_t), ALPHABET_SIZE, output);

    HuffmanTree* huffman_tree = huffman_tree_create();
    huffman_tree_build(cnt, huffman_tree);
    HfmCode codes[ALPHABET_SIZE];
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        codes[i].code = 0;
        codes[i].length = 0;
    }
    const HfmCode zero_code = { .code = 0, .length = 0 };
    get_codes_from_tree(huffman_tree, codes, zero_code);

    uint8_t buf = 0;
    uint8_t buf_size = 0;
    for (uint64_t i = 0; i < file_length; ++i) {
        HfmCode char_code = codes[input_buffer[i]];
        for (int j = char_code.length - 1; j >= 0; --j) {
            const uint8_t bit = (char_code.code >> j) & 1;
            buf |= (bit << (BYTE_LENGTH - 1 - buf_size));
            buf_size += 1;
            if (buf_size == BYTE_LENGTH) {
                fwrite(&buf, sizeof(buf), 1, output);
                buf_size = 0;
                buf = 0;
            }
        }
    }
    if (buf_size != 0) {
        fwrite(&buf, sizeof(buf), 1, output);
    }
    huffman_tree_free_rec(huffman_tree);

    fclose(output);
    free(input_buffer);
    return 0;
}

int hfm_decompress(FILE* source, FILE* output)
{
    HfmWord* input_buffer = NULL;
    size_t file_length = read_file_to_buffer(source, &input_buffer);
    tree_weight_t weights[ALPHABET_SIZE];
    uint64_t original_length;
    memcpy(weights, input_buffer + sizeof(original_length), ALPHABET_SIZE * sizeof(tree_weight_t));
    memcpy(&original_length, input_buffer, sizeof(original_length));
    HuffmanTree* huffman_tree = huffman_tree_create();
    huffman_tree_build(weights, huffman_tree);
    HuffmanTree* cur = huffman_tree;
    size_t written_count = 0;
    for (size_t i = sizeof(weights) + sizeof(original_length); i < file_length; ++i) {
        tree_value_t cur_byte = input_buffer[i];
        for (int j = BYTE_LENGTH - 1; j >= 0; --j) {
            if (((cur_byte >> j) & 1) == 1) {
                cur = cur->right_child;
            } else {
                cur = cur->left_child;
            }
            if (huffman_tree_is_leaf(cur)) {
                fwrite(&(cur->value), sizeof(tree_value_t), 1, output);
                written_count += 1;
                if (written_count == original_length) {
                    break;
                }
                cur = huffman_tree;
            }
        }
        if (written_count == original_length) {
            break;
        }
    }
    huffman_tree_free_rec(huffman_tree);

    fclose(output);
    free(input_buffer);
    return 0;
}

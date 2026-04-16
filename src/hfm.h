#pragma once

#include <stdint.h>
#include <stdio.h>

#define ALPHABET_SIZE 256
#define BYTE_LENGTH 8

#define ERROR_INVALID_OPTIONS 2
#define ERROR_ALLOC 3

enum HUFFMAN_MODE {
	MODE_COMPRESS,
	MODE_DECOMPRESS
};

typedef struct HfmCode_s {
	uint64_t code;
	uint8_t length;
} HfmCode;

typedef unsigned char HfmWord;

extern int hfm_compress(FILE *source, FILE *output);
extern int hfm_decompress(FILE *source, FILE *output);

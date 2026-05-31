#include "hfm.h"
#include "block.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static int compress_file(int fin, int fout)
{
    uint8_t in_buf[BLOCK_SIZE];
    uint8_t out_buf[BLOCK_SIZE];
    uint8_t header_buf[BLOCK_HEADER_SIZE];
    BlockHeader header;
    CodeTable table;

    ssize_t n = read(fin, in_buf, BLOCK_SIZE);
    while (n > 0) {
        uint32_t block_size = hfm_compress_block(table, &header, in_buf, out_buf, (uint32_t)n);
        BlockHeader_write(&header, header_buf);
        write(fout, header_buf, BLOCK_HEADER_SIZE);
        write(fout, table, sizeof(CodeTable));
        write(fout, out_buf, block_size);

        n = read(fin, in_buf, BLOCK_SIZE);
    }

    if (n == -1) {
        fprintf(stderr, "Cannot read from input file\n");
        return -1;
    }

    return 0;
}

static int decompress_file(int fin, int fout) {
    uint8_t in_buf[BLOCK_SIZE];
    uint8_t out_buf[BLOCK_SIZE];
    uint8_t header_buf[BLOCK_HEADER_SIZE];
    BlockHeader header;
    CodeTable table;

    while (1) {
        ssize_t header_size = read(fin, header_buf, BLOCK_HEADER_SIZE);
        if (header_size == 0) {
            return 0;
        }
        if (header_size < 0) {
            fprintf(stderr, "Cannot read block header from input file\n");
            return 1;
        }

        BlockHeader_read(header_buf, &header);
        read(fin, table, sizeof(CodeTable));
        read(fin, in_buf, header.block_size);
        uint32_t result_size = hfm_decompress_block(table, &header, in_buf, out_buf);
        write(fout, out_buf, result_size);
    }

    return 0;
}

int main(int argc, char **argv)
{
    int c = 0;
    char *output_path = NULL;
    char *file_path = NULL;
    enum HUFFMAN_MODE mode = MODE_COMPRESS;

    while ((c = getopt(argc, argv, "hdo:")) != -1) {
        switch (c) {
        case 'd':
            mode = MODE_DECOMPRESS;
            break;
        case 'o':
            output_path = optarg;
            break;
        case 'h':
            printf("Usage: hfm [-d] [-o OUT] FILE\n");
            return 0;
        case '?':
            if (optopt == 'o') {
                fprintf(stderr, "Option -o requires a parameter.\n");
            } else {
                fprintf(stderr, "Unknown option '-%c'.\n", c);
            }
            return ERROR_INVALID_OPTIONS;
        default:
            abort();
        }
    }
    if (output_path == NULL) {
        fprintf(stderr, "Please specify output path via -o option.\n");
        return ERROR_INVALID_OPTIONS;
    }
    if (argc > optind) {
        file_path = argv[optind];
    }
    if (file_path == NULL) {
        fprintf(stderr, "Please specify the file to work with.\n");
        return ERROR_INVALID_OPTIONS;
    }

    int fin = open(file_path, O_RDONLY);
    if (fin == -1) {
        close(fin);
        fprintf(stderr, "Cannot open file %s.", file_path);
        return 1;
    }

    int fout = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fout == -1) {
        close(fin);
        fprintf(stderr, "Cannot open file %s to write.", output_path);
        return 1;
    }

    int rc = 0;

    if (mode == MODE_COMPRESS) {
        rc = compress_file(fin, fout);
    } else if (mode == MODE_DECOMPRESS) {
        rc = decompress_file(fin, fout);
    }

    close(fin);
    close(fout);

    return rc;
}

#include "hfm.h"
#include <stdlib.h>
#include <unistd.h>

static size_t read_file_to_buffer(FILE* stream, HfmWord** buffer)
{
    if (fseek(stream, 0, SEEK_END) != 0) {
        return 0;
    }
    size_t file_length = (size_t)ftell(stream);
    if (fseek(stream, 0, SEEK_SET) != 0) {
        return 0;
    }

    *buffer = malloc(file_length);
    if (*buffer == NULL) {
        return 0;
    }

    if (fread(*buffer, file_length, 1, stream) != 1) {
        return 0;
    }
    return file_length;
}

int main(int argc, char** argv)
{
    int c = 0;
    char* output_path = NULL;
    char* file_path = NULL;
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

    FILE* source = fopen(file_path, "r");
    if (source == NULL) {
        fprintf(stderr, "Cannot open file %s.", file_path);
        return 1;
    }

    FILE* output = fopen(output_path, "w+");
    if (output == NULL) {
        fprintf(stderr, "Cannot open file %s to write.", output_path);
        return 1;
    }

    int rc = 0;

    if (mode == MODE_COMPRESS) {
        rc = hfm_compress(source, output);
    } else if (mode == MODE_DECOMPRESS) {
        rc = hfm_decompress(source, output);
    }

    fclose(source);
    fclose(output);

    return rc;
}

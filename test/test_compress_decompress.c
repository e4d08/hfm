#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hfm.h"

static void test_roundtrip_tmpfile(const uint8_t *data, size_t len) {
    FILE *source = tmpfile();
    assert_non_null(source);
    size_t written = fwrite(data, 1, len, source);
    assert_int_equal(written, len);
    rewind(source);

    FILE *compressed = tmpfile();
    assert_non_null(compressed);

    int ret = hfm_compress(source, compressed);
    assert_int_equal(ret, 0);

    rewind(compressed);

    FILE *decompressed = tmpfile();
    assert_non_null(decompressed);

    ret = hfm_decompress(compressed, decompressed);
    assert_int_equal(ret, 0);

    fseek(decompressed, 0, SEEK_END);
    long decomp_len = ftell(decompressed);
    rewind(decompressed);

    assert_int_equal((size_t)decomp_len, len);

    uint8_t *decomp_data = malloc(len);
    assert_non_null(decomp_data);
    size_t read = fread(decomp_data, 1, len, decompressed);
    assert_int_equal(read, len);
    assert_memory_equal(data, decomp_data, len);

    free(decomp_data);
    fclose(source);
    fclose(compressed);
    fclose(decompressed);
}

static void test_single_char(void **state) {
    (void) state;
    const char *str = "aaaaa";
    test_roundtrip_tmpfile((const uint8_t*)str, strlen(str));
}

static void test_simple_text(void **state) {
    (void) state;
    const char *str = "hello world! this is a test.";
    test_roundtrip_tmpfile((const uint8_t*)str, strlen(str));
}

static void test_binary_data(void **state) {
    (void) state;
    uint8_t data[256];
    for (int i = 0; i < 256; i++) data[i] = (uint8_t)i;
    test_roundtrip_tmpfile(data, sizeof(data));
}

static void test_all_same_byte(void **state) {
    (void) state;
    uint8_t data[1000];
    memset(data, 0xAB, sizeof(data));
    test_roundtrip_tmpfile(data, sizeof(data));
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_single_char),
        cmocka_unit_test(test_simple_text),
        cmocka_unit_test(test_binary_data),
        cmocka_unit_test(test_all_same_byte),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}

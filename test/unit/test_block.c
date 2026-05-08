#include "block.h"
#include "unity.h"
#include <string.h>

static Block test_block;
static uint8_t test_buf[BLOCK_SIZE];

void setUp() {
    memset(test_buf, 0xDE, sizeof(test_buf));
    Block_start_stream(&test_block, test_buf);
}
void tearDown() {}

void test_start_stream_initializes_all_fields() {
    TEST_ASSERT_EQUAL_PTR(test_buf, test_block.buf);
    TEST_ASSERT_EQUAL_INT(0, test_block.pos);
    TEST_ASSERT_EQUAL_UINT8(0x00, test_block.cur_byte);
    TEST_ASSERT_EQUAL_INT(0, test_block.cur_len);
    
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        TEST_ASSERT_EQUAL_UINT8(0x00, test_buf[i]);
    }
}

void test_write_byte_stores_and_increments_pos() {
    TEST_ASSERT_EQUAL_INT(0, Block_write_byte(&test_block, 0xAB));
    TEST_ASSERT_EQUAL_UINT8(0xAB, test_buf[0]);
    TEST_ASSERT_EQUAL_INT(1, test_block.pos);
    TEST_ASSERT_EQUAL_INT(0, test_block.cur_len);
}

void test_write_byte_overflow_returns_error() {
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        TEST_ASSERT_EQUAL_INT(0, Block_write_byte(&test_block, (uint8_t)i));
    }
    TEST_ASSERT_EQUAL_INT(BLOCK_SIZE, test_block.pos);

    TEST_ASSERT_EQUAL_INT(ERR_BLOCK_END, Block_write_byte(&test_block, 0xFF));
    
    TEST_ASSERT_EQUAL_INT(BLOCK_SIZE, test_block.pos);
    TEST_ASSERT_EQUAL_INT(0, test_block.cur_len);
}

void test_write_bit_packs_lsb_first() {
    uint8_t bits[] = {0, 0, 1, 1, 0, 1, 0, 1};
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_EQUAL_INT(0, Block_write_bit(&test_block, bits[i]));
    }
    TEST_ASSERT_EQUAL_INT(1, test_block.pos);
    TEST_ASSERT_EQUAL_UINT8(0xAC, test_buf[0]);
    TEST_ASSERT_EQUAL_INT(0, test_block.cur_len);
}

void test_write_bit_auto_flushes_on_8th_bit() {
    for (int i = 0; i < 4; ++i) Block_write_bit(&test_block, 1);
    TEST_ASSERT_EQUAL_INT(4, test_block.cur_len);
    TEST_ASSERT_EQUAL_INT(0, test_block.pos);

    for (int i = 0; i < 4; ++i) Block_write_bit(&test_block, 0);
    TEST_ASSERT_EQUAL_INT(1, test_block.pos);
    TEST_ASSERT_EQUAL_UINT8(0x0F, test_buf[0]);
    TEST_ASSERT_EQUAL_INT(0, test_block.cur_len);
}

void test_bit_write_overflow_returns_error() {
    for (int i = 0; i < BLOCK_SIZE; ++i) Block_write_byte(&test_block, 0);
    TEST_ASSERT_EQUAL_INT(BLOCK_SIZE, test_block.pos);

    for (int i = 0; i < 7; ++i) Block_write_bit(&test_block, 1);
    TEST_ASSERT_EQUAL_INT(7, test_block.cur_len);

    TEST_ASSERT_EQUAL_INT(ERR_BLOCK_END, Block_write_bit(&test_block, 0));
    TEST_ASSERT_EQUAL_INT(BLOCK_SIZE, test_block.pos);
    TEST_ASSERT_EQUAL_INT(8, test_block.cur_len); 
}

void test_end_stream_flushes_partial_byte() {
    Block_write_bit(&test_block, 1);
    Block_write_bit(&test_block, 0);
    Block_write_bit(&test_block, 1);
    TEST_ASSERT_EQUAL_INT(3, test_block.cur_len);
    TEST_ASSERT_EQUAL_UINT8(0x05, test_block.cur_byte); 

    TEST_ASSERT_EQUAL_INT(0, Block_end_stream(&test_block));
    TEST_ASSERT_EQUAL_INT(1, test_block.pos);
    TEST_ASSERT_EQUAL_UINT8(0x05, test_buf[0]);
    TEST_ASSERT_EQUAL_INT(0, test_block.cur_len);
}

void test_end_stream_with_no_pending_bits_returns_success() {
    Block_write_byte(&test_block, 0x42);
    TEST_ASSERT_EQUAL_INT(0, test_block.cur_len);

    TEST_ASSERT_EQUAL_INT(0, Block_end_stream(&test_block));
    TEST_ASSERT_EQUAL_INT(1, test_block.pos);
}

void test_end_stream_overflow_returns_error() {
    for (int i = 0; i < BLOCK_SIZE; ++i) Block_write_byte(&test_block, 0);
    for (int i = 0; i < 3; ++i) Block_write_bit(&test_block, 1);
    
    TEST_ASSERT_EQUAL_INT(ERR_BLOCK_END, Block_end_stream(&test_block));
    TEST_ASSERT_EQUAL_INT(BLOCK_SIZE, test_block.pos);
}

void test_mixed_writes_sequence() {
    Block_write_byte(&test_block, 0xFF);       
    Block_write_bit(&test_block, 1);          
    Block_write_bit(&test_block, 0);          
    Block_write_byte(&test_block, 0x55);       

    TEST_ASSERT_EQUAL_INT(0, Block_end_stream(&test_block));
    TEST_ASSERT_EQUAL_UINT8(0xFF, test_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x55, test_buf[1]);
    TEST_ASSERT_EQUAL_INT(2, test_block.pos);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_start_stream_initializes_all_fields);
    RUN_TEST(test_write_byte_stores_and_increments_pos);
    RUN_TEST(test_write_byte_overflow_returns_error);
    RUN_TEST(test_write_bit_packs_lsb_first);
    RUN_TEST(test_write_bit_auto_flushes_on_8th_bit);
    RUN_TEST(test_bit_write_overflow_returns_error);
    RUN_TEST(test_end_stream_flushes_partial_byte);
    RUN_TEST(test_end_stream_with_no_pending_bits_returns_success);
    RUN_TEST(test_end_stream_overflow_returns_error);
    RUN_TEST(test_mixed_writes_sequence);
    return UNITY_END();
}

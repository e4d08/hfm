#include "block.h"
#include "unity.h"
#include <string.h>

static Block   test_block;
static uint8_t test_buf[BLOCK_SIZE];

void setUp(void)
{
    memset(test_buf, 0xDE, sizeof(test_buf));
    block_start_stream(&test_block, test_buf);
}

void tearDown(void) {}

/* ─────────────── block_start_stream ─────────────── */

void test_start_stream_initializes_all_fields(void)
{
    TEST_ASSERT_EQUAL_PTR(test_buf, test_block.buf);
    TEST_ASSERT_EQUAL_UINT16(0, test_block.pos);
    TEST_ASSERT_EQUAL_HEX8(0x00, test_block.cur_byte);
    TEST_ASSERT_EQUAL_UINT8(0, test_block.cur_len);

    /* start_stream must zero the output buffer */
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        TEST_ASSERT_EQUAL_HEX8(0x00, test_buf[i]);
    }
}

/* ─────────────── block_write_bit ─────────────── */

void test_write_bit_packs_lsb_first(void)
{
    /* bits: 0 0 1 1 0 1 0 1 → byte = 0b10101100 = 0xAC */
    uint8_t bits[] = { 0, 0, 1, 1, 0, 1, 0, 1 };
    for (int i = 0; i < 8; ++i) {
        block_write_bit(&test_block, bits[i]);
    }
    TEST_ASSERT_EQUAL_UINT16(1, test_block.pos);
    TEST_ASSERT_EQUAL_HEX8(0xAC, test_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0, test_block.cur_len);
}

void test_write_bit_auto_flushes_on_eighth_bit(void)
{
    /* 4 ones → cur_len == 4, nothing flushed yet */
    for (int i = 0; i < 4; ++i)
        block_write_bit(&test_block, 1);
    TEST_ASSERT_EQUAL_UINT8(4, test_block.cur_len);
    TEST_ASSERT_EQUAL_UINT16(0, test_block.pos);

    /* 4 zeros → cur_len reaches 8, byte is flushed */
    for (int i = 0; i < 4; ++i)
        block_write_bit(&test_block, 0);
    TEST_ASSERT_EQUAL_UINT16(1, test_block.pos);
    TEST_ASSERT_EQUAL_HEX8(0x0F, test_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0, test_block.cur_len);
}

void test_write_bit_multiple_bytes(void)
{
    /* Write 16 ones → two bytes of 0xFF */
    for (int i = 0; i < 16; ++i)
        block_write_bit(&test_block, 1);

    TEST_ASSERT_EQUAL_UINT16(2, test_block.pos);
    TEST_ASSERT_EQUAL_HEX8(0xFF, test_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, test_buf[1]);
}

void test_bit_write_overflow_returns_error(void)
{
    /* Fill the buffer completely with whole bytes: BLOCK_SIZE * 8 bits */
    for (int i = 0; i < BLOCK_SIZE * 8; ++i)
        block_write_bit(&test_block, 0);

    TEST_ASSERT_EQUAL_UINT16(BLOCK_SIZE, test_block.pos);
    TEST_ASSERT_EQUAL_UINT8(0, test_block.cur_len);

    /* 7 more bits fit into cur_byte without flushing */
    for (int i = 0; i < 7; ++i)
        block_write_bit(&test_block, 1);
    TEST_ASSERT_EQUAL_UINT8(7, test_block.cur_len);

    /* 8th bit triggers flush → buffer is full → error */
    block_write_bit(&test_block, 0);
    TEST_ASSERT_EQUAL_UINT16(BLOCK_SIZE, test_block.pos);
    TEST_ASSERT_EQUAL_UINT8(8, test_block.cur_len);
}

/* ─────────────── block_end_stream ─────────────── */

void test_end_stream_flushes_partial_byte(void)
{
    /* bits: 1 0 1 → cur_byte = 0b00000101 = 0x05, cur_len = 3 */
    block_write_bit(&test_block, 1);
    block_write_bit(&test_block, 0);
    block_write_bit(&test_block, 1);

    TEST_ASSERT_EQUAL_UINT8(3, test_block.cur_len);
    TEST_ASSERT_EQUAL_HEX8(0x05, test_block.cur_byte);

    block_end_stream(&test_block);

    TEST_ASSERT_EQUAL_UINT16(1, test_block.pos);
    TEST_ASSERT_EQUAL_HEX8(0x05, test_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0, test_block.cur_len);
}

void test_end_stream_with_no_pending_bits_returns_success(void)
{
    /* Write exactly 8 bits → auto-flushed, cur_len == 0 */
    for (int i = 0; i < 8; ++i)
        block_write_bit(&test_block, 0);

    TEST_ASSERT_EQUAL_UINT8(0, test_block.cur_len);
    block_end_stream(&test_block);
    TEST_ASSERT_EQUAL_UINT16(1, test_block.pos);
}

void test_end_stream_overflow_returns_error(void)
{
    /* Fill the buffer to the brim: BLOCK_SIZE * 8 bits */
    for (int i = 0; i < BLOCK_SIZE * 8; ++i)
        block_write_bit(&test_block, 0);

    /* Add partial byte */
    for (int i = 0; i < 3; ++i)
        block_write_bit(&test_block, 1);

    TEST_ASSERT_EQUAL_UINT8(3, test_block.cur_len);

    /* end_stream tries to flush → no room → error */
    block_end_stream(&test_block);
    TEST_ASSERT_EQUAL_UINT16(BLOCK_SIZE, test_block.pos);
}

/* ─────────────── BlockHeader write / read ─────────────── */

void test_header_write_serializes_correctly(void)
{
    BlockHeader hdr = {
        .flags      = BLOCK_UNCOMPRESSED,
        .block_size = 0x1234,
        .data_size  = 0xABCD,
    };
    uint8_t buf[BLOCK_HEADER_SIZE] = { 0 };

    block_header_write(&hdr, buf);

    TEST_ASSERT_EQUAL_HEX8(BLOCK_UNCOMPRESSED, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x12, buf[1]); /* block_size high byte */
    TEST_ASSERT_EQUAL_HEX8(0x34, buf[2]); /* block_size low byte  */
    TEST_ASSERT_EQUAL_HEX8(0xAB, buf[3]); /* data_size high byte  */
    TEST_ASSERT_EQUAL_HEX8(0xCD, buf[4]); /* data_size low byte   */
}

void test_header_read_deserializes_correctly(void)
{
    uint8_t buf[BLOCK_HEADER_SIZE] = {
        0x01,       /* flags      */
        0x12, 0x34, /* block_size */
        0xAB, 0xCD, /* data_size  */
    };
    BlockHeader hdr = { 0 };

    block_header_read(buf, &hdr);

    TEST_ASSERT_EQUAL_HEX8(BLOCK_UNCOMPRESSED, hdr.flags);
    TEST_ASSERT_EQUAL_HEX16(0x1234, hdr.block_size);
    TEST_ASSERT_EQUAL_HEX16(0xABCD, hdr.data_size);
}

void test_header_roundtrip_preserves_values(void)
{
    BlockHeader original = {
        .flags      = 0,
        .block_size = BLOCK_SIZE,
        .data_size  = 1000,
    };
    uint8_t     buf[BLOCK_HEADER_SIZE] = { 0 };
    BlockHeader restored               = { 0 };

    block_header_write(&original, buf);
    block_header_read(buf, &restored);

    TEST_ASSERT_EQUAL_HEX8(original.flags, restored.flags);
    TEST_ASSERT_EQUAL_HEX16(original.block_size, restored.block_size);
    TEST_ASSERT_EQUAL_HEX16(original.data_size, restored.data_size);
}

void test_header_roundtrip_zero_values(void)
{
    BlockHeader original = { .flags = 0, .block_size = 0, .data_size = 0 };
    uint8_t     buf[BLOCK_HEADER_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    BlockHeader restored               = { .flags = 0xAA };

    block_header_write(&original, buf);
    block_header_read(buf, &restored);

    TEST_ASSERT_EQUAL_HEX8(0, restored.flags);
    TEST_ASSERT_EQUAL_HEX16(0, restored.block_size);
    TEST_ASSERT_EQUAL_HEX16(0, restored.data_size);
}

/* ─────────────── Mixed bit sequences ─────────────── */

void test_mixed_bit_pattern(void)
{
    /* Alternating 0, 1, 0, 1, ... with LSB-first → byte = 0b10101010 = 0xAA */
    for (int i = 0; i < 8; ++i)
        block_write_bit(&test_block, (uint8_t)i % 2);

    TEST_ASSERT_EQUAL_HEX8(0xAA, test_buf[0]);

    /* Now all ones → 0xFF */
    for (int i = 0; i < 8; ++i)
        block_write_bit(&test_block, 1);

    TEST_ASSERT_EQUAL_HEX8(0xFF, test_buf[1]);
}

void test_write_bits_then_end_stream_partial(void)
{
    /* 3 full bytes via bits */
    for (int i = 0; i < 24; ++i)
        block_write_bit(&test_block, 1);

    /* 5 more bits (partial byte) */
    for (int i = 0; i < 5; ++i)
        block_write_bit(&test_block, 1);

    TEST_ASSERT_EQUAL_UINT16(3, test_block.pos);
    TEST_ASSERT_EQUAL_UINT8(5, test_block.cur_len);

    block_end_stream(&test_block);

    TEST_ASSERT_EQUAL_UINT16(4, test_block.pos);
    /* 5 ones in LSB → 0b00011111 = 0x1F */
    TEST_ASSERT_EQUAL_HEX8(0x1F, test_buf[3]);
}

int main(void)
{
    UNITY_BEGIN();

    /* block_start_stream */
    RUN_TEST(test_start_stream_initializes_all_fields);

    /* block_write_bit */
    RUN_TEST(test_write_bit_packs_lsb_first);
    RUN_TEST(test_write_bit_auto_flushes_on_eighth_bit);
    RUN_TEST(test_write_bit_multiple_bytes);
    RUN_TEST(test_bit_write_overflow_returns_error);

    /* block_end_stream */
    RUN_TEST(test_end_stream_flushes_partial_byte);
    RUN_TEST(test_end_stream_with_no_pending_bits_returns_success);
    RUN_TEST(test_end_stream_overflow_returns_error);

    /* BlockHeader */
    RUN_TEST(test_header_write_serializes_correctly);
    RUN_TEST(test_header_read_deserializes_correctly);
    RUN_TEST(test_header_roundtrip_preserves_values);
    RUN_TEST(test_header_roundtrip_zero_values);

    /* Mixed patterns */
    RUN_TEST(test_mixed_bit_pattern);
    RUN_TEST(test_write_bits_then_end_stream_partial);

    return UNITY_END();
}

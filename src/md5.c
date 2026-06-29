#include "md5.h"

#include <stdio.h>
#include <string.h>

static const uint32_t md5_shift[64] = {
    7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U,
    5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U,
    4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U,
    6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U
};

static const uint32_t md5_table[64] = {
    0xd76aa478U, 0xe8c7b756U, 0x242070dbU, 0xc1bdceeeU,
    0xf57c0fafU, 0x4787c62aU, 0xa8304613U, 0xfd469501U,
    0x698098d8U, 0x8b44f7afU, 0xffff5bb1U, 0x895cd7beU,
    0x6b901122U, 0xfd987193U, 0xa679438eU, 0x49b40821U,
    0xf61e2562U, 0xc040b340U, 0x265e5a51U, 0xe9b6c7aaU,
    0xd62f105dU, 0x02441453U, 0xd8a1e681U, 0xe7d3fbc8U,
    0x21e1cde6U, 0xc33707d6U, 0xf4d50d87U, 0x455a14edU,
    0xa9e3e905U, 0xfcefa3f8U, 0x676f02d9U, 0x8d2a4c8aU,
    0xfffa3942U, 0x8771f681U, 0x6d9d6122U, 0xfde5380cU,
    0xa4beea44U, 0x4bdecfa9U, 0xf6bb4b60U, 0xbebfbc70U,
    0x289b7ec6U, 0xeaa127faU, 0xd4ef3085U, 0x04881d05U,
    0xd9d4d039U, 0xe6db99e5U, 0x1fa27cf8U, 0xc4ac5665U,
    0xf4292244U, 0x432aff97U, 0xab9423a7U, 0xfc93a039U,
    0x655b59c3U, 0x8f0ccc92U, 0xffeff47dU, 0x85845dd1U,
    0x6fa87e4fU, 0xfe2ce6e0U, 0xa3014314U, 0x4e0811a1U,
    0xf7537e82U, 0xbd3af235U, 0x2ad7d2bbU, 0xeb86d391U
};

static uint32_t left_rotate(uint32_t value, uint32_t count)
{
    return (value << count) | (value >> (32U - count));
}

static void md5_get_state(const MD5_CTX *ctx, uint32_t state[4])
{
    state[0] = ctx->A;
    state[1] = ctx->B;
    state[2] = ctx->C;
    state[3] = ctx->D;
}

static void md5_set_state(MD5_CTX *ctx, const uint32_t state[4])
{
    ctx->A = state[0];
    ctx->B = state[1];
    ctx->C = state[2];
    ctx->D = state[3];
}

static void md5_transform(uint32_t state[4], const uint8_t block[64])
{
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t words[16];
    uint32_t round;

    for (round = 0; round < 16U; ++round) {
        words[round] = (uint32_t)block[round * 4U] |
                       ((uint32_t)block[round * 4U + 1U] << 8U) |
                       ((uint32_t)block[round * 4U + 2U] << 16U) |
                       ((uint32_t)block[round * 4U + 3U] << 24U);
    }

    for (round = 0; round < 64U; ++round) {
        uint32_t f;
        uint32_t g;
        uint32_t temp;

        if (round < 16U) {
            f = (b & c) | ((~b) & d);
            g = round;
        } else if (round < 32U) {
            f = (d & b) | ((~d) & c);
            g = (5U * round + 1U) % 16U;
        } else if (round < 48U) {
            f = b ^ c ^ d;
            g = (3U * round + 5U) % 16U;
        } else {
            f = c ^ (b | (~d));
            g = (7U * round) % 16U;
        }

        temp = d;
        d = c;
        c = b;
        b += left_rotate(a + f + md5_table[round] + words[g], md5_shift[round]);
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

void md5_init(MD5_CTX *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->count = 0U;
    ctx->A = 0x67452301U;
    ctx->B = 0xefcdab89U;
    ctx->C = 0x98badcfeU;
    ctx->D = 0x10325476U;
    (void)memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

void md5_update(MD5_CTX *ctx, const uint8_t *input, size_t length)
{
    uint32_t state[4];
    size_t index = 0U;
    size_t part_len = 0U;
    size_t offset = 0U;

    if (ctx == NULL || (input == NULL && length > 0U)) {
        return;
    }

    index = (size_t)((ctx->count / 8U) % 64U);
    part_len = 64U - index;
    md5_get_state(ctx, state);

    ctx->count += (uint64_t)length * 8U;

    if (length >= part_len) {
        (void)memcpy(&ctx->buffer[index], input, part_len);
        md5_transform(state, ctx->buffer);
        offset = part_len;

        while (offset + 63U < length) {
            md5_transform(state, &input[offset]);
            offset += 64U;
        }

        index = 0U;
    }

    (void)memcpy(&ctx->buffer[index], &input[offset], length - offset);
    md5_set_state(ctx, state);
}

void md5_final(MD5_CTX *ctx, uint8_t digest[MD5_DIGEST_SIZE])
{
    static const uint8_t padding[64] = {0x80U};
    uint8_t bit_length[8];
    size_t index;
    size_t pad_len;

    if (ctx == NULL || digest == NULL) {
        return;
    }

    for (index = 0; index < 8U; ++index) {
        bit_length[index] = (uint8_t)((ctx->count >> (index * 8U)) & 0xffU);
    }

    index = (size_t)((ctx->count / 8U) % 64U);
    pad_len = (index < 56U) ? (56U - index) : (120U - index);

    md5_update(ctx, padding, pad_len);
    md5_update(ctx, bit_length, sizeof(bit_length));

    digest[0] = (uint8_t)(ctx->A & 0xffU);
    digest[1] = (uint8_t)((ctx->A >> 8U) & 0xffU);
    digest[2] = (uint8_t)((ctx->A >> 16U) & 0xffU);
    digest[3] = (uint8_t)((ctx->A >> 24U) & 0xffU);
    digest[4] = (uint8_t)(ctx->B & 0xffU);
    digest[5] = (uint8_t)((ctx->B >> 8U) & 0xffU);
    digest[6] = (uint8_t)((ctx->B >> 16U) & 0xffU);
    digest[7] = (uint8_t)((ctx->B >> 24U) & 0xffU);
    digest[8] = (uint8_t)(ctx->C & 0xffU);
    digest[9] = (uint8_t)((ctx->C >> 8U) & 0xffU);
    digest[10] = (uint8_t)((ctx->C >> 16U) & 0xffU);
    digest[11] = (uint8_t)((ctx->C >> 24U) & 0xffU);
    digest[12] = (uint8_t)(ctx->D & 0xffU);
    digest[13] = (uint8_t)((ctx->D >> 8U) & 0xffU);
    digest[14] = (uint8_t)((ctx->D >> 16U) & 0xffU);
    digest[15] = (uint8_t)((ctx->D >> 24U) & 0xffU);

    (void)memset(ctx, 0, sizeof(*ctx));
}

void md5_string(const char *str, uint8_t digest[MD5_DIGEST_SIZE])
{
    MD5_CTX ctx;

    if (str == NULL || digest == NULL) {
        return;
    }

    md5_init(&ctx);
    md5_update(&ctx, (const uint8_t *)str, strlen(str));
    md5_final(&ctx, digest);
}

int md5_file(const char *filename, uint8_t digest[MD5_DIGEST_SIZE])
{
    FILE *file = fopen(filename, "rb");
    MD5_CTX ctx;
    uint8_t buffer[MD5_BUFFER_SIZE];
    size_t bytes_read;

    if (filename == NULL || digest == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    if (file == NULL) {
        return MD5_ERROR_FILE_OPEN;
    }

    md5_init(&ctx);
    while ((bytes_read = fread(buffer, 1U, sizeof(buffer), file)) > 0U) {
        md5_update(&ctx, buffer, bytes_read);
    }

    if (ferror(file) != 0) {
        (void)fclose(file);
        return MD5_ERROR_FILE_READ;
    }

    (void)fclose(file);
    md5_final(&ctx, digest);
    return MD5_SUCCESS;
}
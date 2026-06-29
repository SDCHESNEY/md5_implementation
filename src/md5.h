#ifndef MD5_H
#define MD5_H

#include <stddef.h>
#include <stdint.h>

#include "config.h"

typedef struct {
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
    uint64_t count;
    uint8_t buffer[64];
} MD5_CTX;

void md5_init(MD5_CTX *ctx);
void md5_update(MD5_CTX *ctx, const uint8_t *input, size_t length);
void md5_final(MD5_CTX *ctx, uint8_t digest[MD5_DIGEST_SIZE]);
void md5_string(const char *str, uint8_t digest[MD5_DIGEST_SIZE]);
int md5_file(const char *filename, uint8_t digest[MD5_DIGEST_SIZE]);

#endif
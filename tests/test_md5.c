#include <stdio.h>
#include <string.h>

#include "../src/md5.h"
#include "../src/utils.h"

typedef struct {
    const char *input;
    const char *expected;
} Md5Vector;

static int assert_digest(const char *input, const char *expected)
{
    uint8_t digest[MD5_DIGEST_SIZE];
    char hex[MD5_HEX_SIZE];

    md5_string(input, digest);
    hex_encode(digest, MD5_DIGEST_SIZE, hex);

    if (strcmp(hex, expected) != 0) {
        fprintf(stderr, "expected %s but got %s for input '%s'\n", expected, hex, input);
        return 1;
    }

    return 0;
}

static int assert_incremental_digest(const char *input, const char *expected, size_t split)
{
    MD5_CTX ctx;
    uint8_t digest[MD5_DIGEST_SIZE];
    char hex[MD5_HEX_SIZE];
    size_t input_len = strlen(input);

    md5_init(&ctx);
    md5_update(&ctx, (const uint8_t *)input, split);
    md5_update(&ctx, (const uint8_t *)(input + split), input_len - split);
    md5_final(&ctx, digest);
    hex_encode(digest, MD5_DIGEST_SIZE, hex);

    if (strcmp(hex, expected) != 0) {
        fprintf(stderr, "incremental expected %s but got %s for input '%s'\n", expected, hex, input);
        return 1;
    }

    return 0;
}

static int assert_binary_digest(void)
{
    static const uint8_t input[] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0xffU};
    MD5_CTX ctx;
    uint8_t digest[MD5_DIGEST_SIZE];
    char hex[MD5_HEX_SIZE];

    md5_init(&ctx);
    md5_update(&ctx, input, sizeof(input));
    md5_final(&ctx, digest);
    hex_encode(digest, MD5_DIGEST_SIZE, hex);

    if (strcmp(hex, "883854cc38bdf2845e86b9b4b7e31192") != 0) {
        fprintf(stderr, "binary expected 883854cc38bdf2845e86b9b4b7e31192 but got %s\n", hex);
        return 1;
    }

    return 0;
}

int main(void)
{
    static const Md5Vector vectors[] = {
        {"", "d41d8cd98f00b204e9800998ecf8427e"},
        {"abc", "900150983cd24fb0d6963f7d28e17f72"},
        {"message digest", "f96b697d7cb7938d525a2f31aaf161d0"}
    };
    size_t index;

    for (index = 0; index < sizeof(vectors) / sizeof(vectors[0]); ++index) {
        if (assert_digest(vectors[index].input, vectors[index].expected) != 0) {
            return 1;
        }
    }

    if (assert_incremental_digest("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b", 13U) != 0) {
        return 1;
    }

    if (assert_binary_digest() != 0) {
        return 1;
    }

    puts("test_md5 passed");
    return 0;
}
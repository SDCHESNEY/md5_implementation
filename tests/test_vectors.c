#include <stdio.h>
#include <string.h>

#include "../src/md5.h"
#include "../src/utils.h"

typedef struct {
    const char *input;
    const char *expected;
} Vector;

int main(void)
{
    static const Vector vectors[] = {
        {"", "d41d8cd98f00b204e9800998ecf8427e"},
        {"a", "0cc175b9c0f1b6a831c399e269772661"},
        {"abc", "900150983cd24fb0d6963f7d28e17f72"},
        {"message digest", "f96b697d7cb7938d525a2f31aaf161d0"},
        {"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"},
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f"},
        {"12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a"}
    };
    size_t index;

    for (index = 0; index < sizeof(vectors) / sizeof(vectors[0]); ++index) {
        uint8_t digest[MD5_DIGEST_SIZE];
        char hex[MD5_HEX_SIZE];

        md5_string(vectors[index].input, digest);
        hex_encode(digest, MD5_DIGEST_SIZE, hex);
        if (strcmp(hex, vectors[index].expected) != 0) {
            fprintf(stderr, "vector %zu failed: expected %s got %s\n", index, vectors[index].expected, hex);
            return 1;
        }
    }

    puts("test_vectors passed");
    return 0;
}
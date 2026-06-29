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

    puts("test_md5 passed");
    return 0;
}
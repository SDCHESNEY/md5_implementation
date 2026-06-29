#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/file_handler.h"
#include "../src/utils.h"

static int write_fixture(const char *path, const char *contents)
{
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        return 1;
    }

    (void)fwrite(contents, 1U, strlen(contents), file);
    (void)fclose(file);
    return 0;
}

int main(void)
{
    const char *path = "tests/tmp_file_handler.txt";
    uint8_t digest[MD5_DIGEST_SIZE];
    char hex[MD5_HEX_SIZE];

    if (write_fixture(path, "hello") != 0) {
        fprintf(stderr, "unable to create fixture\n");
        return 1;
    }

    if (!file_exists(path)) {
        fprintf(stderr, "file_exists failed\n");
        return 1;
    }

    if (file_size(path) != 5U) {
        fprintf(stderr, "file_size failed\n");
        return 1;
    }

    if (hash_file(path, FILE_MODE_TEXT, digest) != MD5_SUCCESS) {
        fprintf(stderr, "hash_file failed\n");
        return 1;
    }

    hex_encode(digest, MD5_DIGEST_SIZE, hex);
    remove(path);

    if (strcmp(hex, "5d41402abc4b2a76b9719d911017c592") != 0) {
        fprintf(stderr, "unexpected digest %s\n", hex);
        return 1;
    }

    if (hash_file("tests/does-not-exist.txt", FILE_MODE_TEXT, digest) == MD5_SUCCESS) {
        fprintf(stderr, "missing file should fail\n");
        return 1;
    }

    puts("test_file_handler passed");
    return 0;
}
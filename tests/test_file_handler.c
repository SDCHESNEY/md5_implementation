#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static int write_binary_fixture(const char *path)
{
    static const uint8_t bytes[] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0xffU};
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        return 1;
    }

    (void)fwrite(bytes, 1U, sizeof(bytes), file);
    (void)fclose(file);
    return 0;
}

static int test_read_stdin(void)
{
    const char *path = "tests/tmp_stdin.bin";
    uint8_t digest[MD5_DIGEST_SIZE];
    char hex[MD5_HEX_SIZE];
    int saved_stdin;

    if (write_binary_fixture(path) != 0) {
        return 1;
    }

    saved_stdin = dup(fileno(stdin));
    if (saved_stdin < 0) {
        remove(path);
        return 1;
    }

    if (freopen(path, "rb", stdin) == NULL) {
        close(saved_stdin);
        remove(path);
        return 1;
    }

    if (read_stdin(digest) != MD5_SUCCESS) {
        close(saved_stdin);
        remove(path);
        return 1;
    }

    if (dup2(saved_stdin, fileno(stdin)) < 0) {
        close(saved_stdin);
        remove(path);
        return 1;
    }

    close(saved_stdin);
    hex_encode(digest, MD5_DIGEST_SIZE, hex);
    remove(path);

    return strcmp(hex, "883854cc38bdf2845e86b9b4b7e31192") != 0;
}

int main(void)
{
    const char *path = "tests/tmp_file_handler.txt";
    const char *binary_path = "tests/tmp_file_handler.bin";
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

    if (write_binary_fixture(binary_path) != 0) {
        fprintf(stderr, "unable to create binary fixture\n");
        return 1;
    }

    if (hash_file(binary_path, FILE_MODE_BINARY, digest) != MD5_SUCCESS) {
        fprintf(stderr, "hash_file binary failed\n");
        remove(binary_path);
        return 1;
    }

    hex_encode(digest, MD5_DIGEST_SIZE, hex);
    remove(binary_path);

    if (strcmp(hex, "883854cc38bdf2845e86b9b4b7e31192") != 0) {
        fprintf(stderr, "unexpected binary digest %s\n", hex);
        return 1;
    }

    if (test_read_stdin() != 0) {
        fprintf(stderr, "read_stdin failed\n");
        return 1;
    }

    if (hash_file("tests/does-not-exist.txt", FILE_MODE_TEXT, digest) == MD5_SUCCESS) {
        fprintf(stderr, "missing file should fail\n");
        return 1;
    }

    puts("test_file_handler passed");
    return 0;
}
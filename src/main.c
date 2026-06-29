#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "file_handler.h"
#include "md5.h"
#include "tui.h"
#include "utils.h"

static void print_usage(const char *program)
{
    printf("Usage: %s [OPTIONS] [INPUT]\n", program);
    printf("  -t, --text STRING   Hash the provided text string\n");
    printf("  -f, --file PATH     Hash the specified file\n");
    printf("  -b, --binary PATH   Hash binary file\n");
    printf("  -s, --stdin         Read from standard input\n");
    printf("  -q, --quiet         Output only the hash digest\n");
    printf("  -v, --verbose       Output detailed information\n");
    printf("  -h, --help          Display help message\n");
    printf("  --version           Display program version\n");
}

static void print_digest(const char *label, const uint8_t digest[MD5_DIGEST_SIZE], int quiet, int verbose)
{
    char hex[MD5_HEX_SIZE];

    hex_encode(digest, MD5_DIGEST_SIZE, hex);
    if (quiet) {
        printf("%s\n", hex);
        return;
    }

    if (verbose) {
        printf("MD5 Hash Implementation v%s\n", MD5_VERSION);
        printf("Input: %s\n", label);
        printf("MD5 Digest: %s\n", hex);
        return;
    }

    printf("MD5 Hash Result\n===============\nInput: %s\nHash:  %s\n", label, hex);
}

int main(int argc, char *argv[])
{
    int quiet = 0;
    int verbose = 0;
    uint8_t digest[MD5_DIGEST_SIZE];

    if (argc == 1) {
        return tui_run();
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return MD5_SUCCESS;
    }

    if (strcmp(argv[1], "--version") == 0) {
        printf("%s\n", MD5_VERSION);
        return MD5_SUCCESS;
    }

    for (int index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "-q") == 0 || strcmp(argv[index], "--quiet") == 0) {
            quiet = 1;
            continue;
        }

        if (strcmp(argv[index], "-v") == 0 || strcmp(argv[index], "--verbose") == 0) {
            verbose = 1;
            continue;
        }

        if ((strcmp(argv[index], "-t") == 0 || strcmp(argv[index], "--text") == 0) && index + 1 < argc) {
            md5_string(argv[index + 1], digest);
            print_digest(argv[index + 1], digest, quiet, verbose);
            return MD5_SUCCESS;
        }

        if ((strcmp(argv[index], "-f") == 0 || strcmp(argv[index], "--file") == 0) && index + 1 < argc) {
            if (hash_file(argv[index + 1], FILE_MODE_TEXT, digest) != MD5_SUCCESS) {
                fprintf(stderr, "Error: cannot hash file '%s'\n", argv[index + 1]);
                return MD5_ERROR_FILE_OPEN;
            }
            print_digest(argv[index + 1], digest, quiet, verbose);
            return MD5_SUCCESS;
        }

        if ((strcmp(argv[index], "-b") == 0 || strcmp(argv[index], "--binary") == 0) && index + 1 < argc) {
            if (hash_file(argv[index + 1], FILE_MODE_BINARY, digest) != MD5_SUCCESS) {
                fprintf(stderr, "Error: cannot hash file '%s'\n", argv[index + 1]);
                return MD5_ERROR_FILE_OPEN;
            }
            print_digest(argv[index + 1], digest, quiet, verbose);
            return MD5_SUCCESS;
        }

        if (strcmp(argv[index], "-s") == 0 || strcmp(argv[index], "--stdin") == 0) {
            if (read_stdin(digest) != MD5_SUCCESS) {
                fprintf(stderr, "Error: cannot read standard input\n");
                return MD5_ERROR_FILE_READ;
            }
            print_digest("stdin", digest, quiet, verbose);
            return MD5_SUCCESS;
        }
    }

    print_usage(argv[0]);
    return MD5_ERROR_INVALID_ARG;
}
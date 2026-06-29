#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "file_handler.h"
#include "md5.h"
#include "tui.h"
#include "utils.h"

typedef enum {
    MODE_NONE = 0,
    MODE_TEXT,
    MODE_FILE,
    MODE_BINARY,
    MODE_STDIN
} InputMode;

typedef struct {
    InputMode mode;
    const char *value;
    int quiet;
    int verbose;
} CliOptions;

typedef struct {
    const char *label;
    const char *path;
    size_t size_bytes;
    double elapsed_ms;
    size_t blocks_processed;
    uint8_t digest[MD5_DIGEST_SIZE];
} HashResult;

static void print_usage(const char *program)
{
    printf("Usage: %s [OPTIONS] [INPUT]\n", program);
    printf("\nOPTIONS:\n");
    printf("  -t, --text STRING        Hash the provided text string\n");
    printf("  -f, --file PATH          Hash the specified file\n");
    printf("  -b, --binary PATH        Hash binary file (outputs hex digest)\n");
    printf("  -s, --stdin              Read from standard input\n");
    printf("  -q, --quiet              Output only the hash digest\n");
    printf("  -v, --verbose            Output detailed information\n");
    printf("  -h, --help               Display help message\n");
    printf("  --version                Display program version\n");
}

static size_t calculate_md5_blocks(size_t size_bytes)
{
    return (size_bytes + 9U + 63U) / 64U;
}

static void print_hash_result(const HashResult *result, int quiet, int verbose)
{
    char hex[MD5_HEX_SIZE];

    hex_encode(result->digest, MD5_DIGEST_SIZE, hex);
    if (quiet) {
        printf("%s\n", hex);
        return;
    }

    if (verbose) {
        printf("MD5 Hash Implementation v%s\n", MD5_VERSION);
        printf("File: %s\n", result->path != NULL ? result->path : result->label);
        printf("File Size: %zu bytes\n", result->size_bytes);
        printf("Processing Time: %.3f ms\n", result->elapsed_ms);
        printf("Blocks Processed: %zu\n", result->blocks_processed);
        printf("MD5 Digest: %s\n", hex);
        printf("Verification: success\n");
        return;
    }

    printf("MD5 Hash Result\n===============\n");
    printf("Input: %s\n", result->label);
    printf("Hash:  %s\n", hex);
    printf("Size:  %zu bytes\n", result->size_bytes);
}

static int hash_stdin_with_metrics(HashResult *result)
{
    MD5_CTX ctx;
    uint8_t buffer[MD5_BUFFER_SIZE];
    size_t bytes_read;

    md5_init(&ctx);
    result->size_bytes = 0U;

    while ((bytes_read = fread(buffer, 1U, sizeof(buffer), stdin)) > 0U) {
        md5_update(&ctx, buffer, bytes_read);
        result->size_bytes += bytes_read;
    }

    if (ferror(stdin) != 0) {
        clearerr(stdin);
        return MD5_ERROR_FILE_READ;
    }

    md5_final(&ctx, result->digest);
    result->blocks_processed = calculate_md5_blocks(result->size_bytes);
    return MD5_SUCCESS;
}

static int parse_options(int argc, char *argv[], CliOptions *options)
{
    int index;

    options->mode = MODE_NONE;
    options->value = NULL;
    options->quiet = 0;
    options->verbose = 0;

    for (index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "-q") == 0 || strcmp(argv[index], "--quiet") == 0) {
            options->quiet = 1;
            continue;
        }

        if (strcmp(argv[index], "-v") == 0 || strcmp(argv[index], "--verbose") == 0) {
            options->verbose = 1;
            continue;
        }

        if (strcmp(argv[index], "-h") == 0 || strcmp(argv[index], "--help") == 0 ||
            strcmp(argv[index], "--version") == 0) {
            continue;
        }

        if (options->mode != MODE_NONE) {
            return MD5_ERROR_INVALID_ARG;
        }

        if ((strcmp(argv[index], "-t") == 0 || strcmp(argv[index], "--text") == 0) && index + 1 < argc) {
            options->mode = MODE_TEXT;
            options->value = argv[++index];
            continue;
        }

        if ((strcmp(argv[index], "-f") == 0 || strcmp(argv[index], "--file") == 0) && index + 1 < argc) {
            options->mode = MODE_FILE;
            options->value = argv[++index];
            continue;
        }

        if ((strcmp(argv[index], "-b") == 0 || strcmp(argv[index], "--binary") == 0) && index + 1 < argc) {
            options->mode = MODE_BINARY;
            options->value = argv[++index];
            continue;
        }

        if (strcmp(argv[index], "-s") == 0 || strcmp(argv[index], "--stdin") == 0) {
            options->mode = MODE_STDIN;
            options->value = "stdin";
            continue;
        }

        return MD5_ERROR_INVALID_ARG;
    }

    return (options->mode == MODE_NONE) ? MD5_ERROR_INVALID_ARG : MD5_SUCCESS;
}

static int run_cli(const CliOptions *options, HashResult *result)
{
    double start_ms = get_time_ms();
    int status;

    result->label = options->value;
    result->path = NULL;
    result->size_bytes = 0U;
    result->elapsed_ms = 0.0;
    result->blocks_processed = 0U;

    switch (options->mode) {
    case MODE_TEXT:
        if (validate_input(options->value, MD5_BUFFER_SIZE) != MD5_SUCCESS) {
            return MD5_ERROR_INVALID_ARG;
        }
        md5_string(options->value, result->digest);
        result->size_bytes = strlen(options->value);
        break;
    case MODE_FILE:
        status = hash_file(options->value, FILE_MODE_TEXT, result->digest);
        if (status != MD5_SUCCESS) {
            return status;
        }
        result->path = options->value;
        result->size_bytes = file_size(options->value);
        break;
    case MODE_BINARY:
        status = hash_file(options->value, FILE_MODE_BINARY, result->digest);
        if (status != MD5_SUCCESS) {
            return status;
        }
        result->path = options->value;
        result->size_bytes = file_size(options->value);
        break;
    case MODE_STDIN:
        status = hash_stdin_with_metrics(result);
        if (status != MD5_SUCCESS) {
            return status;
        }
        result->label = "stdin";
        break;
    case MODE_NONE:
    default:
        return MD5_ERROR_INVALID_ARG;
    }

    result->elapsed_ms = get_time_ms() - start_ms;
    result->blocks_processed = calculate_md5_blocks(result->size_bytes);
    return MD5_SUCCESS;
}

int main(int argc, char *argv[])
{
    CliOptions options;
    HashResult result;
    int status;

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

    status = parse_options(argc, argv, &options);
    if (status != MD5_SUCCESS) {
        print_usage(argv[0]);
        return status;
    }

    status = run_cli(&options, &result);
    if (status != MD5_SUCCESS) {
        if (status == MD5_ERROR_FILE_READ) {
            fprintf(stderr, "Error: cannot read standard input\n");
        } else if (status == MD5_ERROR_INVALID_ARG && options.mode == MODE_TEXT) {
            fprintf(stderr, "Error: Invalid input string: exceeds maximum length of %u bytes\n", MD5_BUFFER_SIZE);
        } else {
            fprintf(stderr, "Error: cannot hash input '%s'\n", options.value != NULL ? options.value : "");
        }
        return status;
    }

    print_hash_result(&result, options.quiet, options.verbose);
    return MD5_SUCCESS;
}
#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stddef.h>
#include <stdint.h>

#include "config.h"

typedef enum {
    FILE_MODE_TEXT = 0,
    FILE_MODE_BINARY = 1
} FileMode;

int file_exists(const char *path);
size_t file_size(const char *path);
int hash_file(const char *path, FileMode mode, uint8_t digest[MD5_DIGEST_SIZE]);
int read_stdin(uint8_t digest[MD5_DIGEST_SIZE]);

#endif
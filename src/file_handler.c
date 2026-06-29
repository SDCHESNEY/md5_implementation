#include "file_handler.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "md5.h"

int file_exists(const char *path)
{
    struct stat info;

    if (path == NULL) {
        return 0;
    }

    return stat(path, &info) == 0;
}

size_t file_size(const char *path)
{
    struct stat info;

    if (path == NULL || stat(path, &info) != 0 || !S_ISREG(info.st_mode)) {
        return 0U;
    }

    return (size_t)info.st_size;
}

int hash_file(const char *path, FileMode mode, uint8_t digest[MD5_DIGEST_SIZE])
{
    struct stat info;

    (void)mode;

    if (path == NULL || digest == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    if (stat(path, &info) != 0) {
        return (errno == EACCES) ? MD5_ERROR_PERMISSION : MD5_ERROR_FILE_OPEN;
    }

    if (!S_ISREG(info.st_mode)) {
        return MD5_ERROR_FILE_OPEN;
    }

    return md5_file(path, digest);
}

int read_stdin(uint8_t digest[MD5_DIGEST_SIZE])
{
    MD5_CTX ctx;
    uint8_t buffer[MD5_BUFFER_SIZE];
    size_t bytes_read;

    if (digest == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    md5_init(&ctx);
    while ((bytes_read = fread(buffer, 1U, sizeof(buffer), stdin)) > 0U) {
        md5_update(&ctx, buffer, bytes_read);
    }

    if (ferror(stdin) != 0) {
        clearerr(stdin);
        return MD5_ERROR_FILE_READ;
    }

    md5_final(&ctx, digest);
    return MD5_SUCCESS;
}
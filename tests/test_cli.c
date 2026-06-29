#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int capture_output(const char *command, char *buffer, size_t buffer_size)
{
    FILE *pipe = popen(command, "r");
    size_t offset = 0U;

    if (pipe == NULL) {
        return 1;
    }

    while (fgets(buffer + offset, (int)(buffer_size - offset), pipe) != NULL) {
        offset = strlen(buffer);
        if (offset >= buffer_size - 1U) {
            break;
        }
    }

    if (pclose(pipe) != 0) {
        return 1;
    }

    return 0;
}

static int require_contains(const char *haystack, const char *needle)
{
    return strstr(haystack, needle) == NULL;
}

int main(void)
{
    char output[1024];
    const char *fixture = "tests/tmp_cli.txt";

    if (write_fixture(fixture, "hello") != 0) {
        fprintf(stderr, "could not write CLI fixture\n");
        return 1;
    }

    output[0] = '\0';
    if (capture_output("./bin/md5 -t hello", output, sizeof(output)) != 0 ||
        require_contains(output, "MD5 Hash Result") != 0 ||
        require_contains(output, "Input: hello") != 0 ||
        require_contains(output, "Hash:  5d41402abc4b2a76b9719d911017c592") != 0 ||
        require_contains(output, "Size:  5 bytes") != 0) {
        fprintf(stderr, "standard text output mismatch\n%s", output);
        remove(fixture);
        return 1;
    }

    output[0] = '\0';
    if (capture_output("./bin/md5 -q -t hello", output, sizeof(output)) != 0 ||
        strcmp(output, "5d41402abc4b2a76b9719d911017c592\n") != 0) {
        fprintf(stderr, "quiet output mismatch\n%s", output);
        remove(fixture);
        return 1;
    }

    output[0] = '\0';
    if (capture_output("./bin/md5 -v -f tests/tmp_cli.txt", output, sizeof(output)) != 0 ||
        require_contains(output, "MD5 Hash Implementation v1.0.0") != 0 ||
        require_contains(output, "File: tests/tmp_cli.txt") != 0 ||
        require_contains(output, "File Size: 5 bytes") != 0 ||
        require_contains(output, "MD5 Digest: 5d41402abc4b2a76b9719d911017c592") != 0 ||
        require_contains(output, "Verification: success") != 0) {
        fprintf(stderr, "verbose output mismatch\n%s", output);
        remove(fixture);
        return 1;
    }

    output[0] = '\0';
    if (capture_output("printf 'hello' | ./bin/md5 -s", output, sizeof(output)) != 0 ||
        require_contains(output, "Input: stdin") != 0 ||
        require_contains(output, "Hash:  5d41402abc4b2a76b9719d911017c592") != 0) {
        fprintf(stderr, "stdin output mismatch\n%s", output);
        remove(fixture);
        return 1;
    }

    remove(fixture);
    puts("test_cli passed");
    return 0;
}
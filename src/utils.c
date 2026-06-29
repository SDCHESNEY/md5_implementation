#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

void hex_encode(const uint8_t *data, size_t len, char *output)
{
    static const char hex[] = "0123456789abcdef";
    size_t index;

    for (index = 0; index < len; ++index) {
        output[index * 2] = hex[(data[index] >> 4) & 0x0fU];
        output[index * 2 + 1] = hex[data[index] & 0x0fU];
    }

    output[len * 2] = '\0';
}

int hex_decode(const char *hex, uint8_t *output, size_t *len)
{
    size_t input_len;
    size_t index;

    if (hex == NULL || output == NULL || len == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    input_len = strlen(hex);
    if ((input_len % 2U) != 0U) {
        return MD5_ERROR_INVALID_ARG;
    }

    for (index = 0; index < input_len; index += 2U) {
        char hi = (char)tolower((unsigned char)hex[index]);
        char lo = (char)tolower((unsigned char)hex[index + 1U]);
        int hi_val = isdigit((unsigned char)hi) ? hi - '0' : hi - 'a' + 10;
        int lo_val = isdigit((unsigned char)lo) ? lo - '0' : lo - 'a' + 10;

        if ((!isdigit((unsigned char)hi) && (hi < 'a' || hi > 'f')) ||
            (!isdigit((unsigned char)lo) && (lo < 'a' || lo > 'f'))) {
            return MD5_ERROR_INVALID_ARG;
        }

        output[index / 2U] = (uint8_t)((hi_val << 4) | lo_val);
    }

    *len = input_len / 2U;
    return MD5_SUCCESS;
}

const char *format_file_size(size_t bytes, char *buffer, size_t buffer_size)
{
    static const char *units[] = {"B", "KB", "MB", "GB"};
    double value = (double)bytes;
    size_t unit = 0;

    while (value >= 1024.0 && unit < (sizeof(units) / sizeof(units[0])) - 1U) {
        value /= 1024.0;
        ++unit;
    }

    if (buffer_size == 0U) {
        return buffer;
    }

    if (unit == 0U) {
        (void)snprintf(buffer, buffer_size, "%zu %s", bytes, units[unit]);
    } else {
        (void)snprintf(buffer, buffer_size, "%.1f %s", value, units[unit]);
    }

    return buffer;
}

double get_time_ms(void)
{
    struct timeval value;

    (void)gettimeofday(&value, NULL);
    return (double)value.tv_sec * 1000.0 + (double)value.tv_usec / 1000.0;
}

int clipboard_copy(const char *text)
{
    const char *commands[] = {
#ifdef __APPLE__
        "pbcopy",
#endif
        "xclip -selection clipboard",
        "xsel --clipboard --input"
    };
    size_t command_index;

    if (text == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    for (command_index = 0U; command_index < sizeof(commands) / sizeof(commands[0]); ++command_index) {
        FILE *pipe = popen(commands[command_index], "w");
        int close_status;

        if (pipe == NULL) {
            continue;
        }

        if (fputs(text, pipe) == EOF) {
            (void)pclose(pipe);
            continue;
        }

        close_status = pclose(pipe);
        if (close_status == 0) {
            return MD5_SUCCESS;
        }
    }

    return MD5_ERROR_FILE_OPEN;
}

int validate_input(const char *input, size_t max_len)
{
    if (input == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    if (strlen(input) > max_len) {
        return MD5_ERROR_INVALID_ARG;
    }

    return MD5_SUCCESS;
}
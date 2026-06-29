#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

#include "config.h"

void hex_encode(const uint8_t *data, size_t len, char *output);
int hex_decode(const char *hex, uint8_t *output, size_t *len);
const char *format_file_size(size_t bytes, char *buffer, size_t buffer_size);
double get_time_ms(void);
int clipboard_copy(const char *text);
int validate_input(const char *input, size_t max_len);

#endif
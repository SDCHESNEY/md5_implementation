#ifndef TUI_H
#define TUI_H

#include <stddef.h>
#include <stdint.h>

#include "config.h"

typedef enum {
    STATE_MAIN_MENU = 0,
    STATE_HASH_TEXT,
    STATE_HASH_FILE,
    STATE_HASH_STDIN,
    STATE_RESULTS,
    STATE_SETTINGS,
    STATE_ABOUT
} UIState;

typedef struct {
    UIState state;
    char input_label[256];
    char digest_hex[MD5_HEX_SIZE];
    char status[256];
} TUIContext;

int tui_init(void);
void tui_cleanup(void);
int tui_run(void);
void tui_display_menu(void);
void tui_display_text_input(void);
void tui_display_file_browser(void);
void tui_display_results(const char *input, const uint8_t digest[MD5_DIGEST_SIZE]);
UIState tui_next_state_for_key(UIState state, int ch);
int tui_format_results(const char *input, const uint8_t digest[MD5_DIGEST_SIZE], char *buffer, size_t buffer_size);

#endif
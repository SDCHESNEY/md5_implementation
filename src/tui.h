#ifndef TUI_H
#define TUI_H

#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "file_handler.h"

typedef enum {
    STATE_MAIN_MENU = 0,
    STATE_HASH_TEXT,
    STATE_HASH_FILE,
    STATE_HASH_STDIN,
    STATE_RESULTS,
    STATE_SETTINGS,
    STATE_ABOUT
} UIState;

typedef enum {
    MENU_TEXT = 0,
    MENU_FILE,
    MENU_BINARY,
    MENU_STDIN,
    MENU_SETTINGS,
    MENU_ABOUT,
    MENU_EXIT,
    MENU_COUNT
} TUIMenuOption;

typedef struct {
    char name[256];
    int is_directory;
    size_t size_bytes;
} TUIFileEntry;

typedef struct {
    UIState state;
    TUIMenuOption menu_selection;
    FileMode file_mode;
    size_t selected_index;
    char input_label[256];
    char text_input[MD5_BUFFER_SIZE + 1U];
    char file_path[512];
    char digest_hex[MD5_HEX_SIZE];
    char status[256];
    double last_elapsed_ms;
    size_t last_size_bytes;
    size_t last_blocks_processed;
    int terminal_rows;
    int terminal_cols;
    int colors_enabled;
} TUIContext;

int tui_init(void);
void tui_cleanup(void);
int tui_run(void);
void tui_context_init(TUIContext *context);
void tui_update_terminal_size(TUIContext *context, int rows, int cols);
int tui_terminal_is_usable(const TUIContext *context);
const char *tui_state_title(UIState state);
void tui_handle_input(TUIContext *context, int ch);
void tui_display_menu(const TUIContext *context);
void tui_display_text_input(const TUIContext *context);
int tui_list_directory(const char *path, TUIFileEntry *entries, size_t max_entries, size_t *entry_count);
void tui_display_file_browser(const TUIContext *context, const TUIFileEntry *entries, size_t entry_count, size_t selected_index);
int tui_prepare_result(TUIContext *context, const char *label, const uint8_t digest[MD5_DIGEST_SIZE], size_t size_bytes, double elapsed_ms);
int tui_compute_text_digest(TUIContext *context);
int tui_compute_file_digest(TUIContext *context);
int tui_compute_stdin_digest(TUIContext *context);
int tui_format_results(const TUIContext *context, char *buffer, size_t buffer_size);
void tui_display_results(const TUIContext *context);

#endif
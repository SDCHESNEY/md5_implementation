#ifndef TUI_H
#define TUI_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "config.h"
#include "file_handler.h"

#define TUI_HISTORY_LIMIT 10U
#define TUI_NAV_DEPTH 32U

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
    time_t modified_time;
} TUIFileEntry;

typedef struct {
    UIState state;
    UIState return_state;
    TUIMenuOption menu_selection;
    FileMode file_mode;
    size_t selected_index;
    size_t text_cursor_pos;
    char input_label[256];
    char result_source[32];
    char text_input[MD5_BUFFER_SIZE + 1U];
    char file_path[512];
    char browser_filter[128];
    char digest_hex[MD5_HEX_SIZE];
    char result_save_path[512];
    char status[256];
    double last_elapsed_ms;
    size_t last_size_bytes;
    size_t last_blocks_processed;
    int terminal_rows;
    int terminal_cols;
    int colors_enabled;
    int editing_save_path;
    int persist_history;
    char history_labels[TUI_HISTORY_LIMIT][256];
    char history_digests[TUI_HISTORY_LIMIT][MD5_HEX_SIZE];
    char history_sources[TUI_HISTORY_LIMIT][32];
    size_t history_count;
    char navigation_paths[TUI_NAV_DEPTH][512];
    size_t navigation_selected_indices[TUI_NAV_DEPTH];
    size_t navigation_depth;
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
size_t tui_collect_visible_entries(const TUIFileEntry *entries,
                                   size_t entry_count,
                                   const char *filter,
                                   size_t *visible_indices,
                                   size_t max_visible);
void tui_display_file_browser(const TUIContext *context, const TUIFileEntry *entries, size_t entry_count, size_t selected_index);
int tui_prepare_result(TUIContext *context, const char *label, const uint8_t digest[MD5_DIGEST_SIZE], size_t size_bytes, double elapsed_ms);
int tui_compute_text_digest(TUIContext *context);
int tui_compute_file_digest(TUIContext *context);
int tui_compute_stdin_digest(TUIContext *context);
int tui_format_results(const TUIContext *context, char *buffer, size_t buffer_size);
int tui_save_results_to_path(const TUIContext *context, const char *path);
void tui_display_results(const TUIContext *context);

#endif
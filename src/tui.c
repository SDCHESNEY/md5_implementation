#include "tui.h"

#include <ctype.h>
#include <curses.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "file_handler.h"
#include "md5.h"
#include "utils.h"

#define TUI_MAX_FILE_ENTRIES 128U

typedef struct {
    size_t row;
    size_t col;
} TUICursorPoint;

static const char *const menu_labels[MENU_COUNT] = {
    "Hash Text String",
    "Hash File",
    "Hash Binary File",
    "Read from Standard Input",
    "Settings",
    "About",
    "Exit"
};

static void tui_join_path(const char *base, const char *name, char *output, size_t output_size)
{
    if (strcmp(base, ".") == 0) {
        (void)snprintf(output, output_size, "%s", name);
    } else if (strcmp(base, "/") == 0) {
        (void)snprintf(output, output_size, "/%s", name);
    } else {
        (void)snprintf(output, output_size, "%s/%s", base, name);
    }
}

static void tui_parent_path(char *path)
{
    char *slash;

    if (strcmp(path, ".") == 0 || strcmp(path, "/") == 0) {
        return;
    }

    slash = strrchr(path, '/');
    if (slash == NULL) {
        (void)snprintf(path, 512U, ".");
        return;
    }

    if (slash == path) {
        slash[1] = '\0';
        return;
    }

    *slash = '\0';
}

static void tui_draw_border_box(int top, int left, int height, int width)
{
    int row;
    int col;
    int bottom = top + height - 1;
    int right = left + width - 1;

    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, right, ACS_URCORNER);
    mvaddch(bottom, left, ACS_LLCORNER);
    mvaddch(bottom, right, ACS_LRCORNER);

    for (col = left + 1; col < right; ++col) {
        mvaddch(top, col, ACS_HLINE);
        mvaddch(bottom, col, ACS_HLINE);
    }

    for (row = top + 1; row < bottom; ++row) {
        mvaddch(row, left, ACS_VLINE);
        mvaddch(row, right, ACS_VLINE);
    }
}

static size_t tui_calculate_blocks(size_t size_bytes)
{
    return (size_bytes + 9U + 63U) / 64U;
}

static void tui_capture_digest_hex(const uint8_t digest[MD5_DIGEST_SIZE], char output[MD5_HEX_SIZE])
{
    hex_encode(digest, MD5_DIGEST_SIZE, output);
}

static void tui_sanitize_preview(const char *input, char *output, size_t output_size)
{
    size_t read_index;
    size_t write_index = 0U;

    if (output_size == 0U) {
        return;
    }

    if (input == NULL) {
        output[0] = '\0';
        return;
    }

    for (read_index = 0U; input[read_index] != '\0' && write_index + 1U < output_size; ++read_index) {
        unsigned char value = (unsigned char)input[read_index];
        output[write_index++] = iscntrl(value) ? ' ' : (char)value;
    }

    output[write_index] = '\0';
}

static const char *tui_detect_result_source(const TUIContext *context)
{
    if (context == NULL) {
        return "Unknown";
    }

    switch (context->state) {
    case STATE_HASH_TEXT:
        return "Text";
    case STATE_HASH_FILE:
        return context->file_mode == FILE_MODE_BINARY ? "Binary File" : "File";
    case STATE_HASH_STDIN:
        return "Stdin";
    default:
        return "Unknown";
    }
}

static int tui_history_path(char *buffer, size_t buffer_size)
{
    const char *home;

    if (buffer == NULL || buffer_size == 0U) {
        return MD5_ERROR_INVALID_ARG;
    }

    home = getenv("HOME");
    if (home == NULL || home[0] == '\0') {
        return MD5_ERROR_FILE_OPEN;
    }

    (void)snprintf(buffer, buffer_size, "%s/.md5_tui_history", home);
    return MD5_SUCCESS;
}

static void tui_history_add(TUIContext *context)
{
    size_t index;
    char preview[256];

    if (context == NULL || context->digest_hex[0] == '\0') {
        return;
    }

    tui_sanitize_preview(context->input_label, preview, sizeof(preview));
    if (preview[0] == '\0') {
        (void)snprintf(preview, sizeof(preview), "%s", context->result_source);
    }

    if (context->history_count == TUI_HISTORY_LIMIT) {
        for (index = 1U; index < TUI_HISTORY_LIMIT; ++index) {
            (void)snprintf(context->history_labels[index - 1U], sizeof(context->history_labels[index - 1U]), "%s", context->history_labels[index]);
            (void)snprintf(context->history_digests[index - 1U], sizeof(context->history_digests[index - 1U]), "%s", context->history_digests[index]);
            (void)snprintf(context->history_sources[index - 1U], sizeof(context->history_sources[index - 1U]), "%s", context->history_sources[index]);
        }
        context->history_count = TUI_HISTORY_LIMIT - 1U;
    }

    (void)snprintf(context->history_labels[context->history_count], sizeof(context->history_labels[context->history_count]), "%s", preview);
    (void)snprintf(context->history_digests[context->history_count], sizeof(context->history_digests[context->history_count]), "%s", context->digest_hex);
    (void)snprintf(context->history_sources[context->history_count], sizeof(context->history_sources[context->history_count]), "%s", context->result_source);
    ++context->history_count;
}

static void tui_save_history(const TUIContext *context)
{
    char path[1024];
    FILE *stream;
    size_t index;

    if (context == NULL || !context->persist_history || tui_history_path(path, sizeof(path)) != MD5_SUCCESS) {
        return;
    }

    stream = fopen(path, "w");
    if (stream == NULL) {
        return;
    }

    for (index = 0U; index < context->history_count; ++index) {
        (void)fprintf(stream,
                      "%s\t%s\t%s\n",
                      context->history_sources[index],
                      context->history_labels[index],
                      context->history_digests[index]);
    }

    (void)fclose(stream);
}

static void tui_load_history(TUIContext *context)
{
    char path[1024];
    FILE *stream;
    char line[1024];

    if (context == NULL || !context->persist_history || tui_history_path(path, sizeof(path)) != MD5_SUCCESS) {
        return;
    }

    stream = fopen(path, "r");
    if (stream == NULL) {
        return;
    }

    while (fgets(line, sizeof(line), stream) != NULL && context->history_count < TUI_HISTORY_LIMIT) {
        char *source = strtok(line, "\t");
        char *label = strtok(NULL, "\t");
        char *digest = strtok(NULL, "\r\n");

        if (source == NULL || label == NULL || digest == NULL) {
            continue;
        }

        (void)snprintf(context->history_sources[context->history_count], sizeof(context->history_sources[context->history_count]), "%s", source);
        (void)snprintf(context->history_labels[context->history_count], sizeof(context->history_labels[context->history_count]), "%s", label);
        (void)snprintf(context->history_digests[context->history_count], sizeof(context->history_digests[context->history_count]), "%s", digest);
        ++context->history_count;
    }

    (void)fclose(stream);
}

static int tui_entry_matches_filter(const char *name, const char *filter)
{
    size_t name_index;
    size_t filter_length;

    if (name == NULL || filter == NULL || filter[0] == '\0' || strcmp(name, "..") == 0) {
        return 1;
    }

    filter_length = strlen(filter);
    for (name_index = 0U; name[name_index] != '\0'; ++name_index) {
        size_t offset;

        for (offset = 0U; offset < filter_length; ++offset) {
            if (name[name_index + offset] == '\0') {
                break;
            }
            if (tolower((unsigned char)name[name_index + offset]) != tolower((unsigned char)filter[offset])) {
                break;
            }
        }

        if (offset == filter_length) {
            return 1;
        }
    }

    return 0;
}

static size_t tui_text_length(const TUIContext *context)
{
    return context == NULL ? 0U : strlen(context->text_input);
}

static void tui_text_insert(TUIContext *context, char ch)
{
    size_t length;

    if (context == NULL) {
        return;
    }

    length = tui_text_length(context);
    if (length + 1U >= sizeof(context->text_input)) {
        (void)snprintf(context->status, sizeof(context->status), "Text input is full.");
        return;
    }

    (void)memmove(&context->text_input[context->text_cursor_pos + 1U],
                  &context->text_input[context->text_cursor_pos],
                  length - context->text_cursor_pos + 1U);
    context->text_input[context->text_cursor_pos] = ch;
    ++context->text_cursor_pos;
}

static void tui_text_backspace(TUIContext *context)
{
    size_t length;

    if (context == NULL || context->text_cursor_pos == 0U) {
        return;
    }

    length = tui_text_length(context);
    (void)memmove(&context->text_input[context->text_cursor_pos - 1U],
                  &context->text_input[context->text_cursor_pos],
                  length - context->text_cursor_pos + 1U);
    --context->text_cursor_pos;
}

static void tui_text_delete(TUIContext *context)
{
    size_t length;

    if (context == NULL) {
        return;
    }

    length = tui_text_length(context);
    if (context->text_cursor_pos >= length) {
        return;
    }

    (void)memmove(&context->text_input[context->text_cursor_pos],
                  &context->text_input[context->text_cursor_pos + 1U],
                  length - context->text_cursor_pos);
}

static TUICursorPoint tui_text_position_for_index(const char *text, size_t index, size_t width)
{
    TUICursorPoint point = {0U, 0U};
    size_t cursor_index;

    if (text == NULL || width == 0U) {
        return point;
    }

    for (cursor_index = 0U; cursor_index < index && text[cursor_index] != '\0'; ++cursor_index) {
        if (text[cursor_index] == '\n') {
            ++point.row;
            point.col = 0U;
        } else {
            if (point.col >= width) {
                ++point.row;
                point.col = 0U;
            }
            ++point.col;
        }
    }

    return point;
}

static size_t tui_text_index_for_point(const char *text, size_t target_row, size_t target_col, size_t width)
{
    size_t best_index = strlen(text);
    size_t text_length = strlen(text);
    size_t index;
    size_t best_distance = (size_t)-1;

    for (index = 0U; index <= text_length; ++index) {
        TUICursorPoint point = tui_text_position_for_index(text, index, width);
        if (point.row == target_row) {
            size_t distance = point.col > target_col ? point.col - target_col : target_col - point.col;
            if (distance < best_distance) {
                best_distance = distance;
                best_index = index;
                if (distance == 0U) {
                    break;
                }
            }
        }
    }

    return best_index;
}

static void tui_text_move_vertical(TUIContext *context, int direction, size_t width)
{
    TUICursorPoint point;
    size_t target_row;

    if (context == NULL || width == 0U || direction == 0) {
        return;
    }

    point = tui_text_position_for_index(context->text_input, context->text_cursor_pos, width);
    if (direction < 0) {
        if (point.row == 0U) {
            return;
        }
        target_row = point.row - 1U;
    } else {
        target_row = point.row + 1U;
    }

    context->text_cursor_pos = tui_text_index_for_point(context->text_input, target_row, point.col, width);
}

static void tui_push_navigation_state(TUIContext *context)
{
    if (context == NULL || context->navigation_depth >= TUI_NAV_DEPTH) {
        return;
    }

    (void)snprintf(context->navigation_paths[context->navigation_depth],
                   sizeof(context->navigation_paths[context->navigation_depth]),
                   "%s",
                   context->file_path);
    context->navigation_selected_indices[context->navigation_depth] = context->selected_index;
    ++context->navigation_depth;
}

static int tui_pop_navigation_state(TUIContext *context)
{
    if (context == NULL || context->navigation_depth == 0U) {
        return 0;
    }

    --context->navigation_depth;
    (void)snprintf(context->file_path,
                   sizeof(context->file_path),
                   "%s",
                   context->navigation_paths[context->navigation_depth]);
    context->selected_index = context->navigation_selected_indices[context->navigation_depth];
    return 1;
}

static void tui_render_small_terminal_message(const TUIContext *context)
{
    clear();
    mvprintw(1, 2, "Terminal size too small (minimum %dx%d required)", MD5_UI_MIN_COLS, MD5_UI_MIN_ROWS);
    mvprintw(3, 2, "Current size: %dx%d", context->terminal_cols, context->terminal_rows);
    mvprintw(5, 2, "Resize the terminal or press 'q' to exit.");
    refresh();
}

static void tui_render_placeholder_screen(const char *title, const char *message)
{
    clear();
    tui_draw_border_box(0, 0, LINES, COLS);
    mvprintw(1, 2, "%s", title);
    mvprintw(3, 2, "%s", message);
    mvprintw(LINES - 2, 2, "Press 'b' to return.");
    refresh();
}

static void tui_render_status_line(const TUIContext *context)
{
    mvprintw(LINES - 2, 2, "%s", context->status);
}

static int tui_handle_menu_key(TUIContext *context, int ch)
{
    switch (ch) {
    case KEY_UP:
        context->menu_selection = (context->menu_selection == 0) ? (TUIMenuOption)(MENU_COUNT - 1) : (TUIMenuOption)(context->menu_selection - 1);
        return 1;
    case KEY_DOWN:
        context->menu_selection = (TUIMenuOption)((context->menu_selection + 1) % MENU_COUNT);
        return 1;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        context->menu_selection = (TUIMenuOption)(ch - '1');
        return 1;
    case '\n':
    case KEY_ENTER:
    case '\r':
        return 2;
    default:
        return 0;
    }
}

int tui_init(void)
{
    WINDOW *window = initscr();

    if (window == NULL) {
        return MD5_ERROR_TUI_INIT;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
    }

    return MD5_SUCCESS;
}

void tui_cleanup(void)
{
    endwin();
}

void tui_context_init(TUIContext *context)
{
    if (context == NULL) {
        return;
    }

    (void)memset(context, 0, sizeof(*context));
    context->state = STATE_MAIN_MENU;
    context->return_state = STATE_MAIN_MENU;
    context->menu_selection = MENU_TEXT;
    context->file_mode = FILE_MODE_TEXT;
    context->selected_index = 0U;
    context->text_cursor_pos = 0U;
    (void)snprintf(context->file_path, sizeof(context->file_path), ".");
    (void)snprintf(context->result_save_path, sizeof(context->result_save_path), "md5_result.txt");
    (void)snprintf(context->status, sizeof(context->status), "Use arrow keys or numbers to select.");
}

void tui_update_terminal_size(TUIContext *context, int rows, int cols)
{
    if (context == NULL) {
        return;
    }

    context->terminal_rows = rows;
    context->terminal_cols = cols;
}

int tui_terminal_is_usable(const TUIContext *context)
{
    return context != NULL && context->terminal_rows >= MD5_UI_MIN_ROWS && context->terminal_cols >= MD5_UI_MIN_COLS;
}

const char *tui_state_title(UIState state)
{
    switch (state) {
    case STATE_MAIN_MENU:
        return "Main Menu";
    case STATE_HASH_TEXT:
        return "Hash Text String";
    case STATE_HASH_FILE:
        return "Hash File";
    case STATE_HASH_STDIN:
        return "Hash Stdin";
    case STATE_RESULTS:
        return "Hash Result";
    case STATE_SETTINGS:
        return "Settings";
    case STATE_ABOUT:
        return "About";
    default:
        return "MD5 Utility";
    }
}

void tui_display_menu(const TUIContext *context)
{
    int index;
    int history_row = LINES - 6;

    clear();
    tui_draw_border_box(0, 0, LINES, COLS);
    mvprintw(1, 2, "MD5 Hash Utility v%s", MD5_VERSION);
    mvprintw(3, 2, "Welcome to the MD5 Hash Utility");
    mvprintw(5, 2, "Select an option:");

    for (index = 0; index < MENU_COUNT; ++index) {
        if (index == (int)context->menu_selection) {
            attron(A_REVERSE);
        }
        mvprintw(7 + index, 4, "[%d] %s", index + 1, menu_labels[index]);
        if (index == (int)context->menu_selection) {
            attroff(A_REVERSE);
        }
    }

    mvprintw(LINES - 4, 2, "Use arrow keys or numbers to select.");
    mvprintw(LINES - 3, 2, "Press Enter to confirm.");
    if (context->history_count > 0U && history_row > 15) {
        size_t history_index = context->history_count - 1U;
        mvprintw(history_row, 2, "Recent: [%s] %s", context->history_sources[history_index], context->history_labels[history_index]);
    }
    tui_render_status_line(context);
    refresh();
}

void tui_display_text_input(const TUIContext *context)
{
    const int box_top = 5;
    const int box_left = 2;
    const int box_height = 9;
    const int box_width = COLS - 4;
    const size_t inner_width = (size_t)(box_width > 2 ? box_width - 2 : 1);
    const size_t inner_height = (size_t)(box_height > 2 ? box_height - 2 : 1);
    TUICursorPoint cursor_point = tui_text_position_for_index(context->text_input, context->text_cursor_pos, inner_width);
    size_t viewport_row = cursor_point.row >= inner_height ? cursor_point.row - inner_height + 1U : 0U;
    size_t index;

    clear();
    tui_draw_border_box(0, 0, LINES, COLS);
    mvprintw(1, 2, "Hash Text String");
    mvprintw(3, 2, "Edit with arrows, Enter for new lines, Ctrl+D to compute, Ctrl+L to clear, Esc to return.");
    tui_draw_border_box(box_top, box_left, box_height, box_width);

    for (index = 0U; index < strlen(context->text_input); ++index) {
        TUICursorPoint point = tui_text_position_for_index(context->text_input, index, inner_width);
        if (point.row >= viewport_row && point.row < viewport_row + inner_height && context->text_input[index] != '\n') {
            mvaddch(box_top + 1 + (int)(point.row - viewport_row), box_left + 1 + (int)point.col, context->text_input[index]);
        }
    }

    mvprintw(LINES - 4,
             2,
             "Characters: %zu  Bytes: %zu  Cursor: %zu",
             strlen(context->text_input),
             strlen(context->text_input),
             context->text_cursor_pos);
    mvprintw(LINES - 3, 2, "[Ctrl+D] Compute  [Ctrl+L] Clear  [Del] Delete  [Esc] Back");
    tui_render_status_line(context);
    move(box_top + 1 + (int)(cursor_point.row - viewport_row), box_left + 1 + (int)cursor_point.col);
    refresh();
}

int tui_list_directory(const char *path, TUIFileEntry *entries, size_t max_entries, size_t *entry_count)
{
    DIR *directory;
    struct dirent *entry;
    size_t count = 0U;

    if (path == NULL || entries == NULL || entry_count == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    directory = opendir(path);
    if (directory == NULL) {
        return MD5_ERROR_FILE_OPEN;
    }

    while ((entry = readdir(directory)) != NULL && count < max_entries) {
        struct stat info;
        char full_path[1024];

        if (strcmp(entry->d_name, ".") == 0) {
            continue;
        }

        (void)snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &info) != 0) {
            continue;
        }

        (void)snprintf(entries[count].name, sizeof(entries[count].name), "%s", entry->d_name);
        entries[count].is_directory = S_ISDIR(info.st_mode) ? 1 : 0;
        entries[count].size_bytes = S_ISREG(info.st_mode) ? (size_t)info.st_size : 0U;
        entries[count].modified_time = info.st_mtime;
        ++count;
    }

    (void)closedir(directory);
    *entry_count = count;
    return MD5_SUCCESS;
}

size_t tui_collect_visible_entries(const TUIFileEntry *entries,
                                   size_t entry_count,
                                   const char *filter,
                                   size_t *visible_indices,
                                   size_t max_visible)
{
    size_t entry_index;
    size_t visible_count = 0U;

    if (entries == NULL || visible_indices == NULL) {
        return 0U;
    }

    for (entry_index = 0U; entry_index < entry_count && visible_count < max_visible; ++entry_index) {
        if (tui_entry_matches_filter(entries[entry_index].name, filter)) {
            visible_indices[visible_count++] = entry_index;
        }
    }

    return visible_count;
}

void tui_display_file_browser(const TUIContext *context, const TUIFileEntry *entries, size_t entry_count, size_t selected_index)
{
    size_t index;
    size_t visible_indices[TUI_MAX_FILE_ENTRIES];
    size_t visible_count = tui_collect_visible_entries(entries, entry_count, context->browser_filter, visible_indices, TUI_MAX_FILE_ENTRIES);

    clear();
    tui_draw_border_box(0, 0, LINES, COLS);
    mvprintw(1, 2, "%s", context->file_mode == FILE_MODE_BINARY ? "Select Binary File to Hash" : "Select File to Hash");
    mvprintw(3, 2, "Current: %s", context->file_path);
    mvprintw(4, 2, "Filter: %s", context->browser_filter[0] == '\0' ? "<none>" : context->browser_filter);

    for (index = 0U; index < visible_count && index < (size_t)(LINES - 11); ++index) {
        const TUIFileEntry *entry = &entries[visible_indices[index]];
        char size_buffer[32];
        char time_buffer[32];
        struct tm *time_info = localtime(&entry->modified_time);

        if (time_info != NULL) {
            (void)strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M", time_info);
        } else {
            (void)snprintf(time_buffer, sizeof(time_buffer), "unknown");
        }

        if (index == selected_index) {
            attron(A_REVERSE);
        }
        mvprintw(6 + (int)index,
                 4,
                 "[%c] %-28s %-6s %10s %16s",
                 entry->is_directory ? 'D' : 'F',
                 entry->name,
                 entry->is_directory ? "dir" : "file",
                 entry->is_directory ? "<dir>" : format_file_size(entry->size_bytes, size_buffer, sizeof(size_buffer)),
                 time_buffer);
        if (index == selected_index) {
            attroff(A_REVERSE);
        }
    }

    if (visible_count == 0U) {
        mvprintw(6, 4, "No entries match the current filter.");
    }

    mvprintw(LINES - 4, 2, "Use arrows and Enter to select. Type to filter, Backspace to edit.");
    mvprintw(LINES - 3, 2, "[Left] Parent  [R]efresh  [B]ack");
    tui_render_status_line(context);
    refresh();
}

int tui_prepare_result(TUIContext *context, const char *label, const uint8_t digest[MD5_DIGEST_SIZE], size_t size_bytes, double elapsed_ms)
{
    if (context == NULL || label == NULL || digest == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    context->return_state = context->state;
    (void)snprintf(context->input_label, sizeof(context->input_label), "%s", label);
    (void)snprintf(context->result_source, sizeof(context->result_source), "%s", tui_detect_result_source(context));
    tui_capture_digest_hex(digest, context->digest_hex);
    context->last_size_bytes = size_bytes;
    context->last_elapsed_ms = elapsed_ms;
    context->last_blocks_processed = tui_calculate_blocks(size_bytes);
    context->editing_save_path = 0;
    context->state = STATE_RESULTS;
    tui_history_add(context);
    tui_save_history(context);
    (void)snprintf(context->status, sizeof(context->status), "Digest ready.");
    return MD5_SUCCESS;
}

int tui_compute_text_digest(TUIContext *context)
{
    uint8_t digest[MD5_DIGEST_SIZE];
    double start_ms;

    if (context == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    if (context->text_input[0] == '\0') {
        (void)snprintf(context->text_input, sizeof(context->text_input), "Sample text");
        context->text_cursor_pos = strlen(context->text_input);
    }

    start_ms = get_time_ms();
    md5_string(context->text_input, digest);
    return tui_prepare_result(context, context->text_input, digest, strlen(context->text_input), get_time_ms() - start_ms);
}

int tui_compute_file_digest(TUIContext *context)
{
    uint8_t digest[MD5_DIGEST_SIZE];
    double start_ms;
    int status;

    if (context == NULL || context->file_path[0] == '\0') {
        return MD5_ERROR_INVALID_ARG;
    }

    start_ms = get_time_ms();
    status = hash_file(context->file_path, context->file_mode, digest);
    if (status != MD5_SUCCESS) {
        return status;
    }

    return tui_prepare_result(context, context->file_path, digest, file_size(context->file_path), get_time_ms() - start_ms);
}

int tui_compute_stdin_digest(TUIContext *context)
{
    uint8_t digest[MD5_DIGEST_SIZE];
    double start_ms;
    int status;

    if (context == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    start_ms = get_time_ms();
    status = read_stdin(digest);
    if (status != MD5_SUCCESS) {
        return status;
    }

    return tui_prepare_result(context, "stdin", digest, 0U, get_time_ms() - start_ms);
}

int tui_format_results(const TUIContext *context, char *buffer, size_t buffer_size)
{
    int written;

    if (context == NULL || buffer == NULL || buffer_size == 0U) {
        return MD5_ERROR_INVALID_ARG;
    }

    written = snprintf(buffer,
                       buffer_size,
                       "Source: %s\nInput: %s\nSize: %zu bytes\nBlocks: %zu\nTime: %.3f ms\n\nMD5 Digest:\n%s\n",
                       context->result_source,
                       context->input_label,
                       context->last_size_bytes,
                       context->last_blocks_processed,
                       context->last_elapsed_ms,
                       context->digest_hex);

    return (written < 0 || (size_t)written >= buffer_size) ? MD5_ERROR_INVALID_ARG : MD5_SUCCESS;
}

int tui_save_results_to_path(const TUIContext *context, const char *path)
{
    char buffer[512];
    FILE *stream;

    if (context == NULL || path == NULL || path[0] == '\0') {
        return MD5_ERROR_INVALID_ARG;
    }

    if (tui_format_results(context, buffer, sizeof(buffer)) != MD5_SUCCESS) {
        return MD5_ERROR_INVALID_ARG;
    }

    stream = fopen(path, "w");
    if (stream == NULL) {
        return MD5_ERROR_FILE_OPEN;
    }

    if (fputs(buffer, stream) == EOF) {
        (void)fclose(stream);
        return MD5_ERROR_FILE_READ;
    }

    (void)fclose(stream);
    return MD5_SUCCESS;
}

void tui_display_results(const TUIContext *context)
{
    char buffer[512];

    clear();
    tui_draw_border_box(0, 0, LINES, COLS);
    mvprintw(1, 2, "Hash Result");
    if (tui_format_results(context, buffer, sizeof(buffer)) == MD5_SUCCESS) {
        mvprintw(3, 2, "%s", buffer);
        if (context->editing_save_path) {
            mvprintw(LINES - 4, 2, "Save path: %s", context->result_save_path);
            mvprintw(LINES - 3, 2, "Type a path, Enter to save, Esc to cancel.");
        } else {
            mvprintw(LINES - 4, 2, "[C]opy Digest  Cop[Y] Summary  [S]ave  [B]ack");
            mvprintw(LINES - 3, 2, "Return target: %s", tui_state_title(context->return_state));
        }
        tui_render_status_line(context);
    } else {
        mvprintw(1, 2, "Unable to render results.");
    }
    refresh();
}

void tui_handle_input(TUIContext *context, int ch)
{
    int action;

    if (context == NULL) {
        return;
    }

    switch (context->state) {
    case STATE_MAIN_MENU:
        action = tui_handle_menu_key(context, ch);
        if (action == 2) {
            switch (context->menu_selection) {
            case MENU_TEXT:
                context->state = STATE_HASH_TEXT;
                break;
            case MENU_FILE:
                context->file_mode = FILE_MODE_TEXT;
                context->selected_index = 0U;
                context->state = STATE_HASH_FILE;
                break;
            case MENU_BINARY:
                context->file_mode = FILE_MODE_BINARY;
                context->selected_index = 0U;
                context->state = STATE_HASH_FILE;
                break;
            case MENU_STDIN:
                context->state = STATE_HASH_STDIN;
                break;
            case MENU_SETTINGS:
                context->state = STATE_SETTINGS;
                break;
            case MENU_ABOUT:
                context->state = STATE_ABOUT;
                break;
            case MENU_EXIT:
                context->state = STATE_RESULTS;
                (void)snprintf(context->status, sizeof(context->status), "Exit selected.");
                break;
            default:
                break;
            }
        }
        break;
    case STATE_HASH_TEXT:
        if (ch == 27) {
            context->state = STATE_MAIN_MENU;
        } else if (ch == 12) {
            context->text_input[0] = '\0';
            context->text_cursor_pos = 0U;
            (void)snprintf(context->status, sizeof(context->status), "Text input cleared.");
        } else if (ch == 4 || ch == 'c' || ch == 'C') {
            if (tui_compute_text_digest(context) != MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Unable to compute text digest.");
            }
        } else if (ch == KEY_LEFT) {
            if (context->text_cursor_pos > 0U) {
                --context->text_cursor_pos;
            }
        } else if (ch == KEY_RIGHT) {
            if (context->text_cursor_pos < tui_text_length(context)) {
                ++context->text_cursor_pos;
            }
        } else if (ch == KEY_UP) {
            tui_text_move_vertical(context, -1, (size_t)(COLS > 6 ? COLS - 6 : 1));
        } else if (ch == KEY_DOWN) {
            tui_text_move_vertical(context, 1, (size_t)(COLS > 6 ? COLS - 6 : 1));
        } else if (ch == KEY_DC) {
            tui_text_delete(context);
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            tui_text_backspace(context);
        } else if (ch == '\n' || ch == KEY_ENTER || ch == '\r') {
            tui_text_insert(context, '\n');
        } else if (ch >= 32 && ch <= 126) {
            tui_text_insert(context, (char)ch);
        }
        break;
    case STATE_HASH_FILE:
        if (ch == 'b' || ch == 'B') {
            context->state = STATE_MAIN_MENU;
        } else if (ch == 'r' || ch == 'R') {
            (void)snprintf(context->status, sizeof(context->status), "Directory refreshed.");
        } else if (ch == KEY_LEFT) {
            if (!tui_pop_navigation_state(context)) {
                tui_parent_path(context->file_path);
                context->selected_index = 0U;
            }
            (void)snprintf(context->status, sizeof(context->status), "Moved to parent directory.");
        } else if (ch == KEY_UP && context->selected_index > 0U) {
            --context->selected_index;
        } else if (ch == KEY_DOWN) {
            TUIFileEntry entries[TUI_MAX_FILE_ENTRIES];
            size_t entry_count = 0U;
            size_t visible_indices[TUI_MAX_FILE_ENTRIES];
            size_t visible_count;

            if (tui_list_directory(context->file_path, entries, TUI_MAX_FILE_ENTRIES, &entry_count) != MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Unable to read directory.");
                break;
            }

            visible_count = tui_collect_visible_entries(entries, entry_count, context->browser_filter, visible_indices, TUI_MAX_FILE_ENTRIES);
            if (context->selected_index + 1U < visible_count) {
                ++context->selected_index;
            }
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            size_t filter_length = strlen(context->browser_filter);
            if (filter_length > 0U) {
                context->browser_filter[filter_length - 1U] = '\0';
                context->selected_index = 0U;
            }
        } else if (ch >= 32 && ch <= 126) {
            size_t filter_length = strlen(context->browser_filter);
            if (filter_length + 1U < sizeof(context->browser_filter)) {
                context->browser_filter[filter_length] = (char)ch;
                context->browser_filter[filter_length + 1U] = '\0';
                context->selected_index = 0U;
            }
        } else if (ch == '\n' || ch == KEY_ENTER || ch == '\r') {
            TUIFileEntry entries[TUI_MAX_FILE_ENTRIES];
            size_t entry_count = 0U;
            size_t visible_indices[TUI_MAX_FILE_ENTRIES];
            size_t visible_count;

            if (tui_list_directory(context->file_path, entries, TUI_MAX_FILE_ENTRIES, &entry_count) != MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Unable to read directory.");
                break;
            }

            visible_count = tui_collect_visible_entries(entries, entry_count, context->browser_filter, visible_indices, TUI_MAX_FILE_ENTRIES);
            if (visible_count == 0U) {
                (void)snprintf(context->status, sizeof(context->status), "No entries match the current filter.");
                break;
            }

            if (context->selected_index >= visible_count) {
                context->selected_index = visible_count - 1U;
            }

            if (entries[visible_indices[context->selected_index]].is_directory) {
                if (strcmp(entries[visible_indices[context->selected_index]].name, "..") == 0) {
                    if (!tui_pop_navigation_state(context)) {
                        tui_parent_path(context->file_path);
                        context->selected_index = 0U;
                    }
                } else {
                    char next_path[sizeof(context->file_path)];
                    tui_push_navigation_state(context);
                    tui_join_path(context->file_path, entries[visible_indices[context->selected_index]].name, next_path, sizeof(next_path));
                    (void)snprintf(context->file_path, sizeof(context->file_path), "%s", next_path);
                    context->selected_index = 0U;
                }
            } else {
                char current_directory[sizeof(context->file_path)];
                char selected_path[sizeof(context->file_path)];

                (void)snprintf(current_directory, sizeof(current_directory), "%s", context->file_path);
                tui_join_path(context->file_path, entries[visible_indices[context->selected_index]].name, selected_path, sizeof(selected_path));
                (void)snprintf(context->file_path, sizeof(context->file_path), "%s", selected_path);
                if (tui_compute_file_digest(context) != MD5_SUCCESS) {
                    (void)snprintf(context->status, sizeof(context->status), "Unable to hash selected file.");
                }
                (void)snprintf(context->file_path, sizeof(context->file_path), "%s", current_directory);
            }
        }
        break;
    case STATE_HASH_STDIN:
        if (ch == 'b' || ch == 'B') {
            context->state = STATE_MAIN_MENU;
        } else if (ch == 'c' || ch == 'C' || ch == '\n' || ch == KEY_ENTER || ch == '\r') {
            if (tui_compute_stdin_digest(context) != MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Unable to read stdin.");
            }
        }
        break;
    case STATE_RESULTS:
        if (context->editing_save_path) {
            if (ch == 27) {
                context->editing_save_path = 0;
                (void)snprintf(context->status, sizeof(context->status), "Save cancelled.");
            } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                size_t length = strlen(context->result_save_path);
                if (length > 0U) {
                    context->result_save_path[length - 1U] = '\0';
                }
            } else if (ch == '\n' || ch == KEY_ENTER || ch == '\r') {
                if (tui_save_results_to_path(context, context->result_save_path) == MD5_SUCCESS) {
                    context->editing_save_path = 0;
                    (void)snprintf(context->status, sizeof(context->status), "Results saved to %s.", context->result_save_path);
                } else {
                    (void)snprintf(context->status, sizeof(context->status), "Unable to save results.");
                }
            } else if (ch >= 32 && ch <= 126) {
                size_t length = strlen(context->result_save_path);
                if (length + 1U < sizeof(context->result_save_path)) {
                    context->result_save_path[length] = (char)ch;
                    context->result_save_path[length + 1U] = '\0';
                }
            }
        } else if (ch == 'b' || ch == 'B') {
            context->state = context->return_state;
        } else if (ch == 'c' || ch == 'C') {
            if (clipboard_copy(context->digest_hex) == MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Digest copied to clipboard.");
            } else {
                (void)snprintf(context->status, sizeof(context->status), "Clipboard unavailable.");
            }
        } else if (ch == 'y' || ch == 'Y') {
            char buffer[512];
            if (tui_format_results(context, buffer, sizeof(buffer)) == MD5_SUCCESS && clipboard_copy(buffer) == MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Result summary copied to clipboard.");
            } else {
                (void)snprintf(context->status, sizeof(context->status), "Clipboard unavailable.");
            }
        } else if (ch == 's' || ch == 'S') {
            context->editing_save_path = 1;
            (void)snprintf(context->status, sizeof(context->status), "Enter a path for the result summary.");
        }
        break;
    case STATE_SETTINGS:
        if (ch == 'b' || ch == 'B') {
            context->state = STATE_MAIN_MENU;
        } else if (ch == 'c' || ch == 'C') {
            context->colors_enabled = !context->colors_enabled;
            (void)snprintf(context->status, sizeof(context->status), "Color output %s.", context->colors_enabled ? "enabled" : "disabled");
        } else if (ch == 'h' || ch == 'H') {
            context->persist_history = !context->persist_history;
            (void)snprintf(context->status, sizeof(context->status), "History persistence %s.", context->persist_history ? "enabled" : "disabled");
        }
        break;
    case STATE_ABOUT:
        if (ch == 'b' || ch == 'B') {
            context->state = STATE_MAIN_MENU;
        }
        break;
    default:
        break;
    }
}

int tui_run(void)
{
    TUIContext context;
    TUIFileEntry entries[TUI_MAX_FILE_ENTRIES];
    size_t entry_count = 0U;
    int key;

    if (tui_init() != MD5_SUCCESS) {
        return MD5_ERROR_TUI_INIT;
    }

    tui_context_init(&context);
    context.colors_enabled = has_colors() ? 1 : 0;
    context.persist_history = 1;
    tui_load_history(&context);

    do {
        int rows;
        int cols;

        getmaxyx(stdscr, rows, cols);
        tui_update_terminal_size(&context, rows, cols);

        if (!tui_terminal_is_usable(&context)) {
            tui_render_small_terminal_message(&context);
        } else {
            switch (context.state) {
            case STATE_MAIN_MENU:
                tui_display_menu(&context);
                break;
            case STATE_HASH_TEXT:
                tui_display_text_input(&context);
                break;
            case STATE_HASH_FILE:
                if (tui_list_directory(context.file_path, entries, TUI_MAX_FILE_ENTRIES, &entry_count) == MD5_SUCCESS) {
                    size_t visible_indices[TUI_MAX_FILE_ENTRIES];
                    size_t visible_count = tui_collect_visible_entries(entries, entry_count, context.browser_filter, visible_indices, TUI_MAX_FILE_ENTRIES);

                    if (visible_count > 0U && context.selected_index >= visible_count) {
                        context.selected_index = visible_count - 1U;
                    }
                    tui_display_file_browser(&context, entries, entry_count, context.selected_index);
                } else {
                    tui_render_placeholder_screen("Hash File", "Unable to read directory. Press 'b' to return.");
                }
                break;
            case STATE_HASH_STDIN:
                tui_render_placeholder_screen("Read from Standard Input", "Press 'c' to read stdin or 'b' to return.");
                break;
            case STATE_RESULTS:
                tui_display_results(&context);
                break;
            case STATE_SETTINGS:
                tui_render_placeholder_screen("Settings", "Press 'c' to toggle color output, 'h' for history, or 'b' to return.");
                break;
            case STATE_ABOUT:
                tui_render_placeholder_screen("About", "MD5 Hash Utility using ncurses with text editing, file filtering, saved results, and session history.");
                break;
            default:
                break;
            }
        }

        key = getch();

        if (!tui_terminal_is_usable(&context) && (key == 'q' || key == 'Q')) {
            break;
        }

        if (key == KEY_RESIZE) {
            clearok(stdscr, TRUE);
            (void)snprintf(context.status, sizeof(context.status), "Terminal resized. Layout refreshed.");
            continue;
        }

        tui_handle_input(&context, key);
    } while (!(context.state == STATE_RESULTS && strcmp(context.status, "Exit selected.") == 0));

    tui_cleanup();
    return MD5_SUCCESS;
}
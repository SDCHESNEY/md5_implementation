#include "tui.h"

#include <curses.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_handler.h"
#include "md5.h"
#include "utils.h"

#define TUI_MAX_FILE_ENTRIES 128U

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
    context->menu_selection = MENU_TEXT;
    context->file_mode = FILE_MODE_TEXT;
    context->selected_index = 0U;
    (void)snprintf(context->file_path, sizeof(context->file_path), ".");
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
    mvprintw(LINES - 2, 2, "%s", context->status);
    refresh();
}

void tui_display_text_input(const TUIContext *context)
{
    clear();
    tui_draw_border_box(0, 0, LINES, COLS);
    mvprintw(1, 2, "Hash Text String");
    mvprintw(3, 2, "Type text, Backspace to edit, Ctrl+D to compute, Ctrl+L to clear, Esc to return.");
    tui_draw_border_box(5, 2, 7, COLS - 4);
    mvprintw(6, 4, "%s", context->text_input);
    mvprintw(LINES - 3, 2, "[Ctrl+D] Compute  [Ctrl+L] Clear  [Esc] Back");
    mvprintw(LINES - 2, 2, "%s", context->status);
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
        ++count;
    }

    (void)closedir(directory);
    *entry_count = count;
    return MD5_SUCCESS;
}

void tui_display_file_browser(const TUIContext *context, const TUIFileEntry *entries, size_t entry_count, size_t selected_index)
{
    size_t index;
    char size_buffer[32];

    clear();
    tui_draw_border_box(0, 0, LINES, COLS);
    mvprintw(1, 2, "%s", context->file_mode == FILE_MODE_BINARY ? "Select Binary File to Hash" : "Select File to Hash");
    mvprintw(3, 2, "Current: %s", context->file_path);

    for (index = 0; index < entry_count && index < (size_t)(LINES - 9); ++index) {
        const TUIFileEntry *entry = &entries[index];
        if (index == selected_index) {
            attron(A_REVERSE);
        }
        mvprintw(5 + (int)index,
                 4,
                 "[%c] %-32s %12s",
                 entry->is_directory ? 'D' : 'F',
                 entry->name,
                 entry->is_directory ? "<dir>" : format_file_size(entry->size_bytes, size_buffer, sizeof(size_buffer)));
        if (index == selected_index) {
            attroff(A_REVERSE);
        }
    }

    mvprintw(LINES - 3, 2, "Use arrows and Enter to select. [R]efresh  [B]ack");
    mvprintw(LINES - 2, 2, "%s", context->status);
    refresh();
}

int tui_prepare_result(TUIContext *context, const char *label, const uint8_t digest[MD5_DIGEST_SIZE], size_t size_bytes, double elapsed_ms)
{
    if (context == NULL || label == NULL || digest == NULL) {
        return MD5_ERROR_INVALID_ARG;
    }

    (void)snprintf(context->input_label, sizeof(context->input_label), "%s", label);
    tui_capture_digest_hex(digest, context->digest_hex);
    context->last_size_bytes = size_bytes;
    context->last_elapsed_ms = elapsed_ms;
    context->last_blocks_processed = tui_calculate_blocks(size_bytes);
    context->state = STATE_RESULTS;
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
                       "File: %s\nSize: %zu bytes\nTime: %.3f ms\n\nMD5 Digest:\n%s\n",
                       context->input_label,
                       context->last_size_bytes,
                       context->last_elapsed_ms,
                       context->digest_hex);

    return (written < 0 || (size_t)written >= buffer_size) ? MD5_ERROR_INVALID_ARG : MD5_SUCCESS;
}

void tui_display_results(const TUIContext *context)
{
    char buffer[512];

    clear();
    tui_draw_border_box(0, 0, LINES, COLS);
    mvprintw(1, 2, "Hash Result");
    if (tui_format_results(context, buffer, sizeof(buffer)) == MD5_SUCCESS) {
        mvprintw(3, 2, "%s", buffer);
        mvprintw(LINES - 4, 2, "Blocks: %zu", context->last_blocks_processed);
        mvprintw(LINES - 3, 2, "[C]opy to Clipboard  [B]ack");
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
            (void)snprintf(context->status, sizeof(context->status), "Text input cleared.");
        } else if (ch == 4 || ch == 'c' || ch == 'C') {
            if (tui_compute_text_digest(context) != MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Unable to compute text digest.");
            }
        } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == '\b') && context->text_input[0] != '\0') {
            size_t length = strlen(context->text_input);
            context->text_input[length - 1U] = '\0';
        } else if (ch >= 32 && ch <= 126) {
            size_t length = strlen(context->text_input);
            if (length + 1U < sizeof(context->text_input)) {
                context->text_input[length] = (char)ch;
                context->text_input[length + 1U] = '\0';
            }
        }
        break;
    case STATE_HASH_FILE:
        if (ch == 'b' || ch == 'B') {
            context->state = STATE_MAIN_MENU;
        } else if (ch == 'r' || ch == 'R') {
            (void)snprintf(context->status, sizeof(context->status), "Directory refreshed.");
        } else if (ch == KEY_UP && context->selected_index > 0U) {
            --context->selected_index;
        } else if (ch == KEY_DOWN) {
            ++context->selected_index;
        } else if (ch == '\n' || ch == KEY_ENTER || ch == '\r') {
            TUIFileEntry entries[TUI_MAX_FILE_ENTRIES];
            size_t entry_count = 0U;

            if (tui_list_directory(context->file_path, entries, TUI_MAX_FILE_ENTRIES, &entry_count) != MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Unable to read directory.");
                break;
            }

            if (entry_count == 0U) {
                break;
            }

            if (context->selected_index >= entry_count) {
                context->selected_index = entry_count - 1U;
            }

            if (entries[context->selected_index].is_directory) {
                if (strcmp(entries[context->selected_index].name, "..") == 0) {
                    tui_parent_path(context->file_path);
                } else {
                    char next_path[sizeof(context->file_path)];
                    tui_join_path(context->file_path, entries[context->selected_index].name, next_path, sizeof(next_path));
                    (void)snprintf(context->file_path, sizeof(context->file_path), "%s", next_path);
                }
                context->selected_index = 0U;
            } else {
                char selected_path[sizeof(context->file_path)];
                tui_join_path(context->file_path, entries[context->selected_index].name, selected_path, sizeof(selected_path));
                (void)snprintf(context->file_path, sizeof(context->file_path), "%s", selected_path);
                if (tui_compute_file_digest(context) != MD5_SUCCESS) {
                    (void)snprintf(context->status, sizeof(context->status), "Unable to hash selected file.");
                }
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
        if (ch == 'b' || ch == 'B') {
            context->state = STATE_MAIN_MENU;
        } else if (ch == 'c' || ch == 'C') {
            if (clipboard_copy(context->digest_hex) == MD5_SUCCESS) {
                (void)snprintf(context->status, sizeof(context->status), "Digest copied to clipboard.");
            } else {
                (void)snprintf(context->status, sizeof(context->status), "Clipboard unavailable.");
            }
        }
        break;
    case STATE_SETTINGS:
        if (ch == 'b' || ch == 'B') {
            context->state = STATE_MAIN_MENU;
        } else if (ch == 'c' || ch == 'C') {
            context->colors_enabled = !context->colors_enabled;
            (void)snprintf(context->status, sizeof(context->status), "Color output %s.", context->colors_enabled ? "enabled" : "disabled");
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
                    if (entry_count > 0U && context.selected_index >= entry_count) {
                        context.selected_index = entry_count - 1U;
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
                tui_render_placeholder_screen("Settings", "Press 'c' to toggle color output or 'b' to return.");
                break;
            case STATE_ABOUT:
                tui_render_placeholder_screen("About", "MD5 Hash Utility using ncurses. Supports text, file, binary, and stdin hashing.");
                break;
            default:
                break;
            }
        }

        key = getch();

        if (!tui_terminal_is_usable(&context) && (key == 'q' || key == 'Q')) {
            break;
        }

        tui_handle_input(&context, key);
    } while (!(context.state == STATE_RESULTS && strcmp(context.status, "Exit selected.") == 0));

    tui_cleanup();
    return MD5_SUCCESS;
}
#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../src/tui.h"

static int test_context_defaults(void)
{
    TUIContext context;

    tui_context_init(&context);
    if (context.state != STATE_MAIN_MENU) {
        return 1;
    }

    return context.menu_selection != MENU_TEXT;
}

static int test_state_transitions(void)
{
    TUIContext context;

    tui_context_init(&context);
    tui_handle_input(&context, '1');
    tui_handle_input(&context, '\n');
    if (context.state != STATE_HASH_TEXT) {
        return 1;
    }

    tui_handle_input(&context, 27);
    if (context.state != STATE_MAIN_MENU) {
        return 1;
    }

    return 0;
}

static int test_terminal_constraints(void)
{
    TUIContext context;

    tui_context_init(&context);
    tui_update_terminal_size(&context, 10, 20);
    if (tui_terminal_is_usable(&context)) {
        return 1;
    }

    tui_update_terminal_size(&context, MD5_UI_MIN_ROWS, MD5_UI_MIN_COLS);
    return !tui_terminal_is_usable(&context);
}

static int test_text_digest_path(void)
{
    TUIContext context;

    tui_context_init(&context);
    (void)snprintf(context.text_input, sizeof(context.text_input), "hello");
    if (tui_compute_text_digest(&context) != MD5_SUCCESS) {
        return 1;
    }

    if (context.state != STATE_RESULTS) {
        return 1;
    }

    return strcmp(context.digest_hex, "5d41402abc4b2a76b9719d911017c592") != 0;
}

static int test_directory_listing(void)
{
    TUIFileEntry entries[32];
    size_t entry_count = 0U;

    if (tui_list_directory(".", entries, 32U, &entry_count) != MD5_SUCCESS) {
        return 1;
    }

    return entry_count == 0U;
}

static int test_result_formatting(void)
{
    TUIContext context;
    char buffer[256];

    tui_context_init(&context);
    (void)snprintf(context.input_label, sizeof(context.input_label), "hello.txt");
    (void)snprintf(context.result_source, sizeof(context.result_source), "File");
    (void)snprintf(context.digest_hex, sizeof(context.digest_hex), "5d41402abc4b2a76b9719d911017c592");
    context.last_size_bytes = 5U;
    context.last_elapsed_ms = 1.234;
    context.last_blocks_processed = 1U;

    if (tui_format_results(&context, buffer, sizeof(buffer)) != MD5_SUCCESS) {
        return 1;
    }

    return strstr(buffer, "5d41402abc4b2a76b9719d911017c592") == NULL || strstr(buffer, "Blocks: 1") == NULL;
}

static int test_multiline_text_editing(void)
{
    TUIContext context;

    tui_context_init(&context);
    context.state = STATE_HASH_TEXT;

    tui_handle_input(&context, 'a');
    tui_handle_input(&context, '\n');
    tui_handle_input(&context, 'b');
    if (strcmp(context.text_input, "a\nb") != 0 || context.text_cursor_pos != 3U) {
        return 1;
    }

    tui_handle_input(&context, KEY_LEFT);
    tui_handle_input(&context, KEY_BACKSPACE);
    if (strcmp(context.text_input, "ab") != 0 || context.text_cursor_pos != 1U) {
        return 1;
    }

    return 0;
}

static int test_file_filtering(void)
{
    TUIFileEntry entries[4] = {
        {"..", 1, 0U, 0},
        {"alpha.txt", 0, 10U, 0},
        {"beta.bin", 0, 20U, 0},
        {"notes", 1, 0U, 0}
    };
    size_t visible_indices[4];
    size_t visible_count = tui_collect_visible_entries(entries, 4U, "be", visible_indices, 4U);

    return visible_count != 2U || visible_indices[0] != 0U || visible_indices[1] != 2U;
}

static int test_result_save_and_history(void)
{
    TUIContext context;
    char path[128];
    char buffer[512];
    FILE *stream;
    size_t bytes_read;

    tui_context_init(&context);
    context.state = STATE_HASH_TEXT;
    (void)snprintf(context.text_input, sizeof(context.text_input), "hello");
    context.text_cursor_pos = strlen(context.text_input);

    if (tui_compute_text_digest(&context) != MD5_SUCCESS) {
        return 1;
    }

    if (context.history_count != 1U || context.return_state != STATE_HASH_TEXT) {
        return 1;
    }

    (void)snprintf(path, sizeof(path), "/tmp/md5_tui_result_%ld.txt", (long)getpid());
    if (tui_save_results_to_path(&context, path) != MD5_SUCCESS) {
        return 1;
    }

    stream = fopen(path, "r");
    if (stream == NULL) {
        return 1;
    }

    bytes_read = fread(buffer, 1U, sizeof(buffer) - 1U, stream);
    buffer[bytes_read] = '\0';
    (void)fclose(stream);
    unlink(path);

    return bytes_read == 0U || strstr(buffer, "Source: Text") == NULL || strstr(buffer, "5d41402abc4b2a76b9719d911017c592") == NULL;
}

int main(void)
{
    if (test_context_defaults() != 0) {
        fprintf(stderr, "context defaults test failed\n");
        return 1;
    }

    if (test_state_transitions() != 0) {
        fprintf(stderr, "state transition test failed\n");
        return 1;
    }

    if (test_terminal_constraints() != 0) {
        fprintf(stderr, "terminal constraints test failed\n");
        return 1;
    }

    if (test_text_digest_path() != 0) {
        fprintf(stderr, "text digest path test failed\n");
        return 1;
    }

    if (test_directory_listing() != 0) {
        fprintf(stderr, "directory listing test failed\n");
        return 1;
    }

    if (test_result_formatting() != 0) {
        fprintf(stderr, "result formatting test failed\n");
        return 1;
    }

    if (test_multiline_text_editing() != 0) {
        fprintf(stderr, "multiline text editing test failed\n");
        return 1;
    }

    if (test_file_filtering() != 0) {
        fprintf(stderr, "file filtering test failed\n");
        return 1;
    }

    if (test_result_save_and_history() != 0) {
        fprintf(stderr, "result save/history test failed\n");
        return 1;
    }

    puts("test_tui passed");
    return 0;
}
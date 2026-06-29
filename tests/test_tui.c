#include <stdio.h>
#include <string.h>

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
    (void)snprintf(context.digest_hex, sizeof(context.digest_hex), "5d41402abc4b2a76b9719d911017c592");
    context.last_size_bytes = 5U;
    context.last_elapsed_ms = 1.234;

    if (tui_format_results(&context, buffer, sizeof(buffer)) != MD5_SUCCESS) {
        return 1;
    }

    return strstr(buffer, "5d41402abc4b2a76b9719d911017c592") == NULL || strstr(buffer, "Size: 5 bytes") == NULL;
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

    puts("test_tui passed");
    return 0;
}
#include "tui.h"

#include <curses.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

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

void tui_display_menu(void)
{
    clear();
    mvprintw(1, 2, "MD5 Hash Utility v%s", MD5_VERSION);
    mvprintw(3, 2, "[1] Hash Text String");
    mvprintw(4, 2, "[2] Hash File");
    mvprintw(5, 2, "[3] Hash Binary File");
    mvprintw(6, 2, "[4] Read from Standard Input");
    mvprintw(7, 2, "[5] Settings");
    mvprintw(8, 2, "[6] About");
    mvprintw(9, 2, "[7] Exit");
    mvprintw(11, 2, "Use number keys to select an option.");
    refresh();
}

void tui_display_text_input(void)
{
    clear();
    mvprintw(1, 2, "Hash Text String");
    mvprintw(3, 2, "Text input UI scaffolded. Use CLI mode until implemented.");
    refresh();
}

void tui_display_file_browser(void)
{
    clear();
    mvprintw(1, 2, "Hash File");
    mvprintw(3, 2, "File browser scaffolded. Use CLI mode until implemented.");
    refresh();
}

int tui_format_results(const char *input, const uint8_t digest[MD5_DIGEST_SIZE], char *buffer, size_t buffer_size)
{
    char hex[MD5_HEX_SIZE];
    int written;

    if (input == NULL || digest == NULL || buffer == NULL || buffer_size == 0U) {
        return MD5_ERROR_INVALID_ARG;
    }

    hex_encode(digest, MD5_DIGEST_SIZE, hex);
    written = snprintf(buffer,
                       buffer_size,
                       "MD5 Hash Result\n===============\nInput: %s\nHash:  %s\n",
                       input,
                       hex);

    return (written < 0 || (size_t)written >= buffer_size) ? MD5_ERROR_INVALID_ARG : MD5_SUCCESS;
}

void tui_display_results(const char *input, const uint8_t digest[MD5_DIGEST_SIZE])
{
    char buffer[256];

    clear();
    if (tui_format_results(input, digest, buffer, sizeof(buffer)) == MD5_SUCCESS) {
        mvprintw(1, 2, "%s", buffer);
    } else {
        mvprintw(1, 2, "Unable to render results.");
    }
    refresh();
}

UIState tui_next_state_for_key(UIState state, int ch)
{
    if (state != STATE_MAIN_MENU) {
        return (ch == 'b' || ch == 'B') ? STATE_MAIN_MENU : state;
    }

    switch (ch) {
    case '1':
        return STATE_HASH_TEXT;
    case '2':
    case '3':
        return STATE_HASH_FILE;
    case '4':
        return STATE_HASH_STDIN;
    case '5':
        return STATE_SETTINGS;
    case '6':
        return STATE_ABOUT;
    case '7':
    case 'q':
    case 'Q':
        return STATE_RESULTS;
    default:
        return state;
    }
}

int tui_run(void)
{
    UIState state = STATE_MAIN_MENU;
    int key;

    if (tui_init() != MD5_SUCCESS) {
        return MD5_ERROR_TUI_INIT;
    }

    do {
        switch (state) {
        case STATE_MAIN_MENU:
            tui_display_menu();
            break;
        case STATE_HASH_TEXT:
            tui_display_text_input();
            break;
        case STATE_HASH_FILE:
            tui_display_file_browser();
            break;
        case STATE_HASH_STDIN:
        case STATE_SETTINGS:
        case STATE_ABOUT:
            clear();
            mvprintw(1, 2, "Screen scaffolded. Press 'b' to return.");
            refresh();
            break;
        case STATE_RESULTS:
            break;
        }

        key = getch();
        state = tui_next_state_for_key(state, key);
    } while (state != STATE_RESULTS);

    tui_cleanup();
    return MD5_SUCCESS;
}
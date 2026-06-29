#include <stdio.h>
#include <string.h>

#include "../src/tui.h"

static int test_state_transitions(void)
{
    if (tui_next_state_for_key(STATE_MAIN_MENU, '1') != STATE_HASH_TEXT) {
        return 1;
    }

    if (tui_next_state_for_key(STATE_HASH_TEXT, 'b') != STATE_MAIN_MENU) {
        return 1;
    }

    return 0;
}

static int test_result_formatting(void)
{
    uint8_t digest[MD5_DIGEST_SIZE] = {
        0x5dU, 0x41U, 0x40U, 0x2aU, 0xbcU, 0x4bU, 0x2aU, 0x76U,
        0xb9U, 0x71U, 0x9dU, 0x91U, 0x10U, 0x17U, 0xc5U, 0x92U
    };
    char buffer[256];

    if (tui_format_results("hello.txt", digest, buffer, sizeof(buffer)) != MD5_SUCCESS) {
        return 1;
    }

    return strstr(buffer, "5d41402abc4b2a76b9719d911017c592") == NULL;
}

int main(void)
{
    if (test_state_transitions() != 0) {
        fprintf(stderr, "state transition test failed\n");
        return 1;
    }

    if (test_result_formatting() != 0) {
        fprintf(stderr, "result formatting test failed\n");
        return 1;
    }

    puts("test_tui passed");
    return 0;
}
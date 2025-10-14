#include "terminal/input.h"
#include "terminal/ncurses_wrapper.h"
#include "utils/logger.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Input system structure */
struct InputSystem {
    KeyState key_states[MAX_TRACKED_KEYS];
    KeyState prev_key_states[MAX_TRACKED_KEYS];
    int last_key;
    size_t pressed_count;
    bool key_repeat_enabled;

    /* Text input */
    bool text_input_active;
    char* text_buffer;
    size_t text_buffer_size;
    size_t text_cursor;
};

InputSystem* input_system_create(void) {
    InputSystem* input = calloc(1, sizeof(InputSystem));
    if (!input) {
        LOG_ERROR("Failed to allocate input system");
        return NULL;
    }

    input->last_key = -1;
    input->key_repeat_enabled = true;
    input->text_input_active = false;

    LOG_DEBUG("Created input system");
    return input;
}

void input_system_destroy(InputSystem* input) {
    if (!input) return;

    free(input);
    LOG_DEBUG("Destroyed input system");
}

void input_system_update(InputSystem* input) {
    if (!input) return;

    /* Save previous states */
    memcpy(input->prev_key_states, input->key_states, sizeof(input->key_states));

    /* Reset transient states */
    input->last_key = -1;
    input->pressed_count = 0;

    /* Poll all keys */
    int ch;
    while ((ch = term_get_char()) != -1) {
        if (ch < 0 || ch >= MAX_TRACKED_KEYS) continue;

        input->last_key = ch;

        /* Update state */
        if (input->prev_key_states[ch] == KEY_STATE_UP ||
            input->prev_key_states[ch] == KEY_STATE_RELEASED) {
            input->key_states[ch] = KEY_STATE_PRESSED;
            input->pressed_count++;
        } else {
            /* Key is being held */
            input->key_states[ch] = KEY_STATE_DOWN;
        }

        /* Handle text input */
        if (input->text_input_active && input->text_buffer) {
            if (ch == KEY_BACKSPACE || ch == 127) {
                /* Backspace */
                if (input->text_cursor > 0) {
                    input->text_cursor--;
                    input->text_buffer[input->text_cursor] = '\0';
                }
            } else if (ch == KEY_ENTER) {
                /* Enter - do nothing, let caller handle */
            } else if (isprint(ch) && input->text_cursor < input->text_buffer_size - 1) {
                /* Add printable character */
                input->text_buffer[input->text_cursor++] = (char)ch;
                input->text_buffer[input->text_cursor] = '\0';
            }
        }
    }

    /* Update released keys */
    for (int i = 0; i < MAX_TRACKED_KEYS; i++) {
        if ((input->prev_key_states[i] == KEY_STATE_PRESSED ||
             input->prev_key_states[i] == KEY_STATE_DOWN) &&
            input->key_states[i] != KEY_STATE_PRESSED &&
            input->key_states[i] != KEY_STATE_DOWN) {
            input->key_states[i] = KEY_STATE_RELEASED;
        }
    }

    /* Transition released keys to up */
    for (int i = 0; i < MAX_TRACKED_KEYS; i++) {
        if (input->prev_key_states[i] == KEY_STATE_RELEASED) {
            input->key_states[i] = KEY_STATE_UP;
        }
    }

    /* Handle key repeat */
    if (!input->key_repeat_enabled) {
        for (int i = 0; i < MAX_TRACKED_KEYS; i++) {
            if (input->key_states[i] == KEY_STATE_DOWN &&
                input->prev_key_states[i] == KEY_STATE_PRESSED) {
                /* Prevent key from staying in down state */
                input->key_states[i] = KEY_STATE_UP;
            }
        }
    }
}

bool input_is_key_pressed(const InputSystem* input, int key) {
    if (!input || key < 0 || key >= MAX_TRACKED_KEYS) return false;
    return input->key_states[key] == KEY_STATE_PRESSED;
}

bool input_is_key_down(const InputSystem* input, int key) {
    if (!input || key < 0 || key >= MAX_TRACKED_KEYS) return false;
    KeyState state = input->key_states[key];
    return state == KEY_STATE_PRESSED || state == KEY_STATE_DOWN;
}

bool input_is_key_released(const InputSystem* input, int key) {
    if (!input || key < 0 || key >= MAX_TRACKED_KEYS) return false;
    return input->key_states[key] == KEY_STATE_RELEASED;
}

KeyState input_get_key_state(const InputSystem* input, int key) {
    if (!input || key < 0 || key >= MAX_TRACKED_KEYS) return KEY_STATE_UP;
    return input->key_states[key];
}

size_t input_get_pressed_count(const InputSystem* input) {
    return input ? input->pressed_count : 0;
}

bool input_is_any_key_pressed(const InputSystem* input) {
    return input && input->pressed_count > 0;
}

int input_get_last_key(const InputSystem* input) {
    return input ? input->last_key : -1;
}

void input_clear_all(InputSystem* input) {
    if (!input) return;

    memset(input->key_states, 0, sizeof(input->key_states));
    memset(input->prev_key_states, 0, sizeof(input->prev_key_states));
    input->last_key = -1;
    input->pressed_count = 0;

    LOG_DEBUG("Cleared all input states");
}

void input_set_key_repeat(InputSystem* input, bool enable) {
    if (!input) return;
    input->key_repeat_enabled = enable;
    LOG_DEBUG("Key repeat %s", enable ? "enabled" : "disabled");
}

bool input_get_key_repeat(const InputSystem* input) {
    return input ? input->key_repeat_enabled : false;
}

void input_begin_text_input(InputSystem* input, char* buffer, size_t buffer_size) {
    if (!input || !buffer || buffer_size == 0) return;

    input->text_input_active = true;
    input->text_buffer = buffer;
    input->text_buffer_size = buffer_size;
    input->text_cursor = strlen(buffer);

    LOG_DEBUG("Began text input");
}

void input_end_text_input(InputSystem* input) {
    if (!input) return;

    input->text_input_active = false;
    input->text_buffer = NULL;
    input->text_buffer_size = 0;
    input->text_cursor = 0;

    LOG_DEBUG("Ended text input");
}

bool input_is_text_input_active(const InputSystem* input) {
    return input && input->text_input_active;
}

const char* input_get_text(const InputSystem* input) {
    return (input && input->text_input_active) ? input->text_buffer : NULL;
}

size_t input_get_text_cursor(const InputSystem* input) {
    return input ? input->text_cursor : 0;
}

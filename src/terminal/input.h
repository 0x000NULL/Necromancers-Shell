#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Input System - Enhanced keyboard input handling
 *
 * Provides key state tracking (pressed, down, released) and input buffering.
 * Works with the ncurses wrapper for terminal input.
 *
 * Usage:
 *   InputSystem* input = input_system_create();
 *   // Each frame:
 *   input_system_update(input);
 *   if (input_is_key_pressed(input, KEY_W)) { move_forward(); }
 *   if (input_is_key_down(input, KEY_SPACE)) { charging_attack(); }
 *   input_system_destroy(input);
 */

/* Forward declaration */
typedef struct InputSystem InputSystem;

/* Key states */
typedef enum {
    KEY_STATE_UP = 0,       /* Not pressed */
    KEY_STATE_PRESSED,      /* Pressed this frame */
    KEY_STATE_DOWN,         /* Held down */
    KEY_STATE_RELEASED      /* Released this frame */
} KeyState;

/* Maximum tracked keys */
#define MAX_TRACKED_KEYS 256

/**
 * Create input system
 *
 * @return Input system pointer or NULL on failure
 */
InputSystem* input_system_create(void);

/**
 * Destroy input system
 *
 * @param input Input system
 */
void input_system_destroy(InputSystem* input);

/**
 * Update input state
 * Call once per frame to poll input and update key states
 *
 * @param input Input system
 */
void input_system_update(InputSystem* input);

/**
 * Check if key was just pressed this frame
 *
 * @param input Input system
 * @param key Key code
 * @return true if pressed this frame
 */
bool input_is_key_pressed(const InputSystem* input, int key);

/**
 * Check if key is currently held down
 * Returns true for multiple frames while key is held
 *
 * @param input Input system
 * @param key Key code
 * @return true if down
 */
bool input_is_key_down(const InputSystem* input, int key);

/**
 * Check if key was just released this frame
 *
 * @param input Input system
 * @param key Key code
 * @return true if released this frame
 */
bool input_is_key_released(const InputSystem* input, int key);

/**
 * Get key state
 *
 * @param input Input system
 * @param key Key code
 * @return Key state
 */
KeyState input_get_key_state(const InputSystem* input, int key);

/**
 * Get number of keys currently pressed
 *
 * @param input Input system
 * @return Count of pressed keys
 */
size_t input_get_pressed_count(const InputSystem* input);

/**
 * Check if any key is pressed
 *
 * @param input Input system
 * @return true if any key is pressed
 */
bool input_is_any_key_pressed(const InputSystem* input);

/**
 * Get last key that was pressed
 * Returns -1 if no key was pressed this frame
 *
 * @param input Input system
 * @return Key code or -1
 */
int input_get_last_key(const InputSystem* input);

/**
 * Clear all key states
 * Useful for menu transitions, etc.
 *
 * @param input Input system
 */
void input_clear_all(InputSystem* input);

/**
 * Enable/disable key repeat
 * When disabled, keys will only trigger once until released
 *
 * @param input Input system
 * @param enable true to enable repeat
 */
void input_set_key_repeat(InputSystem* input, bool enable);

/**
 * Get key repeat setting
 *
 * @param input Input system
 * @return true if repeat is enabled
 */
bool input_get_key_repeat(const InputSystem* input);

/* Text input buffer */

/**
 * Start text input mode
 * Begins accumulating text input into buffer
 *
 * @param input Input system
 * @param buffer Buffer to write text
 * @param buffer_size Size of buffer
 */
void input_begin_text_input(InputSystem* input, char* buffer, size_t buffer_size);

/**
 * End text input mode
 *
 * @param input Input system
 */
void input_end_text_input(InputSystem* input);

/**
 * Check if in text input mode
 *
 * @param input Input system
 * @return true if capturing text input
 */
bool input_is_text_input_active(const InputSystem* input);

/**
 * Get current text input
 *
 * @param input Input system
 * @return Text buffer or NULL if not in text mode
 */
const char* input_get_text(const InputSystem* input);

/**
 * Get text input cursor position
 *
 * @param input Input system
 * @return Cursor position in text buffer
 */
size_t input_get_text_cursor(const InputSystem* input);

/* Common key codes (ASCII + special keys) */

/* Printable ASCII */
#define KEY_SPACE ' '
#define KEY_EXCLAIM '!'
#define KEY_QUOTE '"'
#define KEY_HASH '#'
#define KEY_DOLLAR '$'
#define KEY_PERCENT '%'
#define KEY_AMPERSAND '&'
#define KEY_APOSTROPHE '\''
#define KEY_LPAREN '('
#define KEY_RPAREN ')'
#define KEY_ASTERISK '*'
#define KEY_PLUS '+'
#define KEY_COMMA ','
#define KEY_MINUS '-'
#define KEY_PERIOD '.'
#define KEY_SLASH '/'

/* Numbers */
#define KEY_0 '0'
#define KEY_1 '1'
#define KEY_2 '2'
#define KEY_3 '3'
#define KEY_4 '4'
#define KEY_5 '5'
#define KEY_6 '6'
#define KEY_7 '7'
#define KEY_8 '8'
#define KEY_9 '9'

/* Uppercase letters */
#define KEY_A 'A'
#define KEY_B 'B'
#define KEY_C 'C'
#define KEY_D 'D'
#define KEY_E 'E'
#define KEY_F 'F'
#define KEY_G 'G'
#define KEY_H 'H'
#define KEY_I 'I'
#define KEY_J 'J'
#define KEY_K 'K'
#define KEY_L 'L'
#define KEY_M 'M'
#define KEY_N 'N'
#define KEY_O 'O'
#define KEY_P 'P'
#define KEY_Q 'Q'
#define KEY_R 'R'
#define KEY_S 'S'
#define KEY_T 'T'
#define KEY_U 'U'
#define KEY_V 'V'
#define KEY_W 'W'
#define KEY_X 'X'
#define KEY_Y 'Y'
#define KEY_Z 'Z'

/* Lowercase letters */
#define KEY_a 'a'
#define KEY_b 'b'
#define KEY_c 'c'
#define KEY_d 'd'
#define KEY_e 'e'
#define KEY_f 'f'
#define KEY_g 'g'
#define KEY_h 'h'
#define KEY_i 'i'
#define KEY_j 'j'
#define KEY_k 'k'
#define KEY_l 'l'
#define KEY_m 'm'
#define KEY_n 'n'
#define KEY_o 'o'
#define KEY_p 'p'
#define KEY_q 'q'
#define KEY_r 'r'
#define KEY_s 's'
#define KEY_t 't'
#define KEY_u 'u'
#define KEY_v 'v'
#define KEY_w 'w'
#define KEY_x 'x'
#define KEY_y 'y'
#define KEY_z 'z'

/* Control keys */
#define KEY_ESCAPE 27
#define KEY_ENTER 10
#define KEY_TAB 9
#define KEY_BACKSPACE 127

#endif /* INPUT_H */

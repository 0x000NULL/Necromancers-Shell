# Terminal Interface Documentation

Comprehensive documentation for the Necromancer's Shell terminal interface system.

## Table of Contents

1. [Overview](#1-overview)
   - [Architecture](#architecture)
   - [Component Relationships](#component-relationships)
   - [Platform Considerations](#platform-considerations)
   - [Design Patterns](#design-patterns)

2. [NCurses Wrapper](#2-ncurses-wrapper)
   - [Terminal Initialization](#terminal-initialization)
   - [Terminal Operations](#terminal-operations)
   - [Window Management](#window-management)
   - [Drawing Functions](#drawing-functions)
   - [Cursor Operations](#cursor-operations)
   - [Usage Examples](#ncurses-usage-examples)

3. [Color System](#3-color-system)
   - [Color Types](#color-types)
   - [Color Initialization](#color-initialization)
   - [Color Pair Management](#color-pair-management)
   - [Usage Examples](#color-usage-examples)

4. [Input System](#4-input-system)
   - [Input System Structure](#input-system-structure)
   - [Key State Tracking](#key-state-tracking)
   - [Text Input Mode](#text-input-mode)
   - [Key Codes](#key-codes)
   - [Usage Examples](#input-usage-examples)

5. [Input Handler](#5-input-handler)
   - [Input Handler Structure](#input-handler-structure)
   - [Integration with History](#integration-with-history)
   - [Integration with Autocomplete](#integration-with-autocomplete)
   - [Command Execution Flow](#command-execution-flow)
   - [Usage Examples](#input-handler-usage-examples)

6. [UI Feedback](#6-ui-feedback)
   - [UI Feedback System](#ui-feedback-system)
   - [ANSI Color Codes](#ansi-color-codes)
   - [Message Types](#message-types)
   - [Command Result Display](#command-result-display)
   - [Usage Examples](#ui-feedback-usage-examples)

7. [Integration](#7-integration)
   - [Component Interaction](#component-interaction)
   - [Data Flow](#data-flow)
   - [Initialization Sequence](#initialization-sequence)
   - [Shutdown Sequence](#shutdown-sequence)
   - [Complete Usage Example](#complete-usage-example)

8. [Platform Compatibility](#8-platform-compatibility)
   - [Linux (ncurses)](#linux-ncurses)
   - [Windows (PDCurses)](#windows-pdcurses)
   - [macOS (ncurses)](#macos-ncurses)
   - [Terminal Compatibility](#terminal-compatibility)

9. [Key Bindings](#9-key-bindings)
   - [Standard Keys](#standard-keys)
   - [Special Keys](#special-keys)
   - [Control Sequences](#control-sequences)

10. [Performance](#10-performance)
    - [Rendering Optimization](#rendering-optimization)
    - [Update Frequency](#update-frequency)
    - [Memory Usage](#memory-usage)

11. [Extension Points](#11-extension-points)
    - [Custom Key Handlers](#custom-key-handlers)
    - [Custom Color Schemes](#custom-color-schemes)
    - [Terminal Resize Handling](#terminal-resize-handling)

12. [Error Handling](#12-error-handling)
    - [Initialization Failures](#initialization-failures)
    - [Color Support Detection](#color-support-detection)
    - [Recovery Strategies](#recovery-strategies)

---

## 1. Overview

The Terminal Interface system provides a comprehensive abstraction layer over ncurses for cross-platform terminal management. It consists of five main components that work together to provide keyboard input, color management, and visual feedback.

### Architecture

The system is organized in layers:

```
┌─────────────────────────────────────────┐
│         Application Layer               │
│   (Game Loop, Command Processing)       │
└─────────────────────────────────────────┘
                    │
┌─────────────────────────────────────────┐
│      Input Handler & UI Feedback        │
│  (High-level input & output handling)   │
└─────────────────────────────────────────┘
                    │
┌─────────────────────────────────────────┐
│    Input System    │   Color System     │
│ (Key state track)  │ (Color pairs)      │
└─────────────────────────────────────────┘
                    │
┌─────────────────────────────────────────┐
│         NCurses Wrapper                 │
│  (Low-level terminal abstraction)       │
└─────────────────────────────────────────┘
                    │
┌─────────────────────────────────────────┐
│            NCurses/PDCurses             │
│         (Terminal Library)              │
└─────────────────────────────────────────┘
```

### Component Relationships

- **NCurses Wrapper** (`ncurses_wrapper.c/h`): Low-level abstraction over ncurses, providing cross-platform terminal operations
- **Color System** (`colors.c/h`): Manages color pairs and provides game-specific color palette
- **Input System** (`input.c/h`): Tracks key states (pressed, down, released) with frame-based updates
- **Input Handler** (`input_handler.c/h`): High-level input integration with history, autocomplete, and command execution
- **UI Feedback** (`ui_feedback.c/h`): Formatted output with ANSI colors for user feedback

### Platform Considerations

The system is designed to work across:
- **Linux**: Uses ncurses library
- **Windows**: Can use PDCurses (drop-in replacement for ncurses)
- **macOS**: Uses ncurses library (typically included)

The ncurses wrapper abstracts platform differences, allowing the same code to run on all platforms.

### Design Patterns

1. **Abstraction**: NCurses wrapper hides platform-specific details
2. **Encapsulation**: Opaque structures (InputSystem, InputHandler) hide internal state
3. **Single Responsibility**: Each component has a focused purpose
4. **Resource Management**: Create/destroy pairs for all resources
5. **Error Handling**: Boolean returns and null checks throughout

---

## 2. NCurses Wrapper

The ncurses wrapper (`src/terminal/ncurses_wrapper.c/h`) provides a clean abstraction over the ncurses library.

### Terminal Initialization

**Function**: `term_init()`
**Source**: `src/terminal/ncurses_wrapper.c:10-45`

```c
bool term_init(void);
```

Initializes the terminal system with proper ncurses configuration.

**Implementation Details**:
```c
bool term_init(void) {
    if (g_term_initialized) {
        LOG_WARN("Terminal already initialized");
        return true;
    }

    /* Initialize ncurses */
    if (!initscr()) {
        LOG_ERROR("Failed to initialize ncurses");
        return false;
    }

    /* Configure terminal */
    cbreak();              /* Disable line buffering */
    noecho();              /* Don't echo input */
    keypad(stdscr, TRUE);  /* Enable function keys */
    nodelay(stdscr, TRUE); /* Non-blocking input */
    curs_set(0);           /* Hide cursor */

    /* Initialize colors if available */
    if (has_colors()) {
        start_color();
        use_default_colors();
        LOG_INFO("Terminal supports colors: %d pairs", COLOR_PAIRS);
    }

    g_term_initialized = true;
    return true;
}
```

**Configuration Details**:
- `cbreak()` - Disables line buffering (line 23), allowing character-by-character input
- `noecho()` - Disables automatic echo of typed characters (line 24)
- `keypad(stdscr, TRUE)` - Enables function keys like arrows, F1-F12, etc. (line 25)
- `nodelay(stdscr, TRUE)` - Makes input non-blocking (line 26)
- `curs_set(0)` - Hides the cursor (line 27)
- `start_color()` - Initializes color support (line 31)
- `use_default_colors()` - Allows terminal's default colors to be used (line 32)

**Function**: `term_shutdown()`
**Source**: `src/terminal/ncurses_wrapper.c:47-53`

```c
void term_shutdown(void);
```

Cleans up and shuts down the terminal system.

```c
void term_shutdown(void) {
    if (!g_term_initialized) return;

    LOG_DEBUG("Shutting down terminal");
    endwin();
    g_term_initialized = false;
}
```

Calls `endwin()` to restore terminal to normal mode (line 51).

### Terminal Operations

**Function**: `term_clear()`
**Source**: `src/terminal/ncurses_wrapper.c:55-57`

```c
void term_clear(void);
```

Clears the entire screen buffer (not displayed until `term_refresh()`).

**Function**: `term_refresh()`
**Source**: `src/terminal/ncurses_wrapper.c:59-61`

```c
void term_refresh(void);
```

Updates the physical screen with buffered changes. Essential for double-buffering.

**Function**: `term_get_size()`
**Source**: `src/terminal/ncurses_wrapper.c:63-66`

```c
void term_get_size(int* width, int* height);
```

Retrieves current terminal dimensions.

```c
void term_get_size(int* width, int* height) {
    if (width) *width = COLS;
    if (height) *height = LINES;
}
```

Uses ncurses globals `COLS` and `LINES` (lines 64-65).

### Window Management

**Function**: `term_print()`
**Source**: `src/terminal/ncurses_wrapper.c:68-78`

```c
void term_print(int x, int y, int color_pair, const char* str);
```

Prints a string at specified position with optional color.

```c
void term_print(int x, int y, int color_pair, const char* str) {
    if (!str) return;

    if (color_pair > 0) {
        attron(COLOR_PAIR(color_pair));
    }
    mvprintw(y, x, "%s", str);
    if (color_pair > 0) {
        attroff(COLOR_PAIR(color_pair));
    }
}
```

**Key Details**:
- Activates color pair with `attron()` (line 72)
- Uses `mvprintw()` for positioned printing (line 74)
- Deactivates color pair with `attroff()` (line 76)
- Note: Coordinates are (x, y) but ncurses uses (y, x)

**Function**: `term_printf()`
**Source**: `src/terminal/ncurses_wrapper.c:80-89`

```c
void term_printf(int x, int y, int color_pair, const char* fmt, ...);
```

Formatted printing at position with color support.

```c
void term_printf(int x, int y, int color_pair, const char* fmt, ...) {
    char buffer[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    term_print(x, y, color_pair, buffer);
}
```

Uses 1024-byte buffer (line 81) for formatted output.

### Drawing Functions

**Function**: `term_draw_box()`
**Source**: `src/terminal/ncurses_wrapper.c:91-117`

```c
void term_draw_box(int x, int y, int width, int height, int color_pair);
```

Draws a box using ACS (Alternative Character Set) line-drawing characters.

```c
void term_draw_box(int x, int y, int width, int height, int color_pair) {
    if (color_pair > 0) {
        attron(COLOR_PAIR(color_pair));
    }

    /* Corners */
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);

    /* Horizontal lines */
    for (int i = 1; i < width - 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + height - 1, x + i, ACS_HLINE);
    }

    /* Vertical lines */
    for (int i = 1; i < height - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + width - 1, ACS_VLINE);
    }

    if (color_pair > 0) {
        attroff(COLOR_PAIR(color_pair));
    }
}
```

**ACS Characters Used**:
- `ACS_ULCORNER` - Upper left corner (line 97)
- `ACS_URCORNER` - Upper right corner (line 98)
- `ACS_LLCORNER` - Lower left corner (line 99)
- `ACS_LRCORNER` - Lower right corner (line 100)
- `ACS_HLINE` - Horizontal line (lines 104, 105)
- `ACS_VLINE` - Vertical line (lines 110, 111)

**Function**: `term_draw_hline()`
**Source**: `src/terminal/ncurses_wrapper.c:119-127`

```c
void term_draw_hline(int x, int y, int length, int color_pair);
```

Draws a horizontal line using `ACS_HLINE`.

**Function**: `term_draw_vline()`
**Source**: `src/terminal/ncurses_wrapper.c:129-137`

```c
void term_draw_vline(int x, int y, int length, int color_pair);
```

Draws a vertical line using `ACS_VLINE`.

### Cursor Operations

**Function**: `term_set_cursor()`
**Source**: `src/terminal/ncurses_wrapper.c:139-141`

```c
void term_set_cursor(bool visible);
```

Shows or hides the terminal cursor.

```c
void term_set_cursor(bool visible) {
    curs_set(visible ? 1 : 0);
}
```

**Function**: `term_move_cursor()`
**Source**: `src/terminal/ncurses_wrapper.c:143-145`

```c
void term_move_cursor(int x, int y);
```

Moves the cursor to specified position.

### Input Configuration

**Function**: `term_set_echo()`
**Source**: `src/terminal/ncurses_wrapper.c:147-153`

```c
void term_set_echo(bool enable);
```

Controls whether typed characters are echoed to screen.

**Function**: `term_set_raw()`
**Source**: `src/terminal/ncurses_wrapper.c:155-161`

```c
void term_set_raw(bool enable_raw);
```

Switches between raw and cooked terminal modes.

### Color Detection

**Function**: `term_has_colors()`
**Source**: `src/terminal/ncurses_wrapper.c:163-165`

```c
bool term_has_colors(void);
```

Returns whether terminal supports colors.

**Function**: `term_max_color_pairs()`
**Source**: `src/terminal/ncurses_wrapper.c:167-169`

```c
int term_max_color_pairs(void);
```

Returns maximum number of color pairs supported.

**Function**: `term_get_char()`
**Source**: `src/terminal/ncurses_wrapper.c:171-173`

```c
int term_get_char(void);
```

Non-blocking character input. Returns -1 if no input available.

### NCurses Usage Examples

**Basic Terminal Setup**:
```c
// Initialize terminal
if (!term_init()) {
    fprintf(stderr, "Failed to initialize terminal\n");
    exit(1);
}

// Get terminal size
int width, height;
term_get_size(&width, &height);
printf("Terminal: %dx%d\n", width, height);

// Clear and draw
term_clear();
term_print(10, 5, 0, "Hello, World!");
term_refresh();

// Cleanup
term_shutdown();
```

**Drawing UI Elements**:
```c
term_clear();

// Draw a box
term_draw_box(5, 5, 30, 10, 1);

// Draw title
term_print(8, 6, 2, "Game Status");

// Draw separator
term_draw_hline(6, 7, 28, 1);

// Show cursor at input position
term_set_cursor(true);
term_move_cursor(7, 12);

term_refresh();
```

---

## 3. Color System

The color system (`src/terminal/colors.c/h`) manages ncurses color pairs and provides a game-specific color palette.

### Color Types

**Enum**: `GameColorPair`
**Source**: `src/terminal/colors.h:13-23`

```c
typedef enum {
    COLOR_PAIR_DEFAULT = 0,
    COLOR_PAIR_UI_BORDER = 1,
    COLOR_PAIR_UI_TEXT = 2,
    COLOR_PAIR_UI_HIGHLIGHT = 3,
    COLOR_PAIR_HP_HIGH = 4,
    COLOR_PAIR_HP_LOW = 5,
    COLOR_PAIR_ERROR = 6,
    COLOR_PAIR_SUCCESS = 7,
    COLOR_PAIR_COUNT
} GameColorPair;
```

Defines semantic color pairs for game UI elements:
- `COLOR_PAIR_DEFAULT` - Default terminal colors (0)
- `COLOR_PAIR_UI_BORDER` - Cyan on black for borders (1)
- `COLOR_PAIR_UI_TEXT` - White on black for normal text (2)
- `COLOR_PAIR_UI_HIGHLIGHT` - Yellow on black for highlights (3)
- `COLOR_PAIR_HP_HIGH` - Green on black for high HP (4)
- `COLOR_PAIR_HP_LOW` - Red on black for low HP (5)
- `COLOR_PAIR_ERROR` - Red on black for errors (6)
- `COLOR_PAIR_SUCCESS` - Green on black for success (7)
- `COLOR_PAIR_COUNT` - Total number of color pairs

### Color Initialization

**Function**: `colors_init()`
**Source**: `src/terminal/colors.c:9-43`

```c
bool colors_init(void);
```

Initializes the color system by setting up all game color pairs.

```c
bool colors_init(void) {
    if (!term_has_colors()) {
        LOG_WARN("Terminal does not support colors");
        return false;
    }

    LOG_INFO("Initializing color system with %d game color pairs", COLOR_PAIR_COUNT);

    /* Initialize game color pairs */
    g_game_pairs[COLOR_PAIR_DEFAULT] = 0;

    init_pair(COLOR_PAIR_UI_BORDER, COLOR_CYAN, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_UI_BORDER] = COLOR_PAIR_UI_BORDER;

    init_pair(COLOR_PAIR_UI_TEXT, COLOR_WHITE, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_UI_TEXT] = COLOR_PAIR_UI_TEXT;

    init_pair(COLOR_PAIR_UI_HIGHLIGHT, COLOR_YELLOW, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_UI_HIGHLIGHT] = COLOR_PAIR_UI_HIGHLIGHT;

    init_pair(COLOR_PAIR_HP_HIGH, COLOR_GREEN, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_HP_HIGH] = COLOR_PAIR_HP_HIGH;

    init_pair(COLOR_PAIR_HP_LOW, COLOR_RED, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_HP_LOW] = COLOR_PAIR_HP_LOW;

    init_pair(COLOR_PAIR_ERROR, COLOR_RED, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_ERROR] = COLOR_PAIR_ERROR;

    init_pair(COLOR_PAIR_SUCCESS, COLOR_GREEN, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_SUCCESS] = COLOR_PAIR_SUCCESS;

    LOG_DEBUG("Color system initialized");
    return true;
}
```

**Implementation Details**:
- Checks color support first (line 10)
- Returns false if no color support (line 12)
- Uses `init_pair()` to register each color combination
- Stores mappings in `g_game_pairs` array (line 7)
- All pairs use `COLOR_BLACK` background

### Color Pair Management

**Static Array**: `g_game_pairs`
**Source**: `src/terminal/colors.c:7`

```c
static int g_game_pairs[COLOR_PAIR_COUNT];
```

Internal array mapping game color pair enums to ncurses color pair IDs.

**Function**: `colors_get_game_pair()`
**Source**: `src/terminal/colors.c:45-50`

```c
int colors_get_game_pair(GameColorPair pair);
```

Retrieves the ncurses color pair ID for a game color pair.

```c
int colors_get_game_pair(GameColorPair pair) {
    if (pair < 0 || pair >= COLOR_PAIR_COUNT) {
        return 0;
    }
    return g_game_pairs[pair];
}
```

**Validation**: Bounds checking (line 46) ensures valid enum values.

### Color Usage Examples

**Basic Color Usage**:
```c
// Initialize colors
if (!colors_init()) {
    fprintf(stderr, "Colors not supported\n");
}

// Get color pair for borders
int border_color = colors_get_game_pair(COLOR_PAIR_UI_BORDER);

// Draw colored text
term_print(10, 5, border_color, "Status Window");

// Show HP with color based on health
int hp_color = (hp > 50) ?
    colors_get_game_pair(COLOR_PAIR_HP_HIGH) :
    colors_get_game_pair(COLOR_PAIR_HP_LOW);
term_printf(10, 6, hp_color, "HP: %d/%d", current_hp, max_hp);
```

**Creating UI Elements**:
```c
// Draw UI with semantic colors
int border = colors_get_game_pair(COLOR_PAIR_UI_BORDER);
int text = colors_get_game_pair(COLOR_PAIR_UI_TEXT);
int highlight = colors_get_game_pair(COLOR_PAIR_UI_HIGHLIGHT);

term_draw_box(0, 0, 40, 10, border);
term_print(2, 1, highlight, "Inventory");
term_print(2, 3, text, "Gold: 500");
term_print(2, 4, text, "Items: 12");
```

---

## 4. Input System

The input system (`src/terminal/input.c/h`) provides enhanced keyboard input handling with frame-based key state tracking.

### Input System Structure

**Structure**: `InputSystem`
**Source**: `src/terminal/input.c:9-21`

```c
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
```

**Fields**:
- `key_states[MAX_TRACKED_KEYS]` - Current state of all keys (line 10)
- `prev_key_states[MAX_TRACKED_KEYS]` - Previous frame's key states (line 11)
- `last_key` - Last key pressed this frame (line 12)
- `pressed_count` - Number of keys pressed this frame (line 13)
- `key_repeat_enabled` - Whether key repeat is enabled (line 14)
- `text_input_active` - Text input mode flag (line 17)
- `text_buffer` - Buffer for text input (line 18)
- `text_buffer_size` - Size of text buffer (line 19)
- `text_cursor` - Current cursor position in text (line 20)

**Constant**: `MAX_TRACKED_KEYS`
**Source**: `src/terminal/input.h:34`

```c
#define MAX_TRACKED_KEYS 256
```

Maximum number of keys that can be tracked simultaneously.

**Enum**: `KeyState`
**Source**: `src/terminal/input.h:26-31`

```c
typedef enum {
    KEY_STATE_UP = 0,       /* Not pressed */
    KEY_STATE_PRESSED,      /* Pressed this frame */
    KEY_STATE_DOWN,         /* Held down */
    KEY_STATE_RELEASED      /* Released this frame */
} KeyState;
```

Four-state key tracking:
- `KEY_STATE_UP` - Key is not pressed
- `KEY_STATE_PRESSED` - Key was just pressed this frame (one-frame state)
- `KEY_STATE_DOWN` - Key is being held down
- `KEY_STATE_RELEASED` - Key was just released this frame (one-frame state)

### Key State Tracking

**Function**: `input_system_create()`
**Source**: `src/terminal/input.c:23-36`

```c
InputSystem* input_system_create(void);
```

Creates and initializes an input system.

```c
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
```

**Function**: `input_system_destroy()`
**Source**: `src/terminal/input.c:38-43`

```c
void input_system_destroy(InputSystem* input);
```

Destroys input system and frees memory.

**Function**: `input_system_update()`
**Source**: `src/terminal/input.c:45-117`

```c
void input_system_update(InputSystem* input);
```

Updates input state for current frame. **Must be called once per frame.**

```c
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
```

**Update Algorithm**:
1. Save previous frame's states (line 49)
2. Reset transient states (lines 52-53)
3. Poll all available keys from terminal (line 57)
4. Update key states based on previous state (lines 63-69)
5. Handle text input if active (lines 72-87)
6. Detect released keys (lines 91-98)
7. Transition released keys to up state (lines 101-105)
8. Handle key repeat setting (lines 108-116)

**Key State Functions**:

```c
// Check if key was just pressed this frame
bool input_is_key_pressed(const InputSystem* input, int key);

// Check if key is currently held down
bool input_is_key_down(const InputSystem* input, int key);

// Check if key was just released this frame
bool input_is_key_released(const InputSystem* input, int key);

// Get key state directly
KeyState input_get_key_state(const InputSystem* input, int key);
```

**Source**: `src/terminal/input.c:119-138`

**Function**: `input_is_key_down()`
**Source**: `src/terminal/input.c:124-128`

```c
bool input_is_key_down(const InputSystem* input, int key) {
    if (!input || key < 0 || key >= MAX_TRACKED_KEYS) return false;
    KeyState state = input->key_states[key];
    return state == KEY_STATE_PRESSED || state == KEY_STATE_DOWN;
}
```

Note: Returns true for both PRESSED and DOWN states (line 127).

### Text Input Mode

**Function**: `input_begin_text_input()`
**Source**: `src/terminal/input.c:173-182`

```c
void input_begin_text_input(InputSystem* input, char* buffer, size_t buffer_size);
```

Starts text input mode, capturing characters into provided buffer.

```c
void input_begin_text_input(InputSystem* input, char* buffer, size_t buffer_size) {
    if (!input || !buffer || buffer_size == 0) return;

    input->text_input_active = true;
    input->text_buffer = buffer;
    input->text_buffer_size = buffer_size;
    input->text_cursor = strlen(buffer);

    LOG_DEBUG("Began text input");
}
```

Preserves existing buffer content (line 179).

**Function**: `input_end_text_input()`
**Source**: `src/terminal/input.c:184-193`

```c
void input_end_text_input(InputSystem* input);
```

Ends text input mode.

**Text Input Functions**:

```c
// Check if in text input mode
bool input_is_text_input_active(const InputSystem* input);

// Get current text
const char* input_get_text(const InputSystem* input);

// Get cursor position
size_t input_get_text_cursor(const InputSystem* input);
```

**Source**: `src/terminal/input.c:195-205`

### Key Codes

**Standard ASCII Keys** (`src/terminal/input.h:191-274`):
- Printable: `KEY_SPACE`, `KEY_EXCLAIM`, `KEY_QUOTE`, etc.
- Numbers: `KEY_0` through `KEY_9`
- Uppercase: `KEY_A` through `KEY_Z`
- Lowercase: `KEY_a` through `KEY_z`

**Control Keys** (`src/terminal/input.h:277-280`):
```c
#define KEY_ESCAPE 27
#define KEY_ENTER 10
#define KEY_TAB 9
#define KEY_BACKSPACE 127
```

**Special Keys**: NCurses provides additional key codes (accessed via `keypad()`):
- `KEY_UP`, `KEY_DOWN`, `KEY_LEFT`, `KEY_RIGHT` - Arrow keys
- `KEY_F(n)` - Function keys F1-F12
- `KEY_HOME`, `KEY_END` - Navigation
- `KEY_PPAGE`, `KEY_NPAGE` - Page up/down

### Input Usage Examples

**Basic Input Handling**:
```c
InputSystem* input = input_system_create();

// Game loop
while (running) {
    // Update input (call once per frame)
    input_system_update(input);

    // Check for pressed keys (one-frame trigger)
    if (input_is_key_pressed(input, KEY_SPACE)) {
        player_jump();
    }

    // Check for held keys (continuous)
    if (input_is_key_down(input, KEY_w)) {
        player_move_forward();
    }

    // Check for released keys
    if (input_is_key_released(input, KEY_SPACE)) {
        player_land();
    }

    // Check any key pressed
    if (input_is_any_key_pressed(input)) {
        menu_advance();
    }
}

input_system_destroy(input);
```

**Text Input Example**:
```c
char name_buffer[64] = "";
input_begin_text_input(input, name_buffer, sizeof(name_buffer));

while (!done) {
    input_system_update(input);

    // Display current text
    term_print(10, 5, 0, "Enter name: ");
    term_print(22, 5, 0, input_get_text(input));

    // Check for enter
    if (input_is_key_pressed(input, KEY_ENTER)) {
        done = true;
    }

    term_refresh();
}

input_end_text_input(input);
printf("Name entered: %s\n", name_buffer);
```

**Key Repeat Control**:
```c
// Disable key repeat for menus
input_set_key_repeat(input, false);

while (in_menu) {
    input_system_update(input);

    // Only triggers once per key press
    if (input_is_key_pressed(input, KEY_DOWN)) {
        menu_next_item();
    }
}

// Re-enable for gameplay
input_set_key_repeat(input, true);
```

---

## 5. Input Handler

The input handler (`src/terminal/input_handler.c/h`) provides high-level input integration with command history, autocomplete, and execution.

### Input Handler Structure

**Structure**: `InputHandler`
**Source**: `src/terminal/input_handler.c:12-18`

```c
struct InputHandler {
    CommandRegistry* registry;
    CommandHistory* history;
    Autocomplete* autocomplete;
    struct termios orig_termios;
    bool raw_mode_enabled;
};
```

**Fields**:
- `registry` - Command registry for available commands (line 13)
- `history` - Command history system (line 14)
- `autocomplete` - Autocomplete system (line 15)
- `orig_termios` - Original terminal settings (line 16)
- `raw_mode_enabled` - Raw mode state flag (line 17)

**Function**: `input_handler_create()`
**Source**: `src/terminal/input_handler.c:51-71`

```c
InputHandler* input_handler_create(CommandRegistry* registry);
```

Creates input handler and initializes subsystems.

```c
InputHandler* input_handler_create(CommandRegistry* registry) {
    if (!registry) return NULL;

    InputHandler* handler = malloc(sizeof(InputHandler));
    if (!handler) return NULL;

    handler->registry = registry;
    handler->history = command_history_create(100);
    handler->autocomplete = autocomplete_create(registry);
    handler->raw_mode_enabled = false;

    if (!handler->history || !handler->autocomplete) {
        input_handler_destroy(handler);
        return NULL;
    }

    /* Try to load history */
    input_handler_load_history(handler);

    return handler;
}
```

**Initialization**:
- Creates history with 100 entry limit (line 58)
- Creates autocomplete linked to registry (line 59)
- Loads saved history from disk (line 68)

**Function**: `input_handler_destroy()`
**Source**: `src/terminal/input_handler.c:73-84`

```c
void input_handler_destroy(InputHandler* handler);
```

Destroys handler and saves history.

### Integration with History

The input handler integrates with the command history system for command recall and persistence.

**Function**: `input_handler_read_and_execute()`
**Source**: `src/terminal/input_handler.c:110-131`

```c
CommandResult input_handler_read_and_execute(InputHandler* handler, const char* prompt);
```

Reads a command line and executes it, adding to history.

```c
CommandResult input_handler_read_and_execute(InputHandler* handler, const char* prompt) {
    if (!handler) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Invalid input handler");
    }

    char input[4096];
    if (!input_handler_read_line(handler, prompt, input, sizeof(input))) {
        /* EOF - treat as quit */
        return command_result_exit("EOF received");
    }

    /* Skip empty input */
    if (input[0] == '\0') {
        return command_result_success(NULL);
    }

    /* Add to history */
    command_history_add(handler->history, input);

    /* Execute */
    return input_handler_execute(handler, input);
}
```

**History Integration** (line 127): Every non-empty command is added to history before execution.

**History Management Functions**:

```c
// Get history object
CommandHistory* input_handler_get_history(InputHandler* handler);

// Save history to disk
bool input_handler_save_history(InputHandler* handler);

// Load history from disk
bool input_handler_load_history(InputHandler* handler);
```

**Function**: `input_handler_save_history()`
**Source**: `src/terminal/input_handler.c:170-180`

```c
bool input_handler_save_history(InputHandler* handler) {
    if (!handler || !handler->history) return false;

    char* path = command_history_default_path();
    if (!path) return false;

    bool result = command_history_save(handler->history, path);
    free(path);

    return result;
}
```

Saves to default path (typically `~/.necromancer_history`).

### Integration with Autocomplete

The input handler provides autocomplete functionality for commands.

**Function**: `input_handler_get_autocomplete()`
**Source**: `src/terminal/input_handler.c:162-164`

```c
Autocomplete* input_handler_get_autocomplete(InputHandler* handler);
```

Returns autocomplete object for external use.

**Future Enhancement**: The header file (lines 1-121) documents autocomplete and history search features that are planned for Phase 2 implementation. Currently, basic line reading uses `fgets()` (line 97), but full character-by-character input handling is reserved for future implementation.

### Command Execution Flow

**Function**: `input_handler_execute()`
**Source**: `src/terminal/input_handler.c:133-156`

```c
CommandResult input_handler_execute(InputHandler* handler, const char* input);
```

Parses and executes a command string.

```c
CommandResult input_handler_execute(InputHandler* handler, const char* input) {
    if (!handler || !input) {
        return command_result_error(EXEC_ERROR_INTERNAL, "Invalid parameters");
    }

    /* Parse command */
    ParsedCommand* cmd = NULL;
    ParseResult parse_result = parse_command_string(input, handler->registry, &cmd);

    if (parse_result != PARSE_SUCCESS) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Parse error: %s",
                parse_error_string(parse_result));
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }

    /* Execute command */
    CommandResult result = execute_command(cmd);

    /* Cleanup */
    parsed_command_destroy(cmd);

    return result;
}
```

**Execution Flow**:
1. Parse command string (line 140)
2. Check parse result (line 142)
3. Return error if parsing failed (lines 143-146)
4. Execute parsed command (line 150)
5. Cleanup parsed command (line 153)
6. Return result (line 155)

### Raw Mode Support

**Function**: `enable_raw_mode()`
**Source**: `src/terminal/input_handler.c:22-41`

```c
static bool enable_raw_mode(InputHandler* handler) __attribute__((unused));
```

Enables raw terminal mode for character-by-character input.

```c
static bool enable_raw_mode(InputHandler* handler) {
    if (!isatty(STDIN_FILENO)) return false;

    if (tcgetattr(STDIN_FILENO, &handler->orig_termios) == -1) {
        return false;
    }

    struct termios raw = handler->orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return false;
    }

    handler->raw_mode_enabled = true;
    return true;
}
```

**Note**: Currently unused (line 22) but reserved for Phase 2 line editing implementation.

**Raw Mode Configuration**:
- Disables `ECHO` - no automatic echo (line 31)
- Disables `ICANON` - disables line buffering (line 31)
- Sets `VMIN = 1` - read at least 1 character (line 32)
- Sets `VTIME = 0` - no timeout (line 33)

**Function**: `disable_raw_mode()`
**Source**: `src/terminal/input_handler.c:44-49`

```c
static void disable_raw_mode(InputHandler* handler);
```

Restores original terminal settings.

### Input Handler Usage Examples

**Basic Command Loop**:
```c
CommandRegistry* registry = command_registry_create();
// Register commands...

InputHandler* handler = input_handler_create(registry);

bool running = true;
while (running) {
    // Read and execute command
    CommandResult result = input_handler_read_and_execute(handler, "necromancer> ");

    // Display result
    ui_feedback_command_result(&result);

    // Check for exit
    if (result.should_exit) {
        running = false;
    }

    // Cleanup result
    command_result_destroy(&result);
}

input_handler_destroy(handler);
command_registry_destroy(registry);
```

**Direct Command Execution**:
```c
// Execute command programmatically
CommandResult result = input_handler_execute(handler, "help");
if (result.success) {
    printf("%s\n", result.output);
}
command_result_destroy(&result);
```

**History Access**:
```c
// Get history for display
CommandHistory* history = input_handler_get_history(handler);
size_t count = command_history_count(history);

for (size_t i = 0; i < count; i++) {
    const char* cmd = command_history_get(history, i);
    printf("%zu: %s\n", i + 1, cmd);
}

// Manual history save
input_handler_save_history(handler);
```

---

## 6. UI Feedback

The UI feedback system (`src/terminal/ui_feedback.c/h`) provides formatted output with ANSI colors for consistent user feedback.

### UI Feedback System

**Global State**:
**Source**: `src/terminal/ui_feedback.c:26-34`

```c
static struct {
    bool initialized;
    bool color_enabled;
    bool is_tty;
} g_ui_state = {
    .initialized = false,
    .color_enabled = true,
    .is_tty = false
};
```

Tracks initialization state and color support.

**Function**: `ui_feedback_init()`
**Source**: `src/terminal/ui_feedback.c:36-47`

```c
bool ui_feedback_init(void);
```

Initializes UI feedback system and detects TTY.

```c
bool ui_feedback_init(void) {
    if (g_ui_state.initialized) return true;

    /* Check if stdout is a TTY */
    g_ui_state.is_tty = isatty(STDOUT_FILENO);

    /* Enable colors by default if TTY */
    g_ui_state.color_enabled = g_ui_state.is_tty;

    g_ui_state.initialized = true;
    return true;
}
```

Colors are automatically enabled only if output is a TTY (lines 40, 43).

**Function**: `ui_feedback_shutdown()`
**Source**: `src/terminal/ui_feedback.c:49-51`

```c
void ui_feedback_shutdown(void);
```

Shuts down UI feedback system.

### ANSI Color Codes

**Color Definitions**:
**Source**: `src/terminal/ui_feedback.c:8-24`

```c
/* ANSI color codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_GRAY    "\033[90m"

/* Bold colors */
#define COLOR_BOLD_RED     "\033[1;31m"
#define COLOR_BOLD_GREEN   "\033[1;32m"
#define COLOR_BOLD_YELLOW  "\033[1;33m"
#define COLOR_BOLD_BLUE    "\033[1;34m"
#define COLOR_BOLD_MAGENTA "\033[1;35m"
```

**ANSI Escape Sequence Format**: `\033[<style>;<color>m`
- `\033` - ESC character
- `[` - Control Sequence Introducer (CSI)
- `<style>` - Text style (0=normal, 1=bold)
- `<color>` - Color code (31-37, 90-97)
- `m` - Color command terminator

**Standard Colors** (30-37):
- 31: Red
- 32: Green
- 33: Yellow
- 34: Blue
- 35: Magenta
- 36: Cyan
- 37: White

**Bright Colors** (90-97):
- 90: Bright Black (Gray)

**Function**: `ui_feedback_print_colored()`
**Source**: `src/terminal/ui_feedback.c:61-72`

```c
void ui_feedback_print_colored(const char* color, const char* prefix,
                              const char* message);
```

Core function for printing colored messages.

```c
void ui_feedback_print_colored(const char* color, const char* prefix,
                              const char* message) {
    if (!message) return;

    if (g_ui_state.color_enabled && color) {
        printf("%s%s%s ", color, prefix ? prefix : "", COLOR_RESET);
        printf("%s\n", message);
    } else {
        printf("%s %s\n", prefix ? prefix : "", message);
    }
    fflush(stdout);
}
```

**Behavior**:
- Prints colored prefix if colors enabled (line 66)
- Falls back to plain text if no color support (line 69)
- Always flushes output (line 71)

### Message Types

**Function**: `ui_feedback_success()`
**Source**: `src/terminal/ui_feedback.c:74-76`

```c
void ui_feedback_success(const char* message);
```

Displays success message with green `[SUCCESS]` prefix.

```c
void ui_feedback_success(const char* message) {
    ui_feedback_print_colored(COLOR_BOLD_GREEN, "[SUCCESS]", message);
}
```

**Function**: `ui_feedback_error()`
**Source**: `src/terminal/ui_feedback.c:78-80`

```c
void ui_feedback_error(const char* message);
```

Displays error message with red `[ERROR]` prefix.

```c
void ui_feedback_error(const char* message) {
    ui_feedback_print_colored(COLOR_BOLD_RED, "[ERROR]", message);
}
```

**Function**: `ui_feedback_warning()`
**Source**: `src/terminal/ui_feedback.c:82-84`

```c
void ui_feedback_warning(const char* message);
```

Displays warning message with yellow `[WARNING]` prefix.

```c
void ui_feedback_warning(const char* message) {
    ui_feedback_print_colored(COLOR_BOLD_YELLOW, "[WARNING]", message);
}
```

**Function**: `ui_feedback_info()`
**Source**: `src/terminal/ui_feedback.c:86-88`

```c
void ui_feedback_info(const char* message);
```

Displays info message with blue `[INFO]` prefix.

```c
void ui_feedback_info(const char* message) {
    ui_feedback_print_colored(COLOR_BOLD_BLUE, "[INFO]", message);
}
```

### Command Result Display

**Function**: `ui_feedback_command_result()`
**Source**: `src/terminal/ui_feedback.c:90-109`

```c
void ui_feedback_command_result(const CommandResult* result);
```

Displays command result with appropriate formatting.

```c
void ui_feedback_command_result(const CommandResult* result) {
    if (!result) return;

    if (result->success) {
        if (result->output && result->output[0] != '\0') {
            printf("%s", result->output);
            if (result->output[strlen(result->output) - 1] != '\n') {
                printf("\n");
            }
            fflush(stdout);
        }
    } else {
        /* Display error */
        if (result->error_message) {
            ui_feedback_error(result->error_message);
        } else {
            ui_feedback_error(execution_status_string(result->status));
        }
    }
}
```

**Logic**:
- Success: Print output directly (lines 94-100)
- Failure: Display error message or status (lines 102-108)
- Ensures newline at end (lines 96-98)

### Prompt Display

**Function**: `ui_feedback_prompt()`
**Source**: `src/terminal/ui_feedback.c:111-120`

```c
void ui_feedback_prompt(const char* prompt);
```

Displays command prompt with magenta color.

```c
void ui_feedback_prompt(const char* prompt) {
    if (!prompt) prompt = "> ";

    if (g_ui_state.color_enabled) {
        printf("%s%s%s", COLOR_BOLD_MAGENTA, prompt, COLOR_RESET);
    } else {
        printf("%s", prompt);
    }
    fflush(stdout);
}
```

Default prompt is `"> "` (line 112).

### Autocomplete Display

**Function**: `ui_feedback_autocomplete()`
**Source**: `src/terminal/ui_feedback.c:122-150`

```c
void ui_feedback_autocomplete(char** suggestions, size_t count, int current_index);
```

Displays autocomplete suggestions with selection highlight.

```c
void ui_feedback_autocomplete(char** suggestions, size_t count, int current_index) {
    if (!suggestions || count == 0) return;

    printf("\n");

    if (g_ui_state.color_enabled) {
        printf("%s[Suggestions]%s\n", COLOR_CYAN, COLOR_RESET);
    } else {
        printf("[Suggestions]\n");
    }

    for (size_t i = 0; i < count && i < 20; i++) {  /* Limit to 20 suggestions */
        if ((int)i == current_index) {
            if (g_ui_state.color_enabled) {
                printf("%s> %s%s\n", COLOR_BOLD_GREEN, suggestions[i], COLOR_RESET);
            } else {
                printf("> %s\n", suggestions[i]);
            }
        } else {
            printf("  %s\n", suggestions[i]);
        }
    }

    if (count > 20) {
        printf("  ... and %zu more\n", count - 20);
    }

    fflush(stdout);
}
```

**Display Details**:
- Limits display to 20 suggestions (line 133)
- Highlights current selection with `>` and green color (lines 135-136)
- Shows count of additional suggestions (lines 145-147)

**Function**: `ui_feedback_clear_autocomplete()`
**Source**: `src/terminal/ui_feedback.c:152-161`

```c
void ui_feedback_clear_autocomplete(void);
```

Clears autocomplete display using ANSI escape sequences.

```c
void ui_feedback_clear_autocomplete(void) {
    /* Move cursor up and clear lines */
    /* This is a simple implementation - a full implementation would track
     * how many lines were printed and clear exactly those */
    if (g_ui_state.is_tty) {
        printf("\033[F");  /* Move cursor up */
        printf("\033[K");  /* Clear line */
        fflush(stdout);
    }
}
```

Uses ANSI cursor control:
- `\033[F` - Move cursor up one line (line 157)
- `\033[K` - Clear from cursor to end of line (line 158)

### History Search Display

**Function**: `ui_feedback_history_search()`
**Source**: `src/terminal/ui_feedback.c:163-196`

```c
void ui_feedback_history_search(const char* query, char** results,
                               size_t count, int current_index);
```

Displays history search results with query and selection.

```c
void ui_feedback_history_search(const char* query, char** results,
                               size_t count, int current_index) {
    if (!query) return;

    printf("\n");

    if (g_ui_state.color_enabled) {
        printf("%s[History Search: \"%s\"]%s\n", COLOR_YELLOW, query, COLOR_RESET);
    } else {
        printf("[History Search: \"%s\"]\n", query);
    }

    if (count == 0) {
        printf("  No matches found\n");
    } else {
        for (size_t i = 0; i < count && i < 10; i++) {  /* Limit to 10 results */
            if ((int)i == current_index) {
                if (g_ui_state.color_enabled) {
                    printf("%s> %s%s\n", COLOR_BOLD_GREEN, results[i], COLOR_RESET);
                } else {
                    printf("> %s\n", results[i]);
                }
            } else {
                printf("  %s\n", results[i]);
            }
        }

        if (count > 10) {
            printf("  ... and %zu more\n", count - 10);
        }
    }

    fflush(stdout);
}
```

**Display Details**:
- Shows search query in yellow (line 170)
- Limits results to 10 (line 178)
- Highlights selection (lines 180-181)
- Shows "No matches found" if empty (line 176)

### Color Control

**Function**: `ui_feedback_set_color_enabled()`
**Source**: `src/terminal/ui_feedback.c:53-55`

```c
void ui_feedback_set_color_enabled(bool enabled);
```

Enables or disables color output.

```c
void ui_feedback_set_color_enabled(bool enabled) {
    g_ui_state.color_enabled = enabled && g_ui_state.is_tty;
}
```

Colors can only be enabled if output is a TTY (line 54).

**Function**: `ui_feedback_is_color_enabled()`
**Source**: `src/terminal/ui_feedback.c:57-59`

```c
bool ui_feedback_is_color_enabled(void);
```

Returns current color enable state.

### UI Feedback Usage Examples

**Basic Feedback Messages**:
```c
ui_feedback_init();

// Display different message types
ui_feedback_success("Player saved successfully");
ui_feedback_error("Failed to load game file");
ui_feedback_warning("Save file is corrupted, using backup");
ui_feedback_info("Loading assets...");

ui_feedback_shutdown();
```

**Command Result Display**:
```c
CommandResult result = execute_command(cmd);
ui_feedback_command_result(&result);
command_result_destroy(&result);
```

**Custom Prompt**:
```c
// Standard prompt
ui_feedback_prompt("necromancer> ");

// Custom prompt
ui_feedback_prompt("[dungeon_1]> ");

// Simple prompt
ui_feedback_prompt("> ");
```

**Autocomplete Display**:
```c
char* suggestions[] = {"attack", "cast", "defend", "flee"};
int selected = 1;

ui_feedback_autocomplete(suggestions, 4, selected);

// Output:
// [Suggestions]
//   attack
// > cast
//   defend
//   flee
```

**Disable Colors**:
```c
// For piping or logging
ui_feedback_set_color_enabled(false);
ui_feedback_error("This will be plain text");

// Re-enable
ui_feedback_set_color_enabled(true);
```

---

## 7. Integration

This section describes how all terminal interface components work together.

### Component Interaction

The components form a layered architecture:

```
Application Layer
      |
      v
┌─────────────────────────────────┐
│      Input Handler              │  High-level command I/O
│  - Read/Execute/History         │
└─────────────────────────────────┘
      |                    |
      v                    v
┌──────────────┐    ┌─────────────────┐
│ Input System │    │  UI Feedback    │
│ - Key States │    │  - Colored msgs │
└──────────────┘    └─────────────────┘
      |                    |
      v                    v
┌──────────────────────────────────┐
│        NCurses Wrapper           │  Low-level terminal
│  - Drawing/Colors/Input          │
└──────────────────────────────────┘
      |
      v
┌──────────────────────────────────┐
│     NCurses/PDCurses             │  System library
└──────────────────────────────────┘
```

**Dependencies**:
- **Input Handler** uses: Input System (future), UI Feedback, Command subsystems
- **Input System** uses: NCurses Wrapper (via `term_get_char()`)
- **UI Feedback** uses: ANSI codes directly (stdout)
- **Color System** uses: NCurses Wrapper (via `term_has_colors()`)
- **NCurses Wrapper** uses: NCurses library

### Data Flow

**Input Flow**:
```
Terminal Input
      |
      v
term_get_char() [ncurses_wrapper]
      |
      v
input_system_update() [input]
      |
      v
input_is_key_pressed() [input]
      |
      v
Application
```

**Command Execution Flow**:
```
User Input (stdin)
      |
      v
input_handler_read_line() [input_handler]
      |
      v
command_history_add() [history]
      |
      v
input_handler_execute() [input_handler]
      |
      v
parse_command_string() [parser]
      |
      v
execute_command() [executor]
      |
      v
CommandResult
      |
      v
ui_feedback_command_result() [ui_feedback]
      |
      v
Terminal Output (stdout)
```

**Rendering Flow**:
```
Application
      |
      v
term_clear() [ncurses_wrapper]
      |
      v
term_print() / term_draw_box() [ncurses_wrapper]
      |
      v
COLOR_PAIR(n) [colors]
      |
      v
NCurses buffer
      |
      v
term_refresh() [ncurses_wrapper]
      |
      v
Physical Screen
```

### Initialization Sequence

**Complete Initialization**:
```c
// 1. Initialize logger (if used)
logger_init();

// 2. Initialize terminal
if (!term_init()) {
    fprintf(stderr, "Failed to initialize terminal\n");
    return 1;
}

// 3. Initialize colors
if (!colors_init()) {
    fprintf(stderr, "Warning: Colors not available\n");
}

// 4. Initialize UI feedback
if (!ui_feedback_init()) {
    fprintf(stderr, "Failed to initialize UI feedback\n");
    term_shutdown();
    return 1;
}

// 5. Create input system (if using frame-based input)
InputSystem* input = input_system_create();
if (!input) {
    fprintf(stderr, "Failed to create input system\n");
    ui_feedback_shutdown();
    term_shutdown();
    return 1;
}

// 6. Create command registry
CommandRegistry* registry = command_registry_create();
register_builtin_commands(registry);

// 7. Create input handler (for command-based input)
InputHandler* handler = input_handler_create(registry);
if (!handler) {
    fprintf(stderr, "Failed to create input handler\n");
    input_system_destroy(input);
    command_registry_destroy(registry);
    ui_feedback_shutdown();
    term_shutdown();
    return 1;
}

// Now ready to run
```

**Initialization Order**: Bottom-up - lower-level systems first.

### Shutdown Sequence

**Complete Shutdown**:
```c
// 1. Destroy input handler (saves history)
input_handler_destroy(handler);

// 2. Destroy command registry
command_registry_destroy(registry);

// 3. Destroy input system
input_system_destroy(input);

// 4. Shutdown UI feedback
ui_feedback_shutdown();

// 5. Shutdown terminal
term_shutdown();

// 6. Shutdown logger (if used)
logger_shutdown();
```

**Shutdown Order**: Top-down - higher-level systems first.

### Complete Usage Example

**Full Terminal Application**:
```c
#include "terminal/ncurses_wrapper.h"
#include "terminal/colors.h"
#include "terminal/input.h"
#include "terminal/input_handler.h"
#include "terminal/ui_feedback.h"
#include "commands/registry.h"
#include "commands/builtins.h"

int main(void) {
    /* ===== INITIALIZATION ===== */

    // Initialize terminal
    if (!term_init()) {
        fprintf(stderr, "Failed to initialize terminal\n");
        return 1;
    }

    // Initialize colors
    colors_init();

    // Initialize UI feedback
    if (!ui_feedback_init()) {
        fprintf(stderr, "Failed to initialize UI feedback\n");
        term_shutdown();
        return 1;
    }

    // Create command registry
    CommandRegistry* registry = command_registry_create();
    if (!registry) {
        ui_feedback_error("Failed to create command registry");
        ui_feedback_shutdown();
        term_shutdown();
        return 1;
    }

    // Register builtin commands
    register_builtin_commands(registry);

    // Create input handler
    InputHandler* handler = input_handler_create(registry);
    if (!handler) {
        ui_feedback_error("Failed to create input handler");
        command_registry_destroy(registry);
        ui_feedback_shutdown();
        term_shutdown();
        return 1;
    }

    /* ===== MAIN LOOP ===== */

    ui_feedback_info("Necromancer's Shell initialized");
    ui_feedback_info("Type 'help' for commands or 'quit' to exit");

    bool running = true;
    while (running) {
        // Read and execute command
        CommandResult result = input_handler_read_and_execute(
            handler, "necromancer> "
        );

        // Display result
        ui_feedback_command_result(&result);

        // Check for exit
        if (result.should_exit) {
            running = false;
        }

        // Cleanup result
        command_result_destroy(&result);
    }

    /* ===== SHUTDOWN ===== */

    ui_feedback_info("Shutting down...");

    input_handler_destroy(handler);
    command_registry_destroy(registry);
    ui_feedback_shutdown();
    term_shutdown();

    return 0;
}
```

**Game Loop with Frame-Based Input**:
```c
int main(void) {
    /* Initialization (same as above) */
    term_init();
    colors_init();

    // Create input system for frame-based input
    InputSystem* input = input_system_create();

    /* Game loop */
    bool running = true;
    while (running) {
        // Update input
        input_system_update(input);

        // Handle input
        if (input_is_key_pressed(input, KEY_ESCAPE)) {
            running = false;
        }

        if (input_is_key_down(input, KEY_w)) {
            player_move_north();
        }
        if (input_is_key_down(input, KEY_s)) {
            player_move_south();
        }
        if (input_is_key_down(input, KEY_a)) {
            player_move_west();
        }
        if (input_is_key_down(input, KEY_d)) {
            player_move_east();
        }

        if (input_is_key_pressed(input, KEY_SPACE)) {
            player_attack();
        }

        // Update game logic
        game_update();

        // Render
        term_clear();

        // Draw UI
        int border = colors_get_game_pair(COLOR_PAIR_UI_BORDER);
        int text = colors_get_game_pair(COLOR_PAIR_UI_TEXT);

        term_draw_box(0, 0, 80, 24, border);
        term_print(2, 1, text, "Dungeon Level 5");

        // Draw game world
        render_dungeon();
        render_player();
        render_enemies();

        // Draw HUD
        int hp_color = (player.hp > 50) ?
            colors_get_game_pair(COLOR_PAIR_HP_HIGH) :
            colors_get_game_pair(COLOR_PAIR_HP_LOW);
        term_printf(2, 22, hp_color, "HP: %d/%d", player.hp, player.max_hp);

        term_refresh();

        // Frame timing
        usleep(16667); // ~60 FPS
    }

    /* Cleanup */
    input_system_destroy(input);
    term_shutdown();

    return 0;
}
```

---

## 8. Platform Compatibility

The terminal interface system is designed for cross-platform compatibility.

### Linux (ncurses)

**Library**: `ncurses`

**Installation**:
```bash
# Debian/Ubuntu
sudo apt-get install libncurses-dev

# Fedora/RHEL
sudo dnf install ncurses-devel

# Arch
sudo pacman -S ncurses
```

**Compilation**:
```bash
gcc -o program program.c -lncurses
```

**Features**:
- Full ncurses support
- Color support (256 colors on modern terminals)
- UTF-8 support with `ncursesw`
- Mouse support available
- Terminal resize signals (SIGWINCH)

### Windows (PDCurses)

**Library**: `PDCurses` (Public Domain Curses)

**Installation**:
```bash
# Using vcpkg
vcpkg install pdcurses

# Or download from: https://pdcurses.org/
```

**Compilation**:
```bash
# MinGW
gcc -o program.exe program.c -lpdcurses

# MSVC
cl program.c pdcurses.lib
```

**Features**:
- Drop-in replacement for ncurses
- Windows console support
- Limited color support (16 colors)
- Mouse support available
- Alternative: Use Windows Terminal with better ANSI support

**PDCurses Differences**:
- Some ACS characters may differ
- Color support more limited
- No signals (SIGWINCH not available)
- `cbreak()` may behave differently

### macOS (ncurses)

**Library**: `ncurses` (included with Xcode Command Line Tools)

**Installation**:
```bash
# Install Xcode Command Line Tools
xcode-select --install
```

**Compilation**:
```bash
gcc -o program program.c -lncurses
```

**Features**:
- Full ncurses support (typically ncurses 5.x or 6.x)
- Color support
- UTF-8 support
- Similar to Linux

**macOS Notes**:
- Terminal.app has good ncurses support
- iTerm2 has excellent ncurses and ANSI support
- May need to set `TERM=xterm-256color` for full colors

### Terminal Compatibility

**Recommended Terminal Types**:

| Platform | Terminal | TERM setting | Colors |
|----------|----------|--------------|--------|
| Linux | gnome-terminal | xterm-256color | 256 |
| Linux | konsole | xterm-256color | 256 |
| Linux | xterm | xterm-256color | 256 |
| macOS | Terminal.app | xterm-256color | 256 |
| macOS | iTerm2 | xterm-256color | 256 |
| Windows | Windows Terminal | xterm-256color | 256 |
| Windows | cmd.exe | windows-ansi | 16 |

**Color Detection**:
The system automatically detects color support:
```c
if (term_has_colors()) {
    // Use colors
    start_color();
} else {
    // Fallback to monochrome
}
```

**Terminal Capability Detection**:
```c
// Get max color pairs
int max_pairs = term_max_color_pairs();

// Check if TTY (UI Feedback)
bool is_tty = isatty(STDOUT_FILENO);
```

**UTF-8 Support**:
For UTF-8 character support, link with `ncursesw`:
```bash
gcc -o program program.c -lncursesw
```

**Fallback Strategy**:
```c
// 1. Try color mode
if (term_has_colors()) {
    colors_init();
}

// 2. Disable colors if needed
if (!term_has_colors()) {
    ui_feedback_set_color_enabled(false);
}

// 3. Use plain ASCII if ACS not available
// (Most terminals support ACS, but can check with terminfo)
```

---

## 9. Key Bindings

Complete reference for key codes and bindings in the terminal interface.

### Standard Keys

**Printable ASCII** (`src/terminal/input.h:191-274`):

| Key | Code | Value | Description |
|-----|------|-------|-------------|
| SPACE | KEY_SPACE | 32 | Space bar |
| 0-9 | KEY_0 to KEY_9 | 48-57 | Number keys |
| A-Z | KEY_A to KEY_Z | 65-90 | Uppercase letters |
| a-z | KEY_a to KEY_z | 97-122 | Lowercase letters |

**Symbols**:
- `KEY_EXCLAIM` (!) = 33
- `KEY_QUOTE` (") = 34
- `KEY_HASH` (#) = 35
- `KEY_DOLLAR` ($) = 36
- `KEY_PERCENT` (%) = 37
- `KEY_AMPERSAND` (&) = 38
- `KEY_APOSTROPHE` (') = 39
- `KEY_LPAREN` (() = 40
- `KEY_RPAREN` ()) = 41
- `KEY_ASTERISK` (*) = 42
- `KEY_PLUS` (+) = 43
- `KEY_COMMA` (,) = 44
- `KEY_MINUS` (-) = 45
- `KEY_PERIOD` (.) = 46
- `KEY_SLASH` (/) = 47

### Special Keys

**Control Keys** (`src/terminal/input.h:277-280`):

| Key | Code | Value | Description |
|-----|------|-------|-------------|
| ESCAPE | KEY_ESCAPE | 27 | Escape key |
| ENTER | KEY_ENTER | 10 | Enter/Return |
| TAB | KEY_TAB | 9 | Tab key |
| BACKSPACE | KEY_BACKSPACE | 127 | Backspace/Delete |

**NCurses Special Keys** (requires `keypad()` enabled):

| Key | NCurses Constant | Description |
|-----|------------------|-------------|
| Up Arrow | KEY_UP | Arrow up |
| Down Arrow | KEY_DOWN | Arrow down |
| Left Arrow | KEY_LEFT | Arrow left |
| Right Arrow | KEY_RIGHT | Arrow right |
| Home | KEY_HOME | Home key |
| End | KEY_END | End key |
| Page Up | KEY_PPAGE | Page up |
| Page Down | KEY_NPAGE | Page down |
| Insert | KEY_IC | Insert |
| Delete | KEY_DC | Delete |
| F1-F12 | KEY_F(1) to KEY_F(12) | Function keys |

**Mouse Events** (if mouse enabled):
- KEY_MOUSE - Mouse event occurred

### Control Sequences

**Common Control Character Sequences**:

| Ctrl + Key | ASCII Value | Description |
|------------|-------------|-------------|
| Ctrl+A | 1 | Start of line |
| Ctrl+B | 2 | Move back |
| Ctrl+C | 3 | Interrupt (SIGINT) |
| Ctrl+D | 4 | EOF / Delete |
| Ctrl+E | 5 | End of line |
| Ctrl+F | 6 | Move forward |
| Ctrl+H | 8 | Backspace |
| Ctrl+I | 9 | Tab |
| Ctrl+J | 10 | Newline (Enter) |
| Ctrl+K | 11 | Kill to EOL |
| Ctrl+L | 12 | Clear screen |
| Ctrl+M | 13 | Carriage return |
| Ctrl+N | 14 | Next line |
| Ctrl+P | 16 | Previous line |
| Ctrl+R | 18 | Reverse search |
| Ctrl+U | 21 | Kill line |
| Ctrl+W | 23 | Kill word |
| Ctrl+Z | 26 | Suspend (SIGTSTP) |

**Future Implementation** (Phase 2 - Line Editing):
These control sequences are documented for future line editing support:
- Ctrl+A: Move to start of line
- Ctrl+E: Move to end of line
- Ctrl+K: Kill from cursor to end
- Ctrl+U: Kill entire line
- Ctrl+R: Reverse history search
- Up/Down: Navigate history

**Key Binding Example**:
```c
input_system_update(input);

// Standard keys
if (input_is_key_pressed(input, KEY_ENTER)) {
    submit_input();
}

if (input_is_key_pressed(input, KEY_ESCAPE)) {
    cancel_input();
}

if (input_is_key_pressed(input, KEY_TAB)) {
    autocomplete();
}

// Arrow keys (requires keypad enabled)
if (input_is_key_pressed(input, KEY_UP)) {
    history_previous();
}

if (input_is_key_pressed(input, KEY_DOWN)) {
    history_next();
}

// Function keys
if (input_is_key_pressed(input, KEY_F(1))) {
    show_help();
}

// Control sequences
if (input_is_key_pressed(input, 12)) {  // Ctrl+L
    term_clear();
    redraw_screen();
}
```

---

## 10. Performance

Performance considerations for the terminal interface system.

### Rendering Optimization

**Double Buffering**:
NCurses uses automatic double buffering:
```c
// Draw to back buffer
term_clear();
term_print(10, 5, 0, "Hello");
term_draw_box(0, 0, 80, 24, 1);

// Swap buffers and display
term_refresh();
```

All drawing operations write to a buffer. `term_refresh()` performs the actual screen update.

**Partial Updates**:
NCurses optimizes by only updating changed regions:
```c
// Only changed text is sent to terminal
term_print(10, 5, 0, "New text");
term_refresh();  // Only updates affected area
```

**Clear vs. Redraw**:
```c
// Full clear (expensive)
term_clear();
term_refresh();

// Selective redraw (cheaper)
term_print(x, y, 0, "        ");  // Overwrite with spaces
term_refresh();
```

**Best Practices**:
1. Batch all drawing operations
2. Call `term_refresh()` once per frame
3. Avoid clearing entire screen if not needed
4. Use color pairs instead of attributes for better performance
5. Minimize the number of `attron()`/`attroff()` calls

### Update Frequency

**Recommended Frame Rates**:

| Application Type | Target FPS | Frame Time |
|------------------|------------|------------|
| Command Shell | On-demand | N/A |
| Text Editor | 30-60 | 16-33ms |
| Real-time Game | 30-60 | 16-33ms |
| Animation | 24-30 | 33-41ms |

**Frame Timing Example**:
```c
#include <time.h>

const long FRAME_TIME_NS = 16666667;  // ~60 FPS (16.67ms)

while (running) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Update and render
    input_system_update(input);
    game_update();
    render_frame();
    term_refresh();

    // Calculate sleep time
    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000L +
                      (end.tv_nsec - start.tv_nsec);
    long sleep_ns = FRAME_TIME_NS - elapsed_ns;

    if (sleep_ns > 0) {
        struct timespec sleep_time = {
            .tv_sec = 0,
            .tv_nsec = sleep_ns
        };
        nanosleep(&sleep_time, NULL);
    }
}
```

**Input Polling**:
- Non-blocking input (`nodelay(stdscr, TRUE)`) allows frame-based updates
- `term_get_char()` returns -1 immediately if no input
- No blocking wait for user input

### Memory Usage

**Memory Footprint**:

| Component | Size | Notes |
|-----------|------|-------|
| InputSystem | ~4KB | 256 keys × 2 states + metadata |
| InputHandler | ~32 bytes | Pointers only |
| CommandHistory | Variable | ~100 entries × avg length |
| UI State | ~32 bytes | Global state structure |
| NCurses | ~100KB | Library overhead |

**Input System Memory**:
```c
sizeof(InputSystem) =
    sizeof(KeyState) × MAX_TRACKED_KEYS × 2  // key_states + prev_key_states
    + sizeof(int)                             // last_key
    + sizeof(size_t)                          // pressed_count
    + sizeof(bool)                            // key_repeat_enabled
    + sizeof(bool)                            // text_input_active
    + sizeof(char*)                           // text_buffer
    + sizeof(size_t) × 2                      // text_buffer_size + text_cursor

= 1 × 256 × 2 + 4 + 8 + 1 + 1 + 8 + 16
≈ 550 bytes + padding
```

**Memory Best Practices**:
1. Single InputSystem instance per application
2. Reuse InputHandler across commands
3. Limit history size (default: 100 entries)
4. Free CommandResults after use
5. Use stack buffers for text input when possible

**Text Input Buffer**:
```c
// Stack allocation (preferred for small buffers)
char buffer[256];
input_begin_text_input(input, buffer, sizeof(buffer));

// Heap allocation (for large or dynamic buffers)
char* large_buffer = malloc(4096);
input_begin_text_input(input, large_buffer, 4096);
// ... later ...
input_end_text_input(input);
free(large_buffer);
```

**Optimization Tips**:
1. **Reduce draw calls**: Batch multiple prints before refresh
2. **Limit redraws**: Only redraw changed regions
3. **Color pair reuse**: Use existing color pairs instead of creating new ones
4. **String pooling**: Reuse string buffers for formatting
5. **Minimize logging**: Debug logging can slow down rendering

---

## 11. Extension Points

Ways to extend and customize the terminal interface system.

### Custom Key Handlers

**Adding Custom Key Bindings**:
```c
void handle_custom_keys(InputSystem* input, GameState* game) {
    // Custom game controls
    if (input_is_key_pressed(input, KEY_i)) {
        toggle_inventory(game);
    }

    if (input_is_key_pressed(input, KEY_m)) {
        toggle_map(game);
    }

    if (input_is_key_pressed(input, KEY_c)) {
        open_character_sheet(game);
    }

    // Cheat keys (debug mode only)
    #ifdef DEBUG
    if (input_is_key_down(input, KEY_F(12))) {
        game->player.hp = game->player.max_hp;
    }
    #endif
}

// In game loop
input_system_update(input);
handle_custom_keys(input, game);
```

**Modal Input Handlers**:
```c
typedef enum {
    MODE_NORMAL,
    MODE_INVENTORY,
    MODE_DIALOG,
    MODE_TEXT_INPUT
} InputMode;

void handle_input(InputSystem* input, GameState* game) {
    input_system_update(input);

    switch (game->input_mode) {
        case MODE_NORMAL:
            handle_normal_mode(input, game);
            break;
        case MODE_INVENTORY:
            handle_inventory_mode(input, game);
            break;
        case MODE_DIALOG:
            handle_dialog_mode(input, game);
            break;
        case MODE_TEXT_INPUT:
            handle_text_input_mode(input, game);
            break;
    }
}
```

### Custom Color Schemes

**Extending Color Pairs**:
```c
// In colors.h
typedef enum {
    // ... existing colors ...
    COLOR_PAIR_SUCCESS = 7,

    // Custom game colors
    COLOR_PAIR_MANA = 8,
    COLOR_PAIR_POISON = 9,
    COLOR_PAIR_FIRE = 10,
    COLOR_PAIR_ICE = 11,
    COLOR_PAIR_GOLD = 12,

    COLOR_PAIR_COUNT
} GameColorPair;

// In colors.c
bool colors_init(void) {
    // ... existing initialization ...

    // Custom colors
    init_pair(COLOR_PAIR_MANA, COLOR_BLUE, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_MANA] = COLOR_PAIR_MANA;

    init_pair(COLOR_PAIR_POISON, COLOR_GREEN, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_POISON] = COLOR_PAIR_POISON;

    init_pair(COLOR_PAIR_FIRE, COLOR_RED, COLOR_YELLOW);
    g_game_pairs[COLOR_PAIR_FIRE] = COLOR_PAIR_FIRE;

    init_pair(COLOR_PAIR_ICE, COLOR_CYAN, COLOR_BLUE);
    g_game_pairs[COLOR_PAIR_ICE] = COLOR_PAIR_ICE;

    init_pair(COLOR_PAIR_GOLD, COLOR_YELLOW, COLOR_BLACK);
    g_game_pairs[COLOR_PAIR_GOLD] = COLOR_PAIR_GOLD;

    return true;
}
```

**Theme System**:
```c
typedef struct {
    const char* name;
    short fg;
    short bg;
} ColorTheme;

ColorTheme themes[] = {
    {"dark", COLOR_WHITE, COLOR_BLACK},
    {"light", COLOR_BLACK, COLOR_WHITE},
    {"solarized", COLOR_CYAN, -1},  // -1 = default
    {"cyberpunk", COLOR_MAGENTA, COLOR_BLACK}
};

void apply_theme(const char* theme_name) {
    // Find theme
    ColorTheme* theme = find_theme(theme_name);
    if (!theme) return;

    // Re-initialize color pairs
    init_pair(COLOR_PAIR_UI_TEXT, theme->fg, theme->bg);
    // ... other colors ...
}
```

### Terminal Resize Handling

**SIGWINCH Handler**:
```c
#include <signal.h>

static volatile bool g_resize_pending = false;

void sigwinch_handler(int sig) {
    (void)sig;
    g_resize_pending = true;
}

void setup_resize_handler(void) {
    struct sigaction sa;
    sa.sa_handler = sigwinch_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, NULL);
}

void handle_resize(void) {
    if (!g_resize_pending) return;

    // Update ncurses
    endwin();
    refresh();
    clear();

    // Get new size
    int width, height;
    term_get_size(&width, &height);

    // Adjust UI layout
    adjust_ui_layout(width, height);

    // Redraw everything
    redraw_all();
    term_refresh();

    g_resize_pending = false;
}

// In main loop
while (running) {
    handle_resize();
    // ... rest of loop ...
}
```

**Responsive Layout**:
```c
typedef struct {
    int min_width;
    int min_height;
    int ideal_width;
    int ideal_height;
} LayoutConstraints;

void render_ui(int width, int height) {
    LayoutConstraints ui = {
        .min_width = 40,
        .min_height = 20,
        .ideal_width = 80,
        .ideal_height = 24
    };

    if (width < ui.min_width || height < ui.min_height) {
        // Show "terminal too small" message
        term_clear();
        term_print(0, 0, 0, "Terminal too small!");
        term_printf(0, 1, 0, "Need: %dx%d", ui.min_width, ui.min_height);
        term_refresh();
        return;
    }

    // Adjust layout based on size
    if (width >= ui.ideal_width) {
        render_full_ui();
    } else {
        render_compact_ui();
    }
}
```

**Custom UI Widgets**:
```c
// Progress bar widget
void draw_progress_bar(int x, int y, int width, float progress, int color) {
    int filled = (int)(width * progress);

    term_print(x, y, color, "[");

    for (int i = 0; i < filled; i++) {
        term_print(x + 1 + i, y, color, "=");
    }

    for (int i = filled; i < width - 2; i++) {
        term_print(x + 1 + i, y, color, " ");
    }

    term_print(x + width - 1, y, color, "]");
}

// Menu widget
typedef struct {
    const char** items;
    size_t count;
    int selected;
} Menu;

void draw_menu(const Menu* menu, int x, int y, int highlight_color) {
    for (size_t i = 0; i < menu->count; i++) {
        int color = (i == menu->selected) ? highlight_color : 0;
        const char* prefix = (i == menu->selected) ? "> " : "  ";
        term_printf(x, y + i, color, "%s%s", prefix, menu->items[i]);
    }
}
```

---

## 12. Error Handling

Error handling strategies for the terminal interface system.

### Initialization Failures

**Terminal Initialization**:
```c
if (!term_init()) {
    fprintf(stderr, "Error: Failed to initialize terminal\n");
    fprintf(stderr, "Possible causes:\n");
    fprintf(stderr, "  - TERM environment variable not set\n");
    fprintf(stderr, "  - Terminal type not supported\n");
    fprintf(stderr, "  - NCurses library not installed\n");
    return 1;
}
```

**Common Failures**:
1. **Missing TERM variable**: Set `export TERM=xterm-256color`
2. **Unsupported terminal**: Use a compatible terminal emulator
3. **Missing library**: Install ncurses development package
4. **Permission issues**: Check terminal device permissions

**Graceful Degradation**:
```c
// Try full initialization
if (!term_init()) {
    // Fall back to basic stdio
    fprintf(stderr, "Warning: ncurses unavailable, using basic I/O\n");
    use_basic_io = true;
}

// Try color initialization
if (term_has_colors()) {
    colors_init();
} else {
    fprintf(stderr, "Warning: Colors not supported\n");
    ui_feedback_set_color_enabled(false);
}
```

### Color Support Detection

**Checking Color Capabilities**:
```c
bool setup_colors(void) {
    if (!term_has_colors()) {
        LOG_WARN("Terminal does not support colors");
        ui_feedback_set_color_enabled(false);
        return false;
    }

    int max_pairs = term_max_color_pairs();
    if (max_pairs < COLOR_PAIR_COUNT) {
        LOG_ERROR("Terminal supports only %d color pairs, need %d",
                 max_pairs, COLOR_PAIR_COUNT);
        return false;
    }

    if (!colors_init()) {
        LOG_ERROR("Failed to initialize color system");
        return false;
    }

    return true;
}
```

**Color Fallbacks**:
```c
int get_color_with_fallback(GameColorPair pair) {
    if (term_has_colors()) {
        return colors_get_game_pair(pair);
    }

    // Return 0 (default) if no colors
    return 0;
}

// Usage
int hp_color = get_color_with_fallback(
    (hp > 50) ? COLOR_PAIR_HP_HIGH : COLOR_PAIR_HP_LOW
);
term_printf(10, 5, hp_color, "HP: %d", hp);
```

### Recovery Strategies

**Terminal Corruption Recovery**:
```c
void recover_terminal(void) {
    // Reset terminal to sane state
    term_set_echo(false);
    term_set_raw(false);
    term_set_cursor(false);

    // Clear and redraw
    term_clear();
    redraw_all();
    term_refresh();
}

// Use with signal handlers
void sigint_handler(int sig) {
    (void)sig;
    term_shutdown();  // Clean exit
    exit(0);
}
```

**Handling Invalid Dimensions**:
```c
void check_terminal_size(void) {
    int width, height;
    term_get_size(&width, &height);

    const int MIN_WIDTH = 40;
    const int MIN_HEIGHT = 20;

    if (width < MIN_WIDTH || height < MIN_HEIGHT) {
        term_clear();
        term_print(0, 0, 0, "ERROR: Terminal too small!");
        term_printf(0, 1, 0, "Current: %dx%d", width, height);
        term_printf(0, 2, 0, "Required: %dx%d", MIN_WIDTH, MIN_HEIGHT);
        term_print(0, 4, 0, "Please resize your terminal and restart.");
        term_refresh();

        // Wait for resize or exit
        while (width < MIN_WIDTH || height < MIN_HEIGHT) {
            sleep(1);
            term_get_size(&width, &height);
        }
    }
}
```

**Input Error Handling**:
```c
void safe_input_update(InputSystem* input) {
    if (!input) {
        LOG_ERROR("Null input system");
        return;
    }

    // Clear error state
    errno = 0;

    // Update input
    input_system_update(input);

    // Check for errors
    if (errno != 0) {
        LOG_ERROR("Input error: %s", strerror(errno));
        input_clear_all(input);
    }
}
```

**Exit Cleanup**:
```c
void cleanup_on_exit(void) {
    // Always call term_shutdown() on exit
    term_shutdown();

    // Restore cursor
    printf("\033[?25h");  // Show cursor

    // Reset colors
    printf("\033[0m");

    // Flush output
    fflush(stdout);
}

// Register cleanup
int main(void) {
    atexit(cleanup_on_exit);

    // ... application code ...

    return 0;
}
```

**Logging Errors**:
```c
// Log terminal errors for debugging
bool term_init_with_logging(void) {
    LOG_INFO("Initializing terminal...");

    if (!term_init()) {
        LOG_ERROR("Terminal initialization failed");
        LOG_ERROR("TERM=%s", getenv("TERM") ? getenv("TERM") : "(not set)");
        LOG_ERROR("COLORTERM=%s", getenv("COLORTERM") ? getenv("COLORTERM") : "(not set)");
        return false;
    }

    int width, height;
    term_get_size(&width, &height);
    LOG_INFO("Terminal size: %dx%d", width, height);

    if (term_has_colors()) {
        LOG_INFO("Color support: YES (%d pairs)", term_max_color_pairs());
    } else {
        LOG_WARN("Color support: NO");
    }

    return true;
}
```

---

## Appendix

### File Reference

**Header Files**:
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/ncurses_wrapper.h` - Terminal wrapper API
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/colors.h` - Color system API
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/input.h` - Input system API
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/input_handler.h` - Input handler API
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/ui_feedback.h` - UI feedback API

**Implementation Files**:
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/ncurses_wrapper.c` - Terminal wrapper implementation
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/colors.c` - Color system implementation
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/input.c` - Input system implementation
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/input_handler.c` - Input handler implementation
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/terminal/ui_feedback.c` - UI feedback implementation

### Key Concepts Summary

1. **Abstraction Layers**: NCurses wrapper abstracts platform differences
2. **State Tracking**: Input system maintains frame-based key states
3. **Color Management**: Semantic color pairs for consistent UI
4. **High-Level Integration**: Input handler combines history, autocomplete, execution
5. **User Feedback**: UI feedback provides consistent colored output
6. **Cross-Platform**: Works on Linux, macOS, Windows (with PDCurses)
7. **Performance**: Double-buffered rendering, non-blocking input
8. **Extensibility**: Custom key handlers, colors, widgets

### Best Practices

1. **Initialization Order**: Always initialize terminal before other components
2. **Cleanup**: Always call shutdown functions in reverse order
3. **Frame Updates**: Call `input_system_update()` once per frame
4. **Rendering**: Batch draws and call `term_refresh()` once
5. **Error Handling**: Check return values, handle graceful degradation
6. **Memory Management**: Use create/destroy pairs consistently
7. **Color Detection**: Always check `term_has_colors()` before using colors
8. **Terminal Size**: Validate minimum size, handle resize events

---

**Document Version**: 1.0
**Last Updated**: 2025-10-13
**Authors**: Documentation generated from source code analysis

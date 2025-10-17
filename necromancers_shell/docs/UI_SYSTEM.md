# Interactive UI System Documentation

## Overview

The Interactive UI System provides reusable ncurses-based components for story events, dialogue sequences, and player choices. Built during Phase 7 sprint (Days 1-7).

**Location:** `src/game/ui/`

**Components:**
- `choice_prompt.h/c` - Generic choice prompts with input validation
- `narrative_display.h/c` - Text wrapping and scene rendering
- `story_ui.h` - Convenience header combining all UI systems

## Quick Start

### Include the UI System

```c
#include "src/game/ui/story_ui.h"  // Includes both choice_prompt and narrative_display
#include "src/terminal/platform_curses.h"
#include "src/terminal/colors.h"
```

### Display a Narrative Scene

```c
WINDOW* event_win = newwin(30, 100, 0, 0);
if (!event_win) {
    /* Handle non-interactive mode (tests) */
    return;
}

const char* scene_title = "THE VILLAGE OF ASHBROOK";
const char* paragraphs[] = {
    "You stand at the edge of a small village...",
    "147 souls. All unaware of their fate.",
    "This moment will define who you become."
};

display_narrative_scene(
    event_win,
    scene_title,
    paragraphs,
    3,  // paragraph count
    SCENE_COLOR_WARNING
);

delwin(event_win);
```

### Present a Binary Choice

```c
WINDOW* choice_win = newwin(30, 100, 0, 0);

Choice choices[] = {
    {
        .label = "Harvest souls",
        .key = 'h',
        .description = "+147 souls, +2,800 energy, +13% corruption"
    },
    {
        .label = "Spare the innocent",
        .key = 's',
        .description = "Show mercy, -2% corruption"
    }
};

int selected_index;
bool choice_made = display_choice_prompt(
    choice_win,
    "YOUR DECISION",
    NULL,  // Optional description
    choices,
    2,     // choice count
    &selected_index
);

if (choice_made) {
    if (selected_index == 0) {
        /* User chose to harvest */
    } else {
        /* User chose to spare */
    }
}

delwin(choice_win);
```

## API Reference

### Choice Prompt System

#### `Choice` Structure

```c
typedef struct {
    char label[64];          // Full label text (e.g., "Harvest souls")
    char key;                // Single-key shortcut (e.g., 'h')
    char description[128];   // Additional context
} Choice;
```

#### `display_choice_prompt()`

```c
bool display_choice_prompt(
    WINDOW* win,
    const char* title,
    const char* description,  // Can be NULL
    const Choice* choices,
    size_t choice_count,
    int* selected_index       // Output parameter
);
```

**Returns:** `true` if valid selection made, `false` if max attempts exceeded

**Features:**
- Accepts input by key ('h') OR full label ("harvest")
- Case-insensitive matching
- Input validation with retry logic (max 5 attempts)
- Error messages displayed in red

#### `prompt_yes_no()`

```c
bool prompt_yes_no(
    WINDOW* win,
    const char* title,
    const char* question,
    bool* yes_result  // Output: true if yes, false if no
);
```

Convenience wrapper for binary yes/no decisions.

### Narrative Display System

#### `WrappedText` Structure

```c
typedef struct {
    char** lines;       // Array of wrapped text lines
    size_t line_count;  // Number of lines
    size_t capacity;    // Allocated capacity
} WrappedText;
```

#### `wrap_text()`

```c
WrappedText* wrap_text(const char* text, int max_width);
void wrapped_text_destroy(WrappedText* wrapped);
```

Word-boundary text wrapping. Always pair `wrap_text()` with `wrapped_text_destroy()`.

#### `display_narrative_scene()`

```c
void display_narrative_scene(
    WINDOW* win,
    const char* title,
    const char** paragraphs,
    size_t paragraph_count,
    SceneColor title_color
);
```

Renders a complete narrative scene with:
- Centered title with decorative borders
- Multiple paragraphs with automatic wrapping
- Proper spacing and color coding

**Available Colors:**
- `SCENE_COLOR_TITLE` - Scene title (yellow)
- `SCENE_COLOR_TEXT` - Normal text (white)
- `SCENE_COLOR_EMPHASIS` - Emphasized text (yellow)
- `SCENE_COLOR_WARNING` - Warnings (red)
- `SCENE_COLOR_SUCCESS` - Success messages (green)
- `SCENE_COLOR_DIM` - Secondary text (gray)

#### `display_wrapped_paragraph()`

```c
int display_wrapped_paragraph(
    WINDOW* win,
    int start_y,
    int start_x,
    const char* text,
    int max_width,
    SceneColor color
);
```

**Returns:** Number of lines rendered

#### `wait_for_keypress()`

```c
void wait_for_keypress(WINDOW* win, int y);
```

Displays "Press any key to continue..." and waits for input.

#### `display_dialogue_line()`

```c
int display_dialogue_line(
    WINDOW* win,
    int y,
    const char* speaker,
    const char* dialogue,
    SceneColor speaker_color,
    SceneColor dialogue_color
);
```

Formats dialogue as: `Speaker Name: "Dialogue text..."`

**Returns:** Number of lines rendered

## Color Constants

Defined in `src/terminal/colors.h`:

```c
UI_BORDER       // Border/secondary UI elements
UI_TEXT         // Normal UI text
UI_HIGHLIGHT    // Highlighted text (yellow)
TEXT_NORMAL     // Normal text (white)
TEXT_INFO       // Information (yellow)
TEXT_ERROR      // Error messages (red)
TEXT_SUCCESS    // Success messages (green)
TEXT_WARNING    // Warning messages (red)
TEXT_DIM        // Dimmed text (gray)
```

## Non-Interactive Mode (Testing)

All UI functions gracefully handle `newwin()` returning NULL (non-interactive/test mode):

```c
WINDOW* event_win = newwin(30, 100, 0, 0);
if (!event_win) {
    /* Running in test mode - process event without UI */
    LOG_WARN("No terminal available, running in non-interactive mode");

    /* Execute default behavior or skip UI-dependent code */
    process_event_without_ui(state);
    return true;
}
```

## Memory Management

**Always cleanup ncurses windows:**

```c
WINDOW* win = newwin(30, 100, 0, 0);
if (!win) return;

/* ... use window ... */

delwin(win);  // Always cleanup!
```

**Text wrapping memory:**

```c
WrappedText* wrapped = wrap_text(text, 76);
if (wrapped) {
    /* ... use wrapped text ... */
    wrapped_text_destroy(wrapped);  // Always cleanup!
}
```

**Valgrind verified:** Zero memory leaks in UI system.

## Example: Complete Event Integration

See `src/game/events/ashbrook_event.c` and `src/game/events/thessara_contact_event.c` for complete examples.

### Ashbrook Event Pattern

```c
bool ashbrook_event_callback(GameState* state, uint32_t event_id) {
    /* Non-interactive mode fallback */
    WINDOW* event_win = newwin(30, 100, 0, 0);
    if (!event_win) {
        LOG_WARN("Non-interactive mode - using defaults");
        ashbrook_spare_village(state);
        return true;
    }

    /* Display narrative scene */
    const char* scene_title = "ASHBROOK VILLAGE - DAY 47";
    const char* paragraphs[] = {
        "Paragraph 1...",
        "Paragraph 2...",
        "Paragraph 3..."
    };
    display_narrative_scene(event_win, scene_title, paragraphs, 3, SCENE_COLOR_WARNING);

    /* Present choice */
    Choice choices[] = {
        {.label = "Harvest souls", .key = 'h', .description = "+147 souls"},
        {.label = "Spare village", .key = 's', .description = "Show mercy"}
    };

    int selected;
    bool choice_made = display_choice_prompt(
        event_win, "YOUR DECISION", NULL, choices, 2, &selected
    );

    /* Execute choice */
    if (choice_made) {
        wclear(event_win);

        if (selected == 0) {
            ashbrook_harvest_village(state);
            /* Display outcome scene */
        } else {
            ashbrook_spare_village(state);
            /* Display outcome scene */
        }

        wait_for_keypress(event_win, 20);
    }

    delwin(event_win);
    return true;
}
```

## Testing

Unit tests in `tests/test_choice_prompt.c`:
- Choice structure validation
- Binary/multi-choice setup
- Label/description length checks
- Array iteration and key uniqueness

**Run tests:**
```bash
make test
```

**Valgrind check:**
```bash
valgrind --leak-check=full ./build/test_test_choice_prompt
```

## Future Extensions

Planned features:
- Multi-page scene navigation
- Timed choices with countdown
- Conditional choice availability based on stats
- Choice preview system
- Dialogue tree integration

## Notes

- Text wrapping uses word-boundary breaking (respects spaces)
- Maximum line width: 76 characters (leaves 2-char margins)
- Input validation retries: 5 attempts max
- All strings are null-terminated
- UTF-8 not currently supported (ASCII only)

## Version

**Created:** Phase 7 Sprint (Days 1-7)
**Status:** Production-ready
**Dependencies:** ncurses/PDCurses, logger system
**Test Coverage:** 7/7 tests passing, zero memory leaks

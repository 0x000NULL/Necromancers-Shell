# Command System - Phase 1 MVP

Complete implementation of the command parsing and execution system for Necromancer's Shell.

## Overview

This command system provides a robust, extensible framework for handling user input with features including:

- **Tokenization**: Advanced parsing with quote handling and escape sequences
- **Command Registry**: O(1) command lookup with metadata and validation
- **Argument Parser**: Type-safe parsing with flag support
- **Command Executor**: Clean execution interface with result handling
- **Command History**: Persistent circular buffer (100 commands)
- **Autocomplete**: Context-aware completion using Trie data structure
- **Input Handler**: Integrated line editing with history and completion
- **UI Feedback**: Formatted, colorized output for results and errors

## Architecture

```
command_system.h/c         - Top-level API and initialization
├── tokenizer.h/c          - String tokenization with quotes/escapes
├── registry.h/c           - Command registration and lookup
├── parser.h/c             - Argument parsing and validation
├── executor.h/c           - Command execution
├── history.h/c            - Command history with persistence
├── autocomplete.h/c       - Prefix-based completion
└── commands/
    ├── commands.h         - Built-in command declarations
    ├── builtin.c          - Command registration
    ├── cmd_help.c         - Help command
    ├── cmd_status.c       - Status command
    ├── cmd_quit.c         - Quit/exit command
    ├── cmd_clear.c        - Clear screen command
    └── cmd_log.c          - Logging configuration command
```

## Quick Start

### Basic Usage

```c
#include "commands/command_system.h"
#include "terminal/ui_feedback.h"

int main(void) {
    // Initialize
    logger_init("game.log", LOG_LEVEL_INFO);
    command_system_init();

    // Main loop
    bool running = true;
    while (running) {
        CommandResult result = command_system_process_input("necromancer> ");
        ui_feedback_command_result(&result);

        if (result.should_exit) {
            running = false;
        }

        command_result_destroy(&result);
    }

    // Cleanup
    command_system_shutdown();
    logger_shutdown();
    return 0;
}
```

### Registering Custom Commands

```c
// Define command function
CommandResult cmd_summon(ParsedCommand* cmd) {
    const char* creature = parsed_command_get_arg(cmd, 0);

    if (!creature) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                                   "Must specify creature to summon");
    }

    char msg[256];
    snprintf(msg, sizeof(msg), "Summoning %s...", creature);
    return command_result_success(msg);
}

// Register command
void register_game_commands(void) {
    static FlagDefinition summon_flags[] = {
        {
            .name = "level",
            .short_name = 'l',
            .type = ARG_TYPE_INT,
            .required = false,
            .description = "Creature level"
        }
    };

    CommandInfo summon = {
        .name = "summon",
        .description = "Summon an undead creature",
        .usage = "summon <creature> [--level <n>]",
        .help_text = "Summons the specified undead creature to serve you.",
        .function = cmd_summon,
        .flags = summon_flags,
        .flag_count = 1,
        .min_args = 1,
        .max_args = 1,
        .hidden = false
    };

    command_system_register_command(&summon);
}
```

## Built-in Commands

### help [command]
Display help information. Without arguments, lists all commands. With a command name, shows detailed help.

**Example:**
```
necromancer> help
necromancer> help status
```

### status [--verbose]
Show game state and system information. Use `--verbose` or `-v` for detailed output.

**Example:**
```
necromancer> status
necromancer> status --verbose
```

### quit / exit
Exit the game gracefully.

**Example:**
```
necromancer> quit
```

### clear
Clear the terminal screen.

**Example:**
```
necromancer> clear
```

### log <level> [--file <path>]
Manage logging settings. Set log level and optionally change log file.

**Levels:** trace, debug, info, warn, error, fatal

**Example:**
```
necromancer> log
necromancer> log debug
necromancer> log info --file game.log
```

## Component Details

### Tokenizer

Handles complex string parsing:
- Single quotes: `'literal string'` - no escape processing
- Double quotes: `"string with \n escapes"` - processes escapes
- Escape sequences: `\n`, `\t`, `\r`, `\\`, `\"`, `\'`, `\0`
- Whitespace handling: multiple spaces/tabs treated as single separator

```c
Token* tokens = NULL;
size_t count = 0;
TokenizeResult result = tokenize("echo \"hello\\nworld\"", &tokens, &count);
free_tokens(tokens, count);
```

### Command Registry

Stores command metadata for validation and help:
- O(1) lookup using hash table
- Supports command aliasing (multiple names → same function)
- Flag definitions with types and validation
- Min/max argument counts
- Rich help text

```c
CommandRegistry* reg = command_registry_create();
const CommandInfo* info = command_registry_get(reg, "help");
command_registry_destroy(reg);
```

### Argument Parser

Parses and validates command arguments:
- Flag parsing: `--flag value`, `-f value`
- Boolean flags: `--verbose` (no value needed)
- Type validation: string, int, float, bool
- Required flag checking
- Argument count validation

```c
ParsedCommand* cmd = NULL;
ParseResult result = parse_command_string("status --verbose", registry, &cmd);
const ArgumentValue* verbose = parsed_command_get_flag(cmd, "verbose");
parsed_command_destroy(cmd);
```

### Command History

Circular buffer with file persistence:
- Capacity: 100 commands
- Duplicate filtering (consecutive only)
- File: `~/.necromancers_shell_history`
- Search functionality
- Navigation support (up/down arrows - future)

```c
CommandHistory* history = command_history_create(100);
command_history_add(history, "help");
const char* last = command_history_get(history, 0);
command_history_save(history, "~/.history");
command_history_destroy(history);
```

### Autocomplete

Trie-based prefix completion:
- Command name completion
- Flag name completion (with `--` prefix)
- Context-aware suggestions
- Custom entry support

```c
Autocomplete* ac = autocomplete_create(registry);
char** completions = NULL;
size_t count = 0;
autocomplete_get_completions(ac, "he", &completions, &count);
// completions = ["help"]
autocomplete_free_completions(completions, count);
autocomplete_destroy(ac);
```

### Input Handler

Integrated input handling:
- Line reading with editing
- History integration
- Autocomplete integration
- Automatic history persistence

```c
InputHandler* handler = input_handler_create(registry);
CommandResult result = input_handler_read_and_execute(handler, "> ");
command_result_destroy(&result);
input_handler_destroy(handler);
```

### UI Feedback

Formatted output with ANSI colors:
- Success messages (green)
- Error messages (red)
- Warning messages (yellow)
- Info messages (blue)
- Command results
- Autocomplete display
- History search display

```c
ui_feedback_init();
ui_feedback_success("Command executed successfully");
ui_feedback_error("Invalid argument");
ui_feedback_command_result(&result);
ui_feedback_shutdown();
```

## Testing

Run the test suite:

```bash
make test
```

Individual test files:
- `test_tokenizer.c` - Tokenization tests
- `test_trie.c` - Trie data structure tests
- `test_history.c` - Command history tests

Add more tests in `tests/` directory following the existing pattern.

## Memory Management

All components follow strict memory management:
- Every `create` has a corresponding `destroy`
- All `malloc` calls are checked for NULL
- All allocated strings are properly freed
- Test with valgrind: `make valgrind`

## Error Handling

Comprehensive error reporting:
- Tokenization errors: unclosed quotes, invalid escapes
- Parse errors: unknown commands, invalid flags, wrong arg counts
- Execution errors: command failures, permission denied
- Human-readable error messages throughout

## Performance

- Command lookup: O(1) via hash table
- Autocomplete: O(k + m) where k = prefix length, m = matches
- History access: O(1) for recent commands
- Tokenization: O(n) linear scan

## Future Enhancements (Phase 2+)

Planned improvements:
- [ ] Raw terminal mode for true line editing
- [ ] Up/down arrow history navigation
- [ ] Ctrl+R history search
- [ ] Tab autocomplete with cycling
- [ ] Command aliases
- [ ] Command piping (|)
- [ ] Variable substitution ($var)
- [ ] Inline help hints
- [ ] Command suggestions on typos

## Integration with Game Systems

The command system integrates with:
- **Logger**: All operations are logged
- **State Manager**: Status command shows game state
- **Event System**: Commands can trigger game events (future)
- **Game World**: Commands can interact with entities (future)

External references (set these in main.c):
```c
extern StateManager* g_state_manager;  // For status command
extern CommandRegistry* g_command_registry;  // For help command
```

## File Structure

```
src/commands/
├── command_system.h/c     - Top-level API (2 files)
├── tokenizer.h/c          - Tokenization (2 files)
├── registry.h/c           - Command registry (2 files)
├── parser.h/c             - Argument parsing (2 files)
├── executor.h/c           - Command execution (2 files)
├── history.h/c            - Command history (2 files)
├── autocomplete.h/c       - Autocomplete (2 files)
├── example_integration.c  - Usage examples (1 file)
├── README.md              - This file
└── commands/
    ├── commands.h         - Command declarations (1 file)
    ├── builtin.c          - Registration (1 file)
    ├── cmd_help.c         - Help command (1 file)
    ├── cmd_status.c       - Status command (1 file)
    ├── cmd_quit.c         - Quit command (1 file)
    ├── cmd_clear.c        - Clear command (1 file)
    └── cmd_log.c          - Log command (1 file)

src/terminal/
├── input_handler.h/c      - Input handling (2 files)
└── ui_feedback.h/c        - UI feedback (2 files)

src/utils/
└── trie.h/c               - Trie structure (2 files)

tests/
├── test_tokenizer.c       - Tokenizer tests (1 file)
├── test_trie.c            - Trie tests (1 file)
└── test_history.c         - History tests (1 file)

Total: 29 files
```

## Dependencies

Required:
- `hash_table.h/c` - Existing hash table implementation
- `logger.h/c` - Existing logger
- `state_manager.h` - For status command
- `timing.h` - For status command

Standard libraries:
- `stdio.h`, `stdlib.h`, `string.h`, `ctype.h`
- `unistd.h`, `termios.h` - For terminal control
- `sys/stat.h`, `pwd.h` - For file operations

## License

Part of Necromancer's Shell - see project LICENSE file.

## Contributors

Phase 1 implementation by Claude Code.

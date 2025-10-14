# Built-in Commands Documentation

Comprehensive documentation for all built-in commands in Necromancer's Shell.

**Version:** 1.0
**Last Updated:** 2025-10-13
**Source Directory:** `/home/stripcheese/Necromancers Shell/necromancers_shell/src/commands/commands/`

---

## Table of Contents

1. [Overview](#1-overview)
2. [Command Structure](#2-command-structure)
   - [CommandInfo Structure](#commandinfo-structure)
   - [CommandResult Structure](#commandresult-structure)
   - [ParsedCommand Structure](#parsedcommand-structure)
   - [FlagDefinition Structure](#flagdefinition-structure)
3. [Help Command](#3-help-command)
4. [Status Command](#4-status-command)
5. [Quit/Exit Command](#5-quitexit-command)
6. [Clear Command](#6-clear-command)
7. [Log Command](#7-log-command)
8. [Registration System](#8-registration-system)
9. [Command Patterns](#9-command-patterns)
10. [Adding New Commands](#10-adding-new-commands)
11. [Command Testing](#11-command-testing)
12. [Future Commands](#12-future-commands)

---

## 1. Overview

Built-in commands provide core functionality for Necromancer's Shell, including help, system status, logging control, and shell management. These commands are registered at startup and are immediately available to users.

### Purpose

Built-in commands serve several key purposes:

- **User Guidance**: The `help` command provides comprehensive documentation
- **System Monitoring**: The `status` command shows game and system state
- **Configuration**: The `log` command controls logging behavior
- **Shell Control**: Commands like `clear`, `quit`, and `exit` manage the shell itself

### Command Architecture

The command system uses a registry-based architecture where:

1. Commands are defined using `CommandInfo` structures
2. Each command has an associated execution function
3. Commands are registered with the global `CommandRegistry`
4. The parser validates input against command metadata
5. The executor calls command functions with parsed arguments

### Available Built-in Commands

According to `src/commands/commands/commands.h:7-16`:

```c
/**
 * Built-in Commands
 *
 * Standard commands available in Necromancer's Shell:
 * - help: Display help information
 * - status: Show game state and statistics
 * - quit/exit: Exit the game
 * - clear: Clear the terminal screen
 * - log: Manage logging settings
 */
```

---

## 2. Command Structure

### CommandInfo Structure

Defined in `src/commands/registry.h:45-56`:

```c
/* Command metadata and registration info */
typedef struct {
    const char* name;                /* Command name (e.g., "help") */
    const char* description;         /* Short description */
    const char* usage;               /* Usage string (e.g., "help [command]") */
    const char* help_text;           /* Detailed help text */
    CommandFunction function;        /* Function to execute */
    FlagDefinition* flags;           /* Array of flag definitions */
    size_t flag_count;               /* Number of flags */
    size_t min_args;                 /* Minimum positional arguments */
    size_t max_args;                 /* Maximum positional arguments (0 = unlimited) */
    bool hidden;                     /* Hide from help listing */
} CommandInfo;
```

**Field Descriptions:**

- **name**: Unique identifier for the command (used for lookup and execution)
- **description**: Brief one-line description shown in help listings
- **usage**: Usage pattern showing syntax (e.g., `"help [command]"`)
- **help_text**: Detailed multi-line help text explaining the command
- **function**: Function pointer with signature `CommandResult (*)(ParsedCommand*)`
- **flags**: Array of flag definitions (options like `--verbose`)
- **flag_count**: Number of flags in the flags array
- **min_args**: Minimum number of required positional arguments
- **max_args**: Maximum allowed positional arguments (0 means unlimited)
- **hidden**: If true, command won't appear in help listings

### CommandResult Structure

Defined in `src/commands/executor.h:36-43`:

```c
/* Command execution result */
typedef struct CommandResult {
    ExecutionStatus status;   /* Execution status code */
    bool success;             /* Whether command succeeded */
    char* output;             /* Command output text (may be NULL) */
    char* error_message;      /* Error message (may be NULL) */
    int exit_code;            /* Exit code (0 = success) */
    bool should_exit;         /* Whether game should exit */
} CommandResult;
```

**Field Descriptions:**

- **status**: Execution status from `ExecutionStatus` enum
- **success**: Boolean indicating overall success/failure
- **output**: Text output to display to user (allocated string, may be NULL)
- **error_message**: Error description if command failed (allocated string, may be NULL)
- **exit_code**: Numeric exit code (0 for success, non-zero for errors)
- **should_exit**: Flag indicating shell should exit (used by quit/exit commands)

**ExecutionStatus Enum** (`src/commands/executor.h:26-33`):

```c
typedef enum {
    EXEC_SUCCESS = 0,
    EXEC_ERROR_COMMAND_FAILED,
    EXEC_ERROR_INVALID_COMMAND,
    EXEC_ERROR_PERMISSION_DENIED,
    EXEC_ERROR_NOT_IMPLEMENTED,
    EXEC_ERROR_INTERNAL
} ExecutionStatus;
```

### ParsedCommand Structure

Defined in `src/commands/parser.h:43-50`:

```c
/* Parsed command structure */
typedef struct ParsedCommand {
    const char* command_name;    /* Command name */
    const CommandInfo* info;     /* Command info from registry */
    HashTable* flags;            /* Flag name -> ArgumentValue* */
    char** args;                 /* Positional arguments array */
    size_t arg_count;            /* Number of positional arguments */
    char* raw_input;             /* Original input string */
} ParsedCommand;
```

**Field Descriptions:**

- **command_name**: Name of the command being executed
- **info**: Pointer to CommandInfo from registry (contains metadata)
- **flags**: Hash table mapping flag names to their ArgumentValue structures
- **args**: Array of positional argument strings
- **arg_count**: Number of positional arguments in args array
- **raw_input**: Original unprocessed input string (for logging/debugging)

### FlagDefinition Structure

Defined in `src/commands/registry.h:36-42`:

```c
/* Flag/option definition */
typedef struct {
    const char* name;        /* Flag name (e.g., "verbose") */
    char short_name;         /* Short name (e.g., 'v'), 0 if none */
    ArgumentType type;       /* Expected argument type */
    bool required;           /* Whether flag is required */
    const char* description; /* Help text for this flag */
} FlagDefinition;
```

**ArgumentType Enum** (`src/commands/registry.h:28-33`):

```c
typedef enum {
    ARG_TYPE_STRING,
    ARG_TYPE_INT,
    ARG_TYPE_FLOAT,
    ARG_TYPE_BOOL
} ArgumentType;
```

---

## 3. Help Command

**File:** `src/commands/commands/cmd_help.c`
**Header:** `src/commands/commands/commands.h:18-25`

### Purpose

The `help` command provides comprehensive help information for all commands or detailed information about a specific command.

### Declaration

From `commands.h:18-25`:

```c
/**
 * Help Command
 * Usage: help [command]
 *
 * Displays help information. Without arguments, shows all commands.
 * With argument, shows detailed help for specific command.
 */
CommandResult cmd_help(ParsedCommand* cmd);
```

### Registration

From `builtin.c:10-29`:

```c
/* Help command */
{
    CommandInfo info = {
        .name = "help",
        .description = "Display help information",
        .usage = "help [command]",
        .help_text = "Shows help for all commands or a specific command.\n"
                    "Without arguments, lists all available commands.\n"
                    "With a command name, shows detailed help for that command.",
        .function = cmd_help,
        .flags = NULL,
        .flag_count = 0,
        .min_args = 0,
        .max_args = 1,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

**Key Points:**
- No flags defined
- Accepts 0 or 1 positional arguments
- Not hidden (appears in help listings)

### Implementation Details

#### Entry Point

From `cmd_help.c:13-16`:

```c
CommandResult cmd_help(ParsedCommand* cmd) {
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }
```

The function validates the input command pointer and returns an error if NULL.

#### Output Buffer Creation

From `cmd_help.c:18-24`:

```c
/* Buffer for building help output */
char* output = NULL;
size_t output_size = 0;
FILE* stream = open_memstream(&output, &output_size);
if (!stream) {
    return command_result_error(EXEC_ERROR_INTERNAL, "Failed to create output buffer");
}
```

Uses POSIX `open_memstream()` to create an in-memory stream for building output. This allows using `fprintf()` while automatically managing buffer allocation.

#### Command-Specific Help

When a command name is provided (`cmd_help.c:26-94`):

```c
/* Check if specific command was requested */
const char* cmd_name = parsed_command_get_arg(cmd, 0);

if (cmd_name) {
    /* Show help for specific command */
    const CommandInfo* info = command_registry_get(g_command_registry, cmd_name);
    if (!info) {
        fclose(stream);
        free(output);
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Unknown command: %s", cmd_name);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
    }
```

Process:
1. Get the first positional argument (command name)
2. Look up the command in the global registry
3. Return error if command doesn't exist
4. Format and display detailed help information

**Help Format** (`cmd_help.c:40-94`):

1. **Header**: `=== command_name ===`
2. **Description**: Brief description
3. **Usage**: Usage pattern
4. **Help Text**: Detailed explanation
5. **Options**: List of flags with types and descriptions
6. **Arguments**: Min/max argument requirements

Flag formatting logic (`cmd_help.c:48-84`):

```c
if (info->flag_count > 0) {
    fprintf(stream, "Options:\n");
    for (size_t i = 0; i < info->flag_count; i++) {
        const FlagDefinition* flag = &info->flags[i];
        fprintf(stream, "  ");
        if (flag->short_name) {
            fprintf(stream, "-%c, ", flag->short_name);
        }
        fprintf(stream, "--%s", flag->name);

        /* Show type */
        switch (flag->type) {
            case ARG_TYPE_STRING:
                fprintf(stream, " <string>");
                break;
            case ARG_TYPE_INT:
                fprintf(stream, " <int>");
                break;
            case ARG_TYPE_FLOAT:
                fprintf(stream, " <float>");
                break;
            case ARG_TYPE_BOOL:
                /* Boolean flags don't need value */
                break;
        }

        if (flag->required) {
            fprintf(stream, " (required)");
        }
        fprintf(stream, "\n");

        if (flag->description) {
            fprintf(stream, "      %s\n", flag->description);
        }
    }
    fprintf(stream, "\n");
}
```

#### General Help Listing

When no command is specified (`cmd_help.c:95-128`):

```c
/* Show general help with all commands */
fprintf(stream, "\n=== Necromancer's Shell - Command Help ===\n\n");
fprintf(stream, "Available commands:\n\n");

/* Get all command names */
char** names = NULL;
size_t count = 0;
if (command_registry_get_all_names(g_command_registry, &names, &count)) {
    /* Sort names alphabetically for better presentation */
    for (size_t i = 0; i < count - 1; i++) {
        for (size_t j = i + 1; j < count; j++) {
            if (strcmp(names[i], names[j]) > 0) {
                char* temp = names[i];
                names[i] = names[j];
                names[j] = temp;
            }
        }
    }

    /* Print each command */
    for (size_t i = 0; i < count; i++) {
        const CommandInfo* info = command_registry_get(g_command_registry, names[i]);
        if (info && !info->hidden) {
            fprintf(stream, "  %-12s - %s\n", info->name,
                   info->description ? info->description : "No description");
        }
    }

    command_registry_free_names(names, count);
}

fprintf(stream, "\nType 'help <command>' for detailed information on a specific command.\n");
```

Process:
1. Fetch all command names from registry
2. Sort names alphabetically (simple bubble sort)
3. Display each non-hidden command with description
4. Clean up allocated memory

#### Result Construction

From `cmd_help.c:130-134`:

```c
fclose(stream);
CommandResult result = command_result_success(output);
free(output);
return result;
```

Close the memory stream, create success result (which copies the output), then free the temporary buffer.

### Usage Examples

**Example 1: General Help**

```
> help

=== Necromancer's Shell - Command Help ===

Available commands:

  clear        - Clear the terminal screen
  exit         - Exit the game
  help         - Display help information
  log          - Manage logging settings
  quit         - Exit the game
  status       - Show game state and statistics

Type 'help <command>' for detailed information on a specific command.
```

**Example 2: Specific Command Help**

```
> help status

=== status ===

Description: Show game state and statistics

Usage: status [--verbose]

Displays current game state, player stats, and system information.
Use --verbose or -v for detailed information.

Options:
  -v, --verbose
      Show detailed status information

Arguments:
  Minimum: 0
  Maximum: 0
```

**Example 3: Unknown Command**

```
> help foobar
Error: Unknown command: foobar
```

### Code Walkthrough

1. **Validation** (line 14-16): Check for NULL command pointer
2. **Buffer Setup** (line 18-24): Create memory stream for output building
3. **Argument Check** (line 27): Get optional command name argument
4. **Specific Help** (line 29-94): If command name provided, show detailed help
   - Lookup command in registry (line 31)
   - Error if not found (line 32-38)
   - Format header, description, usage (line 40-42)
   - Display help text if available (line 44-46)
   - Format options/flags (line 48-84)
   - Show argument requirements (line 86-94)
5. **General Help** (line 95-128): If no argument, show all commands
   - Fetch all command names (line 100-103)
   - Sort alphabetically (line 105-113)
   - Print formatted list (line 116-122)
   - Free allocated memory (line 124)
6. **Cleanup** (line 130-134): Close stream, create result, return

### Memory Management

- Input: `ParsedCommand*` is managed by caller
- Output: Memory stream (`output`) is allocated by `open_memstream()` and freed after result creation
- Registry names: Allocated by `command_registry_get_all_names()`, freed with `command_registry_free_names()`
- Result: `command_result_success()` copies the output string internally

---

## 4. Status Command

**File:** `src/commands/commands/cmd_status.c`
**Header:** `src/commands/commands/commands.h:27-33`

### Purpose

The `status` command displays current game state, player statistics, and system information. It provides both basic and verbose output modes.

### Declaration

From `commands.h:27-33`:

```c
/**
 * Status Command
 * Usage: status [--verbose]
 *
 * Shows current game state, player stats, and system information.
 */
CommandResult cmd_status(ParsedCommand* cmd);
```

### Registration

From `builtin.c:31-59`:

```c
/* Status command */
{
    static FlagDefinition status_flags[] = {
        {
            .name = "verbose",
            .short_name = 'v',
            .type = ARG_TYPE_BOOL,
            .required = false,
            .description = "Show detailed status information"
        }
    };

    CommandInfo info = {
        .name = "status",
        .description = "Show game state and statistics",
        .usage = "status [--verbose]",
        .help_text = "Displays current game state, player stats, and system information.\n"
                    "Use --verbose or -v for detailed information.",
        .function = cmd_status,
        .flags = status_flags,
        .flag_count = 1,
        .min_args = 0,
        .max_args = 0,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

**Key Points:**
- Has one flag: `--verbose` / `-v` (boolean, not required)
- Accepts no positional arguments (min=0, max=0)
- Not hidden (appears in help listings)

### Implementation Details

#### Entry Point

From `cmd_status.c:15-20`:

```c
CommandResult cmd_status(ParsedCommand* cmd) {
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }

    bool verbose = parsed_command_has_flag(cmd, "verbose");
```

Validates input and checks for verbose flag.

#### Output Buffer Creation

From `cmd_status.c:22-28`:

```c
/* Buffer for building status output */
char* output = NULL;
size_t output_size = 0;
FILE* stream = open_memstream(&output, &output_size);
if (!stream) {
    return command_result_error(EXEC_ERROR_INTERNAL, "Failed to create output buffer");
}
```

Uses POSIX `open_memstream()` for dynamic output buffer.

#### Game State Display

From `cmd_status.c:30-41`:

```c
fprintf(stream, "\n=== Necromancer's Shell - Status ===\n\n");

/* Game State */
if (g_state_manager) {
    GameState current = state_manager_current(g_state_manager);
    fprintf(stream, "Game State: %s\n", state_manager_state_name(current));
    fprintf(stream, "State Depth: %zu\n", state_manager_depth(g_state_manager));
} else {
    fprintf(stream, "Game State: Unknown (state manager not initialized)\n");
}

fprintf(stream, "\n");
```

Displays:
- Current game state (from state manager)
- State stack depth (number of pushed states)
- Handles case where state manager is not initialized

#### System Time Information

From `cmd_status.c:43-54`:

```c
/* System Time */
time_t now = time(NULL);
struct tm* timeinfo = localtime(&now);
char time_str[64];
strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
fprintf(stream, "System Time: %s\n", time_str);

/* Uptime (if timing system available) */
struct timeval tv;
gettimeofday(&tv, NULL);
double uptime = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
fprintf(stream, "System Uptime: %.2f seconds\n", uptime);
```

Displays:
- Current system time formatted as `YYYY-MM-DD HH:MM:SS`
- System uptime in seconds (from `gettimeofday()`)

Note: The uptime calculation here shows time since Unix epoch, not process uptime. This appears to be a placeholder implementation.

#### Verbose Information

From `cmd_status.c:56-71`:

```c
if (verbose) {
    fprintf(stream, "\n--- Verbose Information ---\n\n");

    /* Memory information (placeholder for now) */
    fprintf(stream, "Memory:\n");
    fprintf(stream, "  Total Allocated: N/A (tracking not yet implemented)\n");
    fprintf(stream, "  Active Allocations: N/A\n");
    fprintf(stream, "\n");

    /* Command system info */
    fprintf(stream, "Command System:\n");
    fprintf(stream, "  Status: Active\n");
    fprintf(stream, "  History: Enabled\n");
    fprintf(stream, "  Autocomplete: Enabled\n");
    fprintf(stream, "\n");
}
```

When `--verbose` flag is set, displays:
- **Memory Information**: Currently placeholder (not yet implemented)
- **Command System**: Status indicators for various subsystems

#### Footer and Result

From `cmd_status.c:73-78`:

```c
fprintf(stream, "Type 'status --verbose' for detailed information.\n");

fclose(stream);
CommandResult result = command_result_success(output);
free(output);
return result;
```

### Usage Examples

**Example 1: Basic Status**

```
> status

=== Necromancer's Shell - Status ===

Game State: MAIN_MENU
State Depth: 1

System Time: 2025-10-13 14:30:45
System Uptime: 1234567.89 seconds
Type 'status --verbose' for detailed information.
```

**Example 2: Verbose Status (using long form)**

```
> status --verbose

=== Necromancer's Shell - Status ===

Game State: IN_GAME
State Depth: 2

System Time: 2025-10-13 14:35:22
System Uptime: 1234872.34 seconds

--- Verbose Information ---

Memory:
  Total Allocated: N/A (tracking not yet implemented)
  Active Allocations: N/A

Command System:
  Status: Active
  History: Enabled
  Autocomplete: Enabled

Type 'status --verbose' for detailed information.
```

**Example 3: Verbose Status (using short form)**

```
> status -v

[Same output as --verbose]
```

### Code Walkthrough

1. **Validation** (line 16-18): Check for NULL command pointer
2. **Flag Check** (line 20): Determine if verbose mode is enabled
3. **Buffer Setup** (line 22-28): Create memory stream for output
4. **Header** (line 30): Print status header
5. **Game State** (line 32-41): Display current state and depth
   - Check if state manager is available
   - Query current state and depth
   - Handle uninitialized case
6. **System Time** (line 43-48): Format and display current time
7. **Uptime** (line 50-54): Calculate and display uptime
8. **Verbose Mode** (line 56-71): If verbose flag set
   - Display memory information (placeholder)
   - Display command system status
9. **Footer** (line 73): Print hint about verbose mode
10. **Cleanup** (line 75-78): Close stream, create result, return

### Dependencies

- `../../core/state_manager.h`: For game state information
- `../../core/timing.h`: For timing-related functions
- POSIX functions: `time()`, `localtime()`, `strftime()`, `gettimeofday()`

### External References

- `g_state_manager`: Global state manager instance (may be NULL)

### Future Enhancements

The code contains placeholders for future features:
- Memory tracking and allocation statistics
- More detailed command system information
- Proper process uptime calculation

---

## 5. Quit/Exit Command

**File:** `src/commands/commands/cmd_quit.c`
**Header:** `src/commands/commands/commands.h:35-41`

### Purpose

The `quit` and `exit` commands provide a graceful way to exit the shell. Both commands are aliases that call the same implementation function.

### Declaration

From `commands.h:35-41`:

```c
/**
 * Quit Command
 * Usage: quit
 *
 * Exits the game gracefully.
 */
CommandResult cmd_quit(ParsedCommand* cmd);
```

Note: Although only `cmd_quit` is declared in the header, both `quit` and `exit` are registered as separate commands that use the same function.

### Registration

#### Quit Command

From `builtin.c:61-78`:

```c
/* Quit command */
{
    CommandInfo info = {
        .name = "quit",
        .description = "Exit the game",
        .usage = "quit",
        .help_text = "Exits the game gracefully, saving any necessary state.",
        .function = cmd_quit,
        .flags = NULL,
        .flag_count = 0,
        .min_args = 0,
        .max_args = 0,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

#### Exit Command (Alias)

From `builtin.c:80-97`:

```c
/* Exit command (alias for quit) */
{
    CommandInfo info = {
        .name = "exit",
        .description = "Exit the game",
        .usage = "exit",
        .help_text = "Exits the game gracefully, saving any necessary state.",
        .function = cmd_quit,
        .flags = NULL,
        .flag_count = 0,
        .min_args = 0,
        .max_args = 0,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

**Key Points:**
- Both commands have identical configuration except for name
- Both use the same `cmd_quit` function
- No flags, no arguments
- Not hidden (both appear in help)

### Implementation Details

The implementation is extremely simple (`cmd_quit.c:1-11`):

```c
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>

CommandResult cmd_quit(ParsedCommand* cmd) {
    (void)cmd; /* Unused */

    const char* farewell = "\nFarewell, Necromancer. The shadows await your return...\n";
    return command_result_exit(farewell);
}
```

#### Implementation Analysis

1. **Line 6**: `(void)cmd;` - Explicitly marks the parameter as unused (suppresses compiler warnings)
2. **Line 8**: Defines a farewell message with thematic flavor text
3. **Line 9**: Returns a special exit result using `command_result_exit()`

#### The Exit Result Function

The `command_result_exit()` function is defined in the executor module (`src/commands/executor.h:71-76`):

```c
/**
 * Create exit result (success + should_exit flag)
 *
 * @param output Output text (copied, may be NULL)
 * @return CommandResult with should_exit = true
 */
CommandResult command_result_exit(const char* output);
```

This function creates a CommandResult with:
- `success = true`
- `should_exit = true`
- `output` set to the farewell message
- `status = EXEC_SUCCESS`

The `should_exit` flag signals to the main loop that the shell should terminate gracefully.

### Usage Examples

**Example 1: Using quit**

```
> quit

Farewell, Necromancer. The shadows await your return...

[Shell exits]
```

**Example 2: Using exit**

```
> exit

Farewell, Necromancer. The shadows await your return...

[Shell exits]
```

**Example 3: Both commands in help**

```
> help

=== Necromancer's Shell - Command Help ===

Available commands:

  clear        - Clear the terminal screen
  exit         - Exit the game
  help         - Display help information
  log          - Manage logging settings
  quit         - Exit the game
  status       - Show game state and statistics
```

### Graceful Shutdown

The quit/exit commands provide a graceful shutdown mechanism:

1. **User Issues Command**: User types `quit` or `exit`
2. **Command Execution**: `cmd_quit()` is called
3. **Exit Result Created**: Function returns with `should_exit = true`
4. **Message Display**: Farewell message is displayed to user
5. **Main Loop Check**: Main loop checks `result.should_exit` flag
6. **Cleanup**: Main loop performs cleanup operations
7. **Termination**: Shell exits

This design allows:
- Displaying a message before exit
- Clean resource deallocation
- Saving state if needed (handled by main loop)
- Avoiding abrupt termination

### Code Walkthrough

The implementation is straightforward:

1. **Line 1-3**: Include necessary headers
2. **Line 5**: Function definition taking ParsedCommand pointer
3. **Line 6**: Mark cmd parameter as unused (no arguments needed)
4. **Line 8**: Define farewell message string
5. **Line 9**: Create and return exit result with message

### Design Rationale

**Why Two Commands?**

Both `quit` and `exit` are provided because:
- Different users have different preferences
- `exit` is common in Unix shells
- `quit` is common in interactive applications
- Providing both improves user experience

**Why So Simple?**

The implementation is minimal because:
- No state needs to be saved (handled by main loop)
- No confirmation is needed (could be added later)
- The command's sole purpose is signaling exit intent
- Actual cleanup happens outside command execution

**Thematic Message**

The farewell message adds flavor to the game:
- Reinforces the necromancer theme
- Provides closure to the session
- Makes the exit feel intentional rather than abrupt

### Future Enhancements

Potential future additions:
- `--force` flag to skip confirmation dialogs
- Unsaved changes warning
- Save state before exit
- Confirmation prompt for certain conditions

---

## 6. Clear Command

**File:** `src/commands/commands/cmd_clear.c`
**Header:** `src/commands/commands/commands.h:43-49`

### Purpose

The `clear` command clears the terminal screen using ANSI escape codes. It provides a clean viewing area for the user.

### Declaration

From `commands.h:43-49`:

```c
/**
 * Clear Command
 * Usage: clear
 *
 * Clears the terminal screen.
 */
CommandResult cmd_clear(ParsedCommand* cmd);
```

### Registration

From `builtin.c:99-116`:

```c
/* Clear command */
{
    CommandInfo info = {
        .name = "clear",
        .description = "Clear the terminal screen",
        .usage = "clear",
        .help_text = "Clears the terminal screen using ANSI escape codes.",
        .function = cmd_clear,
        .flags = NULL,
        .flag_count = 0,
        .min_args = 0,
        .max_args = 0,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

**Key Points:**
- No flags, no arguments
- Simple description mentions ANSI escape codes
- Not hidden

### Implementation Details

Complete implementation (`cmd_clear.c:1-20`):

```c
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

CommandResult cmd_clear(ParsedCommand* cmd) {
    (void)cmd; /* Unused */

    /* Use ANSI escape codes to clear screen */
    /* \033[2J clears screen, \033[H moves cursor to home */
    if (isatty(STDOUT_FILENO)) {
        printf("\033[2J\033[H");
        fflush(stdout);
        return command_result_success(NULL);
    } else {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                                   "Cannot clear: not a terminal");
    }
}
```

#### ANSI Escape Codes

From `cmd_clear.c:9-10`:

```c
/* Use ANSI escape codes to clear screen */
/* \033[2J clears screen, \033[H moves cursor to home */
```

The command uses two ANSI escape sequences:

1. **`\033[2J`**: Clear entire screen
   - `\033` is the escape character (ESC)
   - `[2J` is the command to clear entire display

2. **`\033[H`**: Move cursor to home position (1,1)
   - `[H` moves cursor to top-left corner

These sequences are part of the ANSI X3.64 standard supported by most terminals.

#### Terminal Detection

From `cmd_clear.c:11-18`:

```c
if (isatty(STDOUT_FILENO)) {
    printf("\033[2J\033[H");
    fflush(stdout);
    return command_result_success(NULL);
} else {
    return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                               "Cannot clear: not a terminal");
}
```

**isatty() Check**:
- `isatty(STDOUT_FILENO)` checks if stdout is connected to a terminal
- Returns true for terminal, false for pipes/files
- Prevents attempting to clear when output is redirected

**Why This Matters**:
- Clearing makes no sense for redirected output
- ANSI codes would appear as garbage in files
- Provides clear error message for unsupported contexts

**Output Flushing**:
- `fflush(stdout)` ensures escape codes are sent immediately
- Critical for clearing to happen before next prompt

### Usage Examples

**Example 1: Normal Clear**

```
> status
[... status output ...]

> clear

[Screen is cleared, cursor at top-left]
>
```

**Example 2: Redirected Output**

```
$ ./shell > output.txt
> clear
Error: Cannot clear: not a terminal
```

**Example 3: In Pipeline**

```
$ echo "clear" | ./shell
Error: Cannot clear: not a terminal
```

### Integration with Terminal Systems

The clear command works with Necromancer's Shell's terminal system:

**NCurses Compatibility**:
- The shell uses ncurses for terminal management
- ANSI codes work alongside ncurses
- Clear command complements ncurses screen management

**When to Use Clear**:
- Clean up cluttered output
- Start fresh after many commands
- Prepare for important output
- Remove sensitive information from view

### Code Walkthrough

1. **Line 1-4**: Include headers
   - `commands.h`: Command system interfaces
   - `stdio.h`: For printf() and fflush()
   - `stdlib.h`: Standard library
   - `unistd.h`: For isatty() and STDOUT_FILENO
2. **Line 6**: Function definition
3. **Line 7**: Mark unused parameter
4. **Line 9-10**: Comments explaining ANSI codes
5. **Line 11**: Check if stdout is a terminal
6. **Line 12-14**: If terminal, send escape codes and return success
7. **Line 15-18**: If not terminal, return error with message

### ANSI Escape Code Reference

Other useful ANSI codes (not used but related):

```
\033[0J   - Clear from cursor to end of screen
\033[1J   - Clear from cursor to beginning of screen
\033[2J   - Clear entire screen (used by clear command)
\033[H    - Move cursor to home (1,1) (used by clear command)
\033[2K   - Clear entire line
\033[A    - Move cursor up one line
\033[B    - Move cursor down one line
```

### Alternative Implementations

Other ways to clear terminal (not used):

**Using tput**:
```c
system("tput clear");
```
- Requires external tput command
- More portable but slower
- Adds dependency

**Using terminfo**:
```c
setupterm(NULL, STDOUT_FILENO, NULL);
putp(clear_screen);
```
- More portable than ANSI codes
- Requires terminfo library
- More complex

**Current Implementation Benefits**:
- Fast (no external commands)
- Simple (direct escape codes)
- Works on most modern terminals
- No additional dependencies

### Error Handling

The command handles one error case:

**Non-TTY Output**:
- Detected by `isatty()` returning false
- Returns `EXEC_ERROR_COMMAND_FAILED` status
- Provides clear error message
- Prevents garbage in redirected output

### Performance

The clear command is extremely fast:
- Single printf() call
- No system calls except isatty()
- No memory allocation
- No I/O operations (beyond stdout)

---

## 7. Log Command

**File:** `src/commands/commands/cmd_log.c`
**Header:** `src/commands/commands/commands.h:51-57`

### Purpose

The `log` command manages logging configuration, allowing users to change the log level and optionally set a different log file.

### Declaration

From `commands.h:51-57`:

```c
/**
 * Log Command
 * Usage: log <level> [--file <path>]
 *
 * Manages logging settings. Can set log level and log file.
 */
CommandResult cmd_log(ParsedCommand* cmd);
```

### Registration

From `builtin.c:118-147`:

```c
/* Log command */
{
    static FlagDefinition log_flags[] = {
        {
            .name = "file",
            .short_name = 'f',
            .type = ARG_TYPE_STRING,
            .required = false,
            .description = "Set log file path"
        }
    };

    CommandInfo info = {
        .name = "log",
        .description = "Manage logging settings",
        .usage = "log <level> [--file <path>]",
        .help_text = "Changes the logging level and optionally the log file.\n"
                    "Levels: trace, debug, info, warn, error, fatal\n"
                    "Without arguments, shows current log level.",
        .function = cmd_log,
        .flags = log_flags,
        .flag_count = 1,
        .min_args = 0,
        .max_args = 1,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

**Key Points:**
- Has one flag: `--file` / `-f` (string, not required)
- Accepts 0 or 1 positional arguments (the log level)
- Help text lists valid log levels
- Without arguments, shows current level

### Implementation Details

#### Entry Point and Validation

From `cmd_log.c:7-10`:

```c
CommandResult cmd_log(ParsedCommand* cmd) {
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }
```

#### Showing Current Level

When no arguments provided (`cmd_log.c:12-28`):

```c
/* Get log level argument */
const char* level_str = parsed_command_get_arg(cmd, 0);
if (!level_str) {
    /* Show current log level */
    LogLevel current_level = logger_get_level();
    const char* level_names[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };

    char msg[256];
    snprintf(msg, sizeof(msg), "Current log level: %s\n\n"
            "Available levels: trace, debug, info, warn, error, fatal\n"
            "Usage: log <level> [--file <path>]",
            level_names[current_level]);

    return command_result_success(msg);
}
```

Process:
1. Check if level argument provided
2. If not, get current level from logger
3. Format message showing current level and available options
4. Return success with informational message

#### Log Level Parsing

From `cmd_log.c:30-51`:

```c
/* Parse log level */
LogLevel new_level;
if (strcmp(level_str, "trace") == 0) {
    new_level = LOG_LEVEL_TRACE;
} else if (strcmp(level_str, "debug") == 0) {
    new_level = LOG_LEVEL_DEBUG;
} else if (strcmp(level_str, "info") == 0) {
    new_level = LOG_LEVEL_INFO;
} else if (strcmp(level_str, "warn") == 0) {
    new_level = LOG_LEVEL_WARN;
} else if (strcmp(level_str, "error") == 0) {
    new_level = LOG_LEVEL_ERROR;
} else if (strcmp(level_str, "fatal") == 0) {
    new_level = LOG_LEVEL_FATAL;
} else {
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg),
            "Invalid log level: %s\n"
            "Valid levels: trace, debug, info, warn, error, fatal",
            level_str);
    return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
}
```

**Supported Log Levels** (from `src/utils/logger.h`):
- **trace**: Most verbose, all details
- **debug**: Debugging information
- **info**: Informational messages
- **warn**: Warning messages
- **error**: Error messages
- **fatal**: Fatal errors only

The parsing uses a series of string comparisons. Returns error if level is invalid.

#### Setting Log Level

From `cmd_log.c:53-54`:

```c
/* Set log level */
logger_set_level(new_level);
```

Calls the logger module to update the current log level.

#### Optional File Change

From `cmd_log.c:56-65`:

```c
/* Check for file flag */
const ArgumentValue* file_arg = parsed_command_get_flag(cmd, "file");
if (file_arg && file_arg->type == ARG_TYPE_STRING) {
    /* Reinitialize logger with new file */
    logger_shutdown();
    if (!logger_init(file_arg->value.str_value, new_level)) {
        return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                                   "Failed to change log file");
    }
}
```

Process:
1. Check if `--file` flag is present
2. Verify it has a string value
3. Shutdown current logger (closes current file)
4. Reinitialize logger with new file and level
5. Return error if initialization fails

**Important**: Changing the log file requires shutting down and reinitializing the entire logger system.

#### Success Result

From `cmd_log.c:67-70`:

```c
char msg[256];
snprintf(msg, sizeof(msg), "Log level set to: %s", level_str);
return command_result_success(msg);
```

### Usage Examples

**Example 1: Show Current Level**

```
> log
Current log level: INFO

Available levels: trace, debug, info, warn, error, fatal
Usage: log <level> [--file <path>]
```

**Example 2: Change Log Level**

```
> log debug
Log level set to: debug
```

**Example 3: Change Log Level and File**

```
> log trace --file /tmp/necro_debug.log
Log level set to: trace
```

**Example 4: Using Short Flag Form**

```
> log error -f errors.log
Log level set to: error
```

**Example 5: Invalid Level**

```
> log verbose
Error: Invalid log level: verbose
Valid levels: trace, debug, info, warn, error, fatal
```

### Log Level Hierarchy

Log levels form a hierarchy where setting a level shows that level and above:

```
FATAL   - Only fatal errors
ERROR   - Errors and fatal
WARN    - Warnings, errors, and fatal
INFO    - Info, warnings, errors, and fatal
DEBUG   - Debug, info, warnings, errors, and fatal
TRACE   - Everything (most verbose)
```

### Code Walkthrough

1. **Line 1-5**: Includes
2. **Line 7-10**: Entry point and validation
3. **Line 12-13**: Get level argument
4. **Line 14-28**: If no argument, show current level
   - Get current level from logger (line 16)
   - Define level name array (line 17-19)
   - Format informational message (line 21-25)
   - Return success with message (line 27)
5. **Line 30-51**: Parse log level string
   - Series of string comparisons (line 32-43)
   - Error if no match (line 44-51)
6. **Line 53-54**: Set new log level
7. **Line 56-65**: Check for file flag
   - Get flag value (line 57)
   - Shutdown and reinit logger (line 59-64)
8. **Line 67-70**: Create success result

### Dependencies

**Logger Module** (`src/utils/logger.h`):
- `logger_get_level()`: Get current log level
- `logger_set_level()`: Set log level
- `logger_shutdown()`: Shutdown logger
- `logger_init()`: Initialize logger with file and level
- `LogLevel`: Enum of log levels

### Use Cases

**Debugging**:
```
> log trace --file debug.log
[Run problematic commands]
[Check debug.log for detailed information]
> log info
```

**Production**:
```
> log warn --file /var/log/necromancer.log
```

**Testing**:
```
> log debug
[Run tests]
> log error
```

**Quiet Operation**:
```
> log fatal
[Only critical errors shown]
```

### Error Handling

The command handles several error cases:

1. **Invalid Command**: NULL pointer check
2. **Invalid Log Level**: String comparison with helpful error
3. **File Change Failure**: Logger initialization failure

### Memory Management

- Input: `ParsedCommand*` managed by caller
- Temporary buffers: Stack-allocated `char msg[256]`
- Flag values: Accessed through ParsedCommand (not freed by command)
- Logger internals: Managed by logger module

### Integration with Logger

The log command is a front-end to the logging system:

**Logger Module Responsibilities**:
- File management (opening, closing, rotation)
- Message formatting
- Level filtering
- Thread safety (if applicable)

**Command Responsibilities**:
- User interface
- Input validation
- Level string parsing
- Error reporting

This separation keeps the logger module independent of the command system.

---

## 8. Registration System

The registration system initializes and registers all built-in commands with the command registry at startup.

### Function: register_builtin_commands()

**File:** `src/commands/commands/builtin.c`
**Declaration:** `src/commands/commands/commands.h:59-65`

From `commands.h:59-65`:

```c
/**
 * Register all built-in commands
 *
 * @param registry Command registry to register commands to
 * @return Number of commands registered
 */
int register_builtin_commands(struct CommandRegistry* registry);
```

### Implementation

From `builtin.c:5-150`:

```c
int register_builtin_commands(CommandRegistry* registry) {
    if (!registry) return 0;

    int registered = 0;

    /* Help command */
    {
        CommandInfo info = {
            /* ... */
        };
        if (command_registry_register(registry, &info)) {
            registered++;
        }
    }

    /* Status command */
    {
        /* ... */
    }

    /* Quit command */
    {
        /* ... */
    }

    /* Exit command (alias for quit) */
    {
        /* ... */
    }

    /* Clear command */
    {
        /* ... */
    }

    /* Log command */
    {
        /* ... */
    }

    return registered;
}
```

### Registration Process

#### 1. Validation

From `builtin.c:6`:

```c
if (!registry) return 0;
```

Ensures registry pointer is valid before proceeding.

#### 2. Counter Initialization

From `builtin.c:8`:

```c
int registered = 0;
```

Tracks number of successfully registered commands.

#### 3. Command Blocks

Each command is registered in its own block scope (`builtin.c:10-147`):

```c
/* Command name */
{
    /* Define any static flags */
    static FlagDefinition command_flags[] = {
        /* ... */
    };

    /* Create CommandInfo structure */
    CommandInfo info = {
        .name = "command_name",
        .description = "Brief description",
        .usage = "command_name [args]",
        .help_text = "Detailed help text...",
        .function = cmd_function,
        .flags = command_flags,    /* or NULL */
        .flag_count = 1,           /* or 0 */
        .min_args = 0,
        .max_args = 1,
        .hidden = false
    };

    /* Register command */
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

**Block Scoping Benefits**:
- Encapsulates each command's configuration
- Prevents variable name conflicts
- Makes adding/removing commands easier
- Keeps static flag arrays in separate scopes

#### 4. Flag Definitions

Commands with flags define static arrays (`builtin.c:33-41`, `builtin.c:120-128`):

**Status flags example**:
```c
static FlagDefinition status_flags[] = {
    {
        .name = "verbose",
        .short_name = 'v',
        .type = ARG_TYPE_BOOL,
        .required = false,
        .description = "Show detailed status information"
    }
};
```

**Log flags example**:
```c
static FlagDefinition log_flags[] = {
    {
        .name = "file",
        .short_name = 'f',
        .type = ARG_TYPE_STRING,
        .required = false,
        .description = "Set log file path"
    }
};
```

**Why Static?**:
- Arrays must persist beyond function return
- Registry stores pointers to these arrays
- Static storage ensures lifetime

#### 5. Return Value

From `builtin.c:149`:

```c
return registered;
```

Returns total number of successfully registered commands (should be 6 for all built-in commands).

### Registration Order

Commands are registered in this order:

1. `help` - Help command
2. `status` - Status command
3. `quit` - Quit command
4. `exit` - Exit command (alias)
5. `clear` - Clear screen command
6. `log` - Log configuration command

**Order Considerations**:
- Order doesn't affect functionality (hash table lookup)
- Help command first is conceptually appropriate
- Quit/exit are adjacent (related functionality)
- Log command last (system utility)

### Integration with Main

The registration function is typically called during initialization:

```c
/* Pseudo-code from main initialization */
CommandRegistry* registry = command_registry_create();
if (!registry) {
    fprintf(stderr, "Failed to create command registry\n");
    return 1;
}

int count = register_builtin_commands(registry);
if (count < 6) {
    fprintf(stderr, "Warning: Only %d builtin commands registered\n", count);
}

/* Store in global variable for command access */
g_command_registry = registry;
```

### Error Handling

The function handles registration failures gracefully:

**Individual Command Failure**:
- If a command fails to register, it's skipped
- Other commands continue to register
- Return value indicates how many succeeded

**Registry Validation**:
- NULL registry check prevents crashes
- Returns 0 immediately if registry is NULL

**Silent Failures**:
- Individual registration failures are silent
- Caller can check return value to verify all commands registered
- Could add logging for debugging

### Memory Management

**Static Arrays**:
- Flag definitions are static arrays
- Persist for program lifetime
- No allocation/deallocation needed

**CommandInfo Structures**:
- Created on stack (temporary)
- `command_registry_register()` copies the information
- No memory leaks from temporary structures

**String Literals**:
- All strings are literals (constant storage)
- No dynamic allocation for names/descriptions
- No need to free

### Testing Registration

To verify all commands registered:

```c
int expected = 6;  /* help, status, quit, exit, clear, log */
int actual = register_builtin_commands(registry);
if (actual != expected) {
    fprintf(stderr, "Expected %d commands, got %d\n", expected, actual);
}
```

### Extending the System

To add a new built-in command:

1. Declare function in `commands.h`
2. Implement function in new file (e.g., `cmd_newcmd.c`)
3. Add registration block in `builtin.c`
4. Increment expected count in tests

Example:

```c
/* In commands.h */
CommandResult cmd_save(ParsedCommand* cmd);

/* In builtin.c, add before return statement */
/* Save command */
{
    CommandInfo info = {
        .name = "save",
        .description = "Save current game state",
        .usage = "save [filename]",
        .help_text = "Saves the current game state to a file.\n"
                    "If no filename is provided, uses default save file.",
        .function = cmd_save,
        .flags = NULL,
        .flag_count = 0,
        .min_args = 0,
        .max_args = 1,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

---

## 9. Command Patterns

This section documents common patterns used across built-in command implementations.

### Pattern 1: Input Validation

**Purpose**: Validate command input pointer before use

**Implementation**:

```c
CommandResult cmd_example(ParsedCommand* cmd) {
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }
    /* ... rest of implementation ... */
}
```

**Used By**: All commands (help, status, quit, clear, log)

**Rationale**:
- Prevents null pointer dereference
- Provides clear error message
- Consistent error handling
- Defensive programming practice

### Pattern 2: Output Buffer with Memory Stream

**Purpose**: Build dynamic output string efficiently

**Implementation**:

```c
/* Buffer for building output */
char* output = NULL;
size_t output_size = 0;
FILE* stream = open_memstream(&output, &output_size);
if (!stream) {
    return command_result_error(EXEC_ERROR_INTERNAL, "Failed to create output buffer");
}

/* Build output using fprintf */
fprintf(stream, "Header: %s\n", header);
fprintf(stream, "Value: %d\n", value);

/* Close stream and create result */
fclose(stream);
CommandResult result = command_result_success(output);
free(output);
return result;
```

**Used By**: help, status

**Benefits**:
- Automatic memory management (buffer grows as needed)
- Familiar fprintf() API
- No manual buffer size tracking
- Clean and readable code

**POSIX Requirement**:
```c
#define _POSIX_C_SOURCE 200809L
```
Must be defined before includes for `open_memstream()`.

### Pattern 3: Unused Parameter Marking

**Purpose**: Suppress compiler warnings for unused parameters

**Implementation**:

```c
CommandResult cmd_example(ParsedCommand* cmd) {
    (void)cmd; /* Unused */
    /* ... implementation that doesn't use cmd ... */
}
```

**Used By**: quit, clear

**Rationale**:
- All command functions have same signature (required by function pointer type)
- Some commands don't need to inspect the parsed command
- Explicit marking is clearer than ignoring warnings
- Standard C idiom

### Pattern 4: Flag Checking

**Purpose**: Check if boolean flag is present

**Implementation**:

```c
bool verbose = parsed_command_has_flag(cmd, "verbose");
if (verbose) {
    /* Show additional information */
}
```

**Used By**: status

**Alternative for Value Flags**:

```c
const ArgumentValue* flag_value = parsed_command_get_flag(cmd, "file");
if (flag_value && flag_value->type == ARG_TYPE_STRING) {
    const char* filename = flag_value->value.str_value;
    /* Use filename */
}
```

**Used By**: log

### Pattern 5: Positional Argument Access

**Purpose**: Get positional arguments from command

**Implementation**:

```c
const char* arg = parsed_command_get_arg(cmd, 0);
if (arg) {
    /* Argument provided */
} else {
    /* Argument not provided */
}
```

**Used By**: help, log

**Rationale**:
- Safe access (returns NULL if out of bounds)
- No need to check arg_count manually
- Simple and readable

### Pattern 6: Terminal Detection

**Purpose**: Check if output is to a terminal

**Implementation**:

```c
#include <unistd.h>

if (isatty(STDOUT_FILENO)) {
    /* Output is to terminal */
} else {
    /* Output is redirected */
    return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                               "Command requires terminal");
}
```

**Used By**: clear

**Use Cases**:
- Commands that use ANSI escape codes
- Commands requiring interactive terminal
- Commands that modify terminal state

### Pattern 7: Result Creation

**Purpose**: Create and return command results

**Success with Output**:
```c
return command_result_success("Operation completed successfully");
```

**Success without Output**:
```c
return command_result_success(NULL);
```

**Error**:
```c
return command_result_error(EXEC_ERROR_COMMAND_FAILED, "Operation failed");
```

**Exit**:
```c
return command_result_exit("Goodbye!");
```

**Used By**: All commands

### Pattern 8: Static Flag Definitions

**Purpose**: Define command flags for registration

**Implementation**:

```c
static FlagDefinition command_flags[] = {
    {
        .name = "verbose",
        .short_name = 'v',
        .type = ARG_TYPE_BOOL,
        .required = false,
        .description = "Show detailed output"
    },
    {
        .name = "file",
        .short_name = 'f',
        .type = ARG_TYPE_STRING,
        .required = false,
        .description = "Specify file path"
    }
};
```

**Used By**: Registration system (builtin.c)

**Rationale**:
- Static storage duration (survives function return)
- Registry stores pointers to these definitions
- Clear declarative style
- Type safety

### Pattern 9: String Comparison Chain

**Purpose**: Parse string argument to enum value

**Implementation**:

```c
LogLevel level;
if (strcmp(str, "trace") == 0) {
    level = LOG_LEVEL_TRACE;
} else if (strcmp(str, "debug") == 0) {
    level = LOG_LEVEL_DEBUG;
} else if (strcmp(str, "info") == 0) {
    level = LOG_LEVEL_INFO;
} else {
    return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                               "Invalid level: valid values are trace, debug, info");
}
```

**Used By**: log

**Alternative Approaches**:
- Hash table lookup (more complex, faster for many options)
- String array with loop (more maintainable, same performance)
- Switch on first character (error-prone, harder to read)

**Current Approach Benefits**:
- Simple and clear
- Easy to modify
- Good performance (short lists)
- Explicit error handling

### Pattern 10: External System References

**Purpose**: Access global game systems

**Implementation**:

```c
/* In source file */
extern StateManager* g_state_manager;
extern CommandRegistry* g_command_registry;

/* In function */
if (g_state_manager) {
    GameState state = state_manager_current(g_state_manager);
    /* Use state */
} else {
    /* Handle uninitialized case */
}
```

**Used By**: help (registry), status (state_manager)

**Rationale**:
- Commands need access to game systems
- Global variables for singleton systems
- Null checks handle initialization order
- Clear external dependencies

### Pattern 11: Error Message Formatting

**Purpose**: Create informative error messages with context

**Implementation**:

```c
char error_msg[256];
snprintf(error_msg, sizeof(error_msg),
         "Unknown command: %s", cmd_name);
return command_result_error(EXEC_ERROR_COMMAND_FAILED, error_msg);
```

**Used By**: help, log

**Best Practices**:
- Fixed-size buffer (stack allocated)
- Use snprintf (bounds checking)
- Include contextual information (what failed, why)
- Clear, user-friendly language

### Pattern 12: Optional Argument Handling

**Purpose**: Handle commands with optional arguments

**Implementation**:

```c
const char* arg = parsed_command_get_arg(cmd, 0);
if (!arg) {
    /* Show current state / help */
    return show_current_state();
}

/* Process argument */
return process_argument(arg);
```

**Used By**: help, log

**Pattern Benefits**:
- Single command does multiple related tasks
- No argument = informational mode
- With argument = action mode
- User-friendly (help available via bare command)

---

## 10. Adding New Commands

This section provides a complete guide for adding new built-in commands to Necromancer's Shell.

### Step-by-Step Process

#### Step 1: Design the Command

Before writing code, define:

1. **Command Name**: What users will type
2. **Purpose**: What the command does
3. **Arguments**: Positional arguments needed
4. **Flags**: Optional flags/options
5. **Output**: What the command displays
6. **Error Cases**: What can go wrong

**Example Design**:
```
Command: save
Purpose: Save game state to file
Arguments: [filename] (optional, default: "save.dat")
Flags: --force / -f (overwrite without confirmation)
Output: "Game saved to <filename>"
Errors: File write failure, invalid filename
```

#### Step 2: Declare Function in Header

Add declaration to `src/commands/commands/commands.h`:

```c
/**
 * Save Command
 * Usage: save [filename] [--force]
 *
 * Saves the current game state to a file.
 */
CommandResult cmd_save(ParsedCommand* cmd);
```

**Header Organization**:
- Add after existing command declarations
- Include comment block with usage
- Follow existing style

#### Step 3: Create Implementation File

Create `src/commands/commands/cmd_save.c`:

```c
/* POSIX features if needed */
#define _POSIX_C_SOURCE 200809L

#include "commands.h"
#include "../../core/state_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External references */
extern StateManager* g_state_manager;

CommandResult cmd_save(ParsedCommand* cmd) {
    /* Step 1: Validate input */
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }

    /* Step 2: Get arguments */
    const char* filename = parsed_command_get_arg(cmd, 0);
    if (!filename) {
        filename = "save.dat";  /* Default */
    }

    bool force = parsed_command_has_flag(cmd, "force");

    /* Step 3: Check if file exists (unless force) */
    if (!force) {
        FILE* test = fopen(filename, "r");
        if (test) {
            fclose(test);
            return command_result_error(EXEC_ERROR_COMMAND_FAILED,
                                       "File exists. Use --force to overwrite.");
        }
    }

    /* Step 4: Save state */
    if (!g_state_manager) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                   "State manager not initialized");
    }

    if (!state_manager_save(g_state_manager, filename)) {
        char error[256];
        snprintf(error, sizeof(error), "Failed to save to: %s", filename);
        return command_result_error(EXEC_ERROR_COMMAND_FAILED, error);
    }

    /* Step 5: Success */
    char output[256];
    snprintf(output, sizeof(output), "Game saved to: %s", filename);
    return command_result_success(output);
}
```

**Implementation Checklist**:
- [ ] Include necessary headers
- [ ] Validate input pointer
- [ ] Get arguments and flags
- [ ] Perform command operation
- [ ] Handle errors appropriately
- [ ] Return appropriate result

#### Step 4: Register Command

Add registration block to `src/commands/commands/builtin.c`:

```c
/* Save command */
{
    static FlagDefinition save_flags[] = {
        {
            .name = "force",
            .short_name = 'f',
            .type = ARG_TYPE_BOOL,
            .required = false,
            .description = "Overwrite existing file without confirmation"
        }
    };

    CommandInfo info = {
        .name = "save",
        .description = "Save game state to file",
        .usage = "save [filename] [--force]",
        .help_text = "Saves the current game state to a file.\n"
                    "Default filename is 'save.dat' if not specified.\n"
                    "Use --force to overwrite existing files.",
        .function = cmd_save,
        .flags = save_flags,
        .flag_count = 1,
        .min_args = 0,
        .max_args = 1,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

**Registration Checklist**:
- [ ] Define flag array (if needed)
- [ ] Set command name
- [ ] Set description (brief)
- [ ] Set usage pattern
- [ ] Write help text (detailed)
- [ ] Point to function
- [ ] Set flags and count
- [ ] Set min/max args
- [ ] Set hidden flag
- [ ] Add to builtin.c before return

#### Step 5: Update Build System

Add to `necromancers_shell/src/commands/commands/Makefile` or build configuration:

```makefile
COMMAND_SOURCES = \
    builtin.c \
    cmd_help.c \
    cmd_status.c \
    cmd_quit.c \
    cmd_clear.c \
    cmd_log.c \
    cmd_save.c
```

Or in CMakeLists.txt:

```cmake
set(COMMAND_SOURCES
    commands/builtin.c
    commands/cmd_help.c
    commands/cmd_status.c
    commands/cmd_quit.c
    commands/cmd_clear.c
    commands/cmd_log.c
    commands/cmd_save.c
)
```

#### Step 6: Test the Command

Create test cases:

**Manual Testing**:
```
> help save
[Verify help output]

> save
Game saved to: save.dat

> save test.dat
Game saved to: test.dat

> save test.dat
Error: File exists. Use --force to overwrite.

> save test.dat --force
Game saved to: test.dat

> save test.dat -f
Game saved to: test.dat
```

**Test Checklist**:
- [ ] Help displays correctly
- [ ] Command with no args uses defaults
- [ ] Command with args processes correctly
- [ ] Flags work (both long and short forms)
- [ ] Error cases handled properly
- [ ] Success message is clear

### Template Command

Complete template for copy-paste:

**cmd_template.c**:
```c
/* POSIX features (if needed) */
#define _POSIX_C_SOURCE 200809L

#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External references (if needed) */
// extern StateManager* g_state_manager;

CommandResult cmd_template(ParsedCommand* cmd) {
    /* Validate input */
    if (!cmd) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Invalid command");
    }

    /* Get arguments */
    const char* arg1 = parsed_command_get_arg(cmd, 0);

    /* Get flags */
    bool flag = parsed_command_has_flag(cmd, "flag");

    /* Build output */
    char* output = NULL;
    size_t output_size = 0;
    FILE* stream = open_memstream(&output, &output_size);
    if (!stream) {
        return command_result_error(EXEC_ERROR_INTERNAL,
                                   "Failed to create output buffer");
    }

    /* Command logic */
    fprintf(stream, "Command output here\n");

    /* Return result */
    fclose(stream);
    CommandResult result = command_result_success(output);
    free(output);
    return result;
}
```

**Registration template**:
```c
/* Template command */
{
    static FlagDefinition template_flags[] = {
        {
            .name = "flag",
            .short_name = 'f',
            .type = ARG_TYPE_BOOL,
            .required = false,
            .description = "Flag description"
        }
    };

    CommandInfo info = {
        .name = "template",
        .description = "Brief description",
        .usage = "template [args] [--flag]",
        .help_text = "Detailed help text.\n"
                    "Additional help lines.\n"
                    "Usage examples and notes.",
        .function = cmd_template,
        .flags = template_flags,
        .flag_count = 1,
        .min_args = 0,
        .max_args = 1,
        .hidden = false
    };
    if (command_registry_register(registry, &info)) {
        registered++;
    }
}
```

### Common Pitfalls

**Pitfall 1: Forgetting Static for Flags**
```c
/* WRONG - flags will be deallocated */
FlagDefinition flags[] = { /* ... */ };

/* RIGHT - flags persist */
static FlagDefinition flags[] = { /* ... */ };
```

**Pitfall 2: Not Checking NULL Returns**
```c
/* WRONG - may crash */
const char* arg = parsed_command_get_arg(cmd, 0);
printf("Arg: %s\n", arg);

/* RIGHT - check for NULL */
const char* arg = parsed_command_get_arg(cmd, 0);
if (arg) {
    printf("Arg: %s\n", arg);
}
```

**Pitfall 3: Memory Leaks with Streams**
```c
/* WRONG - output not freed */
FILE* stream = open_memstream(&output, &output_size);
fprintf(stream, "text\n");
fclose(stream);
return command_result_success(output);

/* RIGHT - free after copying */
fclose(stream);
CommandResult result = command_result_success(output);
free(output);
return result;
```

**Pitfall 4: Wrong Argument Counts**
```c
/* WRONG - says min=0 but requires argument */
.min_args = 0,
/* In function: assumes arg exists without checking */

/* RIGHT - match declaration to implementation */
.min_args = 1,  /* If required */
.min_args = 0,  /* If optional, check in function */
```

### Testing Checklist

Use this checklist to test new commands:

#### Functionality Tests
- [ ] Command executes without errors
- [ ] Output is correct and formatted properly
- [ ] Arguments are processed correctly
- [ ] Flags work as expected (long form)
- [ ] Flags work as expected (short form)
- [ ] Default values work when arguments omitted

#### Error Handling Tests
- [ ] Invalid arguments produce clear errors
- [ ] Missing required arguments produce errors
- [ ] Invalid flag values produce errors
- [ ] System errors are caught and reported
- [ ] NULL/empty input handled gracefully

#### Integration Tests
- [ ] Command appears in help listing
- [ ] `help <command>` shows correct information
- [ ] Command works with other commands
- [ ] No memory leaks (test with valgrind)
- [ ] No compiler warnings

#### Documentation Tests
- [ ] Usage string is accurate
- [ ] Help text is clear and complete
- [ ] Description is concise
- [ ] All flags are documented
- [ ] Examples would be clear to users

---

## 11. Command Testing

### Manual Testing

#### Basic Functionality Test

Test each command's core functionality:

```bash
# Help command
> help
> help status
> help nonexistent

# Status command
> status
> status --verbose
> status -v

# Clear command
> clear

# Log command
> log
> log debug
> log trace --file test.log
> log invalid

# Quit command
> quit

# Exit command
> exit
```

#### Error Condition Testing

Test error handling:

```bash
# Invalid arguments
> status extra_arg
> clear extra_arg
> log invalid_level
> log debug --file

# Invalid flags
> help --invalid
> status --unknown
> log --badlag
```

#### Edge Case Testing

Test boundary conditions:

```bash
# Empty/whitespace
>
>
> help

# Long arguments
> help very_long_nonexistent_command_name_that_goes_on_forever

# Special characters
> log "level with spaces"
> log level\nwith\nnewlines

# Multiple flags
> status --verbose --verbose
```

### Automated Testing

#### Unit Test Structure

Create `tests/test_commands.c`:

```c
#include "../src/commands/commands/commands.h"
#include "../src/commands/registry.h"
#include "../src/commands/parser.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

/* Test help command */
void test_help_no_args() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("help", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_help(cmd);
    assert(exec_result.success);
    assert(exec_result.output != NULL);
    assert(strstr(exec_result.output, "Available commands") != NULL);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

void test_help_with_command() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("help status", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_help(cmd);
    assert(exec_result.success);
    assert(exec_result.output != NULL);
    assert(strstr(exec_result.output, "status") != NULL);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

void test_help_unknown_command() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("help nonexistent", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_help(cmd);
    assert(!exec_result.success);
    assert(exec_result.error_message != NULL);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

/* Test status command */
void test_status_basic() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("status", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_status(cmd);
    assert(exec_result.success);
    assert(exec_result.output != NULL);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

void test_status_verbose() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("status --verbose", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_status(cmd);
    assert(exec_result.success);
    assert(exec_result.output != NULL);
    assert(strstr(exec_result.output, "Verbose Information") != NULL);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

/* Test quit command */
void test_quit() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("quit", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_quit(cmd);
    assert(exec_result.success);
    assert(exec_result.should_exit);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

/* Test exit command */
void test_exit() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("exit", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_quit(cmd);
    assert(exec_result.success);
    assert(exec_result.should_exit);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

/* Test log command */
void test_log_show_current() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("log", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_log(cmd);
    assert(exec_result.success);
    assert(exec_result.output != NULL);
    assert(strstr(exec_result.output, "Current log level") != NULL);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

void test_log_set_level() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("log debug", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_log(cmd);
    assert(exec_result.success);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

void test_log_invalid_level() {
    CommandRegistry* reg = command_registry_create();
    register_builtin_commands(reg);

    ParsedCommand* cmd = NULL;
    ParseResult result = parse_command_string("log invalid", reg, &cmd);
    assert(result == PARSE_SUCCESS);

    CommandResult exec_result = cmd_log(cmd);
    assert(!exec_result.success);

    command_result_destroy(&exec_result);
    parsed_command_destroy(cmd);
    command_registry_destroy(reg);
}

/* Test registration */
void test_register_all_commands() {
    CommandRegistry* reg = command_registry_create();

    int count = register_builtin_commands(reg);
    assert(count == 6);  /* help, status, quit, exit, clear, log */

    assert(command_registry_exists(reg, "help"));
    assert(command_registry_exists(reg, "status"));
    assert(command_registry_exists(reg, "quit"));
    assert(command_registry_exists(reg, "exit"));
    assert(command_registry_exists(reg, "clear"));
    assert(command_registry_exists(reg, "log"));

    command_registry_destroy(reg);
}

/* Main test runner */
int main() {
    printf("Running command tests...\n");

    test_help_no_args();
    test_help_with_command();
    test_help_unknown_command();

    test_status_basic();
    test_status_verbose();

    test_quit();
    test_exit();

    test_log_show_current();
    test_log_set_level();
    test_log_invalid_level();

    test_register_all_commands();

    printf("All tests passed!\n");
    return 0;
}
```

#### Running Tests

```bash
# Compile tests
gcc -o test_commands tests/test_commands.c \
    src/commands/commands/*.c \
    src/commands/*.c \
    src/utils/*.c \
    src/core/*.c \
    -I src \
    -Wall -Wextra

# Run tests
./test_commands

# Run with valgrind
valgrind --leak-check=full ./test_commands
```

### Memory Testing

Use valgrind to check for memory leaks:

```bash
# Test individual commands
echo "help" | valgrind --leak-check=full ./necromancer_shell
echo "status" | valgrind --leak-check=full ./necromancer_shell
echo "log debug" | valgrind --leak-check=full ./necromancer_shell

# Test sequence
valgrind --leak-check=full ./necromancer_shell << EOF
help
help status
status
status --verbose
log
log debug
clear
quit
EOF
```

Expected output should show:
```
All heap blocks were freed -- no leaks are possible
```

### Integration Testing

Test commands with the full shell:

**Test Script**: `tests/integration_test.sh`

```bash
#!/bin/bash

SHELL=./necromancer_shell

echo "=== Integration Test Suite ==="

# Test 1: Help command
echo "Test 1: Help command"
echo "help" | $SHELL | grep -q "Available commands"
if [ $? -eq 0 ]; then
    echo "  PASS"
else
    echo "  FAIL"
    exit 1
fi

# Test 2: Status command
echo "Test 2: Status command"
echo "status" | $SHELL | grep -q "Game State"
if [ $? -eq 0 ]; then
    echo "  PASS"
else
    echo "  FAIL"
    exit 1
fi

# Test 3: Log command
echo "Test 3: Log command"
echo "log" | $SHELL | grep -q "Current log level"
if [ $? -eq 0 ]; then
    echo "  PASS"
else
    echo "  FAIL"
    exit 1
fi

# Test 4: Clear command (check no error)
echo "Test 4: Clear command"
echo "clear" | $SHELL 2>&1 | grep -q "Error"
if [ $? -ne 0 ]; then
    echo "  PASS"
else
    echo "  FAIL"
    exit 1
fi

echo "=== All Tests Passed ==="
```

### Continuous Integration

Example GitHub Actions workflow:

```yaml
name: Command Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y valgrind

    - name: Build
      run: make

    - name: Run unit tests
      run: |
        make test
        ./test_commands

    - name: Run integration tests
      run: |
        chmod +x tests/integration_test.sh
        ./tests/integration_test.sh

    - name: Memory leak tests
      run: |
        valgrind --leak-check=full --error-exitcode=1 ./test_commands
```

### Test Coverage

Track which code paths are tested:

```bash
# Compile with coverage
gcc -o test_commands tests/test_commands.c \
    src/commands/commands/*.c \
    --coverage -fprofile-arcs -ftest-arcs

# Run tests
./test_commands

# Generate coverage report
gcov src/commands/commands/*.c
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

---

## 12. Future Commands

This section documents potential future built-in commands that could enhance Necromancer's Shell.

### Planned Commands

Based on code comments and common shell functionality, these commands could be added:

#### 1. Save Command

**Purpose**: Save current game state

```c
CommandResult cmd_save(ParsedCommand* cmd);
```

**Usage**: `save [filename] [--force]`

**Features**:
- Save to default file or specified file
- `--force` flag to overwrite existing
- Compress save data
- Include timestamp

**Implementation Priority**: High

#### 2. Load Command

**Purpose**: Load saved game state

```c
CommandResult cmd_load(ParsedCommand* cmd);
```

**Usage**: `load [filename]`

**Features**:
- Load from specified file
- List available saves
- Validate save file integrity
- Handle version mismatches

**Implementation Priority**: High

#### 3. History Command

**Purpose**: Show command history

```c
CommandResult cmd_history(ParsedCommand* cmd);
```

**Usage**: `history [--clear] [--size N]`

**Features**:
- Display recent commands
- Clear history
- Set history size
- Search history

**Implementation Priority**: Medium

**Note**: History system already exists (`src/terminal/history.h`), command would be a UI wrapper.

#### 4. Alias Command

**Purpose**: Create command aliases

```c
CommandResult cmd_alias(ParsedCommand* cmd);
```

**Usage**: `alias [name=value]`

**Features**:
- Create new aliases
- List existing aliases
- Remove aliases
- Save aliases to config

**Implementation Priority**: Medium

#### 5. Config Command

**Purpose**: View and modify configuration

```c
CommandResult cmd_config(ParsedCommand* cmd);
```

**Usage**: `config [get|set] <key> [value]`

**Features**:
- Get configuration values
- Set configuration values
- List all settings
- Reset to defaults

**Implementation Priority**: Medium

#### 6. Debug Command

**Purpose**: Toggle debug features

```c
CommandResult cmd_debug(ParsedCommand* cmd);
```

**Usage**: `debug [feature] [on|off]`

**Features**:
- Enable/disable debug modes
- Show debug information
- Performance statistics
- Memory diagnostics

**Implementation Priority**: Low

#### 7. Time Command

**Purpose**: Measure command execution time

```c
CommandResult cmd_time(ParsedCommand* cmd);
```

**Usage**: `time <command>`

**Features**:
- Measure execution duration
- Show real/user/sys time
- Multiple run averaging
- Performance comparison

**Implementation Priority**: Low

#### 8. Echo Command

**Purpose**: Display text (for scripting)

```c
CommandResult cmd_echo(ParsedCommand* cmd);
```

**Usage**: `echo [text...]`

**Features**:
- Print arguments
- Support escape sequences
- No newline option (`-n`)
- Color output

**Implementation Priority**: Low

#### 9. Version Command

**Purpose**: Show version information

```c
CommandResult cmd_version(ParsedCommand* cmd);
```

**Usage**: `version [--verbose]`

**Features**:
- Show version number
- Build information
- Dependencies
- Changelog

**Implementation Priority**: Low

#### 10. Macro Command

**Purpose**: Record and replay command sequences

```c
CommandResult cmd_macro(ParsedCommand* cmd);
```

**Usage**: `macro [name] <commands...>`

**Features**:
- Record command sequences
- Replay macros
- List available macros
- Save macros to file

**Implementation Priority**: Low

### Extension Points

The command system is designed for easy extension:

#### Plugin Commands

Future support for dynamically loaded commands:

```c
/* Plugin interface */
typedef struct {
    const char* name;
    const char* version;
    CommandInfo* commands;
    size_t command_count;
    bool (*init)(void);
    void (*shutdown)(void);
} CommandPlugin;

/* Load plugin */
bool load_command_plugin(const char* path);
```

**Benefits**:
- Third-party extensions
- Mod support
- Game-specific commands
- Runtime loading/unloading

#### Scripted Commands

Support for commands defined in script files:

```lua
-- Example Lua command
function cmd_greet(args)
    if args[1] then
        return "Hello, " .. args[1] .. "!"
    else
        return "Hello, Necromancer!"
    end
end
```

**Benefits**:
- No recompilation needed
- Easy prototyping
- User customization
- Rapid development

#### Command Categories

Organize commands into categories:

```c
typedef enum {
    CMD_CATEGORY_SYSTEM,    /* help, quit, clear */
    CMD_CATEGORY_GAME,      /* save, load, status */
    CMD_CATEGORY_DEBUG,     /* log, debug, time */
    CMD_CATEGORY_USER       /* custom commands */
} CommandCategory;
```

**Benefits**:
- Better help organization
- Filtered command lists
- Permission management
- Context-sensitive completions

### Deprecation Strategy

When removing or replacing commands:

1. **Mark as Deprecated**: Set flag in CommandInfo
2. **Show Warning**: Print deprecation message on use
3. **Provide Alternative**: Suggest replacement command
4. **Grace Period**: Keep for several versions
5. **Remove**: Finally delete in major version

Example:

```c
CommandInfo info = {
    .name = "oldcmd",
    .description = "[DEPRECATED] Use newcmd instead",
    .deprecated = true,
    .replacement = "newcmd",
    /* ... */
};
```

### Command Development Roadmap

**Phase 1: Core Functionality** (Current)
- help
- status
- quit/exit
- clear
- log

**Phase 2: Save System**
- save
- load
- autosave configuration

**Phase 3: User Experience**
- history
- alias
- config

**Phase 4: Development Tools**
- debug
- time
- version

**Phase 5: Advanced Features**
- macro
- plugin system
- scripting integration

### Contributing New Commands

To contribute a command:

1. **Propose**: Create issue describing command
2. **Design**: Document behavior and interface
3. **Implement**: Follow patterns in this document
4. **Test**: Write comprehensive tests
5. **Document**: Update this documentation
6. **Submit**: Create pull request

**Review Criteria**:
- Follows existing patterns
- Well-tested (unit + integration)
- Properly documented
- No memory leaks
- Consistent error handling
- User-friendly output

---

## Appendices

### Appendix A: Quick Reference

**Command Summary**:

| Command | Args | Flags | Purpose |
|---------|------|-------|---------|
| help | [command] | none | Show help information |
| status | none | --verbose, -v | Show game and system status |
| quit | none | none | Exit the shell |
| exit | none | none | Exit the shell (alias for quit) |
| clear | none | none | Clear terminal screen |
| log | [level] | --file, -f | Manage logging settings |

**Log Levels**:
- trace: Most detailed
- debug: Debug information
- info: Informational messages
- warn: Warnings
- error: Errors
- fatal: Fatal errors only

### Appendix B: File Reference

**Source Files**:
- `src/commands/commands/commands.h` - Command declarations
- `src/commands/commands/builtin.c` - Command registration
- `src/commands/commands/cmd_help.c` - Help command
- `src/commands/commands/cmd_status.c` - Status command
- `src/commands/commands/cmd_quit.c` - Quit/exit command
- `src/commands/commands/cmd_clear.c` - Clear command
- `src/commands/commands/cmd_log.c` - Log command

**Related Headers**:
- `src/commands/executor.h` - Command execution
- `src/commands/parser.h` - Command parsing
- `src/commands/registry.h` - Command registry
- `src/utils/logger.h` - Logging system

### Appendix C: Common Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| Invalid command | NULL command pointer | Internal error, report bug |
| Unknown command: X | Command not registered | Use `help` to see available commands |
| Cannot clear: not a terminal | Output redirected | Run without redirection |
| Invalid log level: X | Invalid level string | Use: trace, debug, info, warn, error, fatal |
| Failed to create output buffer | Memory allocation failed | Check system resources |

### Appendix D: Best Practices Summary

1. **Always validate input pointers**
2. **Use memory streams for dynamic output**
3. **Mark unused parameters explicitly**
4. **Check terminal status before ANSI codes**
5. **Free allocated memory after use**
6. **Provide clear error messages**
7. **Use static storage for flag definitions**
8. **Handle NULL gracefully**
9. **Test with valgrind**
10. **Document thoroughly**

---

## Conclusion

This documentation provides comprehensive coverage of all built-in commands in Necromancer's Shell. Each command is documented with:

- Complete source code references
- Implementation details and code walkthroughs
- Usage examples
- Memory management notes
- Integration points

The command system provides a solid foundation for the shell with clear patterns for extension. New commands can be added by following the templates and patterns documented here.

**Key Takeaways**:
- Built-in commands use a registry-based architecture
- Commands are defined declaratively with CommandInfo structures
- Implementation follows consistent patterns
- Memory management is handled carefully
- Error handling is comprehensive
- Future extension is well-supported

For questions or contributions, refer to the Contributing section (12.4) or open an issue in the project repository.

---

**Document Version**: 1.0
**Generated**: 2025-10-13
**Shell Version**: Development Build
**Maintainer**: Necromancer's Shell Development Team

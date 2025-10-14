# API Reference

Comprehensive reference for Necromancer's Shell public APIs.

## Table of Contents
- [Core Systems](#core-systems)
- [Command System](#command-system)
- [Terminal](#terminal)
- [Utilities](#utilities)
- [Type Reference](#type-reference)

---

## Core Systems

### Events (events.h)

Event bus for decoupled pub/sub event-driven architecture.

| Function | Signature | Description |
|----------|-----------|-------------|
| event_bus_create | `EventBus* (void)` | Create event bus (line 81) |
| event_bus_destroy | `void (EventBus* bus)` | Destroy event bus and clean up (line 89) |
| event_bus_subscribe | `size_t (EventBus* bus, EventType type, EventCallback callback, void* userdata)` | Subscribe to event type, returns subscription ID (line 101) |
| event_bus_unsubscribe | `bool (EventBus* bus, size_t subscription_id)` | Unsubscribe by ID (line 111) |
| event_bus_unsubscribe_all | `void (EventBus* bus, EventType type)` | Unsubscribe all callbacks for event type (line 119) |
| event_bus_publish | `bool (EventBus* bus, EventType type, void* data)` | Publish event immediately (synchronous) (line 130) |
| event_bus_queue | `bool (EventBus* bus, EventType type, const void* data, size_t data_size)` | Queue event for later dispatch (line 142) |
| event_bus_dispatch | `void (EventBus* bus)` | Dispatch all queued events (line 150) |
| event_bus_clear_queue | `void (EventBus* bus)` | Clear queued events without dispatch (line 157) |
| event_bus_queue_size | `size_t (const EventBus* bus)` | Get number of queued events (line 165) |
| event_bus_subscriber_count | `size_t (const EventBus* bus, EventType type)` | Get subscriber count for event type (line 174) |
| event_bus_total_subscriptions | `size_t (const EventBus* bus)` | Get total active subscriptions (line 182) |
| event_type_name | `const char* (EventType type)` | Get event type name for debugging (line 190) |

**Macros:**
- `EVENT_PUBLISH_DATA(bus, type, data_ptr)` - Publish event with data (line 197)
- `EVENT_QUEUE_DATA(bus, type, data_ptr)` - Queue event with data copy (line 203)
- `EVENT_PUBLISH_SIMPLE(bus, type)` - Publish event without data (line 209)
- `EVENT_QUEUE_SIMPLE(bus, type)` - Queue event without data (line 215)

### Memory (memory.h)

Memory pool allocator with fixed-size blocks and leak tracking.

| Function | Signature | Description |
|----------|-----------|-------------|
| pool_create | `MemoryPool* (size_t block_size, size_t block_count)` | Create memory pool (line 41) |
| pool_destroy | `void (MemoryPool* pool)` | Destroy pool, logs leaks if any (line 49) |
| pool_alloc | `void* (MemoryPool* pool)` | Allocate block from pool (line 57) |
| pool_free | `void (MemoryPool* pool, void* ptr)` | Free block back to pool (line 65) |
| pool_reset | `void (MemoryPool* pool)` | Reset pool, free all blocks at once (line 73) |
| pool_get_stats | `void (const MemoryPool* pool, MemoryStats* stats)` | Get pool statistics (line 81) |
| pool_check_leaks | `bool (const MemoryPool* pool)` | Check for memory leaks (line 90) |
| pool_print_stats | `void (const MemoryPool* pool)` | Print pool statistics via logger (line 97) |

### State Manager (state_manager.h)

Game state machine with stack-based state management.

| Function | Signature | Description |
|----------|-----------|-------------|
| state_manager_create | `StateManager* (void)` | Create state manager (line 53) |
| state_manager_destroy | `void (StateManager* manager)` | Destroy state manager (line 60) |
| state_manager_register | `bool (StateManager* manager, GameState state, const StateCallbacks* callbacks, void* userdata)` | Register state callbacks (line 71) |
| state_manager_push | `bool (StateManager* manager, GameState state)` | Push new state onto stack (line 82) |
| state_manager_pop | `bool (StateManager* manager)` | Pop current state from stack (line 91) |
| state_manager_change | `bool (StateManager* manager, GameState state)` | Change to new state (replaces current) (line 101) |
| state_manager_clear | `void (StateManager* manager)` | Clear all states from stack (line 108) |
| state_manager_update | `void (StateManager* manager, double delta_time)` | Update current state (line 116) |
| state_manager_render | `void (StateManager* manager)` | Render current state (line 123) |
| state_manager_current | `GameState (const StateManager* manager)` | Get current active state (line 131) |
| state_manager_previous | `GameState (const StateManager* manager)` | Get previous state if any (line 139) |
| state_manager_depth | `size_t (const StateManager* manager)` | Get stack depth (line 147) |
| state_manager_is_registered | `bool (const StateManager* manager, GameState state)` | Check if state is registered (line 156) |
| state_manager_state_name | `const char* (GameState state)` | Get state name for debugging (line 164) |

### Timing (timing.h)

Cross-platform timing for game loop, FPS limiting, and delta time.

| Function | Signature | Description |
|----------|-----------|-------------|
| timing_init | `void (void)` | Initialize timing system (line 25) |
| timing_frame_start | `void (void)` | Mark start of frame (line 30) |
| timing_frame_end | `void (int target_fps)` | Mark end of frame and limit FPS (line 37) |
| timing_get_delta | `double (void)` | Get delta time in seconds (line 44) |
| timing_get_fps | `double (void)` | Get current FPS (averaged) (line 51) |
| timing_get_time | `double (void)` | Get time since init in seconds (line 58) |
| timing_sleep_ms | `void (int ms)` | Sleep for milliseconds (line 65) |

### Game Loop (game_loop.h)

Main game loop abstraction with callbacks.

| Function | Signature | Description |
|----------|-----------|-------------|
| game_loop_create | `GameLoop* (const GameLoopCallbacks* callbacks, void* userdata)` | Create game loop (line 77) |
| game_loop_destroy | `void (GameLoop* loop)` | Destroy game loop (line 85) |
| game_loop_run | `bool (GameLoop* loop, unsigned int target_fps)` | Run loop (blocks until stopped) (line 95) |
| game_loop_stop | `void (GameLoop* loop)` | Stop loop (exits after current frame) (line 103) |
| game_loop_pause | `void (GameLoop* loop)` | Pause loop (update paused, render continues) (line 111) |
| game_loop_resume | `void (GameLoop* loop)` | Resume loop (line 118) |
| game_loop_is_running | `bool (const GameLoop* loop)` | Check if running or paused (line 126) |
| game_loop_is_paused | `bool (const GameLoop* loop)` | Check if paused (line 134) |
| game_loop_get_state | `GameLoopState (const GameLoop* loop)` | Get current loop state (line 142) |
| game_loop_get_fps | `double (const GameLoop* loop)` | Get current FPS (line 150) |
| game_loop_get_frame_count | `unsigned long long (const GameLoop* loop)` | Get total frames processed (line 158) |
| game_loop_get_elapsed_time | `double (const GameLoop* loop)` | Get total elapsed time in seconds (line 166) |
| game_loop_set_max_delta | `void (GameLoop* loop, double max_delta)` | Set max delta time (prevents spiral of death) (line 175) |
| game_loop_get_max_delta | `double (const GameLoop* loop)` | Get max delta time (line 183) |

---

## Command System

### Command System (command_system.h)

Top-level API for entire command system.

| Function | Signature | Description |
|----------|-----------|-------------|
| command_system_init | `bool (void)` | Initialize command system (line 33) |
| command_system_shutdown | `void (void)` | Shutdown and save history (line 39) |
| command_system_is_initialized | `bool (void)` | Check if initialized (line 46) |
| command_system_process_input | `CommandResult (const char* prompt)` | Read and execute user input (line 55) |
| command_system_execute | `CommandResult (const char* input)` | Execute command string directly (line 63) |
| command_system_get_registry | `CommandRegistry* (void)` | Get global registry (line 70) |
| command_system_get_history | `CommandHistory* (void)` | Get global history (line 77) |
| command_system_get_autocomplete | `Autocomplete* (void)` | Get global autocomplete (line 84) |
| command_system_get_input_handler | `InputHandler* (void)` | Get input handler (line 91) |
| command_system_register_command | `bool (const CommandInfo* info)` | Register custom command (line 99) |
| command_system_unregister_command | `bool (const char* name)` | Unregister command (line 107) |

### Registry (registry.h)

Central command registry with O(1) lookup.

| Function | Signature | Description |
|----------|-----------|-------------|
| command_registry_create | `CommandRegistry* (void)` | Create registry (line 66) |
| command_registry_destroy | `void (CommandRegistry* registry)` | Destroy registry (line 73) |
| command_registry_register | `bool (CommandRegistry* registry, const CommandInfo* info)` | Register command (line 82) |
| command_registry_unregister | `bool (CommandRegistry* registry, const char* name)` | Unregister command (line 91) |
| command_registry_get | `const CommandInfo* (const CommandRegistry* registry, const char* name)` | Get command info by name (line 100) |
| command_registry_exists | `bool (const CommandRegistry* registry, const char* name)` | Check if command exists (line 109) |
| command_registry_get_all_names | `bool (const CommandRegistry* registry, char*** names, size_t* count)` | Get all command names (line 119) |
| command_registry_free_names | `void (char** names, size_t count)` | Free names array (line 128) |
| command_registry_count | `size_t (const CommandRegistry* registry)` | Get command count (line 136) |

### Tokenizer (tokenizer.h)

Command input tokenization with quote and escape handling.

| Function | Signature | Description |
|----------|-----------|-------------|
| tokenize | `TokenizeResult (const char* input, Token** tokens, size_t* count)` | Tokenize input string (line 50) |
| free_tokens | `void (Token* tokens, size_t count)` | Free tokens array (line 58) |
| tokenize_error_string | `const char* (TokenizeResult result)` | Get error message (line 66) |

### Parser (parser.h)

Command parser for structured command representation.

| Function | Signature | Description |
|----------|-----------|-------------|
| parse_command | `ParseResult (const Token* tokens, size_t token_count, const CommandRegistry* registry, ParsedCommand** output)` | Parse from tokens (line 75) |
| parse_command_string | `ParseResult (const char* input, const CommandRegistry* registry, ParsedCommand** output)` | Parse from string (convenience) (line 87) |
| parsed_command_destroy | `void (ParsedCommand* cmd)` | Destroy parsed command (line 96) |
| parsed_command_get_flag | `const ArgumentValue* (const ParsedCommand* cmd, const char* flag_name)` | Get flag value (line 105) |
| parsed_command_has_flag | `bool (const ParsedCommand* cmd, const char* flag_name)` | Check if flag present (line 115) |
| parsed_command_get_arg | `const char* (const ParsedCommand* cmd, size_t index)` | Get positional argument (line 124) |
| parse_error_string | `const char* (ParseResult result)` | Get error message (line 132) |
| argument_value_create | `ArgumentValue* (const char* str, ArgumentType type)` | Create argument value from string (line 141) |
| argument_value_destroy | `void (ArgumentValue* value)` | Destroy argument value (line 148) |

### Executor (executor.h)

Command execution and result management.

| Function | Signature | Description |
|----------|-----------|-------------|
| execute_command | `CommandResult (ParsedCommand* cmd)` | Execute parsed command (line 51) |
| command_result_success | `CommandResult (const char* output)` | Create success result (line 59) |
| command_result_error | `CommandResult (ExecutionStatus status, const char* error_message)` | Create error result (line 68) |
| command_result_exit | `CommandResult (const char* output)` | Create exit result (line 76) |
| command_result_destroy | `void (CommandResult* result)` | Destroy result and free memory (line 83) |
| execution_status_string | `const char* (ExecutionStatus status)` | Get status message (line 91) |

### History (history.h)

Command history with persistent storage.

| Function | Signature | Description |
|----------|-----------|-------------|
| command_history_create | `CommandHistory* (size_t capacity)` | Create history (line 33) |
| command_history_destroy | `void (CommandHistory* history)` | Destroy history (line 40) |
| command_history_add | `bool (CommandHistory* history, const char* command)` | Add command (ignores duplicates) (line 50) |
| command_history_get | `const char* (const CommandHistory* history, size_t index)` | Get command at index (0=recent) (line 59) |
| command_history_size | `size_t (const CommandHistory* history)` | Get history size (line 67) |
| command_history_capacity | `size_t (const CommandHistory* history)` | Get capacity (line 75) |
| command_history_clear | `void (CommandHistory* history)` | Clear all history (line 82) |
| command_history_save | `bool (const CommandHistory* history, const char* filepath)` | Save to file (line 91) |
| command_history_load | `bool (CommandHistory* history, const char* filepath)` | Load from file (line 100) |
| command_history_search | `bool (const CommandHistory* history, const char* pattern, char*** results, size_t* count)` | Search for pattern (line 111) |
| command_history_free_search_results | `void (char** results, size_t count)` | Free search results (line 120) |
| command_history_default_path | `char* (void)` | Get default history file path (line 128) |

### Autocomplete (autocomplete.h)

Context-aware command and flag completion.

| Function | Signature | Description |
|----------|-----------|-------------|
| autocomplete_create | `Autocomplete* (const CommandRegistry* registry)` | Create autocomplete system (line 44) |
| autocomplete_destroy | `void (Autocomplete* ac)` | Destroy autocomplete (line 51) |
| autocomplete_get_completions | `bool (Autocomplete* ac, const char* input, char*** completions, size_t* count)` | Get completions (auto-context) (line 63) |
| autocomplete_get_completions_ex | `bool (Autocomplete* ac, const char* input, AutocompleteContext context, const char* command_name, char*** completions, size_t* count)` | Get completions with explicit context (line 77) |
| autocomplete_free_completions | `void (char** completions, size_t count)` | Free completions array (line 88) |
| autocomplete_rebuild | `bool (Autocomplete* ac)` | Rebuild index (call after registry changes) (line 96) |
| autocomplete_add_entry | `bool (Autocomplete* ac, const char* entry)` | Add custom completion entry (line 106) |
| autocomplete_remove_entry | `bool (Autocomplete* ac, const char* entry)` | Remove custom entry (line 115) |
| autocomplete_clear_custom_entries | `void (Autocomplete* ac)` | Clear all custom entries (line 122) |

### Built-in Commands (commands/commands.h)

Standard game commands.

| Function | Signature | Description |
|----------|-----------|-------------|
| cmd_help | `CommandResult (ParsedCommand* cmd)` | Help command - display help info (line 25) |
| cmd_status | `CommandResult (ParsedCommand* cmd)` | Status command - show game state (line 33) |
| cmd_quit | `CommandResult (ParsedCommand* cmd)` | Quit command - exit game (line 41) |
| cmd_clear | `CommandResult (ParsedCommand* cmd)` | Clear command - clear screen (line 49) |
| cmd_log | `CommandResult (ParsedCommand* cmd)` | Log command - manage logging (line 57) |
| register_builtin_commands | `int (CommandRegistry* registry)` | Register all built-in commands (line 65) |

---

## Terminal

### Terminal Wrapper (ncurses_wrapper.h)

Cross-platform terminal abstraction over ncurses.

| Function | Signature | Description |
|----------|-----------|-------------|
| term_init | `bool (void)` | Initialize terminal (line 24) |
| term_shutdown | `void (void)` | Shutdown terminal (line 29) |
| term_clear | `void (void)` | Clear screen (line 34) |
| term_refresh | `void (void)` | Refresh screen (display changes) (line 39) |
| term_get_size | `void (int* width, int* height)` | Get terminal dimensions (line 47) |
| term_print | `void (int x, int y, int color_pair, const char* str)` | Print string at position (line 57) |
| term_printf | `void (int x, int y, int color_pair, const char* fmt, ...)` | Print formatted string (line 67) |
| term_draw_box | `void (int x, int y, int width, int height, int color_pair)` | Draw box (line 78) |
| term_draw_hline | `void (int x, int y, int length, int color_pair)` | Draw horizontal line (line 88) |
| term_draw_vline | `void (int x, int y, int length, int color_pair)` | Draw vertical line (line 98) |
| term_set_cursor | `void (bool visible)` | Set cursor visibility (line 105) |
| term_move_cursor | `void (int x, int y)` | Move cursor (line 113) |
| term_set_echo | `void (bool enable)` | Enable/disable input echo (line 120) |
| term_set_raw | `void (bool raw)` | Set raw/cooked mode (line 127) |
| term_has_colors | `bool (void)` | Check if colors supported (line 134) |
| term_max_color_pairs | `int (void)` | Get max color pairs (line 141) |
| term_get_char | `int (void)` | Get character (non-blocking) (line 148) |

### Colors (colors.h)

Color management and game palette.

| Function | Signature | Description |
|----------|-----------|-------------|
| colors_init | `bool (void)` | Initialize color system (line 30) |
| colors_get_game_pair | `int (GameColorPair pair)` | Get game color pair ID (line 38) |

### Input System (input.h)

Enhanced keyboard input with key state tracking.

| Function | Signature | Description |
|----------|-----------|-------------|
| input_system_create | `InputSystem* (void)` | Create input system (line 41) |
| input_system_destroy | `void (InputSystem* input)` | Destroy input system (line 48) |
| input_system_update | `void (InputSystem* input)` | Update input state (call per frame) (line 56) |
| input_is_key_pressed | `bool (const InputSystem* input, int key)` | Check if key pressed this frame (line 65) |
| input_is_key_down | `bool (const InputSystem* input, int key)` | Check if key held down (line 75) |
| input_is_key_released | `bool (const InputSystem* input, int key)` | Check if key released this frame (line 84) |
| input_get_key_state | `KeyState (const InputSystem* input, int key)` | Get key state (line 93) |
| input_get_pressed_count | `size_t (const InputSystem* input)` | Get pressed key count (line 101) |
| input_is_any_key_pressed | `bool (const InputSystem* input)` | Check if any key pressed (line 109) |
| input_get_last_key | `int (const InputSystem* input)` | Get last pressed key (-1 if none) (line 118) |
| input_clear_all | `void (InputSystem* input)` | Clear all key states (line 126) |
| input_set_key_repeat | `void (InputSystem* input, bool enable)` | Enable/disable key repeat (line 135) |
| input_get_key_repeat | `bool (const InputSystem* input)` | Get key repeat setting (line 143) |
| input_begin_text_input | `void (InputSystem* input, char* buffer, size_t buffer_size)` | Start text input mode (line 155) |
| input_end_text_input | `void (InputSystem* input)` | End text input mode (line 162) |
| input_is_text_input_active | `bool (const InputSystem* input)` | Check if in text input mode (line 170) |
| input_get_text | `const char* (const InputSystem* input)` | Get current text input (line 178) |
| input_get_text_cursor | `size_t (const InputSystem* input)` | Get text cursor position (line 186) |

### UI Feedback (ui_feedback.h)

Formatted output for command results and messages.

| Function | Signature | Description |
|----------|-----------|-------------|
| ui_feedback_init | `bool (void)` | Initialize UI feedback (line 26) |
| ui_feedback_shutdown | `void (void)` | Shutdown UI feedback (line 31) |
| ui_feedback_success | `void (const char* message)` | Display success message (line 38) |
| ui_feedback_error | `void (const char* message)` | Display error message (line 45) |
| ui_feedback_warning | `void (const char* message)` | Display warning message (line 52) |
| ui_feedback_info | `void (const char* message)` | Display info message (line 59) |
| ui_feedback_command_result | `void (const CommandResult* result)` | Display command result (line 66) |
| ui_feedback_prompt | `void (const char* prompt)` | Display command prompt (line 73) |
| ui_feedback_autocomplete | `void (char** suggestions, size_t count, int current_index)` | Display autocomplete suggestions (line 82) |
| ui_feedback_clear_autocomplete | `void (void)` | Clear autocomplete display (line 87) |
| ui_feedback_history_search | `void (const char* query, char** results, size_t count, int current_index)` | Display history search results (line 97) |
| ui_feedback_print_colored | `void (const char* color, const char* prefix, const char* message)` | Print colored message (line 107) |
| ui_feedback_set_color_enabled | `void (bool enabled)` | Enable/disable color output (line 115) |
| ui_feedback_is_color_enabled | `bool (void)` | Check if colors enabled (line 122) |

### Input Handler (input_handler.h)

High-level input handling with line editing, history, and autocomplete.

| Function | Signature | Description |
|----------|-----------|-------------|
| input_handler_create | `InputHandler* (CommandRegistry* registry)` | Create input handler (line 40) |
| input_handler_destroy | `void (InputHandler* handler)` | Destroy input handler (line 47) |
| input_handler_read_line | `bool (InputHandler* handler, const char* prompt, char* buffer, size_t buffer_size)` | Read line with editing support (line 59) |
| input_handler_read_and_execute | `CommandResult (InputHandler* handler, const char* prompt)` | Read and execute command (line 70) |
| input_handler_execute | `CommandResult (InputHandler* handler, const char* input)` | Parse and execute command (line 79) |
| input_handler_get_history | `CommandHistory* (InputHandler* handler)` | Get command history (line 87) |
| input_handler_get_autocomplete | `Autocomplete* (InputHandler* handler)` | Get autocomplete system (line 95) |
| input_handler_get_registry | `CommandRegistry* (InputHandler* handler)` | Get command registry (line 103) |
| input_handler_save_history | `bool (InputHandler* handler)` | Save history to default file (line 111) |
| input_handler_load_history | `bool (InputHandler* handler)` | Load history from default file (line 119) |

---

## Utilities

### Logger (logger.h)

Multi-level logging with file and console output.

| Function | Signature | Description |
|----------|-----------|-------------|
| logger_init | `bool (const char* filename, LogLevel level)` | Initialize logger (line 37) |
| logger_shutdown | `void (void)` | Shutdown logger (flush and close) (line 42) |
| logger_set_level | `void (LogLevel level)` | Set log level (line 49) |
| logger_get_level | `LogLevel (void)` | Get current log level (line 56) |
| logger_set_console | `void (bool enable)` | Enable/disable console output (line 63) |
| logger_log | `void (LogLevel level, const char* file, int line, const char* func, const char* fmt, ...)` | Log message (line 74) |

**Macros:**
- `LOG_TRACE(...)` - Log trace message (line 78)
- `LOG_DEBUG(...)` - Log debug message (line 79)
- `LOG_INFO(...)` - Log info message (line 80)
- `LOG_WARN(...)` - Log warning message (line 81)
- `LOG_ERROR(...)` - Log error message (line 82)
- `LOG_FATAL(...)` - Log fatal message (line 83)

### Hash Table (hash_table.h)

Generic string-keyed hash table with linear probing.

| Function | Signature | Description |
|----------|-----------|-------------|
| hash_table_create | `HashTable* (size_t initial_capacity)` | Create hash table (line 32) |
| hash_table_destroy | `void (HashTable* table)` | Destroy hash table (does not free values) (line 40) |
| hash_table_put | `bool (HashTable* table, const char* key, void* value)` | Insert or update key-value pair (line 50) |
| hash_table_get | `void* (const HashTable* table, const char* key)` | Get value by key (line 59) |
| hash_table_contains | `bool (const HashTable* table, const char* key)` | Check if key exists (line 68) |
| hash_table_remove | `void* (HashTable* table, const char* key)` | Remove key-value pair (line 77) |
| hash_table_clear | `void (HashTable* table)` | Clear all entries (line 84) |
| hash_table_size | `size_t (const HashTable* table)` | Get entry count (line 92) |
| hash_table_capacity | `size_t (const HashTable* table)` | Get capacity (line 100) |
| hash_table_load_factor | `float (const HashTable* table)` | Get load factor (0.0-1.0) (line 108) |
| hash_table_foreach | `void (const HashTable* table, HashTableIterator iterator, void* userdata)` | Iterate over entries (line 117) |

### String Utils (string_utils.h)

Safe string operations and utilities.

| Function | Signature | Description |
|----------|-----------|-------------|
| str_safe_copy | `size_t (char* dst, size_t dst_size, const char* src)` | Safe string copy with bounds (line 29) |
| str_safe_concat | `size_t (char* dst, size_t dst_size, const char* src)` | Safe concatenation (line 40) |
| str_length | `size_t (const char* str)` | Get string length (NULL-safe) (line 48) |
| str_compare | `int (const char* s1, const char* s2)` | Compare strings (NULL-safe) (line 57) |
| str_compare_ignore_case | `int (const char* s1, const char* s2)` | Compare ignoring case (line 66) |
| str_equals | `bool (const char* s1, const char* s2)` | Check equality (line 75) |
| str_equals_ignore_case | `bool (const char* s1, const char* s2)` | Check equality ignoring case (line 84) |
| str_starts_with | `bool (const char* str, const char* prefix)` | Check if starts with prefix (line 93) |
| str_ends_with | `bool (const char* str, const char* suffix)` | Check if ends with suffix (line 102) |
| str_trim | `char* (char* str)` | Trim whitespace (in-place) (line 111) |
| str_trim_left | `char* (char* str)` | Trim left whitespace (line 119) |
| str_trim_right | `char* (char* str)` | Trim right whitespace (line 127) |
| str_to_lower | `char* (char* str)` | Convert to lowercase (in-place) (line 135) |
| str_to_upper | `char* (char* str)` | Convert to uppercase (in-place) (line 143) |
| str_duplicate | `char* (const char* str)` | Duplicate string (caller must free) (line 152) |
| str_find_char | `char* (const char* str, char ch)` | Find first occurrence of char (line 161) |
| str_find_char_last | `char* (const char* str, char ch)` | Find last occurrence of char (line 170) |
| str_find_substring | `char* (const char* str, const char* substr)` | Find substring (line 179) |
| str_is_empty_or_whitespace | `bool (const char* str)` | Check if empty or whitespace (line 187) |
| str_is_digits | `bool (const char* str)` | Check if only digits (line 195) |
| str_is_alnum | `bool (const char* str)` | Check if alphanumeric (line 203) |
| str_split | `size_t (char* str, char delimiter, char** tokens, size_t max_tokens)` | Split by delimiter (modifies string) (line 215) |

**String Builder:**

| Function | Signature | Description |
|----------|-----------|-------------|
| str_builder_create | `StringBuilder* (size_t initial_capacity)` | Create string builder (line 227) |
| str_builder_destroy | `void (StringBuilder* builder)` | Destroy builder (line 234) |
| str_builder_append | `bool (StringBuilder* builder, const char* str)` | Append string (line 243) |
| str_builder_append_char | `bool (StringBuilder* builder, char ch)` | Append character (line 252) |
| str_builder_append_format | `bool (StringBuilder* builder, const char* fmt, ...)` | Append formatted string (line 262) |
| str_builder_get | `const char* (const StringBuilder* builder)` | Get string (no ownership transfer) (line 272) |
| str_builder_length | `size_t (const StringBuilder* builder)` | Get length (line 280) |
| str_builder_clear | `void (StringBuilder* builder)` | Clear builder (line 287) |
| str_builder_extract | `char* (StringBuilder* builder)` | Extract string (transfers ownership) (line 297) |

### Trie (trie.h)

Prefix tree for efficient string matching (ASCII optimized).

| Function | Signature | Description |
|----------|-----------|-------------|
| trie_create | `Trie* (void)` | Create trie (line 33) |
| trie_destroy | `void (Trie* trie)` | Destroy trie (line 40) |
| trie_insert | `bool (Trie* trie, const char* str)` | Insert string (line 49) |
| trie_contains | `bool (const Trie* trie, const char* str)` | Check if string exists (line 58) |
| trie_remove | `bool (Trie* trie, const char* str)` | Remove string (line 67) |
| trie_find_with_prefix | `bool (const Trie* trie, const char* prefix, char*** matches, size_t* count)` | Find all strings with prefix (line 78) |
| trie_free_matches | `void (char** matches, size_t count)` | Free matches array (line 87) |
| trie_size | `size_t (const Trie* trie)` | Get string count (line 95) |
| trie_clear | `void (Trie* trie)` | Clear all strings (line 102) |

---

## Type Reference

### Core Types

**EventType** (events.h:22) - Event type enumeration
- `EVENT_NONE`, `EVENT_GAME_START`, `EVENT_GAME_PAUSE`, `EVENT_GAME_RESUME`, `EVENT_GAME_QUIT`
- `EVENT_PLAYER_MOVE`, `EVENT_PLAYER_ATTACK`, `EVENT_PLAYER_DAMAGE_TAKEN`, `EVENT_PLAYER_DIED`, `EVENT_PLAYER_LEVEL_UP`
- `EVENT_COMBAT_START`, `EVENT_COMBAT_END`, `EVENT_ENEMY_SPAWN`, `EVENT_ENEMY_DIED`
- `EVENT_UI_OPEN_MENU`, `EVENT_UI_CLOSE_MENU`, `EVENT_UI_BUTTON_CLICKED`, `EVENT_UI_TEXT_INPUT`
- `EVENT_RESOURCE_LOADED`, `EVENT_SAVE_GAME`, `EVENT_LOAD_GAME`
- `EVENT_CUSTOM_START` (1000) to `EVENT_CUSTOM_END` (9999)

**Event** (events.h:67) - Event data structure
```c
struct Event {
    EventType type;
    void* data;
    size_t data_size;
};
```

**EventCallback** (events.h:74) - Event callback function type
```c
typedef void (*EventCallback)(const Event* event, void* userdata);
```

**MemoryStats** (memory.h:24) - Memory pool statistics
```c
typedef struct {
    size_t total_bytes;
    size_t block_size;
    size_t block_count;
    size_t allocated_blocks;
    size_t peak_usage;
    size_t total_allocs;
    size_t total_frees;
} MemoryStats;
```

**GameState** (state_manager.h:23) - Game state enumeration
- `STATE_NONE`, `STATE_INIT`, `STATE_MAIN_MENU`, `STATE_GAME_WORLD`
- `STATE_DIALOGUE`, `STATE_INVENTORY`, `STATE_PAUSE`, `STATE_SHUTDOWN`

**StateCallbacks** (state_manager.h:39) - State callback functions
```c
typedef struct {
    void (*on_enter)(void* userdata);
    void (*on_exit)(void* userdata);
    void (*on_update)(double delta, void* userdata);
    void (*on_render)(void* userdata);
    void (*on_pause)(void* userdata);
    void (*on_resume)(void* userdata);
} StateCallbacks;
```

**GameLoopCallbacks** (game_loop.h:28) - Game loop callback functions
```c
typedef struct {
    bool (*on_init)(void* userdata);
    void (*on_update)(double delta_time, void* userdata);
    void (*on_render)(void* userdata);
    void (*on_cleanup)(void* userdata);
    void (*on_pause)(void* userdata);
    void (*on_resume)(void* userdata);
} GameLoopCallbacks;
```

**GameLoopState** (game_loop.h:64) - Loop state enumeration
- `LOOP_STATE_STOPPED`, `LOOP_STATE_RUNNING`, `LOOP_STATE_PAUSED`

**LogLevel** (logger.h:21) - Log level enumeration
- `LOG_LEVEL_TRACE`, `LOG_LEVEL_DEBUG`, `LOG_LEVEL_INFO`
- `LOG_LEVEL_WARN`, `LOG_LEVEL_ERROR`, `LOG_LEVEL_FATAL`

### Command System Types

**ArgumentType** (registry.h:28) - Argument type enumeration
- `ARG_TYPE_STRING`, `ARG_TYPE_INT`, `ARG_TYPE_FLOAT`, `ARG_TYPE_BOOL`

**FlagDefinition** (registry.h:36) - Command flag definition
```c
typedef struct {
    const char* name;
    char short_name;
    ArgumentType type;
    bool required;
    const char* description;
} FlagDefinition;
```

**CommandInfo** (registry.h:45) - Command metadata
```c
typedef struct {
    const char* name;
    const char* description;
    const char* usage;
    const char* help_text;
    CommandFunction function;
    FlagDefinition* flags;
    size_t flag_count;
    size_t min_args;
    size_t max_args;
    bool hidden;
} CommandInfo;
```

**CommandFunction** (registry.h:25) - Command execution function type
```c
typedef CommandResult (*CommandFunction)(ParsedCommand* cmd);
```

**Token** (tokenizer.h:27) - Tokenized input token
```c
typedef struct {
    char* value;
    size_t length;
    bool is_quoted;
} Token;
```

**TokenizeResult** (tokenizer.h:34) - Tokenization result codes
- `TOKENIZE_SUCCESS`, `TOKENIZE_ERROR_MEMORY`, `TOKENIZE_ERROR_UNCLOSED_QUOTE`
- `TOKENIZE_ERROR_INVALID_ESCAPE`, `TOKENIZE_ERROR_EMPTY_INPUT`

**ArgumentValue** (parser.h:32) - Parsed argument value (variant type)
```c
typedef struct {
    ArgumentType type;
    union {
        char* str_value;
        int int_value;
        float float_value;
        bool bool_value;
    } value;
} ArgumentValue;
```

**ParsedCommand** (parser.h:43) - Parsed command structure
```c
typedef struct ParsedCommand {
    const char* command_name;
    const CommandInfo* info;
    HashTable* flags;
    char** args;
    size_t arg_count;
    char* raw_input;
} ParsedCommand;
```

**ParseResult** (parser.h:53) - Parse result codes
- `PARSE_SUCCESS`, `PARSE_ERROR_EMPTY_COMMAND`, `PARSE_ERROR_UNKNOWN_COMMAND`
- `PARSE_ERROR_INVALID_FLAG`, `PARSE_ERROR_MISSING_FLAG_VALUE`, `PARSE_ERROR_INVALID_FLAG_VALUE`
- `PARSE_ERROR_TOO_FEW_ARGS`, `PARSE_ERROR_TOO_MANY_ARGS`, `PARSE_ERROR_REQUIRED_FLAG_MISSING`, `PARSE_ERROR_MEMORY`

**ExecutionStatus** (executor.h:26) - Execution status codes
- `EXEC_SUCCESS`, `EXEC_ERROR_COMMAND_FAILED`, `EXEC_ERROR_INVALID_COMMAND`
- `EXEC_ERROR_PERMISSION_DENIED`, `EXEC_ERROR_NOT_IMPLEMENTED`, `EXEC_ERROR_INTERNAL`

**CommandResult** (executor.h:36) - Command execution result
```c
typedef struct CommandResult {
    ExecutionStatus status;
    bool success;
    char* output;
    char* error_message;
    int exit_code;
    bool should_exit;
} CommandResult;
```

**AutocompleteContext** (autocomplete.h:32) - Autocomplete context
- `AUTOCOMPLETE_CONTEXT_COMMAND`, `AUTOCOMPLETE_CONTEXT_FLAG`, `AUTOCOMPLETE_CONTEXT_ARGUMENT`

### Terminal Types

**GameColorPair** (colors.h:13) - Game color palette
- `COLOR_PAIR_DEFAULT`, `COLOR_PAIR_UI_BORDER`, `COLOR_PAIR_UI_TEXT`, `COLOR_PAIR_UI_HIGHLIGHT`
- `COLOR_PAIR_HP_HIGH`, `COLOR_PAIR_HP_LOW`, `COLOR_PAIR_ERROR`, `COLOR_PAIR_SUCCESS`

**KeyState** (input.h:26) - Key state enumeration
- `KEY_STATE_UP`, `KEY_STATE_PRESSED`, `KEY_STATE_DOWN`, `KEY_STATE_RELEASED`

### Utility Types

**HashTableIterator** (hash_table.h:24) - Hash table iterator callback
```c
typedef void (*HashTableIterator)(const char* key, void* value, void* userdata);
```

---

## Usage Examples

### Core System Example
```c
// Initialize core systems
logger_init("game.log", LOG_LEVEL_DEBUG);
timing_init();

// Create event bus
EventBus* bus = event_bus_create();
event_bus_subscribe(bus, EVENT_GAME_START, on_game_start, NULL);

// Create memory pool
MemoryPool* pool = pool_create(1024, 100);

// Create state manager
StateManager* sm = state_manager_create();
state_manager_register(sm, STATE_MAIN_MENU, &menu_callbacks, NULL);

// Game loop
GameLoopCallbacks callbacks = { .on_init = init, .on_update = update, .on_render = render };
GameLoop* loop = game_loop_create(&callbacks, NULL);
game_loop_run(loop, 60);

// Cleanup
game_loop_destroy(loop);
state_manager_destroy(sm);
pool_destroy(pool);
event_bus_destroy(bus);
logger_shutdown();
```

### Command System Example
```c
// Initialize command system
command_system_init();

// Main loop
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
```

### Custom Command Example
```c
// Define command
CommandResult cmd_custom(ParsedCommand* cmd) {
    const char* arg = parsed_command_get_arg(cmd, 0);
    if (!arg) {
        return command_result_error(EXEC_ERROR_INVALID_COMMAND, "Missing argument");
    }
    return command_result_success("Command executed");
}

// Register command
FlagDefinition flags[] = {
    { "verbose", 'v', ARG_TYPE_BOOL, false, "Verbose output" }
};

CommandInfo info = {
    .name = "custom",
    .description = "Custom command",
    .usage = "custom <arg> [--verbose]",
    .function = cmd_custom,
    .flags = flags,
    .flag_count = 1,
    .min_args = 1,
    .max_args = 1
};

command_system_register_command(&info);
```

---

**Note:** All line numbers reference the corresponding header files in `/home/stripcheese/Necromancers Shell/necromancers_shell/src/`.

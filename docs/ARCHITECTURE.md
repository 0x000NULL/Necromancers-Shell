# Architecture and Design

## Table of Contents

1. [System Overview](#1-system-overview)
   - 1.1 [Purpose and Goals](#11-purpose-and-goals)
   - 1.2 [High-Level Architecture](#12-high-level-architecture)
   - 1.3 [Module Organization](#13-module-organization)
   - 1.4 [Layered Architecture](#14-layered-architecture)

2. [Module Architecture](#2-module-architecture)
   - 2.1 [Core Layer](#21-core-layer)
   - 2.2 [Command Layer](#22-command-layer)
   - 2.3 [Terminal Layer](#23-terminal-layer)
   - 2.4 [Utilities Layer](#24-utilities-layer)

3. [Design Patterns](#3-design-patterns)
   - 3.1 [Singleton Pattern](#31-singleton-pattern)
   - 3.2 [Strategy Pattern](#32-strategy-pattern)
   - 3.3 [Observer Pattern](#33-observer-pattern)
   - 3.4 [Factory Pattern](#34-factory-pattern)
   - 3.5 [Circular Buffer](#35-circular-buffer)
   - 3.6 [Trie Data Structure](#36-trie-data-structure)
   - 3.7 [State Machine](#37-state-machine)
   - 3.8 [Callback-Based Architecture](#38-callback-based-architecture)
   - 3.9 [Opaque Pointer Pattern](#39-opaque-pointer-pattern)

4. [Data Flow](#4-data-flow)
   - 4.1 [Command Execution Flow](#41-command-execution-flow)
   - 4.2 [Event Propagation Flow](#42-event-propagation-flow)
   - 4.3 [Game Loop Flow](#43-game-loop-flow)

5. [Module Dependencies](#5-module-dependencies)
   - 5.1 [Dependency Graph](#51-dependency-graph)
   - 5.2 [Dependency Rules](#52-dependency-rules)

6. [Memory Management Strategy](#6-memory-management-strategy)
   - 6.1 [Ownership Patterns](#61-ownership-patterns)
   - 6.2 [Allocation Strategies](#62-allocation-strategies)
   - 6.3 [Memory Pools](#63-memory-pools)

7. [Error Handling Strategy](#7-error-handling-strategy)
   - 7.1 [Return Code Pattern](#71-return-code-pattern)
   - 7.2 [Error Propagation](#72-error-propagation)
   - 7.3 [Logging Integration](#73-logging-integration)

8. [Initialization and Shutdown](#8-initialization-and-shutdown)
   - 8.1 [Startup Sequence](#81-startup-sequence)
   - 8.2 [Shutdown Sequence](#82-shutdown-sequence)

9. [Extensibility Points](#9-extensibility-points)
   - 9.1 [Command Registration](#91-command-registration)
   - 9.2 [Event Subscriptions](#92-event-subscriptions)
   - 9.3 [Custom Data Structures](#93-custom-data-structures)

10. [Performance Considerations](#10-performance-considerations)
    - 10.1 [Time Complexity Goals](#101-time-complexity-goals)
    - 10.2 [Memory Efficiency](#102-memory-efficiency)
    - 10.3 [Frame Rate Control](#103-frame-rate-control)

11. [Platform Portability](#11-platform-portability)
    - 11.1 [Abstraction Layers](#111-abstraction-layers)
    - 11.2 [Platform-Specific Code](#112-platform-specific-code)

12. [Design Decisions](#12-design-decisions)
    - 12.1 [C Language Choice](#121-c-language-choice)
    - 12.2 [ncurses for Terminal UI](#122-ncurses-for-terminal-ui)
    - 12.3 [Hash Table for Command Registry](#123-hash-table-for-command-registry)
    - 12.4 [Event Queue vs Direct Dispatch](#124-event-queue-vs-direct-dispatch)

---

## 1. System Overview

### 1.1 Purpose and Goals

**Necromancer's Shell** is a terminal-based game engine with a sophisticated command-driven interface. The architecture is designed to support:

- Real-time game loop execution with consistent frame rates
- Text-based UI using ncurses for cross-platform terminal rendering
- Extensible command system with autocomplete and history
- Event-driven architecture for decoupled system communication
- Memory-efficient operation with minimal allocations during gameplay

The system emphasizes **modularity**, **performance**, and **maintainability** while providing a rich interactive experience in a terminal environment.

### 1.2 High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         Application                         │
│                          (main.c)                           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │        Core Layer (Game Loop)           │
        │  ┌──────────┐  ┌─────────────────────┐ │
        │  │  Events  │  │   State Machine     │ │
        │  └──────────┘  └─────────────────────┘ │
        │  ┌──────────┐  ┌─────────────────────┐ │
        │  │  Timing  │  │   Memory Pools      │ │
        │  └──────────┘  └─────────────────────┘ │
        └─────────────────────────────────────────┘
                    │                │
         ┌──────────┘                └──────────┐
         ▼                                       ▼
┌──────────────────────┐            ┌──────────────────────┐
│   Command Layer      │            │   Terminal Layer     │
│  ┌────────────────┐  │            │  ┌───────────────┐  │
│  │    Registry    │  │            │  │    ncurses    │  │
│  │   (HashTable)  │  │            │  │    Wrapper    │  │
│  └────────────────┘  │            │  └───────────────┘  │
│  ┌────────────────┐  │            │  ┌───────────────┐  │
│  │    History     │  │            │  │     Input     │  │
│  │ (CircularBuf)  │  │            │  │    Handler    │  │
│  └────────────────┘  │            │  └───────────────┘  │
│  ┌────────────────┐  │            │  ┌───────────────┐  │
│  │  Autocomplete  │  │            │  │     Colors    │  │
│  │     (Trie)     │  │            │  │   UI Feedback │  │
│  └────────────────┘  │            │  └───────────────┘  │
└──────────────────────┘            └──────────────────────┘
         │                                       │
         └───────────┬───────────────────────────┘
                     ▼
        ┌────────────────────────────┐
        │     Utilities Layer        │
        │  ┌──────────────────────┐  │
        │  │   Hash Table (O(1))  │  │
        │  │   Trie (Prefix Tree) │  │
        │  │   Logger (Singleton) │  │
        │  │   String Utilities   │  │
        │  └──────────────────────┘  │
        └────────────────────────────┘
```

### 1.3 Module Organization

The codebase is organized into four primary layers:

```
necromancers_shell/src/
├── core/              # Game engine core (loop, events, state, memory)
├── commands/          # Command system (registry, parser, executor, history)
├── terminal/          # Terminal I/O (ncurses wrapper, input, colors, UI)
└── utils/             # Reusable utilities (hash_table, trie, logger, strings)
```

Each module has:
- **Header file (.h)**: Public interface with opaque pointers
- **Implementation file (.c)**: Private implementation details
- **Clear responsibilities**: Single Responsibility Principle

### 1.4 Layered Architecture

The architecture follows a **strict layered dependency model**:

```
┌───────────────────────────────────────────────────────────┐
│  Layer 4: Application (main.c, game-specific code)       │
│  Dependencies: All layers below                           │
└───────────────────────────────────────────────────────────┘
                         │
┌───────────────────────────────────────────────────────────┐
│  Layer 3: Integration (command_system, game_loop)         │
│  Dependencies: Core, Commands, Terminal, Utils            │
└───────────────────────────────────────────────────────────┘
                         │
┌───────────────────────────────────────────────────────────┐
│  Layer 2: Subsystems (events, registry, input_handler)    │
│  Dependencies: Utils only                                 │
└───────────────────────────────────────────────────────────┘
                         │
┌───────────────────────────────────────────────────────────┐
│  Layer 1: Utilities (hash_table, trie, logger, memory)    │
│  Dependencies: Standard library only                      │
└───────────────────────────────────────────────────────────┘
```

**Rules:**
- Upper layers depend on lower layers
- Lower layers NEVER depend on upper layers
- Utilities have NO dependencies on other project modules
- No circular dependencies between modules

---

## 2. Module Architecture

### 2.1 Core Layer

**Purpose**: Provides the game engine foundation with event-driven architecture and state management.

#### Components

**2.1.1 Game Loop** (`game_loop.h/c`)
- **Responsibility**: Fixed-timestep game loop with configurable FPS
- **Pattern**: Callback-based architecture
- **Features**:
  - `on_init`, `on_update`, `on_render`, `on_cleanup` callbacks
  - Pause/resume support
  - Delta time clamping to prevent "spiral of death"
  - FPS tracking and frame counting

```c
// Example from game_loop.c (lines 54-131)
bool game_loop_run(GameLoop* loop, unsigned int target_fps) {
    // Initialize
    if (loop->callbacks.on_init) {
        if (!loop->callbacks.on_init(loop->userdata)) return false;
    }

    // Main loop
    while (loop->state != LOOP_STATE_STOPPED) {
        timing_frame_start();
        double delta_time = timing_get_delta();

        // Update (only when not paused)
        if (loop->state == LOOP_STATE_RUNNING) {
            if (loop->callbacks.on_update) {
                loop->callbacks.on_update(delta_time, loop->userdata);
            }
        }

        // Render (always, even when paused)
        if (loop->callbacks.on_render) {
            loop->callbacks.on_render(loop->userdata);
        }

        timing_frame_end(target_fps);
    }

    // Cleanup
    if (loop->callbacks.on_cleanup) {
        loop->callbacks.on_cleanup(loop->userdata);
    }
    return true;
}
```

**2.1.2 Event System** (`events.h/c`)
- **Responsibility**: Pub/sub event bus for decoupled communication
- **Pattern**: Observer pattern with event queuing
- **Data Structure**: Array of subscription linked lists indexed by event type
- **Features**:
  - Synchronous publish (immediate dispatch)
  - Asynchronous queue (batch dispatch)
  - Multiple subscribers per event
  - Event data copying for queued events

```c
// Architecture from events.c (lines 26-33)
struct EventBus {
    Subscription* subscriptions[EVENT_COUNT];  // Fixed-size array of lists
    QueuedEvent* event_queue;                  // Dynamic queue
    size_t queue_size;
    size_t queue_capacity;
    size_t next_subscription_id;
    size_t total_subscriptions;
};
```

**Why this design?**
- **O(1)** event type lookup via array indexing
- Linked lists allow dynamic subscription counts
- Queue enables deferred processing (e.g., handle events between frames)

**2.1.3 State Manager** (`state_manager.h/c`)
- **Responsibility**: Game state machine with state stack
- **Pattern**: State pattern with stack-based transitions
- **Features**:
  - Push/pop states (for menus, pause screens)
  - State change (replace current state)
  - Callbacks: `on_enter`, `on_exit`, `on_pause`, `on_resume`
  - States: `STATE_MAIN_MENU`, `STATE_GAME_WORLD`, `STATE_DIALOGUE`, etc.

**2.1.4 Memory Manager** (`memory.h/c`)
- **Responsibility**: Fixed-size block memory pools
- **Pattern**: Pool allocator
- **Features**:
  - Fast O(1) allocation/deallocation
  - Memory leak detection
  - Usage statistics and tracking

**2.1.5 Timing System** (`timing.h/c`)
- **Responsibility**: Frame timing and delta time calculation
- **Features**:
  - FPS limiting
  - Delta time computation
  - Elapsed time tracking

#### Dependencies
- **Internal**: None (self-contained)
- **External**: Logger (utils), standard time.h

#### Data Flow
```
Game Loop
   │
   ├──> Timing: Frame start/end
   │
   ├──> State Manager: Update current state
   │
   └──> Event Bus: Dispatch queued events
```

---

### 2.2 Command Layer

**Purpose**: Provides a complete command-line interface system with history, autocomplete, and execution.

#### Components

**2.2.1 Command System** (`command_system.h/c`)
- **Responsibility**: High-level API coordinating all command subsystems
- **Pattern**: Facade + Singleton (global state)
- **Global State**:

```c
// From command_system.c (lines 8-17)
static struct {
    bool initialized;
    CommandRegistry* registry;
    InputHandler* input_handler;
} g_command_system;

CommandRegistry* g_command_registry = NULL;  // For commands to access
StateManager* g_state_manager = NULL;         // For game integration
```

**Why global state?**
- Commands need access to registry from any context
- Single command system per application (natural singleton)
- Simplifies integration (no need to pass pointers everywhere)

**2.2.2 Command Registry** (`registry.h/c`)
- **Responsibility**: Central database of all available commands
- **Data Structure**: Hash table mapping command names to CommandInfo
- **Pattern**: Factory pattern (creates/stores command metadata)

```c
// From registry.c (lines 9-13)
struct CommandRegistry {
    HashTable* commands; // Maps command name -> CommandInfo*
    size_t count;
};
```

**CommandInfo Structure**:
```c
typedef struct {
    const char* name;                // "help", "quit", etc.
    const char* description;         // Short description
    const char* usage;               // Usage string
    const char* help_text;           // Detailed help
    CommandFunction function;        // Execution function pointer
    FlagDefinition* flags;           // Array of flag definitions
    size_t flag_count;
    size_t min_args;                 // Validation
    size_t max_args;
    bool hidden;                     // Hide from help listing
} CommandInfo;
```

**Time Complexity**: O(1) command lookup via hash table

**2.2.3 Command Parser** (`parser.h/c`)
- **Responsibility**: Parse command strings into structured data
- **Features**:
  - Tokenization with quote handling
  - Flag parsing (`--flag`, `-f`)
  - Argument extraction
  - Validation against CommandInfo

**2.2.4 Command Executor** (`executor.h/c`)
- **Responsibility**: Execute parsed commands and collect results
- **Pattern**: Strategy pattern (different execution strategies per command)

```c
// From executor.h (lines 36-43)
typedef struct CommandResult {
    ExecutionStatus status;
    bool success;
    char* output;                // Dynamically allocated
    char* error_message;         // Dynamically allocated
    int exit_code;
    bool should_exit;            // Signal application exit
} CommandResult;
```

**2.2.5 Command History** (`history.h/c`)
- **Responsibility**: Store and retrieve command history
- **Pattern**: Circular buffer
- **Features**:
  - Persistent storage to disk
  - Duplicate detection
  - Search functionality

```c
// From history.c (lines 13-18)
struct CommandHistory {
    char** commands;    // Circular buffer of commands
    size_t capacity;    // Maximum number of commands
    size_t size;        // Current number of commands
    size_t head;        // Index of most recent command
};
```

**Why circular buffer?**
- Fixed memory footprint (no unbounded growth)
- O(1) add operation
- Efficient for limited history size

**2.2.6 Autocomplete** (`autocomplete.h/c`)
- **Responsibility**: Context-aware command completion
- **Data Structure**: Trie (prefix tree)
- **Features**:
  - Command name completion
  - Flag completion
  - Prefix matching in O(m) where m = prefix length

**2.2.7 Input Handler** (`input_handler.h/c`)
- **Responsibility**: Integrate terminal input with command system
- **Features**:
  - Line editing
  - History navigation (up/down arrows)
  - Tab completion
  - Command execution

#### Dependencies
- **Core**: None (commands can integrate with core but not vice versa)
- **Terminal**: Input handler depends on terminal layer
- **Utils**: Hash table, trie, logger, string utilities

#### Data Flow

See [Section 4.1](#41-command-execution-flow) for detailed flow diagram.

---

### 2.3 Terminal Layer

**Purpose**: Abstracts terminal I/O and provides platform-independent rendering.

#### Components

**2.3.1 ncurses Wrapper** (`ncurses_wrapper.h/c`)
- **Responsibility**: Encapsulate ncurses library
- **Pattern**: Facade pattern
- **Features**:
  - Initialization/shutdown
  - Screen clearing and refresh
  - Box drawing
  - Cursor control
  - Non-blocking input

**Why wrap ncurses?**
- Isolates platform-specific code
- Enables future replacement (e.g., termbox, custom renderer)
- Simplifies API (only expose what we need)

**2.3.2 Input Handler** (`input.h/c`, `input_handler.h/c`)
- **Responsibility**: Read and process keyboard input
- **Features**:
  - Key mapping
  - Input buffering
  - Special key handling (arrows, function keys)

**2.3.3 Colors** (`colors.h/c`)
- **Responsibility**: Manage terminal color pairs
- **Features**:
  - 8 predefined game color pairs
  - Color initialization detection
  - Fallback for non-color terminals

**2.3.4 UI Feedback** (`ui_feedback.h/c`)
- **Responsibility**: Display command results and messages
- **Features**:
  - Success/error message rendering
  - Output formatting
  - Color-coded feedback

#### Dependencies
- **Core**: None
- **Commands**: Input handler integrates with command system
- **Utils**: Logger
- **External**: ncurses library

---

### 2.4 Utilities Layer

**Purpose**: Provide reusable data structures and utilities with no project-specific dependencies.

#### Components

**2.4.1 Hash Table** (`hash_table.h/c`)
- **Data Structure**: Open addressing with linear probing
- **Time Complexity**:
  - Average case: O(1) insert, lookup, delete
  - Worst case: O(n) when high load factor
- **Features**:
  - Automatic resizing
  - String keys
  - Generic void* values
  - Load factor tracking

**Why linear probing?**
- Better cache locality than chaining
- Simpler implementation
- Sufficient for expected load factors (~0.7)

**2.4.2 Trie** (`trie.h/c`)
- **Data Structure**: Prefix tree (128-character ASCII)
- **Time Complexity**:
  - Insert: O(m) where m = string length
  - Search: O(m)
  - Prefix match: O(m + k) where k = number of matches
- **Usage**: Autocomplete system

**Why trie over hash table for autocomplete?**
- Efficient prefix matching (hash tables can't do this)
- Returns all matches with shared prefix
- Optimal for autocomplete use case

**2.4.3 Logger** (`logger.h/c`)
- **Pattern**: Singleton (implicit via static state)
- **Features**:
  - Multiple log levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
  - File and console output
  - Timestamp and source location
  - Formatted output with macros

```c
// From logger.h (lines 78-83)
#define LOG_TRACE(...) logger_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...) logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)  logger_log(LOG_LEVEL_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)  logger_log(LOG_LEVEL_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_FATAL(...) logger_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)
```

**2.4.4 String Utilities** (`string_utils.h/c`)
- **Features**: Common string operations (trim, split, compare, etc.)

#### Why These Utilities?

These utilities are chosen because:
1. **No standard C library equivalents** (hash tables, tries not in stdlib)
2. **Performance critical** (used in hot paths like command lookup)
3. **Reusable** (applicable to any C project)
4. **Simple interface** (easy to understand and maintain)

---

## 3. Design Patterns

### 3.1 Singleton Pattern

**Where**: Logger, Command System

**Implementation**: Static global state with accessor functions

```c
// From logger.c (implied - not shown in excerpts but standard pattern)
static struct {
    FILE* log_file;
    LogLevel current_level;
    bool console_enabled;
    bool initialized;
} g_logger;

bool logger_init(const char* filename, LogLevel level) {
    if (g_logger.initialized) return true;
    // ... initialization
    g_logger.initialized = true;
    return true;
}
```

**Why**:
- Only one logger per application (shared resource)
- Only one command system per application
- Avoids passing logger pointers everywhere

**Trade-offs**:
- **Pro**: Simplified API, global access
- **Con**: Harder to unit test, global state
- **Mitigation**: Init/shutdown functions for cleanup

### 3.2 Strategy Pattern

**Where**: Command execution

**Implementation**: Function pointers in CommandInfo

```c
// From registry.h (lines 25, 50)
typedef struct CommandResult (*CommandFunction)(struct ParsedCommand* cmd);

typedef struct {
    const char* name;
    // ...
    CommandFunction function;  // Different strategy per command
    // ...
} CommandInfo;
```

**How it works**:
1. Each command registers a function pointer
2. Executor calls the function for parsed command
3. Different commands have different execution strategies

**Example**:
```c
// cmd_help.c
CommandResult cmd_help(ParsedCommand* cmd) {
    // Help command strategy
}

// cmd_quit.c
CommandResult cmd_quit(ParsedCommand* cmd) {
    // Quit command strategy
}

// Registration
CommandInfo help_info = { .name = "help", .function = cmd_help };
CommandInfo quit_info = { .name = "quit", .function = cmd_quit };
```

**Why**:
- Open/closed principle (add commands without modifying executor)
- Each command encapsulates its own logic
- Easy to add new commands

### 3.3 Observer Pattern

**Where**: Event system

**Implementation**: Subscription lists with callbacks

```c
// From events.c (lines 10-17, 26-33)
typedef struct Subscription {
    size_t id;
    EventType type;
    EventCallback callback;
    void* userdata;
    bool active;
    struct Subscription* next;  // Linked list
} Subscription;

struct EventBus {
    Subscription* subscriptions[EVENT_COUNT];  // One list per event type
    // ...
};
```

**How it works**:
1. Subjects (event publishers) call `event_bus_publish()`
2. Observers (subscribers) registered via `event_bus_subscribe()`
3. Event bus notifies all observers when event occurs

```c
// From events.c (lines 191-213)
bool event_bus_publish(EventBus* bus, EventType type, void* data) {
    Event event = { .type = type, .data = data };

    // Notify all subscribers
    Subscription* sub = bus->subscriptions[type];
    while (sub) {
        if (sub->active && sub->callback) {
            sub->callback(&event, sub->userdata);
        }
        sub = sub->next;
    }
    return true;
}
```

**Why**:
- Decouples event sources from event handlers
- Multiple systems can react to same event
- Easy to add new event handlers without modifying publishers

### 3.4 Factory Pattern

**Where**: Command registration

**Implementation**: Registry creates and stores command metadata

```c
// From registry.c (lines 16-60, 117-137)
static CommandInfo* duplicate_command_info(const CommandInfo* info) {
    // Deep copy of command info (factory method)
    CommandInfo* copy = malloc(sizeof(CommandInfo));
    // ... copy all fields
    return copy;
}

bool command_registry_register(CommandRegistry* registry, const CommandInfo* info) {
    // Factory: Create internal copy
    CommandInfo* copy = duplicate_command_info(info);
    hash_table_put(registry->commands, info->name, copy);
    return true;
}
```

**Why**:
- Centralized object creation
- Ensures proper memory management
- Allows internal representation to differ from external

### 3.5 Circular Buffer

**Where**: Command history

**Implementation**: Fixed-size array with head pointer

```c
// From history.c (lines 13-18)
struct CommandHistory {
    char** commands;    // Fixed-size array
    size_t capacity;
    size_t size;
    size_t head;        // Index of most recent
};

// Adding (lines 52-84)
bool command_history_add(CommandHistory* history, const char* command) {
    // Move head forward (wrap around)
    history->head = (history->head + 1) % history->capacity;

    // Overwrite oldest if full
    if (history->commands[history->head]) {
        free(history->commands[history->head]);
    }

    history->commands[history->head] = strdup(command);
    // ...
}
```

**Why**:
- **O(1)** add operation
- **Fixed memory** (no unbounded growth)
- Automatically overwrites oldest entries

**Trade-offs**:
- **Pro**: Predictable memory usage, simple implementation
- **Con**: Limited history size
- **Decision**: Acceptable for command history (100-1000 commands sufficient)

### 3.6 Trie Data Structure

**Where**: Autocomplete

**Purpose**: Efficient prefix matching for command completion

**Time Complexity**:
- Insert: O(m) where m = string length
- Prefix search: O(m + k) where k = matches

**Why not hash table?**
- Hash tables can't efficiently find all strings with prefix
- Trie returns all matches with shared prefix
- Optimal for autocomplete: type "he" → ["help", "health", "hello"]

**Space Complexity**: O(ALPHABET_SIZE * N * M) where N = strings, M = avg length
- For ASCII (128 chars): Higher space usage than hash table
- Trade-off: Speed for space (acceptable for command names)

### 3.7 State Machine

**Where**: State manager, game loop

**Implementation**: Stack-based state machine

```c
// From state_manager.h (lines 23-33)
typedef enum {
    STATE_NONE = 0,
    STATE_INIT,
    STATE_MAIN_MENU,
    STATE_GAME_WORLD,
    STATE_DIALOGUE,
    STATE_INVENTORY,
    STATE_PAUSE,
    STATE_SHUTDOWN,
    STATE_COUNT
} GameState;
```

**Operations**:
- **Push**: Add state on top (previous state paused)
- **Pop**: Remove current state (resume previous)
- **Change**: Replace current state

**Example flow**:
```
1. Push STATE_MAIN_MENU          Stack: [MAIN_MENU]
2. Push STATE_GAME_WORLD         Stack: [MAIN_MENU, GAME_WORLD]
3. Push STATE_PAUSE              Stack: [MAIN_MENU, GAME_WORLD, PAUSE]
4. Pop (exit pause)              Stack: [MAIN_MENU, GAME_WORLD]
5. Push STATE_INVENTORY          Stack: [MAIN_MENU, GAME_WORLD, INVENTORY]
```

**Why stack-based?**
- Natural pause/resume behavior
- Previous state context preserved
- Easy to implement modal dialogs and menus

### 3.8 Callback-Based Architecture

**Where**: Game loop, state manager

**Implementation**: Function pointer structs

```c
// From game_loop.h (lines 28-61)
typedef struct {
    bool (*on_init)(void* userdata);
    void (*on_update)(double delta_time, void* userdata);
    void (*on_render)(void* userdata);
    void (*on_cleanup)(void* userdata);
    void (*on_pause)(void* userdata);
    void (*on_resume)(void* userdata);
} GameLoopCallbacks;
```

**Why**:
- **Inversion of control**: Framework calls user code
- **Flexibility**: Different games can plug in different logic
- **Testability**: Easy to mock callbacks for testing

### 3.9 Opaque Pointer Pattern

**Where**: ALL public APIs

**Implementation**: Forward declarations in headers, full definitions in .c files

```c
// hash_table.h
typedef struct HashTable HashTable;  // Opaque pointer

// hash_table.c
struct HashTable {
    // Implementation hidden from users
    HashEntry* entries;
    size_t capacity;
    size_t size;
};
```

**Why**:
- **Encapsulation**: Internal structure hidden
- **ABI stability**: Can change internals without recompiling users
- **Prevention of misuse**: Users can't access internals directly

**C idiom for information hiding** (equivalent to private members in C++)

---

## 4. Data Flow

### 4.1 Command Execution Flow

Complete flow from user input to command execution:

```
┌─────────────────────────────────────────────────────────────────┐
│                        USER TYPES COMMAND                       │
│                         "help --verbose"                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  1. INPUT HANDLER (input_handler.c)                             │
│     - Read line from terminal (with editing support)            │
│     - Handle special keys (Tab → autocomplete, Up → history)    │
│     - Return complete line                                      │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  2. ADD TO HISTORY (history.c)                                  │
│     - Check if not duplicate                                    │
│     - Add to circular buffer                                    │
│     - history->head = (history->head + 1) % capacity            │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  3. TOKENIZE (parser.c)                                         │
│     - Split on whitespace (respecting quotes)                   │
│     - ["help", "--verbose"]                                     │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  4. REGISTRY LOOKUP (registry.c)                                │
│     - hash_table_get(registry->commands, "help")                │
│     - Returns CommandInfo* or NULL (O(1))                       │
└─────────────────────────────────────────────────────────────────┘
                                │
                   ┌────────────┴────────────┐
                   ▼                         ▼
              ┌─────────┐             ┌──────────┐
              │ FOUND   │             │ NOT FOUND│
              └─────────┘             └──────────┘
                   │                         │
                   │                         ▼
                   │              Return error result
                   ▼              "Command not found"
┌─────────────────────────────────────────────────────────────────┐
│  5. PARSE (parser.c)                                            │
│     - Extract arguments and flags                               │
│     - Validate against CommandInfo (min_args, max_args, flags)  │
│     - Build ParsedCommand struct                                │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  6. EXECUTE (executor.c)                                        │
│     - Call: info->function(parsed_command)                      │
│     - Function pointer invocation (Strategy Pattern)            │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  7. COMMAND IMPLEMENTATION (e.g., cmd_help.c)                   │
│     - Execute command logic                                     │
│     - Return CommandResult { success, output, error }           │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  8. UI FEEDBACK (ui_feedback.c)                                 │
│     - Display result.output (green if success)                  │
│     - Display result.error (red if error)                       │
│     - Check result.should_exit (quit command)                   │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  9. CLEANUP (executor.c)                                        │
│     - command_result_destroy() frees output/error strings       │
│     - parsed_command_destroy() frees tokens                     │
└─────────────────────────────────────────────────────────────────┘
```

**Step-by-step walkthrough**:

1. **Input Handler** reads characters from terminal, handles special keys (arrows, tab), returns completed line
2. **History** stores command (unless duplicate) in circular buffer for later recall
3. **Parser** tokenizes command string respecting quotes: `"help --verbose"` → `["help", "--verbose"]`
4. **Registry** looks up "help" in hash table → O(1) lookup returns CommandInfo*
5. **Parser** validates and structures data into ParsedCommand (args, flags)
6. **Executor** calls strategy: `command_info->function(parsed_cmd)`
7. **Command Implementation** executes specific logic (e.g., print help text)
8. **UI Feedback** displays result with appropriate colors
9. **Cleanup** frees all dynamically allocated memory

**Time Complexity**: O(1) for lookup + O(n) for parsing + O(k) for execution where k = command-specific

---

### 4.2 Event Propagation Flow

How events flow through the system:

```
┌─────────────────────────────────────────────────────────────────┐
│                    EVENT SOURCE (Publisher)                     │
│               event_bus_publish(bus, EVENT_PLAYER_DIED, data)   │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                     EVENT BUS (events.c)                        │
│   1. Look up subscription list: subscriptions[EVENT_PLAYER_DIED]│
│   2. Iterate linked list of Subscription nodes                  │
└─────────────────────────────────────────────────────────────────┘
                                │
                    ┌───────────┴───────────┐
                    ▼                       ▼
         ┌──────────────────┐    ┌──────────────────┐
         │   Subscriber 1   │    │   Subscriber 2   │
         │  UI System       │    │  Audio System    │
         └──────────────────┘    └──────────────────┘
                    │                       │
                    ▼                       ▼
         ┌──────────────────┐    ┌──────────────────┐
         │ callback(event,  │    │ callback(event,  │
         │          userdata)│    │          userdata)│
         └──────────────────┘    └──────────────────┘
                    │                       │
                    ▼                       ▼
         ┌──────────────────┐    ┌──────────────────┐
         │ Display death    │    │ Play death sound │
         │ screen           │    │                  │
         └──────────────────┘    └──────────────────┘
```

**Synchronous vs Asynchronous**:

**Synchronous (immediate)**:
```c
event_bus_publish(bus, EVENT_PLAYER_DIED, &player_data);
// All callbacks executed immediately
// Control returns after all subscribers notified
```

**Asynchronous (queued)**:
```c
// Producer: Queue event
event_bus_queue(bus, EVENT_PLAYER_DIED, &player_data, sizeof(player_data));
// Event data copied, control returns immediately

// Consumer: Dispatch queued events (e.g., at end of frame)
event_bus_dispatch(bus);
// All queued events processed in order
```

**Why both modes?**
- **Synchronous**: Immediate response needed (e.g., UI updates)
- **Asynchronous**: Deferred processing (e.g., spawn enemies next frame)
- **Benefit**: Prevents mid-update state corruption

**Example from events.c**:
```c
// Synchronous publish (lines 191-213)
bool event_bus_publish(EventBus* bus, EventType type, void* data) {
    Subscription* sub = bus->subscriptions[type];
    while (sub) {
        if (sub->active && sub->callback) {
            sub->callback(&event, sub->userdata);  // Immediate call
        }
        sub = sub->next;
    }
    return true;
}

// Asynchronous queue (lines 239-271)
bool event_bus_queue(EventBus* bus, EventType type, const void* data, size_t data_size) {
    // Copy data
    qe->event.data = malloc(data_size);
    memcpy(qe->event.data, data, data_size);
    bus->queue_size++;
    // Return immediately (no callbacks invoked yet)
}
```

---

### 4.3 Game Loop Flow

The main game loop structure:

```
┌───────────────────────────────────────────────────────────┐
│                    GAME LOOP START                        │
│                  game_loop_run(loop, 60)                  │
└───────────────────────────────────────────────────────────┘
                            │
                            ▼
┌───────────────────────────────────────────────────────────┐
│  INITIALIZATION PHASE                                     │
│    - Call: callbacks.on_init(userdata)                    │
│    - Returns false? → Exit immediately                    │
└───────────────────────────────────────────────────────────┘
                            │
                            ▼
        ┌───────────────────────────────────┐
        │   MAIN LOOP (while not stopped)   │
        └───────────────────────────────────┘
                            │
    ┌───────────────────────┴───────────────────────┐
    │                                               │
    ▼                                               │
┌────────────────────────────────┐                 │
│  1. TIMING                     │                 │
│     timing_frame_start()       │                 │
│     - Capture current time     │                 │
│     - Calculate delta_time     │                 │
│     - Clamp to max_delta       │                 │
└────────────────────────────────┘                 │
    │                                               │
    ▼                                               │
┌────────────────────────────────┐                 │
│  2. UPDATE (if RUNNING)        │                 │
│     if state == RUNNING:       │                 │
│       callbacks.on_update(dt)  │                 │
│     if state == PAUSED:        │                 │
│       Skip update              │                 │
└────────────────────────────────┘                 │
    │                                               │
    ▼                                               │
┌────────────────────────────────┐                 │
│  3. RENDER (always)            │                 │
│     callbacks.on_render()      │                 │
│     - Render even when paused  │                 │
│     - Show pause overlay       │                 │
└────────────────────────────────┘                 │
    │                                               │
    ▼                                               │
┌────────────────────────────────┐                 │
│  4. FPS LIMITING               │                 │
│     timing_frame_end(60)       │                 │
│     - Sleep if frame too fast  │                 │
│     - Target: 16.67ms/frame    │                 │
└────────────────────────────────┘                 │
    │                                               │
    ▼                                               │
┌────────────────────────────────┐                 │
│  5. CHECK STATE                │                 │
│     if state == STOPPED:       │                 │
│       Break loop               │                 │
│     else:                      │                 │
│       Continue ───────────────────────────────────┘
└────────────────────────────────┘
    │
    ▼
┌───────────────────────────────────────────────────────────┐
│  CLEANUP PHASE                                            │
│    - Call: callbacks.on_cleanup(userdata)                 │
│    - Free resources                                       │
└───────────────────────────────────────────────────────────┘
```

**Key points**:

1. **Delta time clamping** prevents "spiral of death":
```c
// From game_loop.c (lines 88-91)
if (delta_time > loop->max_delta_time) {
    delta_time = loop->max_delta_time;  // Default: 0.1 seconds
}
```

2. **Render always, update conditionally**:
   - Paused games still render (show pause menu)
   - Only game logic paused, not display

3. **FPS limiting** via sleep:
```c
// timing.c (conceptual)
void timing_frame_end(unsigned int target_fps) {
    double frame_time = 1.0 / target_fps;
    double elapsed = get_time() - frame_start;
    if (elapsed < frame_time) {
        sleep(frame_time - elapsed);
    }
}
```

4. **Frame statistics**:
   - Track FPS for display
   - Count total frames
   - Measure elapsed time

---

## 5. Module Dependencies

### 5.1 Dependency Graph

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│                         APPLICATION                             │
│                         (main.c)                                │
│                                                                 │
└────────────┬────────────────────────────────────┬───────────────┘
             │                                    │
             ▼                                    ▼
    ┌────────────────┐                  ┌────────────────┐
    │  COMMAND       │                  │  CORE          │
    │  SYSTEM        │                  │  (Game Loop)   │
    │                │                  │                │
    │ - Registry     │                  │ - Events       │
    │ - History      │                  │ - State Mgr    │
    │ - Autocomplete │                  │ - Timing       │
    │ - Executor     │                  │ - Memory       │
    └────────┬───────┘                  └────────┬───────┘
             │                                    │
             └────────────┬───────────────────────┘
                          │
                          ▼
             ┌────────────────────────┐
             │    TERMINAL LAYER      │
             │                        │
             │  - ncurses wrapper     │
             │  - Input handler       │
             │  - Colors              │
             │  - UI feedback         │
             └────────────┬───────────┘
                          │
                          ▼
             ┌────────────────────────┐
             │    UTILITIES           │
             │                        │
             │  - Hash Table          │
             │  - Trie                │
             │  - Logger              │
             │  - String Utils        │
             │  - Memory Pools        │
             └────────────────────────┘
                          │
                          ▼
             ┌────────────────────────┐
             │   STANDARD LIBRARY     │
             │   (stdlib, string.h,   │
             │    time.h, etc.)       │
             └────────────────────────┘
```

### 5.2 Dependency Rules

**Layering Rules**:

1. **Utilities → Standard Library ONLY**
   - Hash table, trie, logger have NO project dependencies
   - Reusable in any C project

2. **Subsystems → Utilities ONLY**
   - Core (events, state_manager) depends on logger
   - Commands (registry) depends on hash_table
   - Terminal (ncurses_wrapper) depends on logger

3. **Integration → All Below**
   - command_system.c integrates commands + terminal + core
   - game_loop.c integrates core + user callbacks

4. **Application → All**
   - main.c uses command_system, game_loop, terminal

**Circular Dependency Avoidance**:

**Problem**: Command system needs to access state manager, but state manager might want to dispatch commands.

**Solution**: Global references with initialization order:
```c
// From command_system.c (lines 19-23)
CommandRegistry* g_command_registry = NULL;  // Commands can access
StateManager* g_state_manager = NULL;         // Commands can access

// Application sets these:
command_system_init();
g_state_manager = state_manager_create();  // Set after init
```

**Why this works**:
- Weak coupling via global pointers
- Explicit initialization order
- No circular header dependencies

**Alternative considered**: Dependency injection (passing pointers everywhere)
- **Rejected**: Too verbose for C, increases API complexity

---

## 6. Memory Management Strategy

### 6.1 Ownership Patterns

**Rule**: Whoever allocates is responsible for freeing.

**Pattern 1: Caller Owns**
```c
// Caller allocates and frees
char* buffer = malloc(256);
input_handler_read_line(handler, "> ", buffer, 256);
free(buffer);
```

**Pattern 2: Callee Owns (Return Allocated)**
```c
// Function allocates, returns, caller frees
char** names = NULL;
size_t count;
command_registry_get_all_names(registry, &names, &count);
// ... use names
command_registry_free_names(names, count);  // Matching free function
```

**Pattern 3: Internal Ownership**
```c
// Function owns the memory, returns pointer to internal data
const char* cmd = command_history_get(history, 0);
// Pointer valid until next history modification
// Caller MUST NOT free
```

**Pattern 4: Transfer Ownership**
```c
// Object takes ownership, will free on destroy
hash_table_put(table, "key", allocated_value);
// hash_table_destroy() will free the value
```

### 6.2 Allocation Strategies

**Small, Fixed-Size**: Stack allocation
```c
char buffer[256];  // Small temporary buffers on stack
```

**Variable-Size**: Heap allocation with malloc
```c
char* dynamic = malloc(size);
// ... use
free(dynamic);
```

**Frequent Same-Size**: Memory pools
```c
MemoryPool* pool = pool_create(1024, 100);  // 100 blocks of 1024 bytes
void* block = pool_alloc(pool);
// ... use
pool_free(pool, block);  // Fast O(1) free
```

**Collections**: Hash tables, tries
```c
HashTable* table = hash_table_create(capacity);
// Automatically grows as needed
```

### 6.3 Memory Pools

**Purpose**: Fast, predictable allocation for fixed-size objects

**Use Cases**:
- Game entities (enemies, projectiles)
- Event data
- Temporary buffers

**Implementation** (from memory.h):
```c
typedef struct {
    size_t total_bytes;
    size_t block_size;
    size_t block_count;
    size_t allocated_blocks;
    size_t peak_usage;
} MemoryStats;
```

**API**:
```c
MemoryPool* pool = pool_create(1024, 100);  // 100 KB total
void* ptr = pool_alloc(pool);                // O(1) allocation
pool_free(pool, ptr);                        // O(1) deallocation
pool_reset(pool);                            // Free all at once
pool_check_leaks(pool);                      // Debug: find leaks
pool_destroy(pool);
```

**Benefits**:
- **O(1) allocation/deallocation** (vs malloc's variable time)
- **No fragmentation** (fixed-size blocks)
- **Predictable performance** (no system calls after creation)
- **Leak detection** built-in

**Trade-offs**:
- **Fixed block size** (wasted space if objects smaller)
- **Fixed pool size** (exhaustion possible, but detectable)
- **Not general-purpose** (specific use cases)

**When to use pools**:
- Game entities spawned/destroyed frequently
- Temporary data structures in hot paths
- Real-time requirements (no unpredictable malloc pauses)

**When NOT to use pools**:
- Variable-sized data
- Long-lived objects (waste of pool slots)
- Rarely allocated objects

---

## 7. Error Handling Strategy

### 7.1 Return Code Pattern

**Philosophy**: Errors are expected, handle explicitly.

**Pattern**: Boolean or enum return, output via pointers

```c
// Boolean success/failure
bool hash_table_put(HashTable* table, const char* key, void* value);

if (!hash_table_put(table, "key", value)) {
    // Handle error
}

// Enum status codes
CommandResult execute_command(ParsedCommand* cmd);

CommandResult result = execute_command(cmd);
if (result.status != EXEC_SUCCESS) {
    fprintf(stderr, "Error: %s\n", result.error_message);
}
```

**Why not exceptions?**
- C has no exceptions
- Explicit error checking is C idiom
- Forces caller to consider failure cases

### 7.2 Error Propagation

**Layers**:
1. **Low-level**: Return NULL or false
2. **Mid-level**: Log error, propagate up
3. **High-level**: Display to user

**Example**:
```c
// Low-level: hash_table.c
void* hash_table_get(const HashTable* table, const char* key) {
    if (!table || !key) return NULL;  // Invalid input
    // ... search
    return NULL;  // Not found
}

// Mid-level: registry.c
const CommandInfo* command_registry_get(const CommandRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    return hash_table_get(registry->commands, name);
}

// High-level: executor.c
CommandResult execute_command(ParsedCommand* cmd) {
    const CommandInfo* info = command_registry_get(registry, cmd->name);
    if (!info) {
        LOG_ERROR("Command not found: %s", cmd->name);
        return command_result_error(EXEC_ERROR_INVALID_COMMAND,
                                   "Command not found");
    }
    // ...
}

// Application: main.c
CommandResult result = command_system_execute(input);
if (!result.success) {
    ui_feedback_show_error(result.error_message);  // Display to user
}
```

### 7.3 Logging Integration

**All errors are logged** before propagating:

```c
// From events.c (line 124)
if (!bus || type <= EVENT_NONE || type >= EVENT_COUNT || !callback) {
    LOG_ERROR("Invalid event bus, type, or callback");
    return 0;
}
```

**Benefits**:
- Error traces in log file for debugging
- Contextual information (file, line, function)
- Separate from user-facing messages

**Log Levels**:
- **ERROR**: Recoverable errors (command failed)
- **WARN**: Unexpected but handled (duplicate command)
- **INFO**: Normal operations (command executed)
- **DEBUG**: Detailed trace (for development)

---

## 8. Initialization and Shutdown

### 8.1 Startup Sequence

**Order matters** - dependencies must be initialized before dependents.

```
1. Logger Init
   │
   ├─> logger_init("game.log", LOG_LEVEL_DEBUG)
   │   - Open log file
   │   - Set log level
   │
2. Timing Init
   │
   ├─> timing_init()
   │   - Initialize high-resolution timer
   │
3. Terminal Init
   │
   ├─> term_init()
   │   - Initialize ncurses
   │   - Set non-blocking input
   │   - Hide cursor
   │
4. Colors Init
   │
   ├─> colors_init()
   │   - Detect color support
   │   - Initialize color pairs
   │
5. Command System Init
   │
   ├─> command_system_init()
   │   ├─> Create registry (hash table)
   │   ├─> Register built-in commands
   │   ├─> Create input handler
   │   │   ├─> Create history (circular buffer)
   │   │   ├─> Create autocomplete (trie)
   │   │   └─> Load history from disk
   │   └─> Initialize UI feedback
   │
6. Core Systems Init
   │
   ├─> state_manager = state_manager_create()
   ├─> event_bus = event_bus_create()
   │
7. Register States
   │
   ├─> state_manager_register(sm, STATE_MAIN_MENU, &menu_callbacks, userdata)
   ├─> state_manager_register(sm, STATE_GAME_WORLD, &game_callbacks, userdata)
   │
8. Set Global References
   │
   ├─> g_state_manager = state_manager
   │
9. Push Initial State
   │
   ├─> state_manager_push(sm, STATE_MAIN_MENU)
   │
10. Create Game Loop
    │
    ├─> game_loop = game_loop_create(&loop_callbacks, userdata)
    │
11. Run Game Loop
    │
    └─> game_loop_run(game_loop, 60)  // Blocks until quit
```

**Example from main.c** (lines 86-127):
```c
int main(void) {
    // 1. Logger
    logger_init("necromancer_shell.log", LOG_LEVEL_DEBUG);

    // 2. Timing
    timing_init();

    // 3. Terminal
    if (!term_init()) {
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // 4. Colors
    colors_init();

    // 5. Memory pool (example)
    MemoryPool* demo_pool = pool_create(256, 50);

    // 6. Application state
    DemoState state = { /* ... */ };

    // 7. Main loop
    while (state.running) {
        // Game loop inline (will use game_loop.c in full version)
    }

    // Cleanup (see 8.2)
}
```

**Why this order?**
- Logger first (needed by all other systems)
- Terminal early (needed for display)
- Command system before game logic (commands control game)
- State manager before loop (loop queries state)

### 8.2 Shutdown Sequence

**Reverse order of initialization** - destroy dependents before dependencies.

```
1. Stop Game Loop
   │
   ├─> game_loop_stop(loop)
   │   - Sets loop->state = STOPPED
   │   - Loop exits after current frame
   │
2. Cleanup Callback
   │
   ├─> callbacks.on_cleanup(userdata)
   │   - Game-specific cleanup
   │
3. Destroy Game Loop
   │
   ├─> game_loop_destroy(loop)
   │
4. Pop All States
   │
   ├─> state_manager_clear(sm)
   │   - Calls on_exit for all states
   │
5. Destroy State Manager
   │
   ├─> state_manager_destroy(sm)
   │
6. Destroy Event Bus
   │
   ├─> event_bus_destroy(bus)
   │   - Frees all subscriptions
   │   - Frees queued events
   │
7. Command System Shutdown
   │
   ├─> command_system_shutdown()
   │   ├─> Save history to disk
   │   ├─> Destroy input handler
   │   │   ├─> Destroy autocomplete (trie)
   │   │   └─> Destroy history (circular buffer)
   │   ├─> Destroy registry (hash table)
   │   │   └─> Free all CommandInfo
   │   └─> Shutdown UI feedback
   │
8. Destroy Memory Pools
   │
   ├─> pool_check_leaks(pool)  // Debug: log leaks
   ├─> pool_destroy(pool)
   │
9. Terminal Shutdown
   │
   ├─> term_shutdown()
   │   - Restore terminal state
   │   - End ncurses
   │
10. Logger Shutdown
    │
    └─> logger_shutdown()
        - Flush log buffer
        - Close log file
```

**Example from main.c** (lines 170-176):
```c
// Cleanup
pool_check_leaks(demo_pool);
pool_destroy(demo_pool);
term_shutdown();
logger_shutdown();
```

**Critical**: Destroy in reverse order to avoid:
- Use-after-free (accessing destroyed objects)
- Memory leaks (forgetting to free)
- Dangling pointers (referencing freed memory)

---

## 9. Extensibility Points

### 9.1 Command Registration

**How to add a new command**:

1. **Create command implementation** (e.g., `cmd_mycommand.c`):

```c
#include "commands.h"

CommandResult cmd_mycommand(ParsedCommand* cmd) {
    // Command logic here
    return command_result_success("Command executed");
}
```

2. **Define command info** (e.g., in `builtin.c`):

```c
static FlagDefinition mycommand_flags[] = {
    { .name = "verbose", .short_name = 'v', .type = ARG_TYPE_BOOL,
      .required = false, .description = "Verbose output" }
};

static CommandInfo mycommand_info = {
    .name = "mycommand",
    .description = "Does something useful",
    .usage = "mycommand [--verbose]",
    .help_text = "Detailed help text here...",
    .function = cmd_mycommand,
    .flags = mycommand_flags,
    .flag_count = 1,
    .min_args = 0,
    .max_args = 0,
    .hidden = false
};
```

3. **Register command**:

```c
command_registry_register(registry, &mycommand_info);
```

**Automatic benefits**:
- Appears in `help` command
- Autocomplete works automatically (trie updated)
- Parser validates arguments/flags
- History stores command

**No modifications needed** to:
- Registry code
- Executor code
- Parser code
- Autocomplete code

**This is the Strategy Pattern in action** - add new behavior without modifying existing code.

### 9.2 Event Subscriptions

**How to listen for events**:

```c
// Define callback
void on_player_died(const Event* event, void* userdata) {
    PlayerData* player = (PlayerData*)event->data;
    printf("Player %s died!\n", player->name);
}

// Subscribe
EventBus* bus = get_event_bus();
size_t sub_id = event_bus_subscribe(bus, EVENT_PLAYER_DIED,
                                    on_player_died, NULL);

// Publish event (somewhere else in code)
PlayerData player = { .name = "Hero", .health = 0 };
event_bus_publish(bus, EVENT_PLAYER_DIED, &player);

// Unsubscribe when done
event_bus_unsubscribe(bus, sub_id);
```

**Decoupling**:
- Publisher doesn't know about subscribers
- Subscribers don't know about each other
- Easy to add new event handlers
- Easy to remove handlers

**Custom events**:
```c
// Add to events.h
typedef enum {
    // ... existing events
    EVENT_CUSTOM_MY_EVENT = EVENT_CUSTOM_START,
    EVENT_CUSTOM_ANOTHER = EVENT_CUSTOM_START + 1,
    // ...
} EventType;
```

### 9.3 Custom Data Structures

**Utilities are designed for reuse**:

**Hash Table**:
```c
// Use for any string-keyed dictionary
HashTable* player_items = hash_table_create(100);
hash_table_put(player_items, "sword", sword_ptr);
Item* sword = hash_table_get(player_items, "sword");
```

**Trie**:
```c
// Use for any prefix-based search
Trie* locations = trie_create();
trie_insert(locations, "Necropolis");
trie_insert(locations, "Necromancer's Tower");
char** matches = NULL;
size_t count;
trie_find_with_prefix(locations, "Necro", &matches, &count);
// matches = ["Necropolis", "Necromancer's Tower"]
```

**Memory Pool**:
```c
// Use for frequently allocated/deallocated objects
MemoryPool* enemy_pool = pool_create(sizeof(Enemy), 200);
Enemy* enemy = pool_alloc(enemy_pool);
// ... use enemy
pool_free(enemy_pool, enemy);
```

---

## 10. Performance Considerations

### 10.1 Time Complexity Goals

**Critical paths** (executed every frame or per command):

| Operation | Goal | Achieved | Data Structure |
|-----------|------|----------|----------------|
| Command lookup | O(1) | Yes | Hash table |
| Command execution | O(1) dispatch | Yes | Function pointer |
| History add | O(1) | Yes | Circular buffer |
| Autocomplete | O(m + k) | Yes | Trie (m=prefix, k=matches) |
| Event publish | O(n) subscribers | Yes | Linked list per type |
| Memory alloc (pool) | O(1) | Yes | Free list |
| State update | O(1) | Yes | Direct callback |

**Non-critical paths** (infrequent):

| Operation | Complexity | Acceptable |
|-----------|------------|------------|
| Registry init | O(n) | Yes (once at startup) |
| Autocomplete rebuild | O(n*m) | Yes (after registration changes) |
| History save | O(n) | Yes (once at shutdown) |
| Event queue grow | O(n) | Yes (amortized O(1)) |

### 10.2 Memory Efficiency

**Design decisions for memory**:

1. **Circular buffers** over unbounded lists
   - Command history: Fixed 100-1000 entries
   - Event queue: Capped at MAX_EVENT_QUEUE (1024)

2. **Memory pools** for fixed-size allocations
   - Eliminates fragmentation
   - Predictable memory footprint

3. **Trie space trade-off**
   - Higher space usage than hash table
   - Acceptable: Command names are small dataset (~50 commands)
   - Benefit: Autocomplete performance

4. **Opaque pointers**
   - No exposed struct sizes
   - Users can't stack-allocate (preventing waste)

5. **String deduplication**
   - Command names stored once in registry
   - Pointers reused in autocomplete, help, etc.

**Typical memory usage** (estimated):

```
Component           Memory
--------------------------------
Hash table          ~10 KB (100 entries)
Trie                ~50 KB (50 commands, avg 8 chars)
Command history     ~50 KB (500 entries * 100 bytes avg)
Event bus           ~20 KB (100 subscriptions)
Memory pool         User-defined (e.g., 100 KB)
ncurses buffers     ~50 KB
--------------------------------
Total               ~280 KB + user data
```

**This is tiny** by modern standards - suitable even for embedded systems.

### 10.3 Frame Rate Control

**Goal**: Consistent 60 FPS (16.67ms per frame)

**Implementation**:
```c
// From game_loop.c (lines 77-119)
while (loop->state != LOOP_STATE_STOPPED) {
    timing_frame_start();  // Mark frame start

    // Update (variable time)
    double delta_time = timing_get_delta();
    if (loop->state == LOOP_STATE_RUNNING) {
        loop->callbacks.on_update(delta_time, loop->userdata);
    }

    // Render (variable time)
    loop->callbacks.on_render(loop->userdata);

    // Sleep to maintain target FPS
    timing_frame_end(target_fps);
}
```

**Frame time budget** (60 FPS):
```
Total frame time: 16.67ms
├─ Update:         ~5ms (game logic)
├─ Render:         ~5ms (terminal drawing)
├─ Event dispatch: ~1ms
└─ Sleep:          ~5ms (to maintain FPS)
```

**Delta time clamping** prevents spiral of death:
```c
if (delta_time > loop->max_delta_time) {
    delta_time = 0.1;  // Cap at 100ms (10 FPS minimum)
}
```

**Why?** If frame takes too long (e.g., 200ms), next update tries to catch up, making next frame even slower. Clamping prevents this.

---

## 11. Platform Portability

### 11.1 Abstraction Layers

**Terminal abstraction** (`ncurses_wrapper.h`):
- Hides ncurses details
- Allows future replacement (termbox, custom renderer)
- Platform: Linux, macOS, Windows (via PDCurses)

**File I/O abstraction**:
- POSIX functions (`fopen`, `fclose`, `stat`)
- Windows compatibility via MinGW or Cygwin

**Timing abstraction** (`timing.h`):
- Uses platform-specific high-resolution timers
- Linux: `clock_gettime(CLOCK_MONOTONIC)`
- Windows: `QueryPerformanceCounter`
- macOS: `mach_absolute_time`

### 11.2 Platform-Specific Code

**Conditional compilation**:
```c
// From history.c (lines 1-2)
#define _POSIX_C_SOURCE 200809L  // Enable POSIX features

#ifdef _WIN32
    // Windows-specific code
#else
    // POSIX code
#endif
```

**Platform-specific features**:
- **POSIX**: `getpwuid`, `strdup`, `chmod`
- **Windows**: Requires MinGW or polyfills

**Build system** (Makefile):
- Detects platform via `uname`
- Links appropriate libraries:
  - Linux: `-lncurses`
  - macOS: `-lncurses`
  - Windows: `-lpdcurses`

---

## 12. Design Decisions

### 12.1 C Language Choice

**Why C?**

**Pros**:
- **Performance**: Close to metal, minimal overhead
- **Control**: Explicit memory management
- **Portability**: ANSI C runs everywhere
- **Learning**: Understanding low-level systems
- **Simplicity**: No complex language features (templates, inheritance)

**Cons**:
- **Manual memory management**: More code, more bugs
- **No STL**: Must implement data structures
- **Verbose**: More boilerplate than C++/Rust

**Alternative considered**: C++
- **Rejected**: Want to focus on fundamentals, avoid OOP complexity
- C++ would enable RAII, templates, STL (faster development)

**Alternative considered**: Rust
- **Rejected**: Steeper learning curve, less portable
- Rust would provide memory safety, modern features

**Conclusion**: C is appropriate for:
- Educational purposes (understanding fundamentals)
- Terminal applications (ncurses is C)
- Retro game feel (matches aesthetic)

### 12.2 ncurses for Terminal UI

**Why ncurses?**

**Pros**:
- **Industry standard** for terminal UIs (40+ years)
- **Cross-platform** (Linux, macOS, Windows via PDCurses)
- **Feature-rich**: Colors, box drawing, input handling
- **Well-documented**: Extensive examples and guides

**Cons**:
- **Old API**: C-style, not type-safe
- **Global state**: Not thread-safe
- **Complex initialization**: Many setup steps

**Alternative considered**: termbox
- **Rejected**: Less mature, smaller community
- Simpler API but fewer features

**Alternative considered**: Custom renderer
- **Rejected**: Massive undertaking, reinventing wheel
- Would allow more control but not worth effort

**Conclusion**: ncurses is the right choice for terminal games.

### 12.3 Hash Table for Command Registry

**Why hash table?**

**O(1) average case** lookup is critical:
- Commands executed frequently (user input loop)
- Must feel instant (no lag)

**Alternative considered**: Sorted array + binary search
- **Rejected**: O(log n) lookup is slower
- O(n) insertion (need to maintain sort order)
- Would work for small command sets (<100)

**Alternative considered**: Trie
- **Rejected**: Higher memory usage
- Trie is used for autocomplete (where prefix matching is needed)
- Hash table for exact match is more efficient

**Conclusion**: Hash table is optimal for command name → CommandInfo mapping.

### 12.4 Event Queue vs Direct Dispatch

**Why both?**

**Synchronous publish** (`event_bus_publish`):
- **Use case**: Immediate response needed
- **Example**: UI needs to update right now
- **Benefit**: Simple, predictable

**Asynchronous queue** (`event_bus_queue` + `event_bus_dispatch`):
- **Use case**: Defer processing to avoid mid-frame state changes
- **Example**: Enemy spawns at end of frame (not during player update)
- **Benefit**: Prevents race conditions

**Alternative considered**: Only synchronous
- **Rejected**: Can cause state corruption
- Example: Event handler triggers another event, which triggers another...

**Alternative considered**: Only asynchronous
- **Rejected**: Adds latency where not needed
- UI updates would lag by one frame

**Conclusion**: Hybrid approach provides flexibility - developer chooses based on use case.

---

## Summary

**Necromancer's Shell** is architected as a **modular, layered system** with:

1. **Clear separation of concerns**: Core, Commands, Terminal, Utilities
2. **Proven design patterns**: Singleton, Strategy, Observer, Factory
3. **Performance-oriented**: O(1) lookups, memory pools, fixed frame rate
4. **Maintainable**: Opaque pointers, consistent error handling, extensive logging
5. **Extensible**: Easy to add commands, events, states without modifying core

The architecture prioritizes:
- **Performance**: Real-time frame rates, fast command execution
- **Modularity**: Independent components, minimal coupling
- **Reliability**: Explicit error handling, memory leak detection
- **Simplicity**: C idioms, straightforward data structures

This design enables building complex terminal-based games while maintaining code quality and performance.

---

**For Developers**:

When extending the system:
1. Follow the layered dependency model
2. Use existing data structures (hash table, trie, pools)
3. Add logging to all error paths
4. Document public APIs in headers
5. Write cleanup code for every allocation
6. Consider performance implications (hot paths)
7. Test with valgrind for memory leaks

**The architecture is designed to grow** - add new commands, events, and states without modifying the foundation.

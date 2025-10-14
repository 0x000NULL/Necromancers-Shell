# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Necromancer's Shell** is a terminal-based dark fantasy RPG where necromancy meets system administration. Players command undead armies through Unix-like terminal commands in an ncurses interface.

The game combines:
- Command-driven gameplay (raise, harvest, scan, etc.)
- Deep narrative with moral choices and corruption mechanics
- Resource management (soul energy economy)
- Turn-based combat through typed commands
- Multiple endings based on player decisions

## Build Commands

```bash
# Build release version (optimized)
make release

# Build debug version with address/undefined sanitizers
make debug

# Run all tests
make test

# Run specific test
./build/test_test_tokenizer
./build/test_test_trie
./build/test_test_history

# Check for memory leaks
make valgrind

# Static analysis with cppcheck
make analyze

# Format code with clang-format
make format

# Clean all build artifacts
make clean
```

**Important:** The project has TWO source directories:
- `src/` - Base foundation (core, terminal, utils - Phase 0)
- `necromancers_shell/src/` - Extended implementation with command system (Phase 1)

The active development Makefile is at `necromancers_shell/Makefile` and includes the command system.

## Architecture

### Core Systems (src/core/)

**State Manager** - Game state machine with state stack for pause/resume
- States: INIT, MAIN_MENU, GAME_WORLD, DIALOGUE, INVENTORY, PAUSE, SHUTDOWN
- Callbacks: on_enter, on_exit, on_update, on_render, on_pause, on_resume

**Memory Manager** - Pool-based allocator with leak tracking
- MemoryPool for efficient small allocations
- Debug mode tracks all allocations/frees
- Memory statistics available

**Event System** - Pub/sub event bus for game events
- Event types: INPUT, COMBAT, DIALOGUE, STATE_CHANGE, etc.
- Subscribers receive events by type
- Priority-based event handling

**Game Loop** - Fixed timestep with interpolation
- 60 FPS target (configurable)
- Delta time for frame-rate independent logic
- Timing utilities (timing.h) for stopwatch, delays, frame timing

### Terminal Interface (src/terminal/)

**ncurses Wrapper** - Cross-platform terminal abstraction
- Color pair management (8 predefined game pairs)
- Box drawing, text printing
- Input handling with timeout support

**Colors** - ANSI color definitions and game color pairs
- UI_BORDER, UI_HIGHLIGHT, TEXT_NORMAL, TEXT_DIM, TEXT_ERROR, TEXT_SUCCESS, TEXT_WARNING, TEXT_INFO

### Command System (necromancers_shell/src/commands/)

Phase 1 MVP - Production-ready, fully tested (~5,500 LOC)

**Architecture Flow:**
```
Input → Tokenizer → Parser → Executor → CommandResult
              ↓         ↓
           History  Registry (O(1) lookup)
              ↓
         Autocomplete (Trie-based)
```

**Components:**
- **command_system.h/c** - Top-level API, manages global registry and history
- **tokenizer.h/c** - Advanced parsing with quotes ('literal', "escaped\n"), escape sequences
- **registry.h/c** - Command registry with hash table (O(1) lookup), metadata, validation
- **parser.h/c** - Type-safe argument parser with flags (--verbose, -v), type validation
- **executor.h/c** - Command execution with result handling
- **history.h/c** - Circular buffer (100 capacity), persists to ~/.necromancers_shell_history
- **autocomplete.h/c** - Trie-based prefix completion (O(k+m) where k=prefix, m=matches)
- **input_handler.h/c** - Integrated line editing with history/autocomplete
- **ui_feedback.h/c** - Formatted ANSI color output (success=green, error=red, etc.)

**Built-in Commands:**
- `help [command]` - Display command help
- `status [--verbose]` - Show game state
- `quit` / `exit` - Exit gracefully
- `clear` - Clear screen
- `log <level> [--file <path>]` - Manage logging (levels: trace, debug, info, warn, error, fatal)

**Registering New Commands:**
```c
// 1. Define command function
CommandResult cmd_summon(ParsedCommand* cmd) {
    const char* creature = parsed_command_get_arg(cmd, 0);
    // ... implementation
    return command_result_success("Summoned!");
}

// 2. Define flags
static FlagDefinition summon_flags[] = {
    {
        .name = "level",
        .short_name = 'l',
        .type = ARG_TYPE_INT,
        .required = false,
        .description = "Creature level"
    }
};

// 3. Register command
CommandInfo summon = {
    .name = "summon",
    .description = "Summon an undead creature",
    .usage = "summon <creature> [--level <n>]",
    .help_text = "Summons the specified undead creature.",
    .function = cmd_summon,
    .flags = summon_flags,
    .flag_count = 1,
    .min_args = 1,
    .max_args = 1,
    .hidden = false
};
command_system_register_command(&summon);
```

### Utilities (src/utils/)

**Logger** - File/console logging with levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
**Hash Table** - Generic hash table used by command registry
**String Utils** - String manipulation utilities
**Trie** - Prefix tree for autocomplete (ASCII only, 128 chars)

## Memory Management Rules

**Every component follows strict conventions:**
- Every `*_create()` has corresponding `*_destroy()`
- Every `malloc()` is NULL-checked
- All allocated strings are freed
- Use `make valgrind` to verify zero leaks
- Debug builds enable address sanitizer and undefined behavior sanitizer

## Error Handling

**Comprehensive error reporting throughout:**
- Tokenization errors: unclosed quotes, invalid escapes
- Parse errors: unknown commands, invalid flags, wrong arg counts
- Execution errors: command failures, permission denied
- All errors return human-readable messages

## Code Standards

**Required compiler flags:**
- `-std=c11` - C11 standard
- `-Wall -Wextra -Werror -pedantic` - Strict warnings (zero warnings required)
- `-O2 -march=native` - Optimization (release)
- `-g -O0 -fsanitize=address,undefined` - Debug with sanitizers

**Style conventions:**
- Functions: `snake_case` (e.g., `command_system_init`)
- Types: `PascalCase` (e.g., `CommandRegistry`)
- Enums: `UPPER_SNAKE_CASE` (e.g., `STATE_MAIN_MENU`)
- Keep functions under 100 lines
- Comprehensive comments for non-obvious logic

## Testing

**Test files in tests/ directory:**
- Use existing patterns (test_tokenizer.c, test_trie.c, test_history.c)
- Tests exclude main.o from linking
- Run `make test` to build and execute all tests
- 13/13 tests currently passing

## Game Design Context

**Resource Economy:**
- Soul energy is primary currency (types: common, warrior, mage, innocent, corrupted, ancient)
- Corruption system tracks moral choices (0-100%, affects gameplay and endings)
- Territory control provides resources (graveyards, battlefields, villages, crypts, ritual sites)

**Command Gameplay:**
- Necromantic operations: raise, bind, banish, harvest
- Network operations: scan, probe, connect, trace (Death Network)
- Soul management: souls, soulforge, transfer
- Spell system: cast, enchant, curse, ward
- Intelligence: listen, possess, memory, scry
- System management: status, log, upgrade, debug

**Three Ending Paths:**
1. Revenant Route - Resurrect yourself, restore humanity
2. Lich Lord Route - Embrace undeath, conquer living
3. Reaper Route - Become death's administrator, balance life/death

## Development Phases

**Phase 0 (Complete):** Core infrastructure - memory, logging, terminal, events, state manager
**Phase 1 (Complete):** Command system MVP - tokenizer, parser, executor, history, autocomplete
**Phase 2 (Complete):** Core game systems - souls, resources, locations, minions, corruption
**Phase 3+ (Planned):** Raw terminal mode, arrow key navigation, command aliases, piping, combat

## External Dependencies

**Required:**
- ncurses library (`-lncurses`)
- Math library (`-lm`)
- Standard C11 compiler (GCC or Clang)

**Platforms:**
- Linux (primary)
- macOS (supported)
- Windows with PDCurses (planned)

## Phase 2: Core Game Systems (COMPLETE)

**Status:** Fully implemented, tested, and playable

### Systems Implemented

**Soul System (Week 7):**
- 6 soul types: common, warrior, mage, innocent, corrupted, ancient
- Soul quality (0-100) affects energy value
- Soul binding to minions for stat bonuses
- Dynamic filtering and sorting
- 36 unit tests

**Resource Management (Week 7):**
- Soul energy economy
- Mana system with regeneration (10 per hour)
- Corruption tracking (0-100%, 5 levels: Pure, Tainted, Corrupted, Vile, Abyssal)
- Time progression (24-hour days)

**Location/Territory System (Week 8):**
- 5 location types: graveyard, battlefield, village, crypt, ritual_site
- Location discovery mechanics
- Corpse availability per location
- Territory control system
- 18 unit tests

**Minion System (Week 9):**
- 6 minion types: zombie, skeleton, ghoul, wraith, wight, revenant
- Unique stats per type (HP, attack, defense, speed, loyalty)
- Soul binding improves minion stats (10% bonus from soul quality)
- Experience and leveling system (5% per level, minimum +1)
- 13 unit tests

### Commands Implemented (9 playable)

**Soul Management:**
- `souls [filters] [--sort criteria]` - View and filter soul inventory
- `harvest [--count n]` - Harvest souls from corpses at current location

**Minion Management:**
- `raise <type> [name] [--soul id]` - Raise undead minion (costs soul energy)
- `bind <minion_id> <soul_id>` - Bind soul to minion for stat bonus
- `banish <minion_id>` - Destroy minion, return soul to pool
- `minions` - List all raised minions with stats

**Exploration:**
- `scan` - View connected locations from current position
- `probe <location>` - Get location details, discover new areas
- `connect <location>` - Travel to connected location (takes 1-3 hours)

**System:**
- `status [--verbose]` - View all game state (updated with Phase 2 info)

### Statistics

- **67+ unit tests** passing (36 souls + 18 territory + 13 minions)
- **~10,000 lines** of production C code
- **Zero memory leaks** (valgrind verified)
- **Zero compiler warnings** (strict flags: -Wall -Wextra -Werror -pedantic)
- **9 playable game commands** + 4 built-in system commands

### File Structure

```
necromancers_shell/src/game/
├── game_state.h/c           - Central game state manager
├── souls/
│   ├── soul.h/c             - Soul structure and operations
│   └── soul_manager.h/c     - Soul collection management
├── resources/
│   ├── resources.h/c        - Soul energy, mana, time
│   └── corruption.h/c       - Corruption tracking system
├── world/
│   ├── location.h/c         - Location structure and operations
│   └── territory.h/c        - Territory/world manager
└── minions/
    ├── minion.h/c           - Minion structure and operations
    └── minion_manager.h/c   - Minion army management
```

## Important Notes

- The `necromancers_shell/` directory contains the active implementation with Phases 0-2 complete
- The root `src/` directory has the base Phase 0 foundation only
- Use `necromancers_shell/Makefile` for building the full system
- Global variables used: `g_state_manager`, `g_command_registry`, `g_game_state` (defined in main.c)
- All systems are production-ready and fully tested
- See `PHASE2_COMPLETE.md` for detailed implementation notes

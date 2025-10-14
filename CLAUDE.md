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

# Display version information
make version

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

## CI/CD Pipeline

**Status:** Fully operational with automated versioning, testing, and releases

### Version Management

**Files:**
- `necromancers_shell/VERSION` - Single source of truth (plain text: "0.3.0")
- `src/core/version_info.h` - Auto-generated at build time (gitignored)
- `src/core/version.h` - Public API for version access
- `src/core/version.c` - Implementation

**Version API:**
```c
#include "core/version.h"

const char* version_get_string(void);      // "0.3.0"
int version_get_major(void);                // 0
int version_get_minor(void);                // 3
int version_get_patch(void);                // 0
const char* version_get_build_date(void);   // "2025-10-14"
const char* version_get_git_hash(void);     // "46516f4"
void version_print_full(FILE* stream);      // Full version info
```

**Command-line:**
```bash
./build/necromancer_shell --version   # Display version
./build/necromancer_shell --help      # Display help
make version                          # Show version during build
```

### GitHub Actions Workflows

**Location:** `.github/workflows/`

#### CI Workflow (`ci.yml`)
**Triggers:** All branches, PRs to master
**Platforms:** Linux (ubuntu-latest), macOS ARM64 (macos-15), macOS Intel (macos-15-large)
**Jobs:**
1. Install platform-specific dependencies (ncurses)
2. Build release and debug versions
3. Run unit tests (67+ tests)
4. Static analysis with cppcheck (Linux only)
5. Memory leak detection with valgrind (Linux only)
6. Upload build artifacts (7-day retention)

**Environment variable:**
- Set `CI=true` to build portable binaries (skips `-march=native`)

#### Release Workflow (`release.yml`)
**Triggers:** Push to master (skips if commit has `[skip release]`)
**Permissions:** `contents: write`
**Jobs:**

1. **Version Bump:**
   - Read current VERSION file
   - Determine bump type from commit message:
     - `[major]` or `BREAKING` → Major bump (1.0.0 → 2.0.0)
     - `[minor]` or `feat:` → Minor bump (1.0.0 → 1.1.0)
     - Default → Patch bump (1.0.0 → 1.0.1)
   - Update VERSION file
   - Create git tag (e.g., `v1.0.1`)
   - Push changes back to repository

2. **Multi-Platform Build:**
   - Build on Linux, macOS ARM64, macOS Intel in parallel
   - Run full test suite on each platform
   - Package binaries as `.tar.gz` with README and VERSION

3. **Create GitHub Release:**
   - Download all platform artifacts
   - Generate SHA256 checksums
   - Create GitHub release with:
     - Tag: `v1.0.1`
     - Auto-generated release notes
     - Attached artifacts for all platforms
     - Checksums file

### Cross-Platform Support

**Platform Detection in Makefile:**
```makefile
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(UNAME_S),Darwin)
    # macOS: Use Homebrew ncurses
    NCURSES_PREFIX := $(shell brew --prefix ncurses)
    LIBS := -L$(NCURSES_PREFIX)/lib -lncurses -lm
else ifeq ($(UNAME_S),Linux)
    # Linux: Use system ncurses
    LIBS := -lncurses -lm
else
    # Windows: Use PDCurses
    LIBS := -lpdcurses -lm
endif
```

**Platform Compatibility Layer:**
- `src/terminal/platform_curses.h` - Abstracts ncurses/PDCurses
- Automatically includes correct header:
  - Linux/macOS: `#include <ncurses.h>`
  - Windows: `#include <curses.h>` (PDCurses)
- No platform-specific #ifdef blocks needed in main code

**Current Platform Status:**
- ✅ **Linux x64:** Fully supported, tested in CI
- ✅ **macOS ARM64:** Fully supported (Apple Silicon)
- ✅ **macOS Intel:** Fully supported (legacy until 2027)
- ⚠️ **Windows x64:** Infrastructure ready, PDCurses support planned (Phase 3)

### Portability Changes

**Removed `-march=native` flag:**
- Was: `release: CFLAGS += -O2 -DNDEBUG -march=native`
- Now: `release: CFLAGS += -O2 -DNDEBUG` (adds `-march=native` only if `CI` is not set)
- Reason: `-march=native` creates CPU-specific binaries that don't work on different processors
- Impact: Negligible performance difference (~2-5%)

### Release Artifacts

**Naming Convention:**
```
necromancer_shell-{platform}-{arch}-v{version}.tar.gz
```

**Examples:**
- `necromancer_shell-linux-x64-v0.4.0.tar.gz`
- `necromancer_shell-macos-arm64-v0.4.0.tar.gz`
- `necromancer_shell-macos-x64-v0.4.0.tar.gz`
- `SHA256SUMS` - Checksums for all artifacts

**Package Contents:**
```
necromancer_shell-linux-x64-v0.4.0/
├── necromancer_shell    # Binary
├── README.md            # Documentation
└── VERSION              # Version file
```

### Commit Message Conventions

Control version increments via commit messages:

```bash
# Patch bump (0.3.0 → 0.3.1) - Default
git commit -m "fix: Fix bug in soul harvesting"
git commit -m "docs: Update documentation"

# Minor bump (0.3.0 → 0.4.0)
git commit -m "feat: Add new minion type"
git commit -m "Add feature [minor]"

# Major bump (0.3.0 → 1.0.0)
git commit -m "BREAKING: Redesign command system"
git commit -m "Major refactor [major]"

# Skip release (version bump only, no commit)
git commit -m "chore: bump version to 0.4.0 [skip release]"
```

### Monitoring CI/CD

**View workflows:**
- Actions tab: https://github.com/0x000NULL/Necromancers-Shell/actions
- Releases: https://github.com/0x000NULL/Necromancers-Shell/releases

**Build times:**
- CI workflow: ~2-3 minutes per platform
- Release workflow: ~5-7 minutes total (parallel builds)

**Caching:**
- Homebrew packages on macOS (~30s savings)
- No object file caching (build is fast enough without it)

### Developer Workflow

**Local development:**
```bash
# Build with local optimizations
make release

# Build for CI (portable)
CI=true make release

# Check version
make version
./build/necromancer_shell --version
```

**Creating releases:**
1. Commit and push to master
2. CI runs automatically on push
3. If CI passes, release workflow triggers
4. Version auto-incremented based on commit message
5. Binaries built for all platforms
6. GitHub release created automatically

**No manual intervention needed!**

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
**CI/CD (Complete):** Automated versioning, multi-platform builds, GitHub Actions workflows
**Phase 3+ (Planned):** Windows PDCurses support, raw terminal mode, arrow key navigation, command aliases, piping, combat

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

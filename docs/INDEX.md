# Necromancer's Shell - Documentation Index

Welcome to the comprehensive documentation for Necromancer's Shell, a terminal-based dark fantasy RPG where necromancy meets system administration.

---

## Table of Contents

1. [Welcome](#welcome)
2. [Quick Start](#quick-start)
3. [Documentation Organization](#documentation-organization)
4. [Core Documentation](#core-documentation)
5. [Module Documentation](#module-documentation)
6. [Project Documentation](#project-documentation)
7. [Quick Reference](#quick-reference)
8. [Documentation Statistics](#documentation-statistics)
9. [Contributing](#contributing)

---

## Welcome

### What is Necromancer's Shell?

Necromancer's Shell is a command-line RPG that combines dark fantasy storytelling with Unix-like terminal commands. Play as a necromancer managing the dead through a sophisticated shell interface, where every command you type shapes your journey through a morally complex world of death magic and consequences.

### Key Features

- Terminal-based interface using ncurses
- Command-driven gameplay with Unix-inspired commands
- Deep narrative with meaningful moral choices
- Resource management and character progression
- Turn-based tactical combat
- Multiple endings based on player decisions
- Production-quality C implementation with zero tolerance for memory leaks

### Development Status

**Phase 0: Foundation** - COMPLETE
- Core infrastructure (memory management, logging, terminal interface, event system)

**Phase 1: Command System MVP** - COMPLETE
- Advanced tokenizer with quote and escape handling
- Command registry with O(1) lookup
- Type-safe argument parser with flags
- Persistent command history
- Trie-based autocomplete system
- Five built-in commands
- Comprehensive test suite (13/13 tests passing)

**Phase 2+: Game Systems** - Planned
- Game world, entities, and narrative system
- Combat and quest systems
- Save/load and progression systems

---

## Quick Start

### Building the Project

```bash
# Debug build with sanitizers
make debug

# Release build (optimized)
make release

# Run tests
make test

# Check for memory leaks
make valgrind
```

### Running the Shell

```bash
# After building
./build/necromancers_shell

# Try these commands
necromancer> help
necromancer> status
necromancer> quit
```

### First Commands to Try

- `help` - Display all available commands
- `help <command>` - Get detailed help for a specific command
- `status` - View current system status
- `log info` - Set logging level
- `clear` - Clear the screen

---

## Documentation Organization

### Structure

The documentation is organized into three tiers:

1. **Core Documentation** - Architecture, APIs, and testing strategies
2. **Module Documentation** - Detailed guides for each system component
3. **Project Documentation** - Design documents, plans, and milestone reports

### Recommended Reading Order

#### For New Developers

Start here to understand the system:

1. [Architecture and Design](ARCHITECTURE.md) - Understand the overall design
2. [Core Systems](CORE_SYSTEMS.md) - Learn about the engine fundamentals
3. [Command System](COMMAND_SYSTEM.md) - Dive into command processing
4. [API Reference](API_REFERENCE.md) - Reference for public APIs

#### For Feature Development

Jump straight to what you need:

1. [API Reference](API_REFERENCE.md) - Find the functions you need
2. Specific module documentation for your feature area
3. [Testing Guide](TESTING.md) - Write tests for your changes

#### For Testing and Quality Assurance

Focus on verification:

1. [Testing Guide](TESTING.md) - Understand test infrastructure
2. Module-specific testing sections
3. [Architecture](ARCHITECTURE.md#memory-management-strategy) - Memory safety patterns

#### For Users

Learn to play the game:

1. [Built-in Commands](BUILTIN_COMMANDS.md) - Master the command system
2. Quick Start (above) - Get up and running
3. Project story documents - Immerse yourself in the lore

---

## Core Documentation

### 1. [Architecture and Design](ARCHITECTURE.md)

**What**: Comprehensive system architecture, design patterns, and module organization

**For**: Developers who want to understand the overall system design and architectural decisions

**Key Topics**:
- Layered architecture with dependency management
- Design patterns (Singleton, Observer, Strategy, Factory, State Machine)
- Data flow and event propagation
- Memory management strategies and ownership patterns
- Error handling patterns and conventions
- Initialization and shutdown sequences

**Length**: ~2,400 lines | Essential reading for all developers

---

### 2. [API Reference](API_REFERENCE.md)

**What**: Complete reference documentation for all public APIs and functions

**For**: Developers implementing features or integrating with existing systems

**Key Topics**:
- Function signatures with detailed parameter descriptions
- Return values and error codes
- Usage examples for all major APIs
- Thread safety and concurrency notes
- Performance characteristics
- Memory ownership semantics

**Length**: ~3,200 lines | Keep this open while coding

---

### 3. [Testing Guide](TESTING.md)

**What**: Comprehensive testing documentation including test suite, strategies, and best practices

**For**: Developers writing tests or ensuring code quality

**Key Topics**:
- Running the test suite and interpreting results
- Test coverage reports and goals
- Writing new unit tests
- Integration testing strategies
- Memory testing with Valgrind and sanitizers
- Performance benchmarking
- Continuous integration setup

**Length**: ~1,800 lines | Reference when adding tests

---

## Module Documentation

### 4. [Core Systems](CORE_SYSTEMS.md)

**What**: Deep dive into engine fundamentals - events, memory, state, timing, and game loop

**For**: Developers working on core engine functionality or needing to understand system internals

**Key Topics**:
- Event system architecture and pub/sub pattern
- Memory pool allocators and custom allocation strategies
- State manager for game state transitions
- High-precision timing system and FPS control
- Main game loop structure and update cycle
- Integration points between core systems

**Length**: ~2,100 lines | Critical for engine work

**Key Files**:
- `src/core/events.h/c`
- `src/core/memory.h/c`
- `src/core/state_manager.h/c`
- `src/core/timing.h/c`
- `src/core/game_loop.h/c`

---

### 5. [Command System](COMMAND_SYSTEM.md)

**What**: Complete command parsing and execution pipeline from input to result

**For**: Developers adding commands, modifying command processing, or understanding input handling

**Key Topics**:
- Tokenizer with quote and escape sequence handling
- Command parser with flag support
- Command registry with O(1) lookup
- Command history with persistence
- Autocomplete system using trie data structure
- Command execution and result handling
- Error reporting and user feedback

**Length**: ~2,600 lines | Essential for gameplay features

**Key Files**:
- `src/commands/command_system.h/c`
- `src/commands/tokenizer.h/c`
- `src/commands/parser.h/c`
- `src/commands/registry.h/c`
- `src/commands/history.h/c`
- `src/commands/autocomplete.h/c`
- `src/commands/executor.h/c`

**Extension Points**:
- Adding new commands: See [Built-in Commands](BUILTIN_COMMANDS.md#adding-new-commands)
- Custom flag types: See [parser.h](../necromancers_shell/src/commands/parser.h)
- Command aliases: See [registry.h](../necromancers_shell/src/commands/registry.h)

---

### 6. [Terminal Interface](TERMINAL_INTERFACE.md)

**What**: Terminal I/O, rendering, colors, and user interaction using ncurses

**For**: Developers working on UI/UX, visual feedback, or terminal rendering

**Key Topics**:
- ncurses wrapper abstraction for cross-platform support
- Input handling (keyboard, mouse, resize events)
- Color system with 256-color and RGB support
- Window management and layout
- UI feedback system with ANSI colors
- Status bar and prompt rendering
- Terminal feature detection

**Length**: ~2,500 lines | Critical for user experience

**Key Files**:
- `src/terminal/terminal.h/c`
- `src/terminal/input_handler.h/c`
- `src/terminal/ui_feedback.h/c`
- `src/terminal/colors.h/c`

---

### 7. [Utilities](UTILITIES.md)

**What**: Reusable data structures and utility functions used throughout the codebase

**For**: Developers using or extending core utility libraries

**Key Topics**:
- Hash table implementation (O(1) lookups)
- Logger system with levels and file output
- String utilities (split, trim, format)
- Trie data structure for prefix matching
- Dynamic arrays and buffer management
- File I/O utilities
- Math and random number utilities

**Length**: ~2,800 lines | Reference frequently

**Key Files**:
- `src/utils/hash_table.h/c`
- `src/utils/logger.h/c`
- `src/utils/string_utils.h/c`
- `src/utils/trie.h/c`

**Extension Points**:
- Adding new data structures: Follow patterns in hash_table.c
- Custom loggers: See logger.h for callback support

---

### 8. [Built-in Commands](BUILTIN_COMMANDS.md)

**What**: Documentation of all built-in commands with usage examples

**For**: Users learning commands and developers adding new commands

**Key Topics**:
- Command reference for all built-in commands
- Usage examples and common patterns
- Flag reference and option descriptions
- Adding new commands (developer guide)
- Command registration and integration
- Error handling in commands

**Length**: ~2,700 lines | User and developer reference

**Current Commands**:
- `help [command]` - Display help information
- `status [--verbose]` - Show system status
- `quit` / `exit` - Exit the shell
- `clear` - Clear the terminal screen
- `log <level> [--file <path>]` - Manage logging

**Adding New Commands**:
See [BUILTIN_COMMANDS.md#adding-new-commands](BUILTIN_COMMANDS.md#adding-new-commands) for step-by-step guide.

---

## Project Documentation

### Design and Planning Documents

These documents provide context, vision, and planning for the project:

#### [Design Document](../necromancers_shell_design_doc.md)
Comprehensive design document covering game mechanics, story, and technical specifications.

**Key Sections**:
- Game concept and vision
- Core mechanics and gameplay loops
- Technical architecture decisions
- Art and audio direction
- Development milestones

**Length**: Extensive | Read for project vision

---

#### [Implementation Plan](../plan.md)
Detailed implementation plan with timelines, milestones, and task breakdowns.

**Key Sections**:
- Phase-by-phase development plan
- Task dependencies and critical path
- Resource allocation and estimates
- Risk assessment and mitigation
- Testing and QA strategy

**Length**: Comprehensive | Project management reference

---

#### [Story Document (Enhanced)](../necromancers_shell_story_enhanced.md)
Rich narrative document with world-building, character arcs, and story branches.

**Key Sections**:
- World lore and history
- Character backgrounds and motivations
- Main story arc and side quests
- Moral dilemmas and consequences
- Multiple endings

**Length**: Extensive | For writers and designers

---

#### [Phase 1 Completion Report](../PHASE1_IMPLEMENTATION_COMPLETE.md)
Comprehensive report on Phase 1 implementation with metrics and achievements.

**Key Sections**:
- Implementation statistics (29 files, ~5,500 LOC)
- Feature completion checklist
- Build and test results (13/13 tests passing)
- Architecture quality assessment
- Performance characteristics
- Future enhancements for Phase 2+

**Status**: Phase 1 COMPLETE - Command System is production-ready

**Length**: ~430 lines | Milestone documentation

---

### README Files

#### [Main README](../README.md)
Project overview, quick start guide, and essential information.

**Contents**:
- Project description
- Build instructions
- Installation guide
- Development status
- License and authors

---

#### [Command System README](../necromancers_shell/src/commands/README.md)
Quick reference for the command system components.

**Contents**:
- Component overview
- Usage examples
- Integration guide
- File structure

---

#### [Necromancer's Shell Module README](../necromancers_shell/README.md)
Module-level documentation for the main codebase.

**Contents**:
- Directory structure
- Build configuration
- Module dependencies

---

## Quick Reference

### Common Tasks

#### How do I add a new command?

1. See [Built-in Commands - Adding New Commands](BUILTIN_COMMANDS.md#adding-new-commands)
2. See [Command System - Extension Points](COMMAND_SYSTEM.md#extension-points)
3. Follow the pattern in `src/commands/commands/cmd_help.c`

**Quick checklist**:
- Create `cmd_yourcommand.c` in `src/commands/commands/`
- Define handler function: `CommandResult cmd_yourcommand(const Command* cmd)`
- Register in `builtin.c`: `register_command(registry, "yourcommand", ...)`
- Add to help text in `commands.h`

---

#### How do I add a new data structure?

1. See [Utilities - Extension Points](UTILITIES.md#extension-points)
2. Follow patterns in `src/utils/hash_table.c`

**Guidelines**:
- Provide `create()` and `destroy()` functions
- Document time complexity for all operations
- Include NULL checks and error handling
- Write unit tests in `tests/`
- Add to [API Reference](API_REFERENCE.md)

---

#### How do I debug memory leaks?

1. See [Testing - Memory Testing](TESTING.md#memory-testing)
2. Build with debug: `make debug`
3. Run Valgrind: `make valgrind`

**Common tools**:
```bash
# Address sanitizer (built into debug builds)
make debug && ./build/necromancers_shell

# Valgrind for detailed leak analysis
make valgrind

# Check specific test
valgrind --leak-check=full ./build/test_tokenizer
```

---

#### How do I understand the event system?

1. See [Core Systems - Events](CORE_SYSTEMS.md#events)
2. See [Architecture - Observer Pattern](ARCHITECTURE.md#observer-pattern)

**Key concepts**:
- Event types and data structures
- Publishing events: `event_publish(type, data)`
- Subscribing: `event_subscribe(type, callback, userdata)`
- Thread safety considerations

---

#### How do I run tests?

1. See [Testing Guide](TESTING.md)

**Commands**:
```bash
# Run all tests
make test

# Run specific test
./build/test_tokenizer

# Run with coverage
make coverage

# Run with sanitizers
make debug && make test
```

---

#### How do I optimize performance?

1. See [Architecture - Performance Considerations](ARCHITECTURE.md#performance)
2. See module-specific performance sections

**Strategies**:
- Profile first: Identify bottlenecks
- Use appropriate data structures (hash tables for O(1), tries for prefix matching)
- Memory pools for frequent allocations
- Cache-friendly data layout
- Avoid premature optimization

---

### By Role

#### For New Developers

**Recommended reading path**:

1. Start: [Architecture and Design](ARCHITECTURE.md)
   - Understand the overall system design
   - Learn design patterns used throughout

2. Core: [Core Systems](CORE_SYSTEMS.md)
   - Master event system
   - Understand memory management
   - Learn state management

3. Interface: [Command System](COMMAND_SYSTEM.md)
   - See how user input flows through the system
   - Understand command execution pipeline

4. Reference: [API Reference](API_REFERENCE.md)
   - Keep open while coding
   - Learn function signatures and conventions

5. Practice: [Built-in Commands](BUILTIN_COMMANDS.md#adding-new-commands)
   - Add your first command
   - Follow the extension guide

**Time estimate**: 6-8 hours to become productive

---

#### For Feature Development

**Quick start workflow**:

1. [API Reference](API_REFERENCE.md) - Find the functions you need
2. Module-specific documentation for your feature area
3. [Testing Guide](TESTING.md) - Write tests alongside code
4. [Built-in Commands](BUILTIN_COMMANDS.md) - If adding commands

**Example: Adding a new game command**

```c
// 1. Check API reference for command system
// 2. Read COMMAND_SYSTEM.md for patterns
// 3. Look at cmd_help.c as example
// 4. Add command handler
// 5. Register in builtin.c
// 6. Write test in tests/
// 7. Update BUILTIN_COMMANDS.md
```

---

#### For Testing and QA

**Focus areas**:

1. [Testing Guide](TESTING.md) - Full test infrastructure
2. [Architecture - Memory Management](ARCHITECTURE.md#memory-management-strategy)
3. Module-specific testing sections

**Testing workflow**:
```bash
# Full test suite
make test

# Memory testing
make debug
make valgrind

# Coverage report
make coverage

# Static analysis
make analyze
```

**Quality checklist**:
- All tests passing
- No memory leaks (Valgrind clean)
- No warnings with `-Wall -Wextra -Werror`
- Coverage > 80% for new code
- Documentation updated

---

#### For Users

**Getting started**:

1. [Quick Start](#quick-start) - Build and run
2. [Built-in Commands](BUILTIN_COMMANDS.md) - Learn commands
3. [Story Document](../necromancers_shell_story_enhanced.md) - Immerse in lore

**Gameplay**:
```bash
# Start the shell
./build/necromancers_shell

# Learn commands
necromancer> help

# Get detailed help
necromancer> help status

# Play!
```

---

## Documentation Statistics

### Overview

- **Total documentation files**: 13
- **Core documentation files**: 3 (Architecture, API Reference, Testing)
- **Module documentation files**: 5 (Core Systems, Command System, Terminal, Utilities, Commands)
- **Project documentation files**: 5 (Design doc, Plan, Story, Phase report, READMEs)
- **Total lines**: ~18,225 lines in docs/ directory
- **Total project documentation**: ~20,000+ lines including root docs
- **Modules documented**: 8 major modules (Core, Commands, Terminal, Utils, and more)

### Coverage

- **Architecture**: Comprehensive coverage of design patterns and system organization
- **API Reference**: Complete function documentation for all public APIs
- **Testing**: Full test suite documentation and strategies
- **Modules**: Each major system has dedicated documentation
- **Commands**: All built-in commands documented with examples

### Documentation Quality

- Clear hierarchy and navigation
- Consistent formatting and structure
- Code examples for all major features
- Cross-references between related documents
- Extension points clearly marked
- Performance characteristics documented

---

## Contributing

### How to Update Documentation

When you add code, update the relevant documentation:

1. **Adding a function?** Update [API_REFERENCE.md](API_REFERENCE.md)
2. **Adding a command?** Update [BUILTIN_COMMANDS.md](BUILTIN_COMMANDS.md)
3. **Changing architecture?** Update [ARCHITECTURE.md](ARCHITECTURE.md)
4. **Adding tests?** Update [TESTING.md](TESTING.md)
5. **New module?** Create new module documentation and update this INDEX

### Documentation Style Guide

**Formatting**:
- Use Markdown with clear hierarchy (H1 for title, H2 for main sections, H3 for subsections)
- Code blocks with language hints: ```c for C code, ```bash for shell commands
- Use tables for comparisons and reference data
- Keep line length reasonable (80-120 characters)

**Content**:
- Start with "What, Who, Why" - explain purpose, audience, and key topics
- Provide context before diving into details
- Include code examples for all major features
- Document edge cases and error conditions
- Link to related documentation
- Keep descriptions concise but complete

**Organization**:
- Table of contents for documents > 200 lines
- Group related information together
- Use consistent section ordering across similar documents
- Mark extension points and customization opportunities

**Code Examples**:
- Complete, compilable examples when possible
- Show both simple and advanced usage
- Include error handling in examples
- Comment non-obvious code

### Where to Add New Documentation

- **New module?** Create `docs/MODULE_NAME.md` and update this INDEX
- **New command?** Add to [BUILTIN_COMMANDS.md](BUILTIN_COMMANDS.md)
- **Design decision?** Update [ARCHITECTURE.md](ARCHITECTURE.md)
- **New test?** Update [TESTING.md](TESTING.md)
- **API change?** Update [API_REFERENCE.md](API_REFERENCE.md)

### Review Checklist

Before submitting documentation changes:

- [ ] Grammar and spelling checked
- [ ] Code examples tested and working
- [ ] Cross-references updated
- [ ] Table of contents updated (if applicable)
- [ ] INDEX.md updated (if adding new doc)
- [ ] Consistent with style guide
- [ ] Links working (relative paths)
- [ ] Screenshots/diagrams added if helpful

---

## Additional Resources

### Build and Development

- **Makefile**: See `necromancers_shell/Makefile` for all build targets
- **Build Configuration**: Debug, release, test, and analysis targets available
- **Compiler Flags**: Strict warnings (`-Wall -Wextra -Werror -pedantic`)
- **Sanitizers**: Address and undefined behavior sanitizers in debug builds

### Code Quality Tools

- **Valgrind**: Memory leak detection (`make valgrind`)
- **Static Analysis**: Clang static analyzer (`make analyze`)
- **Code Formatting**: ClangFormat configuration (`.clang-format`)
- **Coverage**: Line and function coverage reports (`make coverage`)

### External Resources

- **C11 Standard**: Reference for language features and standard library
- **ncurses Documentation**: Terminal interface library
- **GCC Documentation**: Compiler features and optimizations
- **Valgrind Manual**: Memory debugging techniques

### Community

- **GitHub Repository**: (Add link when available)
- **Issue Tracker**: (Add link when available)
- **Discussions**: (Add link when available)

---

## Navigation

### Quick Links

- [Architecture](ARCHITECTURE.md) | [API Reference](API_REFERENCE.md) | [Testing](TESTING.md)
- [Core Systems](CORE_SYSTEMS.md) | [Command System](COMMAND_SYSTEM.md) | [Terminal](TERMINAL_INTERFACE.md)
- [Utilities](UTILITIES.md) | [Commands](BUILTIN_COMMANDS.md)

### Project Links

- [Design Document](../necromancers_shell_design_doc.md)
- [Implementation Plan](../plan.md)
- [Story Document](../necromancers_shell_story_enhanced.md)
- [Phase 1 Report](../PHASE1_IMPLEMENTATION_COMPLETE.md)
- [Main README](../README.md)

---

**Last Updated**: 2025-10-13

**Documentation Version**: 1.0 (Phase 1 Complete)

**Status**: Active development - Phase 1 complete, Phase 2 planning

---

**Welcome to Necromancer's Shell. Where death is just another system call.**

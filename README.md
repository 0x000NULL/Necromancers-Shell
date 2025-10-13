# Necromancer's Shell

A terminal-based dark fantasy RPG where necromancy meets system administration.

## Description

Necromancer's Shell is a command-line game that combines dark fantasy storytelling with Unix-like terminal commands. Play as a necromancer managing the dead through a sophisticated shell interface.

## Features

- Terminal-based interface using ncurses
- Command-driven gameplay (raise, harvest, scan, etc.)
- Deep narrative with moral choices
- Resource management and progression
- Turn-based combat
- Multiple endings based on player decisions

## Requirements

- C11 compiler (GCC or Clang)
- ncurses library
- Linux, macOS, or Windows (with PDCurses)
- Make build system

## Building

```bash
# Debug build (with sanitizers)
make debug

# Release build (optimized)
make release

# Run tests
make test

# Check for memory leaks
make valgrind
```

## Installation

```bash
make install
```

## Development Status

**Phase 0: Foundation** - In Progress

Current focus: Core infrastructure (memory management, logging, terminal interface, event system)

## License

TBD

## Authors

stripcheese

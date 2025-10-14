# Necromancer's Shell

[![CI](https://github.com/0x000NULL/Necromancers-Shell/actions/workflows/ci.yml/badge.svg)](https://github.com/0x000NULL/Necromancers-Shell/actions/workflows/ci.yml)
[![Release](https://github.com/0x000NULL/Necromancers-Shell/actions/workflows/release.yml/badge.svg)](https://github.com/0x000NULL/Necromancers-Shell/actions/workflows/release.yml)

A terminal-based dark fantasy RPG where necromancy meets system administration.

## Description

Necromancer's Shell is a command-line game that combines dark fantasy storytelling with Unix-like terminal commands. Play as a necromancer managing the dead through a sophisticated shell interface, commanding undead armies, harvesting souls, and navigating moral choices that affect your path to one of three distinct endings.

## Features

- **Terminal-based interface** using ncurses with rich colors and box drawing
- **Command-driven gameplay** with sophisticated command parser and autocomplete
- **Deep narrative** with moral choices affecting corruption level (0-100%)
- **Resource management** (soul energy, mana, corpses, territory)
- **Minion system** - Raise and manage 6 types of undead creatures
- **Soul collection** - 6 soul types with quality ratings and binding mechanics
- **Territory exploration** - Discover and connect to 5 location types
- **Multiple endings** - Three distinct paths based on your choices and corruption level

## Platform Support

| Platform | Status | Download |
|----------|--------|----------|
| **Linux x64** | ✅ Fully Supported | [Latest Release](https://github.com/0x000NULL/Necromancers-Shell/releases/latest) |
| **macOS ARM64** | ✅ Fully Supported | [Latest Release](https://github.com/0x000NULL/Necromancers-Shell/releases/latest) |
| **macOS Intel** | ✅ Fully Supported | [Latest Release](https://github.com/0x000NULL/Necromancers-Shell/releases/latest) |
| **Windows x64** | 🚧 Planned | Coming in Phase 3 |

## Quick Start

### Download Pre-Built Binaries

Download the latest release for your platform:

```bash
# Linux
wget https://github.com/0x000NULL/Necromancers-Shell/releases/latest/download/necromancer_shell-linux-x64-v*.tar.gz
tar -xzf necromancer_shell-linux-x64-v*.tar.gz
cd necromancer_shell-linux-x64-v*/
./necromancer_shell

# macOS (ARM64 - M1/M2/M3)
curl -L -O https://github.com/0x000NULL/Necromancers-Shell/releases/latest/download/necromancer_shell-macos-arm64-v*.tar.gz
tar -xzf necromancer_shell-macos-arm64-v*.tar.gz
cd necromancer_shell-macos-arm64-v*/
./necromancer_shell

# macOS (Intel)
curl -L -O https://github.com/0x000NULL/Necromancers-Shell/releases/latest/download/necromancer_shell-macos-x64-v*.tar.gz
tar -xzf necromancer_shell-macos-x64-v*.tar.gz
cd necromancer_shell-macos-x64-v*/
./necromancer_shell
```

### Building from Source

**Requirements:**
- C11 compiler (GCC 7+ or Clang 10+)
- ncurses development library
- Make build system
- Git

**Install dependencies:**

```bash
# Ubuntu/Debian
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev

# Fedora/RHEL
sudo dnf install gcc ncurses-devel

# macOS
brew install ncurses

# Arch Linux
sudo pacman -S base-devel ncurses
```

**Build:**

```bash
# Clone repository
git clone https://github.com/0x000NULL/Necromancers-Shell.git
cd Necromancers-Shell/necromancers_shell

# Build release version (optimized)
make release

# Run the game
./build/necromancer_shell

# Show version
./build/necromancer_shell --version

# Show help
./build/necromancer_shell --help
```

**Additional build commands:**

```bash
# Build debug version (with sanitizers)
make debug

# Run all tests (67+ unit tests)
make test

# Check for memory leaks
make valgrind

# Static analysis
make analyze

# Format code
make format

# Display version information
make version

# Clean build artifacts
make clean
```

## Gameplay

### Basic Commands

Once in the game, type `help` to see all available commands:

**Soul Management:**
- `souls` - View collected souls with filtering and sorting
- `harvest [--count n]` - Harvest souls from corpses at current location

**Minion Management:**
- `raise <type> [name]` - Raise undead minion (zombie, skeleton, ghoul, wraith, wight, revenant)
- `minions` - List all raised minions with stats
- `bind <minion_id> <soul_id>` - Bind soul to minion for stat bonuses
- `banish <minion_id>` - Destroy minion and return soul

**Exploration:**
- `scan` - View connected locations
- `probe <location>` - Get location details
- `connect <location>` - Travel to a location (takes time)

**System:**
- `status [--verbose]` - View game state, resources, corruption level
- `help [command]` - Get help on commands
- `clear` - Clear screen
- `quit` / `exit` - Exit game

### Soul Types

- **Common** - Basic souls, low energy value
- **Warrior** - Combat-focused, moderate energy
- **Mage** - Magic-attuned, high energy potential
- **Innocent** - Pure souls, high quality, increases corruption when used
- **Corrupted** - Tainted souls, unpredictable
- **Ancient** - Rare, extremely powerful

### Minion Types

- **Zombie** - Tough, slow, low cost
- **Skeleton** - Balanced, reliable
- **Ghoul** - Fast, aggressive
- **Wraith** - Ethereal, high damage
- **Wight** - Elite undead warrior
- **Revenant** - Powerful, sentient undead

## Development Status

**Phase 0: Foundation** ✅ Complete
- Core infrastructure (memory, logging, terminal, events, state manager)

**Phase 1: Command System** ✅ Complete
- Advanced command parser with tokenization, validation, history, and autocomplete
- 5,500+ lines of production C code

**Phase 2: Core Game Systems** ✅ Complete
- Soul system (6 types, quality, binding)
- Resource management (soul energy, mana, corruption, time)
- Location/territory system (5 location types)
- Minion system (6 types, stats, leveling, soul binding)
- 10,000+ lines of production code
- 67+ unit tests passing

**CI/CD Pipeline** ✅ Complete
- Automated versioning with semantic versioning
- Multi-platform builds (Linux, macOS ARM64, macOS Intel)
- GitHub Actions workflows for CI and releases
- Automated GitHub releases with binaries

**Phase 3: Enhanced Gameplay** 🚧 In Progress
- Windows PDCurses support
- Combat system
- Spell casting
- Advanced AI for minions
- Raw terminal mode with arrow key navigation
- Command aliases and piping

**Current Statistics:**
- **~10,000 lines** of production C code
- **67+ unit tests** passing (100% pass rate)
- **Zero memory leaks** (valgrind verified)
- **Zero compiler warnings** (strict flags: `-Wall -Wextra -Werror -pedantic`)
- **3 platforms** supported (Linux, macOS ARM64, macOS Intel)
- **9 playable game commands** + 4 system commands

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork the repository** and create a feature branch
2. **Follow the coding standards** (see CLAUDE.md):
   - C11 standard
   - Strict compiler flags (-Wall -Wextra -Werror -pedantic)
   - Zero warnings policy
   - Memory leak-free (valgrind verified)
3. **Write tests** for new features
4. **Run the full test suite**: `make test`
5. **Format your code**: `make format`
6. **Submit a pull request** with a clear description

### Commit Message Conventions

Use conventional commits to control automated versioning:

- `fix: Description` - Patch bump (0.3.0 → 0.3.1)
- `feat: Description` - Minor bump (0.3.0 → 0.4.0)
- `BREAKING: Description` - Major bump (0.3.0 → 1.0.0)

### CI/CD

All pull requests automatically run:
- Multi-platform builds (Linux, macOS)
- Full test suite (67+ tests)
- Static analysis (cppcheck)
- Memory leak detection (valgrind)

Merges to master automatically create new releases with binaries.

## Project Resources

- **Documentation**: See [CLAUDE.md](CLAUDE.md) for detailed developer documentation
- **Issues**: [GitHub Issues](https://github.com/0x000NULL/Necromancers-Shell/issues)
- **Releases**: [GitHub Releases](https://github.com/0x000NULL/Necromancers-Shell/releases)
- **Actions**: [GitHub Actions](https://github.com/0x000NULL/Necromancers-Shell/actions)

## License

TBD

## Authors

- **stripcheese** - Project creator and lead developer

## Acknowledgments

- Built with [ncurses](https://invisible-island.net/ncurses/) for terminal UI
- CI/CD powered by [GitHub Actions](https://github.com/features/actions)
- Automated releases with semantic versioning

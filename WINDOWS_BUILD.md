# Windows Build Guide for Necromancer's Shell

This guide provides detailed instructions for building Necromancer's Shell on Windows.

## Table of Contents
- [Quick Start (MSYS2)](#quick-start-msys2)
- [System Requirements](#system-requirements)
- [Method 1: MSYS2 (Recommended)](#method-1-msys2-recommended)
- [Method 2: Native MinGW](#method-2-native-mingw-advanced)
- [Building the Project](#building-the-project)
- [Running the Game](#running-the-game)
- [Troubleshooting](#troubleshooting)

## Quick Start (MSYS2)

```bash
# 1. Install MSYS2 from https://www.msys2.org/

# 2. Open "MSYS2 UCRT64" terminal

# 3. Install dependencies
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pdcurses make git

# 4. Clone and build
git clone https://github.com/0x000NULL/Necromancers-Shell.git
cd Necromancers-Shell/necromancers_shell
make release

# 5. Run the game
./build/necromancer_shell
```

## System Requirements

- **Operating System:** Windows 10 or later (Windows 11 recommended)
- **Terminal:** Windows Terminal, MSYS2 terminal, or Command Prompt
- **Disk Space:** ~100 MB for MSYS2 + dependencies
- **Memory:** 4 GB RAM minimum

**Why Windows 10+?**
- Native ANSI color code support (essential for game colors)
- Modern terminal features
- Better POSIX compatibility layer

## Method 1: MSYS2 (Recommended)

MSYS2 provides a Unix-like environment on Windows with GCC and package management.

### Step 1: Install MSYS2

1. Download the installer from [https://www.msys2.org/](https://www.msys2.org/)
2. Run `msys2-x86_64-<date>.exe`
3. Follow the installation wizard (default location: `C:\msys64`)
4. **Important:** When installation completes, check "Run MSYS2 now"

### Step 2: Update MSYS2

In the MSYS2 terminal that opens:

```bash
# Update package database and base packages
pacman -Syu

# If the terminal closes, reopen "MSYS2 MSYS" from Start Menu and run:
pacman -Su
```

### Step 3: Install Build Tools

**Close any open MSYS2 terminals and open "MSYS2 UCRT64"** (not MSYS, not MINGW64):

```bash
# Install GCC compiler, PDCurses library, make, and git
pacman -S mingw-w64-ucrt-x86_64-gcc \
          mingw-w64-ucrt-x86_64-pdcurses \
          make \
          git

# Verify installation
gcc --version      # Should show gcc (GCC) 13.x or later
make --version     # Should show GNU Make 4.x
```

**Why UCRT64?**
- Uses Windows Universal C Runtime (modern and maintained)
- Better compatibility with Windows 10+
- MINGW64 uses older msvcrt.dll (legacy)

### Step 4: Clone the Repository

```bash
# Navigate to your preferred directory
cd ~

# Clone the repository
git clone https://github.com/0x000NULL/Necromancers-Shell.git
cd Necromancers-Shell/necromancers_shell
```

### Step 5: Build

```bash
# Build optimized release version
make release

# Or build debug version (without sanitizers on Windows)
make debug

# Run tests
make test

# Display version information
make version
```

### Step 6: Run the Game

```bash
# From necromancers_shell/ directory
./build/necromancer_shell

# Or with explicit .exe extension
./build/necromancer_shell.exe
```

## Method 2: Native MinGW (Advanced)

This method is for advanced users who want to build without MSYS2.

### Prerequisites

1. **MinGW-w64:** Download from [https://www.mingw-w64.org/](https://www.mingw-w64.org/)
   - Choose: x86_64 architecture, UCRT runtime
   - Add `bin/` directory to PATH

2. **PDCurses:** Build from source
   ```cmd
   # Download from https://pdcurses.org/
   # Extract to C:\pdcurses
   cd C:\pdcurses\wincon
   mingw32-make -f Makefile.mng
   ```

3. **Make:** Download from [GnuWin32](http://gnuwin32.sourceforge.net/packages/make.htm) or use MinGW's mingw32-make

### Build Configuration

```cmd
# Set PDCurses path (if not in C:\pdcurses)
set PDCURSES_PATH=C:\path\to\pdcurses

# Build release version
cd necromancers_shell
mingw32-make release

# Or use GNU make if installed
make release
```

### Notes for Native Build

- The Makefile detects native Windows builds (non-MSYS2) automatically
- You must set `PDCURSES_PATH` if PDCurses is not in `C:\pdcurses`
- Forward slashes in paths work fine in Windows C runtime
- Math library (`-lm`) is built-in on Windows, but included for compatibility

## Building the Project

### Available Make Targets

```bash
make              # Build release version (default)
make release      # Build optimized release version
make debug        # Build debug version
make test         # Build and run all 67+ unit tests
make clean        # Remove all build artifacts
make version      # Display version information
make help         # Show all available targets
make windows-setup # Show Windows setup instructions
```

### Build Modes

**Release Mode:**
- Optimized with `-O2`
- No debug symbols
- Portable (no `-march=native` in CI)
- Recommended for playing the game

**Debug Mode:**
- Debug symbols (`-g`)
- No optimization (`-O0`)
- **Note:** Address sanitizers disabled on Windows (not well-supported in MinGW)
- Use for development and debugging

### Build Artifacts

All build artifacts are in `build/` directory:
- `necromancer_shell` or `necromancer_shell.exe` - Main executable
- `necromancer_shell_debug` - Debug build
- `test_*` - Unit test executables
- `*.o` - Object files
- `*.d` - Dependency files

## Running the Game

### From MSYS2 Terminal

```bash
cd necromancers_shell
./build/necromancer_shell
```

### From Windows Terminal or Command Prompt

```cmd
cd necromancers_shell\build
necromancer_shell.exe
```

### Double-Click Execution

You can double-click `necromancer_shell.exe` in File Explorer, but:
- The game is a terminal application
- It will open a new terminal window
- Window may close immediately if there are errors
- **Recommended:** Run from a terminal for better control

### Command-Line Arguments

```bash
./build/necromancer_shell --version    # Display version
./build/necromancer_shell --help       # Display help
```

## Troubleshooting

### Problem: "ncurses.h: No such file or directory"

**Cause:** PDCurses not installed or not found

**Solution (MSYS2):**
```bash
pacman -S mingw-w64-ucrt-x86_64-pdcurses
```

**Solution (Native):**
- Ensure PDCurses is built
- Set `PDCURSES_PATH` environment variable
- Check `PDCURSES_PATH/include/curses.h` exists

### Problem: "undefined reference to 'initscr'"

**Cause:** Linker cannot find PDCurses library

**Solution (MSYS2):**
```bash
# Verify PDCurses is installed
pacman -Ql mingw-w64-ucrt-x86_64-pdcurses | grep libpdcurses
```

**Solution (Native):**
- Ensure `libpdcurses.a` is in `PDCURSES_PATH/lib/`
- Check Makefile `LIBS` variable includes `-lpdcurses`

### Problem: Colors not displaying correctly

**Cause:** Terminal doesn't support ANSI color codes

**Solutions:**
1. Use **Windows Terminal** (recommended) - Download from Microsoft Store
2. Use **MSYS2 terminal** (supports ANSI colors)
3. Enable ANSI support in Command Prompt (Windows 10+ only):
   ```cmd
   reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 1
   ```

### Problem: "make: command not found"

**Cause:** Make not installed or not in PATH

**Solution (MSYS2):**
```bash
pacman -S make
```

**Solution (Native):**
- Download Make from [GnuWin32](http://gnuwin32.sourceforge.net/packages/make.htm)
- Or use `mingw32-make` instead of `make`

### Problem: "gcc: command not found"

**Cause:** GCC not installed or not in PATH

**Solution (MSYS2):**
```bash
# Ensure you're in UCRT64 terminal (check prompt)
pacman -S mingw-w64-ucrt-x86_64-gcc
```

**Solution (Native):**
- Add MinGW `bin/` directory to PATH
- Example: `C:\mingw64\bin`

### Problem: Tests failing with path errors

**Cause:** History tests may expect Unix-style home directory paths

**Status:** Known issue - Windows uses `%USERPROFILE%` instead of `$HOME`

**Workaround:**
- Tests should pass with Windows home directory detection
- If tests fail, report as a bug with full error message

### Problem: Game crashes on startup

**Debugging steps:**
1. Run in debug mode:
   ```bash
   make debug
   ./build/necromancer_shell_debug
   ```

2. Check log file:
   ```bash
   cat ~/.necromancers_shell_history
   ```

3. Verify terminal size:
   - Minimum recommended: 80x24 characters
   - Resize terminal if too small

4. Check PDCurses version:
   ```bash
   # In MSYS2
   pacman -Qi mingw-w64-ucrt-x86_64-pdcurses
   ```

### Problem: Build artifacts not cleaned

**Solution:**
```bash
# Clean all build files
make clean

# On Windows, also remove any stray .exe files
rm -f *.exe
```

### Problem: Line ending issues after git clone

**Cause:** Git auto-converting LF to CRLF on Windows

**Solution:**
```bash
# Configure git to use LF line endings
git config core.autocrlf input

# Re-clone repository or reset files
git rm --cached -r .
git reset --hard
```

## Performance Notes

### Expected Performance
- **Compile time:** ~2-5 seconds for full build
- **Test suite:** ~1-2 seconds for 67+ tests
- **Runtime FPS:** 60 FPS target (frame-rate locked)

### Performance Compared to Linux
- PDCurses is generally as fast as ncurses
- Windows terminal emulators may have slight overhead
- Game logic is platform-agnostic (same performance)

## Memory Leak Detection

**Important:** Valgrind is not available on Windows.

**Alternatives:**

1. **Dr. Memory** (Recommended):
   ```bash
   # Install via Chocolatey
   choco install drmemory

   # Run with Dr. Memory
   drmemory.exe -- ./build/necromancer_shell_debug
   ```

2. **Debug Build** (Built-in):
   - Debug builds have extra checks
   - Address sanitizers disabled on Windows (MinGW limitation)
   - Rely on Linux/macOS CI for valgrind checks

## Development Workflow

### Recommended Setup

1. **Editor:** Visual Studio Code with C/C++ extension
2. **Terminal:** Windows Terminal with MSYS2 UCRT64 profile
3. **Git:** Git for Windows or MSYS2 git

### VS Code Integration

Create `.vscode/settings.json`:
```json
{
    "terminal.integrated.profiles.windows": {
        "MSYS2 UCRT64": {
            "path": "C:\\msys64\\usr\\bin\\bash.exe",
            "args": ["-l", "-i"],
            "env": {
                "MSYSTEM": "UCRT64",
                "CHERE_INVOKING": "1"
            }
        }
    },
    "terminal.integrated.defaultProfile.windows": "MSYS2 UCRT64"
}
```

### Build and Test Cycle

```bash
# Make changes to source code
# ...

# Rebuild and test
make clean && make release && make test

# Run the game
./build/necromancer_shell
```

## Additional Resources

- **MSYS2 Documentation:** https://www.msys2.org/docs/what-is-msys2/
- **PDCurses Documentation:** https://pdcurses.org/
- **MinGW-w64 Documentation:** https://www.mingw-w64.org/
- **Project Repository:** https://github.com/0x000NULL/Necromancers-Shell
- **Report Issues:** https://github.com/0x000NULL/Necromancers-Shell/issues

## Platform-Specific Notes

### File Paths
- Windows accepts both `\` and `/` as path separators
- The game uses `/` in code (works on both platforms)
- History file location: `%USERPROFILE%\.necromancers_shell_history`

### File Permissions
- Windows doesn't support Unix chmod permissions
- History file uses default Windows ACLs
- Less security than Unix 600 permissions, but acceptable for single-user systems

### Terminal Differences
- PDCurses behaves slightly differently than ncurses
- Box drawing characters may vary by terminal
- ANSI color codes work in modern Windows terminals

### Line Endings
- Source files use LF (Unix style)
- Git configured to maintain LF on Windows
- No CRLF conversion needed

## Getting Help

If you encounter issues not covered in this guide:

1. Check existing GitHub issues
2. Run with debug build and capture output
3. Open a new issue with:
   - Windows version
   - MSYS2 or native MinGW
   - GCC version (`gcc --version`)
   - PDCurses version
   - Full error message
   - Steps to reproduce

## Contributing

Windows users are encouraged to test and contribute:
- Report Windows-specific bugs
- Improve build process
- Test on different Windows versions
- Enhance PDCurses compatibility

See `CONTRIBUTING.md` for contribution guidelines.

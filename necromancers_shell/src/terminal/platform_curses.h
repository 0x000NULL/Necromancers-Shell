/**
 * Platform-agnostic curses header for Necromancer's Shell
 *
 * Automatically includes the correct curses library based on the platform:
 * - ncurses on Linux/macOS/Unix-like systems
 * - PDCurses on Windows
 *
 * This allows the same code to compile and run across all platforms
 * without requiring platform-specific #ifdef blocks in the main codebase.
 */

#ifndef PLATFORM_CURSES_H
#define PLATFORM_CURSES_H

/* Platform detection */
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    /* Windows: Use PDCurses */
    #include <curses.h>
    #define PLATFORM_WINDOWS
#else
    /* Unix-like (Linux, macOS, BSD, etc.): Use ncurses */
    #include <ncurses.h>
    #define PLATFORM_UNIX
#endif

#endif /* PLATFORM_CURSES_H */

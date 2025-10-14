#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdbool.h>

/**
 * Logging System
 *
 * Multi-level logging with file and console output.
 * Thread-safe (future), formatted output, timestamps.
 *
 * Usage:
 *   logger_init("game.log", LOG_LEVEL_DEBUG);
 *   LOG_INFO("Game started");
 *   LOG_ERROR("Failed to load: %s", filename);
 *   logger_shutdown();
 */

/* Log levels */
typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;

/**
 * Initialize logger
 *
 * @param filename Log file path (NULL for stdout only)
 * @param level Minimum log level
 * @return true on success
 */
bool logger_init(const char* filename, LogLevel level);

/**
 * Shutdown logger (flush and close)
 */
void logger_shutdown(void);

/**
 * Set log level
 *
 * @param level New minimum log level
 */
void logger_set_level(LogLevel level);

/**
 * Get current log level
 *
 * @return Current log level
 */
LogLevel logger_get_level(void);

/**
 * Enable/disable console output
 *
 * @param enable true to enable console logging
 */
void logger_set_console(bool enable);

/**
 * Log a message
 *
 * @param level Log level
 * @param file Source file
 * @param line Line number
 * @param func Function name
 * @param fmt Format string
 */
void logger_log(LogLevel level, const char* file, int line,
                const char* func, const char* fmt, ...);

/* Convenience macros */
#define LOG_TRACE(...) logger_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...) logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)  logger_log(LOG_LEVEL_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)  logger_log(LOG_LEVEL_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_FATAL(...) logger_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

#endif /* LOGGER_H */

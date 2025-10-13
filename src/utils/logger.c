#include "utils/logger.h"
#include <stdarg.h>
#include <string.h>
#include <time.h>

/* Logger state */
static struct {
    FILE* file;
    LogLevel level;
    bool console_enabled;
    bool initialized;
} g_logger = {
    .file = NULL,
    .level = LOG_LEVEL_INFO,
    .console_enabled = true,
    .initialized = false
};

/* Log level names */
static const char* level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

/* ANSI color codes (for console) */
static const char* level_colors[] = {
    "\x1b[37m",  /* TRACE - white */
    "\x1b[36m",  /* DEBUG - cyan */
    "\x1b[32m",  /* INFO  - green */
    "\x1b[33m",  /* WARN  - yellow */
    "\x1b[31m",  /* ERROR - red */
    "\x1b[35m"   /* FATAL - magenta */
};

static const char* color_reset = "\x1b[0m";

bool logger_init(const char* filename, LogLevel level) {
    if (g_logger.initialized) {
        logger_shutdown();
    }

    g_logger.level = level;
    g_logger.console_enabled = true;

    if (filename) {
        g_logger.file = fopen(filename, "a");
        if (!g_logger.file) {
            fprintf(stderr, "Failed to open log file: %s\n", filename);
            return false;
        }

        /* Write startup marker */
        time_t now = time(NULL);
        fprintf(g_logger.file, "\n========== Log started: %s", ctime(&now));
        fflush(g_logger.file);
    }

    g_logger.initialized = true;
    return true;
}

void logger_shutdown(void) {
    if (!g_logger.initialized) return;

    if (g_logger.file) {
        time_t now = time(NULL);
        fprintf(g_logger.file, "========== Log ended: %s\n", ctime(&now));
        fclose(g_logger.file);
        g_logger.file = NULL;
    }

    g_logger.initialized = false;
}

void logger_set_level(LogLevel level) {
    g_logger.level = level;
}

LogLevel logger_get_level(void) {
    return g_logger.level;
}

void logger_set_console(bool enable) {
    g_logger.console_enabled = enable;
}

void logger_log(LogLevel level, const char* file, int line,
                const char* func, const char* fmt, ...) {
    /* Check level */
    if (level < g_logger.level) return;

    /* Get timestamp */
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    /* Extract filename from path */
    const char* filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;

    /* Format message */
    va_list args;
    char msg_buf[1024];
    va_start(args, fmt);
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    va_end(args);

    /* Write to file */
    if (g_logger.file) {
        fprintf(g_logger.file, "[%s] [%-5s] [%s:%d %s] %s\n",
                time_buf, level_names[level], filename, line, func, msg_buf);
        fflush(g_logger.file);
    }

    /* Write to console */
    if (g_logger.console_enabled) {
        fprintf(stderr, "%s[%s] [%-5s]%s [%s:%d] %s\n",
                level_colors[level], time_buf, level_names[level],
                color_reset, filename, line, msg_buf);
    }
}

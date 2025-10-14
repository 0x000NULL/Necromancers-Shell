/* POSIX features (strdup, getpwuid) */
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "history.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Platform-specific includes */
#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

/* Command history internal structure */
struct CommandHistory {
    char** commands;    /* Circular buffer of commands */
    size_t capacity;    /* Maximum number of commands */
    size_t size;        /* Current number of commands */
    size_t head;        /* Index of most recent command */
};

CommandHistory* command_history_create(size_t capacity) {
    if (capacity == 0) return NULL;

    CommandHistory* history = malloc(sizeof(CommandHistory));
    if (!history) return NULL;

    history->commands = calloc(capacity, sizeof(char*));
    if (!history->commands) {
        free(history);
        return NULL;
    }

    history->capacity = capacity;
    history->size = 0;
    history->head = 0;

    return history;
}

void command_history_destroy(CommandHistory* history) {
    if (!history) return;

    if (history->commands) {
        for (size_t i = 0; i < history->capacity; i++) {
            free(history->commands[i]);
        }
        free(history->commands);
    }

    free(history);
}

bool command_history_add(CommandHistory* history, const char* command) {
    if (!history || !command) return false;

    /* Ignore empty commands */
    if (command[0] == '\0') return true;

    /* Ignore if same as most recent command */
    if (history->size > 0) {
        const char* last_cmd = command_history_get(history, 0);
        if (last_cmd && strcmp(last_cmd, command) == 0) {
            return true;
        }
    }

    /* Move head forward */
    history->head = (history->head + 1) % history->capacity;

    /* Free old command if buffer is full */
    if (history->commands[history->head]) {
        free(history->commands[history->head]);
    }

    /* Add new command */
    history->commands[history->head] = strdup(command);
    if (!history->commands[history->head]) return false;

    /* Update size */
    if (history->size < history->capacity) {
        history->size++;
    }

    return true;
}

const char* command_history_get(const CommandHistory* history, size_t index) {
    if (!history || index >= history->size) return NULL;

    /* Calculate actual index in circular buffer */
    /* index 0 = most recent = head */
    /* index 1 = head - 1, etc. */
    size_t actual_index;
    if (index <= history->head) {
        actual_index = history->head - index;
    } else {
        actual_index = history->capacity - (index - history->head);
    }

    return history->commands[actual_index];
}

size_t command_history_size(const CommandHistory* history) {
    if (!history) return 0;
    return history->size;
}

size_t command_history_capacity(const CommandHistory* history) {
    if (!history) return 0;
    return history->capacity;
}

void command_history_clear(CommandHistory* history) {
    if (!history) return;

    for (size_t i = 0; i < history->capacity; i++) {
        if (history->commands[i]) {
            free(history->commands[i]);
            history->commands[i] = NULL;
        }
    }

    history->size = 0;
    history->head = 0;
}

bool command_history_save(const CommandHistory* history, const char* filepath) {
    if (!history || !filepath) return false;

    FILE* file = fopen(filepath, "w");
    if (!file) return false;

    /* Write commands from oldest to newest */
    for (size_t i = history->size; i > 0; i--) {
        const char* cmd = command_history_get(history, i - 1);
        if (cmd) {
            fprintf(file, "%s\n", cmd);
        }
    }

    fclose(file);

    /* Set file permissions to 600 (user read/write only) */
    /* Note: Windows file permissions work differently, skip chmod */
#ifndef _WIN32
    chmod(filepath, S_IRUSR | S_IWUSR);
#endif

    return true;
}

bool command_history_load(CommandHistory* history, const char* filepath) {
    if (!history || !filepath) return false;

    FILE* file = fopen(filepath, "r");
    if (!file) {
        /* File not existing is okay */
        return true;
    }

    char line[4096];
    while (fgets(line, sizeof(line), file)) {
        /* Remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        /* Add to history */
        command_history_add(history, line);
    }

    fclose(file);
    return true;
}

bool command_history_search(const CommandHistory* history, const char* pattern,
                           char*** results, size_t* count) {
    if (!history || !pattern || !results || !count) return false;

    *results = NULL;
    *count = 0;

    if (history->size == 0) return true;

    /* Allocate results array (maximum size) */
    char** matches = malloc(history->size * sizeof(char*));
    if (!matches) return false;

    size_t match_count = 0;

    /* Search from most recent to oldest */
    for (size_t i = 0; i < history->size; i++) {
        const char* cmd = command_history_get(history, i);
        if (cmd && strstr(cmd, pattern)) {
            matches[match_count] = strdup(cmd);
            if (!matches[match_count]) {
                /* Cleanup on error */
                for (size_t j = 0; j < match_count; j++) {
                    free(matches[j]);
                }
                free(matches);
                return false;
            }
            match_count++;
        }
    }

    *results = matches;
    *count = match_count;
    return true;
}

void command_history_free_search_results(char** results, size_t count) {
    if (!results) return;

    for (size_t i = 0; i < count; i++) {
        free(results[i]);
    }
    free(results);
}

char* command_history_default_path(void) {
    /* Get home directory - platform-specific */
    const char* home = NULL;

#ifdef _WIN32
    /* Windows: Try USERPROFILE first, then HOMEDRIVE+HOMEPATH */
    home = getenv("USERPROFILE");
    if (!home) {
        const char* homedrive = getenv("HOMEDRIVE");
        const char* homepath = getenv("HOMEPATH");
        if (homedrive && homepath) {
            /* Construct path from HOMEDRIVE and HOMEPATH */
            size_t len = strlen(homedrive) + strlen(homepath) + 1;
            char* constructed_home = malloc(len);
            if (constructed_home) {
                snprintf(constructed_home, len, "%s%s", homedrive, homepath);
                /* Note: This allocated memory will leak, but it's a rare fallback case */
                home = constructed_home;
            }
        }
    }
#else
    /* Unix: Try HOME environment variable, then getpwuid */
    home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            home = pw->pw_dir;
        }
    }
#endif

    if (!home) {
        /* Fallback to current directory */
        return strdup(".necromancers_shell_history");
    }

    /* Allocate path - forward slash works on both Unix and Windows */
    size_t len = strlen(home) + strlen("/.necromancers_shell_history") + 1;
    char* path = malloc(len);
    if (!path) return NULL;

    snprintf(path, len, "%s/.necromancers_shell_history", home);
    return path;
}

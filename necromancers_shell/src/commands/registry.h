#ifndef COMMAND_REGISTRY_H
#define COMMAND_REGISTRY_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Command Registry
 *
 * Central registry for all game commands. Uses hash table for O(1) lookup.
 * Manages command metadata, validation rules, and help text.
 *
 * Usage:
 *   CommandRegistry* reg = command_registry_create();
 *   command_registry_register(reg, cmd_info);
 *   CommandInfo* info = command_registry_get(reg, "help");
 *   command_registry_destroy(reg);
 */

/* Forward declarations */
struct ParsedCommand;
struct CommandResult;

/* Command execution function signature */
typedef struct CommandResult (*CommandFunction)(struct ParsedCommand* cmd);

/* Argument type enumeration */
typedef enum {
    ARG_TYPE_STRING,
    ARG_TYPE_INT,
    ARG_TYPE_FLOAT,
    ARG_TYPE_BOOL
} ArgumentType;

/* Flag/option definition */
typedef struct {
    const char* name;        /* Flag name (e.g., "verbose") */
    char short_name;         /* Short name (e.g., 'v'), 0 if none */
    ArgumentType type;       /* Expected argument type */
    bool required;           /* Whether flag is required */
    const char* description; /* Help text for this flag */
} FlagDefinition;

/* Command metadata and registration info */
typedef struct {
    const char* name;                /* Command name (e.g., "help") */
    const char* description;         /* Short description */
    const char* usage;               /* Usage string (e.g., "help [command]") */
    const char* help_text;           /* Detailed help text */
    CommandFunction function;        /* Function to execute */
    FlagDefinition* flags;           /* Array of flag definitions */
    size_t flag_count;               /* Number of flags */
    size_t min_args;                 /* Minimum positional arguments */
    size_t max_args;                 /* Maximum positional arguments (0 = unlimited) */
    bool hidden;                     /* Hide from help listing */
} CommandInfo;

/* Opaque registry structure */
typedef struct CommandRegistry CommandRegistry;

/**
 * Create command registry
 *
 * @return CommandRegistry pointer or NULL on failure
 */
CommandRegistry* command_registry_create(void);

/**
 * Destroy command registry
 *
 * @param registry Registry to destroy
 */
void command_registry_destroy(CommandRegistry* registry);

/**
 * Register a command
 *
 * @param registry Command registry
 * @param info Command information (copied internally)
 * @return true on success
 */
bool command_registry_register(CommandRegistry* registry, const CommandInfo* info);

/**
 * Unregister a command
 *
 * @param registry Command registry
 * @param name Command name
 * @return true if command was found and removed
 */
bool command_registry_unregister(CommandRegistry* registry, const char* name);

/**
 * Get command info by name
 *
 * @param registry Command registry
 * @param name Command name
 * @return CommandInfo pointer or NULL if not found
 */
const CommandInfo* command_registry_get(const CommandRegistry* registry, const char* name);

/**
 * Check if command exists
 *
 * @param registry Command registry
 * @param name Command name
 * @return true if command is registered
 */
bool command_registry_exists(const CommandRegistry* registry, const char* name);

/**
 * Get all registered command names
 *
 * @param registry Command registry
 * @param names Output array of command names (allocated by function)
 * @param count Output number of commands
 * @return true on success
 */
bool command_registry_get_all_names(const CommandRegistry* registry,
                                   char*** names, size_t* count);

/**
 * Free array returned by command_registry_get_all_names
 *
 * @param names Array to free
 * @param count Number of elements
 */
void command_registry_free_names(char** names, size_t count);

/**
 * Get number of registered commands
 *
 * @param registry Command registry
 * @return Number of commands
 */
size_t command_registry_count(const CommandRegistry* registry);

#endif /* COMMAND_REGISTRY_H */

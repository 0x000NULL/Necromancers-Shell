#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include "registry.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * Autocomplete System
 *
 * Context-aware command and flag completion using Trie.
 * Handles:
 * - Command name completion
 * - Flag completion (--flag, -f)
 * - Partial command completion
 * - Smart filtering based on context
 *
 * Usage:
 *   Autocomplete* ac = autocomplete_create(registry);
 *   char** completions = NULL;
 *   size_t count = 0;
 *   autocomplete_get_completions(ac, "hel", &completions, &count);
 *   // completions = ["help"]
 *   autocomplete_free_completions(completions, count);
 *   autocomplete_destroy(ac);
 */

/* Opaque autocomplete structure */
typedef struct Autocomplete Autocomplete;

/* Autocomplete context - what part of command are we completing? */
typedef enum {
    AUTOCOMPLETE_CONTEXT_COMMAND,    /* Completing command name */
    AUTOCOMPLETE_CONTEXT_FLAG,       /* Completing flag name */
    AUTOCOMPLETE_CONTEXT_ARGUMENT    /* Completing argument */
} AutocompleteContext;

/**
 * Create autocomplete system
 *
 * @param registry Command registry to use
 * @return Autocomplete pointer or NULL on failure
 */
Autocomplete* autocomplete_create(const CommandRegistry* registry);

/**
 * Destroy autocomplete system
 *
 * @param ac Autocomplete to destroy
 */
void autocomplete_destroy(Autocomplete* ac);

/**
 * Get completions for partial input
 * Automatically determines context and returns appropriate completions
 *
 * @param ac Autocomplete system
 * @param input Partial input string
 * @param completions Output array of completion strings (allocated by function)
 * @param count Output number of completions
 * @return true on success
 */
bool autocomplete_get_completions(Autocomplete* ac, const char* input,
                                 char*** completions, size_t* count);

/**
 * Get completions with explicit context
 *
 * @param ac Autocomplete system
 * @param input Partial input string
 * @param context Completion context
 * @param command_name Command name (for flag completion context), may be NULL
 * @param completions Output array of completion strings (allocated by function)
 * @param count Output number of completions
 * @return true on success
 */
bool autocomplete_get_completions_ex(Autocomplete* ac, const char* input,
                                    AutocompleteContext context,
                                    const char* command_name,
                                    char*** completions, size_t* count);

/**
 * Free completions array
 *
 * @param completions Completions array
 * @param count Number of completions
 */
void autocomplete_free_completions(char** completions, size_t count);

/**
 * Rebuild autocomplete index (call after registry changes)
 *
 * @param ac Autocomplete system
 * @return true on success
 */
bool autocomplete_rebuild(Autocomplete* ac);

/**
 * Add custom completion entry
 * Useful for adding dynamic completions (e.g., file names, player names)
 *
 * @param ac Autocomplete system
 * @param entry Completion entry to add
 * @return true on success
 */
bool autocomplete_add_entry(Autocomplete* ac, const char* entry);

/**
 * Remove custom completion entry
 *
 * @param ac Autocomplete system
 * @param entry Completion entry to remove
 * @return true on success
 */
bool autocomplete_remove_entry(Autocomplete* ac, const char* entry);

/**
 * Clear all custom entries
 *
 * @param ac Autocomplete system
 */
void autocomplete_clear_custom_entries(Autocomplete* ac);

#endif /* AUTOCOMPLETE_H */

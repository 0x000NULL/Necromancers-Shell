/* POSIX features (strdup) */
#define _POSIX_C_SOURCE 200809L

#include "autocomplete.h"
#include "tokenizer.h"
#include "../utils/trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Autocomplete internal structure */
struct Autocomplete {
    const CommandRegistry* registry;
    Trie* command_trie;     /* Command names */
    Trie* custom_trie;      /* Custom entries */
};

Autocomplete* autocomplete_create(const CommandRegistry* registry) {
    if (!registry) return NULL;

    Autocomplete* ac = malloc(sizeof(Autocomplete));
    if (!ac) return NULL;

    ac->registry = registry;
    ac->command_trie = trie_create();
    ac->custom_trie = trie_create();

    if (!ac->command_trie || !ac->custom_trie) {
        trie_destroy(ac->command_trie);
        trie_destroy(ac->custom_trie);
        free(ac);
        return NULL;
    }

    /* Build command trie from registry */
    if (!autocomplete_rebuild(ac)) {
        autocomplete_destroy(ac);
        return NULL;
    }

    return ac;
}

void autocomplete_destroy(Autocomplete* ac) {
    if (!ac) return;

    trie_destroy(ac->command_trie);
    trie_destroy(ac->custom_trie);
    free(ac);
}

bool autocomplete_rebuild(Autocomplete* ac) {
    if (!ac) return false;

    /* Clear existing command trie */
    trie_clear(ac->command_trie);

    /* Get all command names and add to trie */
    char** names = NULL;
    size_t count = 0;
    if (!command_registry_get_all_names(ac->registry, &names, &count)) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        trie_insert(ac->command_trie, names[i]);
    }

    command_registry_free_names(names, count);
    return true;
}

bool autocomplete_add_entry(Autocomplete* ac, const char* entry) {
    if (!ac || !entry) return false;
    return trie_insert(ac->custom_trie, entry);
}

bool autocomplete_remove_entry(Autocomplete* ac, const char* entry) {
    if (!ac || !entry) return false;
    return trie_remove(ac->custom_trie, entry);
}

void autocomplete_clear_custom_entries(Autocomplete* ac) {
    if (!ac) return;
    trie_clear(ac->custom_trie);
}

/* Helper: Get last token from input for completion */
static char* get_last_token(const char* input) {
    if (!input || *input == '\0') return strdup("");

    /* Find last non-whitespace sequence */
    const char* end = input + strlen(input);
    const char* start = end;

    /* Skip trailing whitespace */
    while (start > input && isspace(*(start - 1))) {
        start--;
    }

    /* If we're at a whitespace position, we're completing a new token */
    if (start == end) {
        return strdup("");
    }

    /* Find start of last token */
    while (start > input && !isspace(*(start - 1))) {
        start--;
    }

    size_t len = end - start;
    char* token = malloc(len + 1);
    if (!token) return NULL;

    memcpy(token, start, len);
    token[len] = '\0';

    return token;
}

/* Helper: Determine completion context from input */
static AutocompleteContext determine_context(const char* input, char** command_name_out) {
    if (!input || *input == '\0') {
        return AUTOCOMPLETE_CONTEXT_COMMAND;
    }

    /* Tokenize to understand structure */
    Token* tokens = NULL;
    size_t token_count = 0;
    TokenizeResult result = tokenize(input, &tokens, &token_count);

    if (result != TOKENIZE_SUCCESS || token_count == 0) {
        return AUTOCOMPLETE_CONTEXT_COMMAND;
    }

    /* Check if we're completing after whitespace */
    bool ends_with_space = isspace(input[strlen(input) - 1]);

    /* First token is command name */
    if (token_count == 1 && !ends_with_space) {
        free_tokens(tokens, token_count);
        return AUTOCOMPLETE_CONTEXT_COMMAND;
    }

    /* Save command name for flag lookup */
    if (command_name_out) {
        *command_name_out = strdup(tokens[0].value);
    }

    /* Check if last token (or next token if ends with space) is a flag */
    if (!ends_with_space) {
        const char* last = tokens[token_count - 1].value;
        if (last[0] == '-') {
            free_tokens(tokens, token_count);
            return AUTOCOMPLETE_CONTEXT_FLAG;
        }
    }

    free_tokens(tokens, token_count);

    /* Otherwise, completing an argument */
    return AUTOCOMPLETE_CONTEXT_ARGUMENT;
}

bool autocomplete_get_completions(Autocomplete* ac, const char* input,
                                 char*** completions, size_t* count) {
    if (!ac || !input || !completions || !count) return false;

    /* Determine context */
    char* command_name = NULL;
    AutocompleteContext context = determine_context(input, &command_name);

    bool result = autocomplete_get_completions_ex(ac, input, context,
                                                  command_name, completions, count);

    free(command_name);
    return result;
}

bool autocomplete_get_completions_ex(Autocomplete* ac, const char* input,
                                    AutocompleteContext context,
                                    const char* command_name,
                                    char*** completions, size_t* count) {
    if (!ac || !input || !completions || !count) return false;

    *completions = NULL;
    *count = 0;

    /* Get the token we're completing */
    char* token = get_last_token(input);
    if (!token) return false;

    /* Strip leading dashes for flag completion */
    const char* prefix = token;
    if (context == AUTOCOMPLETE_CONTEXT_FLAG) {
        while (*prefix == '-') prefix++;
    }

    char** matches = NULL;
    size_t match_count = 0;

    switch (context) {
        case AUTOCOMPLETE_CONTEXT_COMMAND:
            /* Complete command names */
            trie_find_with_prefix(ac->command_trie, prefix, &matches, &match_count);

            /* Also check custom entries */
            char** custom_matches = NULL;
            size_t custom_count = 0;
            trie_find_with_prefix(ac->custom_trie, prefix, &custom_matches, &custom_count);

            /* Combine results */
            if (custom_count > 0) {
                char** combined = realloc(matches, (match_count + custom_count) * sizeof(char*));
                if (combined) {
                    matches = combined;
                    for (size_t i = 0; i < custom_count; i++) {
                        matches[match_count++] = custom_matches[i];
                        custom_matches[i] = NULL;  /* Transfer ownership */
                    }
                }
                free(custom_matches);
            }
            break;

        case AUTOCOMPLETE_CONTEXT_FLAG:
            /* Complete flag names for the given command */
            if (command_name) {
                const CommandInfo* info = command_registry_get(ac->registry, command_name);
                if (info && info->flag_count > 0) {
                    /* Allocate matches array */
                    matches = malloc(info->flag_count * sizeof(char*));
                    if (matches) {
                        for (size_t i = 0; i < info->flag_count; i++) {
                            const FlagDefinition* flag = &info->flags[i];
                            /* Check if flag name starts with prefix */
                            if (flag->name && strncmp(flag->name, prefix, strlen(prefix)) == 0) {
                                /* Format as --flagname */
                                size_t len = strlen(flag->name) + 3;
                                char* formatted = malloc(len);
                                if (formatted) {
                                    snprintf(formatted, len, "--%s", flag->name);
                                    matches[match_count++] = formatted;
                                }
                            }
                        }
                    }
                }
            }
            break;

        case AUTOCOMPLETE_CONTEXT_ARGUMENT:
            /* For now, no argument completion (could add file completion, etc.) */
            break;
    }

    free(token);

    *completions = matches;
    *count = match_count;
    return true;
}

void autocomplete_free_completions(char** completions, size_t count) {
    if (!completions) return;

    for (size_t i = 0; i < count; i++) {
        free(completions[i]);
    }
    free(completions);
}

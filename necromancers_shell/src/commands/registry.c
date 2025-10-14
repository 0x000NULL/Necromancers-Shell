/* POSIX features (strdup) */
#define _POSIX_C_SOURCE 200809L

#include "registry.h"
#include "../utils/hash_table.h"
#include <stdlib.h>
#include <string.h>

/* Command registry internal structure */
struct CommandRegistry {
    HashTable* commands; /* Maps command name -> CommandInfo* */
    size_t count;        /* Number of registered commands */
};

/* Internal helper to duplicate CommandInfo */
static CommandInfo* duplicate_command_info(const CommandInfo* info) {
    if (!info) return NULL;

    CommandInfo* copy = malloc(sizeof(CommandInfo));
    if (!copy) return NULL;

    /* Copy scalar fields */
    copy->min_args = info->min_args;
    copy->max_args = info->max_args;
    copy->hidden = info->hidden;
    copy->flag_count = info->flag_count;
    copy->function = info->function;

    /* Duplicate strings */
    copy->name = info->name ? strdup(info->name) : NULL;
    copy->description = info->description ? strdup(info->description) : NULL;
    copy->usage = info->usage ? strdup(info->usage) : NULL;
    copy->help_text = info->help_text ? strdup(info->help_text) : NULL;

    /* Duplicate flags array */
    if (info->flag_count > 0 && info->flags) {
        copy->flags = malloc(info->flag_count * sizeof(FlagDefinition));
        if (!copy->flags) {
            free((void*)copy->name);
            free((void*)copy->description);
            free((void*)copy->usage);
            free((void*)copy->help_text);
            free(copy);
            return NULL;
        }

        for (size_t i = 0; i < info->flag_count; i++) {
            copy->flags[i].name = info->flags[i].name ? strdup(info->flags[i].name) : NULL;
            copy->flags[i].short_name = info->flags[i].short_name;
            copy->flags[i].type = info->flags[i].type;
            copy->flags[i].required = info->flags[i].required;
            copy->flags[i].description = info->flags[i].description ?
                                        strdup(info->flags[i].description) : NULL;
        }
    } else {
        copy->flags = NULL;
    }

    return copy;
}

/* Internal helper to free CommandInfo */
static void free_command_info(CommandInfo* info) {
    if (!info) return;

    free((void*)info->name);
    free((void*)info->description);
    free((void*)info->usage);
    free((void*)info->help_text);

    if (info->flags) {
        for (size_t i = 0; i < info->flag_count; i++) {
            free((void*)info->flags[i].name);
            free((void*)info->flags[i].description);
        }
        free(info->flags);
    }

    free(info);
}

CommandRegistry* command_registry_create(void) {
    CommandRegistry* registry = malloc(sizeof(CommandRegistry));
    if (!registry) return NULL;

    registry->commands = hash_table_create(64);
    if (!registry->commands) {
        free(registry);
        return NULL;
    }

    registry->count = 0;
    return registry;
}

void command_registry_destroy(CommandRegistry* registry) {
    if (!registry) return;

    /* Free all stored CommandInfo structures */
    if (registry->commands) {
        char** names = NULL;
        size_t count = 0;
        if (command_registry_get_all_names(registry, &names, &count)) {
            for (size_t i = 0; i < count; i++) {
                CommandInfo* info = (CommandInfo*)hash_table_get(registry->commands, names[i]);
                free_command_info(info);
            }
            command_registry_free_names(names, count);
        }

        hash_table_destroy(registry->commands);
    }

    free(registry);
}

bool command_registry_register(CommandRegistry* registry, const CommandInfo* info) {
    if (!registry || !info || !info->name) return false;

    /* Check if command already exists */
    if (hash_table_contains(registry->commands, info->name)) {
        return false;
    }

    /* Duplicate the command info for internal storage */
    CommandInfo* copy = duplicate_command_info(info);
    if (!copy) return false;

    /* Add to hash table */
    if (!hash_table_put(registry->commands, info->name, copy)) {
        free_command_info(copy);
        return false;
    }

    registry->count++;
    return true;
}

bool command_registry_unregister(CommandRegistry* registry, const char* name) {
    if (!registry || !name) return false;

    CommandInfo* info = (CommandInfo*)hash_table_remove(registry->commands, name);
    if (!info) return false;

    free_command_info(info);
    registry->count--;
    return true;
}

const CommandInfo* command_registry_get(const CommandRegistry* registry, const char* name) {
    if (!registry || !name) return NULL;
    return (const CommandInfo*)hash_table_get(registry->commands, name);
}

bool command_registry_exists(const CommandRegistry* registry, const char* name) {
    if (!registry || !name) return false;
    return hash_table_contains(registry->commands, name);
}

/* Callback for collecting command names */
typedef struct {
    char** names;
    size_t index;
} CollectNamesContext;

static void collect_names_callback(const char* key, void* value, void* userdata) {
    (void)value; /* Unused */
    CollectNamesContext* ctx = (CollectNamesContext*)userdata;
    ctx->names[ctx->index++] = strdup(key);
}

bool command_registry_get_all_names(const CommandRegistry* registry,
                                   char*** names, size_t* count) {
    if (!registry || !names || !count) return false;

    *count = registry->count;
    if (*count == 0) {
        *names = NULL;
        return true;
    }

    *names = malloc(*count * sizeof(char*));
    if (!*names) return false;

    CollectNamesContext ctx = { .names = *names, .index = 0 };
    hash_table_foreach(registry->commands, collect_names_callback, &ctx);

    return true;
}

void command_registry_free_names(char** names, size_t count) {
    if (!names) return;

    for (size_t i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
}

size_t command_registry_count(const CommandRegistry* registry) {
    if (!registry) return 0;
    return registry->count;
}

#define _POSIX_C_SOURCE 200809L  /* for strdup, strcasecmp */

#include "data_loader.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  /* for strcasecmp */
#include <ctype.h>
#include <errno.h>

/**
 * @file data_loader.c
 * @brief Implementation of generic data file parser
 */

#define MAX_LINE_LENGTH 1024
#define MAX_SECTIONS 1000
#define INITIAL_SECTION_CAPACITY 16
#define INITIAL_PROPERTY_CAPACITY 32

/* Global error message storage */
static char g_error_message[512] = {0};

/**
 * @brief Data file structure (opaque)
 */
struct DataFile {
    DataSection* sections;      /**< Array of sections */
    size_t section_count;       /**< Number of sections */
    size_t section_capacity;    /**< Allocated capacity */
    char filepath[256];         /**< Source file path */
};

/* Forward declarations */
static bool parse_section_header(const char* line, char* type_out, char* id_out);
static bool parse_key_value(const char* line, char* key_out, char* value_out);
static DataType infer_value_type(const char* value_str);
static bool parse_value(const char* value_str, DataValue* value_out);
static void trim_whitespace(char* str);
static bool is_comment(const char* line);
static bool is_empty_line(const char* line);
static void data_value_destroy(DataValue* value);
static void data_section_destroy(DataSection* section);

/**
 * @brief Load data file from disk
 */
DataFile* data_file_load(const char* filepath) {
    if (!filepath) {
        snprintf(g_error_message, sizeof(g_error_message), "Filepath is NULL");
        LOG_ERROR("data_file_load: filepath is NULL");
        return NULL;
    }

    /* Open file */
    FILE* file = fopen(filepath, "r");
    if (!file) {
        snprintf(g_error_message, sizeof(g_error_message),
                 "Failed to open file '%s': %s", filepath, strerror(errno));
        LOG_ERROR("%s", g_error_message);
        return NULL;
    }

    /* Allocate data file */
    DataFile* data_file = calloc(1, sizeof(DataFile));
    if (!data_file) {
        LOG_ERROR("Failed to allocate DataFile");
        fclose(file);
        return NULL;
    }

    strncpy(data_file->filepath, filepath, sizeof(data_file->filepath) - 1);
    data_file->section_capacity = INITIAL_SECTION_CAPACITY;
    data_file->sections = calloc(data_file->section_capacity, sizeof(DataSection));
    if (!data_file->sections) {
        LOG_ERROR("Failed to allocate sections array");
        free(data_file);
        fclose(file);
        return NULL;
    }

    /* Parse file line by line */
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    DataSection* current_section = NULL;

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        trim_whitespace(line);

        /* Skip comments and empty lines */
        if (is_comment(line) || is_empty_line(line)) {
            continue;
        }

        /* Check for section header */
        char section_type[64] = {0};
        char section_id[64] = {0};
        if (parse_section_header(line, section_type, section_id)) {
            /* Create new section */
            if (data_file->section_count >= data_file->section_capacity) {
                /* Grow sections array */
                size_t new_capacity = data_file->section_capacity * 2;
                DataSection* new_sections = realloc(data_file->sections,
                                                     new_capacity * sizeof(DataSection));
                if (!new_sections) {
                    snprintf(g_error_message, sizeof(g_error_message),
                             "Failed to grow sections array at line %d", line_number);
                    LOG_ERROR("%s", g_error_message);
                    data_file_destroy(data_file);
                    fclose(file);
                    return NULL;
                }
                data_file->sections = new_sections;
                data_file->section_capacity = new_capacity;
            }

            current_section = &data_file->sections[data_file->section_count++];
            memset(current_section, 0, sizeof(DataSection));

            /* Copy with explicit truncation to avoid warnings */
            size_t type_copy_len = strlen(section_type);
            if (type_copy_len >= sizeof(current_section->section_type)) {
                type_copy_len = sizeof(current_section->section_type) - 1;
            }
            memcpy(current_section->section_type, section_type, type_copy_len);
            current_section->section_type[type_copy_len] = '\0';

            size_t id_copy_len = strlen(section_id);
            if (id_copy_len >= sizeof(current_section->section_id)) {
                id_copy_len = sizeof(current_section->section_id) - 1;
            }
            memcpy(current_section->section_id, section_id, id_copy_len);
            current_section->section_id[id_copy_len] = '\0';
            current_section->property_capacity = INITIAL_PROPERTY_CAPACITY;
            current_section->properties = calloc(current_section->property_capacity, sizeof(DataValue));
            if (!current_section->properties) {
                snprintf(g_error_message, sizeof(g_error_message),
                         "Failed to allocate properties array at line %d", line_number);
                LOG_ERROR("%s", g_error_message);
                data_file_destroy(data_file);
                fclose(file);
                return NULL;
            }

            LOG_TRACE("Parsed section: [%s:%s]", section_type, section_id);
            continue;
        }

        /* Try to parse key=value */
        char key[64] = {0};
        char value[512] = {0};
        if (parse_key_value(line, key, value)) {
            if (!current_section) {
                snprintf(g_error_message, sizeof(g_error_message),
                         "Key-value pair found before any section at line %d", line_number);
                LOG_WARN("%s", g_error_message);
                continue;
            }

            /* Grow properties array if needed */
            if (current_section->property_count >= current_section->property_capacity) {
                size_t new_capacity = current_section->property_capacity * 2;
                DataValue* new_properties = realloc(current_section->properties,
                                                      new_capacity * sizeof(DataValue));
                if (!new_properties) {
                    snprintf(g_error_message, sizeof(g_error_message),
                             "Failed to grow properties array at line %d", line_number);
                    LOG_ERROR("%s", g_error_message);
                    data_file_destroy(data_file);
                    fclose(file);
                    return NULL;
                }
                current_section->properties = new_properties;
                current_section->property_capacity = new_capacity;
            }

            /* Parse and store value */
            DataValue* prop = &current_section->properties[current_section->property_count];
            memset(prop, 0, sizeof(DataValue));

            /* Copy with explicit truncation */
            size_t key_len = strlen(key);
            if (key_len >= sizeof(prop->key)) {
                key_len = sizeof(prop->key) - 1;
            }
            memcpy(prop->key, key, key_len);
            prop->key[key_len] = '\0';

            if (!parse_value(value, prop)) {
                snprintf(g_error_message, sizeof(g_error_message),
                         "Failed to parse value at line %d", line_number);
                LOG_WARN("%s", g_error_message);
                continue;
            }

            current_section->property_count++;
            LOG_TRACE("  %s = %s", key, value);
            continue;
        }

        /* Invalid line */
        snprintf(g_error_message, sizeof(g_error_message),
                 "Invalid syntax at line %d", line_number);
        LOG_WARN("%s", g_error_message);
    }

    fclose(file);

    LOG_INFO("Loaded data file '%s': %zu sections", filepath, data_file->section_count);
    return data_file;
}

/**
 * @brief Destroy data file
 */
void data_file_destroy(DataFile* file) {
    if (!file) return;

    for (size_t i = 0; i < file->section_count; i++) {
        data_section_destroy(&file->sections[i]);
    }

    free(file->sections);
    free(file);
}

/**
 * @brief Get all sections of a specific type
 */
const DataSection** data_file_get_sections(const DataFile* file,
                                             const char* section_type,
                                             size_t* count_out) {
    if (!file || !section_type || !count_out) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    /* Count matching sections */
    size_t count = 0;
    for (size_t i = 0; i < file->section_count; i++) {
        if (strcmp(file->sections[i].section_type, section_type) == 0) {
            count++;
        }
    }

    if (count == 0) {
        *count_out = 0;
        return NULL;
    }

    /* Allocate result array */
    const DataSection** result = malloc(sizeof(DataSection*) * count);
    if (!result) {
        *count_out = 0;
        return NULL;
    }

    /* Fill result array */
    size_t index = 0;
    for (size_t i = 0; i < file->section_count; i++) {
        if (strcmp(file->sections[i].section_type, section_type) == 0) {
            result[index++] = &file->sections[i];
        }
    }

    *count_out = count;
    return result;
}

/**
 * @brief Get a specific section by type and ID
 */
const DataSection* data_file_get_section(const DataFile* file,
                                           const char* section_type,
                                           const char* section_id) {
    if (!file || !section_type || !section_id) {
        return NULL;
    }

    for (size_t i = 0; i < file->section_count; i++) {
        if (strcmp(file->sections[i].section_type, section_type) == 0 &&
            strcmp(file->sections[i].section_id, section_id) == 0) {
            return &file->sections[i];
        }
    }

    return NULL;
}

/**
 * @brief Get property from section
 */
const DataValue* data_section_get(const DataSection* section, const char* key) {
    if (!section || !key) {
        return NULL;
    }

    for (size_t i = 0; i < section->property_count; i++) {
        if (strcmp(section->properties[i].key, key) == 0) {
            return &section->properties[i];
        }
    }

    return NULL;
}

/**
 * @brief Get string value with default
 */
const char* data_value_get_string(const DataValue* value, const char* default_val) {
    if (!value || value->type != DATA_TYPE_STRING) {
        return default_val;
    }
    return value->value.string_value;
}

/**
 * @brief Get int value with default
 */
int64_t data_value_get_int(const DataValue* value, int64_t default_val) {
    if (!value) return default_val;

    if (value->type == DATA_TYPE_INT) {
        return value->value.int_value;
    }

    /* Try to convert string */
    if (value->type == DATA_TYPE_STRING) {
        char* endptr;
        int64_t result = strtoll(value->value.string_value, &endptr, 10);
        if (endptr != value->value.string_value && *endptr == '\0') {
            return result;
        }
    }

    return default_val;
}

/**
 * @brief Get float value with default
 */
double data_value_get_float(const DataValue* value, double default_val) {
    if (!value) return default_val;

    if (value->type == DATA_TYPE_FLOAT) {
        return value->value.float_value;
    }

    /* Try to convert string */
    if (value->type == DATA_TYPE_STRING) {
        char* endptr;
        double result = strtod(value->value.string_value, &endptr);
        if (endptr != value->value.string_value && *endptr == '\0') {
            return result;
        }
    }

    return default_val;
}

/**
 * @brief Get bool value with default
 */
bool data_value_get_bool(const DataValue* value, bool default_val) {
    if (!value) return default_val;

    if (value->type == DATA_TYPE_BOOL) {
        return value->value.bool_value;
    }

    /* Try to convert integer */
    if (value->type == DATA_TYPE_INT) {
        return value->value.int_value != 0;
    }

    /* Try to convert string */
    if (value->type == DATA_TYPE_STRING) {
        const char* str = value->value.string_value;
        if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0 ||
            strcmp(str, "1") == 0) {
            return true;
        }
        if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0 ||
            strcmp(str, "0") == 0) {
            return false;
        }
    }

    return default_val;
}

/**
 * @brief Get array value
 */
const char** data_value_get_array(const DataValue* value, size_t* count_out) {
    if (!value || value->type != DATA_TYPE_ARRAY) {
        if (count_out) *count_out = 0;
        return NULL;
    }

    if (count_out) {
        *count_out = value->array_count;
    }
    return (const char**)value->value.array_values;
}

/**
 * @brief Get total section count
 */
size_t data_file_get_section_count(const DataFile* file) {
    return file ? file->section_count : 0;
}

/**
 * @brief Check if file is valid
 */
bool data_file_is_valid(const DataFile* file) {
    return file != NULL && file->sections != NULL;
}

/**
 * @brief Get last error message
 */
const char* data_file_get_error(void) {
    return g_error_message[0] ? g_error_message : NULL;
}

/* ========== Internal Helper Functions ========== */

/**
 * @brief Parse section header: [TYPE:ID]
 */
static bool parse_section_header(const char* line, char* type_out, char* id_out) {
    if (!line || line[0] != '[') return false;

    /* Find closing bracket */
    const char* end = strchr(line, ']');
    if (!end) return false;

    /* Extract content between brackets */
    char content[128];
    size_t len = (size_t)(end - line - 1);
    if (len >= sizeof(content)) return false;
    strncpy(content, line + 1, len);
    content[len] = '\0';

    /* Find colon separator */
    char* colon = strchr(content, ':');
    if (!colon) return false;

    /* Extract type and ID */
    *colon = '\0';
    size_t type_len = strlen(content);
    size_t id_len = strlen(colon + 1);

    /* Copy with proper bounds checking - use memcpy to avoid truncation warnings */
    if (type_len >= 64) type_len = 63;
    memcpy(type_out, content, type_len);
    type_out[type_len] = '\0';

    if (id_len >= 64) id_len = 63;
    memcpy(id_out, colon + 1, id_len);
    id_out[id_len] = '\0';

    trim_whitespace(type_out);
    trim_whitespace(id_out);

    return type_out[0] != '\0' && id_out[0] != '\0';
}

/**
 * @brief Parse key=value line
 */
static bool parse_key_value(const char* line, char* key_out, char* value_out) {
    if (!line) return false;

    const char* equals = strchr(line, '=');
    if (!equals) return false;

    /* Extract key */
    size_t key_len = (size_t)(equals - line);
    if (key_len >= 64) return false;
    strncpy(key_out, line, key_len);
    key_out[key_len] = '\0';
    trim_whitespace(key_out);

    /* Extract value */
    strncpy(value_out, equals + 1, 511);
    value_out[511] = '\0';
    trim_whitespace(value_out);

    return key_out[0] != '\0' && value_out[0] != '\0';
}

/**
 * @brief Parse value string into DataValue
 */
static bool parse_value(const char* value_str, DataValue* value_out) {
    if (!value_str || !value_out) return false;

    DataType type = infer_value_type(value_str);
    value_out->type = type;

    switch (type) {
        case DATA_TYPE_BOOL: {
            if (strcasecmp(value_str, "true") == 0 || strcasecmp(value_str, "yes") == 0 ||
                strcmp(value_str, "1") == 0) {
                value_out->value.bool_value = true;
            } else {
                value_out->value.bool_value = false;
            }
            return true;
        }

        case DATA_TYPE_INT: {
            char* endptr;
            value_out->value.int_value = strtoll(value_str, &endptr, 10);
            return endptr != value_str && *endptr == '\0';
        }

        case DATA_TYPE_FLOAT: {
            char* endptr;
            value_out->value.float_value = strtod(value_str, &endptr);
            return endptr != value_str && *endptr == '\0';
        }

        case DATA_TYPE_ARRAY: {
            /* Count commas to determine array size */
            size_t count = 1;
            for (const char* p = value_str; *p; p++) {
                if (*p == ',') count++;
            }

            /* Allocate array */
            value_out->value.array_values = malloc(sizeof(char*) * (count + 1));
            if (!value_out->value.array_values) return false;

            /* Split by commas */
            char temp[512];
            strncpy(temp, value_str, sizeof(temp) - 1);
            temp[sizeof(temp) - 1] = '\0';

            size_t index = 0;
            char* token = strtok(temp, ",");
            while (token && index < count) {
                trim_whitespace(token);
                value_out->value.array_values[index++] = strdup(token);
                token = strtok(NULL, ",");
            }
            value_out->value.array_values[index] = NULL;
            value_out->array_count = index;
            return true;
        }

        case DATA_TYPE_STRING:
        default: {
            strncpy(value_out->value.string_value, value_str,
                    sizeof(value_out->value.string_value) - 1);
            value_out->value.string_value[sizeof(value_out->value.string_value) - 1] = '\0';
            return true;
        }
    }
}

/**
 * @brief Infer type from value string
 */
static DataType infer_value_type(const char* value_str) {
    if (!value_str || value_str[0] == '\0') {
        return DATA_TYPE_STRING;
    }

    /* Check for boolean */
    if (strcasecmp(value_str, "true") == 0 || strcasecmp(value_str, "false") == 0 ||
        strcasecmp(value_str, "yes") == 0 || strcasecmp(value_str, "no") == 0) {
        return DATA_TYPE_BOOL;
    }

    /* Check for array (contains comma) */
    if (strchr(value_str, ',') != NULL) {
        return DATA_TYPE_ARRAY;
    }

    /* Check for float (contains decimal point) */
    if (strchr(value_str, '.') != NULL) {
        char* endptr;
        strtod(value_str, &endptr);
        if (endptr != value_str && *endptr == '\0') {
            return DATA_TYPE_FLOAT;
        }
    }

    /* Check for integer */
    char* endptr;
    strtoll(value_str, &endptr, 10);
    if (endptr != value_str && *endptr == '\0') {
        return DATA_TYPE_INT;
    }

    /* Default to string */
    return DATA_TYPE_STRING;
}

/**
 * @brief Trim leading/trailing whitespace in-place
 */
static void trim_whitespace(char* str) {
    if (!str) return;

    /* Trim leading */
    char* start = str;
    while (isspace((unsigned char)*start)) start++;

    /* Trim trailing */
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;

    /* Move trimmed string to beginning and null-terminate */
    size_t len = (size_t)(end - start + 1);
    memmove(str, start, len);
    str[len] = '\0';
}

/**
 * @brief Check if line is a comment
 */
static bool is_comment(const char* line) {
    return line && (line[0] == '#' || line[0] == ';');
}

/**
 * @brief Check if line is empty
 */
static bool is_empty_line(const char* line) {
    return !line || line[0] == '\0';
}

/**
 * @brief Destroy data value (free arrays)
 */
static void data_value_destroy(DataValue* value) {
    if (!value) return;

    if (value->type == DATA_TYPE_ARRAY && value->value.array_values) {
        for (size_t i = 0; i < value->array_count; i++) {
            free(value->value.array_values[i]);
        }
        free(value->value.array_values);
        value->value.array_values = NULL;
    }
}

/**
 * @brief Destroy data section
 */
static void data_section_destroy(DataSection* section) {
    if (!section) return;

    if (section->properties) {
        for (size_t i = 0; i < section->property_count; i++) {
            data_value_destroy(&section->properties[i]);
        }
        free(section->properties);
        section->properties = NULL;
    }
}

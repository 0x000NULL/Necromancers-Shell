#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file data_loader.h
 * @brief Generic data file parser for game configuration
 *
 * Parses INI-like data files with sections and key-value pairs:
 *   [SECTION_TYPE:section_id]
 *   key = value
 *   another_key = 123
 *
 * Supports typed values: strings, integers, floats, bools, and comma-separated arrays.
 * Used to load locations, minions, spells, skills, and artifacts from external files.
 */

/**
 * @brief Data value types supported by parser
 */
typedef enum {
    DATA_TYPE_STRING,    /**< String value (up to 255 chars) */
    DATA_TYPE_INT,       /**< Integer value (int64_t) */
    DATA_TYPE_FLOAT,     /**< Floating point value (double) */
    DATA_TYPE_BOOL,      /**< Boolean value (true/false) */
    DATA_TYPE_ARRAY      /**< Comma-separated array of strings */
} DataType;

/**
 * @brief Single data value (property)
 */
typedef struct {
    char key[64];           /**< Property name */
    DataType type;          /**< Value type */
    union {
        char string_value[256];  /**< String storage */
        int64_t int_value;       /**< Integer storage */
        double float_value;      /**< Float storage */
        bool bool_value;         /**< Boolean storage */
        char** array_values;     /**< Array of strings (NULL-terminated) */
    } value;
    size_t array_count;     /**< Number of elements in array (for arrays only) */
} DataValue;

/**
 * @brief Data section (e.g., [LOCATION:graveyard_01])
 */
typedef struct {
    char section_type[64];   /**< Section type (e.g., "LOCATION") */
    char section_id[64];     /**< Section identifier (e.g., "graveyard_01") */
    DataValue* properties;   /**< Array of key-value pairs */
    size_t property_count;   /**< Number of properties */
    size_t property_capacity;/**< Allocated capacity */
} DataSection;

/**
 * @brief Parsed data file (opaque)
 */
typedef struct DataFile DataFile;

/**
 * @brief Load data file from disk
 *
 * Parses the file and builds section tree. Returns NULL on error
 * (file not found, syntax error, etc.).
 *
 * @param filepath Path to data file
 * @return Loaded DataFile, or NULL on error
 */
DataFile* data_file_load(const char* filepath);

/**
 * @brief Destroy data file and free all memory
 *
 * Frees all sections, properties, and allocated strings.
 *
 * @param file Data file (can be NULL)
 */
void data_file_destroy(DataFile* file);

/**
 * @brief Get all sections of a specific type
 *
 * Returns array of section pointers matching the type.
 * The returned array is owned by the DataFile and should not be freed.
 *
 * Example:
 *   size_t count;
 *   const DataSection** sections = data_file_get_sections(file, "LOCATION", &count);
 *   for (size_t i = 0; i < count; i++) {
 *       // Process sections[i]...
 *   }
 *
 * @param file Data file
 * @param section_type Type to filter (e.g., "LOCATION")
 * @param count_out Output: number of sections found
 * @return Array of DataSection pointers (owned by file, don't free)
 */
const DataSection** data_file_get_sections(const DataFile* file,
                                             const char* section_type,
                                             size_t* count_out);

/**
 * @brief Get a specific section by type and ID
 *
 * @param file Data file
 * @param section_type Section type (e.g., "LOCATION")
 * @param section_id Section ID (e.g., "graveyard_01")
 * @return DataSection pointer, or NULL if not found
 */
const DataSection* data_file_get_section(const DataFile* file,
                                           const char* section_type,
                                           const char* section_id);

/**
 * @brief Get property value from section
 *
 * @param section Data section
 * @param key Property key
 * @return DataValue pointer, or NULL if not found
 */
const DataValue* data_section_get(const DataSection* section, const char* key);

/**
 * @brief Get string value with default fallback
 *
 * If value is NULL or not a string, returns default_val.
 *
 * @param value Data value (can be NULL)
 * @param default_val Default string to return
 * @return String value or default
 */
const char* data_value_get_string(const DataValue* value, const char* default_val);

/**
 * @brief Get int value with default fallback
 *
 * If value is NULL or not an integer, returns default_val.
 * Also converts strings to integers if possible.
 *
 * @param value Data value (can be NULL)
 * @param default_val Default integer to return
 * @return Integer value or default
 */
int64_t data_value_get_int(const DataValue* value, int64_t default_val);

/**
 * @brief Get float value with default fallback
 *
 * If value is NULL or not a float, returns default_val.
 * Also converts strings to floats if possible.
 *
 * @param value Data value (can be NULL)
 * @param default_val Default float to return
 * @return Float value or default
 */
double data_value_get_float(const DataValue* value, double default_val);

/**
 * @brief Get bool value with default fallback
 *
 * If value is NULL or not a boolean, returns default_val.
 * Recognizes: "true", "false", "yes", "no", "1", "0" (case-insensitive).
 *
 * @param value Data value (can be NULL)
 * @param default_val Default boolean to return
 * @return Boolean value or default
 */
bool data_value_get_bool(const DataValue* value, bool default_val);

/**
 * @brief Get array value
 *
 * Returns NULL-terminated array of strings.
 * If value is NULL or not an array, returns NULL.
 * The returned array is owned by the DataValue and should not be freed.
 *
 * @param value Data value (can be NULL)
 * @param count_out Output: number of elements (optional, can be NULL)
 * @return Array of strings, or NULL
 */
const char** data_value_get_array(const DataValue* value, size_t* count_out);

/**
 * @brief Get total number of sections in file
 *
 * @param file Data file
 * @return Total section count
 */
size_t data_file_get_section_count(const DataFile* file);

/**
 * @brief Check if data file loaded successfully
 *
 * @param file Data file (can be NULL)
 * @return true if file is valid and loaded
 */
bool data_file_is_valid(const DataFile* file);

/**
 * @brief Get error message from last parse failure
 *
 * Returns NULL if no error occurred.
 * The returned string is owned by the parser and should not be freed.
 *
 * @return Error message or NULL
 */
const char* data_file_get_error(void);

#endif /* DATA_LOADER_H */

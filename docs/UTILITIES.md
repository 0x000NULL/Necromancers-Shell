# Utilities Library Documentation

## Table of Contents

1. [Overview](#1-overview)
2. [Hash Table](#2-hash-table)
   - [Data Structures](#data-structures)
   - [Hash Function](#hash-function)
   - [Collision Handling](#collision-handling)
   - [Dynamic Resizing](#dynamic-resizing)
   - [API Reference](#hash-table-api-reference)
   - [Performance Analysis](#hash-table-performance)
3. [Logger](#3-logger)
   - [Data Structures](#logger-data-structures)
   - [Log Levels](#log-levels)
   - [Output Formatting](#output-formatting)
   - [API Reference](#logger-api-reference)
   - [Configuration](#logger-configuration)
4. [String Utilities](#4-string-utilities)
   - [Safe String Operations](#safe-string-operations)
   - [String Comparison](#string-comparison)
   - [String Modification](#string-modification)
   - [String Validation](#string-validation)
   - [String Builder](#string-builder)
   - [API Reference](#string-utilities-api-reference)
5. [Trie](#5-trie)
   - [Data Structures](#trie-data-structures)
   - [Insertion Algorithm](#insertion-algorithm)
   - [Search Algorithm](#search-algorithm)
   - [Prefix Matching](#prefix-matching)
   - [API Reference](#trie-api-reference)
   - [Performance Analysis](#trie-performance)
6. [Performance Analysis](#6-performance-analysis)
7. [Integration Examples](#7-integration-examples)
8. [Best Practices](#8-best-practices)
9. [Extension Points](#9-extension-points)
10. [Testing](#10-testing)

---

## 1. Overview

The utilities library provides foundational data structures and helper functions used throughout the Necromancer's Shell project. It consists of four main components:

- **Hash Table**: Generic string-keyed dictionary with automatic resizing
- **Logger**: Multi-level logging system with file and console output
- **String Utilities**: Safe string manipulation and StringBuilder
- **Trie**: Prefix tree for efficient autocomplete functionality

### Design Philosophy

1. **Memory Safety**: All utilities handle NULL pointers gracefully and perform bounds checking
2. **Ownership Clarity**: Clear documentation of memory ownership and caller responsibilities
3. **Performance**: Optimized algorithms with documented time complexity
4. **Modularity**: Each component is independent and can be used separately
5. **Error Handling**: Consistent error handling patterns with boolean return values

### Component Relationships

```
┌─────────────────────────────────────────────────────┐
│                  Application Layer                   │
│  (Commands, Registry, Autocomplete, Game Loop)       │
└─────────────────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────┐
│                 Utilities Library                    │
│                                                       │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌─────┐ │
│  │   Hash   │  │  Logger  │  │  String  │  │Trie │ │
│  │  Table   │  │          │  │  Utils   │  │     │ │
│  └──────────┘  └──────────┘  └──────────┘  └─────┘ │
└─────────────────────────────────────────────────────┘
```

**Usage Patterns:**
- **Command Registry** uses **Hash Table** to store command mappings
- **Autocomplete** uses **Trie** for command prefix matching
- **Parser** uses **String Utils** for input tokenization
- **All Components** use **Logger** for debugging and error reporting

---

## 2. Hash Table

The hash table provides a generic string-keyed dictionary implementation with automatic resizing and collision handling through open addressing with linear probing.

**Location**: `src/utils/hash_table.c`, `src/utils/hash_table.h`

### Data Structures

#### HashEntry (`src/utils/hash_table.c:9-14`)

```c
typedef struct {
    char* key;          /* Owned string */
    void* value;        /* User value */
    bool occupied;      /* Slot occupied */
    bool deleted;       /* Tombstone for removed entries */
} HashEntry;
```

Each entry contains:
- **key**: Owned string (duplicated on insertion)
- **value**: User-managed pointer (not freed by hash table)
- **occupied**: Indicates if slot is in use
- **deleted**: Tombstone marker for removed entries (preserves probing chain)

#### HashTable (`src/utils/hash_table.c:17-22`)

```c
struct HashTable {
    HashEntry* entries;
    size_t capacity;
    size_t size;
    size_t deleted_count;
};
```

- **entries**: Dynamic array of HashEntry structures
- **capacity**: Total number of slots
- **size**: Number of active entries (excludes deleted)
- **deleted_count**: Number of tombstoned entries (triggers rehashing)

### Hash Function

**Implementation**: FNV-1a hash algorithm (`src/utils/hash_table.c:30-39`)

```c
static size_t hash_string(const char* str) {
    size_t hash = 2166136261u;

    while (*str) {
        hash ^= (unsigned char)(*str++);
        hash *= 16777619u;
    }

    return hash;
}
```

**Algorithm**: FNV-1a (Fowler-Noll-Vo hash function)
- **Initial value**: 2166136261u (FNV offset basis)
- **Prime multiplier**: 16777619u (FNV prime)
- **Operation**: XOR each byte with hash, then multiply by prime

**Properties**:
- Fast computation: O(n) where n = string length
- Good distribution for short strings (ideal for command names)
- Non-cryptographic (speed over security)

### Collision Handling

**Strategy**: Open addressing with linear probing (`src/utils/hash_table.c:42-72`)

```c
static HashEntry* find_entry(HashEntry* entries, size_t capacity, const char* key) {
    size_t index = hash_string(key) % capacity;
    HashEntry* tombstone = NULL;

    /* Linear probing */
    for (size_t i = 0; i < capacity; i++) {
        HashEntry* entry = &entries[index];

        if (!entry->occupied) {
            /* Found empty slot */
            if (!entry->deleted) {
                /* Return tombstone if we found one, otherwise this empty slot */
                return tombstone ? tombstone : entry;
            } else {
                /* Remember first tombstone */
                if (!tombstone) {
                    tombstone = entry;
                }
            }
        } else if (strcmp(entry->key, key) == 0) {
            /* Found existing key */
            return entry;
        }

        /* Try next slot */
        index = (index + 1) % capacity;
    }

    /* Table is full (should not happen due to resizing) */
    return tombstone;
}
```

**Algorithm Details**:
1. Compute initial index: `hash(key) % capacity`
2. Linear probe: Check consecutive slots until finding:
   - Empty slot (not occupied, not deleted)
   - Tombstone (deleted entry - can be reused)
   - Matching key (update existing)
3. Wraparound: Use modulo for circular indexing
4. Tombstone optimization: Reuse deleted slots to minimize probing distance

**Worst Case**: O(n) when many collisions occur (mitigated by load factor management)

### Dynamic Resizing

**Trigger Conditions** (`src/utils/hash_table.c:157-158`):
- **Upsize**: When `(size + deleted_count) / capacity > MAX_LOAD_FACTOR (0.7)`
- **Downsize**: When `size < capacity / 4` and `capacity > MIN_CAPACITY (8)`

**Resize Algorithm** (`src/utils/hash_table.c:75-109`):

```c
static bool resize_table(HashTable* table, size_t new_capacity) {
    /* Allocate new entries */
    HashEntry* new_entries = calloc(new_capacity, sizeof(HashEntry));
    if (!new_entries) {
        LOG_ERROR("Failed to allocate hash table entries");
        return false;
    }

    /* Rehash existing entries */
    size_t new_size = 0;
    for (size_t i = 0; i < table->capacity; i++) {
        HashEntry* old_entry = &table->entries[i];
        if (old_entry->occupied && !old_entry->deleted) {
            HashEntry* new_entry = find_entry(new_entries, new_capacity, old_entry->key);
            new_entry->key = old_entry->key;  /* Transfer ownership */
            new_entry->value = old_entry->value;
            new_entry->occupied = true;
            new_entry->deleted = false;
            new_size++;
        } else if (old_entry->occupied) {
            /* Free deleted entries */
            free(old_entry->key);
        }
    }

    /* Replace old table */
    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    table->size = new_size;
    table->deleted_count = 0;

    LOG_DEBUG("Hash table resized to capacity %zu", new_capacity);
    return true;
}
```

**Process**:
1. Allocate new array with `new_capacity`
2. Rehash all active entries into new array
3. Transfer key ownership (avoid reallocation)
4. Free tombstoned entry keys
5. Replace old array with new
6. Reset deleted counter (all tombstones removed)

**Growth Strategy**: Double capacity on upsize (`capacity * 2`)
**Shrink Strategy**: Half capacity on downsize (`capacity / 2`)

### Hash Table API Reference

#### Creation and Destruction

**hash_table_create** (`src/utils/hash_table.c:111-135`)
```c
HashTable* hash_table_create(size_t initial_capacity);
```
- Creates a new hash table with specified initial capacity
- Minimum capacity: 8 entries (`MIN_CAPACITY`)
- Returns: Hash table pointer or NULL on allocation failure
- Caller must call `hash_table_destroy()` to free memory

**hash_table_destroy** (`src/utils/hash_table.c:137-151`)
```c
void hash_table_destroy(HashTable* table);
```
- Frees all keys and the hash table structure
- Does NOT free values (caller's responsibility)
- Safe to call with NULL pointer
- Logs destruction at DEBUG level

#### Insertion and Retrieval

**hash_table_put** (`src/utils/hash_table.c:153-193`)
```c
bool hash_table_put(HashTable* table, const char* key, void* value);
```
- Inserts or updates key-value pair
- Keys are duplicated using `strdup()` (table owns keys)
- Values are stored as-is (caller owns values)
- Triggers resize if load factor exceeds 0.7
- Returns: true on success, false on allocation failure

**hash_table_get** (`src/utils/hash_table.c:195-204`)
```c
void* hash_table_get(const HashTable* table, const char* key);
```
- Retrieves value for given key
- Returns: Value pointer or NULL if not found
- O(1) average, O(n) worst case
- NULL-safe: returns NULL for NULL table or key

**hash_table_contains** (`src/utils/hash_table.c:206-211`)
```c
bool hash_table_contains(const HashTable* table, const char* key);
```
- Checks if key exists in table
- Returns: true if key exists, false otherwise
- More efficient than `hash_table_get()` when only checking existence

#### Removal and Clearing

**hash_table_remove** (`src/utils/hash_table.c:213-235`)
```c
void* hash_table_remove(HashTable* table, const char* key);
```
- Removes key-value pair using tombstone deletion
- Marks entry as deleted (preserves probing chain)
- Returns: Value pointer or NULL if not found
- Triggers shrink if size drops below 25% capacity
- Caller responsible for freeing returned value

**hash_table_clear** (`src/utils/hash_table.c:237-254`)
```c
void hash_table_clear(HashTable* table);
```
- Removes all entries from table
- Frees all keys (not values)
- Resets size and deleted_count to 0
- Does not resize capacity

#### Introspection

**hash_table_size** (`src/utils/hash_table.c:256-258`)
```c
size_t hash_table_size(const HashTable* table);
```
- Returns number of active entries
- Excludes deleted (tombstoned) entries
- NULL-safe: returns 0 for NULL table

**hash_table_capacity** (`src/utils/hash_table.c:260-262`)
```c
size_t hash_table_capacity(const HashTable* table);
```
- Returns total number of slots
- Includes empty and deleted entries
- NULL-safe: returns 0 for NULL table

**hash_table_load_factor** (`src/utils/hash_table.c:264-267`)
```c
float hash_table_load_factor(const HashTable* table);
```
- Returns ratio of size to capacity (0.0 - 1.0)
- Formula: `(float)size / capacity`
- Useful for performance monitoring
- NULL-safe: returns 0.0 for NULL table

#### Iteration

**hash_table_foreach** (`src/utils/hash_table.c:269-278`)
```c
typedef void (*HashTableIterator)(const char* key, void* value, void* userdata);
void hash_table_foreach(const HashTable* table, HashTableIterator iterator, void* userdata);
```
- Iterates over all entries in table
- Calls iterator function for each key-value pair
- Order is arbitrary (based on internal storage)
- userdata passed through to each callback

**Example**:
```c
void print_entry(const char* key, void* value, void* userdata) {
    printf("Key: %s, Value: %p\n", key, value);
}

hash_table_foreach(table, print_entry, NULL);
```

### Hash Table Performance

| Operation | Average Case | Worst Case | Notes |
|-----------|-------------|------------|-------|
| Insert | O(1) | O(n) | Amortized O(1) with resizing |
| Lookup | O(1) | O(n) | Linear probing worst case |
| Delete | O(1) | O(n) | Tombstone deletion |
| Resize | O(n) | O(n) | Rehash all entries |
| Iteration | O(n) | O(n) | Must scan all slots |

**Memory Overhead**:
- Per entry: 24-32 bytes (key pointer, value pointer, 2 bools, padding)
- Total: `capacity * sizeof(HashEntry) + key_string_sizes`
- Load factor maintained at 0.7 max (30% empty slots)

**Best Use Cases**:
- Command registry (string name → command function)
- Environment variables (key → value)
- Small to medium datasets (< 10,000 entries)

---

## 3. Logger

The logger provides a multi-level logging system with file and console output, timestamp formatting, and ANSI color support.

**Location**: `src/utils/logger.c`, `src/utils/logger.h`

### Logger Data Structures

#### LogLevel Enumeration (`src/utils/logger.h:21-28`)

```c
typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;
```

Ordered by severity (TRACE lowest, FATAL highest). Log messages below the configured level are filtered out.

#### Logger State (`src/utils/logger.c:7-17`)

```c
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
```

**Global singleton state**:
- **file**: Log file handle (NULL for stdout-only)
- **level**: Minimum log level to output
- **console_enabled**: Whether to output to stderr
- **initialized**: Initialization flag

### Log Levels

#### Level Names (`src/utils/logger.c:20-22`)
```c
static const char* level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};
```

#### ANSI Color Codes (`src/utils/logger.c:25-32`)
```c
static const char* level_colors[] = {
    "\x1b[37m",  /* TRACE - white */
    "\x1b[36m",  /* DEBUG - cyan */
    "\x1b[32m",  /* INFO  - green */
    "\x1b[33m",  /* WARN  - yellow */
    "\x1b[31m",  /* ERROR - red */
    "\x1b[35m"   /* FATAL - magenta */
};
```

Colors applied to console output only (not file output).

### Output Formatting

#### File Format (`src/utils/logger.c:110-112`)
```
[YYYY-MM-DD HH:MM:SS] [LEVEL] [filename:line function] message
```

Example:
```
[2025-10-13 14:32:15] [INFO ] [main.c:42 main] Game initialized
[2025-10-13 14:32:16] [ERROR] [hash_table.c:79 resize_table] Failed to allocate hash table entries
```

#### Console Format (`src/utils/logger.c:117-119`)
```
[COLOR][YYYY-MM-DD HH:MM:SS] [LEVEL][RESET] [filename:line] message
```

Example (with ANSI colors):
```
\x1b[32m[2025-10-13 14:32:15] [INFO ]\x1b[0m [main.c:42] Game initialized
\x1b[31m[2025-10-13 14:32:16] [ERROR]\x1b[0m [hash_table.c:79] Failed to allocate
```

### Logger API Reference

#### Initialization and Shutdown

**logger_init** (`src/utils/logger.c:36-59`)
```c
bool logger_init(const char* filename, LogLevel level);
```
- Initializes logging system
- filename: Log file path (NULL for stdout-only mode)
- level: Minimum log level to output
- Opens file in append mode
- Writes startup marker with timestamp
- Returns: true on success, false on file open failure
- Can be called multiple times (automatically shuts down previous instance)

**logger_shutdown** (`src/utils/logger.c:61-72`)
```c
void logger_shutdown(void);
```
- Closes log file and writes end marker
- Flushes all pending output
- Safe to call multiple times
- Safe to call if not initialized

#### Configuration

**logger_set_level** (`src/utils/logger.c:74-76`)
```c
void logger_set_level(LogLevel level);
```
- Changes minimum log level at runtime
- Messages below this level are filtered
- Useful for debug vs release builds

**logger_get_level** (`src/utils/logger.c:78-80`)
```c
LogLevel logger_get_level(void);
```
- Returns current minimum log level
- Useful for conditional logging

**logger_set_console** (`src/utils/logger.c:82-84`)
```c
void logger_set_console(bool enable);
```
- Enables/disables console output
- File output unaffected
- Useful for silent operation

#### Logging

**logger_log** (`src/utils/logger.c:86-121`)
```c
void logger_log(LogLevel level, const char* file, int line,
                const char* func, const char* fmt, ...);
```
- Core logging function (not typically called directly)
- Uses printf-style formatting
- Filters by log level
- Extracts filename from full path
- Formats timestamp (YYYY-MM-DD HH:MM:SS)
- Writes to both file and console (if enabled)
- Flushes file immediately for ERROR and FATAL

#### Convenience Macros (`src/utils/logger.h:78-83`)

```c
#define LOG_TRACE(...) logger_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...) logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)  logger_log(LOG_LEVEL_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)  logger_log(LOG_LEVEL_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_FATAL(...) logger_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)
```

**Usage Examples**:
```c
LOG_INFO("Starting game loop");
LOG_DEBUG("Loading config from: %s", config_path);
LOG_WARN("Low memory warning: %zu bytes remaining", free_bytes);
LOG_ERROR("Failed to open file: %s", strerror(errno));
LOG_FATAL("Out of memory - cannot continue");
```

### Logger Configuration

**Recommended Configuration**:

**Debug Build**:
```c
logger_init("debug.log", LOG_LEVEL_DEBUG);
logger_set_console(true);  // Show all logs in terminal
```

**Release Build**:
```c
logger_init("game.log", LOG_LEVEL_INFO);
logger_set_console(false);  // Silent console operation
```

**Testing**:
```c
logger_init(NULL, LOG_LEVEL_TRACE);  // stdout only, verbose
```

### Thread Safety Considerations

**Current Implementation**: Not thread-safe
- Global state without synchronization
- File writes not atomic
- Multiple threads would cause race conditions

**Future Enhancement**: Add mutex for thread safety
```c
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void logger_log(...) {
    pthread_mutex_lock(&log_mutex);
    // ... logging code ...
    pthread_mutex_unlock(&log_mutex);
}
```

---

## 4. String Utilities

Comprehensive string manipulation utilities with NULL-safety, bounds checking, and a dynamic StringBuilder.

**Location**: `src/utils/string_utils.c`, `src/utils/string_utils.h`

### Safe String Operations

#### str_safe_copy (`src/utils/string_utils.c:21-35`)
```c
size_t str_safe_copy(char* dst, size_t dst_size, const char* src);
```
- Copies string with bounds checking
- Always null-terminates destination
- Truncates if source longer than buffer
- Returns: Number of characters copied (excluding null)
- NULL-safe: handles NULL src (sets dst to empty string)

**Example**:
```c
char buffer[16];
size_t copied = str_safe_copy(buffer, sizeof(buffer), "Hello, World!");
// buffer = "Hello, World!\0", copied = 13

size_t copied2 = str_safe_copy(buffer, sizeof(buffer), "This is a very long string");
// buffer = "This is a very \0", copied2 = 15 (truncated)
```

#### str_safe_concat (`src/utils/string_utils.c:38-53`)
```c
size_t str_safe_concat(char* dst, size_t dst_size, const char* src);
```
- Appends string with bounds checking
- Always null-terminates destination
- Truncates if combined length exceeds buffer
- Returns: Total length of result (excluding null)
- NULL-safe: returns current length if src is NULL

**Example**:
```c
char buffer[16] = "Hello";
str_safe_concat(buffer, sizeof(buffer), ", World!");
// buffer = "Hello, World!\0"
```

### String Comparison

#### str_compare (`src/utils/string_utils.c:61-66`)
```c
int str_compare(const char* s1, const char* s2);
```
- Compares two strings lexicographically
- Returns: 0 if equal, <0 if s1 < s2, >0 if s1 > s2
- NULL-safe: NULL < non-NULL, NULL == NULL

#### str_compare_ignore_case (`src/utils/string_utils.c:69-82`)
```c
int str_compare_ignore_case(const char* s1, const char* s2);
```
- Case-insensitive comparison
- Uses `tolower()` for each character
- NULL-safe
- **Time Complexity**: O(min(len(s1), len(s2)))

#### str_equals / str_equals_ignore_case (`src/utils/string_utils.c:85-92`)
```c
bool str_equals(const char* s1, const char* s2);
bool str_equals_ignore_case(const char* s1, const char* s2);
```
- Convenience wrappers returning boolean
- Clearer intent than comparing result to 0

#### str_starts_with (`src/utils/string_utils.c:95-105`)
```c
bool str_starts_with(const char* str, const char* prefix);
```
- Checks if string starts with prefix
- Returns false if either argument is NULL
- **Time Complexity**: O(len(prefix))

**Example**:
```c
str_starts_with("hello world", "hello")  // true
str_starts_with("hello world", "world")  // false
```

#### str_ends_with (`src/utils/string_utils.c:108-117`)
```c
bool str_ends_with(const char* str, const char* suffix);
```
- Checks if string ends with suffix
- Returns false if suffix longer than string
- **Time Complexity**: O(len(suffix))

**Example**:
```c
str_ends_with("hello.txt", ".txt")  // true
str_ends_with("hello.txt", ".c")    // false
```

### String Modification

#### str_trim (`src/utils/string_utils.c:150-173`)
```c
char* str_trim(char* str);
```
- Removes leading and trailing whitespace
- Modifies string in-place
- Uses `isspace()` for whitespace detection
- Returns: Pointer to modified string (same as input)

**Algorithm**:
1. Find first non-whitespace character
2. Find last non-whitespace character
3. Null-terminate after last non-whitespace
4. Use `memmove()` to shift string to beginning if needed

**Example**:
```c
char buffer[] = "  hello world  \n";
str_trim(buffer);
// buffer = "hello world"
```

#### str_trim_left / str_trim_right (`src/utils/string_utils.c:120-147`)
```c
char* str_trim_left(char* str);
char* str_trim_right(char* str);
```
- Trims whitespace from one end only
- In-place modification
- Returns modified string pointer

#### str_to_lower / str_to_upper (`src/utils/string_utils.c:176-195`)
```c
char* str_to_lower(char* str);
char* str_to_upper(char* str);
```
- Converts string case in-place
- Uses `tolower()` / `toupper()`
- Handles ASCII only
- Returns modified string pointer

**Example**:
```c
char buffer[] = "Hello World";
str_to_lower(buffer);
// buffer = "hello world"
```

#### str_duplicate (`src/utils/string_utils.c:198-201`)
```c
char* str_duplicate(const char* str);
```
- Allocates and copies string
- Wrapper for `strdup()` with NULL safety
- Caller must `free()` returned string
- Returns NULL if input is NULL or allocation fails

### String Searching

#### str_find_char (`src/utils/string_utils.c:204-207`)
```c
char* str_find_char(const char* str, char ch);
```
- Finds first occurrence of character
- Wrapper for `strchr()` with NULL safety
- Returns pointer to character or NULL

#### str_find_char_last (`src/utils/string_utils.c:210-213`)
```c
char* str_find_char_last(const char* str, char ch);
```
- Finds last occurrence of character
- Wrapper for `strrchr()` with NULL safety
- Returns pointer to character or NULL

#### str_find_substring (`src/utils/string_utils.c:216-219`)
```c
char* str_find_substring(const char* str, const char* substr);
```
- Finds first occurrence of substring
- Wrapper for `strstr()` with NULL safety
- Returns pointer to start of substring or NULL

### String Validation

#### str_is_empty_or_whitespace (`src/utils/string_utils.c:222-231`)
```c
bool str_is_empty_or_whitespace(const char* str);
```
- Checks if string is NULL, empty, or contains only whitespace
- Uses `isspace()` for whitespace detection
- Returns true for NULL or ""

#### str_is_digits (`src/utils/string_utils.c:234-243`)
```c
bool str_is_digits(const char* str);
```
- Checks if string contains only digits (0-9)
- Returns false for empty string or NULL
- Useful for validating numeric input

**Example**:
```c
str_is_digits("12345")   // true
str_is_digits("123.45")  // false (contains '.')
str_is_digits("")        // false
```

#### str_is_alnum (`src/utils/string_utils.c:246-255`)
```c
bool str_is_alnum(const char* str);
```
- Checks if string contains only alphanumeric characters
- Uses `isalnum()` (letters and digits)
- Returns false for empty string or NULL

### String Splitting

#### str_split (`src/utils/string_utils.c:258-280`)
```c
size_t str_split(char* str, char delimiter, char** tokens, size_t max_tokens);
```
- Splits string by delimiter character
- **Modifies original string** (replaces delimiters with null terminators)
- Stores pointers to tokens in array
- Returns: Number of tokens found (up to max_tokens)

**Example**:
```c
char input[] = "cmd arg1 arg2 arg3";
char* tokens[10];
size_t count = str_split(input, ' ', tokens, 10);
// count = 4
// tokens[0] = "cmd"
// tokens[1] = "arg1"
// tokens[2] = "arg2"
// tokens[3] = "arg3"
// input is now "cmd\0arg1\0arg2\0arg3"
```

**Important**: Do not free individual tokens - they point into the original string!

### String Builder

Dynamic string construction with automatic growth and efficient concatenation.

#### StringBuilder Structure (`src/utils/string_utils.c:14-18`)
```c
struct StringBuilder {
    char* buffer;
    size_t length;
    size_t capacity;
};
```

- **buffer**: Dynamic character array
- **length**: Current string length (excluding null)
- **capacity**: Allocated buffer size
- Default initial capacity: 256 bytes

#### str_builder_create (`src/utils/string_utils.c:284-308`)
```c
StringBuilder* str_builder_create(size_t initial_capacity);
```
- Creates new StringBuilder
- initial_capacity: 0 for default (256 bytes)
- Returns: StringBuilder pointer or NULL on failure
- Must be freed with `str_builder_destroy()`

#### str_builder_destroy (`src/utils/string_utils.c:310-317`)
```c
void str_builder_destroy(StringBuilder* builder);
```
- Frees StringBuilder and internal buffer
- Safe to call with NULL

#### str_builder_append (`src/utils/string_utils.c:341-356`)
```c
bool str_builder_append(StringBuilder* builder, const char* str);
```
- Appends string to builder
- Automatically grows buffer if needed (1.5x growth factor)
- Returns: true on success, false on allocation failure
- NULL string is silently ignored

#### str_builder_append_char (`src/utils/string_utils.c:358-370`)
```c
bool str_builder_append_char(StringBuilder* builder, char ch);
```
- Appends single character
- More efficient than appending 1-character string
- Automatically grows buffer if needed

#### str_builder_append_format (`src/utils/string_utils.c:372-401`)
```c
bool str_builder_append_format(StringBuilder* builder, const char* fmt, ...);
```
- Appends formatted string (printf-style)
- Uses `vsnprintf()` for safe formatting
- Automatically grows buffer to fit result
- Returns: true on success, false on allocation failure

**Example**:
```c
StringBuilder* sb = str_builder_create(0);
str_builder_append(sb, "Player: ");
str_builder_append(sb, player_name);
str_builder_append_format(sb, " (Level %d)", player_level);
printf("%s\n", str_builder_get(sb));
str_builder_destroy(sb);
```

#### str_builder_get (`src/utils/string_utils.c:403-405`)
```c
const char* str_builder_get(const StringBuilder* builder);
```
- Returns pointer to internal string
- String valid until builder is modified or destroyed
- Returns NULL for NULL builder

#### str_builder_length (`src/utils/string_utils.c:407-409`)
```c
size_t str_builder_length(const StringBuilder* builder);
```
- Returns current string length
- Excludes null terminator
- Returns 0 for NULL builder

#### str_builder_clear (`src/utils/string_utils.c:411-416`)
```c
void str_builder_clear(StringBuilder* builder);
```
- Clears content (sets length to 0)
- Does not free buffer (reusable)
- Useful for building multiple strings with same builder

#### str_builder_extract (`src/utils/string_utils.c:418-434`)
```c
char* str_builder_extract(StringBuilder* builder);
```
- Extracts string and transfers ownership to caller
- Builder is reset with new empty buffer
- Caller must `free()` returned string
- Returns NULL if builder is empty or allocation fails

**Example**:
```c
StringBuilder* sb = str_builder_create(0);
str_builder_append(sb, "Result: ");
str_builder_append_format(sb, "%d", 42);

char* result = str_builder_extract(sb);  // Ownership transferred
printf("%s\n", result);
free(result);  // Caller must free

// Builder is now empty and can be reused
str_builder_append(sb, "New string");
str_builder_destroy(sb);
```

### String Utilities API Reference

**Memory Management Summary**:

| Function | Modifies Input | Allocates Memory | Caller Frees |
|----------|----------------|------------------|--------------|
| str_safe_copy | Yes (dst) | No | No |
| str_safe_concat | Yes (dst) | No | No |
| str_trim | Yes | No | No |
| str_to_lower | Yes | No | No |
| str_duplicate | No | Yes | Yes |
| str_split | Yes | No | No (tokens point to input) |
| str_builder_* | No | Yes (internal) | Use destroy/extract |

---

## 5. Trie

Prefix tree implementation optimized for autocomplete and command name matching.

**Location**: `src/utils/trie.c`, `src/utils/trie.h`

### Trie Data Structures

#### TrieNode (`src/utils/trie.c:12-16`)
```c
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool is_end_of_word;
    char* word;  /* Store complete word at terminal nodes for easy retrieval */
} TrieNode;
```

- **children**: Array of 128 pointers (ASCII character set)
- **is_end_of_word**: Marks terminal nodes (complete words)
- **word**: Duplicated full word (stored at terminal nodes only)

**Memory Layout**:
```
Node for "help":
  children['h'] → Node
    children['e'] → Node
      children['l'] → Node
        children['p'] → Node (is_end_of_word = true, word = "help")
```

#### Trie (`src/utils/trie.c:19-22`)
```c
struct Trie {
    TrieNode* root;
    size_t size;
};
```

- **root**: Root node (always present, represents empty string)
- **size**: Number of words stored in trie

**Character Set**: ASCII only (0-127), defined by `ALPHABET_SIZE 128` (`src/utils/trie.c:9`)

### Insertion Algorithm

**trie_insert** (`src/utils/trie.c:104-133`)

```c
bool trie_insert(Trie* trie, const char* str) {
    if (!trie || !str) return false;

    TrieNode* node = trie->root;

    for (const char* c = str; *c != '\0'; c++) {
        unsigned char index = (unsigned char)*c;
        if (index >= ALPHABET_SIZE) {
            /* Non-ASCII characters not supported */
            continue;
        }

        if (!node->children[index]) {
            node->children[index] = trie_node_create();
            if (!node->children[index]) return false;
        }

        node = node->children[index];
    }

    /* Mark as end of word */
    if (!node->is_end_of_word) {
        node->is_end_of_word = true;
        node->word = strdup(str);
        if (!node->word) return false;
        trie->size++;
    }

    return true;
}
```

**Algorithm**:
1. Start at root node
2. For each character in string:
   - Convert to unsigned char (0-127)
   - Skip if non-ASCII (>= 128)
   - Create child node if doesn't exist
   - Move to child node
3. Mark final node as end-of-word
4. Store complete word at terminal node
5. Increment size if new word

**Time Complexity**: O(m) where m = string length
**Space Complexity**: O(m * ALPHABET_SIZE) worst case (one child per character)

**Duplicate Handling**: Inserting same word twice is safe (skips increment on second insert)

### Search Algorithm

**trie_contains** (`src/utils/trie.c:135-149`)

```c
bool trie_contains(const Trie* trie, const char* str) {
    if (!trie || !str) return false;

    TrieNode* node = trie->root;

    for (const char* c = str; *c != '\0'; c++) {
        unsigned char index = (unsigned char)*c;
        if (index >= ALPHABET_SIZE || !node->children[index]) {
            return false;
        }
        node = node->children[index];
    }

    return node->is_end_of_word;
}
```

**Algorithm**:
1. Start at root node
2. For each character:
   - Get child node for character
   - Return false if child doesn't exist
   - Move to child node
3. Return true only if final node is marked as end-of-word

**Time Complexity**: O(m) where m = string length
**Space Complexity**: O(1)

**Important**: Reaching a node is not sufficient - must check `is_end_of_word` flag.
Example: If "help" is inserted, "hel" exists as a path but returns false.

### Prefix Matching

**trie_find_with_prefix** (`src/utils/trie.c:182-206`)

```c
bool trie_find_with_prefix(const Trie* trie, const char* prefix,
                          char*** matches, size_t* count) {
    if (!trie || !prefix || !matches || !count) return false;

    *matches = NULL;
    *count = 0;

    /* Find the node corresponding to the prefix */
    TrieNode* node = trie->root;

    for (const char* c = prefix; *c != '\0'; c++) {
        unsigned char index = (unsigned char)*c;
        if (index >= ALPHABET_SIZE || !node->children[index]) {
            /* Prefix not found, return empty results */
            return true;
        }
        node = node->children[index];
    }

    /* Collect all words in the subtrie */
    size_t capacity = 0;
    collect_words(node, matches, count, &capacity);

    return true;
}
```

**Algorithm**:
1. Navigate to node representing prefix
2. Return empty array if prefix doesn't exist
3. Collect all words in subtree rooted at prefix node
4. Return allocated array of matching strings

**Collection Algorithm** (`src/utils/trie.c:54-81`)

```c
static void collect_words(TrieNode* node, char*** matches, size_t* count, size_t* capacity) {
    if (!node) return;

    if (node->is_end_of_word && node->word) {
        /* Grow array if needed */
        if (*count >= *capacity) {
            size_t new_capacity = (*capacity == 0) ? 8 : (*capacity * 2);
            char** new_matches = realloc(*matches, new_capacity * sizeof(char*));
            if (!new_matches) return;  /* Out of memory, stop collecting */

            *matches = new_matches;
            *capacity = new_capacity;
        }

        /* Add word to results */
        (*matches)[*count] = strdup(node->word);
        if ((*matches)[*count]) {
            (*count)++;
        }
    }

    /* Recursively collect from children */
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            collect_words(node->children[i], matches, count, capacity);
        }
    }
}
```

**Collection Process**:
- Depth-first search through subtree
- Dynamic array with 2x growth factor
- Initial capacity: 8 entries
- Duplicates all matching words

**Memory Management**: Caller must free results using `trie_free_matches()`

**Example**:
```c
// Trie contains: "help", "history", "status"
char** matches = NULL;
size_t count = 0;

trie_find_with_prefix(trie, "h", &matches, &count);
// count = 2, matches = ["help", "history"]

trie_free_matches(matches, count);
```

### Removal Algorithm

**trie_remove** (`src/utils/trie.c:151-180`)

```c
bool trie_remove(Trie* trie, const char* str) {
    if (!trie || !str) return false;

    /* Find the node */
    TrieNode* node = trie->root;

    for (const char* c = str; *c != '\0'; c++) {
        unsigned char index = (unsigned char)*c;
        if (index >= ALPHABET_SIZE || !node->children[index]) {
            return false;  /* String not found */
        }
        node = node->children[index];
    }

    if (!node->is_end_of_word) {
        return false;  /* String not found */
    }

    /* Mark as not end of word */
    node->is_end_of_word = false;
    free(node->word);
    node->word = NULL;
    trie->size--;

    /* Note: We don't actually remove nodes, just mark them as not terminal.
     * Full node removal would require parent tracking. This is simpler and
     * still efficient for our use case. */

    return true;
}
```

**Algorithm**:
1. Navigate to node representing word
2. Return false if word not found or node not terminal
3. Mark node as non-terminal (clear `is_end_of_word` flag)
4. Free stored word string
5. Decrement size

**Important Design Decision**: Nodes are not deleted, only marked as non-terminal.
- **Pros**: Simple implementation, no parent tracking needed
- **Cons**: Memory not reclaimed until trie destroyed
- **Use Case**: Acceptable for command autocomplete (small dataset, infrequent removal)

**Example**:
```c
trie_insert(trie, "help");
trie_insert(trie, "helper");

trie_remove(trie, "help");
// "help" no longer found, but path remains for "helper"

trie_contains(trie, "help");    // false
trie_contains(trie, "helper");  // true
```

### Trie API Reference

#### Creation and Destruction

**trie_create** (`src/utils/trie.c:83-95`)
```c
Trie* trie_create(void);
```
- Creates new empty trie
- Allocates root node
- Returns: Trie pointer or NULL on failure
- Must be freed with `trie_destroy()`

**trie_destroy** (`src/utils/trie.c:97-102`)
```c
void trie_destroy(Trie* trie);
```
- Recursively frees all nodes and stored words
- Safe to call with NULL
- Frees entire tree structure

**trie_node_destroy** (internal, `src/utils/trie.c:40-51`)
```c
static void trie_node_destroy(TrieNode* node);
```
- Recursive helper for destruction
- Post-order traversal (children before parent)
- Frees stored word strings

#### Operations

**trie_insert** - See [Insertion Algorithm](#insertion-algorithm)

**trie_contains** - See [Search Algorithm](#search-algorithm)

**trie_remove** - See [Removal Algorithm](#removal-algorithm)

**trie_find_with_prefix** - See [Prefix Matching](#prefix-matching)

**trie_free_matches** (`src/utils/trie.c:208-215`)
```c
void trie_free_matches(char** matches, size_t count);
```
- Frees array returned by `trie_find_with_prefix()`
- Frees each duplicated string
- Frees array itself
- Safe to call with NULL matches

**trie_size** (`src/utils/trie.c:217-220`)
```c
size_t trie_size(const Trie* trie);
```
- Returns number of words in trie
- O(1) operation (cached in structure)
- Returns 0 for NULL trie

**trie_clear** (`src/utils/trie.c:222-228`)
```c
void trie_clear(Trie* trie);
```
- Removes all words from trie
- Destroys entire tree and creates new root
- Resets size to 0
- More efficient than removing words individually

### Trie Performance

| Operation | Time Complexity | Space Complexity | Notes |
|-----------|----------------|------------------|-------|
| Insert | O(m) | O(m * k) | m = word length, k = alphabet size |
| Search | O(m) | O(1) | |
| Delete | O(m) | O(1) | Lazy deletion (no node removal) |
| Prefix Match | O(m + n*k) | O(n) | n = number of matches |
| Clear | O(total nodes) | O(1) | |

**Memory Analysis**:

Per node: `128 * 8 bytes (pointers) + 1 byte (bool) + 8 bytes (word pointer) = ~1032 bytes`

**Worst Case** (no prefix sharing):
- "a", "b", "c" → 3 nodes → ~3 KB
- "help", "history", "status" → 19 nodes → ~19 KB

**Best Case** (maximum prefix sharing):
- "abc", "abd", "abe" → 5 nodes (a→b→{c,d,e}) → ~5 KB

**Practical Example** (50 command names, average 7 characters):
- Estimated nodes: 200-300 (with typical prefix sharing)
- Memory usage: 200-300 KB
- Acceptable for autocomplete use case

**Space Optimization Opportunities**:
1. Use smaller alphabet (36 chars: a-z, 0-9) → saves 92 pointers per node
2. Use hash table for children instead of array → O(1) lookup, better space
3. Compressed trie (Patricia trie) → merge single-child paths

### Trie Use Cases in Necromancer's Shell

**Command Autocomplete** (`src/commands/autocomplete.c:19-43`):
```c
struct Autocomplete {
    const CommandRegistry* registry;
    Trie* command_trie;     /* Command names */
    Trie* custom_trie;      /* Custom entries */
};
```

**Usage Pattern**:
1. Initialize autocomplete with command registry
2. Build trie with all command names
3. On user input, find all commands with matching prefix
4. Display suggestions to user

**Example Flow**:
```
User types: "h"
→ trie_find_with_prefix(trie, "h", &matches, &count)
→ Display: "help", "history"

User types: "he"
→ trie_find_with_prefix(trie, "he", &matches, &count)
→ Display: "help"

User types: "hel"
→ trie_find_with_prefix(trie, "hel", &matches, &count)
→ Display: "help"
```

---

## 6. Performance Analysis

### Time Complexity Comparison

| Operation | Hash Table | Trie | String Utils | Logger |
|-----------|-----------|------|--------------|--------|
| Insert/Add | O(1) avg | O(m) | N/A | O(1) |
| Lookup | O(1) avg | O(m) | O(n) | N/A |
| Delete | O(1) avg | O(m) | N/A | N/A |
| Prefix Search | O(n) | O(m+k) | O(n) | N/A |
| Iteration | O(capacity) | O(nodes) | N/A | N/A |

Where:
- n = total string length or collection size
- m = query string length
- k = number of matches

### Memory Usage Patterns

**Hash Table**:
- Base: `capacity * 32 bytes` (entries)
- Keys: Sum of key string lengths
- Load factor: 0.7 max (30% overhead)
- **Example**: 100 commands, avg 10-char names → ~8 KB

**Trie**:
- Per node: ~1 KB (128 pointers + metadata)
- Total nodes: Depends on prefix sharing
- **Example**: 100 commands, avg 7 chars → ~200-300 KB

**String Utils**:
- StringBuilder: Initial 256 bytes, grows 1.5x
- String operations: In-place (no allocation) or O(n)

**Logger**:
- Global state: 32 bytes
- Per-log buffer: 1 KB (stack allocated)
- File buffer: System-managed

### Recommendation Table

| Use Case | Recommended Utility | Rationale |
|----------|-------------------|-----------|
| Command registry | Hash Table | O(1) lookup, small dataset |
| Autocomplete | Trie | Efficient prefix matching |
| Command parsing | String Utils | Safe tokenization |
| Error tracking | Logger | Centralized logging |
| Environment vars | Hash Table | Key-value pairs |
| History search | Trie or Sequential | Depends on search pattern |
| Config parsing | String Utils | Safe string manipulation |

### Benchmarking Results

**Note**: Benchmarks not currently in codebase. Recommended test scenarios:

```c
// Hash Table Benchmark
void benchmark_hash_table(void) {
    HashTable* table = hash_table_create(100);

    // Insert 10,000 entries
    uint64_t start = timing_get_ticks();
    for (int i = 0; i < 10000; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        hash_table_put(table, key, (void*)(uintptr_t)i);
    }
    uint64_t insert_time = timing_get_ticks() - start;

    // Lookup 10,000 entries
    start = timing_get_ticks();
    for (int i = 0; i < 10000; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key_%d", i);
        void* value = hash_table_get(table, key);
    }
    uint64_t lookup_time = timing_get_ticks() - start;

    printf("Hash Table: Insert=%lu us, Lookup=%lu us\n",
           insert_time, lookup_time);

    hash_table_destroy(table);
}
```

**Expected Performance** (rough estimates):
- Hash Table: 10,000 inserts + lookups in <1 ms
- Trie: 1,000 inserts in ~1 ms, 1,000 prefix searches in ~2 ms
- String Utils: 10,000 copies/trims in <1 ms
- Logger: 1,000 log writes in ~10-50 ms (file I/O bottleneck)

---

## 7. Integration Examples

### Command Registry Integration

**File**: `src/commands/registry.c:10-13`

```c
struct CommandRegistry {
    HashTable* commands; /* Maps command name -> CommandInfo* */
    size_t count;        /* Number of registered commands */
};
```

**Usage**:
```c
// Registration
CommandRegistry* registry = command_registry_create();
CommandInfo info = {
    .name = "help",
    .function = cmd_help,
    .description = "Show help",
    .min_args = 0,
    .max_args = 1
};
command_registry_register(registry, &info);

// Lookup
CommandInfo* cmd = hash_table_get(registry->commands, "help");
if (cmd) {
    cmd->function(args);  // Execute command
}
```

**Benefits**:
- O(1) command lookup by name
- Dynamic registration/unregistration
- Efficient iteration over all commands

### Autocomplete Integration

**File**: `src/commands/autocomplete.c:13-17`

```c
struct Autocomplete {
    const CommandRegistry* registry;
    Trie* command_trie;     /* Command names */
    Trie* custom_trie;      /* Custom entries */
};
```

**Usage**:
```c
// Initialize
Autocomplete* ac = autocomplete_create(registry);

// Add custom entries
autocomplete_add_entry(ac, "player_name");
autocomplete_add_entry(ac, "player_level");

// Get completions
char** matches = NULL;
size_t count = 0;
autocomplete_get_completions(ac, "pl", &matches, &count);
// matches = ["player_name", "player_level"]

// Display to user
for (size_t i = 0; i < count; i++) {
    printf("  %s\n", matches[i]);
}

trie_free_matches(matches, count);
```

**Benefits**:
- Fast prefix matching for user input
- Separate tries for commands vs. custom entries
- Easy rebuilding when commands change

### String Utils in Parser

**File**: `src/commands/parser.c`

```c
// Parse command line
char input_copy[1024];
str_safe_copy(input_copy, sizeof(input_copy), user_input);

// Trim whitespace
str_trim(input_copy);

// Check if empty
if (str_is_empty_or_whitespace(input_copy)) {
    return NULL;  // Empty command
}

// Split into tokens
char* tokens[32];
size_t token_count = str_split(input_copy, ' ', tokens, 32);

// tokens[0] = command name
// tokens[1..n] = arguments
```

**Benefits**:
- Safe string manipulation (no buffer overflows)
- NULL-safe operations
- In-place modification for efficiency

### Logger Throughout Codebase

**Initialization** (`src/main.c`):
```c
int main(void) {
    #ifdef DEBUG
        logger_init("debug.log", LOG_LEVEL_DEBUG);
    #else
        logger_init("game.log", LOG_LEVEL_INFO);
    #endif

    LOG_INFO("Necromancer's Shell starting...");

    // ... game code ...

    logger_shutdown();
    return 0;
}
```

**Error Handling** (`src/utils/hash_table.c:79-80`):
```c
HashEntry* new_entries = calloc(new_capacity, sizeof(HashEntry));
if (!new_entries) {
    LOG_ERROR("Failed to allocate hash table entries");
    return false;
}
```

**Debug Logging** (`src/utils/hash_table.c:133`):
```c
LOG_DEBUG("Created hash table with capacity %zu", initial_capacity);
```

**Benefits**:
- Centralized error tracking
- Consistent log format
- Easy to disable in production (set level to ERROR/FATAL)
- File + console output for debugging

### Memory Management Integration

**Pattern**: Utilities + Memory Pooling

```c
// Create memory pool for temporary strings
MemoryPool* string_pool = memory_pool_create(4096);

// Parse user input
char* input = memory_pool_alloc(string_pool, 256);
str_safe_copy(input, 256, user_input);
str_trim(input);

// Split into tokens (points into input, no allocation)
char* tokens[32];
size_t count = str_split(input, ' ', tokens, 32);

// Process tokens...

// Free entire pool at once (no individual frees needed)
memory_pool_destroy(string_pool);
```

**Benefits**:
- Reduced malloc/free overhead
- Automatic cleanup on error
- Better cache locality

---

## 8. Best Practices

### When to Use Each Utility

#### Hash Table
**Use When**:
- Need O(1) lookup by string key
- Dataset size known approximately
- Key-value associations needed
- Iteration order doesn't matter

**Avoid When**:
- Need sorted order (use tree instead)
- Need prefix/range queries (use trie instead)
- Keys are integers (use array instead)
- Memory extremely constrained (use linear search)

**Example**: Command registry, environment variables, configuration options

#### Trie
**Use When**:
- Need prefix-based searching
- Autocomplete functionality required
- Many strings share common prefixes
- Memory is available (>1KB per 10 strings)

**Avoid When**:
- No prefix queries needed (use hash table)
- Strings very long (deep trees)
- Character set very large (use compressed trie)
- Few strings (<10, linear search faster)

**Example**: Autocomplete, spell checking, IP routing tables

#### String Utilities
**Use When**:
- Parsing user input
- Building dynamic strings
- Need NULL-safe operations
- Bounds checking required

**Avoid When**:
- Performance critical (use raw C strings with care)
- Binary data (not null-terminated)
- Unicode needed (use UTF-8 library)

**Example**: Command parsing, formatting output, validation

#### Logger
**Use When**:
- Debugging complex issues
- Tracking errors in production
- Need audit trail
- Multiple severity levels

**Avoid When**:
- Performance critical path (use conditional compilation)
- User-facing messages (use direct output)
- Binary data logging (use separate mechanism)

**Example**: Error tracking, debug tracing, system events

### Memory Management Guidelines

#### Hash Table
```c
// Create
HashTable* table = hash_table_create(100);

// Insert (keys are duplicated, values not)
char* my_value = malloc(...);
hash_table_put(table, "key", my_value);  // key duplicated

// Remove (values returned, not freed)
void* removed_value = hash_table_remove(table, "key");
free(removed_value);  // Caller must free

// Destroy (keys freed, values not)
hash_table_foreach(table, free_value_callback, NULL);  // Free all values first
hash_table_destroy(table);  // Then destroy table
```

#### Trie
```c
// Create
Trie* trie = trie_create();

// Insert (strings duplicated)
trie_insert(trie, "command");  // String duplicated internally

// Find (matches allocated)
char** matches = NULL;
size_t count = 0;
trie_find_with_prefix(trie, "cmd", &matches, &count);
// Use matches...
trie_free_matches(matches, count);  // Must free

// Destroy (all internal memory freed)
trie_destroy(trie);  // No cleanup needed before
```

#### String Utils
```c
// Safe copying (no allocation)
char buffer[64];
str_safe_copy(buffer, sizeof(buffer), input);  // No free needed

// Duplication (allocated)
char* copy = str_duplicate(input);
// Use copy...
free(copy);  // Must free

// StringBuilder (allocated)
StringBuilder* sb = str_builder_create(0);
str_builder_append(sb, "text");

// Option 1: Extract (transfer ownership)
char* result = str_builder_extract(sb);
str_builder_destroy(sb);
// Use result...
free(result);  // Must free

// Option 2: Get (borrow reference)
const char* result = str_builder_get(sb);
// Use result immediately (don't store pointer)
str_builder_destroy(sb);
// result now invalid!
```

#### Logger
```c
// Initialize (opens file)
logger_init("game.log", LOG_LEVEL_INFO);

// Logging (no allocation)
LOG_INFO("Message: %s", text);  // Safe, no memory management

// Shutdown (closes file)
logger_shutdown();  // Must call to flush
```

### Error Handling Patterns

#### Check Return Values
```c
// Hash table
HashTable* table = hash_table_create(100);
if (!table) {
    LOG_ERROR("Failed to create hash table");
    return false;
}

if (!hash_table_put(table, "key", value)) {
    LOG_ERROR("Failed to insert into hash table");
    hash_table_destroy(table);
    return false;
}
```

#### NULL Checks
```c
// String utils are NULL-safe
char* input = get_user_input();  // May return NULL
str_trim(input);  // Safe even if input is NULL

// But check before using result
if (!input || str_is_empty_or_whitespace(input)) {
    LOG_WARN("Empty input received");
    return;
}
```

#### Cleanup on Error
```c
Trie* trie = trie_create();
if (!trie) goto error;

StringBuilder* sb = str_builder_create(0);
if (!sb) goto error_trie;

// ... use trie and sb ...

str_builder_destroy(sb);
trie_destroy(trie);
return true;

error_trie:
    trie_destroy(trie);
error:
    return false;
```

### Common Pitfalls

#### 1. Forgetting to Free Values in Hash Table
```c
// WRONG: Memory leak
hash_table_destroy(table);  // Keys freed, values leaked!

// CORRECT: Free values first
hash_table_foreach(table, free_value, NULL);
hash_table_destroy(table);
```

#### 2. Using str_split Results After Modifying Input
```c
char input[] = "cmd arg1 arg2";
char* tokens[10];
str_split(input, ' ', tokens, 10);
// tokens[0] points into input

strcpy(input, "new_input");  // WRONG: Invalidates tokens!
printf("%s\n", tokens[0]);    // Undefined behavior
```

#### 3. Storing StringBuilder Reference After Modification
```c
StringBuilder* sb = str_builder_create(0);
str_builder_append(sb, "hello");
const char* str1 = str_builder_get(sb);

str_builder_append(sb, " world");  // May reallocate buffer
printf("%s\n", str1);  // WRONG: str1 may be invalid!

// CORRECT: Get reference after all modifications
const char* str2 = str_builder_get(sb);
printf("%s\n", str2);
```

#### 4. Not Checking Logger Initialization
```c
// WRONG: Log before init
LOG_INFO("Starting...");  // Undefined behavior

// CORRECT: Init first
logger_init("game.log", LOG_LEVEL_INFO);
LOG_INFO("Starting...");
```

#### 5. Modifying Trie During Iteration
```c
// WRONG: Undefined behavior
char** matches = NULL;
size_t count = 0;
trie_find_with_prefix(trie, "cmd", &matches, &count);

for (size_t i = 0; i < count; i++) {
    trie_remove(trie, matches[i]);  // Modifying during iteration!
}

// CORRECT: Collect then modify
trie_find_with_prefix(trie, "cmd", &matches, &count);
for (size_t i = 0; i < count; i++) {
    trie_remove(trie, matches[i]);  // OK: Not iterating
}
trie_free_matches(matches, count);
```

---

## 9. Extension Points

### Custom Hash Functions

The hash table uses FNV-1a by default. To implement a custom hash function:

```c
// In hash_table.c, replace hash_string function:
static size_t hash_string(const char* str) {
    // Example: djb2 hash
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}
```

**Alternative Hash Functions**:
- **djb2**: Simple, fast, good distribution
- **MurmurHash**: Better distribution, slightly slower
- **CRC32**: Hardware-accelerated on modern CPUs

**Benchmarking**: Add to test suite
```c
void benchmark_hash_functions(void) {
    // Test collision rates for your specific dataset
}
```

### Custom Log Handlers

Currently logger outputs to file and console. To add custom handlers:

```c
// In logger.h, add callback type:
typedef void (*LogHandler)(LogLevel level, const char* message, void* userdata);

// In logger.c, add handler array:
static struct {
    LogHandler handlers[MAX_HANDLERS];
    void* userdata[MAX_HANDLERS];
    size_t handler_count;
} g_handlers;

// Add registration function:
void logger_add_handler(LogHandler handler, void* userdata);

// In logger_log, call handlers:
for (size_t i = 0; i < g_handlers.handler_count; i++) {
    g_handlers.handlers[i](level, msg_buf, g_handlers.userdata[i]);
}
```

**Example Handlers**:
- Network logger (send to remote server)
- GUI logger (display in window)
- Syslog logger (Unix system logging)
- Circular buffer logger (keep last N messages in memory)

### Adding New String Utilities

Common additions:

```c
// String replacement
char* str_replace(char* str, const char* old, const char* new);

// String padding
char* str_pad_left(char* str, size_t width, char pad_char);

// String joining
char* str_join(const char** strings, size_t count, const char* separator);

// Number parsing
bool str_parse_int(const char* str, int* out_value);
bool str_parse_float(const char* str, float* out_value);

// Unicode support
size_t str_utf8_length(const char* str);
bool str_utf8_validate(const char* str);
```

**Implementation Guidelines**:
- Follow existing NULL-safety patterns
- Document memory ownership clearly
- Add to appropriate section in header
- Include usage examples in comments

### Trie Optimizations

#### 1. Compressed Trie (Patricia Trie)
Merge nodes with single child to reduce memory:

```c
typedef struct TrieNode {
    struct TrieNode** children;  // Dynamic array
    char* edge_label;            // String fragment
    size_t child_count;
    bool is_end_of_word;
    char* word;
} TrieNode;
```

**Benefits**: 50-90% memory reduction for typical datasets
**Cost**: More complex insertion/deletion logic

#### 2. Hash Table for Children
Replace array with hash table for sparse character sets:

```c
typedef struct TrieNode {
    HashTable* children;  // char -> TrieNode*
    bool is_end_of_word;
    char* word;
} TrieNode;
```

**Benefits**: O(1) child lookup, 95% memory reduction
**Cost**: Hash table overhead (~32 bytes per node minimum)

#### 3. Custom Alphabet
Restrict to lowercase letters and numbers only:

```c
#define ALPHABET_SIZE 36  // a-z, 0-9

static int char_to_index(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    return -1;  // Invalid character
}
```

**Benefits**: 72% memory reduction (36 vs 128 pointers)
**Cost**: Only lowercase alphanumeric supported

#### 4. Ternary Search Tree
Alternative structure with better space/time tradeoff:

```c
typedef struct TSTNode {
    char c;
    struct TSTNode *left, *equal, *right;
    bool is_end_of_word;
    char* word;
} TSTNode;
```

**Benefits**: O(log n) per character, much better space efficiency
**Cost**: More complex implementation

---

## 10. Testing

### Test Suite Organization

**Test File**: `necromancers_shell/tests/test_trie.c`

Example test structure:
```c
static void test_insert_and_contains(void) {
    Trie* trie = trie_create();
    assert(trie != NULL);

    assert(trie_insert(trie, "help"));
    assert(trie_contains(trie, "help"));
    assert(!trie_contains(trie, "quit"));

    trie_destroy(trie);
    printf("[PASS] test_insert_and_contains\n");
}

int main(void) {
    test_insert_and_contains();
    test_prefix_matching();
    test_remove();
    test_clear();
    printf("\nAll tests passed!\n");
    return 0;
}
```

### Test Coverage

**Recommended Tests for Each Utility**:

#### Hash Table Tests
```c
test_hash_table_create_destroy()
test_hash_table_put_get()
test_hash_table_contains()
test_hash_table_remove()
test_hash_table_clear()
test_hash_table_resize_up()
test_hash_table_resize_down()
test_hash_table_collision_handling()
test_hash_table_foreach()
test_hash_table_null_safety()
```

#### Logger Tests
```c
test_logger_init_shutdown()
test_logger_levels()
test_logger_file_output()
test_logger_console_output()
test_logger_formatting()
test_logger_null_safety()
```

#### String Utils Tests
```c
test_str_safe_copy()
test_str_safe_concat()
test_str_compare()
test_str_trim()
test_str_to_lower()
test_str_split()
test_str_builder_append()
test_str_builder_format()
test_str_builder_extract()
test_str_validation()
```

#### Trie Tests
```c
test_trie_insert_and_contains()  // Implemented in test_trie.c:6-23
test_trie_prefix_matching()      // Implemented in test_trie.c:25-61
test_trie_remove()               // Implemented in test_trie.c:63-82
test_trie_clear()                // Implemented in test_trie.c:84-101
test_trie_null_safety()
test_trie_empty_string()
test_trie_long_strings()
```

### Memory Leak Detection

**Using Valgrind**:
```bash
# Compile with debug symbols
gcc -g -o test_trie tests/test_trie.c src/utils/trie.c -I src

# Run with valgrind
valgrind --leak-check=full --show-leak-kinds=all ./test_trie
```

**Expected Output**:
```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap usage: 150 allocs, 150 frees, 50,000 bytes allocated
==12345==
==12345== All heap blocks were freed -- no leaks are possible
```

### Performance Testing

**Benchmark Template**:
```c
#include "core/timing.h"

void benchmark_hash_table_insert(void) {
    const size_t NUM_ENTRIES = 10000;
    HashTable* table = hash_table_create(100);

    timing_init();
    uint64_t start = timing_get_ticks();

    for (size_t i = 0; i < NUM_ENTRIES; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key_%zu", i);
        hash_table_put(table, key, (void*)i);
    }

    uint64_t elapsed = timing_get_ticks() - start;
    double ms = elapsed / 1000.0;

    printf("Inserted %zu entries in %.2f ms (%.0f entries/sec)\n",
           NUM_ENTRIES, ms, NUM_ENTRIES / (ms / 1000.0));

    hash_table_destroy(table);
    timing_shutdown();
}
```

### Usage in Test Suite

The trie test file demonstrates proper usage patterns:

**Example 1: Basic Operations** (`tests/test_trie.c:6-23`)
```c
static void test_insert_and_contains(void) {
    Trie* trie = trie_create();
    assert(trie != NULL);

    assert(trie_insert(trie, "help"));
    assert(trie_insert(trie, "history"));
    assert(trie_insert(trie, "status"));

    assert(trie_contains(trie, "help"));
    assert(trie_contains(trie, "history"));
    assert(trie_contains(trie, "status"));
    assert(!trie_contains(trie, "quit"));

    assert(trie_size(trie) == 3);

    trie_destroy(trie);
    printf("[PASS] test_insert_and_contains\n");
}
```

**Example 2: Prefix Matching** (`tests/test_trie.c:25-61`)
```c
static void test_prefix_matching(void) {
    Trie* trie = trie_create();
    assert(trie != NULL);

    trie_insert(trie, "help");
    trie_insert(trie, "history");
    trie_insert(trie, "status");
    trie_insert(trie, "stats");

    char** matches = NULL;
    size_t count = 0;

    /* Find with prefix "h" */
    assert(trie_find_with_prefix(trie, "h", &matches, &count));
    assert(count == 2);
    trie_free_matches(matches, count);

    /* Find with prefix "st" */
    assert(trie_find_with_prefix(trie, "st", &matches, &count));
    assert(count == 2);
    trie_free_matches(matches, count);

    trie_destroy(trie);
    printf("[PASS] test_prefix_matching\n");
}
```

### Building and Running Tests

```bash
# Build trie tests
cd necromancers_shell
gcc -g -Wall -Wextra \
    -o test_trie \
    tests/test_trie.c \
    src/utils/trie.c \
    -I src

# Run tests
./test_trie

# Expected output:
# Running trie tests...
# [PASS] test_insert_and_contains
# [PASS] test_prefix_matching
# [PASS] test_remove
# [PASS] test_clear
# All trie tests passed!
```

---

## Appendix: Quick Reference

### Function Summary

#### Hash Table
| Function | Time | Description |
|----------|------|-------------|
| `hash_table_create(capacity)` | O(1) | Create new hash table |
| `hash_table_destroy(table)` | O(n) | Free hash table (not values) |
| `hash_table_put(table, key, value)` | O(1) | Insert/update entry |
| `hash_table_get(table, key)` | O(1) | Get value by key |
| `hash_table_contains(table, key)` | O(1) | Check if key exists |
| `hash_table_remove(table, key)` | O(1) | Remove entry |
| `hash_table_size(table)` | O(1) | Get entry count |
| `hash_table_foreach(table, fn, data)` | O(n) | Iterate all entries |

#### Logger
| Function | Description |
|----------|-------------|
| `logger_init(file, level)` | Initialize logging system |
| `logger_shutdown()` | Close and flush log file |
| `logger_set_level(level)` | Change minimum log level |
| `logger_set_console(enable)` | Enable/disable console output |
| `LOG_TRACE(fmt, ...)` | Log trace message |
| `LOG_DEBUG(fmt, ...)` | Log debug message |
| `LOG_INFO(fmt, ...)` | Log info message |
| `LOG_WARN(fmt, ...)` | Log warning message |
| `LOG_ERROR(fmt, ...)` | Log error message |
| `LOG_FATAL(fmt, ...)` | Log fatal error message |

#### String Utilities
| Function | Allocates | Modifies | Description |
|----------|-----------|----------|-------------|
| `str_safe_copy(dst, size, src)` | No | Yes | Safe string copy |
| `str_safe_concat(dst, size, src)` | No | Yes | Safe concatenation |
| `str_compare(s1, s2)` | No | No | Compare strings |
| `str_equals(s1, s2)` | No | No | Check equality |
| `str_trim(str)` | No | Yes | Remove whitespace |
| `str_to_lower(str)` | No | Yes | Convert to lowercase |
| `str_duplicate(str)` | Yes | No | Duplicate string |
| `str_split(str, delim, tokens, max)` | No | Yes | Split by delimiter |
| `str_builder_create(capacity)` | Yes | No | Create builder |
| `str_builder_append(sb, str)` | Maybe | Yes | Append string |
| `str_builder_get(sb)` | No | No | Get string (borrow) |
| `str_builder_extract(sb)` | Yes | Yes | Extract (transfer) |

#### Trie
| Function | Time | Description |
|----------|------|-------------|
| `trie_create()` | O(1) | Create new trie |
| `trie_destroy(trie)` | O(n) | Free trie and all nodes |
| `trie_insert(trie, str)` | O(m) | Insert string |
| `trie_contains(trie, str)` | O(m) | Check if string exists |
| `trie_remove(trie, str)` | O(m) | Remove string (lazy) |
| `trie_find_with_prefix(trie, prefix, matches, count)` | O(m+k) | Find all matching strings |
| `trie_free_matches(matches, count)` | O(k) | Free match array |
| `trie_size(trie)` | O(1) | Get word count |

Where: n = total items, m = string length, k = number of matches

---

**Document Version**: 1.0
**Last Updated**: 2025-10-13
**Project**: Necromancer's Shell
**Component**: Utilities Library

**See Also**:
- Architecture Documentation
- Command System Documentation
- Core Systems Documentation
- Terminal Interface Documentation

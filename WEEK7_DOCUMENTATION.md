# Week 7: Soul System + Resources - Documentation

**Necromancer's Shell - Week 7 Implementation**
**Date:** 2025-10-13
**Status:** Complete - Production Ready

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Architecture](#architecture)
3. [API Reference](#api-reference)
   - [Soul API](#soul-api)
   - [Soul Manager API](#soul-manager-api)
   - [Resources API](#resources-api)
   - [Corruption API](#corruption-api)
4. [Implementation Details](#implementation-details)
5. [Usage Guide](#usage-guide)
6. [Testing Documentation](#testing-documentation)
7. [Maintenance Guide](#maintenance-guide)

---

## System Overview

### Purpose

The Week 7 implementation provides the core resource management layer for Necromancer's Shell. It introduces four interconnected systems that form the foundation of the game's economy and progression mechanics:

1. **Soul System** - Individual soul management with type, quality, and energy
2. **Soul Manager** - Collection management with filtering, sorting, and querying
3. **Resources** - Primary resource tracking (soul energy, mana, time)
4. **Corruption** - Moral choice tracking with gameplay consequences

### Problem Statement

A dark fantasy RPG needs:
- **Resource Economy**: Tradeable currency (souls) with varying value
- **Moral Consequences**: Player choices that affect gameplay and story
- **Progression Tracking**: Time, energy, and power management
- **Memory-Safe Management**: Zero memory leaks in dynamic collections

### Solution Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Game Commands                          │
│              (raise, harvest, cast, etc.)                   │
└────────────────────┬────────────────────────────────────────┘
                     │
    ┌────────────────┴────────────────┐
    │                                 │
    ▼                                 ▼
┌───────────────┐              ┌──────────────┐
│ Soul Manager  │◄────────────►│  Resources   │
│  (Collection) │              │ (Energy/Time)│
└───────┬───────┘              └──────────────┘
        │                             │
        ▼                             ▼
   ┌─────────┐                 ┌──────────────┐
   │  Soul   │                 │  Corruption  │
   │ (Entity)│                 │   (State)    │
   └─────────┘                 └──────────────┘
```

### Design Philosophy

**Separation of Concerns:**
- `Soul` - Single entity, immutable after creation (except binding)
- `SoulManager` - Collection operations, owned memory
- `Resources` - Scalar values, simple arithmetic
- `Corruption` - State tracking with historical events

**Memory Safety:**
- Every `*_create()` has corresponding `*_destroy()`
- Manager takes ownership of added souls
- Dynamic arrays grow as needed (initial capacity: 100)
- Zero global state (all state passed explicitly)

**Performance:**
- Soul lookup: O(n) linear search (acceptable for hundreds of souls)
- Filtering: O(n) single-pass with early termination
- Sorting: O(n log n) using stdlib `qsort`
- Energy calculation: O(1) with precomputed formulas

---

## Architecture

### Component Breakdown

#### Soul (soul.h/c)
**Purpose**: Represents a single harvested soul
**Responsibility**: Immutable entity with calculated properties
**Location**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/`

**Key Features:**
- 6 soul types with distinct energy ranges
- Quality (0-100) determines energy multiplier
- Auto-generated memories based on type/quality
- Bind/unbind to minions
- Unique ID generation (monotonic counter)

#### Soul Manager (soul_manager.h/c)
**Purpose**: Manages collections of souls
**Responsibility**: CRUD operations, filtering, sorting
**Location**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/`

**Key Features:**
- Dynamic array (grows 2x when full)
- Filter by type, quality, bound status
- Sort by ID, type, quality, energy
- Aggregate queries (total energy, counts)
- Memory ownership (destroys souls on removal)

#### Resources (resources.h/c)
**Purpose**: Core game resources
**Responsibility**: Track energy, mana, time
**Location**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/resources/`

**Key Features:**
- Soul energy (currency from souls)
- Mana (spellcasting resource with regeneration)
- Time tracking (days + hours, 24-hour clock)
- Spend/add/check operations
- Time-of-day descriptors

#### Corruption (corruption.h/c)
**Purpose**: Moral choice tracking
**Responsibility**: Corruption level with event history
**Location**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/resources/`

**Key Features:**
- Corruption value (0-100)
- 5 threshold levels (Pure, Tainted, Compromised, Corrupted, Damned)
- Event history (up to 50 events)
- Penalty calculation (0-50% at max corruption)
- Pure/damned checks for branching logic

### Data Flow

**Soul Collection Flow:**
```
1. soul_create()           → Allocates soul with calculated energy
2. soul_manager_add()      → Manager takes ownership
3. soul_manager_get()      → Retrieve for operations
4. soul_bind()             → Attach to minion
5. soul_manager_remove()   → Destroy and free memory
```

**Energy Flow:**
```
1. Harvest soul            → soul_create() calculates energy
2. Add to manager          → soul_manager_add()
3. Query total energy      → soul_manager_total_unbound_energy()
4. Spend on action         → resources_spend_soul_energy()
5. Update corruption       → corruption_add() if morally questionable
```

**Time Flow:**
```
1. Action consumes time    → resources_advance_time(hours)
2. Time crosses midnight   → day_count++, time_hours wraps
3. Mana regenerates        → resources_regenerate_mana() called per hour
4. Events logged           → corruption_add() with current day
```

### Design Decisions

#### Why Dynamic Arrays?
**Decision**: Use dynamic arrays instead of linked lists for soul storage
**Rationale**:
- Better cache locality (souls stored contiguously)
- Simpler sorting with `qsort`
- Lower memory overhead (no next/prev pointers)
- Acceptable performance for 100-1000 souls

**Trade-off**: O(n) removal with element shifting vs O(1) in linked lists, but removals are rare compared to iteration.

#### Why These Soul Types?
**Decision**: 6 soul types (Common, Warrior, Mage, Innocent, Corrupted, Ancient)
**Rationale**:
- Covers combat/magic/civilian archetypes
- Innocent/Corrupted align with morality system
- Ancient provides high-value rare resource
- Distinct energy ranges create meaningful choices

**Energy Ranges:**
```
Common:    10-20   (baseline, plentiful)
Warrior:   20-40   (combat creatures)
Mage:      30-50   (magic users)
Innocent:  15-25   (triggers corruption)
Corrupted: 25-35   (already tainted)
Ancient:   50-100  (powerful, rare)
```

#### Why 0-100 Scales?
**Decision**: Quality, corruption, mana use 0-100 scale
**Rationale**:
- Intuitive percentage interpretation
- Fits in `uint8_t` for quality/corruption (memory efficient)
- Avoids floating-point precision issues
- Easy threshold checks (corruption >= 80)

**Formula Example** (Common soul energy):
```c
// Quality 0:   10 + (20-10) * (0/100)   = 10
// Quality 50:  10 + (20-10) * (50/100)  = 15
// Quality 100: 10 + (20-10) * (100/100) = 20
energy = base_min + (base_max - base_min) * (quality / 100)
```

#### Opaque vs. Exposed Structs
**Decision**: `SoulManager` is opaque, but `Soul`, `Resources`, `Corruption` are exposed
**Rationale**:
- `SoulManager` - Implementation detail (dynamic array may change)
- `Soul` - Immutable after creation, safe to expose
- `Resources` - Simple struct, no hidden invariants
- `Corruption` - Game state, commands need direct access

---

## API Reference

### Soul API

**Header**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/soul.h`

#### Types

```c
typedef enum {
    SOUL_TYPE_COMMON,     // 10-20 energy
    SOUL_TYPE_WARRIOR,    // 20-40 energy
    SOUL_TYPE_MAGE,       // 30-50 energy
    SOUL_TYPE_INNOCENT,   // 15-25 energy
    SOUL_TYPE_CORRUPTED,  // 25-35 energy
    SOUL_TYPE_ANCIENT,    // 50-100 energy
    SOUL_TYPE_COUNT
} SoulType;

typedef uint8_t SoulQuality;  // 0-100

typedef struct {
    uint32_t id;                              // Unique ID (auto-incremented)
    SoulType type;                            // Soul type
    SoulQuality quality;                      // Quality (0-100)
    char memories[SOUL_MEMORY_MAX_LENGTH];    // Flavor text (max 256 chars)
    uint32_t energy;                          // Calculated energy value
    bool bound;                               // Is bound to minion?
    uint32_t bound_minion_id;                 // Minion ID (0 if unbound)
    time_t timestamp;                         // Harvest timestamp
} Soul;
```

#### Functions

##### soul_create
```c
Soul* soul_create(SoulType type, SoulQuality quality);
```

**Purpose**: Allocates and initializes a new soul.

**Parameters**:
- `type` - Soul type (must be < SOUL_TYPE_COUNT)
- `quality` - Quality value (0-100, clamped if > 100)

**Returns**: Pointer to newly allocated soul, or NULL on failure (invalid type or malloc failure).

**Side Effects**:
- Increments global ID counter
- Calculates energy based on type and quality
- Generates random memories from type-specific templates
- Sets timestamp to current time

**Performance**: O(1)

**Example**:
```c
Soul* soul = soul_create(SOUL_TYPE_WARRIOR, 75);
if (soul) {
    printf("Created soul ID %u with %u energy\n", soul->id, soul->energy);
    soul_destroy(soul);
}
```

##### soul_destroy
```c
void soul_destroy(Soul* soul);
```

**Purpose**: Frees soul memory.

**Parameters**:
- `soul` - Pointer to soul (can be NULL)

**Returns**: Nothing.

**Side Effects**: Frees allocated memory.

**Thread Safety**: Not thread-safe (caller must synchronize).

**Example**:
```c
soul_destroy(soul);  // Safe even if soul is NULL
```

##### soul_type_name
```c
const char* soul_type_name(SoulType type);
```

**Purpose**: Returns human-readable name for soul type.

**Parameters**:
- `type` - Soul type

**Returns**: String like "Common", "Warrior", etc. Returns "Unknown" for invalid types.

**Performance**: O(1)

**Example**:
```c
printf("Type: %s\n", soul_type_name(SOUL_TYPE_MAGE));  // Output: Type: Mage
```

##### soul_calculate_energy
```c
uint32_t soul_calculate_energy(SoulType type, SoulQuality quality);
```

**Purpose**: Calculates energy value for soul based on type and quality.

**Parameters**:
- `type` - Soul type (determines base range)
- `quality` - Quality (0-100, determines position in range)

**Returns**: Calculated energy value.

**Formula**: `base_min + (base_max - base_min) * (quality / 100)`

**Performance**: O(1)

**Example**:
```c
uint32_t energy = soul_calculate_energy(SOUL_TYPE_ANCIENT, 100);
// Result: 100 (maximum ancient soul energy)
```

##### soul_bind
```c
bool soul_bind(Soul* soul, uint32_t minion_id);
```

**Purpose**: Binds soul to a minion, marking it as unavailable for other uses.

**Parameters**:
- `soul` - Pointer to soul
- `minion_id` - ID of minion to bind to

**Returns**: `true` on success, `false` if soul is NULL or already bound.

**Side Effects**: Sets `soul->bound = true` and `soul->bound_minion_id = minion_id`.

**Example**:
```c
if (soul_bind(soul, 42)) {
    printf("Soul bound to minion 42\n");
}
```

##### soul_unbind
```c
bool soul_unbind(Soul* soul);
```

**Purpose**: Unbinds soul from its minion.

**Parameters**:
- `soul` - Pointer to soul

**Returns**: `true` on success, `false` if soul is NULL or not bound.

**Side Effects**: Sets `soul->bound = false` and `soul->bound_minion_id = 0`.

**Example**:
```c
if (soul_unbind(soul)) {
    printf("Soul unbound, now available\n");
}
```

##### soul_generate_memories
```c
void soul_generate_memories(Soul* soul, SoulType type, SoulQuality quality);
```

**Purpose**: Generates flavor text memories based on type and quality.

**Parameters**:
- `soul` - Pointer to soul
- `type` - Soul type (selects memory template pool)
- `quality` - Quality (affects memory clarity)

**Side Effects**:
- Writes to `soul->memories`
- Low quality (< 30) truncates memories with "..."

**Algorithm**: Selects from 4 predefined templates per type using `quality % 4`.

**Example**:
```c
// Called automatically by soul_create(), but can be called manually:
soul_generate_memories(soul, SOUL_TYPE_WARRIOR, 50);
printf("Memories: %s\n", soul->memories);
```

##### soul_get_description
```c
int soul_get_description(const Soul* soul, char* buffer, size_t buffer_size);
```

**Purpose**: Formats human-readable description of soul.

**Parameters**:
- `soul` - Pointer to soul
- `buffer` - Buffer to write description
- `buffer_size` - Size of buffer

**Returns**: Number of characters written (excluding null terminator), or 0 on error.

**Format**: `"<Type> Soul (Quality: X%, Energy: Y) [BOUND to minion Z]\n  Memories: ..."`

**Example**:
```c
char buffer[512];
soul_get_description(soul, buffer, sizeof(buffer));
printf("%s\n", buffer);
// Output:
// Warrior Soul (Quality: 75%, Energy: 35) [BOUND to minion 42]
//   Memories: Battle cries echo through blood-soaked memories
```

---

### Soul Manager API

**Header**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/soul_manager.h`

#### Types

```c
typedef struct SoulManager SoulManager;  // Opaque structure

typedef struct {
    int type;                  // Soul type filter (-1 = any)
    SoulQuality quality_min;   // Minimum quality (0 = no min)
    SoulQuality quality_max;   // Maximum quality (100 = no max)
    int bound_filter;          // -1=any, 0=unbound only, 1=bound only
} SoulFilter;

typedef enum {
    SOUL_SORT_ID,           // Sort by ID (chronological)
    SOUL_SORT_TYPE,         // Sort by type
    SOUL_SORT_QUALITY,      // Sort by quality (ascending)
    SOUL_SORT_QUALITY_DESC, // Sort by quality (descending)
    SOUL_SORT_ENERGY,       // Sort by energy (ascending)
    SOUL_SORT_ENERGY_DESC   // Sort by energy (descending)
} SoulSortCriteria;
```

#### Functions

##### soul_manager_create
```c
SoulManager* soul_manager_create(void);
```

**Purpose**: Creates a new soul manager with initial capacity.

**Returns**: Pointer to manager, or NULL on allocation failure.

**Initial Capacity**: 100 souls (grows 2x when full).

**Performance**: O(1)

**Example**:
```c
SoulManager* manager = soul_manager_create();
if (!manager) {
    fprintf(stderr, "Failed to create soul manager\n");
    return -1;
}
```

##### soul_manager_destroy
```c
void soul_manager_destroy(SoulManager* manager);
```

**Purpose**: Destroys manager and all contained souls.

**Parameters**:
- `manager` - Pointer to manager (can be NULL)

**Side Effects**: Frees all souls and manager memory.

**Example**:
```c
soul_manager_destroy(manager);  // Safe even if NULL
```

##### soul_manager_add
```c
bool soul_manager_add(SoulManager* manager, Soul* soul);
```

**Purpose**: Adds soul to manager (manager takes ownership).

**Parameters**:
- `manager` - Pointer to manager
- `soul` - Pointer to soul

**Returns**: `true` on success, `false` on failure (NULL inputs or realloc failure).

**Side Effects**:
- Manager takes ownership of soul pointer
- Grows internal array if needed (2x capacity)
- Caller should NOT use soul pointer after this call

**Performance**: O(1) amortized (O(n) when growing array).

**Example**:
```c
Soul* soul = soul_create(SOUL_TYPE_MAGE, 80);
if (soul_manager_add(manager, soul)) {
    // Do NOT use 'soul' pointer anymore - manager owns it
    printf("Soul added successfully\n");
}
```

##### soul_manager_remove
```c
bool soul_manager_remove(SoulManager* manager, uint32_t soul_id);
```

**Purpose**: Removes and destroys soul by ID.

**Parameters**:
- `manager` - Pointer to manager
- `soul_id` - ID of soul to remove

**Returns**: `true` if found and removed, `false` otherwise.

**Side Effects**:
- Destroys soul (calls `soul_destroy`)
- Shifts remaining souls down in array

**Performance**: O(n) (linear search + element shift).

**Example**:
```c
if (soul_manager_remove(manager, soul_id)) {
    printf("Soul %u removed\n", soul_id);
}
```

##### soul_manager_get
```c
Soul* soul_manager_get(SoulManager* manager, uint32_t soul_id);
```

**Purpose**: Retrieves soul by ID without removing it.

**Parameters**:
- `manager` - Pointer to manager
- `soul_id` - ID of soul to retrieve

**Returns**: Pointer to soul, or NULL if not found.

**Lifetime**: Pointer valid until soul is removed or manager is destroyed.

**Performance**: O(n) linear search.

**Example**:
```c
Soul* soul = soul_manager_get(manager, soul_id);
if (soul) {
    printf("Found: %s soul with %u energy\n",
           soul_type_name(soul->type), soul->energy);
}
```

##### soul_manager_get_filtered
```c
Soul** soul_manager_get_filtered(SoulManager* manager,
                                 const SoulFilter* filter,
                                 size_t* count_out);
```

**Purpose**: Returns filtered array of soul pointers matching criteria.

**Parameters**:
- `manager` - Pointer to manager
- `filter` - Filter criteria (NULL for all souls)
- `count_out` - Output parameter for count of returned souls

**Returns**: Array of soul pointers (must be freed by caller), or NULL if no matches.

**Memory**: Caller must `free()` returned array (but NOT the soul pointers themselves).

**Performance**: O(n) two-pass (count matches, then fill array).

**Example**:
```c
SoulFilter filter = soul_filter_by_type(SOUL_TYPE_WARRIOR);
size_t count;
Soul** warriors = soul_manager_get_filtered(manager, &filter, &count);
if (warriors) {
    for (size_t i = 0; i < count; i++) {
        printf("Warrior soul: %u energy\n", warriors[i]->energy);
    }
    free(warriors);  // Free the array (not the souls)
}
```

##### soul_manager_sort
```c
void soul_manager_sort(SoulManager* manager, SoulSortCriteria criteria);
```

**Purpose**: Sorts souls in manager by specified criteria.

**Parameters**:
- `manager` - Pointer to manager
- `criteria` - Sort criteria (ID, type, quality, energy)

**Side Effects**: Reorders internal soul array.

**Performance**: O(n log n) using `qsort`.

**Example**:
```c
// Sort by quality descending (highest quality first)
soul_manager_sort(manager, SOUL_SORT_QUALITY_DESC);
```

##### soul_manager_count
```c
size_t soul_manager_count(SoulManager* manager);
```

**Purpose**: Returns total number of souls.

**Parameters**:
- `manager` - Pointer to manager

**Returns**: Count of souls, or 0 if manager is NULL.

**Performance**: O(1)

##### soul_manager_count_by_type
```c
size_t soul_manager_count_by_type(SoulManager* manager, SoulType type);
```

**Purpose**: Counts souls of a specific type.

**Parameters**:
- `manager` - Pointer to manager
- `type` - Soul type to count

**Returns**: Count of matching souls.

**Performance**: O(n) linear scan.

##### soul_manager_total_energy
```c
uint32_t soul_manager_total_energy(SoulManager* manager);
```

**Purpose**: Calculates sum of energy from all souls.

**Returns**: Total energy value.

**Performance**: O(n)

##### soul_manager_total_unbound_energy
```c
uint32_t soul_manager_total_unbound_energy(SoulManager* manager);
```

**Purpose**: Calculates sum of energy from unbound souls only.

**Returns**: Total energy from souls where `bound == false`.

**Use Case**: Displays available energy for player to spend.

**Performance**: O(n)

##### soul_manager_clear
```c
void soul_manager_clear(SoulManager* manager);
```

**Purpose**: Removes and destroys all souls.

**Side Effects**: Calls `soul_destroy` on all souls, sets count to 0.

**Performance**: O(n)

#### Filter Helpers

##### soul_filter_default
```c
SoulFilter soul_filter_default(void);
```

Returns filter matching all souls (no restrictions).

##### soul_filter_by_type
```c
SoulFilter soul_filter_by_type(SoulType type);
```

Returns filter matching only specified type.

##### soul_filter_unbound
```c
SoulFilter soul_filter_unbound(void);
```

Returns filter matching only unbound souls.

##### soul_filter_min_quality
```c
SoulFilter soul_filter_min_quality(SoulQuality min_quality);
```

Returns filter matching souls with quality >= min_quality.

---

### Resources API

**Header**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/resources/resources.h`

#### Types

```c
typedef struct {
    uint32_t soul_energy;    // Current soul energy available
    uint32_t mana;           // Current mana
    uint32_t mana_max;       // Maximum mana capacity
    uint32_t day_count;      // Days elapsed
    uint32_t time_hours;     // Current hour (0-23)
} Resources;
```

#### Functions

##### resources_init
```c
void resources_init(Resources* resources);
```

**Purpose**: Initializes resources to default values.

**Initial Values**:
- `soul_energy = 0`
- `mana = 100`
- `mana_max = 100`
- `day_count = 0`
- `time_hours = 0`

##### resources_add_soul_energy
```c
bool resources_add_soul_energy(Resources* resources, uint32_t amount);
```

**Purpose**: Adds soul energy (no cap).

**Returns**: `true` on success, `false` if resources is NULL.

##### resources_spend_soul_energy
```c
bool resources_spend_soul_energy(Resources* resources, uint32_t amount);
```

**Purpose**: Attempts to spend soul energy.

**Returns**: `true` if successful, `false` if insufficient energy or NULL.

**Example**:
```c
if (resources_spend_soul_energy(&res, 50)) {
    printf("Spent 50 energy\n");
} else {
    printf("Not enough energy\n");
}
```

##### resources_has_soul_energy
```c
bool resources_has_soul_energy(const Resources* resources, uint32_t amount);
```

**Purpose**: Checks if player has sufficient energy.

##### resources_add_mana
```c
bool resources_add_mana(Resources* resources, uint32_t amount);
```

**Purpose**: Adds mana, capped at `mana_max`.

##### resources_spend_mana
```c
bool resources_spend_mana(Resources* resources, uint32_t amount);
```

**Purpose**: Attempts to spend mana.

##### resources_has_mana
```c
bool resources_has_mana(const Resources* resources, uint32_t amount);
```

**Purpose**: Checks if player has sufficient mana.

##### resources_regenerate_mana
```c
void resources_regenerate_mana(Resources* resources, uint32_t amount);
```

**Purpose**: Regenerates mana (typically called per hour), capped at `mana_max`.

##### resources_increase_max_mana
```c
bool resources_increase_max_mana(Resources* resources, uint32_t amount);
```

**Purpose**: Permanently increases mana capacity.

##### resources_advance_time
```c
void resources_advance_time(Resources* resources, uint32_t hours);
```

**Purpose**: Advances game clock by hours.

**Side Effects**: Increments `day_count` when crossing midnight (24 hours).

**Example**:
```c
resources_advance_time(&res, 8);  // 8 hours pass
// If time was 20:00, now it's 4:00 next day
```

##### resources_format_time
```c
int resources_format_time(const Resources* resources, char* buffer, size_t buffer_size);
```

**Purpose**: Formats time as "Day X, HH:00".

**Returns**: Characters written.

##### resources_get_time_of_day
```c
const char* resources_get_time_of_day(const Resources* resources);
```

**Purpose**: Returns descriptive time string.

**Returns**: "midnight", "night", "morning", "afternoon", "evening"

**Ranges**:
- midnight: hour 0
- night: hours 1-5, 22-23
- morning: hours 6-11
- afternoon: hours 12-17
- evening: hours 18-21

---

### Corruption API

**Header**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/resources/corruption.h`

#### Types

```c
typedef enum {
    CORRUPTION_PURE = 0,         // 0-19%
    CORRUPTION_TAINTED = 20,     // 20-39%
    CORRUPTION_COMPROMISED = 40, // 40-59%
    CORRUPTION_CORRUPTED = 60,   // 60-79%
    CORRUPTION_DAMNED = 80       // 80-100%
} CorruptionLevel;

typedef struct {
    char description[128];  // Description of event
    int8_t change;          // Change in corruption (+/-)
    uint32_t day;           // Day when event occurred
} CorruptionEvent;

typedef struct {
    uint8_t corruption;                         // Current corruption (0-100)
    CorruptionEvent events[MAX_CORRUPTION_EVENTS]; // History (max 50)
    size_t event_count;                         // Number of events
} CorruptionState;
```

#### Functions

##### corruption_init
```c
void corruption_init(CorruptionState* state);
```

**Purpose**: Initializes corruption to 0 with empty history.

##### corruption_add
```c
bool corruption_add(CorruptionState* state, uint8_t amount,
                    const char* description, uint32_t day);
```

**Purpose**: Increases corruption (capped at 100) and records event.

**Parameters**:
- `state` - Corruption state
- `amount` - Amount to increase (0-100)
- `description` - Description of corrupting action
- `day` - Current game day

**Example**:
```c
corruption_add(&state, 15, "Harvested innocent souls", res.day_count);
```

##### corruption_reduce
```c
bool corruption_reduce(CorruptionState* state, uint8_t amount,
                       const char* description, uint32_t day);
```

**Purpose**: Decreases corruption (minimum 0) and records event.

##### corruption_get_level
```c
CorruptionLevel corruption_get_level(const CorruptionState* state);
```

**Purpose**: Returns current corruption level threshold.

##### corruption_level_name
```c
const char* corruption_level_name(CorruptionLevel level);
```

**Purpose**: Returns name of corruption level ("Pure", "Damned", etc.).

##### corruption_get_description
```c
const char* corruption_get_description(const CorruptionState* state);
```

**Purpose**: Returns descriptive flavor text for current level.

##### corruption_calculate_penalty
```c
float corruption_calculate_penalty(const CorruptionState* state);
```

**Purpose**: Calculates gameplay penalty multiplier.

**Formula**: `corruption / 200.0`

**Returns**: 0.0 (no penalty) to 0.5 (50% penalty at max corruption).

**Use Case**: Reduce effectiveness of "good" actions as corruption increases.

##### corruption_is_damned
```c
bool corruption_is_damned(const CorruptionState* state);
```

**Purpose**: Checks if corruption >= 80 (damned level).

##### corruption_is_pure
```c
bool corruption_is_pure(const CorruptionState* state);
```

**Purpose**: Checks if corruption < 20 (pure level).

##### corruption_get_latest_event
```c
const CorruptionEvent* corruption_get_latest_event(const CorruptionState* state);
```

**Purpose**: Returns most recent corruption event.

##### corruption_get_event
```c
const CorruptionEvent* corruption_get_event(const CorruptionState* state, size_t index);
```

**Purpose**: Returns event by index (0 = oldest).

---

## Implementation Details

### Soul Manager Storage

**Internal Structure:**
```c
struct SoulManager {
    Soul** souls;       // Dynamic array of soul pointers
    size_t count;       // Current number of souls
    size_t capacity;    // Capacity of souls array
};
```

**Growth Strategy:**
```c
#define INITIAL_CAPACITY 100
#define GROWTH_FACTOR 2

if (count >= capacity) {
    new_capacity = capacity * GROWTH_FACTOR;
    souls = realloc(souls, new_capacity * sizeof(Soul*));
    capacity = new_capacity;
}
```

**Why This Design:**
- Simple implementation with predictable behavior
- Good cache locality for iteration
- Doubling avoids frequent reallocations
- Initial capacity handles typical game sessions

**Memory Usage:**
- Empty manager: `sizeof(SoulManager) + 100 * sizeof(Soul*) = ~808 bytes`
- 100 souls: `808 + 100 * sizeof(Soul) = ~7,608 bytes`
- 200 souls (grown): `808 + 200 * sizeof(Soul*) + 200 * sizeof(Soul) = ~15,608 bytes`

### Filtering Algorithm

**Two-Pass Design:**
```c
// Pass 1: Count matching souls
for each soul:
    if matches_filter(soul, filter):
        match_count++

// Allocate result array
result = malloc(match_count * sizeof(Soul*))

// Pass 2: Fill result array
for each soul:
    if matches_filter(soul, filter):
        result[index++] = soul
```

**Why Not Single-Pass:**
- Need to know result count for allocation
- Could use dynamic array but adds complexity
- Two-pass is simple and predictable

**Filter Matching Logic:**
```c
bool matches = true;

// Check type filter
if (filter->type != -1 && soul->type != filter->type)
    matches = false;

// Check quality filter
if (soul->quality < filter->quality_min || soul->quality > filter->quality_max)
    matches = false;

// Check bound filter
if (filter->bound_filter == 0 && soul->bound)
    matches = false;
else if (filter->bound_filter == 1 && !soul->bound)
    matches = false;
```

**Performance**: O(n) where n = number of souls. Acceptable for hundreds of souls.

### Sorting Implementation

**Uses stdlib qsort:**
```c
void soul_manager_sort(SoulManager* manager, SoulSortCriteria criteria) {
    int (*comparator)(const void*, const void*) = select_comparator(criteria);
    qsort(manager->souls, manager->count, sizeof(Soul*), comparator);
}
```

**Comparator Example** (quality ascending):
```c
static int compare_by_quality(const void* a, const void* b) {
    const Soul* soul_a = *(const Soul**)a;
    const Soul* soul_b = *(const Soul**)b;

    if (soul_a->quality < soul_b->quality) return -1;
    if (soul_a->quality > soul_b->quality) return 1;
    return 0;
}
```

**Performance**: O(n log n) typical, O(n²) worst-case (quicksort).

### Energy Calculation

**Formula** (applied per type):
```c
uint32_t soul_calculate_energy(SoulType type, SoulQuality quality) {
    // Get base range for type
    uint32_t base_min, base_max;
    switch (type) {
        case SOUL_TYPE_COMMON:
            base_min = 10; base_max = 20; break;
        // ... other types
    }

    // Linear interpolation based on quality
    uint32_t range = base_max - base_min;
    uint32_t energy = base_min + (range * quality) / 100;

    return energy;
}
```

**Examples:**
```
Common (10-20):
  quality 0   → 10 + (10 * 0)/100   = 10
  quality 50  → 10 + (10 * 50)/100  = 15
  quality 100 → 10 + (10 * 100)/100 = 20

Ancient (50-100):
  quality 0   → 50 + (50 * 0)/100   = 50
  quality 50  → 50 + (50 * 50)/100  = 75
  quality 100 → 50 + (50 * 100)/100 = 100
```

**Why Linear:** Simple, predictable, no floating-point errors.

### Memory Management

**Ownership Rules:**

1. **Soul Creation:**
   - Caller owns soul after `soul_create()`
   - Caller must call `soul_destroy()` OR transfer to manager

2. **Manager Addition:**
   - `soul_manager_add()` takes ownership
   - Manager is responsible for freeing
   - Caller must NOT use soul pointer after add

3. **Manager Retrieval:**
   - `soul_manager_get()` returns borrowed reference
   - Pointer valid until soul removed or manager destroyed
   - Caller must NOT free the soul

4. **Filtered Results:**
   - `soul_manager_get_filtered()` returns array of borrowed references
   - Caller must `free()` the array, but NOT the soul pointers

**Example Patterns:**

```c
// Pattern 1: Create and add (manager owns)
Soul* soul = soul_create(SOUL_TYPE_MAGE, 80);
soul_manager_add(manager, soul);
// Do NOT use 'soul' pointer anymore

// Pattern 2: Create and keep (caller owns)
Soul* soul = soul_create(SOUL_TYPE_WARRIOR, 60);
printf("Energy: %u\n", soul->energy);
soul_destroy(soul);  // Caller must free

// Pattern 3: Get and use (borrowed reference)
Soul* soul = soul_manager_get(manager, soul_id);
if (soul) {
    printf("Type: %s\n", soul_type_name(soul->type));
    // Do NOT call soul_destroy(soul)
}

// Pattern 4: Filter and iterate (array ownership)
size_t count;
Soul** filtered = soul_manager_get_filtered(manager, &filter, &count);
for (size_t i = 0; i < count; i++) {
    printf("Soul %u\n", filtered[i]->id);
}
free(filtered);  // Free array, not souls
```

### Corruption Event Storage

**Fixed-Size Circular Buffer:**
```c
#define MAX_CORRUPTION_EVENTS 50

typedef struct {
    uint8_t corruption;
    CorruptionEvent events[MAX_CORRUPTION_EVENTS];
    size_t event_count;  // Capped at 50
} CorruptionState;
```

**When Full:**
- Stops recording new events (event_count stays at 50)
- Could be extended to circular buffer (overwrite oldest)
- 50 events sufficient for typical playthrough

**Memory Footprint:**
```c
sizeof(CorruptionState) = 1 + (128 + 1 + 4) * 50 + 8 = ~6,659 bytes
```

---

## Usage Guide

### Basic Soul Operations

**Creating and Managing Souls:**
```c
#include "souls/soul.h"
#include "souls/soul_manager.h"

// Create manager
SoulManager* manager = soul_manager_create();

// Harvest souls (create and add)
Soul* common = soul_create(SOUL_TYPE_COMMON, 50);
Soul* warrior = soul_create(SOUL_TYPE_WARRIOR, 75);
Soul* ancient = soul_create(SOUL_TYPE_ANCIENT, 90);

soul_manager_add(manager, common);
soul_manager_add(manager, warrior);
soul_manager_add(manager, ancient);

// Query total energy
uint32_t total = soul_manager_total_energy(manager);
printf("Total soul energy: %u\n", total);

// Cleanup
soul_manager_destroy(manager);
```

### Filtering Souls

**Find Specific Souls:**
```c
// Get all warrior souls
SoulFilter filter = soul_filter_by_type(SOUL_TYPE_WARRIOR);
size_t count;
Soul** warriors = soul_manager_get_filtered(manager, &filter, &count);

printf("Found %zu warrior souls:\n", count);
for (size_t i = 0; i < count; i++) {
    printf("  ID %u: %u energy, quality %u%%\n",
           warriors[i]->id, warriors[i]->energy, warriors[i]->quality);
}
free(warriors);

// Get high-quality unbound souls
filter = soul_filter_min_quality(75);
filter.bound_filter = 0;  // Unbound only
Soul** quality_souls = soul_manager_get_filtered(manager, &filter, &count);
// ... use and free
```

### Sorting Souls

**Display Sorted Lists:**
```c
// Sort by energy descending (highest first)
soul_manager_sort(manager, SOUL_SORT_ENERGY_DESC);

// Get all souls (already sorted in manager)
size_t count;
Soul** all_souls = soul_manager_get_filtered(manager, NULL, &count);

printf("Souls by energy (highest first):\n");
for (size_t i = 0; i < count; i++) {
    char desc[512];
    soul_get_description(all_souls[i], desc, sizeof(desc));
    printf("%s\n\n", desc);
}
free(all_souls);
```

### Resource Management

**Tracking Player Resources:**
```c
#include "resources/resources.h"

Resources res;
resources_init(&res);

// Start with some energy
resources_add_soul_energy(&res, 100);

// Try to spend energy
if (resources_spend_soul_energy(&res, 50)) {
    printf("Spent 50 energy. Remaining: %u\n", res.soul_energy);
} else {
    printf("Not enough energy!\n");
}

// Cast spell (costs mana)
if (resources_spend_mana(&res, 30)) {
    printf("Spell cast! Mana: %u/%u\n", res.mana, res.mana_max);
}

// Advance time
resources_advance_time(&res, 8);
char time_str[64];
resources_format_time(&res, time_str, sizeof(time_str));
printf("Time: %s (%s)\n", time_str, resources_get_time_of_day(&res));

// Regenerate mana each hour
resources_regenerate_mana(&res, 5);  // +5 mana per hour
```

### Corruption Tracking

**Recording Moral Choices:**
```c
#include "resources/corruption.h"

CorruptionState corruption;
corruption_init(&corruption);

// Player harvests souls
corruption_add(&corruption, 10, "Raised undead from graveyard", res.day_count);
printf("Corruption: %u%% (%s)\n",
       corruption.corruption,
       corruption_level_name(corruption_get_level(&corruption)));

// Player commits atrocity
corruption_add(&corruption, 35, "Harvested innocent villagers", res.day_count);
printf("Corruption: %u%% - %s\n",
       corruption.corruption,
       corruption_get_description(&corruption));

// Check if player is damned
if (corruption_is_damned(&corruption)) {
    printf("You are lost to darkness!\n");
}

// Calculate penalty for "good" actions
float penalty = corruption_calculate_penalty(&corruption);
printf("Good action effectiveness: %.0f%%\n", (1.0f - penalty) * 100);
```

### Integration Example (Command Handler)

**Hypothetical `harvest` Command:**
```c
CommandResult cmd_harvest(ParsedCommand* cmd) {
    // Get target type from arguments
    const char* target = parsed_command_get_arg(cmd, 0);

    // Determine soul type and corruption
    SoulType type;
    int corruption_change = 0;

    if (strcmp(target, "graveyard") == 0) {
        type = SOUL_TYPE_COMMON;
        corruption_change = 5;
    } else if (strcmp(target, "battlefield") == 0) {
        type = SOUL_TYPE_WARRIOR;
        corruption_change = 10;
    } else if (strcmp(target, "village") == 0) {
        type = SOUL_TYPE_INNOCENT;
        corruption_change = 25;  // Highly corrupting
    } else {
        return command_result_error("Unknown harvest target");
    }

    // Create souls (2-5 random)
    int num_souls = 2 + (rand() % 4);
    for (int i = 0; i < num_souls; i++) {
        SoulQuality quality = 30 + (rand() % 70);  // 30-100 quality
        Soul* soul = soul_create(type, quality);
        soul_manager_add(g_soul_manager, soul);

        // Add energy to resources
        resources_add_soul_energy(&g_resources, soul->energy);
    }

    // Add corruption
    char desc[128];
    snprintf(desc, sizeof(desc), "Harvested souls from %s", target);
    corruption_add(&g_corruption, corruption_change, desc, g_resources.day_count);

    // Advance time
    resources_advance_time(&g_resources, 2);  // Harvesting takes 2 hours

    // Build response
    char response[512];
    snprintf(response, sizeof(response),
             "Harvested %d %s souls (+%u energy)\n"
             "Corruption: %u%% (%s)\n"
             "Time: %s",
             num_souls, soul_type_name(type),
             num_souls * 20,  // Approximate energy
             g_corruption.corruption,
             corruption_level_name(corruption_get_level(&g_corruption)),
             resources_format_time(&g_resources, time_str, sizeof(time_str)));

    return command_result_success(response);
}
```

### Advanced Pattern: Energy Economy

**Converting Souls to Spendable Energy:**
```c
// Player harvests souls (adds to manager)
// ...

// When player wants to cast expensive ritual:
uint32_t ritual_cost = 500;

// Check available energy
uint32_t available = soul_manager_total_unbound_energy(g_soul_manager);
if (!resources_has_soul_energy(&g_resources, ritual_cost - available)) {
    // Need to consume souls
    uint32_t needed = ritual_cost - g_resources.soul_energy;

    // Get high-energy unbound souls
    soul_manager_sort(g_soul_manager, SOUL_SORT_ENERGY_DESC);
    SoulFilter filter = soul_filter_unbound();
    size_t count;
    Soul** souls = soul_manager_get_filtered(g_soul_manager, &filter, &count);

    uint32_t consumed_energy = 0;
    for (size_t i = 0; i < count && consumed_energy < needed; i++) {
        uint32_t soul_id = souls[i]->id;
        consumed_energy += souls[i]->energy;

        // Remove soul (converts to energy)
        soul_manager_remove(g_soul_manager, soul_id);
        resources_add_soul_energy(&g_resources, souls[i]->energy);
    }
    free(souls);
}

// Now spend energy
if (resources_spend_soul_energy(&g_resources, ritual_cost)) {
    printf("Ritual cast!\n");
}
```

---

## Testing Documentation

### Test Organization

**Test Files:**
1. **test_soul.c** - Soul entity tests (11 tests)
2. **test_soul_manager.c** - Collection management tests (11 tests)
3. **test_resources.c** - Resources + Corruption tests (14 tests)

**Total**: 36 tests, all passing.

**Location**: `/home/stripcheese/Necromancers Shell/necromancers_shell/tests/`

### Test Categories

#### Unit Tests (Single Function)

**Soul Tests:**
```
test_soul_create_basic          - Basic creation and field initialization
test_soul_create_invalid_type   - Null handling for invalid type
test_soul_create_quality_clamping - Quality clamped to 100
test_soul_destroy_null          - Null handling for destroy
test_soul_type_name             - Type name mapping
test_soul_calculate_energy      - Energy formula correctness
test_soul_bind                  - Binding logic
test_soul_unbind                - Unbinding logic
test_soul_generate_memories     - Memory generation
test_soul_get_description       - Description formatting
test_soul_unique_ids            - ID uniqueness
```

**Soul Manager Tests:**
```
test_soul_manager_create_destroy    - Basic lifecycle
test_soul_manager_add               - Adding souls
test_soul_manager_get               - Retrieval by ID
test_soul_manager_remove            - Removal and cleanup
test_soul_manager_count_by_type     - Type counting
test_soul_manager_total_energy      - Energy aggregation
test_soul_manager_total_unbound_energy - Unbound energy calculation
test_soul_manager_clear             - Clearing all souls
test_soul_manager_get_filtered      - Filtering logic
test_soul_manager_sort              - Sorting algorithms
test_soul_manager_large_scale       - Stress test (200 souls)
```

**Resources Tests:**
```
test_resources_init            - Initialization
test_resources_soul_energy     - Energy add/spend/check
test_resources_mana            - Mana operations with cap
test_resources_time            - Time advancement and rollover
test_resources_format_time     - Time formatting
test_resources_time_of_day     - Time-of-day descriptors
```

**Corruption Tests:**
```
test_corruption_init           - Initialization
test_corruption_add_reduce     - Adding/reducing with caps
test_corruption_levels         - Level threshold logic
test_corruption_level_names    - Name mapping
test_corruption_descriptions   - Description strings
test_corruption_penalty        - Penalty calculation
test_corruption_events         - Event recording and retrieval
test_corruption_event_overflow - Event buffer limits
```

#### Integration Tests

**test_soul_manager_get_filtered** - Tests interaction between:
- Soul creation
- Manager addition
- Filtering by type, quality, bound status
- Memory management (caller frees array)

**test_soul_manager_sort** - Tests:
- Multiple souls with different properties
- Sorting criteria application
- Retrieval of sorted results

#### Stress Tests

**test_soul_manager_large_scale:**
- Creates 200 souls (exceeds initial capacity of 100)
- Verifies dynamic growth
- Tests iteration over large collection
- Validates memory management at scale

### Test Coverage

**What's Tested:**
- All public API functions
- Null pointer handling
- Boundary conditions (0, 100, 255)
- Memory allocation failures (implicitly via valgrind)
- Edge cases (empty collections, max capacity)
- Integration between modules

**What's NOT Tested:**
- Thread safety (not required for single-player game)
- Performance benchmarks (tested manually)
- UI/rendering (not part of these modules)

### Running Tests

**Build and Run All Tests:**
```bash
cd /home/stripcheese/Necromancers\ Shell/necromancers_shell
make test
```

**Run Individual Test:**
```bash
./build/test_soul
./build/test_soul_manager
./build/test_resources
```

**Check for Memory Leaks:**
```bash
make valgrind
# Or manually:
valgrind --leak-check=full --show-leak-kinds=all ./build/test_soul
```

**Expected Output** (all tests):
```
=== Soul System Tests ===
Running test: soul_create_basic
  PASS
Running test: soul_create_invalid_type
  PASS
...
=== Test Results ===
Tests run: 11
Tests passed: 11
Tests failed: 0

All tests PASSED!
```

### Adding New Tests

**Pattern to Follow:**
```c
void test_new_feature(void) {
    TEST_START("test_new_feature");

    // Setup
    Soul* soul = soul_create(SOUL_TYPE_COMMON, 50);

    // Test condition
    TEST_ASSERT(soul != NULL, "Soul creation should succeed");
    TEST_ASSERT(soul->quality == 50, "Quality should be 50");

    // Cleanup
    soul_destroy(soul);

    TEST_PASS();
}

// Add to main():
int main(void) {
    // ... existing tests
    test_new_feature();
    // ...
}
```

**Test Naming Convention:**
- `test_<module>_<function>` for single function tests
- `test_<module>_<feature>` for feature tests
- `test_<module>_<scenario>` for integration tests

**Assertion Guidelines:**
- Test one thing per assertion (clear failure messages)
- Always test null handling
- Test boundary values (0, max, overflow)
- Clean up allocated resources

---

## Maintenance Guide

### Adding New Soul Types

**Step-by-Step Process:**

1. **Update SoulType Enum** (soul.h):
```c
typedef enum {
    SOUL_TYPE_COMMON,
    SOUL_TYPE_WARRIOR,
    SOUL_TYPE_MAGE,
    SOUL_TYPE_INNOCENT,
    SOUL_TYPE_CORRUPTED,
    SOUL_TYPE_ANCIENT,
    SOUL_TYPE_MYTHICAL,  // NEW
    SOUL_TYPE_COUNT
} SoulType;
```

2. **Add Type Name** (soul.c):
```c
static const char* SOUL_TYPE_NAMES[] = {
    "Common", "Warrior", "Mage", "Innocent", "Corrupted", "Ancient",
    "Mythical"  // NEW
};
```

3. **Add Energy Range** (soul.c, `soul_calculate_energy`):
```c
switch (type) {
    // ... existing cases
    case SOUL_TYPE_MYTHICAL:
        base_min = 100;
        base_max = 200;
        break;
}
```

4. **Add Memory Templates** (soul.c):
```c
static const char* MYTHICAL_MEMORIES[] = {
    "Echoes from before time itself began",
    "Power that shaped the very fabric of reality",
    "Knowledge that predates the gods",
    "Essence of creation itself"
};
```

5. **Update Memory Generation** (soul.c, `soul_generate_memories`):
```c
switch (type) {
    // ... existing cases
    case SOUL_TYPE_MYTHICAL:
        templates = MYTHICAL_MEMORIES;
        template_count = sizeof(MYTHICAL_MEMORIES) / sizeof(MYTHICAL_MEMORIES[0]);
        break;
}
```

6. **Add Tests** (test_soul.c):
```c
void test_soul_mythical_energy(void) {
    TEST_START("soul_mythical_energy");

    Soul* soul = soul_create(SOUL_TYPE_MYTHICAL, 100);
    TEST_ASSERT(soul->energy == 200, "Max mythical should be 200 energy");

    soul_destroy(soul);
    TEST_PASS();
}
```

7. **Update Documentation** (WEEK7_DOCUMENTATION.md):
```markdown
SOUL_TYPE_MYTHICAL - Mythical soul (100-200 energy)
```

### Modifying Energy Calculations

**Where to Change:**
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/soul.c`
- Function: `soul_calculate_energy()`

**Current Formula:**
```c
energy = base_min + (base_max - base_min) * (quality / 100)
```

**Example Modification** (exponential scaling):
```c
// Old: Linear
uint32_t range = base_max - base_min;
uint32_t energy = base_min + (range * quality) / 100;

// New: Exponential (quality has more impact at high values)
float quality_factor = (quality / 100.0f);
float scaled = quality_factor * quality_factor;  // Quadratic
uint32_t energy = base_min + (uint32_t)(range * scaled);

// Examples:
// quality 0:   10 + 10 * (0.0 * 0.0)   = 10
// quality 50:  10 + 10 * (0.5 * 0.5)   = 12.5 → 12
// quality 100: 10 + 10 * (1.0 * 1.0)   = 20
```

**What to Update:**
1. Change formula in `soul_calculate_energy()`
2. Update tests in `test_soul_calculate_energy()`
3. Update documentation energy examples
4. Run `make test` to verify

### Extending Filtering

**Adding New Filter Criteria:**

1. **Update SoulFilter Struct** (soul_manager.h):
```c
typedef struct {
    int type;
    SoulQuality quality_min;
    SoulQuality quality_max;
    int bound_filter;
    uint32_t min_energy;     // NEW
    bool only_ancient;       // NEW
} SoulFilter;
```

2. **Update Filter Logic** (soul_manager.c, `soul_manager_get_filtered`):
```c
// In matching loop:
if (filter->min_energy > 0 && soul->energy < filter->min_energy) {
    matches = false;
}

if (filter->only_ancient && soul->type != SOUL_TYPE_ANCIENT) {
    matches = false;
}
```

3. **Add Helper Function** (soul_manager.h/c):
```c
// Header
SoulFilter soul_filter_min_energy(uint32_t min_energy);

// Implementation
SoulFilter soul_filter_min_energy(uint32_t min_energy) {
    SoulFilter filter;
    filter.type = -1;
    filter.quality_min = 0;
    filter.quality_max = 100;
    filter.bound_filter = -1;
    filter.min_energy = min_energy;
    filter.only_ancient = false;
    return filter;
}
```

4. **Update Default Filter** (soul_manager.c):
```c
SoulFilter soul_filter_default(void) {
    SoulFilter filter;
    filter.type = -1;
    filter.quality_min = 0;
    filter.quality_max = 100;
    filter.bound_filter = -1;
    filter.min_energy = 0;        // NEW
    filter.only_ancient = false;   // NEW
    return filter;
}
```

5. **Add Tests:**
```c
void test_soul_manager_filter_by_energy(void) {
    TEST_START("soul_manager_filter_by_energy");

    SoulManager* manager = soul_manager_create();
    soul_manager_add(manager, soul_create(SOUL_TYPE_COMMON, 0));    // 10 energy
    soul_manager_add(manager, soul_create(SOUL_TYPE_WARRIOR, 100)); // 40 energy

    SoulFilter filter = soul_filter_min_energy(30);
    size_t count;
    Soul** filtered = soul_manager_get_filtered(manager, &filter, &count);

    TEST_ASSERT(count == 1, "Should have 1 soul with >= 30 energy");
    free(filtered);

    soul_manager_destroy(manager);
    TEST_PASS();
}
```

### Performance Tuning

**Optimization Opportunities:**

1. **Soul Lookup** (Currently O(n)):
```c
// Current: Linear search
Soul* soul_manager_get(SoulManager* manager, uint32_t soul_id) {
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->souls[i]->id == soul_id) {
            return manager->souls[i];
        }
    }
    return NULL;
}

// Optimized: Hash table lookup O(1)
// Add to SoulManager struct:
struct SoulManager {
    Soul** souls;
    size_t count;
    size_t capacity;
    HashTable* id_lookup;  // NEW: Maps soul_id → array index
};

// On add:
hash_table_insert(manager->id_lookup, soul->id, index);

// On get:
size_t* index = hash_table_get(manager->id_lookup, soul_id);
return index ? manager->souls[*index] : NULL;
```

2. **Filtering** (Currently O(n) two-pass):
```c
// Current: Two-pass (count + fill)

// Optimized: Single-pass with dynamic array
#define INITIAL_RESULT_SIZE 10

Soul** result = malloc(INITIAL_RESULT_SIZE * sizeof(Soul*));
size_t result_count = 0;
size_t result_capacity = INITIAL_RESULT_SIZE;

for (size_t i = 0; i < manager->count; i++) {
    if (matches_filter(manager->souls[i], filter)) {
        if (result_count >= result_capacity) {
            result_capacity *= 2;
            result = realloc(result, result_capacity * sizeof(Soul*));
        }
        result[result_count++] = manager->souls[i];
    }
}
```

3. **Energy Calculation** (Already O(1), but could precompute):
```c
// Current: Calculate on demand
uint32_t soul_calculate_energy(SoulType type, SoulQuality quality) {
    // ... switch statement and formula
}

// Optimized: Lookup table (trade memory for speed)
static uint32_t ENERGY_TABLE[SOUL_TYPE_COUNT][101];  // [type][quality]

// Initialize once at startup:
void soul_init_energy_table(void) {
    for (int type = 0; type < SOUL_TYPE_COUNT; type++) {
        for (int quality = 0; quality <= 100; quality++) {
            ENERGY_TABLE[type][quality] = calculate_energy(type, quality);
        }
    }
}

// Lookup is O(1):
uint32_t energy = ENERGY_TABLE[type][quality];
```

**When to Optimize:**
- Profile first (measure actual bottlenecks)
- Optimize when collection size > 1000 souls
- Optimize when filtering called per frame
- Don't optimize prematurely

**Profiling Commands:**
```bash
# Compile with profiling
gcc -pg -O2 ...

# Run tests
./build/test_soul_manager

# Generate profile
gprof ./build/test_soul_manager gmon.out > profile.txt

# Look for hot functions
grep -A 10 "time seconds" profile.txt
```

---

## Appendix

### File Locations

**Implementation:**
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/soul.h`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/soul.c`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/soul_manager.h`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/souls/soul_manager.c`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/resources/resources.h`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/resources/resources.c`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/resources/corruption.h`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/src/game/resources/corruption.c`

**Tests:**
- `/home/stripcheese/Necromancers Shell/necromancers_shell/tests/test_soul.c`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/tests/test_soul_manager.c`
- `/home/stripcheese/Necromancers Shell/necromancers_shell/tests/test_resources.c`

**Build:**
- `/home/stripcheese/Necromancers Shell/necromancers_shell/Makefile`

### Quick Reference Tables

#### Soul Energy Ranges

| Type       | Min | Max | Use Case                |
|------------|-----|-----|-------------------------|
| Common     | 10  | 20  | Basic minions           |
| Warrior    | 20  | 40  | Combat creatures        |
| Mage       | 30  | 50  | Spellcasting minions    |
| Innocent   | 15  | 25  | High corruption cost    |
| Corrupted  | 25  | 35  | Already tainted         |
| Ancient    | 50  | 100 | Rare, powerful resource |

#### Corruption Thresholds

| Level        | Range   | Description                           |
|--------------|---------|---------------------------------------|
| Pure         | 0-19    | Untainted by dark magic               |
| Tainted      | 20-39   | Beginning to stray                    |
| Compromised  | 40-59   | Morality weakening                    |
| Corrupted    | 60-79   | Embracing darkness                    |
| Damned       | 80-100  | Consumed by evil                      |

#### Time-of-Day Ranges

| Period    | Hours | Description  |
|-----------|-------|--------------|
| Midnight  | 0     | Exact hour   |
| Night     | 1-5   | Early morning|
| Morning   | 6-11  | Daytime      |
| Afternoon | 12-17 | Midday       |
| Evening   | 18-21 | Dusk         |
| Night     | 22-23 | Late evening |

### Build Commands Reference

```bash
# Build release version
make release

# Build debug version
make debug

# Run all tests
make test

# Check memory leaks
make valgrind

# Static analysis
make analyze

# Format code
make format

# Clean build
make clean
```

### Common Pitfalls

**Pitfall 1: Using soul pointer after soul_manager_add()**
```c
// WRONG:
Soul* soul = soul_create(SOUL_TYPE_COMMON, 50);
soul_manager_add(manager, soul);
printf("Energy: %u\n", soul->energy);  // UNDEFINED BEHAVIOR

// CORRECT:
Soul* soul = soul_create(SOUL_TYPE_COMMON, 50);
uint32_t energy = soul->energy;  // Copy before transfer
soul_manager_add(manager, soul);
printf("Energy: %u\n", energy);
```

**Pitfall 2: Not freeing filtered array**
```c
// WRONG:
Soul** filtered = soul_manager_get_filtered(manager, &filter, &count);
// ... use filtered ...
// No free() - MEMORY LEAK

// CORRECT:
Soul** filtered = soul_manager_get_filtered(manager, &filter, &count);
// ... use filtered ...
free(filtered);
```

**Pitfall 3: Modifying filter with wrong sentinel values**
```c
// WRONG:
SoulFilter filter;
filter.type = 0;  // This means SOUL_TYPE_COMMON, not "any"

// CORRECT:
SoulFilter filter = soul_filter_default();  // Sets type = -1 (any)
filter.type = SOUL_TYPE_WARRIOR;  // Now set specific type
```

**Pitfall 4: Time rollover calculation**
```c
// WRONG:
res.time_hours += hours;
if (res.time_hours >= 24) {
    res.day_count++;
    res.time_hours = 0;  // Only handles single day rollover
}

// CORRECT (handles multiple days):
res.time_hours += hours;
while (res.time_hours >= 24) {
    res.time_hours -= 24;
    res.day_count++;
}
```

---

## Conclusion

The Week 7 Soul System and Resources implementation provides a solid foundation for Necromancer's Shell's game economy. All modules are production-ready with comprehensive tests and zero memory leaks.

**Key Achievements:**
- 4 interconnected systems (Soul, SoulManager, Resources, Corruption)
- 36 passing tests with full coverage
- Memory-safe dynamic collections
- Clean, maintainable API design
- Ready for command integration (Week 8+)

**Next Steps (Week 8):**
- Integrate with command system (harvest, raise, souls commands)
- Add UI feedback for corruption/resources
- Implement save/load for game state
- Add minion system (uses soul binding)

**Questions/Issues:**
Contact project maintainer or consult `/home/stripcheese/Necromancers Shell/CLAUDE.md` for project context.

---

**Document Version:** 1.0
**Last Updated:** 2025-10-13
**Author:** Claude Code Documentation System

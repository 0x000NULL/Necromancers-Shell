# Core Systems Documentation

**Necromancer's Shell - Core Game Engine Components**

This document provides comprehensive documentation for the core systems in Necromancer's Shell, including the memory management system, event bus, state manager, timing system, and game loop.

---

## Table of Contents

1. [Overview](#overview)
2. [Memory Pool Allocator](#memory-pool-allocator)
3. [Event Bus System](#event-bus-system)
4. [State Manager](#state-manager)
5. [Timing System](#timing-system)
6. [Game Loop](#game-loop)
7. [Integration Patterns](#integration-patterns)

---

## Overview

The core systems provide the foundational infrastructure for Necromancer's Shell. These systems are designed to be:

- **Performance-focused**: Memory pools reduce allocation overhead
- **Decoupled**: Event bus enables loose component coupling
- **Robust**: Comprehensive error checking and logging
- **Cross-platform**: Works on Windows and POSIX systems

**Location**: `/home/stripcheese/Necromancers Shell/necromancers_shell/src/core/`

**Components**:
- `memory.c/h` - Pool-based memory allocator
- `events.c/h` - Publish/subscribe event system
- `state_manager.c/h` - Game state machine
- `timing.c/h` - Cross-platform timing and FPS control
- `game_loop.c/h` - Main game loop abstraction

---

## Memory Pool Allocator

**Files**: `src/core/memory.c` (267 lines), `src/core/memory.h` (100 lines)

### Purpose

Provides fast, fixed-size block allocation with minimal fragmentation. Tracks allocations for debugging and leak detection. Designed for game objects that have predictable sizes and lifetimes.

### Data Structures

#### MemoryPool (Opaque)
```c
struct MemoryPool {
    void* memory;              /* Raw memory block */
    BlockHeader* free_list;    /* Free block list */
    size_t block_size;         /* Actual block size (including header) */
    size_t user_block_size;    /* User-visible block size */
    size_t block_count;
    size_t allocated_count;
    size_t peak_usage;
    size_t total_allocs;
    size_t total_frees;
};
```
*Source: src/core/memory.c:19-29*

#### BlockHeader (Internal)
```c
typedef struct BlockHeader {
    struct BlockHeader* next;  /* Next in free list */
    bool is_allocated;         /* Allocation flag */
    #ifdef DEBUG
    const char* file;          /* Allocation location */
    int line;
    #endif
} BlockHeader;
```
*Source: src/core/memory.c:9-16*

Each allocated block has a hidden header prepended to it. In DEBUG mode, this header stores allocation tracking information.

#### MemoryStats
```c
typedef struct {
    size_t total_bytes;        /* Total pool size */
    size_t block_size;         /* Size of each block */
    size_t block_count;        /* Total blocks */
    size_t allocated_blocks;   /* Currently allocated */
    size_t peak_usage;         /* Peak allocation count */
    size_t total_allocs;       /* Lifetime allocations */
    size_t total_frees;        /* Lifetime frees */
} MemoryStats;
```
*Source: src/core/memory.h:24-32*

### Core Functions

#### pool_create
```c
MemoryPool* pool_create(size_t block_size, size_t block_count);
```
*Source: src/core/memory.c:37-84*

Creates a memory pool with the specified block size and count.

**Algorithm**:
1. Validates parameters (non-zero sizes)
2. Allocates pool structure
3. Calculates aligned block size (block_size + header, aligned to pointer boundary)
4. Allocates contiguous memory for all blocks
5. Builds free list by linking all blocks together

**Memory Layout**: All blocks are stored contiguously with headers prepended:
```
[Header][User Data][Header][User Data]...
```

**Returns**: Pool pointer or NULL on failure

**Example**:
```c
// Create pool with 100 blocks of 256 bytes each
MemoryPool* pool = pool_create(256, 100);
```

#### pool_alloc
```c
void* pool_alloc(MemoryPool* pool);
```
*Source: src/core/memory.c:104-140*

Allocates a block from the pool.

**Algorithm**:
1. Checks if pool is exhausted (no free blocks)
2. Pops block from free list head
3. Marks block as allocated
4. Updates statistics (allocated_count, total_allocs, peak_usage)
5. In DEBUG mode, fills memory with debug pattern (0xCD)
6. Returns pointer to user data (after header)

**Time Complexity**: O(1)

**Returns**: Pointer to block or NULL if pool exhausted

#### pool_free
```c
void pool_free(MemoryPool* pool, void* ptr);
```
*Source: src/core/memory.c:142-192*

Frees a block back to the pool.

**Algorithm**:
1. Validates pointer is not NULL (NULL is allowed like standard free())
2. Gets block header by subtracting header size from pointer
3. In DEBUG mode: validates pointer is within pool bounds and properly aligned
4. Checks for double-free by inspecting is_allocated flag
5. Marks block as free
6. In DEBUG mode, fills memory with freed pattern (0xDD)
7. Pushes block to free list head
8. Updates statistics

**Time Complexity**: O(1)

**Safety**: Detects double-frees and out-of-bounds pointers in DEBUG mode

#### pool_reset
```c
void pool_reset(MemoryPool* pool);
```
*Source: src/core/memory.c:194-209*

Frees all allocated blocks at once. Much faster than individual frees.

**Algorithm**:
1. Rebuilds entire free list from scratch
2. Marks all blocks as free
3. Resets allocated_count to 0

**Time Complexity**: O(n) where n is block_count

**Use Case**: Per-frame temporary allocations

#### pool_destroy
```c
void pool_destroy(MemoryPool* pool);
```
*Source: src/core/memory.c:87-102*

Destroys pool and frees all memory.

**Behavior**:
- Logs warning if blocks are still allocated (memory leak)
- Calls pool_check_leaks() to report leak details
- Frees pool memory and structure

#### pool_check_leaks
```c
bool pool_check_leaks(const MemoryPool* pool);
```
*Source: src/core/memory.c:223-247*

Checks for memory leaks and logs details.

**Algorithm**:
1. Returns false if no leaks (allocated_count == 0)
2. Logs warning with leak count
3. In DEBUG mode: walks all blocks and reports addresses of leaked blocks

**Returns**: true if leaks detected

### Usage Patterns

**Basic Usage**:
```c
// Create pool
MemoryPool* entity_pool = pool_create(sizeof(Entity), 1000);

// Allocate objects
Entity* e1 = pool_alloc(entity_pool);
Entity* e2 = pool_alloc(entity_pool);

// Use objects...

// Free objects
pool_free(entity_pool, e1);
pool_free(entity_pool, e2);

// Cleanup
pool_destroy(entity_pool);
```

**Per-Frame Temporary Allocations**:
```c
// In game loop
while (running) {
    // Allocate temporary objects
    Particle* p = pool_alloc(temp_pool);
    // ... use particle ...

    // At end of frame, free all at once
    pool_reset(temp_pool);
}
```

**Statistics Monitoring**:
```c
MemoryStats stats;
pool_get_stats(pool, &stats);

if (stats.allocated_blocks > stats.block_count * 0.9) {
    LOG_WARN("Pool nearly exhausted: %.1f%% used",
             (float)stats.allocated_blocks / stats.block_count * 100);
}
```

### Performance Characteristics

- **Allocation**: O(1) - pop from free list
- **Deallocation**: O(1) - push to free list
- **Reset**: O(n) - rebuild free list
- **Memory Overhead**: sizeof(BlockHeader) per block (~16 bytes in DEBUG, ~12 bytes in release)
- **Fragmentation**: None (fixed-size blocks)

### Debug Features

When compiled with `-DDEBUG`:
- Memory is filled with patterns (0xCD for allocated, 0xDD for freed)
- Double-free detection
- Out-of-bounds pointer detection
- Allocation tracking with file/line information
- Leak reporting with block addresses

---

## Event Bus System

**Files**: `src/core/events.c` (336 lines), `src/core/events.h` (219 lines)

### Purpose

Provides a publish/subscribe event system for decoupled game architecture. Allows systems to communicate without direct dependencies. Supports both synchronous (immediate) and asynchronous (queued) event dispatch.

### Data Structures

#### EventType
```c
typedef enum {
    EVENT_NONE = 0,

    /* Game events */
    EVENT_GAME_START,
    EVENT_GAME_PAUSE,
    EVENT_GAME_RESUME,
    EVENT_GAME_QUIT,

    /* Player events */
    EVENT_PLAYER_MOVE,
    EVENT_PLAYER_ATTACK,
    EVENT_PLAYER_DAMAGE_TAKEN,
    EVENT_PLAYER_DIED,
    EVENT_PLAYER_LEVEL_UP,

    /* Combat events */
    EVENT_COMBAT_START,
    EVENT_COMBAT_END,
    EVENT_ENEMY_SPAWN,
    EVENT_ENEMY_DIED,

    /* UI events */
    EVENT_UI_OPEN_MENU,
    EVENT_UI_CLOSE_MENU,
    EVENT_UI_BUTTON_CLICKED,
    EVENT_UI_TEXT_INPUT,

    /* System events */
    EVENT_RESOURCE_LOADED,
    EVENT_SAVE_GAME,
    EVENT_LOAD_GAME,

    /* Custom event range */
    EVENT_CUSTOM_START = 1000,
    EVENT_CUSTOM_END = 9999,

    EVENT_COUNT = 10000
} EventType;
```
*Source: src/core/events.h:22-60*

#### Event
```c
struct Event {
    EventType type;
    void* data;         /* Event-specific data */
    size_t data_size;   /* Size of data (for copying) */
};
```
*Source: src/core/events.h:67-71*

#### EventCallback
```c
typedef void (*EventCallback)(const Event* event, void* userdata);
```
*Source: src/core/events.h:74*

Callback function signature for event subscribers.

#### EventBus (Internal)
```c
struct EventBus {
    Subscription* subscriptions[EVENT_COUNT];  /* Subscription lists per event type */
    QueuedEvent* event_queue;
    size_t queue_size;
    size_t queue_capacity;
    size_t next_subscription_id;
    size_t total_subscriptions;
};
```
*Source: src/core/events.c:26-33*

Uses separate chaining with linked lists for each event type. This allows O(1) subscription lookup by event type.

#### Subscription (Internal)
```c
typedef struct Subscription {
    size_t id;
    EventType type;
    EventCallback callback;
    void* userdata;
    bool active;
    struct Subscription* next;
} Subscription;
```
*Source: src/core/events.c:10-17*

### Core Functions

#### event_bus_create
```c
EventBus* event_bus_create(void);
```
*Source: src/core/events.c:71-93*

Creates an event bus.

**Initialization**:
- Allocates bus structure with calloc (all subscriptions NULL)
- Creates event queue with initial capacity of 128 events
- Can grow up to MAX_EVENT_QUEUE (1024) events

**Returns**: Event bus pointer or NULL on failure

#### event_bus_subscribe
```c
size_t event_bus_subscribe(EventBus* bus, EventType type,
                           EventCallback callback, void* userdata);
```
*Source: src/core/events.c:121-148*

Subscribes to an event type.

**Algorithm**:
1. Validates parameters (bus, type range, callback non-NULL)
2. Allocates subscription structure
3. Assigns unique subscription ID
4. Prepends to subscription list for that event type
5. Updates subscription count

**Time Complexity**: O(1)

**Returns**: Subscription ID (use for unsubscribe) or 0 on failure

**Example**:
```c
void on_player_damage(const Event* event, void* userdata) {
    int* damage = (int*)event->data;
    LOG_INFO("Player took %d damage", *damage);
}

size_t sub_id = event_bus_subscribe(bus, EVENT_PLAYER_DAMAGE_TAKEN,
                                     on_player_damage, NULL);
```

#### event_bus_publish
```c
bool event_bus_publish(EventBus* bus, EventType type, void* data);
```
*Source: src/core/events.c:191-213*

Publishes event immediately (synchronous).

**Algorithm**:
1. Validates parameters
2. Creates Event structure
3. Iterates through subscription list for this event type
4. Calls each active subscriber's callback immediately
5. Logs number of subscribers notified

**Time Complexity**: O(s) where s is number of subscribers for this event type

**Use Case**: Events that must be processed immediately (e.g., critical game state changes)

**Example**:
```c
int damage = 50;
event_bus_publish(bus, EVENT_PLAYER_DAMAGE_TAKEN, &damage);
```

#### event_bus_queue
```c
bool event_bus_queue(EventBus* bus, EventType type, const void* data, size_t data_size);
```
*Source: src/core/events.c:239-271*

Queues event for later dispatch (asynchronous).

**Algorithm**:
1. Validates parameters
2. Grows queue if needed (doubles capacity up to MAX_EVENT_QUEUE)
3. Copies event data into queue-owned memory
4. Increments queue size

**Time Complexity**: O(1) amortized (O(n) when growing queue)

**Use Case**: Events that can be processed at end of frame (e.g., particle effects, sound triggers)

**Example**:
```c
DamageData damage = {.amount = 50, .source = DAMAGE_FIRE};
event_bus_queue(bus, EVENT_PLAYER_DAMAGE_TAKEN, &damage, sizeof(damage));
```

#### event_bus_dispatch
```c
void event_bus_dispatch(EventBus* bus);
```
*Source: src/core/events.c:273-300*

Dispatches all queued events.

**Algorithm**:
1. Iterates through all queued events
2. For each event, calls all subscribers (like publish)
3. Frees queue-owned event data
4. Clears queue size to 0

**Time Complexity**: O(e * s) where e is queued events, s is average subscribers per event

**Use Case**: Called once per frame to process queued events

**Example**:
```c
// Game loop
while (running) {
    process_input();
    update(delta);
    event_bus_dispatch(bus);  // Dispatch queued events
    render();
}
```

#### event_bus_unsubscribe
```c
bool event_bus_unsubscribe(EventBus* bus, size_t subscription_id);
```
*Source: src/core/events.c:150-171*

Unsubscribes using subscription ID.

**Algorithm**:
1. Searches all event types for matching subscription ID
2. Removes from linked list when found
3. Frees subscription structure
4. Updates total subscription count

**Time Complexity**: O(EVENT_COUNT + s) worst case, where s is subscriptions in found list

#### Helper Macros

```c
// Publish event with data
EVENT_PUBLISH_DATA(bus, type, data_ptr)

// Queue event with data (copies data)
EVENT_QUEUE_DATA(bus, type, data_ptr)

// Publish event without data
EVENT_PUBLISH_SIMPLE(bus, type)

// Queue event without data
EVENT_QUEUE_SIMPLE(bus, type)
```
*Source: src/core/events.h:193-216*

### Usage Patterns

**Basic Pub/Sub**:
```c
// System A: Subscribe
void on_enemy_died(const Event* event, void* userdata) {
    GameStats* stats = (GameStats*)userdata;
    stats->enemies_killed++;
}

event_bus_subscribe(bus, EVENT_ENEMY_DIED, on_enemy_died, &game_stats);

// System B: Publish
void kill_enemy(Enemy* enemy) {
    // ... death logic ...
    EVENT_PUBLISH_SIMPLE(bus, EVENT_ENEMY_DIED);
}
```

**Event Data Passing**:
```c
// Define event data structure
typedef struct {
    Vector2 position;
    int damage;
    DamageType type;
} DamageEvent;

// Publisher
DamageEvent dmg = {pos, 50, DAMAGE_FIRE};
EVENT_QUEUE_DATA(bus, EVENT_PLAYER_DAMAGE_TAKEN, &dmg);

// Subscriber
void on_damage(const Event* event, void* userdata) {
    DamageEvent* dmg = (DamageEvent*)event->data;
    apply_damage(dmg->damage);
    spawn_effect(dmg->position, dmg->type);
}
```

**Event Lifetime Management**:
```c
// Initialize
EventBus* bus = event_bus_create();

// Subscribe to events
size_t sub1 = event_bus_subscribe(bus, EVENT_GAME_QUIT, on_quit, NULL);
size_t sub2 = event_bus_subscribe(bus, EVENT_PLAYER_DIED, on_death, NULL);

// Game loop
while (running) {
    // Queue events during frame
    event_bus_queue(bus, EVENT_ENEMY_SPAWN, &spawn_data, sizeof(spawn_data));

    // Dispatch at end of frame
    event_bus_dispatch(bus);
}

// Cleanup
event_bus_unsubscribe(bus, sub1);
event_bus_unsubscribe(bus, sub2);
event_bus_destroy(bus);
```

### Performance Characteristics

- **Subscribe**: O(1) - prepend to list
- **Unsubscribe**: O(EVENT_COUNT + s) - search all lists
- **Publish**: O(s) - iterate subscribers
- **Queue**: O(1) amortized - append to array
- **Dispatch**: O(e * s) - iterate events and subscribers
- **Memory**: ~80KB for subscription table + dynamic queue storage

### Design Decisions

1. **Separate Chaining**: Uses array of linked lists (one per event type) for O(1) subscription lookup
2. **Dynamic Queue**: Queue grows as needed up to MAX_EVENT_QUEUE (1024 events)
3. **Data Copying**: Queued events copy data to ensure lifetime safety
4. **Immediate Publish**: For critical events that need immediate processing
5. **Event Type Enum**: Fixed enum for type safety and fast lookup

---

## State Manager

**Files**: `src/core/state_manager.c` (273 lines), `src/core/state_manager.h` (167 lines)

### Purpose

Manages game states and transitions using a state machine with stack support. Enables pause/resume functionality and clean state transitions.

### Data Structures

#### GameState
```c
typedef enum {
    STATE_NONE = 0,
    STATE_INIT,
    STATE_MAIN_MENU,
    STATE_GAME_WORLD,
    STATE_DIALOGUE,
    STATE_INVENTORY,
    STATE_PAUSE,
    STATE_SHUTDOWN,
    STATE_COUNT
} GameState;
```
*Source: src/core/state_manager.h:23-33*

#### StateCallbacks
```c
typedef struct {
    void (*on_enter)(void* userdata);   /* Called when state becomes active */
    void (*on_exit)(void* userdata);    /* Called when state becomes inactive */
    void (*on_update)(double delta, void* userdata);  /* Called every frame when active */
    void (*on_render)(void* userdata);  /* Called for rendering when active */
    void (*on_pause)(void* userdata);   /* Called when state is pushed over */
    void (*on_resume)(void* userdata);  /* Called when state becomes top again */
} StateCallbacks;
```
*Source: src/core/state_manager.h:39-46*

All callbacks are optional (can be NULL).

#### StateManager (Internal)
```c
struct StateManager {
    StateInfo states[STATE_COUNT];  /* Registered state info */
    StackEntry stack[MAX_STATE_STACK];  /* State stack */
    size_t stack_depth;
};
```
*Source: src/core/state_manager.c:22-26*

Maximum stack depth is 16 (MAX_STATE_STACK).

### Core Functions

#### state_manager_register
```c
bool state_manager_register(StateManager* manager, GameState state,
                            const StateCallbacks* callbacks, void* userdata);
```
*Source: src/core/state_manager.c:70-93*

Registers a state with its callbacks.

**Behavior**:
- Copies callback structure (NULL callbacks are valid)
- Stores userdata pointer
- Warns if state already registered (overwrites)

**Example**:
```c
StateCallbacks menu_callbacks = {
    .on_enter = menu_enter,
    .on_update = menu_update,
    .on_render = menu_render,
    .on_exit = menu_exit
};
state_manager_register(sm, STATE_MAIN_MENU, &menu_callbacks, &menu_state);
```

#### state_manager_push
```c
bool state_manager_push(StateManager* manager, GameState state);
```
*Source: src/core/state_manager.c:95-133*

Pushes a new state onto the stack.

**Algorithm**:
1. Validates state is registered
2. Checks stack depth < MAX_STATE_STACK
3. Calls on_pause on current state (if any)
4. Pushes new state onto stack
5. Calls on_enter on new state
6. Increments stack depth

**Use Case**: Opening menus/dialogs over gameplay (pause game, show menu)

**Example**:
```c
// Player opens inventory
state_manager_push(sm, STATE_INVENTORY);
// Game state is paused, inventory is now active
```

#### state_manager_pop
```c
bool state_manager_pop(StateManager* manager);
```
*Source: src/core/state_manager.c:135-171*

Pops current state from stack.

**Algorithm**:
1. Checks stack is not empty
2. Calls on_exit on current state
3. Decrements stack depth
4. Calls on_resume on previous state (if any)

**Use Case**: Closing menus, returning to previous state

**Example**:
```c
// Player closes inventory
state_manager_pop(sm);
// Inventory exits, game state resumes
```

#### state_manager_change
```c
bool state_manager_change(StateManager* manager, GameState state);
```
*Source: src/core/state_manager.c:173-209*

Changes to new state (replaces current).

**Algorithm**:
1. Validates new state is registered
2. Calls on_exit on current state
3. Replaces top of stack with new state
4. Calls on_enter on new state

**Use Case**: Transitioning between major states (menu to game, game to game over)

**Example**:
```c
// Start game from menu
state_manager_change(sm, STATE_GAME_WORLD);
// Menu exits, game world enters
```

#### state_manager_update
```c
void state_manager_update(StateManager* manager, double delta_time);
```
*Source: src/core/state_manager.c:222-232*

Updates current state.

**Behavior**:
- Calls on_update callback of top state (if not NULL)
- Passes delta_time and userdata to callback

#### state_manager_render
```c
void state_manager_render(StateManager* manager);
```
*Source: src/core/state_manager.c:234-244*

Renders current state.

**Behavior**:
- Calls on_render callback of top state (if not NULL)

### Usage Patterns

**Complete State Setup**:
```c
StateManager* sm = state_manager_create();

// Register states
StateCallbacks menu_cb = {/* callbacks */};
StateCallbacks game_cb = {/* callbacks */};

state_manager_register(sm, STATE_MAIN_MENU, &menu_cb, &menu_data);
state_manager_register(sm, STATE_GAME_WORLD, &game_cb, &game_data);

// Start with menu
state_manager_push(sm, STATE_MAIN_MENU);

// Game loop
while (running) {
    state_manager_update(sm, delta_time);
    state_manager_render(sm);
}

state_manager_destroy(sm);
```

**Pause Menu Pattern**:
```c
// Game is running
// Player presses ESC -> show pause menu
state_manager_push(sm, STATE_PAUSE);
// on_pause called on GAME_WORLD, on_enter called on PAUSE

// Player resumes
state_manager_pop(sm);
// on_exit called on PAUSE, on_resume called on GAME_WORLD
```

**State Transition**:
```c
// Menu -> Game
state_manager_change(sm, STATE_GAME_WORLD);
// on_exit(MENU), on_enter(GAME_WORLD)

// Game -> Game Over
state_manager_change(sm, STATE_GAME_OVER);
// on_exit(GAME_WORLD), on_enter(GAME_OVER)
```

### Design Decisions

1. **Stack-Based**: Allows pause/resume without losing state
2. **Callback-Driven**: States define their own behavior through callbacks
3. **Userdata**: Each state can have associated data
4. **Max Depth 16**: Prevents stack overflow from recursive pushes
5. **Optional Callbacks**: States can omit callbacks they don't need

---

## Timing System

**Files**: `src/core/timing.c` (119 lines), `src/core/timing.h` (68 lines)

### Purpose

Provides cross-platform timing for game loop, FPS limiting, and delta time calculation.

### Data Structures

```c
static struct {
    double start_time;
    double frame_start;
    double last_frame;
    double delta_time;
    double fps;
    int frame_count;
    double fps_timer;
} g_timing = {0};
```
*Source: src/core/timing.c:14-22*

Global timing state (internal).

### Core Functions

#### timing_init
```c
void timing_init(void);
```
*Source: src/core/timing.c:47-57*

Initializes timing system. Must be called before other timing functions.

**Behavior**:
- Records start time
- Resets all timing state to zero

#### timing_frame_start
```c
void timing_frame_start(void);
```
*Source: src/core/timing.c:59-67*

Marks start of frame. Call at beginning of game loop.

**Algorithm**:
1. Records frame start time
2. Calculates delta_time = frame_start - last_frame
3. Clamps delta to 0.1 seconds max (prevents "spiral of death")

**Delta Clamping**: If frame takes too long (>100ms), delta is clamped to prevent physics simulation from exploding.

#### timing_frame_end
```c
void timing_frame_end(int target_fps);
```
*Source: src/core/timing.c:69-93*

Marks end of frame and limits FPS.

**Algorithm**:
1. Calculates frame time = end - start
2. If target_fps > 0:
   - Calculates target frame time = 1 / target_fps
   - If frame time < target, sleeps for difference
3. Records last_frame time
4. Updates FPS counter (averaged over 1 second)

**FPS Calculation**: Counts frames over 1 second, then calculates FPS = frame_count / elapsed_time

**Example**:
```c
timing_init();

while (running) {
    timing_frame_start();

    update(timing_get_delta());
    render();

    timing_frame_end(60);  // 60 FPS target
}
```

#### timing_get_delta
```c
double timing_get_delta(void);
```
*Source: src/core/timing.c:95-97*

Returns delta time in seconds since last frame. Use for frame-rate independent movement.

#### timing_get_fps
```c
double timing_get_fps(void);
```
*Source: src/core/timing.c:99-101*

Returns current FPS (averaged over 1 second).

#### timing_get_time
```c
double timing_get_time(void);
```
*Source: src/core/timing.c:103-105*

Returns time in seconds since timing_init().

#### timing_sleep_ms
```c
void timing_sleep_ms(int ms);
```
*Source: src/core/timing.c:107-118*

Sleeps for specified milliseconds. Cross-platform (uses Sleep on Windows, nanosleep on POSIX).

### Platform-Specific Implementation

**Windows**:
```c
static double get_time_seconds(void) {
    static LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
}
```
*Source: src/core/timing.c:26-38*

Uses QueryPerformanceCounter for high-resolution timing.

**POSIX**:
```c
static double get_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
```
*Source: src/core/timing.c:40-44*

Uses clock_gettime(CLOCK_MONOTONIC) for monotonic time.

### Usage Patterns

**Frame-Rate Independent Movement**:
```c
// Update player position
player.x += player.velocity_x * timing_get_delta();
player.y += player.velocity_y * timing_get_delta();

// If velocity is 100 units/sec and delta is 0.016 (60fps)
// Movement per frame = 100 * 0.016 = 1.6 units
```

**FPS Display**:
```c
double fps = timing_get_fps();
printf("FPS: %.1f\n", fps);
```

**Game Time**:
```c
double elapsed = timing_get_time();
if (elapsed > 300.0) {  // 5 minutes
    trigger_time_event();
}
```

---

## Game Loop

**Files**: `src/core/game_loop.c` (214 lines), `src/core/game_loop.h` (186 lines)

### Purpose

Main game loop abstraction with callback-based lifecycle management. Integrates with timing system for fixed frame rate.

### Data Structures

#### GameLoopCallbacks
```c
typedef struct {
    bool (*on_init)(void* userdata);     /* Called once at init */
    void (*on_update)(double delta_time, void* userdata);  /* Called every frame */
    void (*on_render)(void* userdata);   /* Called every frame */
    void (*on_cleanup)(void* userdata);  /* Called once at cleanup */
    void (*on_pause)(void* userdata);    /* Called when paused */
    void (*on_resume)(void* userdata);   /* Called when resumed */
} GameLoopCallbacks;
```
*Source: src/core/game_loop.h:28-61*

#### GameLoopState
```c
typedef enum {
    LOOP_STATE_STOPPED = 0,
    LOOP_STATE_RUNNING,
    LOOP_STATE_PAUSED
} GameLoopState;
```
*Source: src/core/game_loop.h:64-68*

#### GameLoop (Internal)
```c
struct GameLoop {
    GameLoopCallbacks callbacks;
    void* userdata;
    GameLoopState state;
    unsigned long long frame_count;
    double elapsed_time;
    double current_fps;
    double max_delta_time;
};
```
*Source: src/core/game_loop.c:8-16*

### Core Functions

#### game_loop_create
```c
GameLoop* game_loop_create(const GameLoopCallbacks* callbacks, void* userdata);
```
*Source: src/core/game_loop.c:18-40*

Creates game loop with callbacks and userdata.

**Default Values**:
- state: LOOP_STATE_STOPPED
- frame_count: 0
- max_delta_time: 0.1 seconds

#### game_loop_run
```c
bool game_loop_run(GameLoop* loop, unsigned int target_fps);
```
*Source: src/core/game_loop.c:54-131*

Runs the game loop (blocks until stopped).

**Algorithm**:
1. Calls on_init (aborts if returns false)
2. Sets state to RUNNING
3. Main loop:
   - Calls timing_frame_start()
   - Gets delta_time
   - Clamps delta to max_delta_time
   - If RUNNING: calls on_update(delta_time)
   - Always calls on_render() (even when paused)
   - Increments frame_count
   - Updates FPS counter every second
   - Calls timing_frame_end(target_fps)
4. Calls on_cleanup
5. Returns true

**Loop Exits When**: state becomes LOOP_STATE_STOPPED (via game_loop_stop)

**Paused Behavior**: When paused, on_update is not called but on_render continues

**Example**:
```c
bool init(void* data) {
    // Initialize game
    return true;
}

void update(double dt, void* data) {
    // Update game logic
}

void render(void* data) {
    // Render game
}

void cleanup(void* data) {
    // Cleanup resources
}

GameLoopCallbacks callbacks = {
    .on_init = init,
    .on_update = update,
    .on_render = render,
    .on_cleanup = cleanup
};

GameLoop* loop = game_loop_create(&callbacks, &game_state);
game_loop_run(loop, 60);  // Blocks until stopped
game_loop_destroy(loop);
```

#### game_loop_stop
```c
void game_loop_stop(GameLoop* loop);
```
*Source: src/core/game_loop.c:133-143*

Stops the game loop. Loop exits after current frame completes.

#### game_loop_pause / game_loop_resume
```c
void game_loop_pause(GameLoop* loop);
void game_loop_resume(GameLoop* loop);
```
*Source: src/core/game_loop.c:145-177*

Pauses/resumes the loop. Calls on_pause/on_resume callbacks.

**Paused Behavior**:
- on_update is NOT called
- on_render IS called (so UI can still be rendered)
- Delta time is NOT accumulated in elapsed_time

### Usage Patterns

**Complete Game Setup**:
```c
typedef struct {
    Player player;
    World world;
    // ... game state ...
} GameState;

bool game_init(void* userdata) {
    GameState* state = (GameState*)userdata;
    // Load resources, initialize game
    return true;
}

void game_update(double delta, void* userdata) {
    GameState* state = (GameState*)userdata;
    update_player(&state->player, delta);
    update_world(&state->world, delta);
}

void game_render(void* userdata) {
    GameState* state = (GameState*)userdata;
    render_world(&state->world);
    render_player(&state->player);
}

void game_cleanup(void* userdata) {
    GameState* state = (GameState*)userdata;
    // Free resources
}

int main() {
    GameState state = {0};

    GameLoopCallbacks callbacks = {
        .on_init = game_init,
        .on_update = game_update,
        .on_render = game_render,
        .on_cleanup = game_cleanup
    };

    GameLoop* loop = game_loop_create(&callbacks, &state);
    game_loop_run(loop, 60);  // Run at 60 FPS
    game_loop_destroy(loop);

    return 0;
}
```

**Pause on Focus Loss**:
```c
void on_window_focus_lost() {
    game_loop_pause(loop);
}

void on_window_focus_gained() {
    game_loop_resume(loop);
}
```

### Performance Characteristics

- **Frame Timing**: Uses timing system for accurate frame rate control
- **Delta Clamping**: Prevents "spiral of death" from long frames
- **FPS Counter**: Updated every second with accurate average
- **Pause**: No CPU usage during paused update phase

---

## Integration Patterns

### Complete System Integration

Example of integrating all core systems in a game:

```c
typedef struct {
    EventBus* event_bus;
    StateManager* state_manager;
    MemoryPool* entity_pool;
    MemoryPool* particle_pool;
    // ... other systems ...
} GameContext;

bool game_init(void* userdata) {
    GameContext* ctx = (GameContext*)userdata;

    // Initialize timing
    timing_init();

    // Create event bus
    ctx->event_bus = event_bus_create();

    // Create state manager
    ctx->state_manager = state_manager_create();

    // Register states
    StateCallbacks menu_cb = {/* ... */};
    StateCallbacks game_cb = {/* ... */};
    state_manager_register(ctx->state_manager, STATE_MAIN_MENU, &menu_cb, ctx);
    state_manager_register(ctx->state_manager, STATE_GAME_WORLD, &game_cb, ctx);

    // Create memory pools
    ctx->entity_pool = pool_create(sizeof(Entity), 1000);
    ctx->particle_pool = pool_create(sizeof(Particle), 5000);

    // Subscribe to events
    event_bus_subscribe(ctx->event_bus, EVENT_ENEMY_DIED, on_enemy_died, ctx);
    event_bus_subscribe(ctx->event_bus, EVENT_PLAYER_DAMAGE_TAKEN, on_damage, ctx);

    // Push initial state
    state_manager_push(ctx->state_manager, STATE_MAIN_MENU);

    return true;
}

void game_update(double delta, void* userdata) {
    GameContext* ctx = (GameContext*)userdata;

    // Update current state
    state_manager_update(ctx->state_manager, delta);

    // Dispatch queued events
    event_bus_dispatch(ctx->event_bus);

    // Reset per-frame pools
    pool_reset(ctx->particle_pool);
}

void game_render(void* userdata) {
    GameContext* ctx = (GameContext*)userdata;

    // Render current state
    state_manager_render(ctx->state_manager);
}

void game_cleanup(void* userdata) {
    GameContext* ctx = (GameContext*)userdata;

    // Cleanup state manager
    state_manager_destroy(ctx->state_manager);

    // Cleanup event bus
    event_bus_destroy(ctx->event_bus);

    // Cleanup memory pools
    pool_check_leaks(ctx->entity_pool);
    pool_destroy(ctx->entity_pool);
    pool_destroy(ctx->particle_pool);
}

int main() {
    GameContext ctx = {0};

    GameLoopCallbacks callbacks = {
        .on_init = game_init,
        .on_update = game_update,
        .on_render = game_render,
        .on_cleanup = game_cleanup
    };

    GameLoop* loop = game_loop_create(&callbacks, &ctx);
    game_loop_run(loop, 60);
    game_loop_destroy(loop);

    return 0;
}
```

### Event-Driven State Transitions

```c
void on_player_died(const Event* event, void* userdata) {
    StateManager* sm = (StateManager*)userdata;
    state_manager_change(sm, STATE_GAME_OVER);
}

void on_game_quit(const Event* event, void* userdata) {
    GameLoop* loop = (GameLoop*)userdata;
    game_loop_stop(loop);
}

// Subscribe
event_bus_subscribe(bus, EVENT_PLAYER_DIED, on_player_died, state_manager);
event_bus_subscribe(bus, EVENT_GAME_QUIT, on_game_quit, game_loop);
```

### Memory Pool Per System

```c
typedef struct {
    MemoryPool* pool;
    // ... system data ...
} ParticleSystem;

void particle_system_init(ParticleSystem* sys) {
    sys->pool = pool_create(sizeof(Particle), 5000);
}

Particle* particle_system_spawn(ParticleSystem* sys, Vector2 pos) {
    Particle* p = pool_alloc(sys->pool);
    if (!p) {
        LOG_WARN("Particle pool exhausted");
        return NULL;
    }

    p->position = pos;
    p->lifetime = 1.0f;
    return p;
}

void particle_system_update(ParticleSystem* sys, double delta) {
    // Update particles...

    // Particles can be freed individually or
    // all at once with pool_reset() if they're per-frame
}
```

---

## Summary

The core systems provide a solid foundation for Necromancer's Shell:

- **Memory Management**: Fast pool allocator with leak detection
- **Event System**: Decoupled pub/sub architecture
- **State Management**: Stack-based state machine with callbacks
- **Timing**: Cross-platform timing and FPS control
- **Game Loop**: Clean lifecycle management

All systems are designed to work together and integrate cleanly into the game architecture.

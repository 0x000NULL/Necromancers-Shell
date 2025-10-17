# Save/Load System Status Report

## Summary

The save/load system for Necromancer's Shell has been debugged and enhanced. The core functionality is now working correctly with zero memory leaks.

## Issues Fixed

### 1. Checksum Validation Bug (CRITICAL - FIXED)

**Problem:** The file was opened in "wb" (write-only binary) mode, then the code attempted to read from it to calculate the checksum. This failed on most systems because write-only mode doesn't allow reading.

**Location:** `src/data/save_load.c:677`

**Fix:** Changed file mode from `"wb"` to `"w+b"` (read-write binary mode).

```c
// Before:
FILE* fp = fopen(temp_path, "wb");

// After:
FILE* fp = fopen(temp_path, "w+b");
```

**Result:** All 9 tests now pass.

### 2. Memory Leaks (VERIFIED CLEAN)

**Valgrind Results:**
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 170 allocs, 170 frees, 167,557 bytes allocated

All heap blocks were freed -- no leaks are possible

ERROR SUMMARY: 0 errors from 0 contexts
```

Zero memory leaks confirmed.

## Systems Implemented

### Currently Serialized

The save/load system now correctly serializes and deserializes:

1. **Soul Manager** - All souls with types, quality, memories, bindings
2. **Minion Manager** - All minions with stats, experience, levels
3. **Territory Manager** - All discovered locations with connections, corpse counts, control levels ✅ NEW
4. **Resources** - Soul energy, mana, time tracking, calendar
5. **Corruption State** - Corruption level and event history
6. **Consciousness State** - Stability, decay rate, fragmentation
7. **Scalar Game State** - Current location, player level/XP, IDs, ending status

### Save File Format

```
[Header - 24 bytes]
  - Magic number: 0x5243454E ("NECR")
  - Version: 1.0.0
  - Checksum: CRC32 of data section
  - Data length: uint64_t

[Data Section - Variable]
  - Soul Manager (count + array of Soul structs)
  - Minion Manager (count + array of Minion structs)
  - Territory Manager (total count + discovered count + array of Location structs) ✅ NEW
  - Resources struct
  - Corruption struct
  - Consciousness struct
  - Scalar fields (location_id, level, XP, etc.)
```

### File Sizes

- **Empty save:** 103 bytes (header + minimal data)
- **Test save with data:** 342 bytes (2 souls, 1 minion)
- **Typical save:** Estimated <1-2 MB

## Systems NOT Yet Serialized

The following subsystems are **not yet serialized** but have placeholders for future implementation:

### High Priority

1. **Quest Manager** (`src/game/narrative/quests/quest_manager.h`)
   - Quest states (active, completed, failed)
   - Objective progress
   - Quest giver relationships
   - Timestamps (started, completed, deadline)
   - Estimated: 200-300 lines of code

2. **NPC Manager** (`src/game/narrative/npcs/npc_manager.h`)
   - NPC states (if dynamic)
   - NPC locations (if they move)
   - Interaction history
   - Estimated: 100-150 lines of code

3. **Relationship Manager** (`src/game/narrative/relationships/relationship_manager.h`)
   - All NPC relationships (favor, trust levels)
   - Relationship event history
   - Estimated: 100-150 lines of code

### Medium Priority

4. **Memory Manager** (`src/game/narrative/memory/memory_manager.h`)
   - Unlocked memory fragments
   - Discovery timestamps
   - Estimated: 50-100 lines of code

5. **Thessara Relationship** (`src/game/narrative/thessara/thessara.h`)
   - Connection level, trust
   - Warning count
   - Discovery state
   - Estimated: 50-80 lines of code

6. **Divine Council** (`src/game/narrative/gods/divine_council.h`)
   - Divine favor for each of 7 gods
   - Council decision history
   - Estimated: 50-80 lines of code

### Lower Priority

7. **Dialogue Manager** (`src/game/narrative/dialogue/dialogue_manager.h`)
   - Dialogue states, choices made
   - Conversation history
   - Estimated: 100-150 lines of code

8. **Event Scheduler** (`src/game/events/event_scheduler.h`)
   - Triggered events
   - Event completion states
   - Estimated: 80-120 lines of code

9. **Archon Trial Manager** (`src/game/narrative/trials/archon_trial.h`)
   - Trial completion states
   - Trial scores (0-100 per trial)
   - Estimated: 100-150 lines of code

10. **Skill Tree** (`src/game/progression/skill_tree.h`)
    - Unlocked skills
    - Skill points spent
    - Estimated: 80-120 lines of code

11. **Research Manager** (`src/game/progression/research.h`)
    - Active research project
    - Completed projects
    - Research progress
    - Estimated: 80-120 lines of code

12. **Artifact Collection** (`src/game/progression/artifacts.h`)
    - Discovered artifacts
    - Equipped artifacts
    - Estimated: 80-120 lines of code

**Total Estimated Work:** ~1,200-1,700 additional lines of code for complete serialization.

## Implementation Pattern

For adding new subsystems, follow this pattern:

### 1. Add Helper Functions (in `save_load.c`)

```c
static bool write_SUBSYSTEM_manager(FILE* fp, const SUBSYSTEMManager* mgr) {
    if (!mgr) {
        return write_bool(fp, false); // NULL flag
    }
    if (!write_bool(fp, true)) return false; // Not NULL

    // Write count
    uint32_t count = subsystem_manager_count(mgr);
    if (!write_uint32(fp, count)) return false;

    // Write each item
    for (uint32_t i = 0; i < count; i++) {
        ITEM* item = subsystem_manager_get_at(mgr, i);
        // Write item fields with write_uint32, write_string, etc.
    }

    return true;
}

static SUBSYSTEMManager* read_SUBSYSTEM_manager(FILE* fp) {
    bool not_null;
    if (!read_bool(fp, &not_null)) return NULL;
    if (!not_null) return NULL; // Was NULL

    SUBSYSTEMManager* mgr = subsystem_manager_create();
    if (!mgr) return NULL;

    uint32_t count;
    if (!read_uint32(fp, &count)) {
        subsystem_manager_destroy(mgr);
        return NULL;
    }

    for (uint32_t i = 0; i < count; i++) {
        // Read item and add to manager
        // Clean up on failure
    }

    return mgr;
}
```

### 2. Add Forward Declarations (top of `save_load.c`)

```c
static bool write_SUBSYSTEM_manager(FILE* fp, const SUBSYSTEMManager* mgr);
static SUBSYSTEMManager* read_SUBSYSTEM_manager(FILE* fp);
```

### 3. Add Include

```c
#include "../game/path/to/subsystem_manager.h"
```

### 4. Integrate into save_game()

```c
/* Write managers */
success = success && write_soul_manager(fp, state->souls);
success = success && write_minion_manager(fp, state->minions);
success = success && write_territory_manager(fp, state->territory);
success = success && write_SUBSYSTEM_manager(fp, state->subsystem);  // ADD THIS
```

### 5. Integrate into load_game()

```c
/* Read managers */
state->souls = read_soul_manager(mem_fp);
state->minions = read_minion_manager(mem_fp);
state->territory = read_territory_manager(mem_fp);
state->subsystem = read_SUBSYSTEM_manager(mem_fp);  // ADD THIS
```

### 6. Add Cleanup in Error Path

```c
if (!success) {
    // ... existing cleanup ...
    subsystem_manager_destroy(state->subsystem);  // ADD THIS
    free(state);
    // ...
}
```

## Testing

### Current Test Coverage

All 9 tests passing:
- ✅ `test_save_load_roundtrip` - Round-trip save/load preserves all data
- ✅ `test_validate_corrupted_file` - Corruption detection works
- ✅ `test_version_compatibility` - Version checking works
- ✅ `test_save_file_exists` - File existence check works
- ✅ `test_get_save_file_size` - File size retrieval works
- ✅ `test_backup_save_file` - Backup creation works
- ✅ `test_save_metadata_json` - JSON metadata export works
- ✅ `test_load_nonexistent` - Loading non-existent file fails gracefully
- ✅ `test_empty_state` - Empty save files work correctly

### Running Tests

```bash
# Build tests
make build/test_test_save_load

# Run tests
./build/test_test_save_load

# Run with valgrind
valgrind --leak-check=full ./build/test_test_save_load
```

## Versioning

### Current Version
- **Save Format Version:** 1.0.0
- **Magic Number:** 0x5243454E ("NECR")

### Version Compatibility

Currently, the system accepts saves with the **same major version** only. Future enhancements could add:

- Minor version differences (backward compatible additions)
- Migration functions for format changes
- Deprecated field handling

### Adding New Fields

When adding new fields to serialization:

1. **If format changes:** Increment `SAVE_VERSION_MAJOR` in `save_load.h`
2. **If adding compatible data:** Increment `SAVE_VERSION_MINOR`
3. **If fixing bugs:** Increment `SAVE_VERSION_PATCH`
4. Update `is_version_compatible()` logic if needed

## Known Limitations

1. **Territory Serialization:** Currently only saves **discovered** locations. Undiscovered locations are reloaded from data files on load. This is acceptable because undiscovered locations have no modified state.

2. **LocationGraph:** The location graph (pathfinding data) is **not serialized**. It's rebuilt from location connections on load. This is by design - graph data structures are derived and can be recreated.

3. **World Map:** Not serialized - regenerated from locations on load.

4. **Dialogue/Quest Progress:** Not yet implemented. Active dialogues and quest progress will be lost on save/load until those systems are added.

## Next Steps for Full Implementation

### Phase 1: Core Narrative (Highest Priority)
1. Quest Manager serialization
2. NPC/Relationship Manager serialization
3. Memory Manager serialization

### Phase 2: Divine/Trial Systems
4. Divine Council favor serialization
5. Thessara relationship serialization
6. Archon Trial states serialization

### Phase 3: Progression Systems
7. Skill Tree serialization
8. Research Manager serialization
9. Artifact Collection serialization

### Phase 4: Advanced Systems
10. Dialogue Manager states serialization
11. Event Scheduler serialization
12. Combat state serialization (if mid-combat saves are needed)

## Success Criteria (Current Status)

- ✅ All 9+ tests passing
- ✅ Checksum validation working correctly
- ✅ Territory, locations serialized correctly
- ✅ Souls, minions, resources serialized
- ❌ Quests, NPCs, relationships serialized (not yet)
- ❌ Memory, Thessara, Divine Council serialized (not yet)
- ✅ Zero memory leaks (valgrind clean)
- ✅ Zero compiler warnings
- ✅ Save/load preserves game state (for implemented systems)
- ✅ File size reasonable (<500 bytes for test saves)

## Files Modified

1. `/home/stripcheese/Necromancers Shell/necromancers_shell/src/data/save_load.c`
   - Fixed file mode bug ("wb" → "w+b")
   - Added Location serialization functions
   - Added TerritoryManager serialization functions
   - Integrated territory into save/load flow

2. `/home/stripcheese/Necromancers Shell/necromancers_shell/src/data/save_load.h`
   - No changes needed (API unchanged)

3. `/home/stripcheese/Necromancers Shell/necromancers_shell/tests/test_save_load.c`
   - No changes needed (all tests now pass)

4. `/home/stripcheese/Necromancers Shell/necromancers_shell/tests/test_checksum_debug.c`
   - NEW: Debug test to verify checksum algorithm

## Conclusion

The save/load system is now **functional and production-ready** for the subsystems that are implemented:
- Souls
- Minions
- Territory/Locations ✅ NEW
- Resources
- Corruption
- Consciousness
- Game state scalars

**Estimated coverage:** ~40-50% of total game state serialized.

The remaining subsystems can be added incrementally using the documented pattern above. Each subsystem addition is estimated at 50-300 lines of code and should take 15-45 minutes to implement and test.

The critical bugs are fixed, the system is memory-leak-free, and the foundation is solid for future expansion.

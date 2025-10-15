# Phase 4: Combat System - Weeks 17-18 Implementation Plan

## Test Results Summary (Weeks 15-16)

**All combat systems validated:**
- test_combatant: 11/11 tests PASSED
- test_combat: 12/12 tests PASSED
- test_enemy: 11/11 tests PASSED
- test_encounter: 11/11 tests PASSED

**Total: 45/45 tests passing (100% success rate)**

---

## Week 17: Combat Commands & Mechanics (Days 113-119)

### Overview
Implement player-facing combat commands and core damage calculation systems. Enable players to actually participate in combat through typed commands.

### Day 113-114: Damage Calculation System

**File: `src/game/combat/damage.h`** (~120 lines)

```c
/**
 * @file damage.h
 * @brief Damage calculation and application system
 */

#ifndef DAMAGE_H
#define DAMAGE_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct Combatant Combatant;
typedef struct CombatState CombatState;

/**
 * @brief Damage type enumeration
 */
typedef enum {
    DAMAGE_TYPE_PHYSICAL,     /* Normal attack damage */
    DAMAGE_TYPE_NECROTIC,     /* Death magic damage */
    DAMAGE_TYPE_HOLY,         /* Anti-undead damage */
    DAMAGE_TYPE_PURE          /* Ignores defense */
} DamageType;

/**
 * @brief Attack result information
 */
typedef struct {
    uint32_t damage_dealt;        /* Final damage applied */
    uint32_t base_damage;         /* Pre-mitigation damage */
    uint32_t mitigated_damage;    /* Damage blocked by defense */
    bool is_critical;             /* Critical hit? */
    float critical_multiplier;    /* 1.5x for crits */
    bool was_blocked;             /* Defense blocked hit? */
    DamageType damage_type;       /* Type of damage */
} AttackResult;

/**
 * @brief Calculate damage from attacker to defender
 *
 * Formula:
 * - Base Damage = attacker->attack
 * - Critical Check: 10% chance for 1.5x damage
 * - Defense Mitigation: damage -= (defender->defense / 2)
 * - Defending Bonus: +50% defense if defender is defending
 * - Minimum Damage: Always at least 1 damage
 *
 * @param attacker Attacking combatant
 * @param defender Defending combatant
 * @param damage_type Type of damage to deal
 * @return Attack result with all damage information
 */
AttackResult damage_calculate_attack(
    const Combatant* attacker,
    const Combatant* defender,
    DamageType damage_type
);

/**
 * @brief Apply damage to a combatant and log result
 *
 * Updates combatant health, checks for death, and logs message to combat log.
 *
 * @param combat Combat state (for logging)
 * @param target Target combatant
 * @param result Attack result to apply
 * @return true if target is still alive, false if killed
 */
bool damage_apply_attack(
    CombatState* combat,
    Combatant* target,
    const AttackResult* result
);

/**
 * @brief Calculate healing amount
 *
 * Formula:
 * - Base Heal = healer->attack / 2
 * - Minimum: 10 HP
 * - Maximum: target->health_max - target->health
 *
 * @param healer Healing combatant
 * @param target Target to heal
 * @return Amount of healing to apply
 */
uint32_t damage_calculate_heal(
    const Combatant* healer,
    const Combatant* target
);

/**
 * @brief Apply healing and log result
 *
 * @param combat Combat state (for logging)
 * @param target Target to heal
 * @param amount Healing amount
 */
void damage_apply_heal(
    CombatState* combat,
    Combatant* target,
    uint32_t amount
);

/**
 * @brief Get critical hit chance (default 10%)
 *
 * @return Critical hit chance (0.0 to 1.0)
 */
float damage_get_crit_chance(void);

/**
 * @brief Get critical hit multiplier (default 1.5x)
 *
 * @return Critical damage multiplier
 */
float damage_get_crit_multiplier(void);

/**
 * @brief Roll for critical hit
 *
 * @return true if critical hit, false otherwise
 */
bool damage_roll_critical(void);

/**
 * @brief Get damage type name
 *
 * @param type Damage type
 * @return Human-readable damage type name
 */
const char* damage_type_name(DamageType type);

#endif /* DAMAGE_H */
```

**File: `src/game/combat/damage.c`** (~250 lines)

Implementation details:
- Use existing `rand()` for critical rolls
- Log all attacks/heals to combat log with ANSI colors
- Include detailed damage breakdown in log messages
- Example: "Zombie attacks Guard for 23 damage (30 base - 7 mitigated) - CRITICAL HIT!"

**Integration:**
- Include in `combat.h`
- Use in AI functions (replace manual damage calculation)
- Use in player attack command

---

### Day 115-116: Combat Commands

**File: `src/commands/commands/cmd_attack.c`** (~280 lines)

```c
/**
 * @brief Execute attack command
 *
 * Usage: attack <target_id>
 *
 * Requirements:
 * - Must be in combat (game_state->combat != NULL)
 * - Must be player's turn (combat->phase == COMBAT_PHASE_PLAYER_TURN)
 * - Must not have acted this turn
 * - Target must be valid enemy combatant
 * - Target must be alive
 *
 * Example: attack E1
 */
CommandResult cmd_attack(ParsedCommand* cmd);
```

**Implementation:**
1. Validate game state and combat phase
2. Get active combatant (must be player-controlled)
3. Parse target ID (format: E1, E2, etc.)
4. Find target in enemy forces
5. Use `damage_calculate_attack()` with DAMAGE_TYPE_PHYSICAL
6. Apply damage with `damage_apply_attack()`
7. Mark combatant as acted
8. Advance turn if all player forces acted
9. Return success with attack result

**File: `src/commands/commands/cmd_defend.c`** (~180 lines)

```c
/**
 * @brief Execute defend command
 *
 * Usage: defend
 *
 * Sets defending stance for current turn (+50% defense).
 * Defending resets at start of next turn.
 */
CommandResult cmd_defend(ParsedCommand* cmd);
```

**File: `src/commands/commands/cmd_flee.c`** (~220 lines)

```c
/**
 * @brief Execute flee command
 *
 * Usage: flee
 *
 * Attempt to escape combat:
 * - Base flee chance: 50%
 * - +10% per dead enemy
 * - -20% if any player force HP < 30%
 * - Success: End combat, return to world state
 * - Failure: Lose turn, enemies get free attack
 */
CommandResult cmd_flee(ParsedCommand* cmd);
```

**File: `src/commands/commands/cmd_combat_status.c`** (~200 lines)

```c
/**
 * @brief Show combat status
 *
 * Usage: status (during combat)
 *
 * Display:
 * - Turn number
 * - Combat phase
 * - Player forces (HP, status)
 * - Enemy forces (HP, status)
 * - Turn order
 * - Active combatant
 */
CommandResult cmd_combat_status(ParsedCommand* cmd);
```

**Registration:**
Update `src/commands/commands/builtin.c`:
- Register `attack`, `defend`, `flee` as combat-only commands
- Add availability check: `combat_is_active()`
- Hide from help when not in combat

---

### Day 117: Spell Command (Basic)

**File: `src/commands/commands/cmd_cast_combat.c`** (~300 lines)

```c
/**
 * @brief Cast spell in combat
 *
 * Usage: cast <spell_name> <target_id>
 *
 * Available combat spells (Phase 4):
 * - drain: Deal 15 necrotic damage, heal self for 50% of damage
 * - bolt: Deal 20 pure damage (ignores defense)
 * - weaken: Reduce target defense by 20% for 2 turns
 *
 * Requirements:
 * - Must have sufficient mana (from game_state->resources)
 * - Must be player's turn
 * - Target must be valid
 *
 * Mana costs:
 * - drain: 15 mana
 * - bolt: 20 mana
 * - weaken: 10 mana
 */
CommandResult cmd_cast_combat(ParsedCommand* cmd);
```

**Implementation:**
1. Parse spell name and target
2. Validate mana cost against `game_state->resources->mana`
3. Apply spell effect using `damage_calculate_attack()` with appropriate type
4. Deduct mana from resources
5. Log spell cast to combat log
6. Mark as acted

**Note:** This is a simplified spell system for Phase 4. Full spell system will be Phase 5.

---

### Day 118-119: Victory/Defeat Resolution

**File: `src/game/combat/combat_rewards.h`** (~80 lines)

```c
/**
 * @file combat_rewards.h
 * @brief Victory reward calculation and distribution
 */

typedef struct {
    uint32_t experience_total;       /* Total XP from enemies */
    uint32_t soul_energy_gained;     /* Soul energy from kills */
    uint8_t corruption_change;       /* +corruption if innocent enemies */
    Soul** souls_harvested;          /* New souls from corpses */
    uint8_t soul_count;             /* Number of souls */
} CombatRewards;

/**
 * @brief Calculate rewards from victory
 *
 * @param combat Combat state
 * @return Allocated rewards structure (caller must free)
 */
CombatRewards* combat_calculate_rewards(const CombatState* combat);

/**
 * @brief Apply rewards to game state
 *
 * @param rewards Rewards to apply
 * @param game_state Game state to update
 */
void combat_apply_rewards(const CombatRewards* rewards, GameState* game_state);

/**
 * @brief Free rewards structure
 */
void combat_rewards_destroy(CombatRewards* rewards);
```

**File: `src/game/combat/combat_rewards.c`** (~200 lines)

**Integration with combat.c:**
Update `combat_end()` function:
```c
void combat_end(CombatState* combat, CombatOutcome outcome) {
    combat->phase = COMBAT_PHASE_END;
    combat->outcome = outcome;

    if (outcome == COMBAT_OUTCOME_VICTORY) {
        /* Calculate and apply rewards */
        CombatRewards* rewards = combat_calculate_rewards(combat);
        combat_apply_rewards(rewards, g_game_state);

        /* Log rewards */
        combat_log_message(combat, "Victory! Gained %u experience and %u soul energy",
            rewards->experience_total, rewards->soul_energy_gained);

        combat_rewards_destroy(rewards);
    } else if (outcome == COMBAT_OUTCOME_DEFEAT) {
        combat_log_message(combat, "Defeat! Your minions have fallen.");
        /* TODO: Apply penalties (Phase 5) */
    }
}
```

---

## Week 18: Combat UI & Polish (Days 120-126)

### Overview
Create the visual interface for combat, add polish features, and thoroughly test the complete combat system.

### Day 120-121: Combat UI Rendering

**File: `src/game/combat/combat_ui.h`** (~100 lines)

```c
/**
 * @file combat_ui.h
 * @brief Combat screen rendering
 */

/**
 * @brief Render full combat screen
 *
 * Layout:
 * ┌─────────────────────────────────────┐
 * │ COMBAT - Turn 3                     │
 * ├─────────────────────────────────────┤
 * │ YOUR FORCES:                        │
 * │ [M1] Zombie "Groaner"     [45/60]   │
 * │ [M2] Skeleton "Bones"     [30/30]   │
 * │                                     │
 * │ ENEMIES:                            │
 * │ [E1] Guard                [20/60]   │
 * │ [E2] Priest               [55/60]   │
 * ├─────────────────────────────────────┤
 * │ > Zombie attacks Guard for 15 dmg  │
 * │ > Guard defends (+50% defense)     │
 * │ > Priest heals Guard for 10 HP     │
 * ├─────────────────────────────────────┤
 * │ Active: [M1] Zombie "Groaner"       │
 * │ Commands: attack, defend, flee      │
 * └─────────────────────────────────────┘
 *
 * @param combat Combat state to render
 */
void combat_ui_render(const CombatState* combat);

/**
 * @brief Render combatant health bar
 *
 * Format: [████████░░] 45/60
 *
 * @param current Current HP
 * @param max Maximum HP
 * @param width Bar width in characters
 * @return Formatted health bar string
 */
const char* combat_ui_health_bar(uint32_t current, uint32_t max, int width);

/**
 * @brief Render turn order display
 *
 * Shows initiative order with indicators
 *
 * @param combat Combat state
 */
void combat_ui_render_turn_order(const CombatState* combat);

/**
 * @brief Render combat log (last N messages)
 *
 * @param combat Combat state
 * @param message_count Number of messages to show
 */
void combat_ui_render_log(const CombatState* combat, uint8_t message_count);

/**
 * @brief Render available commands for current phase
 *
 * @param combat Combat state
 */
void combat_ui_render_commands(const CombatState* combat);
```

**File: `src/game/combat/combat_ui.c`** (~400 lines)

**Implementation details:**
- Use ncurses for rendering (colors, boxes)
- Color-code HP bars (green > 70%, yellow 30-70%, red < 30%)
- Use ANSI color pairs from `terminal/colors.h`
- Highlight active combatant
- Show status indicators (defending, stunned, etc.)

**Integration:**
- Add combat render callback to StateManager
- Hook into game loop's render phase
- Clear screen between combat turns

---

### Day 122: Combat State Machine Integration

**Update: `src/commands/commands/cmd_attack.c`**

Add automatic state transitions:
```c
/* After successful attack */
combatant->has_acted_this_turn = true;

/* Check if all player forces have acted */
bool all_acted = true;
for (uint8_t i = 0; i < combat->player_force_count; i++) {
    if (combat->player_forces[i]->is_player_controlled &&
        combatant_is_alive(combat->player_forces[i]) &&
        !combat->player_forces[i]->has_acted_this_turn) {
        all_acted = false;
        break;
    }
}

if (all_acted) {
    /* Transition to enemy turn */
    combat->phase = COMBAT_PHASE_ENEMY_TURN;
    combat_log_message(combat, "Enemy turn begins...");

    /* Process all enemy turns */
    combat_process_ai_turn(combat);

    /* Check for victory/defeat */
    if (combat_check_victory(combat)) {
        combat_end(combat, COMBAT_OUTCOME_VICTORY);
    } else if (combat_check_defeat(combat)) {
        combat_end(combat, COMBAT_OUTCOME_DEFEAT);
    } else {
        /* Start new turn */
        combat_advance_turn(combat);
    }
}
```

**Update: `src/game/combat/combat.c`**

Modify `combat_update()` to handle automatic phase transitions:
```c
void combat_update(CombatState* combat, double delta_time) {
    if (!combat) return;

    switch (combat->phase) {
        case COMBAT_PHASE_PLAYER_TURN:
            /* Wait for player input */
            combat->player_can_act = true;
            break;

        case COMBAT_PHASE_ENEMY_TURN:
            /* Process AI (already handled by command) */
            combat->player_can_act = false;
            break;

        case COMBAT_PHASE_RESOLUTION:
            /* Check win/loss conditions */
            if (combat_check_victory(combat)) {
                combat_end(combat, COMBAT_OUTCOME_VICTORY);
            } else if (combat_check_defeat(combat)) {
                combat_end(combat, COMBAT_OUTCOME_DEFEAT);
            } else {
                combat->phase = COMBAT_PHASE_PLAYER_TURN;
            }
            break;

        case COMBAT_PHASE_END:
            /* Combat finished - transition back to world */
            state_manager_pop_state(g_state_manager);  /* Exit combat */
            break;

        default:
            break;
    }
}
```

---

### Day 123: Tutorial Encounter

**File: `src/game/combat/tutorial_combat.h`** (~50 lines)

```c
/**
 * @file tutorial_combat.h
 * @brief Tutorial combat encounter
 */

/**
 * @brief Create tutorial combat encounter
 *
 * A simple 1v1 fight to teach combat basics:
 * - Player has 1 zombie
 * - Enemy is 1 villager (weak)
 * - Guided prompts for each command
 *
 * @param game_state Game state to set up combat
 * @return true if tutorial created successfully
 */
bool tutorial_combat_create(GameState* game_state);

/**
 * @brief Check if player has completed tutorial
 *
 * @param game_state Game state
 * @return true if tutorial completed
 */
bool tutorial_combat_is_completed(const GameState* game_state);
```

**File: `src/game/combat/tutorial_combat.c`** (~150 lines)

**Tutorial flow:**
1. Create 1 zombie for player (if none exist)
2. Create 1 villager enemy (low stats)
3. Initialize combat with prompts:
   - "Type 'attack E1' to attack the villager"
   - "Try 'defend' to increase your defense"
   - "You can 'flee' to escape combat"
4. After victory, mark tutorial as complete
5. Award bonus soul energy (50) for completion

**Integration:**
- Add tutorial trigger to main menu or first game start
- Store completion in game state (new field: `tutorial_combat_complete`)

---

### Day 124: Unit Tests for Week 17

**File: `tests/test_damage.c`** (~300 lines)

Tests:
1. Basic damage calculation
2. Critical hit mechanics
3. Defense mitigation
4. Defending bonus
5. Minimum damage (1)
6. Damage type variations
7. Healing calculation
8. Attack result validation
9. Null safety
10. Edge cases (0 attack, 0 defense)

**File: `tests/test_combat_commands.c`** (~400 lines)

Tests:
1. Attack command (valid target)
2. Attack command (invalid target)
3. Attack command (not in combat)
4. Attack command (not player turn)
5. Attack command (already acted)
6. Defend command
7. Flee command (success)
8. Flee command (failure)
9. Combat status display
10. Spell casting (drain, bolt, weaken)
11. Mana deduction
12. Turn advancement after actions

**Expected total:** ~25 new tests

---

### Day 125: Integration Testing & Balance

**Test scenarios:**

1. **Full Combat Flow:**
   - Initialize combat with 2 player minions vs 2 enemies
   - Execute complete combat sequence
   - Verify rewards distribution
   - Check state transitions

2. **AI Behavior Validation:**
   - Test all 5 AI behaviors in real combat
   - Verify targeting logic
   - Check healing priorities

3. **Edge Cases:**
   - All player forces killed (defeat)
   - All enemies killed (victory)
   - Flee during various combat states
   - Spell casting with insufficient mana

4. **Performance:**
   - Combat with max combatants (32 vs 32)
   - 100-turn combat duration
   - Memory leak verification (valgrind)

**Balance tuning:**
- Adjust enemy stats if too easy/hard
- Tune critical hit chance (may reduce from 10% to 5%)
- Adjust flee success rates
- Balance spell mana costs

**File: `tests/test_combat_integration.c`** (~500 lines)

---

### Day 126: Documentation & Final Polish

**File: `docs/COMBAT_SYSTEM.md`** (~400 lines)

```markdown
# Combat System Documentation

## Overview
Turn-based combat with initiative-based turn order.

## Commands

### attack <target>
Attack an enemy combatant.

**Usage:** `attack E1`
**Requirements:** Player's turn, hasn't acted, target is alive
**Example:**
```
> attack E1
Zombie attacks Guard for 23 damage (30 base - 7 mitigated) - CRITICAL HIT!
Guard has 37/60 HP remaining.
```

### defend
Enter defensive stance (+50% defense until next turn).

**Usage:** `defend`
**Example:**
```
> defend
Zombie defends! Defense increased to 22 (15 base + 50%).
```

### flee
Attempt to escape combat.

**Usage:** `flee`
**Base chance:** 50%
**Modifiers:**
- +10% per dead enemy
- -20% if any ally HP < 30%

### cast <spell> <target>
Cast a combat spell.

**Available spells:**
- `drain <target>`: 15 damage, heal for 50% (15 mana)
- `bolt <target>`: 20 pure damage, ignores defense (20 mana)
- `weaken <target>`: -20% defense for 2 turns (10 mana)

## Combat Flow

1. **Initialization:**
   - Roll initiative for all combatants
   - Sort by initiative (highest first)
   - Enter PLAYER_TURN phase

2. **Player Turn:**
   - Each player-controlled combatant takes action
   - Commands: attack, defend, flee, cast
   - After all acted, transition to ENEMY_TURN

3. **Enemy Turn:**
   - AI processes all enemy combatants
   - AI uses behavior patterns (aggressive, defensive, etc.)
   - Auto-transition to RESOLUTION

4. **Resolution:**
   - Check victory (all enemies dead)
   - Check defeat (all player forces dead)
   - If neither, start new turn

5. **End:**
   - Apply rewards (victory) or penalties (defeat)
   - Return to world state

## Damage Calculation

```
Base Damage = attacker->attack
Critical Roll = 10% chance for 1.5x damage
Defense Mitigation = damage - (defender->defense / 2)
Defending Bonus = +50% defense if defending
Minimum Damage = 1
```

## Rewards

**Experience:** Sum of all enemy XP rewards
**Soul Energy:** Sum of all enemy soul energy
**Souls:** Harvest souls from enemy corpses
**Corruption:** +corruption if killed innocent enemies

## AI Behaviors

- **Aggressive:** Attack lowest HP enemy
- **Defensive:** Protect allies, heal if HP < 50%
- **Balanced:** Attack or defend based on HP
- **Support:** Prioritize healing allies
- **Tactical:** Focus fire on one target

## UI Reference

See `combat_ui.h` for rendering functions.
```

**Updates to CLAUDE.md:**
Add combat system section with command reference.

**Polish tasks:**
- Add ANSI color to all combat log messages
- Improve combat UI layout
- Add sound effects (text-based: "SLASH!", "BOOM!", etc.)
- Tutorial encounter polish
- Help text for all combat commands

---

## Testing Summary

**Week 17 tests:**
- test_damage.c: ~10 tests
- test_combat_commands.c: ~12 tests
- test_combat_integration.c: ~8 tests
**Subtotal:** ~30 tests

**Week 18 tests:**
- test_combat_ui.c: ~5 tests (rendering validation)
- test_tutorial_combat.c: ~5 tests
**Subtotal:** ~10 tests

**Grand total (Weeks 15-18):** 45 + 30 + 10 = **85 unit tests**

---

## Deliverables Checklist

### Week 17
- [ ] damage.h/c (damage calculation system)
- [ ] cmd_attack.c (attack command)
- [ ] cmd_defend.c (defend command)
- [ ] cmd_flee.c (flee command)
- [ ] cmd_combat_status.c (combat status display)
- [ ] cmd_cast_combat.c (spell casting)
- [ ] combat_rewards.h/c (victory/defeat resolution)
- [ ] test_damage.c (unit tests)
- [ ] test_combat_commands.c (command tests)

### Week 18
- [ ] combat_ui.h/c (UI rendering)
- [ ] tutorial_combat.h/c (tutorial encounter)
- [ ] test_combat_integration.c (integration tests)
- [ ] test_combat_ui.c (UI tests)
- [ ] test_tutorial_combat.c (tutorial tests)
- [ ] COMBAT_SYSTEM.md (documentation)
- [ ] Update CLAUDE.md with combat section
- [ ] Balance tuning and polish

---

## File Count Summary

**New files (Week 17):** 9 implementation + 2 test = 11 files
**New files (Week 18):** 4 implementation + 3 test + 1 doc = 8 files
**Total new files (Weeks 17-18):** 19 files

**Modified files:**
- builtin.c (command registration)
- combat.c (state transitions)
- game_state.h (tutorial_combat_complete field)
- CLAUDE.md (combat documentation)

**Total modified:** 4 files

---

## Estimated Lines of Code

**Week 17:**
- Implementation: ~1,850 lines
- Tests: ~1,200 lines
- **Subtotal:** ~3,050 lines

**Week 18:**
- Implementation: ~1,050 lines
- Tests: ~500 lines
- Documentation: ~400 lines
- **Subtotal:** ~1,950 lines

**Grand total (Weeks 17-18):** ~5,000 lines

**Phase 4 total (Weeks 15-18):** ~7,500 lines

---

## Dependencies

**External:**
- ncurses (UI rendering)
- math.h (rand for criticals)

**Internal:**
- All Phase 2 systems (souls, minions, resources)
- All Phase 3 systems (world, locations)
- Command system (Phase 1)
- State manager (Phase 0)

---

## Success Criteria

### Week 17
1. All damage calculations accurate and tested
2. All combat commands functional
3. Victory/defeat resolution working
4. Rewards properly applied to game state
5. 30+ unit tests passing
6. Zero memory leaks (valgrind)
7. Zero compiler warnings

### Week 18
1. Combat UI renders correctly
2. Tutorial encounter completable
3. Integration tests pass
4. Documentation complete
5. Balance feels fair (playtesting)
6. 40+ total tests passing (includes Week 17)
7. Clean build on Linux + macOS

### Overall Phase 4 Success
1. **85+ unit tests passing** (100% success rate)
2. **Zero memory leaks**
3. **Zero compiler warnings**
4. **Complete combat system** from encounter to rewards
5. **Playable tutorial** teaching all mechanics
6. **Comprehensive documentation**
7. **Ready for Phase 5** (advanced combat features)

---

## Next Steps (Phase 5 Preview)

After completing Weeks 17-18, the combat system will be fully functional but basic. Phase 5 will add:

1. **Status Effects System:**
   - Buffs (haste, strength, shield)
   - Debuffs (poison, slow, curse, stun)
   - Duration tracking
   - Effect stacking

2. **Advanced Spells:**
   - Area-of-effect spells
   - Summon spells (temporary minions)
   - Resurrection (mid-combat revival)
   - Transformation spells

3. **Combat Abilities:**
   - Special minion abilities
   - Ultimate abilities (once per combat)
   - Combo attacks
   - Counter-attacks

4. **Encounter Variety:**
   - Boss encounters
   - Ambush encounters
   - Environmental hazards
   - Multi-wave encounters

5. **Loot System:**
   - Equipment drops
   - Consumable items
   - Rare artifacts
   - Crafting materials

This plan provides a complete roadmap for Weeks 17-18, building directly on the solid foundation of Weeks 15-16.

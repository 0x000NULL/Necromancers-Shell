# Combat System Documentation

## Overview

The Necromancer's Shell combat system is a turn-based tactical combat engine with initiative-based turn order, damage calculation, AI behaviors, and reward distribution.

## Table of Contents

1. [Combat Commands](#combat-commands)
2. [Combat Flow](#combat-flow)
3. [Damage Calculation](#damage-calculation)
4. [AI Behaviors](#ai-behaviors)
5. [Rewards](#rewards)
6. [Combat UI](#combat-ui)
7. [Tutorial](#tutorial)

---

## Combat Commands

### attack \<target\>

Attack an enemy combatant with your active minion.

**Usage:** `attack E1`

**Requirements:**
- Must be in combat
- Must be player's turn
- Active combatant hasn't acted this turn
- Target must be alive enemy

**Example:**
```
> attack E1
Zombie attacks Guard for 23 damage (30 base - 7 mitigated) - CRITICAL HIT!
Guard has 37/60 HP remaining.
```

**Damage Calculation:**
- Base damage = attacker's attack stat
- 10% critical hit chance (1.5x damage)
- Defense mitigation: damage -= (defender->defense / 2)
- Defending bonus: +50% defense if target is defending
- Minimum damage: Always at least 1 HP

---

### defend

Enter a defensive stance, increasing your defense by 50% until the next turn.

**Usage:** `defend`

**Requirements:**
- Must be in combat
- Must be player's turn
- Active combatant hasn't acted this turn

**Example:**
```
> defend
Zombie defends! Defense increased to 22 (15 base + 50%).
```

**Notes:**
- Defending status resets at the start of next turn
- Useful when low on HP or expecting heavy attacks
- All combatants can see defending status

---

### flee

Attempt to escape from combat.

**Usage:** `flee`

**Base Chance:** 50%

**Modifiers:**
- +10% per dead enemy
- -20% if any ally HP < 30%
- Capped between 10% and 95%

**Example (Success):**
```
> flee
You flee from combat! (Success chance: 60%)

Your forces retreat to safety.
```

**Example (Failure):**
```
> flee
Failed to flee! (Success chance: 40%)

Enemies take advantage of your retreat!
Guard hits Zombie for 15 damage!
```

**Notes:**
- On failure, all living enemies get a free attack
- Fleeing ends your turn
- Cannot flee during enemy turn

---

### cast \<spell\> \<target\>

Cast a combat spell using mana.

**Usage:** `cast <spell_name> <target_id>`

**Available Spells:**

#### drain \<target\>
Deal 15 necrotic damage and heal yourself for 50% of damage dealt.

**Mana Cost:** 15
**Example:**
```
> cast drain E1
Zombie casts drain! (-15 mana)

Guard takes 15 necrotic damage
Zombie heals for 7 HP
```

#### bolt \<target\>
Deal 20 pure damage that ignores defense.

**Mana Cost:** 20
**Example:**
```
> cast bolt E1
Skeleton casts bolt! (-20 mana)

Guard takes 20 pure damage (ignores defense)
```

#### weaken \<target\>
Reduce target's defense by 20% for 2 turns.

**Mana Cost:** 10
**Example:**
```
> cast weaken E1
Wraith casts weaken! (-10 mana)

Guard's defense weakened!
-20% defense (12) for 2 turns
```

**Notes:**
- Spells consume mana from your global resources
- Spell casting counts as your turn action
- Insufficient mana prevents casting

---

### status

Display detailed combat status.

**Usage:** `status`

**Shows:**
- Turn number and phase
- All player forces with HP bars
- All enemy forces with HP bars
- Combat phase (Player Turn, Enemy Turn, etc.)
- Active combatant
- Recent combat log messages
- Available commands

**Example:**
```
> status

=== COMBAT STATUS - Turn 3 ===

Phase: Player Turn

YOUR FORCES:
  [M1] Zombie "Groaner"     [==========----------] 45/60 (75%) [DEFENDING]
  [M2] Skeleton "Bones"     [==================--] 30/30 (100%)

ENEMIES:
  [E1] Guard                [======--------------] 20/60 (33%)
  [E2] Priest               [===================-] 55/60 (91%)

--- Recent Events ---
  > Zombie defends! (+50% defense)
  > Guard attacks Zombie for 10 damage!
  > Priest heals Guard for 5 HP

Active: [M2] Skeleton "Bones"
Commands: attack <target>, defend, flee, cast <spell>
```

---

## Combat Flow

### 1. Initialization
1. Roll initiative for all combatants (speed + random 0-50)
2. Sort by initiative (highest first)
3. Enter PLAYER_TURN phase
4. Set turn number to 1

### 2. Player Turn
- Each player-controlled combatant takes one action
- Actions: attack, defend, flee, cast
- After all player forces act, transition to ENEMY_TURN

### 3. Enemy Turn
- AI processes all enemy combatants in turn order
- AI uses behavior patterns (aggressive, defensive, balanced, etc.)
- Automatically transitions to RESOLUTION phase

### 4. Resolution
- Check victory condition (all enemies dead)
- Check defeat condition (all player forces dead)
- If neither: start new turn (increment turn number, reset flags)
- If victory or defeat: enter END phase

### 5. End
- Apply rewards (victory) or penalties (defeat)
- Display results screen
- Return to world state
- Clean up combat resources

---

## Damage Calculation

### Physical Damage
```
Base Damage = attacker->attack
Critical Roll = 10% chance → Base Damage * 1.5
Effective Defense = defender->defense * (is_defending ? 1.5 : 1.0)
Mitigated Damage = Base Damage - (Effective Defense / 2)
Final Damage = max(Mitigated Damage, 1)
```

### Damage Types

**Physical:** Normal attacks, affected by defense
**Necrotic:** Death magic, affected by defense (used by drain spell)
**Holy:** Anti-undead damage, affected by defense
**Pure:** Ignores defense entirely (used by bolt spell)

### Critical Hits
- 10% chance on all attacks
- 1.5x damage multiplier
- Indicated with yellow color and "CRITICAL HIT!" message

### Defending
- +50% defense for one turn
- Resets at start of next turn
- Visible in combat status

---

## AI Behaviors

### Aggressive
**Target Priority:** Lowest HP enemy
**Actions:** Always attacks
**Strategy:** Focus fire to eliminate threats quickly

### Defensive
**Target Priority:** Protect allies
**Actions:** Heal if ally HP < 50%, otherwise attack
**Strategy:** Keep team alive, defensive positioning

### Balanced
**Target Priority:** Varies by situation
**Actions:** Attack when HP > 50%, defend when HP < 50%
**Strategy:** Adaptive based on health status

### Support
**Target Priority:** Lowest HP ally
**Actions:** Prioritize healing over attacking
**Strategy:** Keep allies at high HP

### Tactical
**Target Priority:** Same target as last turn (focus fire)
**Actions:** Attack same enemy until dead, then switch
**Strategy:** Eliminate enemies one at a time

---

## Rewards

### Victory Rewards

**Experience:**
- Sum of all defeated enemy XP values
- Applied to player level (level up every 1000 XP)

**Soul Energy:**
- Sum of all enemy soul energy rewards
- Added to global resources

**Souls:**
- One soul harvested per defeated enemy
- Soul type determined by enemy type:
  - Villager → Innocent soul
  - Guard → Warrior soul
  - Mage → Mage soul
  - Default → Common soul
- Soul quality varies (0-100)

**Corruption:**
- +5 corruption per innocent enemy killed (villagers)
- Tracked in corruption system

### Defeat Penalties

Currently no penalties applied (Phase 5 feature).

### Rewards Screen

Victory:
```
╔═══════════════════════════════════════════════════════════════╗
║                                                               ║
║                         VICTORY!                              ║
║                                                               ║
║              All enemies have been defeated!                  ║
║                                                               ║
╚═══════════════════════════════════════════════════════════════╝

Gained 150 experience!
Gained 75 soul energy!
Harvested 2 souls!
```

---

## Combat UI

### Main Combat Screen

The combat UI uses box-drawing characters and ANSI colors for a rich terminal experience:

```
╔═══════════════════════════════════════════════════════════════╗
║  COMBAT - Turn 3                                              ║
║  Phase: Player Turn                                           ║
╠═══════════════════════════════════════════════════════════════╣
║  YOUR FORCES:                                                 ║
║    [M1] Zombie "Groaner"     [==========----------] 45/60     ║
║    [M2] Skeleton "Bones"     [==================--] 30/30     ║
║                                                               ║
║  ENEMIES:                                                     ║
║    [E1] Guard                [======--------------] 20/60     ║
║    [E2] Priest               [===================-] 55/60     ║
╠═══════════════════════════════════════════════════════════════╣
║  > Zombie attacks Guard for 15 dmg                            ║
║  > Guard defends (+50% defense)                               ║
║  > Priest heals Guard for 10 HP                               ║
╠═══════════════════════════════════════════════════════════════╣
║  Active: [M1] Zombie "Groaner"                                ║
║  Commands: attack <target>, defend, flee, cast <spell>        ║
╚═══════════════════════════════════════════════════════════════╝
```

### HP Bar Colors

- **Green:** HP > 70%
- **Yellow:** 30% ≤ HP ≤ 70%
- **Red:** HP < 30%

### Status Indicators

- `[DEAD]` - Combatant is dead
- `[DEFENDING]` - Currently in defensive stance
- `[ACTED]` - Has already acted this turn

---

## Tutorial

### Tutorial Combat

A simple 1v1 encounter to teach combat basics.

**Setup:**
- Player has 1 tutorial zombie (created if needed)
- Enemy is 1 weak villager (50% normal stats)

**Guided Instructions:**
```
╔═══════════════════════════════════════════════════════════════╗
║                    COMBAT TUTORIAL                            ║
╚═══════════════════════════════════════════════════════════════╝

Welcome to combat! You'll learn the basics of fighting.

A weak villager stands before you. Time to test your skills.

TUTORIAL COMMANDS:
  attack E1  - Attack the enemy villager
  defend     - Enter defensive stance (+50% defense)
  flee       - Attempt to escape (not recommended for tutorial!)
  status     - View detailed combat status

TIP: Try using 'attack E1' to strike the villager!
```

**Completion Bonus:**
- 50 soul energy awarded upon victory
- Tutorial completion tracked (future feature)

**Creating Tutorial:**
```c
CombatState* combat = tutorial_combat_create(game_state);
if (combat) {
    /* Tutorial combat is active */
    /* Display welcome message */
    printf("%s\n", tutorial_combat_get_welcome_message());
    printf("%s\n", tutorial_combat_get_instructions());
}
```

---

## Advanced Features

### State Machine Integration

Combat is integrated with the StateManager system:

```c
/* Enter combat */
state_manager_push(g_state_manager, STATE_COMBAT);
g_game_state->combat = combat_state_create();

/* Exit combat (automatic on victory/defeat) */
state_manager_pop(g_state_manager);
```

### Combat Logging

All combat events are logged to a circular buffer:

```c
combat_log_message(combat, "%s attacks %s for %u damage!",
                   attacker->name, target->name, damage);
```

Log messages support:
- ANSI color codes
- Printf-style formatting
- Circular buffer (100 messages max)
- Retrievable for UI rendering

### Extensibility

The combat system is designed for future expansion:

**Phase 5 Features (Planned):**
- Status effects system (buffs/debuffs)
- Advanced spells (AoE, summons, etc.)
- Equipment and items
- Environmental hazards
- Boss encounters

---

## API Reference

### Combat Creation

```c
CombatState* combat_state_create(void);
void combat_state_destroy(CombatState* combat);
bool combat_add_player_combatant(CombatState* combat, Combatant* combatant);
bool combat_add_enemy_combatant(CombatState* combat, Combatant* combatant);
void combat_initialize(CombatState* combat);
```

### Combat Control

```c
void combat_advance_turn(CombatState* combat);
void combat_end(CombatState* combat, CombatOutcome outcome);
Combatant* combat_get_active_combatant(CombatState* combat);
bool combat_check_victory(const CombatState* combat);
bool combat_check_defeat(const CombatState* combat);
```

### Damage System

```c
AttackResult damage_calculate_attack(const Combatant* attacker,
                                     const Combatant* defender,
                                     DamageType damage_type);
bool damage_apply_attack(CombatState* combat, Combatant* target,
                        const AttackResult* result);
uint32_t damage_calculate_heal(const Combatant* healer, const Combatant* target);
void damage_apply_heal(CombatState* combat, Combatant* target, uint32_t amount);
```

### UI Rendering

```c
void combat_ui_render(const CombatState* combat);
void combat_ui_render_victory(const CombatState* combat);
void combat_ui_render_defeat(const CombatState* combat);
void combat_ui_format_health_bar(char* buf, size_t buf_size,
                                  uint32_t current, uint32_t max, int width);
```

---

## Performance Notes

- Combat state uses dynamic arrays with fixed capacity (32 combatants max)
- Circular log buffer prevents unbounded memory growth
- O(n log n) initiative sorting (only done once at combat start)
- O(n) turn processing per combatant
- No heap allocations during turn processing

---

## Testing

The combat system has comprehensive test coverage:

- **test_combat.c:** Combat state management (12 tests)
- **test_combatant.c:** Combatant creation and manipulation (11 tests)
- **test_enemy.c:** Enemy creation and stats (9 tests)
- **test_encounter.c:** Encounter generation (8 tests)

**Total:** 40+ combat-related tests, all passing

Run tests with:
```bash
make test
./build/test_test_combat
./build/test_test_combatant
```

---

## Known Limitations

1. **Status Effects:** Weaken spell logs effect but doesn't persist (Phase 5)
2. **Limited Spells:** Only 3 basic spells available
3. **No Equipment:** No weapon/armor system yet
4. **No Consumables:** No items usable in combat
5. **Single Target:** All spells are single-target only

These limitations will be addressed in Phase 5: Advanced Combat Features.

---

## Troubleshooting

### Combat not starting?
- Ensure game_state->combat is NULL before creating new combat
- Check that at least 1 player and 1 enemy combatant exist
- Verify combat_initialize() is called after adding combatants

### Commands not working?
- Check combat->phase is COMBAT_PHASE_PLAYER_TURN
- Verify combat->player_can_act is true
- Ensure active combatant hasn't already acted (has_acted_this_turn = false)

### Memory leaks?
- Always call combat_state_destroy() when ending combat
- Don't manually free combatants added to combat (combat owns them)
- Run `make valgrind` to detect leaks

---

## Credits

Combat System implemented in Phase 4 (Weeks 15-18)
Total LOC: ~5,000 lines (implementation + tests + documentation)

Part of the Necromancer's Shell project.

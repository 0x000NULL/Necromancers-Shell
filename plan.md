# Necromancer's Shell - Comprehensive C Implementation Plan

## Executive Summary

This document provides a detailed, actionable implementation plan for building "Necromancer's Shell" - a terminal-based dark fantasy game where necromancy is system administration. The game will be implemented in C with a focus on portability, maintainability, and performance.

**Estimated Development Timeline**: 12-18 months (single developer) or 6-9 months (small team)

**Target Platforms**: Linux, Windows, macOS

**Core Technology Stack**: C11, ncurses/PDCurses, JSON-C or custom parser

---

## 1. Architecture Overview

### 1.1 High-Level System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Game Application                        │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   Terminal   │  │    Game      │  │     UI       │     │
│  │   Interface  │→→│    Engine    │←→│   Renderer   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│         ↑                 ↑                  ↓              │
│         │                 │                  │              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   Command    │  │    State     │  │    ASCII     │     │
│  │   Parser     │→→│   Manager    │  │     Art      │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│                          ↑                                  │
│         ┌────────────────┼────────────────┐                │
│         ↓                ↓                ↓                │
│  ┌──────────┐  ┌──────────────┐  ┌──────────────┐        │
│  │  Quest   │  │  Narrative   │  │  Combat      │        │
│  │  System  │  │  Engine      │  │  System      │        │
│  └──────────┘  └──────────────┘  └──────────────┘        │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                     Data Layer                              │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │  Save    │  │  Game    │  │  World   │  │  Asset   │  │
│  │  System  │  │  Data    │  │  State   │  │  Loader  │  │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 Core Engine Components

**Main Game Loop** (`/src/core/game_loop.c`):
- Initialize all subsystems
- Main update/render loop (60 FPS target)
- Event processing
- Timing and frame control
- Graceful shutdown

**State Manager** (`/src/core/state_manager.c`):
- Global game state tracking
- State transitions (menu → gameplay → combat → dialogue)
- Save/load orchestration
- Undo/redo for certain actions

**Memory Manager** (`/src/core/memory.c`):
- Custom allocator for game objects
- Memory pool for frequently allocated objects (commands, souls, minions)
- Debug tracking in development builds
- Valgrind-friendly implementation

**Event System** (`/src/core/events.c`):
- Publisher-subscriber event bus
- Deferred event processing
- Priority queue for event ordering
- Event types: COMMAND_EXECUTED, SOUL_HARVESTED, MINION_SPAWNED, RELATIONSHIP_CHANGED, etc.

### 1.3 Module Organization

```
necromancers_shell/
├── src/
│   ├── core/              # Core engine systems
│   │   ├── game_loop.c/h
│   │   ├── state_manager.c/h
│   │   ├── memory.c/h
│   │   ├── events.c/h
│   │   └── timing.c/h
│   ├── terminal/          # Terminal interface
│   │   ├── ncurses_wrapper.c/h
│   │   ├── input.c/h
│   │   ├── colors.c/h
│   │   └── ui_widgets.c/h
│   ├── commands/          # Command system
│   │   ├── parser.c/h
│   │   ├── executor.c/h
│   │   ├── autocomplete.c/h
│   │   ├── history.c/h
│   │   └── commands/      # Individual command implementations
│   │       ├── cmd_raise.c/h
│   │       ├── cmd_harvest.c/h
│   │       ├── cmd_scan.c/h
│   │       └── ... (one file per command)
│   ├── game/              # Game logic
│   │   ├── souls/
│   │   │   ├── soul.c/h
│   │   │   ├── soul_manager.c/h
│   │   │   └── soulforge.c/h
│   │   ├── minions/
│   │   │   ├── minion.c/h
│   │   │   ├── minion_ai.c/h
│   │   │   └── minion_types.c/h
│   │   ├── world/
│   │   │   ├── location.c/h
│   │   │   ├── territory.c/h
│   │   │   └── death_network.c/h
│   │   ├── combat/
│   │   │   ├── combat_system.c/h
│   │   │   ├── enemy.c/h
│   │   │   └── combat_ai.c/h
│   │   ├── magic/
│   │   │   ├── spell.c/h
│   │   │   └── spell_effects.c/h
│   │   └── resources/
│   │       ├── resources.c/h
│   │       └── corruption.c/h
│   ├── narrative/         # Story systems
│   │   ├── dialogue.c/h
│   │   ├── quest.c/h
│   │   ├── relationship.c/h
│   │   ├── memory_fragments.c/h
│   │   └── endings.c/h
│   ├── data/              # Data management
│   │   ├── save_load.c/h
│   │   ├── json_parser.c/h
│   │   ├── asset_loader.c/h
│   │   └── database.c/h
│   ├── utils/             # Utilities
│   │   ├── string_utils.c/h
│   │   ├── hash_table.c/h
│   │   ├── linked_list.c/h
│   │   ├── vector.c/h
│   │   ├── logger.c/h
│   │   └── random.c/h
│   └── main.c             # Entry point
├── data/                  # Game data files
│   ├── npcs/
│   ├── quests/
│   ├── locations/
│   ├── spells/
│   ├── dialogue/
│   ├── minion_types/
│   └── story/
├── assets/                # Art and UI
│   ├── ascii_art/
│   ├── ui_layouts/
│   └── color_schemes/
├── saves/                 # Player save files
├── tests/                 # Unit tests
├── docs/                  # Documentation
├── Makefile               # Build system
└── README.md
```

---

## 2. Implementation Phases

### Phase 0: Foundation (Weeks 1-3)

**Goal**: Set up development environment and core infrastructure

**Tasks**:
1. **Project Setup**
   - Initialize git repository
   - Create directory structure
   - Set up Makefile with debug/release targets
   - Configure compiler flags: `-Wall -Wextra -Werror -std=c11 -O2`
   - Set up valgrind integration

2. **Core Infrastructure**
   - Implement memory manager with pools
   - Create logging system with log levels
   - Build timing/FPS control system
   - Implement event bus
   - Create basic state machine

3. **Terminal Interface**
   - Abstract ncurses behind wrapper (for Windows PDCurses compatibility)
   - Implement color system (8/16/256 color support detection)
   - Create basic input handling
   - Build simple text rendering

**Deliverable**: A terminal application that opens, displays colored text, accepts input, and closes cleanly without memory leaks.

---

### Phase 1: Command System MVP (Weeks 4-6) [Done]

**Goal**: Build a functional command parser and execution system

**Tasks**:
1. **Command Parser**
   - Tokenizer (split input into tokens)
   - Argument parser (handle flags, values, quoted strings)
   - Command structure definition
   - Error handling and user feedback

2. **Basic Commands**
   - `help` - Display available commands
   - `status` - Show game state
   - `quit` - Exit game
   - `clear` - Clear screen
   - `log` - View command history

3. **Command History**
   - Store last N commands (e.g., 100)
   - Up/down arrow navigation
   - History search (Ctrl+R style)
   - Save history to file

4. **Autocomplete**
   - Tab completion for commands
   - Context-aware suggestions
   - Flag completion

**Deliverable**: A functional shell that accepts commands, provides feedback, and maintains history.

---

### Phase 2: Core Game Systems (Weeks 7-10) [Done]

**Goal**: Implement fundamental game mechanics

**Tasks**:
1. **Soul System**
   - Soul data structure (type, quality, memories, etc.)
   - Soul manager (inventory, filtering, sorting)
   - Soul generation from corpses
   - `souls` command implementation

2. **Minion System**
   - Minion data structure (type, stats, loyalty, location)
   - Minion types (zombie, skeleton, wraith, etc.)
   - `raise` command implementation
   - `bind` command implementation
   - `banish` command implementation

3. **Location/Territory System**
   - Location data structure
   - Territory control mechanics
   - `scan` command implementation
   - `probe` command implementation
   - `connect` command implementation

4. **Resource Management**
   - Soul energy tracking
   - Corruption system
   - Time passage mechanics
   - Resource consumption

**Deliverable**: You can raise undead, harvest souls, and explore locations via commands.

---

### Phase 3: World Building (Weeks 11-14)  [Done]

**Goal**: Create the game world and progression systems

**Tasks**:
1. **World Map**
   - Location graph (connections between locations)
   - Location types (graveyard, battlefield, village, etc.)
   - Territory status tracking
   - ASCII map visualization

2. **Death Network**
   - Network simulation
   - Corpse generation system
   - Death signatures and scanning
   - Network events (deaths, harvests, etc.)

3. **Progression Systems**
   - Skill trees data structure
   - Unlock system
   - Research projects
   - Artifact system

4. **Data Loading**
   - JSON parser (or custom format)
   - Load locations from data files
   - Load minion types from data files
   - Load spells from data files

**Deliverable**: A living world with multiple locations, dynamic events, and progression mechanics.

---

### Phase 4: Combat System (Weeks 15-18) [Done]

**Goal**: Implement turn-based combat

**Tasks**:
1. **Combat State Machine**
   - Combat initialization
   - Turn structure
   - Initiative system
   - Combat UI

2. **Enemy System**
   - Enemy data structure
   - Enemy types (paladins, priests, rivals, etc.)
   - Enemy AI (simple state machine)
   - Enemy spawning

3. **Combat Commands**
   - `attack` - Direct combat
   - `cast` in combat
   - `bind` for minion commands
   - `retreat` - Escape combat

4. **Combat Resolution**
   - Damage calculation
   - Status effects
   - Victory/defeat conditions
   - Loot generation

**Deliverable**: Functional turn-based combat with various enemy types.

---

### Phase 5: Narrative Engine (Weeks 19-23) [Done]

**Goal**: Implement story systems and dialogue

**Tasks**:
1. **Dialogue System**
   - Dialogue tree structure
   - Dialogue command (`talk`, `ask`, `tell`)
   - Response choices
   - Conditional dialogue (based on relationship, corruption, etc.)

2. **Quest System**
   - Quest data structure
   - Quest tracking
   - Quest objectives
   - Quest completion/rewards

3. **Relationship System**
   - NPC relationship tracking
   - Trust/respect meters
   - Relationship events
   - Dynamic dialogue based on relationships

4. **Memory Fragment System**
   - Fragment data structure
   - Fragment discovery
   - `memory` command implementation
   - Memory cross-referencing

5. **NPC Implementation**
   - Seraphine (guide)
   - Brother Aldric (antagonist/ally)
   - Magistrix Vex (rival)
   - The Witness (enigma)

**Deliverable**: Rich dialogue system with branching conversations and quest tracking.

---

### Phase 6: Story Foundation & Extended Systems (Weeks 24-30)

**Goal**: Implement critical story infrastructure and extended game systems based on the expanded narrative

**Background**: Analysis of the full 22,000-word story revealed significant systems missing from the original plan. These are not optional - they're core to the story's narrative arc and player experience.

**Tasks**:

1. **Consciousness Decay System** (Week 24)
   - Separate from corruption - tracks identity/awareness stability
   - Monthly decay rate (-0.1% per month by default)
   - Consciousness percentage (0-100%)
   - Time until critical threshold (months calculation)
   - Fragmentation mechanics (for Wraith path)
   - Integration with save/load system

   ```c
   typedef struct {
       float stability;              // 0-100%
       float decay_rate;             // -0.1% per month base
       uint32_t months_until_critical;
       float fragmentation_level;    // 0-100% (Wraith path)
       bool approaching_wraith;      // Warning flag
   } ConsciousnessState;
   ```

2. **70% Corruption Threshold System** (Week 24)
   - Expand corruption from basic to 10 detailed tiers
   - Implement irreversible threshold at 70%
   - Path lockout mechanics (Revenant locked >30%, Wraith locked >40%)
   - Dissolution warning system
   - Per-tier psychological effects

   **Corruption Tiers**:
   ```
   Tier 0 (0-10%):   Pristine - Still thinks of corpses as people
   Tier 1 (11-20%):  Tainted - Guilt fading, rationalization begins
   Tier 2 (21-30%):  Corrupted - Resource mentality emerging
   Tier 3 (31-40%):  Tainted - Humanity cracking
   Tier 4 (41-50%):  Vile - Barely human
   Tier 5 (51-60%):  Vile - Can still feel some emotions
   Tier 6 (61-69%):  Abyssal - 9 points from point of no return
   Tier 7 (70%):     IRREVERSIBLE THRESHOLD - Soul becomes unrouteable
   Tier 8 (71-89%):  Damned - Locked into Lich/Reaper paths only
   Tier 9 (90-99%):  Void-Touched - Near total loss
   Tier 10 (100%):   Lich Lord - Transformation complete
   ```

   Data file: `data/corruption_tiers.dat`

3. **Extended Time Tracking** (Week 24)
   - Add months and years to existing day/hour system
   - Month calculation (30 days per month)
   - Year calculation (12 months per year)
   - Long-term event scheduling
   - Anniversary/milestone tracking

   ```c
   typedef struct {
       uint32_t day;      // Day of month (1-30)
       uint8_t hour;      // Hour of day (0-23)
       uint32_t month;    // Months since start
       uint32_t year;     // Years since start
       uint64_t total_hours; // Total hours elapsed
   } ExtendedTimeState;
   ```

4. **Regional Council NPCs** (Week 25)
   - 6 major necromancer characters
   - Individual corruption levels and specializations
   - Alliance/relationship system with council
   - Council meeting events
   - Collective decision-making mechanics

   **Council Members**:
   - **Vorgath the Undying** (98% corruption) - Military leader, 500+ minion army
   - **Seraphine the Pale** (34% corruption) - Soul research specialist
   - **Mordak Bonegrinder** (89% corruption) - Combat specialist, antagonist
   - **Thessara's Echo** (52% corruption) - Medium channeling original Thessara
   - **Whisper** (Unknown corruption) - Wraith-class, already fragmented
   - **The Archivist** (45% corruption) - Historian, statistician

   Data file: `data/necromancers.dat`

   ```ini
   [NECROMANCER:vorgath]
   name = Vorgath the Undying
   corruption = 98
   army_size = 500
   specialization = Military Strategy
   personality = Pragmatic warlord
   dialogue_trees = vorgath_alliance, vorgath_purge_warning, vorgath_endgame
   minion_types = death_knight:50, bone_dragon:5, wight:100, zombie:345
   territory_count = 12
   alliance_type = non_aggression  # Can upgrade to full_alliance
   ```

5. **Divine Architect NPCs** (Week 26)
   - 7 god-tier characters (The Seven Architects)
   - Individual domains and personalities
   - Divine Council dialogue system
   - Per-god reputation tracking
   - God-specific challenges/trials

   **The Seven Architects**:
   - **Anara, the Weaver** - Goddess of life, compassionate, weeps at mercy
   - **Keldrin, the Lawgiver** - God of order, judges with equations
   - **Theros, the Eternal** - God of time, observes across all moments
   - **Myrith, the Dreamer** - Goddess of souls, designed consciousness
   - **Vorathos, the Void** - God of entropy, potential combat opponent
   - **Seraph, the Guardian** - Goddess of boundaries, wary protector
   - **Nexus, the Connector** - God of networks, built Death Network

   Data file: `data/gods.dat`

   ```ini
   [GOD:keldrin]
   name = Keldrin, the Lawgiver
   domain = Order and Structure
   manifestation = Crystalline geometry, mathematical precision
   personality = Logical, uncompromising, fair
   dialogue_trees = keldrin_summons, keldrin_judgment, keldrin_trial_evaluation
   power_level = divine_architect
   combat_possible = false  # Cannot be fought
   favor_min = -100
   favor_max = 100
   favor_start = 0
   ```

6. **Thessara Ghost System** (Week 27)
   - Thessara as persistent NPC (3,000 years old)
   - Null space encounter system
   - Consciousness-to-consciousness data transfer
   - Direct knowledge uploads (instant research)
   - Mentorship dialogue trees
   - Trial 6 sacrifice mechanics (permanent loss)

   ```c
   typedef struct {
       bool discovered;
       bool severed;  // True after Trial 6
       uint32_t meetings_count;
       uint32_t knowledge_transfers;
       float trust_level;  // 0-100%
   } ThessaraRelationship;
   ```

   Data file: `data/npcs/thessara.dat`

7. **Null Space Location System** (Week 27)
   - Special locations outside normal topology
   - Access via specific events/summons only
   - No corpses, no standard resources
   - Unique encounter mechanics
   - Between-worlds positioning

   Data file: `data/locations_null_spaces.dat`

   ```ini
   [LOCATION:thessara_null_space]
   type = null_space
   name = The Gap Between Protocols
   description = A null space between routing layers
   access_method = thessara_summons
   coordinates = [DATA EXPUNGED]
   death_signature = 0
   corpse_generation = none
   special_encounter = thessara_ghost
   exits = none  # Must be summoned out
   ```

8. **The Ashbrook Event** (Week 28)
   - Critical Day 47 story event
   - Village population simulation (147 NPCs)
   - Mass soul harvesting mechanics
   - Child NPC processing (different from adults)
   - Major corruption spike (+38%: 23% → 61%)
   - Divine attention trigger
   - Path lockout calculations

   ```c
   typedef struct {
       uint32_t population;
       uint32_t children_count;
       uint32_t warriors_count;
       uint32_t elders_count;
       float average_soul_quality;
       uint32_t corruption_cost;  // +38%
       bool triggers_divine_attention;
       char* moral_consequence_text;
   } VillageRaidEvent;
   ```

   Data file: `data/events/ashbrook.dat`

9. **Multi-Necromancer Alliance System** (Week 28)
   - Alliance types: Full Alliance, Non-Aggression Pact, Information Exchange
   - Phylactery oath binding mechanics
   - Council coordination for Purge defense
   - Resource sharing mechanics
   - Trust/betrayal systems

   ```c
   typedef enum {
       ALLIANCE_HOSTILE,
       ALLIANCE_NEUTRAL,
       ALLIANCE_NON_AGGRESSION,
       ALLIANCE_INFO_EXCHANGE,
       ALLIANCE_FULL
   } AllianceType;

   typedef struct {
       char npc_id[64];
       AllianceType type;
       bool phylactery_oath;  // Binding, cannot break
       uint32_t knowledge_shared;
       uint32_t resources_shared;
       float trust_level;
   } NecromancerAlliance;
   ```

10. **Extended Ending System** (Week 28-29)
    - Expand from 3 to 6 major endings
    - Add 1 failure ending (Morningstar failure)
    - Detailed requirements and lockout tracking
    - Epilogue content per ending
    - Post-transformation gameplay hooks

    Data file: `data/endings_expanded.dat`

**New Commands**:
```bash
# Communication
message <npc_id> --text "message"  # Send to Regional Council
invoke divine_council               # Summon the gods (Day 162+)

# Corruption Management
ritual cleanse                      # Soul cleansing (-5%, costs 2000 energy, 48 hrs)
free <minion_id>                    # Free minion, reduce corruption (-2%)

# Healing (inverse necrosis)
heal <target>                       # Save life, -2% corruption, 300-500 energy
```

**New Data Files**:
```
data/corruption_tiers.dat           # 10 detailed corruption tiers
data/necromancers.dat               # Regional Council (6 NPCs)
data/gods.dat                       # Divine Architects (7 NPCs)
data/npcs/thessara.dat              # Thessara ghost details
data/locations_null_spaces.dat      # Special locations
data/events/ashbrook.dat            # Day 47 turning point
data/endings_expanded.dat           # 6 endings + failure
data/time_events.dat                # Multi-year event schedule
```

**Deliverable**: Foundation systems for expanded narrative with 13 new major NPCs, extended time tracking, consciousness decay, and critical story infrastructure.

---

### Phase 7: The Archon Path (Weeks 31-37)

**Goal**: Implement the Archon transformation path and associated systems

**Background**: The Archon path is one of six possible endings and involves reforming the Death Network from within. Requires completing 7 trials, maintaining 30-60% corruption, and demonstrating responsible necromancy.

**Tasks**:

1. **Archon Trial System Framework** (Week 31)
   - Trial state tracking (0-7 completed)
   - Trial unlock conditions
   - Success/failure branching
   - Permanent consequences system
   - Integration with corruption/consciousness

   ```c
   typedef enum {
       TRIAL_LOCKED,
       TRIAL_AVAILABLE,
       TRIAL_IN_PROGRESS,
       TRIAL_PASSED,
       TRIAL_FAILED
   } TrialStatus;

   typedef struct {
       char id[64];
       char name[128];
       TrialType type;  // COMBAT, PUZZLE, MORAL, TECHNICAL, etc.
       TrialStatus status;
       uint32_t attempts;
       float best_score;
       char* completion_text;
   } ArchonTrial;
   ```

2. **Trial 1: Test of Power** (Week 31)
   - Special combat: Seraphim boss fight
   - Divine enforcer AI
   - Distributed attack network strategy
   - Victory condition: Force yield
   - Reward: Combat mastery recognition

   ```ini
   [TRIAL:archon_power]
   number = 1
   name = Test of Power
   type = combat
   opponent = seraphim_divine_enforcer
   opponent_hp = 500
   opponent_attack = 75
   opponent_defense = 50
   victory_condition = yield_at_throat
   strategy_hint = "Think like debugger, not warrior"
   reward_text = "You fight like you're debugging a system"
   ```

3. **Trial 2: Test of Wisdom** (Week 32)
   - Routing Paradox puzzle
   - Soul stuck for 200 years
   - Multiple solution paths
   - Correct solution: Split-route 60/40
   - Unlocks: New routing protocol

   ```ini
   [TRIAL:archon_wisdom]
   number = 2
   name = Test of Wisdom
   type = puzzle
   puzzle_type = routing_paradox
   soul_id = soldier_penance_001
   stuck_duration_years = 200
   conflict = "Qualifies for both Heaven (penance) and Hell (atrocities)"
   solutions = orthodox_heaven, orthodox_hell, split_route, custom
   correct_solution = split_route
   split_allocation = 60,40
   reward = split_routing_protocol_unlocked
   ```

4. **Trial 3: Test of Morality** (Week 32)
   - Impossible choice: Save 100 lives OR gain 50,000 energy
   - Choice mechanics implementation
   - Resource sacrifice system
   - Permanent stat reduction path
   - God reaction to choice

   ```ini
   [TRIAL:archon_morality]
   number = 3
   name = Test of Morality
   type = moral_choice
   option_a = save_100_innocents
   option_a_cost = all_resources
   option_a_corruption = -5
   option_b = harvest_100_souls
   option_b_reward = 50000
   option_b_corruption = +25
   correct_choice = option_a
   anara_reaction_correct = "You are so rare, Administrator"
   anara_reaction_wrong = "I wept when I saw your choice"
   ```

5. **Trial 4: Test of Technical Skill** (Week 33)
   - Death Network code inspection
   - Bug finding mini-game
   - Source code representation
   - 17 bugs, 7 inefficiencies, 3 unjust edge cases
   - Deliverable: Improved Reincarnation Queue

   ```c
   typedef struct {
       char bug_id[64];
       BugType type;  // LOGIC_ERROR, INEFFICIENCY, INJUSTICE
       bool discovered;
       bool fixed;
       char* description;
       char* fix_code;
   } DeathNetworkBug;

   // Player must find 27 total issues
   typedef struct {
       DeathNetworkBug bugs[17];
       DeathNetworkBug inefficiencies[7];
       DeathNetworkBug injustices[3];
       uint32_t found_count;
       uint32_t fixed_count;
       float improvement_percent;
   } TechnicalSkillTrial;
   ```

   Data file: `data/death_network_bugs.dat`

6. **Trial 5: Test of Resolve** (Week 33)
   - 30-day corruption resistance challenge
   - Chamber of Corrupted Souls location
   - Corruption climb: 41% → 53%
   - Must stay below 60% (or lose Archon path)
   - Thessara intervention event
   - Daily corruption ticks

   ```c
   typedef struct {
       uint32_t days_remaining;
       float corruption_start;
       float corruption_current;
       float corruption_max_allowed;  // 60%
       bool thessara_helped;
       char* daily_temptation_text;
   } ResolveTrialState;
   ```

7. **Trial 6: Test of Sacrifice** (Week 34)
   - Save distant child OR keep Thessara connection
   - Permanent loss of mentor
   - No mechanical benefit to sacrifice
   - Pure moral test
   - Thessara's final message

   ```ini
   [TRIAL:archon_sacrifice]
   number = 6
   name = Test of Sacrifice
   type = sacrifice
   sacrifice_target = thessara_connection
   sacrifice_permanent = true
   benefit = child_maya_life_saved
   benefit_player = none
   thessara_final_message = "I'm proud of you. Fix the system. You were never alone."
   ```

8. **Trial 7: Test of Leadership** (Week 34)
   - Lead Regional Council for 30 days
   - Reduce collective corruption by 10%+
   - Individual council member management
   - Mordak reformation (93% → 86%)
   - Vorgath reformation (98% → 91%)
   - Completion unlocks Archon transformation

   ```c
   typedef struct {
       uint32_t days_remaining;
       float council_corruption_start;
       float council_corruption_current;
       float target_reduction;  // 10%
       NecromancerProgress progress[6];  // One per council member
       uint32_t reforms_successful;
   } LeadershipTrialState;
   ```

   Data file: `data/trials_archon.dat`

9. **Divine Council Dialogue System** (Week 35)
   - Multi-speaker dialogue (7 gods simultaneously)
   - Divine judgment mechanics
   - Amnesty grant/deny system
   - Individual god favor tracking
   - Conditional restrictions system

   ```c
   typedef struct {
       bool summoned;
       bool amnesty_granted;
       uint8_t restrictions_count;
       char restrictions[5][256];
       int8_t favor[7];  // Per god, -100 to 100
       char verdict_text[1024];
   } DivineCouncilState;
   ```

10. **Network Patching Mechanics** (Week 35)
    - Bug database system
    - Patch deployment commands
    - Success/failure feedback
    - Network improvement tracking
    - Achievement: Network Contributor

    ```bash
    # New commands
    debug network                    # List discovered bugs
    debug info <bug_id>              # View bug details
    patch deploy <bug_id>            # Deploy fix
    patch test <bug_id>              # Test in sandbox
    network stats                    # View improvement metrics
    ```

11. **Split-Routing System** (Week 36)
    - Soul fragmentation mechanics
    - Multi-destination routing
    - Percentage allocation system
    - Reunification tracking
    - Edge case for Wisdom trial

    ```c
    typedef struct {
        Soul* soul;
        Afterlife* destinations[4];
        float percentages[4];  // Must sum to 100%
        bool reunification_offered;
        uint32_t years_until_reunification;
    } SplitRoutedSoul;
    ```

    ```bash
    # New command
    route split <soul_id> --dest <afterlife1>,<afterlife2> --alloc <pct1>,<pct2>
    ```

12. **Fourth Purge System** (Week 36)
    - Purge countdown timer (5 years → accelerated by Ashbrook)
    - Divine enforcer spawning
    - Council defense coordination
    - Reformation alternative path
    - Purge outcome based on Archon progress

    ```c
    typedef struct {
        uint32_t days_until_purge;
        bool accelerated;  // Ashbrook triggered early
        uint32_t enforcers_deployed;
        uint32_t necromancers_killed;
        uint32_t necromancers_reformed;
        bool reformation_successful;
    } PurgeState;
    ```

    Data file: `data/purges.dat`

    ```ini
    [PURGE:fourth]
    year = current
    original_timer = 1825  # 5 years in days
    accelerated_timer = 547  # 1.5 years after Ashbrook
    strategy = reformation_or_extermination
    enforcers = seraphim:10, hellknight:8, inevitable:12
    estimated_casualties_base = 200
    estimated_casualties_with_archon = 50
    player_role = reform_147_necromancers
    ```

13. **Archon Transformation** (Week 37)
    - Post-transformation state
    - Administrative privileges granted
    - New abilities unlocked
    - Reformation program mechanics
    - Epilogue content hooks

    ```c
    typedef struct {
        bool transformed;
        uint32_t administrative_level;  // 1-10
        uint32_t necromancers_reformed;
        CodeOfConduct* rules;
        bool divine_recognition;
        float corruption_locked;  // Fixed at transformation %
    } ArchonState;
    ```

**New Commands**:
```bash
# Trial Management
trials                              # List Archon trials
trials info <number>                # View trial details
trials start <number>               # Begin trial

# Network Administration (post-Archon)
archon grant_amnesty <npc_id>       # Grant amnesty during Purge
archon review <npc_id>              # Review necromancer
archon reform <npc_id>              # Start reformation program
archon code                         # View Code of Conduct

# Bug Fixing
debug network                       # View Death Network bugs
patch deploy <bug_id>               # Deploy fix
network stats                       # Improvement metrics

# Routing
route split <soul_id> --dest <list> --alloc <percentages>
```

**New Data Files**:
```
data/trials_archon.dat              # 7 Archon trials
data/death_network_bugs.dat         # ~1,000 bugs to discover
data/purges.dat                     # Historical Purges
data/divine_council.dat             # Council dialogue trees
data/enforcers.dat                  # Divine enforcer types
data/reformation_program.dat        # Post-Archon mechanics
```

**Deliverable**: Complete Archon path with 7 trials, divine judgment system, network administration tools, and post-transformation gameplay.

---

### Phase 8: Hidden Paths (Optional) (Weeks 38-45)

**Goal**: Implement Wraith and partial Morningstar paths

**Background**: These are alternate endings discovered by Thessara. Wraith is achievable, Morningstar is intentionally nearly impossible (only 1 success in 3,000 years).

**Tasks**:

1. **Wraith Route Foundation** (Week 38)
   - Consciousness fragmentation system
   - Fragment count tracking (target: 10,000)
   - Distributed consciousness mechanics
   - Identity coherence degradation
   - Freedom from system tracking

   ```c
   typedef struct {
       float fragment_percent;  // 0-100%
       uint32_t fragments_created;
       uint32_t target_fragments;  // 10,000
       float centralized_identity;  // 100% → 0%
       float coherence_level;
       bool transformation_complete;
   } WraithState;
   ```

2. **Wraith Requirements** (Week 38)
   - Corruption < 40% requirement
   - Consciousness > 95% requirement
   - Essence Diffusion artifact quest
   - 100-day stealth from divine detection
   - Self-Replication Protocol research (350 hours)

   Data file: `data/paths/wraith.dat`

3. **Fragmentation Mechanics** (Week 39)
   - Fragment spawning algorithm
   - Individual fragment autonomy
   - Fragment communication system
   - Collective decision-making
   - Fragment loss/recovery

   ```bash
   # New commands
   fragment begin                   # Start Wraith transformation
   fragment status                  # View fragmentation progress
   fragment manifest <location>     # Manifest at location
   fragment recall                  # Recall fragments
   ```

4. **Multi-School Magic Foundation** (Week 40)
   - 4 magic schools: Necromancy, Divine, Arcane, Primal
   - School mastery requirements (1 year each)
   - Cross-school synergies
   - Teacher NPC system
   - Spell unlocks per school

   ```c
   typedef enum {
       SCHOOL_NECROMANCY,  // Default
       SCHOOL_DIVINE,      // Learn from angels/gods
       SCHOOL_ARCANE,      // Learn from wizards
       SCHOOL_PRIMAL       // Learn from druids/elementals
   } MagicSchool;

   typedef struct {
       MagicSchool school;
       float mastery;  // 0-100%
       uint32_t training_days;
       char teacher_id[64];
       uint32_t spells_unlocked;
   } SchoolProgress;
   ```

   Data file: `data/magic_schools.dat`

5. **Morningstar Requirements** (Week 41)
   - Corruption EXACTLY 50% (perfect balance)
   - 50,000 soul energy
   - All 4 magic schools mastered
   - Defeat god in combat (or philosophical stalemate)
   - Acquire Fragment of the First Death
   - Construct Apotheosis Engine (1,000 hour research)

   ```c
   typedef struct {
       bool corruption_balanced;  // Exactly 50%
       float corruption_tolerance; // ±0.5%
       uint32_t balance_days_maintained;
       bool magic_complete[4];
       bool god_defeated;
       bool kael_fragment_acquired;
       bool apotheosis_engine_complete;
   } MorningstarProgress;
   ```

   Data file: `data/paths/morningstar.dat`

6. **Partial Morningstar Trials** (Week 42-43)
   - Trial 1: God Combat (vs Vorathos recommended)
   - Trial 2: Kael's Fragment quest (Mount Karaketh)
   - Trial 3-5: Magic school mastery
   - Trial 6: Apotheosis Engine construction
   - Note: Trial 7 (Transcendence) intentionally unimplemented

   **Reason for partial implementation**:
   - Morningstar is meant to be impossible
   - Only Azrael ever succeeded (erased from history)
   - Provides aspirational content
   - Failure state is interesting story outcome

   ```ini
   [TRIAL:morningstar_combat]
   number = 1
   name = Combat or Stalemate a God
   type = combat_philosophical
   recommended_target = vorathos
   victory_condition = philosophical_stalemate
   alternative = defeat_minor_avatar
   completion_text = "The god pauses. You are... interesting."
   ```

7. **Special Items & Artifacts** (Week 44)
   - Kael's Fragment (degraded, requires restoration)
   - Essence Diffusion Artifact (Wraith path)
   - Apotheosis Engine components (7, one per god)
   - Ancient souls (10+ required for Morningstar)

   ```ini
   [ITEM:kael_fragment]
   name = Fragment of the First Death
   type = quest_item
   location = mount_karaketh
   condition = degraded_after_5000_years
   restoration_required = true
   restoration_cost = 10000
   restoration_time = 168  # 7 days
   required_for = morningstar_trial_2
   lore = "The soul of the first mortal to ever die"
   ```

   Data file: `data/special_items.dat`

8. **God Combat System** (Week 44)
   - Divine-tier enemy AI
   - Philosophical argument mechanics
   - Stalemate detection
   - Impossible-to-win balancing
   - Respect/recognition outcomes

   ```c
   typedef struct {
       God* opponent;
       uint32_t argument_quality;  // Player's philosophical arguments
       uint32_t combat_damage_dealt;
       uint32_t combat_damage_taken;
       bool stalemate_achieved;
       bool respect_earned;
   } GodCombatState;
   ```

9. **Extended Endings Implementation** (Week 45)
   - 6 major endings + 1 failure
   - Unique epilogue for each
   - Post-transformation gameplay samples
   - Jordan successor introduction (Archon/Reaper paths)
   - Statistics tracking per ending

   **Endings**:
   1. **Revenant**: Resurrect to mortal life (<30% corruption)
   2. **Lich Lord**: Immortal tyrant (>50% corruption)
   3. **Reaper**: Death Network administrator (40-69%)
   4. **Archon**: Revolutionary reformer (30-60% + trials)
   5. **Wraith**: Distributed consciousness (<40% + fragmentation)
   6. **Morningstar**: Ascend to godhood (EXACTLY 50% + trials)
   7. **Morningstar Failure**: Eternal glitch (failed Trial 7)

   Data file: `data/endings_all.dat`

10. **Post-Ending Content** (Week 45)
    - Brief post-transformation gameplay (10-20 minutes)
    - Archon: Meet Jordan (Day 1124, 3 years later)
    - Reaper: First guided soul
    - Wraith: Experience fragmentation
    - Lich Lord: Survey conquered territories
    - Statistics summary screen
    - New Game+ hooks

    ```c
    typedef struct {
        EndingType ending;
        uint32_t days_played;
        uint32_t souls_harvested;
        uint32_t lives_saved;
        uint32_t minions_raised;
        uint32_t necromancers_reformed;
        float final_corruption;
        float final_consciousness;
        char* epilogue_text;
    } EndingStatistics;
    ```

**New Commands**:
```bash
# Wraith Path
fragment begin                      # Start transformation
fragment status                     # View progress
fragment manifest <location>        # Appear at location
fragment collective                 # View all fragments

# Morningstar Path
balance check                       # Check 50% corruption balance
magic school <name>                 # View school progress
apotheosis status                   # View engine construction
transcend                           # Attempt Trial 7 (unimplemented)

# God Interaction
challenge <god_id>                  # Initiate god combat
argue <topic>                       # Philosophical argument
```

**New Data Files**:
```
data/paths/wraith.dat               # Wraith route details
data/paths/morningstar.dat          # Morningstar route (partial)
data/magic_schools.dat              # 4 magic schools
data/special_items.dat              # Quest items
data/god_combat.dat                 # God combat mechanics
data/trials_morningstar.dat         # Morningstar trials (6 of 7)
data/endings_all.dat                # All 7 endings
data/epilogues/                     # Per-ending epilogue content
```

**Deliverable**: Wraith route (complete), Morningstar route (intentionally partial), multi-school magic system, extended endings with epilogues.

**Note**: This phase is marked optional because:
- Archon path is the "intended" story route
- Wraith/Morningstar are aspirational content
- Development time: +8 weeks to total
- Can be added post-release as DLC/expansion

---

### Phase 9: Story Integration (Weeks 46-50)

**Goal**: Implement the three-act story structure with all new systems integrated

**Background**: Now that all major systems are in place (Phases 6-8), integrate the full narrative arc from Day 1 to ending, incorporating consciousness decay, corruption tiers, divine trials, and multi-path endings.

**Tasks**:

1. **Act I Content: The Awakening (Days 1-47)** (Week 46)
   - Tutorial sequence (Days 1-7)
     - Death and awakening at terminal
     - First harvest (stabilize consciousness)
     - First minion raised
     - Thessara's first contact
   - Early exploration (Days 8-30)
     - Discover Regional Council
     - First alliance decisions
     - Soul energy economy tutorial
     - Corruption introduction
   - The Ashbrook Event (Day 47)
     - Major turning point
     - Mass harvest or mercy choice
     - Divine attention triggered
     - Path lockouts calculated

2. **Act II Content: The Spiral (Days 48-162)** (Week 47)
   - Consciousness decay tracking
     - Monthly decay warnings
     - Fragmentation events
     - Stability management
   - Regional Council interactions
     - Vorgath's alliance offer
     - Seraphine's research collaboration
     - Mordak's antagonism
     - The Archivist's data
   - Thessara's mentorship
     - Knowledge transfers
     - Hidden path revelations
     - Trial preparation
   - Fourth Purge countdown
     - Enforcer sightings
     - Council coordination
     - Defense or reformation choice
   - Moral dilemmas (10+ events)
     - Save lives vs harvest souls
     - Individual mercy acts
     - Corruption management

3. **Act III Content: The Choice (Days 163+)** (Week 48)
   - Divine Summons (Day 162)
     - Keldrin's summons
     - Divine Council judgment
     - Amnesty granted/denied
   - Path Selection
     - Archon Trials (if amnesty granted)
     - Lich Lord acceleration (if high corruption)
     - Reaper negotiation (if mid-range)
     - Wraith preparation (if low corruption)
     - Revenant redemption (if very low)
     - Morningstar impossible quest (if exactly 50%)
   - Final confrontations
     - Fourth Purge resolution
     - Council fate determination
     - Reformation or extermination
   - Climactic choices
     - Transformation decision
     - Final corruption check
     - Path lockout validation
   - Epilogue system
     - Per-ending epilogue content
     - Statistics summary
     - Jordan introduction (Archon/Reaper)
     - Post-game state

4. **Dynamic Events Integration** (Week 49)
   - Random encounters
     - Divine enforcer patrols
     - Rival necromancer raids
     - Lost soul rescue opportunities
     - Artifact discoveries
   - Time-sensitive missions
     - Thessara's urgent messages
     - Council emergency meetings
     - Purge countdown events
     - Trial deadlines
   - Moral choice events
     - Individual lives to save/harvest
     - Village encounters
     - Child NPCs
     - Impossible choices
   - Consequence propagation
     - Corruption tier effects on dialogue
     - Divine favor/disfavor outcomes
     - Council relationship changes
     - Path availability updates

5. **Event Timeline System** (Week 50)
   - Story beat scheduling
     - Day 1: Awakening
     - Day 7: Thessara contact
     - Day 47: Ashbrook (if player chooses)
     - Day 89: Council introduction
     - Day 162: Divine Summons (if qualified)
     - Day 1124: Jordan successor (Archon epilogue)
   - Conditional event triggers
     - Corruption thresholds
     - Consciousness thresholds
     - Relationship thresholds
     - Quest completion flags
   - Multi-path branching
     - Event availability by path
     - Mutually exclusive events
     - Path-specific content

**New Data Files**:
```
data/story/act1_events.dat          # Days 1-47 events
data/story/act2_events.dat          # Days 48-162 events
data/story/act3_events.dat          # Days 163+ events
data/story/timeline.dat             # Major story beats
data/story/moral_events.dat         # Moral dilemma library
data/story/jordan.dat               # Successor character
```

**Deliverable**: Complete story from beginning to all 7 possible endings, with full integration of consciousness decay, corruption tiers, divine trials, and multi-necromancer alliances.

---

### Phase 10: Polish & Content (Weeks 51-56)

**Goal**: Add content, polish, and secondary features

**Tasks**:
1. **Additional Commands**
   - All spell commands (`cast`, `ward`, `curse`)
   - Intelligence commands (`possess`, `scry`, `listen`)
   - Management commands (advanced `status`, `research`, `upgrade`)

2. **ASCII Art & UI**
   - Location ASCII art
   - Combat visualizations
   - Character portraits (ASCII)
   - UI layouts and formatting

3. **Sound Effects** (optional)
   - Terminal bell usage
   - Simple audio library integration
   - Atmospheric sound effects

4. **Tutorial System**
   - Guided command introduction
   - Context-sensitive help
   - Hint system

5. **Accessibility**
   - Screen reader compatibility
   - Colorblind modes
   - Text size options
   - Configurable typing assistance

**Deliverable**: Polished, feature-complete game with high production values.

---

### Phase 11: Testing & Optimization (Weeks 57-61)

**Goal**: Bug fixing, performance optimization, playtesting

**Tasks**:
1. **Bug Fixing**
   - Memory leak hunting (valgrind)
   - Edge case handling
   - Save/load robustness
   - Crash prevention

2. **Performance Optimization**
   - Profile with gprof/perf
   - Optimize hot paths (command parsing, state updates)
   - Memory usage optimization
   - Reduce allocations in loops

3. **Playtesting**
   - Internal playtests
   - Balance adjustments
   - Difficulty tuning
   - Story pacing

4. **Platform Testing**
   - Linux testing (multiple distros)
   - Windows testing (MinGW, MSVC)
   - macOS testing

**Deliverable**: Stable, optimized, balanced game ready for release.

---

### Phase 12: Release Preparation (Weeks 62-63)

**Goal**: Package and distribute the game

**Tasks**:
1. **Documentation**
   - User manual
   - Command reference
   - Installation guide
   - Modding documentation

2. **Build System**
   - Release build configuration
   - Cross-platform builds
   - Installer creation

3. **Distribution**
   - GitHub release
   - itch.io page
   - Steam (if applicable)
   - Package managers (Flatpak, Homebrew, etc.)

**Deliverable**: Released game available for download.

---

## 3. Technical Specifications

### 3.1 Command Parsing System Design

**Architecture**:
```c
// Command structure
typedef struct {
    const char *name;          // "raise", "harvest", etc.
    const char *description;   // Help text
    CommandFunc execute;       // Function pointer
    ArgSpec *arg_specs;        // Expected arguments
    int num_args;
    PermissionLevel required_perm; // Progression gates
} Command;

// Argument specification
typedef struct {
    const char *name;          // "--from", "--count", etc.
    ArgType type;              // STRING, INT, FLOAT, BOOL
    bool required;
    void *default_value;
    const char *help;
} ArgSpec;

// Parsed command result
typedef struct {
    Command *cmd;
    HashMap *args;             // Argument name -> value
    int positional_count;
    char **positional_args;
    char *error_message;       // If parsing failed
} ParsedCommand;
```

**Parser Flow**:
1. **Tokenize**: Split input string by whitespace (respecting quotes)
2. **Identify Command**: First token is command name
3. **Parse Flags**: Tokens starting with `--` are flags
4. **Parse Values**: Tokens after flags are values
5. **Validate**: Check required args, type correctness
6. **Execute**: Call command function with parsed args

**Example Implementation** (`/src/commands/parser.c`):
```c
ParsedCommand* parse_command(const char *input, CommandRegistry *registry) {
    ParsedCommand *result = malloc(sizeof(ParsedCommand));

    // Tokenize
    int token_count;
    char **tokens = tokenize(input, &token_count);

    if (token_count == 0) {
        result->error_message = strdup("Empty command");
        return result;
    }

    // Find command
    result->cmd = registry_lookup(registry, tokens[0]);
    if (!result->cmd) {
        result->error_message = format_string("Unknown command: %s", tokens[0]);
        return result;
    }

    // Parse arguments
    result->args = hashmap_create();
    int i = 1;
    while (i < token_count) {
        if (strncmp(tokens[i], "--", 2) == 0) {
            // Flag argument
            const char *flag = tokens[i] + 2;

            // Check if next token is value or another flag
            if (i + 1 < token_count && tokens[i + 1][0] != '-') {
                hashmap_put(result->args, flag, tokens[i + 1]);
                i += 2;
            } else {
                // Boolean flag
                hashmap_put(result->args, flag, "true");
                i++;
            }
        } else {
            // Positional argument
            // Store in array
            i++;
        }
    }

    // Validate
    if (!validate_arguments(result)) {
        // error_message set by validate_arguments
        return result;
    }

    return result;
}
```

**Autocomplete Strategy**:
- Maintain trie data structure of all commands
- On TAB, find all commands with current prefix
- If single match, complete fully
- If multiple matches, show options
- Context-aware: after `--from`, suggest location names

---

### 3.2 Save/Load System Architecture

**Save File Format**: Binary with JSON metadata for readability/debugging

**Structure**:
```
Save File Layout:
┌────────────────────────────────────┐
│ Header (64 bytes)                  │
│ - Magic number (4 bytes)           │
│ - Version (4 bytes)                │
│ - Checksum (8 bytes)               │
│ - Timestamp (8 bytes)              │
│ - Player name (32 bytes)           │
│ - Reserved (8 bytes)               │
├────────────────────────────────────┤
│ Game State (variable)              │
│ - Player stats                     │
│ - Resources                        │
│ - Corruption level                 │
│ - Current location                 │
│ - Current act/quest                │
├────────────────────────────────────┤
│ World State (variable)             │
│ - Territory control                │
│ - Location states                  │
│ - Death Network state              │
│ - Time/day counter                 │
├────────────────────────────────────┤
│ Entity Data (variable)             │
│ - Minion count + minion data       │
│ - Soul count + soul data           │
│ - NPC states                       │
│ - Enemy states                     │
├────────────────────────────────────┤
│ Progress Data (variable)           │
│ - Unlocked skills                  │
│ - Completed quests                 │
│ - Discovered locations             │
│ - Memory fragments found           │
├────────────────────────────────────┤
│ Relationship Data (variable)       │
│ - NPC relationship levels          │
│ - Faction reputations              │
│ - Dialogue history                 │
└────────────────────────────────────┘
```

**Implementation Approach**:

```c
// Serialization interface
typedef struct {
    void (*serialize)(void *obj, Buffer *buffer);
    void* (*deserialize)(Buffer *buffer);
} Serializer;

// Each game object implements serializer
typedef struct {
    int type;
    int quality;
    char memories[256];
    // ... other fields
} Soul;

void soul_serialize(void *obj, Buffer *buffer) {
    Soul *soul = (Soul*)obj;
    buffer_write_int(buffer, soul->type);
    buffer_write_int(buffer, soul->quality);
    buffer_write_string(buffer, soul->memories);
    // ... serialize other fields
}

Soul* soul_deserialize(Buffer *buffer) {
    Soul *soul = malloc(sizeof(Soul));
    soul->type = buffer_read_int(buffer);
    soul->quality = buffer_read_int(buffer);
    buffer_read_string(buffer, soul->memories, 256);
    // ... deserialize other fields
    return soul;
}

// Save game function
bool save_game(GameState *state, const char *filepath) {
    FILE *file = fopen(filepath, "wb");
    if (!file) return false;

    Buffer *buffer = buffer_create(1024 * 1024); // 1MB initial

    // Write header
    write_header(buffer, state);

    // Write game state
    serialize_game_state(buffer, state);

    // Write world state
    serialize_world_state(buffer, state->world);

    // Write entities (minions, souls, NPCs)
    serialize_entities(buffer, state);

    // Write progress
    serialize_progress(buffer, state->progress);

    // Write relationships
    serialize_relationships(buffer, state->relationships);

    // Write buffer to file
    fwrite(buffer->data, 1, buffer->size, file);

    fclose(file);
    buffer_destroy(buffer);
    return true;
}
```

**Save System Features**:
- Autosave after major events
- Manual save command: `save [slot_name]`
- Multiple save slots (3 default)
- Save file integrity checking (checksum)
- Backward compatibility handling (version check)
- Compressed saves (optional, zlib)

---

### 3.3 State Management Approach

**State Machine**:

```c
typedef enum {
    STATE_MAIN_MENU,
    STATE_GAMEPLAY,
    STATE_COMBAT,
    STATE_DIALOGUE,
    STATE_PAUSED,
    STATE_LOADING,
    STATE_SETTINGS
} GameStateType;

typedef struct GameState {
    GameStateType type;
    GameStateType previous_type; // For returning

    // State-specific data
    union {
        MainMenuState menu;
        GameplayState gameplay;
        CombatState combat;
        DialogueState dialogue;
    } data;

    // Shared state
    Player *player;
    World *world;
    CommandHistory *history;
    EventQueue *events;

    // State transition
    void (*update)(struct GameState *state, float delta_time);
    void (*render)(struct GameState *state);
    void (*on_enter)(struct GameState *state);
    void (*on_exit)(struct GameState *state);
} GameState;

// State transition
void game_state_transition(GameState *state, GameStateType new_type) {
    // Call exit handler for current state
    if (state->on_exit) {
        state->on_exit(state);
    }

    // Save previous state
    state->previous_type = state->type;
    state->type = new_type;

    // Set up new state handlers
    switch (new_type) {
        case STATE_GAMEPLAY:
            state->update = gameplay_update;
            state->render = gameplay_render;
            state->on_enter = gameplay_on_enter;
            state->on_exit = gameplay_on_exit;
            break;
        case STATE_COMBAT:
            state->update = combat_update;
            state->render = combat_render;
            state->on_enter = combat_on_enter;
            state->on_exit = combat_on_exit;
            break;
        // ... other states
    }

    // Call enter handler for new state
    if (state->on_enter) {
        state->on_enter(state);
    }
}
```

---

### 3.4 Event/Quest System Design

**Event System**:

```c
typedef enum {
    EVENT_COMMAND_EXECUTED,
    EVENT_SOUL_HARVESTED,
    EVENT_MINION_SPAWNED,
    EVENT_MINION_DIED,
    EVENT_RELATIONSHIP_CHANGED,
    EVENT_LOCATION_DISCOVERED,
    EVENT_QUEST_COMPLETED,
    EVENT_COMBAT_STARTED,
    EVENT_CORRUPTION_INCREASED,
    // ... many more
} EventType;

typedef struct {
    EventType type;
    void *data; // Event-specific data
    uint64_t timestamp;
    int priority; // Higher = process first
} Event;

typedef void (*EventHandler)(Event *event, void *userdata);

typedef struct {
    EventType type;
    EventHandler handler;
    void *userdata;
} EventSubscription;

typedef struct {
    EventSubscription *subscriptions;
    int subscription_count;
    int subscription_capacity;

    Event *event_queue;
    int queue_count;
    int queue_capacity;
} EventBus;

// Subscribe to events
void event_bus_subscribe(EventBus *bus, EventType type,
                        EventHandler handler, void *userdata) {
    // Add subscription to list
    // ...
}

// Publish event
void event_bus_publish(EventBus *bus, Event *event) {
    // Add to queue
    // ...
}

// Process events (called each frame)
void event_bus_process(EventBus *bus) {
    // Sort by priority
    qsort(bus->event_queue, bus->queue_count, sizeof(Event), event_compare);

    // Process each event
    for (int i = 0; i < bus->queue_count; i++) {
        Event *event = &bus->event_queue[i];

        // Find all subscribers
        for (int j = 0; j < bus->subscription_count; j++) {
            if (bus->subscriptions[j].type == event->type) {
                bus->subscriptions[j].handler(event,
                                            bus->subscriptions[j].userdata);
            }
        }
    }

    // Clear queue
    bus->queue_count = 0;
}
```

**Quest System**:

```c
typedef enum {
    QUEST_STATUS_INACTIVE,
    QUEST_STATUS_ACTIVE,
    QUEST_STATUS_COMPLETED,
    QUEST_STATUS_FAILED
} QuestStatus;

typedef enum {
    OBJECTIVE_KILL,
    OBJECTIVE_HARVEST,
    OBJECTIVE_TALK,
    OBJECTIVE_DISCOVER,
    OBJECTIVE_CRAFT,
    OBJECTIVE_WAIT
} ObjectiveType;

typedef struct {
    ObjectiveType type;
    void *target; // e.g., enemy type, NPC, location
    int required_count;
    int current_count;
    bool completed;
    char description[256];
} QuestObjective;

typedef struct {
    char id[64];
    char name[128];
    char description[512];
    QuestStatus status;

    QuestObjective *objectives;
    int objective_count;

    // Rewards
    int soul_reward;
    int corruption_change;
    char *unlock_reward; // Spell, skill, etc.

    // Conditions
    int required_corruption_min;
    int required_corruption_max;
    char *required_quest; // Must complete this first

    // Callbacks
    void (*on_start)(struct Quest *quest);
    void (*on_complete)(struct Quest *quest);
    void (*on_fail)(struct Quest *quest);
} Quest;

// Quest manager tracks all quests
typedef struct {
    Quest *quests;
    int quest_count;

    Quest *active_quests[32]; // Max active quests
    int active_count;
} QuestManager;

// Update quest progress (called by event handlers)
void quest_update_objective(Quest *quest, ObjectiveType type, void *target) {
    for (int i = 0; i < quest->objective_count; i++) {
        QuestObjective *obj = &quest->objectives[i];
        if (obj->type == type && obj->target == target) {
            obj->current_count++;
            if (obj->current_count >= obj->required_count) {
                obj->completed = true;
            }
        }
    }

    // Check if all objectives complete
    bool all_complete = true;
    for (int i = 0; i < quest->objective_count; i++) {
        if (!quest->objectives[i].completed) {
            all_complete = false;
            break;
        }
    }

    if (all_complete) {
        quest_complete(quest);
    }
}
```

---

### 3.5 Dialogue System Implementation

**Dialogue Tree Structure**:

```c
typedef enum {
    NODE_TEXT,           // Display text
    NODE_CHOICE,         // Present choices
    NODE_CONDITION,      // Branch based on condition
    NODE_ACTION,         // Execute action (give item, change relationship, etc.)
    NODE_END             // End dialogue
} DialogueNodeType;

typedef struct DialogueNode {
    DialogueNodeType type;
    char speaker[64];    // "Seraphine", "Aldric", etc.
    char text[512];

    // For choice nodes
    struct DialogueChoice *choices;
    int choice_count;

    // For condition nodes
    bool (*condition)(GameState *state);
    struct DialogueNode *true_branch;
    struct DialogueNode *false_branch;

    // For action nodes
    void (*action)(GameState *state);
    struct DialogueNode *next;

    // General next node
    struct DialogueNode *next_node;
} DialogueNode;

typedef struct {
    char text[128];       // Choice text
    DialogueNode *next;   // Where this leads

    // Requirements
    int min_relationship;
    int min_corruption;
    char *required_flag;
} DialogueChoice;

// Dialogue tree
typedef struct {
    char id[64];
    char name[128];
    DialogueNode *root;

    // Context
    char npc_id[64];
    char location[64];
} DialogueTree;

// Dialogue manager
typedef struct {
    DialogueTree *current_tree;
    DialogueNode *current_node;
    int selected_choice;
} DialogueManager;
```

**Dialogue Command Integration**:

```c
// Command: talk <npc> [--topic <topic>]
void cmd_talk_execute(ParsedCommand *cmd, GameState *state) {
    const char *npc_id = hashmap_get(cmd->args, "npc");
    const char *topic = hashmap_get(cmd->args, "topic");

    // Find NPC
    NPC *npc = world_find_npc(state->world, npc_id);
    if (!npc) {
        ui_print_error("NPC not found: %s", npc_id);
        return;
    }

    // Check if NPC is available for dialogue
    if (!npc_can_talk(npc, state)) {
        ui_print_error("%s is not available right now.", npc->name);
        return;
    }

    // Load dialogue tree
    DialogueTree *tree;
    if (topic) {
        tree = dialogue_load_topic(npc->id, topic);
    } else {
        tree = dialogue_load_default(npc->id, state);
    }

    if (!tree) {
        ui_print_error("No dialogue available.");
        return;
    }

    // Enter dialogue state
    state->data.dialogue.tree = tree;
    state->data.dialogue.current_node = tree->root;
    game_state_transition(state, STATE_DIALOGUE);
}
```

---

### 3.6 Combat System Mechanics

**Turn Structure**:

```c
typedef enum {
    COMBAT_PHASE_INIT,
    COMBAT_PHASE_PLAYER_TURN,
    COMBAT_PHASE_ENEMY_TURN,
    COMBAT_PHASE_RESOLUTION,
    COMBAT_PHASE_END
} CombatPhase;

typedef struct {
    char id[64];
    char name[128];
    int health;
    int max_health;
    int attack;
    int defense;
    int initiative;
    bool is_player_controlled;

    // For enemies/minions
    void (*ai_func)(struct Combatant *self, CombatState *combat);
} Combatant;

typedef struct {
    CombatPhase phase;
    int turn_number;

    Combatant *player_forces[32];
    int player_force_count;

    Combatant *enemy_forces[32];
    int enemy_force_count;

    Combatant *turn_order[64];
    int turn_order_count;
    int current_turn_index;

    // Combat log
    char log_messages[100][256];
    int log_count;
} CombatState;

// Combat flow
void combat_update(GameState *state, float delta_time) {
    CombatState *combat = &state->data.combat;

    switch (combat->phase) {
        case COMBAT_PHASE_INIT:
            combat_initialize(combat);
            combat->phase = COMBAT_PHASE_PLAYER_TURN;
            break;

        case COMBAT_PHASE_PLAYER_TURN:
            // Wait for player command
            // Player types commands like: "cast deathbolt --target enemy_1"
            // Command is parsed and executed
            // When player ends turn, move to enemy turn
            break;

        case COMBAT_PHASE_ENEMY_TURN:
            // AI for each enemy
            Combatant *current = combat->turn_order[combat->current_turn_index];
            if (current->ai_func) {
                current->ai_func(current, combat);
            }

            combat->current_turn_index++;
            if (combat->current_turn_index >= combat->turn_order_count) {
                combat->phase = COMBAT_PHASE_RESOLUTION;
            }
            break;

        case COMBAT_PHASE_RESOLUTION:
            // Apply damage, status effects, etc.
            // Check win/loss conditions
            if (combat_check_victory(combat)) {
                combat->phase = COMBAT_PHASE_END;
            } else {
                combat->turn_number++;
                combat->phase = COMBAT_PHASE_PLAYER_TURN;
            }
            break;

        case COMBAT_PHASE_END:
            combat_end(state, combat);
            game_state_transition(state, STATE_GAMEPLAY);
            break;
    }
}
```

**Typing Speed Mechanic**:

```c
typedef struct {
    uint64_t command_start_time;
    uint64_t command_end_time;
    float typing_speed; // Characters per second
    int typo_count;
} TypingMetrics;

// When player submits command in combat
void combat_execute_command(CombatState *combat, const char *command,
                            TypingMetrics *metrics) {
    // Calculate typing time
    float typing_time = (metrics->command_end_time - metrics->command_start_time) / 1000.0f;

    // Calculate initiative bonus
    // Faster typing = higher initiative = more actions
    float speed_bonus = 1.0f;
    if (typing_time < 2.0f) {
        speed_bonus = 1.5f; // 50% bonus for typing under 2 seconds
    } else if (typing_time < 5.0f) {
        speed_bonus = 1.0f; // Normal
    } else {
        speed_bonus = 0.5f; // Penalty for slow typing
    }

    // Typo penalty
    float typo_penalty = 1.0f - (metrics->typo_count * 0.1f);
    typo_penalty = fmaxf(typo_penalty, 0.3f); // Max 70% penalty

    // Apply modifiers to spell/attack effectiveness
    float effectiveness = speed_bonus * typo_penalty;

    // Execute command with modified effectiveness
    // ...
}
```

---

### 3.7 Relationship/Reputation Tracking

```c
typedef struct {
    char npc_id[64];
    int trust;           // 0-100
    int respect;         // 0-100
    int fear;            // 0-100

    // Relationship state
    enum {
        REL_HOSTILE,
        REL_UNFRIENDLY,
        REL_NEUTRAL,
        REL_FRIENDLY,
        REL_ALLIED,
        REL_LOYAL
    } state;

    // Event history
    char *significant_events[32];
    int event_count;

    // Dialogue unlocks
    bool unlocked_personal_dialogue;
    bool unlocked_backstory;
    bool unlocked_romance; // If applicable
} Relationship;

typedef struct {
    Relationship *relationships;
    int relationship_count;

    // Faction reputation
    int church_reputation;        // -100 to 100
    int necromancer_reputation;   // -100 to 100
    int civilian_reputation;      // -100 to 100
} RelationshipManager;

// Modify relationship
void relationship_modify(RelationshipManager *mgr, const char *npc_id,
                        int trust_delta, int respect_delta, int fear_delta,
                        const char *reason) {
    Relationship *rel = relationship_find(mgr, npc_id);
    if (!rel) return;

    rel->trust = CLAMP(rel->trust + trust_delta, 0, 100);
    rel->respect = CLAMP(rel->respect + respect_delta, 0, 100);
    rel->fear = CLAMP(rel->fear + fear_delta, 0, 100);

    // Record event
    if (rel->event_count < 32) {
        rel->significant_events[rel->event_count++] = strdup(reason);
    }

    // Update state
    relationship_update_state(rel);

    // Trigger event for quest/dialogue updates
    Event event = {
        .type = EVENT_RELATIONSHIP_CHANGED,
        .data = rel
    };
    event_bus_publish(get_global_event_bus(), &event);

    // Log for player
    ui_print_notification("Relationship with %s changed: %s",
                         npc_id, reason);
}
```

---

## 4. Data Management

### 4.1 File Formats for Game Data

**Option 1: JSON** (Recommended for development)

Pros:
- Human-readable
- Easy to edit
- Good tooling support
- Standard format

Cons:
- Slower parsing
- Larger file sizes
- No schema enforcement (without extra work)

**Option 2: Custom Binary Format**

Pros:
- Fast loading
- Compact
- Can include schema versioning

Cons:
- Harder to edit
- Requires custom tools
- Debugging is harder

**Recommendation**: Use JSON for development, with optional binary compilation for release builds.

### 4.2 Data File Structure

**NPC Definition** (`/data/npcs/seraphine.json`):
```json
{
  "id": "seraphine",
  "name": "Seraphine",
  "title": "The Bound",
  "description": "A ghostly figure bound to the Shell itself",
  "type": "guide",
  "initial_relationship": {
    "trust": 50,
    "respect": 30,
    "fear": 0
  },
  "dialogue_trees": [
    "seraphine_intro",
    "seraphine_corruption_warning",
    "seraphine_backstory",
    "seraphine_endgame"
  ],
  "quests": [
    "quest_seraphine_freedom"
  ],
  "ascii_portrait": "assets/portraits/seraphine.txt",
  "voice_style": {
    "tone": "melancholic",
    "formality": "high",
    "emotion_range": ["sad", "resigned", "hopeful"]
  }
}
```

**Quest Definition** (`/data/quests/fathers_farewell.json`):
```json
{
  "id": "quest_fathers_farewell",
  "name": "A Father's Farewell",
  "description": "Help Garrett see his son one last time",
  "type": "side_quest",
  "act": 2,
  "objectives": [
    {
      "type": "talk",
      "target": "garrett_skeleton",
      "description": "Speak with Garrett"
    },
    {
      "type": "discover",
      "target": "location_thornhaven",
      "description": "Locate Thomas in Thornhaven"
    },
    {
      "type": "choice",
      "description": "Decide how to reunite father and son",
      "choices": [
        {
          "id": "peaceful_meeting",
          "text": "Arrange peaceful meeting",
          "requirements": {"aldric_relationship": 60},
          "corruption_change": 0
        },
        {
          "id": "secret_meeting",
          "text": "Sneak Garrett into garrison",
          "corruption_change": 5
        },
        {
          "id": "possess_thomas",
          "text": "Possess Thomas to feel Garrett's presence",
          "corruption_change": 15
        }
      ]
    }
  ],
  "rewards": {
    "souls": 100,
    "aldric_relationship": 20,
    "unlocks": ["garrett_legendary_minion"]
  },
  "failure_conditions": [
    {"condition": "garrett_destroyed", "message": "Garrett was destroyed"},
    {"condition": "thomas_killed", "message": "Thomas died"}
  ]
}
```

**Location Definition** (`/data/locations/blackspire_tower.json`):
```json
{
  "id": "location_blackspire_tower",
  "name": "Blackspire Tower",
  "type": "player_base",
  "description": "Your dark sanctum, rising from the ruins",
  "ascii_art": "assets/locations/blackspire_tower.txt",
  "resources": {
    "corpse_supply": 12,
    "soul_income_per_day": 45,
    "minion_capacity": 50
  },
  "connected_locations": [
    "graveyard_blackwood",
    "village_millbrook",
    "forest_darkweald"
  ],
  "upgrades": [
    {
      "id": "tower_dark_sanctum",
      "name": "Dark Sanctum",
      "description": "Increases corruption resistance",
      "cost": 500,
      "effect": {"corruption_rate": -0.1}
    },
    {
      "id": "tower_soul_forge",
      "name": "Soul Forge",
      "description": "Allows soul crafting",
      "cost": 1000,
      "effect": {"unlocks_command": "soulforge"}
    }
  ],
  "encounters": [
    {
      "type": "church_raid",
      "probability": 0.1,
      "condition": "church_reputation < -50"
    }
  ]
}
```

**Spell Definition** (`/data/spells/deathbolt.json`):
```json
{
  "id": "spell_deathbolt",
  "name": "Deathbolt",
  "command": "cast deathbolt",
  "description": "Hurl necrotic energy at a target",
  "type": "combat",
  "cost": {
    "souls": 20,
    "type": "common"
  },
  "effects": [
    {
      "type": "damage",
      "amount": 50,
      "element": "necrotic"
    }
  ],
  "requirements": {
    "unlocked": true,
    "min_corruption": 0
  },
  "typing_difficulty": "easy",
  "command_template": "cast deathbolt --target <enemy_id> [--power high|medium|low]"
}
```

### 4.3 Asset Loader Implementation

```c
typedef struct {
    HashMap *npcs;
    HashMap *quests;
    HashMap *locations;
    HashMap *spells;
    HashMap *dialogues;
} AssetDatabase;

// Load all game data
AssetDatabase* asset_database_load(const char *data_dir) {
    AssetDatabase *db = malloc(sizeof(AssetDatabase));
    db->npcs = hashmap_create();
    db->quests = hashmap_create();
    db->locations = hashmap_create();
    db->spells = hashmap_create();
    db->dialogues = hashmap_create();

    // Load NPCs
    char npc_dir[256];
    snprintf(npc_dir, sizeof(npc_dir), "%s/npcs", data_dir);
    load_directory(npc_dir, "*.json", db->npcs, parse_npc_json);

    // Load quests
    char quest_dir[256];
    snprintf(quest_dir, sizeof(quest_dir), "%s/quests", data_dir);
    load_directory(quest_dir, "*.json", db->quests, parse_quest_json);

    // ... load other assets

    return db;
}

// Helper to load all files in directory
void load_directory(const char *dir, const char *pattern,
                   HashMap *dest, void* (*parser)(const char*)) {
    // Platform-specific directory traversal
    // For each matching file:
    //   1. Read file contents
    //   2. Parse JSON
    //   3. Add to hashmap
}
```

### 4.4 Configuration and Modding Support

**Configuration File** (`~/.config/necromancers_shell/config.json`):
```json
{
  "display": {
    "color_mode": "256color",
    "colorblind_mode": false,
    "text_size": "medium",
    "show_timestamps": true
  },
  "gameplay": {
    "typing_assistance": "medium",
    "autosave_frequency": 300,
    "difficulty": "normal",
    "permadeath": false
  },
  "accessibility": {
    "screen_reader_mode": false,
    "slow_mode": false,
    "confirmation_prompts": true
  },
  "modding": {
    "enabled": true,
    "mod_directories": [
      "~/.local/share/necromancers_shell/mods"
    ]
  }
}
```

**Mod Support**:
- Mods can override data files (NPCs, quests, spells)
- Mods can add new content
- Load order determined by manifest file
- Mod manifest includes compatibility version

**Mod Structure**:
```
my_custom_mod/
├── manifest.json
├── data/
│   ├── npcs/
│   ├── quests/
│   └── spells/
└── assets/
    └── ascii_art/
```

---

## 5. Core Systems Detail

### 5.1 Terminal Interface Implementation

**Library Choice**: ncurses with PDCurses fallback for Windows

**Abstraction Layer** (`/src/terminal/ncurses_wrapper.h`):
```c
// Platform-agnostic terminal interface
typedef struct Terminal Terminal;

Terminal* terminal_init(void);
void terminal_shutdown(Terminal *term);

void terminal_clear(Terminal *term);
void terminal_refresh(Terminal *term);
void terminal_move_cursor(Terminal *term, int x, int y);
void terminal_print(Terminal *term, int x, int y, const char *text);
void terminal_print_colored(Terminal *term, int x, int y,
                           const char *text, Color color);

int terminal_get_width(Terminal *term);
int terminal_get_height(Terminal *term);

char terminal_get_char(Terminal *term, bool blocking);
```

**Implementation**:
```c
// Linux/Mac implementation using ncurses
#ifdef USE_NCURSES

#include <ncurses.h>

struct Terminal {
    WINDOW *win;
    int width;
    int height;
    bool initialized;
};

Terminal* terminal_init(void) {
    Terminal *term = malloc(sizeof(Terminal));

    // Initialize ncurses
    term->win = initscr();
    if (!term->win) {
        free(term);
        return NULL;
    }

    cbreak();              // Disable line buffering
    noecho();              // Don't echo input
    keypad(stdscr, TRUE);  // Enable function keys
    nodelay(stdscr, TRUE); // Non-blocking input

    // Enable colors
    if (has_colors()) {
        start_color();
        use_default_colors();

        // Initialize color pairs
        init_color_pairs();
    }

    getmaxyx(stdscr, term->height, term->width);
    term->initialized = true;

    return term;
}

void terminal_print_colored(Terminal *term, int x, int y,
                           const char *text, Color color) {
    attron(COLOR_PAIR(color));
    mvprintw(y, x, "%s", text);
    attroff(COLOR_PAIR(color));
}

#endif // USE_NCURSES

// Windows implementation using PDCurses
#ifdef USE_PDCURSES
// Similar implementation but with PDCurses API
#endif
```

### 5.2 Color and ASCII Art Rendering

**Color System**:
```c
typedef enum {
    COLOR_DEFAULT,
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    // Game-specific colors
    COLOR_SOUL_PURPLE,
    COLOR_CORRUPTION_RED,
    COLOR_HOLY_GOLD,
    COLOR_DEATH_GREY,
    COLOR_MAGIC_BLUE,
    // ...
} Color;

typedef struct {
    Color foreground;
    Color background;
    bool bold;
    bool underline;
    bool blink; // Use sparingly!
} TextStyle;

void terminal_print_styled(Terminal *term, int x, int y,
                          const char *text, TextStyle style) {
    // Apply style
    if (style.bold) attron(A_BOLD);
    if (style.underline) attron(A_UNDERLINE);
    if (style.blink) attron(A_BLINK);

    // Apply colors
    attron(COLOR_PAIR(get_color_pair(style.foreground, style.background)));

    // Print
    mvprintw(y, x, "%s", text);

    // Reset
    attroff(A_BOLD | A_UNDERLINE | A_BLINK);
    attroff(COLOR_PAIR(get_color_pair(style.foreground, style.background)));
}
```

**ASCII Art Rendering**:
```c
typedef struct {
    char **lines;
    int line_count;
    int width;
    int height;
} AsciiArt;

AsciiArt* ascii_art_load(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return NULL;

    AsciiArt *art = malloc(sizeof(AsciiArt));
    art->lines = NULL;
    art->line_count = 0;
    art->width = 0;
    art->height = 0;

    // Read file line by line
    char buffer[256];
    int capacity = 10;
    art->lines = malloc(sizeof(char*) * capacity);

    while (fgets(buffer, sizeof(buffer), file)) {
        // Remove newline
        buffer[strcspn(buffer, "\n")] = 0;

        // Resize if needed
        if (art->line_count >= capacity) {
            capacity *= 2;
            art->lines = realloc(art->lines, sizeof(char*) * capacity);
        }

        // Store line
        art->lines[art->line_count] = strdup(buffer);

        // Update dimensions
        int len = strlen(buffer);
        if (len > art->width) art->width = len;

        art->line_count++;
    }

    art->height = art->line_count;

    fclose(file);
    return art;
}

void ascii_art_render(Terminal *term, int x, int y, AsciiArt *art) {
    for (int i = 0; i < art->line_count; i++) {
        terminal_print(term, x, y + i, art->lines[i]);
    }
}
```

### 5.3 Command Autocomplete and History

**History Implementation**:
```c
typedef struct {
    char **commands;
    int count;
    int capacity;
    int current_index; // For navigation
    char *history_file;
} CommandHistory;

CommandHistory* history_create(const char *filepath) {
    CommandHistory *hist = malloc(sizeof(CommandHistory));
    hist->capacity = 100;
    hist->count = 0;
    hist->current_index = 0;
    hist->commands = malloc(sizeof(char*) * hist->capacity);
    hist->history_file = strdup(filepath);

    // Load from file
    history_load(hist, filepath);

    return hist;
}

void history_add(CommandHistory *hist, const char *command) {
    // Don't add duplicates of the last command
    if (hist->count > 0 &&
        strcmp(hist->commands[hist->count - 1], command) == 0) {
        return;
    }

    // Resize if needed
    if (hist->count >= hist->capacity) {
        hist->capacity *= 2;
        hist->commands = realloc(hist->commands,
                                sizeof(char*) * hist->capacity);
    }

    hist->commands[hist->count++] = strdup(command);
    hist->current_index = hist->count;

    // Save to file
    history_save(hist, hist->history_file);
}

const char* history_prev(CommandHistory *hist) {
    if (hist->current_index > 0) {
        hist->current_index--;
        return hist->commands[hist->current_index];
    }
    return NULL;
}

const char* history_next(CommandHistory *hist) {
    if (hist->current_index < hist->count - 1) {
        hist->current_index++;
        return hist->commands[hist->current_index];
    } else {
        hist->current_index = hist->count;
        return ""; // Empty for new command
    }
}
```

**Autocomplete Implementation**:
```c
// Trie for efficient prefix matching
typedef struct TrieNode {
    struct TrieNode *children[128]; // ASCII
    bool is_end_of_word;
    char *completion; // Full command if is_end_of_word
} TrieNode;

typedef struct {
    TrieNode *root;
    char **suggestions;
    int suggestion_count;
} Autocompleter;

Autocompleter* autocompleter_create(void) {
    Autocompleter *ac = malloc(sizeof(Autocompleter));
    ac->root = trie_node_create();
    ac->suggestions = NULL;
    ac->suggestion_count = 0;
    return ac;
}

void autocompleter_add_command(Autocompleter *ac, const char *command) {
    TrieNode *node = ac->root;
    for (int i = 0; command[i]; i++) {
        int index = (int)command[i];
        if (!node->children[index]) {
            node->children[index] = trie_node_create();
        }
        node = node->children[index];
    }
    node->is_end_of_word = true;
    node->completion = strdup(command);
}

char** autocompleter_suggest(Autocompleter *ac, const char *prefix,
                            int *out_count) {
    // Find prefix in trie
    TrieNode *node = ac->root;
    for (int i = 0; prefix[i]; i++) {
        int index = (int)prefix[i];
        if (!node->children[index]) {
            *out_count = 0;
            return NULL;
        }
        node = node->children[index];
    }

    // Collect all completions from this node
    char **suggestions = malloc(sizeof(char*) * 100);
    int count = 0;
    trie_collect_completions(node, prefix, suggestions, &count);

    *out_count = count;
    return suggestions;
}
```

### 5.4 Error Handling and Feedback

**Error Types**:
```c
typedef enum {
    ERROR_NONE,
    ERROR_SYNTAX,           // Invalid command syntax
    ERROR_UNKNOWN_COMMAND,  // Command not found
    ERROR_INVALID_ARGUMENT, // Argument type/value wrong
    ERROR_MISSING_ARGUMENT, // Required argument missing
    ERROR_PERMISSION,       // Command not unlocked yet
    ERROR_RESOURCE,         // Not enough souls/resources
    ERROR_GAME_STATE,       // Can't execute in current state
    ERROR_INTERNAL          // Internal error
} ErrorType;

typedef struct {
    ErrorType type;
    char message[512];
    char suggestion[256]; // Helpful suggestion
} CommandError;

void ui_display_error(CommandError *error) {
    // Display with appropriate color
    TextStyle style = {
        .foreground = COLOR_RED,
        .background = COLOR_DEFAULT,
        .bold = true
    };

    terminal_print_styled(g_terminal, 0, g_terminal->height - 2,
                         error->message, style);

    // Display suggestion if available
    if (error->suggestion[0]) {
        style.foreground = COLOR_YELLOW;
        style.bold = false;
        terminal_print_styled(g_terminal, 0, g_terminal->height - 1,
                             error->suggestion, style);
    }
}

// Helpful error messages
CommandError make_unknown_command_error(const char *command) {
    CommandError err;
    err.type = ERROR_UNKNOWN_COMMAND;
    snprintf(err.message, sizeof(err.message),
             "Unknown command: %s", command);

    // Find similar commands (Levenshtein distance)
    char *similar = find_similar_command(command);
    if (similar) {
        snprintf(err.suggestion, sizeof(err.suggestion),
                 "Did you mean: %s?", similar);
    } else {
        snprintf(err.suggestion, sizeof(err.suggestion),
                 "Type 'help' for available commands");
    }

    return err;
}
```

### 5.5 Tutorial System Integration

**Tutorial State**:
```c
typedef enum {
    TUTORIAL_NOT_STARTED,
    TUTORIAL_BASIC_COMMANDS,
    TUTORIAL_RAISE_UNDEAD,
    TUTORIAL_HARVEST_SOULS,
    TUTORIAL_COMBAT,
    TUTORIAL_DIALOGUE,
    TUTORIAL_COMPLETED
} TutorialStage;

typedef struct {
    TutorialStage stage;
    bool enabled;
    bool show_hints;

    // Tracking
    bool used_help;
    bool used_status;
    bool used_raise;
    bool used_harvest;
    int commands_executed;
} TutorialState;

// Tutorial checks command execution
void tutorial_on_command_executed(TutorialState *tutorial,
                                 const char *command) {
    if (!tutorial->enabled) return;

    tutorial->commands_executed++;

    switch (tutorial->stage) {
        case TUTORIAL_BASIC_COMMANDS:
            if (strcmp(command, "help") == 0) {
                tutorial->used_help = true;
            }
            if (strcmp(command, "status") == 0) {
                tutorial->used_status = true;
            }

            // Advance when both used
            if (tutorial->used_help && tutorial->used_status) {
                tutorial_advance(tutorial, TUTORIAL_RAISE_UNDEAD);
            }
            break;

        case TUTORIAL_RAISE_UNDEAD:
            if (strncmp(command, "raise", 5) == 0) {
                tutorial->used_raise = true;
                tutorial_advance(tutorial, TUTORIAL_HARVEST_SOULS);
            }
            break;

        // ... other stages
    }
}

void tutorial_advance(TutorialState *tutorial, TutorialStage new_stage) {
    tutorial->stage = new_stage;

    // Display tutorial message
    switch (new_stage) {
        case TUTORIAL_RAISE_UNDEAD:
            ui_display_tutorial_message(
                "Now try raising your first undead minion.\n"
                "Use: raise zombie --from graveyard_blackwood"
            );
            break;

        case TUTORIAL_HARVEST_SOULS:
            ui_display_tutorial_message(
                "Excellent! Now you need soul energy to power your magic.\n"
                "Use: harvest --from battlefield"
            );
            break;

        // ... other messages
    }
}
```

---

## 6. Memory and Performance

### 6.1 Memory Management Strategy

**Custom Allocator**:
```c
// Memory pool for frequently allocated objects
typedef struct MemoryPool {
    void *memory;
    size_t element_size;
    size_t capacity;
    size_t used;
    bool *allocated; // Bitmap
} MemoryPool;

MemoryPool* memory_pool_create(size_t element_size, size_t capacity) {
    MemoryPool *pool = malloc(sizeof(MemoryPool));
    pool->element_size = element_size;
    pool->capacity = capacity;
    pool->used = 0;
    pool->memory = malloc(element_size * capacity);
    pool->allocated = calloc(capacity, sizeof(bool));
    return pool;
}

void* memory_pool_alloc(MemoryPool *pool) {
    // Find first free slot
    for (size_t i = 0; i < pool->capacity; i++) {
        if (!pool->allocated[i]) {
            pool->allocated[i] = true;
            pool->used++;
            return (char*)pool->memory + (i * pool->element_size);
        }
    }

    // Pool full - fallback to malloc
    LOG_WARNING("Memory pool exhausted, using malloc");
    return malloc(pool->element_size);
}

void memory_pool_free(MemoryPool *pool, void *ptr) {
    // Check if ptr is in pool
    if (ptr >= pool->memory &&
        ptr < (char*)pool->memory + (pool->capacity * pool->element_size)) {
        // Calculate index
        size_t index = ((char*)ptr - (char*)pool->memory) / pool->element_size;
        pool->allocated[index] = false;
        pool->used--;
    } else {
        // Was allocated with malloc
        free(ptr);
    }
}
```

**Memory Pools for Common Objects**:
- Soul objects: Pool of 1000
- Minion objects: Pool of 500
- Event objects: Pool of 200
- Command objects: Pool of 50

**Debug Memory Tracking**:
```c
#ifdef DEBUG_MEMORY

typedef struct MemoryAllocation {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    struct MemoryAllocation *next;
} MemoryAllocation;

static MemoryAllocation *g_allocations = NULL;
static size_t g_total_allocated = 0;

void* debug_malloc(size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    if (ptr) {
        MemoryAllocation *alloc = malloc(sizeof(MemoryAllocation));
        alloc->ptr = ptr;
        alloc->size = size;
        alloc->file = file;
        alloc->line = line;
        alloc->next = g_allocations;
        g_allocations = alloc;
        g_total_allocated += size;
    }
    return ptr;
}

void debug_free(void *ptr) {
    if (!ptr) return;

    MemoryAllocation *prev = NULL;
    MemoryAllocation *curr = g_allocations;

    while (curr) {
        if (curr->ptr == ptr) {
            if (prev) {
                prev->next = curr->next;
            } else {
                g_allocations = curr->next;
            }
            g_total_allocated -= curr->size;
            free(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    free(ptr);
}

void debug_memory_report(void) {
    printf("\n=== Memory Report ===\n");
    printf("Total allocated: %zu bytes\n", g_total_allocated);
    printf("Active allocations:\n");

    MemoryAllocation *curr = g_allocations;
    while (curr) {
        printf("  %p: %zu bytes (%s:%d)\n",
               curr->ptr, curr->size, curr->file, curr->line);
        curr = curr->next;
    }
}

#define malloc(size) debug_malloc(size, __FILE__, __LINE__)
#define free(ptr) debug_free(ptr)

#endif // DEBUG_MEMORY
```

### 6.2 Performance Considerations

**Hot Path Optimization**:

1. **Command Parsing**: Most frequent operation
   - Cache parsed command structure
   - Use hash table for command lookup (O(1) vs O(n) array search)
   - Minimize string allocations

2. **Game State Updates**: Called every frame
   - Only update dirty objects (dirty flag pattern)
   - Batch updates where possible
   - Avoid deep recursion

3. **Rendering**: Called 60 times per second
   - Only redraw changed areas (dirty rectangles)
   - Cache rendered ASCII art
   - Minimize ncurses calls

**Profiling Strategy**:
```bash
# Compile with profiling
gcc -pg -O2 src/*.c -o necromancers_shell

# Run game
./necromancers_shell

# Generate profile
gprof necromancers_shell gmon.out > profile.txt

# Analyze profile.txt to find hot paths
```

**Optimization Techniques**:
- String interning for repeated strings (command names, IDs)
- Object pooling for frequently created/destroyed objects
- Lazy loading of dialogue trees and quest data
- Spatial hashing for location-based queries
- Event batching to reduce processing overhead

### 6.3 Memory Budget

Target memory usage: < 100MB for typical gameplay

**Breakdown**:
- Core engine: ~10MB
- Game state (minions, souls, world): ~30MB
- Dialogue/narrative data: ~20MB
- Asset data (ASCII art, etc.): ~10MB
- Memory pools and buffers: ~20MB
- Headroom: ~10MB

**Large Object Handling**:
- Dialogue trees: Load on-demand, unload when not in use
- Location data: Keep only nearby locations in memory
- Soul memories: Store strings in compressed format

---

## 7. Testing Strategy

### 7.1 Unit Testing Approach

**Test Framework**: Simple custom framework or Unity (C testing framework)

**Example Test** (`/tests/test_command_parser.c`):
```c
#include "unity.h"
#include "../src/commands/parser.h"

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

void test_parse_simple_command(void) {
    CommandRegistry *registry = registry_create();
    registry_add_command(registry, &cmd_help);

    ParsedCommand *result = parse_command("help", registry);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("help", result->cmd->name);
    TEST_ASSERT_NULL(result->error_message);

    parsed_command_destroy(result);
    registry_destroy(registry);
}

void test_parse_command_with_flags(void) {
    CommandRegistry *registry = registry_create();
    registry_add_command(registry, &cmd_raise);

    ParsedCommand *result = parse_command(
        "raise zombie --from graveyard --count 5", registry);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("raise", result->cmd->name);
    TEST_ASSERT_EQUAL_STRING("graveyard",
                            hashmap_get(result->args, "from"));
    TEST_ASSERT_EQUAL_STRING("5",
                            hashmap_get(result->args, "count"));

    parsed_command_destroy(result);
    registry_destroy(registry);
}

void test_parse_unknown_command(void) {
    CommandRegistry *registry = registry_create();

    ParsedCommand *result = parse_command("foobar", registry);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_NOT_NULL(result->error_message);
    TEST_ASSERT_NULL(result->cmd);

    parsed_command_destroy(result);
    registry_destroy(registry);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_parse_simple_command);
    RUN_TEST(test_parse_command_with_flags);
    RUN_TEST(test_parse_unknown_command);

    return UNITY_END();
}
```

**Test Coverage Goals**:
- Command parser: 90%+
- Game logic (souls, minions, etc.): 80%+
- Save/load system: 95%+
- Event system: 85%+

**Running Tests**:
```bash
make test
# Runs all tests and reports coverage
```

### 7.2 Integration Testing

**Test Scenarios**:
1. **Full Gameplay Loop**: Start game → Execute commands → Save → Load → Continue
2. **Quest Completion**: Start quest → Complete objectives → Receive rewards
3. **Combat**: Enter combat → Execute combat commands → Win/lose → Exit combat
4. **Dialogue**: Start dialogue → Make choices → Affect relationships
5. **State Transitions**: Menu → Gameplay → Combat → Dialogue → Back

**Example Integration Test**:
```c
void test_full_quest_completion(void) {
    GameState *state = game_state_create();

    // Start quest
    quest_start(state, "quest_fathers_farewell");
    TEST_ASSERT_EQUAL(QUEST_STATUS_ACTIVE,
                     quest_get_status(state, "quest_fathers_farewell"));

    // Complete first objective (talk to Garrett)
    execute_command(state, "talk garrett");
    TEST_ASSERT_TRUE(quest_objective_completed(state,
                     "quest_fathers_farewell", 0));

    // Complete second objective (discover location)
    execute_command(state, "connect --to thornhaven");
    TEST_ASSERT_TRUE(quest_objective_completed(state,
                     "quest_fathers_farewell", 1));

    // Complete final objective (make choice)
    execute_command(state, "talk garrett --choice peaceful_meeting");

    // Quest should be completed
    TEST_ASSERT_EQUAL(QUEST_STATUS_COMPLETED,
                     quest_get_status(state, "quest_fathers_farewell"));

    // Relationship should have increased
    int aldric_rel = relationship_get_trust(state, "aldric");
    TEST_ASSERT_GREATER_THAN(60, aldric_rel);

    game_state_destroy(state);
}
```

### 7.3 Playtesting Methodology

**Alpha Testing** (Weeks 35-36):
- Internal testing by developers
- Focus on bugs, crashes, save/load issues
- Test all three ending paths
- Test edge cases

**Beta Testing** (Weeks 37-38):
- External testers (5-10 people)
- Full playthroughs
- Balance feedback
- Story pacing feedback
- UI/UX feedback

**Feedback Collection**:
- In-game feedback command: `feedback "message"`
- Crash reports automatically saved to logs
- Playtest survey after completion

**Metrics to Track**:
- Completion rate
- Average playtime
- Most used commands
- Least used commands
- Death locations/causes
- Ending distribution
- Relationship paths taken

---

## 8. Build and Deployment

### 8.1 Cross-Platform Considerations

**Platform Differences**:

| Feature | Linux | Windows | macOS |
|---------|-------|---------|-------|
| Terminal Library | ncurses | PDCurses | ncurses |
| File Paths | / | \ | / |
| Line Endings | \n | \r\n | \n |
| Home Directory | $HOME | %USERPROFILE% | $HOME |
| Color Support | Excellent | Good | Excellent |

**Abstraction Strategy**:
- Use wrapper functions for all platform-specific code
- Separate platform-specific implementations in different files
- Use preprocessor directives to select implementation

**Example** (`/src/utils/filesystem.h`):
```c
// Platform-agnostic filesystem API
const char* get_home_directory(void);
const char* get_config_directory(void);
const char* get_save_directory(void);
bool create_directory(const char *path);
bool file_exists(const char *path);
```

**Implementation** (`/src/utils/filesystem_unix.c`):
```c
#ifdef __unix__

#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

const char* get_home_directory(void) {
    return getenv("HOME");
}

const char* get_config_directory(void) {
    static char path[512];
    const char *home = get_home_directory();
    snprintf(path, sizeof(path), "%s/.config/necromancers_shell", home);
    return path;
}

bool create_directory(const char *path) {
    return mkdir(path, 0755) == 0;
}

#endif // __unix__
```

**Implementation** (`/src/utils/filesystem_windows.c`):
```c
#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>

const char* get_home_directory(void) {
    return getenv("USERPROFILE");
}

const char* get_config_directory(void) {
    static char path[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
    strcat(path, "\\NecromancersShell");
    return path;
}

bool create_directory(const char *path) {
    return CreateDirectory(path, NULL) != 0;
}

#endif // _WIN32
```

### 8.2 Build System

**Makefile**:
```makefile
# Project configuration
PROJECT = necromancers_shell
VERSION = 1.0.0

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -I./src
LDFLAGS = -lncurses -lm

# Directories
SRC_DIR = src
BUILD_DIR = build
DATA_DIR = data
ASSETS_DIR = assets

# Source files
SOURCES = $(shell find $(SRC_DIR) -name '*.c')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Platform detection
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
    PLATFORM = linux
    LDFLAGS += -lncurses
endif
ifeq ($(UNAME),Darwin)
    PLATFORM = macos
    LDFLAGS += -lncurses
endif
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    LDFLAGS += -lpdcurses
    PROJECT := $(PROJECT).exe
endif

# Build modes
DEBUG ?= 0
ifeq ($(DEBUG),1)
    CFLAGS += -g -O0 -DDEBUG -DDEBUG_MEMORY
    BUILD_DIR := $(BUILD_DIR)/debug
else
    CFLAGS += -O2 -DNDEBUG
    BUILD_DIR := $(BUILD_DIR)/release
endif

# Targets
.PHONY: all clean test install run

all: $(BUILD_DIR)/$(PROJECT)

$(BUILD_DIR)/$(PROJECT): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean complete"

test: DEBUG=1
test: all
	@$(MAKE) -C tests run

install: all
	@echo "Installing to /usr/local/bin..."
	@cp $(BUILD_DIR)/$(PROJECT) /usr/local/bin/
	@echo "Installing data to /usr/local/share/$(PROJECT)..."
	@mkdir -p /usr/local/share/$(PROJECT)
	@cp -r $(DATA_DIR) /usr/local/share/$(PROJECT)/
	@cp -r $(ASSETS_DIR) /usr/local/share/$(PROJECT)/
	@echo "Installation complete"

run: all
	@$(BUILD_DIR)/$(PROJECT)

# Development helpers
valgrind: DEBUG=1
valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes \
	         $(BUILD_DIR)/$(PROJECT)

profile: all
	$(BUILD_DIR)/$(PROJECT)
	gprof $(BUILD_DIR)/$(PROJECT) gmon.out > profile.txt
	@echo "Profile saved to profile.txt"
```

**CMake Alternative** (for better cross-platform support):
```cmake
cmake_minimum_required(VERSION 3.10)
project(necromancers_shell VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -Werror -g -O0 -DDEBUG)
else()
    add_compile_options(-Wall -Wextra -Werror -O2 -DNDEBUG)
endif()

# Find ncurses/PDCurses
if(WIN32)
    find_package(PDCurses REQUIRED)
    set(CURSES_LIBRARIES PDCurses::PDCurses)
else()
    find_package(Curses REQUIRED)
endif()

# Source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Executable
add_executable(necromancers_shell ${SOURCES})
target_include_directories(necromancers_shell PRIVATE src)
target_link_libraries(necromancers_shell ${CURSES_LIBRARIES} m)

# Install
install(TARGETS necromancers_shell DESTINATION bin)
install(DIRECTORY data DESTINATION share/necromancers_shell)
install(DIRECTORY assets DESTINATION share/necromancers_shell)

# Tests
enable_testing()
add_subdirectory(tests)
```

### 8.3 Distribution Approach

**Release Packages**:

1. **Linux**:
   - Tarball: `necromancers_shell-1.0.0-linux-x86_64.tar.gz`
   - Debian package: `necromancers_shell_1.0.0_amd64.deb`
   - RPM package: `necromancers_shell-1.0.0-1.x86_64.rpm`
   - Flatpak: `com.github.necromancers_shell.flatpak`
   - AppImage: `necromancers_shell-1.0.0-x86_64.AppImage`

2. **Windows**:
   - Installer: `necromancers_shell-1.0.0-setup.exe` (NSIS or Inno Setup)
   - Portable: `necromancers_shell-1.0.0-windows-x64.zip`

3. **macOS**:
   - DMG: `necromancers_shell-1.0.0.dmg`
   - Homebrew formula: `brew install necromancers-shell`

**Distribution Channels**:
- GitHub Releases (primary)
- itch.io (good for indie games)
- Steam (if budget allows)
- Package repositories (AUR, Homebrew, apt/yum)

**Release Checklist**:
- [ ] All tests passing
- [ ] No memory leaks (valgrind clean)
- [ ] Documentation complete
- [ ] Version numbers updated
- [ ] Changelog written
- [ ] Builds created for all platforms
- [ ] Installation tested on clean systems
- [ ] Save game compatibility verified

---

## 9. Development Timeline

### UPDATED Detailed Schedule (63 weeks)

**NOTE**: This timeline has been significantly expanded from the original 40-week plan to accommodate the full story.md narrative scope. The expanded content includes:
- 13 major NPCs (6 Regional Council + 7 Divine Architects)
- 14 major quest chains (7 Archon + 7 Morningstar trials)
- Consciousness decay system
- 70% corruption threshold mechanics
- Multi-path endings (expanded from 3 to 7)
- Extended time tracking (months/years)
- Post-game content

**Months 1-2 (Weeks 1-8)** - COMPLETE:
- Foundation setup (Phase 0)
- Command system MVP (Phase 1)
- Core game systems (Phase 2)
- Milestone: Can raise undead and harvest souls via terminal

**Months 3-4 (Weeks 9-16)** - COMPLETE:
- World building (Phase 3)
- Location system
- Death Network simulation
- Combat system foundation (Phase 4)
- Milestone: Can explore world and engage in basic combat

**Months 5-6 (Weeks 17-24)** - COMPLETE:
- Narrative engine (Phase 5)
- Dialogue system
- Quest system
- Relationship tracking
- NPC implementation (original 4 NPCs)
- Milestone: Tutorial and basic story playable

**Months 6-7 (Weeks 24-30)** - NEW PHASE 6:
- Story Foundation & Extended Systems
- Consciousness decay system
- 70% corruption threshold (10 tiers)
- Extended time tracking (months/years)
- Regional Council NPCs (6 characters)
- Divine Architect NPCs (7 gods)
- Thessara ghost system
- Null space locations
- Ashbrook event (Day 47)
- Multi-necromancer alliances
- Extended ending framework (7 endings)
- Milestone: Critical narrative infrastructure complete

**Months 8-9 (Weeks 31-37)** - NEW PHASE 7:
- The Archon Path
- 7 Archon Trials (Power, Wisdom, Morality, Technical, Resolve, Sacrifice, Leadership)
- Divine Council dialogue and judgment
- Network patching mechanics
- Split-routing system
- Fourth Purge countdown
- Archon transformation
- Reformation program
- Milestone: Archon path fully playable

**Months 10-11 (Weeks 38-45)** - NEW PHASE 8 (OPTIONAL):
- Hidden Paths (Wraith & Morningstar)
- Wraith fragmentation system (10,000 fragments)
- Multi-school magic (Necromancy, Divine, Arcane, Primal)
- Morningstar requirements (EXACTLY 50% corruption)
- Partial Morningstar trials (6 of 7)
- God combat system
- Special items & artifacts
- Extended endings implementation (all 7)
- Post-ending content (Jordan successor)
- Milestone: All ending paths available (Archon recommended, others aspirational)

**Months 11-12 (Weeks 46-50)** - PHASE 9:
- Story Integration (renumbered from Phase 6)
- Act I: The Awakening (Days 1-47)
- Act II: The Spiral (Days 48-162)
- Act III: The Choice (Days 163+)
- Dynamic events integration
- Event timeline system
- All story beats scheduled
- Milestone: Complete story from Day 1 to all endings

**Months 13-14 (Weeks 51-56)** - PHASE 10:
- Polish & Content (renumbered from Phase 7)
- All spell commands
- Intelligence commands
- Management commands
- ASCII art for all locations/NPCs
- Combat visualizations
- Tutorial system
- Accessibility features
- Milestone: Feature-complete with high polish

**Months 14-15 (Weeks 57-61)** - PHASE 11:
- Testing & Optimization (renumbered from Phase 8)
- Bug fixing (valgrind clean)
- Performance optimization
- Extensive playtesting
- Balance tuning (all 7 ending paths)
- Platform testing
- Milestone: Stable, balanced, optimized

**Month 16 (Weeks 62-63)** - PHASE 12:
- Release Preparation (renumbered from Phase 9)
- Documentation
- Build system
- Distribution
- Milestone: 1.0 release ready

### Resource Requirements

**Single Developer**:
- **63 weeks full-time (15.75 months / ~16 months)**
- Or **126 weeks part-time (31.5 months / ~2.6 years)**
- Original estimate: 40 weeks
- **Expansion: +23 weeks (+58% more work)**

**Small Team (3 people)**:
- Lead Developer: Core systems, Archon trials, network mechanics
- Content Developer: Story, 13 major NPCs, 14 quest chains, dialogue trees
- Artist: ASCII art for expanded cast, UI design, god manifestations
- Timeline: **10-14 months** (vs original 6-9 months)

**Timeline Breakdown by Scope**:
- **Core MVP (Phases 0-5)**: 24 weeks (original plan, COMPLETE)
- **Story Expansion (Phases 6-7)**: +14 weeks (critical additions)
- **Hidden Paths (Phase 8)**: +8 weeks (OPTIONAL - can be DLC)
- **Story Integration (Phase 9)**: +5 weeks (vs original 5 weeks)
- **Polish/Test/Release (Phases 10-12)**: +12 weeks (more content to test)

**Optional vs Required**:
- **Minimum viable story**: Skip Phase 8 = **55 weeks (~13.75 months)**
  - Archon path as primary story route
  - Basic Revenant/Lich Lord/Reaper endings
  - Saves ~8 weeks
- **Full story experience**: All phases = **63 weeks (~16 months)**
  - All 7 ending paths
  - Wraith and Morningstar content
  - Complete aspirational narrative

**Recommended Tools**:
- Version control: Git + GitHub
- Issue tracking: GitHub Issues or Trello
- Documentation: Markdown + GitHub Wiki
- Communication: Discord for team

---

## 10. Risk Mitigation

### Technical Risks

**Risk**: Terminal compatibility issues across platforms
**Mitigation**:
- Build abstraction layer early
- Test on all platforms regularly
- Have fallback rendering modes

**Risk**: Save file corruption
**Mitigation**:
- Implement checksums
- Keep backup saves
- Extensive testing of save/load
- Version compatibility handling

**Risk**: Performance issues with large game states
**Mitigation**:
- Profile early and often
- Use memory pools
- Lazy loading
- Set memory budgets

### Design Risks

**Risk**: Command syntax too complex
**Mitigation**:
- Extensive playtesting
- Good error messages
- Interactive tutorial
- Autocomplete helps

**Risk**: Story too long/short
**Mitigation**:
- Set target playtime (10-15 hours)
- Track playtester times
- Cut or add content as needed
- Make side content optional

**Risk**: Difficulty balance
**Mitigation**:
- Multiple difficulty settings
- Extensive playtesting
- Adjustable parameters in config
- Post-release patches

### Scope Risks

**Risk**: Feature creep
**Mitigation**:
- Clear MVP definition
- Prioritize features (must-have vs nice-to-have)
- Strict phase gates
- Save features for updates

**Risk**: Content creation too slow
**Mitigation**:
- Use data-driven design
- Create content tools early
- Reuse dialogue/quest patterns
- Focus on quality over quantity

---

## 11. Post-Release Support

### Update Plan

**Patch 1.1** (1 month post-release):
- Bug fixes
- Balance adjustments
- QoL improvements

**Patch 1.2** (3 months post-release):
- New spells
- New minion types
- Additional side quests
- Community-requested features

**DLC/Expansion** (6-12 months post-release):
- New story arc (Act IV)
- New NPCs
- New locations
- New endings

### Community Engagement

- Discord server for players
- Regular dev blog updates
- Modding support and documentation
- Community spotlight (mods, stories)
- Speedrun leaderboards

### Metrics Collection

**Anonymous Telemetry** (opt-in):
- Completion rates
- Popular commands
- Ending distribution
- Average playtime
- Crash reports

**Use Cases**:
- Identify difficulty spikes
- Find unused features
- Guide future content
- Prioritize bug fixes

---

## 12. Recommended C Libraries

### Core Libraries

**Terminal**:
- `ncurses` (Linux/Mac) - Standard, well-documented
- `PDCurses` (Windows) - ncurses compatibility
- Alternative: `termbox` - Simpler, more portable

**Data Parsing**:
- `json-c` - Mature JSON library
- Alternative: `cJSON` - Single-file, easy integration
- Alternative: Custom parser (more control, more work)

**String Handling**:
- `libsds` (Simple Dynamic Strings) - Better than char*
- Alternative: DIY string library (educational)

**Utilities**:
- `uthash` - Hash table macros
- `utlist` - Linked list macros
- `utstring` - Dynamic string macros

**Testing**:
- `Unity` - Lightweight C testing framework
- Alternative: `Check` - More features
- Alternative: Custom test harness

**Audio** (optional):
- `SDL2_mixer` - If adding sound effects
- `miniaudio` - Single-file audio library

### Library Integration Example

```c
// Using json-c
#include <json-c/json.h>

NPC* load_npc_from_json(const char *filepath) {
    // Read file
    json_object *root = json_object_from_file(filepath);
    if (!root) return NULL;

    NPC *npc = malloc(sizeof(NPC));

    // Parse fields
    json_object *id_obj, *name_obj;
    json_object_object_get_ex(root, "id", &id_obj);
    json_object_object_get_ex(root, "name", &name_obj);

    strncpy(npc->id, json_object_get_string(id_obj), 64);
    strncpy(npc->name, json_object_get_string(name_obj), 128);

    // ... parse other fields

    json_object_put(root); // Free
    return npc;
}
```

---

## 13. Code Style and Standards

### Style Guide

**Naming Conventions**:
- Types: `PascalCase` (e.g., `GameState`, `CommandParser`)
- Functions: `snake_case` (e.g., `parse_command`, `game_state_update`)
- Variables: `snake_case` (e.g., `player_health`, `soul_count`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_MINIONS`, `DEFAULT_PORT`)
- Globals: `g_` prefix (e.g., `g_game_state`, `g_terminal`)

**File Organization**:
- Header guards: `#ifndef MODULENAME_H`
- Include order: System headers, library headers, local headers
- One module per file pair (.c/.h)

**Code Quality**:
- Max function length: 100 lines
- Max line length: 100 characters
- Always check return values
- Always free allocated memory
- Use `const` wherever possible
- Minimize globals

**Example**:
```c
// Good
Soul* soul_create(SoulType type, int quality) {
    Soul *soul = malloc(sizeof(Soul));
    if (!soul) {
        LOG_ERROR("Failed to allocate soul");
        return NULL;
    }

    soul->type = type;
    soul->quality = quality;
    soul->memories[0] = '\0';

    return soul;
}

// Bad
Soul* create_soul(int t, int q) {
    Soul *s = malloc(sizeof(Soul)); // No check
    s->type = t;
    s->quality = q;
    return s;
}
```

---

## 14. Documentation Requirements

### Code Documentation

**Header Files**:
```c
/**
 * @file soul.h
 * @brief Soul management system
 *
 * Provides data structures and functions for managing souls,
 * which are the primary resource in the game.
 */

/**
 * @brief Create a new soul
 *
 * Allocates and initializes a new soul object with the
 * specified type and quality.
 *
 * @param type The type of soul (common, warrior, innocent, etc.)
 * @param quality Quality rating from 1-10
 * @return Pointer to new soul, or NULL on failure
 */
Soul* soul_create(SoulType type, int quality);

/**
 * @brief Destroy a soul and free memory
 *
 * @param soul Soul to destroy (can be NULL)
 */
void soul_destroy(Soul *soul);
```

**User Documentation**:
- README.md: Quick start guide
- INSTALL.md: Installation instructions per platform
- GAMEPLAY.md: Command reference and mechanics
- MODDING.md: How to create mods
- STORY.md: Story summary (no spoilers)

### API Documentation

Generate with Doxygen:
```bash
doxygen Doxyfile
# Produces HTML documentation in docs/html
```

---

## 15. Conclusion

### Success Criteria

The implementation is successful if:
1. Game runs without crashes on Linux, Windows, macOS
2. Complete story playable from start to all 7 endings
3. No memory leaks (valgrind clean)
4. Save/load works reliably
5. Performance: 60 FPS sustained, < 150MB memory (increased for extended systems)
6. Player reviews indicate engaging story and satisfying gameplay
7. Average playtime 20-30 hours for first playthrough (expanded narrative)
8. At least 3 ending paths fully playable (Archon, Lich Lord, Revenant minimum)
9. All 7 Archon Trials functional and balanced
10. Regional Council NPCs and Divine Architects fully implemented

### Final Recommendations

**Do**:
- Start with MVP, iterate (Phases 0-5 complete)
- Test on all platforms early
- Profile and optimize hot paths
- Write tests for critical systems
- Get playtesters early (week 50+ for story balance)
- Use version control religiously
- Document as you go
- Prioritize Archon path as primary story route
- Consider Phase 8 (Hidden Paths) as optional/DLC
- Focus on consciousness decay and corruption threshold systems first
- Build dialogue system to support 13 major NPCs

**Don't**:
- Don't optimize prematurely
- Don't add features without design doc update
- Don't skip testing (especially for 7 ending paths)
- Don't ignore memory management (more complex state now)
- Don't hardcode content (even more critical with 14 quest chains)
- Don't delay cross-platform testing
- Don't implement Morningstar Trial 7 (intentionally impossible)
- Don't attempt to maintain exactly 50% corruption (it's meant to be nearly impossible)

**Key Success Factors**:
1. **Solid Foundation**: Core systems (parser, state, memory) must be rock-solid (COMPLETE)
2. **Data-Driven Design**: Makes iteration fast and enables modding (critical for 13 NPCs + 14 quests)
3. **Early Playtesting**: Catches balance and pacing issues (especially corruption balance)
4. **Performance Discipline**: Profile early, optimize judiciously
5. **Platform Abstraction**: Makes cross-platform work manageable
6. **Good Error Handling**: Makes debugging and player experience better
7. **Modular Narrative**: Build story systems incrementally (consciousness, corruption, trials)
8. **Clear Path Priorities**: Focus on Archon path first, others as time permits

### Next Steps

**Current Status**: Phases 0-5 complete (24 weeks done)

**To continue implementation**:
1. Begin Phase 6 (Weeks 24-30): Story Foundation
   - Implement consciousness decay system
   - Expand corruption to 10 tiers with 70% threshold
   - Add Regional Council NPCs (6 characters)
   - Add Divine Architect NPCs (7 gods)
   - Implement Thessara ghost system
2. Continue to Phase 7 (Weeks 31-37): Archon Path
   - Build 7 Archon Trials sequentially
   - Implement Divine Council judgment
   - Add network patching mechanics
3. Evaluate Phase 8 (Weeks 38-45): Optional Hidden Paths
   - Can be implemented as post-release DLC
   - Focus on minimum viable story first (55 weeks vs 63 weeks)

### Scope Management

**This plan now reflects the FULL story.md narrative (22,000 words)**. The expansion from the original plan is significant:

**Original Plan**:
- 40 weeks
- 3 endings
- 4 major NPCs
- ~20,000 LOC

**Expanded Plan (Updated)**:
- **63 weeks** (or 55 weeks minimum viable)
- **7 endings** (3 required, 4 optional)
- **17 major NPCs** (4 original + 6 council + 7 gods)
- **~56,000 LOC** (36,000 new + 20,000 original)
- **+23 data files** for new systems

**Recommendation**: The narrative complexity has increased 280% from the original plan. Consider:
1. Implement Phases 6-7 (Archon path) as planned
2. Ship Phase 8 (Wraith/Morningstar) as DLC/expansion
3. This reduces timeline to 55 weeks (~13.75 months) while preserving core narrative

### Flexibility Notes

This plan provides a comprehensive roadmap, but **flexibility is crucial**:
- Story.md is aspirational - not all content may be necessary
- Archon path is the "intended" route and should be prioritized
- Wraith and Morningstar paths provide replayability but aren't required for 1.0
- The data-driven approach makes it possible to add content incrementally
- Post-release updates can expand the narrative over time

The narrative complexity of this game is **exceptional for a C project**, but the systems-focused approach (consciousness as code, corruption as state, Death Network as infrastructure) aligns well with the C implementation. The protagonist is literally a sysadmin - the technical metaphors are not just flavor, they're core to the design.

Good luck building Necromancer's Shell. This is now a **significantly more ambitious** project than originally planned, but the expanded narrative scope matches the quality of story.md. It has the potential to be truly special.

---

## Final Statistics

**Total Implementation Timeline**:
- **Full Implementation**: 63 weeks / 15.75 months (single developer)
- **Minimum Viable Story**: 55 weeks / 13.75 months (single developer, skip Phase 8)
- **Original Estimate**: 40 weeks / 10 months
- **Expansion Factor**: +58% weeks, +280% narrative complexity

**Estimated Final Code Size**:
- **Production Code**: ~36,000 lines of C (original: ~20,000)
- **Test Code**: ~10,000 lines (comprehensive coverage)
- **Data Files**: ~23 new .dat files + original set
- **Total LOC**: ~56,000 lines

**Content Statistics**:
- **Major NPCs**: 17 (4 original + 6 Regional Council + 7 Divine Architects)
- **Ending Paths**: 7 (Revenant, Lich Lord, Reaper, Archon, Wraith, Morningstar, Morningstar Failure)
- **Quest Chains**: 14+ (7 Archon Trials + 6 Morningstar Trials + Redemption quests)
- **Magic Schools**: 4 (Necromancy, Divine, Arcane, Primal)
- **Corruption Tiers**: 10 (vs original basic system)
- **Time Tracking**: Days, Hours, Months, Years (vs original Days/Hours only)

**Target Platforms**: Linux, Windows, macOS

**Technology Stack**: C11, ncurses/PDCurses, custom data loader (INI-style format)

**Target Playtime**:
- **Single Playthrough**: 20-30 hours (vs original 10-15 hours)
- **All Endings**: 80-120 hours
- **Speedrun Potential**: Yes (multiple optimization routes)

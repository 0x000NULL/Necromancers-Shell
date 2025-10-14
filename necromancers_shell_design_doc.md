# **"Necromancer's Shell" - Complete Design Document**

## **High Concept**
A terminal-based dark fantasy game where necromancy is literally system administration. You manage the undead through a command-line interface that blends Unix-like commands with arcane spell syntax, building an empire of the dead while navigating moral choices and rival necromancers.

---

## **Core Narrative**

### **Setting: The Deadlands Protocol**
The world operates on two layers:
- **The Living Realm**: Medieval fantasy kingdoms, churches, villages
- **The Death Network**: An invisible supernatural infrastructure connecting all death in the world

You play as **Morven**, a recently-deceased systems administrator who made a pact with death itself. Instead of passing on, you're given the Necromancer's Shell - a mystical terminal that interfaces with the Death Network. Your goal: amass enough power to either resurrect yourself permanently, ascend to lichdom, or break the cycle of death entirely.

### **The Three Paths** (chosen through gameplay)
1. **The Revenant Route**: Resurrect yourself, restore humanity, dismantle the Death Network
2. **The Lich Lord Route**: Embrace undeath, conquer the living, perfect your dark craft
3. **The Reaper Route**: Become death's true administrator, balance life and death, maintain the system

---

## **Command System Architecture**

### **Basic Syntax Philosophy**
Commands blend Unix-style flags with fantasy spell components:
```
raise [target] --from [location] --bind [duration] --fuel [soul_type]
```

### **Core Command Categories**

#### **1. Necromantic Operations (`raise`, `bind`, `banish`, `harvest`)**

**RAISE** - Animate the dead
```
raise zombie --from graveyard_blackwood --count 5
raise skeleton --from battlefield_crimson --class warrior --loyalty 7
raise ghoul --from plague_pit --stats [str:8,agi:4,int:2]
raise wraith --from cursed_manor --invisible true --duration 4h
```

**Complexity Tiers:**
- **Tier 1 (Zombies)**: Simple meat puppets, cheap, decay fast
- **Tier 2 (Skeletons)**: Durable, can follow complex orders, require more soul energy
- **Tier 3 (Ghouls/Wraiths)**: Specialized units with abilities
- **Tier 4 (Death Knights)**: Named champions requiring rare components
- **Tier 5 (Liches/Vampires)**: Sentient undead with their own agendas

**BIND** - Control and command
```
bind zombie_5 --task guard --location tower_north --priority high
bind skeleton_squad_alpha --task harvest --target villagers --stealth true
bind wraith_whisper --task spy --target enemy_necromancer
```

**HARVEST** - Collect soul energy (the game's currency)
```
harvest --target battlefield_aftermath --filter warrior_souls
harvest --from graveyard_blackwood --quality common --quantity 50
harvest --target condemned_prisoner --quality innocent --warning_moral_cost
```

**BANISH** - Release control or destroy undead
```
banish zombie_12 --reason degraded --recycle true
banish all --filter loyalty<3 --confirm
banish enemy_summons --from hex_af349b
```

#### **2. Network Operations (`scan`, `probe`, `connect`, `trace`)**

**SCAN** - Detect death signatures in the network
```
scan --radius 5km --filter fresh_corpses
scan --location enemy_territory --depth deep --stealth
scan --type ancient_tombs --quality legendary
```

**PROBE** - Investigate specific death signatures
```
probe corpse_id_4829 --info [cause_of_death,soul_quality,memories]
probe location haunted_cathedral --threats --opportunities
```

**CONNECT** - Access remote death nodes
```
connect --to crypt_system_elder_mountain
connect --to enemy_necromancer_shell --bruteforce --risk high
```

#### **3. Soul Management (`souls`, `soulforge`, `transfer`)**

**SOULS** - View soul inventory
```
souls --list --sort quality
souls --filter warrior --available
souls --info soul_id_8472 --memories
```

Soul types and their uses:
- **Common (grey)**: Basic fuel, abundant
- **Warrior (red)**: Powers combat undead
- **Mage (blue)**: Fuels complex spells
- **Innocent (white)**: Most powerful but corrupts you
- **Corrupted (black)**: From criminals, cheap but unstable
- **Ancient (gold)**: From historic sites, rare, legendary powers

**SOULFORGE** - Craft and combine souls
```
soulforge --combine [warrior_soul x3] --output elite_warrior_essence
soulforge --purify corrupted_soul --cost 50_common_souls
soulforge --extract memories --from ancient_soul --save to grimoire
```

#### **4. Spell System (`cast`, `enchant`, `curse`, `ward`)**

**CAST** - Direct magical attacks
```
cast deathbolt --target enemy_mage --power high
cast plague_cloud --area village_riverside --duration 3days --lethality 60%
cast soul_rip --target living_champion --save_soul true
```

**WARD** - Defensive magic
```
ward --location tower_sanctum --against church_paladins --strength 8
ward --on self --type anti-scrying --duration 24h
```

**CURSE** - Debuffs and long-term effects
```
curse --target enemy_necromancer --type decay --infection true
curse --area farmlands --effect crop_failure --subtlety high
```

#### **5. Intelligence & Espionage (`listen`, `possess`, `memory`, `scry`)**

**POSSESS** - Take control of corpses/undead for direct action
```
possess wraith_scout --infiltrate castle_thornhaven --record
possess zombie_infiltrator --act_alive --mission steal_artifact
```

**MEMORY** - Extract information from souls
```
memory --from soul_id_9271 --query "treasure location"
memory --from ancient_soul --playback "last_moments"
```

**SCRY** - Remote viewing through death network
```
scry --target enemy_fortress --through undead_inside
scry --location ritual_site --realtime --alert on_activity
```

#### **6. System Management (`status`, `log`, `upgrade`, `debug`)**

**STATUS** - View system state
```
status --minions --sort loyalty
status --resources --detailed
status --threats --severity high
status --corruption_level
```

**LOG** - Review history and events
```
log --recent 10 --filter failures
log --search "church patrol" --timeframe 3days
log --export combat_statistics
```

---

## **Resource Management Systems**

### **Soul Energy Economy**

**Income Sources:**
- Natural deaths in your territory (passive)
- Battlefield harvesting (active, risky)
- Executions and murders (opportunistic)
- Ancient tomb raiding (one-time, dangerous)
- Soul trade with dark entities (expensive, corrupting)

**Expenditures:**
- Raising undead (one-time cost)
- Maintaining undead (ongoing cost)
- Casting spells (per use)
- Upgrading facilities (one-time)
- Bribing dark powers (negotiable)

### **Corruption System**

Every unethical action increases your corruption:
- Harvesting innocent souls: **+5 corruption**
- Killing civilians: **+3 corruption**
- Desecrating holy sites: **+10 corruption**
- Breaking promises to spirits: **+7 corruption**

**Corruption Effects:**
- **0-20**: Mostly human, can still interact normally with living
- **21-40**: Appearance decays, NPCs afraid, some spells stronger
- **41-60**: Becoming monstrous, villages flee, undead loyalty bonus
- **61-80**: Nearly a lich, living flee on sight, powerful dark magic
- **81-100**: Full transformation, can't return, ultimate power, isolation

**BUT** - Some endings require high corruption, others require low

### **Territory Control**

**Your Domain:**
- Start with a ruined tower and small graveyard
- Expand through conquest or negotiation
- Each territory provides resources:
  - **Graveyards**: Corpse supply
  - **Battlefields**: Warrior souls
  - **Villages**: Soul harvest points (ethical dilemma)
  - **Crypts**: Ancient knowledge and artifacts
  - **Ritual sites**: Spell power multipliers

**Territory Status Display:**
```
territories --list
> tower_blackspire [CONTROLLED]
  - corpse_supply: 12/week
  - soul_income: 45/day
  - threat_level: low
  - garrison: 25 skeletons, 5 wraiths

> village_millbrook [CONTESTED]
  - population: 230 living
  - church_presence: moderate
  - opportunity: high
  - moral_cost: harvesting civilians
```

---

## **Progression & Unlocks**

### **Skill Trees**

#### **Reanimation Branch**
1. Basic Zombies → Enhanced Zombies → Ghouls → Abominations
2. Skeletal Servants → Skeleton Warriors → Death Knights
3. Shades → Wraiths → Banshees → Spectral Lords

#### **Soul Craft Branch**
1. Soul Harvesting → Soul Refinement → Soul Fusion
2. Memory Extraction → Memory Implantation → Consciousness Transfer
3. Soul Batteries → Soul Engines → Perpetual Motion (undead never decay)

#### **Network Mastery Branch**
1. Local Scanning → Regional Mapping → Continental Network
2. Basic Encryption → Ghost Protocols → Undetectable Operations
3. Single Connection → Parallel Processing → Hive Mind Control

#### **Dark Arcana Branch**
1. Combat Spells → Area Effects → Cataclysmic Magic
2. Curses → Plagues → Apocalyptic Events
3. Minor Pacts → Demonic Allies → Summon Death Itself

### **Unlock Mechanisms**

**Grimoire System:**
- Find/steal spell books throughout the world
- Each grimoire adds new commands and flags
- "Liber Mortis" unlocks lich transformation
- "Scrolls of Binding" adds loyalty commands
- "Necronomicon Fragment" enables demon summoning

**Research Projects:**
```
research --project soul_compression --duration 3days --cost 500souls
research --project undead_intelligence --requirement lich_corpse
research --project death_network_backdoor --difficulty extreme
```

**Artifacts:**
Physical items that modify your shell capabilities:
- **Staff of Dominion**: +2 to all minion loyalty
- **Crown of Whispers**: Hear all deaths in real-time
- **Phylactery**: Respawn point if killed
- **Death's Ledger**: See everyone's time of death

---

## **Combat System**

### **Battle Interface**

When combat initiates, you enter **Battle Mode**:

```
=== COMBAT INITIATED ===
ENEMIES: 12x Church Paladins, 1x High Priest
YOUR FORCES: 8x Skeleton Warriors, 3x Wraiths, 1x Death Knight

TURN 1 - YOUR ACTION:
> _
```

**Turn-Based Typing Combat:**
- Type commands quickly for better initiative
- Typos cause spells to fizzle or misfire
- Complex spells = higher risk/reward
- Can queue commands for multiple units

**Example Combat Sequence:**
```
> bind skeleton_squad_alpha --attack paladins --formation shield_wall
> cast fear --target paladins --filter low_willpower
> possess death_knight_mordred --charge high_priest
> raise --from fallen_paladins --quick --fuel corrupted_souls
> ward --on self --prepare retreat
```

**Combat Modifiers:**
- **Typing Speed**: Faster = more actions per turn
- **Spell Complexity**: Simple spells easier to type correctly
- **Soul Reserves**: Panic spending for emergency power
- **Minion Loyalty**: Low loyalty = units ignore commands

### **Enemy Types**

**Church Forces:**
- Paladins: High holy damage, anti-undead
- Priests: Healing, turning undead, wards
- Inquisitors: Can trace you through death network
- Angels: Rare, devastating, purge entire armies

**Rival Necromancers:**
- Can counter your spells
- Steal your minions with better binding
- Network warfare (hacking your undead)
- Different specializations (swarms vs. elite units)

**Living Kingdoms:**
- Knights: Heavy armor, resistant to fear
- Mages: Elemental magic, area denial
- Rangers: Ambush tactics, snipe your wraiths
- Siege Weapons: Destroy your towers and fortifications

**Monsters & Aberrations:**
- Demons: Powerful but treacherous allies
- Fey: Reality-bending tricksters
- Eldritch Horrors: Can't be controlled, mutual threats
- Wild Undead: Unbound, attack everyone

---

## **Story Structure & Missions**

### **Act I: The Awakening (Hours 1-3)**

**Tutorial Phase:**
- Wake up dead in your old laboratory
- Discover the Necromancer's Shell
- Learn basic commands through guided rituals
- Raise your first zombie from your own corpse (meta!)
- Escape the church agents who killed you

**Early Missions:**
1. **"Grave Robbing 101"**: Raid a graveyard for corpses
2. **"Revenge is a Dish Served Cold"**: Hunt your murderer
3. **"The First Territory"**: Claim an abandoned tower
4. **"Soul Economics"**: Establish soul harvesting operation
5. **"Rival Emergence"**: Encounter another necromancer

### **Act II: The Rise (Hours 4-8)**

**Expansion Phase:**
- Build your undead army
- Choose specialization path
- Face moral dilemmas about civilian casualties
- Uncover the Death Network's true nature
- Deal with first major threat (church crusade or rival)

**Major Missions:**
1. **"The Siege of Thornhaven"**: Conquer or negotiate with a fortified town
2. **"The Plague Doctor"**: Ally with or eliminate a plague-spreading rival
3. **"Lost Crypt of the First Necromancer"**: Dungeon crawl for ancient knowledge
4. **"The Soul Market"**: Dark deals with extra-planar entities
5. **"Betrayal Protocol"**: Your most powerful minion rebels

### **Act III: The Transformation (Hours 9-12)**

**Endgame Phase:**
- Choose your final path (Revenant/Lich/Reaper)
- Face the ultimate enemy (depends on your choices)
- Make final moral decisions
- Execute your master plan
- Experience consequence-based ending

**Final Missions:**
1. **"The Church's Last Stand"** or **"The Demon's Bargain"**
2. **"Network Override"**: Hack the Death Network itself
3. **"The Lich Ritual"** or **"The Resurrection"** or **"The Coronation"**
4. **"Final Battle"**: Against the ultimate threat
5. **"Legacy"**: Epilogue showing your impact on the world

---

## **Dynamic Events & Challenges**

### **Random Encounters**

**Positive Events:**
- Battlefield discovered (free souls!)
- Necromantic artifact found
- Minion achieves sentience (special unit)
- Ancient ghost offers knowledge trade
- Plague outbreak (dark opportunity)

**Negative Events:**
- Church inquisition hunts you
- Soul blight (corruption spreads to your reserves)
- Undead plague (your minions infect nearby areas)
- Rival necromancer raids your territory
- Death Network instability (commands fail randomly)

### **Moral Dilemmas**

**The Innocent Village:**
- A defenseless village has perfect harvesting conditions
- High reward: 200 innocent souls (massive power)
- Cost: +50 corruption, closes "good" ending path
- Alternative: Protect them, they become allies but cost resources

**The Condemned Prisoner:**
- A prisoner scheduled for execution is actually innocent
- You can harvest their soul (quality: innocent)
- Or investigate and prove innocence (time-consuming, costs soul energy)
- The truth is discoverable through memory extraction

**The Sentient Undead:**
- One of your skeleton warriors develops consciousness
- It begs for release and rest
- Keep it: More powerful but loyalty is uncertain
- Release it: Lose a strong unit but gain karma
- Imprison its consciousness: Ultimate control but corrupting

**The Plague Option:**
- You can create a plague to mass-harvest souls efficiently
- Thousands will die, but you'll become extremely powerful
- Church will unite entire continent against you
- Or develop a cure and trade it for political power

### **Time-Sensitive Missions**

**Countdown Timers:**
```
WARNING: Church Crusade forming
- Paladins gathering: 5 days
- Estimated army size: 500 warriors
- Options: 
  1. Preemptive strike (interrupt formation)
  2. Fortify defenses (prepare for siege)
  3. Diplomatic solution (negotiate through living allies)
  4. Flee (abandon territory)
```

**Decay Timers:**
- Corpses decay → Unusable after 7 days
- Undead require maintenance or fall apart
- Souls in storage slowly fade
- Time pressure forces strategic choices

---

## **Aesthetic & Interface Design**

### **Visual Style**

**ASCII Art Environments:**
```
╔═══════════════════════════════════════╗
║   THE BLACKSPIRE TOWER - SANCTUM     ║
╠═══════════════════════════════════════╣
║                                       ║
║       💀                              ║
║      /│\    NECRONOMICON             ║
║       │     ════════════              ║
║      / \    Current Page: 127        ║
║                                       ║
║   🕯️         🕯️         🕯️           ║
║                                       ║
║   ⚰️  ⚰️  ⚰️  ⚰️  ⚰️  ⚰️            ║
║   [Your Minion Army: 47 Active]      ║
║                                       ║
╚═══════════════════════════════════════╝
```

**Color Scheme:**
- **Green**: Nature, life, healing (rare in this game)
- **Purple**: Soul energy, magic, necromancy
- **Red**: Combat, danger, corruption
- **Blue**: Intelligence, memory, knowledge
- **Grey**: Death, undead, neutral
- **Gold**: Divine, holy, purification
- **Black/White**: Lich transformation progress

### **Sound Design (Optional)**

**Atmospheric Audio:**
- Distant thunder and wind
- Whispers of trapped souls
- Bone clattering when raising skeletons
- Church bells (warning of paladins)
- Your own heartbeat (fades as you become undead)

**Command Feedback:**
- Successful spell: Ethereal chime
- Failed spell: Discordant buzz
- Critical error: Ghostly scream
- Minion death: Crack of breaking bone
- Soul harvest: Whispered sigh

### **UI Elements**

**Status Bar (Always visible):**
```
╔════════════════════════════════════════════════════════╗
║ SOULS: 847 | MINIONS: 47/60 | CORRUPTION: 34% | DAY 12 ║
╚════════════════════════════════════════════════════════╝
```

**Contextual Warnings:**
```
⚠️  WARNING: Corruption approaching critical threshold
⚠️  ALERT: Church forces detected 3km north
⚠️  SYSTEM: 12 minions require maintenance
💀 INFO: Ancient tomb discovered in scrying
✨ UNLOCK: New spell available - "Soul Storm"
```

---

## **Replayability Features**

### **New Game Plus Modes**

**NG+ Modifiers:**
- **Ironman**: Permanent death, one save file
- **Speedrun**: Timer visible, leaderboards
- **Pacifist**: No civilian casualties allowed
- **Purge**: Start with max corruption, full lich
- **Ethical**: Must stay below 20% corruption
- **Challenger**: All enemies 2x stronger

### **Alternative Starts**

1. **The Apprentice**: Start as a necromancer's student (tutorial skip)
2. **The Risen Hero**: You were a legendary warrior, now undead
3. **The Cursed Mage**: Accidentally became a necromancer
4. **The Ancient**: You're a pre-existing lich reawakening

### **Achievement System**

**Hidden Challenges:**
- "Peaceful Death": Complete game without killing civilians
- "Lich Speedrun": Transform into lich within 5 days
- "Army of Darkness": Command 200+ minions simultaneously
- "Ethical Necromancy": Complete game with <10% corruption
- "Dark Lord": 100% corruption, all territories conquered
- "Ghost Writer": Extract and read 100 soul memories
- "Network Admin": Hack rival necromancer's shell
- "Deathless": Complete game without losing any minion

### **Unlockable Content**

**Post-Game Rewards:**
- **New spells**: Unlocked for replays
- **Starting bonuses**: Begin with artifacts/souls
- **Cosmetic grimoire themes**: Change UI appearance
- **Lore codex**: Expanded world information
- **Challenge scenarios**: "What if" missions

---

## **Sample Gameplay Session**

**Early Game Example (15 minutes):**

```
> status
Current Location: Ruined Tower
Minions Active: 3 zombies
Soul Energy: 45 common
Corruption: 8%
Active Threats: None

> scan --radius 5km --filter fresh_corpses
Scanning death network...
Found:
- 2x fresh corpses at village_millbrook (died 2h ago)
- 7x ancient remains at crypt_hollow (100+ years)
- 1x battlefield remnant at ridge_bloody (warriors)

> connect --to village_millbrook
Connecting...
CONNECTION ESTABLISHED
Warning: Church presence detected in area

> scan --detailed
Corpse #1: Miller, age 52, natural death, soul quality: common
Corpse #2: Unknown, age ~30, murdered, soul quality: corrupted

> raise zombie --from corpse_1 --bind loyalty_5
Raising: Miller (zombie_04)
Cost: 15 souls
Miller shambles to unlife...
SUCCESS

> harvest --from corpse_2 --quality corrupted
Harvesting soul...
+25 corrupted souls gained
Warning: Corrupted souls are unstable

> bind zombie_04 --task guard --location tower_entrance
Command accepted. Miller now guarding.

> log --recent 5
[14:23] Zombie_02 maintenance required (decay at 60%)
[14:15] Scan detected church patrol 8km northeast
[14:08] Soul harvest completed: +45 common
[13:47] Research project 40% complete
[13:22] Territory expansion opportunity: crypt_hollow

> research --status
Current Project: Enhanced Skeletal Binding
Progress: 40%
Time Remaining: 2 days
Cost: Already paid (200 souls)

> cast ward --location tower_sanctum --against scrying
Casting ward spell...
Cost: 30 souls
Ward established. Duration: 24 hours.
You are now hidden from magical detection.

> probe --location crypt_hollow --threats
Probing ancient crypt...
Detected threats:
- 3x Wild Undead (hostile, mindless)
- Trapped doorway (magical)
- Ancient guardian (dormant)
Opportunities:
- 15+ ancient corpses (warrior quality)
- Necromantic artifact (unknown)
- Knowledge cache (spellbook possible)

> listen --to church_presence
Intercepting communications...
"...reports of dark activity near Millbrook..."
"...inquisitor dispatched, arrives in 3 days..."
"...purge protocols authorized..."

> status --threats
ACTIVE THREATS:
[MEDIUM] Church Inquisitor (arrives in 3 days)
[LOW] Rival necromancer (distant, no activity)
[UNKNOWN] Crypt guardian (dormant but dangerous)

> _
```

---

## **Technical Features**

### **Save System**
- **Autosave**: Every major action
- **Manual saves**: Through `save` command
- **Save slots**: 3 per profile
- **Cloud sync**: Optional (saves your dark deeds forever)
- **Permadeath mode**: Single save, deleted on death

### **Accessibility**
- **Screen reader support**: Full compatibility
- **Colorblind modes**: Multiple palette options
- **Text size adjustment**: Readable for all
- **Speed settings**: Slow down real-time events
- **Typo forgiveness**: Minor typos auto-correct (toggleable)

### **Modding Support**
- **Custom spells**: JSON-based spell definitions
- **New minion types**: Moddable unit stats
- **Campaign editor**: Create custom stories
- **Asset replacement**: Change ASCII art and colors
- **Community grimoire**: Share spell books

### **Platform Features**
- **Cross-platform**: Windows, Mac, Linux
- **Mobile version**: Touch-friendly command palette
- **Leaderboards**: Speedrun times, highest armies
- **Daily challenges**: Procedural missions
- **Seasonal events**: Halloween special content

---

## **Why This Game Works**

**Unique Selling Points:**
1. **Power fantasy**: Command armies through typing
2. **Moral weight**: Choices genuinely matter
3. **Learning curve**: Intuitive Unix-like syntax
4. **Depth**: Hundreds of command combinations
5. **Atmosphere**: Dark fantasy meets tech thriller
6. **Accessibility**: Text-based = lower barrier to entry
7. **Replayability**: Multiple paths and endings
8. **Typing practice**: Get better at coding while playing!

**Target Audience:**
- Terminal enthusiasts and programmers
- Dark fantasy RPG fans
- Strategy game players
- Anyone who enjoyed "Cultist Simulator" or "Fallen London"
- Players who want meaningful moral choices

**Estimated Playtime:**
- First playthrough: 10-15 hours
- 100% completion: 40+ hours
- Speedrun: 2-4 hours

---

This game would feel like you're **actually** a necromancer using an ancient terminal to command the forces of death, where your typing skill directly translates to magical power, and every command has weight and consequence. The blend of technical precision and dark fantasy creates a unique experience that respects both genres while creating something entirely new.

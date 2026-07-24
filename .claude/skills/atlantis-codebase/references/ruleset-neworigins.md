# NewOrigins ruleset reference

The only ruleset under active development. Five files in `neworigins/`, plus the shared
tables it rewrites at startup.

Contents:
1. `rules.cpp` — the `Globals` struct
2. `extra.cpp` — faction setup, quests, victory
3. `world.cpp` / `map.cpp` — world generation
4. `monsters.cpp`
5. Faction types and activity budgets
6. Distinctive NewOrigins mechanics
7. Regenerating the rules HTML

---

## 1. `rules.cpp` — the `Globals` struct

A single positional initialiser of `GameDefs` (declared in `gamedefs.h`, ~400 fields),
followed by `GameDefs *Globals = &g;`. Every field is commented with its name because the
initialiser is positional — **the comments are the only thing keeping this readable, and
they are not checked by the compiler.** If you add a field to `GameDefs`, add the value at
the same position in all six rulesets plus `unittest/rules.cpp`, and keep the trailing
comment.

Also here: the faction-limit arrays indexed by faction-type points, read by
`Game::AllowedMages` and friends in `game.cpp`.

```cpp
static int am[] = { 1, 2, 3, 4, 5, 6 };        // allowedMages,     by MAGIC points
static int aa[] = { 1, 3, 5, 7, 10, 15 };      // allowedApprentices
static int aw[] = { 0, 15, 30, 50, 75, 100 };  // allowedTaxes,     by WAR/MARTIAL points
static int at[] = { 0, 15, 30, 50, 75, 100 };  // allowedTrades,    by TRADE/MARTIAL points
static int aq[] = { 0, 2, 5, 9, 14, 25 };      // allowedQuartermasters
static int ag[] = { 0, 1, 2, 4, 6, 10 };       // allowedTacticians
static int ma[] = { 0, 10, 25, 40, 60, 90 };   // allowedMartial
```

Values that most shape how NewOrigins plays — check these before reasoning about any
economic or combat behaviour, and remember a `Globals` change is a snapshot change:

| Field | Value | Effect |
|---|---|---|
| `RULESET_NAME` / `RULESET_VERSION` | `"NewOrigins"` / 3.0.0 | Savegame gate; see `conventions.md` §6 |
| `MAX_SPEED` / `PHASED_MOVE_OFFSET` | 8 / 7 | Movement window and phase stagger |
| `START_MONEY` | 10000 | Plus `TurnNumber() * 300` for late joiners |
| `WORK_FRACTION` / `ENTERTAIN_INCOME` | 5 / 30 | Base economy |
| `TAX_BASE_INCOME` | 50 | Per taxer |
| `WHO_CAN_TAX` | `TAX_NORMAL │ TAX_HORSE_AND_RIDING_SKILL │ TAX_MAGE_DAMAGE` | Who qualifies |
| `TAX_PILLAGE_MONTH_LONG` | 1 | Taxing consumes the month |
| `WEATHER_EXISTS` | 0 | No seasonal weather |
| `START_CITIES_EXIST` / `SAFE_START_CITIES` | 0 / 0 | No protected start cities |
| `NEXUS_EXISTS` / `NEXUS_GATE_OUT` | 1 / 1 | Factions start in the Nexus and gate out |
| `FACTION_LIMIT_TYPE` | `FACLIM_FACTION_TYPES` | Faction points, not a flat mage cap |
| `FACTION_POINTS` | 5 | Points to distribute across types |
| `FACTION_ACTIVITY` | `MARTIAL_MERGED` | Tax and trade share one budget |
| `UNDERWORLD_LEVELS` | 1 | Plus nexus and surface |
| `OCEAN` / `CONTINENT_SIZE` / `ARCHIPELAGO` | 55 / 14 / 20 | Map generation shape |
| `DYNAMIC_POPULATION` | 0 | Migration phase is skipped |
| `REGIONS_ECONOMY` | 1 | `ProcessEconomics` runs |
| `OPEN_ENDED` | 0 | `CheckVictory` is called every turn |
| `WORLD_EVENTS` | 1 | `times.<n>` narrative feed is generated |
| `BATTLE_LOG_LEVEL` | `VERBOSE` | Detailed battle reports |
| `ADVANCED_TACTICS`, `EXTENDED_FORT_DEFENCE`, `HALF_RIDING_BONUS`, `OVERWHELMING` | 1/1/1/0 | Combat model |
| `DESTROY_BEHAVIOR` | `PER_SKILL`, `MIN_DESTROY_POINTS` 200, `MAX_DESTROY_PERCENT` 34 | Building demolition |
| `TRANSPORT` | `ALLOW_TRANSPORT │ QM_AFFECT_COST │ QM_AFFECT_DIST` | Quartermaster network |
| `SPOILS_NO_TRADE`, `BUILD_NO_TRADE`, `TRANSPORT_NO_TRADE` | 1 | Trade goods excluded from those systems |

## 2. `extra.cpp`

Three engine hooks and one large static helper.

**`Game::SetupFaction`** (line 42). Gives `START_MONEY + TurnNumber()*300` unclaimed silver,
then unless `noStartLeader`: one leader, made `U_MAGE`, with 30 days each of Observation,
Force, Pattern, Spirit, Gate Lore and Fire, plus 180 days of Combat; `FLAG_BEHIND` set;
starting food if `UPKEEP_MINIMUM_FOOD > 0`; placed in the Nexus (or `pFac->pStartLoc`);
declared unfriendly to the monster faction. New factions therefore start as a single mage
in the Nexus and must gate out.

**`CreateQuest`** (static, line 118). Builds one of `Quest::SLAY`, `HARVEST`, `BUILD`,
`VISIT`, `DEMOLISH` with an item reward. Constants at the top of the file tune it:

```
MINIMUM_ACTIVE_QUESTS      5
MAXIMUM_ACTIVE_QUESTS     20
QUEST_EXPLORATION_PERCENT 30    // exploration threshold before relic quests spawn
QUEST_SPAWN_RATE           7    // spawn attempts per turn
QUEST_SPAWN_CHANCE        70    // percent chance per attempt
QUEST_MAX_REWARD        3000    // silver-equivalent cap on reward items
MAX_DESTINATIONS           5    // for VISIT quests
```

**`Game::CheckVictory`** (line 406). Despite the name, NewOrigins has no winner: it
**always returns `NULL`**. What it actually does each turn is:

1. Tally visited vs. unvisited populated regions, and prune each unit's `visited` set to
   currently-relevant VISIT destinations.
2. Once `visited >= 30%` of populated regions, spawn relic quests up to the maximum, and
   top up to the minimum.
3. Emit a `times.<n>` article — an exploration nudge, an exploration percentage, a hint
   pointing at an incompletely explored region, or a description of an active quest.

So this function is the game's narrative engine, not its endgame. `Game::EndGame` still
exists and fires only if every faction dies.

**`Game::ModifyTablesPerRuleset`** (line 738). The startup table rewrite — see
`data-tables.md` §3. This is where the actual NewOrigins item/skill/object roster is
decided; `gamedata.cpp` alone will mislead you.

## 3. `world.cpp` and `map.cpp`

`Game::CreateWorld` (`world.cpp:376`) is **interactive** — it prompts on stdin for nexus
size, generator choice, and map width/height (both must be multiples of 8). It then:

```
regions.CreateLevels(2 + UNDERWORLD_LEVELS + UNDERDEEP_LEVELS + ABYSS_LEVEL)
SetupNames()
CreateNexusLevel(0, nx, ny, "nexus")
CreateSurfaceLevel(1, xx, yy, …)     // generator 1 = original, 2 = parametrical (Map/MapBuilder)
… underworld / underdeep / abyss levels …
```

The rest of `world.cpp` supplies region typing and starting-city predicates
(`GetRegType`, `GetLevelXScale/YScale`, `CheckRegionExit`, `GetWeather`, `GetStartingCity`,
`CanBeStartingCity`, `MakeStartingCity`, `IsStartingCity`, `IsSafeRegion`) plus the naming
helpers `AGetName` / `AGetNameString`.

`map.cpp` (~3000 lines) is the "parametrical" generator, and it is modern C++ unlike most
of the tree. Its model: `MapBuilder` partitions the grid into `Zone`s (continent / ocean),
zones own `Province`s, provinces own `ZoneRegion`s. It grows and merges zones, specialises
continents, adds volcanoes and lakes, grows terrain by biome, names oceans, then hands off
to the `ARegionList::Create*Level` / `MakeLand` / `GrowTerrain` / `RaceAnchors` /
`FinalSetupGates` family, also defined here. `ARegion::CheckSea` lives here too.

Map generation runs only at `atlantis new`, so changes here do **not** affect the snapshot
suite (which replays existing worlds) — the trade-off is that they are correspondingly
harder to verify, and worth exercising by creating a throwaway game.

## 4. `monsters.cpp`

`Game::CreateVMons` and `Game::GrowVMons` are both early-return stubs. NewOrigins uses the
engine's wandering and lair monsters (`npc.cpp`) rather than a ruleset-specific system.

## 5. Faction types and activity budgets

`FACTION_LIMIT_TYPE = FACLIM_FACTION_TYPES` with `FACTION_POINTS = 5`. A faction
distributes points across the type strings declared in `faction.h`:

```
F_WAR   F_TRADE   F_MAGIC   F_MARTIAL
```

`Faction::type` is an `unordered_map<std::string,int>`; `Faction::FactionTypeStr()` renders
it ("War 2, Magic 3"). Points index the arrays in `rules.cpp` through
`Game::AllowedMages` / `AllowedApprentices` / `AllowedQuarterMasters` /
`AllowedTacticians` / `AllowedTaxes` / `AllowedTrades` / `AllowedMartial`. Note the
`std::max` combinations in `game.cpp`: quartermasters take the better of TRADE and
MARTIAL, tacticians the better of WAR and MARTIAL.

`FACTION_ACTIVITY = MARTIAL_MERGED` means taxing and trading draw on one shared budget
rather than two. Per-region usage is recorded in `Faction::activity`
(`unordered_map<ARegion*, unordered_set<FactionActivity>>`) and enforced by
`Game::ActivityCheck` (`runorders.cpp:171`).

## 6. Distinctive NewOrigins mechanics

- **Nexus start, no start cities.** `START_CITIES_EXIST = 0`; factions begin as a single
  mage leader in the Nexus and gate out. Much of the early game is exploration, which is
  what `CheckVictory` is measuring.
- **Quest/relic narrative.** Quests spawn once 30% of populated regions are explored;
  articles go out through `WriteTimesArticle`. `quests.cpp` holds the completion checks
  (`CheckQuestKillTarget`, `CheckQuestHarvestTarget`, `CheckQuestBuildTarget`,
  `CheckQuestVisitTarget`, …), called from the relevant execution phases.
- **World events.** `WORLD_EVENTS = 1`. Battles and assassinations record `FactBase`
  subclasses (`events-battle.cpp`, `events-assassination.cpp`) during the turn; at the end
  `Events::Write` composes them into a narrative `times.<n>` article with landmark-aware
  location descriptions (`EventLocation`, `Landmark` in `events.h`).
- **Region economy.** `REGIONS_ECONOMY = 1`, `DYNAMIC_POPULATION = 0` — regions develop via
  `ProcessEconomics` and `ARegion::TownDevelopment`/`RoadDevelopment`, but population does
  not migrate.
- **Advanced combat.** `ADVANCED_TACTICS`, `EXTENDED_FORT_DEFENCE`, `HALF_RIDING_BONUS`,
  verbose battle logs with per-weapon `AttackStat` tracking in `army.cpp`.
- **No weather.** `WEATHER_EXISTS = 0`, so `SetWeather` is skipped in `PreProcessTurn` and
  movement costs are terrain-only.

## 7. Regenerating the rules HTML

```bash
make neworigins-rules
```

Runs `./neworigins genrules neworigins_intro.html neworigins.css html/neworigins.html`.
`genrules.cpp` walks the live tables — so enabling an item or changing a `Globals` value is
reflected automatically — but every sentence of prose is hand-written in `genrules.cpp` and
`skillshows.cpp`. Update that prose in the same change as the mechanic, or the generated
rules will confidently describe behaviour the engine no longer has.

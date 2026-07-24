# Architecture reference

Contents:
1. Build system and what links into what
2. Engine file map (root `*.cpp`)
3. Core classes and their responsibilities
4. Ownership and lifetime rules
5. The ruleset hook surface
6. Report generation path

---

## 1. Build system

`Makefile` at the repo root, driven by a `GAME` variable that defaults to `standard`.

```
CFLAGS = -g -I. -I.. -Wall -Werror -std=c++20
```

`-Werror` is on: an unused variable or a signed/unsigned comparison fails the build and CI.

Two object groups are linked into every binary:

- `ENGINE_OBJECTS` — the root `.cpp` files, compiled into `obj/`.
- `RULESET_OBJECTS` — always exactly `extra.o map.o monsters.o rules.o world.o`, compiled
  from `$(GAME)/` into `$(GAME)/obj/`.

So every game directory must contain those five files, no more and no fewer. That is why
`unittest/` also has `extra.cpp`, `map.cpp`, `monsters.cpp`, `rules.cpp`, `world.cpp`:
the test binary is built with `GAME=unittest` and needs a ruleset to link against. Its
`rules.cpp` declares `RULESET_NAME = "UnitTest Atlantis"` and its `world.cpp` stubs out
world generation (`CreateWorld()` is empty, all regions connect, weather is always
`W_NORMAL`).

Targets you will actually use:

```bash
make neworigins                    # → neworigins/neworigins
make unittest && ./unittest/unittest
make all                           # all six binaries; what CI compiles
make all-clean
make neworigins-rules              # regenerates neworigins/html/neworigins.html
```

The unittest link line is `$(filter-out obj/main.o,$(OBJECTS)) $(UNITTEST_OBJECTS)` —
i.e. the whole engine minus `main()`, plus `unittest/main.cpp` and every
`unittest/*_test.cpp`. New test files matching `*_test.cpp` are picked up automatically
by the wildcard; no Makefile edit needed.

## 2. Engine file map

**Entry point and lifecycle**
| File | Purpose |
|---|---|
| `main.cpp` | CLI dispatch: `new`, `run`, `edit`, `check`, `map`, `mapunits`, `genrules`. Calls `ModifyTablesPerRuleset()` before anything else. |
| `game.cpp` | `Game` lifecycle: `NewGame`, `OpenGame`/`SaveGame` (the `game.in`/`game.out` serialiser), `RunGame`, `PreProcessTurn`, `WriteReport`, `WriteTemplates`, unit-number bookkeeping, `WritePlayers`/`ReadPlayers`. |
| `game.h` | The `Game` class — nearly every engine entry point is a `Game` method. Also `CURRENT_ATL_VER`. |
| `edit.cpp` | Interactive GM game editor (`atlantis edit`). |

**Orders**
| File | Purpose |
|---|---|
| `orders.h` / `orders.cpp` | The `O_*` order enum, `OrderStrs`, `Parse1Order`, and one small class per order type (`MoveOrder`, `GiveOrder`, `CastOrder`, …). |
| `parseorders.cpp` | ~72KB. Reads `orders.<n>`, tokenises, validates, and attaches order objects to units. Instant orders (FORM, NAME, flags) take effect here. |
| `runorders.cpp` | ~85KB. `RunOrders()` and the instant/immediate phases: find, enter, promote, attack, steal, give, exchange, destroy, pillage, tax, guard, cast, sell, buy, quit, withdraw. |
| `monthorders.cpp` | ~54KB. Month-long orders: movement (`RunMovementOrders`), sail, teach, study, produce, build, ship construction, idle. |

**World and economy**
| File | Purpose |
|---|---|
| `aregion.cpp` | ~94KB. `ARegion` and `ARegionList`: terrain, exits, visibility, guarding/forbidding, pathfinding, region reports, decay, gates, roads. |
| `economy.cpp` | Population, wages, habitat, development, markets setup, town growth, migration. |
| `market.cpp` / `market.h` | `Market`/`MarketList`: buy/sell entries with price/amount curves. |
| `production.cpp` / `production.h` | `Production`/`ProductionList`: per-region resource pools. |
| `object.cpp` / `object.h` | Buildings and fleets. `O_DUMMY` is the pseudo-object holding units in the open. Fleet capacity/speed/sailing lives here. |
| `mapgen.cpp`, `simplex.cpp`, `graphs.h` | Shared procedural generation primitives (noise, biomes, graph helpers). |
| `namegen.cpp` | Procedural names for regions, rivers, objects, by ethnicity. |

**Actors**
| File | Purpose |
|---|---|
| `unit.cpp` / `unit.h` | ~65KB. The `Unit` class: men, items, skills, movement capacity, visibility, combat readiness, maintenance, reports. |
| `faction.cpp` / `faction.h` | `Faction`: type points, attitudes, activity budgets (TAX/TRADE), errors/events/shows accumulation, report writing. |
| `npc.cpp` | City guards, wandering monsters, lair monsters: creation and growth. |
| `items.cpp` / `items.h` | `Item`, `ItemList`, and the type descriptors: `ItemType`, `ManType`, `MonType`, `WeaponType`, `ArmorType`, `MountType`, `BattleItemType`. |
| `skills.cpp` / `skills.h` | `Skill`, `SkillList`, `SkillType`, plus combat-adjacent descriptors: `SpecialType`, `EffectType`, `RangeType`, `AttribModType`, `HealType`. |

**Combat and magic**
| File | Purpose |
|---|---|
| `battle.cpp` / `battle.h` | `Battle::Run` — free round, normal rounds, routing, spoils, battle log. |
| `army.cpp` / `army.h` | `Army`/`Soldier`: building the battle line, front/back, attack resolution, per-attack statistics (`AttackStat`, `UnitStat`). |
| `specials.cpp` | Special (magical) attacks and shields used by `battle.cpp`. |
| `shields.cpp` / `shields.h` | `ShieldList` — active shields during a battle. |
| `spells.cpp` | ~49KB. All spell parsing and execution. |
| `spells.h` | Included **twice** with different macros: once inside the `Game` class body (`#define GAME_SPELLS`) to declare spell methods, once normally. Add a new spell method inside the `GAME_SPELLS` block or it will not be a `Game` member. |

**Output**
| File | Purpose |
|---|---|
| `template.cpp` | Order-template generation (`template.<n>`) and the ASCII map lines. |
| `genrules.cpp` | ~260KB. Generates the player rules HTML from the live tables. |
| `skillshows.cpp` | ~68KB. Skill descriptions shown in reports and rules. |
| `events.cpp`, `events-battle.cpp`, `events-assassination.cpp`, `events.h` | The world-events system: facts recorded during the turn become the narrative `times.<n>` article. Modern C++ (`std::string`, `std::list`, `std::vector`). |
| `quests.cpp` / `quests.h` | `Quest`/`QuestList`: SLAY, HARVEST, BUILD, VISIT, DELIVER, DEMOLISH, with rewards. |

**Infrastructure**
| File | Purpose |
|---|---|
| `alist.h` / `alist.cpp` | Intrusive singly-linked list `AList`/`AListElem` plus the `forlist`, `forlist_reuse`, `forlist_safe` macros. |
| `astring.h` / `astring.cpp` | `AString`, the legacy string with `gettoken()`, `value()`, `Trunc()`; plus modern `std::string` helpers (`join`, `plural`, `capitalize`, `startsWith`). |
| `fileio.h` / `fileio.cpp` | `Ainfile`/`Aoutfile` (savegame), `Aorders` (order files), `Areport` (reports, tab-indented), `Arules` (rules HTML). |
| `gameio.h` / `gameio.cpp` | Console I/O (`Awrite`, `Adot`) and the RNG facade: `getrandom`, `seedrandom`, `makeRoll`, `clamp`. |
| `i_rand.cpp` / `i_rand.h` | ISAAC PRNG backing `getrandom`. Determinism here is what makes snapshot tests reproducible. |
| `helper.h` | `ATL_VER` packing macros, bit macros. |
| `modify.cpp` | The `Enable*`/`Disable*`/`Modify*` API rulesets use to rewrite global tables at startup. |
| `gamedata.cpp` / `gamedata.h` | ~148KB of master tables plus the enums that index them. See `data-tables.md`. |
| `gamedefs.cpp` / `gamedefs.h` | The `GameDefs` class (every tunable), month names, direction tables. |

## 3. Core classes

**`Game`** (`game.h`) — the god object. Holds `regions`, `factions`, `battles`, the unit
lookup array `ppUnits`, `year`/`month`, and `events`. Nearly every verb in the codebase is
a `Game` method, which is why `game.h` is 600 lines of declarations. Spell methods are
injected via the `spells.h` include trick.

**`ARegion`** (`aregion.h`) — one hex. Terrain `type`, `race`, `population`, `wealth`,
`wages`, optional `town` (`TownInfo*`), `products`, `markets`, `objects`, `neighbors[NDIRS]`,
plus environment fields used by the newer economy (`habitat`, `development`, `elevation`,
`humidity`, `temperature`, `vegetation`, `culture`). `hell` holds units killed this turn;
`farsees` and `passers` support the visibility system.

**`ARegionList`** (`aregion.h`) — all regions, organised into `ARegionArray` levels
(surface, underworld, …). Owns level scaling, region typing, and pathfinding. Several of
its methods (`GetRegType`, `GetLevelXScale`, `CheckRegionExit`, `GetWeather`,
`GetStartingCity`) are **ruleset-defined** in `<game>/world.cpp`.

**`Object`** (`object.h`) — a building, a fleet, or the `O_DUMMY` pseudo-object. Holds the
units inside it. Fleets carry a `ships` list and derive capacity/speed from it.

**`Unit`** (`unit.h`) — a stack of men of one race under one faction. `items`, `skills`,
`flags` (behind, guard, avoid, spoils preferences, sharing), movement bookkeeping
(`movepoints`, `moved`, `phase`), and one slot per order category. Rule-set-specific
methods (`GetAttribute`, `GetProductionBonus`) live in `<game>/extra.cpp`.

**`Faction`** (`faction.h`) — a player. `type` is a map of faction-type string → points
(`F_WAR`, `F_TRADE`, `F_MAGIC`, `F_MARTIAL`). Accumulates `errors`, `events`, `battles`,
`shows` during the turn; these are drained into the report. `activity` tracks per-region
TAX/TRADE usage against the faction's budget.

**`Battle`/`Army`/`Soldier`** — a battle snapshots units into `Soldier`s, arranges them
into front/back lines, runs a free round then normal rounds, and writes results back
(deaths, spoils, experience).

## 4. Ownership and lifetime

- `AList` **owns** its elements: `~AList` and `DeleteAll()` delete them. Removing an
  element without deleting it is `Empty()`/`Remove()`.
- Units live in `Object::units`; objects in `ARegion::objects`; regions in `ARegionList`;
  factions in `Game::factions`. To iterate all units you must nest three loops.
- `Game::ppUnits` is a flat index from unit number to `Unit*`, rebuilt by
  `SetupUnitNums()`. It does **not** own the units.
- Killed units are moved to `ARegion::hell`, not freed, because battle reports and
  farsight can still reference them. `EmptyHell()` runs in `RunGame()` *after* reports
  are written — the comment in `RunOrders()` explains that moving it earlier caused
  segfaults with `IMPROVED_FARSIGHT`.
- Mutating a list while iterating it requires `forlist_safe`, which snapshots the element
  pointers first. Using plain `forlist` while deleting is the single most common crash
  source in this codebase.
- `AString*` fields (`name`, `describe`) are owned by their holder and deleted in the
  destructor; `SetName` deletes the old one.

## 5. The ruleset hook surface

Declared in the engine, defined per ruleset. Adding a hook means implementing it in **all
six** game directories plus `unittest/`, or the link fails.

> **Warning about stale comments.** `game.h` still carries comments like "These are game
> specific, and can be found in extra.cpp" above `AllowedMages`, `CheckUnitMaintenance`,
> `Upgrade*Version`, `MidProcessUnitExtra`, `Unit::GetAttribute` and others. On this
> branch those were consolidated **into the engine** (`game.cpp`, `unit.cpp`,
> `runorders.cpp`) and parameterised by data from `rules.cpp`. Always confirm with grep
> where a hook actually lives before assuming the comment is right.

The verified per-ruleset surface for NewOrigins today:

`neworigins/rules.cpp`
- `static GameDefs g = { … }; GameDefs *Globals = &g;` — a positional initialiser whose
  field order must match `gamedefs.h` exactly. Adding a `GameDefs` field means adding a
  value at the same position in all six rulesets **and** `unittest/rules.cpp`.
- The faction-limit tables: `allowedMages`, `allowedApprentices`, `allowedTaxes`,
  `allowedTrades`, `allowedQuartermasters`, `allowedTacticians`, `allowedMartial`, each
  with a `…Size` companion. Indexed by faction type points; `Game::AllowedMages` and
  friends in `game.cpp` clamp the index and read these arrays.

`neworigins/extra.cpp` — only three engine hooks
- `Game::SetupFaction` — starting unit, skills, money, food, start location, monster attitude.
- `Game::CheckVictory` — exploration tracking, quest spawning, endgame condition.
- `Game::ModifyTablesPerRuleset` — the table rewrite described in `data-tables.md`.

`neworigins/world.cpp`
- `Game::CreateWorld`
- `ARegionList::GetRegType`, `GetLevelXScale`, `GetLevelYScale`, `CheckRegionExit`,
  `GetWeather`, `GetStartingCity`
- `ARegion::CanBeStartingCity`, `MakeStartingCity`, `IsStartingCity`, `IsSafeRegion`
- `AGetName`, `AGetNameString`, plus local `SetupNames`/`CountNames`

`neworigins/map.cpp`
- The `MapBuilder` / `Zone` / `Province` / `ZoneRegion` generator, and the
  `ARegionList::Create*Level`, `MakeLand`, `GrowTerrain`, `RaceAnchors`, gate setup and
  shaft-linking family.

`neworigins/monsters.cpp`
- `Game::CreateVMons`, `Game::GrowVMons` — both effectively stubs here.

Engine-side but ruleset-flavoured (change these only with all six rulesets in mind):
`Game::AllowedMages/Apprentices/QuarterMasters/Tacticians/Taxes/Trades/Martial`,
`Game::ActivityCheck`, `Game::CreateNPCFactions`, `Game::AssessMaintenance` and the
`CheckUnit/Faction/Ally Maintenance|Hunger` family, `Game::MidProcessUnitExtra` /
`PostProcessUnitExtra` (both thin wrappers around `MonsterCheck`),
`Game::Upgrade{Major,Minor,Patch}Version`, `Unit::GetAttribute`,
`Unit::PracticeAttribute`, `Unit::GetProductionBonus`.

## 6. Report generation path

```
Game::WriteReport()
  MakeFactionReportLists()      which regions each faction can see
  CountAllSpecialists()         mage/apprentice/QM/tactician counts
  CountItems()                  faction statistics, if enabled
  Faction::WriteReport()        faction.cpp — header, errors, events, battles, shows,
                                then per-region ARegion::WriteReport()
    ARegion::WriteReport()      aregion.cpp — terrain, weather, wages, products,
                                markets, exits, then units
      Unit::WriteReport()       unit.cpp — name, items, skills, orders
Game::WriteTemplates()
  Faction::WriteTemplate() → ARegion::WriteTemplate()  template.cpp
```

All of it goes through `Areport`, which manages tab indentation and line wrapping. Because
snapshot tests diff these files byte-for-byte, **any change to report wording is a
snapshot change**. That is intentional: it forces a human to confirm the new text.

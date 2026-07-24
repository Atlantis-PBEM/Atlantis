---
name: atlantis-codebase
description: Required orientation for the Atlantis PBEM C++ game engine — architecture, engine/ruleset boundary, the fixed turn pipeline, global data tables, savegame protocol, and the mandatory build/verify workflow. Load this BEFORE reading or editing any file in this repository: any .cpp/.h edit, bug fix, feature, refactor, or question like "how does combat/movement/taxing/production work", "where is X handled", "why does the report say Y". The engine is shared by six rulesets and its turn-phase order, hand-rolled AList/AString primitives, and byte-exact snapshot tests punish edits made without this context. For writing or changing tests specifically, use the atlantis-testing skill as well.
---

# Atlantis engine — orientation and working rules

Atlantis is a play-by-email fantasy strategy engine. Players mail in text orders; once
per turn the GM runs the binary, which loads the entire world from `game.in`, applies
every faction's orders in a fixed sequence, and writes new reports plus `game.out`.

Two properties shape almost every decision you will make here:

1. **The engine is shared.** One set of root-level `.cpp` files is compiled into six
   different game binaries. A change in the root affects all of them, and CI compiles
   all of them with `-Wall -Werror`.
2. **Output is the contract.** Snapshot tests replay 14 stored NewOrigins turns and
   diff the generated reports byte-for-byte. Any change that shifts a random draw, a
   phase order, or a line of report text shows up as a failing diff — which is a
   feature, not a nuisance. It is the main safety net for a codebase with very thin
   unit-test coverage.

We build **NewOrigins** only. The other rulesets exist as reference and as compile
targets; do not add features to them.

## Engine vs. ruleset — the most important boundary

```
Atlantis/
├── *.cpp, *.h          ENGINE — shared by all six binaries. Ruleset-neutral.
│                       Behaviour here must be driven by Globals->… flags, never by
│                       "if this is NewOrigins".
├── gamedata.cpp/.h     Master data tables + the enums that index them (shared).
├── neworigins/         THE RULESET WE DEVELOP. Five files, always the same five:
│   ├── rules.cpp         Globals — the GameDefs struct instance. Tuning knobs.
│   ├── extra.cpp         Faction setup, maintenance, faction limits, victory
│   │                     conditions, quests, ModifyTablesPerRuleset().
│   ├── world.cpp         World creation, map levels, region typing.
│   ├── map.cpp           Procedural map generator (zones/provinces/biomes).
│   └── monsters.cpp      Ruleset-specific monster hooks (mostly stubbed here).
├── basic/ standard/ kingdoms/ fracas/ havilah/   Reference only. Do not develop.
├── unittest/           Boost.UT suites — AND itself a ruleset dir with the same
│                       five stub files, so the engine links standalone.
├── snapshot-tests/     Recorded turn inputs/outputs; the real regression suite.
├── external/boost/     Vendored single-header Boost.UT.
└── GAMEMASTER.md       GM-facing operational docs.
```

The engine declares the hooks; each ruleset directory defines them. `Game::SetupFaction`,
`Game::CheckVictory`, `Game::ModifyTablesPerRuleset`, `Game::CreateWorld`,
`ARegionList::GetRegType` and friends are declared in `game.h` / `aregion.h` but
implemented per ruleset. If you put a NewOrigins rule in the engine, the other five
binaries either break or silently change behaviour.

Be careful with the comments in `game.h`: several say a function "can be found in
extra.cpp" when on this branch it was moved into the engine and parameterised by data
from `rules.cpp` (`AllowedMages`, the maintenance family, `Unit::GetAttribute`,
`Upgrade*Version`). Grep for the definition rather than trusting the comment.

**Rule of thumb:** if the answer to "should Havilah do this too?" is no, it belongs in
`neworigins/`. If it is yes-but-configurably, add a `GameDefs` field and default it off
for every ruleset except NewOrigins.

## The turn pipeline

`main.cpp` dispatches subcommands (`new`, `run`, `edit`, `check`, `map`, `mapunits`,
`genrules`). `run` is the one that matters:

```
OpenGame()          read game.in (world, factions, regions, units, objects)
RunGame()
  PreProcessTurn()  month++/year++, SetupUnitNums, per-faction & per-region DefaultOrders
  ReadPlayers()     players.in
  ReadOrders()      orders.<factionnum> → parseorders.cpp
  RunOrders()       ~30 fixed phases (see references/turn-pipeline.md)
  WriteWorldEvents()→ times.<n>
  WriteReport()     → report.<n>
  WriteTemplates()  → template.<n>
  WritePlayers()    → players.out
SaveGame()          → game.out
```

`Game::RunOrders()` in `runorders.cpp:30` is the spine of the game. Its phase order is
load-bearing game design, not an implementation detail: FIND → ENTER/LEAVE →
PROMOTE/EVICT → combat → STEAL/ASSASSINATE → GIVE → EXCHANGE → DESTROY → PILLAGE → TAX →
GUARD → CAST → SELL → BUY → FORGET → mid-turn → QUIT → WITHDRAW → movement → TEACH →
month-long orders → economics → TELEPORT → TRANSPORT → maintenance → migration →
post-turn. Selling happens before buying so players can fund purchases; taxing happens
after combat so the winner collects. Read `references/turn-pipeline.md` before you move,
add, or reorder anything in that function.

## Core object graph

Everything hangs off `Game`, and ownership follows containment:

```
Game
├── regions   ARegionList → ARegion         aregion.h
│   └── objects  AList<Object>              object.h   (buildings, fleets, O_DUMMY)
│       └── units AList<Unit>               unit.h     ← units always live in an Object
│   ├── products ProductionList             production.h
│   ├── markets  MarketList                 market.h
│   ├── town     TownInfo*                  (nullable)
│   └── hell     AList<Unit>                dead units, drained by EmptyHell()
├── factions  AList<Faction>                faction.h
└── events    Events*                       events.h   world-events feed for times.<n>
```

A `Unit` carries `faction`, `object`, `items` (`ItemList`), `skills` (`SkillList`), and one
parsed order object per order category (`monthorders`, `castorders`, `giveorders`,
`buyorders`, …) defined in `orders.h`. Parsing fills those slots; `runorders.cpp` and
`monthorders.cpp` drain them; `Unit::ClearOrders` resets them.

Every unit is inside an `Object` even in open terrain — that is what `ARegion::GetDummy()`
returns. Code that iterates units must go region → objects → units.

## Where do I change what?

| Task | Go to |
|---|---|
| Tune a number (costs, income, limits, world size) | `neworigins/rules.cpp` — the `GameDefs` struct |
| Add/enable an item, skill, object, terrain, monster | `gamedata.cpp` table + `gamedata.h` enum, then enable in `neworigins/extra.cpp` |
| Change what an order does | `runorders.cpp` (instant) or `monthorders.cpp` (month-long) |
| Change how an order is parsed/validated | `parseorders.cpp` + `orders.h` |
| Combat mechanics | `battle.cpp` (rounds), `army.cpp` (soldiers/armies), `specials.cpp` (magic attacks) |
| Spells | `spells.cpp`; declarations via the `spells.h` include-into-class trick |
| Region economy, wages, population, markets | `economy.cpp`, `market.cpp` |
| Movement, production, building, study, teach | `monthorders.cpp` |
| Report text / templates | `faction.cpp`, `aregion.cpp`, `unit.cpp`, `template.cpp` |
| Player-facing rules HTML | `genrules.cpp`, `skillshows.cpp` |
| Faction setup, victory, quests, maintenance | `neworigins/extra.cpp` |
| Map generation | `neworigins/map.cpp`, `neworigins/world.cpp`, `mapgen.cpp` |
| Save/load format | `Writeout`/`Readin` pairs + `CURRENT_ATL_VER` in `game.h` |

## Before you call a change done

These are not ceremony — each one catches a class of failure that is expensive to find later.

1. **Build the ruleset and the tests.** `-Werror` means a stray unused variable fails CI.
   ```bash
   make neworigins && make unittest && ./unittest/unittest
   ```
2. **Run the snapshot tests** whenever behaviour, ordering, randomness, or report text
   could have moved.
   ```bash
   ./snapshot-tests/run-snapshots.sh
   ```
   If the diff is non-empty, read it line by line and confirm every changed line is an
   intended consequence of your change. Then **stop and show the user the diff and ask
   before regenerating baselines** — silently refreshing snapshots is how a real
   regression gets committed as "expected".
3. **Check the other five rulesets still compile** if you touched anything in the root
   or in `gamedata.*`: `make all`.
4. **Update the player-facing rules** (`genrules.cpp`, `skillshows.cpp`) in the same
   change when you alter mechanics, items, skills, objects, or `Globals`. The rules HTML
   is generated from that source; leaving it stale ships misinformation to players.
   Mention `GAMEMASTER.md` to the user when GM-visible behaviour changes.
5. **Look for the missing test.** Coverage is thin (`unittest/` has three suites today).
   If your change touches logic that can be exercised without a full world, say so and
   ask the user whether to add a suite. Load the **`atlantis-testing`** skill before writing
   or changing any test — it covers the Boost.UT conventions, the constraints imposed by
   `unittest/` being its own ruleset stub, and which layers a given change needs.

## Hazards worth knowing before you edit

**Turn-phase order.** Covered above. Inserting a phase or moving a call inside
`RunOrders()` changes outcomes across every game in progress.

**Memory ownership.** Raw `new`/`delete`; an `AList` owns and deletes its elements.
Deleting during a plain `forlist` is a use-after-free — `forlist_safe` exists precisely
for that, and dead units are moved to `ARegion::hell` rather than freed mid-turn because
farsight and battle reports still reference them. `alist.h` is 50 lines; read it once.

**Parallel enums and tables.** `ItemDefs`, `SkillDefs`, `ObjectDefs`, `TerrainDefs`,
`MonDefs` … are flat arrays in `gamedata.cpp` indexed by the `I_*`, `S_*`, `O_*`, `R_*`
enums in `gamedata.h`. The compiler will not tell you when they drift out of lockstep;
the game will just hand out the wrong item. Insert new entries at the same position in
both, and prefer appending. Savegames store items, skills, objects, terrain and races by
their **name/abbreviation string**, not by index — so reordering enums is survivable but
renaming an `abr` silently invalidates every stored game.

**Table mutation at startup.** `Game::ModifyTablesPerRuleset()` in `neworigins/extra.cpp`
runs before anything else and rewrites the global tables through `modify.cpp` helpers
(`EnableItem`, `DisableSkill`, `ModifyObjectConstruction`, …). The values in
`gamedata.cpp` are defaults, not the effective ruleset. Always check whether the entry
you care about is enabled or overridden there before concluding what the game does.

**Savegame protocol.** `Writeout` and `Readin` are positional and must be edited together
in identical order — there are no field names in `game.out`, only a sequence. A mismatch
does not error, it silently misreads the rest of the file. Two separate version gates
apply (`CURRENT_ATL_VER` in `game.h` for the engine, `RULESET_VERSION` in
`neworigins/rules.cpp` for the ruleset) and they behave differently; the engine one has
no upgrade path at all, so bumping it invalidates every existing save including the 14
snapshot `game.in` files. Read `references/conventions.md` before changing anything
persisted.

**Style.** Follow the idiom of the file you are editing: legacy engine files use
`AList`/`forlist`/`AString` and tabs, and gratuitous modernisation there produces noisy
diffs and snapshot churn. Genuinely new, self-contained subsystems should use modern
C++20 and `std::` types — that is what `events.cpp`, `mapgen.cpp`, and `namegen.cpp`
already do.

## References

Read the one you need; they are not meant to be read all at once.

- `references/architecture.md` — file-by-file map of the engine, class responsibilities, ownership rules.
- `references/turn-pipeline.md` — the full phase sequence with what each phase does and why it sits there.
- `references/data-tables.md` — every global table, its enum, and recipes for adding an item / skill / object / terrain.
- `references/ruleset-neworigins.md` — `Globals` walkthrough, `extra.cpp` hooks, quests, victory, map generation.
- `references/conventions.md` — AList/AString/memory idioms, savegame protocol, style, report-text discipline.

Testing is a separate skill: load **`atlantis-testing`** for anything involving `*_test.cpp`,
the snapshot suite, or deciding what coverage a change needs.

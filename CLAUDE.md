# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Atlantis is a text-based, play-by-email (PBEM) fantasy strategy game engine written in C++20. This is maintained primarily to support **Atlantis New Origins**
(the `neworigins` ruleset), but the repo builds several rulesets that share one game engine. There is no single game — the engine is a library of mechanics, and each ruleset directory supplies data tables, map generation, and rule tweaks on top of it.

## Build

```bash
make <ruleset>
```

Where `<ruleset>` is one of `basic`, `standard`, `fracas`, `kingdoms`, `havilah`, `neworigins`. This produces a binary at `<ruleset>/<ruleset>`. Engine `.o` files go in `obj/`, ruleset-specific `.o` files go in `<ruleset>/obj/`.

```bash
make all              # build every ruleset + unittest
make <ruleset>-clean   # remove build artifacts for one ruleset
make all-clean         # clean everything
make <ruleset>-rules   # regenerate that ruleset's HTML rules doc from genrules
```

Compiler is fixed at `g++ -std=c++20 -Wall -Werror` (see [Makefile](Makefile)) — warnings are
treated as errors, so a build with any warning fails.

## Tests

### Unit tests (Boost.UT-based, in `unittest/`)

```bash
make unittest
cd unittest && ./unittest
```

Each `*_test.cpp` file in `unittest/` is a self-registering Boost.UT suite (framework header at
`external/boost/ut.hpp`); there's no central test list to update — adding a new `*_test.cpp` file is picked up automatically via the Makefile's wildcard. `unittest/testhelper.hpp` / `.cpp` provide `UnitTestHelper`, a friend class of `Game` used to reach into engine internals from tests.
`unittest/extra.cpp`, `map.cpp`, `monsters.cpp`, `rules.cpp`, `world.cpp` are a minimal dummy
ruleset so the engine links for testing.

### Snapshot tests (`snapshot-tests/`)

Replay recorded turns through a built ruleset binary and diff the output byte-for-byte against
checked-in expected output:

```bash
cd snapshot-tests
./run-snapshots.sh                    # everything: game snapshots + rules snapshots, all rulesets
./run-game-snapshots.sh [ruleset]     # replay turns/ (or <ruleset>_turns/) — default ruleset is standard
./run-rules-snapshot.sh [ruleset]     # regenerate genrules HTML and diff against rules/<ruleset>.html
```

These require the ruleset binary to already be built (`make <ruleset>`) and copied/placed as
described in each script. When engine behavior intentionally changes, the corresponding
`update-*-snapshots.sh` script regenerates the checked-in expected output — don't hand-edit
snapshot fixtures.

### Order-checking smoke test

`atlantis check <orderfile> <checkfile>` (via `game.do_orders_check`) parses a player's orders file
against a dummy game/faction and reports errors without running a real turn — useful for validating
order syntax changes in `parseorders.cpp` quickly.

## Architecture

### Engine vs. ruleset split

The engine (root-level `.cpp`/`.h` files, compiled once into `ENGINE_OBJECTS` per the Makefile) is shared code: world model, orders processing, combat, economy, magic. Each ruleset directory
(`basic/`, `standard/`, `fracas/`, `kingdoms/`, `havilah/`, `neworigins/`, `unittest/`) supplies
exactly five files that the engine calls into or that customize behavior:

- `extra.cpp` — implements `Game::SetupFaction`, `Game::CheckVictory`, and
  `Game::ModifyTablesPerRuleset` (item/skill/object table tweaks, victory conditions, faction
  setup hooks specific to that ruleset).
- `world.cpp` — world/map generation for that ruleset (terrain layout, races, etc).
- `map.cpp` — map-file writing logic (hex/geo/wmon/lair/gate views).
- `monsters.cpp` — monster spawn tables for that ruleset.
- `rules.cpp` — ruleset-specific numeric tables referenced from `gamedata.h`/`gamedefs.h` globals (allowed mages/apprentices/taxes/trades/etc per faction size).

`GameDefs` (in [gamedefs.h](gamedefs.h), instantiated as the global `Globals`) is the central knob panel — most ruleset behavior differences (fleet mechanics, activity rules, destroy behavior, maintenance costs, etc.) are flags/values on this struct set from each ruleset's `extra.cpp`, rather than `#ifdef`s scattered through the engine.

`neworigins/` is by far the largest and most actively developed ruleset (extra.cpp alone is ~1600 lines vs. ~150-500 for the others) — when a change is ambiguous about which ruleset(s) it should touch, `neworigins` is the one this fork actually cares about running.

### Core data model

- [aregion.h](aregion.h) / [aregion.cpp](aregion.cpp) — `ARegion` (a hex on the map), `ARegionList`, `ARegionArray` (one level of the map, e.g. surface/underworld). Regions own objects, which own units.
- [object.h](object.h) / [object.cpp](object.cpp) — `Object`, a structure/ship/etc within a region that holds units.
- [unit.h](unit.h) / [unit.cpp](unit.cpp) — `Unit`, the base actor: has items, skills, orders,
  belongs to a `Faction`.
- [faction.h](faction.h) / [faction.cpp](faction.cpp) — `Faction`, a player.
- [items.h](items.h)/[skills.h](skills.h) + `gamedata.cpp` (huge — item/skill/object/monster
  definition tables) — the data-driven definitions engine code looks up by index/tag.

### Orders pipeline (per-turn flow, driven by `Game::RunGame` -> `Game::RunOrders` in `game.cpp`)

1. `parseorders.cpp` — parses a faction's order text into `Order` objects on units.
2. `monthorders.cpp` — the once-a-month order phase (movement, production, building, etc. resolve in a fixed phase order).
3. `runorders.cpp` — executes the resolved orders against the world state.
4. `battle.cpp` / `events-battle.cpp` — combat resolution.
5. `economy.cpp` / `market.cpp` / `production.cpp` — trade goods, markets, production.
6. `text_report_generator.cpp` (+ `indenter.cpp`/`.hpp`) — renders each faction's turn report;
   `json_report_test.cpp` in `unittest/` covers the JSON report path (`JSON_REPORT_VERSION` in
   `game.h`).
7. `events.cpp` / `events-assassination.cpp` — non-combat scripted events.

`quests.cpp`, `npc.cpp`, `army.cpp`, `spells.cpp`/`specials.cpp`, `skillshows.cpp` are supporting subsystems (quest tracking, NPC/monster AI, army composition for battle, magic, in-game skill description text) plumbed through the same Unit/Faction/ARegion model.

`modify.cpp` and `edit.cpp` back the `atlantis edit` GM console for hand-editing a live game.
`namegen.cpp` / `mapgen.cpp` / `simplex.cpp` back procedural world/name generation used by each
ruleset's `world.cpp`.

### Supporting utilities

- `astring.h`/`.cpp` — legacy `AString` type; actively being removed in favor of `std::string` (see recent commit history — "AString cleanup" is an ongoing effort). Prefer `std::string` and the helpers in `strings_util.hpp`/`string_filters.hpp`/`string_parser.hpp` in new code rather than reaching for `AString`.
- `rng.hpp` — the engine's seeded RNG wrapper; `main()` seeds it with a fixed historical seed before `NewGame()` reseeds randomly, which is why snapshot tests can replay deterministically.
- `safe_list.h` — bounds-checked list wrapper used throughout the engine (see `safe_list_test.cpp`).

### Non-engine subproject

`map_viewer/` is an independent TypeScript/Parcel web app (Yarn workspace, not part of the C++
build) for visually inspecting a `hexmap.json` produced by `atlantis map hex hexmap.json`. See
[map_viewer/README.md](map_viewer/README.md) for its own `yarn start` workflow.

## Style notes

- C++20, 4-space indent, K&R bracing for control structures / Allman for classes (see
  [.editorconfig](.editorconfig)); max line length 120.
- Build is `-Wall -Werror`: a change that compiles with warnings will fail `make`.
- `external/boost/ut.hpp` and `external/nlohmann/json.hpp` are vendored single-header libraries; use `make check-libraries` to check/update them from upstream rather than hand-editing.

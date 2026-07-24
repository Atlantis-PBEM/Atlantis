# Conventions, idioms, and the savegame protocol

Contents:
1. Which idiom to use where
2. `AList` and the forlist macros
3. `AString`
4. Memory ownership
5. Randomness and determinism
6. The savegame protocol
7. Report text discipline
8. Compiler and style constraints

---

## 1. Which idiom to use where

The codebase has two generations of C++ living side by side, and both are legitimate.

**Inside existing engine files** (`unit.cpp`, `aregion.cpp`, `runorders.cpp`,
`parseorders.cpp`, `game.cpp`, `battle.cpp`, …) follow the local idiom: `AList` +
`forlist`, `AString`, raw `new`/`delete`, tabs for indentation. This is not nostalgia —
these files are large, the diffs are reviewed by humans, and mixing container styles
inside one function makes ownership harder to reason about. Rewriting a loop you happened
to walk past also perturbs nothing useful and risks snapshot churn.

**In genuinely new, self-contained subsystems** use modern C++20 and `std::` types. The
precedent already exists: `events.cpp` / `events-battle.cpp` (`std::string`, `std::list`,
`std::vector`, namespaces), `mapgen.cpp` and `neworigins/map.cpp` (`std::unordered_set`,
`std::map`, lambdas, `struct`s with methods), `namegen.cpp` (`std::string` API). Newer
fields on old classes follow suit — `Unit::visited` is a `set<string>`,
`Faction::activity` is an `unordered_map`, `Faction::type` is an
`unordered_map<std::string,int>`.

The boundary rule: a new file or a new class can be modern; an existing function should
stay consistent with itself.

## 2. `AList` and the forlist macros

`alist.h` is ~50 lines. `AList` is an intrusive singly-linked list: elements derive from
`AListElem` and carry their own `next` pointer, which means **one object can only be in one
`AList` at a time**. Lists of pointers-to-shared-things use wrapper elements
(`UnitPtr`, `FactionPtr`, `ARegionPtr`, `BattlePtr`) precisely for that reason.

Three iteration macros:

```cpp
forlist(&r->objects) { Object *o = (Object *) elem; … }   // declares elem, _elem2
forlist_reuse(&other)  { … }                              // reuses elem/_elem2 already in scope
forlist_safe(&o->units) { Unit *u = (Unit *) elem; … }    // snapshots pointers first
```

- `forlist` pre-fetches `next` before the body runs, so it tolerates deleting the *current*
  element but not arbitrary mutation of the list.
- Nesting `forlist` inside `forlist` works — the inner one shadows `elem`. This is why the
  region→object→unit triple loop is written the way it is everywhere.
- `forlist_reuse` exists because `forlist` re-declares `elem`; use it for a second loop at
  the same scope level (see `Game::PreProcessTurn`).
- **`forlist_safe` when the body can delete or move elements.** It copies the element
  pointers into an array first and uses `NextLive` to skip entries that vanished. Getting
  this wrong is the classic crash in this codebase.

`AList` owns its elements: the destructor and `DeleteAll()` free them. `Empty()` clears
without deleting; `Remove(elem)` unlinks without deleting.

Casting is manual and unchecked (`(Unit *) elem`). Putting the wrong type in a list is
undefined behaviour with no diagnostic, so keep list contents homogeneous.

## 3. `AString`

A hand-rolled string with a tokenising API built for parsing order files:

```cpp
AString *line  = f.GetStr();
AString *token = line->gettoken();   // caller owns the result — delete it
int n          = token->value();     // 0 if not numeric; strict_value() is stricter
```

Points that trip people up:

- `gettoken()` returns a heap `AString*` you must `delete`; it also consumes from the
  source string. Most parse bugs in this codebase are a missing `delete` or a token used
  after the source was freed.
- `AString` has `operator+` / `operator+=` and constructors from `int`, so report lines are
  built as `AString("Mages: ") + nummages + " (" + max + ")"`.
- `getlegal()` strips characters illegal in names — that is why `SetName` turns
  `"Test Unit || bar"` into `"Test Unit  bar (500)"`.
- `Trunc(n, back)` truncates for report width.
- For new code, `astring.h` also exposes `std::string` helpers: `join`, `plural`,
  `capitalize`, `startsWith`, `endsWith`. Prefer these when you are not parsing.

## 4. Memory ownership

- `AString*` members (`name`, `describe`) are owned by their object; `SetName`/`SetDescribe`
  delete the previous value. Never assign these directly.
- Order objects attached to a `Unit` are owned by the unit and freed in `ClearOrders` or
  by the phase that consumes them (e.g. `RunCastOrders` deletes `castorders` after use).
- Dead units go to `ARegion::hell` and are freed by `EmptyHell()` **after** reports are
  written. Do not "tidy" this by deleting on death: farsight and battle reports still
  dereference those pointers, which is documented in a comment in `RunOrders()`.
- `Game::ppUnits` is a non-owning index rebuilt by `SetupUnitNums()`. If you create or
  destroy units outside the normal paths, that index goes stale.
- There are no smart pointers in the engine. Adding them piecemeal to a class whose
  lifetime is managed by `AList` will double-free.

## 5. Randomness and determinism

`getrandom(n)` / `makeRoll(rolls, sides)` in `gameio.cpp` wrap the ISAAC PRNG in
`i_rand.cpp`. The seed is stored in the savegame (`SaveGame` writes
`f.PutInt(getrandom(10000))`, `OpenGame` calls `seedrandom` with it), which is what makes
snapshot replays reproducible.

Consequence: **any change to how many random numbers are drawn, or in what order, changes
every subsequent draw for the rest of the turn.** Adding a `getrandom` call inside a loop
that runs before combat will alter combat outcomes across the whole snapshot suite even
though your change looks unrelated. When a snapshot diff is much larger than expected,
suspect an RNG consumption change first.

## 6. The savegame protocol

`game.out` (renamed to `game.in` for the next turn) is a positional text file. There are no
field names — only an ordered sequence of ints and strings written by `Aoutfile::PutInt` /
`PutStr` and read back by `Ainfile::GetInt` / `GetStr`.

**Rule 1 — `Writeout` and `Readin` change together, in the same order.** `Game::OpenGame`
carries the comment "The order here must match the order in SaveGame". A mismatch does not
throw; it silently misreads everything downstream, so the failure shows up as a corrupted
world rather than a load error.

**Rule 2 — conditional writes must be conditional on both sides.** Several fields are
written only under a `Globals` flag:

```cpp
if (Globals->PREVENT_SAIL_THROUGH && !Globals->ALLOW_TRIVIAL_PORTAGE)
    f->PutInt(prevdir);
else
    f->PutInt(-1);
```

Note the pattern: this one always writes *something* so the layout stays fixed. Where a
field is genuinely omitted (`if (gate > 0) f->PutInt(gatemonth);`), the reader must apply
the identical condition. Changing a `Globals` flag that gates a write therefore changes the
file layout and breaks old saves.

**Rule 3 — big tables persist symbolically, so names are a wire format.** Items and races
are stored as `ItemDefs[].abr`, skills as `SkillDefs[].abbr`, objects as
`ObjectDefs[].name`, terrain as `TerrainDefs[].type`. Reordering an enum is therefore
survivable; renaming an abbreviation is not — `LookupItem`/`LookupSkill` will fail to
resolve the stored token and `ItemList::Readin` silently drops the item. The same
mechanism means **disabling an item destroys every stored copy of it on next load**,
because `Readin` discards items whose type is `DISABLED`.

**Rule 4 — know which version gate you are touching.** They are two different things and
behave differently:

| Gate | Where | On load |
|---|---|---|
| `CURRENT_ATL_VER` (engine, currently 5.2.5) | `game.h` | Major or minor mismatch → "Incompatible Engine versions!" and **abort**. A newer patch level than the binary → abort. There is **no upgrade path**. |
| `Globals->RULESET_VERSION` | `neworigins/rules.cpp` | An older saved ruleset version calls `Upgrade{Major,Minor,Patch}Version` in `game.cpp`. Currently `UpgradeMajorVersion` returns 0 (abort), Minor and Patch return 1 (accept, no transformation). |

So: bumping the **engine** version invalidates every existing save, including all 14
snapshot `game.in` files, which then have to be regenerated from scratch. Bumping the
**ruleset** minor/patch version is the graceful lever, and `Upgrade*Version` is where a
migration would go.

**Rule 5 — the per-object `Readin` methods take an `ATL_VER v` parameter**
(`Unit::Readin`, `ARegion::Readin`, `Object::Readin`, `Faction::Readin`). That parameter is
the intended mechanism for version-conditional parsing, and today nothing uses it. If you
need to add a field while keeping old saves loadable, that is the lever: bump
`RULESET_VERSION`, and branch on `v` in `Readin`.

Because none of this is currently exercised, treat any change to the persisted format as a
decision worth surfacing to the user rather than making silently.

## 7. Report text discipline

Reports are the product. They are also the regression suite, since snapshot tests diff
them byte-for-byte.

- Build lines with `AString` concatenation and emit through `Areport::PutStr`; `AddTab` /
  `DropTab` manage indentation, so do not hand-roll leading spaces.
- Faction-visible messages accumulate during the turn via `Unit::Event` / `Unit::Error` and
  `Faction::Event` / `Faction::Error`, then drain into the report. Prefer these over
  writing directly.
- Pluralisation helpers exist (`plural`, `join` in `astring.h`); use them rather than
  ad-hoc `"s"` appending, so the text matches surrounding style.
- A wording change is a legitimate snapshot diff. Expect it, verify the diff contains only
  your wording change, and confirm with the user before regenerating baselines.

## 8. Compiler and style constraints

- `-std=c++20 -Wall -Werror`. Unused variables, sign-compare, and reordered initialisers
  are build failures. CI compiles all six binaries on Ubuntu and Windows.
- Indentation is tabs in engine files; newer files (`events.cpp`, `map.cpp`) use 4 spaces.
  Match the file.
- Headers use include guards (`#ifndef UNIT_CLASS`), not `#pragma once`, except in the
  newest files (`mapgen.h`, `namegen.h`). Match the file.
- Many headers forward-declare classes at the top before including each other — the
  include graph is circular by design (`unit.h` ↔ `faction.h` ↔ `aregion.h`). Adding a
  plain `#include` to break a compile error often creates a cycle; forward-declare instead.
- `using namespace std;` is at file scope in several headers (`astring.h`, `fileio.h`,
  `army.h`). Be aware of collisions — `unittest/*_test.cpp` cannot do
  `using namespace boost::ut` at file scope because `ut::events` collides with the game's
  `Events`, which is why the tests alias it as `namespace ut = boost::ut`.

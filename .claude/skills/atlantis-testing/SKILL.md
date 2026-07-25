---
name: atlantis-testing
description: Required guide for writing, changing, or running tests in the Atlantis PBEM C++ engine — Boost.UT unit suites in unittest/, the byte-exact snapshot suite in snapshot-tests/, and which layers a given change needs. Load this BEFORE creating or editing any *_test.cpp, adding assertions, touching unittest/ or snapshot-tests/, regenerating snapshot baselines, or investigating a failing/skipped test. Also load it when a code change needs test coverage decided. The vendored Boost.UT is pinned to an older release than its public docs, `tag()` silently disables tests, and untyped assertions produce undiagnosable failures — so tests written from general C++ knowledge tend to be quietly wrong here.
---

# Writing and running Atlantis tests

Three layers, and they protect different things:

| Layer | Command | Protects |
|---|---|---|
| Unit tests | `make unittest && ./unittest/unittest` | Isolated class and function behaviour |
| Snapshot tests | `./snapshot-tests/run-snapshots.sh` | End-to-end turn behaviour, byte-for-byte |
| Smoke test | `cd smoketest && python3 smoketest.py` | Long-run stability (legacy, not in CI) |

Unit coverage is thin — three suites at present — so the snapshot suite is carrying most of
the regression load. A clean snapshot run is the primary evidence that a change is safe;
a snapshot diff is information to read, not an obstacle to route around.

## Before you write a Boost.UT test: the version pin

The vendored header is **Boost.UT 2.0.0** (`external/boost/ut.hpp`, `#define BOOST_UT_VERSION 2'0'0`).
The project's public README documents a **newer** release (2.3.x). Features described there
may simply not exist here — `format_test_parameter` is a confirmed example that fails to
compile.

Everything in this skill was verified by compiling and running it against the vendored
header with the project's own flags (`-Wall -Werror -std=c++20`). If you want an API not
covered here, grep `external/boost/ut.hpp` for it before using it, rather than trusting the
upstream docs.

## How a test file gets built and run

```bash
make unittest && ./unittest/unittest
```

- Create `unittest/<thing>_test.cpp`. The Makefile globs `$(wildcard unittest/*_test.cpp)`,
  so **no Makefile edit is needed**. A file not ending in `_test.cpp` is silently ignored.
- The binary links the whole engine minus `obj/main.o`, plus `unittest/`'s five ruleset stub
  files and every `*_test.cpp`.
- Suites self-register at static-initialisation time. `unittest/main.cpp` is just `int main() {}`.
- Exit code 0 on success, non-zero on failure. CI keys off this.

**`unittest/` is itself a ruleset** (`GAME=unittest`), which constrains what you can test:

- `unittest/rules.cpp` declares its own `Globals`, `RULESET_NAME = "UnitTest Atlantis"`, and
  its own faction-limit arrays. It is **not** NewOrigins. If your test depends on a `Globals`
  value, that is the file it reads — and that file is shared by every suite in the binary.
- `unittest/world.cpp` stubs world generation: `CreateWorld()` is empty, `CheckRegionExit`
  always returns 1, levels are unscaled, weather is always `W_NORMAL`, no starting cities.
  **There is no world unless you build one by hand**, which is why world-dependent behaviour
  belongs in the snapshot suite instead.

## The namespace pattern is mandatory

```cpp
#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;
```

This is forced by a real symbol clash with `events.h`, not a style preference. Every existing
suite does it this way.

## Skeleton

```cpp
ut::suite<"Skill days"> skill_days_suite = []
{
  using namespace ut;

  // Runs once, at static-init time, when the suite object is constructed.
  Faction *faction = new Faction(1);
  Unit *unit = new Unit(500, faction, 12345);

  "unit id is set correctly"_test = [unit]
  {
    expect(unit->num == 500_i);
  };

  "SetName appends the unit number"_test = [unit]
  {
    unit->SetName(new AString("Test Unit"));
    string current = unit->name->Str();
    string expected = "Test Unit (500)";
    expect(eq(current, expected));
  };
};
```

## Assertions: always give the framework typed operands

This is the highest-value habit in the whole file, because it decides whether a failure is
diagnosable. Both of these compile:

```cpp
expect(1_i == 2);   // failure prints:  test condition: [1 == 2]
expect(1 == 2);     // failure prints:  test condition: [false]      ← tells you nothing
```

The information is lost at the moment you need it most. Two ways to keep it:

**Literal suffixes**, for compile-time constants:

| Suffix | Type | Suffix | Type |
|---|---|---|---|
| `_i` | int | `_u` | unsigned |
| `_s` | short | `_uc` | unsigned char |
| `_l` | long | `_us` | unsigned short |
| `_ll` | long long | `_ul` | unsigned long |
| `_c` | char | `_f` / `_d` / `_ld` | float / double / long double |

**Comparison functions**, for runtime values — and the only sane way to compare strings:

```cpp
expect(eq(a, b));    // ==        expect(gt(a, b));   // >
expect(neq(a, b));   // !=        expect(ge(a, b));   // >=
expect(lt(a, b));    // <         expect(le(a, b));   // <=
```

**`that %`** keeps operand reporting for a general boolean expression:

```cpp
expect(that % value > 0);
```

**Strings:** convert both sides to `std::string`. Comparing `char*` compares pointers, and
the framework cannot print a useful diff:

```cpp
string current = unit->name->Str();
string expected = "Test Unit (500)";
expect(eq(current, expected));
```

**Floating point** derives its epsilon from the decimal places in the literal, which
surprises people. Write the literal at the precision you actually mean to assert:

```cpp
expect(42.1_d  == 42.101);   // passes — one decimal place  → epsilon 0.1
expect(42.10_d == 42.101);   // passes — two decimal places → epsilon 0.01
```

**Messages** attach with `<<`:

```cpp
expect(eq(current, expected)) << "SetName should append the unit number";
```

## Fatal guards

A failed `expect` records the failure and keeps going. When continuing would crash — a null
pointer, an empty container you are about to index — mark the precondition fatal so the test
aborts instead of segfaulting the whole binary:

```cpp
std::vector<int> v(5);
expect(fatal(std::size(v) == 5_ul));   // stops this test if it fails
expect(v[4] == 0_i);                   // only reached if the guard held
```

Given how much of this engine is raw pointers, put a `fatal` on every "the object exists" /
"the list is non-empty" precondition. A crashed test binary reports nothing at all, so one
bad dereference can hide every other suite's results.

## Sections

`should(...)` blocks capture by value with `mutable`, so each gets its own copy of the
enclosing state and they cannot contaminate each other:

```cpp
"[vector]"_test = [] {
  std::vector<int> v(5);

  should("resize bigger") = [=]() mutable {
    v.resize(10);
    expect(std::size(v) == 10_ul);
  };

  should("resize smaller") = [=]() mutable {
    v.resize(0);
    expect(std::size(v) == 0_ul);
  };

  expect(std::size(v) == 5_ul);   // outer v untouched
};
```

Caution: this copies. For engine objects held by raw pointer, copying the *pointer* still
aliases the same object — the isolation is shallow. Construct a fresh object per section when
the mutation is destructive.

## `skip` works; `tag` silently disables

```cpp
skip / "not ready yet"_test = [] { … };     // reported as SKIPPED, as intended
```

**`tag(...)` will quietly stop your test from running.** Verified against the vendored header:
a tagged test executes only when the runner config selects that tag, and `unittest/main.cpp`
installs no config. So this never runs:

```cpp
tag("slow") / "expensive check"_test = [] { … };   // reported SKIPPED, not FAILED
```

Because it prints as SKIPPED rather than failing, it is easy to miss in a long run and you
end up believing you have coverage you do not. Avoid `tag` here unless you are also adding a
runner configuration.

## Constraints specific to this codebase

- **Suite bodies run at static-init time**, and ordering across translation units is
  unspecified. Never let one suite depend on another having run first.
- **Global state is shared across the whole binary.** `Globals`, `ItemDefs`, `SkillDefs` and
  the rest are process-wide. `faction_test.cpp` pushes `F_WAR`/`F_TRADE`/`F_MAGIC` into
  `FactionTypes` because `Game::Game()` normally does it — and that mutation is visible to
  every other suite. If your test mutates a global table, it can break an unrelated suite,
  and the failure will appear to belong to the other file.
- **Prefer pure functions.** Anything needing no world is a clean target: `GetLevelByDays` /
  `GetDaysByLevel`, `SkillCost`, `LookupItem` / `LookupSkill`, `AString` tokenising, capacity
  and weight arithmetic, name formatting, faction type strings.
- **"No world" means none is built _for_ you — not that you cannot build one by hand.** The
  unittest ruleset's `CreateWorld()` is empty, but `Globals` and all the global tables
  (`ItemDefs`, `SkillDefs`, `TerrainDefs`, …) still exist. A single region with the objects,
  units, products and markets you need is a legitimate, hand-constructible fixture. Reserve
  the snapshot suite for genuinely global behaviour — full map generation, multi-turn
  interactions, whole-report byte diffs — not for every method that happens to touch a
  region. See **Reaching world / report / RNG-dependent code** below.
- **Leaks are tolerated.** Existing suites `new` their fixtures and never delete. Do not add
  teardown the surrounding code does not expect.
- **Write an independent oracle.** When testing a formula, re-derive the expected value from
  the specification rather than calling the sibling engine function. Otherwise a consistently
  wrong pair of functions will round-trip its way to a green suite and prove nothing.
- **When writing tests, write only tests. Never change non-test code to make a test pass or
  read more nicely** — not even code that looks buggy. A test's job is to describe the engine
  as it is, so it can catch the day that behavior changes. Assert the *current* behavior and
  lock it in. If, while writing tests, you become convinced a non-test function is genuinely
  wrong, you may add a *comment* to that function (and to the test) flagging the behavior and
  pointing at the pinning test — but leave the code itself untouched and raise the suspected
  bug with the user as a separate, explicit decision. Trace surprising logic by hand before
  calling it a bug: `AList::Remove`'s `lastelem` handling reads like a bug but is correct, and
  a test written against the "obvious fix" would have been wrong. `unittest/alist_test.cpp` is
  the reference example of this behavior-locking style.

## Reaching world / report / RNG-dependent code

Methods that "need a world" are usually still unit-testable with one of three techniques.
Most of `aregion.cpp`'s once-deferred methods are covered this way (see the `aregion_*_test.cpp`
suites), leaving only true map generation to the snapshot suite.

**1. Capture report text through an `Areport` backed by a temp file.** Every `Write*` method
(`WriteReport`, `WriteEconomy`, `WriteProducts`, `WriteMarkets`, `WriteExits`, `WriteTemplate`)
emits into an `Areport`. Point one at a scratch file, call the method, read the file back, and
assert on the text. `ShortPrint`/`Print` return an `AString` directly — no capture needed.

```cpp
std::string capture(std::function<void(Areport*)> emit) {
    const char *scratch = "aregion_report.tmp";
    std::remove(scratch);                       // Areport::OpenByName refuses a non-empty file
    Areport rep; rep.OpenByName(scratch);
    emit(&rep);
    rep.Close();
    std::ifstream in(scratch); std::stringstream ss; ss << in.rdbuf();
    std::remove(scratch);
    return ss.str();
}
// expect(capture([&](Areport *r){ reg->WriteEconomy(r, fac, 1); }).find("Wages: $0.") != npos);
```

**2. Seed the RNG for anything using `getrandom()`.** `seedrandom(int)` (from `gameio.h`) makes
`Setup`, `SetupProds`/`SetupPop`, `LairCheck`/`MakeLair`, `FindGate(-1)` and the decay checks
reproducible. Do **not** pin the exact draw sequence — it is brittle. Instead assert (a) the
_reproducibility property_ — same seed ⇒ identical result, by running the method twice from the
same seed — and (b) the deterministic post-conditions that hold regardless of the draws (e.g.
`Setup` always creates the dummy object; population is never negative). Watch for constructors
that themselves draw: `Production(item, amt)` bumps `amount` by `getrandom()` under
`RANDOM_ECONOMY`, so seed first or set the field explicitly.

**3. Build the unit/faction fixtures by hand.** Guard/tax/observation/notify methods just walk
`region → objects → units`. Construct units, set `guard`/`type`/skills/items, wire faction
attitudes with `SetAttitude`, and call. (One caveat: attribute-driven paths like
`GetAttribute("observation")` return 0 unless the ruleset defines the attribute mods, which the
unittest ruleset does not — so `GetObservation` can only be exercised in its degenerate form.)

**Use the recorded turns as a data source.** `snapshot-tests/neworigins_turns/turn_*/` holds real
NewOrigins `game.in`/`game.out` and `report.*`/`template.*` files. Mine them — do **not** copy
them verbatim into a test:
- **Expected report strings:** grep a `report.*` for the exact wording your `Write*` assertion
  should match, so the format stays honest against the real ruleset (e.g. `Wages: $13.5 (Max:
  $595)`, `Products: none.`, `Exits:`). This keeps a unit test's text in sync with what players
  actually see.
- **Realistic fixture shapes:** read a populated region block to see plausible product/market
  layouts, wage productivities, populations and terrain, then reproduce a minimal version by
  hand. It shows you what "normal" values look like without guessing.
- **Format/field reference:** `game.in` is the positional savegame; consult it when testing
  `Readin`/`Writeout` round-trips to see field order and sentinels (`NO_RACE`, `none`).

Caveat on overlap: report-text unit tests partially duplicate the snapshot suite (which diffs
whole reports). They still earn their place — they are fast, target one method, and pin specific
edge cases the 14 recorded turns may never hit — but assert a focused substring, don't re-diff a
whole report the snapshot suite already owns.

## Covering a file or class: enumerate, don't cherry-pick

When the task is "test file X" or "cover class Y" (not a single function), coverage here is
**opt-in per method**: a method is only exercised if some test names it. The unittest binary
links into no game, so nothing else calls your code. This produces a specific, dangerous
blind spot — **dead code and logic-broken code that nothing calls are invisible to every
layer at once**: the snapshot suite never reaches an uncalled method, and the unit suite
won't either unless you deliberately wrote a test for it. A method can be outright wrong and
still show green across all 28 snapshot turns and the whole unit suite. (This really
happened: `ARegion::GetNearestProd` expanded the wrong region's neighbors for years, was
never called, and deleting it failed no test.)

So do not stop at the handful of obvious pure functions. Work the whole surface:

1. **Enumerate every method up front.** `grep -nE 'ClassName::' file.cpp` (and the class
   body in the header for methods defined elsewhere). That list — not your intuition — is
   the coverage denominator.
2. **Cross-reference call sites.** For each method, `grep -rn 'MethodName' --include=*.cpp`.
   Zero call sites on a non-trivial method is a finding in itself: it is dead code. Flag it
   for deletion or a pinning test; do not silently skip it as "not worth testing".
3. **Classify each method** into: *tested*, *hand-constructible gap* (reachable with
   `new ARegion()` + objects/units/factions built by hand — write the test), or
   *world-dependent* (needs regions/markets/RNG/reports — belongs to the snapshot suite).
   Private methods are usually only reachable through a world-dependent caller; note them
   rather than pretending they are covered.
4. **Report the inventory as a number, not a vibe.** "N methods; X tested; Y untested — Z of
   those are hand-constructible (listed), the rest are world-dependent." A per-method table
   is the deliverable, so the gaps are explicit and reviewable rather than assumed away.

Grep-and-classify is cheap; a wrong-but-uncalled method surviving every suite is not.

## Which layers does this change need?

| Change | Unit | Snapshot | `make all` |
|---|---|---|---|
| Comment, docs, rules HTML text | – | – | – |
| Pure refactor, no behaviour change | yes | yes (must be clean) | yes if engine |
| Bug fix in isolated logic | yes — add one if missing | yes | yes if engine |
| Order execution / turn phase | yes if isolable | **required** | yes |
| Data table entry, enable/disable | – | **required** | **required** |
| `Globals` value | – | **required** | yes |
| Report or template wording | – | **required**, diff will be non-empty | – |
| Save/load format | – | **required** | yes |
| New `GameDefs` field | – | required | **required** — all six `rules.cpp` plus `unittest/rules.cpp` |
| New unit test file only | run it | not needed — `unittest/` links into no game binary | – |

When in doubt, run all three. The suites are fast relative to the cost of shipping a silent
behaviour change into live games.

Per the project rules, if you touch logic that could be unit-tested and is not, say so and
ask whether to add a suite rather than quietly skipping it.

## References

- `references/boost-ut-api.md` — the fuller verified API surface: parameterized tests over
  values and types, BDD `given`/`when`/`then` with `mut()`, exception and abort assertions,
  custom matchers, `constant<>`, runner/reporter configuration. Read when the basics above
  do not cover what you need.
- `references/snapshot-tests.md` — how the snapshot harness works, how to read a diff, the
  baseline-regeneration policy, the smoke test, and CI. Read before running, interpreting,
  or regenerating snapshots.

For engine architecture, the turn pipeline, and data tables, use the `atlantis-codebase`
skill instead — this one covers only testing.

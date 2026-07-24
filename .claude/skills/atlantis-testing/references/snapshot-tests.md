# Snapshot tests, smoke test, and CI

Contents:
1. What the snapshot suite is
2. Running it
3. Mechanism
4. Reading a failure
5. Adding a turn vs. regenerating baselines
6. Smoke test
7. CI
8. Local build note (macOS)

---

## 1. What the snapshot suite is

Recorded games. `snapshot-tests/` stores complete turn inputs and the exact outputs the
engine produced for them; the harness replays each turn and diffs the result byte-for-byte.

With unit coverage as thin as it is, this is the regression suite that actually catches
things. It is also the reason report wording, RNG draw order, and turn-phase order are all
effectively part of the project's contract: change any of them and the diff lights up.

## 2. Running it

```bash
./snapshot-tests/run-snapshots.sh
```

This calls `run-game-snapshots.sh` twice — once for `standard`, once for `neworigins`. Both
binaries must already be built (`make standard neworigins`, or `make all`); the script exits
with a clear message if they are missing.

## 3. Mechanism

For each turn `0..N`, where N comes from `<game>_turns/turn` (currently 13 for NewOrigins,
so 14 turns per game):

1. copy `turn_<n>/game.in`, `players.in`, `orders.3` into the working directory
2. run the binary
3. move every produced file into `output/turn_<n>/`
4. `diff -ur turn_<n> output/turn_<n>` — **any** difference fails the run

Each stored turn directory holds both the inputs and the expected outputs:

```
game.in  game.out  players.in  players.out  orders.3
report.1  report.3  template.3  times.*  engine-output.txt
```

`engine-output.txt` is diffed too, so even the console phase banners the engine prints are
part of the contract. A change to a `Awrite("Running X Orders...")` line will fail the suite.

Turns chain: each turn's `game.in` is the previous turn's `game.out`. That is why a single
behavioural change usually produces a cascade of diffs from the first affected turn onward.

## 4. Reading a failure

The script prints the full diff before cleaning up. Work through it rather than around it:

- **Diffs confined to what you intended** → likely correct; confirm each line.
- **Diffs starting at turn N and cascading into N+1, N+2 …** → expected, because of the
  chaining above. Verify that turn N's diff fully explains the later ones.
- **Diffs in unrelated systems** (different combat outcomes, different monster placement,
  different market prices) → suspect a change in how many random numbers get drawn, or in
  what order. The RNG seed is stored in the savegame, so replays are deterministic; insert
  or remove a single `getrandom` call and every subsequent draw shifts.
- **Diffs only in `engine-output.txt`** → a phase banner or ordering change.
- **A crash rather than a diff** → the script prints the engine output and stops.

## 5. Adding a turn vs. regenerating baselines

**Adding a new turn** (extending coverage) is what `new-turn.sh` is for:

```bash
cd snapshot-tests/neworigins_turns && ./new-turn.sh
```

It seeds `turn_<n+1>` from the previous turn's `game.out` / `players.out` / `template.3`,
waits for you to edit the orders, then runs the engine to produce the new expected output and
bumps the `turn` counter. It appends; it does not touch existing turns.

**Regenerating existing baselines** means replacing recorded expected output with whatever
the engine now produces. That discards the previous behaviour record, and it converts any
regression hiding in the diff into "expected output" that nothing downstream will ever catch
again.

So: do not do it unilaterally. Show the user the diff, explain which changes are intended
consequences of the change in hand, and get agreement first. If part of the diff cannot be
explained, that part is the bug.

## 6. Smoke test

`smoketest/smoketest.py` drives many turns with a bot generating orders. It is legacy Python 2
(`print` statements, CVS references) and is not wired into CI. Do not rely on it as a gate;
raise it only if the user asks about long-run stability. Any new tooling here should be
Python 3, per the project rules.

## 7. CI

`.github/workflows/` — on push to `master` and `stable`, and on every pull request:

| Job | Does |
|---|---|
| `compile` | `make all-clean && make all` on Ubuntu **and** Windows; uploads all six binaries |
| `test` | downloads the Ubuntu binaries, runs `./run-snapshots.sh` |
| `unittest` | `make unittest`, then `./unittest` |

Two implications for any change: it must compile all six rulesets, not just NewOrigins, and
it must compile on Windows — so avoid POSIX-only APIs.

## 8. Local build note (macOS)

The committed Makefile sets `CPLUS = g++`, which resolves to clang on macOS and does not build
this tree cleanly under `-Werror` (clang rejects VLAs that GCC accepts). A local override to a
real GCC (`g++-13`) is what makes `make` work there.

If you hit VLA or similar `-Werror` failures on macOS in files you did not touch, check the
compiler before assuming you broke something. Do not commit a hardcoded `g++-13` without
making it platform-conditional, or you break the Linux and Windows CI runners.

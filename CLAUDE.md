# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Who you are
You are a software egnineer. Your task is to write code and tests, fix bugs. You have deep expertise in C/C++ and Python.

## What This Project Is

Atlantis PBEM is a fantasy strategy game engine (Play-by-Email). Players submit text orders each turn; the engine processes them and generates reports. Multiple rulesets (Basic, Standard, Kingdoms, Fracas, Havilah, NewOrigins) share a common engine library.

We are primary building only 1 ruleset - NewOrigins. Other rulesets can be used as reference only.

## AI rules
- REQUIRED: invoke the `atlantis-codebase` skill BEFORE reading or editing any code in this repository — every bug fix, feature, refactor, or question about how the engine works. It explains the engine/ruleset boundary, the turn pipeline, the global data tables, the savegame protocol, and the build workflow that the rest of these rules assume.
- REQUIRED: invoke the `atlantis-testing` skill BEFORE adding or changing ANY test — creating or editing a file in `unittest/`, writing or modifying assertions, touching `snapshot-tests/`, regenerating snapshot baselines, or investigating a failing or skipped test. It is also required when deciding what test coverage a change needs. Our vendored Boost.UT is older than its public docs, `tag()` silently disables tests, and untyped assertions produce failures nobody can diagnose — tests written from general C++ knowledge are quietly wrong here. Load it in addition to `atlantis-codebase`, not instead of it.
- ALWAYS tell when you are loading and using a skill
- Always ask most important questions to clarify the task, find gaps in the task or ambiguity that needs to be resolved to improve quality of the AI output
- Always run tests (`make unittest && cd unittest && ./unittest` from the root) to validate that changes are working
- If there is a missing unittest, always ask to add more unittests if you find gaps
- ALWAYS comment non-straightforward code patterns. When code relies on a nuance that a competent reader (human or AI) could miss and get wrong — for example, macro variable shadowing, ordering that must be preserved, a pointer left dangling on purpose, bookkeeping that looks buggy but is correct, an intentional leak, positional save/load fields, an invariant enforced elsewhere — add a comment that states the nuance, why it holds, and what breaks if someone "fixes" it. Prefer explaining the trap over restating the code. If you trace a surprising pattern by hand and confirm it is correct, leave that finding as a comment so nobody has to re-derive it. Comments explaining behavior are allowed on any file, including code you must not otherwise change (e.g. while writing tests). Good reference examples: the `AList::Remove`/`NextLive` notes in `alist.cpp` and the nested-`forlist` idiom notes in `unittest/alist_test.cpp`.
- All skills scripts must be written using Python3

## Build Commands
```bash
make all              # Build all game binaries + unit tests
make <game>           # Build a specific game (basic, standard, kingdoms, fracas, havilah, neworigins)
make unittest         # Build unit tests only
make all-clean        # Clean all build artifacts
```

Game binaries are built into `<game>/` subdirectories (e.g., `neworigins/neworigins`).

### Testing Notes

- Unit tests in `unittest/*_test.cpp` use Boost.UT and cover individual subsystems (build orders, spells, markets, victory conditions, JSON reports, etc.)
- Snapshot tests in `snapshot-tests/` use stored turn input/output pairs; they fall back to CMake-built binaries if Makefile binaries aren't present
- CI runs compile (Linux/Windows/macOS matrix), unit tests, and snapshot tests on every push to `master`

# Turn pipeline reference

Contents:
1. The outer loop (`RunGame`)
2. Order lifecycle: text → object → execution
3. `RunOrders()` phase by phase
4. Month-long orders in detail
5. Movement phases
6. Where to hook new work

---

## 1. The outer loop

`main.cpp` → `atlantis run`:

```
Game::OpenGame()                 read game.in; version-checked against CURRENT_ATL_VER
Game::RunGame()                  game.cpp:1045
  PreProcessTurn()               month++ (wraps to year++ after month 11);
                                 SetupUnitNums(); Faction::DefaultOrders();
                                 per-region SetWeather / SetGateStatus / DefaultOrders
  ReadPlayers()                  players.in — new factions, passwords, GM directives
  RemoveInactiveFactions()       if Globals->MAX_INACTIVE_TURNS != -1
  ReadOrders()                   orders.<factionnum> for each faction
  RunOrders()                    the phase sequence, below
  WriteWorldEvents()             if Globals->WORLD_EVENTS — Events::Write → times.<n>
  WriteReport()                  report.<n>
  WriteTemplates()               template.<n>
  battles.DeleteAll()
  EmptyHell()                    only now are dead units freed
  WritePlayers()                 players.out
  DeleteDeadFactions()
Game::SaveGame()                 game.out
```

Files a turn consumes and produces, all in the working directory:

| In | Out |
|---|---|
| `game.in` | `game.out` |
| `players.in` | `players.out` |
| `orders.<factionnum>` | `report.<factionnum>`, `template.<factionnum>`, `times.<n>` |

## 2. Order lifecycle

1. **Parse** — `parseorders.cpp`. `Game::ParseOrders` reads lines, `Parse1Order` maps the
   verb to an `O_*` constant, and `Game::ProcessOrder` dispatches to a
   `Process<Verb>Order` method.
2. **Store** — most handlers allocate an order object from `orders.h` and attach it to the
   unit: a single slot (`monthorders`, `castorders`, `stealorders`, `attackorders`,
   `joinorders`, `teleportorders`) or an `AList` (`giveorders`, `buyorders`, `sellorders`,
   `findorders`, `withdraworders`, `exchangeorders`, `transportorders`, `forgetorders`).
3. **Instant orders execute during parsing** — FORM/END, NAME, DESCRIBE, BEHIND, AVOID,
   GUARD, AUTOTAX, COMBAT, WEAPON, ARMOR, REVEAL, SPOILS, SHARE, PASSWORD, OPTION,
   DECLARE, FACTION. They change unit or faction state immediately, which is why order
   *file* line order matters for them but not for phased orders.
4. **Execute** — `runorders.cpp` / `monthorders.cpp`, in the phase order below.
5. **Clear** — `Unit::ClearOrders` at end of turn; `TurnOrder` blocks (`TURN`/`ENDTURN`)
   are re-queued for the next turn instead.

`Game::DoOrdersCheck` (`atlantis check`) runs the same parser against a dummy game with
an `OrdersCheck` collector so players can validate orders without running a turn. Any new
order handler must behave sanely when `pCheck` is non-null — that is the "syntax check
only" mode.

## 3. `RunOrders()` phase by phase

`runorders.cpp:30`. The rationale column is why the phase sits where it does; treat these
as game-design commitments, not implementation trivia.

| # | Phase | Call | Why here |
|---|---|---|---|
| 1 | FIND | `RunFindOrders` | Pure information, no side effects. |
| 2 | ENTER/LEAVE | `RunEnterOrders(0)` | Position units in buildings before combat, so fort bonuses apply. |
| 3 | PROMOTE/EVICT | `RunPromoteOrders` | Ownership settled before anyone can be attacked out of a building. |
| 4 | Combat | `DoAttackOrders`, `DoAutoAttacks` | Declared attacks then automatic ones; before any economic action, so the dead cannot tax or trade. |
| 5 | STEAL/ASSASSINATE | `RunStealOrders` | After combat: survivors are the valid targets. |
| 6 | GIVE | `DoGiveOrders` | Transfers after combat so spoils can be redistributed same turn. |
| 7 | ENTER NEW | `RunEnterOrders(1)` | Units that just received a new building/ship can enter it. |
| 8 | EXCHANGE | `DoExchangeOrders` | Two-sided, both units must still be alive and co-located. |
| 9 | DESTROY | `RunDestroyOrders` | After occupancy settles. |
| 10 | PILLAGE | `RunPillageOrders` | Before TAX — pillaging wrecks the population that taxers would draw from. |
| 11 | TAX | `RunTaxOrders` | Contested taxation resolved after combat decided who holds the hex. |
| 12 | GUARD 1 | `DoGuard1Orders` | Newly-guarding units take effect after this turn's taxation. |
| 13 | Magic | `ClearCastEffects`, `RunCastOrders` | Invisibility and other effects reset, then spells cast. |
| 14 | SELL | `RunSellOrders` | Before BUY, so players can finance purchases with the same turn's sales. |
| 15 | BUY | `RunBuyOrders` | |
| 16 | FORGET | `RunForgetOrders` | After study-relevant work; frees skill slots. |
| 17 | Mid-turn | `MidProcessTurn` → `MidProcessUnitExtra` | Ruleset hook, per unit (`<game>/extra.cpp`). |
| 18 | QUIT | `RunQuitOrders` | |
| 19 | cleanup | `DeleteEmptyUnits` | |
| 20 | WITHDRAW | `DoWithdrawOrders` | If `Globals->ALLOW_WITHDRAW`. |
| 21 | Movement | `RunMovementOrders` | Consolidated MOVE + SAIL, phased (see §5). Movement is late so all economic actions resolved where the unit started. |
| 22 | fleet cleanup | `SinkUncrewedFleets`, `DrownUnits` | Consequences of movement. |
| 23 | | `FindDeadFactions` | |
| 24 | TEACH | `RunTeachOrders` | Before STUDY: teaching must be applied to this month's study. |
| 25 | Month-long | `RunMonthOrders` | STUDY, PRODUCE, BUILD, ENTERTAIN, WORK, IDLE — see §4. |
| 26 | Economics | `ProcessEconomics` | Region growth after production consumed resources. |
| 27 | TELEPORT | `RunTeleportOrders` | After movement, so teleport targets reflect the settled map. |
| 28 | TRANSPORT | `CheckTransportOrders`, `RunTransportOrders` | If `Globals->TRANSPORT & ALLOW_TRANSPORT`. |
| 29 | Maintenance | `AssessMaintenance` | Upkeep charged on the end-of-turn unit composition. |
| 30 | Migration | `ProcessMigration` | If `Globals->DYNAMIC_POPULATION`. |
| 31 | Post-turn | `PostProcessTurn` | Victory check (`CheckVictory`), `PostProcessUnitExtra`, region `PostTurn`. |
| 32 | cleanup | `DeleteEmptyUnits`, `RemoveEmptyObjects` | |

Note `EmptyHell()` is deliberately **not** here — it runs in `RunGame()` after reports, so
that farsight and battle reports can still dereference dead units.

## 4. Month-long orders

`Game::RunMonthOrders` (`monthorders.cpp:1042`) walks regions and dispatches on
`Unit::monthorders->type`:

- `O_STUDY` → `RunStudyOrders` → `Do1StudyOrder`. Cost from `SkillCost`, days added via
  `Unit::Study`, level derived by `GetLevelByDays`. Teaching already applied in phase 24.
- `O_PRODUCE` → `RunProduceOrders`. Region `Production` pools are contested: `ValidProd`
  filters eligible units, `FindAttemptedProd` sums demand, `RunAProduction` divides the
  pool proportionally. This is why two units producing the same resource each get less.
- `O_BUILD` → `Run1BuildOrder` / `RunBuildShipOrder` / `RunBuildHelpers` /
  `AddNewBuildings`. Ship construction goes through `ShipConstruction` and `CreateShip`.
- `O_ENTERTAIN`, `O_WORK` → income from `Globals->ENTERTAIN_INCOME` and region wages.
- `O_TAX`, `O_PILLAGE` when `Globals->TAX_PILLAGE_MONTH_LONG` is set (it is, for
  NewOrigins) — these consume the month even though they executed in phases 10–11.
- `O_IDLE` → `RunIdleOrders`.

## 5. Movement phases

`RunMovementOrders` (`monthorders.cpp:38`) is not a simple loop. Movement is resolved in
*phases* so that units moving at different speeds interleave correctly and interception
works:

- Each unit gets `movepoints` from `Unit::CalcMovePoints`, bounded by `Globals->MAX_SPEED`.
- `Unit::MoveType` picks walk/ride/fly/swim/sail from carrying capacity vs. weight
  (`Weight()`, `WalkingCapacity()`, `RidingCapacity()`, `FlyingCapacity()`,
  `SwimmingCapacity()`).
- Terrain cost comes from `ARegion::MoveCost`, adjusted by roads and weather.
- `Globals->PHASED_MOVE_OFFSET` staggers when each unit acts within the movement window.
- After each step, `DoMovementAttacks` gives guards and ADVANCE-ing units the chance to
  intercept. `DoMoveEnter` handles entering objects at the destination.
- Sailing is folded in: a fleet moves as one `Object`, needing `FleetSailingSkill`
  sailors; `SailThroughCheck` enforces `PREVENT_SAIL_THROUGH`.

Changing move-point arithmetic or phase offsets will move units into different hexes and
will change essentially every snapshot report. Expect that, and verify the diff is
consistent with the intent.

## 6. Where to hook new work

- **New order verb**: add to the `O_*` enum and `OrderStrs` (`orders.h`/`orders.cpp`), an
  order class if it carries data, a `Process<Verb>Order` in `parseorders.cpp` with a
  declaration in `game.h`, a storage slot on `Unit` (cleared in `ClearOrders`), a `Run…`
  call placed deliberately in `RunOrders()`, and rules text in `genrules.cpp`.
- **Per-unit work each turn, ruleset-specific**: `MidProcessUnitExtra` (mid-turn) or
  `PostProcessUnitExtra` (end of turn) in `neworigins/extra.cpp`. Prefer these over
  editing the engine loop.
- **Per-region end-of-turn work**: `ARegion::PostTurn` (`aregion.cpp`).
- **New narrative event**: record a `FactBase` subclass via `Game::RecordFact` during the
  turn; `WriteWorldEvents` turns facts into the `times.<n>` article. See `events.h` and
  `events-battle.cpp` for the pattern.

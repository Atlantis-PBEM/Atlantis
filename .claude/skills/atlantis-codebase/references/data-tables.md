# Global data tables reference

Contents:
1. The parallel-array pattern and why it bites
2. Table inventory
3. Startup mutation via `modify.cpp`
4. Recipe: add an item
5. Recipe: add a skill
6. Recipe: add an object (building or ship)
7. Recipe: add a terrain type or monster
8. Lookup helpers

---

## 1. The parallel-array pattern

`gamedata.h` declares enums. `gamedata.cpp` defines flat C arrays indexed by those enums.
`items.h` / `skills.h` / `object.h` / `aregion.h` declare the element structs and the
`extern` pointers.

```
gamedata.h            gamedata.cpp                items.h
enum { I_SWORD, … }   ItemType id[] = { … }       extern ItemType *ItemDefs;
                      ItemType *ItemDefs = id;
```

Index `I_SWORD` must land on the sword row. Nothing enforces this — not the compiler, not
a test. If you insert an enum value without inserting a row at the same position, every
item from that point on shifts by one and the game hands out the wrong goods, silently.

Practical consequences:

- **Append rather than insert** whenever you can.
- If you must insert, edit the enum and the array in the same edit, at the same position,
  and re-read both.
- Counts are derived, never hand-written: `int NUMMONSTERS = sizeof(md)/sizeof(md[0]);`.
  Keep it that way.
- Savegames are **symbolic** for the big tables: items and races persist as `ItemDefs[].abr`,
  skills as `SkillDefs[].abbr`, objects as `ObjectDefs[].name`, terrain as
  `TerrainDefs[].type`. So reordering an enum does not corrupt saves — but **renaming an
  abbreviation or name does**, because `LookupItem`/`LookupSkill`/`ParseObject` will no
  longer resolve the stored token and the item/skill/object is dropped on load. Treat
  those strings as a wire format.

## 2. Table inventory

All in `gamedata.cpp` unless noted.

| Table | Element type (header) | Enum / index | Notes |
|---|---|---|---|
| `ItemDefs` | `ItemType` (items.h) | `I_*` | Everything ownable: races, resources, weapons, armor, mounts, ships, trade goods, magic items, monsters. `NITEMS` derived. |
| `ManDefs` | `ManType` (items.h) | via `ItemType` man rows | Race skill caps, ethnicity for name generation. |
| `MonDefs` | `MonType` (items.h) | monster items | Attack/defense/hits/regen, spoils, `preferredTerrain`/`forbiddenTerrain` for movement AI. |
| `WeaponDefs` | `WeaponType` (items.h) | weapon items | Class, attack type, num attacks, bonuses, `bonusMalus` matchups. |
| `ArmorDefs` | `ArmorType` (items.h) | armor items | `saves[NUM_WEAPON_CLASSES]` out of `from`. |
| `MountDefs` | `MountType` (items.h) | mount items | Skill required, min/max bonus, hampered-flight bonus. |
| `BattleItemDefs` | `BattleItemType` (items.h) | battle items | One-use / mage-only combat items. |
| `SkillDefs` | `SkillType` (skills.h) | `S_*` | `abbr` is the wire token. `flags` (MAGIC, CAST, FOUNDATION, APPRENTICE, DISABLED, NOSTUDY…), `depends[3]`, `special`, `range`. |
| `ShowDefs` | `ShowType` (skills.h) | — | Skill-level descriptions, paired with `skillshows.cpp`. |
| `ObjectDefs` | `ObjectType` (object.h) | `O_*` | Buildings and ships: `protect`, `capacity`, `sailors`, construction cost/skill, decay, `defenceArray`. |
| `TerrainDefs` | `TerrainType` (aregion.h) | `R_*` | `type` string is the wire token. Products, races, coastal races, monster frequency, lairs, economy. |
| `SpecialDefs` | `SpecialType` (skills.h) | by key string | Combat specials: targeting flags, shields, damage entries. `NUMSPECIALS`. |
| `EffectDefs` | `EffectType` (skills.h) | by key string | Combat effects and their cancels. |
| `RangeDefs` | `RangeType` (skills.h) | by key string | Spell range classes. |
| `AttribDefs` | `AttribModType` (skills.h) | by key string | Derived attributes: `"observation"`, `"stealth"`, `"wind"`, `"entertainment"`. Composed of `AttribModItem` entries from skills/items. |
| `HealDefs`, `MagicHealDefs` | `HealType` (skills.h) | level | Healing rates. |
| `Globals` | `GameDefs` (gamedefs.h) | — | Defined in `<game>/rules.cpp`, not here. |

String-keyed tables are searched with `FindSkill`, `FindSpecial`, `FindEffect`,
`FindRange`, `FindAttrib` — those return `nullptr` for unknown keys, so check.

## 3. Startup mutation via `modify.cpp`

The tables in `gamedata.cpp` are **defaults shared by all rulesets**. Each ruleset rewrites
them at startup in `Game::ModifyTablesPerRuleset()` (`<game>/extra.cpp`), which `main()`
calls before opening any game file.

`modify.cpp` provides the API:

```
EnableItem / DisableItem            ModifyItemName / Flags / Type / Weight / BasePrice
ModifyItemProductionSkill / Output / Input / Capacities / Speed / Escape
EnableSkill / DisableSkill          ModifySkillDependancy / Flags / Cost / Special / Range
EnableObject / DisableObject        ModifyObjectFlags / Decay / Production / Monster /
                                    Construction / Manpower / Defence / Name
ModifyRaceSkills / SkillLevels      ModifyMonster* (attack, defense, hits, skills,
                                    special, spoils, threat)
ModifyWeapon* / ModifyArmor* / ModifyMount*
ClearTerrainRaces / ModifyTerrainRace / CoastRace / Items / WMons / Lair / Economy / Flags
ModifySpecial* / ModifyEffect* / ModifyRange* / ModifyAttribMod / ModifyHealing
```

**Read `neworigins/extra.cpp:738` before concluding what any item, skill, or object does
in NewOrigins.** Many entries are `DISABLED` by default in `gamedata.cpp` and enabled
there, and several have their production skills, costs, or flags rewritten.

Disabled entries are skipped everywhere: markets, production, rules HTML generation, and
`ItemList::Readin` (which silently drops disabled items when loading a save — so disabling
an item that players hold destroys their stock on next load).

## 4. Recipe: add an item

1. `gamedata.h` — add `I_YOURITEM` to the item enum. Append if at all possible.
2. `gamedata.cpp` — add the `ItemType` row at the matching position. Fields, in order:
   `name`, `names` (plural), `abr` (the wire token — pick a unique 3–4 char code),
   `flags`, production (`pSkill`, `pLevel`, `pMonths`, `pOut`, `pInput[4]`), magic
   production (`mSkill`, `mLevel`, `mOut`, `mInput[4]`), `weight`, `type` (the `IT_*`
   bitmask), `baseprice`, `combat`, capacities, `speed`, hitch, multipliers,
   `max_inventory`, escape fields, grant-skill fields.
3. If it is a weapon/armor/mount/battle item/monster/man, add the matching row to
   `WeaponDefs` / `ArmorDefs` / `MountDefs` / `BattleItemDefs` / `MonDefs` / `ManDefs`.
   Those tables are keyed by `abbr` matching the item's `abr`.
4. `neworigins/extra.cpp` — `EnableItem(I_YOURITEM)` in `ModifyTablesPerRuleset`, plus any
   `Modify*` tuning.
5. Make it obtainable: a terrain product (`ModifyTerrainItems`), a market entry
   (`economy.cpp` city-market setup), a production recipe, or monster spoils.
6. `genrules.cpp` — the rules HTML enumerates items from the live tables, but any prose
   describing the new item must be written by hand.
7. Build all six rulesets (`make all`) — a new enum value shifts nothing for them, but a
   misplaced row does.

## 5. Recipe: add a skill

1. `gamedata.h` — `S_YOURSKILL` in the skill enum (`NSKILLS` is the terminator).
2. `gamedata.cpp` — `SkillType` row: `name`, `abbr`, `cost`, `flags`, `special`, `range`,
   `depends[3]`. Magic skills need `SkillType::MAGIC`; castable ones need `CAST`;
   `FOUNDATION` marks a base magic school; `APPRENTICE` marks apprentice-usable.
3. `skillshows.cpp` — a `ShowDefs` description per level, or players see nothing when they
   study it.
4. `neworigins/extra.cpp` — `EnableSkill` if it ships disabled; `ModifySkillDependancy` /
   `ModifySkillCost` for ruleset tuning.
5. If it grants a derived attribute, add an `AttribModItem` to the relevant `AttribDefs`
   entry (that is how TRUE seeing feeds `"observation"` and INVI feeds `"stealth"`).
6. If it is castable, add parsing and execution in `spells.cpp` and declare the methods
   inside the `#ifdef GAME_SPELLS` block of `spells.h`.
7. Race caps: `SkillMax` consults `ManDefs[].skills` — a skill no race can learn is dead
   weight. Use `ModifyRaceSkills` if needed.

## 6. Recipe: add an object

1. `gamedata.h` — `O_YOUROBJECT` in the object enum.
2. `gamedata.cpp` — `ObjectType` row: `name` (the wire token), `flags` (`CANENTER`,
   `CANMODIFY`, `TRANSPORT`, `GROUP`, `NEVERDECAY`, `DISABLED`), `protect`, `capacity`,
   `sailors`, `maxMages`, construction `item`/`cost`/`skill`/`level`, decay fields,
   `monster`, `productionAided`, `defenceArray[NUM_ATTACK_TYPES]`.
3. Ships additionally need an item entry with `IT_SHIP` and to be recognised by
   `ObjectIsShip`; fleets aggregate ships inside a single `O_FLEET` object.
4. `neworigins/extra.cpp` — `EnableObject`, `ModifyObjectConstruction`, etc.
5. Buildings that aid production must set `productionAided`; `RunAProduction` consults it.

## 7. Recipe: terrain and monsters

**Terrain** — add `R_*` in `gamedata.h`, a `TerrainType` row (`name`, `plural`, `type`,
`marker`, `similar_type`, `flags`, `pop`, `wages`, `economy`, `movepoints`, `prods[7]`,
`races[4]`, `coastal_races[3]`, monster frequencies, `lairChance`, `lairs[6]`), then wire
it into map generation in `neworigins/world.cpp` / `map.cpp`. `marker` is the ASCII map
character; `type` is the savegame token.

**Monster** — a monster is an item with `IT_MONSTER` plus a `MonDefs` row keyed by the
same `abbr`. `preferredTerrain` / `forbiddenTerrain` drive wandering-monster movement:
monsters roam freely in preferred terrain, at half chance into neutral terrain that
borders preferred terrain, and never into forbidden terrain. Lair monsters attach to
objects via `ObjectType::monster` and `Game::MakeLMon`.

## 8. Lookup helpers

| Function | Where | Returns |
|---|---|---|
| `LookupItem(AString*)` | items.cpp | item index from `abr`, `-1` if unknown |
| `ParseItem`, `ParseGiveableItem`, `ParseTransportableItem` | items.cpp | parse from player text, respecting DISABLED |
| `LookupSkill`, `ParseSkill` | skills.cpp | skill index from `abbr` / player text |
| `LookupObject`, `ParseObject` | object.cpp | object index |
| `FindSkill/Special/Effect/Range/Attrib(char const*)` | skills.cpp | pointer or `nullptr` |
| `ItemString`, `ItemDescription` | items.cpp | report/rules text |
| `ObjectDescription` | object.cpp | rules text |

`Parse*` functions honour the DISABLED flag; `Lookup*` generally do not. Use `Parse*` for
anything driven by player input so disabled content stays invisible.

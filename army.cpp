#include "army.h"
#include "logger.hpp"
#include "gamedata.h"
#include "rng.hpp"

#include <assert.h>
#include <iterator>
#include <list>

void unit_stat_control::Clear(UnitStat& us) {
    us.attackStats.clear();
}

AttackStat* unit_stat_control::FindStat(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    for (auto &stat : us.attackStats) {
        std::string effectName = effect == std::nullopt
            ? ""
            : effect->get().specialname;

        if (stat.weaponIndex == weaponIndex && stat.effect == effectName) {
            return &stat;
        }
    }

    return NULL;
}

void unit_stat_control::TrackSoldier(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect, int attackType, int weaponClass) {
    AttackStat* s = unit_stat_control::FindStat(us, weaponIndex, effect);
    if (s == NULL) {
        AttackStat stat;
        if (effect != std::nullopt) {
            stat.effect = effect->get().specialname;
        }
        stat.weaponIndex = weaponIndex;
        stat.attackType = attackType;
        stat.weaponClass = weaponClass;
        stat.soldiers = 1;
        stat.attacks = 0;
        stat.failed = 0;
        stat.missed = 0;
        stat.blocked = 0;
        stat.hit = 0;
        stat.damage = 0;
        stat.killed = 0;

        us.attackStats.push_back(stat);

        return;
    }

    s->soldiers++;
}

void unit_stat_control::RecordAttack(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    AttackStat* s = unit_stat_control::FindStat(us, weaponIndex, effect);
    assert(s != NULL);

    s->attacks++;
}

void unit_stat_control::RecordAttackFailed(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    AttackStat* s = unit_stat_control::FindStat(us, weaponIndex, effect);
    assert(s != NULL);

    s->failed++;
}

void unit_stat_control::RecordAttackMissed(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    AttackStat* s = unit_stat_control::FindStat(us, weaponIndex, effect);
    assert(s != NULL);

    s->missed++;
}

void unit_stat_control::RecordAttackBlocked(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    AttackStat* s = unit_stat_control::FindStat(us, weaponIndex, effect);
    assert(s != NULL);

    s->blocked++;
}

void unit_stat_control::RecordHit(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect, int damage) {
    AttackStat* s = unit_stat_control::FindStat(us, weaponIndex, effect);
    assert(s != NULL);

    s->hit++;
    s->damage += damage;
}

void unit_stat_control::RecordKill(UnitStat& us, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    AttackStat* s = unit_stat_control::FindStat(us, weaponIndex, effect);
    assert(s != NULL);

    s->killed++;
}

void ArmyStats::TrackUnit(Unit *unit) {
    UnitStat roundStat;
    roundStat.unitName = unit->name;

    UnitStat battleStat;
    battleStat.unitName = unit->name;

    roundStats.insert(std::pair<int, UnitStat>(unit->num, roundStat));
    battleStats.insert(std::pair<int, UnitStat>(unit->num, battleStat));
}

void ArmyStats::ClearRound() {
    for (auto &kv : roundStats) {
        unit_stat_control::Clear(kv.second);
    }
}

void ArmyStats::TrackSoldier(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect, int attackType, int weaponClass) {
    unit_stat_control::TrackSoldier(roundStats[unitNumber],  weaponIndex, effect, attackType, weaponClass);
    unit_stat_control::TrackSoldier(battleStats[unitNumber], weaponIndex, effect, attackType, weaponClass);
}

void ArmyStats::RecordAttack(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    unit_stat_control::RecordAttack(roundStats[unitNumber],  weaponIndex, effect);
    unit_stat_control::RecordAttack(battleStats[unitNumber], weaponIndex, effect);
}

void ArmyStats::RecordAttackFailed(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    unit_stat_control::RecordAttackFailed(roundStats[unitNumber],  weaponIndex, effect);
    unit_stat_control::RecordAttackFailed(battleStats[unitNumber], weaponIndex, effect);
}

void ArmyStats::RecordAttackMissed(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    unit_stat_control::RecordAttackMissed(roundStats[unitNumber],  weaponIndex, effect);
    unit_stat_control::RecordAttackMissed(battleStats[unitNumber], weaponIndex, effect);
}

void ArmyStats::RecordAttackBlocked(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    unit_stat_control::RecordAttackBlocked(roundStats[unitNumber],  weaponIndex, effect);
    unit_stat_control::RecordAttackBlocked(battleStats[unitNumber], weaponIndex, effect);
}

void ArmyStats::RecordHit(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect, int damage) {
    unit_stat_control::RecordHit(roundStats[unitNumber],  weaponIndex, effect, damage);
    unit_stat_control::RecordHit(battleStats[unitNumber], weaponIndex, effect, damage);
}

void ArmyStats::RecordKill(int unitNumber, int weaponIndex, std::optional<std::reference_wrapper<SpecialType>> effect) {
    unit_stat_control::RecordKill(roundStats[unitNumber],  weaponIndex, effect);
    unit_stat_control::RecordKill(battleStats[unitNumber], weaponIndex, effect);
}


enum {
    WIN_NO_DEAD,
    WIN_DEAD,
    LOSS
};

Soldier::Soldier(Unit * u,Object * o,int regtype,int r,int ass)
{
    int i;

    race = r;
    unit = u;
    building = 0;

    healing = 0;
    healtype = 0;
    healitem = -1;
    canbehealed = 1;
    regen = 0;

    armor = -1;
    riding = -1;
    weapon = -1;

    attacks = 1;
    hitDamage = 1;
    attacktype = ATTACK_COMBAT;

    special = NULL;
    slevel = 0;

    askill = 0;

    dskill[ATTACK_COMBAT] = 0;
    dskill[ATTACK_ENERGY] = -2;
    dskill[ATTACK_SPIRIT] = -2;
    dskill[ATTACK_WEATHER] = -2;
    dskill[ATTACK_RIDING] = 0;
    dskill[ATTACK_RANGED] = 0;
    for (int i=0; i<NUM_ATTACK_TYPES; i++)
        protection[i] = 0;
    damage = 0;
    hits = unit->GetAttribute("toughness");
    if (hits < 1) hits = 1;
    maxhits = hits;
    amuletofi = 0;
    battleItems.clear();

    /* Special case to allow protection from ships */
    if (o->IsFleet() && o->capacity < 1 && static_cast<size_t>(o->shipno) < o->ships.size()) {
        int objectno;

        auto calc_def = [](Item *i) {
            auto obid = lookup_object(ItemDefs[i->type].name);
            int prot = 0;
            if (obid >= 0 && ObjectDefs[obid].protect > 0) prot = ObjectDefs[obid].protect;
            int total_def = 0;
            for(int j=0; j<NUM_ATTACK_TYPES; j++) {
                total_def += ObjectDefs[obid].defenceArray[j];
            }
            return prot * total_def;
        };
        auto compare_ship = [calc_def](Item *ship1, Item *ship2) {
            return calc_def(ship1) > calc_def(ship2);
        };
        std::list<Item *> sorted_ships;
        // make a copy of the ships itemlist that we can sort as we want without modifying the underlying list
        std::copy(o->ships.begin(), o->ships.end(), std::back_inserter(sorted_ships));
        sorted_ships.sort(compare_ship);

        i = 0;
        for(auto ship : sorted_ships) {
            if (o->shipno == i) {
                objectno = lookup_object(ItemDefs[ship->type].name);
                if (objectno >= 0 && ObjectDefs[objectno].protect > 0) {
                    o->capacity = ObjectDefs[objectno].protect * ship->num;
                    o->type = objectno;
                }
                o->shipno++;
            }
            i++;
            if (o->capacity > 0) break;
        }
    }
    /* Building bonus */
    if (o->capacity) {
        building = o->type;
        //should the runes spell be a base or a bonus?
        for (int i=0; i<NUM_ATTACK_TYPES; i++) {
            if (Globals->ADVANCED_FORTS) {
                protection[i] += ObjectDefs[o->type].defenceArray[i];
            } else
                dskill[i] += ObjectDefs[o->type].defenceArray[i];
        }
        if (o->runes) {
            dskill[ATTACK_ENERGY] = std::max(dskill[ATTACK_ENERGY], o->runes);
            dskill[ATTACK_SPIRIT] = std::max(dskill[ATTACK_SPIRIT], o->runes);
        }
        o->capacity--;
    }

    /* Is this a monster? */
    if (ItemDefs[r].type & IT_MONSTER) {
        auto mp = find_monster(ItemDefs[r].abr, (ItemDefs[r].type & IT_ILLUSION))->get();
        if((u->type == U_WMON) || (ItemDefs[r].flags & ItemType::MANPRODUCE))
            name = mp.name + " in " + unit->name;
        else
            name = mp.name + " controlled by " + unit->name;
        askill = mp.attackLevel;
        dskill[ATTACK_COMBAT] += mp.defense[ATTACK_COMBAT];
        if (mp.defense[ATTACK_ENERGY] > dskill[ATTACK_ENERGY]) dskill[ATTACK_ENERGY] = mp.defense[ATTACK_ENERGY];
        if (mp.defense[ATTACK_SPIRIT] > dskill[ATTACK_SPIRIT]) dskill[ATTACK_SPIRIT] = mp.defense[ATTACK_SPIRIT];
        if (mp.defense[ATTACK_WEATHER] > dskill[ATTACK_WEATHER]) dskill[ATTACK_WEATHER] = mp.defense[ATTACK_WEATHER];
        dskill[ATTACK_RIDING] += mp.defense[ATTACK_RIDING];
        dskill[ATTACK_RANGED] += mp.defense[ATTACK_RANGED];
        damage = 0;
        hits = mp.hits;
        if (hits < 1) hits = 1;
        maxhits = hits;
        attacks = mp.numAttacks;
        hitDamage = mp.hitDamage;
        if (!attacks) attacks = 1;
        special = mp.special;
        slevel = mp.specialLevel;
        if (Globals->MONSTER_BATTLE_REGEN) {
            regen = mp.regen;
            if (regen < 0) regen = 0;
        }
        return;
    }

    name = unit->name;

    SetupHealing();

    SetupSpell();
    SetupCombatItems();

    // Set up armor
    for (i = 0; i < MAX_READY; i++) {
        // Check preferred armor first.
        int item = unit->readyArmor[i];
        if (item == -1) break;
        item = unit->get_armor(ItemDefs[item].abr, ass);
        if (item != -1) {
            armor = item;
            break;
        }
    }
    if (armor == -1) {
        for (auto armorType : ArmorDefs) {
            int item = unit->get_armor(armorType.abbr, ass);
            if (item != -1) {
                armor = item;
                break;
            }
        }
    }

    //
    // Check if this unit is mounted
    //
    int terrainflags = TerrainDefs[regtype].flags;
    int canFly = (terrainflags & TerrainType::FLYINGMOUNTS);
    int canRide = (terrainflags & TerrainType::RIDINGMOUNTS);
    int ridingBonus = 0;
    if (canFly || canRide) {
        //
        // Mounts of some type _are_ allowed in this region
        //
        int item = -1;
        if (ItemDefs[race].type & IT_MOUNT) {
            // If the man is a mount (Centaurs), then the only option
            // they have for riding is the built-in one
            item = unit->get_mount(ItemDefs[race].abr, canFly, canRide, ridingBonus);
        } else {
            for (const auto& mount : MountDefs) {
                // See if this mount is an option
                item = unit->get_mount(mount.abbr, canFly, canRide, ridingBonus);
                if (item == -1) continue;
                // No riding other men in combat
                if (ItemDefs[item].type & IT_MAN) {
                    item = -1;
                    ridingBonus = 0;
                    continue;
                }
                break;
            }
        }

        // Modify riding bonus for half bonus
        if (Globals->HALF_RIDING_BONUS) ridingBonus = (ridingBonus + 1) / 2;

        // Defer adding the combat bonus until we know if the weapon
        // allows it.  The defense bonus for riding can be added now
        // however.
        dskill[ATTACK_RIDING] += ridingBonus;
        riding = item;
    }

    //
    // Find the correct weapon for this soldier.
    //
    int attackBonus = 0;
    int defenseBonus = 0;
    int numAttacks = 1;
    int numHitDamage = 1;
    // hitDamage
    for (i = 0; i < MAX_READY; i++) {
        // Check the preferred weapon first.
        int item = unit->readyWeapon[i];
        if (item == -1) break;
        item = unit->get_weapon(
            ItemDefs[item].abr, riding, ridingBonus, attackBonus, defenseBonus, numAttacks, numHitDamage
        );
        if (item != -1) {
            weapon = item;
            break;
        }
    }
    if (weapon == -1) {
        for (auto& weapontype : WeaponDefs) {
            int item = unit->get_weapon(
                weapontype.abbr, riding, ridingBonus, attackBonus, defenseBonus, numAttacks, numHitDamage
            );
            if (item != -1) {
                weapon = item;
                break;
            }
        }
    }
    // If we did not get a weapon, set attack and defense bonuses to
    // combat skill (and riding bonus if applicable).
    if (weapon == -1) {
        attackBonus = unit->GetAttribute("combat") + ridingBonus;
        defenseBonus = attackBonus;
        numAttacks = 1;
    } else {
        // Okay.  We got a weapon.  If this weapon also has a special
        // and we don't have a special set, use that special.
        // Weapons (like Runeswords) which are both weapons and battle
        // items will be skipped in the battle items setup and handled
        // here.
        if ((ItemDefs[weapon].type & IT_BATTLE) && special == NULL) {
            auto pBat = find_battle_item(ItemDefs[weapon].abr)->get();
            special = pBat.special;
            slevel = pBat.skillLevel;
        }
    }

    unit->PracticeAttribute("combat");

    // Set the attack and defense skills
    // These will include the riding bonus if they should be included.
    askill += attackBonus;
    dskill[ATTACK_COMBAT] += defenseBonus;
    attacks = numAttacks;
    hitDamage = numHitDamage;
}

void Soldier::SetupSpell()
{
    if (unit->type != U_MAGE && unit->type != U_GUARDMAGE) return;

    if (unit->combat != -1) {
        slevel = unit->GetSkill(unit->combat);
        if (!slevel) {
            //
            // The unit can't cast this spell!
            //
            unit->combat = -1;
            return;
        }

        SkillType *pST = &SkillDefs[unit->combat];
        if (!(pST->flags & SkillType::COMBAT)) {
            //
            // This isn't a combat spell!
            //
            unit->combat = -1;
            return;
        }

        special = pST->special ? pST->special.value().c_str() : nullptr;
        unit->Practice(unit->combat);
    }
}

void Soldier::SetupCombatItems()
{
    int exclusive = 0;

    for (auto pBat : BattleItemDefs) {
        int item = unit->get_battle_item(pBat.abbr);
        if (item == -1) continue;

        // If we are using the ready command, skip this item unless
        // it's the right one, or unless it is a shield which doesn't
        // need preparing.
        if (!Globals->USE_PREPARE_COMMAND ||
                ((unit->readyItem == -1) &&
                 (Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_NORMAL)) ||
                (item == unit->readyItem) ||
                (pBat.flags & BattleItemType::SHIELD)) {
            if ((pBat.flags & BattleItemType::SPECIAL) && special != NULL) {
                // This unit already has a special attack so give the item
                // back to the unit as they aren't going to use it.
                unit->items.SetNum(item, unit->items.GetNum(item)+1);
                continue;
            }
            if (pBat.flags & BattleItemType::MAGEONLY &&
                    unit->type != U_MAGE && unit->type != U_GUARDMAGE &&
                    unit->type != U_APPRENTICE) {
                // Only mages/apprentices can use this item so give the
                // item back to the unit as they aren't going to use it.
                unit->items.SetNum(item, unit->items.GetNum(item)+1);
                continue;
            }

            if (pBat.flags & BattleItemType::EXCLUSIVE) {
                if (exclusive) {
                    // Can only use one exclusive item, and we already
                    // have one, so give the extras back.
                    unit->items.SetNum(item, unit->items.GetNum(item)+1);
                    continue;
                }
                exclusive = 1;
            }

            if (pBat.flags & BattleItemType::MAGEONLY) {
                unit->Practice(S_MANIPULATE);
            }

            /* Make sure amulets of invulnerability are marked */
            if (item == I_AMULETOFI) {
                amuletofi = 1;
            }

            battleItems.insert(item);

            if (pBat.flags & BattleItemType::SPECIAL) {
                special = pBat.special;
                slevel = pBat.skillLevel;
            }

            if (pBat.flags & BattleItemType::SHIELD) {
                auto sp = find_special(pBat.special ? pBat.special : "").value().get();
                for (int i = 0; i < 4; i++) {
                    if (sp.shield[i] == NUM_ATTACK_TYPES) {
                        for (int j = 0; j < NUM_ATTACK_TYPES; j++) {
                            if (dskill[j] < pBat.skillLevel)
                                dskill[j] = pBat.skillLevel;
                        }
                    } else if (sp.shield[i] >= 0) {
                        if (dskill[sp.shield[i]] < pBat.skillLevel)
                            dskill[sp.shield[i]] = pBat.skillLevel;
                    }
                }
            }
        } else {
            // We are using prepared items and this item is NOT the one
            // we have prepared, so give it back to the unit as they won't
            // use it.
            unit->items.SetNum(item, unit->items.GetNum(item)+1);
            continue;
        }
    }
}

bool Soldier::has_effect(const std::string& effect)
{
    if (effect.empty()) return false;

    return effects[effect];
}

void Soldier::set_effect(const std::string& effect)
{
    if (effect.empty()) return;
    int i;

    auto e = FindEffect(effect.c_str());
    if (!e) return;

    askill += e->get().attackVal;

    for (i = 0; i < 4; i++) {
        if (e->get().defMods[i].type != -1)
            dskill[e->get().defMods[i].type] += e->get().defMods[i].val;
    }

    if (e->get().cancelEffect != NULL) clear_effect(e->get().cancelEffect);

    if (!(e->get().flags & EffectType::EFF_NOSET)) effects[effect] = true;
}

void Soldier::clear_effect(const std::string& effect)
{
    if (effect.empty()) return;
    int i;

    auto e = FindEffect(effect.c_str());
    if (!e) return;

    askill -= e->get().attackVal;

    for (i = 0; i < 4; i++) {
        if (e->get().defMods[i].type != -1)
            dskill[e->get().defMods[i].type] -= e->get().defMods[i].val;
    }

    effects[effect] = false;
}

void Soldier::clear_one_time_effects(void)
{
    for (auto eff : EffectDefs) {
        if (has_effect(eff.name) && (eff.flags & EffectType::EFF_ONESHOT))
            clear_effect(eff.name);
    }
}

bool Soldier::armor_protect(int weaponClass)
{
    auto armor_type = (armor > 0) ? find_armor(ItemDefs[armor].abr) : std::nullopt;
    if (!armor_type) return false;
    int chance = armor_type->get().saves[weaponClass];

    if (chance <= 0) return false;
    if (chance > rng::get_random(armor_type->get().from)) return true;

    return false;
}

void Soldier::RestoreItems()
{
    if (healing && healitem != -1) {
        if (healitem == I_HERBS) {
            unit->items.SetNum(healitem,
                    unit->items.GetNum(healitem) + healing);
        } else if (healitem == I_HEALPOTION) {
            unit->items.SetNum(healitem,
                    unit->items.GetNum(healitem)+1);
        }
    }
    if (weapon != -1)
        unit->items.SetNum(weapon,unit->items.GetNum(weapon) + 1);
    if (armor != -1)
        unit->items.SetNum(armor,unit->items.GetNum(armor) + 1);
    if (riding != -1 && !(ItemDefs[riding].type & IT_MAN))
        unit->items.SetNum(riding,unit->items.GetNum(riding) + 1);

    for (auto pBat : BattleItemDefs) {
        int item = lookup_item(pBat.abbr);
        if (battleItems.count(item)) unit->items.SetNum(item, unit->items.GetNum(item) + 1);
    }
}

void Soldier::Alive(int state)
{
    RestoreItems();

    if (state == LOSS) {
        unit->canattack = 0;
        unit->routed = 1;
        /* Guards with amuletofi will not go off guard */
        if (!amuletofi &&
            (unit->guard == GUARD_GUARD || unit->guard == GUARD_SET)) {
            unit->guard = GUARD_NONE;
        }
    } else {
        unit->advancefrom = 0;
    }

    if (state == WIN_DEAD) {
        unit->canattack = 0;
        unit->nomove = 1;
    }
}

void Soldier::Dead()
{
    RestoreItems();

    unit->SetMen(race,unit->GetMen(race) - 1);
}

Army::Army(Unit *ldr, std::list<Location *>& locs, int regtype, int ass)
{
    stats = ArmyStats();

    int tacspell = 0;
    Unit * tactician = ldr;

    leader = ldr;
    round = 0;
    tac = ldr->GetAttribute("tactics");
    count = 0;
    hitstotal = 0;
    tactics_bonus = 0;

    if (ass) {
        count = 1;
        ldr->losses = 0;
    } else {
        for(const auto l: locs) {
            Unit *u = l->unit;
            count += u->GetSoldiers();
            u->losses = 0;
            int temp = u->GetAttribute("tactics");
            if (temp > tac) {
                tac = temp;
                tactician = u;
            }
        }
    }
    // If TACTICS_NEEDS_WAR is enabled, we don't want to push leaders
    // from tact-4 to tact-5! Also check that we have skills, otherwise
    // we get a nasty core dump ;)
    if (Globals->TACTICS_NEEDS_WAR && (tactician->skills.size() != 0)) {
        int currskill = tactician->skills.GetDays(S_TACTICS)/tactician->GetMen();
        if (currskill < 450 - Globals->SKILL_PRACTICE_AMOUNT) {
            tactician->PracticeAttribute("tactics");
        }
    } else { // Only Globals->TACTICS_NEEDS_WAR == 0
        tactician->PracticeAttribute("tactics");
    }
    soldiers = new SoldierPtr[count];
    int x = 0;
    int y = count;

    for(const auto l : locs) {
        Unit *u = l->unit;
        stats.TrackUnit(u);

        Object * obj = l->obj;
        if (ass) {
            for(auto it : u->items) {
                if (!it) continue;

                ItemType &item = ItemDefs[it->type];
                // Bug when assassinating STED/CATA if they are the only type in the unit
                // Should they be able to be assassinted? Unknown, but for now, allow it and preference
                // men first, but if the unit only has sted or cata, choose 1.
                if (item.type & IT_MAN) {
                    soldiers[x] = new Soldier(u, obj, regtype, it->type, ass);
                    hitstotal = soldiers[x]->hits;
                    ++x;
                    goto finished_army;
                }
                // If we get here we didn't find a man, so.. choose a MANPRODUCE item
                if (item.flags & ItemType::MANPRODUCE) {
                    soldiers[x] = new Soldier(u, obj, regtype, it->type, ass);
                    hitstotal = soldiers[x]->hits;
                    ++x;
                    goto finished_army;
                }
            }
        } else {
            for(auto it : u->items) {
                if (IsSoldier(it->type)) {
                    for (int i = 0; i < it->num; i++) {
                        ItemType &item = ItemDefs[ it->type ];
                        if (((item.type & IT_MAN) || (item.flags & ItemType::MANPRODUCE)) && u->GetFlag(FLAG_BEHIND)) {
                            --y;
                            soldiers[y] = new Soldier(u, obj, regtype, it->type);
                            hitstotal += soldiers[y]->hits;
                        } else {
                            soldiers[x] = new Soldier(u, obj, regtype, it->type);
                            hitstotal += soldiers[x]->hits;
                            ++x;
                        }
                    }
                }
            }
        }
    }

finished_army:
    tac = tac + tacspell;

    canfront = x;
    canbehind = count;
    notfront = count;
    notbehind = count;

    hitsalive = hitstotal;

    if (!NumFront()) {
        canfront = canbehind;
        notfront = notbehind;
    }
}

Army::~Army()
{
    delete [] soldiers;
}

void Army::Reset() {
    canfront = notfront;
    canbehind = notbehind;
    notfront = notbehind;
    tactics_bonus = 0;
}

void Army::WriteLosses(Battle * b) {
    b->AddLine(leader->name + " loses " + std::to_string(count - NumAlive()) + ".");

    if (notbehind != count) {
        std::list<Unit *> units;
        for (int i=notbehind; i<count; i++) {
            if (std::find(units.begin(), units.end(), soldiers[i]->unit) == units.end()) {
                units.push_back(soldiers[i]->unit);
            }
        }

        int comma = 0;
        std::string damaged;
        for(const auto u : units) {
            if (comma) {
                damaged += ", " + std::to_string(u->num);
            } else {
                damaged = "Damaged units: " + std::to_string(u->num);
                comma = 1;
            }
        }
        b->AddLine(damaged + ".");
    }
}

void Army::GetMonSpoils(ItemList& spoils, int monitem, int free)
{
    if ((Globals->MONSTER_NO_SPOILS > 0) &&
            (free >= Globals->MONSTER_SPOILS_RECOVERY)) {
        // This monster is in it's period of absolutely no spoils.
        return;
    }

    /* First, silver */
    auto mp = find_monster(ItemDefs[monitem].abr, (ItemDefs[monitem].type & IT_ILLUSION))->get();
    int silv = mp.silver;
    if ((Globals->MONSTER_NO_SPOILS > 0) && (free > 0)) {
        // Adjust the spoils for length of freedom.
        silv *= (Globals->MONSTER_SPOILS_RECOVERY-free);
        silv /= Globals->MONSTER_SPOILS_RECOVERY;
    }
    spoils.SetNum(I_SILVER, spoils.GetNum(I_SILVER) + rng::get_random(silv));

    int thespoil = mp.spoiltype;

    if (thespoil == -1) return;
    if (thespoil == IT_NORMAL && rng::get_random(2) && !Globals->SPOILS_NO_TRADE) thespoil = IT_TRADE;

    int count = 0;
    int i;
    for (i=0; i<NITEMS; i++) {
        if (
            (ItemDefs[i].type & thespoil) && !(ItemDefs[i].type & IT_SPECIAL) && !(ItemDefs[i].type & IT_SHIP) &&
            !(ItemDefs[i].type & IT_NEVER_SPOIL) && (ItemDefs[i].baseprice <= mp.silver) &&
            !(ItemDefs[i].flags & ItemType::DISABLED)
        ) {
            count ++;
        }
    }
    if (count == 0) return;
    count = rng::get_random(count) + 1;

    for (i=0; i<NITEMS; i++) {
        if (
            (ItemDefs[i].type & thespoil) && !(ItemDefs[i].type & IT_SPECIAL) && !(ItemDefs[i].type & IT_SHIP) &&
            !(ItemDefs[i].type & IT_NEVER_SPOIL) && (ItemDefs[i].baseprice <= mp.silver) &&
            !(ItemDefs[i].flags & ItemType::DISABLED)
        ) {
            count--;
            if (count == 0) {
                thespoil = i;
                break;
            }
        }
    }

    int val = rng::get_random(mp.silver * 2);
    if ((Globals->MONSTER_NO_SPOILS > 0) && (free > 0)) {
        // Adjust for length of monster freedom.
        val *= (Globals->MONSTER_SPOILS_RECOVERY-free);
        val /= Globals->MONSTER_SPOILS_RECOVERY;
    }

    spoils.SetNum(
        thespoil,
        spoils.GetNum(thespoil) + (val + rng::get_random(ItemDefs[thespoil].baseprice)) / ItemDefs[thespoil].baseprice
    );
}

void Army::Regenerate(Battle *b)
{
    for (int i = 0; i < count; i++) {
        Soldier *s = soldiers[i];
        if (i<notbehind) {
            int diff = s->maxhits - s->hits;
            if (diff > 0) {
                std::string aName = s->name;

                if (s->damage != 0) {
                    b->AddLine(aName + " takes " + std::to_string(s->damage) + " hits bringing it to " +
                        std::to_string(s->hits) + "/" + std::to_string(s->maxhits) + ".");
                    s->damage = 0;
                } else {
                    b->AddLine(aName + " takes no hits leaving it at " + std::to_string(s->hits) + "/" +
                        std::to_string(s->maxhits) + ".");
                }
                if (s->regen) {
                    int regen = s->regen;
                    if (regen > diff) regen = diff;
                    s->hits += regen;
                    b->AddLine(aName + " regenerates " + std::to_string(regen) + " hits bringing it to " +
                        std::to_string(s->hits) + "/" + std::to_string(s->maxhits) + ".");
                }
            }
        }
    }
}

void Army::Lose(Battle *b, ItemList& spoils)
{
    WriteLosses(b);
    for (int i=0; i<count; i++) {
        Soldier *s = soldiers[i];
        if (i < notbehind) {
            s->Alive(LOSS);
        } else {
            if ((s->unit->type==U_WMON) && (ItemDefs[s->race].type&IT_MONSTER))
                GetMonSpoils(spoils,s->race,s->unit->free);
            s->Dead();
        }
        delete s;
    }
}

void Army::Tie(Battle * b)
{
    WriteLosses(b);
    for (int x=0; x<count; x++) {
        Soldier * s = soldiers[x];
        if (x<NumAlive()) {
            s->Alive(WIN_DEAD);
        } else {
            s->Dead();
        }
        delete s;
    }
}

int Army::CanBeHealed()
{
    for (int i=notbehind; i<count; i++) {
        Soldier * temp = soldiers[i];
        if (temp->canbehealed) return 1;
    }
    return 0;
}

void Army::DoHeal(Battle * b)
{
    // Do magical healing
    for (int i = 5; i > 0; --i) {
        int rate = MagicHealDefs[i].rate;
        DoHealLevel(b, i, rate, 0);
    }

    // Use HPOT
    DoHealLevel(b, 6, 70, 1);

    // Do Normal healing
    for (int i = 5; i > 0; --i) {
        int rate = HealDefs[i].rate;
        DoHealLevel(b, i, rate, 1);
    }
}

void Army::DoHealLevel(Battle *b, int level, int rate, int useItems)
{
    for (int i=0; i<NumAlive(); i++) {
        Soldier * s = soldiers[i];
        int n = 0;
        if (!CanBeHealed()) break;
        if (s->healtype <= 0) continue;
        // This should be here.. Use the best healing first
        if (s->healtype != level) continue;
        if (!s->healing) continue;
        if (useItems) {
            if (s->healitem == -1) continue;
            if (s->healitem != I_HEALPOTION) s->unit->Practice(S_HEALING);
        } else {
            if (s->healitem != -1) continue;
            s->unit->Practice(S_MAGICAL_HEALING);
        }

        while (s->healing) {
            if (!CanBeHealed()) break;
            int j = rng::get_random(count - NumAlive()) + notbehind;
            Soldier * temp = soldiers[j];
            if (temp->canbehealed) {
                s->healing--;
                if (rng::get_random(100) < rate) {
                    n++;
                    soldiers[j] = soldiers[notbehind];
                    soldiers[notbehind] = temp;
                    notbehind++;
                } else
                    temp->canbehealed = 0;
            }
        }
        if (useItems && s->healitem == I_HEALPOTION) {
            b->AddLine(s->unit->name + " heals " + std::to_string(n) + " using healing potion with " +
                std::to_string(rate) + "% chance.");
        } else {
            b->AddLine(s->unit->name + " heals " + std::to_string(n) + " with " + std::to_string(rate) + "% chance.");
        }
    }
}

void Army::Win(Battle * b, ItemList& spoils)
{
    int wintype;

    DoHeal(b);

    WriteLosses(b);

    int na = NumAlive();
    int loses_percent = 100;
    if (na != 0 && count != 0) {
        loses_percent = 100 - ((na * 100) / count);
    }

    if (loses_percent > 0 && loses_percent >= Globals->BATTLE_STOP_MOVE_PERCENT) wintype = WIN_DEAD;
    else wintype = WIN_NO_DEAD;

    std::vector<Unit *> units;

    for (int x = 0; x < count; x++) {
        Soldier * s = soldiers[x];
        if (x<NumAlive()) s->Alive(wintype);
        else s->Dead();
    }

    for(auto i : spoils) {
        if (i && na) {
            int ns;

            do {
                units.clear();
                // Make a list of units who can get this type of spoil
                for (int x = 0; x < na; x++) {
                    if (soldiers[x]->unit->CanGetSpoil(i)) {
                        units.push_back(soldiers[x]->unit);
                    }
                }

                ns = units.size();
                if (ItemDefs[i->type].type & IT_SHIP) {
                    int t = rng::get_random(ns);
                    Unit *u = units[t];
                    if (u && u->CanGetSpoil(i)) {
                        u->items.SetNum(i->type, i->num);
                        u->faction->DiscoverItem(i->type, 0, 1);
                        i->num = 0;
                    }
                    break;
                }
                while (ns > 0 && i->num >= ns) {
                    int chunk = 1;
                    if (!ItemDefs[i->type].weight) {
                        chunk = i->num / ns;
                    }
                    for(auto it = units.begin(); it != units.end();) {
                        auto u = *it;
                        if (u->CanGetSpoil(i)) {
                            u->items.SetNum(i->type, u->items.GetNum(i->type) + chunk);
                            u->faction->DiscoverItem(i->type, 0, 1);
                            i->num -= chunk;
                            ++it;
                        } else {
                            it = units.erase(it);
                            ns--;
                        }
                    }
                }
                while (ns > 0 && i->num > 0) {
                    int t = rng::get_random(ns);
                    auto u = units[t];
                    // get an iterator to that element of the array for later.
                    auto it = units.begin() + t;
                    if (u && u->CanGetSpoil(i)) {
                        u->items.SetNum(i->type, u->items.GetNum(i->type) + 1);
                        u->faction->DiscoverItem(i->type, 0, 1);
                        i->num--;
                    } else {
                        it = units.erase(it);
                        ns--;
                    }
                }
                units.clear();
            } while (ns > 0 && i->num > 0);
        }
    }

    for (int x = 0; x < count; x++) {
        Soldier *s = soldiers[x];
        delete s;
    }
}

int Army::Broken()
{
    if (Globals->ARMY_ROUT == GameDefs::ARMY_ROUT_FIGURES) {
        if ((NumAlive() << 1) < count) return 1;
    } else {
        if ((hitsalive << 1) < hitstotal) return 1;
    }
    return 0;
}

int Army::NumSpoilers()
{
    int na = NumAlive();
    int count = 0;
    for (int x=0; x<na; x++) {
        Unit * u = soldiers[x]->unit;
        if (!(u->flags & FLAG_NOSPOILS)) count++;
    }
    return count;
}

int Army::NumAlive()
{
    return notbehind;
}

int Army::CanAttack()
{
    return canbehind;
}

int Army::NumFront()
{
    return (canfront + notfront - canbehind);
}

int Army::NumBehind() {
    return NumAlive() - NumFront();
}

int Army::NumFrontHits() {
    int totHits = 0;

    for (int i = 0; i < canfront; i++) {
        totHits += soldiers[i]->maxhits;
    }
    for (int i = canbehind; i < notfront; i++) {
        totHits += soldiers[i]->maxhits;
    }
    return totHits;
}

Soldier * Army::GetAttacker(int i,int &behind)
{
    Soldier * retval = soldiers[i];
    if (i<canfront) {
        soldiers[i] = soldiers[canfront-1];
        soldiers[canfront-1] = soldiers[canbehind-1];
        soldiers[canbehind-1] = retval;
        canfront--;
        canbehind--;
        behind = 0;
        return retval;
    }
    soldiers[i] = soldiers[canbehind-1];
    soldiers[canbehind-1] = soldiers[notfront-1];
    soldiers[notfront-1] = retval;
    canbehind--;
    notfront--;
    behind = 1;
    return retval;
}

int Army::GetTargetNum(char const *special, bool canAttackBehind)
{
    int tars = NumFront();
    if (canAttackBehind) {
        tars = NumAlive();
    }

    if (tars == 0) {
        canfront = canbehind;
        notfront = notbehind;
        tars = NumFront();
        if (tars == 0) return -1;
    }

    auto sp = find_special(special ? special : "");

    if (sp && sp->get().targflags) {
        int validtargs = 0;
        int i, start = -1;

        for (i = 0; i < canfront; i++) {
            if (CheckSpecialTarget(special, i)) {
                validtargs++;
                // slight scan optimisation - skip empty initial sequences
                if (start == -1) start = i;
            }
        }
        for (i = canbehind; i < notfront; i++) {
            if (CheckSpecialTarget(special, i)) {
                validtargs++;
                // slight scan optimisation - skip empty initial sequences
                if (start == -1) start = i;
            }
        }
        if (validtargs) {
            int targ = rng::get_random(validtargs);

            for (i = start; i < notfront; i++) {
                if (i == canfront) i = canbehind;
                if (CheckSpecialTarget(special, i)) {
                    if (!targ--) return i;
                }
            }
        }
    } else {
        int i = rng::get_random(tars);
        if (canAttackBehind) {
            return i;
        }
        else {
            if (i < canfront) return i;
            return i + canbehind - canfront;
        }
    }

    return -1;
}

int Army::GetEffectNum(char const *effect)
{
    int validtargs = 0;
    int i, start = -1;

    for (i = 0; i < canfront; i++) {
        if (soldiers[i]->has_effect(effect)) {
            validtargs++;
            // slight scan optimisation - skip empty initial sequences
            if (start == -1) start = i;
        }
    }
    for (i = canbehind; i < notfront; i++) {
        if (soldiers[i]->has_effect(effect)) {
            validtargs++;
            // slight scan optimisation - skip empty initial sequences
            if (start == -1) start = i;
        }
    }
    if (validtargs) {
        int targ = rng::get_random(validtargs);
        for (i = start; i < notfront; i++) {
            if (i == canfront) i = canbehind;
            if (soldiers[i]->has_effect(effect)) {
                if (!targ--) return i;
            }
        }
    }
    return -1;
}

Soldier * Army::GetTarget(int i)
{
    return soldiers[i];
}

int pow(int b,int p)
{
    int b2 = b;
    for (int i=1; i<p; i++) {
        b2 *= b;
    }
    return b2;
}

int Hits(int a,int d)
{
    int tohit = 1,tomiss = 1;
    if (a>d) {
        tohit = pow(2,a-d);
    } else if (d>a) {
        tomiss = pow(2,d-a);
    }
    if (rng::get_random(tohit+tomiss) < tohit) return 1;
    return 0;
}

int Army::RemoveEffects(int num, char const *effect)
{
    int ret = 0;
    for (int i = 0; i < num; i++) {
        //
        // Try to find a target unit.
        //
        int tarnum = GetEffectNum(effect);
        if (tarnum == -1) continue;
        Soldier *tar = GetTarget(tarnum);

        //
        // Remove the effect
        //
        tar->clear_effect(effect);
        ret++;
    }
    return(ret);
}

const WeaponBonusMalus* GetWeaponBonusMalus(const WeaponType& attacker, const WeaponType& target) {
    for (int i = 0; i < MAX_WEAPON_BM_TARGETS; i++) {
        const WeaponBonusMalus *bm = &attacker.bonusMalus[i];
        if (!bm->weaponAbbr) continue;

        if (std::string(bm->weaponAbbr) == target.abbr) {
            return bm;
        }
    }

    return NULL;
}

int Army::DoAnAttack(Battle * b, char const *special, int numAttacks, int attackType,
        int attackLevel, int flags, int weaponClass, char const *effect,
        int mountBonus, Soldier *attacker, Army *attackers, bool attackbehind, int attackDamage)
{
    auto sp = special != NULL ? find_special(special) : std::nullopt;

    // if special is defined then it is magical attack and not attack by the physical weapon soldier has
    int weaponIndex = !sp ? attacker->weapon : -1;
    attackers->stats.TrackSoldier(attacker->unit->num, weaponIndex, sp, attackType, weaponClass);

    /* 1. Check against Global effects (not sure how yet) */
    /* 2. Attack shield */
    int combat = 0;
    int canShield = 0;
    switch(attackType) {
        case ATTACK_RANGED:
            canShield = 1;
            // fall through
        case ATTACK_COMBAT:
        case ATTACK_RIDING:
            combat = 1;
            break;
        case ATTACK_ENERGY:
        case ATTACK_WEATHER:
        case ATTACK_SPIRIT:
            canShield = 1;
            break;
    }

    if (canShield) {
        auto correctShield = [&attackType](std::shared_ptr<Shield> sh) { return sh->shieldtype == attackType; };
        auto compareShield = [](std::shared_ptr<Shield> s1, std::shared_ptr<Shield> s2) {
            return s1->shieldskill < s2->shieldskill;
        };
        auto validShields = shields | std::views::filter(correctShield);
        auto maxShield = std::max_element(validShields.begin(), validShields.end(), compareShield);

        auto hi = (maxShield != validShields.end()) ? *maxShield : nullptr;
        if (hi) {
            /* Check if we get through shield */
            if (!Hits(attackLevel, hi->shieldskill)) {
                return -1;
            }

            if (effect != NULL && !combat) {
                /* We got through shield... if killing spell, destroy shield */
                std::erase(shields, hi);
            }
        }
    }

    //
    // Now, loop through and do attacks
    //
    int ret = 0;
    for (int i = 0; i < numAttacks; i++) {
        /* 3. Get the target */
        int tarnum = GetTargetNum(special, attackbehind);
        if (tarnum == -1) continue;
        Soldier * tar = GetTarget(tarnum);
        int tarFlags = 0;
        if (tar->weapon != -1) {
            auto weapon = find_weapon(ItemDefs[tar->weapon].abr)->get();
            tarFlags = weapon.flags;
        }

        /* 4. Add in any effects, if applicable */
        int tlev = 0;
        if (attackType != NUM_ATTACK_TYPES)
            tlev = tar->dskill[ attackType ];
        if (sp != std::nullopt) {
            if ((sp->get().effectflags & SpecialType::FX_NOBUILDING) && tar->building)
                tlev -= 2;
        }

        /* 4.1 Check whether defense is allowed against this weapon */
        if ((flags & WeaponType::NODEFENSE) && (tlev > 0)) tlev = 0;

        if (!(flags & WeaponType::RANGED)) {
            /* 4.2 Check relative weapon length */
            int attLen = 1;
            int defLen = 1;
            if (flags & WeaponType::LONG) attLen = 2;
            else if (flags & WeaponType::SHORT) attLen = 0;
            if (tarFlags & WeaponType::LONG) defLen = 2;
            else if (tarFlags & WeaponType::SHORT) defLen = 0;
            if (attLen > defLen) attackLevel++;
            else if (defLen > attLen) tlev++;
        }

        /* 4.3 Add bonuses versus mounted */
        if (tar->riding != -1) attackLevel += mountBonus;

        // 4.4 Check for weapon inflicted bonuses
        if (weaponIndex != -1 && tar->weapon != -1) {
            auto attackerWeapon = find_weapon(ItemDefs[weaponIndex].abr)->get();
            auto targetWeapon = find_weapon(ItemDefs[tar->weapon].abr)->get();

            const WeaponBonusMalus *attackerBm = GetWeaponBonusMalus(attackerWeapon, targetWeapon);
            const WeaponBonusMalus *defenderBm = GetWeaponBonusMalus(targetWeapon, attackerWeapon);

            // attacker will get bonus to attack if defender uses weapon to which attackers weapon has bonus
            if (attackerBm) {
                attackLevel += attackerBm->attackModifer;
            }

            // defender will get bonus to defense if attacker uses weapon to which defenders weapon has bonus
            if (defenderBm) {
                tlev += defenderBm->defenseModifer;
            }
        }

        /* 5. Attack soldier */
        if (attackType != NUM_ATTACK_TYPES) {
            attackers->stats.RecordAttack(attacker->unit->num, weaponIndex, sp);

            if (!(flags & WeaponType::ALWAYSREADY)) {
                int failchance = 2;
                if (Globals->ADVANCED_FORTS) {
                    failchance += (tar->protection[attackType]+1)/2;
                }

                if (rng::get_random(failchance)) {
                    attackers->stats.RecordAttackFailed(attacker->unit->num, weaponIndex, sp);
                    continue;
                }
            }

            if (combat) {
                /* 5.1 Add advanced tactics bonus */
                if (!Hits(attackLevel + attackers->tactics_bonus, tlev + tactics_bonus)) {
                    attackers->stats.RecordAttackMissed(attacker->unit->num, weaponIndex, sp);
                    continue;
                }
            } else {
                if (!Hits(attackLevel, tlev)) {
                    attackers->stats.RecordAttackMissed(attacker->unit->num, weaponIndex, sp);
                    continue;
                }
            }
        }

        /* 6. If attack got through, apply effect, or kill */
        if (effect == NULL) {
            /* 7. Last chance... Check armor */
            if (tar->armor_protect(weaponClass)) {
                attackers->stats.RecordAttackBlocked(attacker->unit->num, weaponIndex, sp);
                continue;
            }

            attackers->stats.RecordHit(attacker->unit->num, weaponIndex, sp, attackDamage);

            /* 8. Seeya! */
            Kill(tarnum, attackDamage);
            if (tar->hits == 0) {
                attackers->stats.RecordKill(attacker->unit->num, weaponIndex, sp);
            }

            ret++;
            if ((ItemDefs[tar->race].type & IT_MAN) &&
                (ItemDefs[attacker->race].type & IT_UNDEAD)) {
                if (rng::get_random(100) < Globals->UNDEATH_CONTAGION) {
                    attacker->unit->raised++;
                    tar->canbehealed = 0;
                }
            }
        } else {
            if (tar->has_effect(effect)) {
                continue;
            }
            tar->set_effect(effect);
            ret++;
        }
    }
    return ret;
}

void Army::Kill(int killed, int damage)
{
    Soldier *temp = soldiers[killed];

    if (temp->amuletofi) return;

    if (Globals->ARMY_ROUT == GameDefs::ARMY_ROUT_HITS_INDIVIDUAL)
        hitsalive--;

    temp->damage += std::min(temp->hits, damage);
    temp->hits = std::max(0, temp->hits - damage);

    if (temp->hits > 0) return;

    temp->unit->losses++;
    if (Globals->ARMY_ROUT == GameDefs::ARMY_ROUT_HITS_FIGURE) {
        if (ItemDefs[temp->race].type & IT_MONSTER) {
            auto mp = find_monster(ItemDefs[temp->race].abr, (ItemDefs[temp->race].type & IT_ILLUSION))->get();
            hitsalive -= mp.hits;
        } else {
            // Assume everything that is a solder and isn't a monster is a
            // man.
            hitsalive--;
        }
    }

    if (killed < canfront) {
        soldiers[killed] = soldiers[canfront-1];
        soldiers[canfront-1] = temp;
        killed = canfront - 1;
        canfront--;
    }

    if (killed < canbehind) {
        soldiers[killed] = soldiers[canbehind-1];
        soldiers[canbehind-1] = temp;
        killed = canbehind-1;
        canbehind--;
    }

    if (killed < notfront) {
        soldiers[killed] = soldiers[notfront-1];
        soldiers[notfront-1] = temp;
        killed = notfront-1;
        notfront--;
    }

    soldiers[killed] = soldiers[notbehind-1];
    soldiers[notbehind-1] = temp;
    notbehind--;
}

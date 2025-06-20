// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program, in the file license.txt. If not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// See the Atlantis Project web page for details:
// http://www.prankster.com/project
//
// END A3HEADER
// MODIFICATIONS
// Date        Person          Comment
// ----        ------          -------
// 2001/Jul/08 Joseph Traub    Moved functions t here from game.cpp
//
#include "game.h"
#include "gamedata.h"

void Game::EnableSkill(int sk)
{
    if (sk < 0 || sk > (NSKILLS-1)) return;
    SkillDefs[sk].flags &= ~SkillType::DISABLED;
}

void Game::DisableSkill(int sk)
{
    if (sk < 0 || sk > (NSKILLS-1)) return;
    SkillDefs[sk].flags |= SkillType::DISABLED;
}

void Game::ModifySkillDependancy(int sk, int i, char const *dep, int lev)
{
    if (sk < 0 || sk > (NSKILLS-1)) return;
    if (i < 0 || i >= (int)(sizeof(SkillDefs[sk].depends)/sizeof(SkillDefs[sk].depends[0]))) return;
    if (dep && !FindSkill(dep)) return;
    if (lev < 0) return;
    SkillDefs[sk].depends[i].skill = dep;
    SkillDefs[sk].depends[i].level = lev;
}
void Game::ModifySkillFlags(int sk, int flags)
{
    if (sk < 0 || sk > (NSKILLS-1)) return;
    SkillDefs[sk].flags = flags;
}

void Game::ModifySkillCost(int sk, int cost)
{
    if (sk < 0 || sk > (NSKILLS-1)) return;
    if (cost < 0) return;
    SkillDefs[sk].cost = cost;
}

void Game::modify_skill_special(int sk, const std::string& special)
{
    if (sk < 0 || sk > (NSKILLS-1)) return;
    if (!find_special(special)) return;
    SkillDefs[sk].special = special;
}

void Game::modify_skill_range(int sk, const std::string& range)
{
    if (sk < 0 || sk > (NSKILLS-1)) return;
    if (!find_range(range)) return;
    SkillDefs[sk].range = range;
}


void Game::EnableItem(int item)
{
    if (item < 0 || item > (NITEMS-1)) return;
    ItemDefs[item].flags &= ~ItemType::DISABLED;
}

void Game::DisableItem(int item)
{
    if (item < 0 || item > (NITEMS-1)) return;
    ItemDefs[item].flags |= ItemType::DISABLED;
}

void Game::ModifyItemName(int it, char const *name, char const *names)
{
    if (it < 0 || it > (NITEMS-1)) return;
    ItemDefs[it].name = name;
    ItemDefs[it].names = names;
}

void Game::ModifyItemFlags(int it, int flags)
{
    if (it < 0 || it > (NITEMS-1)) return;
    ItemDefs[it].flags = flags;
}

void Game::ModifyItemType(int it, int type)
{
    if (it < 0 || it > (NITEMS-1)) return;
    ItemDefs[it].type = type;
}

void Game::ModifyItemWeight(int it, int weight)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (weight < 0) weight = 0;
    ItemDefs[it].weight = weight;
}

void Game::ModifyItemBasePrice(int it, int price)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (price < 0) price = 0;
    ItemDefs[it].baseprice = price;
}

void Game::ModifyItemCapacities(int it, int wlk, int rid, int fly, int swm)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (wlk < 0) wlk = 0;
    if (rid < 0) rid = 0;
    if (fly < 0) fly = 0;
    if (swm < 0) swm = 0;
    ItemDefs[it].walk = wlk;
    ItemDefs[it].ride = rid;
    ItemDefs[it].fly = fly;
    ItemDefs[it].swim = swm;
}

void Game::ModifyItemSpeed(int it, int speed)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (speed < 0) speed = 0;
    ItemDefs[it].speed = speed;
}

void Game::ModifyItemProductionBooster(int it, int item, int bonus)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (item < -1 || item > (NITEMS-1)) return;
    ItemDefs[it].mult_item = item;
    ItemDefs[it].mult_val = bonus;
}

void Game::ModifyItemHitch(int it, int item, int capacity)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (item < -1 || item > (NITEMS-1)) return;
    if (capacity < 0) return;
    ItemDefs[it].hitchItem = item;
    ItemDefs[it].hitchwalk = capacity;
}

void Game::ModifyItemProductionSkill(int it, char const *sk, int lev)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (sk && !FindSkill(sk)) return;
    ItemDefs[it].pSkill = sk;
    ItemDefs[it].pLevel = lev;
}

void Game::ModifyItemProductionOutput(int it, int months, int count)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (count < 0) count = 0;
    if (months < 0) months = 0;
    ItemDefs[it].pMonths = months;
    ItemDefs[it].pOut = count;
}

void Game::ModifyItemProductionInput(int it, int i, int input, int amount)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (i < 0 || i >= (int)(sizeof(ItemDefs[it].pInput)/sizeof(ItemDefs[it].pInput[0]))) return;
    if (input < -1 || input > (NITEMS-1)) return;
    if (amount < 0) amount = 0;
    ItemDefs[it].pInput[i].item = input;
    ItemDefs[it].pInput[i].amt = amount;
}

void Game::ModifyItemMagicSkill(int it, char *sk, int lev)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (sk && !FindSkill(sk)) return;
    ItemDefs[it].mSkill = sk;
    ItemDefs[it].mLevel = lev;
}

void Game::ModifyItemMagicOutput(int it, int count)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (count < 0) count = 0;
    ItemDefs[it].mOut = count;
}

void Game::ModifyItemMagicInput(int it, int i, int input, int amount)
{
    if (it < 0 || it > (NITEMS-1)) return;
    if (i < 0 || i >= (int)(sizeof(ItemDefs[it].mInput)/sizeof(ItemDefs[it].mInput[0]))) return;
    if (input < -1 || input > (NITEMS-1)) return;
    if (amount < 0) amount = 0;
    ItemDefs[it].mInput[i].item = input;
    ItemDefs[it].mInput[i].amt = amount;
}

void Game::ModifyItemEscape(int it, int escape, char const *skill, int val)
{
    if (it < 0 || it > (NITEMS-1)) return;
    ItemDefs[it].escape = escape;
    ItemDefs[it].esc_skill = skill;
    ItemDefs[it].esc_val = val;
}

void Game::modify_race_skill_levels(const std::string& race, int spec, int def)
{
    auto mt = find_race(race);
    if (!mt) return;
    if (spec < 0) spec = 0;
    if (def < 0) def = 0;
    mt->get().speciallevel = spec;
    mt->get().defaultlevel = def;
}

void Game::modify_race_skills(const std::string& race, int i, const std::string& sk)
{
    auto mt = find_race(race);
    if (!mt) return;
    if (i < 0 || i >= (int)(sizeof(mt->get().skills) / sizeof(mt->get().skills[0]))) return;
    if (!sk.empty() && !FindSkill(sk.c_str())) return;
    mt->get().skills[i] = sk.empty() ? std::nullopt : std::make_optional(sk);
}

void Game::modify_monster_attack_level(const std::string& mon, int lev)
{
    auto monster = find_monster(mon, 0);
    if (!monster) return;
    if (lev < 0) return;
    monster->get().attackLevel = lev;
}

void Game::modify_monster_defense(const std::string& mon, int defenseType, int level)
{
    auto monster = find_monster(mon, 0);
    if (!monster) return;
    if (defenseType < 0 || defenseType > (NUM_ATTACK_TYPES -1)) return;
    monster->get().defense[defenseType] = level;
}

void Game::modify_monster_attacks_and_hits(const std::string& mon, int num, int hits, int regen, int hitDamage)
{
    auto monster = find_monster(mon, 0);
    if (!monster) return;
    if (num < 0) return;
    if (hits < 0) return;
    if (regen < 0) return;
    if (hitDamage < 0) return;
    monster->get().numAttacks = num;
    monster->get().hits = hits;
    monster->get().regen = regen;
    monster->get().hitDamage = hitDamage;
}

void Game::modify_monster_skills(const std::string& mon, int tact, int stealth, int obs)
{
    auto monster = find_monster(mon, 0);
    if (!monster) return;
    if (tact < 0) return;
    if (stealth < 0) return;
    if (obs < 0) return;
    monster->get().tactics = tact;
    monster->get().stealth = stealth;
    monster->get().obs = obs;
}

void Game::modify_monster_special(const std::string& mon, const std::string& special, int lev)
{
    auto monster = find_monster(mon, 0);
    if (!monster) return;
    if (!special.empty() && !find_special(special)) return;
    if (lev < 0) return;
    monster->get().special = special.c_str();
    monster->get().specialLevel = lev;
}

void Game::modify_monster_spoils(const std::string& mon, int silver, int spoilType)
{
    auto monster = find_monster(mon, 0);
    if (!monster) return;
    if (spoilType < -1) return;
    if (silver < 0) return;
    monster->get().silver = silver;
    monster->get().spoiltype = spoilType;
}

void Game::modify_monster_threat(const std::string& mon, int num, int hostileChance)
{
    auto monster = find_monster(mon, 0);
    if (!monster) return;
    if (num < 0) return;
    if (hostileChance < 0 || hostileChance > 100) return;
    monster->get().hostile = hostileChance;
    monster->get().number = num;
}

void Game::modify_weapon_skills(const std::string& weap, const std::string& baseSkill, const std::string& orSkill)
{
    auto weapon = find_weapon(weap);
    if (!weapon) return;
    if (!baseSkill.empty() && !FindSkill(baseSkill.c_str())) return;
    if (!orSkill.empty() && !FindSkill(orSkill.c_str())) return;
    weapon->get().baseSkill = baseSkill.c_str();
    weapon->get().orSkill = orSkill.c_str();
}

void Game::modify_weapon_flags(const std::string& weap, int flags)
{
    auto weapon = find_weapon(weap);
    if (!weapon) return;
    weapon->get().flags = flags;
}

void Game::modify_weapon_attack(const std::string& weap, int wclass, int attackType, int numAtt, int hitDamage)
{
    auto weapon = find_weapon(weap);
    if (!weapon) return;
    if (wclass < 0 || wclass > (NUM_WEAPON_CLASSES - 1)) return;
    if (attackType < 0 || attackType > (NUM_ATTACK_TYPES - 1)) return;
    weapon->get().weapClass = wclass;
    weapon->get().attackType = attackType;
    weapon->get().numAttacks = numAtt;
    weapon->get().hitDamage = hitDamage;
}

void Game::modify_weapon_bonuses(const std::string& weap, int attack, int defense, int vsMount)
{
    auto weapon = find_weapon(weap);
    if (!weapon) return;
    weapon->get().attackBonus = attack;
    weapon->get().defenseBonus = defense;
    weapon->get().mountBonus = vsMount;
}

void Game::modify_weapon_bonus_malus(
    const std::string& weap, int index, const std::string& weaponAbbr, int attackModifer, int defenseModifer
) {
    auto weapon = find_weapon(weap);
    if (!weapon) return;
    if (index < 0 || index >= (int)(sizeof(weapon->get().bonusMalus)/sizeof(weapon->get().bonusMalus[0]))) return;
    if (weapon->get().bonusMalus[index].weaponAbbr) {
        delete weapon->get().bonusMalus[index].weaponAbbr;
    }
    weapon->get().bonusMalus[index].weaponAbbr = weaponAbbr.c_str();
    weapon->get().bonusMalus[index].attackModifer = attackModifer;
    weapon->get().bonusMalus[index].defenseModifer = defenseModifer;
}

void Game::modify_armor_flags(const std::string& armor, int flags)
{
    auto pa = find_armor(armor);
    if (!pa) return;
    pa->get().flags = flags;
}

void Game::modify_armor_save_from(const std::string& armor, int from)
{
    auto pa = find_armor(armor);
    if (!pa) return;
    if (from < 0) return;
    pa->get().from = from;
}

void Game::modify_armor_save_value(const std::string& armor, int wclass, int val)
{
    auto pa = find_armor(armor);
    if (!pa) return;
    if (wclass < 0 || wclass > (NUM_WEAPON_CLASSES - 1)) return;
    if (val < 0 || val > pa->get().from) return;
    pa->get().saves[wclass] = val;
}

void Game::modify_mount_skill(const std::string& mount, const std::string& skill)
{
    auto pm = find_mount(mount);
    if (!pm) return;
    if (!skill.empty() && !FindSkill(skill.c_str())) return;
    pm->get().skill = skill.c_str();
}

void Game::modify_mount_bonuses(const std::string& mount, int min, int max, int hampered)
{
    auto pm = find_mount(mount);
    if (!pm) return;
    if (min < 0) return;
    if (max < 0) return;
    if (hampered < min) return;
    pm->get().minBonus = min;
    pm->get().maxBonus = max;
    pm->get().maxHamperedBonus = hampered;
}

void Game::modify_mount_special(const std::string& mount, const std::string& special, int level)
{
    auto pm = find_mount(mount);
    if (!pm) return;
    if (!special.empty() && !find_special(special)) return;
    if (level < 0) return;
    pm->get().mountSpecial = special.c_str();
    pm->get().specialLev = level;
}

void Game::EnableObject(int obj)
{
    if (obj < 0 || obj > (NOBJECTS-1)) return;
    ObjectDefs[obj].flags &= ~ObjectType::DISABLED;
}

void Game::DisableObject(int obj)
{
    if (obj < 0 || obj > (NOBJECTS-1)) return;
    ObjectDefs[obj].flags |= ObjectType::DISABLED;
}

void Game::ModifyObjectFlags(int ob, int flags)
{
    if (ob < 0 || ob > (NOBJECTS-1)) return;
    ObjectDefs[ob].flags = flags;
}

void Game::ModifyObjectDecay(int ob, int maxMaint, int maxMonthDecay, int mFact)
{
    if (ob < 0 || ob > (NOBJECTS-1)) return;
    if (maxMonthDecay > maxMaint) return;
    if (maxMaint < 0) return;
    if (maxMonthDecay < 0) return;
    if (mFact < 0) return;
    ObjectDefs[ob].maxMaintenance = maxMaint;
    ObjectDefs[ob].maxMonthlyDecay = maxMonthDecay;
    ObjectDefs[ob].maintFactor = mFact;
}

void Game::ModifyObjectProduction(int ob, int it)
{
    if (ob < 0 || ob > (NOBJECTS-1)) return;
    if (it < -1 || it > (NITEMS -1)) return;
    ObjectDefs[ob].productionAided = it;
}

void Game::ModifyObjectMonster(int ob, int monster)
{
    if (ob < 0 || ob > (NOBJECTS-1)) return;
    if (monster < -1 || monster > (NITEMS -1)) return;
    ObjectDefs[ob].monster = monster;
}

void Game::ModifyObjectConstruction(int ob, int it, int num, char const *sk, int lev)
{
    if (ob < 0 || ob > (NOBJECTS-1)) return;
    if ((it < -1 && it != I_WOOD_OR_STONE) || it > (NITEMS -1)) return;
    if (num < 0) return;
    if (sk && !FindSkill(sk)) return;
    if (lev < 0) return;
    ObjectDefs[ob].item = it;
    ObjectDefs[ob].cost = num;
    ObjectDefs[ob].skill = sk;
    ObjectDefs[ob].level = lev;
}

void Game::ModifyObjectManpower(int ob, int prot, int cap, int sail, int mages)
{
    if (ob < 0 || ob > (NOBJECTS-1)) return;
    if (prot < 0) return;
    if (cap < 0) return;
    if (sail < 0) return;
    if (mages < 0) return;
    ObjectDefs[ob].protect = prot;
    ObjectDefs[ob].capacity = cap;
    ObjectDefs[ob].sailors = sail;
    ObjectDefs[ob].maxMages = mages;
}

void Game::ModifyObjectDefence(int ob, int co, int en, int sp, int we, int ri, int ra)
{
    if (ob < 0 || ob > (NOBJECTS-1)) return;
    //if (val < 0) return;  // we could conceivably have a negative value
                                // associated with a structure
    ObjectDefs[ob].defenceArray[0] = co;
    ObjectDefs[ob].defenceArray[1] = en;
    ObjectDefs[ob].defenceArray[2] = sp;
    ObjectDefs[ob].defenceArray[3] = we;
    ObjectDefs[ob].defenceArray[4] = ri;
    ObjectDefs[ob].defenceArray[5] = ra;
}

void Game::ModifyObjectName(int ob, char const *name)
{
    if (ob < 0 || ob > (NOBJECTS-1)) return;
    ObjectDefs[ob].name = name;
}

void Game::ClearTerrainRaces(int t)
{
    if (t < 0 || t > R_NUM-1) return;
    unsigned int c;
    for (c = 0; c < sizeof(TerrainDefs[t].races)/sizeof(TerrainDefs[t].races[0]); c++) {
        TerrainDefs[t].races[c] = -1;
    }
    for (c = 0; c < sizeof(TerrainDefs[t].coastal_races)/sizeof(TerrainDefs[t].coastal_races[0]); c++) {
        TerrainDefs[t].coastal_races[c] = -1;
    }
}

void Game::ModifyTerrainRace(int t, int i, int r)
{
    if (t < 0 || t > (R_NUM -1)) return;
    if (i < 0 || i >= (int)(sizeof(TerrainDefs[t].races)/sizeof(TerrainDefs[t].races[0]))) return;
    if (r < -1 || r > NITEMS-1) r = -1;
    if (r != -1 && !(ItemDefs[r].type & IT_MAN)) r = -1;
    TerrainDefs[t].races[i] = r;
}

void Game::ModifyTerrainCoastRace(int t, int i, int r)
{
    if (t < 0 || t > (R_NUM -1)) return;
    if (i < 0 || i >= (int)(sizeof(TerrainDefs[t].coastal_races)/sizeof(TerrainDefs[t].coastal_races[0]))) return;
    if (r < -1 || r > NITEMS-1) r = -1;
    if (r != -1 && !(ItemDefs[r].type & IT_MAN)) r = -1;
    TerrainDefs[t].coastal_races[i] = r;
}

void Game::ClearTerrainItems(int terrain)
{
    if (terrain < 0 || terrain > R_NUM-1) return;

    for (unsigned int c = 0; c < sizeof(TerrainDefs[terrain].prods)/sizeof(TerrainDefs[terrain].prods[0]); c++) {
        TerrainDefs[terrain].prods[c].product = -1;
        TerrainDefs[terrain].prods[c].chance = 0;
        TerrainDefs[terrain].prods[c].amount = 0;
    }
}

void Game::ModifyTerrainItems(int terrain, int i, int p, int c, int a)
{
    if (terrain < 0 || terrain > (R_NUM -1)) return;
    if (i < 0 || i >= (int)(sizeof(TerrainDefs[terrain].prods)/sizeof(TerrainDefs[terrain].prods[0]))) return;
    if (p < -1 || p > NITEMS-1) p = -1;
    if (c < 0 || c > 100) c = 0;
    if (a < 0) a = 0;
    TerrainDefs[terrain].prods[i].product = p;
    TerrainDefs[terrain].prods[i].chance = c;
    TerrainDefs[terrain].prods[i].amount = a;
}

void Game::ModifyTerrainWMons(int t, int freq, int smon, int bigmon, int hum)
{
    if (t < 0 || t > (R_NUM -1)) return;
    if (freq < 0) freq = 0;
    if (smon < -1 || smon > NITEMS-1) smon = -1;
    if (bigmon < -1 || bigmon > NITEMS-1) bigmon = -1;
    if (hum < -1 || hum > NITEMS-1) hum = -1;
    TerrainDefs[t].wmonfreq = freq;
    TerrainDefs[t].smallmon = smon;
    TerrainDefs[t].bigmon = bigmon;
    TerrainDefs[t].humanoid = hum;
}

void Game::ModifyTerrainLairChance(int t, int chance)
{
    if (t < 0 || t > (R_NUM -1)) return;
    if (chance < 0 || chance > 100) chance = 0;
    // Chance is percent out of 100 that should have some lair
    TerrainDefs[t].lairChance = chance;
}

void Game::ModifyTerrainLair(int t, int i, int l)
{
    if (t < 0 || t > (R_NUM -1)) return;
    if (i < 0 || i >= (int)(sizeof(TerrainDefs[t].lairs)/sizeof(TerrainDefs[t].lairs[0]))) return;
    if (l < -1 || l > NOBJECTS-1) l = -1;
    TerrainDefs[t].lairs[i] = l;
}

void Game::ModifyTerrainEconomy(int t, int pop, int wages, int econ, int move)
{
    if (t < 0 || t > (R_NUM -1)) return;
    if (pop < 0) pop = 0;
    if (wages < 0) wages = 0;
    if (econ < 0) econ = 0;
    if (move < 1) move = 1;
    TerrainDefs[t].pop = pop;
    TerrainDefs[t].wages = wages;
    TerrainDefs[t].economy = econ;
    TerrainDefs[t].movepoints = move;
}

void Game::ModifyTerrainFlags(int t, int flags)
{
    if (t < 0 || t > (R_NUM -1)) return;
    TerrainDefs[t].flags = flags;
}

void Game::modify_battle_item_flags(const std::string& item, int flags)
{
    auto pb = find_battle_item(item);
    if (!pb) return;
    pb->get().flags = flags;
}

void Game::modify_battle_item_special(const std::string& item, const std::string& special, int level)
{
    auto pb = find_battle_item(item);
    if (!pb) return;
    if (!find_special(special)) return;
    if (level < 0) return;
    pb->get().special = special.c_str();
    pb->get().skillLevel = level;
}

void Game::ModifySpecialTargetFlags(char const *special, int targetflags)
{
    auto sp = find_special(special ? special : "");
    if (!sp) return;
    sp->get().targflags = targetflags;
}

void Game::ModifySpecialTargetObjects(char const *special, int index, int obj)
{
    auto sp = find_special(special ? special : "");
    if (!sp) return;
    if (index < 0 || index >= (int)(sizeof(sp->get().buildings) / sizeof(sp->get().buildings[0]))) return;
    if ((obj != -1 && obj < 1) || (obj > (NOBJECTS-1))) return;
    sp->get().buildings[index] = obj;
}

void Game::ModifySpecialTargetItems(char const *special, int index, int item)
{
    auto sp = find_special(special ? special : "");
    if (!sp) return;
    if (index < 0 || index >= (int)(sizeof(sp->get().targets) / sizeof(sp->get().targets[0]))) return;
    if (item < -1 || item > (NITEMS-1)) return;
    sp->get().targets[index] = item;
}

void Game::ModifySpecialTargetEffects(char const *special, int index, char const *effect)
{
    auto sp = find_special(special ? special : "");
    if (!sp) return;
    if (index < 0 || index >= (int)(sizeof(sp->get().effects) / sizeof(sp->get().effects[0]))) return;
    if (effect && !FindEffect(effect)) return;
    sp->get().effects[index] = effect;
}

void Game::ModifySpecialEffectFlags(char const *special, int effectflags)
{
    auto sp = find_special(special ? special : "");
    if (!sp) return;
    sp->get().effectflags = effectflags;
}

void Game::ModifySpecialShields(char const *special, int index, int type)
{
    auto sp = find_special(special ? special : "");
    if (!sp) return;
    if (index < 0 || index >= (int)(sizeof(sp->get().shield) / sizeof(sp->get().shield[0]))) return;
    if (type < -1 || type > (NUM_ATTACK_TYPES)) return;
    sp->get().shield[index] = type;
}

void Game::ModifySpecialDefenseMods(char const *special, int index, int type, int val)
{
    auto sp = find_special(special ? special : "");
    if (!sp) return;
    if (index < 0 || index >= (int)(sizeof(sp->get().defs) / sizeof(sp->get().defs[0]))) return;
    if (type < -1 || type > (NUM_ATTACK_TYPES)) return;
    sp->get().defs[index].type = type;
    sp->get().defs[index].val = val;
}

void Game::ModifySpecialDamage(char const *special, int index, int type, int min,
        int val, int flags, int cls, char const *effect, int hitDamage)
{
    auto sp = find_special(special ? special : "");
    if (!sp) return;
    if (index < 0 || index >= (int)(sizeof(sp->get().damage) / sizeof(sp->get().damage[0]))) return;
    if (effect && !FindEffect(effect)) return;
    if (type < -1 || type > NUM_ATTACK_TYPES) return;
    if (cls < -1 || cls > (NUM_WEAPON_CLASSES-1)) return;
    if (min < 0) return;
    sp->get().damage[index].type = type;
    sp->get().damage[index].minnum = min;
    sp->get().damage[index].value = val;
    sp->get().damage[index].flags = flags;
    sp->get().damage[index].dclass = cls;
    sp->get().damage[index].effect = effect;
    sp->get().damage[index].hitDamage = hitDamage;
}

void Game::ModifyEffectFlags(char const *effect, int flags)
{
    auto ep = FindEffect(effect);
    if (!ep) return;
    ep->get().flags = flags;
}

void Game::ModifyEffectAttackMod(char const *effect, int val)
{
    auto ep = FindEffect(effect);
    if (!ep) return;
    ep->get().attackVal = val;
}

void Game::ModifyEffectDefenseMod(char const *effect, int index, int type, int val)
{
    auto ep = FindEffect(effect);
    if (!ep) return;
    if (type < 0 || type > NUM_ATTACK_TYPES) return;
    if (index < 0 || index >= (int)(sizeof(ep->get().defMods) / sizeof(ep->get().defMods[0]))) return;
    ep->get().defMods[index].type = type;
    ep->get().defMods[index].val = val;
}

void Game::ModifyEffectCancelEffect(char const *effect, char *uneffect)
{
    auto ep = FindEffect(effect);
    if (!ep) return;
    if (uneffect && !FindEffect(uneffect)) return;
    ep->get().cancelEffect = uneffect;
}

void Game::modify_range_flags(const std::string& range, int flags)
{
    auto rp = find_range(range);
    if (!rp) return;
    rp->get().flags = flags;
}

void Game::modify_range_class(const std::string& range, int rclass)
{
    auto rp = find_range(range);
    if (!rp) return;
    if (rclass < 0 || rclass > (RangeType::NUMRANGECLASSES-1)) return;
    rp->get().rangeClass = rclass;
}

void Game::modify_range_multiplier(const std::string& range, int mult)
{
    auto rp = find_range(range);
    if (!rp) return;
    if (mult < 1) return;
    rp->get().rangeMult = mult;
}

void Game::modify_range_level_penalty(const std::string& range, int pen)
{
    auto rp = find_range(range);
    if (!rp) return;
    if (pen < 0) return;
    rp->get().crossLevelPenalty = pen;
}

void Game::ModifyAttribMod(char const *mod, int index, int flags, char const *ident, int type, int val)
{
    auto mp = FindAttrib(mod);
    if (!mp) return;
    if (index < 0 || index >= (int)(sizeof(mp->get().mods) / sizeof(mp->get().mods[0]))) return;
    if (!ident) return;
    if (type < 0 || type > AttribModItem::NUMMODTYPE-1) return;

    mp->get().mods[index].flags = flags;
    mp->get().mods[index].ident = ident;
    mp->get().mods[index].modtype = type;
    mp->get().mods[index].val = val;
}

void Game::ModifyHealing(int level, int patients, int success)
{
    if (level < 1 || level > 5) return;
    HealDefs[level].num = patients;
    HealDefs[level].rate = success;
}


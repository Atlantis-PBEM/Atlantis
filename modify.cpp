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

/** \file
 * Functions which modify the game's data structures.
 * To suit particular versions of Atlantis, it is sometimes necessary
 * to alter objects, monsters, skills, etc.
 * modify.cpp is where all of the relevant functions to do this are contained.
 */

/// Enable a skill
/** Makes a skill available in play. 
\arg \c sk One of the values from the NSKILLS enum in gamedata.h
*/
void Game::EnableSkill(int sk)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	SkillDefs[sk].flags &= ~SkillType::DISABLED;
}

/// Enable a skill
/** Makes a skill unavailable in play. Please note that it is up to the GM
to disable any other skills that might be dependent upon this one in the skill
tree, and to generally make sure that the skill tree isn't messed up.
\arg \c sk One of the values from the NSKILLS enum in gamedata.h
*/
void Game::DisableSkill(int sk)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	SkillDefs[sk].flags |= SkillType::DISABLED;
}

/// Change the dependency for a skill.
/** Makes a skill available in play. 
\arg \c sk One of the values from the NSKILLS enum in gamedata.h
\arg \c i The dependency array that will be changed.
\arg \c *dep The skill required in the dependency (eg. FORC, if sk is FIRE)
\arg \c lev The level of skill required in the dependency
*/
void Game::ModifySkillDependancy(int sk, int i, char *dep, int lev)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	if(i < 0 || i >= (int)(sizeof(SkillDefs[sk].depends)/sizeof(SkillDepend)))
		return;
	if (dep && (FindSkill(dep) == NULL)) return;
	if(lev < 0) return;
	SkillDefs[sk].depends[i].skill = dep;
	SkillDefs[sk].depends[i].level = lev;
}

/// Modify the flags for a skill.
/** Alters some flags on a skill. Some of the flags that can be set are: 
SkillType::BATTLEREP, SkillType::MAGIC, SkillType::FOUNDATION.
You can set more than one flag by using the '|' or operator. See skills.h
for the full list.

\arg \c sk One of the values from the NSKILLS enum in gamedata.h
\arg \c flags The flags that are going to be set
*/
void Game::ModifySkillFlags(int sk, int flags)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	SkillDefs[sk].flags = flags;
}

/// Modify the cost (in silver) to study a skill for a month.
/**
\arg \c sk One of the values from the NSKILLS enum in gamedata.h
\arg \c cost An amount of silver

*/
void Game::ModifySkillCost(int sk, int cost)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	if(cost < 0) return;
	SkillDefs[sk].cost = cost;
}

/// Modify the special effects associated with skills.
/** Mostly associated with spells, eg. fireball, banish_demon, etc.

\arg \c sk One of the values from the NSKILLS enum in gamedata.h
\arg \c special A special string, eg. "lightning", "clear_skies"

*/
void Game::ModifySkillSpecial(int sk, char *special)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	if (special && (FindSpecial(special) == NULL)) return;
	SkillDefs[sk].special = special;
}

/// Modify the range of a skill.
/** This refers to the physical range within the game world.
See skills.h for the full list.
\arg \c sk One of the values from the NSKILLS enum in gamedata.h
\arg \c cost An amount of silver

*/
void Game::ModifySkillRange(int sk, char *range)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	if (range && (FindRange(range) == NULL)) return;
	SkillDefs[sk].range = range;
}

void Game::EnableItem(int item)
{
	if(item < 0 || item > (NITEMS-1)) return;
	ItemDefs[item].flags &= ~ItemType::DISABLED;
}

void Game::DisableItem(int item)
{
	if(item < 0 || item > (NITEMS-1)) return;
	ItemDefs[item].flags |= ItemType::DISABLED;
}

void Game::ModifyItemFlags(int it, int flags)
{
	if(it < 0 || it > (NITEMS-1)) return;
	ItemDefs[it].flags = flags;
}

void Game::ModifyItemType(int it, int type)
{
	if(it < 0 || it > (NITEMS-1)) return;
	ItemDefs[it].type = type;
}

void Game::ModifyItemWeight(int it, int weight)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(weight < 0) weight = 0;
	ItemDefs[it].weight = weight;
}

void Game::ModifyItemBasePrice(int it, int price)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(price < 0) price = 0;
	ItemDefs[it].baseprice = price;
}

void Game::ModifyItemCapacities(int it, int wlk, int rid, int fly, int swm)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(wlk < 0) wlk = 0;
	if(rid < 0) rid = 0;
	if(fly < 0) fly = 0;
	if(swm < 0) swm = 0;
	ItemDefs[it].walk = wlk;
	ItemDefs[it].ride = rid;
	ItemDefs[it].fly = fly;
	ItemDefs[it].swim = swm;
}

void Game::ModifyItemProductionBooster(int it, int item, int bonus)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(item < -1 || item > (NITEMS-1)) return;
	ItemDefs[it].mult_item = item;
	ItemDefs[it].mult_val = bonus;
}

void Game::ModifyItemHitch(int it, int item, int capacity)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(item < -1 || item > (NITEMS-1)) return;
	if(capacity < 0) return;
	ItemDefs[it].hitchItem = item;
	ItemDefs[it].hitchwalk = capacity;
}

void Game::ModifyItemProductionSkill(int it, char *sk, int lev)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if (sk && (FindSkill(sk) == NULL)) return;
	ItemDefs[it].pSkill = sk;
	ItemDefs[it].pLevel = lev;
}

void Game::ModifyItemProductionOutput(int it, int months, int count)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(count < 0) count = 0;
	if(months < 0) months = 0;
	ItemDefs[it].pMonths = months;
	ItemDefs[it].pOut = count;
}

void Game::ModifyItemProductionInput(int it, int i, int input, int amount)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(i < 0 || i >= (int)(sizeof(ItemDefs[it].pInput)/sizeof(Materials)))
		return;
	if(input < -1 || input > (NITEMS-1)) return;
	if(amount < 0) amount = 0;
	ItemDefs[it].pInput[i].item = input;
	ItemDefs[it].pInput[i].amt = amount;
}

void Game::ModifyItemMagicSkill(int it, char *sk, int lev)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if (sk && (FindSkill(sk) == NULL)) return;
	ItemDefs[it].mSkill = sk;
	ItemDefs[it].mLevel = lev;
}

void Game::ModifyItemMagicOutput(int it, int count)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(count < 0) count = 0;
	ItemDefs[it].mOut = count;
}

void Game::ModifyItemMagicInput(int it, int i, int input, int amount)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(i < 0 || i >= (int)(sizeof(ItemDefs[it].mInput)/sizeof(Materials)))
		return;
	if(input < -1 || input > (NITEMS-1)) return;
	if(amount < 0) amount = 0;
	ItemDefs[it].mInput[i].item = input;
	ItemDefs[it].mInput[i].amt = amount;
}

void Game::ModifyRaceSkillLevels(char *r, int spec, int def)
{
	ManType *mt = FindRace(r);
	if (mt == NULL) return;
	if(spec < 0) spec = 0;
	if(def < 0) def = 0;
	mt->speciallevel = spec;
	mt->defaultlevel = def;
}

void Game::ModifyRaceSkills(char *r, int i, char *sk)
{
	ManType *mt = FindRace(r);
	if (mt == NULL) return;
	if(i < 0 || i >= (int)(sizeof(mt->skills) / sizeof(mt->skills[0]))) return;
	if (sk && (FindSkill(sk) == NULL)) return;
	mt->skills[i] = sk;
}

void Game::ModifyMonsterAttackLevel(char *mon, int lev)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(lev < 0) return;
	pM->attackLevel = lev;
}

void Game::ModifyMonsterDefense(char *mon, int defenseType, int level)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(defenseType < 0 || defenseType > (NUM_ATTACK_TYPES -1)) return;
	pM->defense[defenseType] = level;
}

void Game::ModifyMonsterAttacksAndHits(char *mon, int num, int hits, int regen)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(num < 0) return;
	if(hits < 0) return;
	if(regen < 0) return;
	pM->numAttacks = num;
	pM->hits = hits;
	pM->regen = regen;
}

void Game::ModifyMonsterSkills(char *mon, int tact, int stealth, int obs)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(tact < 0) return;
	if(stealth < 0) return;
	if(obs < 0) return;
	pM->tactics = tact;
	pM->stealth = stealth;
	pM->obs = obs;
}

void Game::ModifyMonsterSpecial(char *mon, char *special, int lev)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if (special && (FindSpecial(special) == NULL)) return;
	if(lev < 0) return;
	pM->special = special;
	pM->specialLevel = lev;
}

void Game::ModifyMonsterSpoils(char *mon, int silver, int spoilType)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(spoilType < -1) return;
	if(silver < 0) return;
	pM->silver = silver;
	pM->spoiltype = spoilType;
}

void Game::ModifyMonsterThreat(char *mon, int num, int hostileChance)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(num < 0) return;
	if(hostileChance < 0 || hostileChance > 100) return;
	pM->hostile = hostileChance;
	pM->number = num;
}

void Game::ModifyWeaponSkills(char *weap, char *baseSkill, char *orSkill)
{
	WeaponType *pw = FindWeapon(weap);
	if (pw == NULL) return;
	if (baseSkill && (FindSkill(baseSkill) == NULL)) return;
	if (orSkill && (FindSkill(orSkill) == NULL)) return;
	pw->baseSkill = baseSkill;
	pw->orSkill = orSkill;
}

void Game::ModifyWeaponFlags(char *weap, int flags)
{
	WeaponType *pw = FindWeapon(weap);
	if (pw == NULL) return;
	pw->flags = flags;
}

void Game::ModifyWeaponAttack(char *weap, int wclass, int attackType,
		int numAtt)
{
	WeaponType *pw = FindWeapon(weap);
	if (pw == NULL) return;
	if(wclass < 0 || wclass > (NUM_WEAPON_CLASSES - 1)) return;
	if(attackType < 0 || attackType > (NUM_ATTACK_TYPES - 1)) return;
	pw->weapClass = wclass;
	pw->attackType = attackType;
	pw->numAttacks = numAtt;
}

void Game::ModifyWeaponBonuses(char *weap, int attack, int defense, int vsMount)
{
	WeaponType *pw = FindWeapon(weap);
	if (pw == NULL) return;
	pw->attackBonus = attack;
	pw->defenseBonus = defense;
	pw->mountBonus = vsMount;
}

void Game::ModifyArmorFlags(char *armor, int flags)
{
	ArmorType *pa = FindArmor(armor);
	if (pa == NULL) return;
	pa->flags = flags;
}

void Game::ModifyArmorSaveFrom(char *armor, int from)
{
	ArmorType *pa = FindArmor(armor);
	if (pa == NULL) return;
	if(from < 0) return;
	pa->from = from;
}

void Game::ModifyArmorSaveValue(char *armor, int wclass, int val)
{
	ArmorType *pa = FindArmor(armor);
	if (pa == NULL) return;
	if(wclass < 0 || wclass > (NUM_WEAPON_CLASSES - 1)) return;
	if(val < 0 || val > pa->from) return;
	pa->saves[wclass] = val;
}

void Game::ModifyMountSkill(char *mount, char *skill)
{
	MountType *pm = FindMount(mount);
	if (pm == NULL) return;
	if (skill && (FindSkill(skill) == NULL)) return;
	pm->skill = skill;
}

void Game::ModifyMountBonuses(char *mount, int min, int max, int hampered)
{
	MountType *pm = FindMount(mount);
	if (pm == NULL) return;
	if(min < 0) return;
	if(max < 0) return;
	if(hampered < min) return;
	pm->minBonus = min;
	pm->maxBonus = max;
	pm->maxHamperedBonus = hampered;
}

void Game::ModifyMountSpecial(char *mount, char *special, int level)
{
	MountType *pm = FindMount(mount);
	if (pm == NULL) return;
	if (special && (FindSpecial(special) == NULL)) return;
	if(level < 0) return;
	pm->mountSpecial = special;
	pm->specialLev = level;
}

void Game::EnableObject(int obj)
{
	if(obj < 0 || obj > (NOBJECTS-1)) return;
	ObjectDefs[obj].flags &= ~ObjectType::DISABLED;
}

void Game::DisableObject(int obj)
{
	if(obj < 0 || obj > (NOBJECTS-1)) return;
	ObjectDefs[obj].flags |= ObjectType::DISABLED;
}

void Game::ModifyObjectFlags(int ob, int flags)
{
	if(ob < 0 || ob > (NOBJECTS-1)) return;
	ObjectDefs[ob].flags = flags;
}

void Game::ModifyObjectDecay(int ob, int maxMaint, int maxMonthDecay, int mFact)
{
	if(ob < 0 || ob > (NOBJECTS-1)) return;
	if(maxMonthDecay > maxMaint) return;
	if(maxMaint < 0) return;
	if(maxMonthDecay < 0) return;
	if(mFact < 0) return;
	ObjectDefs[ob].maxMaintenance = maxMaint;
	ObjectDefs[ob].maxMonthlyDecay = maxMonthDecay;
	ObjectDefs[ob].maintFactor = mFact;
}

void Game::ModifyObjectProduction(int ob, int it)
{
	if(ob < 0 || ob > (NOBJECTS-1)) return;
	if(it < -1 || it > (NITEMS -1)) return;
	ObjectDefs[ob].productionAided = it;
}

void Game::ModifyObjectMonster(int ob, int monster)
{
	if(ob < 0 || ob > (NOBJECTS-1)) return;
	if(monster < -1 || monster > (NITEMS -1)) return;
	ObjectDefs[ob].monster = monster;
}

void Game::ModifyObjectConstruction(int ob, int it, int num, char *sk, int lev)
{
	if(ob < 0 || ob > (NOBJECTS-1)) return;
	if((it < -1 && it != I_WOOD_OR_STONE) || it > (NITEMS -1))
		return;
	if(num < 0) return;
	if (sk && FindSkill(sk) == NULL) return;
	if(lev < 0) return;
	ObjectDefs[ob].item = it;
	ObjectDefs[ob].cost = num;
	ObjectDefs[ob].skill = sk;
	ObjectDefs[ob].level = lev;
}

void Game::ModifyObjectManpower(int ob, int prot, int cap, int sail, int mages)
{
	if(ob < 0 || ob > (NOBJECTS-1)) return;
	if(prot < 0) return;
	if(cap < 0) return;
	if(sail < 0) return;
	if(mages < 0) return;
	ObjectDefs[ob].protect = prot;
	ObjectDefs[ob].capacity = cap;
	ObjectDefs[ob].sailors = sail;
	ObjectDefs[ob].maxMages = mages;
}

void Game::ModifyObjectDefence(int ob, int co, int en, int sp, int we, int ri, int ra)
{
	if(ob < 0 || ob > (NOBJECTS-1)) return;
	//if(val < 0) return;	// we could conceivably have a negative value 
								// associated with a structure
	ObjectDefs[ob].defenceArray[0] = co;
	ObjectDefs[ob].defenceArray[1] = en;
	ObjectDefs[ob].defenceArray[2] = sp;
	ObjectDefs[ob].defenceArray[3] = we;
	ObjectDefs[ob].defenceArray[4] = ri;
	ObjectDefs[ob].defenceArray[5] = ra;
}

void Game::ClearTerrainRaces(int t)
{
	if(t < 0 || t > R_NUM-1) return;
	unsigned int c;
	for(c = 0; c < sizeof(TerrainDefs[t].races)/sizeof(int); c++) {
		TerrainDefs[t].races[c] = -1;
	}
	for(c = 0; c < sizeof(TerrainDefs[t].coastal_races)/sizeof(int); c++) {
		TerrainDefs[t].coastal_races[c] = -1;
	}
}

void Game::ModifyTerrainRace(int t, int i, int r)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(i < 0 || i >= (int)(sizeof(TerrainDefs[t].races)/sizeof(int))) return;
	if(r < -1 || r > NITEMS-1) r = -1;
	if(r != -1 && !(ItemDefs[r].type & IT_MAN)) r = -1;
	TerrainDefs[t].races[i] = r;
}

void Game::ModifyTerrainCoastRace(int t, int i, int r)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(i < 0 || i >= (int)(sizeof(TerrainDefs[t].coastal_races)/sizeof(int)))
		return;
	if(r < -1 || r > NITEMS-1) r = -1;
	if(r != -1 && !(ItemDefs[r].type & IT_MAN)) r = -1;
	TerrainDefs[t].coastal_races[i] = r;
}

void Game::ClearTerrainItems(int terrain)
{
	if(terrain < 0 || terrain > R_NUM-1) return;

	for(unsigned int c = 0;
			c < sizeof(TerrainDefs[terrain].prods)/sizeof(Product);
			c++) {
		TerrainDefs[terrain].prods[c].product = -1;
		TerrainDefs[terrain].prods[c].chance = 0;
		TerrainDefs[terrain].prods[c].amount = 0;
	}
}

void Game::ModifyTerrainItems(int terrain, int i, int p, int c, int a)
{
	if(terrain < 0 || terrain > (R_NUM -1)) return;
	if(i < 0 || i >= (int)(sizeof(TerrainDefs[terrain].prods)/sizeof(Product)))
		return;
	if(p < -1 || p > NITEMS-1) p = -1;
	if(c < 0 || c > 100) c = 0;
	if(a < 0) a = 0;
	TerrainDefs[terrain].prods[i].product = p;
	TerrainDefs[terrain].prods[i].chance = c;
	TerrainDefs[terrain].prods[i].amount = a;
}

void ModifyTerrainWMons(int t, int freq, int smon, int bigmon, int hum)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(freq < 0) freq = 0;
	if(smon < -1 || smon > NITEMS-1) smon = -1;
	if(bigmon < -1 || bigmon > NITEMS-1) bigmon = -1;
	if(hum < -1 || hum > NITEMS-1) hum = -1;
	TerrainDefs[t].wmonfreq = freq;
	TerrainDefs[t].smallmon = smon;
	TerrainDefs[t].bigmon = bigmon;
	TerrainDefs[t].humanoid = hum;
}

void ModifyTerrainLairChance(int t, int chance)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(chance < 0 || chance > 100) chance = 0;
	// Chance is percent out of 100 that should have some lair
	TerrainDefs[t].lairChance = chance;
}

void ModifyTerrainLair(int t, int i, int l)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(i < 0 || i >= (int)(sizeof(TerrainDefs[t].lairs)/sizeof(int))) return;
	if(l < -1 || l > NOBJECTS-1) l = -1;
	TerrainDefs[t].lairs[i] = l;
}

void Game::ModifyTerrainEconomy(int t, int pop, int wages, int econ, int move)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(pop < 0) pop = 0;
	if(wages < 0) wages = 0;
	if(econ < 0) econ = 0;
	if(move < 1) move = 1;
	TerrainDefs[t].pop = pop;
	TerrainDefs[t].wages = wages;
	TerrainDefs[t].economy = econ;
	TerrainDefs[t].movepoints = move;
}

void Game::ModifyBattleItemFlags(char *item, int flags)
{
	BattleItemType *pb = FindBattleItem(item);
	if (pb == NULL) return;
	pb->flags = flags;
}

void Game::ModifyBattleItemSpecial(char *item, char *special, int level)
{
	BattleItemType *pb = FindBattleItem(item);
	if (pb == NULL) return;
	if (special && (FindSpecial(special) == NULL)) return;
	if(level < 0) return;
	pb->special = special;
	pb->skillLevel = level;
}

void Game::ModifySpecialTargetFlags(char *special, int targetflags)
{
	SpecialType *sp = FindSpecial(special);
	if (sp == NULL) return;
	sp->targflags = targetflags;
}

void Game::ModifySpecialTargetObjects(char *special, int index, int obj)
{
	SpecialType *sp = FindSpecial(special);
	if (sp == NULL) return;
	if(index < 0 || index > 3) return;
	if((obj != -1 && obj < 1) || (obj > (NOBJECTS-1))) return;
	sp->buildings[index] = obj;
}

void Game::ModifySpecialTargetItems(char *special, int index, int item)
{
	SpecialType *sp = FindSpecial(special);
	if (sp == NULL) return;
	if(index < 0 || index > 7) return;
	if(item < -1 || item > (NITEMS-1)) return;
	sp->targets[index] = item;
}

void Game::ModifySpecialTargetEffects(char *special, int index, char *effect)
{
	SpecialType *sp = FindSpecial(special);
	if (sp == NULL) return;
	if(index < 0 || index > 3) return;
	if (effect && (FindEffect(effect) == NULL)) return;
	sp->effects[index] = effect;
}

void Game::ModifySpecialEffectFlags(char *special, int effectflags)
{
	SpecialType *sp = FindSpecial(special);
	if (sp == NULL) return;
	sp->effectflags = effectflags;
}

void Game::ModifySpecialShields(char *special, int index, int type)
{
	SpecialType *sp = FindSpecial(special);
	if (sp == NULL) return;
	if(index < 0 || index > 4) return;
	if(type < -1 || type > (NUM_ATTACK_TYPES)) return;
	sp->shield[index] = type;
}

void Game::ModifySpecialDefenseMods(char *special, int index, int type, int val)
{
	SpecialType *sp = FindSpecial(special);
	if (sp == NULL) return;
	if(index < 0 || index > 4) return;
	if(type < -1 || type > (NUM_ATTACK_TYPES)) return;
	sp->defs[index].type = type;
	sp->defs[index].val = val;
}

void Game::ModifySpecialDamage(char *special, int index, int type, int min,
		int val, int flags, int cls, char *effect)
{
	SpecialType *sp = FindSpecial(special);
	if (sp == NULL) return;
	if(index < 0 || index > 4) return;
	if (effect && (FindEffect(effect) == NULL)) return;
	if(type < -1 || type > NUM_ATTACK_TYPES) return;
	if(cls < -1 || cls > (NUM_WEAPON_CLASSES-1)) return;
	if(min < 0) return;
	sp->damage[index].type = type;
	sp->damage[index].minnum = min;
	sp->damage[index].value = val;
	sp->damage[index].flags = flags;
	sp->damage[index].dclass = cls;
	sp->damage[index].effect = effect;
}

void Game::ModifyEffectFlags(char *effect, int flags)
{
	EffectType *ep = FindEffect(effect);
	if (ep == NULL) return;
	ep->flags = flags;
}

void Game::ModifyEffectAttackMod(char *effect, int val)
{
	EffectType *ep = FindEffect(effect);
	if (ep == NULL) return;
	ep->attackVal = val;
}

void Game::ModifyEffectDefenseMod(char *effect, int index, int type, int val)
{
	EffectType *ep = FindEffect(effect);
	if (ep == NULL) return;
	if(type < 0 || type > NUM_ATTACK_TYPES) return;
	if(index < 0 || index > 4) return;
	ep->defMods[index].type = type;
	ep->defMods[index].val = val;
}

void Game::ModifyEffectCancelEffect(char *effect, char *uneffect)
{
	EffectType *ep = FindEffect(effect);
	if (ep == NULL) return;
	if (uneffect && (FindEffect(uneffect) == NULL)) return;
	ep->cancelEffect = uneffect;
}

void Game::ModifyRangeFlags(char *range, int flags)
{
	RangeType *rp = FindRange(range);
	if (rp == NULL) return;
	rp->flags = flags;
}

void Game::ModifyRangeClass(char *range, int rclass)
{
	RangeType *rp = FindRange(range);
	if (rp == NULL) return;
	if(rclass < 0 || rclass > (RangeType::NUMRANGECLASSES-1)) return;
	rp->rangeClass = rclass;
}

void Game::ModifyRangeMultiplier(char *range, int mult)
{
	RangeType *rp = FindRange(range);
	if (rp == NULL) return;
	if(mult < 1) return;
	rp->rangeMult = mult;
}

void Game::ModifyRangeLevelPenalty(char *range, int pen)
{
	RangeType *rp = FindRange(range);
	if (rp == NULL) return;
	if(pen < 0) return;
	rp->crossLevelPenalty = pen;
}

void Game::ModifyAttribMod(char *mod, int index, int flags, char *ident,
		int type, int val)
{
	AttribModType *mp = FindAttrib(mod);
	if (mp == NULL) return;
	if (index < 0 || index > 5) return;
	if (!ident) return;
	if (type < 0 || type > AttribModItem::NUMMODTYPE-1) return;

	mp->mods[index].flags = flags;
	mp->mods[index].ident = ident;
	mp->mods[index].modtype = type;
	mp->mods[index].val = val;
}

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

void Game::ModifyBaseSkills(int base, int sk1, int sk2, int sk3, int sk4, int sk5)
{
	if(base < 0 || base > (NSKILLS-1)) return;
	if(SkillDefs[base].baseskill != base) return;
    if(sk1 > -1 && sk1 < NSKILLS) SkillDefs[sk1].baseskill = base;
    if(sk2 > -1 && sk2 < NSKILLS) SkillDefs[sk2].baseskill = base;
    if(sk3 > -1 && sk3 < NSKILLS) SkillDefs[sk3].baseskill = base;
    if(sk4 > -1 && sk4 < NSKILLS) SkillDefs[sk4].baseskill = base;
    if(sk5 > -1 && sk5 < NSKILLS) SkillDefs[sk5].baseskill = base;
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

void Game::ModifySkillName(int sk, char *name, char *abbr)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	SkillDefs[sk].name = name;
	SkillDefs[sk].abbr = abbr;
}




/// Switch an item on so it can be used in-game.
/** 
\arg \c item One of the values from the NITEMS enum in gamedata.h
*/
void Game::EnableItem(int item)
{
	if(item < 0 || item > (NITEMS-1)) return;
	ItemDefs[item].flags &= ~ItemType::DISABLED;
}

/// Switch an item off so it cannot be used in-game.
/** 
\arg \c item One of the values from the NITEMS enum in gamedata.h
*/
void Game::DisableItem(int item)
{
	if(item < 0 || item > (NITEMS-1)) return;
	ItemDefs[item].flags |= ItemType::DISABLED;
}

/// Modify some of the flags that affect an item's behavior.
/** The flags are things like ItemType::NOMARKET, ItemType::NOTRANSPORT, etc.
\arg \c it One of the values from the NITEMS enum in gamedata.h
\arg \c flags A series of flags as defined in the ItemType class in items.h
*/
void Game::ModifyItemFlags(int it, int flags)
{
	if(it < 0 || it > (NITEMS-1)) return;
	ItemDefs[it].flags = flags;
}

/// Set what type of Item this is.
/** The type is an enum, and can be things like IT_MAN | IT_LEADER, etc.
See the enum at line 46 of items.h for more details.
\arg \c it One of the values from the NITEMS enum in gamedata.h
\arg \c type The type of object as defined in the an enum in items.h
*/
void Game::ModifyItemType(int it, int type)
{
	if(it < 0 || it > (NITEMS-1)) return;
	ItemDefs[it].type = type;
}

/// Change the weight of an item
/** Weight 0 items may still have a fractional weight - see the 
FRACTIONAL_WEIGHT gamedef in rules.cpp
\arg \c it One of the values from the NITEMS enum in gamedata.h
\arg \c type The weight of the object
*/
void Game::ModifyItemWeight(int it, int weight)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(weight < 0) weight = 0;
	ItemDefs[it].weight = weight;
}

/// Change the base price of an item
/** The base price can go up or down depending on the economy of a hex.
Prices tend to go up in cities, and down in the boonies.
\arg \c it One of the values from the NITEMS enum in gamedata.h
\arg \c type The base price of the object
*/

void Game::ModifyItemName(int it, char *name, char *names, char *abr)
{
	if(it < 0 || it > (NITEMS-1)) return;
	ItemDefs[it].name = name;
	ItemDefs[it].names = names;
	ItemDefs[it].abr = abr;
}

void Game::ModifyItemBasePrice(int it, int price)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(price < 0) price = 0;
	ItemDefs[it].baseprice = price;
}

/// Change how much an item will let a unit carry
/** A unit can only move at a speed if it has enough capacity to carry 
all of its weight

\arg \c it One of the values from the NITEMS enum in gamedata.h
\arg \c wlk How much the item can carry at walking speed
\arg \c rid How much the item can carry at riding speed
\arg \c fly How much the item can carry when flying
\arg \c swm How much the item can carry when swimming
*/
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

/// Change the item that gives you a bonus when producing this item
/** eg. This item would be wood, and the other item might be an I_AXE
\arg \c it One of the values from the NITEMS enum in gamedata.h (eg. I_WOOD)
\arg \c item The tool, also one of the values from the NITEMS enum (eg. I_AXE)
\arg \c bonus How much is added to the production skill
*/
void Game::ModifyItemProductionBooster(int it, int item, int bonus)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(item < -1 || item > (NITEMS-1)) return;
	ItemDefs[it].mult_item = item;
	ItemDefs[it].mult_val = bonus;
}

/// Change the item that can be hitched to this item to pull it along
/** A hitched wagon (or other conveyance) will be pulled at walking speed,
unless of course you have enough horses to carry it.
\arg \c it One of the values from the NITEMS enum in gamedata.h (eg. I_WAGON)
\arg \c item The tool, also one of the values from the NITEMS enum (eg. I_HORS)
\arg \c capacity How much can be pulled along in the wagon
*/
void Game::ModifyItemHitch(int it, int item, int capacity)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(item < -1 || item > (NITEMS-1)) return;
	if(capacity < 0) return;
	ItemDefs[it].hitchItem = item;
	ItemDefs[it].hitchwalk = capacity;
}

/// Change the skill required to produce this item
/** eg. This item would be wood, and the skill would be "LUMB"
Set this to NULL if the item cannot be produced through normal means.
\arg \c it One of the values from the NITEMS enum in gamedata.h (eg. I_WOOD)
\arg \c *sk The skill as a string, eg. "LUMB", "ARMO", etc.
\arg \c lev How much skill is needed to produce the item 
                (eg. PARM requires ARMO of level 3)
*/
void Game::ModifyItemProductionSkill(int it, char *sk, int lev)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if (sk && (FindSkill(sk) == NULL)) return;
	ItemDefs[it].pSkill = sk;
	ItemDefs[it].pLevel = lev;
}

/// Change the number of man hours required to produce 1 item
/** eg. PARM needs 3 man hours. This doesn't include the skill multiplier,
so a skill-3 unit will produce 3 times as much as this number.
\arg \c it One of the values from the NITEMS enum in gamedata.h (eg. I_WOOD)
\arg \c months The number of man months required to produce 1 item
\arg \c lev How much skill is needed to produce the item 
                (eg. PARM requires ARMO of level 3)
*/
void Game::ModifyItemProductionOutput(int it, int months, int count)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(count < 0) count = 0;
	if(months < 0) months = 0;
	ItemDefs[it].pMonths = months;
	ItemDefs[it].pOut = count;
}

/// Change the raw materials required to produce 1 item
/** eg. 1 SWOR needs 1 IRON, 1 PARM needs 3 IRON. 
Set the variable 'it' to -1 to require no item.
\arg \c it One of the values from the NITEMS enum in gamedata.h (eg. I_WOOD)
\arg \c i There can be up to 4 raw materials needed. Set i to 0, 1, 2 or 3 for each slot
\arg \c input Which raw material is required. This is one of the values from 
                      the NITEMS enum
\arg \c amount How much of the raw material is needed to produce the item 
*/
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

/// Change the magical skill required to produce this item
/** eg. This item would be CARP, and the skill would be "CRMA"
If you have normal skills and magical skills, they are an either-or proposition
eg. you don't need both WEAP-3 and ESWO to create MSWO. Set this to NULL if 
no magical skill can produce this item.
\arg \c it One of the values from the NITEMS enum in gamedata.h (eg. I_CARP)
\arg \c *sk The skill as a string, eg. "CRMA", "CRRI", etc.
\arg \c lev How much skill is needed to produce the item
*/
void Game::ModifyItemMagicSkill(int it, char *sk, int lev)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if (sk && (FindSkill(sk) == NULL)) return;
	ItemDefs[it].mSkill = sk;
	ItemDefs[it].mLevel = lev;
}

/// Change the number of items produced per spell
/** eg. ESWO produces 5 MSWO per casting. Note that this is the opposite way
around to mundane items.
\arg \c it One of the values from the NITEMS enum in gamedata.h (eg. I_WOOD)
\arg \c count The number of items produced per casting
*/
void Game::ModifyItemMagicOutput(int it, int count)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(count < 0) count = 0;
	ItemDefs[it].mOut = count;
}

/// Change the raw materials required to produce 1 item via magical means
/** eg. 1 MSWO needs 1 SWOR, 1 MARM needs 1 PARM. 
Set the variable 'it' to -1 to require no item.
\arg \c it One of the values from the NITEMS enum in gamedata.h (eg. I_WOOD)
\arg \c i There can be up to 4 raw materials needed. Set i to 0, 1, 2 or 3 for each slot
\arg \c input Which raw material is required. This is one of the values from 
                      the NITEMS enum
\arg \c amount How much of the raw material is needed to produce the item 
*/
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

/// Change the base skill levels for specialised and non-specialised skills.
/** For most races, this will be 3 for special skills, and 2 for non-special
For orcs, it's 4 and 1; for leaders it's 5 and 5 (although it could be 0 and 5)
\arg \c *r One of the races, eg. "ORC", "HELF", "LEAD"
\arg \c spec The level of any specialised skills that the race knows. 
\arg \c def The default maximum level of any skill
*/
void Game::ModifyRaceSkillLevels(char *r, int spec, int def)
{
	ManType *mt = FindRace(r);
	if (mt == NULL) return;
	if(spec < 0) spec = 0;
	if(def < 0) def = 0;
	mt->speciallevel = spec;
	mt->defaultlevel = def;
}

/// Change the number of hits it takes to kill a race in combat.
/** 
\arg \c *r One of the races, eg. "ORC", "HELF", "LEAD"
\arg \c i How many hit points (should be 1 or more)
*/
void Game::ModifyRaceHits(char *r, int num)
/* BS 030825 Multiple Hit Men */
{
	ManType *mt = FindRace(r);
	if (mt == NULL) return;
	if(num < 1) num = 1;
	mt->hits = num;
}

/// Change the specialised skills that a race knows.
/** 
\arg \c *r One of the races, eg. "ORC", "HELF", "LEAD"
\arg \c i Which slot is being changed (there are six, from 0-5)
\arg \c *sk The skill to be added to the race's specialised skill list, eg. "LUMB"
*/
void Game::ModifyRaceSkills(char *r, int i, char *sk)
{
	ManType *mt = FindRace(r);
	if (mt == NULL) return;
	if(i < 0 || i >= (int)(sizeof(mt->skills) / sizeof(mt->skills[0]))) return;
	if (sk && (FindSkill(sk) == NULL)) return;
	mt->skills[i] = sk;
}

void Game::ModifyRaceSkills(char *r, char *sk1, char *sk2, char *sk3, char *sk4, char *sk5, char *sk6)
{
	ManType *mt = FindRace(r);
	if (mt == NULL) return;
	if (sk1 && (FindSkill(sk1) == NULL)) sk1 = NULL;
	if (sk2 && (FindSkill(sk1) == NULL)) sk2 = NULL;
	if (sk3 && (FindSkill(sk1) == NULL)) sk3 = NULL;
	if (sk4 && (FindSkill(sk1) == NULL)) sk4 = NULL;
	if (sk5 && (FindSkill(sk1) == NULL)) sk5 = NULL;
	if (sk6 && (FindSkill(sk1) == NULL)) sk6 = NULL;
	mt->skills[0] = sk1;
	mt->skills[1] = sk2;
	mt->skills[2] = sk3;
	mt->skills[3] = sk4;
	mt->skills[4] = sk5;
	mt->skills[5] = sk6;
}

/// Change the attacking skill level for a monster (ie it's COMB skill)
/** 
\arg \c *mon The monster being changed (eg. "BALR")
\arg \c lev The attacking level of the monster.
*/
void Game::ModifyMonsterAttackLevel(char *mon, int lev)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
//	if(lev < 0) return;   //disabled by BS so that illusions could be modified.
	pM->attackLevel = lev;
}

/// Change the defensive level for a monster
/** defenseType can either be a number from 0-5, or an enum from items.h (line 36)
\arg \c *mon The monster being changed (eg. "BALR")
\arg \c defenseType The type of attack being defended against (eg. ATTACK_COMBAT)
\arg \c level The new defense level of the monster
*/
void Game::ModifyMonsterDefense(char *mon, int defenseType, int level)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(defenseType < 0 || defenseType > (NUM_ATTACK_TYPES -1)) return;
	pM->defense[defenseType] = level;
}

/// Change a monster's number of attacks, number of hits, and rate of regeneration
/** 
\arg \c *mon The monster being changed (eg. "BALR")
\arg \c num The number of attacks
\arg \c hits The number of hits that the monster can take before dying
\arg \c regen The number of hits that the monster gets back per round of combat
*/
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

/// Change the offensive skills that a monster knows
/** Monsters can give their unit a stea, tact and obse score, although this is not 
cumulative with the unit's current score.
\arg \c *mon The monster being changed (eg. "BALR")
\arg \c tact The monster's tactics score
\arg \c stealth The monster's stealth score
\arg \c obs The monster's observation score
*/
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

/// Change the type or effectiveness of a monster's special attack
/** eg. Dragon's breath, balrog's hellfire spell.
\arg \c *mon The monster being changed (eg. "BALR")
\arg \c *special The name of the special attack (eg. "hellfire", "black_wind")
\arg \c lev The level that the special is cast at
*/
void Game::ModifyMonsterSpecial(char *mon, char *special, int lev)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if (special && (FindSpecial(special) == NULL)) return;
	if(lev < 0) return;
	pM->special = special;
	pM->specialLevel = lev;
}

/// Change the type or amount of spoils that a monster gives out
/** eg. balrogs give out magical items, wolves give silver, kobolds give normal items
Set the item type to -1 if you don't want items given out.
\arg \c *mon The monster being changed (eg. "BALR")
\arg \c silver The base value of the silver and items that are given out
\arg \c spoilType The type of items that are given out as spoils
*/
void Game::ModifyMonsterSpoils(char *mon, int silver, int spoilType)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(spoilType < -1) return;
	if(silver < 0) return;
	pM->silver = silver;
	pM->spoiltype = spoilType;
}

/// Change how likely the monster is to beat things up, and how many will appear
/** This is expressed as a percentage chance of an attack (per turn?)
\arg \c *mon The monster being changed (eg. "BALR")
\arg \c num The maximum number of monsters that will appear
\arg \c hostileChance The percentage chance that a monster will attack someone
*/
void Game::ModifyMonsterThreat(char *mon, int num, int hostileChance)
{
	MonType *pM = FindMonster(mon, 0);
	if (pM == NULL) return;
	if(num < 0) return;
	if(hostileChance < 0 || hostileChance > 100) return;
	pM->hostile = hostileChance;
	pM->number = num;
}

/// Modify the skills required to wield a weapon
/** There's no difference between baseskill or orskill - you can 
wield it if you have either of them.
\arg \c *weap The name of the weapon, eg SWOR
\arg \c *baseSkill The primary skill required to wield the weapon
\arg \c *orSkill Another skill that can be used to wield the weapon
*/
void Game::ModifyWeaponSkills(char *weap, char *baseSkill, char *orSkill)
{
	WeaponType *pw = FindWeapon(weap);
	if (pw == NULL) return;
	if (baseSkill && (FindSkill(baseSkill) == NULL)) return;
	if (orSkill && (FindSkill(orSkill) == NULL)) return;
	pw->baseSkill = baseSkill;
	pw->orSkill = orSkill;
}

/// Modify the flags that alter a weapon's behavior
/** The flags are things like WeaponType::NEEDSKILL, WeaponType::RANGED, etc.
There's a full list in items.h, line 200 on
\arg \c *weap The name of the weapon, eg SWOR
\arg \c *baseSkill The primary skill required to wield the weapon
\arg \c *orSkill Another skill that can be used to wield the weapon
*/
void Game::ModifyWeaponFlags(char *weap, int flags)
{
	WeaponType *pw = FindWeapon(weap);
	if (pw == NULL) return;
	pw->flags = flags;
}

/// Modify the class and type of a weapon's attack
/**
\arg \c *weap The name of the weapon, eg "SWOR"
\arg \c wclass The class of the weapon, eg. ARMOR_PIERCING, SLASHING
\arg \c attackType The type of attack, eg. ATTACK_RANGED, ATTACK_COMBAT
*/
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

/// Modify the bonus that a weapon gets in attack, defense and 
///when fighting mounted opponents
/**
\arg \c *weap The name of the weapon, eg "SWOR"
\arg \c attack The weapon's bonus to attack
\arg \c defense The weapon's bonus in defense
\arg \c vsMount The weapon's bonus against mounted opponents
*/
void Game::ModifyWeaponBonuses(char *weap, int attack, int defense, int vsMount)
{
	WeaponType *pw = FindWeapon(weap);
	if (pw == NULL) return;
	pw->attackBonus = attack;
	pw->defenseBonus = defense;
	pw->mountBonus = vsMount;
}

/// Modify the flags that govern an armor's behavior
/** Currently there's only one flag: ArmorType::USEINASSASSINATE
\arg \c *armor The armor's name, eg. "CLAR"
\arg \c flags The flags to apply to the armor.
*/
void Game::ModifyArmorFlags(char *armor, int flags)
{
	ArmorType *pa = FindArmor(armor);
	if (pa == NULL) return;
	pa->flags = flags;
}

/// Modify the divisor for the save chances in ModifyArmorSaveValue
/** eg. If this is 300, and the save for COMBAT is 100, then the armor
has 100 in 300 chances of protecting it's wearer vs. COMBAT attacks.
\arg \c *armor The armor's name, eg. "CLAR"
\arg \c from The saving throw's divisor
*/
void Game::ModifyArmorSaveFrom(char *armor, int from)
{
	ArmorType *pa = FindArmor(armor);
	if (pa == NULL) return;
	if(from < 0) return;
	pa->from = from;
}

/// Modify the numerator for the armor's save chances
/** eg. If this is 100 for COMBAT, and the savefrom is 300, then the armor
has 100 in 300 chances of protecting it's wearer vs. COMBAT attacks.
\arg \c *armor The armor's name, eg. "CLAR"
\arg \c wclass The saving throw vs. a particular type of attack (eg. SLASHING)
\arg \c val The numerator in the saving throw. (eg. 100)
*/
void Game::ModifyArmorSaveValue(char *armor, int wclass, int val)
{
	ArmorType *pa = FindArmor(armor);
	if (pa == NULL) return;
	if(wclass < 0 || wclass > (NUM_WEAPON_CLASSES - 1)) return;
	if(val < 0 || val > pa->from) return;
	pa->saves[wclass] = val;
}


void Game::ModifyArmorSaveAll(char *armor, int from, int melee, int armourpiercing, int magic)
{
	ArmorType *pa = FindArmor(armor);
	if (pa == NULL) return;
	if(from < 0 || melee < 0 || armourpiercing < 0 || magic < 0) return;
	pa->from = from;
	pa->saves[SLASHING] = melee;
	pa->saves[CRUSHING] = melee;
	pa->saves[CLEAVING] = melee;
	pa->saves[PIERCING] = melee;
	pa->saves[ARMORPIERCING] = armourpiercing;
	pa->saves[MAGIC_ENERGY] = magic;
	pa->saves[MAGIC_SPIRIT] = magic;
	pa->saves[MAGIC_WEATHER] = magic;
}

/// Modify the skill required to ride a mount
/** see line 2488 in gamedata.cpp
\arg \c *mount The mount name, eg. "HORS"
\arg \c *skill The skill's name eg. "RIDI"
*/
void Game::ModifyMountSkill(char *mount, char *skill)
{
	MountType *pm = FindMount(mount);
	if (pm == NULL) return;
	if (skill && (FindSkill(skill) == NULL)) return;
	pm->skill = skill;
}

/// Modify the bonuses given when mounts are ridden into combat
/** 
\arg \c *mount The mount name, eg. "HORS"
\arg \c min The minimum bonus given
\arg \c max The maximum bonus given
\arg \c hampered The bonus given when a mount is hampered 
								(eg. WING horses underground/in tunnels?)
*/
void Game::ModifyMountBonuses(char *mount, int min, int max, int hampered)
{
	MountType *pm = FindMount(mount);
	if (pm == NULL) return;
	if(min < 0) return;
	if(max < 0) return;
//	if(hampered < min) return;
	pm->minBonus = min;
	pm->maxBonus = max;
	pm->maxHamperedBonus = hampered;
}

/// Modify the special attacks that a mount has
/** The only mount this is used for atm is the fear attacks 
		vs. mounted troops that camels get
\arg \c *mount The mount name, eg. "HORS"
\arg \c *special The special attack (eg. "spook_horses")
\arg \c level The level of the attack
*/
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

void Game::EnableHexside(int hex)
{
	if(hex < 0 || hex > (NHEXSIDES-1)) return;
	HexsideDefs[hex].flags &= ~HexsideType::DISABLED;
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

void Game::ModifyTerrainWMons(int t, int freq, int smon, int bigmon, int hum)
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

void Game::ModifyTerrainLairChance(int t, int chance)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(chance < 0 || chance > 100) chance = 0;
	// Chance is percent out of 100 that should have some lair
	TerrainDefs[t].lairChance = chance;
}

void Game::ModifyTerrainLair(int t, int i, int l)
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
	if(cls < -1 || cls > (NUM_WEAPON_CLASSES)) return;   //NUM_WEAPON_CLASSES is allowed for courage, which ignores armour completely
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

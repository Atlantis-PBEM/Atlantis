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
	if(sk < 0 || sk > (NSKILLS-1)) return;
	SkillDefs[sk].flags &= ~SkillType::DISABLED;
}

void Game::DisableSkill(int sk)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	SkillDefs[sk].flags |= SkillType::DISABLED;
}

void Game::ModifySkillDependancy(int sk, int i, int dep, int lev)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	if(i < 0 || i >= (int)(sizeof(SkillDefs[sk].depends)/sizeof(SkillDepend)))
		return;
	if(dep < -1 || dep > (NSKILLS-1)) return;
	if(lev < 0) return;
	SkillDefs[sk].depends[i].skill = dep;
	SkillDefs[sk].depends[i].level = lev;
}

void Game::ModifySkillFlags(int sk, int flags)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	SkillDefs[sk].flags = flags;
}

void Game::ModifySkillCost(int sk, int cost)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	if(cost < 0) return;
	SkillDefs[sk].cost = cost;
}

void Game::ModifySkillSpecial(int sk, int special)
{
	if(sk < 0 || sk > (NSKILLS-1)) return;
	SkillDefs[sk].special = special;
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

void Game::ModifyItemProductionSkill(int it, int sk, int lev)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(sk < -1 || sk > (NSKILLS-1)) return;
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

void Game::ModifyItemMagicSkill(int it, int sk, int lev)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(sk < -1 || sk > (NSKILLS-1)) return;
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

void Game::ModifyRaceSkillLevels(int r, int spec, int def)
{
	if(r < 0 || r > (NUMMAN-1)) return;
	if(spec < 0) spec = 0;
	if(def < 0) def = 0;
	ManDefs[r].speciallevel = spec;
	ManDefs[r].defaultlevel = def;
}

void Game::ModifyRaceSkills(int r, int i, int sk)
{
	if(r < 0 || r > (NUMMAN-1)) return;
	if(i < 0 || i >= (int)(sizeof(ManDefs[r].skills)/sizeof(int))) return;
	if(sk < -1 || sk > (NSKILLS-1)) sk = -1;

	ManDefs[r].skills[i] = sk;
}

void Game::ModifyMonsterAttackLevel(int mon, int lev)
{
	if(mon < 0 || mon > (NUMMONSTERS - 1)) return;
	if(lev < 0) return;
	MonDefs[mon].attackLevel = lev;
}

void Game::ModifyMonsterDefense(int mon, int defenseType, int level)
{
	if(mon < 0 || mon > (NUMMONSTERS - 1)) return;
	if(defenseType < 0 || defenseType > (NUM_ATTACK_TYPES -1)) return;
	MonDefs[mon].defense[defenseType] = level;
}

void Game::ModifyMonsterAttacksAndHits(int mon, int numattacks, int hits)
{
	if(mon < 0 || mon > (NUMMONSTERS - 1)) return;
	if(numattacks < 0) return;
   	if(hits < 0) return;
	MonDefs[mon].numAttacks = numattacks;
	MonDefs[mon].hits = hits;
}

void Game::ModifyMonsterSkills(int mon, int tact, int stealth, int obs)
{
	if(mon < 0 || mon > (NUMMONSTERS - 1)) return;
	if(tact < 0) return;
	if(stealth < 0) return;
	if(obs < 0) return;
	MonDefs[mon].tactics = tact;
	MonDefs[mon].stealth = stealth;
	MonDefs[mon].obs = obs;
}

void Game::ModifyMonsterSpecial(int mon, int special, int lev)
{
	if(mon < 0 || mon > (NUMMONSTERS - 1)) return;
	if(special < -1 || special > (NUMSPECIALS -1)) return;
	if(lev < 0) return;
	MonDefs[mon].special = special;
	MonDefs[mon].specialLevel = lev;
}

void Game::ModifyMonsterSpoils(int mon, int silver, int spoilType)
{
	if(mon < 0 || mon > (NUMMONSTERS - 1)) return;
	if(spoilType < -1) return;
	if(silver < 0) return;
	MonDefs[mon].silver = silver;
	MonDefs[mon].spoiltype = spoilType;
}

void Game::ModifyMonsterThreat(int mon, int num, int hostileChance)
{
	if(mon < 0 || mon > (NUMMONSTERS - 1)) return;
	if(num < 0) return;
	if(hostileChance < 0 || hostileChance > 100) return;
	MonDefs[mon].hostile = hostileChance;
	MonDefs[mon].number = num;
}

void Game::ModifyWeaponSkills(int weap, int baseSkill, int orSkill)
{
	if(weap < 0 || weap > (NUMWEAPONS - 1)) return;
	if(baseSkill < -1 || baseSkill > (NSKILLS - 1)) return;
	if(orSkill < -1 || orSkill > (NSKILLS - 1)) return;
	WeaponDefs[weap].baseSkill = baseSkill;
	WeaponDefs[weap].orSkill = orSkill;
}

void Game::ModifyWeaponFlags(int weap, int flags)
{
	if(weap < 0 || weap > (NUMWEAPONS - 1)) return;
	WeaponDefs[weap].flags = flags;
}

void Game::ModifyWeaponAttack(int weap, int wclass, int attackType, int numAtt)
{
	if(weap < 0 || weap > (NUMWEAPONS - 1)) return;
	if(wclass < 0 || wclass > (NUM_WEAPON_CLASSES - 1)) return;
	if(attackType < 0 || attackType > (NUM_ATTACK_TYPES - 1)) return;
	WeaponDefs[weap].weapClass = wclass;
	WeaponDefs[weap].attackType = attackType;
	WeaponDefs[weap].numAttacks = numAtt;
}

void Game::ModifyWeaponBonuses(int weap, int attack, int defense, int vsMount)
{
	if(weap < 0 || weap > (NUMWEAPONS -1)) return;
	WeaponDefs[weap].attackBonus = attack;
	WeaponDefs[weap].defenseBonus = defense;
	WeaponDefs[weap].mountBonus = vsMount;
}

void Game::ModifyArmorFlags(int armor, int flags)
{
	if(armor < 0 || armor > (NUMARMORS - 1)) return;
	ArmorDefs[armor].flags = flags;
}

void Game::ModifyArmorSaveFrom(int armor, int from)
{
	if(armor < 0 || armor > (NUMARMORS - 1)) return;
	if(from < 0) return;
	ArmorDefs[armor].from = from;
}

void Game::ModifyArmorSaveValue(int armor, int wclass, int val)
{
	if(armor < 0 || armor > (NUMARMORS - 1)) return;
	if(wclass < 0 || wclass > (NUM_WEAPON_CLASSES - 1)) return;
	if(val < 0 || val > ArmorDefs[armor].from) return;
	ArmorDefs[armor].saves[wclass] = val;
}

void Game::ModifyMountSkill(int mount, int skill)
{
	if(mount < 0 || mount > (NUMMOUNTS - 1)) return;
	if(skill < -1 || skill > (NSKILLS - 1)) return;
	MountDefs[mount].skill = skill;
}

void Game::ModifyMountBonuses(int mount, int min, int max, int hampered)
{
	if(mount < 0 || mount > (NUMMOUNTS - 1)) return;
	if(min < 0) return;
	if(max < 0) return;
	if(hampered < min) return;
	MountDefs[mount].minBonus = min;
	MountDefs[mount].maxBonus = max;
	MountDefs[mount].maxHamperedBonus = hampered;
}

void Game::ModifyMountSpecial(int mount, int special, int level)
{
	if(mount < 0 || mount > (NUMMOUNTS - 1)) return;
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	if(level < 0) return;
	MountDefs[mount].mountSpecial = special;
	MountDefs[mount].specialLev = level;
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

void Game::ModifyObjectConstruction(int ob, int it, int num, int sk, int lev)
{
	if(ob < 0 || ob > (NOBJECTS-1)) return;
	if((it < -1 && it != I_WOOD_OR_STONE) || it > (NITEMS -1))
		return;
	if(num < 0) return;
	if(sk < -1 || sk > (NSKILLS - 1)) return;
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

void Game::ModifyBattleItemFlags(int item, int flags)
{
	if(item < 0 || item > (NUMBATTLEITEMS-1)) return;
	BattleItemDefs[item].flags = flags;
}

void Game::ModifyBattleItemSpecial(int item, int index, int level)
{
	if(item < 0 || item > (NUMBATTLEITEMS-1)) return;
	if(level < 0) return;

	BattleItemDefs[item].index = index;
	BattleItemDefs[item].skillLevel = level;
}

void Game::ModifySpecialTargetFlags(int special, int targetflags)
{
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	SpecialDefs[special].targflags = targetflags;
}

void Game::ModifySpecialTargetObjects(int special, int index, int obj)
{
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	if(index < 0 || index > 3) return;
	if((obj != -1 && obj < 1) || (obj > (NOBJECTS-1))) return;
	SpecialDefs[special].buldings[index] = obj;
}

void Game::ModifySpecialTargetItems(int special, int index, int item)
{
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	if(index < 0 || index > 7) return;
	if(item < -1 || item > (NITEMS-1)) return;
	SpecialDefs[special].targets[index] = item;
}

void Game::ModifySpecialTargetEffects(int special, int index, int effect)
{
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	if(index < 0 || index > 3) return;
	if(effect < -1 || effect > (NUMEFFECTS-1)) return;
	SpecialDefs[special].effects[index] = effect;
}

void Game::ModifySpecialEffectFlags(int special, int effectflags)
{
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	SpecialDefs[special].effectflags = effectflags;
}

void Game::ModifySpecialShieldData(int special, int index, int type, int val)
{
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	if(index < 0 || index > 4) return;
	if(type < -1 || type > (NUM_ATTACK_TYPES)) return;
	SpecialDefs[special].shield[index].type = type;
	SpecialDefs[special].shield[index].value = val;
}

void Game::ModifySpecialDamage(int special, int index, int type, int min,
		int val, int flags, int cls, int effect)
{
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	if(index < 0 || index > 4) return;
	if(effect < 0 || effect > (NUMEFFECTS-1)) return;
	if(type < -1 || type > NUM_ATTACK_TYPES) return;
	if(cls < -1 || cls > (NUM_WEAPON_CLASSES-1)) return;
	if(min < 0) return;
	SpecialDefs[special].damage[index].type = type;
	SpecialDefs[special].damage[index].minnum = min;
	SpecialDefs[special].damage[index].value = val;
	SpecialDefs[special].damage[index].flags = flags;
	SpecialDefs[special].damage[index].dclass = cls;
	SpecialDefs[special].damage[index].effect = effect;
}

void Game::ModifySpecialTargetLevelAdj(int special, int lev)
{
	if(special < 0 || special > (NUMSPECIALS-1)) return;
	SpecialDefs[special].targetLevAdj = lev;
}

void Game::ModifyEffectFlags(int effect, int flags)
{
	if(effect < 0 || effect > (NUMEFFECTS-1)) return;
	EffectDefs[effect].flags = flags;
}

void Game::ModifyEffectAttackMod(int effect, int val)
{
	if(effect < 0 || effect > (NUMEFFECTS-1)) return;
	EffectDefs[effect].attackVal = val;
	
}

void Game::ModifyEffectDefenseMod(int effect, int index, int type, int val)
{
	if(effect < 0 || effect > (NUMEFFECTS-1)) return;
	if(type < 0 || type > NUM_ATTACK_TYPES) return;
	if(index < 0 || index > 4) return;
	EffectDefs[effect].defMods[index].type = type;
	EffectDefs[effect].defMods[index].val = val;
}

void Game::ModifyEffectCancelEffect(int effect, int uneffect)
{
	if(effect < 0 || effect > (NUMEFFECTS-1)) return;
	if(uneffect < 0 || uneffect > (NUMEFFECTS-1)) return;
	EffectDefs[effect].cancelEffect = uneffect;
}

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
	if(dep < 0 || dep > (NSKILLS-1)) return;
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
	if(special < 0 || special > (NUMSPECIALS-1)) return;
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
	if(item < 0 || item > (NITEMS-1)) return;
	ItemDefs[it].mult_item = item;
	ItemDefs[it].mult_val = bonus;
}

void Game::ModifyItemProductionSkill(int it, int sk, int lev)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(sk < 0 || sk > (NSKILLS-1)) return;
	ItemDefs[it].pSkill = sk;
	ItemDefs[it].pLevel = lev;
}

void Game::ModifyItemProductionOutput(int it, int months, int count)
{
	if(it < 0 || it > (NITEMS-1)) return;
	ItemDefs[it].pMonths = months;
	ItemDefs[it].pOut = count;
}

void Game::ModifyItemProductionInput(int it, int i, int input, int amount)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(input < 0 || input > (NITEMS-1)) return;
	if(i < 0 || i >= (int)(sizeof(ItemDefs[it].pInput)/sizeof(Materials)))
		return;
	if(amount < 0) amount = 0;
	ItemDefs[it].pInput[i].item = input;
	ItemDefs[it].pInput[i].amt = amount;
}

void Game::ModifyItemMagicSkill(int it, int sk, int lev)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(sk < 0 || sk > (NSKILLS-1)) return;
	ItemDefs[it].mSkill = sk;
	ItemDefs[it].mLevel = lev;
}

void Game::ModifyItemMagicOutput(int it, int count)
{
	if(it < 0 || it > (NITEMS-1)) return;
	ItemDefs[it].mOut = count;
}

void Game::ModifyItemMagicInput(int it, int i, int input, int amount)
{
	if(it < 0 || it > (NITEMS-1)) return;
	if(input < 0 || input > (NITEMS-1)) return;
	if(i < 0 || i >= (int)(sizeof(ItemDefs[it].mInput)/sizeof(Materials)))
		return;
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
	if(sk < 0 || sk > (NSKILLS-1)) sk = -1;

	ManDefs[r].skills[i] = sk;
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


void Game::ClearTerrainRaces(int t)
{
	if(t < 0 || t > R_NUM-1) return;
	unsigned int c;
	for(c = 0; c < sizeof(TerrainDefs[t].races)/sizeof(int); c++) {
		TerrainDefs[t].races[c] = -1;
	}
	for(c = 0; c < sizeof(TerrainDefs[t].coastal_races)/sizeof(int); c++) {
		TerrainDefs[t].races[c] = -1;
	}
}

void Game::ModifyTerrainRace(int t, int i, int r)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(i < 0 || i >= (int)(sizeof(TerrainDefs[t].races)/sizeof(int))) return;
	if(r > NITEMS-1) r = -1;
	if(r != -1 && !(ItemDefs[r].type & IT_MAN)) r = -1;
	TerrainDefs[t].races[i] = r;
}

void Game::ModifyTerrainCoastRace(int t, int i, int r)
{
	if(t < 0 || t > (R_NUM -1)) return;
	if(i < 0 || i >= (int)(sizeof(TerrainDefs[t].coastal_races)/sizeof(int)))
		return;
	if(r > NITEMS-1) r = -1;
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
	if(p > NITEMS-1) p = -1;
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
	if(smon > NITEMS-1) smon = -1;
	if(bigmon > NITEMS-1) bigmon = -1;
	if(hum > NITEMS-1) hum = -1;
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
	if(l > NOBJECTS-1) l = -1;
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

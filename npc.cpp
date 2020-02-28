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
#include "game.h"
#include "gamedata.h"

void Game::CreateCityMons()
{
	if (!Globals->CITY_MONSTERS_EXIST) return;

	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		if ((r->type == R_NEXUS) || r->IsStartingCity() || r->town) {
			CreateCityMon(r, 100, 1);
		}
	}
}

void Game::CreateWMons()
{
	if (!Globals->WANDERING_MONSTERS_EXIST) return;

	GrowWMons(50);
}

void Game::CreateLMons()
{
	if (!Globals->LAIR_MONSTERS_EXIST) return;

	GrowLMons(50);
}

void Game::GrowWMons(int rate)
{
	//
	// Now, go through each 8x8 block of the map, and make monsters if
	// needed.
	//
	int level;
	for (level = 0; level < regions.numLevels; level++) {
		ARegionArray *pArr = regions.pRegionArrays[level];

		for (int xsec=0; xsec < pArr->x; xsec+=8) {
			for (int ysec=0; ysec < pArr->y; ysec+=16) {
				int mons=0;
				int wanted=0;

				for (int x=0; x < 8; x++) {
					if (x+xsec > pArr->x) break;

					for (int y=0; y < 16; y+=2) {
						if (y+ysec > pArr->y) break;

						ARegion *reg = pArr->GetRegion(x+xsec, y+ysec+x%2);
						if (reg && reg->zloc == level && !reg->IsGuarded()) {
							mons += reg->CountWMons();
							/*
							 * Make sure there is at least one monster type
							 * enabled for this region
							 */
							int avail = 0;
							int mon = TerrainDefs[reg->type].smallmon;
							if (!((mon == -1) ||
								 (ItemDefs[mon].flags & ItemType::DISABLED)))
								avail = 1;
							mon = TerrainDefs[reg->type].bigmon;
							if (!((mon == -1) ||
								 (ItemDefs[mon].flags & ItemType::DISABLED)))
								avail = 1;
							mon = TerrainDefs[reg->type].humanoid;
							if (!((mon == -1) ||
								 (ItemDefs[mon].flags & ItemType::DISABLED)))
								avail = 1;

							if (avail) {
								wanted += TerrainDefs[reg->type].wmonfreq;
							}
						}
					}
				}

				wanted /= 10;
				wanted -= mons;
				wanted = (wanted*rate + getrandom(100))/100;

				// printf("\n\n WANTED WMON at (xsec: %d, ysec: %d) : %d \n\n", xsec, ysec, wanted);

				if (wanted > 0) {
					// TODO: instead of loop guard need to check how many available regions
					// are there and random them
					int loop_guard = 1000;
					for (int i=0; i < wanted;) {
						int x = getrandom(8);
						int y = getrandom(16);
						if (y%2 == 1) {
							if (y > 0) {
								y -= 1;
							} else {
								y = 0;
							}
						}

						ARegion *reg = pArr->GetRegion(x + xsec, y + ysec + x%2);

						if (reg && reg->zloc == level && !reg->IsGuarded() && MakeWMon(reg)) {
							i++;
						}

						// In worst case scenario it will randomly pick same not matching regions
						// with potential of infinitie loop (ie dodgy RNG)
						loop_guard--;
						if (loop_guard == 0) break;
					}
				}
			}
		}
	}
}

void Game::GrowLMons(int rate)
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		//
		// Don't make lmons in guarded regions
		//
		if (r->IsGuarded()) continue;
		
		forlist(&r->objects) {
			Object *obj = (Object *) elem;
			if (obj->units.Num()) continue;
			int montype = ObjectDefs[obj->type].monster;
			int grow=!(ObjectDefs[obj->type].flags&ObjectType::NOMONSTERGROWTH);
			if ((montype != -1) && grow) {
				if (getrandom(100) < rate) {
					MakeLMon(obj);
				}
			}
		}
	}
}

int Game::MakeWMon(ARegion *pReg)
{
	if (!Globals->WANDERING_MONSTERS_EXIST) return 0;

	if (TerrainDefs[pReg->type].wmonfreq == 0) return 0;

	int montype = TerrainDefs[pReg->type].smallmon;
	if (getrandom(2) && (TerrainDefs[pReg->type].humanoid != -1))
		montype = TerrainDefs[pReg->type].humanoid;
	if (TerrainDefs[pReg->type].bigmon != -1 && !getrandom(8)) {
		montype = TerrainDefs[pReg->type].bigmon;
	}
	if ((montype == -1) || (ItemDefs[montype].flags & ItemType::DISABLED))
		return 0;

	MonType *mp = FindMonster(ItemDefs[montype].abr,
			(ItemDefs[montype].type & IT_ILLUSION));
	Faction *monfac = GetFaction(&factions, monfaction);
	Unit *u = GetNewUnit(monfac, 0);
	u->MakeWMon(mp->name, montype, (mp->number+getrandom(mp->number)+1)/2);
	u->MoveUnit(pReg->GetDummy());
	return(1);
}

void Game::MakeLMon(Object *pObj)
{
	if (!Globals->LAIR_MONSTERS_EXIST) return;
	if (ObjectDefs[pObj->type].flags & ObjectType::NOMONSTERGROWTH) return;

	int montype = ObjectDefs[pObj->type].monster;

	if (montype == I_TRENT)
		montype = TerrainDefs[pObj->region->type].bigmon;

	if (montype == I_CENTAUR)
		montype = TerrainDefs[pObj->region->type].humanoid;

	if ((montype == -1) || (ItemDefs[montype].flags & ItemType::DISABLED))
		return;

	MonType *mp = FindMonster(ItemDefs[montype].abr,
			(ItemDefs[montype].type & IT_ILLUSION));
	Faction *monfac = GetFaction(&factions, monfaction);
	Unit *u = GetNewUnit(monfac, 0);
	switch(montype) {
		case I_IMP:
			u->MakeWMon("Demons", I_IMP, getrandom(mp->number + 1));

			mp = FindMonster(ItemDefs[I_DEMON].abr,
					(ItemDefs[I_DEMON].type & IT_ILLUSION));
			u->items.SetNum(I_DEMON, getrandom(mp->number + 1));

			mp = FindMonster(ItemDefs[I_BALROG].abr,
					(ItemDefs[I_BALROG].type & IT_ILLUSION));
			u->items.SetNum(I_BALROG, getrandom(mp->number + 1));
			break;
		case I_SKELETON:
			u->MakeWMon("Undead", I_SKELETON, getrandom(mp->number + 1));

			mp = FindMonster(ItemDefs[I_UNDEAD].abr,
					(ItemDefs[I_UNDEAD].type & IT_ILLUSION));
			u->items.SetNum(I_UNDEAD, getrandom(mp->number + 1));

			mp = FindMonster(ItemDefs[I_LICH].abr,
					(ItemDefs[I_LICH].type & IT_ILLUSION));
			u->items.SetNum(I_LICH, getrandom(mp->number + 1));
			break;
		case I_MAGICIANS:
			u->MakeWMon("Evil Mages", I_MAGICIANS,
					(mp->number + getrandom(mp->number) + 1) / 2);

			mp = FindMonster(ItemDefs[I_SORCERERS].abr,
					(ItemDefs[I_SORCERERS].type & IT_ILLUSION));
			u->items.SetNum(I_SORCERERS,
					getrandom(mp->number + 1));
			u->SetFlag(FLAG_BEHIND, 1);
			u->MoveUnit(pObj);

			u = GetNewUnit(monfac, 0);

			mp = FindMonster(ItemDefs[I_WARRIORS].abr,
					(ItemDefs[I_WARRIORS].type & IT_ILLUSION));
			u->MakeWMon(mp->name, I_WARRIORS,
					(mp->number + getrandom(mp->number) + 1) / 2);

			break;
		case I_DARKMAGE:
			u->MakeWMon("Dark Mages", I_DARKMAGE, (getrandom(mp->number) + 1));

			mp = FindMonster(ItemDefs[I_MAGICIANS].abr,
					(ItemDefs[I_MAGICIANS].type & IT_ILLUSION));
			u->items.SetNum(I_MAGICIANS,
					(mp->number + getrandom(mp->number) + 1) / 2);

			mp = FindMonster(ItemDefs[I_SORCERERS].abr,
					(ItemDefs[I_SORCERERS].type & IT_ILLUSION));
			u->items.SetNum(I_SORCERERS, getrandom(mp->number + 1));

			mp = FindMonster(ItemDefs[I_DARKMAGE].abr,
					(ItemDefs[I_DARKMAGE].type & IT_ILLUSION));
			u->items.SetNum(I_DARKMAGE, getrandom(mp->number + 1));
			u->SetFlag(FLAG_BEHIND, 1);
			u->MoveUnit(pObj);

			u = GetNewUnit(monfac, 0);

			mp = FindMonster(ItemDefs[I_DROW].abr,
					(ItemDefs[I_DROW].type & IT_ILLUSION));
			u->MakeWMon(mp->name, I_DROW,
					(mp->number + getrandom(mp->number) + 1) / 2);

			break;
		case I_ILLYRTHID:
			u->MakeWMon(mp->name, I_ILLYRTHID,
					(mp->number + getrandom(mp->number) + 1) / 2);
			u->SetFlag(FLAG_BEHIND, 1);
			u->MoveUnit(pObj);

			u = GetNewUnit(monfac, 0);

			mp = FindMonster(ItemDefs[I_SKELETON].abr,
					(ItemDefs[I_SKELETON].type & IT_ILLUSION));
			u->MakeWMon("Undead", I_SKELETON, getrandom(mp->number + 1));

			mp = FindMonster(ItemDefs[I_UNDEAD].abr,
					(ItemDefs[I_UNDEAD].type & IT_ILLUSION));
			u->items.SetNum(I_UNDEAD, getrandom(mp->number + 1));
			break;
		case I_STORMGIANT:
			if (getrandom(3) < 1) {
				montype = I_CLOUDGIANT;
				mp = FindMonster(ItemDefs[montype].abr,
						(ItemDefs[montype].type & IT_ILLUSION));
			}
			u->MakeWMon(mp->name, montype,
					(mp->number + getrandom(mp->number) + 1) / 2);
			break;
		default:
			u->MakeWMon(mp->name, montype,
					(mp->number + getrandom(mp->number) + 1) / 2);
			break;
	}
	u->MoveUnit(pObj);
}

Unit *Game::MakeManUnit(Faction *fac, int mantype, int num, int level, int weaponlevel, int armor, int behind)
{
	Unit *u = GetNewUnit(fac);
	ManType *men = FindRace(ItemDefs[mantype].abr);
	int *fitting;

	// Check skills:
	int scomb = men->defaultlevel;
	int sxbow = men->defaultlevel;
	int slbow = men->defaultlevel;
	for (unsigned int i=0;
		i<(sizeof(men->skills)/sizeof(men->skills[0]));
			i++) {
				if (men->skills[i] == NULL) continue;
				if (FindSkill(men->skills[i]) == FindSkill("COMB"))
					scomb = men->speciallevel;
				if (FindSkill(men->skills[i]) == FindSkill("XBOW"))
					sxbow = men->speciallevel;
				if (FindSkill(men->skills[i]) == FindSkill("LBOW"))
					slbow = men->speciallevel;
	}
	int combat = scomb;
	AString *s = new AString("COMB");
	int sk = LookupSkill(s);
	if (behind) {
		if (slbow >= sxbow) {
			*s = AString("LBOW");
			sk = LookupSkill(s);
			combat = slbow;
		} else {
			*s = AString("XBOW");
			sk = LookupSkill(s);
			combat = sxbow;
		}
	}
	if (combat < level) weaponlevel += level - combat;
	int weapon = -1;
	int witem = -1;
	fitting = new int[NUMWEAPONS];
	while (weapon == -1) {
		int n = 0;
		for (int i=0; i<NUMWEAPONS; i++) {
			fitting[i] = 0;
			AString *it = new AString(WeaponDefs[i].abbr);
			if (ItemDefs[LookupItem(it)].flags & ItemType::DISABLED) continue;
			// disregard picks!
			AString *ps = new AString("PICK");
			if (LookupItem(it) == LookupItem(ps)) continue;
			
			// Sort out the more exotic weapons!
			int producelevel = ItemDefs[LookupItem(it)].pLevel;
			if (ItemDefs[LookupItem(it)].pSkill != FindSkill("WEAP")->abbr) continue;

			AString *s1 = new AString(WeaponDefs[i].baseSkill);
			AString *s2 = new AString(WeaponDefs[i].orSkill);			
			if ((WeaponDefs[i].flags & WeaponType::RANGED)
				&& (!behind)) continue;
			int attack = WeaponDefs[i].attackBonus;
			if (attack < (producelevel-1)) attack = producelevel-1;
			if ((LookupSkill(s1) == sk)
				|| (LookupSkill(s2) == sk)) {
				if ((behind) && (attack + combat <= weaponlevel)) {
					fitting[i] = 1;
					if (WeaponDefs[i].attackBonus == weaponlevel) fitting[i] = 5;
					n += fitting[i];
				} else if ((!behind) && (attack == weaponlevel)) {
					fitting[i] = 1;
					//if (WeaponDefs[i].attackBonus == weaponlevel) fitting[i] = 5;
					n += fitting[i];
				} else continue;
			} else {
				// make Javelins possible
				AString *cs = new AString("COMB");
				if ((behind) && (scomb > combat)) {
					if ((WeaponDefs[i].flags & WeaponType::RANGED)
						&& ((LookupSkill(s1) == LookupSkill(cs))
							|| (LookupSkill(s2) == LookupSkill(cs)))) {
							fitting[i] = 1;
							n++;
					}
				}
			}
		}
		
		if (n < 1) {
			weaponlevel++;
			continue;
		} else {
			int secondtry = -1;
			while(secondtry <= 0) {
				weapon = -1;
				int w = getrandom(n);
				/*
				Awrite(AString("Roll: ") + w);
				*/
				n = -1;
				for (int i=0; i<NUMWEAPONS; i++) {
					if (fitting[i]) {
						n += fitting[i];
						/*
						Awrite(WeaponDefs[i].abbr);
						*/
						if ((n >= w) && (weapon == -1))
							weapon = i;
					}
				}
				if (weapon >= 0) {
					AString *ws = new AString(WeaponDefs[weapon].abbr);
					witem = LookupItem(ws);
					secondtry++;
					if (men->CanUse(witem)) break;
				}
			}
		}
	}
	delete[] fitting;
	// Check again which skills the weapon uses
	AString *ws1 = new AString(WeaponDefs[weapon].baseSkill);
	AString *ws2 = new AString(WeaponDefs[weapon].orSkill);
	if ((LookupSkill(ws1) != sk) && (LookupSkill(ws2) != sk))
		sk = LookupSkill(ws1);
	int maxskill = men->defaultlevel;
	int special = 0;
	for (unsigned int i=0;
		i<(sizeof(men->skills)/sizeof(men->skills[0]));
		i++) {
		if (FindSkill(men->skills[i]) == FindSkill(SkillDefs[sk].abbr)) {
			special = 1;
		}		
	}
	if (special) maxskill = men->speciallevel;
	if (level > maxskill) level = maxskill;
	u->SetMen(mantype, num);
	/*
	Awrite(AString("Unit (") + u->num + ") -> chose " + ItemDefs[witem].name);
	*/
	u->items.SetNum(witem, num);
	u->SetSkill(sk, level);
	if (behind) u->SetFlag(FLAG_BEHIND,1);
	if (armor) {
		int ar = I_PLATEARMOR;
		if (!men->CanUse(ar)) ar = I_CHAINARMOR;
		if (!men->CanUse(ar)) ar = I_LEATHERARMOR;
		u->items.SetNum(ar, num);
	}
	return u;
}

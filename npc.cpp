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
	if(!Globals->CITY_MONSTERS_EXIST) return;

	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		if (r->type == R_NEXUS || r->IsStartingCity() || r->town) {
			CreateCityMon(r, 100, 1);
		}
	}
}

void Game::CreateWMons()
{
	if(!Globals->WANDERING_MONSTERS_EXIST) return;

	GrowWMons(50);
}

void Game::CreateLMons()
{
	if(!Globals->LAIR_MONSTERS_EXIST) return;

	GrowLMons(50);
}

void Game::GrowWMons(int rate)
{
	//
	// Now, go through each 8x8 block of the map, and make monsters if
	// needed.
	//
	int level;
	for(level = 0; level < regions.numLevels; level++) {
		ARegionArray *pArr = regions.pRegionArrays[level];
		int xsec;
		for (xsec=0; xsec< pArr->x / 8; xsec++) {
			for (int ysec=0; ysec< pArr->y / 16; ysec++) {
				/* OK, we have a sector. Count mons, and wanted */
				int mons=0;
				int wanted=0;
				for (int x=0; x<8; x++) {
					for (int y=0; y<16; y+=2) {
						ARegion *reg = pArr->GetRegion(x+xsec*8, y+ysec*16+x%2);
						if (reg && !reg->IsGuarded()) {
							mons += reg->CountWMons();
							/*
							 * Make sure there is at least one monster type
							 * enabled for this region
							 */
							int avail = 0;
							int mon = TerrainDefs[reg->type].smallmon;
							if(!((mon == -1) ||
								 (ItemDefs[mon].flags & ItemType::DISABLED)))
								avail = 1;
							mon = TerrainDefs[reg->type].bigmon;
							if(!((mon == -1) ||
								 (ItemDefs[mon].flags & ItemType::DISABLED)))
								avail = 1;
							mon = TerrainDefs[reg->type].humanoid;
							if(!((mon == -1) ||
								 (ItemDefs[mon].flags & ItemType::DISABLED)))
								avail = 1;

							if(avail)
								wanted += TerrainDefs[reg->type].wmonfreq;
						}
					}
				}

				wanted /= 10;
				wanted -= mons;
				wanted = (wanted*rate + getrandom(100))/100;
				if (wanted > 0) {
					for (int i=0; i< wanted;) {
						int m=getrandom(8);
						int n=getrandom(8)*2+m%2;
						ARegion *reg = pArr->GetRegion(m+xsec*8, n+ysec*16);
						if (reg && !reg->IsGuarded() && MakeWMon(reg)) {
							i++;
						}
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
	if(!Globals->WANDERING_MONSTERS_EXIST) return 0;

	if (TerrainDefs[pReg->type].wmonfreq == 0) return 0;

	int montype = TerrainDefs[pReg->type].smallmon;
	if (getrandom(2) && (TerrainDefs[pReg->type].humanoid != -1))
		montype = TerrainDefs[pReg->type].humanoid;
	if (TerrainDefs[pReg->type].bigmon != -1 && !getrandom(8)) {
		montype = TerrainDefs[pReg->type].bigmon;
	}
	if((montype == -1) || (ItemDefs[montype].flags & ItemType::DISABLED))
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
	if(!Globals->LAIR_MONSTERS_EXIST) return;
	if(ObjectDefs[pObj->type].flags & ObjectType::NOMONSTERGROWTH) return;

	int montype = ObjectDefs[pObj->type].monster;

	if (montype == I_TRENT)
		montype = TerrainDefs[pObj->region->type].bigmon;

	if (montype == I_CENTAUR)
		montype = TerrainDefs[pObj->region->type].humanoid;

	if((montype == -1) || (ItemDefs[montype].flags & ItemType::DISABLED))
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

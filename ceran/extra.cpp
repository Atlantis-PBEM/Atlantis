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
//
// This file contains extra game-specific functions
//
#include "game.h"
#include "gamedata.h"

int Game::SetupFaction( Faction *pFac )
{
	pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 50;

	if(pFac->noStartLeader)
		return 1;

	ARegion *reg = NULL;
	if(pFac->pStartLoc) {
		reg = pFac->pStartLoc;
	} else if(!Globals->MULTI_HEX_NEXUS) {
		reg = (ARegion *)(regions.First());
	} else {
		ARegionArray *pArr = regions.GetRegionArray(ARegionArray::LEVEL_NEXUS);
		while(!reg) {
			reg = pArr->GetRegion(getrandom(pArr->x), getrandom(pArr->y));
		}
	}

	//
	// Set up first unit.
	//
	Unit *temp2 = GetNewUnit( pFac );
	temp2->SetMen( I_LEADERS, 1 );
	temp2->reveal = REVEAL_FACTION;

	temp2->type = U_MAGE;
	temp2->Study(S_PATTERN, 30);
	temp2->Study(S_SPIRIT, 30);
	temp2->Study(S_GATE_LORE, 30);

	if (TurnNumber() >= 12) {
		temp2->Study(S_FORCE, 30);
		temp2->Study(S_FIRE, 30);
		temp2->Study(S_COMBAT, 30);
	}
	if(TurnNumber() >= 24) {
		temp2->Study(S_PATTERN, 60);
		temp2->Study(S_SPIRIT, 60);
		temp2->Study(S_FORCE, 90);
		temp2->Study(S_EARTH_LORE, 30);
		temp2->Study(S_STEALTH, 30);
		temp2->Study(S_OBSERVATION, 30);
	}
	if(TurnNumber() >= 36) {
		temp2->Study(S_TACTICS, 90);
		temp2->Study(S_COMBAT, 60);
	}

	if (Globals->UPKEEP_MINIMUM_FOOD > 0)
	{
		if (!(ItemDefs[I_FOOD].flags & ItemType::DISABLED))
			temp2->items.SetNum(I_FOOD, 6);
		else if (!(ItemDefs[I_FISH].flags & ItemType::DISABLED))
			temp2->items.SetNum(I_FISH, 6);
		else if (!(ItemDefs[I_LIVESTOCK].flags & ItemType::DISABLED))
			temp2->items.SetNum(I_LIVESTOCK, 6);
		else if (!(ItemDefs[I_GRAIN].flags & ItemType::DISABLED))
			temp2->items.SetNum(I_GRAIN, 2);
		temp2->items.SetNum(I_SILVER, 10);
	}

	temp2->MoveUnit( reg->GetDummy() );

	return( 1 );
}

Faction *Game::CheckVictory()
{
	ARegion *reg;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		forlist(&r->objects) {
			Object *obj = (Object *)elem;
			if(obj->type != O_BKEEP) continue;
			if(obj->units.Num()) return NULL;
			reg = r;
			break;
		}
	}
	{
		// Now see find the first faction guarding the region
		forlist(&reg->objects) {
			Object *o = reg->GetDummy();
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				if(u->guard == GUARD_GUARD) return u->faction;
			}
		}
	}
	return NULL;
}

void Game::ModifyTablesPerRuleset(void)
{
	if(Globals->APPRENTICES_EXIST)
		EnableSkill(S_MANIPULATE);

	if((Globals->UNDERDEEP_LEVELS > 0) || (Globals->UNDERWORLD_LEVELS > 1)) {
		EnableItem(I_MUSHROOM);
		EnableItem(I_HEALPOTION);
		EnableItem(I_ROUGHGEM);
		EnableItem(I_GEMS);
		EnableItem(I_MWOLF);
		EnableItem(I_MSPIDER);
		EnableItem(I_MOLE);
		EnableSkill(S_GEMCUTTING);
		EnableSkill(S_MONSTERTRAINING);
	}

	if(!Globals->GATES_EXIST)
		DisableSkill(S_GATE_LORE);

	if (Globals->FULL_TRUESEEING_BONUS) {
		ModifyAttribMod("observation", 1, AttribModItem::SKILL,
				"TRUE", AttribModItem::UNIT_LEVEL, 1);
	}
	if (Globals->IMPROVED_AMTS) {
		ModifyAttribMod("observation", 2, AttribModItem::ITEM,
				"AMTS", AttribModItem::CONSTANT, 3);
	}
	if (Globals->FULL_INVIS_ON_SELF) {
		ModifyAttribMod("stealth", 3, AttribModItem::SKILL,
				"INVI", AttribModItem::UNIT_LEVEL, 1);
	}

	if(Globals->NEXUS_IS_CITY) {
		ClearTerrainRaces(R_NEXUS);
		ModifyTerrainRace(R_NEXUS, 0, I_HIGHELF);

		ClearTerrainItems(R_NEXUS);
		ModifyTerrainItems(R_NEXUS, 0, I_HERBS, 100, 20);
		ModifyTerrainItems(R_NEXUS, 1, I_ROOTSTONE, 100, 10);
		ModifyTerrainItems(R_NEXUS, 2, I_MITHRIL, 100, 10);
		ModifyTerrainItems(R_NEXUS, 3, I_YEW, 100, 10);
		ModifyTerrainItems(R_NEXUS, 4, I_IRONWOOD, 100, 10);
		ModifyTerrainEconomy(R_NEXUS, 1000, 15, 50, 2);
	}

	EnableItem(I_PICK);
	EnableItem(I_SPEAR);
	EnableItem(I_AXE);
	EnableItem(I_HAMMER);
	EnableItem(I_MCROSSBOW);
	EnableItem(I_MWAGON);
	EnableItem(I_GLIDER);
	EnableItem(I_NET);
	EnableItem(I_LASSO);
	EnableItem(I_BAG);
	EnableItem(I_SPINNING);
	EnableItem(I_LEATHERARMOR);
	EnableItem(I_CLOTHARMOR);
	EnableItem(I_BOOTS);

	EnableItem(I_PIRATES);
	EnableItem(I_KRAKEN);
	EnableItem(I_MERFOLK);
	EnableItem(I_ELEMENTAL);

	EnableItem(I_FAIRY);
	EnableItem(I_LIZARDMAN);
	EnableItem(I_URUK);
	EnableItem(I_GOBLINMAN);
	EnableItem(I_HOBBIT);
	EnableItem(I_GNOLL);
	EnableItem(I_DROWMAN);
	EnableItem(I_MERC);
	EnableItem(I_TITAN);
	EnableItem(I_AMAZON);
	EnableItem(I_OGREMAN);
	EnableItem(I_GNOME);
	EnableItem(I_HIGHLANDER);
	EnableItem(I_MINOTAUR);
	EnableItem(I_GREYELF);

	EnableItem(I_LANCE);
	EnableItem(I_MUSHROOM);

	EnableItem(I_RRAT);
	EnableItem(I_NOOGLE);
	EnableItem(I_MUTANT);

	EnableItem(I_BAXE);
	EnableItem(I_MBAXE);
	EnableItem(I_ADMANTIUM);
	EnableItem(I_ADSWORD);
	EnableItem(I_ADBAXE);
	EnableItem(I_IMARM);
	EnableItem(I_ADRING);
	EnableItem(I_ADPLATE);
	EnableItem(I_CAMEL);

	EnableItem(I_DROW);
	EnableItem(I_HYDRA);
	EnableItem(I_STORMGIANT);
	EnableItem(I_CLOUDGIANT);
	EnableItem(I_ILLYRTHID);
	EnableItem(I_SORCERERS);
	EnableItem(I_MAGICIANS);
	EnableItem(I_DARKMAGE);
	EnableItem(I_WARRIORS);
	EnableItem(I_ICEDRAGON);

	EnableItem(I_HEALPOTION);
	EnableItem(I_ROUGHGEM);
	EnableItem(I_GEMS);
	EnableItem(I_JAVELIN);
	EnableItem(I_PIKE);
	EnableItem(I_MWOLF);
	EnableItem(I_MSPIDER);
	EnableItem(I_MOLE);
	EnableItem(I_BPLATE);
	EnableItem(I_FSWORD);
	EnableItem(I_MCHAIN);
	EnableItem(I_QSTAFF);
	EnableItem(I_SABRE);
	EnableItem(I_MACE);
	EnableItem(I_MSTAR);
	EnableItem(I_DAGGER);
	EnableItem(I_PDAGGER);
	EnableItem(I_BHAMMER);
	EnableItem(I_SHORTBOW);
	EnableItem(I_BOW);
	EnableItem(I_HEAVYCROSSBOW);
	EnableItem(I_HARP);

	EnableSkill(S_WEAPONCRAFT);
	EnableSkill(S_ARMORCRAFT);
	EnableSkill(S_GEMCUTTING);
	EnableSkill(S_CAMELTRAINING);
	EnableSkill(S_MONSTERTRAINING);

	EnableObject(O_ISLE);
	EnableObject(O_DERELICT);
	EnableObject(O_OCAVE);
	EnableObject(O_WHIRL);

	EnableObject(O_ROADN);
	EnableObject(O_ROADNW);
	EnableObject(O_ROADNE);
	EnableObject(O_ROADSW);
	EnableObject(O_ROADSE);
	EnableObject(O_ROADS);
	EnableObject(O_TEMPLE);
	EnableObject(O_MQUARRY);
	EnableObject(O_AMINE);
	EnableObject(O_PRESERVE);
	EnableObject(O_SACGROVE);

	EnableObject(O_DCLIFFS);
	EnableObject(O_MTOWER);
	EnableObject(O_WGALLEON);
	EnableObject(O_HUT);
	EnableObject(O_STOCKADE);
	EnableObject(O_CPALACE);
	EnableObject(O_NGUILD);
	EnableObject(O_AGUILD);
	EnableObject(O_ATEMPLE);
	EnableObject(O_HTOWER);
	EnableObject(O_HPTOWER);

	EnableObject(O_MAGETOWER);
	EnableObject(O_DARKTOWER);
	EnableObject(O_GIANTCASTLE);
	EnableObject(O_ILAIR);
	EnableObject(O_ICECAVE);
	EnableObject(O_BOG);

	EnableObject(O_TRAPPINGHUT);
	EnableObject(O_STABLE);
	EnableObject(O_MSTABLE);
	EnableObject(O_TRAPPINGLODGE);
	EnableObject(O_FAERIERING);
	EnableObject(O_ALCHEMISTLAB);
	EnableObject(O_OASIS);
	EnableObject(O_GEMAPPRAISER);

	ModifyTerrainRace(R_TUNDRA, 2, I_GNOLL);
	ModifyTerrainCoastRace(R_DESERT, 0, I_BARBARIAN);
	ModifyTerrainRace(R_FOREST, 2, I_HIGHELF);
	ModifyTerrainCoastRace(R_FOREST, 1, I_WOODELF);
	ClearTerrainItems(R_FOREST);
	ModifyTerrainItems(R_FOREST, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_FOREST, 1, I_FUR, 100, 10);
	ModifyTerrainItems(R_FOREST, 2, I_HERBS, 100, 10);
	ModifyTerrainItems(R_FOREST, 3, I_IRONWOOD, 25, 5);
	ModifyTerrainItems(R_FOREST, 4, I_YEW, 25, 5);
	ModifyTerrainItems(R_FOREST, 5, I_MWOLF, 5, 5);
	ClearTerrainItems(R_CERAN_FOREST1);
	ModifyTerrainItems(R_CERAN_FOREST1, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_CERAN_FOREST1, 1, I_FUR, 100, 10);
	ModifyTerrainItems(R_CERAN_FOREST1, 2, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_FOREST1, 3, I_IRONWOOD, 25, 5);
	ModifyTerrainItems(R_CERAN_FOREST1, 4, I_YEW, 25, 5);
	ModifyTerrainItems(R_CERAN_FOREST1, 5, I_MWOLF, 5, 5);
	ModifyTerrainItems(R_CERAN_FOREST1, 6, I_STONE, 10, 4);
	ClearTerrainItems(R_CERAN_FOREST2);
	ModifyTerrainItems(R_CERAN_FOREST2, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_CERAN_FOREST2, 1, I_FUR, 100, 10);
	ModifyTerrainItems(R_CERAN_FOREST2, 2, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_FOREST2, 3, I_IRONWOOD, 25, 5);
	ModifyTerrainItems(R_CERAN_FOREST2, 4, I_YEW, 25, 5);
	ModifyTerrainItems(R_CERAN_FOREST2, 5, I_MWOLF, 5, 5);
	ModifyTerrainItems(R_CERAN_FOREST2, 6, I_WHORSE, 10, 4);
	ClearTerrainItems(R_CERAN_FOREST3);
	ModifyTerrainItems(R_CERAN_FOREST3, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_CERAN_FOREST3, 1, I_FUR, 100, 10);
	ModifyTerrainItems(R_CERAN_FOREST3, 2, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_FOREST3, 3, I_IRONWOOD, 25, 5);
	ModifyTerrainItems(R_CERAN_FOREST3, 4, I_YEW, 25, 5);
	ModifyTerrainItems(R_CERAN_FOREST3, 5, I_MWOLF, 5, 5);
	ModifyTerrainItems(R_CERAN_FOREST3, 6, I_MUSHROOM, 10, 10);
	ClearTerrainItems(R_CERAN_MYSTFOREST);
	ModifyTerrainItems(R_CERAN_MYSTFOREST, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_CERAN_MYSTFOREST, 1, I_FUR, 50, 15);
	ModifyTerrainItems(R_CERAN_MYSTFOREST, 2, I_HERBS, 50, 20);
	ModifyTerrainItems(R_CERAN_MYSTFOREST, 3, I_IRONWOOD, 50, 10);
	ModifyTerrainItems(R_CERAN_MYSTFOREST, 4, I_YEW, 50, 10);
	ModifyTerrainItems(R_CERAN_MYSTFOREST, 5, I_MWOLF, 5, 5);
	ModifyTerrainItems(R_CERAN_MYSTFOREST, 6, I_MSPIDER, 5, 5);
	ClearTerrainItems(R_CERAN_MYSTFOREST1);
	ModifyTerrainItems(R_CERAN_MYSTFOREST1, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_CERAN_MYSTFOREST1, 1, I_FUR, 50, 15);
	ModifyTerrainItems(R_CERAN_MYSTFOREST1, 2, I_HERBS, 50, 20);
	ModifyTerrainItems(R_CERAN_MYSTFOREST1, 3, I_IRONWOOD, 50, 10);
	ModifyTerrainItems(R_CERAN_MYSTFOREST1, 4, I_YEW, 50, 10);
	ModifyTerrainItems(R_CERAN_MYSTFOREST1, 5, I_MWOLF, 5, 5);
	ModifyTerrainItems(R_CERAN_MYSTFOREST1, 6, I_MSPIDER, 5, 5);
	ClearTerrainItems(R_CERAN_MYSTFOREST2);
	ModifyTerrainItems(R_CERAN_MYSTFOREST2, 0, I_WOOD, 100, 20);
	ModifyTerrainItems(R_CERAN_MYSTFOREST2, 1, I_FUR, 50, 15);
	ModifyTerrainItems(R_CERAN_MYSTFOREST2, 2, I_HERBS, 50, 20);
	ModifyTerrainItems(R_CERAN_MYSTFOREST2, 3, I_IRONWOOD, 50, 10);
	ModifyTerrainItems(R_CERAN_MYSTFOREST2, 4, I_YEW, 50, 10);
	ModifyTerrainItems(R_CERAN_MYSTFOREST2, 5, I_MWOLF, 5, 5);
	ModifyTerrainItems(R_CERAN_MYSTFOREST2, 6, I_MSPIDER, 5, 5);
	ClearTerrainItems(R_MOUNTAIN);
	ModifyTerrainItems(R_MOUNTAIN, 0, I_IRON, 100, 20);
	ModifyTerrainItems(R_MOUNTAIN, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_MOUNTAIN, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_MOUNTAIN, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_MOUNTAIN, 4, I_ADMANTIUM, 5, 5);
	ModifyTerrainItems(R_MOUNTAIN, 5, I_MWOLF, 5, 5);
	ClearTerrainItems(R_CERAN_MOUNTAIN1);
	ModifyTerrainItems(R_CERAN_MOUNTAIN1, 0, I_IRON, 100, 20);
	ModifyTerrainItems(R_CERAN_MOUNTAIN1, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_CERAN_MOUNTAIN1, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN1, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN1, 4, I_ADMANTIUM, 5, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN1, 5, I_MWOLF, 5, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN1, 6, I_HERBS, 25, 10);
	ClearTerrainItems(R_CERAN_MOUNTAIN2);
	ModifyTerrainItems(R_CERAN_MOUNTAIN2, 0, I_IRON, 100, 20);
	ModifyTerrainItems(R_CERAN_MOUNTAIN2, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_CERAN_MOUNTAIN2, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN2, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN2, 4, I_ADMANTIUM, 5, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN2, 5, I_MWOLF, 10, 10);
	ModifyTerrainItems(R_CERAN_MOUNTAIN2, 6, I_WOOD, 10, 4);
	ClearTerrainItems(R_CERAN_MOUNTAIN3);
	ModifyTerrainItems(R_CERAN_MOUNTAIN3, 0, I_IRON, 100, 20);
	ModifyTerrainItems(R_CERAN_MOUNTAIN3, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_CERAN_MOUNTAIN3, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN3, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN3, 4, I_ADMANTIUM, 5, 5);
	ModifyTerrainItems(R_CERAN_MOUNTAIN3, 5, I_MWOLF, 10, 10);
	ClearTerrainItems(R_CERAN_HILL);
	ModifyTerrainItems(R_CERAN_HILL, 0, I_IRON, 80, 15);
	ModifyTerrainItems(R_CERAN_HILL, 1, I_STONE, 100, 30);
	ModifyTerrainItems(R_CERAN_HILL, 2, I_MITHRIL, 10, 5);
	ModifyTerrainItems(R_CERAN_HILL, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CERAN_HILL, 4, I_HERBS, 10, 10);
	ModifyTerrainItems(R_CERAN_HILL, 5, I_MWOLF, 5, 20);
	ClearTerrainItems(R_CERAN_HILL1);
	ModifyTerrainItems(R_CERAN_HILL1, 0, I_IRON, 80, 15);
	ModifyTerrainItems(R_CERAN_HILL1, 1, I_STONE, 100, 30);
	ModifyTerrainItems(R_CERAN_HILL1, 2, I_MITHRIL, 10, 5);
	ModifyTerrainItems(R_CERAN_HILL1, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CERAN_HILL1, 4, I_HERBS, 10, 6);
	ModifyTerrainItems(R_CERAN_HILL1, 5, I_MWOLF, 5, 20);
	ClearTerrainItems(R_CERAN_HILL2);
	ModifyTerrainItems(R_CERAN_HILL2, 0, I_IRON, 80, 15);
	ModifyTerrainItems(R_CERAN_HILL2, 1, I_STONE, 100, 30);
	ModifyTerrainItems(R_CERAN_HILL2, 2, I_MITHRIL, 10, 5);
	ModifyTerrainItems(R_CERAN_HILL2, 3, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CERAN_HILL2, 4, I_HORSE, 10, 5);
	ModifyTerrainItems(R_CERAN_HILL2, 5, I_MWOLF, 5, 20);
	ClearTerrainItems(R_TUNDRA);
	ModifyTerrainItems(R_TUNDRA, 0, I_FUR, 100, 10);
	ModifyTerrainItems(R_TUNDRA, 1, I_HERBS, 100, 10);
	ModifyTerrainItems(R_TUNDRA, 2, I_MWOLF, 10, 15);
	ClearTerrainItems(R_CERAN_TUNDRA1);
	ModifyTerrainItems(R_CERAN_TUNDRA1, 0, I_FUR, 100, 10);
	ModifyTerrainItems(R_CERAN_TUNDRA1, 1, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_TUNDRA1, 2, I_MWOLF, 10, 15);
	ModifyTerrainItems(R_CERAN_TUNDRA1, 3, I_WOOD, 10, 4);
	ClearTerrainItems(R_CERAN_TUNDRA2);
	ModifyTerrainItems(R_CERAN_TUNDRA2, 0, I_FUR, 100, 10);
	ModifyTerrainItems(R_CERAN_TUNDRA2, 1, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_TUNDRA2, 2, I_MWOLF, 10, 15);
	ModifyTerrainItems(R_CERAN_TUNDRA2, 3, I_STONE, 10, 4);
	ClearTerrainItems(R_CERAN_TUNDRA3);
	ModifyTerrainItems(R_CERAN_TUNDRA3, 0, I_FUR, 100, 10);
	ModifyTerrainItems(R_CERAN_TUNDRA3, 1, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_TUNDRA3, 2, I_MWOLF, 10, 15);
	ModifyTerrainItems(R_CERAN_TUNDRA3, 3, I_IRON, 10, 4);
	ClearTerrainItems(R_CERAN_PLAIN1);
	ModifyTerrainItems(R_CERAN_PLAIN1, 0, I_HORSE, 100, 20);
	ModifyTerrainItems(R_CERAN_PLAIN1, 1, I_WHORSE, 25, 5);
	ModifyTerrainItems(R_CERAN_PLAIN1, 2, I_STONE, 10, 4);
	ModifyTerrainItems(R_CERAN_PLAIN1, 3, I_HERBS, 10, 5);
	ClearTerrainItems(R_CERAN_PLAIN2);
	ModifyTerrainItems(R_CERAN_PLAIN2, 0, I_HORSE, 100, 20);
	ModifyTerrainItems(R_CERAN_PLAIN2, 1, I_WHORSE, 25, 5);
	ModifyTerrainItems(R_CERAN_PLAIN2, 2, I_ROOTSTONE, 5, 4);
	ModifyTerrainItems(R_CERAN_PLAIN2, 3, I_WOOD, 10, 4);
	ClearTerrainItems(R_CERAN_PLAIN3);
	ModifyTerrainItems(R_CERAN_PLAIN3, 0, I_HORSE, 100, 20);
	ModifyTerrainItems(R_CERAN_PLAIN3, 1, I_WHORSE, 25, 5);
	ModifyTerrainItems(R_CERAN_PLAIN3, 2, I_IRON, 10, 4);
	ModifyTerrainItems(R_CERAN_PLAIN3, 3, I_MITHRIL, 5, 3);
	ClearTerrainItems(R_CERAN_SWAMP1);
	ModifyTerrainItems(R_CERAN_SWAMP1, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_SWAMP1, 1, I_FLOATER, 25, 5);
	ModifyTerrainItems(R_CERAN_SWAMP1, 2, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_SWAMP1, 3, I_MUSHROOM, 10, 5);
	ModifyTerrainItems(R_CERAN_SWAMP1, 4, I_IRON, 10, 4);
	ClearTerrainItems(R_CERAN_SWAMP2);
	ModifyTerrainItems(R_CERAN_SWAMP2, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_SWAMP2, 1, I_FLOATER, 25, 5);
	ModifyTerrainItems(R_CERAN_SWAMP2, 2, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_SWAMP2, 3, I_MUSHROOM, 10, 5);
	ModifyTerrainItems(R_CERAN_SWAMP2, 4, I_IRONWOOD, 10, 4);
	ClearTerrainItems(R_CERAN_SWAMP3);
	ModifyTerrainItems(R_CERAN_SWAMP3, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_SWAMP3, 1, I_FLOATER, 25, 5);
	ModifyTerrainItems(R_CERAN_SWAMP3, 2, I_HERBS, 100, 10);
	ModifyTerrainItems(R_CERAN_SWAMP3, 3, I_MUSHROOM, 10, 5);
	ModifyTerrainItems(R_CERAN_SWAMP3, 4, I_STONE, 10, 4);
	ClearTerrainItems(R_CERAN_JUNGLE1);
	ModifyTerrainItems(R_CERAN_JUNGLE1, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_JUNGLE1, 1, I_HERBS, 100, 20);
	ModifyTerrainItems(R_CERAN_JUNGLE1, 2, I_MUSHROOM, 25, 5);
	ModifyTerrainItems(R_CERAN_JUNGLE1, 3, I_IRONWOOD, 10, 4);
	ModifyTerrainItems(R_CERAN_JUNGLE1, 4, I_YEW, 10, 4);
	ClearTerrainItems(R_CERAN_JUNGLE2);
	ModifyTerrainItems(R_CERAN_JUNGLE2, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_JUNGLE2, 1, I_HERBS, 100, 20);
	ModifyTerrainItems(R_CERAN_JUNGLE2, 2, I_MUSHROOM, 25, 5);
	ModifyTerrainItems(R_CERAN_JUNGLE2, 3, I_FUR, 10, 20);
	ModifyTerrainItems(R_CERAN_JUNGLE2, 4, I_YEW, 10, 4);
	ClearTerrainItems(R_CERAN_JUNGLE3);
	ModifyTerrainItems(R_CERAN_JUNGLE3, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_JUNGLE3, 1, I_HERBS, 100, 20);
	ModifyTerrainItems(R_CERAN_JUNGLE3, 2, I_MUSHROOM, 25, 5);
	ModifyTerrainItems(R_CERAN_JUNGLE3, 3, I_IRONWOOD, 10, 4);
	ModifyTerrainItems(R_CERAN_JUNGLE3, 4, I_STONE, 10, 4);
	ClearTerrainItems(R_UFOREST);
	ModifyTerrainItems(R_UFOREST, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_UFOREST, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_UFOREST, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_UFOREST, 3, I_MUSHROOM, 20, 10);
	ModifyTerrainItems(R_UFOREST, 4, I_MSPIDER, 5, 10);
	ClearTerrainItems(R_CERAN_UFOREST1);
	ModifyTerrainItems(R_CERAN_UFOREST1, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST1, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST1, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST1, 3, I_MUSHROOM, 20, 10);
	ModifyTerrainItems(R_CERAN_UFOREST1, 4, I_MSPIDER, 5, 10);
	ModifyTerrainItems(R_CERAN_UFOREST1, 5, I_MITHRIL, 5, 10);
	ClearTerrainItems(R_CERAN_UFOREST2);
	ModifyTerrainItems(R_CERAN_UFOREST2, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST2, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST2, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST2, 3, I_MUSHROOM, 20, 10);
	ModifyTerrainItems(R_CERAN_UFOREST2, 4, I_MSPIDER, 5, 10);
	ModifyTerrainItems(R_CERAN_UFOREST2, 5, I_ROOTSTONE, 5, 10);
	ClearTerrainItems(R_CERAN_UFOREST3);
	ModifyTerrainItems(R_CERAN_UFOREST3, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST3, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST3, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_CERAN_UFOREST3, 3, I_IRONWOOD, 10, 10);
	ModifyTerrainItems(R_CERAN_UFOREST3, 4, I_YEW, 10, 10);
	ModifyTerrainItems(R_CERAN_UFOREST3, 5, I_MSPIDER, 5, 10);
	ClearTerrainItems(R_CERAN_CAVERN1);
	ModifyTerrainItems(R_CERAN_CAVERN1, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_CAVERN1, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_CERAN_CAVERN1, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_CERAN_CAVERN1, 3, I_IRONWOOD, 10, 10);
	ModifyTerrainItems(R_CERAN_CAVERN1, 4, I_YEW, 10, 10);
	ModifyTerrainItems(R_CERAN_CAVERN1, 5, I_MSPIDER, 5, 10);
	ClearTerrainItems(R_GROTTO);
	ModifyTerrainItems(R_GROTTO, 0, I_MUSHROOM, 25, 10);
	ModifyTerrainItems(R_GROTTO, 1, I_ROOTSTONE, 25, 10);
	ModifyTerrainItems(R_GROTTO, 2, I_ADMANTIUM, 30, 10);
	ModifyTerrainItems(R_GROTTO, 3, I_MSPIDER, 5, 10);
	ClearTerrainItems(R_CERAN_GROTTO1);
	ModifyTerrainItems(R_CERAN_GROTTO1, 0, I_MUSHROOM, 10, 25);
	ModifyTerrainItems(R_CERAN_GROTTO1, 1, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CERAN_GROTTO1, 2, I_ADMANTIUM, 30, 10);
	ModifyTerrainItems(R_CERAN_GROTTO1, 3, I_MSPIDER, 5, 20);
	ModifyTerrainItems(R_CERAN_GROTTO1, 4, I_ROUGHGEM, 25, 25);
	ModifyTerrainItems(R_CERAN_GROTTO1, 5, I_MITHRIL, 25, 5);
	ClearTerrainItems(R_DFOREST);
	ModifyTerrainItems(R_DFOREST, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_DFOREST, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_DFOREST, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_DFOREST, 3, I_MUSHROOM, 100, 20);
	ModifyTerrainItems(R_DFOREST, 4, I_ROUGHGEM, 10, 10);
	ModifyTerrainItems(R_DFOREST, 5, I_MSPIDER, 10, 20);
	ModifyTerrainItems(R_DFOREST, 6, I_MWOLF, 5, 10);
	ClearTerrainItems(R_CERAN_DFOREST1);
	ModifyTerrainItems(R_CERAN_DFOREST1, 0, I_WOOD, 100, 10);
	ModifyTerrainItems(R_CERAN_DFOREST1, 1, I_STONE, 100, 10);
	ModifyTerrainItems(R_CERAN_DFOREST1, 2, I_IRON, 100, 10);
	ModifyTerrainItems(R_CERAN_DFOREST1, 3, I_MUSHROOM, 100, 20);
	ModifyTerrainItems(R_CERAN_DFOREST1, 4, I_ROUGHGEM, 20, 20);
	ModifyTerrainItems(R_CERAN_DFOREST1, 5, I_MSPIDER, 10, 20);
	ModifyTerrainItems(R_CERAN_DFOREST1, 6, I_MWOLF, 5, 10);
	ClearTerrainItems(R_CHASM);
	ModifyTerrainItems(R_CHASM, 0, I_ROUGHGEM, 10, 20);
	ModifyTerrainItems(R_CHASM, 1, I_STONE, 100, 20);
	ModifyTerrainItems(R_CHASM, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_CHASM, 3, I_MUSHROOM, 10, 10);
	ModifyTerrainItems(R_CHASM, 4, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CHASM, 5, I_MSPIDER, 5, 20);
	ClearTerrainItems(R_CERAN_CHASM1);
	ModifyTerrainItems(R_CERAN_CHASM1, 0, I_ROUGHGEM, 50, 20);
	ModifyTerrainItems(R_CERAN_CHASM1, 1, I_STONE, 100, 20);
	ModifyTerrainItems(R_CERAN_CHASM1, 2, I_MITHRIL, 25, 5);
	ModifyTerrainItems(R_CERAN_CHASM1, 3, I_MUSHROOM, 10, 10);
	ModifyTerrainItems(R_CERAN_CHASM1, 4, I_ROOTSTONE, 25, 5);
	ModifyTerrainItems(R_CERAN_CHASM1, 5, I_MSPIDER, 5, 20);
	ModifyTerrainItems(R_CERAN_CHASM1, 6, I_IRON, 100, 20);

	ModifyObjectFlags(O_BKEEP, ObjectType::NEVERDECAY);
	ModifyObjectFlags(O_DCLIFFS, ObjectType::CANENTER|ObjectType::NEVERDECAY);
	ModifyObjectConstruction(O_DCLIFFS, I_ROOTSTONE, 50, "DRAG", 3);

	// Make GateLore, ConstructGate and PortalLore take twice as long to study.
	ModifySkillFlags(S_GATE_LORE,
			SkillType::MAGIC | SkillType::CAST | SkillType::SLOWSTUDY);
	ModifySkillFlags(S_CONSTRUCT_GATE,
			SkillType::MAGIC | SkillType::CAST | SkillType::SLOWSTUDY);
	ModifySkillFlags(S_PORTAL_LORE,
			SkillType::MAGIC | SkillType::CAST | SkillType::SLOWSTUDY);
	DisableItem(I_TAROTCARDS);
	// new weapontable
	DisableItem(I_SUPERBOW);
	DisableItem(I_DOUBLEBOW);
	ModifyItemType(I_LONGBOW,IT_ADVANCED | IT_WEAPON);
	ModifyItemBasePrice(I_LONGBOW,200);
	ModifyItemBasePrice(I_BOW,120);
	ModifyItemBasePrice(I_SHORTBOW,60);
	ModifyItemBasePrice(I_CROSSBOW,60);
	ModifyItemBasePrice(I_MCROSSBOW,400);
	ModifyItemBasePrice(I_HEAVYCROSSBOW,200);
	ModifyItemProductionSkill(I_LONGBOW,"WEAP",5);
	ModifyItemProductionSkill(I_BOW,"WEAP",3);
	ModifyItemProductionSkill(I_SHORTBOW,"WEAP",1);
	ModifyItemProductionSkill(I_CROSSBOW,"WEAP",1);
	ModifyItemProductionSkill(I_MCROSSBOW,"WEAP",5);
	ModifyItemProductionSkill(I_HEAVYCROSSBOW,"WEAP",4);
	ModifyItemProductionInput(I_LONGBOW,0,I_YEW,1);
	ModifyItemProductionInput(I_BOW,0,I_IRONWOOD,1);
	ModifyItemProductionInput(I_SHORTBOW,0,I_WOOD,1);
	ModifyItemProductionInput(I_CROSSBOW,0,I_WOOD,1);
	ModifyItemProductionInput(I_MCROSSBOW,0,I_YEW,1);
	ModifyItemProductionInput(I_HEAVYCROSSBOW,0,I_IRONWOOD,1);
	ModifyItemMagicSkill(I_FSWORD, NULL, 0);
	ModifyItemMagicInput(I_FSWORD, 0, -1, 0);
	ModifyItemMagicOutput(I_FSWORD, 0);
	ModifyItemProductionSkill(I_FSWORD, "WCRA", 5);
	ModifyItemProductionInput(I_FSWORD, 0, I_MSWORD, 1);
	ModifyItemProductionInput(I_FSWORD, 0, I_MITHRIL, 2);
	ModifyItemProductionOutput(I_FSWORD, 3, 1);
	ModifyWeaponFlags("MXBO",
			WeaponType::NEEDSKILL | WeaponType::RANGED |
			WeaponType::NOATTACKERSKILL);
	ModifyWeaponSkills("MXBO", "XBOW", NULL);
	ModifyWeaponAttack("MXBO",ARMORPIERCING,ATTACK_RANGED,1);
	ModifyWeaponBonuses("MXBO",4,0,0);
	ModifyWeaponFlags("LBOW",
			WeaponType::NEEDSKILL | WeaponType::RANGED |
			WeaponType::NOATTACKERSKILL);
	ModifyWeaponSkills("LBOW", "LBOW", NULL);
	ModifyWeaponAttack("LBOW",ARMORPIERCING,ATTACK_RANGED,
			WeaponType::NUM_ATTACKS_SKILL);
	ModifyWeaponBonuses("LBOW",0,0,0);
	ModifyWeaponAttack("BOW",PIERCING,ATTACK_RANGED,1);
	ModifyWeaponBonuses("BOW",0,0,0);
	ModifyWeaponAttack("SHBO",PIERCING,ATTACK_RANGED,1);
	ModifyWeaponBonuses("SHBO",-2,0,0);
	ModifyWeaponBonuses("SPEA",1,1,2);
	// new armortable
	ModifyItemWeight(I_MPLATE,2);
	ModifyItemWeight(I_IMARM,3);
	ModifyItemWeight(I_ADPLATE,2);
	ModifyArmorSaveFrom("LARM", 300);
	ModifyArmorSaveValue("LARM", SLASHING,100);
	ModifyArmorSaveValue("LARM", PIERCING,75);
	ModifyArmorSaveValue("LARM", CRUSHING,60);
	ModifyArmorSaveValue("LARM", CLEAVING,75);
	ModifyArmorSaveValue("LARM", ARMORPIERCING,0);
	ModifyArmorSaveFrom("CARM", 300);
	ModifyArmorSaveValue("CARM", SLASHING,150);
	ModifyArmorSaveValue("CARM", PIERCING,100);
	ModifyArmorSaveValue("CARM", CRUSHING,75);
	ModifyArmorSaveValue("CARM", CLEAVING,150);
	ModifyArmorSaveValue("CARM", ARMORPIERCING,0);
	ModifyArmorSaveFrom("PARM", 300);
	ModifyArmorSaveValue("PARM", SLASHING,200);
	ModifyArmorSaveValue("PARM", PIERCING,200);
	ModifyArmorSaveValue("PARM", CRUSHING,225);
	ModifyArmorSaveValue("PARM", CLEAVING,150);
	ModifyArmorSaveValue("PARM", ARMORPIERCING,0);
	ModifyArmorSaveFrom("MARM", 300);
	ModifyArmorSaveValue("MARM", SLASHING,270);
	ModifyArmorSaveValue("MARM", PIERCING,270);
	ModifyArmorSaveValue("MARM", CRUSHING,285);
	ModifyArmorSaveValue("MARM", CLEAVING,225);
	ModifyArmorSaveValue("MARM", ARMORPIERCING,200);
	ModifyArmorSaveFrom("IMTH", 300);
	ModifyArmorSaveValue("IMTH", SLASHING,270);
	ModifyArmorSaveValue("IMTH", PIERCING,270);
	ModifyArmorSaveValue("IMTH", CRUSHING,285);
	ModifyArmorSaveValue("IMTH", CLEAVING,225);
	ModifyArmorSaveValue("IMTH", ARMORPIERCING,225);
	ModifyArmorSaveFrom("ARNG", 300);
	ModifyArmorSaveValue("ARNG", SLASHING,285);
	ModifyArmorSaveValue("ARNG", PIERCING,270);
	ModifyArmorSaveValue("ARNG", CRUSHING,240);
	ModifyArmorSaveValue("ARNG", CLEAVING,240);
	ModifyArmorSaveValue("ARNG", ARMORPIERCING,240);
	ModifyArmorSaveFrom("AARM", 300);
	ModifyArmorSaveValue("AARM", SLASHING,285);
	ModifyArmorSaveValue("AARM", PIERCING,285);
	ModifyArmorSaveValue("AARM", CRUSHING,285);
	ModifyArmorSaveValue("AARM", CLEAVING,240);
	ModifyArmorSaveValue("AARM", ARMORPIERCING,270);

	// Modify the various spells which are allowed to cross levels
	if(Globals->EASIER_UNDERWORLD) {
		ModifyRangeFlags("rng_teleport", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_farsight", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_clearsky", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_weather", RangeType::RNG_CROSS_LEVELS);
	}

	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		EnableSkill(S_QUARTERMASTER);
		EnableObject(O_CARAVANSERAI);
	}

	return;
}

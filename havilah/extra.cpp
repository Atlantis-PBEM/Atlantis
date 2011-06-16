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

	if (pFac->noStartLeader)
		return 1;

	//
	// Set up first unit.
	//
	Unit *temp2 = GetNewUnit( pFac );
	temp2->SetMen( I_LEADERS, 1 );
	temp2->reveal = REVEAL_FACTION;

/*
	temp2->type = U_MAGE;
	temp2->Study(S_PATTERN, 30);
	temp2->Study(S_SPIRIT, 30);
	temp2->Study(S_GATE_LORE, 30);

	if (TurnNumber() >= 25) {
		temp2->Study(S_PATTERN, 60);
		temp2->Study(S_SPIRIT, 60);
		temp2->Study(S_FORCE, 90);
		temp2->Study(S_COMBAT, 30);
	}
*/

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

	ARegion *reg = NULL;
	if (pFac->pStartLoc) {
		reg = pFac->pStartLoc;
	} else if (!Globals->MULTI_HEX_NEXUS) {
		reg = (ARegion *)(regions.First());
	} else {
		ARegionArray *pArr = regions.GetRegionArray(ARegionArray::LEVEL_NEXUS);
		while(!reg) {
			reg = pArr->GetRegion(getrandom(pArr->x), getrandom(pArr->y));
		}
	}
	temp2->MoveUnit( reg->GetDummy() );

	// Try to auto-declare all factions unfriendly to Creatures,
	// since all they do is attack you
	pFac->SetAttitude(2, 1);

	return( 1 );
}

Faction *Game::CheckVictory()
{
	int visited, unvisited;

	visited = 0;
	unvisited = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		if (TerrainDefs[r->type].similar_type != R_OCEAN &&
				r->Population() > 0) {
			if (r->visited)
				visited++;
			else
				unvisited++;
		}
	}

	printf("Players have visited %d regions; %d unvisited.\n", visited, unvisited);
	return NULL;
}

void Game::ModifyTablesPerRuleset(void)
{
	if (Globals->APPRENTICES_EXIST)
		EnableSkill(S_MANIPULATE);

	if (!Globals->GATES_EXIST)
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

	if (Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
		ClearTerrainRaces(R_NEXUS);
		ModifyTerrainRace(R_NEXUS, 0, I_HIGHELF);
		ModifyTerrainRace(R_NEXUS, 1, I_VIKING);
		ModifyTerrainRace(R_NEXUS, 2, I_PLAINSMAN);
		ClearTerrainItems(R_NEXUS);
		ModifyTerrainItems(R_NEXUS, 0, I_IRON, 100, 10);
		ModifyTerrainItems(R_NEXUS, 1, I_WOOD, 100, 10);
		ModifyTerrainItems(R_NEXUS, 2, I_STONE, 100, 10);
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
	EnableItem(I_BAXE);
	EnableItem(I_MBAXE);
	EnableItem(I_IMARM);
	EnableItem(I_SUPERBOW);
	EnableItem(I_LANCE);
	EnableItem(I_JAVELIN);
	EnableItem(I_PIKE);
	EnableItem(I_AEGIS);
	EnableItem(I_WINDCHIME);
	EnableItem(I_GATE_CRYSTAL);
	EnableItem(I_STAFFOFH);
	EnableItem(I_SCRYINGORB);
	EnableItem(I_CORNUCOPIA);
	EnableItem(I_BOOKOFEXORCISM);
	EnableItem(I_HOLYSYMBOL);
	EnableItem(I_CENSER);

	EnableItem(I_FOOD);
	EnableSkill(S_COOKING);
	EnableSkill(S_CREATE_FOOD);

	DisableItem(I_STAFFOFL);
	DisableItem(I_GNOME);

	EnableItem(I_LONGBOAT);
	EnableItem(I_CLIPPER);
	EnableItem(I_GALLEON);
	EnableItem(I_AGALLEON);
	DisableItem(I_LONGSHIP);
	DisableItem(I_KNARR);
	DisableItem(I_GALLEY);
	DisableItem(I_TRIREME);
	DisableItem(I_COG);
	DisableItem(I_CARRACK);

	EnableSkill(S_ARMORCRAFT);
	EnableSkill(S_WEAPONCRAFT);
	EnableSkill(S_CREATE_AEGIS);
	EnableSkill(S_CREATE_WINDCHIME);
	EnableSkill(S_CREATE_GATE_CRYSTAL);
	EnableSkill(S_CREATE_STAFF_OF_HEALING);
	EnableSkill(S_CREATE_SCRYING_ORB);
	EnableSkill(S_CREATE_CORNUCOPIA);
	EnableSkill(S_CREATE_BOOK_OF_EXORCISM);
	EnableSkill(S_CREATE_HOLY_SYMBOL);
	EnableSkill(S_CREATE_CENSER);

	DisableSkill(S_CREATE_STAFF_OF_LIGHTNING);

	EnableObject(O_ROADN);
	EnableObject(O_ROADNE);
	EnableObject(O_ROADNW);
	EnableObject(O_ROADS);
	EnableObject(O_ROADSE);
	EnableObject(O_ROADSW);
	EnableObject(O_TEMPLE);
	EnableObject(O_MQUARRY);
	EnableObject(O_AMINE);
	EnableObject(O_PRESERVE);
	EnableObject(O_SACGROVE);
	EnableObject(O_MTOWER);
	EnableObject(O_MFORTRESS);
	EnableObject(O_MCITADEL);
	ModifyObjectName(O_MFORTRESS, "Magical Fortress");
	ModifyObjectName(O_MCASTLE, "Magical Castle");
	ModifyObjectManpower(O_TOWER,
		ObjectDefs[O_TOWER].protect,
		ObjectDefs[O_TOWER].capacity,
		ObjectDefs[O_TOWER].sailors,
		0);
	ModifyObjectManpower(O_FORT,
		ObjectDefs[O_FORT].protect,
		ObjectDefs[O_FORT].capacity,
		ObjectDefs[O_FORT].sailors,
		1);
	ModifyObjectManpower(O_CASTLE,
		ObjectDefs[O_CASTLE].protect,
		ObjectDefs[O_CASTLE].capacity,
		ObjectDefs[O_CASTLE].sailors,
		2);
	ModifyObjectManpower(O_CITADEL,
		ObjectDefs[O_CITADEL].protect,
		ObjectDefs[O_CITADEL].capacity,
		ObjectDefs[O_CITADEL].sailors,
		4);
	ModifyObjectManpower(O_AGALLEON,
		ObjectDefs[O_AGALLEON].protect,
		ObjectDefs[O_AGALLEON].capacity,
		ObjectDefs[O_AGALLEON].sailors,
		1);

	ModifyTerrainItems(R_PLAIN, 0, I_HORSE, 25, 20);
	ModifyTerrainItems(R_PLAIN, 1, -1, 0, 0);
	ModifyTerrainItems(R_TUNDRA, 2, I_WHORSE, 25, 5);
	ModifyTerrainItems(R_DESERT, 3, I_HORSE, 75, 20);
	ModifyTerrainItems(R_JUNGLE, 3, I_IRONWOOD, 20, 5);
	ModifyTerrainItems(R_UFOREST, 4, I_IRONWOOD, 20, 5);
	ModifyTerrainItems(R_MOUNTAIN, 4, I_WOOD, 10, 5);
	ModifyTerrainItems(R_MOUNTAIN, 5, I_YEW, 10, 5);

	ModifyItemMagicInput(I_RINGOFI, 0, I_MITHRIL, 1);
	ModifyItemMagicInput(I_RINGOFI, 1, I_SILVER, 600);
	ModifyItemMagicInput(I_CLOAKOFI, 0, I_FUR, 1);
	ModifyItemMagicInput(I_CLOAKOFI, 1, I_SILVER, 800);
	ModifyItemMagicInput(I_STAFFOFF, 0, I_IRONWOOD, 1);
	ModifyItemMagicInput(I_STAFFOFF, 1, I_SILVER, 500);
	ModifyItemMagicInput(I_STAFFOFL, 0, I_IRONWOOD, 1);
	ModifyItemMagicInput(I_STAFFOFL, 1, I_SILVER, 900);
	ModifyItemMagicInput(I_AMULETOFTS, 0, I_ROOTSTONE, 1);
	ModifyItemMagicInput(I_AMULETOFTS, 1, I_SILVER, 500);
	ModifyItemMagicInput(I_AMULETOFP, 0, I_STONE, 1);
	ModifyItemMagicInput(I_AMULETOFP, 1, I_SILVER, 200);
	ModifyItemMagicInput(I_RUNESWORD, 0, I_MSWORD, 1);
	ModifyItemMagicInput(I_RUNESWORD, 1, I_SILVER, 600);
	ModifyItemMagicInput(I_SHIELDSTONE, 0, I_STONE, 1);
	ModifyItemMagicInput(I_SHIELDSTONE, 1, I_SILVER, 200);
	ModifyItemMagicInput(I_MCARPET, 0, I_FUR, 1);
	ModifyItemMagicInput(I_MCARPET, 1, I_SILVER, 400);
	ModifyItemMagicInput(I_PORTAL, 0, I_ROOTSTONE, 1);
	ModifyItemMagicInput(I_PORTAL, 1, I_SILVER, 500);
	ModifyItemMagicInput(I_FSWORD, 0, I_MSWORD, 1);
	ModifyItemMagicInput(I_FSWORD, 1, I_SILVER, 600);

	ModifyHealing(2, 15, 60);
	ModifyHealing(4, 50, 80);

	EnableObject(O_ISLE);
	EnableObject(O_DERELICT);
	EnableObject(O_OCAVE);
	EnableObject(O_WHIRL);
	DisableObject(O_BKEEP);
	DisableObject(O_PALACE);
	EnableItem(I_PIRATES);
	EnableItem(I_KRAKEN);
	EnableItem(I_MERFOLK);
	EnableItem(I_ELEMENTAL);

	if ((Globals->UNDERDEEP_LEVELS > 0) || (Globals->UNDERWORLD_LEVELS > 1)) {
		EnableItem(I_MUSHROOM);
		EnableItem(I_HEALPOTION);
		EnableItem(I_ROUGHGEM);
		EnableItem(I_GEMS);
		EnableSkill(S_GEMCUTTING);
	}

	// Modify the various spells which are allowed to cross levels
	if (Globals->EASIER_UNDERWORLD) {
		ModifyRangeFlags("rng_teleport", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_farsight", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_clearsky", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_weather", RangeType::RNG_CROSS_LEVELS);
	}

	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		EnableSkill(S_QUARTERMASTER);
		EnableObject(O_CARAVANSERAI);
	}
	// XXX -- This is just here to preserve existing behavior
	ModifyItemProductionBooster(I_AXE, I_HAMMER, 1);
	return;
}

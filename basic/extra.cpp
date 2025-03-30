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

/** \file
 * Extra parts added to the game for a particular version.
 * extra.cpp contains all of the version-specific functions necessary
 * to alter a game's data structures to suit the GM.
 */

#include "game.h"
#include "gamedata.h"

/// Run the initial setup for a faction
/** This includes setting initial silver, creating your first leader,
adding any starting skills (eg. gate) and sticking him in the world.
*/
int Game::SetupFaction( Faction *pFac )
{
	pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 50;

	if (pFac->noStartLeader)
		return 1;

	//
	// Set up first unit.
	//
	Unit *temp2 = GetNewUnit( pFac );
	temp2->SetMen(I_LEADERS, 1);
	pFac->DiscoverItem(I_LEADERS, 0, 1);
	temp2->reveal = REVEAL_FACTION;

	if (TurnNumber() >= 12) {
		temp2->type = U_MAGE;
		temp2->Study(S_PATTERN, 30);
		temp2->Study(S_SPIRIT, 30);
		temp2->Study(S_GATE_LORE, 30);
	}

	if (Globals->UPKEEP_MINIMUM_FOOD > 0)
	{
		if (!(ItemDefs[I_FOOD].flags & ItemType::DISABLED)) {
			temp2->items.SetNum(I_FOOD, 6);
			pFac->DiscoverItem(I_FOOD, 0, 1);
		} else if (!(ItemDefs[I_FISH].flags & ItemType::DISABLED)) {
			temp2->items.SetNum(I_FISH, 6);
			pFac->DiscoverItem(I_FISH, 0, 1);
		} else if (!(ItemDefs[I_LIVESTOCK].flags & ItemType::DISABLED)) {
			temp2->items.SetNum(I_LIVESTOCK, 6);
			pFac->DiscoverItem(I_LIVESTOCK, 0, 1);
		} else if (!(ItemDefs[I_GRAIN].flags & ItemType::DISABLED)) {
			temp2->items.SetNum(I_GRAIN, 2);
			pFac->DiscoverItem(I_GRAIN, 0, 1);
		}
		temp2->items.SetNum(I_SILVER, 10);
	}

	ARegion *reg = NULL;
	if (pFac->pStartLoc) {
		reg = pFac->pStartLoc;
	} else if (!Globals->MULTI_HEX_NEXUS) {
		reg = regions.front();
	} else {
		ARegionArray *pArr = regions.GetRegionArray(ARegionArray::LEVEL_NEXUS);
		while(!reg) {
			reg = pArr->GetRegion(rng::get_random(pArr->x), rng::get_random(pArr->y));
		}
	}
	temp2->MoveUnit( reg->GetDummy() );

	return( 1 );
}

/// Check to see whether a player has won the game.
/** This is left null in standard, since it's an open ended game.
See wyreth if you're after an example of close-ended conditions.
*/
Faction *Game::CheckVictory()
{
	return NULL;
}

/// Modify certain starting statistics of the world's data structures.
/** There are two types of changes in here, those caused by altering
values in rules.cpp, and those added by the GM to change the default
settings of the world. See the modify.cpp file if you want to make changes
to your own game.
Please don't alter standard -- copy it and alter that copy as detailed
in the GAMEMASTER file.
*/
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
		ModifyRangeFlags("rng_portal", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_farsight", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_clearsky", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_weather", RangeType::RNG_CROSS_LEVELS);
	}

	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		EnableSkill(S_QUARTERMASTER);
		EnableObject(O_CARAVANSERAI);
	}

	EnableItem(I_LONGBOAT);
	EnableItem(I_CLIPPER);
	EnableItem(I_GALLEON);
	EnableItem(I_AGALLEON);
	DisableItem(I_LONGSHIP);
	DisableItem(I_RAFT);
	DisableItem(I_COG);
	DisableItem(I_GALLEY);
	DisableItem(I_CORSAIR);
	DisableItem(I_AIRSHIP);
	DisableItem(I_CLOUDSHIP);
	ModifyObjectManpower(O_GALLEON,
			 ObjectDefs[O_GALLEON].protect,
			 ObjectDefs[O_GALLEON].capacity,
			 ObjectDefs[O_GALLEON].sailors,
			 0);

	return;
}

const char *ARegion::movement_forbidden_by_ruleset(Unit *u, ARegion *origin, ARegionList& regs) { return nullptr; }

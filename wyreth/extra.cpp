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

    if (TurnNumber() >= 25) {
		temp2->Study(S_PATTERN, 60);
		temp2->Study(S_SPIRIT, 60);
		temp2->Study(S_FORCE, 90);
		temp2->Study(S_COMBAT, 30);
    }

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

int Game::AllowedMages( Faction *pFac )
{
	int points = pFac->type[F_MAGIC];

	if (points < 0) points = 0;
	if (points > allowedMagesSize - 1) points = allowedMagesSize - 1;

    return allowedMages[points];
}

int Game::AllowedApprentices( Faction *pFac )
{
	int points = pFac->type[F_MAGIC];

	if (points < 0) points = 0;
	if (points > allowedApprenticesSize - 1)
		points = allowedApprenticesSize - 1;

    return allowedApprentices[points];
}

int Game::AllowedTaxes( Faction *pFac )
{
	int points = pFac->type[F_WAR];

	if (points < 0) points = 0;
	if (points > allowedTaxesSize - 1) points = allowedTaxesSize - 1;

    return allowedTaxes[points];
}

int Game::AllowedTrades( Faction *pFac )
{
	int points = pFac->type[F_TRADE];

	if (points < 0) points = 0;
	if (points > allowedTradesSize - 1) points = allowedTradesSize - 1;

    return allowedTrades[points];
}

void Game::ModifyTablesPerRuleset(void)
{
	if(Globals->APPRENTICES_EXIST)
	   	EnableSkill(S_MANIPULATE);

    if(!Globals->GATES_EXIST)
		DisableSkill(S_GATE_LORE);

	if(Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
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

	EnableSkill(S_ARMORCRAFT);
	EnableSkill(S_WEAPONCRAFT);

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

	EnableObject(O_ISLE);
	EnableObject(O_DERELICT);
	EnableObject(O_OCAVE);
	EnableObject(O_WHIRL);
	EnableItem(I_PIRATES);
	EnableItem(I_KRAKEN);
	EnableItem(I_MERFOLK);
	EnableItem(I_ELEMENTAL);

	if((Globals->UNDERDEEP_LEVELS > 0) || (Globals->UNDERWORLD_LEVELS > 1)) {
		EnableItem(I_MUSHROOM);
		EnableItem(I_HEALPOTION);
		EnableItem(I_ROUGHGEM);
		EnableItem(I_GEMS);
		EnableSkill(S_GEMCUTTING);
	}

	// Modify the various spells which are allowed to cross levels
	if(Globals->EASIER_UNDERWORLD) {
		ModifyRangeFlags(RANGE_TELEPORT, RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags(RANGE_FARSIGHT, RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags(RANGE_CLEAR_SKIES, RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags(RANGE_WEATHER_LORE, RangeType::RNG_CROSS_LEVELS);
	}

	// XXX -- This is just here to preserve existing behavior
	ModifyItemProductionBooster(I_AXE, I_HAMMER, 1);
	return;
}

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
	// Factions can only be added at the start of the game
	if(TurnNumber() != 1)
	{
		return (0);
	}

	// Lets see if there is an available regions for this unit
	ARegion *pReg = 0;
	forlist(&regions) {
		pReg = (ARegion *)elem;
		ARegionArray *pRA = regions.pRegionArrays[pReg->zloc];

		if(!pReg->CanBeStartingCity(pRA)) {
			pReg = 0;
			continue;
		}

		if(pReg->IsStartingCity()) {
			// This region has already been set up.
			pReg = 0;
			continue;
		}
		break;
	}

	if(!pReg) {
		// We couldn't find a region to make the faction in.
		return 0;
	}

    pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 50;

	// Make a citadel for this faction
	Object *obj = new Object(pReg);
	obj->num = pReg->buildingseq++;
	obj->name = new AString(AString("Palace [")+obj->num+"]");
	obj->type = O_PALACE;
	obj->incomplete = 0;
	obj->inner = -1;
	pReg->objects.Add(obj);

    //
    // Set up first unit.
    //
    Unit *temp2 = GetNewUnit( pFac );
    temp2->SetMen( I_MAN, 1 );
    temp2->reveal = REVEAL_FACTION;
	temp2->MoveUnit(obj);

    return( 1 );
}

Faction *Game::CheckVictory()
{
	Faction *pVictor = 0;

	// First, if there is only one living faction, it is the winner.
	forlist(&factions) {
		Faction *pFac = (Faction *)elem;
		if (pFac->IsNPC()) continue;
		if(pFac->exists) {
			if(pVictor) {
				// This is the second faction we've found.  No winner.
				pVictor = 0;
				break;
			}
			pVictor = pFac;
		}
	}

	if(pVictor)
		return pVictor;

	// Next, if one faction holds all citadels, it is the winner
	{
		forlist(&regions) {
			ARegion *pReg = (ARegion *)elem;
			if(pReg->IsStartingCity()) {
				forlist(&(pReg->objects)) {
					Object *pObj = (Object *)elem;
					if(pObj->type != O_PALACE) {
						continue;
					}
					Unit *u = pObj->GetOwner();
					if(!u) {
						// Noone controls
						return 0;
					}
					if(!pVictor) {
						pVictor = u->faction;
					} else {
						if(pVictor != u->faction) {
							return 0;
						}
					}
					// We've found one citadel.. no need to keep going on
					// this object
					break;
				}
			}
		}
	}
	return pVictor;
}

int Game::AllowedMages( Faction *pFac )
{
    return 5;
}

int Game::AllowedApprentices( Faction *pFac )
{
    return 10;
}

int Game::AllowedTaxes( Faction *pFac )
{
    return -1;
}

int Game::AllowedTrades( Faction *pFac )
{
    return -1;
}

void Game::ModifyTablesPerRuleset(void)
{
	/* There are no foodstuffs in conquest */
	DisableSkill(S_FISHING);
	DisableSkill(S_FARMING);
	DisableSkill(S_RANCHING);

	/* Enable apprentices if we should */
    if(Globals->APPRENTICES_EXIST)
		EnableSkill(S_MANIPULATE);

	/* Enable the man type */
	EnableItem(I_MAN);

	/* Disable production enhancer buildings */
	DisableObject(O_RANCH);
	DisableObject(O_FARM);

	/* All men are created equal */
	ClearTerrainRaces(R_PLAIN);
	ModifyTerrainRace(R_PLAIN, 0, I_MAN);
	ClearTerrainRaces(R_FOREST);
	ModifyTerrainRace(R_FOREST, 0, I_MAN);
	ClearTerrainRaces(R_MOUNTAIN);
	ModifyTerrainRace(R_MOUNTAIN, 0, I_MAN);
	ClearTerrainRaces(R_SWAMP);
	ModifyTerrainRace(R_SWAMP, 0, I_MAN);
	ClearTerrainRaces(R_ISLAND_PLAIN);
	ModifyTerrainRace(R_ISLAND_PLAIN, 0, I_MAN);
	ClearTerrainRaces(R_ISLAND_MOUNTAIN);
	ModifyTerrainRace(R_ISLAND_MOUNTAIN, 0, I_MAN);
	ClearTerrainRaces(R_ISLAND_SWAMP);
	ModifyTerrainRace(R_ISLAND_SWAMP, 0, I_MAN);

	if(!Globals->GATES_EXIST)
		DisableSkill(S_GATE_LORE);

	if(Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
		ClearTerrainRaces(R_NEXUS);
		ModifyTerrainRace(R_NEXUS, 0, I_MAN);
		ClearTerrainItems(R_NEXUS);
		ModifyTerrainItems(R_NEXUS, 0, I_IRON, 100, 10);
		ModifyTerrainItems(R_NEXUS, 1, I_WOOD, 100, 10);
		ModifyTerrainItems(R_NEXUS, 2, I_STONE, 100, 10);

		ModifyTerrainEconomy(R_NEXUS, 1000, 15, 50, 2);
	}

	/* Limit the resources on the islands and remove fish */
	ClearTerrainItems(R_OCEAN);

	ClearTerrainItems(R_ISLAND_PLAIN);
	ModifyTerrainItems(R_ISLAND_PLAIN, 0, I_HORSE, 100, 20);

	ClearTerrainItems(R_ISLAND_SWAMP);
	ModifyTerrainItems(R_ISLAND_SWAMP, 0, I_WOOD, 100, 10);

	ClearTerrainItems(R_ISLAND_MOUNTAIN);
	ModifyTerrainItems(R_ISLAND_MOUNTAIN, 0, I_IRON, 100, 20);
	return;
}

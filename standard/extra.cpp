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

    //
    // Set up first unit.
    //
    Unit *temp2 = GetNewUnit( pFac );
    temp2->SetMen( I_LEADERS, 1 );
    temp2->reveal = REVEAL_FACTION;

    if (TurnNumber() >= 12) {
        temp2->type = U_MAGE;
        temp2->Study(S_PATTERN, 30);
        temp2->Study(S_SPIRIT, 30);
        temp2->Study(S_GATE_LORE, 30);
    }

    ARegion *reg = NULL;
	if(!Globals->MULTI_HEX_NEXUS) {
		reg = (ARegion *)(regions.First());
	} else {
		ARegionArray * pArr = regions.GetRegionArray(ARegionArray::LEVEL_NEXUS);
		while(!reg) {
			reg = pArr->GetRegion(getrandom(pArr->x), getrandom(pArr->y));
		}
	}
	temp2->MoveUnit( reg->GetDummy() );

    return( 1 );
}

Faction *Game::CheckVictory()
{
	return NULL;
}

int Game::AllowedMages( Faction *pFac )
{
    switch( pFac->type[ F_MAGIC ]) {
		case 0: return 0;
		case 1: return 1;
		case 2: return 2;
		case 3: return 3;
		case 4: return 5;
		case 5: return 7;
    }
    return 0;
}

int Game::AllowedApprentices( Faction *pFac )
{
    switch( pFac->type[ F_MAGIC ]) {
		case 0: return 0;
		case 1: return 2;
		case 2: return 4;
		case 3: return 6;
		case 4: return 10;
		case 5: return 14;
	}
	return 0;
}

int Game::AllowedTaxes( Faction *pFac )
{
    switch( pFac->type[ F_WAR ]) {
		case 0: return 0;
		case 1: return 10;
		case 2: return 24;
		case 3: return 40;
		case 4: return 60;
		case 5: return 100;
	}
	return 0;
}

int Game::AllowedTrades( Faction *pFac )
{
	switch( pFac->type[ F_TRADE ]) {
		case 0: return 0;
		case 1: return 10;
		case 2: return 24;
		case 3: return 40;
		case 4: return 60;
		case 5: return 100;
	}
    return 0;
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

	if((Globals->UNDERDEEP_LEVELS > 0) || (Globals->UNDERWORLD_LEVELS > 1)) {
		EnableItem(I_MUSHROOM);
		EnableItem(I_HEALPOTION);
		EnableItem(I_ROUGHGEM);
		EnableItem(I_GEMS);
		EnableSkill(S_GEMCUTTING);
	}

	return;
}

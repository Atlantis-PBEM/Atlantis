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
/// For unit tests, factions get 1 leader with combat 3.
int Game::SetupFaction( Faction *pFac ) {
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
    // combat skill 3
	temp2->Study(S_COMBAT, 180);
    // Set up flags
	temp2->SetFlag(FLAG_BEHIND, 1);

    // Put the unit in the first region (which will be the one city for the test game)
	ARegion *reg = regions.front();
	temp2->MoveUnit(reg->GetDummy());

    return 1;
}

/// Check to see whether a player has won the game.
/// This is NULL in the unittests.
Faction *Game::CheckVictory() { return NULL; }

/// Modify certain starting statistics of the world's data structures.
void Game::ModifyTablesPerRuleset(void) {
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		EnableSkill(S_QUARTERMASTER);
		EnableObject(O_CARAVANSERAI);
	}
}

const char *ARegion::movement_forbidden_by_ruleset(Unit *u, ARegion *origin, ARegionList& regs) { return nullptr; }

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
/// which for a unit test is to do nothing
int Game::SetupFaction( Faction *pFac ) { return 1; }

/// Check to see whether a player has won the game.
/// This is NULL in the unittests.
Faction *Game::CheckVictory() { return NULL; }

/// Modify certain starting statistics of the world's data structures.
void Game::ModifyTablesPerRuleset(void) { }

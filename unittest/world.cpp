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
// MODIFICATIONS
// Date		Person			Comments
// ----		------			--------
// 2000/SEP/06 Joseph Traub	  Added base man cost to allow races to have
//							   different base costs
#include "game.h"
#include "gamedata.h"

int AGetName(int town, ARegion *reg) { return town != 0 ? 1 : 0; }
char const *AGetNameString( int name ) { return (name == 0) ? "region" : "town"; }

void Game::CreateWorld() { }

int ARegionList::GetRegType( ARegion *pReg ) { return 0; }

// Unit test levels are unscaled.
int ARegionList::GetLevelXScale(int level) { return 1; }

// Unit test levels are unscaled.
int ARegionList::GetLevelYScale(int level) { return 1; }

// Unit test regions are fully connected
int ARegionList::CheckRegionExit(ARegion *pFrom, ARegion *pTo ) { return 1; }

int ARegionList::GetWeather( ARegion *pReg, int month ) { return W_NORMAL; }

int ARegion::CanBeStartingCity( ARegionArray *pRA ) { return 1; }

void ARegion::MakeStartingCity() { }

int ARegion::IsStartingCity() { return 0; }

int ARegion::IsSafeRegion() { return 0; }

ARegion *ARegionList::GetStartingCity(ARegion *AC, int i, int level, int maxX, int maxY) { return NULL; }

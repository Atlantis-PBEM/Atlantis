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
// Date        Person            Comments
// ----        ------            --------
// 2001/Mar/03 Joseph Traub      Moved some of the monster stuff here
//
#include "gamedata.h"
#include "game.h"

int Game::MakeWMon( ARegion *pReg )
{
	if(!Globals->WANDERING_MONSTERS_EXIST) return 0;

	if (TerrainDefs[pReg->type].wmonfreq == 0) {
        return 0;
    }

	int montype = TerrainDefs[ pReg->type ].smallmon;
	if (getrandom(2))
		montype = TerrainDefs[ pReg->type ].humanoid;
	if (TerrainDefs[ pReg->type ].bigmon != -1 && !getrandom(8)) {
		montype = TerrainDefs[ pReg->type ].bigmon;
	}

	int mondef = ItemDefs[montype].index;

	Faction *monfac = GetFaction( &factions, 2 );

	Unit *u = GetNewUnit( monfac, 0 );
	u->MakeWMon( MonDefs[mondef].name, montype,
			(MonDefs[mondef].number +
			 getrandom(MonDefs[mondef].number) + 1) / 2);
	u->MoveUnit( pReg->GetDummy() );
	return( 1 );
}

void Game::MakeLMon( Object *pObj )
{

	if(!Globals->LAIR_MONSTERS_EXIST) return;

	if(ObjectDefs[pObj->type].flags & ObjectType::NO_MON_GROWTH)
		return;

	int montype = ObjectDefs[ pObj->type ].monster;

	if (montype == I_TRENT) {
		montype = TerrainDefs[ pObj->region->type].bigmon;
	}
	if (montype == I_CENTAUR) {
		montype = TerrainDefs[ pObj->region->type ].humanoid;
	}
	int mondef = ItemDefs[montype].index;
	Faction *monfac = GetFaction( &factions, 2 );
	Unit *u = GetNewUnit( monfac, 0 );
	if (montype == I_IMP) {
		u->MakeWMon( "Demons", I_IMP,
				getrandom( MonDefs[MONSTER_IMP].number + 1 ));
		u->items.SetNum( I_DEMON,
				getrandom( MonDefs[MONSTER_DEMON].number + 1 ));
		u->items.SetNum( I_BALROG,
				getrandom( MonDefs[MONSTER_BALROG].number + 1 ));
	} else if (montype == I_SKELETON) {
		u->MakeWMon( "Undead", I_SKELETON,
				getrandom( MonDefs[MONSTER_SKELETON].number + 1 ));
		u->items.SetNum( I_UNDEAD,
				getrandom( MonDefs[MONSTER_UNDEAD].number + 1 ));
		u->items.SetNum( I_LICH,
				getrandom( MonDefs[MONSTER_LICH].number + 1 ));
	} else {
		u->MakeWMon( MonDefs[mondef].name, montype,
				(MonDefs[mondef].number +
				 getrandom( MonDefs[mondef].number ) + 1) / 2);
	}

	u->MoveUnit( pObj );
}

void Game::CreateVMons()
{
	// No victory monsters in this game
	return;
}

void Game::GrowVMons()
{
	// No victory monsters in this game
	return;
}

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

    temp2->MoveUnit( ((ARegion *) (regions.First()))->GetDummy() );

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
	switch( pFac->type[ F_MAGIC ]) {
		case 0: return 1;
		case 1: return 2;
		case 2: return 3;
		case 3: return 4;
		case 4: return 6;
		case 5: return 8;
		case 6: return 12;
    }
    return 0;
}

int Game::AllowedApprentices( Faction *pFac )
{
	switch( pFac->type[ F_MAGIC ]) {
		case 0: return 1;
		case 1: return 3;
		case 2: return 5;
		case 3: return 7;
		case 4: return 11;
		case 5: return 15;
		case 6: return 20;
	}
	return 0;
}

int Game::AllowedTaxes( Faction *pFac )
{
	switch( pFac->type[ F_WAR ]) {
		case 0: return 1;
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
		case 0: return 1;
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

	EnableItem(I_LANCE);
	EnableItem(I_FIRESPEAR);
	EnableItem(I_ICESPEAR);
	EnableItem(I_MUSHROOM);

	EnableSkill(S_TAME_DRAGON);
	EnableSkill(S_WEAPONCRAFT);
	EnableSkill(S_ARMORCRAFT);

	EnableItem(I_FAIRY);
	EnableItem(I_LIZARDMAN);
	EnableItem(I_URUK);
	EnableItem(I_GOBLINMAN);
	EnableItem(I_HOBBIT);
	EnableItem(I_GNOLL);
	EnableItem(I_DROW);
	EnableItem(I_MERC);
	EnableItem(I_TITAN);
	EnableItem(I_AMAZON);
	EnableItem(I_OGREMAN);
	EnableItem(I_GNOME);
	EnableItem(I_HIGHLANDER);
	EnableItem(I_MINOTAUR);

	ModifyTerrainRaces(R_NEXUS, I_FAIRY, -1, -1, -1, -1);
    ModifyTerrainItems(R_NEXUS, I_HERBS, 100, 20, I_ROOTSTONE, 100, 10,
			I_MITHRIL, 100, 10, I_YEW, 100, 10, I_IRONWOOD, 100, 10);
	ModifyTerrainEconomy(R_NEXUS, 1000, 15, 50, 2);
	return;
}

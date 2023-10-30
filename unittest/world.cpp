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
char const *AGetNameString( int name ) { return (name == 0) ? "Testing Wilds" : "Basictown"; }

void Game::CreateWorld() {
    Awrite("Creating world");
    regions.CreateLevels(1);    
    // because of the way regions are numbered, if you want 4 hexes you need a height of 4 and a width of 2.
    regions.CreateSurfaceLevel(0, 2, 4, nullptr);
	
	ARegion *reg = regions.GetRegion(0,0,0);
	reg->MakeStartingCity();
 }

int ARegionList::GetRegType( ARegion *pReg ) { return 0; }

// Unit test levels are unscaled.
int ARegionList::GetLevelXScale(int level) { return 1; }

// Unit test levels are unscaled.
int ARegionList::GetLevelYScale(int level) { return 1; }

// Unit test regions are fully connected
int ARegionList::CheckRegionExit(ARegion *pFrom, ARegion *pTo ) { return 1; }

int ARegionList::GetWeather( ARegion *pReg, int month ) { return W_NORMAL; }

int ARegion::CanBeStartingCity( ARegionArray *pRA ) { return 1; }

void ARegion::MakeStartingCity() {
    if (!Globals->TOWNS_EXIST) return;
	if (town) delete town;
	
	AddTown(TOWN_CITY);

	if (!Globals->START_CITIES_EXIST) return;

	town->hab = 125 * Globals->CITY_POP / 100;
	while (town->pop < town->hab) town->pop += getrandom(200)+200;
	town->dev = TownDevelopment();

	float ratio;
	for (auto& m : markets) delete m; // Free the allocated object
	markets.clear(); // empty the vector.

	Market *m;
	if (Globals->START_CITIES_START_UNLIMITED) {
		for (int i=0; i<NITEMS; i++) {
			if ( ItemDefs[i].flags & ItemType::DISABLED ) continue;
			if ( ItemDefs[ i ].type & IT_NORMAL ) {
				if (i==I_SILVER || i==I_LIVESTOCK || i==I_FISH || i==I_GRAIN)
					continue;
				m = new Market(M_BUY,i,(ItemDefs[i].baseprice*5/2),-1, 5000,5000,-1,-1);
				markets.push_back(m);
			}
		}
		ratio = ItemDefs[race].baseprice / ((float)Globals->BASE_MAN_COST * 10);
		// hack: include wage factor of 10 in float calculation above
		m = new Market(M_BUY,race,(int)(Wages()*4*ratio),-1, 5000,5000,-1,-1);
		markets.push_back(m);
		if (Globals->LEADERS_EXIST) {
			ratio=ItemDefs[I_LEADERS].baseprice/((float)Globals->BASE_MAN_COST * 10);
			// hack: include wage factor of 10 in float calculation above
			m = new Market(M_BUY,I_LEADERS,(int)(Wages()*4*ratio), -1,5000,5000,-1,-1);
			markets.push_back(m);
		}
	} else {
		SetupCityMarket();
		ratio = ItemDefs[race].baseprice / ((float)Globals->BASE_MAN_COST * 10);
		// hack: include wage factor of 10 in float calculation above
		/* Setup Recruiting */
		m = new Market( M_BUY, race, (int)(Wages()*4*ratio), Population()/5, 0, 10000, 0, 2000 );
		markets.push_back(m);
		if ( Globals->LEADERS_EXIST ) {
			ratio=ItemDefs[I_LEADERS].baseprice/((float)Globals->BASE_MAN_COST * 10);
			// hack: include wage factor of 10 in float calculation above
			m = new Market( M_BUY, I_LEADERS, (int)(Wages()*4*ratio), Population()/25, 0, 10000, 0, 400 );
			markets.push_back(m);
		}
	}
}

int ARegion::IsStartingCity() { return town != nullptr; }

int ARegion::IsSafeRegion() { return 0; }

ARegion *ARegionList::GetStartingCity(ARegion *AC, int i, int level, int maxX, int maxY) { return NULL; }

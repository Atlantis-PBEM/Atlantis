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
#include "game.h"
#include "gamedata.h"

void Game::CreateCityMons()
{
    if( !Globals->CITY_MONSTERS_EXIST )
    {
        return;
    }

    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        if (r->type == R_NEXUS || r->IsStartingCity() || r->town)
        {
            CreateCityMon( r, 100 );
        }
    }
}

void Game::CreateWMons()
{
    if( !Globals->WANDERING_MONSTERS_EXIST )
    {
        return;
    }

    GrowWMons( 50 );
}

void Game::CreateLMons()
{
    if( !Globals->LAIR_MONSTERS_EXIST )
    {
        return;
    }

    GrowLMons( 50 );
}

void Game::GrowWMons(int rate)
{
    //
    // Now, go through each 8x8 block of the map, and make monsters if
    // needed.
    //
    int level;
    for( level = 0; level < regions.numLevels; level++ )
    {
        ARegionArray *pArr = regions.pRegionArrays[ level ];
        int xsec;
        for (xsec=0; xsec< pArr->x / 8; xsec++)
        {
            for (int ysec=0; ysec< pArr->y / 16; ysec++)
            {
                /* OK, we have a sector. Count mons, and wanted */
                int mons=0;
                int wanted=0;
                for (int x=0; x<8; x++)
                {
                    for (int y=0; y<16; y+=2)
                    {
                        ARegion *reg = pArr->GetRegion(x+xsec*8,y+ysec*16+x%2);
                        if (!reg->IsGuarded())
                        {
                            mons += reg->CountWMons();
                            wanted += TerrainDefs[reg->type].wmonfreq;
                        }
                    }
                }

                wanted /= 10;
                wanted -= mons;
                wanted = (wanted*rate + getrandom(100))/100;
                if (wanted > 0)
                {
                    for (int i=0; i< wanted;)
                    {
                        int m=getrandom(8);
                        int n=getrandom(8)*2+m%2;
                        ARegion *reg = pArr->GetRegion(m+xsec*8,n+ysec*16);
                        if (!reg->IsGuarded() && MakeWMon( reg ))
                        {
                            i++;
                        }
                    }
                }
            }
        }
    }
}

void Game::GrowLMons( int rate )
{
    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        //
        // Don't make lmons in guarded regions
        //
        if (r->IsGuarded()) continue;
        
        forlist(&r->objects) {
            Object * obj = (Object *) elem;
            if (obj->units.Num()) continue;
            int montype = ObjectDefs[obj->type].monster;
			int grow = !(ObjectDefs[obj->type].flags & ObjectType::NO_MON_GROWTH);
            if ((montype != -1) && grow) {
                if (getrandom(100) < rate) {
                    MakeLMon( obj );
                }
            }
        }
    }
}

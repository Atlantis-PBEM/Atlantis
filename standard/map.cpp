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

#include <stdio.h>
#include <string.h>
#include "game.h"
#include "gamedata.h"

int ARegion::CheckSea(int dir, int range, int remainocean)
{
	if (type != R_OCEAN) return 0;
	if (range-- < 1) return 1;
	for (int d2 = -1; d2< 2; d2++) {
		int direc = (dir + d2 + NDIRS) % NDIRS;
		ARegion *newregion = neighbors[direc];
		if (!newregion) continue;
		remainocean += newregion->CheckSea(dir, range, remainocean);
		if (remainocean) break;
	}
	return remainocean;
}


void ARegionList::CreateAbyssLevel(int level, char const *name)
{
	MakeRegions(level, 4, 4);
	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_NEXUS;

	ARegion *reg = NULL;
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			reg = pRegionArrays[level]->GetRegion(x, y);
			if (!reg) continue;
			reg->SetName("Abyssal Plains");
			reg->type = R_DESERT;
			reg->wages = -2;
		}
	}

	int tempx, tempy;
	if (Globals->GATES_EXIST) {
		int gateset = 0;
		do {
			tempx = getrandom(4);
			tempy = getrandom(4);
			reg = pRegionArrays[level]->GetRegion(tempx, tempy);
			if (reg) {
				gateset = 1;
				numberofgates++;
				reg->gate = -1;
			}
		} while(!gateset);
	}

	FinalSetup(pRegionArrays[level]);

	ARegion *lair = NULL;
	do {
		tempx = getrandom(4);
		tempy = getrandom(4);
		lair = pRegionArrays[level]->GetRegion(tempx, tempy);
	} while(!lair || lair == reg);
	Object *o = new Object(lair);
	o->num = lair->buildingseq++;
	o->name = new AString(AString(ObjectDefs[O_BKEEP].name)+" ["+o->num+"]");
	o->type = O_BKEEP;
	o->incomplete = 0;
	o->inner = -1;
	lair->objects.Add(o);
}


void ARegionList::CreateNexusLevel(int level, int xSize, int ySize, char const *name)
{
	MakeRegions(level, xSize, ySize);

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_NEXUS;

	AString nex_name = Globals->WORLD_NAME;
	nex_name += " Nexus";

	int x, y;
	for (y = 0; y < ySize; y++) {
		for (x = 0; x < xSize; x++) {
			ARegion *reg = pRegionArrays[level]->GetRegion(x, y);
			if (reg) {
				reg->SetName(nex_name.Str());
				reg->type = R_NEXUS;
			}
		}
	}

	FinalSetup(pRegionArrays[level]);

	for (y = 0; y < ySize; y++) {
		for (int x = 0; x < xSize; x++) {
			ARegion *reg = pRegionArrays[level]->GetRegion(x, y);
			if (reg && Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
				reg->MakeStartingCity();
				if (Globals->GATES_EXIST) {
					numberofgates++;
				}
			}
		}
	}
}

void ARegionList::CreateSurfaceLevel(int level, int xSize, int ySize, char const *name)
{
	if (Globals->ICOSAHEDRAL_WORLD) {
		MakeIcosahedralRegions(level, xSize, ySize);
	} else {
		MakeRegions(level, xSize, ySize);
	}

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;
	int sea = Globals->OCEAN;
	if (Globals->SEA_LIMIT)
		sea = sea * (100 + 2 * Globals->SEA_LIMIT) / 100;
			
	MakeLand(pRegionArrays[level], sea, Globals->CONTINENT_SIZE);
	
	CleanUpWater(pRegionArrays[level]);

	SetupAnchors(pRegionArrays[level]);
	
	GrowTerrain(pRegionArrays[level], 0);
	
	AssignTypes(pRegionArrays[level]);

	SeverLandBridges(pRegionArrays[level]);

	if (Globals->LAKES) RemoveCoastalLakes(pRegionArrays[level]);
	
	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateIslandLevel(int level, int nPlayers, char const *name)
{
	int xSize, ySize;
	xSize = 20 + (nPlayers + 3) / 4 * 6 - 2;
	ySize = xSize;

	MakeRegions(level, xSize, ySize);

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;

	MakeCentralLand(pRegionArrays[level]);
	MakeIslands(pRegionArrays[level], nPlayers);
	RandomTerrain(pRegionArrays[level]);

	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateUnderworldLevel(int level, int xSize, int ySize,
		char const *name)
{
	if (Globals->ICOSAHEDRAL_WORLD) {
		MakeIcosahedralRegions(level, xSize, ySize);
	} else {
		MakeRegions(level, xSize, ySize);
	}

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERWORLD;

	SetRegTypes(pRegionArrays[level], R_NUM);

	SetupAnchors(pRegionArrays[level]);

	GrowTerrain(pRegionArrays[level], 1);

	AssignTypes(pRegionArrays[level]);

	MakeUWMaze(pRegionArrays[level]);

	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateUnderdeepLevel(int level, int xSize, int ySize,
		char const *name)
{
	if (Globals->ICOSAHEDRAL_WORLD) {
		MakeIcosahedralRegions(level, xSize, ySize);
	} else {
		MakeRegions(level, xSize, ySize);
	}

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERDEEP;

	SetRegTypes(pRegionArrays[level], R_NUM);

	SetupAnchors(pRegionArrays[level]);

	GrowTerrain(pRegionArrays[level], 1);

	AssignTypes(pRegionArrays[level]);

	MakeUWMaze(pRegionArrays[level]);

	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::MakeRegions(int level, int xSize, int ySize)
{
	Awrite("Making a level...");

	ARegionArray *arr = new ARegionArray(xSize, ySize);
	pRegionArrays[level] = arr;

	//
	// Make the regions themselves
	//
	int x, y;
	for (y = 0; y < ySize; y++) {
		for (x = 0; x < xSize; x++) {
			if (!((x + y) % 2)) {
				ARegion *reg = new ARegion;
				reg->SetLoc(x, y, level);
				reg->num = Num();

				//
				// Some initial values; these will get reset
				//
				reg->type = -1;
				reg->race = -1;  
				reg->wages = -1; 
				
				Add(reg);
				arr->SetRegion(x, y, reg);
			}
		}
	}

	SetupNeighbors(arr);

	Awrite("");
}

void ARegionList::SetupNeighbors(ARegionArray *pRegs)
{
	int x, y;
	for (x = 0; x < pRegs->x; x++) {
		for (y = 0; y < pRegs->y; y++) {
			ARegion *reg = pRegs->GetRegion(x, y);
			if (!reg) continue;
			NeighSetup(reg, pRegs);
		}
	}
}

void ARegionList::MakeIcosahedralRegions(int level, int xSize, int ySize)
{
	int scale, x2, y2;

	Awrite("Making an icosahedral level...");

	scale = xSize / 10;
	if (scale < 1) {
		Awrite("Can't create an icosahedral level with xSize < 10!");
		return;
	}
	if (ySize < scale * 10) {
		Awrite("ySize must be at least xSize!");
		return;
	}

	// Create the arrays as the specified size, as some code demands that
	// the RegionArray be multiples of 8 in each direction
	ARegionArray *arr = new ARegionArray(xSize, ySize);
	pRegionArrays[level] = arr;

	// but we'll only use up to multiples of 10, as that is required
	// by the geometry of the resulting icosahedron.  The best choice
	// would be to satisfy both criteria by choosing a multiple of 40,
	// of course (remember that sublevels are halved in size though)!
	xSize = scale * 10;
	ySize = xSize;

	//
	// Make the regions themselves
	//
	int x, y;
	for (y = 0; y < ySize; y++) {
		for (x = 0; x < xSize; x++) {
			if (!((x + y) % 2)) {
				// These cases remove all the hexes that are cut out to
				// make the world join up into a big icosahedron (d20).
				if (y < 2) {
					if (x)
						continue;
				}
				else if (y <= 3 * scale) {
					x2 = x % (2 * scale);
					if (y < 3 * x2 && y <= 3 * (2 * scale - x2))
						continue;
				}
				else if (y < 7 * scale - 1) {
					// Include all of this band
				}
				else if (y < 10 * scale - 2) {
					x2 = (x + 2 * scale + 1) % (2 * scale);
					y2 = 10 * scale - 1 - y;
					if (y2 < 3 * x2 && y2 <= 3 * (2 * scale - x2))
						continue;
				}
				else {
					if (x != 10 * scale - 1)
						continue;
				}

				ARegion *reg = new ARegion;
				reg->SetLoc(x, y, level);
				reg->num = Num();

				//
				// Some initial values; these will get reset
				//
				reg->type = -1;
				reg->race = -1; // 
				reg->wages = -1; // initially store: name
				reg->population = -1; // initially used as flag
				reg->elevation = -1;

				Add(reg);
				arr->SetRegion(x, y, reg);
			}
		}
	}

	SetupIcosahedralNeighbors(arr);

	Awrite("");
}

void ARegionList::SetupIcosahedralNeighbors(ARegionArray *pRegs)
{
	int x, y;

	for (x = 0; x < pRegs->x; x++) {
		for (y = 0; y < pRegs->y; y++) {
			ARegion *reg = pRegs->GetRegion(x, y);
			if (!reg) continue;
			IcosahedralNeighSetup(reg, pRegs);
		}
	}
}

void ARegionList::MakeLand(ARegionArray *pRegs, int percentOcean,
		int continentSize)
{
	int total, ocean;

	total = 0;
	for (int x=0; x < pRegs->x; x++)
		for (int y=0; y < pRegs->y; y++)
			if (pRegs->GetRegion(x, y))
				total++;
	ocean = total;

	Awrite("Making land");
	while (ocean > (total * percentOcean) / 100) {
		int sz = getrandom(continentSize);
		sz = sz * sz;

		int tempx = getrandom(pRegs->x);
		int yoff = pRegs->y / 40;
		int yband = pRegs->y / 2 - 2 * yoff;
		int tempy = (getrandom(yband)+yoff) * 2 + tempx % 2;

		ARegion *reg = pRegs->GetRegion(tempx, tempy);
		if (!reg) continue;
		ARegion *newreg = reg;
		ARegion *seareg = reg;
		
		// Archipelago or Continent?
		if (getrandom(100) < Globals->ARCHIPELAGO) {
			// Make an Archipelago:
			sz = sz / 5 + 1;
			int first = 1;
			int tries = 0;
			for (int i=0; i<sz; i++) {
				int direc = getrandom(NDIRS);
				newreg = reg->neighbors[direc];
				while (!newreg) {
					direc = getrandom(6);
					newreg = reg->neighbors[direc];
				}
				tries++;
				for (int m = 0; m < 2; m++) {
					seareg = newreg;
					newreg = seareg->neighbors[direc];
					if (!newreg) break;
				}
				if (!newreg) break;
				if (newreg) {
					seareg = newreg;
					newreg = seareg->neighbors[getrandom(NDIRS)];
					if (!newreg) break;
					// island start point (~3 regions away from last island)
					seareg = newreg;
					if (first) {
						seareg = reg;
						first = 0;
					}
					if (seareg->type == -1) {
						reg = seareg;
						tries = 0;
						reg->type = R_NUM;
						ocean--;
					} else {
						if (tries > 5) break;
						continue;
					}
					int growit = getrandom(20);
					int growth = 0;
					int growch = 2;
					// grow this island
					while (growit > growch) {
						growit = getrandom(20);
						tries = 0;
						int newdir = getrandom(NDIRS);
						while (direc == reg->GetRealDirComp(newdir))
							newdir = getrandom(NDIRS);
						newreg = reg->neighbors[newdir];
						while ((!newreg) && (tries < 36)) {
							while (direc == reg->GetRealDirComp(newdir))
								newdir = getrandom(NDIRS);
							newreg = reg->neighbors[newdir];
							tries++;
						}
						if (!newreg) continue;
						reg = newreg;
						tries = 0;
						if (reg->type == -1) {
							reg->type = R_NUM;
							growth++;
							if (growth > growch) growch = growth;
							ocean--;
						} else continue;
					}
				}
			}
		} else {
			// make a continent
			if (reg->type == -1) {
				reg->type = R_NUM;
				ocean--;
			}
			for (int i=0; i<sz; i++) {
				int dir = getrandom(NDIRS);
				if ((reg->yloc < yoff*2) && ((dir < 2) || (dir == NDIRS-1))
					&& (getrandom(4) < 3)) continue;
				if ((reg->yloc > (yband+yoff)*2) && ((dir < 5) && (dir > 1))
					&& (getrandom(4) < 3)) continue;				
				ARegion *newreg = reg->neighbors[dir];
				if (!newreg) break;
				int polecheck = 0;
				for (int v=0; v < NDIRS; v++) {
					ARegion *creg = newreg->neighbors[v];
					if (!creg) polecheck = 1;
				}
				if (polecheck) break;
				reg = newreg;
				if (reg->type == -1) {
					reg->type = R_NUM;
					ocean--;
				}
			}
		}
	}

	// At this point, go back through and set all the rest to ocean
	SetRegTypes(pRegs, R_OCEAN);
	Awrite("");
}

void ARegionList::MakeCentralLand(ARegionArray *pRegs)
{
	for (int i = 0; i < pRegs->x; i++) {
		for (int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if (!reg) continue;
			// Initialize region to ocean.
			reg->type = R_OCEAN;
			// If the region is close to the edges, it stays ocean
			if (i < 8 || i >= pRegs->x - 8 || j < 8 || j >= pRegs->y - 8)
				continue;
			// If the region is within 10 of the edges, it has a 50%
			// chance of staying ocean.
			if (i < 10 || i >= pRegs->x - 10 || j < 10 || j >= pRegs->y - 10) {
				if (getrandom(100) > 50) continue;
			}

			// Otherwise, set the region to land.
			reg->type = R_NUM;
		}
	}
}

void ARegionList::MakeIslands(ARegionArray *pArr, int nPlayers)
{
	// First, make the islands along the top.
	int i;
	int nRow = (nPlayers + 3) / 4;
	for (i = 0; i < nRow; i++)
		MakeOneIsland(pArr, 10 + i * 6, 2);
	// Next, along the left.
	nRow = (nPlayers + 2) / 4;
	for (i = 0; i < nRow; i++)
		MakeOneIsland(pArr, 2, 10 + i * 6);
	// The islands along the bottom.
	nRow = (nPlayers + 1) / 4;
	for (i = 0; i < nRow; i++)
		MakeOneIsland(pArr, 10 + i * 6, pArr->y - 6);
	// And the islands on the right.
	nRow = nPlayers / 4;
	for (i = 0; i < nRow; i++)
		MakeOneIsland(pArr, pArr->x - 6, 10 + i * 6);
}

void ARegionList::MakeOneIsland(ARegionArray *pRegs, int xx, int yy)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ARegion *pReg = pRegs->GetRegion(i + xx, j + yy);
			if (!pReg) continue;
			pReg->type = R_NUM;
		}
	}
}

void ARegionList::CleanUpWater(ARegionArray *pRegs)
{
	Awrite("Converting Scattered Water");
	int dotter = 0;
	for (int ctr = 0; ctr < Globals->SEA_LIMIT+1; ctr++) {
		for (int i = 0; i < pRegs->x; i++) {
			for (int j = 0; j < pRegs->y; j++) {
				ARegion *reg = pRegs->GetRegion(i, j);
				int remainocean = 0;
				if ((!reg) || (reg->type != R_OCEAN)) continue;
				for (int d = 0; d < NDIRS; d++) {
					remainocean += reg->CheckSea(d, Globals->SEA_LIMIT, remainocean);
				}
				if (dotter++%2000 == 0) Adot();
				if (remainocean > 0) continue;
				reg->wages = 0;
				if (getrandom(100) < Globals->LAKES) {
						reg->type = R_LAKE;
				} else reg->type = R_NUM;
			}
		}
	}
	Awrite("");
}

void ARegionList::RemoveCoastalLakes(ARegionArray *pRegs)
{
	Awrite("Removing coastal 'lakes'");
	for (int c = 0; c < 2; c++) {
		for (int i = 0; i < pRegs->x; i++) {
			for (int j = 0; j < pRegs->y; j++) {
				ARegion *reg = pRegs->GetRegion(i, j);
				if ((!reg) || (reg->type != R_LAKE)) continue;
				if (reg->IsCoastal() > 0) {
					reg->type = R_OCEAN;
					reg->wages = -1;
					Adot();
				} else if (reg->wages <= 0) { // name the Lake
					int wage1 = 0;
					int count1 = 0;
					int wage2 = 0;
					int count2 = 0;
					int temp = 0;
					for (int d = 0; d < NDIRS; d++) {
						ARegion *newregion = reg->neighbors[d];
						if (!newregion) continue;
						// name after neighboring lake regions preferrentially
						if ((newregion->wages > 0) &&
								(newregion->type == R_LAKE)) {
							count1 = 1;
							wage1 = newregion->wages;
							break;
						}
						if ((TerrainDefs[newregion->type].similar_type !=
									R_OCEAN) && (newregion->wages > -1)) {
							if (newregion->wages == wage1) count1++;
							else if (newregion->wages == wage2) count2++;
							else if (count2 == 0) {
								wage2 = newregion->wages;
								count2++;
							}
							if (count2 > count1) {
								temp = wage1;
								wage1 = wage2;
								wage2 = temp;
								int tmpin = count1;
								count1 = count2;
								count2 = tmpin;
							}
						}
					}
					if (count1 > 0) reg->wages = wage1;
				}
			}
		}
	}
	Awrite("");
}

void ARegionList::SeverLandBridges(ARegionArray *pRegs)
{
	Awrite("Severing land bridges");
	// mark land hexes to delete
	for (int i = 0; i < pRegs->x; i++) {
		for (int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if ((!reg) || (TerrainDefs[reg->type].similar_type == R_OCEAN))
				continue;
			if (reg->IsCoastal() != 4) continue;
			int tidych = Globals->SEVER_LAND_BRIDGES;
			for (int d = 0; d < NDIRS; d++) {
				ARegion *newregion = reg->neighbors[d];
				if ((!newregion) ||
						(TerrainDefs[newregion->type].similar_type == R_OCEAN))
					continue;
				if (newregion->IsCoastal() == 4) tidych = tidych * 2;
			}
			if (getrandom(100) < (tidych)) reg->wages = -2;
		}
	}
	// now change to ocean
	for (int i = 0; i < pRegs->x; i++) {
		for (int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if ((!reg) || (reg->wages > -2)) continue;
			reg->type = R_OCEAN;
			reg->wages = -1;
			Adot();
		}
	}
	Awrite("");
}

void ARegionList::SetRegTypes(ARegionArray *pRegs, int newType)
{
	for (int i = 0; i < pRegs->x; i++) {
		for (int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if (!reg) continue;
			if (reg->type == -1) reg->type = newType;
		}
	}
}

void ARegionList::SetupAnchors(ARegionArray *ta)
{
	// Now, setup the anchors
	Awrite("Setting up the anchors");
	int skip = 250;
	int f = 2;
	if (Globals->TERRAIN_GRANULARITY) {
		skip = Globals->TERRAIN_GRANULARITY;
		while (skip > 5) {
			f++;
			skip -= 5;
			if (skip < 1) skip = 1;
		}
		skip = 100 * ((skip+3) * f + 2) / (skip + f - 2);
	}
	int dotter = 0;
	for (int x=0; x<(ta->x)/f; x++) {
		for (int y=0; y<(ta->y)/(f*2); y++) {
			if (getrandom(1000) > skip) continue;
			ARegion *reg = 0;
			for (int i=0; i<4; i++) {
				int tempx = x * f + getrandom(f);
				int tempy = y * f * 2 + getrandom(f)*2 + tempx%2;
				reg = ta->GetRegion(tempx, tempy);
				if (!reg)
					continue;
				if (reg->type == R_NUM) {
					reg->type = GetRegType(reg);
					reg->population = 1;
					if (TerrainDefs[reg->type].similar_type != R_OCEAN)
						reg->wages = AGetName(0, reg);
					break;
				}
			}
			if (dotter++%30 == 0) Adot();
		}
	}

	Awrite("");
}

void ARegionList::GrowTerrain(ARegionArray *pArr, int growOcean)
{
	Awrite("Growing Terrain...");
	for (int j=0; j<30; j++) {
		int x, y;
		for (x = 0; x < pArr->x; x++) {
			for (y = 0; y < pArr->y; y++) {
				ARegion *reg = pArr->GetRegion(x, y);
				if (!reg) continue;
				reg->population = 1;
			}
		}
		for (x = 0; x < pArr->x; x++) {
			for (y = 0; y < pArr->y; y++) {
				ARegion *reg = pArr->GetRegion(x, y);
				if (!reg) continue;
				if ((j > 0) && (j < 21) && (getrandom(3) < 2)) continue;
				if (reg->type == R_NUM) {
				
					// Check for Lakes
					if (Globals->LAKES &&
						(getrandom(100) < (Globals->LAKES/10 + 1))) {
							reg->type = R_LAKE;
							break;
					}
					// Check for Odd Terrain
					if (getrandom(1000) < Globals->ODD_TERRAIN) {
						reg->type = GetRegType(reg);
						if (TerrainDefs[reg->type].similar_type != R_OCEAN)
							reg->wages = AGetName(0, reg);
						break;
					}
					

					int init = getrandom(6);
					for (int i=0; i<NDIRS; i++) {
						ARegion *t = reg->neighbors[(i+init) % NDIRS];
						if (t) {
							if (t->population < 1) continue;
							if (t->type != R_NUM &&
								(TerrainDefs[t->type].similar_type!=R_OCEAN ||
								 (growOcean && (t->type != R_LAKE)))) {
								if (j==0) t->population = 0;
								reg->population = 0;
								reg->race = t->type;
								reg->wages = t->wages;
								break;
							}
						}
					}
				}
			}
		}

		for (x = 0; x < pArr->x; x++) {
			for (y = 0; y < pArr->y; y++) {
				ARegion *reg = pArr->GetRegion(x, y);
				if (!reg) continue;
				if (reg->type == R_NUM && reg->race != -1)
					reg->type = reg->race;
			}
		}
	}
}

void ARegionList::RandomTerrain(ARegionArray *pArr)
{
	int x, y;
	for (x = 0; x < pArr->x; x++) {
		for (y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if (!reg) continue;

			if (reg->type == R_NUM) {
				int adjtype = 0;
				int adjname = -1;
				for (int d = 0; d < NDIRS; d++) {
					ARegion *newregion = reg->neighbors[d];
					if (!newregion) continue;
					if ((TerrainDefs[newregion->type].similar_type !=
								R_OCEAN) && (newregion->type != R_NUM) &&
							(newregion->wages > 0)) {
						adjtype = newregion->type;
						adjname = newregion->wages;
					}
				}
				if (adjtype && !Globals->CONQUEST_GAME) {
					reg->type = adjtype;
					reg->wages = adjname;
				} else {
					reg->type = GetRegType(reg);
					reg->wages = AGetName(0, reg);
				}
			}
		}
	}
}

void ARegionList::MakeUWMaze(ARegionArray *pArr)
{
	int x, y;

	for (x = 0; x < pArr->x; x++) {
		for (y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if (!reg) continue;

			for (int i=D_NORTH; i< NDIRS; i++) {
				int count = 0;
				for (int j=D_NORTH; j< NDIRS; j++)
					if (reg->neighbors[j]) count++;
				if (count <= 1) break;

				ARegion *n = reg->neighbors[i];
				if (n) {
					if (n->xloc < x || (n->xloc == x && n->yloc < y))
						continue;
					if (!CheckRegionExit(reg, n)) {
						count = 0;
						for (int k = D_NORTH; k<NDIRS; k++) {
							if (n->neighbors[k]) count++;
						}
						if (count <= 1) break;
						n->neighbors[reg->GetRealDirComp(i)] = 0;
						reg->neighbors[i] = 0;
					}
				}
			}
		}
	}
}

void ARegionList::AssignTypes(ARegionArray *pArr)
{
	// RandomTerrain() will set all of the un-set region types and names.
	RandomTerrain(pArr);
}

void ARegionList::UnsetRace(ARegionArray *pArr)
{
	int x, y;
	for (x = 0; x < pArr->x; x++) {
		for (y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if (!reg) continue;
			reg->race = - 1;
		}
	}
}

void ARegionList::RaceAnchors(ARegionArray *pArr)
{
	Awrite("Setting Race Anchors");
	UnsetRace(pArr);
	int x, y;
	int wigout = 0;
	for (x = 0; x < pArr->x; x++) {
		for (y = 0; y < pArr->y; y++) {
			// Anchor distribution: depends on GROW_RACES value
			int jiggle = 4 + 2 * Globals->GROW_RACES;
			if ((y + ((x % 2) * jiggle/2)) % jiggle > 1) continue;
			int xoff = x + 2 - getrandom(3) - getrandom(3);
			ARegion *reg = pArr->GetRegion(xoff, y);
			if (!reg) continue;
			
			if ((reg->type == R_LAKE) && (!Globals->LAKESIDE_IS_COASTAL))
				continue;
			if (TerrainDefs[reg->type].flags & TerrainType::BARREN) continue;
			
			reg->race = -1;
			wigout = 0; // reset sanity
			
			if (TerrainDefs[reg->type].similar_type == R_OCEAN) {
				// setup near coastal race here
				int d = getrandom(NDIRS);
				int ctr = 0;
				ARegion *nreg = reg->neighbors[d];
				if (!nreg) continue;
				while((ctr++ < 20) && (reg->race == -1)) {
					if (TerrainDefs[nreg->type].similar_type != R_OCEAN) {
						int rnum =
							sizeof(TerrainDefs[nreg->type].coastal_races) /
							sizeof(int);
						
						while ( reg->race == -1 || 
								(ItemDefs[reg->race].flags & ItemType::DISABLED)) {
							reg->race = 
								TerrainDefs[nreg->type].coastal_races[getrandom(rnum)];
							if (++wigout > 100) break;
						}
					} else {
						int dir = getrandom(NDIRS);
						if (d == nreg->GetRealDirComp(dir)) continue;
						if (!(nreg->neighbors[dir])) continue;
						nreg = nreg->neighbors[dir];
					}
				}
			} else {
				// setup noncoastal race here
				int rnum = sizeof(TerrainDefs[reg->type].races)/sizeof(int);
				
				while ( reg->race == -1 || 
						(ItemDefs[reg->race].flags & ItemType::DISABLED)) {
					reg->race = TerrainDefs[reg->type].races[getrandom(rnum)];
					if (++wigout > 100) break;
				}
			}
			
			/* leave out this sort of check for the moment
			if (wigout > 100) {
				// do something!
				Awrite("There is a problem with the races in the ");
				Awrite(TerrainDefs[reg->type].name);
				Awrite(" region type");
			}
			*/
			
			if (reg->race == -1) {
				cout << "Hey! No race anchor got assigned to the " 
					<< TerrainDefs[reg->type].name 
					<< " at " << x << "," << y << "\n";
			}
		}
	}
}

void ARegionList::GrowRaces(ARegionArray *pArr)
{
	Awrite("Growing Races");
	RaceAnchors(pArr);
	int a, x, y;
	for (a = 0; a < 25; a++) {
		for (x = 0; x < pArr->x; x++) {
			for (y = 0; y < pArr->y; y++) {
				ARegion *reg = pArr->GetRegion(x, y);
				if ((!reg) || (reg->race == -1)) continue;

				for (int dir = 0; dir < NDIRS; dir++) {
					ARegion *nreg = reg->neighbors[dir];
					if ((!nreg) || (nreg->race != -1)) continue;
					int iscoastal = 0;
					int cnum = sizeof(TerrainDefs[reg->type].coastal_races) /
						sizeof(int);
					for (int i=0; i<cnum; i++) {
						if (reg->race ==
								TerrainDefs[reg->type].coastal_races[i])
							iscoastal = 1;
					}
					// Only coastal races may pass from sea to land
					if ((TerrainDefs[nreg->type].similar_type == R_OCEAN) &&
							(!iscoastal))
						continue;

					int ch = getrandom(5);
					if (iscoastal) {
						if (TerrainDefs[nreg->type].similar_type == R_OCEAN)
							ch += 2;
					} else {
						ManType *mt = FindRace(ItemDefs[reg->race].abr);
						if (mt->terrain==TerrainDefs[nreg->type].similar_type)
							ch += 2;
						int rnum = sizeof(TerrainDefs[nreg->type].races) /
							sizeof(int);
						for (int i=0; i<rnum; i++) {
							if (TerrainDefs[nreg->type].races[i] == reg->race)
								ch++;
						}
					}
					if (ch > 3) nreg->race = reg->race;
				}
			}
		}
	}
}

void ARegionList::FinalSetup(ARegionArray *pArr)
{
	int x, y;
	for (x = 0; x < pArr->x; x++) {
		for (y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if (!reg) continue;

			if ((TerrainDefs[reg->type].similar_type == R_OCEAN) &&
					(reg->type != R_LAKE)) {
				if (pArr->levelType == ARegionArray::LEVEL_UNDERWORLD)
					reg->SetName("The Undersea");
				else if (pArr->levelType == ARegionArray::LEVEL_UNDERDEEP)
					reg->SetName("The Deep Undersea");
				else {
					AString ocean_name = Globals->WORLD_NAME;
					ocean_name += " Ocean";
					reg->SetName(ocean_name.Str());
				}
			} else {
				if (reg->wages == -1) reg->SetName("Unnamed");
				else if (reg->wages != -2)
					reg->SetName(AGetNameString(reg->wages));
				else
					reg->wages = -1;
			}

			reg->Setup();
		}
	}
}

void ARegionList::MakeShaft(ARegion *reg, ARegionArray *pFrom,
		ARegionArray *pTo)
{
	if (TerrainDefs[reg->type].similar_type == R_OCEAN) return;

	int tempx = reg->xloc * pTo->x / pFrom->x +
		getrandom(pTo->x / pFrom->x);
	int tempy = reg->yloc * pTo->y / pFrom->y +
		getrandom(pTo->y / pFrom->y);
	//
	// Make sure we get a valid region.
	//
	tempy += (tempx + tempy) % 2;

	ARegion *temp = pTo->GetRegion(tempx, tempy);
	if (!temp)
		return;
	if (TerrainDefs[temp->type].similar_type == R_OCEAN) return;

	Object *o = new Object(reg);
	o->num = reg->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = temp->num;
	reg->objects.Add(o);

	o = new Object(reg);
	o->num = temp->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = reg->num;
	temp->objects.Add(o);
}

void ARegionList::MakeShaftLinks(int levelFrom, int levelTo, int odds)
{
	ARegionArray *pFrom = pRegionArrays[levelFrom];
	ARegionArray *pTo = pRegionArrays[levelTo];

	int x, y;
	for (x = 0; x < pFrom->x; x++) {
		for (y = 0; y < pFrom->y; y++) {
			ARegion *reg = pFrom->GetRegion(x, y);
			if (!reg) continue;

			if (getrandom(odds) != 0) continue;

			MakeShaft(reg, pFrom, pTo);
		}
	}
}

void ARegionList::SetACNeighbors(int levelSrc, int levelTo, int maxX, int maxY)
{
	ARegionArray *ar = GetRegionArray(levelSrc);

	for (int x = 0; x < ar->x; x++) {
		for (int y = 0; y < ar->y; y++) {
			ARegion *AC = ar->GetRegion(x, y);
			if (!AC) continue;
			if (Globals->START_CITIES_EXIST) {
				for (int i=0; i<NDIRS; i++) {
					if (AC->neighbors[i]) continue;
					ARegion *pReg = GetStartingCity(AC, i, levelTo, maxX, maxY);
					if (!pReg) continue;
					AC->neighbors[i] = pReg;
					pReg->MakeStartingCity();
					if (Globals->GATES_EXIST) {
						numberofgates++;
					}
				}
			}
			else
			{
				// If we don't have starting cities, then put portals
				// from the nexus to a variety of terrain types.
				// These will transport the user to a randomly
				// selected region of the chosen terrain type.
				ARegionArray *to = GetRegionArray(levelTo);
				for (int type = R_PLAIN; type <= R_TUNDRA; type++) {
					int found = 0;
					for (int x2 = 0; !found && x2 < maxX; x2++)
						for (int y2 = 0; !found && y2 < maxY; y2++) {
							ARegion *reg = to->GetRegion(x2, y2);
							if (!reg)
								continue;
							if (reg->type == type) {
								found = 1;
								Object *o = new Object(AC);
								o->num = AC->buildingseq++;
								o->name = new AString(AString("Gateway to ") +
									TerrainDefs[type].name + " [" + o->num + "]");
								o->type = O_GATEWAY;
								o->incomplete = 0;
								o->inner = reg->num;
								AC->objects.Add(o);
							}
						}
				}
			}
		}
	}
}

void ARegionList::InitSetupGates(int level)
{

	if (!Globals->GATES_EXIST) return;

	ARegionArray *pArr = pRegionArrays[level];

	int i, j, k;
	for (i=0; i<pArr->x / 8; i++) {
		for (j=0; j<pArr->y / 16; j++) {
			for (k=0; k<5; k++) {
				int tempx = i*8 + getrandom(8);
				int tempy = j*16 + getrandom(8)*2 + tempx%2;
				ARegion *temp = pArr->GetRegion(tempx, tempy);
				if (temp && TerrainDefs[temp->type].similar_type != R_OCEAN &&
						temp->gate != -1) {
					numberofgates++;
					temp->gate = -1;
					break;
				}
			}
		}
	}
}

void ARegionList::FixUnconnectedRegions()
{
}

void ARegionList::FinalSetupGates()
{
	int ngates, log10, *used, i;

	if (!Globals->GATES_EXIST) return;

	ngates = numberofgates;

	if (Globals->DISPERSE_GATE_NUMBERS) {
		log10 = 0;
		while (ngates > 0) {
			ngates /= 10;
			log10++;
		}
		ngates = 10;
		while (log10 > 0) {
			ngates *= 10;
			log10--;
		}
	}

	used = new int[ngates];

	for (i = 0; i < ngates; i++)
		used[i] = 0;

	forlist(this) {
		ARegion *r = (ARegion *) elem;

		if (r->gate == -1) {
			int index = getrandom(ngates);
			while (used[index]) {
				if (Globals->DISPERSE_GATE_NUMBERS) {
					index = getrandom(ngates);
				} else {
					index++;
					index = index % ngates;
				}
			}
			r->gate = index+1;
			used[index] = 1;
			// setting up gatemonth
			r->gatemonth = getrandom(12);;
		}
	}
	delete used;
}

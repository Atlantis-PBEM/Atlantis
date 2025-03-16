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
#include "../game.h"
#include "../gamedata.h"

/// For unit testing, just do nothing.
int ARegion::CheckSea(int dir, int range, int remainocean) { return 1; }

/// For unit testing just do nothing.
void ARegionList::CreateAbyssLevel(int level, char const *name) { }

/// For unit testing do nothing.
void ARegionList::CreateNexusLevel(int level, int xSize, int ySize, char const *name) { }

void ARegionList::CreateSurfaceLevel(int level, int xSize, int ySize, char const *name) {
	// For the test world, make a very very small 2x2 world with 1 town in the plains
	// and 1 hex of each of forest, mountains, and desert.
	MakeRegions(level, xSize, ySize);
	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;
	AssignTypes(pRegionArrays[level]);
	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateIslandLevel(int level, int nPlayers, char const *name) { }
void ARegionList::CreateIslandRingLevel(int level, int xSize, int ySize, char const *name) {}
void ARegionList::CreateUnderworldRingLevel(int level, int xSize, int ySize, char const *name) {}

void ARegionList::CreateUnderworldLevel(int level, int xSize, int ySize, char const *name) {
	MakeRegions(level, xSize, ySize);
	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERWORLD;
	AssignTypes(pRegionArrays[level]);
	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateUnderdeepLevel(int level, int xSize, int ySize, char const *name) { }

void ARegionList::MakeRegions(int level, int xSize, int ySize)
{
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
				reg->num = regions.size();

				//
				// Some initial values; these will get reset
				//
				reg->type = -1;
				reg->race = -1;
				reg->wages = -1;

				reg->level = arr;
				regions.push_back(reg);
				arr->SetRegion(x, y, reg);
				Adot();
			}
		}
	}

	SetupNeighbors(arr);
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

void ARegionList::MakeIcosahedralRegions(int level, int xSize, int ySize) { }

void ARegionList::SetupIcosahedralNeighbors(ARegionArray *pRegs) { }

void ARegionList::MakeLand(ARegionArray *pRegs, int percentOcean, int continentSize) { }

void ARegionList::MakeRingLand(ARegionArray *pReg, int minDistance, int maxDistance) { }

void ARegionList::MakeCentralLand(ARegionArray *pRegs) { }

void ARegionList::MakeIslands(ARegionArray *pArr, int nPlayers) { }

void ARegionList::MakeOneIsland(ARegionArray *pRegs, int xx, int yy) { }

void ARegionList::CleanUpWater(ARegionArray *pRegs) { }

void ARegionList::RemoveCoastalLakes(ARegionArray *pRegs) { }

void ARegionList::SeverLandBridges(ARegionArray *pRegs) { }

void ARegionList::SetRegTypes(ARegionArray *pRegs, int newType) { }

void ARegionList::SetupAnchors(ARegionArray *ta) { }

void ARegionList::GrowTerrain(ARegionArray *pArr, int growOcean) { }

void ARegionList::RandomTerrain(ARegionArray *pArr) { }

void ARegionList::MakeUWMaze(ARegionArray *pArr) { }

void ARegionList::AssignTypes(ARegionArray *pArr) {
	// we have a fixed world, so just assign the types.
	int terrains[] = { R_PLAIN, R_FOREST, R_MOUNTAIN, R_DESERT };
	int uwterrains[] = { R_CAVERN };
	int loc = 0;

	int *t_array = (pArr->levelType == ARegionArray::LEVEL_UNDERWORLD) ? uwterrains : terrains;

	for (auto x = 0; x < pArr->x; x++) {
		for (auto y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if (!reg) continue;

			reg->type = t_array[loc++];
			reg->race = TerrainDefs[reg->type].races[0];
		}
	}
}

void ARegionList::UnsetRace(ARegionArray *pArr) { }

void ARegionList::RaceAnchors(ARegionArray *pArr) { }

void ARegionList::GrowRaces(ARegionArray *pArr) { }

void ARegionList::FinalSetup(ARegionArray *pArr) {
	for (auto x = 0; x < pArr->x; x++) {
		for (auto y = 0; y < pArr->y; y++) {
			ARegion *reg = pArr->GetRegion(x, y);
			if (!reg) continue;
			reg->SetName(AGetNameString(0));
			reg->Setup();
		}
	}
 }

void ARegionList::MakeShaft(ARegion *reg, ARegionArray *pFrom, ARegionArray *pTo) {
	ARegion *toReg = pTo->GetRegion(0, 0);
	if (!toReg) return;

	Object *o = new Object(reg);
	o->num = reg->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = toReg->num;
	reg->objects.push_back(o);

	o = new Object(toReg);
	o->num = toReg->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = reg->num;
	toReg->objects.push_back(o);
}

void ARegionList::MakeShaftLinks(int levelFrom, int levelTo, int odds) {
	ARegionArray *pFrom = pRegionArrays[levelFrom];
	ARegionArray *pTo = pRegionArrays[levelTo];

	if (!pFrom || !pTo) return;
	// we are ignoring the odds and always creating the shaft
	MakeShaft(pFrom->GetRegion(0, 0), pFrom, pTo);
}

void ARegionList::SetACNeighbors(int levelSrc, int levelTo, int maxX, int maxY) { }

void ARegionList::InitSetupGates(int level) { }

void ARegionList::FixUnconnectedRegions() { }

void ARegionList::FinalSetupGates() { }

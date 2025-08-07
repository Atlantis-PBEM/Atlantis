#include <stdio.h>
#include <string.h>
#include "../game.h"
#include "../gamedata.h"

/// For unit testing, just do nothing.
int ARegion::CheckSea(int dir, int range, int remainocean) { return 1; }

/// For unit testing just do nothing.
void ARegionList::create_abyss_level(int level, const std::string& name) { }

/// For unit testing do nothing.
void ARegionList::create_nexus_level(int level, int xSize, int ySize, const std::string& name) { }

void ARegionList::create_surface_level(int level, int xSize, int ySize, const std::string& name) {
    // For the test world, make a very very small 2x2 world with 1 town in the plains
    // and 1 hex of each of forest, mountains, and desert.
    MakeRegions(level, xSize, ySize);
    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;
    AssignTypes(pRegionArrays[level]);
    FinalSetup(pRegionArrays[level]);
}

void ARegionList::create_island_level(int level, int nPlayers, const std::string& name) { }
void ARegionList::create_island_ring_level(int level, int xSize, int ySize, const std::string& name) {}
void ARegionList::create_underworld_ring_level(int level, int xSize, int ySize, const std::string& name) {}

void ARegionList::create_underworld_level(int level, int xSize, int ySize, const std::string& name) {
    MakeRegions(level, xSize, ySize);
    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERWORLD;
    AssignTypes(pRegionArrays[level]);
    FinalSetup(pRegionArrays[level]);
}

void ARegionList::create_underdeep_level(int level, int xSize, int ySize, const std::string& name) { }

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
            reg->set_name(AGetNameString(0));
            reg->Setup();
        }
    }
 }

void ARegionList::MakeShaft(ARegion *reg, ARegionArray *pFrom, ARegionArray *pTo) {
    ARegion *toReg = pTo->GetRegion(0, 0);
    if (!toReg) return;

    Object *o = new Object(reg);
    o->num = reg->buildingseq++;
    o->set_name("Shaft");
    o->type = O_SHAFT;
    o->incomplete = 0;
    o->inner = toReg->num;
    reg->objects.push_back(o);

    o = new Object(toReg);
    o->num = toReg->buildingseq++;
    o->set_name("Shaft");
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

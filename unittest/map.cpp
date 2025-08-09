#include <stdio.h>
#include <string.h>
#include "../game.h"
#include "../gamedata.h"

/// For unit testing, just do nothing.
int ARegion::CheckSea(int, int, int) { return 1; }

/// For unit testing just do nothing.
void ARegionList::create_abyss_level(int, const std::string&) { }

/// For unit testing do nothing.
void ARegionList::create_nexus_level(int, int, int, const std::string&) { }

void ARegionList::create_surface_level(int level, int xSize, int ySize, const std::string& name) {
    // For the test world, make a very very small 2x2 world with 1 town in the plains
    // and 1 hex of each of forest, mountains, and desert.
    MakeRegions(level, xSize, ySize);
    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;
    AssignTypes(pRegionArrays[level]);
    FinalSetup(pRegionArrays[level]);
}

void ARegionList::create_island_level(int, int, const std::string&) { }
void ARegionList::create_island_ring_level(int, int, int, const std::string&) { }
void ARegionList::create_underworld_ring_level(int, int, int, const std::string&) { }

void ARegionList::create_underworld_level(int level, int xSize, int ySize, const std::string& name) {
    MakeRegions(level, xSize, ySize);
    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERWORLD;
    AssignTypes(pRegionArrays[level]);
    FinalSetup(pRegionArrays[level]);
}

void ARegionList::create_underdeep_level(int, int, int, const std::string&) { }

void ARegionList::MakeLand(ARegionArray *, int, int) { }

void ARegionList::MakeRingLand(ARegionArray *, int, int) { }

void ARegionList::MakeCentralLand(ARegionArray *) { }

void ARegionList::MakeIslands(ARegionArray *, int) { }

void ARegionList::MakeOneIsland(ARegionArray *, int, int) { }

void ARegionList::CleanUpWater(ARegionArray *) { }

void ARegionList::RemoveCoastalLakes(ARegionArray *) { }

void ARegionList::SeverLandBridges(ARegionArray *) { }

void ARegionList::SetRegTypes(ARegionArray *, int) { }

void ARegionList::SetupAnchors(ARegionArray *) { }

void ARegionList::GrowTerrain(ARegionArray *, int) { }

void ARegionList::RandomTerrain(ARegionArray *) { }

void ARegionList::MakeUWMaze(ARegionArray *) { }

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

void ARegionList::UnsetRace(ARegionArray *) { }

void ARegionList::RaceAnchors(ARegionArray *) { }

void ARegionList::GrowRaces(ARegionArray *) { }

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

void ARegionList::MakeShaft(ARegion *reg, ARegionArray *, ARegionArray *pTo) {
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

void ARegionList::MakeShaftLinks(int levelFrom, int levelTo, int) {
    ARegionArray *pFrom = pRegionArrays[levelFrom];
    ARegionArray *pTo = pRegionArrays[levelTo];

    if (!pFrom || !pTo) return;
    // we are ignoring the odds and always creating the shaft
    MakeShaft(pFrom->GetRegion(0, 0), pFrom, pTo);
}

void ARegionList::SetACNeighbors(int, int, int, int) { }

void ARegionList::InitSetupGates(int) { }

void ARegionList::FixUnconnectedRegions() { }

void ARegionList::FinalSetupGates() { }

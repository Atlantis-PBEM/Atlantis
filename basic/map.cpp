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


void ARegionList::create_abyss_level(int level, const std::string& name)
{
    MakeRegions(level, 4, 4);
    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_NEXUS;

    ARegion *reg = NULL;
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            reg = pRegionArrays[level]->GetRegion(x, y);
            if (!reg) continue;
            reg->set_name("Abyssal Plains");
            reg->type = R_DESERT;
            reg->wages = -2;
        }
    }

    int tempx, tempy;
    if (Globals->GATES_EXIST) {
        int gateset = 0;
        do {
            tempx = rng::get_random(4);
            tempy = rng::get_random(4);
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
        tempx = rng::get_random(4);
        tempy = rng::get_random(4);
        lair = pRegionArrays[level]->GetRegion(tempx, tempy);
    } while(!lair || lair == reg);
    Object *o = new Object(lair);
    o->num = lair->buildingseq++;
    o->set_name(ObjectDefs[O_BKEEP].name);
    o->type = O_BKEEP;
    o->incomplete = 0;
    o->inner = -1;
    lair->objects.push_back(o);
}


void ARegionList::create_nexus_level(int level, int xSize, int ySize, const std::string& name)
{
    MakeRegions(level, xSize, ySize);
    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_NEXUS;

    std::string nex_name = Globals->WORLD_NAME;
    nex_name += " Nexus";

    int x, y;
    for (y = 0; y < ySize; y++) {
        for (x = 0; x < xSize; x++) {
            ARegion *reg = pRegionArrays[level]->GetRegion(x, y);
            if (reg) {
                reg->set_name(nex_name);
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

void ARegionList::create_surface_level(int level, int xSize, int ySize, const std::string& name)
{
    if (Globals->ICOSAHEDRAL_WORLD) {
        MakeIcosahedralRegions(level, xSize, ySize);
    } else {
        MakeRegions(level, xSize, ySize);
    }

    pRegionArrays[level]->set_name(std::string(name));
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

void ARegionList::create_island_level(int level, int nPlayers, const std::string& name)
{
    int xSize, ySize;
    xSize = 20 + (nPlayers + 3) / 4 * 6 - 2;
    ySize = xSize;

    MakeRegions(level, xSize, ySize);

    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;

    MakeCentralLand(pRegionArrays[level]);
    MakeIslands(pRegionArrays[level], nPlayers);
    RandomTerrain(pRegionArrays[level]);

    if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

    FinalSetup(pRegionArrays[level]);
}

void ARegionList::create_island_ring_level(int, int, int, const std::string&)
{
    throw "create_island_ring_level not implemented for this game ruleset";
}

void ARegionList::create_underworld_ring_level(int, int, int, const std::string&)
{
    throw "create_underworld_ring_level not implemented for this game ruleset";
}

void ARegionList::create_underworld_level(int level, int xSize, int ySize, const std::string& name)
{
    if (Globals->ICOSAHEDRAL_WORLD) {
        MakeIcosahedralRegions(level, xSize, ySize);
    } else {
        MakeRegions(level, xSize, ySize);
    }

    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERWORLD;

    SetRegTypes(pRegionArrays[level], R_NUM);

    SetupAnchors(pRegionArrays[level]);

    GrowTerrain(pRegionArrays[level], 1);

    AssignTypes(pRegionArrays[level]);

    MakeUWMaze(pRegionArrays[level]);

    if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

    FinalSetup(pRegionArrays[level]);
}

void ARegionList::create_underdeep_level(int level, int xSize, int ySize, const std::string& name)
{
    if (Globals->ICOSAHEDRAL_WORLD) {
        MakeIcosahedralRegions(level, xSize, ySize);
    } else {
        MakeRegions(level, xSize, ySize);
    }

    pRegionArrays[level]->set_name(name);
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
    logger::write("Making a level...");

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
            }
        }
    }

    SetupNeighbors(arr);

    logger::write("");
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

    logger::write("Making an icosahedral level...");

    scale = xSize / 10;
    if (scale < 1) {
        logger::write("Can't create an icosahedral level with xSize < 10!");
        return;
    }
    if (ySize < scale * 10) {
        logger::write("ySize must be at least xSize!");
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
                reg->num = regions.size();

                //
                // Some initial values; these will get reset
                //
                reg->type = -1;
                reg->race = -1; //
                reg->wages = -1; // initially store: name
                reg->population = -1; // initially used as flag

                regions.push_back(reg);
                arr->SetRegion(x, y, reg);
            }
        }
    }

    SetupIcosahedralNeighbors(arr);

    logger::write("");
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

    logger::write("Making land");
    while (ocean > (total * percentOcean) / 100) {
        int sz = rng::get_random(continentSize);
        sz = sz * sz;

        int tempx = rng::get_random(pRegs->x);
        int yoff = pRegs->y / 40;
        int yband = pRegs->y / 2 - 2 * yoff;
        int tempy = (rng::get_random(yband)+yoff) * 2 + tempx % 2;

        ARegion *reg = pRegs->GetRegion(tempx, tempy);
        if (!reg) continue;
        ARegion *newreg = reg;
        ARegion *seareg = reg;

        // Archipelago or Continent?
        if (rng::get_random(100) < Globals->ARCHIPELAGO) {
            // Make an Archipelago:
            sz = sz / 5 + 1;
            int first = 1;
            int tries = 0;
            for (int i=0; i<sz; i++) {
                int direc = rng::get_random(NDIRS);
                newreg = reg->neighbors[direc];
                while (!newreg) {
                    direc = rng::get_random(6);
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
                    newreg = seareg->neighbors[rng::get_random(NDIRS)];
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
                    int growit = rng::get_random(20);
                    int growth = 0;
                    int growch = 2;
                    // grow this island
                    while (growit > growch) {
                        growit = rng::get_random(20);
                        tries = 0;
                        int newdir = rng::get_random(NDIRS);
                        while (direc == reg->GetRealDirComp(newdir))
                            newdir = rng::get_random(NDIRS);
                        newreg = reg->neighbors[newdir];
                        while ((!newreg) && (tries < 36)) {
                            while (direc == reg->GetRealDirComp(newdir))
                                newdir = rng::get_random(NDIRS);
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
                int dir = rng::get_random(NDIRS);
                if ((reg->yloc < yoff*2) && ((dir < 2) || (dir == (NDIRS-1)))
                    && (rng::get_random(4) < 3)) continue;
                if ((reg->yloc > (yband+yoff)*2) && ((dir < 5) && (dir > 1))
                    && (rng::get_random(4) < 3)) continue;
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
    logger::write("");
}

void ARegionList::MakeRingLand(ARegionArray *, int, int) { }

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
                if (rng::get_random(100) > 50) continue;
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
    logger::write("Converting Scattered Water");
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
                if (dotter++%2000 == 0) logger::dot();
                if (remainocean > 0) continue;
                reg->wages = 0;
                if (rng::get_random(100) < Globals->LAKES) {
                        reg->type = R_LAKE;
                } else reg->type = R_NUM;
            }
        }
    }
    logger::write("");
}

void ARegionList::RemoveCoastalLakes(ARegionArray *pRegs)
{
    logger::write("Removing coastal 'lakes'");
    for (int c = 0; c < 2; c++) {
        for (int i = 0; i < pRegs->x; i++) {
            for (int j = 0; j < pRegs->y; j++) {
                ARegion *reg = pRegs->GetRegion(i, j);
                if ((!reg) || (reg->type != R_LAKE)) continue;
                if (reg->IsCoastal() > 0) {
                    reg->type = R_OCEAN;
                    reg->wages = -1;
                    logger::dot();
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
    logger::write("");
}

void ARegionList::SeverLandBridges(ARegionArray *pRegs)
{
    logger::write("Severing land bridges");
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
            if (rng::get_random(100) < (tidych)) reg->wages = -2;
        }
    }
    // now change to ocean
    for (int i = 0; i < pRegs->x; i++) {
        for (int j = 0; j < pRegs->y; j++) {
            ARegion *reg = pRegs->GetRegion(i, j);
            if ((!reg) || (reg->wages > -2)) continue;
            reg->type = R_OCEAN;
            reg->wages = -1;
            logger::dot();
        }
    }
    logger::write("");
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
    logger::write("Setting up the anchors");
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
            if (rng::get_random(1000) > skip) continue;
            ARegion *reg = 0;
            for (int i=0; i<4; i++) {
                int tempx = x * f + rng::get_random(f);
                int tempy = y * f * 2 + rng::get_random(f)*2 + tempx%2;
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
            if (dotter++%30 == 0) logger::dot();
        }
    }

    logger::write("");
}

void ARegionList::GrowTerrain(ARegionArray *pArr, int growOcean)
{
    logger::write("Growing Terrain...");
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
                if ((j > 0) && (j < 21) && (rng::get_random(3) < 2)) continue;
                if (reg->type == R_NUM) {

                    // Check for Lakes
                    if (Globals->LAKES &&
                        (rng::get_random(100) < (Globals->LAKES/10 + 1))) {
                            reg->type = R_LAKE;
                            break;
                    }
                    // Check for Odd Terrain
                    if (rng::get_random(1000) < Globals->ODD_TERRAIN) {
                        reg->type = GetRegType(reg);
                        if (TerrainDefs[reg->type].similar_type != R_OCEAN)
                            reg->wages = AGetName(0, reg);
                        break;
                    }


                    int init = rng::get_random(6);
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

            for (int i=D_NORTH; i != NDIRS; i++) {
                int count = 0;
                for (int j=D_NORTH; j != NDIRS; j++)
                    if (reg->neighbors[j]) count++;
                if (count <= 1) break;

                ARegion *n = reg->neighbors[i];
                if (n) {
                    if (n->xloc < x || (n->xloc == x && n->yloc < y))
                        continue;
                    if (!CheckRegionExit(reg, n)) {
                        count = 0;
                        for (int k = D_NORTH; k != NDIRS; ++k) {
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
    logger::write("Setting Race Anchors");
    UnsetRace(pArr);
    int x, y;
    int wigout = 0;
    for (x = 0; x < pArr->x; x++) {
        for (y = 0; y < pArr->y; y++) {
            // Anchor distribution: depends on GROW_RACES value
            int jiggle = 4 + 2 * Globals->GROW_RACES;
            if ((y + ((x % 2) * jiggle/2)) % jiggle > 1) continue;
            int xoff = x + 2 - rng::get_random(3) - rng::get_random(3);
            ARegion *reg = pArr->GetRegion(xoff, y);
            if (!reg) continue;

            if ((reg->type == R_LAKE) && (!Globals->LAKESIDE_IS_COASTAL)) continue;
            if (TerrainDefs[reg->type].flags & TerrainType::BARREN) continue;

            reg->race = -1;
            wigout = 0; // reset sanity

            if (TerrainDefs[reg->type].similar_type == R_OCEAN) {
                // setup near coastal race here
                int d = rng::get_random(NDIRS);
                int ctr = 0;
                ARegion *nreg = reg->neighbors[d];
                if (!nreg) continue;
                while((ctr++ < 20) && (reg->race == -1)) {
                    if (TerrainDefs[nreg->type].similar_type != R_OCEAN) {
                        int rnum = sizeof(TerrainDefs[nreg->type].coastal_races) /
                            sizeof(TerrainDefs[nreg->type].coastal_races[0]);

                        while ( reg->race == -1 || (ItemDefs[reg->race].flags & ItemType::DISABLED)) {
                            reg->race = TerrainDefs[nreg->type].coastal_races[rng::get_random(rnum)];
                            if (++wigout > 100) break;
                        }
                    } else {
                        int dir = rng::get_random(NDIRS);
                        if (d == nreg->GetRealDirComp(dir)) continue;
                        if (!(nreg->neighbors[dir])) continue;
                        nreg = nreg->neighbors[dir];
                    }
                }
            } else {
                // setup noncoastal race here
                int rnum = sizeof(TerrainDefs[reg->type].races)/sizeof(TerrainDefs[reg->type].races[0]);

                while ( reg->race == -1 ||
                        (ItemDefs[reg->race].flags & ItemType::DISABLED)) {
                    reg->race = TerrainDefs[reg->type].races[rng::get_random(rnum)];
                    if (++wigout > 100) break;
                }
            }

            /* leave out this sort of check for the moment
            if (wigout > 100) {
                // do something!
                logger::write("There is a problem with the races in the ");
                logger::write(TerrainDefs[reg->type].name);
                logger::write(" region type");
            }
            */

            if (reg->race == -1) {
                logger::write(
                    "Hey! No race anchor got assigned to the " + TerrainDefs[reg->type].name + " at " +
                    std::to_string(x) + "," + std::to_string(y)
                );
            }
        }
    }
}

void ARegionList::GrowRaces(ARegionArray *pArr)
{
    logger::write("Growing Races");
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
                        sizeof(TerrainDefs[reg->type].coastal_races[0]);
                    for (int i = 0; i < cnum; i++) {
                        if (reg->race == TerrainDefs[reg->type].coastal_races[i])
                            iscoastal = 1;
                    }
                    // Only coastal races may pass from sea to land
                    if ((TerrainDefs[nreg->type].similar_type == R_OCEAN) && (!iscoastal)) continue;

                    int ch = rng::get_random(5);
                    if (iscoastal) {
                        if (TerrainDefs[nreg->type].similar_type == R_OCEAN)
                            ch += 2;
                    } else {
                        auto mt = find_race(ItemDefs[reg->race].abr)->get();
                        if (mt.terrain==TerrainDefs[nreg->type].similar_type) ch += 2;
                        int rnum = sizeof(TerrainDefs[nreg->type].races) / sizeof(TerrainDefs[nreg->type].races[0]);
                        for (int i=0; i<rnum; i++) {
                            if (TerrainDefs[nreg->type].races[i] == reg->race) ch++;
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
                    reg->set_name("The Undersea");
                else if (pArr->levelType == ARegionArray::LEVEL_UNDERDEEP)
                    reg->set_name("The Deep Undersea");
                else {
                    std::string ocean_name = Globals->WORLD_NAME;
                    ocean_name += " Ocean";
                    reg->set_name(ocean_name);
                }
            } else {
                if (reg->wages == -1) reg->set_name("Unnamed");
                else if (reg->wages != -2)
                    reg->set_name(AGetNameString(reg->wages));
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
        rng::get_random(pTo->x / pFrom->x);
    int tempy = reg->yloc * pTo->y / pFrom->y +
        rng::get_random(pTo->y / pFrom->y);
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
    o->set_name("Shaft");
    o->type = O_SHAFT;
    o->incomplete = 0;
    o->inner = temp->num;
    reg->objects.push_back(o);

    o = new Object(reg);
    o->num = temp->buildingseq++;
    o->set_name("Shaft");
    o->type = O_SHAFT;
    o->incomplete = 0;
    o->inner = reg->num;
    temp->objects.push_back(o);
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

            if (rng::get_random(odds) != 0) continue;

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
                                o->set_name("Gateway to " + std::string(TerrainDefs[type].name));
                                o->type = O_GATEWAY;
                                o->incomplete = 0;
                                o->inner = reg->num;
                                AC->objects.push_back(o);
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
                int tempx = i*8 + rng::get_random(8);
                int tempy = j*16 + rng::get_random(8)*2 + tempx%2;
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

    for(const auto r : regions) {
        if (r->gate == -1) {
            int index = rng::get_random(ngates);
            while (used[index]) {
                if (Globals->DISPERSE_GATE_NUMBERS) {
                    index = rng::get_random(ngates);
                } else {
                    index++;
                    index = index % ngates;
                }
            }
            r->gate = index+1;
            used[index] = 1;
            // setting up gatemonth
            r->gatemonth = rng::get_random(12);;
        }
    }
    delete[] used;
}

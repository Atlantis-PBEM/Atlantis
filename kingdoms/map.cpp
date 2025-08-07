#include <stdlib.h>
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

int ARegion::Slope()
{
    int retval = 0;
    for (int i=0; i<NDIRS; i++) {
        ARegion *n = neighbors[i];
        if (!n) continue;
        int d = std::abs(n->elevation - elevation);
        if (d > retval) retval = d;
    }
    return retval;
}

int ARegion::SurfaceWater()
{
    int retval = 0;
    for (int i=0; i<NDIRS; i++) {
        ARegion *n = neighbors[i];
        if (!n) continue;
        int d = std::abs(n->humidity - humidity);
        if (d > retval) retval = d;
    }
    return retval;
}

int ARegion::Soil()
{
    int retval = 0;
    for (int i=0; i<NDIRS; i++) {
        ARegion *n = neighbors[i];
        if (!n) continue;
        int d = std::abs(n->vegetation - vegetation);
        if (d > retval) retval = d;
    }
    return retval;
}

int ARegion::Winds()
{
    int retval = 0;
    for (int i=0; i<NDIRS; i++) {
        ARegion *n = neighbors[i];
        if (!n) continue;
        int d = std::abs(n->temperature - temperature);
        if (d > retval) retval = d;
    }
    return retval;
}

int ARegion::TerrainFactor(int value, int average)
{
    int retval = 0;
    int df = std::abs(value-average);
    if (df > 9) {
        if (df < 15) retval = 1;
        else retval = df - 14;
    }
    return retval;
}

int ARegion::TerrainProbability(int terrain)
{
    int retval = 0;
    int l=0;
    switch(terrain) {
        case R_PLAIN:
            l = 10;
            if (elevation > l)
                retval += (elevation-l) * (elevation-l) / 36;
            retval += TerrainFactor(vegetation, 50);
            retval += TerrainFactor(humidity, 50);
            if (temperature > 52) retval += TerrainFactor(temperature, 52);
            retval += Slope() * Slope() / 16;
            break;
        case R_FOREST:
            l = 93;
            if (vegetation < l)
                retval += (l - vegetation) * (l - vegetation) / 26;
            l = 55;
            if (temperature > l) retval += (temperature-l) * (temperature-l) / 24;
            l = 30;
            if (temperature < l) retval += (temperature-l) * (temperature-l) / 32;
            retval += TerrainFactor(humidity, 65);
            break;
        case R_MOUNTAIN:
            l = 90;
            if (elevation < l)
                retval += (l-elevation) * (l-elevation) / 10;
            if ((10 - Slope()) > 0)
                retval += (10 - Slope()) * (10 - Slope()) / 8;
            if (Slope() < 30) retval += TerrainFactor(Slope(),30);
            break;
        case R_SWAMP:
            l = 95;
            if (humidity < l) retval += (l-humidity) * (l-humidity) / 24;
            retval += TerrainFactor(elevation, 15);
            retval += (SurfaceWater()-3) * (SurfaceWater()-3) / 32;
            break;
        case R_JUNGLE:
            l = 90;
            if (vegetation < l)
                retval += (l-vegetation) * (l-vegetation) / 56;
            retval += (100-temperature) * (100-temperature) / 49;
            retval += Soil() * Soil() / 49;
            retval += TerrainFactor(humidity, 85);
            break;
        case R_DESERT:
            l = 10;
            if (humidity > l)
                retval += (humidity-l) * (humidity-l) / 32;
            l = 91;
            if (temperature < l)
                retval += (l-temperature) * (l-temperature) / 64;
            if (vegetation > 25) retval += TerrainFactor(vegetation, 15);
            break;
        case R_TUNDRA:
            l = 3;
            if (temperature > l)
                retval += (temperature-l) * (temperature-l) / 9;
            retval += Winds() * Winds() / 5;
            retval += TerrainFactor(vegetation, 5);
            break;
        default: // LAKE
            for (int i=0; i < NDIRS; i++) {
                ARegion *nr = neighbors[i];
                if (!nr) continue;
                if (nr->type == R_OCEAN) retval += 500;
            }
            if (SurfaceWater() < 52)
                retval += (52-SurfaceWater()) * (52-SurfaceWater()) / 7;
            retval += TerrainFactor(humidity, 85) / 5;
        }
    return retval;
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

    pRegionArrays[level]->set_name(name);
    pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;
    int sea = Globals->OCEAN;
    if (Globals->SEA_LIMIT)
        sea = sea * (100 + 2 * Globals->SEA_LIMIT) / 100;

    InitGeographicMap(pRegionArrays[level]);

    MakeLand(pRegionArrays[level], sea, Globals->CONTINENT_SIZE);

    RescaleFractalParameters(pRegionArrays[level]);

    CleanUpWater(pRegionArrays[level]);

    SetFractalTerrain(pRegionArrays[level]);

    GrowTerrain(pRegionArrays[level], 0);

    AssignTypes(pRegionArrays[level]);

    SeverLandBridges(pRegionArrays[level]);

    if (Globals->LAKES) RemoveCoastalLakes(pRegionArrays[level]);

    NameRegions(pRegionArrays[level]);

    if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

    FinalSetup(pRegionArrays[level]);
}

void ARegionList::create_island_ring_level(int level, int xSize, int ySize, const std::string& name)
{
    throw "create_island_ring_level not implemented for this game ruleset";
}

void ARegionList::create_underworld_ring_level(int level, int xSize, int ySize, const std::string& name)
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

                regions.push_back(reg);
                arr->SetRegion(x, y, reg);
            }
        }
    }

    SetupIcosahedralNeighbors(arr);

    logger::write("");
}

void ARegionList::InitGeographicMap(ARegionArray *pRegs)
{
    GeoMap geo = GeoMap(pRegs->x, pRegs->y);
    // Parameters: scatter(1-4), smoothing (1-100)
    geo.Generate(3, 25);
    geo.ApplyGeography(pRegs);
    logger::write("");
}

void ARegionList::MakeLand(ARegionArray *pRegs, int percentOcean,
        int continentSize)
{
    int total, ocean;
    total = 0;
    for (int x=0; x < pRegs->x; x++) {
        for (int y=0; y < pRegs->y; y++) {
            ARegion *r = pRegs->GetRegion(x,y);
            if (!r) continue;
            total++;
        }
    }
    ocean = total;

    logger::write("Making land");
    int fractal = 0;
    if (!fractal) {
        while (ocean > (total * percentOcean) / 100) {
            logger::dot();
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
    } else {

    /* Fractal land creation */
    int sealevel = 99;

    // First lower the sea level
    logger::write("Lower sea level");
    int fractalpercent = ((100 - percentOcean) * 90 / 100);
    while (ocean > (total * (percentOcean + fractalpercent)) / 100) {
        for (int x=0; x < pRegs->x; x++) {
            for (int y=0; y < pRegs->y/2; y++) {
                ARegion *reg = pRegs->GetRegion(x, (y*2 + x%2));
                if (!reg) continue;
                if ((reg->type == -1) && (reg->elevation > sealevel)) {
                    int cont = 0;
                    for (int i=0; i < NDIRS; i++) {
                        ARegion *nr = reg->neighbors[i];
                        if (!nr) continue;
                        if (nr->elevation > sealevel) {
                            cont++;
                            if (cont >= 2) break;
                            for (int k=-1; k < 2; k++) {
                                int d = i + k;
                                if (d < 0) d += NDIRS;
                                if (d >= NDIRS) d = NDIRS - d;
                                ARegion *ar = nr->neighbors[d];
                                if (!ar) continue;
                                if (ar->elevation > sealevel) cont++;
                            }
                        }
                        if (cont >= 2) break;
                    }
                    if (cont >= 2) {
                        reg->type = R_NUM;
                        ocean--;
                    }
                }
            }
        }
        sealevel--;
        logger::dot();
    }
    logger::write("");

    logger::write("Add further land");
    int coast = sealevel;
    int dotter = 0;
    while (ocean > (total * (percentOcean)) / 100) {
        int sz = rng::get_random(continentSize);
        sz = sz * sz;
        dotter++;
        int tempx = rng::get_random(pRegs->x);
        int yoff = pRegs->y / 40;
        int yband = pRegs->y / 2 - 2 * yoff;
        int tempy = (rng::get_random(yband)+yoff) * 2 + tempx % 2;
        int margin = rng::get_random(sealevel/10 + 2) + 2;
        ARegion *reg = pRegs->GetRegion(tempx, tempy);
        if (!reg) continue;
        ARegion *newreg = reg;
        sealevel = coast;
        while (reg->elevation < sealevel-margin) {
            int tries = 0;
            while ((reg->elevation < sealevel-margin) && (tries < 25)) {
                tempx = rng::get_random(pRegs->x);
                tempy = (rng::get_random(yband)+yoff) * 2 + tempx % 2;
                newreg = pRegs->GetRegion(tempx, tempy);
                if (!reg) continue;
                reg = newreg;
            }
            sealevel--;
            margin = rng::get_random(sealevel/10 + 2) + 2;
        }

        int cont = 0;
        for (int i=0; i<NDIRS; i++) {
            ARegion *nr = reg->neighbors[i];
            if (!nr) continue;
            if (nr->elevation >= reg->elevation-3) cont++;
        }
        if (cont < 3) continue;

        if (dotter%50 == 0) logger::dot();
        // make a continent
        if (reg->type == -1) {
            reg->type = R_NUM;
            ocean--;
        }
        int dir;
        for (int i=0; i<sz; i++) {
            int dr = rng::get_random(NDIRS);
            int maxe1, d1 = -1;
            for (int d=0; d<NDIRS; d++) {
                dir = dr + d;
                while (dir >= NDIRS) dir -= NDIRS;
                newreg = reg->neighbors[dir];
                if (!newreg) continue;
                if ((newreg->type == -1) && (newreg->elevation > sealevel/2)) {
                    if (newreg->elevation+rng::get_random(10) > maxe1) {
                        maxe1 = newreg->maxwages;
                        d1 = dir;
                    }
                }
            }
            if (d1 >= 0) dir = d1;
                else dir = rng::get_random(NDIRS);

            if ((reg->yloc < yoff*2) && ((dir < 2) || (dir = NDIRS-1))
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
                reg->type = R_NUM;
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

void ARegionList::RescaleFractalParameters(ARegionArray *pArr)
{
    logger::write("Rescaling fractal parameters...");
    int elev_min = 100, humi_min = 100, vege_min = 100, cult_min = 100;
    int elev_max = 0, humi_max = 0, vege_max = 0, cult_max = 0;
    for (int x = 0; x < pArr->x; x++) {
        for (int y = 0; y < pArr->y; y++) {
            ARegion *reg = pArr->GetRegion(x, y);
            if (!reg) continue;
            if (reg->type == R_NUM) {
                if (reg->elevation < elev_min) elev_min = reg->elevation;
                if (reg->elevation > elev_max) elev_max = reg->elevation;
                if (reg->humidity < humi_min) humi_min = reg->humidity;
                if (reg->humidity > humi_max) humi_max = reg->humidity;
                if (reg->vegetation < vege_min) vege_min = reg->vegetation;
                if (reg->vegetation > vege_max) vege_max = reg->vegetation;
                if (reg->culture < cult_min) cult_min = reg->culture;
                if (reg->culture > cult_max) cult_max = reg->culture;
            }
        }
    }
    if (humi_max > 100) humi_max = 100;
    if (humi_min < 0) humi_min = 0;
    for (int x = 0; x < pArr->x; x++) {
        for (int y = 0; y < pArr->y; y++) {
            ARegion *reg = pArr->GetRegion(x, y);
            if (!reg) continue;
            if (reg->type == R_NUM) {
                int old = reg->elevation;
                reg->elevation = 100 * (old - elev_min) / (elev_max - elev_min);
                if (rng::get_random(elev_max - elev_min) < (100 * (old - elev_min) % (elev_max - elev_min)))
                    reg->elevation++;
                old = reg->humidity;
                reg->humidity = 100 * (old - humi_min) / (humi_max - humi_min);
                if (rng::get_random(humi_max - humi_min) < (100 * (old - humi_min) % (humi_max - humi_min)))
                    reg->humidity++;
                old = reg->vegetation;
                reg->vegetation = 100 * (old - vege_min) / (vege_max - vege_min);
                if (rng::get_random(vege_max - vege_min) < (100 * (old - vege_min) % (vege_max - vege_min)))
                    reg->vegetation++;
                old = reg->culture;
                reg->culture = 100 * (old - cult_min) / (cult_max - cult_min);
                if (rng::get_random(cult_max - cult_min) < (100 * (old - cult_min) % (cult_max - cult_min)))
                    reg->culture++;

            }
        }
    }
}

void ARegionList::SetFractalTerrain(ARegionArray *pArr)
{
    // First count total land hexes
    int land = 0;
    for (int x = 0; x < pArr->x; x++) {
        for (int y = 0; y < pArr->y; y++) {
            ARegion *reg = pArr->GetRegion(x, y);
            if (!reg) continue;
            if (reg->type == R_NUM) land++;
        }
    }
    for (int l=0; l < 2; l++) {
        int set = 0;

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
        for (int x=0; x<(pArr->x)/f; x++) {
            for (int y=0; y<(pArr->y)/(f*2); y++) {
                if (rng::get_random(1000) > skip) continue;
                for (int i=0; i<4; i++) {
                    int tempx = x * f + rng::get_random(f);
                    int tempy = y * f * 2 + rng::get_random(f)*2 + tempx%2;
                    ARegion *reg = pArr->GetRegion(tempx, tempy);
                    if (!reg) continue;

                    if (reg->type == R_NUM) {
                        int max = 0;
                        int mtype = -1;
                        int ttype = 0;
                        int limit = 1000;
                        for (int t = R_PLAIN; t <= (R_TUNDRA+1); t++) {
                            int prob = reg->TerrainProbability(t);
                            ttype = t;
                            if (t == R_TUNDRA+1) ttype = R_LAKE;
                            if (l==0) {
                                switch(t) {
                                    case R_PLAIN: // PLAIN amount
                                        limit = 100;
                                        break;
                                    case R_FOREST: // FOREST amount
                                        limit = 100;
                                        break;
                                    case R_MOUNTAIN: // MOUNTAIN amount
                                        limit = 100;
                                        break;
                                    case R_SWAMP: // SWAMP amount
                                        limit = 100;
                                        break;
                                    case R_JUNGLE: // JUNGLE amount
                                        limit = 100;
                                        break;
                                    case R_DESERT: // DESERT amount
                                        limit = 100;
                                        break;
                                    case R_TUNDRA: // TUNDRA amount
                                        limit = 100;
                                        break;
                                    default: // LAKES
                                        limit = Globals->LAKES;
                                }
                            }
                            prob = limit - prob;
                            if (prob < 0) continue;
                            if (prob > max) {
                                max = prob;
                                mtype = ttype;
                            }
                        }
                        if (mtype > -1) {
                            reg->type = mtype;
                            reg->population = 1;
                            // if (TerrainDefs[reg->type].similar_type != R_OCEAN)
                            //  reg->wages = AGetName(0, reg);
                            set++;
                        }
                    }
                }
            }
        }
        logger::write("Set " + std::to_string(set) + " land regions of " + std::to_string(land) + " total.");
    }
}

void ARegionList::NameRegions(ARegionArray *pArr)
{
    logger::write("Naming Regions");
    int unnamed = 1;
    for (int x = 0; x < pArr->x; x++) {
        for (int y = 0; y < pArr->y; y++) {
            ARegion *r = pArr->GetRegion(x,y);
            if (!r) continue;
            r->wages = -1;
            // r->wages = AGetName(0, reg);
            r->population = 1;
        }
    }

    while(unnamed) {
        unnamed = 0;
        int sz = Globals->CONTINENT_SIZE / 3 + 1;
        int tnamedx[R_TUNDRA+1];
        int tnamedy[R_TUNDRA+1];
        for (int i=0; i < R_TUNDRA+1; i++) {
            tnamedx[i] = -1;
            tnamedy[i] = -1;
        }
        for (int x1 = 0; x1 < pArr->x; x1++) {
            for (int y1 = 0; y1 < pArr->y; y1++) {
                ARegion *r1 = pArr->GetRegion(x1, y1);
                if (!r1) continue;
                if ((r1->type < 0) || (r1->type == R_NUM) || (r1->type == R_OCEAN)) continue;
                if (r1->wages >= 0) continue;
                if (TerrainDefs[r1->type].similar_type > R_TUNDRA) continue;
                int lastnamed = 0;
                int xmin = tnamedx[TerrainDefs[r1->type].similar_type];
                int ymin = tnamedy[TerrainDefs[r1->type].similar_type];
                int dx = std::abs(r1->xloc - xmin);
                int dy = std::abs((r1->yloc - ymin) / 2);
                if ((xmin > 0) && (r1->xloc > xmin))
                    lastnamed += dx;
                if ((ymin > 0) && (r1->yloc > ymin))
                    lastnamed += dy;
                if ((xmin < 0) && (ymin < 0))
                    lastnamed = 2 * Globals->CONTINENT_SIZE;
                if (lastnamed > (3 * Globals->CONTINENT_SIZE / 2)) {
                    r1->wages = AGetName(0, r1);
                    r1->population = (rng::get_random(2) + sz / 2)
                        * (rng::get_random(2) + sz);

                    if (r1->xloc > xmin)
                        tnamedx[TerrainDefs[r1->type].similar_type] = r1->xloc;
                    if (r1->yloc > ymin)
                        tnamedy[TerrainDefs[r1->type].similar_type] = r1->yloc;
                    unnamed = 1;
                }
            }
        }
        int named_a_reg = 1;
        while(named_a_reg) {
            named_a_reg = 0;
            for (int x = 0; x < pArr->x; x++) {
                for (int y = 0; y < pArr->y; y++) {
                    ARegion *reg = pArr->GetRegion(x,y);
                    if ((!reg) || (reg->type == R_OCEAN)
                        || (reg->wages >= 0) || (reg->type == R_NUM)) continue;
                    int name1 = -99, name2 = -99;
                    int nw1 = 0, nw2 = 0, nc1 = 0, nc2 = 0, nn = 0;
                    int db = rng::get_random(NDIRS);
                    for (int d=0; d < NDIRS; d++) {
                        int dir = (d + db + NDIRS) % NDIRS;
                        ARegion *n = reg->neighbors[dir];
                        if ((!n) || (n->type == R_NUM)) continue;
                        if (TerrainDefs[n->type].similar_type != TerrainDefs[reg->type].similar_type) {
                            nn++;
                            continue;
                        }
                        if ((n->type == R_OCEAN) || (n->wages < 0)) continue;
                        if ((name1 < 0) || (n->wages == name1)) {
                            name1 = n->wages;
                            nw1 += n->population;
                            nc1++;
                        } else if ((name2 < 0) || (n->wages == name2)) {
                            name2 = n->wages;
                            nw2 += n->population;
                            nc2++;
                        }
                    }
                    if ((nc1 > 0) || (nc2 > 0)) {
                        if (nw1 > nw2) {
                            reg->wages = name1;
                            float npop = (nc1 * (nw1 + nc1 + nn) / (nc1 + 1 + nn))
                                + nn + (nc1 - 1);
                            if (npop > (nw1 / nc1)) npop = nw1 / nc1;
                            reg->population = (int) npop;
                            named_a_reg = 1;
                        } else {
                            reg->wages = name2;
                            float npop = (nc2 * (nw2 + nc2) / (nc2 + 1 + nn))
                                + nn + (nc2 - 1);
                            if (npop > (nw2 / nc2)) npop = nw2 / nc2;
                            reg->population = (int) npop;
                            named_a_reg = 1;
                        }
                    }
                }
            }
        }
        logger::dot();
    }
    logger::write("");
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
            skip -= 5;  // yes, that's intended!
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
                    // if (TerrainDefs[reg->type].similar_type != R_OCEAN)
                    //  reg->wages = AGetName(0, reg);
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
                        // if (TerrainDefs[reg->type].similar_type != R_OCEAN)
                        //  reg->wages = AGetName(0, reg);
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
                                // reg->wages = t->wages;
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

            for (int i=D_NORTH; i != NDIRS; ++i) {
                int count = 0;
                for (int j=D_NORTH; j != NDIRS; ++j)
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
    UnsetRace(pArr);
    int x, y;
    for (x = 0; x < pArr->x; x++) {
        for (y = 0; y < pArr->y; y++) {
            // Anchor distribution: depends on GROW_RACES value
            int jiggle = 4 + 2 * Globals->GROW_RACES;
            if ((y + ((x % 2) * jiggle/2)) % jiggle > 1) continue;
            int xoff = x + 2 - rng::get_random(3) - rng::get_random(3);
            ARegion *reg = pArr->GetRegion(xoff, y);
            if (!reg) continue;

            if ((reg->type == R_LAKE) && (!Globals->LAKESIDE_IS_COASTAL))
                continue;

            reg->race = -1;

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
                        reg->race = TerrainDefs[nreg->type].coastal_races[rng::get_random(rnum)];
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
                reg->race = TerrainDefs[reg->type].races[rng::get_random(rnum)];
            }
        }
    }
}

void ARegionList::GrowRaces(ARegionArray *pArr)
{
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
                    for (int i=0; i<cnum; i++) {
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

GeoMap::GeoMap(int x, int y)
{
    y = y/2;
    int max = (x > y) ? x : y;
    size = 8;
    while (size < 2*max) size *= 2;
    xscale = (size / x);
    yscale = (size / y);
    xoff = (size - x) / 2;
    yoff = (size - y) / 2;
    Geography g;
    geomap.clear();
    g.elevation = -1;
    g.humidity = -1;
    g.temperature = -1;
    g.vegetation = -1;
    g.culture = -1;
    for (int a=0; a<x; a++) {
        for (int b=0; b<y; b++) {
            long int coords = (size+1)*a + b;
            geomap.insert(std::make_pair(coords, g));
        }
    }
}

void GeoMap::Generate(int spread, int smoothness)
{
    int step = size / 2;
    while (step > 16) step = step / 2;
    logger::write(
        "Generating GeoMap - size: " + std::to_string(size) + ", xscale: " + std::to_string(xscale) +
        ", yscale: " + std::to_string(yscale)
    );
    for (int x = 0; x <= size; x += step) {
        for (int y = 0; y <= size; y += step) {
            Geography g;
            int tval = (size/2 - std::abs(size/2 - y)) * 25 / (size/2);
            g.elevation = rng::get_random(30)+rng::get_random(30)+20;
            g.humidity = rng::get_random(30)+rng::get_random(30)+20;
            g.temperature = rng::get_random(tval/2)+rng::get_random(tval/2)+tval;
            g.vegetation = rng::get_random(30)+rng::get_random(30)+20;
            g.culture = rng::get_random(20)+rng::get_random(20)+tval;
            long int coords = (size+1) * x + y;
            if (x >= size) {
                g.elevation = GetElevation((x-size), y);
                g.humidity = GetHumidity((x-size), y);
                g.temperature = GetTemperature((x-size), y);
                g.vegetation = GetVegetation((x-size), y);
                g.culture = GetCulture((x-size), y);
            }
            geomap.erase(coords);
            geomap.insert(std::make_pair(coords, g));
        }
    }
    int frac = 25;
    while (step > 1) {
        int nextstep = step/2 + step%2;
        for (int x = 0; x <= size; x += step) {
            for (int y = 0; y <= size; y += step) {
                int av_ele = 0;
                int av_hum = 0;
                int av_tem = 0;
                int av_veg = 0;
                int av_cul = 0;
                int xcoor = x + nextstep;
                int ycoor = y + nextstep;
                if ((xcoor > size) || (ycoor > size)) continue;
                int nb = 0;
                for (int a = -1; a < 2; a += 2) {
                    for (int b = -1; b < 2; b += 2) {
                        int nx = xcoor + nextstep * a;
                        int ny = ycoor + nextstep * b;
                        if (nx > size) nx = nx - size;
                        if (nx < 0) nx = size - nx;
                        int ge = GetElevation(nx, ny);
                        int gh = GetHumidity(nx, ny);
                        int gt = GetTemperature(nx, ny);
                        int gv = GetVegetation(nx, ny);
                        int gc = GetCulture(nx, ny);
                        if (ge != 0) {
                            av_ele += ge;
                            av_hum += gh;
                            av_tem += gt;
                            av_veg += gv;
                            av_cul += gc;
                            nb++;
                        } else {
                            av_ele += rng::get_random(5)+rng::get_random(5)+45;
                            av_hum += rng::get_random(20)+rng::get_random(20)+30;
                            av_tem += rng::get_random(5)+1;
                            av_veg += rng::get_random(20)+rng::get_random(20)+30;
                            av_cul += rng::get_random(15)+1;
                            nb++;
                        }
                    }
                }
                Geography g;
                int r1 = rng::get_random(2*frac) - frac;
                int r2 = rng::get_random(2*frac) - frac;
                int r3 = (rng::get_random(frac) - frac)/2;
                int r4 = rng::get_random(2*frac) - frac;
                int r5 = rng::get_random(2*frac) - frac;
                g.elevation = av_ele/nb + av_ele%nb + r1;
                g.humidity = av_hum/nb + av_hum%nb + r2;
                int tmean = (size/2 - std::abs(size/2 - ycoor)) * 100 / (size/2);
                g.temperature = (tmean + av_tem/nb + av_tem%nb + r3)/2;
                g.vegetation = av_veg/nb + av_veg%nb + r4;
                g.culture = av_cul/nb + av_cul%nb + r5;
                if (g.elevation < 1) g.elevation = rng::get_random(5);
                if (g.temperature < 1) g.temperature = rng::get_random(5);
                if (g.vegetation < 1) g.vegetation = rng::get_random(5);
                if (g.culture < 1) g.culture = rng::get_random(10);
                if (g.elevation > 100) g.elevation = 100 - rng::get_random(4);
                if (g.vegetation > 100) g.vegetation = 100 - rng::get_random(4);
                if (g.temperature > 100) {
                    //g.humidity -= g.temperature - 100;
                    g.temperature = 100 - rng::get_random(4);
                }
                if (g.humidity < 1) g.humidity = rng::get_random(5);
                if (g.humidity > 100) g.humidity = 100 - rng::get_random(4);
                long int coords = (size+1) * xcoor + ycoor;
                geomap.erase(coords);
                geomap.insert(std::make_pair(coords, g));

            }
        }
        logger::dot();
        for (int x = 0; x <= size; x += step) {
            for (int y = 0; y <= size; y += step) {
                for (int i = 0; i < 2; i++) {
                    int av_ele = 0;
                    int av_hum = 0;
                    int av_tem = 0;
                    int av_veg = 0;
                    int av_cul = 0;
                    int dx, dy;
                    if (i==0) {
                        dx = x + nextstep;
                        dy = y;
                    } else {
                        dx = x;
                        dy = y + nextstep;
                    }
                    if ((dx > size) || (dy > size)) continue;
                    int nb = 0;
                    for (int a = -1; a < 2; a += 2) {
                        int nx = dx + a*nextstep;
                        if (nx > size) nx = nx - size;
                        if (nx < 0) nx = size - nx;
                        int ge = GetElevation(nx, dy);
                        int gh = GetHumidity(nx, dy);
                        int gt = GetTemperature(nx, dy);
                        int gv = GetVegetation(nx, dy);
                        int gc = GetCulture(nx, dy);
                        if (ge != 0) {
                            av_ele += ge;
                            av_hum += gh;
                            av_tem += gt;
                            av_veg += gv;
                            av_cul += gc;
                            nb++;
                        } else {
                            av_ele += rng::get_random(5)+rng::get_random(5)+45;
                            av_hum += rng::get_random(20)+rng::get_random(20)+30;
                            av_tem += rng::get_random(5)+1;
                            av_veg += rng::get_random(20)+rng::get_random(20)+30;
                            av_cul += rng::get_random(15)+1;
                            nb++;
                        }
                    }
                    for (int b = -1; b < 2; b += 2) {
                        int ny = dy + b*nextstep;
                        int ge = GetElevation(dx, ny);
                        int gh = GetHumidity(dx, ny);
                        int gt = GetTemperature(dx, ny);
                        int gv = GetVegetation(dx, ny);
                        int gc = GetCulture(dx, ny);
                        if (ge != 0) {
                            av_ele += ge;
                            av_hum += gh;
                            av_tem += gt;
                            av_veg += gv;
                            av_cul += gc;
                            nb++;
                        } else {
                            av_ele += rng::get_random(5)+rng::get_random(5)+45;
                            av_hum += rng::get_random(20)+rng::get_random(20)+30;
                            av_tem += rng::get_random(5)+1;
                            av_veg += rng::get_random(20)+rng::get_random(20)+30;
                            av_cul += rng::get_random(15)+1;
                            nb++;
                        }
                    }

                    Geography g;
                    int r1 = rng::get_random(2*frac) - frac;
                    int r2 = rng::get_random(2*frac) - frac;
                    int r3 = (rng::get_random(frac) - frac)/2;
                    int r4 = rng::get_random(2*frac) - frac;
                    int r5 = rng::get_random(2*frac) - frac;
                    g.elevation = av_ele/nb + av_ele%nb + r1;
                    g.humidity = av_hum/nb + av_hum%nb + r2;
                    int tmean = (size/2 - std::abs(size/2 - dy)) * 100 / (size/2);
                    g.temperature = (tmean + av_tem/nb + av_tem%nb + r3)/2;
                    g.vegetation = av_veg/nb + av_veg%nb + r4;
                    g.culture = av_cul/nb + av_cul%nb + r5;
                    if (g.elevation < 1) g.elevation = rng::get_random(5);
                    if (g.temperature < 1) g.temperature = rng::get_random(5);
                    if (g.vegetation < 1) g.vegetation = rng::get_random(5);
                    if (g.culture < 1) g.culture = rng::get_random(10);
                    if (g.elevation > 100) g.elevation = 100 - rng::get_random(4);
                    if (g.vegetation > 100) g.vegetation = 100 - rng::get_random(4);
                    if (g.temperature > 100) {
                        //g.humidity -= g.temperature - 100;
                        g.temperature = 100 - rng::get_random(4);
                    }
                    if (g.humidity < 1) g.humidity = rng::get_random(5);
                    if (g.humidity > 100) g.humidity = 100 - rng::get_random(4);
                    long int coords = (size+1) * dx + dy;
                    geomap.erase(coords);
                    geomap.insert(std::make_pair(coords, g));

                }
            }
        }
        frac = (frac * (200-smoothness)/200);
        step = nextstep;
    }
    logger::write("");
}

int GeoMap::GetElevation(int x, int y)
{
    long int coords = (size+1)*x + y;
    std::map<long int,Geography>::iterator it = geomap.find(coords);
    if (it != geomap.end()) {
        Geography g = it->second;
        return g.elevation;
    }
    return 0;
}

int GeoMap::GetHumidity(int x, int y)
{
    long int coords = (size+1)*x + y;
    std::map<long int,Geography>::iterator it = geomap.find(coords);
    if (it != geomap.end()) {
        Geography g = it->second;
        return g.humidity;
    }
    return 0;
}

int GeoMap::GetTemperature(int x, int y)
{
    long int coords = (size+1)*x + y;
    std::map<long int,Geography>::iterator it = geomap.find(coords);
    if (it != geomap.end()) {
        Geography g = it->second;
        return g.temperature;
    }
    return 0;
}

int GeoMap::GetVegetation(int x, int y)
{
    long int coords = (size+1)*x + y;
    std::map<long int,Geography>::iterator it = geomap.find(coords);
    if (it != geomap.end()) {
        Geography g = it->second;
        return g.vegetation;
    }
    return 0;
}

int GeoMap::GetCulture(int x, int y)
{
    long int coords = (size+1)*x + y;
    std::map<long int,Geography>::iterator it = geomap.find(coords);
    if (it != geomap.end()) {
        Geography g = it->second;
        return g.culture;
    }
    return 0;
}

void GeoMap::ApplyGeography(ARegionArray *pArr)
{
    int x, y;
    int hmin = 100;
    int hmax = 0;
    for (x = 0; x < pArr->x; x++) {
        for (y = 0; y < pArr->y; y++) {
            ARegion *reg = pArr->GetRegion(x, y);
            if (!reg) continue;
            int cx = (x * xscale); // x+xoff;
            int cy = ((y/2 + x%2) * yscale); // + yoff;
            int ctr = 0;
            for (int dx = -3; dx < 4; dx++) {
                for (int dy = -3; dy < 4; dy++) {
                    if (std::abs(dx * dy) == 9) continue;
                    int f = 64;
                    for (int i = std::abs(dx * dy); i > 0; i--) f = f / 2;
                    int lx = cx + dx;
                    int ly = cy + dy;
                    if ((dx == 0) && (dy == 0)) f = 12 / Globals->TERRAIN_GRANULARITY;
                    reg->elevation += f * GetElevation(lx, ly);
                    reg->humidity += f * GetHumidity(lx, ly);
                    reg->temperature += f * GetTemperature(lx, ly);
                    reg->vegetation += f * GetVegetation(lx, ly);
                    reg->culture += f * GetCulture(lx, ly);
                    ctr += f;
                }
            }
            reg->elevation = reg->elevation / ctr;
            reg->humidity = reg->humidity / ctr;
            reg->temperature = reg->temperature / ctr;
            reg->vegetation = reg->vegetation / ctr;
            reg->culture = reg->culture / ctr;
            if (reg->humidity > hmax) hmax = reg->humidity;
            if (reg->humidity < hmin) hmin = reg->humidity;
        }
    }
}

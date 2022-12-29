// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 2022 Valdis Zobēla
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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

#include "mapgen.h"
#include "simplex.h"

#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <iostream>
#include <string>
#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#ifndef M_E
    #define M_E 2.7182818284590452354
#endif

// #define T_UNKNOWN        0
// #define T_LAKE           2
// #define T_RIVER          3
// #define T_VOLKANO        5

const int MIN_TEMP = -1000;
const int MAX_TEMP = 1000;

const int MAX_RAINFALL = 1000;

const std::vector<Biome> BIOMES = {
    // -10
    { .name = B_TUNDRA, .feritality = 0.6, .temp = { -1000,    0 }, .rainfall = {   0,  200 } },
    { .name = B_DESERT, .feritality = 1.2, .temp = { -1000,    0 }, .rainfall = { 201,  300 } },
    { .name = B_PLAINS, .feritality = 0.6, .temp = { -1000,    0 }, .rainfall = { 301,  500 } },
    { .name = B_FOREST, .feritality = 0.6, .temp = { -1000,    0 }, .rainfall = { 501, 1000 } },

    // 0
    { .name = B_TUNDRA, .feritality = 0.8, .temp = {     1,   10 }, .rainfall = {   0,  100 } },
    { .name = B_DESERT, .feritality = 1.2, .temp = {     1,   30 }, .rainfall = { 101,  200 } },
    { .name = B_PLAINS, .feritality = 1.0, .temp = {     1,   10 }, .rainfall = { 201,  400 } },
    { .name = B_FOREST, .feritality = 1.2, .temp = {     1,   10 }, .rainfall = { 401, 1000 } },

    // 10
    { .name = B_DESERT, .feritality = 1.2, .temp = {    11,   30 }, .rainfall = {   0,  200 } },
    { .name = B_PLAINS, .feritality = 1.0, .temp = {    11,   20 }, .rainfall = { 201,  300 } },
    { .name = B_FOREST, .feritality = 1.0, .temp = {    11,   20 }, .rainfall = { 301,  500 } },
    { .name = B_SWAMP,  .feritality = 1.0, .temp = {    11,   20 }, .rainfall = { 501, 1000 } },

    // 20
    { .name = B_DESERT, .feritality = 1.2, .temp = {    21,   30 }, .rainfall = {   0,  100 } },
    { .name = B_PLAINS, .feritality = 1.0, .temp = {    21,   30 }, .rainfall = { 101,  200 } },
    { .name = B_FOREST, .feritality = 1.2, .temp = {    21,   30 }, .rainfall = { 201,  400 } },
    { .name = B_JUNGLE, .feritality = 0.8, .temp = {    21,   30 }, .rainfall = { 401,  500 } },
    { .name = B_SWAMP,  .feritality = 0.8, .temp = {    21,   30 }, .rainfall = { 501, 1000 } },

    // 30
    { .name = B_DESERT, .feritality = 1.0, .temp = {    31,   40 }, .rainfall = {   0,  100 } },
    { .name = B_PLAINS, .feritality = 1.2, .temp = {    31,   40 }, .rainfall = { 101,  300 } },
    { .name = B_FOREST, .feritality = 1.2, .temp = {    31,   40 }, .rainfall = { 301,  400 } },
    { .name = B_JUNGLE, .feritality = 1.0, .temp = {    31,   40 }, .rainfall = { 401,  600 } },
    { .name = B_SWAMP,  .feritality = 0.8, .temp = {    31,   40 }, .rainfall = { 601, 1000 } },

    // 40
    { .name = B_DESERT, .feritality = 1.0, .temp = {    41,   50 }, .rainfall = {   0,  200 } },
    { .name = B_PLAINS, .feritality = 1.2, .temp = {    41,   50 }, .rainfall = { 201,  300 } },
    { .name = B_FOREST, .feritality = 1.2, .temp = {    41,   50 }, .rainfall = { 301,  400 } },
    { .name = B_JUNGLE, .feritality = 1.0, .temp = {    41,   50 }, .rainfall = { 401, 1000 } },

    // 50
    { .name = B_DESERT, .feritality = 1.0, .temp = {    51,   60 }, .rainfall = {   0,  300 } },
    { .name = B_PLAINS, .feritality = 1.2, .temp = {    51,   60 }, .rainfall = { 301,  400 } },
    { .name = B_JUNGLE, .feritality = 1.0, .temp = {    51,   60 }, .rainfall = { 401, 1000 } },

    // 60
    { .name = B_DESERT, .feritality = 1.0, .temp = {    61, 1000 }, .rainfall = {   0,  400 } },
    { .name = B_PLAINS, .feritality = 1.2, .temp = {    61, 1000 }, .rainfall = { 401,  500 } },
    { .name = B_JUNGLE, .feritality = 1.0, .temp = {    61, 1000 }, .rainfall = { 501, 1000 } },
};

bool Range::in(const int value) const {
    return value >= min && value <= max;
}

bool Biome::match(const Cell* cell) const {
    return temp.in(cell->temperature) && rainfall.in(cell->rainfall);
}

CellGraph::CellGraph(CellMap* map) {
	this->map = map;
}

CellGraph::~CellGraph() {

}

Cell* CellGraph::get(int id) {
    return map->items[id];
}

std::vector<int> CellGraph::neighbors(int id) {
    Cell* current = map->items[id];

    std::vector<int> list;

    addCell(current,  0, -1, list);
    addCell(current,  1, -1, list);
    addCell(current,  1,  0, list);
    addCell(current,  1,  1, list);
    addCell(current,  0,  1, list);
    addCell(current, -1,  1, list);
    addCell(current, -1,  0, list);
    addCell(current, -1, -1, list);

    return list;
}

double CellGraph::cost(int current, int next) {
    return 1;
}

void CellGraph::setInclusion(CellInclusionFunction includeFn) {
    this->includeFn = includeFn;
}

void CellGraph::addCell(Cell* current, int dx, int dy, std::vector<int>& list) {
    Cell* n = map->get(current->x + dx, current->y + dy);
    if (n && includeFn(current, n)) {
        list.push_back(n->index);
    }
}

CellMap::CellMap(int width, int height) {
    this->width = width;
    this->height = height;

    int len = width * height;
    items.reserve(len);
    
    for (int i = 0; i < len; i++) {
        int x;
        int y;
        coords(i, x, y);

        items.push_back(new Cell({
            .index = i,
            .x = x,
            .y = y,
            .biome = B_UNKNOWN,
            .elevation = 0,
            .temperature = 0,
            .saturation = 0,
            .evoparation = 0,
            .rainfall = 0,
        }));
    }
}

CellMap::~CellMap() {
    for (auto item : items) {
        delete item;
    }
}

Cell* CellMap::get(int x, int y) {
    if (!normalize(x, y)) {
        return NULL;
    }

    return items[index(x, y)];
}

bool CellMap::normalize(int& x, int& y) {
    if (y < 0 || y >= height) {
        return false;
    }

    if (x < 0) {
        x += width;
    }

    if (x >= width) {
        x = x % width;
    }

    return true;
}

int CellMap::index(int x, int y) {
    return x + y * width;
}

void CellMap::coords(int index, int& x, int& y) {
    y = index / width;
    x = index % width;
}

bool Blob::includes(Cell* cell) {
    return std::find(std::begin(items), std::end(items), cell) != std::end(items);
}

bool Blob::add(Cell* cell) {
    if (includes(cell)) {
        return false;
    }

    items.push_back(cell);

    return true;
}

void Blob::add(Blob* blob) {
    for (auto item : blob->items) {
        items.push_back(item);
    }
}

const double T_K = 273.0;

double saturation(int temperature) {
    double T = temperature + T_K;
    double S = pow(M_E, 77.345 + 0.0057 * T - 7235.0 / T) / pow(T, 8.2);

    // if (temperature < 0) {
    //     S = pow(S, -temperature / 16.0 + 1.0);
    // }
    
    return S;
}

double density(double saturation, int temperature) {
    double T = temperature + T_K;
    double P = (0.0022 * saturation) / T;

    return P;
}

const double TEMP_ALT_CHANGE = 6.0 / 1000;  // temperature changes by 6°C every 1000m

double temperature(double minTemp, double maxTemp, double axialTiltRad, double latitudeRad, int elevation) {
    double tempRange = maxTemp - minTemp;

    double tR = cos((latitudeRad - axialTiltRad) / 1.5);
    double tE = elevation * TEMP_ALT_CHANGE;
    double t = tempRange * tR - tE + minTemp;

    return t;
}

const double RAD = M_PI / 180.0;

inline double degToRad(double deg) {
    return deg * RAD;
}

// static const Edge LEFT_WIND[MOISTURE_SOURCES] = {
//     {  0,  1, 0.1 },
//     { -1,  1, 0.2 },
//     { -1,  0, 0.7 },
//     { -1, -1, 0.2 },
//     {  0, -1, 0.1 }
// };

// static const Edge RIGHT_WIND[MOISTURE_SOURCES] = {
//     { 0,  1, 0.1 },
//     { 1,  1, 0.2 },
//     { 1,  0, 0.7 },
//     { 1, -1, 0.2 },
//     { 0, -1, 0.1 }
// };

const int MOISTURE_SOURCES = 8;
const Edge WIND[MOISTURE_SOURCES] = {
    { -1, -1, 0.20 },
    { -1,  0, 0.70 },
    { -1,  1, 0.20 },
    {  0, -1, 0.10 },
    {  0,  1, 0.10 },
    {  1, -1, 0.05 },
    {  1,  0, 0.05 },
    {  1,  1, 0.05 }
};

const double RAINFALL = 0.1;

//                  | rainfall
// -----------------+---------
// temperature up   | decrease
// temperature down | increase
// altitude up      | increase
// altitude down    | decrease

void simulateRain(CellMap& map, Cell* target, double windAngle, const Edge* windMatrix) {
    int x = target->x;
    int y = target->y;

    int totalMoisture = 0;
    double rainfall = 0;

    const int RANGE = 4;
    const double MOISTURE_FADE = 4.0;
    const double MOISTURE_TO_MM = 20.0;

    for (int dx = -RANGE; dx <= RANGE; dx++) {
        for (int dy = -RANGE; dy <= RANGE; dy++) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            auto cell = map.get(x + dx, y + dy);
            if (cell == NULL) {
                continue;
            }

            int distance = sqrt(dx * dx + dy * dy);
            if (distance > RANGE) {
                continue;
            }

            if (!cell->evoparation) {
                continue;
            }

            double moisture = cell->evoparation * (MOISTURE_FADE / distance);

            int dE = target->elevation <= 0
                ? 0
                : std::max(0, target->elevation) - std::max(0, cell->elevation);

            double rain = 0;
            if (dE > 0 && moisture > 0) {
                rain = (dE * RAINFALL * moisture) / 500.0;
            }

            rainfall += rain;
            totalMoisture += moisture;
        }
    }

    target->rainfall = std::min(MAX_RAINFALL, (int) round(rainfall / MOISTURE_TO_MM));
}

Map::Map(int width, int height) : map(CellMap(width, height)) {
    minTemp = 0;
    maxTemp = 60;

    frequency = 5.0;
    amplitude = 0.5;
    redistribution = 1.0;
    evoparation = 1.0;
    
    waterPercent = 0.2;
    mountainPercent = 0.2;
}

Blob* fillByElevation(CellMap* map, Cell* start, int biome, Range elevation) {
    Blob* blob = new Blob();
    blob->biome = biome;

    CellGraph graph = CellGraph(map);
    graph.setInclusion([ elevation ](Cell* current, Cell* next) {
        return elevation.in(next->elevation) && next->biome == B_UNKNOWN;
    });

    auto result = graphs::breadthFirstSearch(graph, start->index);

    for (auto kv : result) {
        auto cell = graph.get(kv.first);
        cell->biome = biome;
        blob->add(cell);
    }

    return blob;
}

Blob* fillByBiome(CellMap* map, Cell* start, const Biome* biome) {
    Blob* blob = new Blob();
    blob->biome = biome->name;

    CellGraph graph = CellGraph(map);
    graph.setInclusion([ biome ](Cell* current, Cell* next) {
        return biome->match(next) && next->biome == B_UNKNOWN;
    });

    auto result = graphs::breadthFirstSearch(graph, start->index);

    for (auto kv : result) {
        auto cell = graph.get(kv.first);
        cell->biome = biome->name;
        blob->add(cell);
    }

    return blob;
}

void Map::Generate() {
    int len = map.width * map.height;

    SimplexNoise* noise = new SimplexNoise(frequency, amplitude);
    std::vector<Blob*> blobs;

    // 0. elevation
    const int ELEVATION = 16000;    // elevation range is 16km
    int minElevation = ELEVATION;
    int maxElevation = 0;
    std::map<int, int> hist;
    for (int i = 0; i < len; i++) {
        auto cell = map.items[i];

        double nx = (double) cell->x / map.width;
        double ny = (double) cell->y / map.height;

        double e = pow((noise->cylinderFractal(3, nx, ny) + 1.0) / 2.0, redistribution);
        
        cell->elevation = round(e * ELEVATION);
        minElevation = std::min(minElevation, cell->elevation);
        maxElevation = std::max(maxElevation, cell->elevation);
        
        ++hist[cell->elevation];
    }

    // 1. determine sea level
    std::cout << "1. determine sea level" << std::endl;

    int maxWaterCells = len * waterPercent;
    int waterCells = 0;
    int seaLevel = minElevation;

    int maxMountainCells = len * mountainPercent;
    int mountainCells = 0;
    int mountainLevel = maxElevation;

    for (auto &kv : hist) {
        if (waterCells >= maxWaterCells) {
            break;
        }

        seaLevel = kv.first;
        waterCells += kv.second;
    }

    for (auto iter = hist.rbegin(); iter != hist.rend(); iter++) {
        if (mountainCells >= maxMountainCells) {
            break;
        }

        mountainLevel = iter->first;
        mountainCells += iter->second;
    }


    // 2. determine water and mountains
    std::cout << "2. determine water and mountains" << std::endl;

    for (auto item : map.items) {
        if (item->biome != B_UNKNOWN) {
            continue;
        }

        if (item->elevation <= seaLevel) {
            auto blob = fillByElevation(&map, item, B_WATER, { minElevation, seaLevel });
            blobs.push_back(blob);
        }
        else if (item->elevation >= mountainLevel) {
            auto blob = fillByElevation(&map, item, B_MOUNTAINS, { mountainLevel, maxElevation });
            blobs.push_back(blob);
        }
    }


    // 3. temperature and moisture saturation
    std::cout << "3. temperature and moisture saturation" << std::endl;

    const double halfHeight = map.height / 2.0;
    for (auto item : map.items) {
        item->elevation -= seaLevel;

        bool isWater = item->biome == B_WATER;
        double lat = ((halfHeight - item->y) / halfHeight) * 90.0;
        int tempElevation = isWater ? 0 : item->elevation;

        
        item->temperature = round(temperature(minTemp, maxTemp, degToRad(/* 23.5 */ 0), degToRad(lat), tempElevation));
        item->temperature = std::min(MAX_TEMP, std::max(MIN_TEMP, item->temperature));
        item->saturation = round(saturation(item->temperature));

        if (isWater) {
            item->evoparation = round(pow(item->saturation, evoparation));
        }
        else {
            item->evoparation = 0;
        }
    }


    // 4. rain
    std::cout << "4. rain" << std::endl;

    /*
        Each cell can produce certain amount of moisture via evaporation.
        Wind moves evoparation from neigbour regions according to the wind direction (see table above).
        Incoming moisture combines with local evoparation.
        Certain amount of moisture will fall of as a rain.
        Remaining moisture will move to the neighbour regions.

        Elevation change impacts wind and rainfall. High mountains will create wind barier, and rain shadow
        from the opostie side of the mountain.
    */

    for (int i = 0; i < 2; i++) {
        for (int x = 0; x < map.width; x++) {
            for (int y = 0; y < map.height; y++) {
                auto target = map.get(x, y);

                simulateRain(map, target, 0.0, WIND);
            }
        }
    }

    // 5. biomes
    std::cout << "5. biomes" << std::endl;

    // assign biomes
    for (auto item : map.items) {
        if (item->biome != B_UNKNOWN) {
            continue;
        }

        // find biome
        const Biome* biome = NULL;
        for (auto &b : BIOMES) {
            if (b.match(item)) {
                biome = &b;
                break;
            }
        }

        if (biome == NULL) {
            std::cout << "NO BIOME for [" << item->x << ", " << item->y << "]"
                      << " " << item->elevation << "m"
                      << ", " << item->temperature << "°C"
                      << ", " << item->rainfall << "mm"
                      << std::endl;

            exit(1);
        }

        auto blob = fillByBiome(&map, item, biome);
        blobs.push_back(blob);
    }

    int unknown = 0;
    for (auto item : map.items) {
        if (item->biome == B_UNKNOWN) {
            unknown++;

            const Biome* biome = NULL;
            for (auto &b : BIOMES) {
                if (b.match(item)) {
                    biome = &b;
                    break;
                }
            }

            std::cout << biome->name
                      << " [" << item->x << ", " << item->y << "]"
                      << " " << item->elevation << "m"
                      << ", " << item->temperature << "°C"
                      << ", " << item->rainfall << "mm"
                      << std::endl;
        }
    }

    std::cout << unknown << " B_UNKNOWN" << std::endl;

    std::cout << "DONE" << std::endl;
}

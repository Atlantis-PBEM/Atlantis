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

#include <algorithm>
#include <list>
#include <unordered_set>
#include <stack>
#include <random>

enum ZoneType {
	UNDECIDED,	// zone type not yet determined
	OCEAN,		// no land will be generated in this zone
	CONTINENT,	// normal land will grow
	STRAIT		// ocean, but formed at two contient border area
	// ARCHIPELAGO	// another round of smaller zone generation will be executed to make many smaller islands
};

struct Coords {
	int x;
	int y;

	int Distance(const Coords& other);
};

int hexDistance(const Coords& a, const Coords& b) {
    int qA = a.x;
    int rA = (a.y - a.x) / 2;
    int sA = -qA - rA;

    int qB = b.x;
    int rB = (b.y - b.x) / 2;
    int sB = -qB - rB;

	int q = qA - qB;
	int r = rA - rB;
	int s = sA - sB;

	int d = (std::abs(q) + std::abs(r) + std::abs(s)) / 2;
	return d;
}

int Coords::Distance(const Coords& other) {
	return hexDistance(*this, other);
}

struct Zone;
struct Province;

struct ZoneRegion {
	int id;
	Coords location;
	bool exclude;
	int biome;
	Zone *zone;
	Province *province;
	ARegion* region;
	ZoneRegion *neighbors[NDIRS];

	bool HaveBorderWith(Zone* zone);
	bool HaveBorderWith(ZoneRegion* other);
	bool IsInner();
	bool IsOuter();
	bool AtBorderWith(ZoneType type);
	std::vector<Zone *> GetNeihborZones();
	int CountNeighbors(Zone* zone);
	int CountNeighbors(Province* province);
	std::map<int, int> CountNeighborBiomes();
};

std::map<int, int> ZoneRegion::CountNeighborBiomes() {
	std::map<int, int> stats;
	for (int i = 0; i < NDIRS; i++) {
		auto n = this->neighbors[i];
		if (n == NULL) continue;

		if (stats.find(n->biome) == stats.end()) {
			stats[n->biome] = 1;
		}
		else {
			stats[n->biome] += 1;
		}
	}

	return stats;
}

int ZoneRegion::CountNeighbors(Zone* zone) {
	int count = 0;
	for (int i = 0; i < NDIRS; i++) {
		auto n = this->neighbors[i];
		if (n != NULL && n->zone == zone) count++;
	}

	return count;
}

int ZoneRegion::CountNeighbors(Province* province) {
	int count = 0;
	for (int i = 0; i < NDIRS; i++) {
		auto n = this->neighbors[i];
		if (n != NULL && n->province == province) count++;
	}

	return count;
}

std::vector<Zone *> ZoneRegion::GetNeihborZones() {
	std::vector<Zone *> items;
	items.reserve(NDIRS);
	
	for (int i = 0; i < NDIRS; i++) {
		auto n = this->neighbors[i];
		if (n == NULL) continue;
		if (n->zone == zone) continue;

		auto z = n->zone;
		for (size_t j = 0; j < items.size(); j++) {
			if (items[j] == z) {
				z = NULL;
				break;
			}
		}

		if (z != NULL) items.push_back(z);
	}

	return items;
}

bool ZoneRegion::HaveBorderWith(Zone* zone) {
	for (int i = 0; i < NDIRS; i++) {
		auto n = this->neighbors[i];
		if (n == NULL) continue;
		if (n->zone == zone) return true;
	}

	return false;
}

bool ZoneRegion::HaveBorderWith(ZoneRegion* other) {
	for (int i = 0; i < NDIRS; i++) {
		auto n = this->neighbors[i];
		if (n == NULL) continue;
		if (n == other) return true;
	}

	return false;
}

bool ZoneRegion::IsInner() {
	for (int i = 0; i < NDIRS; i++) {
		auto n = this->neighbors[i];
		if (n == NULL) continue;
		if (n->zone != this->zone) return false;
	}

	return true;
}

bool ZoneRegion::IsOuter() {
	return !IsInner();
}

struct Province {
	int id;
	int h;
	int biome;
	Zone* zone;
	std::map<int, ZoneRegion *> regions;

	Coords GetLocation();
	void AddRegion(ZoneRegion* region);
	void RemoveRegion(ZoneRegion* region);
	bool Grow();
	int GetLatitude();
	int GetSize();
	std::unordered_set<Province *> GetNeighbors();
	std::unordered_set<int> GetNeighborBiomes();
};


std::unordered_set<Province *> Province::GetNeighbors() {
	std::unordered_set<Province *> items;

	for (auto &kv : this->regions) {
		auto reg = kv.second;

		for (int i = 0; i < NDIRS; i++) {
			auto n = reg->neighbors[i];
			if (n == NULL) continue;

			if (n->zone == this->zone && n->province != this) {
				items.insert(n->province);
			}
		}
	}

	return items;
}

std::unordered_set<int> Province::GetNeighborBiomes() {
	std::unordered_set<int> biomes;
	
	auto tmp = this->GetNeighbors();
	for (auto &n : tmp) {
		if (n->biome != -1) biomes.insert(n->biome);
	}

	return biomes;
}

Coords Province::GetLocation() {
	int xMin = -1;
	int yMin;
	int xMax;
	int yMax;

	for (auto &kv : regions) {
		auto reg = kv.second;

		if (xMin == -1) {
			xMin = reg->location.x;
			yMin = reg->location.y;
			xMax = xMin;
			yMax = yMin;

			continue;
		}
		
		xMin = std::min(xMin, reg->location.x);
		yMin = std::min(yMin, reg->location.y);
		xMax = std::max(xMax, reg->location.x);
		yMax = std::max(yMax, reg->location.y);
	}

	return {
		.x = xMin + (xMax - xMin + 1) / 2,
		.y = yMin + (yMax - yMin + 1) / 2
	};
}

int Province::GetLatitude() {
	auto loc = GetLocation();
	int lat = ( loc.y * 8 ) / this->h;
	if (lat > 3) {
		lat = std::max(0, 7 - lat);
	}

	return lat;
}

int Province::GetSize() {
	return this->regions.size();
}

void Province::AddRegion(ZoneRegion* region) {
	region->province = this;
	this->regions.insert(std::make_pair(region->id, region));
}

void Province::RemoveRegion(ZoneRegion* region) {
	if (region->province == this) {
		region->province = NULL;
	}

	this->regions.erase(region->id);
}

bool Province::Grow() {
	std::vector<ZoneRegion *> candidates;
	for (auto &kv : regions) {
		auto reg = kv.second;

		for (int i = 0; i < NDIRS; i++) {
			auto n = reg->neighbors[i];
			if (n == NULL) continue;
			if (n->zone != this->zone) continue;
			if (n->province != NULL) continue;

			candidates.push_back(n);
		}
	}

	if (candidates.size() == 0) return false;

	ZoneRegion* next = candidates[getrandom(candidates.size())];
	int connections = next->CountNeighbors(this);

	// 2d6
	int roll = makeRoll(2, 6);
	int diff = 12 - connections;

	// connections: 5, diff: 7, prob: 58%
	// connections: 4, diff: 8, prob: 41%
	// connections: 3, diff: 9, prob: 27%
	// connections: 2, diff: 10, prob: 16%
	// connections: 1, diff: 11, prob: 8%

	if (GetSize() == 1 || roll >= diff) {
		AddRegion(next);
	}

	return true;
}


struct Zone {
	~Zone();

	int id;
	Coords location;
	ZoneType type;
	std::map<int, Zone *> neighbors;
	std::map<int, ZoneRegion *> regions;
	std::map<int, Province *> provinces;

	void RemoveRegion(ZoneRegion* region);
	void AddRegion(ZoneRegion* region);
	void SetConnections();
	std::unordered_set<ZoneRegion *> TraverseRegions();
	bool CheckZoneIntegerity();
	bool AtBorderWith(Zone* zone);
	void RemoveNeighbor(Zone* zone);
	void AddNeighbor(Zone* zone);

	bool IsIsland();

	Province* CreateProvince(ZoneRegion* region, int h);
	void RemoveProvince(Province* province);
};

Zone::~Zone() {
	for (auto &kv : provinces) {
		delete kv.second;
	}
	provinces.clear();
}

Province* Zone::CreateProvince(ZoneRegion* region, int h) {
	Province* province = new Province;
	province->id = this->provinces.size();
	province->zone = this;
	province->h = h;
	province->biome = -1;

	province->AddRegion(region);
	this->provinces.insert(std::make_pair(province->id, province));

	return province;
}

void Zone::RemoveProvince(Province* province) {
	if (province->zone == this) {
		province->zone = NULL;
	}

	this->provinces.erase(province->id);

	delete province;
}

bool ZoneRegion::AtBorderWith(ZoneType type) {
	for (int i = 0; i < NDIRS; i++) {
		auto n = this->neighbors[i];
		if (n == NULL) continue;
		if (n->zone != this->zone && n->zone->type == type) return true;
	}

	return false;
}

bool Zone::IsIsland() {
	if (this->type != ZoneType::CONTINENT) return false;

	for (auto &kv : this->neighbors) {
		auto zone = kv.second;
		if (zone->type == ZoneType::CONTINENT) return false;
	}

	return true;
}

void Zone::RemoveRegion(ZoneRegion* region) {
	if (region->zone == this) {
		region->zone = NULL;
	}

	this->regions.erase(region->id);
}

void Zone::AddRegion(ZoneRegion* region) {
	if (region == NULL) {
		Awrite("Region cannot be NULL");
		exit(1);
	}

	region->zone = this;
	this->regions.insert(std::make_pair(region->id, region));

	if (this->regions.size() == 1) {
		this->location = region->location;
	}
}

void Zone::RemoveNeighbor(Zone* zone) {
	zone->neighbors.erase(this->id);
	this->neighbors.erase(zone->id);
}

void Zone::AddNeighbor(Zone* zone) {
	zone->neighbors.insert(std::make_pair(this->id, this));
	this->neighbors.insert(std::make_pair(zone->id, zone));
}

void Zone::SetConnections() {
	std::unordered_set<Zone *> visited;
	
	for (auto &region : this->regions) {
		for (auto &n : region.second->neighbors) {
			if (n == NULL) continue;
			if (n->zone == this) continue;

			if (n->zone == NULL) {
				Awrite("Region zone cannot be NULL");
				exit(1);
			}

			visited.insert(n->zone);
		}
	}

	this->neighbors.clear();
	for (auto &z : visited) {
		AddNeighbor(z);
	}
}

std::unordered_set<ZoneRegion *> Zone::TraverseRegions() {
	std::unordered_set<ZoneRegion *> v;
	std::stack<ZoneRegion *> s;

	if (this->regions.size() == 0) return v;

	s.push(this->regions.begin()->second);
	while (!s.empty()) {
		ZoneRegion* reg = s.top();
		s.pop();
		v.insert(reg);

		for (int i = 0; i < NDIRS; i++) {
			auto n = reg->neighbors[i];
			if (n == NULL) continue;
			if (n->zone != this) continue;
			if (v.find(n) != v.end()) continue;

			s.push(n);
		}
	}

	return v;
}

bool Zone::CheckZoneIntegerity() {
	if (this->regions.size() == 0) {
		return true;
	}

	auto v = this->TraverseRegions();

	return v.size() == this->regions.size();
}

bool Zone::AtBorderWith(Zone* zone) {
	for (auto &z : zone->neighbors) {
		if (this == z.second) return true;
	}

	return false;
}

class MapBuilder {
public:
	MapBuilder(ARegionArray* aregs);
	~MapBuilder();

	size_t maxZones;
	int gapMin;
	int gapMax;
	int volcanoesMin;
	int volcanoesMax;
	int lakesMin;
	int lakesMax;

	int w;
	int h;
	std::map<int, Zone *> zones;
	std::vector<ZoneRegion *> regions;
	size_t maxContinentArea;

	ZoneRegion* GetRegion(int x, int y);
	ZoneRegion* GetRegion(Coords location);
	void CreateZones(int minDistance, int maxAtempts);
	void GrowZones();
	void SpecializeZones(size_t continents, int continentAreaFraction);
	void GrowTerrain();
	void GrowLandInZone(Zone* zone);
	void SetOceanNames();
	void ClearEmptyZones();
	void MergeZoneInto(Zone* src, Zone* dest);
	void SplitZone(Zone* zone);
	void Clear();
	void ConnectZones();
	Zone* CreateZone(ZoneType type);
	Zone* GetNotIsland();
	Zone* GetZoneOfMaxSize(ZoneType type, size_t maxSize);
	void AddVolcanoes();
	void AddLakes();

private:
	int lastZoneId;
};

int EstimateMaxZones(int area, int radius) {
	int zoneArea = (1 + (12 * radius + 6 * (radius - 1) * radius) / 2);
	int maxZones = area / zoneArea;

	return maxZones;
}

int GetRegionIndex(const int x, const int y, const int w, const int h) {
	int xx = (x + w) % w;
	int yy = (y + h) % h;
	
	if ((xx + yy) % 2) {
		return -1;
	}

	int i = xx / 2 + yy * w / 2;

	return i;
}

Zone* MapBuilder::GetNotIsland() {
	for (auto &kv : this->zones) {
		auto zone = kv.second;
		if (zone->type != ZoneType::CONTINENT) continue;

		if (!zone->IsIsland()) {
			return zone;
		}
	}

	return NULL;
}

Zone* MapBuilder::GetZoneOfMaxSize(ZoneType type, size_t maxSize) {
	for (auto &kv : this->zones) {
		auto zone = kv.second;
		if (zone->type != type) continue;

		if (zone->regions.size() <= maxSize) {
			return zone;
		}
	}

	return NULL;
}

MapBuilder::MapBuilder(ARegionArray* aregs) {
	this->w = aregs->x;
	this->h = aregs->y;
	this->lastZoneId = 0;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (!((x + y) % 2)) {
				ZoneRegion *reg = new ZoneRegion;
				reg->id = GetRegionIndex(x, y, this->w, this->h);
				reg->location.x = x;
				reg->location.y = y;
				reg->zone = NULL;
				reg->exclude = false;
				reg->province = NULL;
				reg->region = aregs->GetRegion(x, y);
				reg->biome = -1;

				if (reg->location.x != reg->region->xloc || reg->location.y != reg->region->yloc) {
					Awrite("Region location do not match");
					exit(1);
				}

				this->regions.push_back(reg);
			}
		}
	}

	for (auto &item : this->regions) {
		ARegion* areg = item->region;
		for (int i = 0; i < NDIRS; i++) {
			ARegion* nreg = areg->neighbors[i];

			item->neighbors[i] = nreg == NULL
				? NULL
				: this->GetRegion(nreg->xloc, nreg->yloc);
		}
	}
}

Zone* MapBuilder::CreateZone(ZoneType type) {
	Zone* zone = new Zone;
	zone->id = this->lastZoneId++;
	zone->type = type;

	this->zones.insert(std::make_pair(zone->id, zone));

	return zone;
}

void MapBuilder::Clear() {
	for (auto &region : this->regions) {
		delete region;
	}

	for (auto &zone : this->zones) {
		delete zone.second;
	}

	this->regions.clear();
	this->zones.clear();
}

MapBuilder::~MapBuilder() {
	Clear();
}

void MapBuilder::ConnectZones() {
	for (auto &zone : this->zones) {
		zone.second->SetConnections();
	}
}

ZoneRegion* MapBuilder::GetRegion(const int x, const int y) {
	return this->regions[GetRegionIndex(x, y, this->w, this->h)];
}

ZoneRegion* MapBuilder::GetRegion(Coords location) {
	return this->GetRegion(location.x, location.y);
}

void MapBuilder::ClearEmptyZones() {
	int count = 0;
	std::vector<int> keys;
	for (auto &z : this->zones) {
		keys.push_back(z.first);
	}

	for (auto &key : keys) {
		Zone* zone = this->zones[key];
		if (zone->regions.size() == 0) {
			delete zone;
			this->zones.erase(key);
			count++;
		}
	}

	Awrite(AString("Removed ") + count + " empty zones");
}

void MapBuilder::CreateZones(int minDistance, int maxAtempts) {
	Awrite("Create zones");

	int attempts = 0;
	while (this->zones.size() < this->maxZones && attempts++ < maxAtempts) {
		Coords location = {
			.x = getrandom(this->w),
			.y = getrandom(this->h)
		};

		if ((location.x + location.y) % 2) continue;

		int distance = w + h;
		for (auto &kv : this->zones) {
			auto zone = kv.second;
			distance = std::min(distance, hexDistance(zone->location, location));
			if (distance < minDistance) break;
		}

		if (distance < minDistance) continue;

		Zone* z = CreateZone(ZoneType::UNDECIDED);
		ZoneRegion* r = this->GetRegion(location);
		z->AddRegion(r);

		attempts = 0;
	}
}

void MapBuilder::GrowZones() {
	Awrite("Grow zones");

	std::unordered_set<Zone *> v;

	// run loop while there are regions not assigned to any zone
	std::vector<ZoneRegion *> nextRegions;
	while (this->zones.size() != v.size()) {
		for (auto &kv : this->zones) {
			auto zone = kv.second;
			if (v.find(zone) != v.end()) {
				// already processed zone
				continue;
			}

			// find all regions where zone can grow
			nextRegions.clear();
			for (auto &kv2 : zone->regions) {
				auto region = kv2.second;
				for (auto &n : region->neighbors) {
					if (n == NULL) continue;
					if (n->zone != NULL) continue;

					nextRegions.push_back(n);
				}
			}

			if (nextRegions.size() == 0) {
				v.insert(zone);
				continue;
			}

			// select one region to grow
			ZoneRegion* next = nextRegions[getrandom(nextRegions.size())];
			int connections = 0;
			for (auto &n : next->neighbors) {
				if (n != NULL && n->zone == zone) {
					connections++;
				}
			}

			// 2d6
			int roll = makeRoll(2, 6);
			int diff = 12 - connections;

			// connections: 5, diff: 7, prob: 58%
			// connections: 4, diff: 8, prob: 41%
			// connections: 3, diff: 9, prob: 27%
			// connections: 2, diff: 10, prob: 16%
			// connections: 1, diff: 11, prob: 8%

			if (zone->regions.size() == 1 || roll >= diff) {
				zone->AddRegion(next);
			}
		}
	}

	ClearEmptyZones();
	ConnectZones();
} 

void MapBuilder::MergeZoneInto(Zone* src, Zone* dest) {
	if (src == NULL) {
		Awrite("src cannot be null");
		exit(1);
	}
	
	if (dest == NULL) {
		Awrite("dest cannot be null");
		exit(1);
	}

	if (!src->AtBorderWith(dest)) {
		Awrite("Zones must have common border");
		exit(1);
	}

	std::unordered_set<Zone *> conn;
	for (auto &kv : src->regions) {
		auto reg = kv.second;

		auto zones = reg->GetNeihborZones();
		for (auto &z : zones) {
			conn.insert(z);
		}

		dest->AddRegion(kv.second);
	}

	src->regions.clear();
	src->neighbors.clear();

	dest->SetConnections();
	for (auto &z : conn) {
		z->SetConnections();
	}
}

void MapBuilder::SplitZone(Zone* zone) {
	if (zone == NULL) {
		Awrite("Cannot split NULL zone");
		exit(1);
	}

	auto blob = zone->TraverseRegions();
	while (zone->regions.size() != blob.size()) {
		Zone* newZone = CreateZone(zone->type);
		
		for (auto &region : blob) {
			zone->RemoveRegion(region);
			newZone->AddRegion(region);
		}

		for (auto &z : zone->neighbors) {
			z.second->SetConnections();
		}

		zone->SetConnections();
		newZone->SetConnections();

		blob = zone->TraverseRegions();
	}
}

Zone* FindConnectedContinent(Zone* zone) {
	Awrite("Searching for continent");
	for (auto &kv : zone->neighbors) {
		auto n = kv.second;
		if (n->type == ZoneType::CONTINENT) {
			return n;
		}
	}

	return NULL;
}

std::vector<ZoneRegion *> FindBorderRegions(Zone* zone, Zone* borderZone, const int depth, const bool useRandom = false) {
	std::unordered_set<ZoneRegion *> visited;

	const int diff = 3;

	std::vector<ZoneRegion *> v;
	for (int i = 0; i < depth; i++) {
		if (i == 0) {
			for (auto &kv : zone->regions) {
				auto &region = kv.second;
				if (!region->HaveBorderWith(borderZone)) continue;

				if (useRandom && depth == 1) {
					int roll = makeRoll(1, 5);
					if (roll < diff) continue;
				}
				
				visited.insert(region);
			}

			continue;
		}

		v.clear();
		for (auto &region : visited) {
			for (auto &next : region->neighbors) {
				if (next == NULL) continue;
				if (next->zone != zone) continue;
				if (visited.find(next) != visited.end()) continue;

				if (i == depth - 1) {
					int roll = makeRoll(1, 5);
					if (roll < diff) continue;
				}
				
				v.push_back(region);
			}
		}

		for (auto &region : v) {
			visited.insert(region);
		}
	}

	std::vector<ZoneRegion *> list;
	list.reserve(visited.size());
	for (auto &region : visited) {
		list.push_back(region);
	}

	return list;
}

void MapBuilder::SpecializeZones(size_t continents, int continentAreaFraction) {
	Awrite("Specialize zones");

	int maxArea = (this->w * this->h * continentAreaFraction) / 200;
	int attempts = 0;

	Awrite("Place continent cores");
	std::vector<Zone *> cores;
	attempts = 0;
	while (attempts++ < 1000 && cores.size() != continents) {
		Zone* candidate = this->zones[getrandom(this->zones.size())];

		for (auto &core : cores) {
			if (core == candidate || core->AtBorderWith(candidate)
				|| core->location.Distance(candidate->location) < 8) {
				candidate = NULL;
				break;
			}
		}

		if (candidate != NULL) {
			cores.push_back(candidate);
			attempts = 0;
		}
	}

	int coveredArea = 0;
	std::vector<Zone *> S;
	for (auto &zone : cores) {
		zone->type = ZoneType::CONTINENT;
		coveredArea += zone->regions.size();
		S.push_back(zone);
	}

	Awrite("Grow continents");
	attempts = 0;
	std::vector<Zone *> next;
	while (attempts++ < 10000 && coveredArea <= maxArea && S.size() > 0) {
		int i = getrandom(S.size());
		Zone* zone = S[i];

		if (zone->regions.size() > maxContinentArea) {
			S.erase(S.begin() + i);
			continue;
		}

		next.clear();
		for (auto &nz : zone->neighbors) {
			Zone* nextZone = nz.second;
			if (nextZone->type != ZoneType::UNDECIDED) continue;
			next.push_back(nextZone);
		}

		if (next.size() == 0) {
			S.erase(S.begin() + i);
			continue;
		}

		Zone* target = next[getrandom(next.size())];

		coveredArea += target->regions.size();
		MergeZoneInto(target, zone);
	}

	ClearEmptyZones();

	std::vector<Zone *> oceans;
	for (auto &kv : this->zones) {
		auto zone = kv.second;
		if (zone->regions.size() != 0 && zone->type == ZoneType::UNDECIDED) {
			oceans.push_back(zone);
		}
	}

	Awrite("Add gaps between continents");
	Zone* nonIsland = GetNotIsland();
	while (nonIsland != NULL) {
		Awrite("Starting border cleanup ");
		Zone* otherZone = FindConnectedContinent(nonIsland);

		int depthRoll = makeRoll(1, this->gapMax - this->gapMin) + this->gapMin;
		int randomRoll = makeRoll(1, 2);
		int sideRoll = makeRoll(1, 2);

		int sideADepth;
		int sideBDepth;
		if (sideRoll == 1) {
			sideADepth = depthRoll / 2;
			sideBDepth = depthRoll - sideADepth;
		}
		else {
			sideBDepth = depthRoll / 2;
			sideADepth = depthRoll - sideBDepth;
		}

		bool sideARandom = false;
		bool sideBRandom = false;
		if (depthRoll == 1) {
			if (sideADepth == 0) {
				sideADepth = 1;
				sideARandom = true;	
			}
			else {
				sideBDepth = 1;
				sideBRandom = true;	
			}
		}
		else if (depthRoll == 2) {
			sideBRandom = depthRoll == 2 && randomRoll == 1;
			sideARandom = depthRoll == 2 && randomRoll == 2;
		}

		// find regions that are on border
		Zone* strait = CreateZone(ZoneType::STRAIT);
		std::list<ZoneRegion *> list;
		if (sideADepth) {
			auto tmp = FindBorderRegions(nonIsland, otherZone, sideADepth, sideARandom);
			for (auto &r : tmp) {
				list.push_back(r);
			}
		}

		if (sideBDepth) {
			auto tmp = FindBorderRegions(otherZone, nonIsland, sideBDepth, sideBRandom);
			for (auto &r : tmp) {
				list.push_back(r);
			}
		}

		for (auto &r : list) {
			nonIsland->RemoveRegion(r);
			otherZone->RemoveRegion(r);
			strait->AddRegion(r);
		}
		
		SplitZone(strait);

		if (sideADepth > 0) SplitZone(nonIsland);
		if (sideBDepth > 0) SplitZone(otherZone);

		ConnectZones();
		nonIsland = GetNotIsland();
	}

	ClearEmptyZones();

	Awrite("Grow oceans");
	int oceanCores = std::max(1, (int) oceans.size() / 4);

	// C++17 technically no longer has std::random_shuffle.  It's been replaced with std::shuffle and some boilerplate
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(oceans.begin(), oceans.end(), g);

	oceans.resize(oceanCores);
	for (auto &zone : oceans) {
		zone->type = ZoneType::OCEAN;
	}

	attempts = 0;
	while (oceans.size() > 0) {
		int i = getrandom(oceans.size());
		Zone* ocean = oceans[i];

		std::vector<Zone *> next;
		for (auto &kv : ocean->neighbors) {
			auto zone = kv.second;
			if (zone->type == ZoneType::UNDECIDED) {
				next.push_back(zone);
			}
		}

		if (next.size() > 0) {
			MergeZoneInto(next[getrandom(next.size())], ocean);
		}
		else {
			oceans.erase(oceans.begin() + i);
		}
	}

	ClearEmptyZones();
	ConnectZones();


	Awrite("Cleanup oceans, islands and straits");

	std::stack<Zone *> toCleanUp;
	for (auto &kv : this->zones) {
		auto zone = kv.second;
		int size = zone->regions.size();

		switch (zone->type) {
			case ZoneType::CONTINENT:
			case ZoneType::OCEAN:
				if (size <= 3) toCleanUp.push(zone);
				break;
			case ZoneType::STRAIT:
				toCleanUp.push(zone);
				break;
			case ZoneType::UNDECIDED:
				Awrite("An unexpected situation occured.  zone type was UNDECIDED. Ignoring/skipping it.");
				break;
		}
	}

	while (!toCleanUp.empty()) {
		Zone* src = toCleanUp.top();
		size_t size = src->regions.size();

		toCleanUp.pop();

		if (size == 0) continue;

		switch (src->type) {
			case ZoneType::CONTINENT: {
				for (auto &kv : src->neighbors) {
					auto n = kv.second;
					if (n->type == ZoneType::CONTINENT) continue;

					// merge into nearest ocean or strait
					MergeZoneInto(src, n);
					break;
				}
				break;
			}

			case ZoneType::OCEAN: {
				size_t roll = makeRoll(3, 12);
				if (size > roll) break;

				for (auto &kv : src->neighbors) {
					auto n = kv.second;
					if (n->type != ZoneType::OCEAN) continue;

					// merge into nearest ocean
					MergeZoneInto(src, n);
					break;
				}

				break;
			}

			case ZoneType::STRAIT: {
				size_t maxStraitSize = 12 + makeRoll(1, 6) - 3;
				if (size > maxStraitSize) continue;

				std::vector<Zone *> otherStraits;
				for (auto &kv : src->neighbors) {
					auto n = kv.second;
					if (n->type != ZoneType::STRAIT) continue;
					if (n->regions.size() > maxStraitSize) continue;

					// merge surrounding straits into src
					otherStraits.push_back(n);
				}

				for (auto &n : otherStraits) {
					if (src->regions.size() > maxStraitSize) break;

					MergeZoneInto(n, src);
				}

				break;
			}

			case ZoneType::UNDECIDED: {
				Awrite("An unexpected situation occured.  zone type was UNDECIDED. Ignoring/skipping it.");
				break;
			}
		}
	}

	ClearEmptyZones();

	// make straits surounded only by water to be ZoneType::OCEAN
	for (auto &kv : this->zones) {
		auto zone = kv.second;
		if (zone->type != ZoneType::STRAIT) continue;

		bool skip = false;
		for (auto &n : zone->neighbors) {
			if (n.second->type != ZoneType::STRAIT && n.second->type != ZoneType::OCEAN) {
				skip = true;
				break;
			}
		}

		if (skip) continue;

		zone->type = ZoneType::OCEAN;
	}
}

const int BIOME_ZONES = 4;
const int MAX_BIOMES = 8;
const int BIOMES[BIOME_ZONES][MAX_BIOMES] = {
	// 0: Arctic regions
	{ 4, R_TUNDRA, R_MOUNTAIN, R_FOREST, R_PLAIN, -1, -1, -1 },

	// 1: Colder regions
	{ 5, R_TUNDRA, R_MOUNTAIN, R_FOREST, R_PLAIN, R_SWAMP, -1, -1 },

	// 2: Warmer regions
	{ 7, R_TUNDRA, R_MOUNTAIN, R_FOREST, R_PLAIN, R_SWAMP, R_JUNGLE, R_DESERT },

	// 3: Tropical regions
	{ 5, R_MOUNTAIN, R_PLAIN, R_SWAMP, R_JUNGLE, R_DESERT, -1, -1 }
};

void MapBuilder::AddVolcanoes() {
	Awrite("Adding volcanoes");
	// volcanos will be added anywhere

	size_t count = makeRoll(1, this->volcanoesMax - this->volcanoesMin) + this->volcanoesMin;
	int distance = (std::min(this->w, this->h / 2) * 2) / count + 2;

	int cols = count / makeRoll(1, 4) + 1;
	int rows = count / cols + 1;
	int dX = this->w / cols;
	int dY = this->h / rows;

	std::vector<ZoneRegion *> candidates;
	std::vector<Coords> volcanoes;
	volcanoes.reserve(cols * rows);

	for (int c = 0; c < cols && volcanoes.size() < count; c++) {
		for (int r = 0; r < rows && volcanoes.size() < count; r++) {
			// if (makeRoll(1, 3) <= 1) continue;

			int attempts = 1000;
			while (attempts-- > 0) {
				int x = c * dX + getrandom(dX);
				int y = r * dY + getrandom(dY);
				if ((x + y) % 2) continue;

				attempts = 0;

				ZoneRegion* seed = this->GetRegion(x, y);
				for (size_t i = 0; i < volcanoes.size(); i++) {
					int d = volcanoes[i].Distance(seed->location);
					if (d < distance) {
						seed = NULL;
						break;
					}
				}
				if (seed == NULL) continue;

				volcanoes.push_back(seed->location);

				int volcano = makeRoll(1, 3) - 1;
				int mountains = makeRoll(2, 3);

				Zone* zone = this->CreateZone(ZoneType::CONTINENT);
				seed->biome = R_VOLCANO;
				seed->region->type = R_VOLCANO;
				seed->region->wages = AGetName(0, seed->region);

				seed->zone->RemoveRegion(seed);
				zone->AddRegion(seed);

				while (volcano > 0 || mountains > 0) {
					candidates.clear();
					for (auto &kv : zone->regions) {
						auto region = kv.second;
						
						if (region->IsInner()) continue;

						for (int i = 0; i < NDIRS; i++) {
							auto n = region->neighbors[i];
							if (n == NULL) continue;
							if (n->zone == zone) continue;

							candidates.push_back(n);
						}
					}

					size_t regionSize = zone->regions.size();
					for (int i = 0; regionSize == zone->regions.size(); i++) {
						auto &next = candidates[i % candidates.size()];
						int neigbors = next->CountNeighbors(zone);

						// 2d6
						int roll = makeRoll(2, 6);
						int diff = 12 - neigbors;
						if (regionSize == 1 || candidates.size() == 1) {
							diff = 0;
						}

						// connections: 5, diff: 7, prob: 58%
						// connections: 4, diff: 8, prob: 41%
						// connections: 3, diff: 9, prob: 27%
						// connections: 2, diff: 10, prob: 16%
						// connections: 1, diff: 11, prob: 8%

						if (roll >= diff) {
							if (volcano > 0) {
								next->biome = R_VOLCANO;
								next->region->type = R_VOLCANO;
								next->region->wages = seed->region->wages;

								volcano--;
							}
							else {
								next->biome = R_MOUNTAIN;
								next->region->type = R_MOUNTAIN;
								next->region->wages = seed->region->wages;

								mountains--;
							}

							next->zone->RemoveRegion(next);
							zone->AddRegion(next);
						}
					}
				}
			}
		}
	}

	this->ConnectZones();
}

void MapBuilder::AddLakes() {
	Awrite("Adding lakes");
	// lakes can appear only on the land, not near water and not on the mountains

	int attempts = 1000;
	int count = makeRoll(1, this->lakesMax - this->lakesMin) + this->lakesMin;
	int distance = (std::min(this->w, this->h / 2) * 2) / count;

	std::vector<Coords> lakes;
	lakes.reserve(count);

	while (attempts-- > 0 && count > 0) {
		int x = getrandom(this->w);
		int y = getrandom(this->h);
		if ((x + y) % 2) continue;

		ZoneRegion* lake = this->GetRegion(x, y);
		if (lake->zone->type != ZoneType::CONTINENT) continue;
		if (!lake->IsInner()) continue;
		if (lake->biome == R_VOLCANO || lake->biome == R_MOUNTAIN) continue;

		for (size_t i = 0; i < lakes.size(); i++) {
			int d = lakes[i].Distance(lake->location);
			if (d < distance) {
				lake = NULL;
				break;
			}
		}

		if (lake == NULL) continue;

		lakes.push_back(lake->location);

		lake->biome = R_LAKE;
		lake->region->type = R_LAKE;
		lake->region->wages = AGetName(0, lake->region);

		count--;
	}
}

void MapBuilder::GrowTerrain() {
	for (auto &kv : this->zones) {
		auto zone = kv.second;
		if (zone->type == ZoneType::OCEAN || zone->type == ZoneType::STRAIT || zone->type == ZoneType::UNDECIDED) {
			Awrite("Grow ocean");
			if (zone->type == ZoneType::UNDECIDED) {
				zone->type = ZoneType::OCEAN;
			}

			for (auto &reg : zone->regions) {
				reg.second->biome = R_OCEAN;
				reg.second->region->type = R_OCEAN;
			}
		}

		if (zone->type == ZoneType::CONTINENT) {
			this->GrowLandInZone(zone);
		}
	}

	this->AddVolcanoes();
	this->AddLakes();
}

void MapBuilder::GrowLandInZone(Zone* zone) {
	Awrite("Growing land in zone");

	if (!zone->CheckZoneIntegerity()) {
		Awrite("Land zone cannot be grown as it have lost integrity");
		exit(1);
	}

	// start with creating provinces
	std::vector<ZoneRegion *> candidates;
	for (auto &kv : zone->regions) {
		candidates.push_back(kv.second);
	}

	// get average province size for this zone
	size_t provinceSize = makeRoll(2, 4) + 6;
	size_t provinceCount = zone->regions.size() / provinceSize + 1;

	// absolute size one province cannot exceed
	size_t maxProvinceSize = zone->regions.size() / 2 + 2;

	// place province seeds so that there are at least 2 free hexes between
	std::vector<ZoneRegion *> S;
	int attempts = 0;
	while (attempts++ < 1000 && S.size() < provinceCount) {
		ZoneRegion* next = candidates[getrandom(candidates.size())];

		for (auto &seed : S) {
			if (seed->location.Distance(next->location) < 3) {
				next = NULL;
				break;
			}
		}
		if (next == NULL) continue;

		S.push_back(next);
		attempts = 0;
	}

	// assign seeds to provinces
	std::vector<Province *> provinces;
	for (auto &seed : S) {
		auto p = zone->CreateProvince(seed, this->h);
		provinces.push_back(p);
	}

	// as each islands must have min 2 biomes, then there must be min 2 provinces
	if (zone->provinces.size() < 2) {
		attempts = 0;
		while (attempts++ < 1000) {
			ZoneRegion* next = candidates[getrandom(candidates.size())];
			if (next->province != NULL) continue;
			
			provinces.push_back(zone->CreateProvince(next, this->h));
			break;
		}
	}

	// grow all provinces in random order
	while (provinces.size() > 0) {
		int i = getrandom(provinces.size());
		auto p = provinces[i];
		size_t size = p->GetSize();

		if (size >= maxProvinceSize || (size >= provinceSize + getrandom(5) - 2) || !p->Grow()) {
			provinces.erase(provinces.begin() + i);
			continue;
		}
	}

	// find "holes" and assign them to the provinces
	provinces.clear();
	for (auto &kv : zone->regions) {
		auto reg = kv.second;
		if (reg->province == NULL) {
			provinces.push_back(zone->CreateProvince(reg, this->h));
		}
	}

	while (provinces.size() > 0) {
		int i = getrandom(provinces.size());
		auto p = provinces[i];

		if (!p->Grow()) {
			provinces.erase(provinces.begin() + i);
			continue;
		}
	}

	// merge all 1 hex provinces with other provinces
	if (zone->provinces.size() > 2) {
		Province* small;
		do {
			small = NULL;
			for (auto &kv : zone->provinces) {
				auto p = kv.second;
				if (p->GetSize() > 1) continue;

				small = p;
				break;
			}

			if (small == NULL) continue;

			// find neighbor provinces
			auto tmp = small->GetNeighbors();
			Province* target = NULL;
			for (auto x : tmp) {
				if (target == NULL) {
					target = x;
					continue;
				}

				if (target->GetSize() > x->GetSize()) {
					target = x;
				}
			}

			for (auto &kv : small->regions) {
				auto reg = kv.second;
				target->AddRegion(reg);
			}
			zone->RemoveProvince(small);
		}
		while (small != NULL);
	}

	// set biomes for provinces	
	std::unordered_set<int> biomes;
	for (auto &kv : zone->provinces) {
		auto p = kv.second;

		int lat = p->GetLatitude();
		auto latBiomes = BIOMES[lat];
		int biomeCount = latBiomes[0];

		std::vector<int> weights;
		weights.resize(biomeCount);
		int w = 0;
		switch ((int) biomes.size()) {
			// biomes = 0, all weights are equal
			case 0:
				for (int i = 0; i < biomeCount; i++) {
					weights.at(i) = ++w;
				}
				break;

			// biomes = 1, all weights are equal except already present biome where weight will be 0
			case 1:
				for (int i = 0; i < biomeCount; i++) {
					int biome = latBiomes[i + 1];
					weights.at(i) = biomes.find(biome) == biomes.end()
						? ++w
						: 0;
				}
				break;

			// biomes > 2
			// neighbor province biomes: 1 (1/7)
			// already present biomeson the islands: 2 (2/7)
			// missing biome: 4 (4/7)
			default:
				auto nb = p->GetNeighborBiomes();
				for (int i = 0; i < biomeCount; i++) {
					int biome = latBiomes[i + 1];
					if (nb.find(biome) != nb.end()) {
						weights.at(i) = ++w;
					}
					else if (biomes.find(biome) != biomes.end()) {
						weights.at(i) = ++w;
						w += 1;
					}
					else {
						weights.at(i) = ++w;
						w += 3;
					}
				}
				break;
		}

		int roll = makeRoll(1, w);
		for (int i = biomeCount - 1; i >= 0; i--) {
			int diff = weights.at(i);
			if (diff == 0) continue;
			if (roll >= diff) {
				int biome = latBiomes[i + 1];
				p->biome = biome;
				biomes.insert(biome);
				break;
			}
		}
	}

	// set biomes for regions
	for (auto &kv : zone->provinces) {
		auto p = kv.second;

		int name = -1;
		for (auto &rkv : p->regions) {
			auto reg = rkv.second;

			reg->biome = p->biome;
			reg->region->type = p->biome;

			if (name == -1) {
				name = AGetName(0, reg->region);
			}

			reg->region->wages = name;
		}
	}
}

void MapBuilder::SetOceanNames() {
	for (auto &kv : this->zones) {
		auto ocean = kv.second;
		if (ocean->type == ZoneType::OCEAN || ocean->type == ZoneType::STRAIT) {
			int nameIndex = AGetName(0, ocean->regions.begin()->second->region);
			std::string name = AGetNameString(nameIndex);
			name = ocean->type == ZoneType::OCEAN
				? name + " Sea"
				: name + " Strait";

			for (auto &reg : ocean->regions) {
				reg.second->region->SetName(name.c_str());
			}
		}
	}
}

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

// void ARegionList::CreateConstrainedSurfaceLevel(int level, int xSize, int ySize, char const *name, int contients,
// 	int landMass, int maxContinentSize,
// 		int gapMin,
// 		int gapMax,
// 		int volcanoesMin,
// 		int volcanoesMax,
// 		int lakesMin,
// 		int lakesMax
// 	) {
// 	if (Globals->ICOSAHEDRAL_WORLD) {
// 		// this is not supported
// 		throw "Icosahedral maps are not supported";
// 	}

// 	MakeRegions(level, xSize, ySize);
// 	pRegionArrays[level]->SetName(name);
// 	pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;

// 	int area = xSize * ySize / 2;
// 	const int radius = 1;

// 	MapBuilder* builder = new MapBuilder(pRegionArrays[level]);
// 	builder->maxContinentArea = maxContinentSize;
// 	builder->maxZones = EstimateMaxZones(area, radius);
// 	builder->gapMin = gapMin;
// 	builder->gapMax = gapMax;
// 	builder->volcanoesMin = volcanoesMin;
// 	builder->volcanoesMax = volcanoesMax;
// 	builder->lakesMin = lakesMin;
// 	builder->lakesMax = lakesMax;;

// 	builder->CreateZones(radius + 2, 1000);
// 	builder->GrowZones();
// 	builder->SpecializeZones(contients, landMass);
// 	builder->GrowTerrain();

// 	// SeverLandBridges(pRegionArrays[level]);

// 	if (Globals->LAKES) RemoveCoastalLakes(pRegionArrays[level]);
// 	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

// 	FinalSetup(pRegionArrays[level]);

// 	builder->SetOceanNames();

// 	/////

// 	int continents = 0;
// 	int continentArea = 0;
// 	int waterArea = 0;
// 	for (auto &kv : builder->zones) {
// 		auto zone = kv.second;
// 		if (zone->type == ZoneType::CONTINENT) {
// 			continentArea += zone->regions.size();
// 			continents++;
// 		}
// 		else {
// 			waterArea += zone->regions.size();
// 		}
// 	}

// 	Awrite(AString("Zones: ") + (int) builder->zones.size());
// 	Awrite(AString("Continents: ") + continents);
// 	Awrite(AString("Contient area: ") + continentArea);
// 	Awrite(AString("Water area: ") + waterArea);

// 	delete builder;
// }

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

	if (Globals->LAKES) RemoveCoastalLakes(pRegionArrays[level]);
	
	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateIslandRingLevel(int level, int xSize, int ySize, char const *name)
{
	MakeRegions(level, xSize, ySize);
	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_SURFACE;

	MakeRingLand(pRegionArrays[level], 12, 20);

	CleanUpWater(pRegionArrays[level]);

	SetupAnchors(pRegionArrays[level]);

	GrowTerrain(pRegionArrays[level], 0);

	AssignTypes(pRegionArrays[level]);

	PlaceVolcanos(pRegionArrays[level]);

	SeverLandBridges(pRegionArrays[level]);

	if (Globals->LAKES) RemoveCoastalLakes(pRegionArrays[level]);
	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	// Put the altars in the ring around the center
	ARegion *center = pRegionArrays[level]->GetRegion(xSize/2, ySize/2);
	for (int i = 0; i < NDIRS; i++) {
		ARegion *n = center->neighbors[i];
		if (n) {
			Object *o = new Object(n);
			o->num = n->buildingseq++;
			string altar_name = string(ObjectDefs[O_RITUAL_ALTAR].name) + " [" + to_string(o->num) + "]";
			o->name = new AString(altar_name);
			o->type = O_RITUAL_ALTAR;
			n->objects.Add(o);
		}
	}
	FinalSetup(pRegionArrays[level]);
}

void ARegionList::CreateUnderworldRingLevel(int level, int xSize, int ySize, char const *name)
{
	if (Globals->ICOSAHEDRAL_WORLD) {
		MakeIcosahedralRegions(level, xSize, ySize);
	} else {
		MakeRegions(level, xSize, ySize);
	}

	pRegionArrays[level]->SetName(name);
	pRegionArrays[level]->levelType = ARegionArray::LEVEL_UNDERWORLD;

	ARegion *reg = pRegionArrays[level]->GetRegion(xSize/2, ySize/2);
	reg->type = R_BARREN;

	// Break all connections between barrens and surrounding hexes
	for (int i = 0; i < NDIRS; i++) {
		ARegion *n = reg->neighbors[i];
		if (n) {
			reg->neighbors[i] = nullptr;
			n->neighbors[reg->GetRealDirComp(i)] = nullptr;
		}
	}

	SetRegTypes(pRegionArrays[level], R_NUM);

	SetupAnchors(pRegionArrays[level]);

	GrowTerrain(pRegionArrays[level], 1);

	AssignTypes(pRegionArrays[level]);

	MakeUWMaze(pRegionArrays[level]);

	if (Globals->LAKES) RemoveCoastalLakes(pRegionArrays[level]);

	if (Globals->GROW_RACES) GrowRaces(pRegionArrays[level]);

	// Connect the center region to the surface via a shaft
	ARegionArray *surface = pRegionArrays[level-1];
	ARegion *center = surface->GetRegion(surface->x/2, surface->y/2);
	Object *o = new Object(reg);
	o->num = reg->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = center->num;
	reg->objects.Add(o);

	o = new Object(center);
	o->num = center->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = reg->num;
	center->objects.Add(o);

	// Put the monolith in the underworld center
	o = new Object(reg);
	o->num = reg->buildingseq++;
	string monolith_name = string(ObjectDefs[O_DORMANT_MONOLITH].name) + " [" + to_string(o->num) + "]";
	o->name = new AString(monolith_name);
	o->type = O_DORMANT_MONOLITH;
	reg->objects.Add(o);

	FinalSetup(pRegionArrays[level]);

}

void ARegionList::CreateUnderworldLevel(int level, int xSize, int ySize, char const *name)
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

	if (Globals->LAKES) RemoveCoastalLakes(pRegionArrays[level]);
	
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

	if (Globals->LAKES) RemoveCoastalLakes(pRegionArrays[level]);
	
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
				
				reg->level = arr;
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
					direc = getrandom(NDIRS);
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
						while (direc == reg->GetRealDirComp(newdir)) {
							newdir = getrandom(NDIRS);
						}
						newreg = reg->neighbors[newdir];
						while ((!newreg) && (tries < 36)) {
							while (direc == reg->GetRealDirComp(newdir)) {
								newdir = getrandom(NDIRS);
							}
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
				if ((reg->yloc < yoff*2) && ((dir < 2) || (dir == (NDIRS-1)))
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

void ARegionList::MakeRingLand(ARegionArray *pRegs, int minDistance, int maxDistance) {
	ARegion *center = pRegs->GetRegion(pRegs->x / 2, pRegs->y / 2);
	if (!center) { throw "Center region not found"; }

	Awrite("Making land in ring");
	for (int i = 0; i < pRegs->x; i++) {
		for (int j = 0; j < pRegs->y; j++) {
			ARegion *reg = pRegs->GetRegion(i, j);
			if (!reg) continue;
			// By default, everything is ocean
			reg->type = R_OCEAN;

			// If the regions is between minDistance and maxDistance from the center, 65% chance of being land.
			int distance = GetPlanarDistance(center, reg, 1000, -1);
			bool awayFromEdge = i >= 4 && i <= pRegs->x - 4 && j >= 4 && j <= pRegs->y - 4;
			if (distance >= minDistance && distance <= maxDistance && getrandom(100) < 60) {
				reg->type = R_NUM;
			} else if (distance >= maxDistance && awayFromEdge && getrandom(100) < 20) {
				MakeOneIsland(pRegs, i, j);
			} else if (distance < minDistance && distance > 4 && getrandom(100) < 15) {
				MakeOneIsland(pRegs, i, j);
			}
		}
	}

	Awrite("Perturbing the coastlines");
	for (int iter = 0; iter < 10; iter++) {
		for (int i = 0; i < pRegs->x; i++) {
			for (int j = 0; j < pRegs->y; j++) {
				ARegion *reg = pRegs->GetRegion(i, j);
				if (!reg) continue;
				int distance = GetPlanarDistance(center, reg, 1000, -1);
				int different = 0;
				for (int d = 0; d < NDIRS; d++) {
					ARegion *newreg = reg->neighbors[d];
					if (!newreg) continue;
					if (newreg->type != reg->type) different++;
				}

				if (distance <= minDistance + 2 && distance > 3) {
					// inner coastline.  High chance of land becoming ocean, low chance of ocean becoming land.
					// Chance is proportional to neighbors that are different.
					if (getrandom(100) < (different * (reg->type == R_NUM ? 2 : 1))) {
						reg->wages = -2;
					}
				} else if (distance >= maxDistance - 1 && i >= 2 && i <= pRegs->x - 2 && j >= 2 && j <= pRegs->y - 2) {
					// outer coastline. Moderate chance of ocean becoming land, lower chance of land becoming ocean.
					// Chance is proportional to neighbors that are different.
					if (getrandom(100) < (different * (reg->type != R_NUM ? 3 : 2))) {
						reg->wages = -2;
					}
				}
			}
		}
		// Apply the changes
		for (int i = 0; i < pRegs->x; i++) {
			for (int j = 0; j < pRegs->y; j++) {
				ARegion *reg = pRegs->GetRegion(i, j);
				if (!reg) continue;
				// If the region changed type, swap it to the new type and clear the flag.
				if (reg->wages == -2) reg->type = (reg->type == R_OCEAN ? R_NUM : R_OCEAN);
				reg->wages = -1;
			}
		}
	}

	Awrite("Adding the barrens");
	center->type = R_BARREN;
	for (int d = 0; d < NDIRS; d++) {
		ARegion *newreg = center->neighbors[d];
		if (!newreg) continue;
		newreg->type = R_BARREN;
	}

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
							if (t->type != R_NUM && t->type != R_BARREN &&
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
			if (reg->type == R_BARREN) continue;

			for (int i=D_NORTH; i<= NDIRS; i++) {
				int count = 0;
				for (int j=D_NORTH; j< NDIRS; j++)
					if (reg->neighbors[j]) count++;
				if (count <= 1) break;

				ARegion *n = reg->neighbors[i];
				if (n) {
					if (n->type == R_BARREN) continue;
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
						int rnum = sizeof(TerrainDefs[nreg->type].coastal_races) /
							sizeof(TerrainDefs[nreg->type].coastal_races[0]);
						
						while (reg->race == -1 || (ItemDefs[reg->race].flags & ItemType::DISABLED)) {
							reg->race = TerrainDefs[nreg->type].coastal_races[getrandom(rnum)];
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
				int rnum = sizeof(TerrainDefs[reg->type].races)/sizeof(TerrainDefs[reg->type].races[0]);
				
				while ( reg->race == -1 || (ItemDefs[reg->race].flags & ItemType::DISABLED)) {
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
						sizeof(TerrainDefs[reg->type].coastal_races[0]);
					for (int i=0; i<cnum; i++) {
						if (reg->race == TerrainDefs[reg->type].coastal_races[i])
							iscoastal = 1;
					}
					// Only coastal races may pass from sea to land
					if ((TerrainDefs[nreg->type].similar_type == R_OCEAN) && (!iscoastal)) continue;

					int ch = getrandom(5);
					if (iscoastal) {
						if (TerrainDefs[nreg->type].similar_type == R_OCEAN)
							ch += 2;
					} else {
						ManType *mt = FindRace(ItemDefs[reg->race].abr);
						if (mt->terrain==TerrainDefs[nreg->type].similar_type)
							ch += 2;
						int rnum = sizeof(TerrainDefs[nreg->type].races) / sizeof(TerrainDefs[nreg->type].races[0]);
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

			if ((TerrainDefs[reg->type].similar_type == R_OCEAN) && (reg->type != R_LAKE)) {
				if (pArr->levelType == ARegionArray::LEVEL_UNDERWORLD) {
					reg->SetName("The Undersea");
				}
				else if (pArr->levelType == ARegionArray::LEVEL_UNDERDEEP) {
					reg->SetName("The Deep Undersea");
				}
				else {
					AString ocean_name = Globals->WORLD_NAME;
					ocean_name += " Ocean";
					reg->SetName(ocean_name.Str());
				}
			} else {
				if (reg->wages == -1)
					reg->SetName("The Void");
				else if (reg->wages != -2)
					reg->SetName(AGetNameString(reg->wages));
				else
					reg->wages = -1;
			}

			reg->Setup();
		}
	}
}

void ARegionList::MakeShaft(ARegion *reg, ARegionArray *pFrom, ARegionArray *pTo)
{
	if (TerrainDefs[reg->type].similar_type == R_OCEAN) return;
	if (reg->type == R_BARREN) return;

	int tempx = reg->xloc * pTo->x / pFrom->x + getrandom(pTo->x / pFrom->x);
	int tempy = reg->yloc * pTo->y / pFrom->y + getrandom(pTo->y / pFrom->y);
	//
	// Make sure we get a valid region.
	//
	tempy += (tempx + tempy) % 2;

	ARegion *temp = pTo->GetRegion(tempx, tempy);
	if (!temp) return;
	if (TerrainDefs[temp->type].similar_type == R_OCEAN) return;
	if (temp->type == R_BARREN) return;

	Object *o = new Object(reg);
	o->num = reg->buildingseq++;
	o->name = new AString(AString("Shaft [") + o->num + "]");
	o->type = O_SHAFT;
	o->incomplete = 0;
	o->inner = temp->num;
	reg->objects.Add(o);

	o = new Object(temp);
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
				// The previous code would *always* choose the northernmost and westernmost location
				// of the specific terrain.  The new algorithm is going to be to
				//
				// 1. collect all the candidates of each terrain type
				//    a. a hex is a candidate if it is close (within 2hex) of a hex containing each of the following
				//       1. wood
				//       2. iron
				//	     3. stone
				//       4. food (grain or livestock)
				//       5. mounts (horse or camel)
				// 2. choose a random candidate from each set of valid candidates.
				// Hopefully we won't have a situation where the is no viable candidate.  If we do, we will error out.

				// First, we need to find the candidates
				ARegionArray *to = GetRegionArray(levelTo);
				std::map<int, std::vector<ARegion *> > candidates;
				for (int type = R_PLAIN; type <= R_TUNDRA; type++) {
					candidates[type] = to->get_starting_region_candidates(type);
				}
				// Now for each type, choose a random candidate
				std::mt19937 gen{std::random_device{}()}; // generates random numbers
				std::map<int, ARegion *> dests;
				for (int type = R_PLAIN; type <= R_TUNDRA; type++) {
					if (candidates[type].size() == 0) {
						cout << "Error: No valid candidate found for gateway to " << TerrainDefs[type].name << "\n";
						exit(1);
					}
					Awrite("Found " + to_string(candidates[type].size()) + " candidates for " +
						TerrainDefs[type].name + " gateway");
					std::uniform_int_distribution<std::size_t> dist(0, candidates[type].size() - 1);
					int index = dist(gen);
					ARegion *dest = candidates[type][index];
					ARegion *best = dest;
					int tries = 50;
					int maxMin = -1;
					while(tries--) {
						// See if it's too close to any existing dest
						int minDist = 1000;
						for (int otherTypes = R_PLAIN; otherTypes < type; otherTypes++) {
							ARegion *otherDest = dests[otherTypes];
							int curDist = GetPlanarDistance(dest, otherDest, 0);
							if (curDist < minDist) minDist = curDist;
						}
						if (minDist > 20) break;
						if (minDist > maxMin) {
							maxMin = minDist;
							best = dest;
						}
						// too close, try again
						index = dist(gen);
						dest = candidates[type][index];
					}
					// store the best we have so far
					Awrite("Best distance of " + to_string(maxMin) + " for " + TerrainDefs[type].name);
					dests[type] = best;
				}

				for (int type = R_PLAIN; type <= R_TUNDRA; type++) {
					Object *o = new Object(AC);
					o->num = AC->buildingseq++;
					o->name = new AString(AString("Gateway to ") + TerrainDefs[type].name + " [" + o->num + "]");
					o->type = O_GATEWAY;
					o->incomplete = 0;
					o->inner = dests[type]->num;
					AC->objects.Add(o);
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
						temp->type != R_VOLCANO && temp->type != R_BARREN &&
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
	ARegion *r, *head, *tail, *neighbors[NDIRS], *n;
	int attempts, max, i, j, count, offset, x, y, xscale, yscale;
	Object *o;

	forlist(this) {
		r = (ARegion *) elem;
		r->distance = -1;
		r->next = 0;
	}

	// Build a list of all the regions we know we can get to:
	// The nexus and anywhere that has a gate
	head = 0;
	tail = 0;
	forlist_reuse(this) {
		r = (ARegion *) elem;
		if (r->zloc == ARegionArray::LEVEL_NEXUS || r->gate == -1) {
			r->distance = 0;
			r->next = head;
			head = r;
			if (!tail)
				tail = r;
		}
	}
	while (head) {
		tail = FindConnectedRegions(head, tail, 1);
		head = head->next;
	}
	attempts = 0;
	do {
		max = 0;
		count = 0;
		forlist(this) {
			r = (ARegion *) elem;
			if (r->distance == -1) {
				count++;
			}
			if (r->distance > max)
				max = r->distance;
		}
		if (count > 0) {
			i = getrandom(count);
			forlist(this) {
				r = (ARegion *) elem;
				if (r->distance == -1) {
					if (!i)
						break;
					i--;
				}
			}
			// Found an unconnected region
			// Try to link it in
			n = 0;
			// first, see if we can knock down a wall
			// sadly we can only knock down all the walls at once
			for (i = 0; i < NDIRS; i++)
				neighbors[i] = r->neighbors[i];
			if (Globals->ICOSAHEDRAL_WORLD) {
				IcosahedralNeighSetup(r, pRegionArrays[r->zloc]);
			} else {
				NeighSetup(r, pRegionArrays[r->zloc]);
			}
			offset = getrandom(NDIRS);
			for (i = 0; i < NDIRS; i++) {
				if (r->neighbors[(i + offset) % NDIRS] &&
						r->neighbors[(i + offset) % NDIRS]->distance != -1) {
					break;
				}
			}
			for (j = 0; j < NDIRS; j++) {
				// restore all the walls other than the one
				// we meant to break
				if (i != j)
					r->neighbors[(j + offset) % NDIRS] = neighbors[(j + offset) % NDIRS];
			}
			if (i < NDIRS) {
				// also restore the link on the other side
				n = r->neighbors[(i + offset) % NDIRS];
				for (j = 0; j < NDIRS; j++)
					neighbors[j] = n->neighbors[j];
				if (Globals->ICOSAHEDRAL_WORLD) {
					IcosahedralNeighSetup(n, pRegionArrays[r->zloc]);
				} else {
					NeighSetup(n, pRegionArrays[n->zloc]);
				}
				for (j = 0; j < NDIRS; j++)
					if (n->neighbors[j] != r)
						n->neighbors[j] = neighbors[j];
			} else if (TerrainDefs[r->type].similar_type != R_OCEAN) {
				// couldn't break a wall
				// so try to put in a shaft
				if (r->zloc > ARegionArray::LEVEL_SURFACE) {
					x = r->xloc * GetLevelXScale(r->zloc) / GetLevelXScale(r->zloc - 1);
					y = r->yloc * GetLevelYScale(r->zloc) / GetLevelYScale(r->zloc - 1);
					xscale = GetLevelXScale(r->zloc) / GetLevelXScale(r->zloc - 1);
					yscale = 2 * GetLevelYScale(r->zloc) / GetLevelYScale(r->zloc - 1);
					for (i = 0; !n && i < xscale; i++)
						for (j = 0; !n && j < yscale; j++) {
							n = pRegionArrays[r->zloc - 1]->GetRegion(x + i, y + j);
							if (n && TerrainDefs[n->type].similar_type == R_OCEAN)
								n = 0;
						}
					if (n) {
						o = new Object(n);
						o->num = n->buildingseq++;
						o->name = new AString(AString("Shaft [") + o->num + "]");
						o->type = O_SHAFT;
						o->incomplete = 0;
						o->inner = r->num;
						n->objects.Add(o);

						o = new Object(r);
						o->num = r->buildingseq++;
						o->name = new AString(AString("Shaft [") + o->num + "]");
						o->type = O_SHAFT;
						o->incomplete = 0;
						o->inner = n->num;
						r->objects.Add(o);
					}
				}
				if (!n) {
					// None of that worked
					// can we put in a gate?
					if (Globals->GATES_EXIST &&
							!getrandom(10)) {
						r->gate = -1;
						r->distance = 0;
						n = r;
					}
				}
			}
			if (n) {
				head = n;
				head->next = 0;
				tail = head;
				while (head) {
					tail = FindConnectedRegions(head, tail, 1);
					head = head->next;
				}
				attempts = 0;
			}
		}
		attempts++;
	} while (count > 0 && attempts < 1000);

	if (count > 0) {
		printf("Unable to link up %d hexes!\n", count);
	}
	printf("Maximum distance from the Nexus: %d.\n", max);
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
			r->gatemonth = getrandom(12);
		}
	}
	delete[] used;
}

// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 2024 Valdis Zobēla
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

#include "aregion.h"

const RegionEdge* RegionEdge::create(const int id, ARegion* left, ARegion* right, const int left_dir, const int right_dir) {
    RegionEdge *edge = new RegionEdge(id, left, right, left_dir, right_dir);
    return edge;
}

const int RegionEdge::get_id() {
    return this->id;
}

ARegion* RegionEdge::get_left() {
    return this->left;
}

ARegion* RegionEdge::get_right() {
    return this->right;
}

const int RegionEdge::get_left_dir() {
    return this->left_dir;
}

const int RegionEdge::get_right_dir() {
    return this->right_dir;
}

void RegionEdges::add(const RegionEdges::itemType edge) {
    this->items.insert({ edge->get_id(), edge });

    add_to_cache(this->neighborsCache, edge->get_left(), edge->get_right(), edge->get_left_dir());
    add_to_cache(this->neighborsCache, edge->get_right(), edge->get_left(), edge->get_right_dir());
}

void add_to_cache(RegionEdges::cacheType& cache, const RegionEdges::regionType source, RegionEdges::regionType target, const int dir) {
    auto rec = cache.find(source);
    if (rec == cache.end()) {
        RegionEdges::regionArrayType neighbors;
        std::fill(neighbors.begin(), neighbors.end(), nullptr);

        cache.insert({ source, neighbors });
        rec = cache.find(source);
    }

    rec->second[dir] = target;
}

const RegionEdges::regionArrayType& RegionEdges::neighbors(const RegionEdges::regionType region) {
    auto rec = this->neighborsCache.find(region);
    if (rec == this->neighborsCache.end()) {
        RegionEdges::regionArrayType neighbors;
        std::fill(neighbors.begin(), neighbors.end(), nullptr);

        return neighbors;
    }

    return rec->second;
}

RegionEdges::iterator RegionEdges::begin() {
    return this->items.begin();
}

RegionEdges::iterator RegionEdges::end() {
    return this->items.end();
}

RegionEdges::regionType RegionEdges::get_neighbor(const RegionEdges::regionType region, const int dir) {
    auto rec = this->neighborsCache.find(region);
    if (rec == this->neighborsCache.end()) {
        return nullptr;
    }

    return rec->second[dir];
}

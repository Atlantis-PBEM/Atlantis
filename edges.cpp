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

////////////////////
///// RegionEdge

RegionEdge* RegionEdge::create(const int id, ARegion* left, ARegion* right, const int left_dir, const int right_dir) {
    return new RegionEdge(id, left, right, left_dir, right_dir);
}

int RegionEdge::get_id() const {
    return this->id;
}

ARegion* RegionEdge::get_left() const {
    return this->left;
}

ARegion* RegionEdge::get_right() const {
    return this->right;
}

int RegionEdge::get_left_dir() const {
    return this->left_dir;
}

int RegionEdge::get_right_dir() const {
    return this->right_dir;
}


////////////////////
///// RegionEdges

RegionEdges::RegionEdges() {
    this->owner = nullptr;
    std::fill(neighbors.begin(), neighbors.end(), nullptr);
}

RegionEdges::RegionEdges(RegionEdges::regionType owner) {
    this->owner = owner;
    std::fill(neighbors.begin(), neighbors.end(), nullptr);
}

void RegionEdges::add(const RegionEdges::itemType edge) {
    this->items.insert({ edge->get_id(), edge });

    if (edge->get_left() == this->owner) {
        this->neighbors[edge->get_left_dir()] = edge->get_right();
    } else {
        this->neighbors[edge->get_right_dir()] = edge->get_left();
    }
}

void RegionEdges::remove(const int id) {
    auto rec = this->items.find(id);
    if (rec == this->items.end()) {
        return;
    }

    auto edge = rec->second;
    if (edge->get_left() == this->owner) {
        this->neighbors[edge->get_left_dir()] = nullptr;
    } else {
        this->neighbors[edge->get_right_dir()] = nullptr;
    }

    this->items.erase(rec);
}

RegionEdges::regionType RegionEdges::get_neighbor(const int dir) {
    return this->neighbors[dir];
}

const int RegionEdges::size() const {
    return items.size();
}

RegionEdges::itemType RegionEdges::get(const int id) const {
    auto rec = this->items.find(id);
    if (rec == this->items.end()) {
        return nullptr;
    }

    return rec->second;
}

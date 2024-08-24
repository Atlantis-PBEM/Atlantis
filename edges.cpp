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

inline const int opposite_dir(const int dir) {
    return (dir + 3) % 6;
}

RegionEdge* RegionEdge::create(const int id, ARegion* left, ARegion* right, const int dir) {
    return new RegionEdge(true, id, left, right, dir, opposite_dir(dir));
}

RegionEdge* RegionEdge::create_directed(const int id, ARegion* left, ARegion* right, const int dir) {
    return new RegionEdge(false, id, left, right, dir, opposite_dir(dir));
}

// int RegionEdge::get_id() const {
//     return this->id;
// }

ARegion* RegionEdge::get_left() const {
    return this->left;
}

ARegion* RegionEdge::get_right() const {
    return this->right;
}

// int RegionEdge::get_left_dir() const {
//     return this->left_dir;
// }

// int RegionEdge::get_right_dir() const {
//     return this->right_dir;
// }

const bool RegionEdge::other_side(const RegionEdge::regionType source, RegionEdge::side &side) const {
    if (source == left) {
        side.region = right;
        side.direction = left_dir;

        return true;
    }

    if (source == right && !is_undirected) {
        side.region = left;
        side.direction = right_dir;

        return true;
    }

    return false;
}

////////////////////
///// RegionEdges

RegionEdges::RegionEdges() {
    this->owner = nullptr;
    std::fill(neighbors.begin(), neighbors.end(), nullptr);
}

RegionEdges::RegionEdges(RegionEdges::region_type owner) : RegionEdges() {
    this->owner = owner;
}

void RegionEdges::add(const RegionEdges::value_type edge) {
    this->_edges.insert({ edge->id, edge });

    RegionEdge::side side;
    if (edge->other_side(this->owner, side)) {
        this->neighbors[side.direction] = side.region;
    }
}

void RegionEdges::remove(const RegionEdges::id_type id) {
    auto rec = this->_edges.find(id);
    if (rec == this->_edges.end()) {
        return;
    }

    RegionEdge::side side;
    if (rec->second->other_side(this->owner, side)) {
        this->neighbors[side.direction] = nullptr;
    }

    this->_edges.erase(rec);
}

RegionEdges::region_type RegionEdges::get_neighbor(const int dir) const {
    return this->neighbors[dir];
}

const int RegionEdges::size() const {
    return _edges.size();
}

RegionEdges::value_type RegionEdges::get(const RegionEdges::id_type id) const {
    auto rec = this->_edges.find(id);
    if (rec == this->_edges.end()) {
        return nullptr;
    }

    return rec->second;
}

RegionEdges::value_type RegionEdges::get_edge(const int dir) const {
    for (auto& [_, edge] : this->_edges) {
        RegionEdge::side side;
        if (edge->other_side(this->owner, side) && side.direction == dir) {
            return edge;
        }
    }

    return nullptr;
}

RegionEdges::iterator RegionEdges::begin() const noexcept {
    return RegionEdges::iterator(_edges.begin());
}

RegionEdges::iterator RegionEdges::end() const noexcept {
    return RegionEdges::iterator(_edges.end());
}

RegionEdges::iterator::reference RegionEdges::iterator::operator*() const {
    return internal_iterator->second;
}

RegionEdges::iterator::pointer RegionEdges::iterator::operator&() const {
    return std::addressof(**this);
}

RegionEdges::iterator &RegionEdges::iterator::operator++() {
    ++internal_iterator;
    return *this;
}

RegionEdges::iterator RegionEdges::iterator::operator++(int) {
    const iterator ov = *this;
    ++*this;
    return ov;
}

bool RegionEdges::iterator::operator== (const RegionEdges::iterator& that) const noexcept {
    return internal_iterator == that.internal_iterator;
}

bool RegionEdges::iterator::operator!= (const RegionEdges::iterator& that) const noexcept {
    return !(*this == that);
}


////////////////////
///// GameEdges

GameEdges::~GameEdges() {
    for (auto& kv : _edges) {
        delete kv.second;
    }

    _edges.clear();
}

const std::size_t GameEdges::size() const {
    return _edges.size();
}

const GameEdges::id_type GameEdges::get_last_edge_id() const {
    return this->_last_edge_id;
}

void GameEdges::set_last_edge_id(const GameEdges::id_type id) {
    this->_last_edge_id = id;
}

GameEdges::value_type GameEdges::get(const GameEdges::id_type id) const {
    auto kv = _edges.find(id);
    if (kv == _edges.end()) {
        return nullptr;
    }

    return kv->second;
}

GameEdges::value_type GameEdges::create(ARegion *left, ARegion *right, const int dir) {
    auto edge = find(left, right);
    if (edge == nullptr) {
        edge = RegionEdge::create(this->next_edge_id(), left, right, dir);
        add(edge);
    }

    return edge;
}

GameEdges::value_type GameEdges::create_directed(ARegion *left, ARegion *right, const int dir) {
    auto edge = find(left, right);
    if (edge == nullptr) {
        edge = RegionEdge::create_directed(this->next_edge_id(), left, right, dir);
        add(edge);
    }

    return edge;
}

void GameEdges::add(GameEdges::value_type edge) {
    _edges.insert({ edge->id, edge });

    edge->get_left()->edges.add(edge);

    if (edge->is_undirected) {
        edge->get_right()->edges.add(edge);
    }
}

GameEdges::value_type GameEdges::find(const ARegion *left, const ARegion *right) const {
    for (const auto& kv : _edges) {
        auto edge = kv.second;

        if (edge->get_left() == left && edge->get_right() == right) {
            return edge;
        }

        if (edge->get_left() == right && edge->get_right() == left) {
            return edge;
        }
    }

    return nullptr;
}

const bool GameEdges::has(const ARegion *left, const ARegion *right) const {
    return find(left, right) != nullptr;
}

const bool GameEdges::remove(const GameEdges::id_type id) {
    const auto kv = _edges.find(id);
    if (kv == _edges.end()) {
        return false;
    }

    const auto edge = kv->second;
    const auto left = edge->get_left();
    const auto right = edge->get_right();

    left->edges.remove(id);
    right->edges.remove(id);

    _edges.erase(kv);
    delete edge;

    return true;
}

const bool GameEdges::remove(const RegionEdge *edge) {
    return this->remove(edge->id);
}

const GameEdges::id_type GameEdges::next_edge_id() {
    return ++this->_last_edge_id;
}


GameEdges::iterator GameEdges::begin() const noexcept {
    return GameEdges::iterator(_edges.begin());
}

GameEdges::iterator GameEdges::end() const noexcept {
    return GameEdges::iterator(_edges.end());
}

GameEdges::iterator::reference GameEdges::iterator::operator*() const {
    return internal_iterator->second;
}

GameEdges::iterator::pointer GameEdges::iterator::operator&() const {
    return std::addressof(**this);
}

GameEdges::iterator &GameEdges::iterator::operator++() {
    ++internal_iterator;
    return *this;
}

GameEdges::iterator GameEdges::iterator::operator++(int) {
    const iterator ov = *this;
    ++*this;
    return ov;
}

bool GameEdges::iterator::operator== (const GameEdges::iterator& that) const noexcept {
    return internal_iterator == that.internal_iterator;
}

bool GameEdges::iterator::operator!= (const GameEdges::iterator& that) const noexcept {
    return !(*this == that);
}

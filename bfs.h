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

#pragma once

#include <vector>
#include <queue>
#include <unordered_map>

template <typename T>
struct BFSNode {
    T* value;
    int distance;
};

template <typename T>
class BFS {
public:
    virtual ~BFS() { }

    std::unordered_map<T*, BFSNode<T>> reached;
    std::queue<BFSNode<T>> frontier;

    void addFrontier(T* value, int distance) {
        BFSNode<T> node = { value: value, distance: distance };

        frontier.push(node);
        reached.insert(std::pair<T*, BFSNode<T>>(value, node));
    }

    void markReached(T* value, int distance) {
        BFSNode<T> node = { value: value, distance: distance };

        reached.insert(std::pair<T*, BFSNode<T>>(value, node));
    }

    void search(T* start) {
        if (start != NULL) {
            // if start is null, then frontier must be initalized before calling search
            addFrontier(start, 0);
            add(start, 0);
        }

        while (!frontier.empty()) {
            auto current = frontier.front();
            frontier.pop();

            auto nodes = next(current.value);
            for (auto n : nodes) {
                auto node = reached.find(n);
                if (node != reached.end()) {
                    continue;
                }

                int distance = current.distance + 1;
                if (add(n, distance)) {
                    addFrontier(n, distance);
                }
                else {
                    // if add returnes false, we do not expand this node
                    markReached(n, distance);
                }
            }
        }
    }

protected:
    virtual std::vector<T*> next(T* node) = 0;
    virtual bool add(T* node, const int distance) const = 0;
};

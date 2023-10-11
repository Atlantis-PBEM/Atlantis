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

namespace graphs {
    inline unsigned int hash(unsigned int value) {
        value = ((value >> 16) ^ value) * 0x45d9f3b;
        value = ((value >> 16) ^ value) * 0x45d9f3b;
        value = (value >> 16) ^ value;
        
        return value;
    }

    struct Location2D {
        int x;
        int y;

        bool operator == (const Location2D& other) const {
            return x == other.x && y == other.y;
        }

        bool operator != (const Location2D& other) const {
            return !operator==(other);
        }

        bool operator > (const Location2D& b) const  {
            return std::tie(x, y) > std::tie(b.x, b.y);
        }

        bool operator < (const Location2D& b) const {
            return std::tie(x, y) < std::tie(b.x, b.y);
        }
    };

    struct Location3D {
        int x;
        int y;
        int z;

        bool operator==(const Location3D& other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        bool operator!=(const Location3D& other) const {
            return !operator==(other);
        }

        bool operator > (const Location3D& b) const  {
            return std::tie(x, y, z) > std::tie(b.x, b.y, b.z);
        }

        bool operator < (const Location3D& b) const {
            return std::tie(x, y, z) < std::tie(b.x, b.y, b.z);
        }
    };
}

namespace std {
    template <> struct hash<graphs::Location2D> {
        std::size_t operator()(const graphs::Location2D& id) const noexcept {
            int seed = 1430287;
            seed = seed * 7302013  ^ graphs::hash(id.x);
            seed = seed * 7302013  ^ graphs::hash(id.y);

            return std::hash<int>()(seed);
        }
    };

    template <> struct hash<graphs::Location3D> {
        std::size_t operator()(const graphs::Location3D& id) const noexcept {
            int seed = 1430287;
            seed = seed * 7302013  ^ graphs::hash(id.x);
            seed = seed * 7302013  ^ graphs::hash(id.y);
            seed = seed * 7302013  ^ graphs::hash(id.z);

            return std::hash<int>()(seed);
        }
    };
}

namespace graphs {
    template <class TKey, class TNode>
    class Graph {
    public:
        virtual ~Graph() { }

        virtual TNode get(TKey id) = 0;
        virtual std::vector<TKey> neighbors(TKey id) = 0;
        virtual double cost(TKey current, TKey next) = 0;

    };

    template <class T, class TPriority>
    class PriorityQueue {
    public:
        typedef std::pair<TPriority, T> Element;

        std::priority_queue<Element, std::vector<Element>, std::greater<Element>> elements;

        inline bool empty() const {
            return elements.empty();
        }

        inline void put(T item, TPriority priority) {
            elements.emplace(priority, item);
        }

        T get() {
            T bestItem = elements.top().second;
            elements.pop();

            return bestItem;
        }
    };

    template <class T>
    struct Node {
        T key;
        int distance;
    };

    template <class TLocation, class TGraph>
    std::unordered_map<TLocation, Node<TLocation>>
    breadthFirstSearch(TGraph& graph, TLocation start) {
        std::queue<Node<TLocation>> frontier;
        frontier.push({ start, 0 });

        std::unordered_map<TLocation, Node<TLocation>> cameFrom;
        cameFrom[start] = { start, 0 };

        while (!frontier.empty()) {
            Node<TLocation> current = frontier.front();
            frontier.pop();

            for (TLocation next : graph.neighbors(current.key)) {
                if (cameFrom.find(next) == cameFrom.end()) {
                    frontier.push({ next, current.distance + 1 });
                    cameFrom[next] = current;
                }
            }
        }

        return cameFrom;
    }

    template <class TLocation, class TGraph>
    std::unordered_map<TLocation, Node<TLocation>>
    breadthFirstSearch(TGraph& graph, TLocation start, TLocation goal) {
        std::queue<Node<TLocation>> frontier;
        frontier.push({ start, 0 });

        std::unordered_map<TLocation, Node<TLocation>> cameFrom;
        cameFrom[start] = { start, 0 };

        while (!frontier.empty()) {
            Node<TLocation> current = frontier.front();
            frontier.pop();

            if (current.key == goal) {
                break;
            }
            
            for (TLocation next : graph.neighbors(current.key)) {
                if (cameFrom.find(next) == cameFrom.end()) {
                    frontier.push({ next, current.distance + 1 });
                    cameFrom[next] = current;
                }
            }
        }

        return cameFrom;
    }

    template <class TLocation, class TGraph>
    std::unordered_map<TLocation, Node<TLocation>>
    breadthFirstSearch(TGraph& graph, TLocation start, int maxDistance) {
        std::queue<Node<TLocation>> frontier;
        frontier.push({ start, 0 });

        std::unordered_map<TLocation, Node<TLocation>> cameFrom;
        cameFrom[start] = { start, 0 };

        while (!frontier.empty()) {
            Node<TLocation> current = frontier.front();
            frontier.pop();

            if (current.distance > maxDistance) {
                continue;
            }

            for (TLocation next : graph.neighbors(current.key)) {
                if (cameFrom.find(next) == cameFrom.end()) {
                    frontier.push({ next, current.distance + 1 });
                    cameFrom[next] = current;
                }
            }
        }

        return cameFrom;
    }

    template <class TLocation, class TGraph>
    void dijkstraSearch(TGraph& graph, TLocation start,
        std::unordered_map<TLocation, TLocation>& cameFrom,
        std::unordered_map<TLocation, double>& costSoFar
    ) {
        PriorityQueue<TLocation, double> frontier;
        frontier.put(start, 0);

        cameFrom[start] = start;
        costSoFar[start] = 0;
        
        while (!frontier.empty()) {
            TLocation current = frontier.get();

            for (TLocation next : graph.neighbors(current)) {
                double cost = graph.cost(current, next) + 1;
                double newCost = costSoFar[current] + cost;
                if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next]) {
                    costSoFar[next] = newCost;
                    cameFrom[next] = current;
                    frontier.put(next, newCost);
                }
            }
        }
    }

    template <class TLocation, class TGraph>
    void dijkstraSearch(TGraph& graph, TLocation start, TLocation goal,
        std::unordered_map<TLocation, TLocation>& cameFrom,
        std::unordered_map<TLocation, double>& costSoFar
    ) {
        PriorityQueue<TLocation, double> frontier;
        frontier.put(start, 0);

        cameFrom[start] = start;
        costSoFar[start] = 0;
        
        while (!frontier.empty()) {
            TLocation current = frontier.get();

            if (current == goal) {
                break;
            }

            for (TLocation next : graph.neighbors(current)) {
                double cost = graph.cost(current, next);
                double newCost = costSoFar[current] + cost;
                if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next]) {
                    costSoFar[next] = newCost;
                    cameFrom[next] = current;
                    frontier.put(next, newCost);
                }
            }
        }
    }
}


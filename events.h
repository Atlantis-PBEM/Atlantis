// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 2020 Valdis Zobēla
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

#ifndef EVENTS_CLASS
#define EVENTS_CLASS

class Events;

class FactBase;
class BattleFact;

#include "unit.h"
#include <string>
#include <list>

enum EventCategory {
    EVENT_BATTLE,
    EVENT_CITY_CAPTURE,
    EVENT_MONSTER_HUNT,
    EVENT_MONSTER_AGGRESSION
};

struct Event {
    EventCategory category;
    int score;
    std::string text;
};


class FactBase {
public:
    virtual ~FactBase() = 0;
    virtual void GetEvents(std::list<Event> &events) = 0;
};

struct BattleSide {
    int factionNum;
    std::string factionName;

    int unitNum;
    std::string unitName;

    int total;

    int mages;
    int monsters;
    int undead;
    int fmi;
    
    int lost;
    
    int magesLost;
    int fmiLost;
    int undeadLost;
    int monstersLost;

    void AssignUnit(Unit* unit);
    void AssignArmy(Army* army);
};

struct EventLocation {
    int x;
    int y;
    int z;
    int terrainType;
    std::string province;
    std::string settlement;
    int settlementType;

    std::string getTerrain();
    void Assign(ARegion* region);
};

class BattleFact : public FactBase {
public:
    BattleFact();
    ~BattleFact();
    
    void GetEvents(std::list<Event> &events);

    EventLocation location;
    BattleSide attacker;
    BattleSide defender;

    int outcome;    // BATTLE_LOST, BATTLE_WON, BATTLE_DRAW
};

class Events {
public:
    Events();
    ~Events();

    std::string Write(std::string worldName, std::string month, int year);

    void AddFact(FactBase *fact);

private:
    std::list<FactBase *> facts;
};

#endif

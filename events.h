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

/// Interface for an event message
class IEvent
{
public:
	virtual ~IEvent() {}

	virtual std::string str(ARegionList &regions) const = 0;
	virtual void writeJson(JsonReport &of, ARegionList &regions) const = 0;
};

/// Just a string
class StrEvent : public IEvent
{
public:
	explicit StrEvent(const std::string &str): str_(str) {}

	std::string str(ARegionList &) const override { return str_; }

	void writeJson(JsonReport &of, ARegionList &regions) const override;

protected:
	std::string str_; ///< raw string
};

/// Unit collecting taxes in region
class TaxEvent : public IEvent
{
public:
	TaxEvent(const std::string &uname, int amt, ARegion *reg);

	std::string str(ARegionList &regions) const override;
	void writeJson(JsonReport &of, ARegionList &regions) const override;

protected:
	std::string uname_; ///< unit name
	int amt_;           ///< amount of taxes
	ARegion *reg_;      ///< region
};

/// Unit giving items to another unit
class GiveEvent : public IEvent
{
public:
	GiveEvent(const std::string &gname, int inum, int amt, const std::string &tname, bool reversed);

	std::string str(ARegionList &regions) const override;
	void writeJson(JsonReport &of, ARegionList &regions) const override;

protected:
	std::string gname_; ///< giver
	int inum_;          ///< item id
	int amt_;           ///< count
	std::string tname_; ///< target
	bool reversed_;     ///< receive instead of give
};

/// Unit producing items
class ProduceEvent : public IEvent
{
public:
	ProduceEvent(const std::string &uname, int inum, int amt, ARegion *reg);

	std::string str(ARegionList &regions) const override;
	void writeJson(JsonReport &of, ARegionList &regions) const override;

protected:
	std::string uname_; ///< producing unit
	int inum_;          ///< item index
	int amt_;           ///< amount produced
	ARegion *reg_;      ///< region produced in
};

#endif

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

#include "gameio.h"
#include "events.h"
#include <map>
#include <algorithm>
#include "gamedata.h"
#include <sstream>

FactBase::~FactBase() {

}

void BattleSide::AssignUnit(Unit* unit) {
	this->factionName = unit->faction->name().Str();
	this->factionNum = unit->faction->num;
	this->unitName = unit->name->Str();
	this->unitNum = unit->num;
}

void BattleSide::AssignArmy(Army* army) {
	this->total = army->count;

	for (int i = 0; i < army->count; i++) {
		auto soldier = army->soldiers[i];

        bool lost = soldier->hits == 0;
        if (lost) this->lost++;

        ItemType& item = ItemDefs[soldier->race];

        if (item.flags & ItemType::MANPRODUCE) {
            this->fmi++;
            if (lost) this->fmiLost++;
            continue;
        }
        
        if (item.type & IT_UNDEAD) {
            this->undead++;
            if (lost) this->undeadLost++;
            continue;
        }

        if (item.type & IT_MONSTER) {
            this->monsters++;
            if (lost) this->monstersLost++;
            continue;
        }

        auto unit = soldier->unit;
        if (unit == NULL) {
            continue;
        }

        auto type = unit->type;
        if (type == U_MAGE || type == U_GUARDMAGE || type == U_APPRENTICE) {
            this->mages++;
            if (lost) this->magesLost++;
        }
    }
}

std::string EventLocation::getTerrain() {
    auto terrainName = TerrainDefs[this->terrainType].name;
    return terrainName;
}

void EventLocation::Assign(ARegion* region) {
    this->x = region->xloc;
	this->y = region->yloc;
	this->z = region->zloc;
	this->terrainType = region->type;
	this->province = region->name->Str();
    
    if (region->town) {
        this->settlement = region->town->name->Str();
        this->settlementType = region->town->TownType();
    }
}


/////-----


Events::Events() {
}

Events::~Events() {
    for (auto &fact : this->facts) {
        delete fact;
    }

    this->facts.clear();
}

void Events::AddFact(FactBase *fact) {
    this->facts.push_back(fact);
}

bool compareEvents(const Event &first, const Event &second) {
    return first.score < second.score;
}

std::list<string> wrapText(std::string input, std::size_t width) {
    std::size_t curpos = 0;
    std::size_t nextpos = 0;

    std::list<std::string> lines;
    std::string substr = input.substr(curpos, width + 1);

    while (substr.length() == width + 1 && (nextpos = substr.rfind(' ')) != input.npos) {
        lines.push_back(input.substr(curpos, nextpos));
        
        curpos += nextpos + 1;
        substr = input.substr(curpos, width + 1);
    }

    if (curpos != input.length()) {
        lines.push_back(input.substr(curpos, input.npos));
    }

    return lines;
}

std::string makeLine(std::size_t width, bool odd, std::string text)  {
    std::string line = (odd ? "( " : " )");

    std::size_t left = (width - text.size()) / 2;

    while (line.size() < left) line += ' ';
    line += text;
    while (line.size() < (width - 2)) line += ' ';

    line += (odd ? " )" : "(");
    line += "\n";

    return line;
}

std::string Events::Write(std::string worldName, std::string month, int year) {
    std::list<Event> events;

    for (auto &fact : this->facts) {
        fact->GetEvents(events);
    }

    std::map<EventCategory, std::vector<Event>> categories;
    for (auto &event : events) {
        if (categories.find(event.category) == categories.end()) {
            std::vector<Event> list = {};
            categories.insert(std::pair<EventCategory, std::vector<Event>>(event.category, list));
        }

        categories[event.category].push_back(event);
    }

    std::string text =  "   _.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._\n";
                text += ".----      - ---     --     ---   -----   - --       ----  ----   -     ----.\n";
                text += " )                                                                         (\n";

    std::list<std::string> lines;
    for (auto &cat : categories) {
        auto list = cat.second;

        std::sort(list.begin(), list.end(), compareEvents);
        list.resize(std::min((int) list.size(), 10));

        int n = std::min((int) list.size(), getrandom(3) + 1);
        while (n-- > 0) {
            int i = getrandom(list.size());

            if (lines.size() > 0) {
                lines.push_back("");
                lines.push_back(".:*~*:._.:*~*:._.:*~*:.");
                lines.push_back("");
            }

            auto tmp = wrapText(list[i].text, 65);
            for (auto &line : tmp) {
                lines.push_back(line);
            }

            list.erase(list.begin() + i);
        }
    }

    bool noNews = lines.size() == 0;

    lines.push_front("");
    lines.push_front(month + ", Year " + std::to_string(year));
    lines.push_front(worldName + " Events");

    if (noNews) {
        lines.push_back("--== Nothing mentionable happened in the world this month ==--");
    }

    if (lines.size() % 2) {
        lines.push_back("");
    }

    int n = 3;
    for (auto &line : lines) {
        text += makeLine(77, (n++) % 2, line);
    }

    text += "(__       _       _       _       _       _       _       _       _       __)\n";
    text += "    `-._.-' (___ _) `-._.-' `-._.-' )     ( `-._.-' `-._.-' (__ _ ) `-._.-'\n";
    text += "            ( _ __)                (_     _)                (_ ___)\n";
    text += "            (__  _)                 `-._.-'                 (___ _)\n";
    text += "            `-._.-'                                         `-._.-'\n";

    return text;
}

//----------------------------------------------------------------------------
void StrEvent::writeJson(JsonReport &of, ARegionList &) const
{
	of.PutStr(str_.c_str());
}

//----------------------------------------------------------------------------
TaxEvent::TaxEvent(const std::string &uname, int amt, ARegion *reg)
: uname_(uname)
, amt_(amt)
, reg_(reg)
{
}

std::string TaxEvent::str(ARegionList &regions) const
{
	std::ostringstream os;
	os << uname_ << ": Collects $" << amt_ << " in taxes in ";
	os << reg_->ShortPrint(&regions) << '.';
	return os.str();
}

void TaxEvent::writeJson(JsonReport &of, ARegionList &regions) const
{
	of.StartDict(NULL);
	of.PutPairStr("type", "Tax");
	of.PutPairStr("name", uname_.c_str());
	of.PutPairInt("amount", amt_);

	of.StartDict("region");
	reg_->CPrint(&of, &regions, false);
	of.EndDict();

	of.EndDict();
}

//----------------------------------------------------------------------------
GiveEvent::GiveEvent(const std::string &gname, int inum, int amt, const std::string &tname, bool reversed)
: gname_(gname)
, inum_(inum)
, amt_(amt)
, tname_(tname)
, reversed_(reversed)
{
}

std::string GiveEvent::str(ARegionList &) const
{
	std::ostringstream os;
	if (reversed_)
		os << tname_ << ": Receives " << ItemString(inum_, amt_) << " from " << gname_ << '.';
	else
		os << gname_ << ": Gives " << ItemString(inum_, amt_) << " to " << tname_ << '.';

	return os.str();
}

void GiveEvent::writeJson(JsonReport &of, ARegionList &regions) const
{
	of.StartDict(NULL);
	of.PutPairStr("type", reversed_ ? "Receive" : "Give");
	of.PutPairStr("giver", gname_.c_str());
	of.PutPairStr("receiver", tname_.c_str());
	of.PutPairStr("abbr", ItemDefs[inum_].abr);
	of.PutPairInt("amount", amt_);
	of.EndDict();
}

//----------------------------------------------------------------------------
ProduceEvent::ProduceEvent(const std::string &uname, int inum, int amt, ARegion *reg)
: uname_(uname)
, inum_(inum)
, amt_(amt)
, reg_(reg)
{
}

std::string ProduceEvent::str(ARegionList &regions) const
{
	std::ostringstream os;
	os << uname_ << ": Produces " << ItemString(inum_, amt_) << " in " << reg_->ShortPrint(&regions) << '.';

	return os.str();
}

void ProduceEvent::writeJson(JsonReport &of, ARegionList &regions) const
{
	of.StartDict(NULL);
	of.PutPairStr("type", "Produce");
	of.PutPairStr("name", uname_.c_str());
	of.PutPairStr("abbr", ItemDefs[inum_].abr);
	of.PutPairInt("amount", amt_);

	of.StartDict("region");
	reg_->CPrint(&of, &regions, false);
	of.EndDict();

	of.EndDict();
}


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

#include "events.h"
#include "gameio.h"
#include "astring.h"

#include <memory>
#include <stdexcept>
#include <sstream>

using namespace std;

AssassinationFact::AssassinationFact() {
    this->victim = BattleSide();
    this->location = EventLocation();
}

AssassinationFact::~AssassinationFact() {

}

const vector<string> SENTIMENT = {
    "fearsome",
    "alarming",
    "horrifying",
    "frightening",
    "worrisome"
};

const vector<string> LOCALS = {
    "citizens",
    "inhabitants",
    "locals",
    "commoners"
};

const vector<string> FEELING = {
    "shocked",
    "stunned",
    "horrified",
    "terrified",
    "afraid",
    "scared"
};

void AssassinationFact::GetEvents(std::list<Event> &events) {
    std::ostringstream buffer;

    buffer
        << capitalize(oneOf(SENTIMENT))
        << " news was coming from"
        ;

    if (!this->location.settlement.empty()) {
        buffer
            << " the " << townType(this->location.settlementType)
            << " of " << this->location.settlement
            << ", which lies in"
            ;
    }

    buffer
        << " the " << this->location.GetTerrainName(true)
        << " of " << this->location.province
        << ". " << capitalize(oneOf(LOCALS))
        << "were " << oneOf(FEELING)
        << " by the assassination"
        ;

    if (this->outcome != BATTLE_LOST) {
        buffer << " attempt";
    }

    if (!this->location.settlement.empty()) {
        buffer << " in the " << townType(this->location.settlementType);
    }

    if (this->outcome == BATTLE_LOST) {
        buffer << ". The assassin escaped unnoticed.";
    }
    else {
        buffer << "; thankfully, the victim survived.";
    }

    events.push_back({
        category: EventCategory::EVENT_ASSASSINATION,
        score: 1,
        text: buffer.str()
    });
}

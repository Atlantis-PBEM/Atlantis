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
#include "rng.hpp"
#include "string_filters.hpp"

using namespace std;

AssassinationFact::AssassinationFact() {
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
        << (rng::one_of(SENTIMENT) | filter::capitalize)
        << " news were coming from the"
        << " " << this->location.GetTerrainName(true)
        << " of " << this->location.province
        << "."
        ;

    buffer
        << " " << (rng::one_of(LOCALS) | filter::capitalize)
        << " were " << rng::one_of(FEELING)
        << " by the assassination"
        ;

    if (this->outcome != BATTLE_LOST) {
        buffer << " attempt";
    }

    auto mark = this->location.GetSignificantLandmark();
    if (mark) {
        buffer
            << " " << (mark->distance == 0 ? "in" : "near")
            << " the " << mark->title;
    }

    if (this->outcome == BATTLE_LOST) {
        buffer << ". The assassin escaped unnoticed.";
    }
    else if (this->outcome == BATTLE_DRAW) {
        buffer << "; thankfully, the victim survived, but the assassin was able to escape unnoticed.";
    }
    else {
        buffer << "; thankfully, the victim survived.";
    }

    events.push_back({
        .category = EventCategory::EVENT_ASSASSINATION,
        .score = 1,
        .text = buffer.str()
    });
}

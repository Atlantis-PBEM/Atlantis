#include "events.h"
#include "logger.hpp"

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

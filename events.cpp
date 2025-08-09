#include "logger.hpp"
#include "events.h"
#include "gamedata.h"
#include "graphs.h"
#include "object.h"
#include "rng.hpp"

#include <map>
#include <queue>
#include <algorithm>

std::string townType(const int type) {
    switch (type) {
        case TOWN_VILLAGE: return "village";
        case TOWN_TOWN:    return "town";
        case TOWN_CITY:    return "city";
        default:           return "unknown";
    }
}

FactBase::~FactBase() { }

void BattleSide::AssignUnit(Unit* unit) {
    this->factionName = unit->faction->name;
    this->factionNum = unit->faction->num;
    this->unitName = unit->name;
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

        if (item.type & IT_MONSTER && !(item.type & IT_UNDEAD)) {
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

const std::string EventLocation::GetTerrainName(const bool plural) {
    TerrainType &terrain = TerrainDefs[this->terrainType];
    auto terrainName = plural ? terrain.plural : terrain.name;
    return terrainName;
}

void populateSettlementLandmark(std::vector<Landmark> &landmarks, ARegion *reg, const int distance) {
    if (!reg->town) {
        return;
    }

    std::string name = reg->town->name;
    std::string title = townType(reg->town->TownType()) + " of " + name;

    landmarks.push_back({
        .type = events::LandmarkType::SETTLEMENT,
        .name = name,
        .title = title,
        .distance = distance,
        .weight = 10,
        .x = reg->xloc,
        .y = reg->yloc,
        .z = reg->zloc
    });
}

void populateRegionLandmark(std::vector<Landmark> &landmarks, ARegion *source, ARegion *reg, const int distance) {
    TerrainType& terrain = TerrainDefs[reg->type];
    int alias = terrain.similar_type;

    std::string name = reg->name;
    std::string sourceName = source->name;
    if (name == sourceName) {
        return;
    }

    std::string title = std::string(terrain.plural) + " of " + name;
    int weight = 1;

    events::LandmarkType type = events::LandmarkType::UNKNOWN;
    if (alias == R_MOUNTAIN) {
        type = events::LandmarkType::MOUNTAIN;
    }
    else if (alias == R_FOREST || alias == R_JUNGLE) {
        type = events::LandmarkType::FOREST;
    }
    else if (alias == R_VOLCANO) {
        type = events::LandmarkType::VOLCANO;
        weight = 2;
    }
    else if (alias == R_OCEAN) {
        type = name.ends_with("River") ? events::LandmarkType::RIVER : events::LandmarkType::OCEAN;
    }
    else if (name.ends_with("River")) {
        type = events::LandmarkType::FORD;
        weight = 2;
    }

    if (type == events::LandmarkType::UNKNOWN) {
        return;
    }

    landmarks.push_back({
        .type = type,
        .name = name,
        .title = title,
        .distance = distance,
        .weight = weight,
        .x = reg->xloc,
        .y = reg->yloc,
        .z = reg->zloc
    });
}

void populateForitifcationLandmark(std::vector<Landmark> &landmarks, ARegion *reg, const int distance) {
    int protect = 0;
    Object *building = NULL;

    for(const auto obj : reg->objects) {
        ObjectType& type = ObjectDefs[obj->type];

        if (type.flags & ObjectType::GROUP) {
            continue;
        }

        if (obj->IsFleet()) {
            continue;
        }

        if (protect >= type.protect) {
            continue;
        }

        protect = type.protect;
        building = obj;
    }

    if (!building) {
        return;
    }

    std::string name = building->name;
    std::string title = std::string(ObjectDefs[building->type].name) + " " + name;

    landmarks.push_back({
        .type = events::LandmarkType::FORTIFICATION,
        .name = name,
        .title = title,
        .distance = distance,
        .weight = 5,
        .x = reg->xloc,
        .y = reg->yloc,
        .z = reg->zloc
    });
}

bool compareLandmarks(const Landmark &first, const Landmark &second) {
    // Making this a bit more explicit since if you end up with 2 equidistant, equal weight landmarks, then
    // it will be arbitary which landmark is chosen based on the initial ordering in the vector being sorted,
    // which is based solely on an *unordered* map, which means the ordering is not guaranteed to be the same
    // across runs even with identical input since it's based on memory layout on the executing machine.

    // prefer shorter distances.
    if (first.distance != second.distance) {
        return first.distance < second.distance;
    }
    // prefer more weighty locales (citys > fortifications > volcano/river > other terrains)
    if (first.weight != second.weight) {
        return first.weight > second.weight;
    }
    // if everything else is equal, prefer one with the lower x coordinate
    if (first.x != second.x) {
        return first.x < second.x;
    }
    // If they are *still* equal, prefer the smaller y coordinate
    return first.y < second.y;
}

EventLocation EventLocation::Create(ARegion* region) {
    EventLocation loc;

    loc.x = region->xloc;
    loc.y = region->yloc;
    loc.z = region->zloc;
    loc.terrainType = region->type;
    loc.province = region->name;

    if (region->town) {
        loc.settlement = region->town->name;
        loc.settlementType = region->town->TownType();
    }

    loc.landmarks = { };

    auto items = breadthFirstSearch(region, 4);
    for (auto &kv : items) {
        auto reg = kv.second.key;
        auto distance = kv.second.distance;

        populateSettlementLandmark(loc.landmarks, reg, distance);
        populateRegionLandmark(loc.landmarks, region, reg, distance);
        populateForitifcationLandmark(loc.landmarks, reg, distance);
    }

    std::sort(std::begin(loc.landmarks), std::end(loc.landmarks), compareLandmarks);

    return loc;
}

const Landmark* EventLocation::GetSignificantLandmark() {
    if (this->landmarks.empty()) {
        return NULL;
    }

    return &(this->landmarks.at(0));
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
    // return true if first should go before second
    return first.score > second.score;
}

std::list<std::string> wrapText(std::string input, std::size_t width) {
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

        int n = std::min((int) list.size(), rng::get_random(3) + 3);
        while (n-- > 0) {
            int i = rng::get_random(list.size());

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
    text += "    '-._.-' (___ _) '-._.-' '-._.-' )     ( '-._.-' '-._.-' (__ _ ) '-._.-'\n";
    text += "            ( _ __)                (_     _)                (_ ___)\n";
    text += "            (__  _)                 '-._.-'                 (___ _)\n";
    text += "            '-._.-'                                         '-._.-'\n";

    return text;
}

AnnihilationFact::AnnihilationFact() {
    this->message = "";
}

AnnihilationFact::~AnnihilationFact() {
}

void AnnihilationFact::GetEvents(std::list<Event> &events) {
    events.push_back({
        .category = EventCategory::EVENT_ANNIHILATION,
        .score = 1000,
        .text = this->message
    });
}

AnomalyFact::AnomalyFact() {
    this->location = nullptr;
}

AnomalyFact::~AnomalyFact() {
}

void AnomalyFact::GetEvents(std::list<Event> &events) {
    events.push_back({
        .category = EventCategory::EVENT_ANOMALY,
        .score = 100,
        .text = "A strange anomaly was detected in region " + location->print() + "."
    });
}

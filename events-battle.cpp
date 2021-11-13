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

#include <memory>
#include <string>
#include <stdexcept>

template<typename ... Args> std::string string_format( const std::string& format, Args ... args )
{
    int size = std::snprintf(nullptr, 0, format.c_str(), args ... ) + 1;
    if (size <= 0) {
        throw std::runtime_error( "Error during formatting." );
    }
    
    std::unique_ptr<char[]> buf(new char[ size ]); 
    snprintf(buf.get(), size, format.c_str(), args ...);

    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

BattleFact::BattleFact() {
    this->attacker = BattleSide();
    this->defender = BattleSide();
    this->location = EventLocation();
}

BattleFact::~BattleFact() {

}

const int N_VARIANTS = 4;

const char* ADJECTIVE[N_VARIANTS] = {
    "Some",
    "Many",
    "Several",
    "Dozen"
};

const char* REPORTING[N_VARIANTS] = {
    "traders",
    "pilgrims",
    "travelers",
    "adventurers"
};

const char* CHANNEL[N_VARIANTS] = {
    "they have heard about",
    "refugees are worried about",
    "locals are rumoring about",
    "have heard a tell about"
};

const char* SMALL_BATTLE[N_VARIANTS] = {
    "encounter",
    "fight",
    "skrimish",
    "battle"
};

const char* BATTLE[N_VARIANTS] = {
    "clash",
    "fight",
    "assault",
    "battle"
};

const char* ONE_SIDE[N_VARIANTS] = {
    "a faction",
    "a rebels",
    "a villans",
    "an opposition"
};

const char* TWO_SIDES[N_VARIANTS] = {
    "hostile forces",
    "enemies",
    "two armies",
    "combatants"
};

const char* HUNTERS[N_VARIANTS] = {
    "an andventurers",
    "a hunters",
    "a witchers",
    "a rangers"
};

const char* SIZES[N_VARIANTS] = {
    "couple",
    "few",
    "several",
    "many"
};

const char* ACTION_SUCCESS[N_VARIANTS] = {
    "slain",
    "murdered",
    "put to the sword",
    "expelled"
};

const char* ACTION_ATTEMPT[N_VARIANTS] = {
    "attacked",
    "ambushed",
    "tried to cast out",
    "tried to expel"
};

const char* NOUN[N_VARIANTS] = {
    "terror",
    "fear",
    "horror",
    "dread"
};

std::string relativeSize(int size) {
    if (size < 3) return SIZES[0];
    if (size < 12) return SIZES[1];
    if (size < 24) return SIZES[2];
    
    return SIZES[3];
}

std::string townType(int type) {
    switch (type)
    {
        case TOWN_VILLAGE: return "village";
        case TOWN_TOWN:    return "town";
        case TOWN_CITY:    return "city";
        default:           return "unknown";
    }
}

void BattleFact::GetEvents(std::list<Event> &events) {
    // Some traders are telling that they have heard about
    // Some traders are telling that refugees are worried about
    // Some traders are telling that locals are rumoring about
    // Some traders are telling that have heard a tell about
    std::string text = string_format("%s %s are telling that %s ",
        ADJECTIVE[getrandom(N_VARIANTS)],
        REPORTING[getrandom(N_VARIANTS)],
        CHANNEL[getrandom(N_VARIANTS)]
    );

    if (this->defender.factionNum == 1) {
        // city capture
        if (this->outcome == BATTLE_WON) {
            // plains of Cefelat where in the Toadfield city guards were slain by a faction
            text += string_format("%ss of %s where in the %s %s guards were %s by %s.",
                this->location.getTerrain().c_str(),
                this->location.province.c_str(),
                this->location.settlement.c_str(),
                townType(this->location.settlementType).c_str(),
                ACTION_SUCCESS[getrandom(N_VARIANTS)],
                ONE_SIDE[getrandom(N_VARIANTS)]
            );
        }
        else {
            // plains of Cefelat where in the Toadfield a villans attacked guards but were unsuccessful.
            text += string_format("%ss of %s where in the %s %s %s %s guards but were unsuccessful.",
                this->location.getTerrain().c_str(),
                this->location.province.c_str(),
                this->location.settlement.c_str(),
                townType(this->location.settlementType).c_str(),
                ONE_SIDE[getrandom(N_VARIANTS)],
                ACTION_ATTEMPT[getrandom(N_VARIANTS)]
            );
        }

        events.push_back({ EventCategory::EVENT_CITY_CAPTURE, this->location.settlementType + 1, text });

        return;
    }

    if (this->defender.factionNum == 2) {
        // monster hunt
        if (this->outcome == BATTLE_WON) {
            // a witchers who have slain Demons freeing the plains of Cefelat from their terror.
            text += string_format("%s who have %s %s freeing the %ss of %s from their %s.",
                HUNTERS[getrandom(N_VARIANTS)],
                ACTION_SUCCESS[getrandom(N_VARIANTS)],
                this->defender.unitName.c_str(),
                this->location.getTerrain().c_str(),
                this->location.province.c_str(),
                NOUN[getrandom(N_VARIANTS)]
            );
        }
        else {
            // a witchers who tried to slain Demons in the plains of Cefelat but were all slain by their prey.
            text += string_format("%s who tried to %s %s in the %ss of %s but were all %s by their prey.",
                HUNTERS[getrandom(N_VARIANTS)],
                ACTION_SUCCESS[getrandom(N_VARIANTS)],
                this->defender.unitName.c_str(),
                this->location.getTerrain().c_str(),
                this->location.province.c_str(),
                ACTION_SUCCESS[getrandom(N_VARIANTS)]
            );
        }

        events.push_back({ EventCategory::EVENT_MONSTER_HUNT, 1, text });
        return;
    }

    if (this->attacker.factionNum == 2) {
        // monster aggression
        if (this->outcome == BATTLE_WON) {
            // Demons in the plains of Cefelat continue to cause %s on local inhabitants.
            text += string_format("%s in the %ss of %s continue to cause %s on local inhabitants.",
                this->attacker.unitName.c_str(),
                this->location.getTerrain().c_str(),
                this->location.province.c_str(),
                NOUN[getrandom(N_VARIANTS)]
            );
        }
        else {
            // Demons tried to cause fear in the plains of Cefelat bet were slain by a witchers.
            text += string_format("%s tried to cause %s in the %ss of %s bet were %s by %s.",
                this->attacker.unitName.c_str(),
                NOUN[getrandom(N_VARIANTS)],
                this->location.getTerrain().c_str(),
                this->location.province.c_str(),
                ACTION_SUCCESS[getrandom(N_VARIANTS)],
                HUNTERS[getrandom(N_VARIANTS)]
            );
        }

        events.push_back({ EventCategory::EVENT_MONSTER_AGGRESSION, 1, text });
        return;
    }

    // PvP
    int total = this->attacker.total + this->defender.total;
    int totalLost = this->attacker.lost + this->defender.lost;
    int totalMages = this->attacker.mages + this->defender.mages;
    int totalMonsters = this->attacker.monsters + this->defender.monsters;
    int totalUndead = this->attacker.undead + this->defender.undead;
    int totalFMI = this->attacker.fmi + this->defender.fmi;

    int minLost = std::min(this->attacker.lost, this->defender.lost);
    int maxLost = std::max(this->attacker.lost, this->defender.lost);
    bool isSlaughter = total > 10 && (minLost == 0 || (maxLost / minLost) > 10);

    int score = 0;

    if (total <= 100) {
        // encounter
        // location known

        std::string result;
        if (this->outcome == BATTLE_DRAW) {
            result = "and neither side won";
        }
        else if (isSlaughter) {
            result = "ended in a slaughter";
        }
        else {
            result = totalLost > total / 2
                ? "and some men died"
                : "and many soldiers will never fight again";
        }

        // a small encounter between hostile forces in the woods of Sansaor where some men died.
        text += string_format("a small %s between %s in the %ss of %s where %s.",
            SMALL_BATTLE[getrandom(N_VARIANTS)],
            TWO_SIDES[getrandom(N_VARIANTS)],
            this->location.getTerrain().c_str(),
            this->location.province.c_str(),
            result.c_str()
        );

        score = 1;
    }
    else if (total <= 500) {
        // local conflict
        // location known
        // number of looses known

        int unceretanity = totalLost / 3;   // 33%
        int lost = totalLost + getrandom(unceretanity) - (unceretanity / 2);

        std::string result;
        if (this->outcome == BATTLE_DRAW) {
            result = "and neither side won";
        }
        else if (isSlaughter) {
            result = "ended in a slaughter";
        }
        else {
            result = "and many soldiers will never fight again";
        }

        // a battle between two armies in the woods of Sansaor with .
        text += string_format("a %s between %s in the %ss of %s with %i killed from both sides %s.",
            SMALL_BATTLE[getrandom(N_VARIANTS)],
            TWO_SIDES[getrandom(N_VARIANTS)],
            this->location.getTerrain().c_str(),
            this->location.province.c_str(),
            lost,
            result.c_str()
        );

        score = 2;
    }
    else if (total <= 2500) {
        // regional conflict
        // location known
        // number of looses known
        // mages, monsters, fmi

        std::string specials;
        if (totalFMI + totalMages + totalMonsters + totalUndead) {
            specials = " with use of ";

            bool second = false;
            if (totalMages) {
                specials += "magic";
                second = true;
            }

            if (totalFMI) {
                if (second) specials += ", ";
                specials += "mechanisms";
                second = true;
            }

            if (totalUndead) {
                if (second) specials += ", ";
                specials += "undead";
                second = true;
            }

            if (totalMonsters) {
                if (second) specials += ", ";
                specials += "monsters";
                second = true;
            }
        }

        int unceretanity = totalLost / 5;   // 20%
        int lost = totalLost + getrandom(unceretanity) - (unceretanity / 2);

        std::string result;
        if (this->outcome == BATTLE_DRAW) {
            result = "and neither side won";
        }
        else if (isSlaughter) {
            result = "ended in a slaughter";
        }
        else {
            result = "and many soldiers will never fight again";
        }

        std::string sides;
        int roll = getrandom(10);
        if (roll >= 8) {
            // 20 %
            sides = string_format("%s and %s",
                this->attacker.factionName.c_str(),
                this->defender.factionName.c_str()
            );
        }
        else if (roll >= 6) {
            // 20%
            sides = string_format("%s and %s",
                (getrandom(2) >= 1 ? this->attacker.factionName.c_str() : this->defender.factionName.c_str()),
                ONE_SIDE[getrandom(N_VARIANTS)]
            );
        }
        else {
            // 60%
            sides = TWO_SIDES[getrandom(N_VARIANTS)];
        }

        // a battle with use of magic between two armies in the woods of Sansaor with .
        text += string_format("a %s%s between %s in the %ss of %s with %i killed from both sides %s.",
            BATTLE[getrandom(N_VARIANTS)],
            specials.c_str(),
            sides.c_str(),
            this->location.getTerrain().c_str(),
            this->location.province.c_str(),
            lost,
            result.c_str()
        );

        score = 3;
    }
    else {
        // continental conflict / epic conflict
        // location known
        // number of looses known
        // mages, monsters, fmi

        std::string specials;
        if (totalFMI + totalMages + totalMonsters + totalUndead) {
            specials = " with use of ";

            bool second = false;
            if (totalMages) {
                specials += relativeSize(totalMages) + " mages";
                second = true;
            }

            if (totalFMI) {
                if (second) specials += ", ";
                specials += relativeSize(totalFMI) + " mechanisms";
                second = true;
            }

            if (totalUndead) {
                if (second) specials += ", ";
                specials += relativeSize(totalUndead) + " undead";
                second = true;
            }

            if (totalMonsters) {
                if (second) specials += ", ";
                specials += relativeSize(totalMonsters) + " monsters";
                second = true;
            }
        }

        int unceretanity = totalLost / 10;   // 10%
        int lost = totalLost + getrandom(unceretanity) - (unceretanity / 2);

        std::string result;
        if (this->outcome == BATTLE_DRAW) {
            result = "and neither side won";
        }
        else if (isSlaughter) {
            result = "ended in a slaughter";
        }
        else {
            result = "and many soldiers will never fight again";
        }

        std::string sides = string_format("%s and %s",
            this->attacker.factionName.c_str(),
            this->defender.factionName.c_str()
        );

        // a battle with use of magic between two armies in the woods of Sansaor with .
        text += string_format("a epic %s%s between %s in the %ss of %s with %i killed from both sides %s.",
            BATTLE[getrandom(N_VARIANTS)],
            specials.c_str(),
            sides.c_str(),
            this->location.getTerrain().c_str(),
            this->location.province.c_str(),
            lost,
            result.c_str()
        );

        score = 5;
    }

    if (totalMages > 0) score *= 2;
    if (totalMonsters > 0) score += 1;
    if (totalUndead > 0) score += 1;
    if (totalFMI > 0) score += 1;

    events.push_back({ EventCategory::EVENT_BATTLE, score, text });
}

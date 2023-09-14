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
#include <stdexcept>
#include <sstream>

using namespace std;

BattleFact::BattleFact() {
    this->attacker = BattleSide();
    this->defender = BattleSide();
    this->location = EventLocation();
    this->fortificationType = -1;
}

BattleFact::~BattleFact() {

}

const vector<string> ADJECTIVE = {
    "A few",
    "Handful",
    "Some",
    "Many",
    "Numerous",
    "Multiple",
    "Several",
    "Dozen"
};

const vector<string> REPORTING = {
    "traders",
    "merchants",
    "pilgrims",
    "travelers",
    "wanderers",
    "adventurers",
    "peasants",
    "bards",
    "druids",
    "scouts",
    "dockers",
    "refugees",
    "locals",
    "commoners"
};

const vector<string> CHANNEL = {
    "are talking",
    "are rumoring",
    "are worried",
    "have heard",
    "are whispering",
    "are discussing"
};

const vector<string> BATTLE = {
    "an encounter",
    "a fight",
    "a conflict",
    "a clash",
    "the skirmish",
    "the battle"
};

const vector<string> ONE_SIDE = {
    "a faction",
    "a rebels",
    "a villans",
    "an opposition",
    "a partisans"
};

const vector<string> TWO_SIDES = {
    "hostile forces",
    "enemies",
    "two armies",
    "combatants"
};

const vector<string> HUNTERS = {
    "adventurers",
    "hunters",
    "daredevils",
    "witchers",
    "rangers"
};

const vector<string> SIZES = {
    "couple",
    "few",
    "several",
    "many"
};

const vector<string> ACTION_SUCCESS = {
    "slew",
    "murdered",
    "killed",
    "extinguished",
    "destroyed",
    "decimated",
    "annihilated",
    "massacred",
    "put to the sword",
    "expelled"
};

const vector<string> ACTION_ATTEMPT = {
    "attacked",
    "ambushed",
    "tried to cast out",
    "tried to expel"
};

const vector<string> FEAR_NOUN = {
    "terror",
    "fear",
    "anxiety",
    "horror",
    "dread"
};

string relativeSize(int size) {
    if (size < 3) return SIZES[0];
    if (size < 12) return SIZES[1];
    if (size < 24) return SIZES[2];
    
    return SIZES[3];
}

const int sizeRaiting(const int size) {
    if (size <= 100) return 1;
    if (size <= 500) return 2;
    if (size <= 2500) return 3;

    return 5;
}

const int combinedRaiting(const int attackerSize, const int defenderSize) {
    int att = sizeRaiting(attackerSize);
    int def = sizeRaiting(defenderSize);

    int minRaiting = std::min(att, def);
    if (minRaiting == 1) {
        return minRaiting;
    }

    int delta = std::abs(att - def);
    return delta >= 2 ? minRaiting + 1 : minRaiting;
}

const Event cityCapture(BattleFact* fact) {
    std::ostringstream buffer;

    if (fact->outcome == BATTLE_WON) {
        buffer
            << oneOf(ADJECTIVE)                               // Some
            << " " << oneOf(REPORTING)                        // merchants
            << " " << oneOf(CHANNEL)                          // are talking
            << " about the"                                   // about the
            << " " << townType(fact->location.settlementType) // city
            << " of"                                          // of
            << " " << fact->location.settlement               // Hardwood
            << " that lies in the"                            // that lies in the
            << " " << fact->location.province                 // Silver Valley
            << " " << fact->location.GetTerrainName(true)     // plains
            << ", where"                                      //, where
            << " " << oneOf(ONE_SIDE)                         // a faction
            << " " << oneOf(ACTION_SUCCESS)                   // slew
            << " guards."                                     // guards.
            ;
    }
    else {
        buffer
            << oneOf(ADJECTIVE)                               // Some
            << " " << oneOf(REPORTING)                        // merchants
            << " " << oneOf(CHANNEL)                          // are talking
            << " about the"                                   // about the
            << " " << townType(fact->location.settlementType) // city
            << " of"                                          // of
            << " " << fact->location.settlement               // Hardwood
            << " that lies in the"                            // that lies in the
            << " " << fact->location.province                 // Silver Valley
            << " " << fact->location.GetTerrainName(true)     // plains
            << ", where"                                      //, where
            << " " << oneOf(ONE_SIDE)                         // a faction
            << " " << oneOf(ACTION_ATTEMPT)                   // tried to cast out
            << " guards but were unsuccessful."               // guards but were unsuccessful.
            ;
    }

    return {
        category: EventCategory::EVENT_CITY_CAPTURE,
        score: fact->location.settlementType + 2,
        text: buffer.str()
    };
}

const Event monsterHunt(BattleFact* fact) {
    std::ostringstream buffer;

    auto mark = fact->location.GetSignificantLandmark();

    if (fact->outcome == BATTLE_WON) {
        buffer
            << oneOf(ADJECTIVE)                               // Some
            << " " << oneOf(REPORTING)                        // merchants
            << " " << oneOf(CHANNEL)                          // are talking
            << " about"                                       // about
            << " " << oneOf(HUNTERS)                          // witchers
            << " who have"                                    // who have
            << " " << oneOf(ACTION_SUCCESS)                   // slain
            << " " << fact->defender.unitName                 // Demons
            ;
        
        if (mark) {
            buffer << " near " << mark->title << ".";
        }

        buffer
            << " Freeing"                                     // Freeing
            << " the " << fact->location.GetTerrainName(true) // the plains
            << " of"                                          // of
            << " " << fact->location.province                 // Cefelat
            << " from their"                                  // from their
            << " " << oneOf(FEAR_NOUN) << "."                 // terror.
            ;
    }
    else {
        buffer
            << oneOf(ADJECTIVE)                           // Some
            << " " << oneOf(REPORTING)                    // merchants
            << " " << oneOf(CHANNEL)                      // are talking
            << " about"                                   // about
            << " " << oneOf(HUNTERS)                      // witchers
            << " who attempted to"                        // who attempted to
            << " " << oneOf(ACTION_SUCCESS)               // slain
            << " " << fact->defender.unitName             // Demons
            << " roaming"                                 // roaming
            << " the " << fact->location.GetTerrainName(true) // the plains
            << " of"                                      // of
            << " " << fact->location.province             // Cefelat
            ;
        
        if (mark) {
            buffer << " near " << mark->title << ".";
        }

        buffer << " But all of them were slain by their prey.";
    }

    return {
        category: EventCategory::EVENT_MONSTER_HUNT,
        score: 1,
        text: buffer.str()
    };
}

const Event monsterAggresion(BattleFact* fact) {
    std::ostringstream buffer;

    auto mark = fact->location.GetSignificantLandmark();

    if (fact->outcome == BATTLE_WON) {
        buffer
            << "In the " << fact->location.GetTerrainName(true)  // In the plains
            << " of " << fact->location.province             // of Cefelat
            ;

        if (mark) {
            buffer << ", near " << mark->title << ",";
        }

        buffer
            << " " << fact->attacker.unitName                // Demons
            << " who continue to cause " << oneOf(FEAR_NOUN) // continue to cause terror
            << " to local inhabitants."                      // to local inhabitants.
            ;
    }
    else {
        buffer
            << "A group of " << oneOf(HUNTERS)                          // A group of hunters
            << " " << oneOf(ACTION_SUCCESS)                             // extinguished
            << " the " << fact->attacker.unitName                       // the Demons
            << " who inflicted " << oneOf(FEAR_NOUN)                    // who inflicted terror
            << " on the inhabitants of the " << fact->location.province // on the inhabitants of the Cefelat
            << " " << fact->location.GetTerrainName(true)        // plains
            ;

        if (mark) {
            buffer << " near " << mark->title;
        }
        
        buffer << ".";
    }

    return {
        category: EventCategory::EVENT_MONSTER_AGGRESSION,
        score: 1,
        text: buffer.str()
    };
}

const Event pvpBattle(BattleFact* fact) {
    std::ostringstream buffer;

    int total = fact->attacker.total + fact->defender.total;
    int totalLost = fact->attacker.lost + fact->defender.lost;
    int totalMages = fact->attacker.mages + fact->defender.mages;
    int totalMonsters = fact->attacker.monsters + fact->defender.monsters;
    int totalUndead = fact->attacker.undead + fact->defender.undead;
    int totalFMI = fact->attacker.fmi + fact->defender.fmi;

    int minLost = std::min(fact->attacker.lost, fact->defender.lost);
    int maxLost = std::max(fact->attacker.lost, fact->defender.lost);

    bool isSlaughter = total > 10 && (minLost == 0 || (maxLost / minLost) > 10);

    int score = combinedRaiting(fact->attacker.total, fact->attacker.total);

    buffer
        << oneOf(ADJECTIVE)                               // Some
        << " " << oneOf(REPORTING)                        // merchants
        << " " << oneOf(CHANNEL)                          // are talking
        << " about"                                   // about
        ;

    if (total <= 100) {
        // encounter
        // location known

        buffer
            << " " << oneOf(BATTLE)
            << " between " << oneOf(TWO_SIDES)
            << " happened in the " << fact->location.GetTerrainName(true)
            << " of " << fact->location.province
            ;

        if (fact->outcome == BATTLE_DRAW) {
            buffer << " where neither side won.";
        }
        else if (isSlaughter) {
            buffer << " where battle ended in a slaughter of one of the sides.";
        }
        else {
            if (totalLost > total / 2) {
                buffer << " where some men died.";
            }
            else {
                buffer << " where many soldiers will never fight again.";
            }
        }
    }
    else if (total <= 500) {
        // local conflict
        // location known
        // number of looses known

        int unceretanity = totalLost / 3;   // 33%
        int lost = totalLost + getrandom(unceretanity) - (unceretanity / 2);

        buffer
            << " " << oneOf(BATTLE)                                         // a battle
            << " between " << oneOf(TWO_SIDES)                              // between hostile forces
            << " happened in the " << fact->location.GetTerrainName(true)                // happened in the plains
            << " of " << fact->location.province                            // of Cefelat
            << " where " << lost << " " << plural(lost, "combatant", "combatants") // where 75 combatants
            << " " << plural(lost, "was", "were") << " killed."             // were killed
            ;
        
        if (isSlaughter) {
            buffer << (fact->outcome == BATTLE_WON ? " The attackers slaughtered all defenders." : " The defenders were furious and put all attackers to the sword.");
        }
        else {
            if (fact->outcome == BATTLE_DRAW) {
                buffer << " Neither side won.";
            }
            else {
                buffer << (fact->outcome == BATTLE_WON ? " The attackers were victorious.." : " The defenders stood firm.");
            }
        }
    }
    else if (total <= 2500) {
        // regional conflict
        // location known
        // number of looses known
        // mages, monsters, fmi

        int unceretanity = totalLost / 5;   // 20%
        int lost = totalLost + getrandom(unceretanity) - (unceretanity / 2);

        buffer
            << " " << oneOf(BATTLE) // a battle
            << " between"             // between
            ;

        int roll = getrandom(10);
        if (roll >= 8) {
            // 20 %
            buffer
                << " " << fact->attacker.factionName
                << " and " << fact->defender.factionName
                ;
        }
        else if (roll >= 6) {
            // 20%
            buffer
                << " " << (getrandom(2) ? fact->attacker.factionName : fact->defender.factionName)
                << " and " << oneOf(ONE_SIDE)
                ;
        }
        else {
            // 60%
            buffer
                << " " << oneOf(TWO_SIDES)
                ;
        }

        buffer
            << " happened in the " << fact->location.GetTerrainName(true)                // happened in the plains
            << " of " << fact->location.province                            // of Cefelat
            << " where " << lost << plural(lost, "combatant", "combatants") // where 75 combatants
            << " " << plural(lost, "was", "were") << " killed."             // were killed
            ;

        std::string specials;
        if (totalFMI + totalMages + totalMonsters + totalUndead) {
            if (totalMages) {
                buffer << " Powerful magic was used in the battle.";
            }

            if (totalFMI) {
                buffer << " War engines were decimating the battlefield.";
            }

            if (totalUndead) {
                buffer << " Disgusting undead caused fear in the combatants.";
            }

            if (totalMonsters) {
                buffer << " Fearsome magical monsters were collecting their prey.";
            }
        }

        if (isSlaughter) {
            buffer << (fact->outcome == BATTLE_WON ? " The attackers slaughtered all defenders." : " The defenders were furious and put all attackers to the sword.");
        }
        else {
            if (fact->outcome == BATTLE_DRAW) {
                buffer << " Neither side won.";
            }
            else {
                buffer << (fact->outcome == BATTLE_WON ? " The attackers were victorious.." : " The defenders stood firm.");
            }
        }
    }
    else {
        // continental conflict / epic conflict
        // location known
        // number of looses known
        // mages, monsters, fmi

        int unceretanity = totalLost / 10;   // 10%
        int lost = totalLost + getrandom(unceretanity) - (unceretanity / 2);

        buffer
            << " an epic battle between"
            << " " << fact->attacker.factionName
            << " and " << fact->defender.factionName
            << " happened in the " << fact->location.GetTerrainName(true)                // happened in the plains
            << " of " << fact->location.province                            // of Cefelat
            << " where " << lost << plural(lost, "combatant", "combatants") // where 75 combatants
            << " " << plural(lost, "was", "were") << " killed."             // were killed
            ;

        std::string specials;
        if (totalFMI + totalMages + totalMonsters + totalUndead) {
            if (totalMages) {
                buffer << " Powerful magic was used in the battle.";
            }

            if (totalFMI) {
                buffer << " War engines were decimating the battlefield.";
            }

            if (totalUndead) {
                buffer << " Disgusting undead caused fear in the combatants.";
            }

            if (totalMonsters) {
                buffer << " Fearsome magical monsters were collecting their prey.";
            }
        }

        if (isSlaughter) {
            buffer << (fact->outcome == BATTLE_WON ? " The attackers slaughtered all defenders." : " The defenders were furious and put all attackers to the sword.");
        }
        else {
            if (fact->outcome == BATTLE_DRAW) {
                buffer << " Neither side won.";
            }
            else {
                buffer << (fact->outcome == BATTLE_WON ? " The attackers were victorious.." : " The defenders stood firm.");
            }
        }
    }

    auto mark = fact->location.GetSignificantLandmark();
    if (!fact->fortification.empty()) {
        buffer
            << " This battle will be known as Siege of the"
            << " " << ObjectDefs[fact->fortificationType].name
            << " " << fact->fortification << ".";
    }
    else if (mark) {
        buffer
            << " This battle will be known as Battle"
            << " " <<(mark->distance == 0 ? "of" : "near")
            ;

        if (mark->type == events::LandmarkType::FORD) {
            buffer << " the " << mark->name << " wade";
        }
        else {
            buffer << " the " << mark->title;
        }

        buffer << ".";
    }
    else {
        buffer
            << " This battle will known as Battle in the"
            << " " << fact->location.province
            << " " << fact->location.GetTerrainName(true)
            << "."
            ;
    }

    if (totalMages > 0) score *= 2;
    if (totalMonsters > 0) score += 1;
    if (totalUndead > 0) score += 1;
    if (totalFMI > 0) score += 1;

    return {
        category: EventCategory::EVENT_BATTLE,
        score: score,
        text: buffer.str()
    };
}

void BattleFact::GetEvents(std::list<Event> &events) {
    if (this->defender.factionNum == 1) {
        events.push_back(cityCapture(this));
        return;
    }

    if (this->defender.factionNum == 2) {
        events.push_back(monsterHunt(this));
        return;
    }

    if (this->attacker.factionNum == 2) {
        events.push_back(monsterAggresion(this));
        return;
    }

    events.push_back(pvpBattle(this));
}

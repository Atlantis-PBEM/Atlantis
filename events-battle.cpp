#include "logger.hpp"
#include "events.h"
#include "rng.hpp"

#include <memory>
#include <stdexcept>
#include <sstream>
#include "strings_util.hpp"

static const std::vector<std::string> ADJECTIVE = {
    "A few",
    "Handful",
    "Some",
    "Many",
    "Numerous",
    "Multiple",
    "Several",
    "Dozen"
};

static const std::vector<std::string> REPORTING = {
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

static const std::vector<std::string> CHANNEL = {
    "are talking",
    "are rumoring",
    "are worried",
    "have heard",
    "are whispering",
    "are discussing"
};

static const std::vector<std::string> BATTLE = {
    "an encounter",
    "a fight",
    "a conflict",
    "a clash",
    "the skirmish",
    "the battle"
};

static const std::vector<std::string> ONE_SIDE = {
    "a faction",
    "a rebels",
    "a villans",
    "an opposition",
    "a partisans"
};

static const std::vector<std::string> TWO_SIDES = {
    "hostile forces",
    "enemies",
    "two armies",
    "combatants"
};

static const std::vector<std::string> HUNTERS = {
    "adventurers",
    "hunters",
    "daredevils",
    "witchers",
    "rangers"
};

static const std::vector<std::string> ACTION_SUCCESS = {
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

static const std::vector<std::string> ACTION_ATTEMPT = {
    "attacked",
    "ambushed",
    "tried to cast out",
    "tried to expel"
};

static const std::vector<std::string> FEAR_NOUN = {
    "terror",
    "fear",
    "anxiety",
    "horror",
    "dread"
};

static constexpr int sizeRating(const int size) {
    if (size <= 100) return 1;
    if (size <= 500) return 2;
    if (size <= 2500) return 3;

    return 5;
}

static constexpr int combinedRating(const int attackerSize, const int defenderSize) {
    int att = sizeRating(attackerSize);
    int def = sizeRating(defenderSize);

    int minRating = std::min(att, def);
    if (minRating == 1) {
        return minRating;
    }

    int delta = std::abs(att - def);
    return delta >= 2 ? minRating + 1 : minRating;
}

static Event cityCapture(BattleFact* fact) {
    std::ostringstream buffer;

    if (fact->outcome == BATTLE_WON) {
        buffer
            << rng::one_of(ADJECTIVE)
            << " " << rng::one_of(REPORTING)
            << " " << rng::one_of(CHANNEL)
            << " about the " << townType(fact->location.settlementType)
            << " of " << fact->location.settlement
            << " that lies in the " << fact->location.province
            << " " << fact->location.GetTerrainName(true)
            << ", where" << " " << rng::one_of(ONE_SIDE)
            << " " << rng::one_of(ACTION_SUCCESS)
            << " guards.";
    }
    else {
        buffer
            << rng::one_of(ADJECTIVE)
            << " " << rng::one_of(REPORTING)
            << " " << rng::one_of(CHANNEL)
            << " about the " << townType(fact->location.settlementType)
            << " of " << fact->location.settlement
            << " that lies in the " << fact->location.province
            << " " << fact->location.GetTerrainName(true)
            << ", where " << rng::one_of(ONE_SIDE)
            << " " << rng::one_of(ACTION_ATTEMPT)
            << " guards but were unsuccessful.";
    }

    return {
        .category = EventCategory::EVENT_CITY_CAPTURE,
        .score = fact->location.settlementType + 2,
        .text = buffer.str()
    };
}

static Event monsterHunt(BattleFact* fact) {
    std::ostringstream buffer;

    auto mark = fact->location.GetSignificantLandmark();

    if (fact->outcome == BATTLE_WON) {
        buffer
            << rng::one_of(ADJECTIVE)
            << " " << rng::one_of(REPORTING)
            << " " << rng::one_of(CHANNEL)
            << " about " << rng::one_of(HUNTERS)
            << " who have " << rng::one_of(ACTION_SUCCESS)
            << " " << fact->defender.unitName;

        if (mark) {
            buffer << " near " << mark->title << ".";
        }

        buffer
            << " Freeing the " << fact->location.GetTerrainName(true)
            << " of " << fact->location.province
            << " from their " << rng::one_of(FEAR_NOUN) << ".";
    }
    else {
        buffer
            << rng::one_of(ADJECTIVE)
            << " " << rng::one_of(REPORTING)
            << " " << rng::one_of(CHANNEL)
            << " about " << rng::one_of(HUNTERS)
            << " who attempted to " << rng::one_of(ACTION_SUCCESS)
            << " " << fact->defender.unitName
            << " roaming the " << fact->location.GetTerrainName(true)
            << " of " << fact->location.province;

        if (mark) {
            buffer << " near " << mark->title << ".";
        }

        buffer << " But all of them were slain by their prey.";
    }

    return {
        .category = EventCategory::EVENT_MONSTER_HUNT,
        .score = 1,
        .text = buffer.str()
    };
}

static Event monsterAggresion(BattleFact* fact) {
    std::ostringstream buffer;

    auto mark = fact->location.GetSignificantLandmark();

    if (fact->outcome == BATTLE_WON) {
        buffer
            << "In the " << fact->location.GetTerrainName(true)
            << " of " << fact->location.province;

        if (mark) {
            buffer << ", near " << mark->title << ",";
        }

        buffer
            << " " << fact->attacker.unitName
            << " who continue to cause "
            << rng::one_of(FEAR_NOUN)
            << " to local inhabitants.";
    } else {
        buffer
            << "A group of " << rng::one_of(HUNTERS)
            << " " << rng::one_of(ACTION_SUCCESS)
            << " the " << fact->attacker.unitName
            << " who inflicted " << rng::one_of(FEAR_NOUN)
            << " on the inhabitants of the " << fact->location.province
            << " " << fact->location.GetTerrainName(true)
            ;

        if (mark) {
            buffer << " near " << mark->title;
        }

        buffer << ".";
    }

    return {
        .category = EventCategory::EVENT_MONSTER_AGGRESSION,
        .score = 1,
        .text = buffer.str()
    };
}

static Event pvpBattle(BattleFact* fact) {
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

    int score = combinedRating(fact->attacker.total, fact->attacker.total);

    buffer
        << rng::one_of(ADJECTIVE)
        << " " << rng::one_of(REPORTING)
        << " " << rng::one_of(CHANNEL)
        << " about";

    if (total <= 100) {
        // encounter
        // location known

        buffer
            << " " << rng::one_of(BATTLE)
            << " between " << rng::one_of(TWO_SIDES)
            << " happened in the " << fact->location.GetTerrainName(true)
            << " of " << fact->location.province;

        if (fact->outcome == BATTLE_DRAW) {
            buffer << " where neither side won.";
        }
        else if (isSlaughter) {
            buffer << " where battle ended in a slaughter of one of the sides.";
        }
        else {
            if (totalLost > total / 2) buffer << " where some men died.";
            else buffer << " where many soldiers will never fight again.";
        }
    } else if (total <= 500) {
        // local conflict
        // location known
        // number of looses known

        int unceretanity = totalLost / 3;   // 33%
        int lost = totalLost + rng::get_random(unceretanity) - (unceretanity / 2);

        buffer
            << " " << rng::one_of(BATTLE)
            << " between " << rng::one_of(TWO_SIDES)
            << " happened in the " << fact->location.GetTerrainName(true)
            << " of " << fact->location.province
            << " where " << lost << " " << strings::plural(lost, "combatant", "combatants")
            << " " << strings::plural(lost, "was", "were") << " killed.";

        if (isSlaughter) {
            buffer << (
                fact->outcome == BATTLE_WON ?
                " The attackers slaughtered all defenders." :
                " The defenders were furious and put all attackers to the sword."
            );
        }
        else {
            if (fact->outcome == BATTLE_DRAW) buffer << " Neither side won.";
            else buffer << (
                fact->outcome == BATTLE_WON ? " The attackers were victorious.." : " The defenders stood firm."
            );
        }
    } else if (total <= 2500) {
        // regional conflict
        // location known
        // number of looses known
        // mages, monsters, fmi

        int unceretanity = totalLost / 5;   // 20%
        int lost = totalLost + rng::get_random(unceretanity) - (unceretanity / 2);

        buffer
            << " " << rng::one_of(BATTLE)
            << " between";

        int roll = rng::get_random(10);
        if (roll >= 8) {
            // 20 %
            buffer
                << " " << fact->attacker.factionName
                << " and " << fact->defender.factionName;
        } else if (roll >= 6) {
            // 20%
            buffer
                << " " << (rng::get_random(2) ? fact->attacker.factionName : fact->defender.factionName)
                << " and " << rng::one_of(ONE_SIDE);
        } else {
            // 60%
            buffer << " " << rng::one_of(TWO_SIDES);
        }

        buffer
            << " happened in the " << fact->location.GetTerrainName(true)
            << " of " << fact->location.province
            << " where " << lost << " " << strings::plural(lost, "combatant", "combatants")
            << " " << strings::plural(lost, "was", "were") << " killed.";

        std::string specials;
        if (totalFMI + totalMages + totalMonsters + totalUndead) {
            if (totalMages) buffer << " Powerful magic was used in the battle.";
            if (totalFMI) buffer << " War engines were decimating the battlefield.";
            if (totalUndead) buffer << " Disgusting undead caused fear in the combatants.";
            if (totalMonsters) buffer << " Fearsome magical monsters were collecting their prey.";
        }

        if (isSlaughter) {
            buffer << (
                fact->outcome == BATTLE_WON ?
                " The attackers slaughtered all defenders." :
                " The defenders were furious and put all attackers to the sword."
            );
        } else {
            if (fact->outcome == BATTLE_DRAW) buffer << " Neither side won.";
            else buffer << (
                fact->outcome == BATTLE_WON ? " The attackers were victorious.." : " The defenders stood firm."
            );
        }
    } else {
        // continental conflict / epic conflict
        // location known
        // number of looses known
        // mages, monsters, fmi

        int unceretanity = totalLost / 10;   // 10%
        int lost = totalLost + rng::get_random(unceretanity) - (unceretanity / 2);

        buffer
            << " an epic battle between"
            << " " << fact->attacker.factionName
            << " and " << fact->defender.factionName
            << " happened in the " << fact->location.GetTerrainName(true)
            << " of " << fact->location.province
            << " where " << lost << " " << strings::plural(lost, "combatant", "combatants")
            << " " << strings::plural(lost, "was", "were") << " killed."
            ;

        std::string specials;
        if (totalFMI + totalMages + totalMonsters + totalUndead) {
            if (totalMages) buffer << " Powerful magic was used in the battle.";
            if (totalFMI) buffer << " War engines were decimating the battlefield.";
            if (totalUndead) buffer << " Disgusting undead caused fear in the combatants.";
            if (totalMonsters) buffer << " Fearsome magical monsters were collecting their prey.";
        }

        if (isSlaughter) {
            buffer << (
                fact->outcome == BATTLE_WON ?
                " The attackers slaughtered all defenders." :
                " The defenders were furious and put all attackers to the sword."
            );
        } else {
            if (fact->outcome == BATTLE_DRAW) buffer << " Neither side won.";
            else buffer << (
                fact->outcome == BATTLE_WON ? " The attackers were victorious.." : " The defenders stood firm."
            );
        }
    }

    auto mark = fact->location.GetSignificantLandmark();
    if (!fact->fortification.empty()) {
        buffer
            << " This battle will be known as Siege of the"
            << " " << ObjectDefs[fact->fortificationType].name
            << " " << fact->fortification << ".";
    } else if (mark) {
        buffer
            << " This battle will be known as Battle"
            << " " <<(mark->distance == 0 ? "of" : "near");

        if (mark->type == events::LandmarkType::FORD) buffer << " the " << mark->name << " wade";
        else buffer << " the " << mark->title;

        buffer << ".";
    } else {
        buffer
            << " This battle will known as Battle in the"
            << " " << fact->location.province
            << " " << fact->location.GetTerrainName(true)
            << ".";
    }

    if (totalMages > 0) score *= 2;
    if (totalMonsters > 0) score += 1;
    if (totalUndead > 0) score += 1;
    if (totalFMI > 0) score += 1;

    return {
        .category = EventCategory::EVENT_BATTLE,
        .score = score,
        .text = buffer.str()
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

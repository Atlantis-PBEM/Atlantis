#pragma once
#ifndef QUEST_H
#define QUEST_H

#include "unit.h"
#include "items.h"
#include <set>
#include <string>
#include <algorithm>

class Quest
{
    public:
        Quest();
        ~Quest();

        enum {
            SLAY,
            HARVEST,
            BUILD,
            VISIT,
            DELIVER,
            DEMOLISH
        };
        int type;
        int target;
        Item objective;
        int building;
        int regionnum;
        std::string regionname;
        std::set<std::string> destinations;
        std::vector<Item> rewards;
        std::string get_rewards();
};

class QuestList
{
    std::list<std::shared_ptr<Quest>> quests;
public:
    using iterator = typename std::list<std::shared_ptr<Quest>>::iterator;

    int read_quests(std::istream& f);
    void write_quests(std::ostream& f);

    int check_kill_target(Unit *u, ItemList& reward, std::string *quest_rewards);
    int check_harvest_target(ARegion *r,    int item, int harvested, int max, Unit *u, std::string *quest_rewards);
    int check_build_target(ARegion *r, int building, Unit *u, std::string *quest_rewards);
    int check_visit_target(ARegion *r, Unit *u, std::string *quest_rewards);
    int check_demolish_target(ARegion *r, int building, Unit *u, std::string *quest_rewards);

    inline void push_back(std::shared_ptr<Quest> q) { quests.push_back(q); }
    inline iterator begin() { return quests.begin(); }
    inline iterator end() { return quests.end(); }
    inline size_t erase(std::shared_ptr<Quest> q) { return std::erase(quests, q); }
    inline size_t size() { return quests.size(); }

    std::string distribute_rewards(Unit *u, std::shared_ptr<Quest> q);
};

extern QuestList quests;

#endif // QUEST_H

// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#ifndef QUEST_CLASS
#define QUEST_CLASS

#include "astring.h"
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
		int	type;
		int	target;
		Item objective;
		int	building;
		int	regionnum;
		AString	regionname;
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
	int check_harvest_target(ARegion *r,	int item, int harvested, int max, Unit *u, std::string *quest_rewards);
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

#endif

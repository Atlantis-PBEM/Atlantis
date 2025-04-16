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

#include "quests.h"
#include "object.h"
#include "rng.h"
#include <iterator>
#include <memory>

QuestList quests;

Quest::Quest()
{
	type = -1;
	target = -1;
	objective.type = -1;
	objective.num = 0;
	building = -1;
	regionnum = -1;
	regionname = "-";
}

Quest::~Quest()
{
	rewards.clear();
}

std::string Quest::get_rewards()
{
	std::string quest_rewards;
	bool first = true;

	quest_rewards = "Quest rewards: ";

	if (rewards.size() == 0) {
		return quest_rewards + "none.";
	}

	for(auto i: rewards) {
		if (!first) quest_rewards += ", ";
		first = false;
		quest_rewards += ItemString(i.type, i.num);
	}
	quest_rewards += ".";

	return quest_rewards;
}

int QuestList::read_quests(std::istream& f)
{
	int count, dests, rewards;
	std::shared_ptr<Quest> quest;
	std::string name;

	quests.clear();

	f >> count;
	if (count < 0)
		return 0;
	while (count-- > 0) {
		quest = std::make_shared<Quest>();
		f >> quest->type;
		switch (quest->type) {
			case Quest::SLAY:
				f >> quest->target;
				break;
			case Quest::HARVEST:
				quest->objective.Readin(f);
				f >> quest->regionnum;
				break;
			case Quest::BUILD:
				std::getline(f >> std::ws, name);
				quest->building = lookup_object(name);
				f >> std::ws >> quest->regionname;
				break;
			case Quest::VISIT:
				std::getline(f >> std::ws, name);
				quest->building = lookup_object(name);
				f >> dests;
				while (dests-- > 0) {
					std::getline(f >> std::ws, name);
					quest->destinations.insert(name);
				}
				break;
			case Quest::DEMOLISH:
				f >> quest->target;
				f >> quest->regionnum;
				break;
			default:
				f >> quest->target;
				quest->objective.Readin(f);
				std::getline(f >> std::ws, name);
				quest->building = lookup_object(name);
				f >> quest->regionnum;
				f >> std::ws >> quest->regionname;
				f >> dests;
				while (dests-- > 0) {
					std::getline(f >> std::ws, name);
					quest->destinations.insert(name);
				}
				break;
		}
		f >> rewards;
		while (rewards-- > 0) {
			Item item;
			item.Readin(f);
			if (-1 == item.type)
				return 0;
			quest->rewards.push_back(item);
		}
		quests.push_back(quest);
	}

    return 1;
}

void QuestList::write_quests(std::ostream& f)
{
	f << quests.size() << '\n';
	for(auto q: quests) {
		f << q->type << '\n';
		switch(q->type) {
			case Quest::SLAY:
				f << q->target << '\n';
				break;
			case Quest::HARVEST:
				q->objective.Writeout(f);
				f << q->regionnum << '\n';
				break;
			case Quest::BUILD:
				f << (q->building == -1 ? "NO_OBJECT" : ObjectDefs[q->building].name) << '\n';
				f << q->regionname << '\n';
				break;
			case Quest::VISIT:
				f << (q->building == -1 ? "NO_OBJECT" : ObjectDefs[q->building].name) << '\n';
				f << q->destinations.size() << '\n';
				for (auto dest: q->destinations) {
					f << dest << '\n';
				}
				break;
			case Quest::DEMOLISH:
				f << q->target << '\n';
				f << q->regionnum << '\n';
				break;
			default:
				f << q->target << '\n';
				q->objective.Writeout(f);
				f << (q->building == -1 ? "NO_OBJECT" : ObjectDefs[q->building].name) << '\n';
				f << q->regionnum << '\n';
				f << q->regionname << '\n';
				f << q->destinations.size() << '\n';
				for(auto dest: q->destinations) {
					f << dest << '\n';
				}
				break;
		}
		f << q->rewards.size() << '\n';
		for(auto i: q->rewards) {
			i.Writeout(f);
		}
	}
	f << 0 << '\n';
}

std::string QuestList::distribute_rewards(Unit *u, std::shared_ptr<Quest> q)
{
	for(auto i: q->rewards) {
		u->items.SetNum(i.type, u->items.GetNum(i.type) + i.num);
		u->faction->DiscoverItem(i.type, 0, 1);
	}
	return q->get_rewards();
}

int QuestList::check_kill_target(Unit *u, ItemList& reward, std::string *quest_rewards)
{
	for(auto q: quests) {
		if (q->type == Quest::SLAY && q->target == u->num) {
			// This dead thing was the target of a quest!
			for(auto i: q->rewards) {
				reward.SetNum(i.type, reward.GetNum(i.type) + i.num);
			}
			*quest_rewards = q->get_rewards();
			erase(q); // this is safe since we immediately return and don't use the iterator again
			return 1;
		}
	}
	return 0;
}

int QuestList::check_harvest_target(ARegion *r, int item, int harvested, int max, Unit *u, std::string *quest_rewards)
{
	for(auto q: quests) {
		if (q->type == Quest::HARVEST && q->regionnum == r->num && q->objective.type == item) {
			if (rng::get_random(max) < harvested) {
				*quest_rewards = distribute_rewards(u, q);
				erase(q); // this is safe since we immediately return and don't use the iterator again
				return 1;
			}
		}
	}
	return 0;
}

int QuestList::check_build_target(ARegion *r, int building, Unit *u, std::string *quest_rewards)
{
	for(auto q: quests) {
		if (q->type == Quest::BUILD && q->building == building && q->regionname == r->name) {
			*quest_rewards = distribute_rewards(u, q);
			erase(q); // this is safe since we immediately return and don't use the iterator again
			return 1;
		}
	}
	return 0;
}

int QuestList::check_visit_target(ARegion *r, Unit *u, std::string *quest_rewards)
{
	std::set<std::string> intersection;

	for(auto q: quests) {
		if (q->type != Quest::VISIT) continue;
		if (!q->destinations.count(r->name)) continue;
		for(const auto o : r->objects) {
			if (o->type == q->building) {
				u->visited.insert(r->name);
				intersection.clear();
				set_intersection(
					q->destinations.begin(),
					q->destinations.end(),
					u->visited.begin(),
					u->visited.end(),
					inserter(intersection, intersection.begin()),
					std::less<std::string>()
				);
				if (intersection.size() == q->destinations.size()) {
					// This unit has visited the required buildings in all those regions, so they completed a quest
					*quest_rewards = distribute_rewards(u, q);
					erase(q); // this is safe since we immediately return and don't use the iterator again
					return 1;
				}
			}
		}
	}
	return 0;
}

int QuestList::check_demolish_target(ARegion *r, int building, Unit *u, std::string *quest_rewards)
{
	for(auto q: quests) {
		if (q->type == Quest::DEMOLISH && q->regionnum == r->num && q->target == building) {
			*quest_rewards = distribute_rewards(u, q);
			erase(q); // this is safe since we immediately return and don't use the iterator again
			return 1;
		}
	}
	return 0;
}


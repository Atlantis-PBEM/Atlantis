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
//
// This file contains extra game-specific functions
//
#include "game.h"
#include "gamedata.h"
#include "quests.h"
#include <cmath>
#include <string>
#include <iterator>
#include <memory>

using namespace std;

#define MINIMUM_ACTIVE_QUESTS		5
#define MAXIMUM_ACTIVE_QUESTS		20
#define QUEST_EXPLORATION_PERCENT	30
#define QUEST_SPAWN_RATE		7
#define QUEST_MAX_REWARD		3000
#define QUEST_SPAWN_CHANCE		70
#define MAX_DESTINATIONS		5

int Game::SetupFaction( Faction *pFac )
{
	// Check if a faction can be started due to end game conditions
	if(rulesetSpecificData.value("victory_type", "") == "annihilation") {
		ARegionArray *surface = regions.get_first_region_array_of_type(ARegionArray::LEVEL_SURFACE);
		ARegion *surface_center = surface->GetRegion(surface->x / 2, surface->y / 2);

		int count = 0;
		for (int i = 0; i < 6; i++) {
			ARegion *r = surface_center->neighbors[i];
			// search that region for an altar
			for(const auto o : r->objects) {
				if (o->type == O_EMPOWERED_ALTAR) count++;
			}
		}
		// if all altars to the center are empowered, then factions cannot be started
		if (count == 6) return 0;
	}

	pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 300;

	if (pFac->noStartLeader) {
		return 1;
	}

	//
	// Set up first unit.
	//
	Unit *temp2 = GetNewUnit( pFac );
	temp2->SetMen(I_LEADERS, 1);
	pFac->DiscoverItem(I_LEADERS, 0, 1);
	temp2->reveal = REVEAL_FACTION;

	//
	// Set up magic
	//
	temp2->type = U_MAGE;
	temp2->Study(S_OBSERVATION, 30);
	temp2->Study(S_FORCE, 30);
	temp2->Study(S_PATTERN, 30);
	temp2->Study(S_SPIRIT, 30);
	temp2->Study(S_GATE_LORE, 30);
	temp2->Study(S_FIRE, 30);

	// Set up health
	temp2->Study(S_COMBAT, 180);

	// Set up flags
	temp2->SetFlag(FLAG_BEHIND, 1);
	temp2->SetFlag(FLAG_NOCROSS_WATER, 0);
	temp2->SetFlag(FLAG_HOLDING, 0);
	temp2->SetFlag(FLAG_NOAID, 0);

	if (Globals->UPKEEP_MINIMUM_FOOD > 0)
	{
		if (!(ItemDefs[I_FOOD].flags & ItemType::DISABLED)) {
			temp2->items.SetNum(I_FOOD, 6);
			pFac->DiscoverItem(I_FOOD, 0, 1);
		} else if (!(ItemDefs[I_FISH].flags & ItemType::DISABLED)) {
			temp2->items.SetNum(I_FISH, 6);
			pFac->DiscoverItem(I_FISH, 0, 1);
		} else if (!(ItemDefs[I_LIVESTOCK].flags & ItemType::DISABLED)) {
			temp2->items.SetNum(I_LIVESTOCK, 6);
			pFac->DiscoverItem(I_LIVESTOCK, 0, 1);
		} else if (!(ItemDefs[I_GRAIN].flags & ItemType::DISABLED)) {
			temp2->items.SetNum(I_GRAIN, 2);
			pFac->DiscoverItem(I_GRAIN, 0, 1);
		}
		temp2->items.SetNum(I_SILVER, 10);
	}

	ARegion *reg = NULL;
	if (pFac->pStartLoc) {
		reg = pFac->pStartLoc;
	} else if (!Globals->MULTI_HEX_NEXUS) {
		reg = regions.front();
	} else {
		ARegionArray *pArr = regions.GetRegionArray(ARegionArray::LEVEL_NEXUS);
		while(!reg) {
			reg = pArr->GetRegion(rng::get_random(pArr->x), rng::get_random(pArr->y));
		}
	}
	temp2->MoveUnit(reg->GetDummy());

	if (Globals->LAIR_MONSTERS_EXIST || Globals->WANDERING_MONSTERS_EXIST) {
		// Try to auto-declare all player factions unfriendly
		// to Creatures, since all they do is attack you.
		pFac->set_attitude(monfaction, A_UNFRIENDLY);
	}

	return( 1 );
}

static void CreateQuest(ARegionList& regions, int monfaction)
{
	int d, count, temple, i, j, clash, reward_count;
	ARegion *r;
	AString rname;
	map <string, int> temples;
	map <string, int>::iterator it;
	string stlstr;
	int destprobs[MAX_DESTINATIONS] = { 0, 0, 80, 20, 0 };
	int destinations[MAX_DESTINATIONS];
	string destnames[MAX_DESTINATIONS];
	set<string> intersection;

	std::shared_ptr<Quest> q = std::make_shared<Quest>();
	q->type = -1;

	// Set up quest rewards
	count = 0;
	for (i=0; i<NITEMS; i++) {
		if (
				((ItemDefs[i].type & IT_ADVANCED) || (ItemDefs[i].type & IT_MAGIC)) &&
				ItemDefs[i].baseprice <= QUEST_MAX_REWARD &&
				!(ItemDefs[i].type & IT_SPECIAL) &&
				!(ItemDefs[i].type & IT_SHIP) &&
				!(ItemDefs[i].type & IT_NEVER_SPOIL) &&
				!(ItemDefs[i].flags & ItemType::DISABLED)) {
			count ++;
		}
	}

	// No items? Are we playing a game without items?
	if (count == 0) return;

	count = rng::get_random(count) + 1;

	for (i=0; i<NITEMS; i++) {
		if (
				((ItemDefs[i].type & IT_ADVANCED) || (ItemDefs[i].type & IT_MAGIC)) &&
				ItemDefs[i].baseprice <= QUEST_MAX_REWARD &&
				!(ItemDefs[i].type & IT_SPECIAL) &&
				!(ItemDefs[i].type & IT_SHIP) &&
				!(ItemDefs[i].type & IT_NEVER_SPOIL) &&
				!(ItemDefs[i].flags & ItemType::DISABLED)) {
			count--;
			if (count == 0) {
				// Quest reward is based on QUEST_MAX_REWARD silver
				reward_count = (QUEST_MAX_REWARD + rng::get_random(QUEST_MAX_REWARD / 2)) / ItemDefs[i].baseprice;

				printf("\nQuest reward: %s x %d.\n", ItemDefs[i].name.c_str(), reward_count);

				// Setup reward
				Item item;
				item.type = i;
				item.num = reward_count;

				q->rewards.push_back(item);
				break;
			}
		}
	}

	d = rng::get_random(100);
	if (d < 60) {
		// SLAY quest
		q->type = Quest::SLAY;
		count = 0;
		// Count our current monsters
		for(const auto r : regions) {
			if (TerrainDefs[r->type].similar_type == R_OCEAN) continue;
			// No need to check if quests do not require exploration
			if (!r->visited && QUEST_EXPLORATION_PERCENT != 0) continue;
			for(const auto o : r->objects) {
				for(const auto u : o->units) {
					if (u->faction->num == monfaction) count++;
				}
			}
		}
		if (!count) return;
		// pick one as the object of the quest
		d = rng::get_random(count);
		for(const auto r : regions) {
			if (TerrainDefs[r->type].similar_type == R_OCEAN) continue;
			// No need to check if quests do not require exploration
			if (!r->visited && QUEST_EXPLORATION_PERCENT != 0) continue;
			for(const auto o : r->objects) {
				for(const auto u : o->units) {
					if (u->faction->num == monfaction) {
						if (!d--) q->target = u->num;
					}
				}
			}
		}
		for(const auto& q2 : quests) {
			if (q2->type == Quest::SLAY && q2->target == q->target) {
				// Don't hunt the same monster twice
				q->type = -1;
				break;
			}
		}
	} else if (d < 80) {
		// Create a HARVEST quest
		count = 0;
		for(const auto r : regions) {
			// Do allow lakes though
			if (r->type == R_OCEAN)
				continue;
			// No need to check if quests do not require exploration
			if (!r->visited && QUEST_EXPLORATION_PERCENT != 0)
				continue;
			for (const auto& p : r->products) {
				if (p->itemtype != I_SILVER)
					count++;
			}
		}
		count = rng::get_random(count);
		for(const auto r : regions) {
			// Do allow lakes though
			if (r->type == R_OCEAN)
				continue;
			// No need to check if quests do not require exploration
			if (!r->visited && QUEST_EXPLORATION_PERCENT != 0)
				continue;
			for (const auto& p : r->products) {
				if (p->itemtype != I_SILVER) {
					if (!count--) {
						q->type = Quest::HARVEST;
						q->regionnum = r->num;
						q->objective.type = p->itemtype;
						q->objective.num = 1;
					}
				}
			}
		}
		r = regions.GetRegion(q->regionnum);
		rname = r->name;
		for(const auto& q2: quests) {
			if (q2->type == Quest::HARVEST) {
				r = regions.GetRegion(q2->regionnum);
				if (rname == r->name) {
					// Don't have 2 harvest quests
					// active in the same region
					q->type = -1;
				}
			}
		}
	} else if (d < 100) {
		// Create a BUILD or VISIT quest
		// Find all our current temples
		temple = O_TEMPLE;
		for(const auto r : regions) {
			// No need to check if quests do not require exploration
			if (r->Population() > 0 && (r->visited || QUEST_EXPLORATION_PERCENT == 0)) {
				stlstr = r->name;
				// This looks like a null operation, but
				// actually forces the map<> element creation
				temples[stlstr];
				for(const auto o : r->objects) {
					if (o->type == temple) {
						temples[stlstr]++;
					}
				}
			}
		}
		// Work out how many destnations to use, based on destprobs[]
		for (i = 0, count = 0; i < MAX_DESTINATIONS; i++)
			count += destprobs[i];
		d = rng::get_random(count);
		for (count = 0; d >= destprobs[count]; count++)
			d -= destprobs[count];
		count++;
		if (count > (int) temples.size()) {
			q->type = -1;
			count = -1;
		}
		// Choose that many unique regions
		for (i = 0; i < count; i++) {
			do {
				destinations[i] = rng::get_random(temples.size());
				// give a slight preference to regions with temples
				for (it = temples.begin(), j = 0;
						j < destinations[i];
						it++, j++)
				// ...by rerolling (only once) if we get a
				// templeless region first time
				if (!it->second)
					destinations[i] = rng::get_random(temples.size());
				// make sure we haven't chosen duplicates
				clash = 0;
				for (j = 0; j < i; j++)
					if (destinations[i] == destinations[j])
						clash = 1;
			} while (clash);
		}
		// Look up the names of the chosen regions
		for (it = temples.begin(); it != temples.end(); it++) {
			for (i = 0; i < count; i++) {
				if (!destinations[i]--) {
					destnames[i] = it->first;
				}
			}
		}
		// If any of them don't have a temple, then make a quest to
		// build a temple there
		for (i = 0; i < count; i++) {
			if (!temples[destnames[i]]) {
				q->type = Quest::BUILD;
				q->building = temple;
				q->regionname = destnames[i].c_str();
				break;
			}
		}
		if (i == count) {
			// They all had temples, so make a VISIT quest
			q->type = Quest::VISIT;
			q->building = temple;
			for (j = 0; j < count; j++) {
				q->destinations.insert(destnames[j]);
			}
		}
		if (q->type == Quest::BUILD) {
			for(const auto& q2: quests) {
				if (q2->type == Quest::BUILD && q->building == q2->building && q->regionname == q2->regionname) {
					// Don't have 2 build quests
					// active in the same region
					q->type = -1;
				}
			}
		} else if (q->type == Quest::VISIT) {
			// Make sure that a given region is only in one
			// pilgrimage at a time
			for(const auto& q2: quests) {
				if (q2->type == Quest::VISIT && q->building == q2->building) {
					intersection.clear();
					set_intersection(
						q->destinations.begin(),
						q->destinations.end(),
						q2->destinations.begin(),
						q2->destinations.end(),
						inserter(intersection,
							intersection.begin()),
						less<string>()
					);
					if (intersection.size() > 0)
						q->type = -1;
				}
			}
		}
	}
	if (q->type != -1)
		quests.push_back(q);
}

// Just a quick function to count the number of empowered altars.
int report_and_count_empowered_altars(ARegionList& regions, std::list<Faction *>& factions) {
	ARegionArray *surface = regions.get_first_region_array_of_type(ARegionArray::LEVEL_SURFACE);
	ARegion *surface_center = surface->GetRegion(surface->x / 2, surface->y / 2);

	int count = 0;
	for (int i = 0; i < 6; i++) {
		ARegion *r = surface_center->neighbors[i];
		// search that region for an altar
		for(const auto o : r->objects) {
			if (o->type == O_EMPOWERED_ALTAR) {
				count++;
				for(const auto f : factions) {
					if (f->is_npc) continue;
					f->event("The altar in " + string(r->ShortPrint().const_str()) + " is fully empowered.",
						"anomaly", r);
				}
			}
		}
	}
	return count;
}

void empower_random_altar(ARegionList& regions, std::list<Faction *>& factions) {
	ARegionArray *surface = regions.get_first_region_array_of_type(ARegionArray::LEVEL_SURFACE);
	ARegion *surface_center = surface->GetRegion(surface->x / 2, surface->y / 2);

	std::vector<Object *> unempowered_altars;
	for (int i = 0; i < 6; i++) {
		ARegion *r = surface_center->neighbors[i];
		// search that region for an altar
		for(const auto o : r->objects) {
			if (o->type == O_RITUAL_ALTAR) unempowered_altars.push_back(o);
		}
	}
	// pick a random altar to empower
	int num = rng::get_random(unempowered_altars.size());
	Object *o = unempowered_altars[num];
	o->type = O_EMPOWERED_ALTAR;
	o->set_name(ObjectDefs[O_EMPOWERED_ALTAR].name);
	// notify all factions.
	for(const auto f : factions) {
		if (f->is_npc) continue;
		f->event("The altar in " + string(o->region->ShortPrint().const_str()) + " is fully empowered.",
			"anomaly", o->region);
	}

	// find all current anomalies and entities
	std::vector<Object *> anomalies;
	std::vector<Unit *> entities;
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			if (o->type == O_ENTITY_CAGE) anomalies.push_back(o);

			for(const auto u : o->units) {
				int i = u->items.GetNum(I_IMPRISONED_ENTITY);
				for(int j = 0; j < i; j++) {
					// put a unit in multiple times if it has multiple entities
					entities.push_back(u);
				}
			}
		}
	}
	if (anomalies.size() + entities.size() < unempowered_altars.size()) {
		// We have unspawend entities and anomalies, so just return without removing one.
		return;
	}
	// If we have any anomalies, remove the one farthest from the center.
	if (anomalies.size() > 0) {
		int max_dist = 0;
		Object *far_anomaly = nullptr;
		for (auto& anomaly : anomalies) {
			int dist = regions.find_distance_between_regions(anomaly->region, surface_center);
			if (dist > max_dist) {
				max_dist = dist;
				far_anomaly = anomaly;
			}
		}
		if (far_anomaly) {
			// remove the farthest anomaly
			// Just in case, move any units in the anomaly.
			for(const auto u : far_anomaly->units) u->MoveUnit(far_anomaly->region->GetDummy());

			std::erase(far_anomaly->region->objects, far_anomaly);
			delete far_anomaly;

			// Notify any factions in that region that the anomaly has been removed.
			auto reg_faction = far_anomaly->region->PresentFactions();
			for(const auto f : reg_faction) {
				if (f->is_npc) continue;
				f->event("The anomaly in " + string(far_anomaly->region->ShortPrint().const_str()) + " vanishes.",
					"anomaly", far_anomaly->region);
			}
			return;
		}
	}
	// Ok, we couldn't remove any anomalies, so remove the farthest entity instead.
	if (entities.size() > 0) {
		int max_dist = 0;
		Unit *far_entity = nullptr;
		for (auto& entity : entities) {
			int dist = regions.find_distance_between_regions(entity->object->region, surface_center);
			if (dist > max_dist) {
				max_dist = dist;
				far_entity = entity;
			}
		}
		if (far_entity) {
			far_entity->items.SetNum(I_IMPRISONED_ENTITY, far_entity->items.GetNum(I_IMPRISONED_ENTITY) - 1);
			far_entity->event(ItemString(I_IMPRISONED_ENTITY, 1) + " vanishes suddenly.", "anomaly");
			return;
		}
	}
	// We somehow got here without removing anything, so just return.
	return;
}


int report_and_count_anomalies(ARegionList& regions, std::list<Faction *>& factions) {
	int count = 0;
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			if (o->type == O_ENTITY_CAGE) {
				count++;
				for(const auto f : factions) {
					if (f->is_npc) continue;
					f->event("A strange anomaly has been seen in " + string(r->ShortPrint().const_str()) + ".",
						"anomaly", r);
				}
			}
		}
	}
	return count;
}

int report_and_count_entities(ARegionList& regions, std::list<Faction *>& factions) {
	int count = 0;
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				if (u->items.GetNum(I_IMPRISONED_ENTITY) > 0) {
					count += u->items.GetNum(I_IMPRISONED_ENTITY);
					for(const auto f : factions) {
						if (f->is_npc) continue;
						f->event("An imprisoned entity has been spotted in " + string(r->ShortPrint().const_str()) +
							" in the possession of " + string(u->GetName(0).const_str()) + ".",
							"anomaly", r);
					}
				}
			}
		}
	}
	return count;
}

Faction *Game::CheckVictory()
{
	int visited, unvisited;
	int d, i, count;
	int dir;
	unsigned ucount;
	ARegion *r, *start;
	Object *o;
	Location *l;
	AString message, times, temp;
	map <string, int> vRegions, uvRegions;
	map <string, int>::iterator it;
	string stlstr;
	set<string> intersection, un;
	set<string>::iterator it2;
	Faction *winner = nullptr;

	for(const auto& q: quests) {
		if (q->type != Quest::VISIT) continue;
		for (auto dest: q->destinations) {
			un.insert(dest);
		}
	}

	visited = 0;
	unvisited = 0;
	for(const auto r : regions) {
		if (r->Population() > 0) {
			stlstr = r->name;
			if (r->visited) {
				visited++;
				vRegions[stlstr]++;
			} else {
				unvisited++;
				uvRegions[stlstr]++;
			}
		}
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				intersection.clear();
				set_intersection(
					u->visited.begin(), u->visited.end(),
					un.begin(), un.end(),
					inserter(intersection, intersection.begin()),
					less<string>()
				);
				u->visited = intersection;
			}
		}
	}

	printf("Players have visited %d regions; %d unvisited.\n", visited, unvisited);

	if (visited >= (unvisited + visited) * QUEST_EXPLORATION_PERCENT / 100) {
		// Exploration phase complete: start creating relic quests
		for (i = 0; i < QUEST_SPAWN_RATE; i++) {
			if (quests.size() < MAXIMUM_ACTIVE_QUESTS && rng::get_random(100) < QUEST_SPAWN_CHANCE)
				CreateQuest(regions, monfaction);
		}
		while (quests.size() < MINIMUM_ACTIVE_QUESTS) {
			CreateQuest(regions, monfaction);
		}
	}

	if (unvisited) {
		// Tell the players to get exploring :-)
		if (visited > 9 * unvisited) {
			// 90% explored; specific hints
			d = rng::get_random(12);
		} else if (visited > 3 * unvisited) {
			// 75% explored; some general hints
			d = rng::get_random(8);
		} else {
			// lots of unexplored area; just tell them to explore
			d = rng::get_random(6);
		}
		if (d == 2) {
			message = "Be productive and strong; "
				"explore new land and find a way to survive.";
			WriteTimesArticle(message);
		} else if (d == 3) {
			message = "Go into all the world, and tell all "
				"people that new world is great.";
			WriteTimesArticle(message);
		} else if (d == 4 || d == 5) {
			message = "Players have visited ";
			message += (visited * 100 / (visited + unvisited));
			message += "% of all inhabited regions.";
			WriteTimesArticle(message);
		} else if (d == 6) {
			// report an incompletely explored region
			count = 0;
			// see how many incompletely explored regions we have
			for (it = vRegions.begin(); it != vRegions.end(); it++) {
				if (uvRegions[it->first] > 0)
					count++;
			}
			if (count > 0) {
				// choose one, and find it
				count = rng::get_random(count);
				for (it = vRegions.begin(); it != vRegions.end(); it++) {
					if (uvRegions[it->first] > 0)
						if (!count--)
							break;
				}
				// pick a hex within that region, and find it
				count = rng::get_random(it->second);
				for(const auto r : regions) {
					if (it->first == r->name) {
						if (!count--) {
							// report this hex
							message = "The ";
							message += TerrainDefs[TerrainDefs[r->type].similar_type].name;
							message += " of ";
							message += r->name;
							if (TerrainDefs[r->type].similar_type == R_TUNNELS)
								message += " are";
							else
								message += " is";
							message += " only partly explored.";
							WriteTimesArticle(message);
						}
					}
				}
			}
		} else if (d == 7) {
			// report a completely unknown region
			count = 0;
			// see how many completely unexplored regions we have
			for (it = uvRegions.begin(); it != uvRegions.end(); it++) {
				if (vRegions[it->first] == 0)
					count++;
			}
			if (count > 0) {
				// choose one, and find it
				count = rng::get_random(count);
				for (it = uvRegions.begin(); it != uvRegions.end(); it++) {
					if (vRegions[it->first] == 0) {
						if (!count--)
							break;
					}
				}
				// pick a hex within that region, and find it
				count = rng::get_random(it->second);
				for(const auto r : regions) {
					if (it->first == r->name) {
						if (!count--) {
							// report this hex
							dir = -1;
							start = regions.FindNearestStartingCity(r, &dir);
							message = "The ";
							message += TerrainDefs[TerrainDefs[r->type].similar_type].name;
							message += " of ";
							message += r->name;
							if (start == r) {
								message += ", containing ";
								message += start->town->name;
								message += ",";
							} else if (start && dir != -1) {
								message += ", ";
								if (r->zloc != start->zloc && dir != MOVE_IN)
									message += "through a shaft ";
								switch (dir) {
									case D_NORTH:
									case D_NORTHWEST:
										message += "north of";
										break;
									case D_NORTHEAST:
										message += "east of";
										break;
									case D_SOUTH:
									case D_SOUTHEAST:
										message += "south of";
										break;
									case D_SOUTHWEST:
										message += "west of";
										break;
									case MOVE_IN:
										message += "through a shaft in";
										break;
								}
								message += " ";
								message += start->town->name;
								message += ",";
							}
							if (TerrainDefs[r->type].similar_type == R_TUNNELS)
								message += " have";
							else
								message += " has";
							message += " yet to be visited by exiles from destroyed worlds.";
							WriteTimesArticle(message);
						}
					}
				}
			}
		} else if (d > 7) {
			// report exact coords of an unexplored hex
			count = rng::get_random(unvisited);
			for(const auto r : regions) {
				if (r->Population() > 0 && !r->visited) {
					if (!count--) {
						message = "The people of the ";
						message += r->ShortPrint();
						switch (rng::get_random(4)) {
							case 0:
								message += " have not been visited by exiles.";
								break;
							case 1:
								message += " are still in need of your guidance.";
								break;
							case 2:
								message += " have not yet been graced by your presence.";
								break;
							case 3:
								message += " are still in need of your guidance.";
								break;
						}
						WriteTimesArticle(message);
					}
				}
			}
		}
	}

	std::vector<std::shared_ptr<Quest>> questsWithProblems;
	for(const auto& q: quests) {
		switch(q->type) {
			case Quest::SLAY:
				l = regions.FindUnit(q->target);
				if (!l || l->unit->faction->num != monfaction) {
					// Something has gone wrong with this quest!
					// shouldn't ever happen, but...
					questsWithProblems.push_back(q);
					if (l) delete l;
				} else {
					message = "Quest: In the ";
					message += TerrainDefs[TerrainDefs[l->region->type].similar_type].name;
					message += " of ";
					message += l->region->name;
					if (l->obj->type == O_DUMMY)
						message += " roams";
					else
						message += " lurks";
					message += " the ";
					message += l->unit->name;
					message += ".  Free the world from this menace and be rewarded!";
					WriteTimesArticle(message);
					delete l;
				}

				break;
			case Quest::HARVEST:
				r = regions.GetRegion(q->regionnum);
				message = "Quest: Seek a token of the Ancient Ones legacy amongst the ";
				message += ItemDefs[q->objective.type].names;
				message += " of ";
				message += r->name;
				message += ".";
				WriteTimesArticle(message);
				break;
			case Quest::BUILD:
				message = "Quest: Build a ";
				message += ObjectDefs[q->building].name;
				message += " in ";
				message += q->regionname;
				message += " for the glory of the Gods.";
				WriteTimesArticle(message);
				break;
			case Quest::VISIT:
				message = "Quest: Show your devotion by visiting ";
				message += ObjectDefs[q->building].name;
				message += "s in ";
				ucount = 0;
				for (it2 = q->destinations.begin();
					it2 != q->destinations.end();
					it2++) {
					ucount++;
					if (ucount == q->destinations.size()) {
						message += " and ";
					} else if (ucount > 1) {
						message += ", ";
					}
					message += it2->c_str();
				}
				message += ".";
				WriteTimesArticle(message);
				break;
			case Quest::DEMOLISH:
				r = regions.GetRegion(q->regionnum);
				if (r)
					o = r->GetObject(q->target);
				else
					o = 0;
				if (!r || !o) {
					// Something has gone wrong with this quest!
					// shouldn't ever happen, but...
					questsWithProblems.push_back(q);
				} else {
					message = "Quest: Tear down the blasphemous ";
					message += o->name;
					message += " : ";
					message += ObjectDefs[o->type].name;
					message += " in ";
					message += r->name;
					message += "!";
					WriteTimesArticle(message);
				}
				break;
			default:
				break;
		}
	}
	for(const auto& q: questsWithProblems) quests.erase(q);

	if(rulesetSpecificData.value("victory_type", "") == "city_vote") {
		std::map <int, int> votes; // track votes per faction id
		int total_cities = 0; // total cities possible for vote count

		for(const auto r : regions) {
			// Ignore anything but the surface
			if (r->level->levelType != ARegionArray::LEVEL_SURFACE) continue;
			if (!r->town || (r->town->TownType() != TOWN_CITY)) continue;

			total_cities++;

			string name = r->town->name;
			string possible_faction = name.substr(0, name.find_first_of(" \t\n"));
			// The first word of the name was not all numeric, don't count for anyone
			if (!all_of(
				possible_faction.begin(),
				possible_faction.end(),
				[](unsigned char ch){ return std::isdigit(ch); }
			)) continue;
			// Now that we know it's all numeric, convert it to an int
			int faction_id = stoi(possible_faction);

			// Make sure it's a valid faction
			Faction *f = GetFaction(factions, faction_id);
			if (!f || f->is_npc) continue;

			auto vote = votes.find(faction_id);
			if (vote == votes.end()) {
				votes[faction_id] = 1;
			} else {
				vote->second++;
			}
		}

		// Set up the voting result to be reported if we are far enough in
		string message = "Voting results: \n";

		int max_vote = -1;
		bool tie = false;
		Faction *maxFaction = nullptr;
		for (const auto& vote : votes) {
			Faction *f = GetFaction(factions, vote.first);
			if (vote.second > max_vote) {
				max_vote = vote.second;
				maxFaction = f;
				tie = false;
			} else if (vote.second == max_vote) {
				tie = true;
				maxFaction = nullptr;
			}
			message += "Faction " + f->name + " has " + to_string(vote.second) + " votes.\n";
		}

		// See if we have enough votes to even report the info.  Since a win requires 50% + 1, we can start reporting
		// once someone has more than 25% of the cities.
		if (max_vote > (total_cities / 4)) {
			// Now see if we have a winner at all
			if (max_vote > ((total_cities / 2) + 1)) {
				winner = maxFaction;
				message += "\n" + winner->name + " has enough votes and has won the game!";
			} else {
				int percent = floor((max_vote * 100) / total_cities);
				if (tie) {
					message += "\nThere is a tie for the most votes with multiple factions having ";
				} else {
					message += string("\n") + "The current leader is " + maxFaction->name + " with ";
				}
				message += to_string(max_vote) + "/" + to_string(total_cities) + " votes (" + to_string(percent) + "%).";
			}
			WriteTimesArticle(message);
		}
	}

	// Check for victory conditions for the annihilation win
	if(rulesetSpecificData.value("victory_type", "") == "annihilation") {
		// before turn 50 none of the end game code will be active.
		if (TurnNumber() < 50) return nullptr;

		int empowered_altars = report_and_count_empowered_altars(regions, factions);
		// If we have hit turn 70, rather than spawning a new anomaly, we will activate a random altar and if
		// needed, destroy a randomly chosen entity.
		if (TurnNumber() >= 70 && empowered_altars < 6) {
			empower_random_altar(regions, factions);
			empowered_altars++;
		}

		int entities = report_and_count_entities(regions, factions);
		int anomalies = report_and_count_anomalies(regions, factions);

		int completed_entities = empowered_altars + entities;

		// This function will count the number of entities in the game + number of activated altars.
		// If this number is < 6, then we have a chance of spawning a new anomaly.  This chance starts at 10%
		// for the first anomaly after turn 50 and then increases by 12% for each entity or active altar already
		// existing.
		if (completed_entities < 6) {
			int chance = 10 + (completed_entities * 12);
			Awrite(AString("Endgame: entities: ") + completed_entities + ", anomalies: " + anomalies +
				", chance: " + chance + "%");
			if (rng::get_random(100) < chance) {
				// Okay, let's see if we can spawn a new entity
				// If we can, see if we already have those anomalies and report them to all factions if so.
				if (anomalies + completed_entities >= 6) {
					// We have all the anomalies that we can have still, so cannot spawn any more.
					return nullptr;
				}

				// Ok, we can spawn a new anomaly.  Let's do it.
				// We want a random land hex that is not a city and that does not have an anomaly and is not guarded.
				ARegion *r = nullptr;
				ARegionArray *surface = regions.get_first_region_array_of_type(ARegionArray::LEVEL_SURFACE);
				while (r == nullptr) {
					r = (ARegion *)surface->GetRegion(rng::get_random(surface->x), rng::get_random(surface->y));
					if (r == nullptr) continue;

					// An anomaly won't spawn in the ocean or in a barren region or in a city or a guarded region.
					TerrainType type = TerrainDefs[r->type];
					if (type.similar_type == R_OCEAN || type.similar_type == R_BARREN || r->town || r->IsGuarded()) {
						r = nullptr;
						continue;
					}
					// Make sure it doesn't already have an anomaly
					for(const auto o : r->objects) {
						if (o->type == O_ENTITY_CAGE) {
							r = nullptr;
							break;
						}
					}
					if(!r) continue;
				}

				// Okay, we have a new region to spawn an anomaly in.  Let's do it.
				Object *o = new Object(r);
				ObjectType ob = ObjectDefs[O_ENTITY_CAGE];
				o->type = O_ENTITY_CAGE;
				o->num = r->buildingseq++;
				o->set_name(ob.name);
				if (ob.flags & ObjectType::SACRIFICE) {
					o->incomplete = -(ob.sacrifice_amount);
				}
				r->objects.push_back(o);
				// Now tell all the factions about it.
				for(const auto f : factions) {
					if (f->is_npc) continue;
					f->event("A strange anomaly has appeared in " + string(r->ShortPrint().const_str()) + ".",
						"anomaly", r);
				}
				Awrite(AString("Spawned new anomaly at ") + r->ShortPrint() + ".");
			}

			// If we haven't completed all the entities, then noone can win yet.
			return nullptr;
		}

		// We have all entities completed, but.. have all altars been empowered?  if not, nooone can win yet.
		if (empowered_altars < 6) {
			Awrite(AString("Only ") + empowered_altars + " altars have been empowered, no winner yet.");
			return nullptr;
		}

		// Ok we have completed all entities, so we can check for if a faction has won.
		// the winner will be the owner of the world breaker monolith if and only if either
		// 1) all living factions are allied with the owner of the monolith
		// or
		// 2) the entire surface has been annihilated.

		// FInd the owner of the monolith.
		ARegionArray *underworld = regions.GetRegionArray(ARegionArray::LEVEL_UNDERWORLD);
		ARegion *center = underworld->GetRegion(underworld->x / 2, underworld->y / 2);
		for(const auto o : center->objects) {
			if (o->type == O_ACTIVE_MONOLITH) {
				Unit *owner = o->GetOwner();
				// If noone owns the monolith, then we have no possible winner yet.
				if (owner) winner = owner->faction;
				break;
			}
		}

		// No one owns the monolith, so no one can win yet.
		if (!winner) {
			if (TurnNumber() < 100) {
				// If it's before turn 100, we can't declare a winner yet
				Awrite(AString("No monolith owner found, no winner yet."));
				return nullptr;
			}
			// If the monolith is unowned on turn 100 or later, the monsters win.
			return GetFaction(factions, monfaction); // monsters win
		}

		// Ok, we have a possible winner, check for sufficient alive factions mutually allied to the monolith owner.
		int allied_count = 0;
		int total_factions = 0;
		for(const auto f : factions) {
			if (f->is_npc) continue;
			if (f == winner) continue;
			total_factions++;
			// This faction is not allied to the monolith owner, so they don't count
			if (f->get_attitude(winner->num) != A_ALLY) continue;
			// The winner is not allied to this faction, so they don't count;
			if (winner->get_attitude(f->num) != A_ALLY) continue;
			allied_count++;
		}

		int needed_percent = rulesetSpecificData.value("allied_percent", 100);
		int current_percent = (allied_count * 100) / total_factions;
		if (current_percent >= needed_percent) {
			// We have enough factions allied to the monolith owner, so they win.
			return winner;
		}

		// No winner yet, so check if the surface has been completely destroyed.
		int total_surface = 0;
		int total_annihilated = 0;
		for(const auto r : regions) {
			if (r->level->levelType != ARegionArray::LEVEL_SURFACE) continue;
			total_surface++;
			if (TerrainDefs[r->type].flags & TerrainType::ANNIHILATED) total_annihilated++;
		}

		int needed_surface = rulesetSpecificData.value("annihilate_percent", 100);
		int current_surface = (total_annihilated * 100) / total_surface;
		if (current_surface >= needed_surface) {
			// The surface has been sufficiently destroyed, so the monolith owner wins.
			return winner;
		}
		// No winner yet, so clear the potential winner and return null.
		winner = nullptr;
	}

	return winner;
}

void Game::ModifyTablesPerRuleset(void)
{
	if (Globals->APPRENTICES_EXIST)
		EnableSkill(S_MANIPULATE);

	if (!Globals->GATES_EXIST)
		DisableSkill(S_GATE_LORE);

	if (Globals->FULL_TRUESEEING_BONUS) {
		ModifyAttribMod("observation", 1, AttribModItem::SKILL, "TRUE", AttribModItem::UNIT_LEVEL, 1);
	}
	if (Globals->IMPROVED_AMTS) {
		ModifyAttribMod("observation", 2, AttribModItem::ITEM, "AMTS", AttribModItem::CONSTANT, 3);
	}
	if (Globals->FULL_INVIS_ON_SELF) {
		ModifyAttribMod("stealth", 3, AttribModItem::SKILL, "INVI", AttribModItem::UNIT_LEVEL, 1);
	}

	if (Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
		ClearTerrainRaces(R_NEXUS);
		ModifyTerrainRace(R_NEXUS, 0, I_HIGHELF);
		ModifyTerrainRace(R_NEXUS, 1, I_MAN);
		ModifyTerrainRace(R_NEXUS, 2, I_HILLDWARF);
		ClearTerrainItems(R_NEXUS);
		ModifyTerrainItems(R_NEXUS, 0, I_IRON, 100, 10);
		ModifyTerrainItems(R_NEXUS, 1, I_WOOD, 100, 10);
		ModifyTerrainItems(R_NEXUS, 2, I_STONE, 100, 10);
		ModifyTerrainEconomy(R_NEXUS, 1000, 15, 50, 2);
	}

	EnableItem(I_CAMEL);
	EnableItem(I_MCROSSBOW);
	EnableItem(I_MWAGON);
	EnableItem(I_GLIDER);
	EnableItem(I_LEATHERARMOR);
	EnableItem(I_SPEAR);
	EnableItem(I_JAVELIN);
	EnableItem(I_MSHIELD);
	EnableItem(I_ISHIELD);
	EnableItem(I_WSHIELD);
	EnableItem(I_AEGIS);
	EnableItem(I_WINDCHIME);
	EnableItem(I_GATE_CRYSTAL);
	EnableItem(I_STAFFOFH);
	EnableItem(I_SCRYINGORB);
	EnableItem(I_CORNUCOPIA);
	EnableItem(I_BOOKOFEXORCISM);
	EnableItem(I_HOLYSYMBOL);
	EnableItem(I_CENSER);
	EnableItem(I_FSWORD);
	EnableItem(I_MUSHROOM);
	EnableItem(I_HEALPOTION);
	EnableItem(I_GEMS);
	EnableItem(I_PIKE);
	EnableItem(I_BAXE);

	// Tools
	EnableItem(I_PICK);
	EnableItem(I_AXE);
	EnableItem(I_HAMMER);
	EnableItem(I_NET);
	EnableItem(I_LASSO);
	EnableItem(I_BAG);
	EnableItem(I_SPINNING);

	// FMI
	EnableItem(I_CATAPULT);
	EnableItem(I_STEEL_DEFENDER);

	//
	// Change craft: adamantium
	//
	EnableItem(I_ADMANTIUM);
	EnableItem(I_ADSWORD);
	EnableItem(I_ADRING);
	EnableItem(I_ADPLATE);
	ModifyItemProductionSkill(I_ADMANTIUM, "MINI", 5);
	ModifyItemProductionSkill(I_ADSWORD, "WEAP", 5);
	ModifyItemProductionSkill(I_ADBAXE, "WEAP", 5);
	ModifyItemProductionSkill(I_ADRING, "ARMO", 5);
	ModifyItemProductionSkill(I_ADPLATE, "ARMO", 5);

	// Artifacts of power
	DisableItem(I_RELICOFGRACE);

	// Disable items
	DisableItem(I_SUPERBOW);
	DisableItem(I_BOOTS);
	DisableItem(I_CLOTHARMOR);
	DisableItem(I_MBAXE);
	DisableItem(I_ADBAXE);
	DisableItem(I_ROUGHGEM);

	// No staff of lightning
	DisableSkill(S_CREATE_STAFF_OF_LIGHTNING);
	DisableItem(I_STAFFOFL);

	EnableSkill(S_ENCHANT_SHIELDS);
	EnableSkill(S_CREATE_AEGIS);
	EnableSkill(S_CREATE_WINDCHIME);
	EnableSkill(S_CREATE_GATE_CRYSTAL);
	EnableSkill(S_CREATE_STAFF_OF_HEALING);
	EnableSkill(S_CREATE_SCRYING_ORB);
	EnableSkill(S_CREATE_CORNUCOPIA);
	EnableSkill(S_CREATE_BOOK_OF_EXORCISM);
	EnableSkill(S_CREATE_HOLY_SYMBOL);
	EnableSkill(S_CREATE_CENSER);
	EnableSkill(S_CREATE_FLAMING_SWORD);
	EnableSkill(S_TRANSMUTATION);
	DisableSkill(S_CAMELTRAINING);
	DisableSkill(S_RANCHING);

	// No endurance
	DisableSkill(S_ENDURANCE);

	DisableSkill(S_GEMCUTTING);

	// Food
	EnableSkill(S_COOKING);
	EnableItem(I_FOOD);

	// Magic

	ModifySkillDependancy(S_RAISE_UNDEAD, 0, "SUSK", 3);
	ModifySkillDependancy(S_SUMMON_LICH, 0, "RAIS", 3);
	ModifySkillDependancy(S_DRAGON_LORE, 1, "WOLF", 3);

	ModifyItemMagicOutput(I_SKELETON, 200);
	ModifyItemMagicOutput(I_WOLF, 200);
	ModifyItemMagicOutput(I_IMP, 200);
	ModifyItemMagicOutput(I_UNDEAD, 100);
	ModifyItemMagicOutput(I_EAGLE, 100);
	ModifyItemMagicOutput(I_DEMON, 100);
	ModifyItemMagicOutput(I_DRAGON, 20);
	ModifyItemMagicOutput(I_LICH, 30);
	ModifyItemEscape(I_IMP, ItemType::ESC_LEV_LINEAR | ItemType::LOSE_LINKED, "SUIM", 20);
	ModifyItemEscape(I_DEMON, ItemType::ESC_LEV_LINEAR | ItemType::LOSE_LINKED, "SUDE", 20);
	ModifyItemEscape(I_BALROG, ItemType::ESC_LEV_LINEAR | ItemType::LOSE_LINKED, "SUBA", 20);

	//
	// Roads
	//
	EnableObject(O_ROADN);
	EnableObject(O_ROADNE);
	EnableObject(O_ROADNW);
	EnableObject(O_ROADS);
	EnableObject(O_ROADSE);
	EnableObject(O_ROADSW);
	ModifyObjectConstruction(O_ROADN, I_STONE, 30, "BUIL", 2);
	ModifyObjectConstruction(O_ROADNE, I_STONE, 30, "BUIL", 2);
	ModifyObjectConstruction(O_ROADNW, I_STONE, 30, "BUIL", 2);
	ModifyObjectConstruction(O_ROADS, I_STONE, 30, "BUIL", 2);
	ModifyObjectConstruction(O_ROADSE, I_STONE, 30, "BUIL", 2);
	ModifyObjectConstruction(O_ROADSW, I_STONE, 30, "BUIL", 2);

	EnableObject(O_TEMPLE);
	EnableObject(O_MQUARRY);
	EnableObject(O_AMINE);
	EnableObject(O_PRESERVE);
	EnableObject(O_SACGROVE);
	EnableObject(O_MTOWER);
	EnableObject(O_MFORTRESS);
	EnableObject(O_MCITADEL);
	EnableObject(O_STABLE);
	EnableObject(O_MSTABLE);
	EnableObject(O_HUT);
	EnableObject(O_TRAPPINGLODGE);
	EnableObject(O_FAERIERING);
	EnableObject(O_ALCHEMISTLAB);
	EnableObject(O_OASIS);
	EnableObject(O_TRAPPINGHUT);

	DisableObject(O_GEMAPPRAISER);
	DisableObject(O_PALACE);

	ModifyObjectName(O_MFORTRESS, "Magical Fortress");
	ModifyObjectName(O_MCASTLE, "Magical Castle");

	EnableObject(O_ISLE);
	EnableObject(O_DERELICT);
	EnableObject(O_OCAVE);
	EnableObject(O_WHIRL);

	//
	// Monsters
	//
	EnableItem(I_PIRATES);
	EnableItem(I_KRAKEN);
	EnableItem(I_MERFOLK);
	EnableItem(I_ELEMENTAL);
	EnableItem(I_HYDRA);
	EnableItem(O_BOG);
	EnableItem(I_ICEDRAGON);
	EnableItem(O_ICECAVE);
	EnableItem(I_ILLYRTHID);
	EnableItem(O_ILAIR);
	EnableItem(I_DEVIL);

	EnableItem(I_STORMGIANT);
	EnableItem(I_CLOUDGIANT);
	EnableItem(O_GIANTCASTLE);

	EnableItem(I_WARRIORS);

	EnableItem(I_DARKMAGE);
	EnableItem(O_DARKTOWER);

	EnableItem(I_MAGICIANS);
	EnableItem(O_MAGETOWER);

	//
	// Change races
	//
	DisableItem(I_ESKIMO);
	DisableItem(I_TRIBESMAN);
	DisableItem(I_NOMAD);
	DisableItem(I_TRIBALELF);
	DisableItem(I_VIKING);
	DisableItem(I_BARBARIAN);
	DisableItem(I_DARKMAN);
	DisableItem(I_DESERTDWARF);
	DisableItem(I_PLAINSMAN);
	DisableItem(I_SEAELF);
	DisableItem(I_GREYELF);
	DisableItem(I_MINOTAUR);
	DisableItem(I_OGREMAN);
	DisableItem(I_HOBBIT);

	ModifyItemBasePrice(I_LEADERS, 700);

	EnableItem(I_MAN);
	ModifyItemBasePrice(I_MAN, 40);
	ModifyRaceSkillLevels("HUMN", 4, 2);
	ModifyRaceSkills("HUMN", 0, "BUIL");
	ModifyRaceSkills("HUMN", 1, "RIDI");
	ModifyRaceSkills("HUMN", 2, "COMB");
	ModifyRaceSkills("HUMN", 3, "MINI");
	ModifyRaceSkills("HUMN", 4, "FARM");
	ModifyRaceSkills("HUMN", 5, "COOK");

	EnableItem(I_HILLDWARF);
	ModifyItemBasePrice(I_HILLDWARF, 40);
	ModifyRaceSkillLevels("HDWA", 5, 2);
	ModifyRaceSkills("HDWA", 0, "ARMO");
	ModifyRaceSkills("HDWA", 1, "WEAP");
	ModifyRaceSkills("HDWA", 2, "QUAR");
	ModifyRaceSkills("HDWA", 3, "MINI");
	ModifyRaceSkills("HDWA", 4, "BUIL");

	EnableItem(I_ICEDWARF);
	ModifyItemBasePrice(I_ICEDWARF, 40);
	ModifyRaceSkillLevels("IDWA", 5, 2);
	ModifyRaceSkills("IDWA", 0, "COMB");
	ModifyRaceSkills("IDWA", 1, "WEAP");
	ModifyRaceSkills("IDWA", 2, "MINI");
	ModifyRaceSkills("IDWA", 3, "FISH");
	ModifyRaceSkills("IDWA", 4, "ARMO");

	EnableItem(I_HIGHELF);
	ModifyItemBasePrice(I_HIGHELF, 40);
	ModifyRaceSkillLevels("HELF", 5, 2);
	ModifyRaceSkills("HELF", 0, "HORS");
	ModifyRaceSkills("HELF", 1, "FISH");
	ModifyRaceSkills("HELF", 2, "LBOW");
	ModifyRaceSkills("HELF", 3, "SHIP");
	ModifyRaceSkills("HELF", 4, "SAIL");

	EnableItem(I_WOODELF);
	ModifyItemBasePrice(I_WOODELF, 40);
	ModifyRaceSkillLevels("WELF", 5, 2);
	ModifyRaceSkills("WELF", 0, "LUMB");
	ModifyRaceSkills("WELF", 1, "LBOW");
	ModifyRaceSkills("WELF", 2, "ENTE");
	ModifyRaceSkills("WELF", 3, "CARP");
	ModifyRaceSkills("WELF", 4, "FISH");
	ModifyRaceSkills("WELF", 5, "COOK");

	EnableItem(I_GNOME);
	ModifyItemBasePrice(I_GNOME, 30);
	ModifyRaceSkillLevels("GNOM", 5, 2);
	ModifyRaceSkills("GNOM", 0, "HERB");
	ModifyRaceSkills("GNOM", 1, "QUAR");
	ModifyRaceSkills("GNOM", 2, "ENTE");
	ModifyRaceSkills("GNOM", 3, "XBOW");
	ModifyRaceSkills("GNOM", 4, "HEAL");
	ModifyItemCapacities(I_GNOME,7,0,0,0);
	ModifyItemWeight(I_GNOME, 5);

	EnableItem(I_CENTAURMAN);
	ModifyItemBasePrice(I_CENTAURMAN, 70);
	ModifyRaceSkillLevels("CTAU", 5, 2);
	ModifyRaceSkills("CTAU", 0, "LUMB");
	ModifyRaceSkills("CTAU", 1, "HORS");
	ModifyRaceSkills("CTAU", 2, "RIDI");
	ModifyRaceSkills("CTAU", 3, "HEAL");
	ModifyRaceSkills("CTAU", 4, "FARM");

	EnableItem(I_LIZARDMAN);
	ModifyItemBasePrice(I_LIZARDMAN, 40);
	ModifyRaceSkillLevels("LIZA", 5, 2);
	ModifyRaceSkills("LIZA", 0, "HUNT");
	ModifyRaceSkills("LIZA", 1, "HERB");
	ModifyRaceSkills("LIZA", 2, "CARP");
	ModifyRaceSkills("LIZA", 3, "SAIL");
	ModifyRaceSkills("LIZA", 4, "HEAL");

	EnableItem(I_GOBLINMAN);
	ModifyItemBasePrice(I_GOBLINMAN, 30);
	ModifyRaceSkillLevels("GBLN", 5, 2);
	ModifyRaceSkills("GBLN", 0, "QUAR");
	ModifyRaceSkills("GBLN", 1, "XBOW");
	ModifyRaceSkills("GBLN", 2, "SHIP");
	ModifyRaceSkills("GBLN", 3, "WEAP");
	ModifyRaceSkills("GBLN", 4, "ENTE");
	ModifyItemCapacities(I_GOBLINMAN,7,0,0,0);
	ModifyItemWeight(I_GOBLINMAN, 5);

	EnableItem(I_GNOLL);
	ModifyItemBasePrice(I_GNOLL, 40);
	ModifyRaceSkillLevels("GNOL", 5, 2);
	ModifyRaceSkills("GNOL", 0, "HORS");
	ModifyRaceSkills("GNOL", 1, "HUNT");
	ModifyRaceSkills("GNOL", 2, "COMB");
	ModifyRaceSkills("GNOL", 3, "ARMO");
	ModifyRaceSkills("GNOL", 4, "CARP");
	ModifyRaceSkills("GNOL", 5, "COOK");

	EnableItem(I_ORC);
	ModifyItemBasePrice(I_ORC, 40);
	ModifyRaceSkillLevels("ORC", 5, 2);
	ModifyRaceSkills("ORC", 0, "MINI");
	ModifyRaceSkills("ORC", 1, "LUMB");
	ModifyRaceSkills("ORC", 2, "COMB");
	ModifyRaceSkills("ORC", 3, "BUIL");
	ModifyRaceSkills("ORC", 4, "SHIP");

	// Underworld races
	EnableItem(I_DROWMAN);
	ModifyItemBasePrice(I_DROWMAN, 40);
	ModifyRaceSkillLevels("DRLF", 5, 2);
	ModifyRaceSkills("DRLF", 0, "COMB");
	ModifyRaceSkills("DRLF", 1, "HUNT");
	ModifyRaceSkills("DRLF", 2, "LBOW");
	ModifyRaceSkills("DRLF", 3, "LUMB");
	ModifyRaceSkills("DRLF", 4, "COOK");

	EnableItem(I_UNDERDWARF);
	ModifyItemBasePrice(I_UNDERDWARF, 40);
	ModifyRaceSkillLevels("UDWA", 5, 2);
	ModifyRaceSkills("UDWA", 0, "WEAP");
	ModifyRaceSkills("UDWA", 1, "ARMO");
	ModifyRaceSkills("UDWA", 2, "QUAR");
	ModifyRaceSkills("UDWA", 3, "MINI");
	ModifyRaceSkills("UDWA", 4, "BUIL");


	//
	// Change races per terrain
	//

	// Upper world
	// TODO: add ocean

	ClearTerrainRaces(R_PLAIN);
	ModifyTerrainRace(R_PLAIN, 0, I_HIGHELF);
	ModifyTerrainRace(R_PLAIN, 1, I_CENTAURMAN);
	ModifyTerrainRace(R_PLAIN, 2, I_GNOLL);
	ModifyTerrainRace(R_PLAIN, 3, I_MAN);
	ModifyTerrainCoastRace(R_PLAIN, 0, I_HIGHELF);
	ModifyTerrainCoastRace(R_PLAIN, 1, I_CENTAURMAN);
	ModifyTerrainCoastRace(R_PLAIN, 2, I_MAN);
	ModifyTerrainEconomy(R_PLAIN, 800, 12, 40, 1);

	ClearTerrainRaces(R_FOREST);
	ModifyTerrainRace(R_FOREST, 0, I_WOODELF);
	ModifyTerrainRace(R_FOREST, 1, I_CENTAURMAN);
	ModifyTerrainRace(R_FOREST, 2, I_HIGHELF);
	ModifyTerrainCoastRace(R_FOREST, 0, I_WOODELF);
	ModifyTerrainCoastRace(R_FOREST, 1, I_CENTAURMAN);
	ModifyTerrainCoastRace(R_FOREST, 2, I_HIGHELF);
	ModifyTerrainEconomy(R_FOREST, 600, 12, 20, 2);

	ClearTerrainRaces(R_MOUNTAIN);
	ModifyTerrainRace(R_MOUNTAIN, 0, I_HILLDWARF);
	ModifyTerrainRace(R_MOUNTAIN, 1, I_ORC);
	ModifyTerrainRace(R_MOUNTAIN, 2, I_MAN);
	ModifyTerrainCoastRace(R_MOUNTAIN, 0, I_HILLDWARF);
	ModifyTerrainCoastRace(R_MOUNTAIN, 1, I_ORC);
	ModifyTerrainCoastRace(R_MOUNTAIN, 2, I_MAN);
	ModifyTerrainEconomy(R_MOUNTAIN, 400, 11, 20, 2);

	ClearTerrainRaces(R_SWAMP);
	ModifyTerrainRace(R_SWAMP, 0, I_LIZARDMAN);
	ModifyTerrainRace(R_SWAMP, 1, I_GOBLINMAN);
	ModifyTerrainRace(R_SWAMP, 2, I_GNOLL);
	ModifyTerrainRace(R_SWAMP, 3, I_ORC);
	ModifyTerrainCoastRace(R_SWAMP, 0, I_LIZARDMAN);
	ModifyTerrainCoastRace(R_SWAMP, 1, I_GOBLINMAN);
	ModifyTerrainCoastRace(R_SWAMP, 2, I_GNOLL);
	ModifyTerrainEconomy(R_SWAMP, 500, 11, 10, 2);

	ClearTerrainRaces(R_JUNGLE);
	ModifyTerrainRace(R_JUNGLE, 0, I_ORC);
	ModifyTerrainRace(R_JUNGLE, 1, I_WOODELF);
	ModifyTerrainRace(R_JUNGLE, 2, I_LIZARDMAN);
	ModifyTerrainRace(R_JUNGLE, 3, I_GNOME);
	ModifyTerrainCoastRace(R_JUNGLE, 0, I_ORC);
	ModifyTerrainCoastRace(R_JUNGLE, 1, I_WOODELF);
	ModifyTerrainCoastRace(R_JUNGLE, 2, I_LIZARDMAN);
	ModifyTerrainEconomy(R_JUNGLE, 500, 11, 20, 2);

	ClearTerrainRaces(R_DESERT);
	ModifyTerrainRace(R_DESERT, 0, I_GNOLL);
	ModifyTerrainRace(R_DESERT, 1, I_GOBLINMAN);
	ModifyTerrainRace(R_DESERT, 2, I_MAN);
	ModifyTerrainCoastRace(R_DESERT, 0, I_GNOLL);
	ModifyTerrainCoastRace(R_DESERT, 1, I_GOBLINMAN);
	ModifyTerrainCoastRace(R_DESERT, 2, I_MAN);
	ModifyTerrainEconomy(R_DESERT, 400, 11, 10, 1);

	ClearTerrainRaces(R_TUNDRA);
	ModifyTerrainRace(R_TUNDRA, 0, I_ICEDWARF);
	ModifyTerrainRace(R_TUNDRA, 1, I_GNOME);
	ModifyTerrainRace(R_TUNDRA, 2, I_GNOLL);
	ModifyTerrainCoastRace(R_TUNDRA, 0, I_ICEDWARF);
	ModifyTerrainCoastRace(R_TUNDRA, 1, I_GNOME);
	ModifyTerrainCoastRace(R_TUNDRA, 2, I_GNOLL);
	ModifyTerrainEconomy(R_TUNDRA, 400, 11, 10, 2);

	// Underworld terrain

	ClearTerrainRaces(R_CAVERN);
	ModifyTerrainRace(R_CAVERN, 0, I_DROWMAN);
	ModifyTerrainRace(R_CAVERN, 1, I_UNDERDWARF);
	ModifyTerrainRace(R_CAVERN, 2, I_ORC);
	ModifyTerrainCoastRace(R_CAVERN, 0, I_UNDERDWARF);
	ModifyTerrainCoastRace(R_CAVERN, 1, I_GOBLINMAN);
	ModifyTerrainCoastRace(R_CAVERN, 2, I_ORC);
	ModifyTerrainEconomy(R_CAVERN, 300, 11, 10, 2);

	ClearTerrainRaces(R_UFOREST);
	ModifyTerrainRace(R_UFOREST, 0, I_DROWMAN);
	ModifyTerrainRace(R_UFOREST, 1, I_GNOME);
	ModifyTerrainRace(R_UFOREST, 2, I_GOBLINMAN);
	ModifyTerrainCoastRace(R_UFOREST, 0, I_DROWMAN);
	ModifyTerrainCoastRace(R_UFOREST, 1, I_GNOME);
	ModifyTerrainCoastRace(R_UFOREST, 2, I_GOBLINMAN);
	ModifyTerrainEconomy(R_UFOREST, 400, 11, 10, 2);

	ClearTerrainRaces(R_CHASM);
	ModifyTerrainRace(R_CHASM, 0, I_DROWMAN);
	ModifyTerrainRace(R_CHASM, 1, I_GNOME);
	ModifyTerrainRace(R_CHASM, 2, I_GOBLINMAN);
	ModifyTerrainCoastRace(R_CHASM, 0, I_UNDERDWARF);
	ModifyTerrainCoastRace(R_CHASM, 1, I_DROWMAN);
	ModifyTerrainCoastRace(R_CHASM, 2, I_GOBLINMAN);
	ModifyTerrainEconomy(R_CHASM, 200, 11, 10, 4);

	ClearTerrainRaces(R_TUNNELS);
	ModifyTerrainEconomy(R_TUNNELS, 0, 0, 0, 2);


	// Modify the various spells which are allowed to cross levels
	if (Globals->EASIER_UNDERWORLD) {
		ModifyRangeFlags("rng_teleport", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_portal", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_farsight", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_clearsky", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_weather", RangeType::RNG_CROSS_LEVELS);
	}

	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		EnableSkill(S_QUARTERMASTER);
		EnableObject(O_CARAVANSERAI);
		if (Globals->EASIER_UNDERWORLD) ModifyRangeLevelPenalty("rng_transport", 4);
	}

	// NO7 - Enable the various parts of the victory conditions
	EnableObject(O_RITUAL_ALTAR);
	EnableObject(O_EMPOWERED_ALTAR);
	EnableObject(O_ENTITY_CAGE);
	EnableObject(O_DORMANT_MONOLITH);
	EnableObject(O_ACTIVE_MONOLITH);
	EnableItem(I_IMPRISONED_ENTITY);
	EnableSkill(S_ANNIHILATION);
	ModifyRangeFlags("rng_annihilate", RangeType::RNG_SURFACE_ONLY | RangeType::RNG_CROSS_LEVELS);

	// Weapon BM example

	// Make SWOR to have malus of -1 on attack and -2 on defense vs. SPEA
	// ModifyWeaponBonusMalus("SWOR", 0, "SPEA", -1, -2);

	// At the same time give SPEA bonus of 2 on attacka and 2 on defense vs. SWOR
	// ModifyWeaponBonusMalus("SPEA", 0, "SWOR", 2, 2);

	// set up game specific tracked data
	rulesetSpecificData.clear();

	// this set is for the NO7 annihilation win condition
	rulesetSpecificData["victory_type"] = "annihilation";
	rulesetSpecificData["allowed_annihilates"] = 3;
	rulesetSpecificData["allied_percent"] = 50;
	rulesetSpecificData["annihilate_percent"] = 10;
	rulesetSpecificData["random_annihilates"] = true;

	// this set is for the city vote win condition, not active for NO7
	// rulesetSpecificData["victory_type"] = "city_vote";
	return;
}

const char *ARegion::movement_forbidden_by_ruleset(Unit *u, ARegion *origin, ARegionList& regions) {
	ARegionArray *surface = regions.get_first_region_array_of_type(ARegionArray::LEVEL_SURFACE);
	ARegion *surface_center = surface->GetRegion(surface->x / 2, surface->y / 2);

	ARegionArray *this_level = this->level;
	ARegion *this_center = this_level->GetRegion(this_level->x / 2, this_level->y / 2);

	if (this == this_center || this == surface_center) {
		// This is a center region.  You can only enter here if all the altars have been empowered.

		int count = 0;
		for (int i = 0; i < 6; i++) {
			ARegion *r = surface_center->neighbors[i];
			// search that region for an altar
			for(const auto o : r->objects) {
				if (o->type == O_EMPOWERED_ALTAR) {
					count++;
				}
			}
		}
		if (count < 6) {
			return "A mystical barrier";
		}
	}
	return nullptr;
}

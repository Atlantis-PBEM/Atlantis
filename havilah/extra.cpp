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
#include "indenter.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>

using namespace std;

#define RELICS_REQUIRED_FOR_VICTORY	7
#define MINIMUM_ACTIVE_QUESTS		3
#define MAXIMUM_ACTIVE_QUESTS		10
#define QUEST_EXPLORATION_PERCENT	100
#define QUEST_SPAWN_RATE		4
#define QUEST_SPAWN_CHANCE		40
#define MAX_DESTINATIONS		5

int Game::SetupFaction( Faction *pFac )
{
	pFac->unclaimed = Globals->START_MONEY + TurnNumber() * 50;

	if (pFac->noStartLeader)
		return 1;

	//
	// Set up first unit.
	//
	Unit *temp2 = GetNewUnit( pFac );
	temp2->SetMen(I_LEADERS, 1);
	pFac->DiscoverItem(I_LEADERS, 0, 1);
	temp2->reveal = REVEAL_FACTION;

	if (TurnNumber() >= 25) {
		temp2->type = U_MAGE;
		temp2->Study(S_PATTERN, 30);
		temp2->Study(S_SPIRIT, 30);
		temp2->Study(S_GATE_LORE, 30);
	}

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
		reg = (ARegion *)(regions.First());
	} else {
		ARegionArray *pArr = regions.GetRegionArray(ARegionArray::LEVEL_NEXUS);
		while(!reg) {
			reg = pArr->GetRegion(getrandom(pArr->x), getrandom(pArr->y));
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

static void CreateQuest(ARegionList *regions, int monfaction)
{
	Quest *q, *q2;
	Item *item;
	int d, count, temple, i, j, clash;
	ARegion *r;
	Object *o;
	Unit *u;
	AString rname;
	map <string, int> temples;
	map <string, int>::iterator it;
	string stlstr;
	int destprobs[MAX_DESTINATIONS] = { 0, 0, 80, 20, 0 };
	int destinations[MAX_DESTINATIONS];
	string destnames[MAX_DESTINATIONS];
	set<string> intersection;

	q = new Quest;
	q->type = -1;
	item = new Item;
	item->type = I_RELICOFGRACE;
	item->num = 1;
	q->rewards.Add(item);
	d = getrandom(100);
	if (d < 40) {
		// SLAY quest
		q->type = Quest::SLAY;
		count = 0;
		// Count our current monsters
		forlist(regions) {
			r = (ARegion *) elem;
			if (TerrainDefs[r->type].similar_type == R_OCEAN)
				continue;
			if (!r->visited)
				continue;
			forlist(&r->objects) {
				o = (Object *) elem;
				forlist(&o->units) {
					u = (Unit *) elem;
					if (u->faction->num == monfaction) {
						count++;
					}
				}
			}
		}
		if (!count)
			return;
		// pick one as the object of the quest
		d = getrandom(count);
		forlist_reuse(regions) {
			r = (ARegion *) elem;
			if (TerrainDefs[r->type].similar_type == R_OCEAN)
				continue;
			if (!r->visited)
				continue;
			forlist(&r->objects) {
				o = (Object *) elem;
				forlist(&o->units) {
					u = (Unit *) elem;
					if (u->faction->num == monfaction) {
						if (!d--) {
							q->target = u->num;
							
						}
					}
				}
			}
		}
		forlist_reuse(&quests) {
			q2 = (Quest *) elem;
			if (q2->type == Quest::SLAY &&
					q2->target == q->target) {
				// Don't hunt the same monster twice
				q->type = -1;
				break;
			}
		}
	} else if (d < 70) {
		// Create a HARVEST quest
		count = 0;
		forlist(regions) {
			r = (ARegion *) elem;
			// Do allow lakes though
			if (r->type == R_OCEAN)
				continue;
			if (!r->visited)
				continue;
			for (const auto& p : r->products) {
				if (p->itemtype != I_SILVER)
					count++;
			}
		}
		count = getrandom(count);
		forlist_reuse(regions) {
			r = (ARegion *) elem;
			// Do allow lakes though
			if (r->type == R_OCEAN)
				continue;
			if (!r->visited)
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
		r = regions->GetRegion(q->regionnum);
		rname = *r->name;
		forlist_reuse(&quests) {
			q2 = (Quest *) elem;
			if (q2->type == Quest::HARVEST) {
				r = regions->GetRegion(q2->regionnum);
				if (rname == *r->name) {
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
		forlist(regions) {
			r = (ARegion *) elem;
			if (r->Population() > 0 && r->visited) {
				stlstr = r->name->Str();
				// This looks like a null operation, but
				// actually forces the map<> element creation
				temples[stlstr];
				forlist(&r->objects) {
					o = (Object *) elem;
					if (o->type == temple) {
						temples[stlstr]++;
					}
				}
			}
		}
		// Work out how many destnations to use, based on destprobs[]
		for (i = 0, count = 0; i < MAX_DESTINATIONS; i++)
			count += destprobs[i];
		d = getrandom(count);
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
				destinations[i] = getrandom(temples.size());
				// give a slight preference to regions with temples
				for (it = temples.begin(), j = 0;
						j < destinations[i];
						it++, j++)
				// ...by rerolling (only once) if we get a
				// templeless region first time
				if (!it->second)
					destinations[i] = getrandom(temples.size());
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
			forlist(&quests) {
				q2 = (Quest *) elem;
				if (q2->type == Quest::BUILD &&
						q->building == q2->building &&
						q->regionname == q2->regionname) {
					// Don't have 2 build quests
					// active in the same region
					q->type = -1;
				}
			}
		} else if (q->type == Quest::VISIT) {
			// Make sure that a given region is only in one
			// pilgrimage at a time
			forlist(&quests) {
				q2 = (Quest *) elem;
				if (q2->type == Quest::VISIT &&
						q->building == q2->building) {
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
		quests.Add(q);
	else
		delete q;
}

Faction *Game::CheckVictory()
{
	int visited, unvisited;
	int d, i, count, reliccount;
	int units, leaders, men, silver, stuff;
	int skilldays, magicdays, skilllevels, magiclevels;
	int dir, found;
	unsigned ucount;
	Quest *q;
	Item *item;
	ARegion *r, *start;
	Object *o;
	Unit *u;
	Faction *f;
	Skill *s;
	Location *l;
	AString message, times, temp;
	map <string, int> vRegions, uvRegions;
	map <string, int>::iterator it;
	string stlstr;
	set<string> intersection, un;
	set<string>::iterator it2;

	forlist(&quests) {
		q = (Quest *) elem;
		if (q->type != Quest::VISIT)
			continue;
		for (it2 = q->destinations.begin();
				it2 != q->destinations.end();
				it2++) {
			un.insert(*it2);
		}
	}
	visited = 0;
	unvisited = 0;
	forlist_reuse(&regions) {
		r = (ARegion *) elem;
		if (r->Population() > 0) {
			stlstr = r->name->Str();
			if (r->visited) {
				visited++;
				vRegions[stlstr]++;
			} else {
				unvisited++;
				uvRegions[stlstr]++;
			}
		}
		forlist(&r->objects) {
			o = (Object *) elem;
			forlist(&o->units) {
				u = (Unit *) elem;
				intersection.clear();
				set_intersection(u->visited.begin(),
					u->visited.end(),
					un.begin(),
					un.end(),
					inserter(intersection,
						intersection.begin()),
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
			if (quests.Num() < MAXIMUM_ACTIVE_QUESTS &&
					getrandom(100) < QUEST_SPAWN_CHANCE)
				CreateQuest(&regions, monfaction);
		}
		while (quests.Num() < MINIMUM_ACTIVE_QUESTS) {
			CreateQuest(&regions, monfaction);
		}
	}
	if (unvisited) {
		// Tell the players to get exploring :-)
		if (visited > 9 * unvisited) {
			// 90% explored; specific hints
			d = getrandom(12);
		} else if (visited > 3 * unvisited) {
			// 75% explored; some general hints
			d = getrandom(8);
		} else {
			// lots of unexplored area; just tell them to explore
			d = getrandom(6);
		}
		if (d == 2) {
			message = "Be productive and multiply; "
				"fill the land and subdue it.";
			WriteTimesArticle(message);
		} else if (d == 3) {
			message = "Go into all the world, and tell all "
				"people of your fall from grace.";
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
				count = getrandom(count);
				for (it = vRegions.begin(); it != vRegions.end(); it++) {
					if (uvRegions[it->first] > 0)
						if (!count--)
							break;
				}
				// pick a hex within that region, and find it
				count = getrandom(it->second);
				forlist(&regions) {
					r = (ARegion *) elem;
					if (it->first == r->name->Str()) {
						if (!count--) {
							// report this hex
							message = "The ";
							message += TerrainDefs[TerrainDefs[r->type].similar_type].name;
							message += " of ";
							message += *(r->name);
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
				count = getrandom(count);
				for (it = uvRegions.begin(); it != uvRegions.end(); it++) {
					if (vRegions[it->first] == 0) {
						if (!count--)
							break;
					}
				}
				// pick a hex within that region, and find it
				count = getrandom(it->second);
				forlist(&regions) {
					r = (ARegion *) elem;
					if (it->first == r->name->Str()) {
						if (!count--) {
							// report this hex
							dir = -1;
							start = regions.FindNearestStartingCity(r, &dir);
							message = "The ";
							message += TerrainDefs[TerrainDefs[r->type].similar_type].name;
							message += " of ";
							message += *(r->name);
							if (start == r) {
								message += ", containing ";
								message += *start->town->name;
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
								message += *start->town->name;
								message += ",";
							}
							if (TerrainDefs[r->type].similar_type == R_TUNNELS)
								message += " have";
							else
								message += " has";
							message += " yet to be visited by exiles from the Eternal City.";
							WriteTimesArticle(message);
						}
					}
				}
			}
		} else if (d > 7) {
			// report exact coords of an unexplored hex
			count = getrandom(unvisited);
			forlist(&regions) {
				ARegion *r = (ARegion *)elem;
				if (r->Population() > 0 && !r->visited) {
					if (!count--) {
						message = "The people of the ";
						message += r->ShortPrint(&regions);
						switch (getrandom(4)) {
							case 0:
								message += " have not been visited by exiles.";
								break;
							case 1:
								message += " have not yet heard the news of your rebellion.";
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

	// See if anyone has won by collecting enough relics of grace
	forlist_reuse(&factions) {
		f = (Faction *) elem;
		// No accidentally sending all the Guardsmen
		// or Creatures to the Eternal City!
		if (f->is_npc)
			continue;
		reliccount = 0;
		forlist(&regions) {
			r = (ARegion *) elem;
			forlist(&r->objects) {
				o = (Object *) elem;
				forlist(&o->units) {
					u = (Unit *) elem;
					if (u->faction == f) {
						reliccount += u->items.GetNum(I_RELICOFGRACE);
					}
				}
			}
		}
		if (reliccount >= RELICS_REQUIRED_FOR_VICTORY) {
			// This faction has earned the right to go home
			units = 0;
			leaders = 0;
			men = 0;
			silver = f->unclaimed;
			stuff = 0;
			skilldays = 0;
			magicdays = 0;
			skilllevels = 0;
			magiclevels = 0;
			forlist(&regions) {
				r = (ARegion *) elem;
				forlist(&r->objects) {
					o = (Object *) elem;
					forlist(&o->units) {
						u = (Unit *) elem;
						if (u->faction == f) {
							units++;
							forlist(&u->items) {
								item = (Item *) elem;
								if (ItemDefs[item->type].type & IT_LEADER)
									leaders += item->num;
								else if (ItemDefs[item->type].type & IT_MAN)
									men += item->num;
								else if (ItemDefs[item->type].type & IT_MONEY)
									silver += item->num * ItemDefs[item->type].baseprice;
								else
									stuff += item->num * ItemDefs[item->type].baseprice;
									
							}
							forlist_reuse(&u->skills) {
								s = (Skill *) elem;
								if (SkillDefs[s->type].flags & SkillType::MAGIC) {
									magicdays += s->days * SkillDefs[s->type].cost;
									magiclevels += GetLevelByDays(s->days / u->GetMen()) * u->GetMen();
								} else {
									skilldays += s->days * SkillDefs[s->type].cost;
									skilllevels += GetLevelByDays(s->days / u->GetMen()) * u->GetMen();
								}
							}
							// Should really move this unit somewhere they'll be cleaned up,
							// but given that the appropriate place for that function is
							// r->hell, this doesn't seem right given what's happened.
							// In this case, I'm willing to leak memory :-)
							o->units.Remove(u);
						}
					}
				}
			}
			f->exists = 0;
			f->quit = QUIT_WON_GAME;
			f->temformat = TEMPLATE_OFF;
			temp = " have acquired ";
			if (reliccount == 1) {
				temp += "a ";
				temp += ItemDefs[I_RELICOFGRACE].name;
			} else {
				temp += reliccount;
				temp += " ";
				temp += ItemDefs[I_RELICOFGRACE].names;
			}
			temp += " and returned to the Eternal City.";
			message = AString("You") + temp;
			times = *f->name + temp;
			f->event(message.const_str(), "gameover");
			message = "You";
			times += "\n\nThey";
			temp = " returned after ";
			temp += TurnNumber() - f->startturn;
			temp += " months, with ";
			temp += units;
			temp += " unit";
			if (units != 1)
				temp += "s";
			temp += " comprising ";
			if (leaders > 0) {
				temp += leaders;
				temp += " leader";
				if (leaders != 1)
					temp += "s";
			}
			if (leaders > 0 && men > 0)
				temp += " and ";
			if (men > 0) {
				temp += men;
				temp += " other m";
				if (leaders != 1)
					temp += "en";
				else
					temp += "an";
			}
			message += temp;
			times += temp;
			if (silver > 0 || stuff > 0) {
				message += ", and bringing with you ";
				times += ", and bringing with them ";
				temp = "";
				if (silver > 0) {
					temp += silver;
					temp += " silver";
				}
				if (silver > 0 && stuff > 0)
					temp += " and ";
				if (stuff > 0) {
					temp += "goods worth ";
					temp += stuff;
					temp += " silver";
				}
				temp += ".";
				message += temp;
				times += temp;
			}
			if (skilllevels > 0 || magiclevels > 0) {
				temp = " had acquired ";
				if (skilllevels > 0) {
					temp += skilllevels;
					temp += " level";
					if (skilllevels != 1)
						temp += "s";
					temp += " in mundane skills, worth ";
					temp += (int) (skilldays / 30);
					temp += " silver in tuition costs";
				}
				if (skilllevels > 0 && magiclevels > 0)
					temp += ", and ";
				if (magiclevels > 0) {
					temp += magiclevels;
					temp += " level";
					if (magiclevels != 1)
						temp += "s";
					temp += " in magic skills, worth ";
					temp += (int) (magicdays / 30);
					temp += " silver in tuition costs";
				}
				temp += ".";
				message += "  You";
				message += temp;
				times += "  They";
				times += temp;
			}
			f->event(message.const_str(), "gameover");
			WriteTimesArticle(times);

			string filename = "winner." + to_string(f->num);
			ofstream wf(filename, ios::out | ios::ate);

			if (wf.is_open()) {
				message = TurnNumber();
				message += ", ";
				message += f->startturn;
				message += ", ";
				message += units;
				message += ", ";
				message += leaders;
				message += ", ";
				message += men;
				message += ", ";
				message += silver;
				message += ", ";
				message += stuff;
				message += ", ";
				message += skilllevels;
				message += ", ";
				message += skilldays;
				message += ", ";
				message += magiclevels;
				message += ", ";
				message += magicdays;
				message += ", ";
				message += f->num;
				message += ", ";
				message += *f->address;
				message += ", ";
				message += *f->name;
				message += "\n";
				wf << message;
			}
		}
	}

	forlist_reuse(&regions) {
		r = (ARegion *) elem;
		forlist(&r->objects) {
			o = (Object *) elem;
			if (o->type == O_BKEEP) {
				if (!o->incomplete) {
					// You didn't think this was a
					// _win_ condition, did you?
					message = "A blasphemous tower has been completed!\n\n"
						"The connection between Havilah and the Eternal City has been severed.\n\n"
						"The light fails; darkness falls forever, and all life perishes under endless ice.";
					WriteTimesArticle(message);
					return GetFaction(&factions, monfaction);
				}
				if (o->incomplete <= ObjectDefs[o->type].cost / 2) {
					// Half done; make a quest to destroy it
					found = 0;
					forlist(&quests) {
						q = (Quest *) elem;
						if (q->type == Quest::DEMOLISH &&
								q->target == o->num &&
								q->regionnum == r->num) {
							found = 1;
							break;
						}
					}
					if (!found) {
						q = new Quest;
						q->type = Quest::DEMOLISH;
						item = new Item;
						item->type = I_RELICOFGRACE;
						item->num = 1;
						q->rewards.Add(item);
						q->target = o->num;
						q->regionnum = r->num;
						quests.Add(q);
					}
				}
			}
		}
	}

	forlist_reuse(&quests) {
		q = (Quest *) elem;
		switch(q->type) {
			case Quest::SLAY:
				l = regions.FindUnit(q->target);
				if (!l || l->unit->faction->num != monfaction) {
					// Something has gone wrong with this quest!
					// shouldn't ever happen, but...
					quests.Remove(q);
					delete q;
					if (l) delete l;
				} else {
					message = "In the ";
					message += TerrainDefs[TerrainDefs[l->region->type].similar_type].name;
					message += " of ";
					message += *(l->region->name);
					if (l->obj->type == O_DUMMY)
						message += " roams";
					else
						message += " lurks";
					message += " the ";
					message += *(l->unit->name);
					message += ".  Free the world from this menace and be rewarded!";
					WriteTimesArticle(message);
					delete l;
				}

				break;
			case Quest::HARVEST:
				r = regions.GetRegion(q->regionnum);
				message = "Seek a token of the Maker's bounty amongst the ";
				message += ItemDefs[q->objective.type].names;
				message += " of ";
				message += *r->name;
				message += ".";
				WriteTimesArticle(message);
				break;
			case Quest::BUILD:
				message = "Build a ";
				message += ObjectDefs[q->building].name;
				message += " in ";
				message += q->regionname;
				message += " for the glory of the Maker.";
				WriteTimesArticle(message);
				break;
			case Quest::VISIT:
				message = "Show your devotion by visiting ";
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
					quests.Remove(q);
					delete q;
				} else {
					message = "Tear down the blasphemous ";
					message += *o->name;
					message += " : ";
					message += ObjectDefs[o->type].name;
					message += " in ";
					message += *r->name;
					message += "!";
					WriteTimesArticle(message);
				}
				break;
			default:
				break;
		}
	}

	return NULL;
}

void Game::ModifyTablesPerRuleset(void)
{
	if (Globals->APPRENTICES_EXIST)
		EnableSkill(S_MANIPULATE);

	if (!Globals->GATES_EXIST)
		DisableSkill(S_GATE_LORE);

	if (Globals->FULL_TRUESEEING_BONUS) {
		ModifyAttribMod("observation", 1, AttribModItem::SKILL,
				"TRUE", AttribModItem::UNIT_LEVEL, 1);
	}
	if (Globals->IMPROVED_AMTS) {
		ModifyAttribMod("observation", 2, AttribModItem::ITEM,
				"AMTS", AttribModItem::CONSTANT, 3);
	}
	if (Globals->FULL_INVIS_ON_SELF) {
		ModifyAttribMod("stealth", 3, AttribModItem::SKILL,
				"INVI", AttribModItem::UNIT_LEVEL, 1);
	}

	if (Globals->NEXUS_IS_CITY && Globals->TOWNS_EXIST) {
		ClearTerrainRaces(R_NEXUS);
		ModifyTerrainRace(R_NEXUS, 0, I_HIGHELF);
		ModifyTerrainRace(R_NEXUS, 1, I_VIKING);
		ModifyTerrainRace(R_NEXUS, 2, I_PLAINSMAN);
		ClearTerrainItems(R_NEXUS);
		ModifyTerrainItems(R_NEXUS, 0, I_IRON, 100, 10);
		ModifyTerrainItems(R_NEXUS, 1, I_WOOD, 100, 10);
		ModifyTerrainItems(R_NEXUS, 2, I_STONE, 100, 10);
		ModifyTerrainEconomy(R_NEXUS, 1000, 15, 50, 2);
	}

	EnableItem(I_PICK);
	EnableItem(I_SPEAR);
	EnableItem(I_AXE);
	EnableItem(I_HAMMER);
	EnableItem(I_MCROSSBOW);
	EnableItem(I_MWAGON);
	EnableItem(I_GLIDER);
	EnableItem(I_NET);
	EnableItem(I_LASSO);
	EnableItem(I_BAG);
	EnableItem(I_SPINNING);
	EnableItem(I_LEATHERARMOR);
	ModifyArmorFlags("LARM", ArmorType::USEINASSASSINATE);
	ModifyWeaponAttack("DBOW",
			ARMORPIERCING,
			ATTACK_RANGED,
			WeaponType::NUM_ATTACKS_HALF_SKILL, 1);
	// Make DBOWs require just LBOW, not XBOW?  And downgrade
	// them from ARMORPIERCING to just PIERCING?
	ModifyWeaponAttack("RUNE",
			SLASHING,
			ATTACK_COMBAT,
			WeaponType::NUM_ATTACKS_HALF_SKILL, 1);
	// EnableItem(I_CLOTHARMOR);
	EnableItem(I_BOOTS);
	EnableItem(I_BAXE);
	// EnableItem(I_MBAXE);
	// EnableItem(I_IMARM);
	// EnableItem(I_SUPERBOW);
	// EnableItem(I_LANCE);
	// EnableItem(I_JAVELIN);
	// EnableItem(I_PIKE);
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
	EnableItem(I_RELICOFGRACE);

	// Cut down the number of trade items to improve
	// chances of good trade routes
	DisableItem(I_FIGURINES);
	DisableItem(I_TAROTCARDS);
	DisableItem(I_CAVIAR);
	DisableItem(I_CHOCOLATE);
	DisableItem(I_ROSES);
	DisableItem(I_VELVET);
	DisableItem(I_CASHMERE);
	DisableItem(I_WOOL);

	// EnableItem(I_FOOD);
	// EnableSkill(S_COOKING);
	// EnableSkill(S_CREATE_FOOD);

	DisableItem(I_STAFFOFL);
	DisableItem(I_GNOME);

	// EnableSkill(S_ARMORCRAFT);
	// EnableSkill(S_WEAPONCRAFT);
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
	EnableSkill(S_TRANSMUTATION);
	EnableSkill(S_BLASPHEMOUS_RITUAL);
	EnableSkill(S_ENDURANCE);

	ModifySkillDependancy(S_RAISE_UNDEAD, 0, "SUSK", 3);
	ModifySkillDependancy(S_SUMMON_LICH, 0, "RAIS", 3);
	// ModifySkillDependancy(S_CREATE_AURA_OF_FEAR, 0, "DEMO", 1);

	ModifyItemMagicOutput(I_SKELETON, 200);
	ModifyItemMagicOutput(I_WOLF, 200);
	ModifyItemMagicOutput(I_IMP, 200);
	ModifyItemMagicOutput(I_UNDEAD, 100);
	ModifyItemMagicOutput(I_EAGLE, 100);
	ModifyItemMagicOutput(I_DEMON, 100);
	ModifyItemMagicOutput(I_DRAGON, 20);
	ModifyItemMagicOutput(I_LICH, 20);
	ModifyItemEscape(I_IMP,
			ItemType::ESC_LEV_LINEAR | ItemType::LOSE_LINKED,
			"SUIM",
			20);
	ModifyItemEscape(I_DEMON,
			ItemType::ESC_LEV_LINEAR | ItemType::LOSE_LINKED,
			"SUDE",
			20);
	ModifyItemEscape(I_BALROG,
			ItemType::ESC_LEV_LINEAR | ItemType::LOSE_LINKED,
			"SUBA",
			20);
	ModifyMonsterSpecial("LICH", "icebreath", 4);
	ModifyMonsterSpecial("BALR", "fear", 6);
	ModifyMonsterAttacksAndHits("SKEL", 2, 2, 0, 1);
	ModifyMonsterAttacksAndHits("UNDE", 10, 10, 0, 1);
	ModifyMonsterAttacksAndHits("DEMO", 6, 6, 0, 1);
	ModifyMonsterAttackLevel("EAGL", 4);
	ModifyMonsterDefense("EAGL", ATTACK_COMBAT, 4);
	ModifyMonsterDefense("EAGL", ATTACK_WEATHER, 4);
	ModifyMonsterDefense("WOLF", ATTACK_ENERGY, 2);
	ModifyMonsterDefense("WOLF", ATTACK_WEATHER, 2);
	ModifyMonsterAttacksAndHits("EAGL", 3, 3, 0, 1);
	ModifyMonsterSkills("LICH", 4, 0, 3);
	ModifyMonsterSkills("BALR", 5, 1, 2);
	ModifyMonsterSkills("DEMO", 2, 2, 3);
	ModifyMonsterSkills("WOLF", 1, 2, 3);
	ModifyMonsterSkills("EAGL", 2, 2, 4);
	ModifyMonsterSpoils("BALR", 30000, IT_MAGIC);

	DisableSkill(S_CREATE_STAFF_OF_LIGHTNING);

	ModifyRaceSkills("NOMA", 3, "RIDI");
	ModifyRaceSkills("DMAN", 3, "WEAP");
	ModifyRaceSkills("BARB", 0, "RIDI");
	ModifyRaceSkills("HELF", 0, "MANI");
	ModifyRaceSkills("HELF", 3, "LBOW");

	EnableObject(O_ROADN);
	EnableObject(O_ROADNE);
	EnableObject(O_ROADNW);
	EnableObject(O_ROADS);
	EnableObject(O_ROADSE);
	EnableObject(O_ROADSW);
	EnableObject(O_TEMPLE);
	EnableObject(O_MQUARRY);
	EnableObject(O_AMINE);
	EnableObject(O_PRESERVE);
	EnableObject(O_SACGROVE);
	EnableObject(O_MTOWER);
	EnableObject(O_MFORTRESS);
	EnableObject(O_MCITADEL);
	ModifyObjectName(O_MFORTRESS, "Magical Fortress");
	ModifyObjectName(O_MCASTLE, "Magical Castle");
	ModifyObjectManpower(O_TOWER,
		ObjectDefs[O_TOWER].protect,
		ObjectDefs[O_TOWER].capacity,
		ObjectDefs[O_TOWER].sailors,
		0);
	ModifyObjectManpower(O_FORT,
		ObjectDefs[O_FORT].protect,
		ObjectDefs[O_FORT].capacity,
		ObjectDefs[O_FORT].sailors,
		1);
	ModifyObjectManpower(O_CASTLE,
		ObjectDefs[O_CASTLE].protect,
		ObjectDefs[O_CASTLE].capacity,
		ObjectDefs[O_CASTLE].sailors,
		2);
	ModifyObjectManpower(O_CITADEL,
		ObjectDefs[O_CITADEL].protect,
		ObjectDefs[O_CITADEL].capacity,
		ObjectDefs[O_CITADEL].sailors,
		4);
	// DisableObject(O_BKEEP);
	ModifyObjectName(O_BKEEP, "Black Tower");
	ModifyObjectFlags(O_BKEEP, ObjectType::CANENTER |
		ObjectType::NEVERDECAY |
		ObjectType::CANMODIFY);
	ModifyObjectMonster(O_BKEEP, -1);
	ModifyObjectConstruction(O_BKEEP, I_ROOTSTONE, 666, NULL, 0);

	ModifyTerrainItems(R_PLAIN, 0, I_HORSE, 25, 20);
	ModifyTerrainItems(R_PLAIN, 1, -1, 0, 0);
	ModifyTerrainItems(R_TUNDRA, 2, I_WHORSE, 25, 5);
	ModifyTerrainItems(R_DESERT, 3, I_HORSE, 25, 10);
	ModifyTerrainItems(R_JUNGLE, 3, I_IRONWOOD, 20, 5);
	ModifyTerrainItems(R_UFOREST, 4, I_IRONWOOD, 20, 5);

	ModifyItemMagicInput(I_RINGOFI, 0, I_MITHRIL, 1);
	ModifyItemMagicInput(I_RINGOFI, 1, I_SILVER, 600);
	ModifyItemMagicInput(I_CLOAKOFI, 0, I_FUR, 1);
	ModifyItemMagicInput(I_CLOAKOFI, 1, I_SILVER, 800);
	ModifyItemMagicInput(I_STAFFOFF, 0, I_IRONWOOD, 1);
	ModifyItemMagicInput(I_STAFFOFF, 1, I_SILVER, 500);
	ModifyItemMagicInput(I_STAFFOFL, 0, I_IRONWOOD, 1);
	ModifyItemMagicInput(I_STAFFOFL, 1, I_SILVER, 900);
	ModifyItemMagicInput(I_AMULETOFTS, 0, I_ROOTSTONE, 1);
	ModifyItemMagicInput(I_AMULETOFTS, 1, I_SILVER, 500);
	ModifyItemMagicInput(I_AMULETOFP, 0, I_STONE, 1);
	ModifyItemMagicInput(I_AMULETOFP, 1, I_SILVER, 200);
	ModifyItemMagicInput(I_RUNESWORD, 0, I_MSWORD, 1);
	ModifyItemMagicInput(I_RUNESWORD, 1, I_SILVER, 600);
	ModifyItemMagicInput(I_SHIELDSTONE, 0, I_STONE, 1);
	ModifyItemMagicInput(I_SHIELDSTONE, 1, I_SILVER, 200);
	ModifyItemMagicInput(I_MCARPET, 0, I_FUR, 1);
	ModifyItemMagicInput(I_MCARPET, 1, I_SILVER, 400);
	ModifyItemMagicInput(I_PORTAL, 0, I_ROOTSTONE, 1);
	ModifyItemMagicInput(I_PORTAL, 1, I_SILVER, 500);
	ModifyItemMagicInput(I_FSWORD, 0, I_MSWORD, 1);
	ModifyItemMagicInput(I_FSWORD, 1, I_SILVER, 600);

	ModifyHealing(2, 15, 60);
	ModifyHealing(4, 50, 80);

	EnableObject(O_ISLE);
	EnableObject(O_DERELICT);
	EnableObject(O_OCAVE);
	EnableObject(O_WHIRL);
	DisableObject(O_PALACE);
	EnableItem(I_PIRATES);
	EnableItem(I_KRAKEN);
	EnableItem(I_MERFOLK);
	EnableItem(I_ELEMENTAL);

	if ((Globals->UNDERDEEP_LEVELS > 0) || (Globals->UNDERWORLD_LEVELS > 1)) {
		EnableItem(I_MUSHROOM);
		EnableItem(I_HEALPOTION);
		EnableItem(I_ROUGHGEM);
		EnableItem(I_GEMS);
		EnableSkill(S_GEMCUTTING);
	}

	// Modify the various spells which are allowed to cross levels
	if (Globals->EASIER_UNDERWORLD) {
		ModifyRangeFlags("rng_teleport", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_farsight", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_clearsky", RangeType::RNG_CROSS_LEVELS);
		ModifyRangeFlags("rng_weather", RangeType::RNG_CROSS_LEVELS);
	}

	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		EnableSkill(S_QUARTERMASTER);
		EnableObject(O_CARAVANSERAI);
	}
	return;
}

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
#include <string>
#include <iterator>

#define MINIMUM_ACTIVE_QUESTS		5
#define MAXIMUM_ACTIVE_QUESTS		20
#define QUEST_EXPLORATION_PERCENT	30
#define QUEST_SPAWN_RATE		7
#define QUEST_MAX_REWARD		6000
#define QUEST_SPAWN_CHANCE		70
#define MAX_DESTINATIONS		5

int Game::SetupFaction( Faction *pFac )
{
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
	temp2->Study(S_OBSERVATION, 180);
	temp2->Study(S_FORCE, 90);
	temp2->Study(S_PATTERN, 90);
	temp2->Study(S_SPIRIT, 90);
	temp2->Study(S_GATE_LORE, 30);
	temp2->Study(S_FIRE, 30);

	// Set up health
	temp2->Study(S_COMBAT, 180);
	temp2->Study(S_ENDURANCE, 180);

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
		pFac->SetAttitude(monfaction, A_UNFRIENDLY);
	}

	return( 1 );
}

static void CreateQuest(ARegionList *regions, int monfaction)
{
	Quest *q, *q2;
	Item *item;
	int d, count, temple, i, j, clash, reward_count;
	ARegion *r;
	Object *o;
	Unit *u;
	Production *p;
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

	count = getrandom(count) + 1;

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
				reward_count = (QUEST_MAX_REWARD + getrandom(QUEST_MAX_REWARD / 2)) / ItemDefs[i].baseprice;

				printf("\nQuest reward: %s x %d.\n", ItemDefs[i].name, reward_count);
				
				// Setup reward
				item = new Item;
				item->type = i;
				item->num = reward_count;

				q->rewards.Add(item);
				break;
			}
		}
	}

	// 25% chance to drop I_RELICOFGRACE from quest in addition to regular reward
	d = getrandom(100);
	if (d < 25) {
		item = new Item;
		item->type = I_RELICOFGRACE;
		item->num = 1;
		q->rewards.Add(item);	
		printf("\nQuest reward: Relic.\n");
	}

	d = getrandom(100);
	if (d < 60) {
		// SLAY quest
		q->type = Quest::SLAY;
		count = 0;
		// Count our current monsters
		forlist(regions) {
			r = (ARegion *) elem;
			if (TerrainDefs[r->type].similar_type == R_OCEAN)
				continue;
			// No need to check if quests do not require exploration
			if (!r->visited && QUEST_EXPLORATION_PERCENT != 0)
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
			// No need to check if quests do not require exploration
			if (!r->visited && QUEST_EXPLORATION_PERCENT != 0)
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
	} else if (d < 80) {
		// Create a HARVEST quest
		count = 0;
		forlist(regions) {
			r = (ARegion *) elem;
			// Do allow lakes though
			if (r->type == R_OCEAN)
				continue;
			// No need to check if quests do not require exploration
			if (!r->visited && QUEST_EXPLORATION_PERCENT != 0)
				continue;
			forlist(&r->products) {
				p = (Production *) elem;
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
			// No need to check if quests do not require exploration
			if (!r->visited && QUEST_EXPLORATION_PERCENT != 0)
				continue;
			forlist(&r->products) {
				p = (Production *) elem;
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
			// No need to check if quests do not require exploration
			if (r->Population() > 0 && (r->visited || QUEST_EXPLORATION_PERCENT == 0)) { 
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
	int dir;
	unsigned ucount;
	Quest *q;
	ARegion *r, *start;
	Object *o;
	Unit *u;
	Faction *f;
	Location *l;
	AString message, times, temp, filename;
	Arules wf;
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
							message += " yet to be visited by exiles from destroyed worlds.";
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
					message = "Quest: In the ";
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
				message = "Quest: Seek a token of the Ancient Ones legacy amongst the ";
				message += ItemDefs[q->objective.type].names;
				message += " of ";
				message += *r->name;
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
				for(it2 = q->destinations.begin();
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
					message = "Quest: Tear down the blasphemous ";
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
	ModifyArmorFlags("LARM", ArmorType::USEINASSASSINATE);
	ModifyWeaponAttack("DBOW",
			ARMORPIERCING,
			ATTACK_RANGED,
			WeaponType::NUM_ATTACKS_HALF_SKILL);
	ModifyWeaponAttack("RUNE",
			SLASHING,
			ATTACK_COMBAT,
			WeaponType::NUM_ATTACKS_HALF_SKILL);
	EnableItem(I_CLOTHARMOR);
	EnableItem(I_BOOTS);
	EnableItem(I_BAXE);
	EnableItem(I_LANCE);
	EnableItem(I_SPEAR);
	EnableItem(I_PIKE);
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
	EnableItem(I_ROUGHGEM);
	EnableItem(I_GEMS);

	// Artifacts of power
	EnableItem(I_RELICOFGRACE);
	ModifyItemName(I_RELICOFGRACE, "artifact of power", "artifacts of power");

	// Tools
	EnableItem(I_PICK);
	EnableItem(I_AXE);
	EnableItem(I_HAMMER);
	EnableItem(I_NET);
	EnableItem(I_LASSO);
	EnableItem(I_BAG);
	EnableItem(I_SPINNING);

	ModifyItemProductionSkill(I_PIKE, "WEAP", 2);
	ModifyItemProductionSkill(I_LANCE, "WEAP", 2);
	//
	// Change craft: adamantium
	//
	EnableItem(I_ADMANTIUM);
	EnableItem(I_ADSWORD);
	EnableItem(I_ADRING);
	EnableItem(I_ADPLATE);
	ModifyItemProductionSkill(I_ADMANTIUM, "MINI", 5);
	ModifyItemProductionSkill(I_ADSWORD, "WEAP", 5);
	ModifyItemProductionSkill(I_ADRING, "ARMO", 5);
	ModifyItemProductionSkill(I_ADPLATE, "ARMO", 5);

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
	DisableItem(I_MINK);
	DisableItem(I_DYES);

	// Disable items
	DisableItem(I_SUPERBOW);

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
	EnableSkill(S_ENDURANCE);
	EnableSkill(S_GEMCUTTING);
	DisableSkill(S_CAMELTRAINING);
	DisableSkill(S_RANCHING);

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
	EnableObject(O_GEMAPPRAISER);

	DisableObject(O_PALACE);

	ModifyObjectName(O_MFORTRESS, "Magical Fortress");
	ModifyObjectName(O_MCASTLE, "Magical Castle");

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
	ModifyItemMagicInput(I_SHIELDSTONE, 0, I_STONE, 1);
	ModifyItemMagicInput(I_SHIELDSTONE, 1, I_SILVER, 200);
	ModifyItemMagicInput(I_MCARPET, 0, I_FUR, 1);
	ModifyItemMagicInput(I_MCARPET, 1, I_SILVER, 400);
	ModifyItemMagicInput(I_PORTAL, 0, I_ROOTSTONE, 1);
	ModifyItemMagicInput(I_PORTAL, 1, I_SILVER, 500);
	ModifyItemMagicInput(I_FSWORD, 0, I_MSWORD, 1);
	ModifyItemMagicInput(I_FSWORD, 1, I_SILVER, 600);

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
	
	// New v2 Monsters
	EnableItem(I_HYDRA);
	EnableItem(O_BOG);

	EnableItem(I_ICEDRAGON);
	EnableItem(O_ICECAVE);

	EnableItem(I_ILLYRTHID);
	EnableItem(O_ILAIR);

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
	ModifyRaceSkills("IDWA", 2, "GCUT");
	ModifyRaceSkills("IDWA", 3, "FARM");
	ModifyRaceSkills("IDWA", 4, "ARMO");

	EnableItem(I_UNDERDWARF);
	ModifyItemBasePrice(I_UNDERDWARF, 40);
	ModifyRaceSkillLevels("UDWA", 5, 2);
	ModifyRaceSkills("UDWA", 0, "ARMO");
	ModifyRaceSkills("UDWA", 1, "WEAP");
	ModifyRaceSkills("UDWA", 2, "COMB");
	ModifyRaceSkills("UDWA", 3, "MINI");
	ModifyRaceSkills("UDWA", 4, "GCUT");
	
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

	EnableItem(I_DROWMAN);
	ModifyItemBasePrice(I_DROWMAN, 40);
	ModifyRaceSkillLevels("DRLF", 5, 2);
	ModifyRaceSkills("DRLF", 0, "HERB");
	ModifyRaceSkills("DRLF", 1, "LBOW");
	ModifyRaceSkills("DRLF", 2, "COMB");
	ModifyRaceSkills("DRLF", 3, "WEAP");
	ModifyRaceSkills("DRLF", 4, "HEAL");

	EnableItem(I_GNOME);
	ModifyItemBasePrice(I_GNOME, 30);
	ModifyRaceSkillLevels("GNOM", 5, 2);
	ModifyRaceSkills("GNOM", 0, "HERB");
	ModifyRaceSkills("GNOM", 1, "QUAR");
	ModifyRaceSkills("GNOM", 2, "ENTE");
	ModifyRaceSkills("GNOM", 3, "XBOW");
	ModifyRaceSkills("GNOM", 4, "GCUT");
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
	ModifyRaceSkills("GNOL", 4, "FISH");

	EnableItem(I_ORC);
	ModifyItemBasePrice(I_ORC, 40);
	ModifyRaceSkillLevels("ORC", 5, 2);
	ModifyRaceSkills("ORC", 0, "MINI");
	ModifyRaceSkills("ORC", 1, "LUMB");
	ModifyRaceSkills("ORC", 2, "COMB");
	ModifyRaceSkills("ORC", 3, "BUIL");
	ModifyRaceSkills("ORC", 4, "SHIP");


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
	ModifyTerrainEconomy(R_MOUNTAIN, 600, 12, 20, 2);

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
	ModifyTerrainEconomy(R_DESERT, 500, 11, 10, 1);

	ClearTerrainRaces(R_TUNDRA);
	ModifyTerrainRace(R_TUNDRA, 0, I_ICEDWARF);
	ModifyTerrainRace(R_TUNDRA, 1, I_GNOME);
	ModifyTerrainRace(R_TUNDRA, 2, I_GNOLL);
	ModifyTerrainCoastRace(R_TUNDRA, 0, I_ICEDWARF);
	ModifyTerrainCoastRace(R_TUNDRA, 1, I_GNOME);
	ModifyTerrainCoastRace(R_TUNDRA, 2, I_GNOLL);
	ModifyTerrainEconomy(R_TUNDRA, 400, 11, 10, 2);

	// Underworld

	ClearTerrainRaces(R_CAVERN);
	ModifyTerrainRace(R_CAVERN, 0, I_DROWMAN);
	ModifyTerrainRace(R_CAVERN, 1, I_UNDERDWARF);
	ModifyTerrainRace(R_CAVERN, 2, I_MAN);
	ModifyTerrainRace(R_CAVERN, 3, I_GNOME);
	ModifyTerrainCoastRace(R_CAVERN, 0, I_MAN);
	ModifyTerrainCoastRace(R_CAVERN, 1, I_UNDERDWARF);
	ModifyTerrainCoastRace(R_CAVERN, 2, I_DROWMAN);
	ModifyTerrainEconomy(R_CAVERN, 300, 12, 10, 1);

	ClearTerrainRaces(R_UFOREST);
	ModifyTerrainRace(R_UFOREST, 0, I_DROWMAN);
	ModifyTerrainRace(R_UFOREST, 1, I_UNDERDWARF);
	ModifyTerrainRace(R_UFOREST, 2, I_GOBLINMAN);
	ModifyTerrainRace(R_UFOREST, 3, I_MAN);
	ModifyTerrainCoastRace(R_UFOREST, 0, I_DROWMAN);
	ModifyTerrainCoastRace(R_UFOREST, 1, I_UNDERDWARF);
	ModifyTerrainCoastRace(R_UFOREST, 2, I_GOBLINMAN);
	ModifyTerrainEconomy(R_UFOREST, 500, 12, 10, 2);

	ClearTerrainRaces(R_CHASM);
	ModifyTerrainRace(R_CHASM, 0, I_UNDERDWARF);
	ModifyTerrainRace(R_CHASM, 1, I_DROWMAN);
	ModifyTerrainRace(R_CHASM, 2, I_GOBLINMAN);
	ModifyTerrainRace(R_CHASM, 3, I_ORC);
	ModifyTerrainCoastRace(R_CHASM, 0, I_UNDERDWARF);
	ModifyTerrainCoastRace(R_CHASM, 1, I_DROWMAN);
	ModifyTerrainCoastRace(R_CHASM, 2, I_GOBLINMAN);
	ModifyTerrainEconomy(R_CHASM, 0, 0, 0, 3);
	// ModifyTerrainWMons(R_CHASM, 20, I_DEMON, I_BALROG, I_ETTIN);

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

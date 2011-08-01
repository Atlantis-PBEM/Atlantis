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

/*
	temp2->type = U_MAGE;
	temp2->Study(S_PATTERN, 30);
	temp2->Study(S_SPIRIT, 30);
	temp2->Study(S_GATE_LORE, 30);

	if (TurnNumber() >= 25) {
		temp2->Study(S_PATTERN, 60);
		temp2->Study(S_SPIRIT, 60);
		temp2->Study(S_FORCE, 90);
		temp2->Study(S_COMBAT, 30);
	}
*/

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

Faction *Game::CheckVictory()
{
	int visited, unvisited, d, moncount, reliccount;
	int units, leaders, men, silver, stuff;
	int skilldays, magicdays, skilllevels, magiclevels;
	Quest *q, *q2;
	Item *item;
	ARegion *r;
	Object *o;
	Unit *u;
	Faction *f;
	Skill *s;
	Location *l;
	AString message, filename;
	Arules wf;

	visited = 0;
	unvisited = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		if (TerrainDefs[r->type].similar_type != R_OCEAN &&
				r->Population() > 0) {
			if (r->visited)
				visited++;
			else
				unvisited++;
		}
	}

	printf("Players have visited %d regions; %d unvisited.\n", visited, unvisited);

	if (!unvisited) {
		// Exploration phase complete: start creating relic quests
		while (quests.Num() < 5) {
			q = new Quest;
			q->type = -1;
			item = new Item;
			item->type = I_RELICOFGRACE;
			item->num = 1;
			q->rewards.Add(item);
			d = getrandom(100);
			if (d < 100) {
				// SLAY quest
				q->type = Quest::SLAY;
				moncount = 0;
				// Count our current monsters
				forlist(&regions) {
					r = (ARegion *) elem;
					if (TerrainDefs[r->type].similar_type == R_OCEAN)
						continue;
					forlist(&r->objects) {
						o = (Object *) elem;
						forlist(&o->units) {
							u = (Unit *) elem;
							if (u->faction->num == monfaction) {
								moncount++;
							}
						}
					}
				}
				if (!moncount)
					continue;
				// pick one as the object of the quest
				d = getrandom(moncount);
				forlist_reuse(&regions) {
					r = (ARegion *) elem;
					if (TerrainDefs[r->type].similar_type == R_OCEAN)
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
			}
			if (q->type != -1)
				quests.Add(q);
		}
	} else {
		// Tell the players to get exploring :-)
		d = getrandom(6);
		if (d == 0) {
			message = "Be productive and multiply; "
				"fill the land and subdue it.";
			WriteTimesArticle(message);
		} else if (d == 1) {
			message = "Go into all the world, and tell all "
				"people of your fall from grace.";
			WriteTimesArticle(message);
		} else if (d < 4) {
			message = "Players have visited ";
			message += (visited * 100 / (visited + unvisited));
			message += "% of all inhabited regions.";
			WriteTimesArticle(message);
		} else if (d == 4) {
			// Give some pointer as to where still needs exploring
		}
	}

	// See if anyone has won by collecting enough relics of grace
	forlist_reuse(&factions) {
		f = (Faction *) elem;
		// No accidentally sending all the Guardmen
		// or Creatures to the Eternal City!
		if (f->IsNPC())
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
		if (reliccount > 6) {
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
			message = "You have acquired ";
			if (reliccount == 1) {
				message += "a ";
				message += ItemDefs[I_RELICOFGRACE].name;
			} else {
				message += reliccount;
				message += " ";
				message += ItemDefs[I_RELICOFGRACE].names;
			}
			message += " and returned to the Eternal City.";
			f->Event(message);
			message = "You returned after ";
			message += TurnNumber() - f->startturn;
			message += " months, with ";
			message += units;
			message += " unit";
			if (units != 1)
				message += "s";
			message += " comprising ";
			if (leaders > 0) {
				message += leaders;
				message += " leader";
				if (leaders != 1)
					message += "s";
			}
			if (leaders > 0 && men > 0)
				message += " and ";
			if (men > 0) {
				message += men;
				message += " other m";
				if (leaders != 1)
					message += "en";
				else
					message += "an";
			}
			if (silver > 0 || stuff > 0) {
				message += ", and bringing with you ";
				if (silver > 0) {
					message += silver;
					message += " silver";
				}
				if (silver > 0 && stuff > 0)
					message += " and ";
				if (stuff > 0) {
					message += "goods worth ";
					message += stuff;
					message += " silver";
				}
				message += ".";
			}
			if (skilllevels > 0 || magiclevels > 0) {
				message += "  You had acquired ";
				if (skilllevels > 0) {
					message += skilllevels;
					message += " level";
					if (skilllevels != 1)
						message += "s";
					message += " in mundane skills, worth ";
					message += (int) (skilldays / 30);
					message += " silver in tuition costs";
				}
				if (skilllevels > 0 && magiclevels > 0)
					message += ", and ";
				if (magiclevels > 0) {
					message += (int) (magiclevels / 30);
					message += " level";
					if (magiclevels != 1)
						message += "s";
					message += " in magic skills, worth ";
					message += magicdays;
					message += " silver in tuition costs";
				}
				message += ".";
			}
			f->Event(message);

			filename = "winner.";
			filename += f->num;
			if (wf.OpenByName(filename) != -1) {
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
				wf.PutNoFormat(message);

				wf.Close();
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
					return GetFaction(&factions, monfaction);
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
			WeaponType::NUM_ATTACKS_HALF_SKILL);
	// Make DBOWs require just LBOW, not XBOW?  And downgrade
	// them from ARMORPIERCING to just PIERCING?
	ModifyWeaponAttack("RUNE",
			SLASHING,
			ATTACK_COMBAT,
			WeaponType::NUM_ATTACKS_HALF_SKILL);
	// EnableItem(I_CLOTHARMOR);
	EnableItem(I_BOOTS);
	EnableItem(I_BAXE);
	// EnableItem(I_MBAXE);
	// EnableItem(I_IMARM);
	// EnableItem(I_SUPERBOW);
	// EnableItem(I_LANCE);
	// EnableItem(I_JAVELIN);
	// EnableItem(I_PIKE);
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

	EnableItem(I_LONGBOAT);
	EnableItem(I_CLIPPER);
	EnableItem(I_GALLEON);
	EnableItem(I_AGALLEON);
	DisableItem(I_LONGSHIP);
	DisableItem(I_KNARR);
	DisableItem(I_GALLEY);
	DisableItem(I_TRIREME);
	DisableItem(I_COG);
	DisableItem(I_CARRACK);

	// EnableSkill(S_ARMORCRAFT);
	// EnableSkill(S_WEAPONCRAFT);
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
	EnableSkill(S_SACRIFICE);

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
	ModifyObjectManpower(O_AGALLEON,
		ObjectDefs[O_AGALLEON].protect,
		ObjectDefs[O_AGALLEON].capacity,
		ObjectDefs[O_AGALLEON].sailors,
		1);
	ModifyObjectName(O_BKEEP, "Black Tower");
	ModifyObjectFlags(O_BKEEP, ObjectType::CANENTER |
		ObjectType::NEVERDECAY |
		ObjectType::CANMODIFY);
	ModifyObjectMonster(O_BKEEP, -1);
	ModifyObjectConstruction(O_BKEEP, I_ROOTSTONE, 666, NULL, 0);

	ModifyTerrainItems(R_PLAIN, 0, I_HORSE, 25, 20);
	ModifyTerrainItems(R_PLAIN, 1, -1, 0, 0);
	ModifyTerrainItems(R_TUNDRA, 2, I_WHORSE, 25, 5);
	ModifyTerrainItems(R_DESERT, 3, I_HORSE, 75, 20);
	ModifyTerrainItems(R_JUNGLE, 3, I_IRONWOOD, 20, 5);
	ModifyTerrainItems(R_UFOREST, 4, I_IRONWOOD, 20, 5);
	ModifyTerrainItems(R_MOUNTAIN, 4, I_WOOD, 10, 5);
	ModifyTerrainItems(R_MOUNTAIN, 5, I_YEW, 10, 5);

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

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

#include <time.h>

#include "game.h"
#include "gamedata.h"
#include "fileio.h"

#include <map>
#include <sstream>

AString NumToWord(int n)
{
	if (n > 20) return AString(n);
	switch(n) {
		case  0: return AString("zero");
		case  1: return AString("one");
		case  2: return AString("two");
		case  3: return AString("three");
		case  4: return AString("four");
		case  5: return AString("five");
		case  6: return AString("six");
		case  7: return AString("seven");
		case  8: return AString("eight");
		case  9: return AString("nine");
		case 10: return AString("ten");
		case 11: return AString("eleven");
		case 12: return AString("twelve");
		case 13: return AString("thirteen");
		case 14: return AString("fourteen");
		case 15: return AString("fifteen");
		case 16: return AString("sixteen");
		case 17: return AString("seventeen");
		case 18: return AString("eighteen");
		case 19: return AString("nineteen");
		case 20: return AString("twenty");
	}
	return AString("error");
}

int StudyRate(int days, int exp)
{
	SkillList *sl = new SkillList;
	sl->SetDays(1,days);
	sl->SetExp(1,exp);
	int rate = sl->GetStudyRate(1, 1);
	delete sl;
	return rate;
}

// "a", "a and b", "a, b and c" (or "or" instead of "and").
static std::string joinList(const std::vector<std::string>& items, const char *last = "and") {
	std::string out;
	for (size_t n = 0; n < items.size(); n++) {
		if (n > 0) out += (n == items.size() - 1) ? (std::string(" ") + last + " ") : ", ";
		out += items[n];
	}
	return out;
}

// The terrain types a player can meet: the surface terrains flagged SHOW_RULES, lakes when
// the ruleset makes them (LAKES), and the underground terrains every ruleset with an
// underworld uses (each world.cpp creates caverns, underforests, tunnels and chasms).
static std::vector<int> worldTerrains() {
	std::vector<int> terrains;
	for (int t = 0; t < R_NUM; t++) {
		if (TerrainDefs[t].flags & TerrainType::SHOW_RULES) terrains.push_back(t);
	}
	if (Globals->LAKES > 0) terrains.push_back(R_LAKE);
	if (Globals->UNDERWORLD_LEVELS > 0) {
		for (int t : { R_CAVERN, R_UFOREST, R_TUNNELS, R_CHASM }) terrains.push_back(t);
	}
	return terrains;
}

// Point usage in FactionTypes order -- the order the turn report uses -- e.g.
// "3 points on Martial and 2 points on Magic". (fac.type is an unordered_map, so iterating it
// directly gave an arbitrary order.)
void writeFactionPointUsage(std::ostringstream& buffer, Faction &fac) {
	std::vector<std::string> items;
	for (auto &ft : *FactionTypes) {
		int value = fac.type[ft];
		if (value <= 0) continue;
		items.push_back(std::to_string(value) + " " + plural(value, "point", "points") + " on " + ft);
	}
	buffer << joinList(items);
}

// Exactly the string the turn report shows for a faction type ("Martial 3, Magic 2").
void writeFactionDefinition(std::ostringstream& buffer, Faction &fac) {
	buffer << fac.FactionTypeStr().Str();
}

// Writes "would be able to ... [, but could not ...]" for a faction type. Every number comes
// from the ruleset's limit tables (AllowedMartial/Taxes/Trades/Mages/Apprentices/
// QuarterMasters/Tacticians), including limits that are not zero at 0 points -- NewOrigins
// allows 1 mage and 1 apprentice with no Magic points -- so the text can't contradict the
// table printed above it.
void Game::WriteFactionTypeDescription(std::ostringstream& buffer, Faction &fac) {
	auto count = [](int n, const char *one, const char *many) {
		return std::to_string(n) + " " + plural(n, one, many);
	};
	std::vector<std::string> can, cannot, have, haveNone;

	if (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT) {
		int nw = AllowedTaxes(&fac);
		int nt = AllowedTrades(&fac);
		if (nw > 0) can.push_back("tax in " + count(nw, "region", "regions"));
		else cannot.push_back("tax in any region");
		if (nt > 0) can.push_back("perform trade in " + count(nt, "region", "regions"));
		else cannot.push_back("perform trade in any region");
	} else {
		int nma = AllowedMartial(&fac);
		bool merged = Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED;
		if (nma > 0) {
			can.push_back(std::string("perform ") + (merged ? "tax and trade" : "tax or trade") +
				" in " + count(nma, "region", "regions"));
		} else {
			cannot.push_back("tax or trade in any region");
		}
	}

	std::string apprentices = std::string(Globals->APPRENTICE_NAME) + "s";
	struct Limit { int n; std::string one, many; bool shown; };
	Limit limits[] = {
		{ AllowedMages(&fac), "mage", "mages", true },
		{ AllowedApprentices(&fac), Globals->APPRENTICE_NAME, apprentices,
			(bool) Globals->APPRENTICES_EXIST },
		{ AllowedQuarterMasters(&fac), "quartermaster", "quartermasters",
			(bool) (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) },
		{ AllowedTacticians(&fac), "tactician", "tacticians", (bool) Globals->TACTICS_NEEDS_WAR },
	};
	for (auto &l : limits) {
		if (!l.shown) continue;
		if (l.n > 0) have.push_back(count(l.n, l.one.c_str(), l.many.c_str()));
		else haveNone.push_back(l.many);
	}
	if (!have.empty()) can.push_back("have " + joinList(have));
	if (!haveNone.empty()) cannot.push_back("have any " + joinList(haveNone, "or"));

	if (!can.empty()) buffer << "would be able to " << joinList(can);
	if (!cannot.empty()) {
		buffer << (can.empty() ? "" : ", but ") << "could not " << joinList(cannot, "or");
	}
}

// True if some enabled item is refused by TRANSPORT/DISTRIBUTE -- the same test as
// ParseTransportableItem in items.cpp. Used to point the order entries at the list of such
// items in the "Transportation of goods" section.
static int SomeItemsNotTransportable()
{
	for (int i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (ItemDefs[i].flags & (ItemType::NOTRANSPORT | ItemType::CANTGIVE)) return 1;
	}
	return 0;
}

// LLS - converted HTML tags to lowercase
int Game::GenRules(const AString &rules, const AString &css,
		const AString &intro)
{
	Ainfile introf;
	Arules f;
	AString temp, temp2;
	int cap;
	int i, j, k, l;
	int last = -1;
	AString skname;
	SkillType *pS;

	if (f.OpenByName(rules) == -1) {
		return 0;
	}

	// Which faction areas govern taxing and trade. With FACTION_ACTIVITY DEFAULT they are War
	// and Trade; with MARTIAL / MARTIAL_MERGED (NewOrigins) both are Martial (Game::Game()
	// registers only Martial and Magic, and AllowedTaxes/AllowedTrades/AllowedMartial,
	// AllowedQuarterMasters use the Martial points). Every sentence that names the faction
	// area needed for taxing, pillaging, producing, building or quartermasters uses these, so
	// a Martial ruleset never tells players about War or Trade factions.
	int martial_areas = (Globals->FACTION_ACTIVITY != FactionActivityRules::DEFAULT);
	AString tax_factions = martial_areas ?
		"factions with Faction Points in Martial" : "War factions";
	AString trade_factions = martial_areas ?
		"factions with Faction Points in Martial" : "Trade factions";
	auto capitalized = [](const AString &text) {
		AString result = text;
		std::string str = result.Str();
		if (!str.empty()) str[0] = toupper(str[0]);
		return AString(str.c_str());
	};

	if (introf.OpenByName(intro) == -1) {
		return 0;
	}

	int qm_exist = (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT);
	if (qm_exist) {
		/* Make sure the S_QUARTERMASTER skill is enabled */
		if (SkillDefs[S_QUARTERMASTER].flags & SkillType::DISABLED)
			qm_exist = 0;
	}
	int found = 0;
	if (qm_exist) {
		/* Make there is an enabled building with transport set */
		for (i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (ObjectDefs[i].flags & ObjectType::TRANSPORT) {
				found = 1;
				break;
			}
		}
		if (!found) qm_exist = 0;
	}

	f.PutStr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 "
			"Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">");
	f.Enclose(1, "html");
	f.Enclose(1, "head");
	f.PutStr("<meta http-equiv=\"Content-Type\" content=\"text/html; "
			"charset=utf-8\">");
	f.PutStr(AString("<link type=\"text/css\" rel=\"stylesheet\" href=\"")+
			css + "\">");
	temp = AString(Globals->RULESET_NAME) + " " +
		ATL_VER_STR(Globals->RULESET_VERSION);
	temp2 = temp + " Rules";
	f.TagText("title", temp2);
	f.Enclose(0, "head");
	f.Enclose(1, "body");
	f.Enclose(1, "center");
	f.TagText("h1", AString("Rules for ") + temp);
	f.TagText("h1", AString("Based on Atlantis v") +
			ATL_VER_STR(CURRENT_ATL_VER));
	f.TagText("h2", AString("Copyright 1996 by Geoff Dunbar"));
	f.TagText("h2", AString("Based on Russell Wallace's Draft Rules"));
	f.TagText("h2", AString("Copyright 1993 by Russell Wallace"));
	char buf[500];
	time_t tval = time(NULL);
	struct tm *ltval = localtime(&tval);
	strftime(buf, 500, "%B %d, %Y", ltval);
	f.TagText("h3", AString("Last Change: ")+buf);
	f.Enclose(0, "center");
	f.ClassTagText("div", "rule", "");
	temp = "Note: This document is subject to change, as errors are found "
		"and corrected, and rules sometimes change. Be sure you have the "
		"latest available copy.";
	f.Paragraph(temp);
	f.LinkRef("table_of_contents");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Table of Contents");
	temp = AString("Thanks to ") +
		f.Link("mailto:ken@satori.gso.uri.edu","Kenneth Casey")+
		" for putting together this table of contents.";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#intro", "Introduction"));
	f.Enclose(1, "li");
	f.PutStr(f.Link("#playing", "Playing Atlantis"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#playing_factions", "Factions"));
	f.TagText("li", f.Link("#playing_units", "Units"));
	f.TagText("li", f.Link("#playing_turns", "Turns"));
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr(f.Link("#world", "The World"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#world_regions", "Regions"));
	f.TagText("li", f.Link("#region_resources", "Region Resources"));
	f.TagText("li", f.Link("#world_structures", "Structures"));
	if (Globals->NEXUS_EXISTS) {
		temp = "Atlantis Nexus";
		f.TagText("li", f.Link("#world_nexus", temp));
	}
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr(f.Link("#movement", "Movement"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#movement_normal", "Normal Movement"));
	if (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		f.TagText("li", f.Link("#movement_sailing", "Sailing"));
	f.TagText("li", f.Link("#movement_order", "Order of Movement"));
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr(f.Link("#skills", "Skills"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#skills_limitations", "Limitations"));
	f.TagText("li", f.Link("#skills_studying", "Studying"));
	f.TagText("li", f.Link("#skills_teaching", "Teaching"));
	f.TagText("li", f.Link("#skills_skillreports", "Skill Reports"));
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr(f.Link("#economy", "The Economy"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#economy_maintenance", "Maintenance Costs"));
	f.TagText("li", f.Link("#economy_recruiting", "Recruiting"));
	f.TagText("li", f.Link("#economy_items", "Items"));
	if (Globals->TOWNS_EXIST)
		f.TagText("li", f.Link("#economy_towns", "Villages, Towns, Cities"));
	f.TagText("li", f.Link("#economy_buildings",
				"Buildings and Trade Structures"));
	if (!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED))
		f.TagText("li", f.Link("#economy_roads", "Roads"));
	if (Globals->DECAY)
		f.TagText("li", f.Link("#economy_builddecay", "Building Decay"));
	int may_sail = (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED)) &&
		(!(SkillDefs[S_SHIPBUILDING].flags & SkillType::DISABLED));
	if (may_sail)
		f.TagText("li", f.Link("#economy_ships", "Ships"));
	f.TagText("li", f.Link("#economy_advanceditems", "Advanced Items"));
	f.TagText("li", f.Link("#economy_income", "Income"));
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f.TagText("li", f.Link("#economy_entertainment", "Entertainment"));
	f.TagText("li", f.Link("#economy_taxingpillaging", "Taxing/Pillaging"));
	if (qm_exist)
		f.TagText("li", f.Link("#economy_transport", "Transporting goods"));
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr(f.Link("#com", "Combat"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#com_attitudes", "Attitudes"));
	f.TagText("li", f.Link("#com_attacking", "Attacking"));
	f.TagText("li", f.Link("#com_muster", "The Muster"));
	f.TagText("li", f.Link("#com_thebattle", "The Battle"));
	f.TagText("li", f.Link("#com_victory", "Victory!"));
	f.Enclose(0, "ul");
	f.Enclose(0, "li");

	int has_stea = !(SkillDefs[S_STEALTH].flags & SkillType::DISABLED);
	int has_obse = !(SkillDefs[S_OBSERVATION].flags & SkillType::DISABLED);
	int app_exist = (Globals->APPRENTICES_EXIST);
	if (app_exist) {
		found = 0;
		/* Make sure we have a skill with the APPRENTICE flag */
		for (i = 0; i < NSKILLS; i++) {
			if (SkillDefs[i].flags & SkillType::DISABLED) continue;
			if (SkillDefs[i].flags & SkillType::APPRENTICE) {
				found = 1;
				break;
			}
		}
		if (!found) app_exist = 0;
	}
	if (has_stea || has_obse) {
		if (has_stea) temp = "Stealth";
		else temp = "";
		if (has_obse) {
			if (has_stea) temp += " and ";
			temp += "Observation";
		}
		f.Enclose(1, "li");
		f.PutStr(f.Link("#stealthobs", temp));
		if (has_stea) {
			f.Enclose(1, "ul");
			f.TagText("li", f.Link("#stealthobs_stealing", "Stealing"));
			f.TagText("li", f.Link("#stealthobs_assassination",
						"Assassination"));
			f.Enclose(0, "ul");
		}
		f.Enclose(0, "li");
	}
	f.Enclose(1, "li");
	f.PutStr(f.Link("#magic", "Magic"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#magic_skills", "Magic Skills"));
	f.TagText("li", f.Link("#magic_foundations", "Foundations"));
	f.TagText("li", f.Link("#magic_furtherstudy", "Further Magic Study"));
	f.TagText("li", f.Link("#magic_usingmagic", "Using Magic"));
	f.TagText("li", f.Link("#magic_incombat", "Mages In Combat"));
	if (app_exist) {
		temp = "#magic_";
		temp += Globals->APPRENTICE_NAME;
		temp += "s";
		temp2 = (char) toupper(Globals->APPRENTICE_NAME[0]);
		temp2 += Globals->APPRENTICE_NAME + 1;
		temp2 += "s";
		f.TagText("li", f.Link(temp, temp2));
	}
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr(f.Link("#nonplayers", "Non-Player Units"));
	f.Enclose(1, "ul");
	if (Globals->TOWNS_EXIST && Globals->CITY_MONSTERS_EXIST) {
		f.TagText("li", f.Link("#nonplayers_guards",
					"City and Town Guardsmen"));
	}
	if (Globals->WANDERING_MONSTERS_EXIST) {
		f.TagText("li", f.Link("#nonplayers_monsters", "Wandering Monsters"));
	}
	f.TagText("li", f.Link("#nonplayers_controlled", "Controlled Monsters"));
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr(f.Link("#orders", "Orders"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#orders_abbreviations", "Abbreviations"));
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr(f.Link("#ordersummary", "Order Summary"));
	f.Enclose(1, "ul");
	f.TagText("li", f.Link("#address", "address"));
	f.TagText("li", f.Link("#advance", "advance"));
	if (Globals->USE_WEAPON_ARMOR_COMMAND)
		f.TagText("li", f.Link("#armor", "armor"));
	if (has_stea)
		f.TagText("li", f.Link("#assassinate", "assassinate"));
	f.TagText("li", f.Link("#attack", "attack"));
	f.TagText("li", f.Link("#autotax", "autotax"));
	f.TagText("li", f.Link("#avoid", "avoid"));
	f.TagText("li", f.Link("#behind", "behind"));
	f.TagText("li", f.Link("#build", "build"));
	f.TagText("li", f.Link("#buy", "buy"));
	f.TagText("li", f.Link("#cast", "cast"));
	f.TagText("li", f.Link("#claim", "claim"));
	f.TagText("li", f.Link("#combat", "combat"));
	if (Globals->FOOD_ITEMS_EXIST)
		f.TagText("li", f.Link("#consume", "consume"));
	f.TagText("li", f.Link("#declare", "declare"));
	f.TagText("li", f.Link("#describe", "describe"));
	f.TagText("li", f.Link("#destroy", "destroy"));
	if (qm_exist)
		f.TagText("li", f.Link("#distribute", "distribute"));
	f.TagText("li", f.Link("#enter", "enter"));
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f.TagText("li", f.Link("#entertain", "entertain"));
	f.TagText("li", f.Link("#evict", "evict"));
	f.TagText("li", f.Link("#exchange", "exchange"));
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		f.TagText("li", f.Link("#faction", "faction"));

	if (Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f.TagText("li", f.Link("#find", "find"));
	}

	f.TagText("li", f.Link("#forget", "forget"));
	f.TagText("li", f.Link("#form", "form"));
	f.TagText("li", f.Link("#give", "give"));
	f.TagText("li", f.Link("#guard", "guard"));
	f.TagText("li", f.Link("#hold", "hold"));
	f.TagText("li", f.Link("#idle", "idle"));
	f.TagText("li", f.Link("#join", "join"));
	f.TagText("li", f.Link("#leave", "leave"));
	f.TagText("li", f.Link("#move", "move"));
	f.TagText("li", f.Link("#name", "name"));
	f.TagText("li", f.Link("#noaid", "noaid"));
	int move_over_water = 0;
	if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE)
		move_over_water = 1;
	if (!move_over_water) {
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (ItemDefs[i].swim > 0) move_over_water = 1;
		}
	}
	if (move_over_water)
		f.TagText("li", f.Link("#nocross", "nocross"));
	f.TagText("li", f.Link("#option", "option"));
	f.TagText("li", f.Link("#password", "password"));
	f.TagText("li", f.Link("#pillage", "pillage"));
	if (Globals->USE_PREPARE_COMMAND)
		f.TagText("li", f.Link("#prepare", "prepare"));
	f.TagText("li", f.Link("#produce", "produce"));
	f.TagText("li", f.Link("#promote", "promote"));
	f.TagText("li", f.Link("#quit", "quit"));
	f.TagText("li", f.Link("#restart", "restart"));
	f.TagText("li", f.Link("#reveal", "reveal"));
	if (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		f.TagText("li", f.Link("#sail", "sail"));
	if (Globals->TOWNS_EXIST)
		f.TagText("li", f.Link("#sell", "sell"));
	f.TagText("li", f.Link("#share", "share"));
	f.TagText("li", f.Link("#show", "show"));
	f.TagText("li", f.Link("#spoils", "spoils"));
	if (has_stea)
		f.TagText("li", f.Link("#steal", "steal"));
	f.TagText("li", f.Link("#study", "study"));
	f.TagText("li", f.Link("#take", "take"));
	f.TagText("li", f.Link("#tax", "tax"));
	f.TagText("li", f.Link("#teach", "teach"));
	if (qm_exist)
		f.TagText("li", f.Link("#transport", "transport"));
	f.TagText("li", f.Link("#turn", "turn"));
	if (Globals->USE_WEAPON_ARMOR_COMMAND)
		f.TagText("li", f.Link("#weapon", "weapon"));
	if (Globals->ALLOW_WITHDRAW)
		f.TagText("li", f.Link("#withdraw", "withdraw"));
	f.TagText("li", f.Link("#work", "work"));
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.TagText("li", f.Link("#sequenceofevents", "Sequence of Events"));
	f.TagText("li", f.Link("#reportformat", "Report Format"));
	f.TagText("li", f.Link("#hintsfornew", "Hints for New Players"));
	if (Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f.Enclose(1, "li");
		f.PutStr(f.Link("#specialcommands", "Special Commands"));
		f.Enclose(1, "ul");
		f.TagText("li", f.Link("#_create", "#Create"));
		f.TagText("li", f.Link("#_resend", "#Resend"));
		f.TagText("li", f.Link("#_times", "#Times"));
		f.TagText("li", f.Link("#_rumor", "#Rumor"));
		f.TagText("li", f.Link("#_remind", "#Remind"));
		f.TagText("li", f.Link("#_email", "#Email"));
		f.Enclose(0, "ul");
		f.Enclose(0, "li");
	}
	f.TagText("li", f.Link("#credits", "Credits"));
	f.Enclose(0, "ul");
	f.Paragraph("Index of Tables");
	f.Paragraph("");
	f.Enclose(1, "ul");
	if (Globals->FACTION_LIMIT_TYPE==GameDefs::FACLIM_FACTION_TYPES)
		f.TagText("li", f.Link("#tablefactionpoints",
					"Table of Faction Points"));
	f.TagText("li", f.Link("#tableitemweights", "Table of Item Weights"));
	if (may_sail)
		f.TagText("li", f.Link("#tableshipcapacities",
					"Table of Ship Capacities"));
	if (Globals->RACES_EXIST)
		f.TagText("li", f.Link("#tableraces", "Table of Races"));
	if (Globals->REQUIRED_EXPERIENCE)
		f.TagText("li", f.Link("#studyprogress", "Table of Study Progress"));
	f.TagText("li", f.Link("#tableiteminfo", "Table of Item Information"));
	f.TagText("li", f.Link("#tablebuildings", "Table of Buildings"));
	f.TagText("li", f.Link("#tabletradestructures",
				"Table of Trade Structures"));
	if (!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED))
		f.TagText("li", f.Link("#tableroadstructures",
					"Table of Road Structures"));
	if (may_sail)
		f.TagText("li", f.Link("#tableshipinfo", "Table of Ship Information"));
	if (Globals->LIMITED_MAGES_PER_BUILDING) {
		f.TagText("li",
				f.Link("#tablemagebuildings", "Table of Mages/Building"));
	}
	f.Enclose(0, "ul");
	f.LinkRef("intro");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Introduction");
	AString *in;
	while((in = introf.GetStr()) != NULL) {
		f.PutStr(*in);
		delete in;
	}
	f.LinkRef("playing");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Playing Atlantis");
	temp = "Atlantis (as you undoubtedly already know) is a play by email "
		"game.  When you sign up for Atlantis, you will be sent a turn "
		"report (via email).  Your report completely details your position "
		"in the game.  After going over this report, and possibly "
		"communicating with other players in the game, you determine your "
		"course of action, and create a file of \"orders\", which you then "
		"send back to the Atlantis server. Then, at a regular interval "
		"(often one week), Atlantis collects all the orders, runs another "
		"turn (covering one month in game time), and sends all the players "
		"another report.";
	f.Paragraph(temp);
	f.LinkRef("playing_factions");
	f.TagText("h3", "Factions:");
	temp = "A player's position is called a \"faction\".  Each faction has "
		"a name and a number (the number is assigned by the computer, and "
		"used for entering orders). Each player is allowed to play one and "
		"ONLY one faction at any given time. Each faction is composed of a "
		"number of \"units\", each unit being a group of one or more people "
		"loyal to the faction.  You start the game with a single unit "
		"consisting of one character, plus a sum of money.  More people can "
		"be hired during the course of the game, and formed into more "
		"units.  (In these rules, the word \"character\" generally refers "
		"either to a unit consisting of only one person, or to a person "
		"within a larger unit.)";
	f.Paragraph(temp);
	temp = "A faction is considered destroyed, and the player knocked out "
		"of the game, if ever all its people are killed or disbanded (i.e. "
		"the faction has no units left).  The program does not consider "
		"your starting character to be special; if your starting character "
		"gets killed, you will probably have been thinking of that character "
		"as the leader of your faction, so some other character can be "
		"regarded as having taken the dead leader's place (assuming of "
		"course that you have at least one surviving unit!).  As far as the "
		"computer is concerned, as long as any unit of the faction "
		"survives, the faction is not wiped out.  (If your faction is "
		"wiped out, you can rejoin the game with a new starting "
		"character.)";
	// Game::RemoveInactiveFactions removes a faction whose last orders are MAX_INACTIVE_TURNS
	// or more turns old; Faction::WriteReport starts warning 3 turns before that.
	if (Globals->MAX_INACTIVE_TURNS != -1) {
		temp += AString(" A faction that sends no orders for ") +
			Globals->MAX_INACTIVE_TURNS + " turns in a row is also removed from "
			"the game; your turn report warns you when this is about to "
			"happen.";
	}
	f.Paragraph(temp);
	Faction fac;

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		temp = "A faction has one pre-set limit; it may not contain more than ";
		temp += AString(AllowedMages(&fac)) + " mages";
		if (app_exist) {
			temp += AString("and ") + AllowedApprentices(&fac)
				+ " " + Globals->APPRENTICE_NAME + "s";
		}
		temp += ". Magic is a rare art, and only a few in the world can "
			"master it. Aside from that, there  is no limit to the number "
			"of units a faction may contain, nor to how many items can be "
			"produced or regions taxed.";
		f.Paragraph(temp);
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		temp = "Each faction has a type; this is decided by the player, "
			"and determines what the faction may do.  The faction has ";
		temp += Globals->FACTION_POINTS;
		// The faction areas depend on FACTION_ACTIVITY: Game::Game() registers War/Trade/Magic
		// for DEFAULT but Martial/Magic otherwise, and ParseFactionType accepts only the
		// registered names. Describing War and Trade to a Martial ruleset (as this paragraph
		// used to) tells players to use areas the FACTION order rejects.
		// What counts as trade activity, mirroring every ActivityCheck(..., TRADE) call site:
		// PRODUCE of anything but silver (monthorders.cpp RunUnitProduce / ValidProd -- WORK
		// and ENTERTAIN produce silver and are exempt), BUILD unless BUILD_NO_TRADE, and
		// TRANSPORT/DISTRIBUTE unless TRANSPORT_NO_TRADE.
		AString trade_activity = "producing goods and materials";
		int build_counts = !Globals->BUILD_NO_TRADE;
		int transport_counts = qm_exist && !Globals->TRANSPORT_NO_TRADE;
		if (build_counts) {
			trade_activity += (transport_counts ? ", " : ", and ");
			trade_activity += "building ships and buildings";
		}
		if (transport_counts) {
			trade_activity += ", and using the TRANSPORT or DISTRIBUTE orders";
		}
		trade_activity += ". Working and entertaining do not count, since "
			"they only earn silver";
		if (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT) {
			temp += " Faction Points, which may be spent on any of the 3 "
				"Faction Areas, War, Trade, and Magic.  The faction type may "
				"be changed at the beginning of each turn, so a faction can "
				"change and adapt to the conditions around it.  Faction Points "
				"spent on War determine the number of regions in which factions "
				"can obtain income by taxing or pillaging";
			if (Globals->TACTICS_NEEDS_WAR) {
				temp += ", and also determines the number of level 5 tactics "
					"leaders (tacticians) that a faction can train";
			}
			temp += ". Faction Points spent "
				"on Trade determine the number of regions in which a faction "
				"may conduct trade activity. Trade activity includes ";
			temp += trade_activity;
			temp += ". ";
			if (qm_exist) {
				temp += "Faction points spent on Trade also determine the "
					"number of quartermaster units a trade faction can have. ";
			}
		} else {
			// Martial replaces both War and Trade: AllowedMartial gives the region limit, and
			// AllowedQuarterMasters / AllowedTacticians take max(Trade|War, Martial) points.
			// The per-region counting mirrors Faction::GetActivityCost.
			temp += " Faction Points, which may be spent on either of the 2 "
				"Faction Areas, Martial and Magic.  The faction type may "
				"be changed at the beginning of each turn, so a faction can "
				"change and adapt to the conditions around it.  Faction Points "
				"spent on Martial determine the number of regions in which a "
				"faction can obtain income by taxing or pillaging, or conduct "
				"trade activity. Trade activity includes ";
			temp += trade_activity;
			temp += ".";
			if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED) {
				temp += " Each region counts only once, however many of these "
					"activities the faction performs there.";
			} else {
				temp += " Each kind of activity counts separately, so taxing "
					"and trading in the same region counts twice.";
			}
			temp += " ";
			if (qm_exist || Globals->TACTICS_NEEDS_WAR) {
				temp += "Faction points spent on Martial also determine the "
					"number of ";
				if (qm_exist) temp += "quartermaster units";
				if (qm_exist && Globals->TACTICS_NEEDS_WAR) temp += " and ";
				if (Globals->TACTICS_NEEDS_WAR) {
					temp += "level 5 tactics leaders (tacticians)";
				}
				temp += " a faction can have. ";
			}
		}
		temp += "Faction Points spent on Magic determine the number of mages ";
		if (app_exist) {
			temp += "and ";
			temp += Globals->APPRENTICE_NAME;
			temp += "s ";
		}
		temp += "the faction may have (more information on all of the "
			"faction activities is in further sections of the rules). Here "
			"is a chart detailing the limits on factions by Faction Points:";
		f.Paragraph(temp);
		f.LinkRef("tablefactionpoints");
		f.Enclose(1, "center");
		f.Enclose(1, "table border=\"1\"");
		f.Enclose(1, "tr");
		f.TagText("th", "Faction Points");

		for (auto &fp : *FactionTypes) {
			if (fp == F_WAR) {
				temp = "War (max tax regions";
				if (Globals->TACTICS_NEEDS_WAR)
					temp += " / tacticians";
				temp += ")";
				f.TagText("th", temp);
			}

			if (fp == F_TRADE) {
				temp = "Trade (max trade regions";
				if (qm_exist)
					temp += " / quartermasters";
				temp += ")";
				f.TagText("th", temp);
			}

			if (fp == F_MARTIAL) {
				temp = "Martial (";

				if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED) {
					temp += "max tax and trade regions";
				}

				if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL) {
					temp += "max tax or trade regions";
				}

				if (qm_exist) temp += " / quartermasters";
				if (Globals->TACTICS_NEEDS_WAR) temp += " / tacticians";
				temp += ")";
				f.TagText("th", temp);
			}

			if (fp == F_MAGIC) {
				temp = "Magic (max mages";
				if (app_exist) {
					temp += " / ";
					temp += Globals->APPRENTICE_NAME;
					temp += "s";
				}
				temp += ")";
				f.TagText("th", temp);
			}
		}
		
		f.Enclose(0, "tr");
		int i;
		for (i = 0; i <= Globals->FACTION_POINTS; i++) {
			for (auto &fp : *FactionTypes) {
				fac.type[fp] = i;
			}

			f.Enclose(1, "tr");
			f.Enclose(1, "td align=\"center\" nowrap");
			f.PutStr(i);
			f.Enclose(0, "td");

			for (auto &fp : *FactionTypes) {
				if (fp == F_WAR) {
					f.Enclose(1, "td align=\"center\" nowrap");
					temp = AllowedTaxes(&fac);
					if (Globals->TACTICS_NEEDS_WAR) temp+= AString(" / ") + AllowedTacticians(&fac);
					f.PutStr(temp);
					f.Enclose(0, "td");
				}

				if (fp == F_TRADE) {
					f.Enclose(1, "td align=\"center\" nowrap");
					temp = AllowedTrades(&fac);
					if (qm_exist) temp += AString(" / ") + AllowedQuarterMasters(&fac);
					f.PutStr(temp);
					f.Enclose(0, "td");
				}

				if (fp == F_MARTIAL) {
					f.Enclose(1, "td align=\"center\" nowrap");
					temp = AllowedMartial(&fac);
					if (qm_exist) temp += AString(" / ") + AllowedQuarterMasters(&fac);
					if (Globals->TACTICS_NEEDS_WAR) temp+= AString(" / ") + AllowedTacticians(&fac);
					f.PutStr(temp);
					f.Enclose(0, "td");
				}

				if (fp == F_MAGIC) {
					f.Enclose(1, "td align=\"center\" nowrap");
					temp = AllowedMages(&fac);
					if (app_exist) temp += AString(" / ") + AllowedApprentices(&fac);
					f.PutStr(temp);
					f.Enclose(0, "td");
				}
			}
			
			f.Enclose(0, "tr");
		}
		f.Enclose(0, "table");
		f.Enclose(0, "center");
		f.PutStr("<P></P>");

		std::ostringstream buffer;

		int count = FactionTypes->size();
		int singleValue = Globals->FACTION_POINTS / count;
		int reminder = Globals->FACTION_POINTS % count;
		for (auto &fp : *FactionTypes) {
			int value = singleValue;
			if (reminder > 0) {
				value += 1;
				reminder--;
			}

			fac.type[fp] = value;
		}

		buffer << "For example, a well rounded faction might spend ";
		writeFactionPointUsage(buffer, fac);
		buffer << ".";

		buffer << " ";

		buffer << "This faction's type would appear as \"";
		writeFactionDefinition(buffer, fac);
		buffer << "\", and ";
		WriteFactionTypeDescription(buffer, fac);
		buffer << ".";

		f.Paragraph(buffer.str());

		buffer.clear();
		buffer.str("");

		if (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT) {
			fac.type[F_WAR] = Globals->FACTION_POINTS;
			fac.type[F_TRADE] = 0;
		}
		else {
			fac.type[F_MARTIAL] =  Globals->FACTION_POINTS;
		}
		fac.type[F_MAGIC] = 0;

		buffer << "As another example, a specialized faction might spend all ";
		writeFactionPointUsage(buffer, fac);
		buffer << ".";

		buffer << " ";

		buffer << "This faction's type would appear as \"";
		writeFactionDefinition(buffer, fac);
		buffer << "\", and ";
		WriteFactionTypeDescription(buffer, fac);
		buffer << ".";

		f.Paragraph(buffer.str());

		// A new faction gets 1 point in every registered area (Faction::Faction), so the
		// unspent count depends on how many areas the ruleset has (2 for Martial/Magic).
		int areas = FactionTypes->size();
		if (Globals->FACTION_POINTS > areas) {
			int rem = Globals->FACTION_POINTS - areas;
			temp = "Note that it is possible to have a faction type with "
				"less than ";
			temp += Globals->FACTION_POINTS;
			temp += " points spent. In fact, a starting faction has one point spent on each of ";
			for (int n = 0; n < areas; n++) {
				if (n > 0) temp += (n == areas - 1) ? " and " : ", ";
				temp += (*FactionTypes)[n].c_str();
			}
			temp += ", leaving ";
			temp += AString(rem) + " point" + (rem==1?"":"s") + " unspent.";
			f.Paragraph(temp);
		}
	}
	{
		// Every ruleset's SetupFaction sets unclaimed = START_MONEY + TurnNumber() * K and
		// returns at once for a faction with noStartLeader, so probing it with a temporary
		// faction on turns 1 and 2 gives the starting silver and the per-turn extra without
		// hard-coding K (300 in NewOrigins). year/month are restored afterwards.
		int saveYear = year, saveMonth = month;
		Faction probe;
		probe.noStartLeader = 1;
		year = 1; month = 0;
		SetupFaction(&probe);
		int firstTurn = probe.unclaimed;
		month = 1;
		SetupFaction(&probe);
		int perTurn = probe.unclaimed - firstTurn;
		year = saveYear; month = saveMonth;

		temp = "When a faction starts the game, it is given a one-man unit and ";
		temp += firstTurn;
		temp += " silver in unclaimed money";
		if (perTurn > 0) {
			temp += AString(" (a faction that joins later gets ") + perTurn +
				" silver more for each turn the game has been running)";
		}
		temp += ".";
	}
	temp += "  Unclaimed money is cash that your "
		"whole faction has access to, but cannot be taken away in battle ("
		"silver in a unit's possessions can be taken in battle).  This allows "
		"a faction to get started without presenting an enticing target for "
		"other factions. Units in your faction may use the ";
	temp += f.Link("#claim", "CLAIM") + " order to take this silver, and use "
		"it to buy goods or recruit men";
	if (Globals->ALLOW_WITHDRAW) {
		temp += ", or use the ";
		temp += f.Link("#withdraw", "WITHDRAW");
		temp += " order to withdraw goods directly";
	}
	temp += ".";
	f.Paragraph(temp);
	temp = "An example faction is shown below, consisting of a starting "
		"character, Merlin the Magician, who has formed two more units, "
		"Merlin's Guards and Merlin's Workers. Each unit is assigned a "
		"unit number by the computer (completely independent of the "
		"faction number); this is used for entering orders. Here, the "
		"player has chosen to give his faction the same name (\"Merlin "
		"the Magician\") as his starting character. Alternatively, you "
		"can call your faction something like \"The Great Northern "
		"Mining Company\" or whatever.";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "pre");
	f.ClearWrapTab();
	if (Globals->LEADERS_EXIST) {
		f.WrapStr("* Merlin the Magician (17), Merlin (27), leader [LEAD].  "
				"Skills: none.");
	} else {
		f.WrapStr("* Merlin the Magician (17), Merlin (27), man [MAN].  "
				"Skills: none.");
	}
	if (Globals->RACES_EXIST) {
		f.WrapStr("* Merlin's Guards (33), Merlin (27), 20 vikings [VIKI], "
				"20 swords [SWOR]. Skills: none.");
		f.WrapStr("* Merlin's Workers (34), Merlin (27), 50 vikings "
				"[VIKI].  Skills: none.");
	} else {
		f.WrapStr("* Merlin's Guards (33), Merlin (27), 20 men [MAN], "
				"20 swords [SWOR]. Skills: none.");
		f.WrapStr("* Merlin's Workers (34), Merlin (27), 50 men [MAN].  "
				"Skills: none.");
	}
	f.Enclose(0, "pre");
	f.LinkRef("playing_units");
	f.TagText("h3", "Units:");
	temp = "A unit is a grouping together of people, all loyal to the "
		"same faction. The people in a unit share skills and possessions, "
		"and execute the same orders each month. The reason for having "
		"units of many people, rather than keeping track of individuals, "
		"is to simplify the game play.  The computer does not keep track of "
		"individual names, possessions, or skills for people in the same "
		"unit, and all the people in a particular unit must be in the same "
		"place at all times.  If you want to send people in the same unit "
		"to different places, you must split up the unit.  Apart from "
		"this, there is little difference between having one unit of 50 people, "
		"or 50 units of one person each, except that the former is very "
		"much easier to handle.";
	f.Paragraph(temp);
	if (Globals->RACES_EXIST) {
		temp = "There are different races that make up the population of "
			"Atlantis. (See the section on skills for a list of these.)";
		if (Globals->LEADERS_EXIST) {
			temp += " In addition, there are \"leaders\", who are presumed "
				"to be of one of the other races, but are all the same "
				"in game terms.";
		}
	} else {
		temp = "Units are made of of ordinary people";
		if (Globals->LEADERS_EXIST) {
			temp += "as well as leaders";
		}
		temp += ".";
	}
	if (Globals->LEADERS_EXIST&&Globals->SKILL_LIMIT_NONLEADERS) {
		temp += " Units made up of normal people may only know one skill, "
			"and cannot teach other units.  Units made up of leaders "
			"may know as many skills as desired, and may teach other "
			"units to speed the learning process.";
	}
	if (Globals->LEADERS_EXIST) {
		temp += " Leaders and normal people may not be mixed in the same "
			"unit. However, leaders are more expensive to recruit and "
			"maintain (see the sections on ";
		temp += f.Link("#economy_maintenance", "maintenance costs") + " and " +
			f.Link("#economy_recruiting", "recruiting") + ").";
	}
	if (Globals->RACES_EXIST) {
		temp += " A unit is treated as the least common denominator of "
			"the people within it, so a unit made up of two races with "
			"different strengths and weaknesses will have all the "
			"weaknesses, and none of the strengths of either race.";
	}
	f.Paragraph(temp);
	f.LinkRef("playing_turns");
	f.TagText("h3", "Turns:");
	temp = "Each turn, the Atlantis server takes the orders file that "
		"you mailed to it, and assigns the orders to the respective units. "
		"All units in your faction are completely loyal to you, and will "
		"execute the orders to the best of their ability. If the unit does "
		"something unintended, it is generally because of incorrect orders; "
		"a unit will not purposefully betray you.";
	f.Paragraph(temp);
	temp = "A turn is equal to one game month.  A unit can do many actions "
		"at the start of the month, that only take a matter of hours, such "
		"as buying and selling commodities, or fighting an opposing "
		"faction.  Each unit can also do exactly one action that takes up "
		"the entire month, such as harvesting resources or moving from one "
		"region to another.  These are called month long orders, and they "
		"are ";
	temp += f.Link("#advance", "ADVANCE") + ", ";
	temp += f.Link("#build", "BUILD") + ", ";
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		temp += f.Link("#entertain", "ENTERTAIN") + ", ";
	temp += f.Link("#idle", "IDLE") + ", ";
	temp += f.Link("#move", "MOVE") + ", ";
	if (Globals->TAX_PILLAGE_MONTH_LONG)
		temp += f.Link("#pillage", "PILLAGE") + ", ";
	temp += f.Link("#produce", "PRODUCE") + ", ";
	if (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		temp += f.Link("#sail", "SAIL") + ", ";
	temp += f.Link("#study", "STUDY") + ", ";
	if (Globals->TAX_PILLAGE_MONTH_LONG)
		temp += f.Link("#tax", "TAX") + ", ";
	temp += f.Link("#teach", "TEACH") + " and ";
	temp += f.Link("#work", "WORK") + ".";
	f.Paragraph(temp);
	// Every Process*Order for a month long order deletes an existing one with an
	// "Overwriting previous month-long order" error, except that ProcessMoveOrder and
	// ProcessSailOrder append to an existing MOVE/SAIL. Game::DefaultWorkOrder then gives
	// units without one WORK (or TAX with AUTOTAX), skipping the Nexus and NPC factions.
	temp = "If you give a unit more than one month long order, only the last "
		"one is carried out, and your report warns you that the earlier one "
		"was overwritten. Several ";
	temp += f.Link("#move", "MOVE") + " orders";
	if (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED)) {
		temp += AString(", or several ") + f.Link("#sail", "SAIL") + " orders,";
	}
	temp += " are the exception: they are joined into one longer move.";
	if (Globals->DEFAULT_WORK_ORDER) {
		temp += " A unit that is not given any month long order will ";
		temp += f.Link("#work", "WORK") + " for the month";
		if (Globals->TAX_PILLAGE_MONTH_LONG) {
			temp += " (or ";
			temp += f.Link("#tax", "TAX") + ", if its ";
			temp += f.Link("#autotax", "AUTOTAX") + " flag is set and it is "
				"able to tax)";
		}
		temp += ", except in the Nexus. Use the ";
		temp += f.Link("#idle", "IDLE") + " order for a unit that should do "
			"nothing.";
	}
	f.Paragraph(temp);
	f.LinkRef("world");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "The World");
	// Terrain list built from the same SHOW_RULES flag as the region resources table, so the
	// two can't disagree (it used to be a fixed list without Volcano).
	{
		std::vector<std::string> terrains;
		for (i = 0; i < R_NUM; i++) {
			if (!(TerrainDefs[i].flags & TerrainType::SHOW_RULES)) continue;
			std::string name = TerrainDefs[i].name;
			if (!name.empty()) name[0] = toupper(name[0]);
			terrains.push_back(name);
		}
		if (Globals->LAKES > 0) terrains.push_back("Lake");
		temp = "The Atlantis world is divided for game purposes into "
			"hexagonal regions.  Each region has a name, and one of the "
			"following terrain types:  ";
		temp += joinList(terrains, "or").c_str();
	}
	temp += " (there may be other types of terrain to be discovered as the "
		"game progresses). Regions can contain units belonging to "
		"players; they can also contain structures such as buildings";
	if (may_sail)
		temp += " and fleets";
	temp += ". Two units in the same region can normally interact, unless "
		"one of them is concealed in some way.  Two units in different "
		"regions cannot normally interact.  NOTE: Combat is an exception "
		"to this.";
	// ARegionArray::GetRegion wraps x; ARegionList::NeighSetup gives the top and bottom rows
	// no northern/southern neighbours, so the flat map wraps east-west only.
	if (!Globals->ICOSAHEDRAL_WORLD) {
		temp += " The world wraps around from east to west: travelling off "
			"the eastern edge brings you back on the western edge. It does "
			"not wrap from north to south.";
	}
	f.Paragraph(temp);
	// Levels: CreateLevels with UNDERWORLD/UNDERDEEP/ABYSS levels below the surface;
	// ARegion::ShortPrint adds the level name to coordinates off the surface; each ruleset's
	// world.cpp links the levels with two-way O_SHAFT objects (inner location, MOVE IN), and
	// its CheckRegionExit removes some exits between underground regions.
	if (Globals->UNDERWORLD_LEVELS + Globals->UNDERDEEP_LEVELS + Globals->ABYSS_LEVEL > 0) {
		f.LinkRef("world_levels");
		temp = "The world has more than one level. The regions described so "
			"far are on the surface";
		if (Globals->NEXUS_EXISTS) {
			temp += ", and the ";
			temp += f.Link("#world_nexus", "Atlantis Nexus") + " is a level of "
				"its own";
		}
		temp += ". Below the surface lies the underworld";
		if (Globals->UNDERDEEP_LEVELS > 0) {
			temp += ", and deeper still the underdeep";
		}
		if (Globals->ABYSS_LEVEL) {
			temp += ", and at the very bottom the abyss";
		}
		temp += ". The levels below the surface are smaller than the surface, "
			"and have terrain of their own, such as caverns, underground "
			"forests and tunnels. Regions that are not on the surface show "
			"their level as part of their coordinates, for example "
			"\"underforest (3,5,underworld)\". Underground, regions are not "
			"always connected to all of their neighbors: some of the exits "
			"between underground regions are blocked.";
		f.Paragraph(temp);
		temp = "The levels are connected by shafts. A shaft is a structure "
			"that \"contains an inner location\"; it leads to a region on "
			"another level, where there is a shaft leading back. A unit inside "
			"a shaft can travel through it with the ";
		temp += f.Link("#move", "MOVE") + " IN order (for example, MOVE 3 IN to "
			"enter shaft 3 and go through it).";
		f.Paragraph(temp);
	}
	f.LinkRef("world_regions");
	f.TagText("h3", "Regions:");
	temp = "Here is a sample region, as it might appear on your turn report:";
	f.Paragraph(temp);
	f.Paragraph("");

	int manidx = -1;
	int leadidx = -1;

	// Pick the sample races from ENABLED items only (the first IT_MAN used to be vikings,
	// which NewOrigins disables) and use them consistently in the sample and the prose.
	for (i = 0; i < NITEMS; i++) {
		if (!(ItemDefs[i].type & IT_MAN)) continue;
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (ItemDefs[i].type & IT_LEADER) {
			if (leadidx == -1) leadidx = i;
		} else {
			if (manidx == -1) manidx = i;
		}
	}

	f.Enclose(1, "pre");
	f.ClearWrapTab();
	temp = "plain (172,110) in Turia, 500 peasants";
	if (Globals->RACES_EXIST)
		temp += AString(" (") + ItemDefs[manidx].names + ")";
	int money = (500 * (15 - Globals->MAINTENANCE_COST));
	temp += AString(", $") + money + ".";
	f.WrapStr(temp);
	f.WrapStr("------------------------------------------------------");
	f.AddWrapTab();
	if (Globals->WEATHER_EXISTS)
		f.WrapStr("The weather was clear last month; it will be clear next "
				"month.");
	temp = AString("Wages: $15.0 (Max: $") + (money/Globals->WORK_FRACTION) +
			").";
	f.WrapStr(temp);
	f.WrapStr("Wanted: none.");
	// Recruits on sale: Market::PostTurn sets the amount to population/25 (leaders /125) and
	// the price to wages * 4 * baseprice / (10 * BASE_MAN_COST) -- 60 * ratio at $15 wages.
	temp = AString("For Sale: ") + (500 / 25) + " ";
	temp += AString(ItemDefs[manidx].names) + " [" + ItemDefs[manidx].abr + "]";
	temp += " at $";
	float ratio = ItemDefs[manidx].baseprice / (float)Globals->BASE_MAN_COST;
	temp += (int)(60*ratio);
	if (Globals->LEADERS_EXIST) {
		ratio = ItemDefs[leadidx].baseprice/(float)Globals->BASE_MAN_COST;
		temp += AString(", ") + (500 / 125) + " " + ItemDefs[leadidx].names + " [" +
			ItemDefs[leadidx].abr + "] at $";
		temp += (int)(60*ratio);
	}
	temp += ".";
	f.WrapStr(temp);
	temp = AString("Entertainment available: $") +
		(money/Globals->ENTERTAIN_FRACTION) + ".";
	f.WrapStr(temp);
	temp = "Products: ";
	if (Globals->FOOD_ITEMS_EXIST)
		temp += "23 grain [GRAI], ";
	temp += "37 horses [HORS].";
	f.WrapStr(temp);
	f.PutNoFormat("");
	f.PutNoFormat("Exits:");
	f.WrapStr("North : ocean (172,108) in Atlantis Ocean.");
	f.WrapStr("Northeast : ocean (173,109) in Atlantis Ocean.");
	f.WrapStr("Southeast : ocean (173,111) in Atlantis Ocean.");
	f.WrapStr("South : plain (172,112) in Turia.");
	f.WrapStr("Southwest : plain (171,111) in Turia.");
	f.WrapStr("Northwest : plain (171,109) in Turia.");
	f.PutNoFormat("");
	f.DropWrapTab();
	temp = "* Hans Shadowspawn (15), Merry Pranksters (14), ";
	int sampleman = Globals->LEADERS_EXIST ? leadidx : manidx;
	temp2 = AString(ItemDefs[sampleman].name) + " [" + ItemDefs[sampleman].abr + "]";
	{
		// Unit::WriteReport shows your own units' weight and capacities
		// (fly/ride/walk/swim). Computed from a real Unit holding the same items so the
		// numbers match the engine. Deliberately not deleted: genrules runs once and exits,
		// and a Unit that was never placed in the world isn't worth destructing here.
		Unit *sample = new Unit();
		sample->items.SetNum(sampleman, 1);
		sample->items.SetNum(I_SILVER, 500);
		temp += temp2 + ", 500 silver [SILV]. Weight: " + sample->items.Weight() +
			". Capacity: " + sample->FlyingCapacity() + "/" + sample->RidingCapacity() +
			"/" + sample->WalkingCapacity() + "/" + sample->SwimmingCapacity() +
			". Skills: none.";
	}
	f.WrapStr(temp);
	temp = AString("- Vox Populi (13), ") + temp2 + ".";
	f.WrapStr(temp);
	f.Enclose(0, "pre");
	temp = "This report gives all of the available information on this "
		"region.  The region type is plain, the name of the surrounding area "
		"is Turia, and the coordinates of this region are (172,110).  The "
		"population of this region is 500 ";
	if (Globals->RACES_EXIST)
		temp += ItemDefs[manidx].names;
	else
		temp += "peasants";
	temp += AString(", and there is $") + money + " of taxable income ";
	temp += "currently in this region.  Then, under the dashed line, are "
		"various details about items for sale, wages, etc.  Finally, "
		"there is a list of all visible units.  Units that belong to your "
		"faction will be so denoted by a '*', whereas other faction's "
		"units are preceded by a '-' (see ";
	temp += f.Link("#reportformat", "Report Format") + " for other markers). "
		"For your own units, the report also shows their total weight and how "
		"much they can carry when flying, riding, walking and swimming.";
	if (Globals->TOWNS_EXIST) {
		// ARegion::ShortPrint adds ", contains <town> [<size>]" to the first line.
		temp += " If the region has a settlement, the first line also gives its "
			"name and size, for example \"plain (172,110) in Turia, contains "
			"Tarmellion [town]\".";
	}
	if (Globals->GATES_EXIST) {
		// ARegion::WriteReport shows the gate only to units with Gate Lore.
		temp += " If you have a unit that knows Gate Lore, the report also "
			"tells you when there is a Gate in the region.";
	}
	f.Paragraph(temp);
	temp = "Since Atlantis is made up of hexagonal regions, the coordinate "
		"system is not always exactly intuitive.  Here is the layout of "
		"Atlantis regions:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "pre");
	f.PutNoFormat("   ____        ____");
	f.PutNoFormat("  /    \\      /    \\");
	f.PutNoFormat(" /(0,0) \\____/(2,0) \\____/");
	f.PutNoFormat(" \\      /    \\      /    \\     N");
	f.PutNoFormat("  \\____/(1,1) \\____/(3,1) \\_   |");
	f.PutNoFormat("  /    \\      /    \\      /    |");
	f.PutNoFormat(" /(0,2) \\____/(2,2) \\____/     |");
	f.PutNoFormat(" \\      /    \\      /    \\   W-O-E");
	f.PutNoFormat("  \\____/(1,3) \\____/(3,3) \\_   |");
	f.PutNoFormat("  /    \\      /    \\      /    S");
	f.PutNoFormat(" /(0,4) \\____/(2,4) \\____/");
	f.PutNoFormat(" \\      /    \\      /    \\");
	f.PutNoFormat("  \\____/      \\____/");
	f.PutNoFormat("  /    \\      /    \\");
	f.Enclose(0, "pre");
	temp = "Note that there are \"holes\" in the coordinate system; there "
		"is no region (1,2), for instance.  This is due to the hexagonal "
		"system of regions.";
	f.Paragraph(temp);
	// Terrains with no population: TerrainDefs[].pop == 0 after the ruleset's table changes
	// (in NewOrigins: ocean, volcano, lake and tunnels; chasm has population there).
	temp = "Most regions are similar to the region shown above, but there "
		"are certain exceptions.  ";
	{
		std::vector<std::string> empty;
		for (int t : worldTerrains()) {
			if (TerrainDefs[t].pop == 0) empty.push_back(TerrainDefs[t].plural);
		}
		if (!empty.empty()) {
			std::string list = joinList(empty);
			list[0] = toupper(list[0]);
			temp += list.c_str();
			temp += " have no population.";
		}
	}
	if (Globals->TOWNS_EXIST)
		temp += " Some regions will contain villages, towns, and cities. "
			"More information on these is available in the section on the "
			"economy.";
	f.Paragraph(temp);
	if (Globals->ICOSAHEDRAL_WORLD) {
		temp = "A further complication is that the world of ";
		temp += Globals->WORLD_NAME;
		temp += " is not, as many primitive folk assume, flat, "
			"but is actually approximately spherical. "
			"However, rendering the surface of a three "
			"dimensional object in two dimensions is a long "
			"standing cartographical problem. Here is an example "
			"of rendering a different spherical world in two "
			"dimensional form:";
		f.Paragraph(temp);
		temp = "<img src=\"Goode_homolosine_projection.jpg\" "
			"alt=\"Goode Homolosine Projection\">";
		f.Paragraph(temp);
		temp = "Observe that the map appears to have triangular "
			"chunks cut out at the poles, and that areas near "
			"the poles are spread apart from each other despite "
			"being close together in reality. These same "
			"effects are observed in the maps we have of ";
		temp += Globals->WORLD_NAME;
		temp += ". So some regions will have exits that appear to "
			"be rather distant; this is an indicator that the "
			"region is on the edge of one of these triangular "
			"chunks. The intervening regions are not missing, "
			"but are the result of folding a three dimensional "
			"object into two dimensions.";
		f.Paragraph(temp);
		temp = "These apparant spacial warps are actually just "
			"relics of a two dimensional coordinate system "
			"being applied to a three dimensional surface. "
			"Should a unit travel over one of these edges, "
			"there will be a path back to their starting region, "
			"although the path back might not be the direction "
			"you expect it to be, as \"north\" changes relative "
			"direction when you travel around the pole in the "
			"polar regions, so each time you cross one of these "
			"edges, you are effectively turning 60 degrees as "
			"well as moving.";
		f.Paragraph(temp);
	}

	f.LinkRef("region_resources");
	f.TagText("h3", "Region resources:");
	// ARegion::SetupProds (economy.cpp): each listed resource is rolled once, when the world
	// is created, with the given percentage chance (weight is always 1); disabled items are
	// skipped. Every region with an economy also gets grain or livestock (or fish, on the
	// coast, with COASTAL_FISH).
	temp = "Here is a list of the resources you can find in each type of "
		"region. The percentage is the chance that a region of that type has "
		"the resource; this is decided when the world is created.";
	if (Globals->FOOD_ITEMS_EXIST) {
		temp += " In addition, every region with a population produces either "
			"grain or livestock";
		if (Globals->COASTAL_FISH) temp += " (or fish, if it is on the coast)";
		temp += ".";
	}
	f.Paragraph(temp);
	f.Enclose(1, "center");
	f.Enclose(1, "table border=\"1\"");
	f.Enclose(1, "tr");
	f.Enclose(1, "td colspan=\"2\"");
	f.PutStr("Region type");
	f.Enclose(0, "td");
	f.Enclose(1, "td colspan=\"4\"");
	f.PutStr("Resources");
	f.Enclose(0, "td");
	f.Enclose(0, "tr");

	for (int i : worldTerrains()) {
		int first = 1;
		f.Enclose(1, "tr");
		f.Enclose(1, "td colspan=\"2\"");
		f.PutStr(TerrainDefs[i].name);
		f.Enclose(0, "td");

		f.Enclose(1, "td colspan=\"4\"");
		AString temp = "";

		for (unsigned int c = 0; c < sizeof(TerrainDefs[i].prods)/sizeof(Product); c++) {
			if (TerrainDefs[i].prods[c].product == -1) continue;
			if (ItemDefs[TerrainDefs[i].prods[c].product].flags & ItemType::DISABLED)
				continue;

			if (first == 0) {
				temp = temp + AString(", ");
			}
			temp = temp + ItemDefs[TerrainDefs[i].prods[c].product].name + " (" + TerrainDefs[i].prods[c].chance + "%)" ;
			first = 0;
		}
		if (temp.Len() > 0) {
			temp = temp + AString(".");
		} else {
			temp = AString("none.");
		}
		f.PutStr(temp);
		f.Enclose(0, "td");
		f.Enclose(0, "tr");
	}
	f.Enclose(0, "table");
	f.Enclose(0, "center");

	f.LinkRef("world_structures");
	f.TagText("h3", "Structures:");
	temp = "Regions may also contain structures, such as buildings";
	if (may_sail)
		temp += " or fleets";
	temp += ". These will appear directly below the list of units.  Here is "
		"a sample structure:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "pre");
	f.ClearWrapTab();
	f.WrapStr("+ Temple of Agrik [3] : Tower.");
	f.AddWrapTab();
	temp = "- High Priest Chafin (9), ";
	temp += temp2 + ", sword [SWOR]";
	f.WrapStr(temp);
	temp = "- Rowing Doom (188), ";
	temp += AString("10 ") + ItemDefs[manidx].names + " [" + ItemDefs[manidx].abr + "]";
	temp += ", 10 swords [SWOR].";
	f.WrapStr(temp);
	f.Enclose(0, "pre");
	temp = "The structure lists the name, the number, and what type of "
		"structure it is (more information of the types of structures "
		"can be found in the section on the economy). Following this "
		"is a list of units inside the structure.";
	if (has_stea)
		temp += " Units within a structure are always visible, even if "
			"they would otherwise not be seen.";
	f.Paragraph(temp);
	// Object::GetOwner is simply the first unit in the object's list (entering appends);
	// Object::ForbiddenBy refuses entry when the owner's faction is less than Friendly to the
	// unit (never for Gateways); naming, describing, PROMOTE, EVICT and DESTROY check the
	// owner. RunEnterOrders(1) runs new units' ENTER after GIVE.
	temp = "Units inside structures are still considered to be in the "
		"region, and other units can interact with them; however, they "
		"may gain benefits, such as defensive bonuses in combat from being "
		"inside a building.  The owner of a structure is the first unit "
		"listed under it on the turn report; normally this is the unit that "
		"has been inside it the longest, and if the owner leaves, the next "
		"unit in the list becomes the owner.  Only the owner can rename or ";
	temp += f.Link("#destroy", "DESTROY") + " the structure, hand over "
		"ownership with ";
	temp += f.Link("#promote", "PROMOTE") + ", or throw other units out with ";
	temp += f.Link("#evict", "EVICT") + ".  A unit may enter a structure if it "
		"is empty, if it is owned by a unit of its own faction, or if the "
		"owner's faction has declared the unit's faction Friendly or Ally.";
	f.Paragraph(temp);
	temp = "Newly formed units carry out their ";
	temp += f.Link("#enter", "ENTER") + " orders after ";
	temp += f.Link("#give", "GIVE") + " orders have been processed, so they can "
		"be given men and goods before they enter a structure.";
	f.Paragraph(temp);
	// Object::Report suffixes: ", needs N" (unfinished), ", contains an inner location"
	// (shafts, Gateways), ", closed to player units" (no CANENTER: lairs and the like).
	temp = "Besides buildings";
	if (may_sail) temp += " and fleets";
	temp += ", a region can contain other kinds of structure. The report adds "
		"a note after a structure's type when something about it is special: "
		"\"needs N\" means the structure is not finished yet, and needs N "
		"more units of work; \"contains an inner location\" means that it "
		"leads somewhere else, and can be travelled through with ";
	temp += f.Link("#move", "MOVE") + " IN; and \"closed to player units\" "
		"means that your units cannot enter it.";
	if (Globals->LAIR_MONSTERS_EXIST) {
		temp += " Monster lairs are structures of this last kind (see ";
		temp += f.Link("#nonplayers_monsters", "Wandering Monsters") + ").";
	}
	if (!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED)) {
		temp += " Roads are structures too (see ";
		temp += f.Link("#economy_roads", "Roads") + ").";
	}
	f.Paragraph(temp);
	if (Globals->NEXUS_EXISTS) {
		f.LinkRef("world_nexus");
		temp = "Atlantis Nexus:";
		f.TagText("h3", temp);
		temp = "Note: the following section contains some details that "
			"you may wish to skip over until you have had a chance to "
			"read the rest of the rules, and understand the mechanics "
			"of Atlantis.  However, be sure to read this section before "
			"playing, as it will affect your early plans in Atlantis.";
		f.Paragraph(temp);
		temp = "When a faction first starts in Atlantis, it begins with "
			"one unit, in a special region called the Atlantis Nexus.";
		if (Globals->MULTI_HEX_NEXUS)
			temp += " These regions exist ";
		else
			temp += " This region exists ";
		if (!Globals->NEXUS_IS_CITY) {
			temp += "outside of the normal world of Atlantis, and as such ";
			if (Globals->MULTI_HEX_NEXUS)
				temp += "have ";
			else
				temp += "has ";
			temp += "no products or marketplaces; ";
			if (Globals->MULTI_HEX_NEXUS)
				temp += "they merely serve ";
			else
				temp += "it merely serves ";
			temp += "as the magical entry into Atlantis.";
		} else {
			temp += "outside of the normal world of Atlantis, but ";
			if (Globals->MULTI_HEX_NEXUS)
				temp += "each contains ";
			else
				temp += "contains ";
			temp += "a starting city with all its benefits";
			if (Globals->GATES_EXIST)
				temp += ", including a gate";
			temp += ". ";
			if (Globals->MULTI_HEX_NEXUS)
				temp += "They also serve ";
			else
				temp += "It also serves ";
			temp += "as the magical entry into ";
			temp += Globals->WORLD_NAME;
			temp += ".";
		}
		f.Paragraph(temp);
		// ARegion::IsSafeRegion is true for the Nexus in every ruleset (battles refused in
		// Game::RunBattle); summoning spells, EVICT and WITHDRAW each refuse the Nexus too.
		temp = "The Nexus is a safe place: no battles can take place there, "
			"creatures cannot be summoned there, and the ";
		temp += f.Link("#evict", "EVICT");
		if (Globals->ALLOW_WITHDRAW) {
			temp += AString(" and ") + f.Link("#withdraw", "WITHDRAW") + " orders do";
		} else {
			temp += " order does";
		}
		temp += " not work there.";
		f.Paragraph(temp);
		if (!Globals->START_CITIES_EXIST) {
			// These are O_GATEWAY objects ("Gateway to <terrain> [n]" in the report), created
			// in ARegionList::MakeNexus-style setup in each ruleset's map.cpp and resolved in
			// Game::DoAMoveOrder (MOVE IN from an O_GATEWAY picks a region of that terrain).
			// Call them Gateways, as the report does -- "portal" is also a magic item.
			temp = "The Nexus contains Gateways that provide one-way "
				"transportation to various terrain types.  "
				"A unit that enters one of these Gateways (by "
				"entering the Gateway and moving IN) will "
				"be transported to a region of the matching "
				"terrain type.  The region chosen is somewhat "
				"random, but will prefer to place players in "
				"towns where no other players are present.  "
				"Once a unit has passed through a Gateway, there "
				"is no way to return to the Nexus.";
		} else if (Globals->MULTI_HEX_NEXUS) {
			temp = "From the Nexus hexes, there are exits either to other "
				"Nexus hexes, or to starting cities in Atlantis.  Units may "
				"move through these exits as normal, but once in a starting "
				"city, there is no way to regain entry to the Nexus.";
		} else {
			temp = "From the Atlantis Nexus, there are six exits into the "
				"starting cities of Atlantis.  Units may move through "
				"these exits as normal, but once through an exit, there is "
				"no return path to the Nexus.";
		}
		if (Globals->GATES_EXIST &&
				(Globals->NEXUS_GATE_OUT || Globals->NEXUS_IS_CITY)) {
			temp += " It is also possible to use Gate Lore to get out of "
				"the Nexus";
			if (Globals->NEXUS_GATE_OUT && !Globals->NEXUS_IS_CITY)
				temp += " (but not to return)";
			temp += ".";
		}
		int nexus_gate_lore = Globals->GATES_EXIST &&
			(Globals->NEXUS_GATE_OUT || Globals->NEXUS_IS_CITY);
		if (Globals->START_CITIES_EXIST) {
			temp += " The ";
			if (!Globals->MULTI_HEX_NEXUS)
				temp += "six ";
			temp += "starting cities offer much to a starting faction; ";
			if (Globals->START_CITIES_START_UNLIMITED) {
				if (!Globals->SAFE_START_CITIES && Globals->CITY_MONSTERS_EXIST)
					temp += "until someone conquers the guardsmen, ";
				temp += "there are unlimited amounts of many materials and men "
					"(though the prices are often quite high).";
			} else {
				temp += "there are materials as well as a very large supply of "
					"men (though the prices are often quite high).";
			}
			if (Globals->SAFE_START_CITIES || Globals->CITY_MONSTERS_EXIST)
				temp += " In addition, ";
			if (Globals->SAFE_START_CITIES)
				temp += "no battles are allowed in starting cities";
			if (Globals->CITY_MONSTERS_EXIST) {
				if (Globals->SAFE_START_CITIES) temp += " and ";
				temp += "the starting cities are guarded by strong guardsmen, "
					"keeping any units within the city ";
				if (!Globals->SAFE_START_CITIES)
					temp += "much safer ";
				else
					temp += "safe ";
				temp += "from attack. See the section on Non-Player Units for "
					"more information on city guardsmen";
			}
			temp += ". ";
			temp += "As a drawback, these cities tend to be extremely crowded, "
				"and most factions will wish to leave the starting cities when "
				"possible.";
			f.Paragraph(temp);
			temp = "It is always possible to enter any starting city from the "
				"nexus";
			if (!Globals->SAFE_START_CITIES)
				temp += ", even if that starting city has been taken over and "
					"guarded by another faction";
			temp += ". This is due to the transportation from the Nexus to the "
				"starting city being magical in nature.";
			if (!Globals->SAFE_START_CITIES)
				temp += " Once in the starting city however, no guarantee of "
					"safety is given.";
			f.Paragraph(temp);
			int num_methods = 1 + (Globals->GATES_EXIST?1:0) + (may_sail?1:0);
			char const *methods[] = {"You must go ", "The first is ", "The second is "};
			int method = 1;
			if (num_methods == 1) method = 0;
			temp = AString("There ") + (num_methods == 1?"is ":"are ") +
				NumToWord(num_methods) + " method" + (num_methods == 1?" ":"s ") +
				"of departing the starting cities. ";
			temp += methods[method++];
			temp += " by land, but keep in mind that the lands immediately "
				"surrounding the starting cities will tend to be highly "
				"populated, and possibly quite dangerous to travel.";
			if (may_sail) {
				temp += AString(" ") + methods[method];
				temp += " by sea; all of the starting cities lie against an "
					"ocean, and a faction may easily purchase wood and "
					"construct a ship to ";
				temp += f.Link("#sail", "SAIL");
				temp += " away.  Be wary of pirates seeking to prey on new "
					"factions, however!";
			}
			if (Globals->GATES_EXIST) {
				temp += " And last, rumors of a magical Gate Lore suggest yet "
					"another way to travel from the starting cities.  The rumors "
					"are vague, but factions wishing to travel far from the "
					"starting cities, taking only a few men with them, might "
					"wish to pursue this method.";
			}
		}
		f.Paragraph(temp);
		if (!Globals->START_CITIES_EXIST || nexus_gate_lore) {
			f.Paragraph("Examples:");
		}
		if (!Globals->START_CITIES_EXIST) {
			// ParseDir turns a number into MOVE_ENTER + n, so "MOVE 2 IN" enters object 2 and
			// then moves IN during movement; ENTER runs in the instant ENTER/LEAVE phase, before
			// movement, so "ENTER 2" followed by "MOVE IN" does the same in two orders.
			temp = "Your report of the Nexus lists its Gateways, for example "
				"\"+ Gateway to forest [2] : Gateway, contains an inner "
				"location.\" To leave the Nexus through Gateway 2 with a "
				"single order:";
			temp2 = "MOVE 2 IN";
			f.CommandExample(temp, temp2);
			temp = "Or enter Gateway 2 first, and then move in:";
			temp2 = "ENTER 2\nMOVE IN";
			f.CommandExample(temp, temp2);
		}
		if (nexus_gate_lore) {
			// Game::ProcessCastGateLore: "RANDOM" is a random jump. From the Nexus,
			// RunGateJump accepts any open surface gate (nexgate), and the Nexus gate is always
			// open (ARegion::SetGateStatus). Without UNITS only the mage jumps.
			temp = "A mage who knows Gate Lore can leave the Nexus with a "
				"random gate jump:";
			temp2 = "CAST GATE RANDOM";
			f.CommandExample(temp, temp2);
		}
	}

	f.LinkRef("movement");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Movement");
	if (may_sail)
		temp = "There are two main methods of movement in Atlantis.  The "
			"first ";
	else
		temp = "The main method of movement in Atlantis ";
	temp += "is done using the ";
	temp += f.Link("#move", "MOVE") + " order (or the " +
		f.Link("#advance", "ADVANCE");
	temp += " order), and moves units individually from one region to "
		"another. ";
	if (may_sail) {
		temp += "The other method is done using the ";
		temp += f.Link("#sail", "SAIL") + " order, which can sail a ";
		temp += "fleet, including all of its occupants from one region to "
			"another. ";
	}
	temp += "Certain powerful mages may also teleport themselves, or even "
		"other units, but the knowledge of the workings of this magic is "
		"carefully guarded.";
	f.Paragraph(temp);

	f.LinkRef("movement_normal");
	f.TagText("h3", "Normal Movement:");
	temp = "In one month, a unit can issue a single ";
	// Unit::MoveType / CalcMovePoints: walk, ride or fly on land; in water (similar type
	// ocean) a unit whose swimming capacity covers its weight swims, at the speed of the
	// items that let it swim. A flying unit with the "wind" attribute (Summon Wind) gets
	// FLEET_WIND_BOOST extra points, capped at MAX_SPEED.
	std::string water_regions = Globals->LAKES > 0 ? "ocean or lake regions" : "ocean regions";
	temp += f.Link("#move", "MOVE") + " order, using one or more of its "
		"movement points. There are four modes of travel: walking, riding, "
		"flying and swimming. Walking units have ";
	temp += NumToWord(ItemDefs[I_LEADERS].speed) + " movement point" +
		(ItemDefs[I_LEADERS].speed==1?"":"s") + ", riding units have ";
	temp += NumToWord(ItemDefs[I_HORSE].speed) + ", and flying units have ";
	temp += NumToWord(ItemDefs[I_WHORSE].speed) + "; swimming units move at the "
		"speed of whatever lets them swim. ";
	if (!(SkillDefs[S_SUMMON_WIND].flags & SkillType::DISABLED) &&
			Globals->FLEET_WIND_BOOST > 0) {
		temp += AString("A flying unit that can call up the wind (for example with "
			"the Summon Wind skill) gets ") + NumToWord(Globals->FLEET_WIND_BOOST) +
			" extra movement points, up to a maximum of " +
			NumToWord(Globals->MAX_SPEED) + ". ";
	}
	temp += "A unit will automatically use the fastest mode of travel "
		"it has available. The ";
	temp += f.Link("#advance", "ADVANCE") + " order is the same as " +
		f.Link("#move", "MOVE") + ", except that it implies attacks on " +
		"units which try to forbid access; see the section on combat for " +
		"details.";
	f.Paragraph(temp);

	temp = "Some races can swim, and there are creatures and items that let "
		"units fly or swim; their descriptions say so. A unit can swim if the "
		"swimming capacity of its people, creatures and items is at least as "
		"great as the weight of the unit and everything it carries. Swimming "
		"units can move into and through ";
	temp += water_regions.c_str();
	temp += ", where each region costs its normal movement points (roads do not "
		"help them). To climb out of the water onto land, a swimming unit "
		"must also be able to walk, ride or fly. Swimming units do not drown, "
		"and can leave a fleet at sea.";
	f.Paragraph(temp);

	temp = "Flying units are not initially available to starting players. "
		"A unit can ride provided that the riding capacity of its "
		"mounts (such as horses) is at least as great as the weight of its "
		"people and all other items. A unit can walk provided that the carrying "
		"capacity of its people";
	if (!(ItemDefs[I_HORSE].flags & ItemType::DISABLED)) {
		if (!(ItemDefs[I_WAGON].flags & ItemType::DISABLED)) temp += ", ";
		else temp += " and ";
		temp += "horses";
	}
	if (!(ItemDefs[I_WAGON].flags & ItemType::DISABLED) &&
			(!(ItemDefs[I_HORSE].flags & ItemType::DISABLED))) {
		temp += ", and wagons";
	}
	temp += " is at least as great as the weight of all its other items";
	if (!(ItemDefs[I_WAGON].flags & ItemType::DISABLED) &&
			(!(ItemDefs[I_HORSE].flags & ItemType::DISABLED))) {
		temp += ", and provided that it has at least as many horses as "
			"wagons (otherwise the excess wagons count as weight, not "
			"capacity)";
	}
	temp += ". Otherwise the unit cannot issue a ";
	temp += f.Link("#move", "MOVE") + " order.";
	temp += AString(" Most people weigh ") + ItemDefs[manidx].weight +
		" units and can carry " + (ItemDefs[manidx].walk - ItemDefs[manidx].weight) +
		" units (some races are lighter or stronger; see their descriptions); "
		"data for items is as follows:";
	f.Paragraph(temp);
	f.LinkRef("tableitemweights");
	f.Enclose(1, "center");
	f.Enclose(1, "table border=\"1\"");
	f.Enclose(1, "tr");
	f.TagText("td", "");
	f.TagText("th", "Weight");
	f.TagText("th", "Capacity");
	f.Enclose(0, "tr");
	for (i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[i].type & IT_NORMAL)) continue;
		pS = FindSkill(ItemDefs[i].pSkill);
		if (pS && (pS->flags & SkillType::DISABLED)) continue;
		last = 0;
		for (j = 0; j < (int) (sizeof(ItemDefs->pInput) /
				sizeof(ItemDefs->pInput[0])); j++) {
			k = ItemDefs[i].pInput[j].item;
			if (k != -1 && (ItemDefs[k].flags & ItemType::DISABLED))
				last = 1;
			if (k != -1 && !(ItemDefs[k].type & IT_NORMAL)) last = 1;
		}
		if (last == 1) continue;
		f.Enclose(1, "tr");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr(ItemDefs[i].name);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr(ItemDefs[i].weight);
		f.Enclose(0, "td");
		cap = ItemDefs[i].walk - ItemDefs[i].weight;
		f.Enclose(1, "td align=\"left\" nowrap");
		if (ItemDefs[i].walk || (ItemDefs[i].hitchItem != -1)) {
			if (ItemDefs[i].hitchItem == -1)
				f.PutStr(cap);
			else {
				temp = (cap + ItemDefs[i].hitchwalk);
				temp += " (with ";
				temp += ItemDefs[ItemDefs[i].hitchItem].name;
				temp += ")";
				f.PutStr(temp);
			}
		} else {
			f.PutStr("&nbsp;");
		}
		f.Enclose(0, "td");
		f.Enclose(0, "tr");
	}
	f.Enclose(0, "table");
	f.Enclose(0, "center");
	if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) {
		temp = "A unit which can fly is capable of travelling over water";
		if (Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_MUST_LAND) {
			// Game::DrownUnits runs once, after all movement, and only looks at units that
			// are not inside a fleet.
			temp += ", but if the unit ends its turn over a water hex (and is "
				"not aboard a fleet), then it will drown. Drowning is only "
				"checked after all movement for the month is done";
		}
		temp += ".";
		f.Paragraph(temp);
	}

	// ARegion::MoveCost: walking, riding and swimming units pay TerrainDefs[].movepoints;
	// flying always costs 1; connected roads give walkers and riders cost - cost/2 (i.e.
	// halved, rounded up, minimum 1). The list is built from the terrain table so it can't go
	// stale (it used to be a fixed list missing volcanoes and the underground terrains).
	temp = "Since regions are hexagonal, each region has six neighbouring "
		"regions to the north, northeast, southeast, south, southwest and "
		"northwest.  Moving from one region to another normally takes one "
		"movement point, except that the following terrain types take more "
		"movement points for walking, riding or swimming units to enter:";
	{
		std::map<int, std::vector<std::string>> byCost;
		for (int t : worldTerrains()) {
			if (TerrainDefs[t].movepoints > 1) {
				std::string name = TerrainDefs[t].name;
				name[0] = toupper(name[0]);
				byCost[TerrainDefs[t].movepoints].push_back(name);
			}
		}
		std::vector<std::string> groups;
		for (auto &kv : byCost) {
			groups.push_back(joinList(kv.second) + " (" + std::to_string(kv.first) +
				" movement points)");
		}
		temp += " ";
		temp += joinList(groups, "and").c_str();
		temp += ". Flying units always need only one movement point to enter a "
			"region.";
	}
	if (!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED)) {
		temp += " Connected roads halve the cost for walking and riding units, "
			"rounding up (see ";
		temp += f.Link("#economy_roads", "Roads") + ").";
	}
	if (Globals->WEATHER_EXISTS) {
		temp += " Also, during certain seasons (depending on the latitude "
			"of the region), all units (including flying ones) have a "
			"harder time and travel will take twice as many movement "
			"points as normal, as freezing weather makes travel difficult; "
			"in the tropics, seasonal hurricane winds and torrential "
			"rains have a similar effect.";
	}
	temp += AString(" Units may not move through ") + water_regions.c_str() + " ";
	if (may_sail) {
		temp += "without using the ";
		temp += f.Link("#sail", "SAIL") + " order";
	}
	if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) {
		temp += " unless they can swim or fly";
		if (Globals->FLIGHT_OVER_WATER==GameDefs::WFLIGHT_MUST_LAND) {
			temp += ", and even then, flying units must end their "
				"movement on land or else drown";
		}
	}
	temp += ".";
	f.Paragraph(temp);
	// Unit::Forbids / Game::DoAMoveOrder: a guard stops Unfriendly or Hostile units it can see
	// and catch; the stopped unit's move ends; an ADVANCE attacks the guards instead.
	temp = "A region may be guarded (see the ";
	temp += f.Link("#guard", "GUARD") + " order). Units on guard stop "
		"Unfriendly and Hostile units that they can see and catch from "
		"entering the region, and a unit that is stopped cannot move any "
		"further that month. A unit using the ";
	temp += f.Link("#advance", "ADVANCE") + " order attacks the guards "
		"instead.";
	f.Paragraph(temp);
	temp = "Units may also enter or exit structures while moving.  Moving "
		"into or out of a structure does not use any movement points at "
		"all.  Note that a unit can also use the ";
	temp += f.Link("#enter", "ENTER") + " and " + f.Link("#leave", "LEAVE");
	temp += " orders to move in and out of structures, without issuing a ";
	temp += f.Link("#move", "MOVE") + " order.";
	f.Paragraph(temp);
	if (Globals->UNDERWORLD_LEVELS || Globals->UNDERDEEP_LEVELS) {
		// Game::DoAMoveOrder: MOVE IN uses the unit's current object's inner location, so the
		// unit must be inside it (a structure number before IN enters it first).
		temp = "Finally, certain structures contain interior passages to "
			"other regions.  A unit inside such a structure can go through "
			"the passage with the ";
		temp += f.Link("#move", "MOVE") + " IN order, or enter the structure and "
			"go through it in one order, for example MOVE 3 IN; the movement "
			"point cost is equal to the normal cost to enter the destination "
			"region.";
		f.Paragraph(temp);
	}
	temp = "Example: One man with a horse, sword, and chain mail wants to "
		"move north, then northeast.  The capacity of the horse is ";
	cap = ItemDefs[I_HORSE].ride - ItemDefs[I_HORSE].weight;
	temp += cap;
	temp += " and the weight of the man and other items is ";
	int weight = ItemDefs[I_MAN].weight + ItemDefs[I_SWORD].weight +
			ItemDefs[I_CHAINARMOR].weight;
	temp += weight;
	if (cap > weight)
		temp += ", so he can ride";
	else
		temp += ", so he must walk";
	if (Globals->WEATHER_EXISTS)
		temp += ". The month is April, so he has ";
	else
		temp += " and has ";
	if (cap > weight)
		temp += NumToWord(ItemDefs[I_HORSE].speed);
	else
		temp += NumToWord(ItemDefs[I_MAN].speed);
	int travel = ItemDefs[I_HORSE].speed;
	temp += " movement point";
	temp += AString((ItemDefs[I_HORSE].speed == 1) ? "" : "s") + ". ";
	temp += "He issues the order MOVE NORTH NORTHEAST. First he moves north, "
		"into a plain region.  This uses ";
	int cost = TerrainDefs[R_PLAIN].movepoints;
	temp += NumToWord(cost) + " movement point" + (cost == 1?"":"s") + ".";
	travel -= cost;
	if (travel > TerrainDefs[R_FOREST].movepoints) {
		temp += " Then he moves northeast, into a forest region. This uses ";
		cost = TerrainDefs[R_FOREST].movepoints;
		temp += NumToWord(cost) + " movement point" + (cost == 1?"":"s") + ",";
		travel -= cost;
		temp += " so the movement is completed with ";
		temp += NumToWord(travel) + " to spare.";
	} else {
		temp += " He does not have the ";
		cost = TerrainDefs[R_FOREST].movepoints;
		temp += NumToWord(cost) + " movement point" + (cost == 1?"":"s");
		temp += " needed to move into the forest region to the northeast, "
			"so the movement is halted at this point.  The remaining move"
			"will be added to his orders for the next turn, before any";
		temp += f.Link("#turn", "TURN") + " orders are processed.";
	}
	f.Paragraph(temp);

	if (may_sail) {
		f.LinkRef("movement_sailing");
		f.TagText("h3", "Sailing:");
		temp = "Movement by sea is in some ways similar. It does not use the ";
		temp += f.Link("#move", "MOVE") + " order however.  Instead, the " +
			"owner of a fleet must issue the " + f.Link("#sail", "SAIL") +
			" order, and other units wishing to help sail the fleet must " +
			"also issue the " + f.Link("#sail", "SAIL") + " order. ";
		temp += "The fleet will then, if possible, make the indicated "
			"movement, carrying all units on the fleet with it.  Units on "
			"board the fleet, but not aiding in the sailing of the fleet, "
			"may execute other orders while the fleet is sailing.  A unit "
			"which does not wish to travel with the fleet should leave the "
			"fleet in a coastal region, before the ";
		// ARegion::IsCoastalOrLakeside (used by SAIL and by ship building) counts lakes as
		// water whatever LAKESIDE_IS_COASTAL says; that flag only affects IsCoastal.
		temp += f.Link("#sail", "SAIL") + " order is processed.  (A coastal " +
			"region is defined as a non-ocean region with at least one "
			"adjacent ocean";
		if (Globals->LAKES > 0) temp += " or lake";
		temp += " region.)";
		f.Paragraph(temp);
		temp = AString("Note that a unit on board a fleet while it is ") +
			"sailing may not " + f.Link("#move", "MOVE") +
			" later in the turn, even if he doesn't issue the " +
			f.Link("#sail", "SAIL");
		temp += " order; sailing is considered to take the whole month. "
			"Also, units may not remain on guard while on board a sailing "
			"fleet; they will have to reissue the ";
		temp += f.Link("#guard", "GUARD") +  " 1 order to guard a " +
			"region after sailing.";
		// ARegion::CheckFleets: a fleet at sea with no living unit aboard is removed.
		temp += " A fleet that is left at sea with no one aboard is lost.";
		f.Paragraph(temp);
		// Object::GetFleetSpeed: a fleet moves at the speed of its slowest ship (the Speed
		// column below), plus the bonuses. Wind: every unit aboard adds GetAttribute("wind") *
		// 12 * FLEET_WIND_BOOST, divided by the sailors the fleet needs, capped at
		// FLEET_WIND_BOOST -- so one level of wind gives the full bonus to a fleet needing up
		// to 12 sailors, whatever FLEET_WIND_BOOST is. (This used to quote the Longboat's
		// speed, which NewOrigins disables.)
		temp = "Each type of ship has a speed (see the table below), which is "
			"the number of movement points it gets per turn; a fleet moves at "
			"the speed of its slowest ship.";
		if (!(SkillDefs[S_SUMMON_WIND].flags & SkillType::DISABLED) &&
				Globals->FLEET_WIND_BOOST > 0) {
			temp += AString(" Units aboard that can call up the wind (for example "
				"with the Summon Wind skill) can add up to ") +
				NumToWord(Globals->FLEET_WIND_BOOST) + " movement point" +
				(Globals->FLEET_WIND_BOOST == 1 ? "" : "s") + ". The full bonus "
				"needs one level of wind-calling for every 12 sailors the fleet "
				"needs; less gives a partial bonus.";
		}
		if (Globals->FLEET_CREW_BOOST > 0) {
			temp += " Ships get an extra movement point for each "
				"time they double the number of required crew, "
				"up to a maximum of ";
			temp += NumToWord(Globals->FLEET_CREW_BOOST);
			temp += " extra point";
			if (Globals->FLEET_CREW_BOOST > 1) temp += "s";
			temp += ".";
		}
		if (Globals->FLEET_LOAD_BOOST > 0) {
			temp += " Ships get extra movement points if they are "
				"only lightly loaded. One extra point is given "
				"if they are at 1/2 capacity or less";
			if (Globals->FLEET_LOAD_BOOST > 1) {
				temp += "; a second is given if they are at "
					"1/4 capacity or less";
				if (Globals->FLEET_LOAD_BOOST > 2) {
					temp += ", and so on up to a maximum of ";
					temp += NumToWord(Globals->FLEET_LOAD_BOOST);

				}
			}
			temp += ".";
		}
		// Flying fleets are exempt from every terrain rule of SAIL: both "Can't sail inland"
		// gates in Do1SailOrder test fleet->flying < 1, and Object::SailThroughCheck returns 1
		// for a flying fleet before looking at terrain. BUILD's coastal check
		// (ProcessBuildOrder) likewise skips ships with fly > 0, but the "Can't build in an
		// ocean" check before it applies to every ship. A fleet counts as flying only if every
		// ship in it flies (Object::FleetCapacity). Only mention any of this when the ruleset
		// actually has an enabled flying ship.
		int flying_ships = 0;
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_SHIP)) continue;
			if (ItemDefs[i].fly > 0) flying_ships = 1;
		}
		temp += " A fleet can move from an ocean region to another "
			"ocean region, or from a coastal region to an ocean "
			"region, or from an ocean region to a coastal region.";
		if (Globals->PREVENT_SAIL_THROUGH) {
			// Object::SailThroughCheck: prevdir is the direction back to where the fleet came
			// from. It may go back that way, or out to water if, going round the region one
			// way or the other from the entry side to the exit side, every neighbour passed is
			// water. So a one-hex island can be sailed through; a strip of land separating two
			// waters cannot be crossed. (The old text claimed the opposite.)
			temp += " A fleet may not cross land that separates two bodies "
				"of water. When a fleet that has sailed into a land region "
				"sails out again in the same turn, it can always go back the "
				"way it came; it can leave by another side only if, going "
				"around the region in one direction or the other from the side "
				"it entered by to the side it leaves by, every region it passes "
				"is water. This means that a fleet can sail past a small island "
				"by stopping in it, but cannot use a coastal region as a short "
				"cut across a strip of land.";
			if (Globals->ALLOW_TRIVIAL_PORTAGE) {
				temp += " Ships ending their movement in a land hex may "
					"sail out along any side connecting to water on the "
					"next turn.";
			}
		}
		if (flying_ships) {
			temp += " Fleets made up entirely of flying ships are the "
				"exception to all of the above: they may sail into any adjacent region, over "
				"land or water. A fleet with even one "
				"ship that cannot fly follows the normal rules. Apart from "
				"the terrain, flying fleets obey all the other rules for "
				"fleets, including capacity, sailors and movement costs.";
		}
		temp += " Ships can only be constructed in coastal regions";
		if (flying_ships) {
			temp += ", except for flying ships, which can be constructed "
				"in any land region";
		}
		temp += ". For a "
			"fleet to enter any region only costs one movement point; the "
			"cost of two movement points for entering, say, a forest "
			"coastal region, does not apply.";
		if (Globals->WEATHER_EXISTS) {
			temp += " Ships do, however, only get half movement points "
				"during the winter months (or monsoon months in the "
				"tropical latitudes).";
		}
		f.Paragraph(temp);
		temp = "A fleet can only move if the total weight of everything "
			"aboard does not exceed the fleet's capacity (the rules do "
			"not prevent an overloaded fleet from staying afloat, only "
			"from moving).  Also, there must be enough sailors aboard "
			"(using the ";
		temp += f.Link("#sail", "SAIL") + " order), to sail the fleet, or ";
		// Do1SailOrder: sum of Sailing level * men over every unit aboard with a SAIL order
		// must be at least GetFleetSize(), the sum of each ship's sailors (weight / 50).
		temp += "it will not go anywhere.  The number of sailors a fleet needs "
			"is the total for all of its ships, as given in the table below. "
			"Each unit aboard that issues the ";
		temp += f.Link("#sail", "SAIL") + " order (including the owner) "
			"counts as many sailors as its number of men times its level of "
			"sailing skill; thus a 1 man unit with level 4 sailing skill can "
			"sail a ship that needs 4 sailors alone.  (See the section on "
			"skills for further details on skills.)  The capacities, speeds, "
			"sailors needed and costs in labor units of the various basic "
			"ship types are as follows:";
		f.Paragraph(temp);
		f.LinkRef("tableshipcapacities");
		f.Enclose(1, "center");
		f.Enclose(1, "table border=\"1\"");
		f.Enclose(1, "tr");
		f.TagText("td", "Class");
		f.TagText("th", "Capacity");
		f.TagText("th", "Speed");
		f.TagText("th", "Cost");
		f.TagText("th", "Sailors");
		f.TagText("th", "Skill");
		f.Enclose(0, "tr");
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_SHIP)) continue;
			int pub = 1;
			for (int c = 0; c < (int) sizeof(ItemDefs->pInput)/(int) sizeof(Materials); c++) {
				int m = ItemDefs[i].pInput[c].item;
				if (m != -1) {
					if (ItemDefs[m].flags & ItemType::DISABLED) pub = 0;
					if ((ItemDefs[m].type & IT_ADVANCED) ||
						(ItemDefs[m].type & IT_MAGIC)) pub = 0;
				}
			}
			if (pub == 0) continue;
			if (ItemDefs[i].mLevel > 0) continue;
			int slevel = ItemDefs[i].pLevel;
			if (slevel > 3) continue;
			f.Enclose(1, "tr");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].name);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].fly > 0 ? ItemDefs[i].fly : ItemDefs[i].swim);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].speed);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].pMonths);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].weight/50);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(slevel);
			f.Enclose(0, "td");
			f.Enclose(0, "tr");
		}					
		for (i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (!ObjectIsShip(i)) continue;
			if (ItemDefs[ObjectDefs[i].item].flags & ItemType::DISABLED)
				continue;
			int normal = (ItemDefs[ObjectDefs[i].item].type & IT_NORMAL);
			normal |= (ItemDefs[ObjectDefs[i].item].type & IT_TRADE);
			if (!normal) continue;
			f.Enclose(1, "tr");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ObjectDefs[i].name);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ObjectDefs[i].capacity);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[ObjectDefs[i].item].speed);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ObjectDefs[i].cost);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ObjectDefs[i].sailors);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(AString("")+1);
			f.Enclose(0, "td");
			f.Enclose(0, "tr");
		}
		f.Enclose(0, "table");
		f.Enclose(0, "center");
		temp = "The skill column is the level of shipbuilding skill required "
			"to build that ship type.";
		f.Paragraph(temp);
	}
	f.LinkRef("movement_order");
	f.TagText("h3", "Order of Movement:");
	temp = "This section is probably unimportant to beginning players, but "
		"it can be helpful for more experienced players.";
	f.Paragraph(temp);
	temp = "Movement in Atlantis is processed one hex of movement at a "
		"time, region by region. "
		"Atlantis cycles through all of the regions; for each region, "
		"it finds any units that are due to move, and moves them (if "
		"they can move) one hex (and only one hex). "
		"After processing all the regions, it conducts any battles "
		"that result from these movements. "
		"After it has gone through all of the regions, units will "
		"have moved at most one hex, so it goes back and does the "
		"whole process again. "
		"This is repeated until all units have had the opportunity "
		"to move their allowed distance. "
		"Units' movement is spread out over these phases "
		"proportionally to their speed, so a unit riding at speed "
		"4 would move twice as often as one walking at speed 2. "
		"If the unit requires more than one move to enter a "
		"particular region, then it will move once it has accumulated "
		"enough movement points to do so. "
		"Note that these movement points can be carried over from "
		"one month to another if a MOVE (or ADVANCE) command did "
		"not complete in the month";
	// Game::RunMovementOrders saves the leftover points (savedmovement) only toward the
	// first remaining direction; DoAMoveOrder uses them only if next month's step is in that
	// same direction, otherwise they are lost. The example uses the most expensive terrain a
	// walker can meet, so it stays right for every ruleset.
	{
		int hardest = -1;
		for (int t : worldTerrains()) {
			if (hardest == -1 || TerrainDefs[t].movepoints > TerrainDefs[hardest].movepoints)
				hardest = t;
		}
		int walk = ItemDefs[I_MAN].speed;
		int cost = hardest == -1 ? 1 : TerrainDefs[hardest].movepoints;
		if (Globals->WEATHER_EXISTS) cost *= 2;
		if (cost > walk) {
			temp += AString(" - for example, a unit on foot trying to move into a ") +
				TerrainDefs[hardest].name + " region";
			if (Globals->WEATHER_EXISTS) temp += " in winter";
			temp += " would not have enough movement points to enter in one "
				"turn, but if it continues the same move on the next turn, it "
				"would use the accumulated points from the last month and "
				"manage to enter at last";
		}
	}
	temp += ". The saved points can only be used if the first step of the "
		"next month's move is in the same direction; otherwise they are "
		"lost.";
	f.Paragraph(temp);
	// RunMovementOrders, per phase: DoMoveEnter for every unit, then every fleet, then
	// every MOVE/ADVANCE unit.
	temp = "Within each phase, any steps that enter or leave structures are "
		"carried out first, then fleets move, and then units using ";
	temp += f.Link("#move", "MOVE") + " or " + f.Link("#advance", "ADVANCE") +
		" move.";
	f.Paragraph(temp);
	if (may_sail) {
		temp = "Sailing is handled the same way, with one minor "
			"difference: where units using MOVE or ADVANCE will "
			"be prevented from entering a GUARDed region (where "
			"the guards are unfriendly or worse to the moving "
			"unit), fleets will instead enter the region and "
			"then be stopped by the guards. ";
		f.Paragraph(temp);
	}

	temp = "The following table shows when exactly units will move, "
		"given their base movement speed.  The \"x\"s mark the phases "
		"in which a unit of that speed will move. "
		"If you wish to make units of different speeds move together "
		"(for example, to coordinate an attack), you may need to "
		"tell the faster units to PAUSE in their movement.  See the ";
	temp += f.Link("#move", "MOVE");
	temp += " order for details.";
	f.Paragraph(temp);

	f.Enclose(1, "center");
	f.Enclose(1, "table border=\"1\"");
	f.Enclose(1, "tr");
	f.Enclose(1, "td colspan=\"2\" rowspan=\"2\"");
	f.Enclose(0, "td");
	f.Enclose(1, AString("td colspan=\"") + Globals->MAX_SPEED + "\"");
	f.PutStr("Movement Phase");
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	f.Enclose(1, "tr");
	for (i = 0; i < Globals->MAX_SPEED; i++) {
		f.TagText("th", i + 1);
	}
	f.Enclose(0, "tr");
	for (i = 0; i < Globals->MAX_SPEED; i++) {
		f.Enclose(1, "tr");
		if (!i) {
			f.Enclose(1, AString("td rowspan=\"") + Globals->MAX_SPEED + "\"");
			f.PutStr("Speed");
			f.Enclose(0, "td");
		}
		f.TagText("th", i + 1);
		k = Globals->PHASED_MOVE_OFFSET;
		for (j = 0; j < Globals->MAX_SPEED; j++) {
			k += i + 1;
			if (k >= Globals->MAX_SPEED) {
				f.TagText("td", "x");
				k -= Globals->MAX_SPEED;
			} else {
				f.TagText("td", "");
			}
		}
		f.Enclose(0, "tr");
	}

	f.Enclose(0, "table");
	f.Enclose(0, "center");

	f.LinkRef("skills");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Skills");
	temp = "The most important thing distinguishing one character from "
		"another in Atlantis is skills.  The following skills are "
		"available: ";
	int comma = 0;
	found = 0;
	last = -1;
	for (i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].flags & SkillType::DISABLED) continue;
		if (SkillDefs[i].flags & SkillType::APPRENTICE) continue;
		if (SkillDefs[i].flags & SkillType::MAGIC) continue;
		found = 0;
		for (j = 0; j < 3; j++) {
			SkillType *pS = FindSkill(SkillDefs[i].depends[j].skill);
			if (pS && !(pS->flags & SkillType::DISABLED)) {
				found = 1;
				break;
			}
		}
		if (found) continue;
		if (last == -1) {
			last = i;
			continue;
		}

		temp += SkillDefs[last].name;
		temp += ", ";
		last = i;
		comma++;
	}
	if (last != -1) {
		if (comma) temp += "and ";
		temp += SkillDefs[last].name;
	}

	// The list above skips magic and apprentice skills, and skills that require another
	// skill first.
	temp += ". These are the basic skills; there are also more advanced "
		"skills, which can only be studied once the skills they depend on "
		"have been learned";
	if (Globals->LEADERS_EXIST && !Globals->MAGE_NONLEADERS) {
		temp += ", and magic skills, which only leaders can study (see ";
	} else {
		temp += ", and magic skills (see ";
	}
	temp += f.Link("#magic", "Magic") + ")";
	temp += ". When a unit possesses a skill, he also has a skill level "
		"to go with it.  Generally, a higher level makes a unit better at "
		"what the skill does, but not every skill improves in the same way; "
		"the description of each skill explains what its levels do.";
	f.Paragraph(temp);
	f.LinkRef("skills_limitations");
	f.TagText("h3", "Limitations:");
	if (Globals->LEADERS_EXIST && Globals->SKILL_LIMIT_NONLEADERS) {
		temp = "A unit made up of leaders may know one or more skills; "
			"for the rest of this section, the word \"leader\" will refer "
			"to such a unit.  Other units, those which contain "
			"non-leaders, will be refered to as normal units. A normal "
			"unit may only know one skill.";
		f.Paragraph(temp);
	}
	if (!Globals->RACES_EXIST) {
		if (Globals->SKILL_LIMIT_NONLEADERS) {
			temp = "A unit may only learn one skill. ";
		} else {
			temp = "A unit may learn as many skills as it requires. ";
		}
		ManType *mt = FindRace("MAN");
		if (mt != NULL) {
			temp += "Skills can be learned up to a maximum level of ";
			temp += mt->defaultlevel;
			temp += ".";
		}
		f.Paragraph(temp);
	}

	if (Globals->RACES_EXIST) {
		temp = "Skills may be learned up to a maximum level depending on "
			"the race of the studying unit (remembering that for units "
			"containing more than one race, the maximum is determined by "
			"the least common denominator).  Every race has a normal "
			"maximum skill level, and a list of skills that they "
			"specialize in, and can learn up to higher level. ";
		// The table below shows that most races already reach the leaders' level in their
		// specialized skills, so the real difference is the non-specialized maximum and magic
		// (Game::DoStudyOrder: "Only leaders may study magic" unless MAGE_NONLEADERS).
		ManType *lt = Globals->LEADERS_EXIST ? FindRace("LEAD") : NULL;
		if (lt) {
			temp += AString("Leaders can learn every skill up to level ") +
				lt->defaultlevel;
			if (!Globals->MAGE_NONLEADERS) temp += ", and only leaders can study magic";
			temp += ". ";
		}
		if (!Globals->SKILL_LIMIT_NONLEADERS) {
			// With SKILL_LIMIT_NONLEADERS off there is no limit on the number of skills.
			temp += "A unit, leader or not, may know any number of skills. ";
		}
		temp += "Here is a list of the races (including leaders) and the "
			"information on normal skill levels and specialized skills.";
		f.Paragraph(temp);
		f.LinkRef("tableraces");
		f.Enclose(1, "center");
		f.Enclose(1, "table border=\"1\"");
		f.Enclose(1, "tr");
		f.TagText("th", "Race/Type");
		f.TagText("th", "Specialized Skills");
		f.TagText("th", "Max Level (specialized skills)");
		f.TagText("th", "Max Level (non-specialized skills)");
		f.Enclose(0, "tr");
		temp2 = "MANI";
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_MAN)) continue;
			f.Enclose(1, "tr");
			ManType *mt = FindRace(ItemDefs[i].abr);
			f.Enclose(1, "td align=\"left\" nowrap");
			f.PutStr(ItemDefs[i].names);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"left\" nowrap");
			int spec = 0;
			comma = 0;
			temp = "";
			for (j=0; j<(int)(sizeof(mt->skills)/sizeof(mt->skills[0])); j++) {
				pS = FindSkill(mt->skills[j]);
				if (!pS) continue;
				if (temp2 == pS->abbr &&
						Globals->MAGE_NONLEADERS) {
					spec = 1;
					if (comma) temp += ", ";
					temp += "all magical skills";
					comma++;
					continue;
				}
				if (pS->flags & SkillType::DISABLED) continue;
				spec = 1;
				if (comma) temp += ", ";
				temp += pS->name;
				comma++;
			}
			if (!spec) temp = "None.";
			f.PutStr(temp);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"left\" nowrap");
			if (spec)
				f.PutStr(mt->speciallevel);
			else
				f.PutStr("--");
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"left\" nowrap");
			f.PutStr(mt->defaultlevel);
			f.Enclose(0, "td");
			f.Enclose(0, "tr");
		}
		f.Enclose(0, "table");
		f.Enclose(0, "center");
	}
	// Skill days are stored per UNIT (total over all men); the level comes from days per man
	// (Unit::GetRealSkill -> GetLevelByDays: 1, 3, 6, 10, 15 months for levels 1-5). BUY adds
	// men without adding days, so the average drops; GIVE moves the giving men's share of the
	// days (SkillList::Split, rounded down, remainder stays) and adds it to the receiver
	// (SkillList::Combine). Unit::AdjustSkills then caps each skill at the lowest maximum of the
	// unit's races and DISCARDS the excess -- the one case where months are lost. It runs right
	// after a GIVE of men, but for BUY it runs before the men are added, so bought men only
	// trigger the cap the next time the unit studies (or receives men).
	temp = "If units are merged together, their skills are averaged out. "
		"No rounding off is done; rather, the computer keeps track for each "
		"unit of the total number of days of training it has in each skill, "
		"and the unit's skill level is based on the average number of days "
		"per man. When units are split up, these days are divided as evenly "
		"as possible among the people in the unit, and no days are lost in "
		"the split.";
	if (Globals->RACES_EXIST) {
		temp += " However, a unit's skill can never be higher than the "
			"maximum level of the least capable race in it: if men who can "
			"only reach a lower level join the unit, the skill is reduced to "
			"that level and any training above it is lost. For men given to "
			"the unit this happens at once; for men bought by the unit, the "
			"next time it studies.";
	}
	f.Paragraph(temp);
	{
		// The examples are written as report lines. SkillList::Report shows
		// "<name> [<abbr>] <level> (<days per man>)", with days per man = total / men rounded
		// down (the total itself is kept exactly), and still shows a skill at level 0 while
		// any training remains. Everything below is computed with the same engine functions
		// (SkillStrs, GetLevelByDays, GetDaysByLevel) so it can't drift from the report.
		auto shown = [](int total, int men) {
			int perman = total / men;
			return SkillStrs(S_COMBAT) + " " + GetLevelByDays(perman) + " (" +
				perman + ")";
		};
		temp = "In your report, each skill is shown with its level, followed by "
			"the number of days of training per man in brackets, for example ";
		temp += shown(GetDaysByLevel(2), 1) + ". The number shown is rounded "
			"down, but the unit keeps all of its training. Level 1 needs ";
		temp += AString(GetDaysByLevel(1)) + " days of training per man, level 2 "
			"needs " + GetDaysByLevel(2) + ", level 3 needs " + GetDaysByLevel(3) +
			", level 4 needs " + GetDaysByLevel(4) + " and level 5 needs " +
			GetDaysByLevel(5) + " days.";
		if (!Globals->REQUIRED_EXPERIENCE) {
			// StudyRateAdjustment returns a flat 30 days when REQUIRED_EXPERIENCE is 0.
			temp += " A month of study gives 30 days of training.";
		}
		f.Paragraph(temp);

		const int men = 10;
		int total = GetDaysByLevel(2) * men;
		f.Paragraph("Example of buying men:");
		temp = AString("A unit of ") + men + " men shows " + shown(total, men) +
			", which is " + total + " days of training in total. If it buys 20 "
			"more men, the 30 men share the same " + total + " days, and the "
			"unit shows " + shown(total, 30) + ". If it buys 21 men instead, "
			"the 31 men have " + (total / 31) + " days each (rounded down), and "
			"the unit shows " + shown(total, 31) + ", since level 1 needs " +
			GetDaysByLevel(1) + " days.";
		f.Paragraph(temp);

		int totalA = GetDaysByLevel(3) * men;
		int totalB = GetDaysByLevel(1) * men;
		int moved = totalA * 5 / men;          // SkillList::Split: share of the men given
		f.Paragraph("Example of giving men:");
		temp = AString("Unit A has ") + men + " men and shows " +
			shown(totalA, men) + ", which is " + totalA + " days in total. Unit B "
			"has " + men + " men and shows " + shown(totalB, men) + ", which is " +
			totalB + " days in total. A gives 5 men to B. The 5 men take their "
			"share of A's training with them, " + moved + " days. A keeps 5 men "
			"and " + (totalA - moved) + " days, and still shows " +
			shown(totalA - moved, 5) + ". B now has 15 men and " + totalB +
			" + " + moved + " = " + (totalB + moved) + " days, and shows " +
			shown(totalB + moved, 15) + ", since level 2 needs " +
			GetDaysByLevel(2) + " days. Skills that only one of the units knows "
			"are averaged over the new number of men in the same way.";
		f.Paragraph(temp);
	}
	f.LinkRef("skills_studying");
	f.TagText("h3", "Studying:");
	temp = "For a unit to gain level 1 of a skill, they must gain one "
		"months worth of training in that skill by issuing the ";
	temp += f.Link("#study", "STUDY") + " order. ";
	if (Globals->REQUIRED_EXPERIENCE) {
		temp += "Initially, a unit will gain a full months worth of training "
			"(30 day equivalents). However, high levels of "
			"skill also require experience, which is gained by performing "
			"actions making use of the skill. Any such experience "
			"is unknown even to a unit's owner, but will allow the unit to study "
			"at a much faster rate to higher skill levels. A unit continuing study but not "
			"acquiring experience will find it's study rate reduced as it progresses. ";
		f.Paragraph(temp);
		int months2 = 0;
		int months3 = 0;
		int months5 = 0;
		int mlevel = 0;
		int tries5 = 60;
		int days = 0;
		int dneeded = GetDaysByLevel(2);
		while(days < dneeded) {
			days += StudyRate(days, 0);
			months2++;
			months3++;
			months5++;
		}
		dneeded = GetDaysByLevel(3);
		int tries = 60;
		while((days < dneeded) && (tries > 0)) {
			tries--;
			days += StudyRate(days, 0);
			months3++;
			months5++;
		}
		dneeded = GetDaysByLevel(5);
		int rate = StudyRate(days, 0);
		while((days < dneeded) && (rate > 0) && (tries5 >0)) {
			tries5--;
			rate = StudyRate(days, 0);
			days += rate;
			months5++;
		}		
		mlevel = GetLevelByDays(days);	
		temp = "To illustrate this, a unit would have to spend ";
		temp += NumToWord(months2);
		temp +=	" months studying to gain level 2 in a skill without any experience. ";
		if (tries > 0) {
			temp += "In order to reach level 3 only by studying, a total of ";
			temp += NumToWord(months3);
			temp += " months must be spent. ";
		} else {
			temp += "Said unit would for all practical purposes be unable to reach "
				"skill level 3 by studying without experience. ";
		}
		temp += "The maximum skill level that is possible through "
			"continuous study only (i.e. no experience involved) is ";
		temp += NumToWord(mlevel) + ", ";
		if (months5 > 36) {
			temp += "although it is hardly feasible without any experience at all, ";
		}
		temp += "taking ";
		temp += NumToWord(months5) + " months of studying to achieve. "
			"Note that this assumes that the unit type is allowed to "
			"achieve this level at all ";
		temp += f.Link("#skills_limitations", "(see skill limitations)") + ". ";
		/* Example with 30 experience */
		months2 = 0;
		months3 = 0;
		days = 0;
		dneeded = GetDaysByLevel(2);
		while(days < dneeded) {
			days += StudyRate(days, 30);
			months2++;
			months3++;
		}
		dneeded = GetDaysByLevel(3);
		tries = 60;
		while((days < dneeded) && (tries > 0)) {
			tries--;
			days += StudyRate(days, 30);
			months3++;
		}
		temp += "A unit will start with 30 experience in it's race's specialized "
			"skills. In comparison, units with this amount of experience will reach level 2 in "
			"just ";
		temp += NumToWord(months2);
		temp += " months of study, ";
		if (tries > 0) {
			temp += " and in order to reach level 3 only by studying, a total of ";
			temp += NumToWord(months3);
			temp += " months must be spent by a unit starting with 30 experience. ";
		} else {
			temp += "but even such a unit would be unable to reach skill level 3 merely "
				"through study, but will have to gain some experience along the way. ";
		}	
		temp += "The study progress is shown in the following table:";
		f.Paragraph(temp);
		f.LinkRef("studyprogress");
		f.Enclose(1, "center");
		f.Enclose(1, "table border=\"1\"");
		f.Enclose(1, "tr");
		f.TagText("th", "Unit type");
		f.TagText("th", "starts with");
		f.TagText("th", "1 month");
		f.TagText("th", "2 months");
		f.TagText("th", "3 months");
		f.TagText("th", "4 months");
		f.TagText("th", "5 months");
		f.TagText("th", "6 months");
		f.TagText("th", "7 months");
		f.TagText("th", "8 months");
		f.TagText("th", "9 months");
		f.Enclose(0, "tr");
		f.Enclose(1, "tr");
		f.TagText("td", "non-specialized");
		days = 0;
		int level = 0;
		int plevel = 0;
		int next = StudyRate(days, 0);
		temp = "&nbsp;";
		temp += level;
		temp += "&nbsp;<font size='-1'>(";
		temp += days;
		temp += "+";
		temp += next;
		temp += ")</font>";
		f.TagText("td", temp);
		for (int m = 0; m < 9; m++) {
			days += StudyRate(days, 0);
			next = StudyRate(days, 0);
			level = GetLevelByDays(days);
			temp = "&nbsp;";
			if (level > plevel) temp += "<b>";
			temp += level;
			if (level > plevel) {
				plevel = level;
				temp += "</b>";
			}
			temp += "&nbsp;<font size='-1'>(";
			temp += days;
			temp += "+";
			temp += next;
			temp += ")</font>";
			f.TagText("td", temp);
		}
		f.Enclose(0, "tr");
		f.Enclose(1, "tr");
		f.TagText("td", "specialized");
		days = 0;
		level = 0;
		plevel = 0;
		next = StudyRate(days, 30);
		temp = "&nbsp;";
		temp += level;
		temp += "&nbsp;<font size='-1'>(";
		temp += days;
		temp += "+";
		temp += next;
		temp += ")</font>";
		f.TagText("td", temp);
		for (int m = 0; m < 9; m++) {
			days += StudyRate(days, 30);
			next = StudyRate(days, 30);
			level = GetLevelByDays(days);
			temp = "&nbsp;";
			if (level > plevel) temp += "<b>";
			temp += level;
			if (level > plevel) {
				plevel = level;
				temp += "</b>";
			}
			temp += "&nbsp;<font size='-1'>(";
			temp += days;
			temp += "+";
			temp += next;
			temp += ")</font>";
			f.TagText("td", temp);
		}
		f.Enclose(0, "tr");
		f.Enclose(0, "table");
		f.Enclose(0, "center");
		temp = "";
		temp += "Each skill is listed with it's skill level, followed "
			"(in parentheses) by first the number of day equivalents "
			"already achieved, a plus sign, and the number of "
			"day equivalents expected to be gained with the next "
			"study order. This last amount will vary with experience "
			"and with progress in skill training (more training in the "
			"skill requiring considerably more experience in order to "
			"continue studying). This information is also listed for each "
			"unit in the report. As can be seen for the specialized units "
			"above, surplus experience will garner a considerable bonus to "
			"the study rate. ";
		f.Paragraph(temp);
		temp = "";
		
	} else {
		temp += "To raise this skill level "
			"to 2, the unit must add an additional two months worth of "
			"training.  Then, to raise this to skill level 3 requires another "
			"three months worth of training, and so forth. ";
	}
	temp += "A month of training is gained when a unit uses the ";
	temp += f.Link("#study", "STUDY") + " order.  Note that study months "
		"do not need to be consecutive; for a unit to go from level 1 to "
		"level 2, he can study for a month, do something else for a month, "
		"and then go back and complete the rest of his studies.";
	// DoStudyOrder: a target level keeps the STUDY order going until reached, and is lowered
	// to the unit's maximum (with a message) if it is higher.
	temp += " A unit can also be given a level to study to, for example ";
	temp += f.Link("#study", "STUDY") + " COMBAT 3; it then keeps studying "
		"each month until it reaches that level. If the level is higher than "
		"the unit can reach, it studies up to its maximum instead.";
	if (Globals->SKILL_PRACTICE_AMOUNT > 0) {
		// Unit::Practice: SKILL_PRACTICE_AMOUNT days per man (without REQUIRED_EXPERIENCE),
		// once per month, only with some training already and below the unit's maximum;
		// a prerequisite at or below the skill's current level gets the practice instead.
		// Teachers practise the skill they teach (Game::Do1TeachOrder).
		temp += "  A unit can also increase its level of training by "
			"using a skill.  This progress is ";
		if (Globals->SKILL_PRACTICE_AMOUNT < 11)
			temp += "much slower than";
		else if (Globals->SKILL_PRACTICE_AMOUNT < 30)
			temp += "slower than";
		else if (Globals->SKILL_PRACTICE_AMOUNT == 30)
			temp += "the same as";
		else if (Globals->SKILL_PRACTICE_AMOUNT < 61)
			temp += "faster than";
		else
			temp += "much faster than";
		temp += " studying";
		if (!Globals->REQUIRED_EXPERIENCE) {
			temp += AString(": each use gives ") + Globals->SKILL_PRACTICE_AMOUNT +
				" days of training per man";
		}
		temp += ".  Only one skill can be improved through "
			"practice in any month; if multiple skills are used, only the "
			"first will be improved.  A skill will only improve with "
			"practice if the unit has first studied the rudiments of the "
			"skill, and not beyond the highest level the unit can reach. "
			"If the skill needs other skills, and one of those is not "
			"higher than the skill being used, that skill is improved "
			"instead. A unit that teaches a skill also practises it.";
	}
	f.Paragraph(temp);
	temp = "A unit that no longer wants a skill can use the ";
	temp += f.Link("#forget", "FORGET") + " order to lose all of its training "
		"in it.";
	f.Paragraph(temp);
	// Study costs, built from the enabled skills so that no exception is missed (Quartermaster
	// used to be left out). Magic skills are grouped together; any other skill whose cost
	// differs from Combat's is listed by name. DoStudyOrder charges SkillCost * men, and
	// refuses the whole order if the unit can't pay for every man.
	{
		int base = SkillDefs[S_COMBAT].cost;
		std::map<int, std::vector<std::string>> byCost;
		std::map<int, int> magicCosts;
		for (int sk = 0; sk < NSKILLS; sk++) {
			if (SkillDefs[sk].flags & SkillType::DISABLED) continue;
			if (SkillDefs[sk].flags & SkillType::MAGIC) {
				magicCosts[SkillDefs[sk].cost]++;
				continue;
			}
			if (SkillDefs[sk].cost == base) continue;
			std::string name = SkillDefs[sk].name;
			name[0] = toupper(name[0]);
			byCost[SkillDefs[sk].cost].push_back(name);
		}
		std::vector<std::string> parts;
		for (auto &kv : byCost) {
			parts.push_back(joinList(kv.second) + " ($" + std::to_string(kv.first) + ")");
		}
		if (magicCosts.size() == 1 && magicCosts.begin()->first != base) {
			parts.push_back("magic skills ($" +
				std::to_string(magicCosts.begin()->first) + ")");
		} else if (magicCosts.size() > 1) {
			parts.push_back("magic skills (which vary; see each skill's description)");
		}
		temp = AString("Most skills cost $") + base + " per person per month to "
			"study (in addition to normal maintenance costs)";
		if (!parts.empty()) {
			temp += ". The exceptions are ";
			temp += joinList(parts).c_str();
		}
		temp += ". The unit must be able to pay for every man in it, or it "
			"will not study at all.";
		f.Paragraph(temp);
	}
	f.LinkRef("skills_teaching");
	f.TagText("h3", "Teaching:");
	temp = AString("A unit with a teacher can learn up to twice as fast ") +
		"as normal. The " + f.Link("#teach", "TEACH") + " order is used to ";
	temp += "spend the month teaching one or more other units (your own or "
		"another factions).  The unit doing the teaching must have a skill "
		"level greater than the unit doing the studying.  (Note: for all "
		"skill uses, it is skill level, not number of months of training, "
		"that counts. Thus, a unit with 1 month of training is effectively "
		"the same as a unit with 2 months of training, since both have a "
		"skill level of 1.)  The units being taught simply issue the ";
	// Do1TeachOrder checks target->faction->GetAttitude(teacher's faction): the STUDENT's
	// faction must regard the teacher's as Friendly or better (own faction counts as Ally).
	// The old text had the direction reversed.
	temp += f.Link("#study", "STUDY") + " order normally (if the student "
		"belongs to another faction, that faction must have declared the "
		"teacher's faction Friendly or Ally).  Each person "
		"can only teach up to " + Globals->STUDENTS_PER_TEACHER +
		" student" + (Globals->STUDENTS_PER_TEACHER == 1?"":"s") + " in a ";
	temp += "month; additional students dilute the training.  Thus, if 1 "
		"teacher teaches ";
	temp += AString(2*Globals->STUDENTS_PER_TEACHER) +
		" men, each man being taught will gain 1 1/2 months of training, "
		"not 2 months.";
	f.Paragraph(temp);
	// Do1TeachOrder: students are looked up in the teacher's region; levels compared with
	// GetRealSkill (studied training only); a student's extra days are capped at 30 per man,
	// so more teachers can't give more than one extra month. The extra days are only used if
	// the student's own STUDY succeeds.
	temp = "The teacher and the students must be in the same region. Only "
		"levels gained by training count for teaching; a level given by an "
		"item does not. A student can gain at most one extra month of "
		"training each month, however many teachers it has. If the "
		"student's study fails (for example because it cannot pay, or is "
		"already at its maximum level), the teaching is wasted.";
	f.Paragraph(temp);
	temp = "Note that it is quite possible for a single unit to teach two "
		"or more other units different skills in the same month, provided "
		"that the teacher has a higher skill level than each student in "
		"the skill that that student is studying, and that there are no "
		"more than ";
	temp += AString(Globals->STUDENTS_PER_TEACHER) + " student";
	temp += (Globals->STUDENTS_PER_TEACHER == 1 ? "" : "s");
	temp += " per teacher.";
	f.Paragraph(temp);
	if (Globals->LEADERS_EXIST) {
		temp = "Note: Only leaders may use the ";
		temp += f.Link("#teach", "TEACH") + " order.";
		f.Paragraph(temp);
	}
	f.LinkRef("skills_skillreports");
	f.TagText("h3", "Skill Reports:");
	// Skill reports are triggered by Unit::Study/Practice reaching a new level, by GIVE UNIT
	// of a unit with skills, and by items that grant a skill; SHOW SKILL accepts any level up
	// to the highest the faction has reached.
	temp = "When a unit of a faction reaches a new skill level for the first "
		"time (by studying or practice, by being given to the faction, or "
		"by getting an item that grants the skill), the faction will be "
		"given a report on special abilities that a unit with this skill "
		"level has. This report can be shown again at any time, for any "
		"level up to the highest one the faction has reached, using the ";
	temp += f.Link("#show", "SHOW") + " SKILL [skill] [level] order. For "
		"example, when a faction ";
	temp += "learned the skill Shoemaking level 3 for the first time, it "
		"might receive the following (obviously farcical) report:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "pre");
	f.ClearWrapTab();
	f.WrapStr("Shoemaking [SHOE] 3: A unit with this skill may PRODUCE "
			"Sooper Dooper Air Max Winged Sandals.");
	f.Enclose(0, "pre");
	f.LinkRef("economy");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "The Economy");
	temp = "The unit of currency in Atlantis is the silver piece. Silver "
		"is a normal item, with zero weight, appearing in your unit's "
		"reports. Silver is used for such things as buying items, and "
		"unit's maintenance.";
	f.Paragraph(temp);
	f.LinkRef("economy_maintenance");
	f.TagText("h3", "Maintenance Costs:");
	temp = "IMPORTANT:  Each and every character in Atlantis requires a "
		"maintenance fee each month. Anyone who ends the month without "
		"this maintenance cost has a ";
	temp += Globals->STARVE_PERCENT;
	temp += " percent chance of ";
	if (Globals->SKILL_STARVATION != GameDefs::STARVE_NONE) {
		temp += "starving, leading to the following effects:";
		f.Paragraph(temp);
		f.Enclose(1, "ul");
		f.Enclose(1, "li");
		if (Globals->SKILL_STARVATION == GameDefs::STARVE_MAGES)
			temp = "If the unit is a mage, it";
		else if (Globals->SKILL_STARVATION == GameDefs::STARVE_LEADERS)
			temp = "If the unit is a leader, it";
		else
			temp = "A unit";
		temp += " will lose a skill level in some of its skills.";
		f.PutStr(temp);
		f.Enclose(0, "li");
		if (Globals->SKILL_STARVATION != GameDefs::STARVE_ALL) {
			f.Enclose(1, "li");
			f.PutStr("Otherwise, it will starve to death.");
			f.Enclose(0, "li");
		}
		f.Enclose(1, "li");
		f.PutStr("If a unit should forget a skill level and it knows none, "
				"it will starve to death.");
		f.Enclose(0, "li");
		f.Enclose(0, "ul");
		temp = "";
	} else {
		// Unit::Short: walks the unit's men, ordinary men before leaders, rolling
		// STARVE_PERCENT for each and subtracting that man's cost from the debt, until the debt
		// is covered. NPC units never starve.
		temp += "starving to death. This is rolled separately for each man "
			"the unit cannot pay for, and ordinary men are at risk before "
			"leaders. ";
	}
	temp += "It is up to you to make sure that your people have enough money ";
	if (Globals->UPKEEP_MINIMUM_FOOD > 0)
		temp += "and food ";
	temp +=	"available. Money ";
	// Food sharing (CheckFactionMaintenance(0)) and allied food (CheckAllyMaintenance) run in
	// AssessMaintenance whenever FOOD_ITEMS_EXIST, not only when a minimum food ration
	// (UPKEEP_MINIMUM_FOOD) is required -- so gate their mention on FOOD_ITEMS_EXIST.
	if (Globals->FOOD_ITEMS_EXIST)
		temp += "and food ";
	temp += "will be shared automatically between your units "
		"in the same region, if one is starving and another has more than "
		"enough; but this will not happen between units in different "
		"regions (for other purposes, money is only shared with units that "
		"have set ";
	temp += f.Link("#share", "SHARE") + " 1). If you have silver in your "
		"unclaimed fund, then that silver will be automatically claimed by "
		"units that would otherwise starve. ";
	if (Globals->UPKEEP_MINIMUM_FOOD && Globals->ALLOW_WITHDRAW) {
		temp += "Similarly, food will automatically be ";
		temp +=	f.Link("#withdraw", "withdraw");
		temp += "n if needed and unclaimed funds are available. ";
	}
	temp += "Lastly, if a faction is allied to yours, their units will "
		"provide surplus cash ";
	if (Globals->FOOD_ITEMS_EXIST)
		temp += "or food ";
	temp += "to your units for maintenance, as a last resort.";
	f.Paragraph(temp);
	temp = "";
	if (Globals->MULTIPLIER_USE == GameDefs::MULT_NONE) {
		temp += AString("This fee is ") +
			Globals->MAINTENANCE_COST + " silver for a normal character";
		if (Globals->LEADERS_EXIST) {
			temp += AString(", and ") + Globals->LEADER_COST +
				" silver for a leader";
		}
	} else {
		if (Globals->MULTIPLIER_USE == GameDefs::MULT_MAGES) {
			temp += "Mages ";
		} else if (Globals->MULTIPLIER_USE==GameDefs::MULT_LEADERS &&
				Globals->LEADERS_EXIST) {
			temp += "Leaders ";
		} else {
			temp += "All units ";
		}
		temp += "pay a fee based on the number of skill levels the character "
			"has.  This fee is the maximum of $";
		temp += AString(Globals->MAINTENANCE_MULTIPLIER) + " per skill level";
		temp += " and a cost of $";
		temp += AString(Globals->MAINTENANCE_COST) + " for normal characters";
		temp += AString(" or $") + Globals->LEADER_COST + " for leaders";
		if (Globals->MULTIPLIER_USE != GameDefs::MULT_ALL) {
			temp += ". All other characters pay a fee of ";
			temp += Globals->MAINTENANCE_COST;
			temp += " silver for a normal character";
			if (Globals->LEADERS_EXIST) {
				temp += ", and ";
				temp += Globals->LEADER_COST;
				temp += " silver for a leader";
			}
		}
	}
	temp += ".";
	if (Globals->FOOD_ITEMS_EXIST) {
		// The old loop printed "one unit of , grain, ..." (its separator test was off by one).
		std::vector<std::string> foods;
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_FOOD)) continue;
			foods.push_back(ItemDefs[i].name);
		}
		temp += " Units may use one ";
		temp += joinList(foods, "or").c_str();
		temp += " instead of each ";
		temp += Globals->UPKEEP_FOOD_VALUE;
		temp += " silver of maintenance owed. ";
		if (Globals->UPKEEP_MINIMUM_FOOD > 0) {
			temp += "A unit must be given at least ";
			temp +=	Globals->UPKEEP_MINIMUM_FOOD;
			temp += " maintenance per man in the form of food. ";
		}
		if (Globals->UPKEEP_MAXIMUM_FOOD >= 0) {
			temp += "At most ";
			temp += Globals->UPKEEP_MAXIMUM_FOOD;
			temp += " silver worth of food can be counted against each "
					"man's maintenance. ";
		}
		temp += "A unit may use the ";
		temp += f.Link("#consume", "CONSUME") + " order to specify that it "
			"wishes to use food items in preference to silver.";
		// Which foods are worth more eaten than sold? A "Wanted" food market starts at up to
		// 149% of base price with RANDOM_ECONOMY (SetupCityMarket) and an unused one drifts up
		// to 1.5x its start (Market::PostTurn), so ~2.24x base is the most it can fetch.
		// NOMARKET foods (meals) are never traded at all.
		{
			std::vector<std::string> eat, sell, nomarket;
			for (i = 0; i < NITEMS; i++) {
				if (ItemDefs[i].flags & ItemType::DISABLED) continue;
				if (!(ItemDefs[i].type & IT_FOOD)) continue;
				if (ItemDefs[i].flags & ItemType::NOMARKET) {
					nomarket.push_back(ItemDefs[i].names);
					continue;
				}
				int maxprice = ItemDefs[i].baseprice *
					(Globals->RANDOM_ECONOMY ? 149 : 100) / 100;
				if (Globals->VARIABLE_ECONOMY) maxprice = maxprice * 3 / 2;
				if (maxprice < Globals->UPKEEP_FOOD_VALUE)
					eat.push_back(ItemDefs[i].names);
				else
					sell.push_back(ItemDefs[i].names);
			}
			if (!eat.empty()) {
				temp += " Settlements pay less than ";
				temp += Globals->UPKEEP_FOOD_VALUE;
				temp += " silver for ";
				temp += joinList(eat).c_str();
				temp += ", so eating them is worth more than selling them";
				if (!sell.empty()) {
					temp += "; ";
					temp += joinList(sell).c_str();
					temp += " can sometimes be sold for more";
				}
				temp += ".";
			} else if (!sell.empty()) {
				temp += " Note that these items are usually worth more when sold "
					"in settlements, so selling them and using the money is more "
					"economical than eating them.";
			}
			if (!nomarket.empty()) {
				std::string list = joinList(nomarket);
				list[0] = toupper(list[0]);
				temp += " ";
				temp += list.c_str();
				temp += " cannot be bought or sold in markets.";
			}
		}
	};
	f.Paragraph(temp);
	// Payment order, mirroring Game::AssessMaintenance (runorders.cpp). Each step there is a
	// separate pass over every unit in region/object/unit list order (= report order), and each
	// unit takes all it needs from a source before the next unit is considered. The list below
	// omits the minimum-food ("hunger") phase that runs first when UPKEEP_MINIMUM_FOOD > 0, so
	// it is only printed when that is 0 (true for every shipped ruleset); a ruleset that sets it
	// would need that phase and the food withdrawal added here.
	if (Globals->UPKEEP_MINIMUM_FOOD == 0) {
		temp = "";
		if (Globals->FOOD_ITEMS_EXIST) {
			// CheckUnitMaintenanceItem / CheckFactionMaintenanceItem: eat = ceil(needed / value),
			// and the overshoot is simply discarded.
			temp += "Food is always eaten in whole items. A unit eats enough "
				"items to cover what it still owes, rounded up, and any value "
				"beyond that is lost";
			if (Globals->MAINTENANCE_COST < Globals->UPKEEP_FOOD_VALUE) {
				temp += AString(": a unit that owes only ") +
					Globals->MAINTENANCE_COST + " silver still uses up a "
					"whole item worth " + Globals->UPKEEP_FOOD_VALUE;
			}
			temp += ". This means that many small units eat more food than "
				"one large unit with the same number of men. ";
		}
		temp += "Maintenance is paid in the following steps. Each step is "
			"applied to every unit before the next one starts. Within a "
			"step, units are handled one at a time in the order they appear "
			"in the region, as shown in your turn report, and each unit takes "
			"everything it needs before the next unit gets anything.";
		f.Paragraph(temp);
		// The order is the region's object list (the dummy "open" object is created first in
		// ARegion::Setup, then buildings/fleets in list order) and each object's unit list --
		// the same lists ARegion::WriteReport prints, unsorted. They survive save/load in order.
		// Unit::MoveUnit (move, ENTER, LEAVE, sail) removes and APPENDS, and FORM places the new
		// unit with MoveUnit too, so those units go to the back of their new group.
		temp = "That order is: first the units in the open, then the units in "
			"each building and ship, in the order the buildings and ships are "
			"listed. It stays the same from turn to turn, except that a unit "
			"which moves into the region, enters or leaves a building or ship, "
			"or is newly formed is placed at the end of its new group. Your "
			"turn report therefore shows the order that was used that turn, "
			"and the order next turn will be the same for all units that stay "
			"where they are.";
		f.Paragraph(temp);
		f.Enclose(1, "ol");
		if (Globals->FOOD_ITEMS_EXIST) {
			f.TagText("li", AString("Units that have issued ") +
				f.Link("#consume", "CONSUME") +
				" UNIT or CONSUME FACTION use their own food.");
			f.TagText("li", "Units that have issued CONSUME FACTION use "
				"food held by your other units in the same region.");
		}
		f.TagText("li", "Each unit uses its own silver.");
		f.TagText("li", "Units use silver held by your other units in the "
			"same region.");
		if (Globals->FOOD_ITEMS_EXIST) {
			f.TagText("li", "All units, whether or not they have issued "
				"CONSUME, use their own food, and then food held by your "
				"other units in the same region.");
		}
		f.TagText("li", "Units use silver from your unclaimed fund.");
		temp = "Units use silver";
		if (Globals->FOOD_ITEMS_EXIST) temp += ", and then food,";
		temp += " held by units in the same region whose factions have "
			"declared you Allied.";
		f.TagText("li", temp);
		f.TagText("li", "Units that still cannot pay starve, as described "
			"above.");
		f.Enclose(0, "ol");
		if (Globals->FOOD_ITEMS_EXIST) {
			// Fixed order of the per-item passes in Game::CheckUnitMaintenance and friends.
			const int food_order[] = { I_FOOD, I_GRAIN, I_LIVESTOCK, I_FISH };
			std::vector<std::string> names;
			for (int item : food_order) {
				if (ItemDefs[item].flags & ItemType::DISABLED) continue;
				names.push_back(ItemDefs[item].names);
			}
			if (names.size() > 1) {
				temp = "Whenever food is used, the different kinds are "
					"used in this order: ";
				for (size_t n = 0; n < names.size(); n++) {
					if (n > 0) temp += (n == names.size() - 1) ? " and " : ", ";
					temp += names[n].c_str();
				}
				temp += ".";
				f.Paragraph(temp);
			}
		}
	}
	f.LinkRef("economy_recruiting");
	f.TagText("h3", "Recruiting:");
	temp = "People may be recruited in a region.  The total amount of "
		"recruits available per month in a region, and the amount that must "
		"be paid per person recruited, are shown in the region description. "
		"The ";
	temp += f.Link("#buy", "BUY") + " order is used to recruit new people. ";
	temp += "New recruits will not have any skills or items.  Note that the "
		"process of recruiting a new unit is somewhat counter-intuitive; it "
		"is necessary to ";
	temp += f.Link("#form", "FORM")+" an empty unit, ";
	temp += f.Link("#give", "GIVE")+" the empty unit some money, and have it ";
	temp += f.Link("#buy", "BUY") + " people; see the description of the ";
	temp += f.Link("#form", "FORM")+ " order for further details.";
	f.Paragraph(temp);
	// ARegion::SetupEditRegion / Market::PostTurn: only the region's own race (accepted as
	// PEASANT too) and leaders are for sale; amounts are population/25 and /125, reset each
	// month; price = wages * 4 * race base price / BASE_MAN_COST.
	temp = "Only the race that lives in the region";
	if (Globals->LEADERS_EXIST) temp += ", and leaders,";
	temp += " can be recruited there; the local race can also be bought as "
		"PEASANT. Each month, one in 25 of the region's people is available "
		"for recruiting";
	if (Globals->LEADERS_EXIST) temp += ", and one in 125 as leaders";
	temp += ". The price is about four times the region's wages, adjusted "
		"for the race";
	if (Globals->LEADERS_EXIST) temp += " (leaders cost much more)";
	temp += ". If units try to recruit more people than are available, the "
		"recruits are shared out between them in proportion to how many "
		"each tried to buy.";
	f.Paragraph(temp);
	// Game::GetBuyAmount refuses BUY of men for mages, apprentices and quartermasters, and
	// mixing leaders with normal men; different non-leader races may be mixed.
	temp = "Mages, ";
	temp += AString(Globals->APPRENTICE_NAME) + "s and quartermasters cannot "
		"recruit more men";
	if (Globals->LEADERS_EXIST)
		temp += ", and leaders cannot be recruited into a unit of normal men "
			"or the other way round (different races of normal men can be "
			"mixed)";
	temp += ". Recruiting into a unit that has skills spreads its training "
		"over more men, which can lower its skill levels (see ";
	temp += f.Link("#skills_limitations", "Skills") + ").";
	f.Paragraph(temp);
	f.LinkRef("economy_items");
	f.TagText("h3", "Items:");
	temp = "A unit may have a number of possessions, referred to as "
		"\"items\".  Some details were given above in the section on "
		"Movement, but many things were left out. Here is a table giving "
		"some information about common items in Atlantis. The number in "
		"brackets after the weight is how much the item can carry when "
		"walking, besides itself; 0 means that it can walk by itself but "
		"carries nothing else. Man-months per item is the amount of work "
		"needed to produce one item:";
	f.Paragraph(temp);
	f.LinkRef("tableiteminfo");
	f.Enclose(1, "center");
	f.Enclose(1, "table border=\"1\"");
	f.Enclose(1, "tr");
	f.TagText("td", "&nbsp;");
	f.TagText("th", "Skill (min level)");
	f.TagText("th", "Material");
	f.TagText("th", "Man-months per item");
	f.TagText("th", "Weight (capacity)");
	f.TagText("th", "Extra Information");
	f.Enclose(0, "tr");
	for (i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[i].type & IT_NORMAL)) continue;
		pS = FindSkill(ItemDefs[i].pSkill);
		if (pS && (pS->flags & SkillType::DISABLED)) continue;
		last = 0;
		for (j = 0; j < (int) (sizeof(ItemDefs->pInput) /
						sizeof(ItemDefs->pInput[0])); j++) {
			k = ItemDefs[i].pInput[j].item;
			if (k != -1 &&
					!(ItemDefs[k].flags & ItemType::DISABLED) &&
					!(ItemDefs[k].type & IT_NORMAL))
				last = 1;
		}
		if (last == 1) continue;
		f.Enclose(1, "tr");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr(ItemDefs[i].name);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		if (pS != NULL) {
			temp = pS->name;
			temp += AString(" (") + ItemDefs[i].pLevel + ")";
			f.PutStr(temp);
		}
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		comma = 0;
		temp = "";
		if (ItemDefs[i].flags & ItemType::ORINPUTS)
			temp = "Any of : ";
		for (j = 0; j < (int) (sizeof(ItemDefs->pInput) /
						sizeof(ItemDefs->pInput[0])); j++) {
			k = ItemDefs[i].pInput[j].item;
			if (k < 0 || (ItemDefs[k].flags&ItemType::DISABLED))
				continue;
			if (comma) temp += ", ";
			temp += ItemDefs[i].pInput[j].amt;
			temp += " ";
			if (ItemDefs[i].pInput[j].amt > 1)
				temp += ItemDefs[k].names;
			else
				temp += ItemDefs[k].name;
			comma = 1;
		}
		f.PutStr(temp);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		if (ItemDefs[i].pMonths) {
			temp = ItemDefs[i].pMonths;
		} else {
			temp = "&nbsp;";
		}
		f.PutStr(temp);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		temp = ItemDefs[i].weight;
		cap = ItemDefs[i].walk - ItemDefs[i].weight;
		if (ItemDefs[i].walk || (ItemDefs[i].hitchItem != -1)) {
			temp += " (";
			if (ItemDefs[i].hitchItem == -1)
				temp += cap;
			else {
				temp += (cap + ItemDefs[i].hitchwalk);
				temp += " with ";
				temp += ItemDefs[ItemDefs[i].hitchItem].name;
			}
			temp += ")";
		}
		f.PutStr(temp);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\"");
		temp = "";
		if (ItemDefs[i].type & IT_WEAPON) {
			WeaponType *wp = FindWeapon(ItemDefs[i].abr);
			if (wp->attackBonus || wp->defenseBonus ||
					(wp->flags & WeaponType::RANGED) ||
					(wp->flags & WeaponType::NEEDSKILL)) {
				if (wp->flags & WeaponType::RANGED)
					temp += "Ranged weapon";
				else
					temp += "Weapon";
				temp += " which gives ";
				if (wp->attackBonus > -1) temp += "+";
				temp += wp->attackBonus;
				temp += " on attack";
				temp += " and ";
				if (wp->defenseBonus > -1) temp += "+";
				temp += wp->defenseBonus;
				temp += " on defense";
				if (wp->flags & WeaponType::NEEDSKILL) {
					pS = FindSkill(wp->baseSkill);
					if (pS && !(pS->flags & SkillType::DISABLED))
						temp += AString(" (needs ") + pS->name;
					pS = FindSkill(wp->orSkill);
					if (pS && !(pS->flags & SkillType::DISABLED))
						temp += AString(" or ") + pS->name;
					temp += " skill)";
				}
				temp += ".<br />";
			}
			if (wp->numAttacks < 0) {
				temp += "Gives 1 attack every ";
				temp += -(wp->numAttacks);
				temp += " rounds.<br />";
			}
		}
		if (ItemDefs[i].type & IT_MOUNT) {
			MountType *mp = FindMount(ItemDefs[i].abr);
			pS = FindSkill(mp->skill);
			if (pS && !(pS->flags & SkillType::DISABLED)) {
				temp += "Gives a riding bonus with the ";
				temp += pS->name;
				temp += " skill.<br />";
			}
		}
		if (ItemDefs[i].type & IT_ARMOR) {
			ArmorType *at = FindArmor(ItemDefs[i].abr);
			temp += "Gives a ";
			temp += at->saves[SLASHING];
			temp += " in ";
			temp += at->from;
			temp += " chance to survive a normal hit.<br />";
			if ((at->flags & ArmorType::USEINASSASSINATE) && has_stea) {
				temp += "May be used during assassinations.<br />";
			}
		}
		if (ItemDefs[i].type & IT_FOOD) {
			temp += AString("Can be eaten instead of ") + Globals->UPKEEP_FOOD_VALUE +
				" silver of maintenance.<br />";
		}
		if (ItemDefs[i].type & IT_BATTLE) {
			// Battle items (shields here) used to get an empty cell; reuse the same text the
			// item description gives (ShowSpecial, as in ItemDescription).
			BattleItemType *bt = FindBattleItem(ItemDefs[i].abr);
			if (bt && (bt->flags & BattleItemType::SHIELD)) {
				temp += AString("Provides ") + ShowSpecial(bt->special, bt->skillLevel, 1, 1) +
					"<br />";
			}
		}
		if (ItemDefs[i].type & IT_TOOL) {
			for (j = 0; j < NITEMS; j++) {
				if (ItemDefs[j].flags & ItemType::DISABLED) continue;
				if (ItemDefs[j].mult_item != i) continue;
				if (!(ItemDefs[j].type & IT_NORMAL)) continue;
				pS = FindSkill(ItemDefs[j].pSkill);
				if (!pS || (pS->flags & SkillType::DISABLED)) continue;
				last = 0;
				for (k = 0; k < (int) (sizeof(ItemDefs->pInput) /
						sizeof(ItemDefs->pInput[0])); k++) {
					l = ItemDefs[j].pInput[k].item;
					if (l != -1 &&
							!(ItemDefs[l].flags & ItemType::DISABLED) &&
							!(ItemDefs[l].type & IT_NORMAL))
						last = 1;
				}
				if (last == 1) continue;
				temp += AString("+") + ItemDefs[j].mult_val +
					" bonus when producing " + ItemDefs[j].names + ".<br />";
			}
		}
		f.PutStr(temp);
		f.Enclose(0, "td");
		f.Enclose(0, "tr");
	}
	f.Enclose(0, "table");
	f.Enclose(0, "center");
	// Game::RunUnitProduce: output = (men * level + tool bonus) / pMonths, limited by the
	// materials the unit and its SHARE-ing units hold; PRODUCE n caps it and re-queues the rest.
	temp = "The items in the table are produced with the ";
	temp += f.Link("#produce", "PRODUCE") + " order. Other things are obtained "
		"in other ways: silver is earned (see ";
	temp += f.Link("#economy_income", "Income") + "), men are recruited with ";
	temp += f.Link("#buy", "BUY");
	if (may_sail) temp += AString(", ships are built with ") + f.Link("#build", "BUILD");
	temp += ", magic items are created with spells, and trade goods can only "
		"be bought. The skill and the raw materials needed to produce one "
		"item are in the table above. In a month, a unit produces as many "
		"items as its work allows (see below), but no more than the "
		"materials held by the unit";
	temp += " (and by units of yours in the same region that have set ";
	temp += f.Link("#share", "SHARE") + " 1) are enough for. A unit can also "
		"be told how many items to make, for example PRODUCE 10 SWORDS; it "
		"then stops when it has made that many, and continues next month if "
		"it has not.";
	f.Paragraph(temp);
	
	temp = "If an item requires raw materials, then the specified "
		"amount of each material is consumed for each item produced. ";
	temp += "The higher the skill of the unit, the more productive each "
		"man-month of work will be.  Thus, without tools, five men at skill "
		"level one are exactly equivalent to one man at skill level 5 in "
		"terms of output. Items which require multiple man-months to produce "
		"will still benefit from higher skill level units, just not as "
		"quickly.  For example, if a unit of six level one men wanted to "
		"produce something which required three man-months per item, that "
		"unit could produce two of them in one month.  If their skill level "
		"was raised to two, then they could produce four of them in a month. "
		"At level three, they could then produce 6 per month.";
	f.Paragraph(temp);
	
	temp = "Some items may allow each man to produce multiple output "
		"items per raw material or have other differences from these basic "
		"rules.  Those items will explain their differences in the "
		"description of the item.";
	f.Paragraph(temp);

	// temp used to be printed even when this branch was skipped, repeating the previous
	// paragraph; the Paragraph call is now inside the branch.
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if (martial_areas) {
			// PRODUCE of anything but silver calls ActivityCheck(TRADE), which for Martial
			// rules counts against AllowedMartial -- zero regions with no Martial points.
			temp = "Producing items counts as trade activity, so it counts "
				"toward your faction's Martial region limit, and a faction "
				"without Faction Points in Martial cannot issue ";
			temp += f.Link("#produce", "PRODUCE") + " orders at all, regardless "
				"of skill levels.";
		} else {
			temp = "Only Trade factions can issue ";
			temp += f.Link("#produce", "PRODUCE") + " orders however, regardless "
				"of skill levels.";
		}
		f.Paragraph(temp);
	}
	
	// Unit::GetProductionBonus: min(tools held, men) * mult_val, added to men * level. Only the
	// producing unit's own tools count.
	temp = "Tools increase production. The amount of work a unit does in a "
		"month is the number of men times their skill level, plus the "
		"bonus of the tools it holds: each tool adds the bonus shown in the "
		"table (for example +1), but only one tool per man counts. For "
		"example, 4 men at level 2 with 4 tools that give +1 do 4 x 2 + 4 = "
		"12 man-months of work, enough for 12 items that need one man-month "
		"each. The tools must be held by the producing unit itself. Tools "
		"may also increase production of advanced items and of other tools; "
		"each item's description (see the ";
	temp += f.Link("#show", "SHOW") + " order) lists what it helps to produce.";
	f.Paragraph(temp);
	temp = "If an item does not list a raw material it may be produced "
		"directly from the land. Each region generally has at least one item "
		"that can be produced there.  Shown on the description of a region "
		"is a list of the items that can be produced, and the amount of "
		"each that can be produced per month.  This amount depends on the "
		"region type. ";
	if (Globals->RANDOM_ECONOMY) {
		temp += "It also varies from region to region of the same type. ";
	}
	// Game::RunAProduction: each unit gets amount * its share of the attempted total, rounded
	// down, computed in report order on what is left -- so the remainder goes to later units.
	temp += "If the units in a region attempt to produce more of a commodity "
		"than can be produced that month, then the amount available is "
		"distributed among the producers in proportion to how much each "
		"could have produced. Each share is rounded down, and what is left "
		"over goes to the units further down the list in the turn report.";
	f.Paragraph(temp);
	// Markets: Wanted = M_SELL (players SELL), For Sale = M_BUY (players BUY). Market::PostTurn
	// moves the price 1/5 of the way toward a target each month: buying raises a For Sale
	// target above the start price, selling lowers a Wanted target, and with no sales a Wanted
	// target is 1.5x its start price. Amounts grow with the region's population between each
	// market's thresholds; ARegion::WriteMarkets skips markets with amount 0.
	f.LinkRef("economy_markets");
	f.TagText("h3", "Markets:");
	temp = "The region report shows the region's markets. \"Wanted\" lists "
		"goods that the region will buy from you, with the ";
	temp += f.Link("#sell", "SELL") + " order, and \"For Sale\" lists goods "
		"you can buy, with the " + f.Link("#buy", "BUY") + " order; each "
		"entry gives the amount and the price per item. Every region where "
		"people live sells recruits (see Recruiting); other markets are found "
		"in settlements.";
	if (Globals->VARIABLE_ECONOMY) {
		temp += " The amount in each market is renewed every month, and grows "
			"as the region's population grows; a market with nothing to trade "
			"is not shown. Prices change with trade: buying an item makes it "
			"more expensive, and selling an item makes the region pay less "
			"for it. Prices move a fifth of the way toward their new level "
			"each month, so the effect builds up over several months. A "
			"wanted item that nobody sells slowly becomes more valuable, up "
			"to half again its starting price.";
	}
	f.Paragraph(temp);
	if (Globals->TOWNS_EXIST) {
		f.LinkRef("economy_towns");
		f.TagText("h3", "Villages, Towns, and Cities:");
		// ARegion::AddTown -> SetupCityMarket, once, at founding: food markets (open at once),
		// then up to 4 wanted and 2 for-sale normal goods and the trade goods, each with its
		// own population threshold above the founding population. Nothing ties them to the
		// village/town/city class.
		temp = "Some regions in Atlantis contain villages, towns, and "
			"cities.  A settlement adds to the wages, population, and tax "
			"income of the region it is in. ";
		if (Globals->FOOD_ITEMS_EXIST) {
			std::vector<std::string> foods;
			for (int it : { I_GRAIN, I_LIVESTOCK, I_FISH }) {
				if (!(ItemDefs[it].flags & ItemType::DISABLED))
					foods.push_back(ItemDefs[it].name);
			}
			temp += "Every settlement wants ";
			temp += joinList(foods).c_str();
			temp += " from the start. ";
		}
		temp += "Its other markets are chosen when it is founded, and open one "
			"by one as the region's population grows, so a settlement that "
			"grows will trade in more goods. Which goods a settlement trades "
			"does not depend on whether it is a village, a town or a city.";
		f.Paragraph(temp);
		// Population growth runs in Game::ProcessEconomics -> ARegion::Grow, only when
		// DYNAMIC_POPULATION or REGIONS_ECONOMY is set, and only for regions a player unit has
		// visited (ARegion::visited, set during maintenance). Rural growth: Grow() moves the
		// population toward a target that player PRODUCE activity (not silver) raises.
		// Settlement growth: TownGrowth() sets the target from market activity -- "Wanted"
		// (M_SELL) markets the players sell into, food counting double and food other than fish
		// forming the quota, and "For Sale" (M_BUY) trade items the players buy, which also
		// feeds the region's improvement/development in PostTurn. The size class is recomputed
		// from population and development each turn (TownInfo::TownType), so it can go down too.
		// town->hab (the growth limit that damps ARegion::Grow) is computed by
		// ARegion::TownHabitat only once, in SetTownType when the settlement is founded;
		// nothing recomputes it, so buildings and development added during play do not change
		// it (see Atlantis-TODO.md item 3.4 -- TownHabitat also has flag/type and ItemDefs[-1]
		// bugs). Hence the text says the limit is fixed at founding and names no buildings.
		// Development still matters: it feeds TownType (size class) and wages. Earth Lore and
		// Clear Skies only help indirectly: while active they add to wages (Wages) and production
		// (UpdateProducts) and give extra rounds of development recovery toward
		// maxdevelopment (PostTurn).
		if (Globals->DYNAMIC_POPULATION || Globals->REGIONS_ECONOMY) {
			// Two separate quantities, kept apart here because they are easy to confuse:
			// - Population (ARegion::Grow / TownGrowth): rural peasants move toward a target
			//   raised by PRODUCE of the region's own resources (Production activity); a
			//   settlement's target comes from market activity. In TownGrowth every sale into a
			//   "Wanted" market counts (food, fish included, x2), but the denominator ("tot") is
			//   only the non-fish food wanted plus the trade items a city sells -- so fish and
			//   other wanted goods help but are never *required*.
			// - Development (PostTurn): rises when this turn's "improvement" exceeds it.
			//   Improvement comes from PRODUCE of goods made from materials (RunUnitProduce),
			//   BUILD of buildings and ships (Run1BuildOrder, ShipConstruction) and buying a
			//   city's trade items (TownGrowth). Roads raise the odds (RoadDevelopment lowers
			//   "progress"). Pillage lowers it; recovery toward maxdevelopment gets one extra
			//   round per level of Earth Lore / Clear Skies. It is not saved between turns.
			// Development does not add people; it feeds Wages() and TownType().
			temp = "Two things about a region change over time: its "
				"population and its development. Population is the number of "
				"people living in the region, both in the countryside and in "
				"its settlement. Development measures how economically "
				"advanced the region is. Development does not add people by "
				"itself, but it raises the wages in the region and helps "
				"decide whether its settlement is a village, a town or a "
				"city.";
			f.Paragraph(temp);
			temp = "<b>Population.</b> The population of a region only "
				"changes once players have visited it. The number of peasants "
				"in the countryside slowly moves toward a level set by the "
				"terrain. A settlement grows when players trade at its "
				"markets: the more you trade with it, the faster it grows, up "
				"to a limit that is set when the settlement is founded. "
				"Buildings do not change this limit.";
			f.Paragraph(temp);
			temp = "<b>Development.</b> A region's development rises with "
				"economic activity there, and roads leading out of the region "
				"make it rise faster. Pillaging lowers development, and it then "
				"slowly comes back.";
			f.Paragraph(temp);
			temp = "This table shows which activities help a region's "
				"population and its development:";
			f.Paragraph(temp);
			{
				// Rows mirror the code paths described in the comment above: Grow() (countryside,
				// region resources), TownGrowth() (settlement markets: non-trade "Wanted" goods,
				// food x2, and trade items bought from a city -- trade items sold TO a city are
				// not counted), and the improvement sources (RunUnitProduce, BUILD, city trade
				// items bought). Silver produced by WORK/ENTERTAIN counts toward neither.
				struct Row { AString what; const char *pop; const char *dev; };
				std::vector<Row> rows;
				rows.push_back({ AString("Producing the region's resources (") +
					f.Link("#produce", "PRODUCE") + ")", "Yes (countryside)", "No" });
				rows.push_back({ AString("Making goods from materials (") +
					f.Link("#produce", "PRODUCE") + ")", "No", "Yes" });
				rows.push_back({ AString("Constructing buildings and ships (") +
					f.Link("#build", "BUILD") + ")", "No", "Yes" });
				rows.push_back({ AString("Working or entertaining"), "No", "No" });
				if (Globals->FOOD_ITEMS_EXIST) {
					rows.push_back({ AString("Selling a settlement the food it wants (") +
						f.Link("#sell", "SELL") + ")", "Yes (counts double)", "No" });
				}
				rows.push_back({ AString("Selling a settlement other goods it wants, "
					"except trade items (") + f.Link("#sell", "SELL") + ")", "Yes", "No" });
				rows.push_back({ AString("Buying trade items that a city sells (") +
					f.Link("#buy", "BUY") + ")", "Yes", "Yes" });
				rows.push_back({ AString("Selling trade items to a city (") +
					f.Link("#sell", "SELL") + ")", "No", "No" });
				f.Enclose(1, "center");
				f.Enclose(1, "table border=\"1\"");
				f.Enclose(1, "tr");
				f.TagText("th", "Activity");
				f.TagText("th", "Population");
				f.TagText("th", "Development");
				f.Enclose(0, "tr");
				for (auto &row : rows) {
					f.Enclose(1, "tr");
					f.TagText("td", row.what);
					f.Enclose(1, "td align=\"center\"");
					f.PutStr(row.pop);
					f.Enclose(0, "td");
					f.Enclose(1, "td align=\"center\"");
					f.PutStr(row.dev);
					f.Enclose(0, "td");
					f.Enclose(0, "tr");
				}
				f.Enclose(0, "table");
				f.Enclose(0, "center");
			}
			temp = "The wages in a region depend on its development, on the "
				"roads leading out of it, and on the size of its settlement.";
			if (!(SkillDefs[S_EARTH_LORE].flags & SkillType::DISABLED) ||
					!(SkillDefs[S_CLEAR_SKIES].flags & SkillType::DISABLED)) {
				// PostTurn of the casting month: SetIncome/UpdateProducts see the spell level
				// (Wages() +12 per level; grain and livestock amounts only), and those stored
				// values are what players use the NEXT month; the flags are then cleared. The
				// extra recovery rounds only move development back toward maxdevelopment, and
				// pillaging (ARegion::Pillage) is the only thing that lowers development in play.
				temp += " Earth Lore and Clear Skies improve a region's economy "
					"for one month: during the month after the spell is cast, the "
					"region's wages and the amount of grain and livestock that can "
					"be produced are higher. They also help a region that has been "
					"pillaged win back its lost development faster. Development won "
					"back this way stays, but the spells never raise development "
					"above what the region had before it was pillaged.";
			}
			f.Paragraph(temp);
			// town->hab is never recomputed after founding (Atlantis-TODO.md 3.4 (c)), and
			// AdjustPop gives the settlement a share of growth proportional to its remaining
			// room (hab - pop), so hab is a hard ceiling. In the recorded NewOrigins game some
			// settlements can therefore never reach the next class and no village can become a
			// city -- hence "not every", rather than promising promotion.
			temp = "<b>Villages, towns and cities.</b> Whether a settlement is "
				"a village, a town or a city depends on its population and on "
				"the region's development, and is checked every turn. A "
				"settlement that grows enough, or whose region becomes more "
				"developed, can move up to a larger type and gains the markets "
				"described above. But because each settlement's size limit is "
				"fixed when it is founded, not every village can become a town, "
				"and not every town can become a city. A settlement whose "
				"population or development falls, for example because the "
				"region is pillaged, can also drop back to a smaller type.";
			f.Paragraph(temp);
		}
		// SetupCityMarket: two trade goods wanted (250-349% of base price with
		// MORE_PROFITABLE_TRADE_GOODS) and two for sale (100-189%), opening only at high
		// population; a market whose threshold lies beyond the maximum is never created.
		temp = "Trade items have no use other than trading. Each settlement "
			"is given two trade items that it wants and two that it sells, "
			"but these markets only open when the region's population is "
			"large, so in practice they are found in large towns and cities, "
			"and some settlements never get them.";
		if (Globals->RANDOM_ECONOMY && Globals->MORE_PROFITABLE_TRADE_GOODS) {
			temp += " The profit margins are high: a settlement that wants a "
				"trade item pays between two and a half and three and a half "
				"times its base price, while one that sells it asks between "
				"one and two times its base price.";
		} else {
			temp += " The profit margins on these items are usually quite high.";
		}
		{
			std::vector<std::string> trade;
			for (i = 0; i < NITEMS; i++) {
				if (ItemDefs[i].flags & (ItemType::DISABLED | ItemType::NOMARKET)) continue;
				if (ItemDefs[i].type & IT_TRADE) trade.push_back(ItemDefs[i].names);
			}
			if (!trade.empty()) {
				temp += " The trade items are ";
				temp += joinList(trade).c_str();
				temp += ".";
			}
		}
		f.Paragraph(temp);
	}
	f.LinkRef("economy_buildings");
	f.TagText("h3", "Buildings and Trade Structures:");
	temp = "Construction of buildings ";
	if (may_sail) temp += "and ships ";
	temp += "goes as follows: each unit of work on a building requires a "
		"unit of the required resource and a man-month of work by a "
		"character with the appropriate skill and level; higher skill "
		"levels allow work to proceed faster (still using one unit of "
		"the required resource per unit of work done). ";
	// Game::Run1BuildOrder: work = men * level, capped by the materials (own and SHARE-ing
	// units) and by the work still needed; wood-or-stone uses stone first. New buildings
	// get the first free number 1-99; with none left, "BUILD: The region is full."
	temp += "In a month a unit does as many units of work as its number of "
		"men times its skill level, limited by the materials it has and by "
		"the work that is still needed. ";
	if (!(ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
			!(ItemDefs[I_STONE].flags & ItemType::DISABLED)) {
		temp += "A structure that can be built of wood or stone uses stone "
			"first. ";
	}
	temp += "A region can hold at most 99 structures. ";
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if (Globals->BUILD_NO_TRADE) {
			temp += "Any faction can issue ";
		} else {
			temp += AString("Again, only ") + trade_factions + " can issue ";
		}
		temp += f.Link("#build", "BUILD") + " orders. ";
	}
	temp += "Here is a table of the various building types:";
	f.Paragraph(temp);
	f.LinkRef("tablebuildings");
	f.Enclose(1, "center");
	f.Enclose(1, "table border=\"1\"");
	f.Enclose(1, "tr");
	f.TagText("td", "");
	f.TagText("th", "Size");
	f.TagText("th", "Cost");
	f.TagText("th", "Material");
	f.TagText("th", "Skill (min level)");
	if (Globals->LIMITED_MAGES_PER_BUILDING) {
		f.TagText("th", "Mages");
	}
	f.Enclose(0, "tr");
	for (i = 0; i < NOBJECTS; i++) {
		if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
		if (!ObjectDefs[i].protect) continue;
		pS = FindSkill(ObjectDefs[i].skill);
		if (pS == NULL) continue;
		if (pS->flags & SkillType::MAGIC) continue;
		if (ObjectIsShip(i)) continue;
		j = ObjectDefs[i].item;
		if (j == -1) continue;
		/* Need the >0 since item could be WOOD_OR_STONE (-2) */
		if (j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
		if (j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
		/* Okay, this is a valid object to build! */
		f.Enclose(1, "tr");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr(ObjectDefs[i].name);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr(ObjectDefs[i].protect);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr(ObjectDefs[i].cost);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		if (j == I_WOOD_OR_STONE)
			temp = "wood or stone";
		else
			temp = ItemDefs[j].name;
		f.PutStr(temp);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		temp = pS->name;
		temp += AString(" (") + ObjectDefs[i].level + ")";
		f.PutStr(temp);
		f.Enclose(0, "td");
		if (Globals->LIMITED_MAGES_PER_BUILDING) {
			f.Enclose(1, "td align=\"left\" nowrap");
			f.PutStr(ObjectDefs[i].maxMages);
			f.Enclose(0, "td");
		}
		f.Enclose(0, "tr");
	}
	f.Enclose(0, "table");
	f.Enclose(0, "center");
	// Battle: only the first <protect> men inside get the bonus; ENTER has no capacity check.
	temp = "Size is the number of men inside the building who get its "
		"defensive bonus in combat; any number of units may enter it. Cost "
		"is both the number of man-months of labor and the number of units "
		"of material required to complete the building.  ";
	if (Globals->LIMITED_MAGES_PER_BUILDING) {
		temp += "Mages is the number of mages that the building "
			"provides study facilities for, to enable unhindered "
			"study above level 2 in magical skills.  ";
	}
	temp += "There are possibly other buildings which can be built that "
		"require more advanced resources, or odd skills to construct.   "
		"The description of a skill will include any buildings which "
		"it allows to be built.";
	f.Paragraph(temp);
	temp = "There are other structures that increase the maximum production "
		"of certain items in regions";
	if (!(ObjectDefs[O_MINE].flags & ObjectType::DISABLED))
		temp += "; for example, a Mine will increase the amount of iron "
			"that is available to be mined in a region";
	temp += ".  To construct these structures requires a high skill level in "
		"the production skill related to the item that the structure will "
		"help produce. ";
	{
		// Production structures built with the Building skill instead of the skill of what they
		// aid (Inn, Temple in NewOrigins). Listed from the table so none is forgotten.
		std::vector<std::string> bld;
		for (int o = 0; o < NOBJECTS; o++) {
			if (ObjectDefs[o].flags & ObjectType::DISABLED) continue;
			if (ObjectDefs[o].protect || ObjectIsShip(o)) continue;
			if (ObjectDefs[o].productionAided == -1) continue;
			AString sk = ObjectDefs[o].skill ? ObjectDefs[o].skill : "";
			if (LookupSkill(&sk) != S_BUILDING) continue;
			std::string name = ObjectDefs[o].name;
			bld.push_back(name + "s");
		}
		if (!bld.empty()) {
			temp += "(";
			temp += joinList(bld).c_str();
			temp += bld.size() == 1 ? " are an exception" : " are exceptions";
			temp += " to this rule, requiring the Building skill.) ";
		}
	}
	temp += "This bonus in production is available to any unit in the "
		"region; there is no need to be inside the structure.";
	f.Paragraph(temp);
	// Mirrors ARegion::UpdateProducts (economy.cpp): step = base / 2, then for each
	// completed structure aiding the product, step /= 2 and bonus += step. Every division
	// rounds DOWN, so the 25% / 12.5% / 6.25% shares are each rounded down on their own, and
	// the first share is 25% of the base rounded down. It runs in PostTurn, so a structure
	// finished during a month counts from the next month; incomplete ones never count.
	temp = "The first structure built in a region will increase the maximum "
		"production of the related product by 25%; the amount added by each "
		"additional structure will be half of the effect of the previous one, "
		"so 12.5% for the second, 6.25% for the third, and so on. Each of "
		"these amounts is calculated from the region's normal production and "
		"rounded down to a whole number on its own, so small amounts are soon "
		"rounded down to nothing: if you build enough of the same type of "
		"structure in a region, the new structures may not add any production "
		"at all. Only completed structures count, and a structure finished "
		"during a month raises the production from the following month.";
	f.Paragraph(temp);
	{
		auto withStructures = [](int base, int count) {
			int step = base / 2;
			int bonus = 0;
			for (int n = 0; n < count; n++) {
				step /= 2;
				bonus += step;
			}
			return base + bonus;
		};
		const int bases[] = { 20, 15 };
		temp = "For example, here is the maximum production of a product in "
			"two regions, one where it is normally 20 and one where it is "
			"normally 15, with up to three structures:";
		f.Paragraph(temp);
		f.Enclose(1, "center");
		f.Enclose(1, "table border=\"1\"");
		f.Enclose(1, "tr");
		f.TagText("th", "Normal production");
		f.TagText("th", "No structure");
		f.TagText("th", "1 structure");
		f.TagText("th", "2 structures");
		f.TagText("th", "3 structures");
		f.Enclose(0, "tr");
		for (int base : bases) {
			f.Enclose(1, "tr");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(base);
			f.Enclose(0, "td");
			for (int count = 0; count <= 3; count++) {
				int now = withStructures(base, count);
				temp = AString(now);
				if (count > 0) {
					temp += AString(" (+") + (now - withStructures(base, count - 1)) + ")";
				}
				f.Enclose(1, "td align=\"center\"");
				f.PutStr(temp);
				f.Enclose(0, "td");
			}
			f.Enclose(0, "tr");
		}
		f.Enclose(0, "table");
		f.Enclose(0, "center");
		temp = "In the first region, the first structure adds 25% of 20, which "
			"is 5. The second adds 12.5% of 20, which is 2.5, rounded down to "
			"2. The third adds 6.25% of 20, which is 1.25, rounded down to 1. "
			"In the second region, 25% of 15 is 3.75, so the first structure "
			"only adds 3, the second adds 1 (1.875 rounded down), and the "
			"third adds nothing (0.9375 rounded down).";
		f.Paragraph(temp);
	}
	f.LinkRef("tabletradestructures");
	f.Enclose(1, "center");
	f.Enclose(1, "table border=\"1\"");
	f.Enclose(1, "tr");
	f.TagText("td", "");
	f.TagText("th", "Cost");
	f.TagText("th", "Material");
	f.TagText("th", "Skill (level)");
	f.TagText("th", "Production Aided");
	f.Enclose(0, "tr");
	for (i = 0; i < NOBJECTS; i++) {
		if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
		if (ObjectDefs[i].protect) continue;
		if (ObjectIsShip(i)) continue;
		j = ObjectDefs[i].productionAided;
		if (j == -1) continue;
		if (ItemDefs[j].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[j].type & IT_NORMAL)) continue;
		pS = FindSkill(ObjectDefs[i].skill);
		if (pS == NULL) continue;
		if (pS->flags & SkillType::MAGIC) continue;
		j = ObjectDefs[i].item;
		if (j == -1) continue;
		/* Need the >0 since item could be WOOD_OR_STONE (-2) */
		if (j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
		if (j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
		/* Okay, this is a valid object to build! */
		f.Enclose(1, "tr");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr(ObjectDefs[i].name);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr(ObjectDefs[i].cost);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		if (j == I_WOOD_OR_STONE)
			temp = "wood or stone";
		else
			temp = ItemDefs[j].name;
		f.PutStr(temp);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		temp = pS->name;
		if (ObjectDefs[i].level > 1)
			temp += AString(" (") + ObjectDefs[i].level + ")";
		f.PutStr(temp);
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		if (ObjectDefs[i].productionAided == I_SILVER)
			f.PutStr("entertainment");
		else
			f.PutStr(ItemDefs[ObjectDefs[i].productionAided].names);
		f.Enclose(0, "td");
		f.Enclose(0, "tr");
	}
	f.Enclose(0, "table");
	f.Enclose(0, "center");
	temp = "Note that these structures will not increase the availability "
		"of an item in a region which does not already have that item "
		"available. Also, Trade structures do not offer defensive bonuses "
		"(which is why they do not have a size associated with them).  As "
		"with regular buildings, the Cost is the number of man-months of "
		"labor and also the number of units of raw material required to "
		"complete the structure. ";
	if (!(ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
			!(ItemDefs[I_STONE].flags & ItemType::DISABLED)) {
		temp += "You can use two different materials (wood or stone) to "
			"construct most trade structures. ";
	}
	temp += "It is possible that there are structures not listed above "
		"which require either advanced resources to build or which "
		"increase the production of advanced resources.  The skill "
		"description for a skill will always note if new structures may "
		"be built based on knowing that skill.";
	f.Paragraph(temp);
	if (!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED)) {
		f.LinkRef("economy_roads");
		f.TagText("h3", "Roads:");
		temp = "There is another type of structure called roads.  They do "
			"not protect units, nor aid in the production of resources, but "
			"do aid movement, and can improve the economy of a hex.";
		f.Paragraph(temp);
		temp = "Roads are directional and are only considered to reach from "
			"one hexside to the center of the hex.  To gain a movement "
			"bonus, there must be two connecting roads, one in each "
			"adjacent hex.  Building more than one road in the same direction "
		"gives no additional benefit. "
			"If a road in the given direction is connected, units move "
			"along that road at half cost to a minimum of 1 movement point.";
		f.Paragraph(temp);
		temp = "For example: If a unit is moving northwest, then hex it is "
			"in must have a northwest road, and the hex it is moving into "
			"must have a southeast road.";
		f.Paragraph(temp);
		// The old text ("at least two adjoining hexes", "raises the wages by 1 point") no longer
		// matched the engine. ARegion::RoadDevelopment (economy.cpp) follows connected roads
		// via RoadDevelopmentBonus/TraceConnectedRoad (aregion.cpp), up to 16 hexes: each
		// reached hex gives +1 for a town, +1 if its development > ours + 9 (+2 per hop), +1 if
		// development * 2 > ours * 5. A hex without a town gets half. The points are converted
		// to development-equivalent with diminishing returns (5 each at first, capped ~45) and
		// added in ARegion::Wages; they also raise the odds of development rising (PostTurn).
		temp = "To gain an economy bonus, a hex's roads must connect to roads "
			"in neighboring hexes. The bonus depends on where the connected "
			"roads lead: every hex that can be reached along connected roads, "
			"up to 16 hexes away, gives a bonus point if it contains a "
			"settlement, another if it is more developed than this hex, and "
			"another if it is much more developed. A hex that has a settlement "
			"gets all of these points, and a hex without one gets half of them "
			"(rounded down). The bonus raises the hex's wages, typically by a "
			"fraction of a silver up to a few silver per man, and makes its "
			"development rise faster. The first points count the most, and the "
			"total bonus is limited, so a large road network gives diminishing "
			"returns.";
		f.Paragraph(temp);
		{
			// Copies of the two engine formulas, kept here so the table below is computed rather
			// than hand-written. If ARegion::Wages() or ARegion::RoadDevelopment() change, change
			// these to match.
			auto wagesFor = [](int dv) {          // ARegion::Wages(), without the town term
				int wages = 0, level = 1, last = 0;
				while (dv >= level) {
					wages++;
					last = level;
					level += wages + 1;
				}
				wages *= 10;
				if (dv > last) wages += 10 * (dv - last) / (level - last);
				return wages;                     // in tenths of a silver
			};
			auto roadDevelopment = [](int points) { // conversion loop in ARegion::RoadDevelopment()
				int bonus = 5, leveloff = 1, plateau = 4, total = 0;
				while (points > 0 && total < 46) {
					points--;
					if (leveloff >= plateau && bonus > 1) {
						bonus--;
						leveloff = 1;
						plateau--;
					}
					leveloff++;
					total += bonus;
				}
				return total;
			};
			auto money = [](int tenths) {
				return AString("$") + (tenths / 10) + "." + (tenths % 10);
			};
			const int devs[] = { 20, 40, 70 };
			const int points[] = { 1, 2, 4, 8 };
			temp = "This table shows how the bonus raises the wages in a hex "
				"without a settlement, for three hexes with different wages. The "
				"points are the ones the hex actually receives, after they have "
				"been halved:";
			f.Paragraph(temp);
			f.Enclose(1, "center");
			f.Enclose(1, "table border=\"1\"");
			f.Enclose(1, "tr");
			f.TagText("th", "Wages without a road bonus");
			for (int p : points) {
				f.TagText("th", AString(p) + (p == 1 ? " point" : " points"));
			}
			f.Enclose(0, "tr");
			for (int dev : devs) {
				f.Enclose(1, "tr");
				f.Enclose(1, "td align=\"center\"");
				f.PutStr(money(wagesFor(dev)));
				f.Enclose(0, "td");
				for (int p : points) {
					f.Enclose(1, "td align=\"center\"");
					f.PutStr(money(wagesFor(dev + roadDevelopment(p))));
					f.Enclose(0, "td");
				}
				f.Enclose(0, "tr");
			}
			f.Enclose(0, "table");
			f.Enclose(0, "center");
		}
		f.LinkRef("tableroadstructures");
		f.Enclose(1, "center");
		f.Enclose(1, "table border=\"1\"");
		f.Enclose(1, "tr");
		f.TagText("td", "");
		f.TagText("th", "Cost");
		f.TagText("th", "Material");
		f.TagText("th", "Skill (min level)");
		f.Enclose(0, "tr");
		for (i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (ObjectDefs[i].productionAided != -1) continue;
			if (ObjectDefs[i].protect) continue;
			// Transport structures (Caravanserai) have no protection or production either;
			// they are described under Transportation of goods instead.
			if (ObjectDefs[i].flags & ObjectType::TRANSPORT) continue;
			if (ObjectIsShip(i)) continue;
			pS = FindSkill(ObjectDefs[i].skill);
			if (pS == NULL) continue;
			if (pS->flags & SkillType::MAGIC) continue;
			j = ObjectDefs[i].item;
			if (j == -1) continue;
			/* Need the >0 since item could be WOOD_OR_STONE (-2) */
			if (j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
			if (j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
			/* Okay, this is a valid object to build! */
			f.Enclose(1, "tr");
			f.Enclose(1, "td align=\"left\" nowrap");
			f.PutStr(ObjectDefs[i].name);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"left\" nowrap");
			f.PutStr(ObjectDefs[i].cost);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"left\" nowrap");
			if (j == I_WOOD_OR_STONE)
				temp = "wood or stone";
			else
				temp = ItemDefs[j].name;
			f.PutStr(temp);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"left\" nowrap");
			temp = pS->name;
			temp += AString(" (") + ObjectDefs[i].level + ")";
			f.PutStr(temp);
			f.Enclose(0, "td");
			f.Enclose(0, "tr");
		}
		f.Enclose(0, "table");
		f.Enclose(0, "center");
	}
	if (Globals->DECAY) {
		f.LinkRef("economy_builddecay");
		f.TagText("h3", "Building Decay:");
		temp = "Some structures will decay over time if they are not "
			"maintained. Difficult terrain and bad weather will speed up "
			"this decay. Maintnenance involves having units with the "
			"appropriate level of skill expend a small amount of the "
			"material used to build the structure and labor on a fairly "
			"regular basis in the exactly same manner as they would work on "
			"the building it if it was not completed. In other words, enter "
			"the structure and issue the BUILD command with no parameters. "
			"If a structure will need maintenance, that information will be "
			"related in the object information given about the structure. "
			"If a structure is allowed to decay, it will not give any of "
			"its bonuses until it is repaired.";
		f.Paragraph(temp);
	}
	if (may_sail) {
		f.LinkRef("economy_ships");
		f.TagText("h3", "Ships:");
		temp = "Ships are constructed similarly to buildings, with "
			"a few small differences. "
			"Firstly, they tend to be constructed out of wood, "
			"not stone. "
			"Secondly, their construction tends to depend on the "
			"Shipbuilding skill, not the Building skill. "
			"Thirdly, ships can only be built in a land region next to "
			"the ocean";
		// BUILD (parseorders.cpp): no building in an ocean region at all; ships need
		// IsCoastalOrLakeside (flying ships excepted, see Sailing).
		if (Globals->LAKES > 0) temp += " or a lake";
		temp += " (nothing at all can be built in an ocean region). "
			"Fourthly, while unfinished buildings appear in the "
			"region, and may be entered by other units, "
			"unfinished ships appear only in their builder's "
			"inventory until they are complete.  If the builder ";
		temp += f.Link("#move", "MOVE");
		temp += "s or they are in a fleet that ";
		temp += f.Link("#sail", "SAIL");
		temp += "s while they have an unfinished ship in their "
			"possession, the ship will be discarded and lost. ";
		temp += "Finally, ships are never interacted with as objects "
			"directly, but when completed are placed in Fleet "
			"objects.  Fleets may contain one or more ships, and "
			"may be entered like other buildings.";
		f.Paragraph(temp);
		temp = "";
		// Same gate as the buildings paragraph: ShipConstruction only checks trade activity
		// when BUILD_NO_TRADE is off.
		if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			if (Globals->BUILD_NO_TRADE) {
				temp += "Any faction can issue ";
			} else {
				temp += AString("Only ") + trade_factions + " can issue ";
			}
			temp += f.Link("#build", "BUILD") + " orders. ";
		}
		temp += "Here is a table of the various ship types:";
		f.Paragraph(temp);
		f.LinkRef("tableshipinfo");
		f.Enclose(1, "center");
		f.Enclose(1, "table border=\"1\"");
		f.Enclose(1, "tr");
		f.TagText("td", "Class");
		f.TagText("th", "Capacity");
		f.TagText("th", "Speed");
		f.TagText("th", "Cost");
		f.TagText("th", "Sailors");
		f.TagText("th", "Skill");
		f.Enclose(0, "tr");
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_SHIP)) continue;
			int pub = 1;
			for (int c = 0; c < (int) sizeof(ItemDefs->pInput)/(int) sizeof(Materials); c++) {
				int m = ItemDefs[i].pInput[c].item;
				if (m != -1) {
					if (ItemDefs[m].flags & ItemType::DISABLED) pub = 0;
					if ((ItemDefs[m].type & IT_ADVANCED) ||
						(ItemDefs[m].type & IT_MAGIC)) pub = 0;
				}
			}
			if (pub == 0) continue;
			if (ItemDefs[i].mLevel > 0) continue;
			int slevel = ItemDefs[i].pLevel;
			if (slevel > 3) continue;
			f.Enclose(1, "tr");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].name);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].fly > 0 ? ItemDefs[i].fly : ItemDefs[i].swim);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].speed);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].pMonths);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[i].weight/50);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(slevel);
			f.Enclose(0, "td");
			f.Enclose(0, "tr");
		}					
		for (i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (!ObjectIsShip(i)) continue;
			if (ItemDefs[ObjectDefs[i].item].flags & ItemType::DISABLED)
				continue;
			int normal = (ItemDefs[ObjectDefs[i].item].type & IT_NORMAL);
			normal |= (ItemDefs[ObjectDefs[i].item].type & IT_TRADE);
			if (!normal) continue;
			f.Enclose(1, "tr");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ObjectDefs[i].name);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ObjectDefs[i].capacity);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ItemDefs[ObjectDefs[i].item].speed);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ObjectDefs[i].cost);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(ObjectDefs[i].sailors);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(AString("")+1);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"center\"");
			f.PutStr(AString("no"));
			f.Enclose(0, "td");
			f.Enclose(0, "tr");
		}
		f.Enclose(0, "table");
		f.Enclose(0, "center");
		temp = "The capacity of a ship is the maximum weight that the "
			"ship may have aboard and still move. The cost is both "
			"the man-months of labor and the number of units of "
			"material required to complete the ship. The sailors "
			"are the number of skill levels of the Sailing skill "
			"that must be aboard the ship (and issuing the ";
		temp += f.Link("#sail", "SAIL") + " order) in order for the ship "
			"to sail. The speed is the number of movement points the ship "
			"gets per month, and the skill is the level of Shipbuilding "
			"needed to build it.";
		f.Paragraph(temp);
		// Game::CreateShip: joins the fleet the builder is inside if both fly or both sail;
		// otherwise a new fleet is made and the builder moved into it (leaving any building).
		temp = "When a ship is finished, if its builder is inside a fleet, "
			"the ship is added to that fleet";
		{
			int fly = 0;
			for (int it = 0; it < NITEMS; it++) {
				if (ItemDefs[it].flags & ItemType::DISABLED) continue;
				if ((ItemDefs[it].type & IT_SHIP) && ItemDefs[it].fly > 0) fly = 1;
			}
			if (fly) temp += " (flying and sailing ships cannot be mixed in a fleet)";
		}
		temp += "; otherwise a new fleet is created to hold the ship, and the "
			"builder moves into it, leaving any building it was in.  A fleet "
			"has the combined "
			"capacity and sailor requirement of its constituent "
			"vessels, and moves at the speed of its slowest ship. ";
		f.Paragraph(temp);
	}
	f.LinkRef("economy_advanceditems");
	f.TagText("h3", "Advanced Items:");
	temp = "There are also certain advanced items that highly skilled units "
		"can produce. These are not available to starting players, but can "
		"be discovered through study.  When a unit is skilled enough to "
		"produce one of these items, he will receive a skill report "
		"describing the production of this item. Production of advanced "
		"items is generally done in a manner similar to the normal items.";
	f.Paragraph(temp);
	f.LinkRef("economy_income");
	f.TagText("h3", "Income:");
	temp = "Units can earn money with the ";
	temp += f.Link("#work", "WORK") + " order.  This means that the unit "
		"spends the month performing manual work for wages. The amount to "
		"be earned from this is usually not very high, so it is generally "
		"a last resort to be used if one is running out of money. The "
		"current wages are shown in the region description for each region, "
		"for example \"Wages: $15.0 (Max: $500)\": each working man earns "
		"the wage, but the region pays out at most the Max in total each "
		"month; if more work is done than that, the Max is shared out in "
		"proportion to how much each unit worked. All units may ";
	temp += f.Link("#work", "WORK") + ", regardless of skills";
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp += " or faction type, and working does not count toward any "
			"faction limit";
	temp += ".";
	if (Globals->DEFAULT_WORK_ORDER) {
		temp += " A unit that is not given any month long order works "
			"automatically (see ";
		temp += f.Link("#playing_turns", "Turns") + ").";
	}
	f.Paragraph(temp);
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		f.LinkRef("economy_entertainment");
		f.TagText("h3", "Entertainment:");
		temp = "Units with the Entertainment skill can use it to earn "
			"money.  A unit with Entertainment level 1 will earn ";
		temp += AString(Globals->ENTERTAIN_INCOME) +
			" silver per man by issuing the ";
		temp += f.Link("#entertain", "ENTERTAIN") + " order.  The total "
			"amount of money that can be earned this way is shown in the "
			"region descriptions.  Higher levels of Entertainment skill can "
			"earn more, so a character with Entertainment skill 2 can earn "
			"twice as much money as one with skill 1 (and uses twice as "
			"much of the demand for entertainment in the region). Note that "
			"entertainment income is much less, per region, than the income "
			"available through working or taxing.";
		if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			temp += " All factions may have entertainers, regardless of "
				"faction type.";
		}
		f.Paragraph(temp);
	}

	f.LinkRef("economy_taxingpillaging");
	f.TagText("h3", "Taxing/Pillaging:");
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp = capitalized(tax_factions) + " ";
	else
		temp = "Factions ";
	temp += "may collect taxes in a region.  This is done using the ";
	temp += f.Link("#tax", "TAX") + " order (which is ";
	if (!Globals->TAX_PILLAGE_MONTH_LONG) temp += "not ";
	temp += "a month long order). The amount of tax money that can be "
		"collected each month in a region is shown in the region "
		"description. ";
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE) {
		temp += "Any unit may ";
		temp += f.Link("#tax", "TAX");
	} else {
		AString temp3;
		int prev = 0, hold = 0;
		temp += "A unit may ";
		temp += f.Link("#tax", "TAX");
		temp += " if it ";
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_COMBAT_SKILL | GameDefs::TAX_BOW_SKILL |
				 GameDefs::TAX_RIDING_SKILL | GameDefs::TAX_STEALTH_SKILL)) {
			int	prev2 = 0, hold2 = 0;
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			temp2 = "has ";
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_COMBAT_SKILL) {
				temp3 = "Combat";
				hold2 = 1;
			}
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_BOW_SKILL) {
				if (hold2) {
					temp2 += temp3;
					prev2 = 1;
				}
				if (prev2) temp2 += ", ";
				temp2 += "Longbow";
				prev2 = 1;
				temp3 = "Crossbow";
				hold2 = 1;
			}
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_RIDING_SKILL) {
				if (hold2) {
					if (prev2) temp2 += ", ";
					temp2 += temp3;
					prev2 = 1;
				}
				temp3 = "Riding";
				hold2 = 1;
			}
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_STEALTH_SKILL) {
				if (hold2) {
					if (prev2) temp2 += ", ";
					temp2 += temp3;
					prev2= 1;
				}
				temp3 = "Stealth";
				hold2 = 1;
			}
			if (prev2) temp2 += " or ";
			temp2 += temp3;
			temp2 += " skill of at least level 1";
			hold = 1;
		}
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_ANY_WEAPON | GameDefs::TAX_USABLE_WEAPON |
				 GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL |
				 GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON)) {
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			temp2 = "has ";
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANY_WEAPON)
				temp2 += "a weapon (regardless of skill requirements)";
			else if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_WEAPON)
				temp2 += "a weapon and the appropriate skill to use it";
			else if (Globals->WHO_CAN_TAX &
					(GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL |
					 GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON)) {
				AString temp3;
				int prev2 = 0, hold2 = 0;
				if (Globals->WHO_CAN_TAX &
						GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL) {
					temp2 += "Combat skill of at least level 1 and a "
						"weapon which does not require any skill";
					temp3 = "Riding skill of at least level 1 and "
						"a weapon which requires riding skill";
					prev2 = 1;
					hold2 = 1;
				}
				if (Globals->WHO_CAN_TAX &
						GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON) {
					if (hold2) {
						temp2 += ", ";
						temp2 += temp3;
					}
					temp3 = "a Bow (Longbow or Crossbow) skill and a weapon "
						"which requires that skill";
					hold2 = 1;
				}
				if (prev2) temp2 += " or ";
				temp2 += temp3;
			}
			hold = 1;
		}
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_HORSE | GameDefs::TAX_HORSE_AND_RIDING_SKILL)) {
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			temp2 = "has a mount";
			if (!(Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE))
				temp2 += " and sufficient skill to ride it in combat";
			hold = 1;
		}
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_ANY_MAGE | GameDefs::TAX_MAGE_DAMAGE |
				 GameDefs::TAX_MAGE_FEAR | GameDefs::TAX_MAGE_OTHER)) {
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			temp2 = "is a mage ";
			if ((Globals->WHO_CAN_TAX &
						(GameDefs::TAX_MAGE_DAMAGE |
						 GameDefs::TAX_MAGE_FEAR |
						 GameDefs::TAX_MAGE_OTHER)) ==
					(GameDefs::TAX_MAGE_DAMAGE | GameDefs::TAX_MAGE_FEAR |
					 GameDefs::TAX_MAGE_OTHER)) {
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_COMBAT_SPELL)
					temp2 += "who has a combat spell set";
				else
					temp2 += "who knows a combat spell";
			} else {
				int hold2 = 0, prev2 = 0;
				AString temp3;
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_COMBAT_SPELL)
					temp2 += "whose combat spell ";
				else
					temp2 += "who knows a spell which ";
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_DAMAGE) {
					temp3 = "damages enemies";
					hold2 = 1;
				}
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_FEAR) {
					if (hold2) {
						temp2 += temp3;
						prev2 = 1;
					}
					temp3 = "weakens opponents";
					hold2 = 1;
				}
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_OTHER) {
					if (hold2) {
						if (prev2) temp2 += ", ";
						temp2 += temp3;
						prev2 = 1;
					}
					temp3 = "protects in combat";
					hold2 = 1;
				}
				if (prev2) temp2 += " or ";
				temp2 += temp3;
			}
			hold = 1;
		}
		if (Globals->WHO_CAN_TAX &
				(GameDefs::TAX_BATTLE_ITEM |
				 GameDefs::TAX_USABLE_BATTLE_ITEM)) {
			if (hold) {
				if (prev) temp += ", ";
				temp += temp2;
				prev= 1;
			}
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_BATTLE_ITEM)
				temp2 = "has a ";
			else
				temp2 = "can use a ";
			temp2 += "magical item which gives a special attack in combat";
			hold = 1;
		}
		if (prev) temp += " or ";
		temp += temp2;
		temp += ". ";
	}
	if (Globals->WHO_CAN_TAX &
			(GameDefs::TAX_CREATURES | GameDefs::TAX_ILLUSIONS)) {
		if (Globals->WHO_CAN_TAX & GameDefs::TAX_CREATURES) {
			temp += "Summoned ";
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS)
				temp += "and illusory ";
		} else if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS)
			temp += "Illusory ";
		temp += "creatures will assist in taxation. ";
	}
	// Unit::Taxers: a unit qualifying through a skill (or as a mage) taxes with every man;
	// otherwise only as many men as it has qualifying items (weapons, mounts, ...).
	if (!(Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE)) {
		temp += "If a unit can tax because of a skill, every man in it taxes; "
			"if it can only tax because of its equipment, only as many men "
			"tax as it has suitable items, so 10 men with 4 swords tax as 4 "
			"men. ";
	}
	temp +=	"Each taxing character can collect $";
	temp += AString(Globals->TAX_BASE_INCOME) + ", though if the number of "
		"taxers would tax more than the available tax income, the tax "
		"income is split evenly among all taxers.";
	f.Paragraph(temp);
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp = capitalized(tax_factions) + " ";
	else
		temp = "Factions ";
	// Game::RunPillageOrders: all pillaging units in the region, of every faction, count
	// toward the threshold (pillagers * 2 * TAX_BASE_INCOME >= wealth). ARegion::Pillage
	// sets wealth to 0, removes a third of the development, and kills some people.
	temp += "may also pillage a region. To do this requires enough combat "
		"ready men pillaging in the region (counting the pillagers of all "
		"factions together) to tax half of the available money in the "
		"region. The total amount of money that can be pillaged will then "
		"be shared out between every combat ready unit that issues the ";
	temp += f.Link("#pillage", "PILLAGE") + " order. The amount of money "
		"collected is equal to twice the available tax money. However, the "
		"economy of the region will be seriously damaged by pillaging: "
		"no tax money is left that month, the region loses a third of its "
		"development, some of its people are killed, and it will only "
		"slowly recover over time.  Note that ";
	temp += f.Link("#pillage", "PILLAGE") + " comes before " +
		f.Link("#tax", "TAX") + ", so a unit performing " +
		f.Link("#tax", "TAX") + " will collect no money in that region that "
		"month.";
	f.Paragraph(temp);
	temp = "It is possible to safeguard one's tax income in regions one "
		"controls.  Units which have the Guard flag set (using the ";
	temp += f.Link("#guard", "GUARD") + " order) will block " +
		f.Link("#tax", "TAX") + " orders issued by other factions in the same "
		"region, unless you have declared the faction in question Friendly. "
		"Units on guard will also block ";
	temp += f.Link("#pillage", "PILLAGE") + " orders issued by other "
		"factions in the same region, regardless of your attitude towards "
		"the faction in question, and they will attempt to prevent "
		"Unfriendly and Hostile units that they can see and catch from "
		"entering the region.  Only units which are able to tax may be on "
		"guard, and there are further restrictions on who may guard a region; "
		"see the ";
	temp += f.Link("#guard", "GUARD") + " order.  Units on guard ";
	if (has_stea)
		temp += " are always visible regardless of Stealth skill, and ";
	temp += "will be marked as being \"on guard\" in the region description.";
	f.Paragraph(temp);

	if (qm_exist) {
		f.LinkRef("economy_transport");
		f.TagText("H3", "Transportation of goods");

		temp = capitalized(trade_factions) + " may train Quartermaster units. "
			"A Quartermaster unit may accept ";
		temp += f.Link("#transport", "TRANSPORT") + "ed items from "
			"any unit within " + Globals->LOCAL_TRANSPORT + " hex";
		if (Globals->LOCAL_TRANSPORT != 1)
			temp += "es";
		temp +=	" distance from the hex containing the quartermaster. "
			"Quartermasters may also ";
		temp += f.Link("#distribute", "DISTRIBUTE") + " items to any "
			"unit within " + Globals->LOCAL_TRANSPORT + " hex";
		if (Globals->LOCAL_TRANSPORT != 1)
			temp += "es";
		temp +=	" distance from the hex containing the quartermaster "
			"and may ";
		temp += f.Link("#transport", "TRANSPORT") +
			" items to another quartermaster up to " +
			Globals->NONLOCAL_TRANSPORT + " hex";
		if (Globals->NONLOCAL_TRANSPORT != 1)
			temp += "es";
		temp += " distant.";
		if (Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST) {
			temp += " The distance a quartermaster can ";
			temp += f.Link("#transport", "TRANSPORT") + " items to "
				"another quartermaster will increase with the level of "
				"skill possessed by the quartermaster unit.";
		}
		f.Paragraph(temp);
		// CheckTransportOrders: within LOCAL_TRANSPORT anyone may TRANSPORT to a quartermaster
		// for free; DISTRIBUTE and longer TRANSPORT need a quartermaster owning a COMPLETED
		// TRANSPORT structure. Those structures used to be listed under Roads (no protection, no
		// production); they are now described here with what they cost to build.
		temp = AString("Transport within ") + Globals->LOCAL_TRANSPORT + " hex";
		if (Globals->LOCAL_TRANSPORT != 1) temp += "es";
		temp += " is free. To ";
		temp += f.Link("#distribute", "DISTRIBUTE") + " items, or to ";
		temp += f.Link("#transport", "TRANSPORT") + " them further, a "
			"quartermaster must be the owner of a completed structure which "
			"allows transportation of items. ";
		{
			std::vector<std::string> structs;
			for (i = 0; i < NOBJECTS; i++) {
				if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
				if (!(ObjectDefs[i].flags & ObjectType::TRANSPORT)) continue;
				std::string d = ObjectDefs[i].name;
				pS = FindSkill(ObjectDefs[i].skill);
				int it = ObjectDefs[i].item;
				if (pS && it != -1) {
					d += " (" + std::to_string(ObjectDefs[i].cost) + " ";
					d += it == I_WOOD_OR_STONE ? "wood or stone" : ItemDefs[it].name;
					d += std::string(", ") + pS->name + " " +
						std::to_string(ObjectDefs[i].level) + ")";
				}
				structs.push_back(d);
			}
			temp += structs.size() == 1 ? "This structure is the " :
				"The structures which allow this are the ";
			temp += joinList(structs).c_str();
			temp += ".";
		}
		f.Paragraph(temp);

		if (Globals->SHIPPING_COST > 0) {
			temp = "The cost of transport items from one quartermaster to "
				"another is based on the weight of the items and costs ";
			temp += Globals->SHIPPING_COST;
			temp += " silver per weight unit.";
			if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST) {
				temp += " The cost of shipping is increased for units with a "
					"lower quartermaster skill, dropping to the minimum "
					"above when the unit is at the maximum skill level.";
			}
			f.Paragraph(temp);
		}
		// Cost per weight = SHIPPING_COST * (4 - (level+1)/2) with QM_AFFECT_COST; range =
		// NONLOCAL_TRANSPORT + (level+1)/3 with QM_AFFECT_DIST (both runorders.cpp).
		if ((Globals->TRANSPORT & (GameDefs::QM_AFFECT_COST | GameDefs::QM_AFFECT_DIST)) &&
				Globals->NONLOCAL_TRANSPORT > 0) {
			f.Paragraph("Transport to another quartermaster, by the skill of the "
				"quartermaster sending it:");
			f.Enclose(1, "center");
			f.Enclose(1, "table border=\"1\"");
			f.Enclose(1, "tr");
			f.TagText("th", "Quartermaster level");
			f.TagText("th", "Range (hexes)");
			if (Globals->SHIPPING_COST > 0) f.TagText("th", "Cost per weight unit");
			f.Enclose(0, "tr");
			for (int lvl = 1; lvl <= 5; lvl++) {
				int range = Globals->NONLOCAL_TRANSPORT;
				if (Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST) range += (lvl + 1) / 3;
				int cost = Globals->SHIPPING_COST;
				if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST) cost *= 4 - (lvl + 1) / 2;
				// TagText would close with "</td align=...>", so open and close by hand.
				AString cells[] = { AString(lvl), AString(range), AString("$") + cost };
				int ncells = Globals->SHIPPING_COST > 0 ? 3 : 2;
				f.Enclose(1, "tr");
				for (int c = 0; c < ncells; c++) {
					f.Enclose(1, "td align=\"center\"");
					f.PutStr(cells[c]);
					f.Enclose(0, "td");
				}
				f.Enclose(0, "tr");
			}
			f.Enclose(0, "table");
			f.Enclose(0, "center");
		}
		{
		}

		temp = "Quartermasters must be single man units";
		if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			temp += ", and a faction is limited in the number of "
				"quartermasters it may have at any one time";
		}
		temp += ". ";
		// Same gate as the TRANSPORT/DISTRIBUTE order entries: CheckTransportOrders skips the
		// TRADE ActivityCheck when TRANSPORT_NO_TRADE is set.
		if (!Globals->TRANSPORT_NO_TRADE) {
			temp += "Both the ";
			temp += f.Link("#transport", "TRANSPORT") + " and " +
				f.Link("#distribute", "DISTRIBUTE") + " orders count as "
				"trade activity in the hex of the unit issuing the order. ";
		}
		temp += "The target unit must be at least FRIENDLY to the unit "
			"which issues the order.";
		// CheckTransportOrders measures range with GetPlanarDistance, adding the rng_transport
		// crossLevelPenalty (10000000 in the base table; no ruleset overrides it) per level
		// crossed, and GetPlanarDistance returns 10000000 outright for the Nexus. That can only
		// exceed the range when the range is checked at all: a range of 0 means unlimited and
		// skips the check. The longest range is NONLOCAL_TRANSPORT plus the quartermaster bonus
		// (level + 1) / 3, at most 2 for a level 5 quartermaster.
		{
			int penalty = 10000000;
			RangeType *rt = FindRange("rng_transport");
			if (rt) penalty = rt->crossLevelPenalty;
			int longest = std::max(Globals->LOCAL_TRANSPORT,
				Globals->NONLOCAL_TRANSPORT);
			if (Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST) longest += 2;
			if (Globals->LOCAL_TRANSPORT > 0 && Globals->NONLOCAL_TRANSPORT > 0 &&
					penalty > longest) {
				temp += " Items cannot be transported or distributed to a unit "
					"on a different level of the world (for example, between "
					"the surface and the underworld)";
				if (Globals->NEXUS_EXISTS) {
					temp += ", nor to or from the Nexus";
				}
				temp += "; they have to be moved there some other way.";
			}
		}

		f.Paragraph(temp);

		// Built from the item tables rather than hard-coded: ParseTransportableItem (items.cpp)
		// rejects every enabled item flagged NOTRANSPORT or CANTGIVE, and nothing else in
		// Check/RunTransportOrders restricts items. The old fixed list omitted livestock and
		// described NewOrigins' magic items as "created using artifact lore". Each item is put
		// in the first category it matches; a category is named only if something in it is
		// blocked, with any transportable members listed as exceptions, and blocked items that
		// fit no category (e.g. livestock) are named individually.
		{
			struct Category {
				const char *label;
				std::vector<std::string> blocked, allowed;
			};
			Category cats[] = {
				{ Globals->LEADERS_EXIST ? "men (including leaders)" : "men", {}, {} },
				{ "ships", {}, {} },
				{ "mounts", {}, {} },
				{ "war machines", {}, {} },
				{ "creatures (including summoned and illusionary ones)", {}, {} },
				{ "magic items", {}, {} },
			};
			std::vector<std::string> other;
			for (i = 0; i < NITEMS; i++) {
				if (ItemDefs[i].flags & ItemType::DISABLED) continue;
				int t = ItemDefs[i].type;
				int c = -1;
				if (t & IT_MAN) c = 0;
				else if (t & IT_SHIP) c = 1;
				else if (t & IT_MOUNT) c = 2;
				// Built monsters (catapult, steel defender) are war machines; others are creatures.
				else if ((t & IT_MONSTER) && ItemDefs[i].pSkill) c = 3;
				else if (t & IT_MONSTER) c = 4;
				else if (t & IT_MAGIC) c = 5;
				int blocked = (ItemDefs[i].flags &
					(ItemType::NOTRANSPORT | ItemType::CANTGIVE)) != 0;
				if (c < 0) {
					if (blocked) other.push_back(ItemDefs[i].names);
					continue;
				}
				(blocked ? cats[c].blocked : cats[c].allowed).push_back(
					ItemDefs[i].names);
			}
			std::vector<std::string> parts;
			for (auto &cat : cats) {
				if (cat.blocked.empty()) continue;
				std::string part = cat.label;
				if (!cat.allowed.empty()) {
					part += " (except ";
					for (size_t n = 0; n < cat.allowed.size(); n++) {
						if (n > 0) part += (n == cat.allowed.size() - 1) ? " and " : ", ";
						part += cat.allowed[n];
					}
					part += ")";
				}
				parts.push_back(part);
			}
			for (auto &name : other) parts.push_back(name);

			if (!parts.empty()) {
				f.LinkRef("transport_items");
				temp = "Not all items can be ";
				temp += f.Link("#transport", "TRANSPORT") + "ed or ";
				temp += f.Link("#distribute", "DISTRIBUTE") + "d. ";
				std::string list;
				for (size_t n = 0; n < parts.size(); n++) {
					if (n > 0) list += (n == parts.size() - 1) ? ", and " : ", ";
					list += parts[n];
				}
				list[0] = toupper(list[0]);
				temp += list.c_str();
				temp += " cannot be sent this way; they have to be carried or "
					"sailed from one place to another by a unit. An order to "
					"transport or distribute one of them is rejected as an "
					"invalid item.";
				f.Paragraph(temp);
			}
		}
	}

	f.LinkRef("com");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Combat");
	temp = "Combat occurs when one unit attacks another.  The computer then "
		"gathers together all the units on the attacking side, and all the "
		"units on the defending side, and the two sides fight until an "
		"outcome is reached.";
	f.Paragraph(temp);
	f.LinkRef("com_attitudes");
	f.TagText("h3", "Attitudes:");
	temp = "Which side a faction's units will fight on depends on declared "
		"attitudes.  A faction can have one of the following attitudes "
		"towards another faction:  Ally, Friendly, Neutral, Unfriendly or "
		"Hostile.  Each faction has a general attitude, called the \"Default "
		"Attitude\", that it normally takes towards other factions; this is "
		"initially Neutral, but can be changed.  It is also possible to ";
	temp += f.Link("#declare", "DECLARE") + " attitudes to specific "
		"factions, e.g. ";
	temp += f.Link("#declare", "DECLARE") + " 27 ALLY will declare the "
		"Ally attitude to faction 27.  (Note that this does not necessarily "
		"mean that faction 27 has decided to treat you as an ally.)";
	f.Paragraph(temp);
	temp = "Ally means that you will fight to defend units of that faction "
		"whenever they come under attack, if you have non-avoiding units in "
		"the region where the attack occurs. ";
	// ARegion::CanGuard with STRICT_GUARD: a new guard needs every existing guard to be Ally.
	if (Globals->STRICT_GUARD) {
		temp += "Also, while your units are on guard in a region, a unit of "
			"another faction can only go on guard there if you have declared "
			"its faction Ally (see the ";
		temp += f.Link("#guard", "GUARD") + " order). ";
	}
	if (has_stea) {
		temp += " You will also attempt to prevent any theft or "
			"assassination attempts against units of the faction";
		if (has_obse) {
			temp += ", if you are capable of seeing the unit which is "
				"attempting the crime";
		}
		temp += ". ";
	}
	temp += "It also has the implications of the Friendly attitude.";
	f.Paragraph(temp);
	temp = "Friendly means that you will accept gifts from units of that "
		"faction.  This includes the giving of items, units of people, and "
		"the teaching of skills.  You will also admit units of that faction "
		"into buildings or fleets owned by one of your units, and you will "
		"permit units of that faction to collect taxes (but not pillage) "
		"in regions where you have units on guard.";
	f.Paragraph(temp);
	temp = "Unfriendly means that you will not admit units of that faction "
		"into any region where you have units on guard.  You will not, "
		"however, automatically attack unfriendly units which are already "
		"present.";
	f.Paragraph(temp);
	temp = "Hostile means that any of your units which do not have the "
		"Avoid Combat flag set (using the ";
	temp += f.Link("#avoid", "AVOID") + " order) will attack any units of "
		"that faction wherever they find them.";
	f.Paragraph(temp);
	temp = "If a unit can see another unit, but ";
	if (has_obse) {
		temp += "does not have high enough Observation skill to determine "
			"its faction,";
	} else {
		temp += "it is not revealing its faction,";
	}
	temp += " it will treat the unit using the faction's default attitude, "
		"even if the unit belongs to an Unfriendly or Hostile faction, "
		"because it does not know the unit's identity.  However, if your "
		"faction has declared an attitude of Friendly or Ally towards that "
		"unit's faction, the unit will be treated with the better attitude; "
		"it is assumed that the unit will produce proof of identity when "
		"relevant.";
	if (has_stea) {
		temp += " (See the section on stealth for more information on when "
			"units can see each other.)";
	}
	f.Paragraph(temp);
	temp = "If a faction declares Unfriendly or Hostile as default attitude "
		"(the latter is a good way to die fast), it will block or attack "
		"all unidentified units, unless they belong to factions for which a "
		"Friendly or Ally attitude has been specifically declared.";
	if (has_stea) {
		temp += " Units which cannot be seen at all cannot be directly "
			"blocked or attacked, of course.";
	}
	f.Paragraph(temp);
	f.LinkRef("com_attacking");
	f.TagText("h3", "Attacking:");
	temp = "A unit can attack another by issuing an ";
	temp += f.Link("#attack", "ATTACK") + " order. A unit that does not "
		"have Avoid Combat set will automatically attack any Hostile units "
		"it identifies as such.";
	if (has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		temp += " When a unit issues the ";
		temp += f.Link("#attack", "ATTACK") + " order, or otherwise "
			"decides to attack another unit, it must first be able to "
			"attack the unit. ";
		if (has_stea && !(SkillDefs[S_RIDING].flags & SkillType::DISABLED))
			temp += "There are two conditions for this; the first is that the";
		else
			temp += "The";
		if (has_stea) {
			// Unit::CanSee / CanCatch delegate to the faction: any unit of the attacker's
			// faction in the region with enough Observation / Riding is enough.
			temp += " attacking faction must be able to see the unit that it "
				"wishes to attack: it is enough that any of its units in the "
				"region can see it. More information is available on this "
				"in the stealth section of the rules.";
		}
		if (!(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
			if (has_stea) {
				f.Paragraph(temp);
				temp = "Secondly, the";
			}
			temp += " attacking faction must be able to catch the unit it "
				"wishes to attack (again, any of its units in the region "
				"will do).  A unit may only catch a unit if its "
				"effective Riding skill is greater than or equal to the "
				"target unit's effective Riding skill; otherwise, the "
				"target unit just rides away from the attacking unit.  "
				"Effective Riding is the unit's Riding skill, but with "
				"a potential maximum; if the unit can not ride, the "
				"effective Riding skill is 0; if the unit can ride, the "
				"maximum effective Riding is 3; if the unit can fly, the "
				"maximum effective Riding is 5. Note that the effective "
				"Riding also depends on whether the unit is attempting to "
				"attack or defend; for attack purposes, only one man in "
				"the unit needs to be able to ride or fly (generally, this "
				"means one of the men must possess a horse, or other form "
				"of transportation), whereas for defense purposes the entire "
				"unit needs to be able to ride or fly (usually meaning "
				"that every man in the unit must possess a horse or other "
				"form of speedier transportation). Also, note that for a "
				"unit to be able to use its defensive Riding ability to "
				"avoid attack, the unit cannot be in a building, fleet, or "
				"structure of any type, and it cannot be on guard. A unit's "
				"defensive Riding is also limited by the weakest of its "
				"mounts.";
		}
	}
	f.Paragraph(temp);
	temp = "A unit which is on guard, and is Unfriendly towards a unit, "
		"will deny access to units using the ";
	temp += f.Link("#move", "MOVE") + " order to enter its region. ";
	if (has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		temp += "Note that to deny access to a unit, at least one unit "
			"from the same faction as the unit guarding the hex must satisfy "
			"the above requirements. ";
	}
	temp += "A unit using ";
	temp += f.Link("#advance", "ADVANCE") + " instead of " +
		f.Link("#move", "MOVE") + " to enter a region, will attack any "
		"units that attempt to deny it access.  If the advancing unit loses "
		"the battle, it will be forced to retreat to the previous region it "
		"moved through.  If the unit wins the battle, it is allowed to "
		"continue to move, provided that it has enough movement points and "
		"its side did not suffer losses that stop it (see ";
	temp += f.Link("#com_victory", "Victory!") + ").";
	f.Paragraph(temp);
	if (has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		temp = "Note that ";
		if (has_stea && !(SkillDefs[S_RIDING].flags & SkillType::DISABLED))
			temp += "these restrictions do ";
		else
			temp += "this restriction does ";
		temp += "not apply for sea combat, as ";
		if (has_stea)
			temp += "units within a fleet are always visible";
		if (!(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
			if (has_stea) temp += ", and";
			temp += " Riding does not play a part in combat on board fleets";
		}
		temp += ".";
		f.Paragraph(temp);
	}
	f.LinkRef("com_muster");
	f.TagText("h3", "The Muster:");
	temp = "Once the attack has been made, the sides are gathered.  Although "
		"the ";
	temp += f.Link("#attack", "ATTACK") + " order takes a unit rather than "
		"a faction as its parameter (mainly so that unidentified units can "
		"be attacked), an attack is basically considered to be by an entire "
		"faction, against an entire faction and its allies.";
	f.Paragraph(temp);
	temp = "On the attacking side are all units of the attacking faction in "
		"the region where the fight is taking place, except those with Avoid "
		"Combat set.  A unit which has explicitly (or implicitly via ";
	temp += f.Link("#advance", "ADVANCE") + ") issued an " +
		f.Link("#attack", "ATTACK") + " order will join the fight anyway, "
		"regardless of whether Avoid Combat is set.";
	f.Paragraph(temp);
	temp = "Also on the attacking side are all units of other factions that "
		"attacked the target faction (implicitly or explicitly) in the "
		"region where the fight is taking place.  In other words, if several "
		"factions attack one, then all their armies join together to attack "
		"at the same time (even if they are enemies and will later fight "
		"each other).";
	// Game::GetAFacs: also joins (without an order) any unit not set to avoid combat whose
	// faction is Hostile to the target, and any unit on ADVANCE that is not allied to it.
	temp += " Units of other factions in the region also join the attack "
		"without being ordered to, if they do not have Avoid Combat set and "
		"their faction has declared the target Hostile, or if they are using ";
	temp += f.Link("#advance", "ADVANCE") + " and are not allied with the "
		"target.";
	f.Paragraph(temp);
	// Game::GetDFacs / GetSides: every non-avoiding unit of the defending faction joins,
	// identified or not; an avoiding one only if attacked or both identified and caught.
	temp = "On the defending side are all units belonging to the defending "
		"faction that do not have Avoid Combat set.  A unit of the target "
		"faction with Avoid Combat set stays out of the battle, unless it "
		"was the unit attacked, or the attackers can both identify its "
		"faction and catch it. ";
	if (has_stea) {
		temp += "(This means that Avoid Combat is mostly useful for high "
			"stealth scouts.) ";
	}
	temp += "Also, all non-avoiding units located in the target region "
		"belonging to factions allied with the defending unit will join "
		"in on the defending side";
	if (Globals->ALLIES_NOAID)
		temp += ", provided that at least one of the units belonging to "
			"the defending faction is not set to noaid.";
	else
		temp += ".";
	f.Paragraph(temp);
	temp = "Units in adjacent regions can also become involved.  This is "
		"the exception to the general rule that you cannot interact with "
		"units in a different region.";
	f.Paragraph(temp);
	temp = "If a faction has at least one unit involved in the initial "
		"region, then any units in adjacent regions will join the fight, "
		"if they could reach the region and do not have Avoid Combat set. "
		"There are a few flags that units may set to affect this; a unit "
		"with the Hold flag (set using the ";
	temp += f.Link("#hold", "HOLD") + " order) will not join battles in "
		"adjacent regions.  This flag applies to both attacking and "
		"defending factions.  A unit with the Noaid flag (set using the ";
	// Battle.cpp: adjacent help is cut off only if every unit of that side in the battle
	// region is NOAID. EXTENDED_FORT_DEFENCE: helpers keep their own building's protection.
	temp += f.Link("#noaid", "NOAID") + " order) will receive no aid from "
		"adjacent hexes when attacked, or when it issues an attack; but "
		"help from adjacent hexes is only refused if every unit of that "
		"side in the battle region has the Noaid flag.";
	if (Globals->EXTENDED_FORT_DEFENCE) {
		temp += " Units that join from an adjacent region keep the "
			"protection of the building they are in.";
	}
	f.Paragraph(temp);
	temp = "Example:  A fight starts in region A, in the initial combat "
		"phase (before any movement has occurred).  The defender has a unit "
		"of soldiers in adjacent region B.  They have 2 movement points at "
		"this stage. ";
	temp += "They will buy horses later in the turn, so that when "
		"they execute their ";
	temp += f.Link("#move", "MOVE") + " order they will have 4 movement "
		"points, but right now they have 2. ";
	if (Globals->WEATHER_EXISTS)
		temp += "Region A is forest, but fortunately it is summer, ";
	else
		temp += "Region A is forest, ";
	temp += "so the soldiers can join the fight.";
	f.Paragraph(temp);
	temp = "It is important to note that the units in nearby regions do not "
		"actually move to the region where the fighting happens; the "
		"computer only checks that they could move there.  (In game world "
		"terms, presumably they did move there to join the fight, and then "
		"moved back where they started.)  The computer checks for weight "
		"allowances and terrain types when determining whether a unit could "
		"reach the scene of the battle. Note that the use of fleets is not "
		"allowed in this virtual movement.";
	f.Paragraph(temp);
	temp = "If you order an attack on an ally (either with the ";
	temp += f.Link("#attack", "ATTACK") + " order, or if your ally has "
		"declared you Unfriendly, by attempting to ";
	temp += f.Link("#advance", "ADVANCE") +" into a region which he is "
		"guarding), then your commander will decide that a mistake has "
		"occurred somewhere, and withdraw your troops from the fighting "
		"altogether.  Thus, your units will not attack that faction in "
		"that region. Note that you will always defend an ally against "
		"attack, even if it means that you fight against other factions "
		"that you are allied with.";
	f.Paragraph(temp);
	f.LinkRef("com_thebattle");
	f.TagText("h3", "The Battle:");
	temp = "The troops having lined up, the fight begins.";
	if (!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED)) {
		// Army::Army takes the highest Tactics of any unit on each side. With ADVANCED_TACTICS
		// (Battle::Run) the better side gets the difference, capped at 3, as a bonus that only
		// lasts the first round and only for physical attacks (Army::Reset clears it).
		temp += " Each side uses the highest Tactics skill of any of its "
			"units.  If one side's Tactics skill is better than the other "
			"side's, ";
		if (Globals->ADVANCED_TACTICS) {
			temp += "then that side gets a bonus equal to the difference (at "
				"most +3) to the attack and defense skills of its soldiers in "
				"the first round of the battle. The bonus applies to melee, "
				"riding and ranged attacks, but not to spells.";
		} else {
			temp += "then that side gets a free round of attacks.";
		}
	}
	f.Paragraph(temp);
	temp = "In each combat round, the combatants each get to attack once, in "
		"a random order. ";
	if (!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED) && !Globals->ADVANCED_TACTICS) {
		temp += "(In a free round of attacks, only one side's forces get to "
			"attack.) ";
	}
	temp += "Each combatant will attempt to hit a randomly selected enemy. "
		"If he hits, and the target has no armor, then the target is "
		"automatically killed (creatures with several hit points lose one "
		"hit point instead).  Armor may provide extra defense against "
		"otherwise successful attacks, and some magic items can protect "
		"their wearer completely.";
	f.Paragraph(temp);
	temp = "The basic skill used in battle is the Combat skill; this is "
		"used for hand to hand fighting.  If one soldier tries to hit "
		"another using most weapons, there is a 50% chance that the "
		"attacker will get an opportunity for a lethal blow.  If the "
		"attacker does get that opportunity, then there is a contest "
		"between his combat skill (modified by weapon attack bonus) and "
		"the defender's combat skill (modified by weapon defense bonus). "
		"Some weapons may not allow combat skill to affect defense (e.g. "
		"bows), and others may allow different skills to be used on "
		"defense (or offense).";
	f.Paragraph(temp);
	temp = "If the skills are equal, then there is a 1:1 (i.e. 50%) "
		"chance that the attack will succeed.  If the attacker's skill is 1 "
		"higher then there is a 2:1 (i.e. 66%) chance, if the attacker's "
		"skill is 2 higher then there is a 4:1 (i.e. 80%) chance, 3 higher "
		"means an 8:1 (i.e. 88%) chance, and so on. Similarly if the "
		"defender's skill is 1 higher, then there is only a 1:2 (i.e. 33%) "
		"chance, etc.";
	f.Paragraph(temp);
	temp = "";
	temp = "There are a variety of weapons in the world which can increase "
		"a soldier's skill on attack or defense.  Better weapons will "
		"generally convey better bonuses, but not all weapons are as good "
		"in all situations.  Specifics about the bonuses conferred by "
		"specific weapons can be found both in these rules (for most basic "
		"weapons), and in the descriptions of the weapons themselves. Troops "
		"which are fighting hand-to-hand without specific weapons are "
		"assumed to be irregularly armed with makeshift weapons such as "
		"clubs, pitchforks, torches, etc. ";
	f.Paragraph(temp);

	temp = " Possession of a mount, and the appropriate skill to use that "
		"mount will also confer a bonus to the effective Combat skill. The "
		"amount of the bonus will depend on the level of the appropriate "
		"skill and the mount in question.  Some mounts are better than "
		"others, and may provide better bonus, but may also require higher "
		"levels of skill to get any bonus at all.  Some terrain might not "
		"allow mounts to give a combat advantage.";
	f.Paragraph(temp);

	temp = "Certain weapons may provide different attack and defense "
		"bonuses, or have additional attack bonuses against mounted "
		"opponents or other special characteristics. These bonuses will "
		"be listed in the item descriptions in the turn reports.";
	f.Paragraph(temp);
	
	temp = "Some melee weapons may be defined as Long or Short (this is "
		"relative to a normal weapon, e.g. the sword). A soldier wielding "
		"a longer weapon than his opponent gets a +1 bonus to his attack "
		"skill.";
	f.Paragraph(temp);
	temp = "Ranged weapons are slightly different from melee weapons.  The "
		"target will generally not get any sort of combat bonus to defense "
		"against a ranged attack.";
	f.Paragraph(temp);
	temp = "Some weapons, including some ranged weapons, may only attack "
		"every other round, or even less frequently. When a weapon is not "
		"able to attack every round, this will be specified in the item "
		"description.";
	f.Paragraph(temp);
	temp = "Weapons may have one of several different attack types: "
		"Slashing, Piercing, Crushing, Cleaving and Armor Piercing.  "
		"Different types of armor may give different survival chances "
		"against a successful attack of different types.";
	f.Paragraph(temp);
	// Unit::GetWeapon: NOATTACKERSKILL (ranged) weapons attack with their own skill (e.g.
	// Longbow) plus the weapon bonus, and defend with the weapon's defense bonus only.
	temp = "A soldier using a ranged weapon attacks with his skill in that "
		"weapon (for example Longbow) rather than Combat, but defends as if "
		"he had a Combat skill of 0, even if he has an actual Combat skill. "
		"This is the trade off for being able to hit from the back line of "
		"fighting.";
	f.Paragraph(temp);
	temp = "Being inside a building confers a bonus to defense.  ";
	// Without ADVANCED_FORTS, Soldier::Soldier adds the object's defenceArray to the defence
	// skill for EVERY attack type (combat, energy, spirit, weather, riding, ranged), so the
	// bonus applies against magic too. ADVANCED_FORTS (Kingdoms only) uses protection[]
	// instead, so keep the original wording there.
	if (Globals->ADVANCED_FORTS) {
		temp += "This bonus is effective against ranged as well as melee "
			"weapons.  ";
	} else {
		temp += "The size of the bonus depends on the building and on the "
			"type of attack, and it applies against every kind of attack, "
			"including ranged weapons and magic; the description of each "
			"building shows its bonuses.  ";
	}
	temp += "The number of men that a building can protect is equal to its "
		"size. The size of the various common buildings was listed in the ";
	temp += f.Link("#tablebuildings", "Table of Buildings") + " earlier. ";
	// Ships protect through the fleet special case in Soldier::Soldier: the ship item's name
	// is looked up in ObjectDefs (LookupObject ignores the DISABLED flag), and the fleet
	// protects protect * number-of-that-ship men. Only mention it if an enabled ship does.
	{
		int protecting_ships = 0;
		for (int s = 0; s < NITEMS; s++) {
			if (ItemDefs[s].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[s].type & IT_SHIP)) continue;
			AString sname = ItemDefs[s].name;
			int sobj = LookupObject(&sname);
			if (sobj >= 0 && ObjectDefs[sobj].protect > 0) protecting_ships = 1;
		}
		if (protecting_ships) {
			temp += "Some ships also protect the men aboard in the same way. "
				"Each such ship protects its own number of men, so a fleet of "
				"several of them protects that many times as many; the "
				"description of each ship shows how many men it protects and "
				"the bonuses it gives. ";
		}
	}
	f.Paragraph(temp);
	temp = "If there are too many units in a building to all gain "
		"protection from it, then those units who have been in the building "
		"longest will gain protection.  (Note that these units appear first "
		"on the turn report.)";
	if (!(ObjectDefs[O_FORT].flags & ObjectType::DISABLED)) {
		temp += " If a unit of 200 men is inside a Fort (capacity ";
		temp += ObjectDefs[O_FORT].protect;
		temp += "), then the first ";
		temp += ObjectDefs[O_FORT].protect;
		temp += " men in the unit will gain the full bonus, and the other ";
		temp += (200 - ObjectDefs[O_FORT].protect);
		temp += " will gain no protection.";
	}
	f.Paragraph(temp);
	temp = "Units which have the Behind flag set are at the rear and "
		"cannot be attacked by any means until all non-Behind units have "
		"been wiped out.  On the other hand, neither can they attack with "
		"melee weapons, but only with ranged weapons or magic.  Once all "
		"front-line units have been wiped out, then the Behind flag no "
		"longer has any effect.  Only the men in a unit stay behind; "
		"monsters in a unit with the Behind flag fight in the front line.";
	f.Paragraph(temp);
	f.LinkRef("com_victory");
	f.TagText("h3", "Victory!");
	// Army::Broken: more than half the soldiers dead (strictly). Battle::Run: both broken in
	// the same round is a draw unless one side is wiped out; 100 rounds max, then a draw.
	temp = "Combat rounds continue until one side has lost more than half "
		"of its soldiers. The victorious side is then awarded one free round "
		"of attacks, after which the battle is over.  If both sides pass "
		"that point in the same round, the battle is a draw (unless one "
		"side has been wiped out), and neither side gets a free round. A "
		"battle that is still going on after 100 rounds also ends in a "
		"draw.";
	f.Paragraph(temp);
	/* XXX -- Here is where to put the ROUT information */
	if (!(SkillDefs[S_HEALING].flags & SkillType::DISABLED) &&
			!(ItemDefs[I_HERBS].flags & SkillType::DISABLED)) {
		// Army::DoHeal / DoHealLevel: HealDefs[level] = {attempts multiplier, % chance};
		// attempts per man = num * HEALS_PER_MAN. Best healers go first; healing potions
		// (level 6 entry) and Magical Healing (MagicHealDefs) also heal after battle.
		int attempts = HealDefs[1].num * Globals->HEALS_PER_MAN;
		int same = 1;
		for (int l = 2; l <= 5; l++)
			if (HealDefs[l].num != HealDefs[1].num) same = 0;
		temp = "Units with the Healing skill have a chance of being able "
			"to heal casualties of the winning side, so that they recover "
			"rather than dying.  ";
		if (same) {
			temp += AString("Each man with this skill can attempt to heal ") +
				attempts + " casualties, whatever his skill level";
		} else {
			temp += AString("Each man with this skill can attempt to heal ") +
				attempts + " casualties per skill level";
		}
		temp += AString("; a higher level improves the chance of success, "
			"which is ") + HealDefs[1].rate + "% at level 1";
		for (int l = 2; l <= 5; l++) {
			temp += (l == 5) ? AString(" and ") : AString(", ");
			temp += AString(HealDefs[l].rate) + "% at level " + l;
		}
		temp += ". Each attempt however requires one unit of Herbs, which "
			"is thereby used up. Only one attempt at Healing may be made per "
			"casualty, and the best healers try first. Healing occurs "
			"automatically, after the battle is over, by any living healers "
			"on the winning side.";
		if (!(ItemDefs[I_HEALPOTION].flags & ItemType::DISABLED)) {
			temp += AString(" Healing potions also heal casualties after a battle, "
				"with a ") + HealDefs[6].rate + "% chance each, without "
				"needing herbs.";
		}
		if (!(SkillDefs[S_MAGICAL_HEALING].flags & SkillType::DISABLED)) {
			temp += " Mages with Magical Healing also heal casualties after a "
				"battle, without herbs (see the skill description).";
		}
		f.Paragraph(temp);
	}
	// Battle::GetSpoils: each losing unit loses items in proportion to its dead; about half
	// of those (rounded at random) become spoils, the rest are destroyed; IT_ALWAYS_SPOIL /
	// IT_NEVER_SPOIL override; dead wild monsters add their own spoils; draws give none.
	temp = "Each unit on the losing side loses a share of its items equal "
		"to the share of its men who died. About half of these items are "
		"found and collected by the winning side, and the rest are "
		"destroyed (some items are always recovered, and some never are). "
		"Wild monsters that are killed also leave spoils of their own. A battle "
		"that ends in a draw gives no spoils. "
		"Each item which is recovered is picked up by one of the "
		"survivors able to carry it (see the ";
	temp += f.Link("#spoils", "SPOILS") + " command) at random, so the "
		"winners generally collect loot in proportion to their number of "
		"surviving men.";
	f.Paragraph(temp);
	temp = "If you are expecting to fight an enemy who is carrying so "
		"much equipment that you would not be able to move after picking "
		"it up, and you want to move to another region later that month, it "
		"may be worth issuing some orders to drop items (with the ";
	temp += f.Link("#give", "GIVE") + " 0 order) or to prevent yourself "
		"picking up certain types of spoils (with the ";
	temp += f.Link("#spoils", "SPOILS") + " order) in case you win the "
		"battle!";
	f.Paragraph(temp);
	// Mirrors Army::Win / Army::Tie / Soldier::Alive in army.cpp. The 5% is a literal in
	// Army::Win (not a GameDefs field), so it is stated literally here; if it ever becomes
	// configurable, interpolate it instead. loses_percent = 100 - floor(survivors * 100 / count)
	// is the loss percentage rounded UP, which is why "any loss above 4%" is the exact rule
	// (21 men losing 1 = 4.76% -> stopped; 25 men losing 1 = exactly 4% -> not stopped).
	temp = "If the winning side lost 5% or more of its soldiers in a "
		"battle, every unit on that side will not be allowed to move, or "
		"attack again, for the rest of the turn. The losses are counted "
		"across the whole side, not unit by unit, after any healing, and "
		"the percentage is rounded up, so in practice any loss above 4% "
		"counts. Each battle is judged on its own: losses from separate "
		"battles are not added together. If a battle ends indecisively, "
		"every surviving unit on both sides is stopped in the same way, "
		"regardless of losses. Surviving units on the losing side cannot "
		"attack again that turn, but they may still move. They are also "
		"taken off guard";
	// Soldier::Alive(LOSS): routed = 1, guard cleared unless the unit has an Amulet of
	// Invulnerability; Game::Do1Attack refuses routed targets with ONLY_ROUT_ONCE.
	if (!(ItemDefs[I_AMULETOFI].flags & ItemType::DISABLED)) {
		temp += " (unless they carry an amulet of invulnerability)";
	}
	if (Globals->ONLY_ROUT_ONCE) {
		temp += ", and they cannot be the target of another attack that turn, "
			"though they may still be drawn into other battles as defenders";
	}
	temp += ".";
	f.Paragraph(temp);
	if (has_stea || has_obse) {
		f.LinkRef("stealthobs");
		f.ClassTagText("div", "rule", "");
		temp = (has_stea ? "Stealth" : "");
		if (has_obse) {
			if (has_stea) temp += " and ";
			temp += "Observation";
		}
		f.TagText("h2", temp);
		if (has_stea && has_obse) {
			temp = "The Stealth skill is used to hide units, while the "
				"Observation skill is used to see units that would otherwise "
				"be hidden. A unit can be seen only if you have at least "
				"one unit in the same region, with an Observation skill at "
				"least as high as that unit's Stealth skill. If your "
				"Observation skill is equal to the unit's Stealth skill, "
				"you will see the unit, but not the name of the owning "
				"faction. If your Observation skill is higher than the "
				"unit's Stealth skill, you will also see the name of the "
				"faction that owns the unit.";
		} else if (has_stea) {
			temp = "The Stealth skill is used to hide units. A unit can be "
				"seen only if it doesn't know the Stealth skill and if you "
				"have at least one unit in the same region.";
		} else if (has_obse) {
			temp = "The Observation skill is used to see information about "
				"units that would otherwise be hidden.  If your unit knows "
				"the Observation skill, it will see the name of the faction "
				"that owns any unit in the same region.";
		}
		f.Paragraph(temp);
		if (has_stea && has_obse) {
			// Unit::GetAttribute("stealth"/"observation"): skill plus the best single bonus
			// from items or spells (bonuses do not stack); stealth uses the worst value in the
			// unit; being on guard forces stealth to 0. Faction::CanSee: Mind Reading 2+ shows
			// the faction of any unit it can see.
			temp = "These checks use a unit's effective Stealth and "
				"Observation: its skill level plus the bonus from the best "
				"single item or spell that helps it (bonuses from several "
				"items or spells do not add up). The descriptions of those "
				"items and spells give their bonuses.";
			if (!(SkillDefs[S_MIND_READING].flags & SkillType::DISABLED)) {
				temp += " A faction with a unit that knows Mind Reading at "
					"level 2 or higher sees the faction of every unit it can "
					"see.";
			}
			f.Paragraph(temp);
		}
		if (has_stea) {
			temp = "Regardless of Stealth skill, units are always visible "
				"when participating in combat; when guarding a region with "
				"the Guard flag; or when in a building or aboard a fleet.";
			if (has_obse) {
				temp += " However, in order to see the faction that owns "
					"the unit, you will still need a higher Observation "
					"skill than the unit's Stealth skill.";
				temp += " A unit on guard counts as having a Stealth of 0, so "
					"any Observation reveals its faction.";
			}
			temp += " A unit that has used ";
			temp += f.Link("#reveal", "REVEAL") + " is always visible.";
			f.Paragraph(temp);
			f.LinkRef("stealthobs_stealing");
			f.TagText("h3", "Stealing:");
			temp = AString("The ") + f.Link("#steal", "STEAL") +
				" order is a way to steal items from other player factions"
				" without a battle. The order can only be issued by a one-man"
				" unit. The order specifies a target unit; the thief will then"
				" attempt to steal the specified item from the target unit.";
			f.Paragraph(temp);
			if (has_obse) {
				temp = "If the thief has higher Stealth than any of the "
					"target faction's units have Observation (i.e. the "
					"thief cannot be seen by the target faction), the theft "
					"will succeed.";
				if (Globals->HARDER_ASSASSINATION){
					temp += " While stealing, the thief has a -1 penalty to his "
						"Stealth Skill.";
				}
			} else {
				temp = "The thief must know Stealth to attempt theft.";
			}
			// Do1Steal: the thief must see the target; a thief inside a building or fleet is
			// visible to everyone (Faction::CanSee) and so always caught; the guardsmen's
			// faction counts as a watcher too.
			temp += " The thief must be able to see the target unit. A thief "
				"inside a building or aboard a fleet is visible to everyone, so "
				"he will always be caught.";
			if (Globals->CITY_MONSTERS_EXIST)
				temp += " City guardsmen who can see the thief also stop the theft.";
			temp += " The target faction will be told what was stolen, but "
				"not by whom.  If the specified item is silver, then $200 "
				"or half the total available, whichever is less, will be "
				"stolen.  If it is any other item, then only one will be "
				"stolen (if available).";
			f.Paragraph(temp);
			if (has_obse) {
				temp = "Any unit with high enough Observation to see the "
					"thief will see the attempt to steal, whether the "
					"attempt is successful or not.  Allies of the target "
					"unit will prevent the theft, if they have high enough "
					"Observation to see the unit trying to steal.";
				f.Paragraph(temp);
			}
			f.LinkRef("stealthobs_assassination");
			f.TagText("h3", "Assassination:");
			temp = AString("The ") + f.Link("#assassinate", "ASSASSINATE") +
				" order is a way to kill another person without attacking "
				"and going through an entire battle. This order can only be "
				"issued by a one-man unit, and specifies a target unit.  If "
				"the target unit contains more than one person, then one "
				"will be singled out at random.";
			f.Paragraph(temp);
			if (has_obse) {
				temp = "Success for assassination is determined as for "
					"theft, i.e. the assassin will fail if any of the "
					"target faction's units can see him.  In this case, "
					"the assassin will flee, and the target faction will "
					"be informed which unit made the attempt.  As with "
					"theft, allies of the target unit will prevent the "
					"assassination from succeeding, if their Observation "
					"level is high enough.";

				if (Globals->HARDER_ASSASSINATION) {
					temp += " While attemping an assassination, the assassin "
						"has a ";
					if (Globals->IMPROVED_AMTS) {
						temp += "-1";
					} else {
						temp += "-2";
					}
					temp += " penalty to his Stealth Skill.";
				}

				f.Paragraph(temp);
				temp = "";
			} else {
				temp = "The assasin must know Stealh to attempt "
					"assassination.";
			}
			if (has_obse) {
				temp += "If the assassin has higher stealth than any of the "
					"target faction's units have Observation, then a "
					"one-on-one ";
			} else {
				temp += " A one-on-one ";
			}
			temp += "fight will take place between the assassin and the "
				"target character.  The assassin automatically gets a "
				"free round of attacks";
			if (Globals->MAX_ASSASSIN_FREE_ATTACKS) {
				temp += ", except he is limited to ";
				temp += Globals->MAX_ASSASSIN_FREE_ATTACKS;
				temp += " total during the free round";
			}
			temp += "; after that, the battle is handled like a normal "
				"fight, with the exception that neither assassin nor "
				"victim can use any armor";
			temp2 = "";
			last = -1;
			comma = 0;
			for (i = 0; i < NITEMS; i++) {
				if (!(ItemDefs[i].type & IT_ARMOR)) continue;
				if (!(ItemDefs[i].type & IT_NORMAL)) continue;
				if (ItemDefs[i].flags & ItemType::DISABLED) continue;
				ArmorType *at = FindArmor(ItemDefs[i].abr);
				if (at == NULL) continue;
				if (!(at->flags & ArmorType::USEINASSASSINATE)) continue;
				if (last == -1) {
					last = i;
					continue;
				}
				temp2 += ItemDefs[last].name;
				temp2 += ", ";
				last = i;
				comma++;
			}
			if (comma) temp2 += "or ";
			if (last != -1) {
				temp2 += ItemDefs[last].name;
				temp += " except ";
				temp += temp2;
			}
			temp += ".";
			if (last == -1)
				temp += " Armor ";
			else
				temp += " Most armor ";
			temp += "is forbidden for the assassin because it would "
				"make it too hard to sneak around, and for the victim "
				"because he was caught by surprise with his armor off. If "
				"the assassin wins, the target faction is told merely that "
				"the victim was assassinated, but not by whom.  If the "
				"victim wins, then the target faction learns which unit "
				"made the attempt.  (Of course, this does not necessarily "
				"mean that the assassin's faction is known.)  The winner of "
				"the fight takes spoils as usual: since only one man dies, "
				"only his share of the unit's items can be lost, and about "
				"half of that is recovered (see ";
			temp += f.Link("#com_victory", "Victory!") + ").";
			// Battle::GetSpoils skips Amulets of True Seeing for an assassin with a Ring
			// (ass == 2), so a Ring-wearing assassin cannot pick one up.
			if (!(ItemDefs[I_AMULETOFTS].flags & ItemType::DISABLED) &&
					!(ItemDefs[I_RINGOFI].flags & ItemType::DISABLED)) {
				temp += AString(" An assassin wearing a ") + ItemDefs[I_RINGOFI].name +
					" cannot take an " + ItemDefs[I_AMULETOFTS].name + ".";
			}
			// Assassination battles: both sides are put outside any structure, no tactics, no
			// helpers; the assassin must see the target; Game::RunBattle refuses a target whose
			// faction the assassin has declared Ally.
			temp += " In an assassination, neither side gets protection from "
				"buildings or ships, Tactics plays no part, and no other units "
				"join the fight. The assassin must be able to see the target, "
				"and you cannot assassinate a unit of a faction you have "
				"declared Ally.";
			f.Paragraph(temp);
			temp = f.Link("#steal", "STEAL") + " and " +
				f.Link("#assassinate", "ASSASSINATE") +
				" are not month long orders, and do not interfere with other "
				"activities, but a unit can only issue one " +
				f.Link("#steal", "STEAL") + " order or one " +
				f.Link("#assassinate", "ASSASSINATE") + " order in a month.";
			f.Paragraph(temp);
		}
	}
	f.LinkRef("magic");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Magic");
	temp = "A character enters the world of magic in Atlantis by beginning "
		"study on one of the Foundation magic skills.  Only one man units";
	if (!Globals->MAGE_NONLEADERS && Globals->LEADERS_EXIST)
		temp += ", with the man being a leader,";
	temp += " are permitted to study these skills. ";
	if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
		temp += "The number of these units (known as \"magicians\" or "
			"\"mages\") that a faction may own is ";
		if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT)
			temp += "limited.";
		else
			temp += "determined by the faction's type.";
		temp += " Any attempt to gain more, either through study, or by "
			"transfer from another faction, will fail.  In addition, mages ";
	} else {
		temp += "Mages ";
	}
	temp += "may not ";
	temp += f.Link("#give", "GIVE") + " men at all; once a unit becomes a "
		"mage (by studying one of the Foundations), the unit number is "
		"fixed. (The mage may be given to another faction using the ";
	temp += f.Link("#give", "GIVE") + " UNIT order.)";
	// GIVE refuses men to/from mages and apprentices; GetBuyAmount refuses their BUY of men;
	// DoStudyOrder refuses turning an apprentice into a mage and vice versa.
	temp += " Mages may not receive men or recruit them either.";
	if (app_exist) {
		temp += AString(" The same applies to ") + Globals->APPRENTICE_NAME +
			"s. An " + Globals->APPRENTICE_NAME + " can never become a mage, "
			"and a mage can never become an " + Globals->APPRENTICE_NAME + ".";
	}
	f.Paragraph(temp);
	f.LinkRef("magic_skills");
	f.TagText("h3", "Magic Skills:");
	temp = "Magic skills are the same as normal skills, with a few "
		"differences.  The basic magic skills, called Foundations, are ";
	last = -1;
	comma = 0;
	j = 0;
	for (i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].flags & SkillType::DISABLED) continue;
		if (!(SkillDefs[i].flags & SkillType::FOUNDATION)) continue;
		j++;
		if (last == -1) {
			last = i;
			continue;
		}
		temp += SkillDefs[last].name;
		temp += ", ";
		comma++;
		last = i;
	}
	if (comma) temp += "and ";
	temp += SkillDefs[last].name;
	temp += ". To become a mage, a unit undertakes study in one of these "
		"Foundations.  As a unit studies the Foundations, he will be able "
		"to study deeper into the magical arts; the additional skills that "
		"he may study will be indicated on your turn report.";
	f.Paragraph(temp);
	temp = "There are two major differences between Magic skills and most "
		"normal skills. The first is that the ability to study Magic "
		"skills sometimes depends on lower level Magic skills. "
		"Magic skills cannot be learnt to a higher level than "
		"the skills they depend upon. For example, if a Magic skill "
		"requires Spirit 2 to begin to study, then it can never be "
		"studied to a level higher than the mage's Spirit skill, so "
		"in order to increase that skill to level 3, his Spirit skill "
		"would first have to be increased to level 3. "
		"The Magic skills that a mage may study are listed on his "
		"turn report, so he knows which areas he may pursue. "
		"Studying higher in the Foundation skills, and certain other "
		"Magic skills, will make other skills available to the mage. "
		"Secondly, study into a magic skill above level 2 requires "
		"that the mage be located in some sort of building which can ";
	if (!Globals->LIMITED_MAGES_PER_BUILDING) {
		temp += "offer protection.  Trade structures do not count. ";
	} else {
		temp += "offer specific facilities for mages.  Certain types of "
			"buildings can offer shelter and support and a proper "
			"environment, some more so than others. ";
	}
	temp += "If the mage is not in such a structure, his study rate is cut "
			"in half, as he does not have the proper environment and "
			"equipment for research.";
	// Game::DoStudyOrder: halved outside a building, in an unfinished one, or when the
	// object's mage slots for the month are used up; slots go to mages in the order they are
	// processed (report order), and only mages studying magic at level 2+ use one.
	temp += " The building must be finished.";
	if (Globals->LIMITED_MAGES_PER_BUILDING) {
		temp += " Each building can only support a limited number of mages "
			"each month (see the table below); the mages listed first in the "
			"building get the places, and only mages studying a magic skill "
			"they already know at level 2 or more use one. A mage who does not "
			"get a place studies at half rate.";
	}
	f.Paragraph(temp);

	if (Globals->LIMITED_MAGES_PER_BUILDING) {
		temp = "It is possible that there are advanced buildings not listed "
			"here which also can support mages.  The description of a "
			"building will tell you for certain.  The common buildings and "
			"the mages a building of that type can support follows:";
		f.Paragraph(temp);
		f.LinkRef("tablemagebuildings");
		f.Enclose(1, "center");
		f.Enclose(1, "table border=\"1\"");
		f.Enclose(1, "tr");
		f.TagText("td", "");
		f.TagText("th", "Mages");
		f.Enclose(0, "tr");
		for (i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (!ObjectDefs[i].maxMages) continue;
			pS = FindSkill(ObjectDefs[i].skill);
			if (pS == NULL) continue;
			if (pS->flags & SkillType::MAGIC) continue;
			k = ObjectDefs[i].item;
			if (k == -1) continue;
			/* Need the >0 since item could be WOOD_OR_STONE (-2) */
			if (k > 0 && (ItemDefs[k].flags & ItemType::DISABLED)) continue;
			if (k > 0 && !(ItemDefs[k].type & IT_NORMAL)) continue;
			/* Okay, this is a valid object to build! */
			f.Enclose(1, "tr");
			f.Enclose(1, "td align=\"left\" nowrap");
			f.PutStr(ObjectDefs[i].name);
			f.Enclose(0, "td");
			f.Enclose(1, "td align=\"left\" nowrap");
			f.PutStr(ObjectDefs[i].maxMages);
			f.Enclose(0, "td");
			f.Enclose(0, "tr");    // was missing, leaving every row unclosed
		}
		f.Enclose(0, "table");
		f.Enclose(0, "center");
		// Object::FleetCapacity: a fleet supports the sum of maxMages of the ObjectDefs entry
		// named like each ship it holds.
		std::vector<std::string> ships;
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_SHIP)) continue;
			AString sname = ItemDefs[i].name;
			int ot = LookupObject(&sname);
			if (ot > 0 && ObjectDefs[ot].maxMages > 0) {
				ships.push_back(std::string(ItemDefs[i].name) + " " +
					std::to_string(ObjectDefs[ot].maxMages));
			}
		}
		if (!ships.empty()) {
			temp = "Fleets can also support mages: each ship adds its own "
				"places (";
			temp += joinList(ships).c_str();
			temp += ").";
			f.Paragraph(temp);
		}
	}

	f.LinkRef("magic_foundations");
	f.TagText("h3", "Foundations:");
	temp = "The ";
	temp += NumToWord(j);
	temp += " Foundation skills are called ";
	last = -1;
	comma = 0;
	for (i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].flags & SkillType::DISABLED) continue;
		if (!(SkillDefs[i].flags & SkillType::FOUNDATION)) continue;
		if (last == -1) {
			last = i;
			continue;
		}
		temp += SkillDefs[last].name;
		temp += ", ";
		comma++;
		last = i;
	}
	if (comma) temp += "and ";
	temp += SkillDefs[last].name;
	temp += ".";
	/* XXX -- This needs better handling! */
	/* Add each foundation here if it exists */
	if (!(SkillDefs[S_FORCE].flags & SkillType::DISABLED)) {
		temp += " Force indicates the quantity of magical energy that a "
			"mage is able to channel (a Force rating of 0 does not mean "
			"that the mage can channel no magical energy at all, but only "
			"a minimal amount).";
	}
	if (!(SkillDefs[S_PATTERN].flags & SkillType::DISABLED)) {
		temp += " Pattern indicates ability to handle complex patterns, and "
			"is important for things like healing and nature spells. ";
	}
	if (!(SkillDefs[S_SPIRIT].flags & SkillType::DISABLED)) {
		temp += " Spirit deals with meta-effects that lie outside the scope "
			"of the physical world.";
	}
	f.Paragraph(temp);

	f.LinkRef("magic_furtherstudy");
	f.TagText("h3", "Further Magic Study:");
	temp = "Once a mage has begun study of one or more Foundations, more "
		"skills that he may study will begin to show up on his report. "
		"These skills are the skills that give a mage his power.  As with "
		"normal skills, when a mage achieves a new level of a magic skill, "
		"he will be given a skill report, describing the new powers (if "
		"any) that the new skill confers.  The ";
	temp += f.Link("#show", "SHOW") + " order may be used to show this "
		"information on future reports.";
	f.Paragraph(temp);

	f.LinkRef("magic_usingmagic");
	f.TagText("h3", "Using Magic:");
	temp = "A mage may use his magical power in three different ways, "
		"depending on the type of spell he wants to use.  Some spells, "
		"once learned, take effect automatically and are considered "
		"always to be in use; these spells do not require any order to "
		"take effect.";
	f.Paragraph(temp);
	temp = "Secondly, some spells are for use in combat. A mage may specify "
		"that he wishes to use a spell in combat by issuing the ";
	// Soldier::SetupSpell: only unit->combat is used; it persists between turns. COMBAT with
	// no spell clears it. With a combat spell set, special-attack battle items are not used
	// (SetupCombatItems). Only mages (not apprentices) can set one.
	temp += f.Link("#combat", "COMBAT") + " order.  A combat spell "
		"specified in this way will only be used if the mage finds "
		"himself taking part in a battle. A mage can only use one combat "
		"spell (this includes shields), and the setting stays until it is "
		"changed; ";
	temp += f.Link("#combat", "COMBAT") + " with no spell clears it. A mage "
		"with a combat spell set does not use battle items that give a "
		"special attack.";
	f.Paragraph(temp);
	temp = "The third type of spell use is for spells that take an entire "
		"month to cast.  These spells are cast by the mage issuing the ";
	temp += f.Link("#cast", "CAST") + " order. Because " +
		f.Link("#cast", "CAST") + " takes an entire month, a mage may use "
		"only one of this type of spell each turn. Note, however, that a ";
	temp += f.Link("#cast", "CAST") + " order is not a month long order; "
		"a mage may still ";
	temp += f.Link("#move", "MOVE") + ", ";
	temp += f.Link("#study", "STUDY") + ", or use any other month long order. ";
	temp += "The justification for this (as well as being for game balance) "
		"is that a spell drains a mage of his magic power for the month, "
		"but does not actually take the entire month to cast.";
	// Game::RunACastOrder practises the spell; ARegion::NotifySpell tells other factions'
	// mages in the region who know the related lore.
	temp += " Casting a spell gives the mage practice in it. Other factions' "
		"mages in the region who know the related lore may notice that a "
		"spell has been cast.";
	f.Paragraph(temp);
	temp = "The description that a mage receives when he first learns a "
		"spell specifies the manner in which the spell is used (automatic, "
		"in combat, or by casting).";
	f.Paragraph(temp);
	f.LinkRef("magic_incombat");
	f.TagText("h3", "Magic in Combat:");
	temp = "NOTE: This section is rather vague, and quite advanced.  You "
		"may want to wait until you have figured out other parts of "
		"Atlantis before trying to understand exactly all of the rules in "
		"this section.";
	f.Paragraph(temp);
	temp = "Although the magic skills and spells are unspecified in these "
		"rules, left for the players to discover, the rules for combat "
		"spells' interaction are spelled out here.  There are six types "
		"of attacks, and defenses: Combat, Riding, Ranged, Energy, Weather, "
		"and Spirit.  Every attack and defense has a type, and only the "
		"appropriate defense is effective against an attack.  Shields (see "
		"below) only exist against Ranged, Energy, Weather and Spirit "
		"attacks; melee (Combat and Riding) attacks never meet a shield.";
	f.Paragraph(temp);
	// Battle::NormalRound: every shield caster adds a new shield at the start of every round,
	// so a removed shield is effectively recast the next round.
	temp = "Defensive spells are cast at the beginning of each round of "
		"combat, and will have a type of attack they deflect, and skill "
		"level (Defensive spells are generally called Shields).  Each mage "
		"with a shield spell casts it again every round.  Every "
		"time an attack is launched against an army, it must first attack "
		"the highest level Shield of the same type as the attack, before "
		"it may attack a soldier directly. Note that an attack only has "
		"to attack the highest Shield, any other Shields of the same "
		"type are ignored for that attack.";
	f.Paragraph(temp);
	temp = "An attack spell (and any other type of attack) also has an "
		"attack type, and attack level, and a number of blows it deals. "
		"When the attack spell is cast, it is matched up against the most "
		"powerful defensive spell of the appropriate type that the other "
		"army has cast.  If the other army has not cast any applicable "
		"defensive spells, the attack goes through unmolested.  Unlike "
		"normal combat however, men are at a disadvantage to defending "
		"against spells: everyone starts with an effective defense of -2 "
		"against magic. Being inside a building adds that building's "
		"bonus, and some items give protection too.  Some monsters "
		"have bonuses to resisting some attacks but are more susceptible "
		"to others. The skill level of the attack spell and the effective "
		"skill for defense are matched against each other.  The formula "
		"for determining the victor between a defensive and offensive "
		"spell is the same as for a contest of soldiers; if the levels "
		"are equal, there is a 1:1 chance of success, and so on.  If the "
		"offensive spell is victorious, the offensive spell deals its blows "
		"to the defending army.  Otherwise, the attack spell disperses, and "
		"the defending spell remains in place.  Spells that kill do not "
		"destroy the Shield they get through.";
	f.Paragraph(temp);
	temp = "Some spells do not actually kill enemies, but rather have some "
		"negative effect on them. These spells are treated the same as "
		"normal spells; if there is a Shield of the same type as them, "
		"they must attack the Shield before attacking the army, and if "
		"they get through, that Shield is destroyed (thus, it can be useful "
		"to have more than one Shield of the same type, as the next one "
		"takes its place). "
		"Ranged attacks that go through a defensive spell also must "
		"match their skill level against that of the defensive spell in "
		"question.  However, they do not destroy the defensive spell when "
		"they are successful.";
	f.Paragraph(temp);

	if (app_exist) {
		temp = (char) toupper(Globals->APPRENTICE_NAME[0]);
		temp += Globals->APPRENTICE_NAME + 1;
		f.LinkRef(AString("magic_") + Globals->APPRENTICE_NAME + "s");
		f.TagText("h3", temp + "s:");
		temp += "s may be created by having a unit study ";
		comma = 0;
		last = -1;
		for (i = 0; i < NSKILLS; i++) {
			if (SkillDefs[i].flags & SkillType::DISABLED) continue;
			if (!(SkillDefs[i].flags & SkillType::APPRENTICE)) continue;
			if (last == -1) {
				last = i;
				continue;
			}
			temp += SkillDefs[last].name;
			temp += ", ";
			comma++;
			last = i;
		}
		if (comma) temp += "or ";
		temp += SkillDefs[last].name;
		temp += ". ";
		temp += "Like Mages, only one man units";
		if (!Globals->MAGE_NONLEADERS && Globals->LEADERS_EXIST)
			temp += ", with the man being a leader,";
		temp += " may become ";
		temp += (char) toupper(Globals->APPRENTICE_NAME[0]);
		temp += Globals->APPRENTICE_NAME + 1;
		temp += "s. ";
		temp += (char) toupper(Globals->APPRENTICE_NAME[0]);
		temp += Globals->APPRENTICE_NAME + 1;
		// Apprentices can't study spells or set COMBAT, but Game::RunACastOrder lets them cast
		// a spell granted by an item they hold.
		temp += "s cannot study spells or use combat spells, but they may use "
			"items which otherwise only mages can use, including casting a "
			"spell that an item gives them.";
		f.Paragraph(temp);
	}

	f.LinkRef("nonplayers");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Non-Player Units");
	temp = "There are a number of units that are not controlled by players "
		"that may be encountered in Atlantis.  Most information about "
		"these units must be discovered in the course of the game, but a "
		"few basics are below.";
	f.Paragraph(temp);
	if (Globals->TOWNS_EXIST && Globals->CITY_MONSTERS_EXIST) {
		f.LinkRef("nonplayers_guards");
		f.TagText("h3", "City and Town Guardsmen:");
		temp = "All settlements, including villages, begin with guardsmen in "
			"them.  These "
			"units will defend any units that are attacked in the city or "
			"town, and will also prevent theft and assassination attempts, ";
		if (has_obse)
			temp += "if their Observation level is high enough. ";
		else
			temp += "if they can see the criminal. ";
		// Game::DoGuard1Orders refuses player guards while guardsmen are present; the
		// guardsmen faction is Neutral (so Unit::Forbids never stops anyone) and its units
		// have Hold set (they never join battles in neighbouring regions).
		temp += "They are on guard, and will prevent other units from "
			"taxing or pillaging, and while they are on guard, player units "
			"cannot go on guard in that region. They do not stop units from "
			"entering the region, and they only fight in their own region. ";
		if (Globals->START_CITIES_EXIST && Globals->SAFE_START_CITIES)
			temp += "Except in the starting cities, the ";
		else
			temp += "The ";
		temp += "guards may be killed by players, although they will form "
			"again if the city is left unguarded.";
		// Game::CreateCityMon: CITY_GUARD * (TownType()+1) armed leaders with Combat at that
		// level; AdjustCityMon regrows them by a tenth of full strength per month;
		// AdjustCityMons reforms them (GUARD_REGEN % per month, at 10% strength) only when no
		// unit at all is on guard in the region.
		// The counts hold for leader guards; without leaders CreateCityMon builds two units of
		// 3/4 strength each, so the numbers are only stated when LEADERS_EXIST.
		if (Globals->LEADERS_EXIST) {
			temp += AString(" A village has ") + Globals->CITY_GUARD + " guardsmen, a "
				"town " + (2 * Globals->CITY_GUARD) + " and a city " +
				(3 * Globals->CITY_GUARD) + ", armed and with a Combat skill of 1, 2 "
				"or 3 respectively.";
		}
		temp += AString(" Guardsmen who survive a fight regain a tenth of ") +
			"their full strength each month. If all of them are killed, each "
			"month there is a " + Globals->GUARD_REGEN + "% chance that they form "
			"again, at a tenth of their full strength, but only if no unit is on "
			"guard in the region.";
		f.Paragraph(temp);
		if (Globals->START_CITIES_EXIST &&
				(Globals->SAFE_START_CITIES ||
				Globals->START_CITY_GUARDS_PLATE ||
				Globals->START_CITY_MAGES)) {
			if (Globals->SAFE_START_CITIES || Globals->START_CITY_GUARDS_PLATE) {
				temp = "Note that the city guardsmen in the starting cities "
					"of Atlantis possess ";
				if (Globals->SAFE_START_CITIES)
					temp += "Amulets of Invincibility ";
				if (Globals->START_CITY_GUARDS_PLATE) {
					if (Globals->SAFE_START_CITIES) temp += "and ";
					temp += "plate armor ";
				}
				temp += "in addition to being more numerous and ";
				if (Globals->SAFE_START_CITIES)
					temp += "may not be defeated.";
				else
					temp += "are therefore harder to kill.";
			}

			if (Globals->START_CITY_MAGES) {
				if (Globals->AMT_START_CITY_GUARDS)
					temp += " Additionally, in ";
				else
					temp += "In ";
				temp += "the starting cities, Mage Guards will be found. "
					"These mages are adept at the fire spell";
				if (!Globals->SAFE_START_CITIES) {
					temp += " making any attempt to control a starting "
						"city a much harder proposition";
				}
				temp += ".";
			}
			f.Paragraph(temp);
		}
	}
	if (Globals->WANDERING_MONSTERS_EXIST) {
		f.LinkRef("nonplayers_monsters");
		f.TagText("h3", "Wandering Monsters:");
		temp = "There are a number of monsters who wander free throughout "
			"Atlantis.  They will occasionally attack player units, so be "
			"careful when wandering through the wilderness.";
		// Game::CheckWMonAttack: once a month, chance = aggression / max(100, 300 - N), N = the
		// player men the monster can see and catch; the target is picked weighted by men.
		temp += " Each month, a monster may attack once. The chance depends on "
			"its aggression (shown in its description) and on how many men it "
			"can see and catch in the region: it is the aggression divided by "
			"300 minus the number of those men, but never divided by less than "
			"100. So even the most aggressive monster attacks a lone man only "
			"about a third of the time, but is sure to attack when 200 or more "
			"men are around. Its target is chosen at random, so units with "
			"more men are more likely to be picked. Only player units are "
			"attacked, and units the monster cannot see or catch are safe.";
		f.Paragraph(temp);

		temp = "Some monsters live in lairs, caves, and other structures players cannot enter. "
			"Such monsters will never leave their habitat and will never wander around, "
			"but they can attack player units present in the region. The willingness to attack is "
			"dependent on the monster's aggression level. It is worth reminding that monsters "
			"inside the lair will always be visible to the player regardless of their stealth score "
			"as any other unit in the structure. ";
		// Game::GrowLMons: an empty lair in an unguarded region respawns with LAIR_FREQUENCY %.
		temp += AString("Each month, an empty lair has a ") + Globals->LAIR_FREQUENCY +
			"% chance of spawning new monsters if the old ones are killed. Players can guard regions with lairs, and monsters will not spawn there.";
		f.Paragraph(temp);

		temp = "Other monsters do not live in lairs but wander freely. Wandering monsters can spawn in any "
			"unguarded region regardless of whether there is a lair. Guarding will prevent monsters from spawning "
			"in a particular region. Their willingness to attack depends on their aggression level, and monsters "
			"can advance to neighboring regions while moving. Some monsters could have preferred terrains that they "
			"like more than others, and then they will be willing to enter such regions more likely than others. At "
			"the same time, some terrains could be so uncomfortable that monsters will never enter them. If a monster "
			"has particular terrain preferences, he will try to be close to the habitat he likes, and he will not go "
			"deeper into the territory that is not connected with his preferred terrain.";
		f.Paragraph(temp);

		// npc.cpp: no spawning in guarded regions; monthorders.cpp: wandering monsters never
		// move into a guarded town; runorders.cpp CheckWMonAttack has no guard check; the
		// Creatures faction is Neutral by default, so Unit::Forbids only stops a monster if
		// the guard's faction has declared Creatures Unfriendly/Hostile (and sees/catches it).
		temp = "A small tip to the players about guarding: guarding a region stops new monsters "
			"from appearing there, and wandering monsters will not move into a town that is guarded. "
			"It does not stop monsters that are already in the region, including monsters in lairs, "
			"from attacking. Elsewhere, your guards only stop a wandering monster from entering if "
			"your faction has declared the Creatures faction Unfriendly or Hostile, and can see and "
			"catch the monster. Remember also that monsters that never appear give no loot. Monster "
			"hunting is a desirable activity because it is fun, and you can get a great reward like "
			"silver, magical items, weapons, etc.";
		f.Paragraph(temp);

		// Monster combat participation, derived from the ordinary muster rules (Game::GetSides /
		// GetAFacs in battle.cpp) plus two facts about monsters: every wandering and lair
		// monster belongs to the one monster faction (Game::MakeWMon uses monfaction), and
		// Unit::SetMonFlags gives every monster unit guard = GUARD_AVOID and FLAG_HOLDING.
		// - Attacking: GetAFacs/GetSides only add an avoiding unit if it is the attacker itself,
		//   so other monsters in the region stay out. Each monster rolls its own attack
		//   (Game::CheckWMonAttack), so two attacks are two separate battles.
		// - Attacked: an avoiding unit of the target's faction joins the defence only if the
		//   attackers can identify it (Faction::CanSee == 2, i.e. observation > stealth) and
		//   catch it (Faction::CanCatch) -- Game::CanAttack.
		// - Adjacent regions: units outside the battle region only join if FLAG_HOLDING is off,
		//   which it never is for monsters.
		temp = "All wandering monsters, including those living in lairs, belong "
			"to a single monster faction, and every monster unit has Avoid "
			"Combat and Hold set. Under the normal rules for ";
		temp += f.Link("#com_muster", "who joins a battle") + ", this means:";
		f.Paragraph(temp);
		f.Enclose(1, "ul");
		f.TagText("li", "When a monster attacks, it fights alone. Other monster "
			"units in the same region do not join in. Each monster decides "
			"separately whether to attack, so if several monsters in a region "
			"attack in the same month, each attack is a separate battle.");
		temp = "When you attack a monster, other monster units in the same "
			"region join its defense only if you could have attacked them "
			"directly as well: your faction must be able to ";
		if (has_obse) {
			temp += "identify them (having a unit whose Observation is higher "
				"than their Stealth)";
		} else {
			temp += "identify them";
		}
		temp += " and to catch them (see ";
		temp += f.Link("#com_attacking", "attacking") + "). Monsters in a lair "
			"can always be caught, but still have to be identified.";
		f.TagText("li", temp);
		f.TagText("li", "Monsters never join a battle in a neighboring region, "
			"whether another monster is attacking there or being attacked.");
		f.Enclose(0, "ul");

		f.TagText("h4", "Monster movement probability table");
		// Mirrors the wandering-monster branch of Unit::DefaultOrders (unit.cpp): the move is
		// drawn uniformly from a list with 4 "stay" entries, 2 entries per enterable neighbor of
		// preferred terrain (every enterable terrain counts as preferred if the monster has no
		// preferences), and 1 per neighbor of neutral terrain that itself borders preferred
		// terrain. Disliked (forbidden) terrain and terrain the monster cannot cross get none.
		temp = "Each month, a wandering monster that is not in a lair either "
			"stays where it is or moves to one neighboring region. Only "
			"neighboring regions the monster is able to enter count: it never "
			"enters terrain it dislikes or terrain it cannot cross (such as "
			"water, for a monster that cannot swim), and it only enters a "
			"region of neutral terrain if that region borders terrain the "
			"monster prefers. A monster without terrain preferences treats "
			"every region it can enter as preferred. A particular preferred "
			"region is twice as likely to be chosen as a particular neutral "
			"one, and staying where it is counts as much as two preferred "
			"regions. A monster's description shows the terrains it prefers "
			"and dislikes.";
		f.Paragraph(temp);
		temp = "The table shows the resulting chances. Directions is the number "
			"of neighboring regions the monster can move into; Preferred and "
			"Neutral are how many of those are of preferred and of neutral "
			"terrain. Move Preferred and Move Neutral are the chances that the "
			"monster moves into one of the preferred or one of the neutral "
			"regions, shared equally between them; Stay is the chance that it "
			"stays where it is. For example, a monster with 3 possible "
			"directions, 2 of them preferred and 1 neutral, moves into one of "
			"the preferred regions 44% of the time (22% each), into the neutral "
			"region 11% of the time, and stays 44% of the time. Percentages are "
			"rounded down, so a row may not add up to exactly 100%.";
		f.Paragraph(temp);
		f.Enclose(1, "table border=\"1\"");

		f.Enclose(1, "thead");
			f.Enclose(1, "tr");
				f.TagText("th", "Directions");
				f.TagText("th", "Preferred");
				f.TagText("th", "Neutral");
				f.TagText("th", "Move Preferred");
				f.TagText("th", "Move Neutral");
				f.TagText("th", "Stay");
			f.Enclose(0, "tr");
		f.Enclose(0, "thead");

		int matrix[3][2];
		matrix[2][0] = 4;	// stay

		f.Enclose(1, "tbody");
		for (int dirs = 1; dirs <= 6; dirs++) {
			for (int preferedDirs = dirs; preferedDirs >= 0; preferedDirs--) {
				const int neutralDirs = dirs - preferedDirs;

				matrix[0][0] = preferedDirs * 2;	// prefered
				matrix[1][0] = neutralDirs;			// neutral

				int totalCases = 0;
				for (int i = 0; i < 3; i++) {
					totalCases += matrix[i][0];
				}

				for (int i = 0; i < 3; i++) {
					matrix[i][1] = matrix[i][0] * 100 / totalCases;
				}

				f.Enclose(1, "tr");
					f.TagText("td", AString("") + dirs);
					f.TagText("td", AString("") + preferedDirs);
					f.TagText("td", AString("") + neutralDirs);


					for (int i = 0; i < 3; i++) {
						f.TagText("td", AString("") + matrix[i][1] + "%");
					}
				f.Enclose(0, "tr");
			}
		}
		f.Enclose(0, "tbody");
		f.Enclose(0, "table");
	}
	f.LinkRef("nonplayers_controlled");
	f.TagText("h3", "Controlled Monsters:");
	temp = "Through various magical methods, you may gain control of "
		"certain types of monsters. These monsters are just another item "
		"in a unit's inventory, with a few special rules. Monsters will "
		"be able to carry things at their speed of movement; use the ";
	temp += f.Link("#show", "SHOW") + " ITEM order to determine the "
		"carrying capacity and movement speed of a monster. Monsters will "
		"also fight for the controlling unit in combat; the monster's "
		"description (shown with SHOW ITEM) gives its combat strength, "
		"such as its skills, attacks and hit points. Also, note that a monster will "
		"always fight from the front rank, even if the controlling unit "
		"has the behind flag set. Whether or not you are allowed to give a "
		"monster to other units depends on the type of monster; some may be "
		"given freely, while others must remain with the controlling unit.";
	if (Globals->RELEASE_MONSTERS) {
		temp += " All monsters may be released completely by using the ";
		temp += f.Link("#give", "GIVE") + " order targetting unit 0.  When "
			"this is done, the monster will become a wandering monster.";
	} else {
		// RELEASE_MONSTERS off: GIVE 0 of a monster simply destroys it.
		temp += " Giving a monster to unit 0 with the ";
		temp += f.Link("#give", "GIVE") + " order gets rid of it; it does not "
			"become a wandering monster.";
	}
	f.Paragraph(temp);
	// Game::MonsterCheck, run mid-turn (CHECK_MONSTER_CONTROL_MID_TURN) or at the end of the
	// turn: per item type, LOSS_CHANCE decays a share each month, HAS_SKILL frees them all if
	// the controller lacks the skill level, ESC_* gives an escape chance that grows with the
	// number held and shrinks with the skill level; LOSE_LINKED frees all linked types at once.
	// Escaped monsters become wild monsters in the region. Which rule a monster follows is in
	// its own and its summoning skill's description, so only the general rules are given here.
	temp = "Controlled monsters are checked every month, ";
	temp += Globals->CHECK_MONSTER_CONTROL_MID_TURN ?
		"in the middle of the turn (before movement). " :
		"at the end of the turn. ";
	temp += "Depending on the type of monster, they may break free if the "
		"controlling unit no longer has the skill needed to control them, "
		"slowly fade away a few at a time, or have a chance of escaping that "
		"grows with the number of them the unit controls and shrinks with its "
		"skill. When some monsters escape, related types of monsters held by "
		"the same unit may escape with them. Monsters that break free become "
		"wild monsters in the region. The description of each monster, and of "
		"the skill used to control it, tells you which of these applies.";
	f.Paragraph(temp);
	f.LinkRef("orders");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Orders");
	temp = "To enter orders for Atlantis, you should send a mail message "
		"to the Atlantis server, containing the following:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "pre");
	f.ClearWrapTab();
	f.WrapStr("#ATLANTIS faction-no <password>");
	f.PutNoFormat("");
	f.WrapStr("UNIT unit-no");
	f.WrapStr("...orders...");
	f.PutNoFormat("");
	f.WrapStr("UNIT unit-no");
	f.WrapStr("...orders...");
	f.PutNoFormat("");
	f.WrapStr("#END");
	f.Enclose(0, "pre");
	temp = "For example, if your faction number (shown at the top of your "
		"report) is 27, your password is \"foobar\", and you have two "
		"units numbered 5 and 17:";
	f.Paragraph(temp);
	f.Paragraph("");
	f.Enclose(1, "pre");
	f.WrapStr("#ATLANTIS 27 \"foobar\"");
	f.PutNoFormat("");
	f.WrapStr("UNIT 5");
	f.WrapStr("...orders...");
	f.PutNoFormat("");
	f.WrapStr("UNIT 17");
	f.WrapStr("...orders...");
	f.PutNoFormat("");
	f.WrapStr("#END");
	f.Enclose(0, "pre");
	temp = "Thus, orders for each unit are given separately, and indicated "
		"with the UNIT keyword.  (In the case of an order, such as the "
		"command to rename your faction, that is not really for any "
		"particular unit, it does not matter which unit issues the command; "
		"but some particular unit must still issue it.)";
	f.Paragraph(temp);
	temp = "IMPORTANT: You MUST use the correct #ATLANTIS line or else your "
		"orders will be ignored.";
	f.Paragraph(temp);
	temp = "If you have a password set, you must specify it on your "
		"#atlantis line, or the game will reject your orders.  See the ";
	temp += f.Link("#password", "PASSWORD") + " order for more details.";
	f.Paragraph(temp);
	temp = "Each type of order is designated by giving a keyword as the "
		"first non-blank item on a line.  Parameters are given after this, "
		"separated by spaces or tabs. Blank lines are permitted, as are "
		"comments; anything after a semicolon is treated as a comment, "
		"even if the semicolon is in the middle of a word, unless it is "
		"inside double quotes.";
	f.Paragraph(temp);
	temp = "The parser is not case sensitive, so all commands may be given "
		"in upper case, lower case or a mixture of the two.  However, when "
		"supplying names containing spaces, the name must be surrounded "
		"by double quotes, or else underscore characters must be used in "
		"place of spaces in the name.  Underscores only work for the names "
		"of things in the game, such as items, skills and structures; text "
		"that you make up yourself, such as the name or description of a "
		"unit, must be put in double quotes.  (These things apply to the "
		"#ATLANTIS and #END lines as well as to order lines.)";
	f.Paragraph(temp);
	temp = "You may precede orders with the at sign (@), in which case they "
		"will appear in the Template at the bottom of your report.  This is "
		"useful for orders which your units repeat for several months in a "
		"row.";
	f.Paragraph(temp);
	// Orders are processed in two ways: some (CLAIM, DECLARE, FACTION, FORM, TURN and the
	// flag orders) are applied by the parser while the orders file is read; the rest are
	// stored and run in the phases of Game::RunOrders (see Sequence of Events).
	temp = "Most orders are carried out at a fixed point in the turn, as "
		"described in the ";
	temp += f.Link("#sequenceofevents", "Sequence of Events") + ". Each unit "
		"may also have one month long order (see ";
	temp += f.Link("#playing_turns", "Turns") + "). A few orders, such as ";
	temp += f.Link("#claim", "CLAIM") + ", " + f.Link("#declare", "DECLARE") +
		", " + f.Link("#faction", "FACTION") + ", " + f.Link("#form", "FORM") +
		" and orders that set a unit's flags, such as ";
	temp += f.Link("#behind", "BEHIND") + " or " + f.Link("#hold", "HOLD") +
		", take effect as soon as your orders are read, before anything else "
		"happens in the turn.";
	f.Paragraph(temp);
	f.LinkRef("orders_abbreviations");
	f.TagText("h3", "Abbreviations:");
	temp = "All common items and skills have abbreviations that can be used "
		"when giving orders, for brevity.  Any time you see the item on your "
		"report, it will be followed by the abbreviation.  Please be careful "
		"using these, as they can easily be confused.";
	f.Paragraph(temp);
	f.LinkRef("ordersummary");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Order Summary");
	temp = "To specify a [unit], use the unit number.  If specifying a "
		"unit that will be created this turn, use the form \"NEW #\" if "
		"the unit belongs to your faction, or \"FACTION # NEW #\" if the "
		"unit belongs to a different faction.  See the ";
	temp += f.Link("#form", "FORM");
	temp += " order for a more complete description.  [faction] means that "
		"a faction number is required; [object] means that an object "
		"number (generally the number of a building or fleet) is required. "
		"[item] means an item (like wood or longbow) that a unit can have "
		"in its possession. [flag] is an argument taken by several orders, "
		"that sets or unsets a flag for a unit. A [flag] value must be "
		"either 1 (set the flag) or 0 (unset the flag); TRUE and FALSE, "
		"YES and NO, and ON and OFF can be used as well.  Other parameters "
		"are generally numbers or names.";
	f.Paragraph(temp);
	temp = "IMPORTANT: Remember that names containing spaces (e.g., "
		"\"Plate Armor\"), must be surrounded by double quotes, or the "
		"spaces must be replaced with underscores \"_\" (e.g., Plate_Armor).";
	f.Paragraph(temp);
	temp = "Also remember that anything used in an example is just that, "
		"an example and makes no guarantee that such an item, structure, "
		"or skill actually exists within the game.";
	f.Paragraph(temp);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("address");
	f.TagText("h4", "ADDRESS [new address]");
	f.Paragraph("Change the email address to which your reports are sent.");
	f.Paragraph("Example:");
	temp = "Change your faction's email address to atlantis@rahul.net.";
	temp2 = "ADDRESS atlantis@rahul.net";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("advance");
	f.TagText("h4", "ADVANCE [dir] ...");
	temp = "This is the same as the ";
	temp += f.Link("#move", "MOVE");
	temp += " order, except that it implies attacks on units which attempt "
		"to forbid access.  See the ";
	temp += f.Link("#move", "MOVE") + " order for details.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Move north, then northwest, attacking any units that forbid "
		"access to the regions.";
	temp2 = "ADVANCE N NW";
	f.CommandExample(temp, temp2);
	temp = "In order, move north, then enter structure number 1, move "
		"through an inner route, and finally move southeast. Will attack "
		"any units that forbid access to any of these locations.";
	temp2 = "ADVANCE N 1 IN SE";
	f.CommandExample(temp, temp2);

	if (Globals->USE_WEAPON_ARMOR_COMMAND) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("armor");
		f.TagText("h4", "ARMOR [item1] [item2] [item3] [item4]");
		f.TagText("h4", "ARMOR");
		temp = "This command allows you to set a list of preferred armor "
			"for a unit.  After searching for armor on the preferred "
			"list, the standard armor precedence takes effect if an armor "
			"hasn't been set.  The second form clears the preferred armor "
			"list.";
		f.Paragraph(temp);
		f.Paragraph("Examples");
		temp = "Set the unit to select chain armor before plate armor.";
		temp2 = "ARMOR CARM PARM";
		f.CommandExample(temp, temp2);
		temp = "Clear the preferred armor list.";
		temp2 = "ARMOR";
		f.CommandExample(temp, temp2);
	}

	if (has_stea) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("assassinate");
		f.TagText("h4", "ASSASSINATE [unit]");
		temp = "Attempt to assassinate the specified unit, or one of the "
			"unit's people if the unit contains more than one person.  The "
			"order may only be issued by a one-man unit.";
		f.Paragraph(temp);
		temp = "A unit may only attempt to assassinate a unit which is able "
			"to be seen.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Assassinate unit number 177.";
		temp2 = "ASSASSINATE 177";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("attack");
	f.TagText("h4", "ATTACK [unit] ... ");
	temp = "Attack a target unit.  If multiple ATTACK orders are given, "
		"all of the targets will be attacked.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "To attack units 17, 431, and 985:";
	temp2 = "ATTACK 17\nATTACK 431 985";
	f.CommandExample(temp, temp2);
	temp = "or:";
	temp2 = "ATTACK 17 431 985";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("autotax");
	f.TagText("h4", "AUTOTAX [flag]");
	if (Globals->TAX_PILLAGE_MONTH_LONG) {
		// Game::DefaultWorkOrder only applies AUTOTAX to units without a month long order
		// (never in the Nexus); RunTaxOrders clears the flag if the unit cannot tax.
		temp = "AUTOTAX 1 causes the unit to ";
		temp += f.Link("#tax", "TAX") + " in every month in which it has "
			"not been given another month long order, instead of working, "
			"until the flag is unset. If the unit turns out not to be able to "
			"tax, the flag is removed. AUTOTAX 0 unsets the flag.";
	} else {
		temp = "AUTOTAX 1 causes the unit to attempt to tax every turn "
			"(without requiring the TAX order) until the flag is unset. "
			"AUTOTAX 0 unsets the flag.";
	}
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "To cause the unit to attempt to tax every turn.";
	temp2 = "AUTOTAX 1";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("avoid");
	f.TagText("h4", "AVOID [flag]");
	// Battle muster (GetAFacs/GetDFacs): an avoiding unit only fights if it attacks, is the
	// target, or belongs to the target faction in that hex and the attackers can see and
	// catch it. It never joins to help allies or from an adjacent hex.
	temp = "AVOID 1 instructs the unit to avoid combat wherever possible. "
		"The unit will not enter combat unless it issues an ATTACK order, "
		"it is itself attacked, or the unit's faction is attacked in the "
		"unit's hex and the attackers can see and catch it. An avoiding unit "
		"never joins a battle to help an ally. AVOID 0 cancels this.";
	f.Paragraph(temp);
	temp = "The Guard and Avoid Combat flags are mutually exclusive; "
		"setting one automatically cancels the other.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set the unit to avoid combat when possible.";
	temp2 = "AVOID 1";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("behind");
	f.TagText("h4", "BEHIND [flag]");
	temp = "BEHIND 1 sets the unit to be behind other units in combat.  "
		"BEHIND 0 cancels this.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set the unit to be in front in combat.";
	temp2 = "BEHIND 0";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("build");
	f.TagText("h4", "BUILD");
	f.TagText("h4", "BUILD [object type]");
	f.TagText("h4", "BUILD HELP [unit]");
	temp = "BUILD given with no parameters causes the unit to perform "
		"work on ";
	if (may_sail)
		temp += "an unfinished ship it possesses, or on ";
	temp += "the object that it is currently inside.  BUILD given with an "
		"[object type] (such as \"Tower\" or \"Galleon\") instructs "
		"the unit to begin work on a new object of the type given. ";
	temp += "The final form instructs the unit to assist the target unit "
		"in its current building task, even if that task was begun "
		"this same turn. This help will be rejected if the unit you "
		"are trying to help does not consider you to be friendly.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "To build a new tower.";
	temp2 = "BUILD Tower";
	f.CommandExample(temp, temp2);
	temp = "To help unit 5789 build a structure.";
	temp2 = "BUILD HELP 5789";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("buy");
	f.TagText("h4", "BUY [quantity] [item]");
	f.TagText("h4", "BUY ALL [item]");
	temp = "Attempt to buy a number of the given item from a city or town "
		"marketplace, or to buy new people in any region where people are "
		"available for recruiting.  If the unit can't afford as many as "
		"[quantity], it will attempt to buy as many as it can. If the "
		"demand for the item (from all units in the region) is greater "
		"than the number available, the available items will be split "
		"among the buyers in proportion to the amount each buyer attempted "
		"to buy. ";
	if (Globals->RACES_EXIST) {
		temp += "When buying people, specify the race of the people "
			"as the [item], or you may use PEASANT or PEASANTS "
			"to recruit whichever race is present in the region. ";
	}
	temp += "If the second form is specified, the unit will attempt to buy "
		"as many as it can afford.";
	temp += " Not every unit may recruit people; see ";
	temp += f.Link("#economy_recruiting", "Recruiting") + ".";
	f.Paragraph(temp);
	f.Paragraph(AString("Example") + (Globals->RACES_EXIST?"s":"") + ":");
	temp = "Buy one plate armor from the city market.";
	temp2 = "BUY 1 \"Plate Armor\"";
	f.CommandExample(temp, temp2);
	if (Globals->RACES_EXIST) {
		// Use an enabled race (barbarians are disabled in NewOrigins).
		temp = AString("Recruit 5 ") + ItemDefs[manidx].names + " into the "
			"current unit. (This will dilute the skills that the unit has.)";
		temp2 = AString("BUY 5 ") + ItemDefs[manidx].abr;    // names may contain spaces
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("cast");
	f.TagText("h4", "CAST [skill] [arguments]");
	// Each spell is its own skill; the tokens after the name are its arguments (there is no
	// "cast at level N"). A unit has one cast order slot, so a later CAST replaces an earlier.
	temp = "Cast the given spell.  Note that most spell names contain "
		"spaces; be sure to enclose the name in quotes!  [arguments] "
		"depends on which spell you are casting; when you are able to cast "
		"a spell, the skill description will tell you the syntax.  Each "
		"spell is a skill of its own, and is always cast at the mage's "
		"level in it.  A unit can only cast one spell per turn; if it is "
		"given more than one CAST order, only the last one counts.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Cast the spell called \"Super Spell\".";
	temp2 = "CAST \"Super Spell\"";
	f.CommandExample(temp, temp2);
	if (!(SkillDefs[S_FARSIGHT].flags & SkillType::DISABLED)) {
		temp = "Cast Farsight to view region (12,8).";
		temp2 = "CAST Farsight REGION 12 8";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("claim");
	f.TagText("h4", "CLAIM [amount]");
	temp = "Claim an amount of the faction's unclaimed silver, and give it "
		"to the unit issuing the order.  The claiming unit may then spend "
		"the silver or give it to another unit.  CLAIM is carried out while "
		"your orders are read, so the silver can be used by any of the "
		"unit's orders that turn.  If you claim more than you have, the "
		"unit gets what is left.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Claim 100 silver.";
	temp2 = "CLAIM 100";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("combat");
	f.TagText("h4", "COMBAT [spell]");
	// ParseOrders COMBAT: no argument clears it; only U_MAGE; clears readyItem, and PREPARE
	// clears combat (PREPARE_NORMAL).
	temp = "Set the given spell as the spell that the unit will cast in "
		"combat.  This order may only be given if the unit is a mage and "
		"can cast the spell in question";
	if (app_exist) temp += AString(" (") + Globals->APPRENTICE_NAME + "s cannot)";
	temp += ".  COMBAT with no spell clears the combat spell.";
	if (Globals->USE_PREPARE_COMMAND != GameDefs::PREPARE_NONE) {
		temp += AString(" Setting a combat spell clears any item set with ") +
			f.Link("#prepare", "PREPARE") + ", and the other way round.";
	}
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Instruct the unit to use the spell \"Super Spell\", when the "
		"unit is involved in a battle.";
	temp2 = "COMBAT \"Super Spell\"";
	f.CommandExample(temp, temp2);

	if (Globals->FOOD_ITEMS_EXIST) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("consume");
		f.TagText("h4", "CONSUME UNIT");
		f.TagText("h4", "CONSUME FACTION");
		f.TagText("h4", "CONSUME");
		temp = "The CONSUME order instructs the unit to use food items in "
			"preference to silver for maintenance costs. CONSUME UNIT tells "
			"the unit to use food items that are in that unit's possession "
			"before using silver. CONSUME FACTION tells the unit to use any "
			"food items that the faction owns (in the same region as the "
			"unit) before using silver. CONSUME tells the unit to use "
			"silver before food items (this is the default).";
		// AssessMaintenance: CONSUME UNIT/FACTION units eat their own food first
		// (CheckUnitMaintenance(1)), then CONSUME FACTION units eat from ANY other unit of the
		// faction in the region (CheckFactionMaintenance(1) does not check the donor's flags).
		// Unflagged units still eat food after the silver steps (CheckUnitMaintenance(0)).
		temp += " Note that the food of a unit is not reserved for that unit "
			"unless it has issued CONSUME UNIT or CONSUME FACTION itself: "
			"units in the same region with CONSUME FACTION may eat any food "
			"it holds before any silver is used. Units that have not issued "
			"CONSUME still eat food once the available silver has run out. "
			"See the section on ";
		temp += f.Link("#economy_maintenance", "maintenance costs") +
			" for the full order in which maintenance is paid.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Tell a unit to use food items in the unit's possession for "
			"maintenance costs.";
		temp2 = "CONSUME UNIT";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("declare");
	f.TagText("h4", "DECLARE [faction] [attitude]");
	f.TagText("h4", "DECLARE [faction]");
	f.TagText("h4", "DECLARE DEFAULT [attitude]");
	temp = "The first form of the DECLARE order sets the attitude of your "
		"faction towards the given faction.  The second form cancels any "
		"attitude towards the given faction (so your faction's attitude "
		"towards that faction will be its default attitude).  The third "
		"form sets your faction's default attitude.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Declare your faction to be hostile to faction 15.";
	temp2 = "DECLARE 15 hostile";
	f.CommandExample(temp, temp2);
	temp = "Set your faction's attitude to faction 15 to its default "
		"attitude.";
	temp2 = "DECLARE 15";
	f.CommandExample(temp, temp2);
	temp = "Set your faction's default attitude to friendly.";
	temp2 = "DECLARE DEFAULT friendly";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("describe");
	f.TagText("h4", "DESCRIBE UNIT [new description]");
	f.TagText("h4", "DESCRIBE SHIP [new description]");
	f.TagText("h4", "DESCRIBE BUILDING [new description]");
	f.TagText("h4", "DESCRIBE OBJECT [new description]");
	f.TagText("h4", "DESCRIBE STRUCTURE [new description]");
	temp = "Change the description of the unit, or of the object the unit "
		"is in (of which the unit must be the owner). Put the description "
		"in double quotes; without them, only the first word is used. Some "
		"characters, such as parentheses, are removed. If "
		"no description is given, the description will be cleared out. The "
		"last four are completely identical and serve to modify the "
		"description of the object you are currently in.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set the unit's description to read \"Merlin's helper\".";
	temp2 = "DESCRIBE UNIT \"Merlin's helper\"";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("destroy");
	f.TagText("h4", "DESTROY");
	// Game::RunDestroyOrders: owner with at least one man; only modifiable objects (fleets
	// are not); INSTANT removes it at once, otherwise each month removes up to
	// men x max(1, Building level) (PER_SKILL) or men points, capped per structure at
	// max(MIN_DESTROY_POINTS, MAX_DESTROY_PERCENT of its cost); the removed points become
	// "needs N" and can be rebuilt; when nothing is left the units are moved outside.
	temp = "Destroy the object you are in (of which you must be the owner, "
		"and the owning unit must have at least one man). The order cannot "
		"be used at sea, and fleets cannot be destroyed (to get rid of "
		"ships, give them away with ";
	temp += f.Link("#give", "GIVE") + " 0).";
	if (Globals->DESTROY_BEHAVIOR == DestroyBehavior::INSTANT) {
		temp += " The structure is destroyed at once, and the units inside "
			"are moved outside.";
	} else {
		temp += " Destroying a structure takes time. Each month, the owner "
			"tears down up to ";
		if (Globals->DESTROY_BEHAVIOR == DestroyBehavior::PER_SKILL)
			temp += "one point per man times its Building skill level (at "
				"least one point per man)";
		else
			temp += "one point per man";
		temp += AString(", but never more than ") + Globals->MIN_DESTROY_POINTS +
			" points or " + Globals->MAX_DESTROY_PERCENT + "% of the "
			"structure's cost, whichever is more. A partly destroyed "
			"structure is unfinished, just like one that is being built, "
			"and can be repaired with ";
		temp += f.Link("#build", "BUILD") + ". When nothing is left, the "
			"structure is gone and the units inside are moved outside.";
	}
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Destroy the current object";
	temp2 = "DESTROY";
	f.CommandExample(temp, temp2);

	if (qm_exist) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("distribute");
		f.TagText("h4", "DISTRIBUTE [unit] [num] [item]");
		f.TagText("h4", "DISTRIBUTE [unit] ALL [item]");
		f.TagText("h4", "DISTRIBUTE [unit] ALL [item] EXCEPT [amount]");
		temp = "Distribute the specified items to the given friendly unit. "
			"In the second form, all of that item are distributed.  In the "
			"last form, all of that item except for the specified amount "
			"are distributed.";
		temp += " The unit issuing the distribute order must have the "
			"quartermaster skill, and be the owner of a transport "
			"structure.";
		// Range and cost mirror Game::CheckTransportOrders / RunTransportOrders: the long-range
		// branch (NONLOCAL_TRANSPORT + quartermaster bonus) requires o->type == O_TRANSPORT, so
		// DISTRIBUTE always gets LOCAL_TRANSPORT. Shipping is charged only when
		// dist > LOCAL_TRANSPORT, which a validated DISTRIBUTE can never be -- hence "always free".
		// A range of 0 means unlimited in that code, and then distributing CAN cost silver, so
		// both sentences are only printed for a positive LOCAL_TRANSPORT.
		if (Globals->LOCAL_TRANSPORT > 0) {
			temp += AString(" The recipient must be within ") +
				Globals->LOCAL_TRANSPORT +
				(Globals->LOCAL_TRANSPORT == 1 ? " hex" : " hexes") +
				" of the distributing unit";
			if (Globals->NONLOCAL_TRANSPORT > Globals->LOCAL_TRANSPORT ||
					Globals->NONLOCAL_TRANSPORT == 0) {
				temp += "; unlike ";
				temp += f.Link("#transport", "TRANSPORT");
				temp += ", distribute never gets the longer range available "
					"to quartermasters";
			}
			temp += ".";
			if (Globals->SHIPPING_COST > 0) {
				temp += " Since shipping costs only apply to goods sent "
					"further than this, distributing never costs any "
					"silver.";
			}
		}
		if (SomeItemsNotTransportable()) {
			temp += " Some items cannot be distributed; see the ";
			temp += f.Link("#transport_items", "list of such items") + ".";
		}
		if (!Globals->TRANSPORT_NO_TRADE) {
			temp += " Use of this order counts as trade activity in "
				"the hex.";
		}
		f.Paragraph(temp);
		f.Paragraph("Examples:");
		temp = "Distribute 10 STON to unit 1234";
		temp2 = "DISTRIBUTE 1234 10 STON";
		f.CommandExample(temp, temp2);
		temp = "Distribute all except 10 SWOR to unit 3432";
		temp2 = "DISTRIBUTE 3432 ALL SWOR EXCEPT 10";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("enter");
	f.TagText("h4", "ENTER [object]");
	temp = "Attempt to enter the specified object.  If issued from inside "
		"another object, the unit will first leave the object it is "
		"currently in.  The order will only work if the target object is "
		"unoccupied, or is owned by a unit in your faction, or is owned by "
		"a faction which has declared you Friendly.  Structures that are "
		"closed to player units cannot be entered at all.  ENTER is carried "
		"out at the very start of the turn, before any battles.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Enter fleet number 114.";
	temp2 = "ENTER 114";
	f.CommandExample(temp, temp2);

	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("entertain");
		f.TagText("h4", "ENTERTAIN");
		temp = "Spend the month entertaining the populace to earn money.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Entertain for money.";
		temp2 = "ENTERTAIN";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("evict");
	f.TagText("h4", "EVICT [unit] ...");
	temp = "Evict the specified unit from the object of which you are "
		"currently the owner.  If multiple EVICT orders are given, all "
		"of the units will be evicted.  EVICT does not work in the Nexus, "
		"and units cannot be evicted from a fleet at sea unless they can "
		"swim (and have not set ";
	temp += f.Link("#nocross", "NOCROSS") + ").";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Evict units 415 and 698 from an object that this unit owns.";
	temp2 = "EVICT 415 698";
	f.CommandExample(temp, temp2);
	temp = "or";
	temp2 = "EVICT 415\nEVICT 698";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("exchange");
	f.TagText("h4", "EXCHANGE [unit] [quantity given] [item given] "
			"[quantity expected] [item expected]");
	temp = "This order allows any two units that can see each other, to "
		"trade items regardless of faction stances.  The orders given by "
		"the two units must be complementary.  If either unit involved does "
		"not have the items it is offering, or if the exchange orders given "
		"are not complementary, the exchange is aborted.  The amounts must "
		"match exactly: offering more than the other unit expects also "
		"aborts the exchange.  Men and ships may not be exchanged, nor items "
		"that cannot be given.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Exchange 10 LBOW for 10 SWOR with unit 1310";
	temp2 = "EXCHANGE 1310 10 LBOW 10 SWOR";
	f.CommandExample(temp, temp2);
	temp = "Unit 1310 would issue (assuming the other unit is 3453)";
	temp2 = "EXCHANGE 3453 10 SWOR 10 LBOW";
	f.CommandExample(temp, temp2);

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("faction");
		f.TagText("h4", "FACTION [type] [points] ...");
		// The accepted type names come from FactionTypes, which depends on FACTION_ACTIVITY
		// (see the faction intro above); a ruleset using Martial rejects WAR and TRADE.
		int martial_types = (Globals->FACTION_ACTIVITY != FactionActivityRules::DEFAULT);
		temp = "Attempt to change your faction's type.  In the order, you ";
		if (martial_types) {
			temp += "can specify up to two faction types (MARTIAL and MAGIC) "
				"and the number of faction points to assign to each type; if "
				"you are assigning points to only one type, you may omit the "
				"other.";
		} else {
			temp += "can specify up to three faction types (WAR, TRADE, and "
				"MAGIC) and the number of faction points to assign to each "
				"type; if you are assigning points to only one or two types, "
				"you may omit the types that will not have any points.";
		}
		f.Paragraph(temp);
		temp = "Changing the number of faction points assigned to MAGIC may "
			"be tricky. Increasing the MAGIC points will always succeed, but "
			"if you decrease the number of points assigned to MAGIC, you "
			"must make sure that you have only the number of magic-skilled "
			"leaders allowed by the new number of MAGIC points BEFORE you "
			"change your point distribution. For example, if you have 3 "
			"mages (3 points assigned to MAGIC), but want to use one of "
			"those points for ";
		temp += (martial_types ? "MARTIAL" : "WAR or TRADE");
		temp += " (change to MAGIC 2), you must "
			"first get rid of one of your mages by either giving it to "
			"another faction or ordering it to ";
		temp += f.Link("#forget", "FORGET") + " all its magic skills. ";
		temp += "If you have too many mages";
		// ProcessFactionOrder also rolls back when CountApprentices exceeds the new limit.
		if (app_exist) {
			temp += AString(" or ") + Globals->APPRENTICE_NAME + "s";
		}
		temp += " for the number of points you "
			"try to assign to MAGIC, the FACTION order will fail.";
		if (qm_exist) {
			temp += " Similar problems could occur with ";
			temp += (martial_types ? "MARTIAL" : "TRADE");
			temp += " points and the number of quartermasters controlled by "
				"the faction.";
		}
		f.Paragraph(temp);
		// ProcessFactionOrder applies the new type immediately while the orders file is being
		// read (before any order runs), rolls back to the type from just before that order on
		// failure, and nothing reads Faction::lastchange -- so there is no cooldown.
		temp = "The FACTION order takes effect as soon as your orders are "
			"read, before any other order is carried out, so all of this "
			"turn's limits already use the new faction type. If you issue "
			"more than one FACTION order, the last one that succeeds is the "
			"one that counts; one that fails leaves the faction type as it "
			"was before that order. There is no limit on how often you may "
			"change your faction type.";
		f.Paragraph(temp);
		f.Paragraph("Examples:");
		if (martial_types) {
			// Same split as the "well rounded faction" example in the faction intro.
			int martial = (Globals->FACTION_POINTS + 1) / 2;
			int magic = Globals->FACTION_POINTS / 2;
			temp = AString("Assign ") + martial + " faction points to MARTIAL "
				"and " + magic + " to MAGIC.";
			temp2 = AString("FACTION MARTIAL ") + martial + " MAGIC " + magic;
		} else {
			temp = "Assign 2 faction points to WAR, 2 to TRADE, and 1 to MAGIC.";
			temp2 = "FACTION WAR 2 TRADE 2 MAGIC 1";
		}
		f.CommandExample(temp, temp2);
		temp = "Become a pure magic faction (assign all points to magic).";
		temp2 = "FACTION MAGIC ";
		temp2 += Globals->FACTION_POINTS;
		f.CommandExample(temp, temp2);
	}

	if (Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("find");
		f.TagText("h4", "FIND [faction]");
		f.TagText("h4", "FIND ALL");
		temp = "Find the email address of the specified faction or of all "
			"factions.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Find the email address of faction 4.";
		temp2 = "FIND 4";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("forget");
	f.TagText("h4", "FORGET [skill]");
	temp = "Forget the given skill. This order is useful for ";
	if (Globals->SKILL_LIMIT_NONLEADERS) {
		temp += "normal units who wish to learn a new skill, but already "
			"know a different skill. It can also be used for ";
	}
	temp += "a mage";
	if (Globals->APPRENTICES_EXIST) {
		if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
			temp += ", ";
		}
		else {
			temp += ", or ";
		}
		temp += Globals->APPRENTICE_NAME;
	}
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		temp += ", or quartermaster";
	}
	temp += " who wish to become a normal "
		"unit. A common reason for this is to be able to change faction "
		"points.";
	// FORGET runs mid-turn (RunForgetOrders), but FACTION is applied while orders are read,
	// so the FACTION order has to wait for the next turn. A mage stays a mage until all its
	// magic skills are gone.
	temp += " A mage only becomes a normal unit once it has forgotten all of "
		"its magic skills.";
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		temp += " Note that FORGET is carried out during the turn, while ";
		temp += f.Link("#faction", "FACTION") + " takes effect as soon as your "
			"orders are read; so forget the skills first, and send the "
			"FACTION order the following turn.";
	}
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Forget knowledge of Mining.";
	temp2 = "FORGET Mining";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("form");
	f.TagText("h4", "FORM [alias]");
	temp = "Form a new unit.  The newly created unit will be in your "
		"faction, in the same region as the unit which formed it, and in "
		"the same structure if any.  It will start off, however, with no "
		"people or items; you should, in the same month, issue orders to "
		"transfer people into the new unit, or have it recruit members. The "
		"new unit will inherit its flags from the unit that forms it, such "
		"as avoiding, behind, revealing and sharing, with the exception "
		"of the guard and autotax flags.  If the new unit wants to "
		"guard or automatically tax then those flags will have to be "
		"explicitly set in its orders.";
	f.Paragraph(temp);
	temp = "The FORM order is followed by a list of orders for the newly "
		"created unit.  This list is terminated by the END keyword, after "
		"which orders for the original unit resume.";
	f.Paragraph(temp);
	temp = "The purpose of the \"alias\" parameter is so that you can refer "
		"to the new unit. You will not know the new unit's number until "
		"you receive the next turn report.  To refer to the new unit in "
		"this set of orders, pick an alias number (the only restriction on "
		"this is that it must be at least 1, and you should not create two "
		"units in the same region in the same month, with the same alias "
		"numbers).  The new unit can then be referred to as NEW <alias> in "
		"place of the regular unit number.  If an alias has already been "
		"used in the region that month, the second FORM fails and the "
		"orders for that unit are ignored.";
	f.Paragraph(temp);
	temp = "You can refer to newly created units belonging to other "
		"factions, if you know what alias number they are, e.g. FACTION 15 "
		"NEW 2 will refer to faction 15's newly created unit with alias 2.";
	f.Paragraph(temp);
	temp = "Note: If a unit moves out of the region in which it was formed "
		"(by the ";
	temp += f.Link("#move", "MOVE") + " order, or otherwise), the alias "
		"will no longer work. This is to prevent conflicts with other units "
		"that may have the same alias in other regions.";
	f.Paragraph(temp);
	temp = "If the demand for recruits in that region that month is much "
		"higher than the supply, it may happen that the new unit does not "
		"gain all the recruits you ordered it to buy, or it may not gain "
		"any recruits at all.  If the new units gains at least one recruit, "
		"the unit will form possessing any unused silver and all the other "
		"items it was given.  If no recruits are gained at all, the empty "
		"unit will be dissolved, and the silver and any other items it was "
		"given will revert to the first unit you have in that region.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "This set of orders for unit 17 would create two new units with "
		"alias numbers 1 and 2, name them Merlin's Guards and Merlin's "
		"Workers, set the description for Merlin's Workers, have both units "
		"recruit men, and have Merlin's Guards study combat.  Merlin's "
		"Workers will have the default order ";
	temp += f.Link("#work", "WORK") + ", as all newly created units do. The "
		"unit that created these two then pays them enough money (using the "
		"NEW keyword to refer to them by alias numbers) to cover the costs "
		"of recruitment and the month's maintenance.";
	temp2 = "UNIT 17\n";
	temp2 += "FORM 1\n";
	temp2 += "    NAME UNIT \"Merlin's Guards\"\n";
	if (Globals->RACES_EXIST)
		temp2 += AString("    BUY 5 ") + ItemDefs[manidx].abr + "\n";
	else
		temp2 += "    BUY 5 men\n";
	temp2 += "    STUDY COMBAT\n";
	temp2 += "END\n";
	temp2 += "FORM 2\n";
	temp2 += "    NAME UNIT \"Merlin's Workers\"\n";
	temp2 += "    DESCRIBE UNIT \"wearing dirty overalls\"\n";
	if (Globals->RACES_EXIST)
		temp2 += AString("    BUY 15 ") + ItemDefs[manidx].abr + "\n";
	else
		temp2 += "    BUY 15 men\n";
	temp2 += "END\n";
	temp2 += "CLAIM 3000\n";    // matches the 1000 + 2000 given below
	temp2 += "GIVE NEW 1 1000 silver\n";
	temp2 += "GIVE NEW 2 2000 silver\n";
	f.CommandExample(temp,temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("give");
	f.TagText("h4", "GIVE [unit] [quantity] [item]");
	f.TagText("h4", "GIVE [unit] ALL [item]");
	f.TagText("h4", "GIVE [unit] ALL [item] EXCEPT [quantity]");
	f.TagText("h4", "GIVE [unit] ALL [item class]");
	f.TagText("h4", "GIVE [unit] UNIT");
	temp = "The first form of the GIVE order gives a quantity of an item to "
		"another unit. The second form of the GIVE order will give all of "
		"a given item to another unit.  The third form will give all of an "
		"item except for a specific quantity to another unit.  The fourth "
		"form will give all items of a specific type to another unit.  The "
		"final form of the GIVE order gives the entire unit to the "
		"specified unit's faction.";
	f.Paragraph(temp);
	temp = "The classes of items which are acceptable for the fourth form of "
		"this order are, NORMAL, ADVANCED, TRADE, MAN or MEN, MONSTER or "
		"MONSTERS, MAGIC, WEAPON or WEAPONS, ARMOR, MOUNT or MOUNTS, BATTLE, "
		"SPECIAL, TOOL or TOOLS, FOOD, SHIP or SHIPS and ITEM or ITEMS "
		"(which is the combination of all of the previous categories).";
	f.Paragraph(temp);
	temp = "A unit may only give items, including silver, to a unit which "
		"it is able to see, unless the faction of the target unit has "
		"declared you Friendly or better.  If the target unit is not a "
		"member of your faction, then its faction must have declared you "
		"Friendly, with a couple of exceptions. First, silver may be given "
		"to any unit, regardless of factional affiliation. Secondly, men "
		"may not be given to units in other factions (you must give the "
		"entire unit); the reason for this is to prevent highly skilled "
		"units from being sabotaged with a ";
	temp += f.Link("#give", "GIVE") + " order.";
	f.Paragraph(temp);
	temp = "Unfinished ships are given like other items, although a unit "
		"may only have one unfinished ship of a given type at a time. "
		"To give unfinished ships, add the \"UNFINISHED\" keyword to "
		"the beginning of the [item] specifier.";
	f.Paragraph(temp);
	temp = "Completed ships which are part of a fleet may be given too; "
		"the owner of the fleet they are currently in must issue the ";
	temp += f.Link("#give", "GIVE");
	temp += " order, and give the ships to the owner of the fleet that "
		"should receive the ships.  If the recipient is not the owner "
		"of a fleet, then a new fleet will be created owned by the "
		"recipient, and the recipient is moved into it.";
	f.Paragraph(temp);
	// GIVE UNIT (DoGiveOrder): the receiving faction must regard you Friendly, must have room
	// under its mage/apprentice/quartermaster limits, and must not be a non-player faction.
	temp = "When giving a whole unit, the receiving faction must have "
		"declared you Friendly, and it cannot be a non-player faction such "
		"as monsters or guards";
	if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
		temp += "; a mage";
		if (app_exist) temp += AString(", ") + Globals->APPRENTICE_NAME;
		if (qm_exist) temp += " or quartermaster";
		temp += " can only be given to a faction that has room for one more "
			"under its limits";
	}
	temp += ".";
	f.Paragraph(temp);
	temp = "There are also a few restrictions on orders given by units who "
		"have been given to another faction. If the receiving faction is not "
		"allied to the giving faction, the unit may not issue the ";
	temp += f.Link("#advance", "ADVANCE") + " order, or issue any more ";
	temp += f.Link("#give", "GIVE") + " orders.  Both of these rules are to "
		"prevent unfair sabotage tactics.";
	f.Paragraph(temp);
	temp = "If 0 is specified as the unit number, then the items are "
		"discarded.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Give 10 swords to unit 4573.";
	temp2 = "GIVE 4573 10 swords";
	f.CommandExample(temp, temp2);
	temp = "Give 5 chain armor to the new unit, alias 2, belonging to "
		"faction 14.";
	temp2 = "GIVE FACTION 14 NEW 2 5 \"Chain armor\"";
	f.CommandExample(temp, temp2);
	temp = "Give control of this unit to the faction owning unit 75.";
	temp2 = "GIVE 75 UNIT";
	f.CommandExample(temp, temp2);
	{
		// Use the first enabled ship (the Longboat is disabled in NewOrigins).
		int ship = -1;
		for (i = 0; i < NITEMS && ship == -1; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (ItemDefs[i].type & IT_SHIP) ship = i;
		}
		if (ship != -1) {
			temp = AString("Give our unfinished ") + ItemDefs[ship].name +
				" to unit 95.";
			temp2 = AString("GIVE 95 1 UNFINISHED ") + ItemDefs[ship].abr;
			f.CommandExample(temp, temp2);
			temp = AString("Transfer 2 ") + ItemDefs[ship].names +
				" to the fleet commanded by unit 83.";
			temp2 = AString("GIVE 83 2 ") + ItemDefs[ship].abr;
			f.CommandExample(temp, temp2);
		}
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("guard");
	f.TagText("h4", "GUARD [flag]");
	// Authoritative description of guarding; other sections link here. Mirrors
	// ARegion::CanTax (blocked unless the guard's faction is Friendly+ to the taxer),
	// ARegion::CanPillage (any other faction), Unit::Forbids (Unfriendly/Hostile units the
	// guard can see and catch) and Game::DoGuard1Orders (restrictions; GUARD_SET only becomes
	// GUARD_GUARD there, after the PILLAGE and TAX phases in RunOrders).
	temp = "GUARD 1 puts the unit issuing the order on guard. Units on guard "
		"prevent units of factions that you have not declared Friendly (or "
		"Ally) from collecting taxes in the region, and prevent units of any "
		"other faction from pillaging it. They also stop Unfriendly and "
		"Hostile units from entering the region, but only units that your "
		"faction can see and catch (see ";
	temp += f.Link("#com_attacking", "attacking") + "). GUARD 0 cancels "
		"Guard status.";
	f.Paragraph(temp);
	temp = "A unit can only be on guard if:";
	f.Paragraph(temp);
	f.Enclose(1, "ul");
	f.TagText("li", "it is able to tax (see the TAX order);");
	if (!Globals->OCEAN_GUARD) {
		f.TagText("li", "it is not in an ocean region;");
	}
	if (Globals->STRICT_GUARD) {
		f.TagText("li", "every unit of another faction that is already on "
			"guard in the region belongs to a faction that has declared the "
			"unit's faction Ally. In practice, only one faction and its allies "
			"can guard a region at the same time;");
	}
	if (Globals->CITY_MONSTERS_EXIST) {
		f.TagText("li", "there are no city or town guardsmen on guard in the "
			"region.");
	}
	f.Enclose(0, "ul");
	temp = "A GUARD 1 order takes effect after ";
	temp += f.Link("#tax", "TAX") + " and " + f.Link("#pillage", "PILLAGE") +
		" orders have been carried out, so it does not stop taxing or "
		"pillaging in the month it is given, but it does stop units from "
		"entering the region later that month. A unit that moves or sails "
		"is taken off guard.";
	if (Globals->WANDERING_MONSTERS_EXIST) {
		temp += " Guarding also stops new monsters from appearing in the "
			"region (see ";
		temp += f.Link("#nonplayers_monsters", "Wandering Monsters") + ").";
	}
	f.Paragraph(temp);
	temp = "The Guard and Avoid Combat flags are mutually exclusive; "
		"setting one automatically cancels the other.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Instruct the current unit to be on guard.";
	temp2 = "GUARD 1";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("hold");
	f.TagText("h4", "HOLD [flag]");
	// Units helping from an adjacent region never actually leave their building (and keep
	// its protection with EXTENDED_FORT_DEFENCE); HOLD only keeps the unit out of those battles.
	temp = "HOLD 1 instructs the issuing unit to never join a battle in "
		"regions the unit is not in, for example to keep a unit out of "
		"its neighbours' fights.  HOLD 0 cancels holding status.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Instruct the unit to avoid combat in other regions.";
	temp2 = "HOLD 1";
	f.CommandExample(temp, temp2);

	// ProcessIdleOrder: a month long order that replaces any other; RunIdleOrders reports
	// "Sits idle." It exists mainly to stop the default WORK order.
	f.ClassTagText("div", "rule", "");
	f.LinkRef("idle");
	f.TagText("h4", "IDLE");
	temp = "Do nothing for the month. IDLE is a month long order";
	if (Globals->DEFAULT_WORK_ORDER) {
		temp += ", and its main use is to stop a unit from ";
		temp += f.Link("#work", "WORK") + "ing, which a unit without any "
			"month long order does automatically";
	}
	temp += ".";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Do nothing this month.";
	temp2 = "IDLE";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("join");
	f.TagText("h4", "JOIN [unit]");
	f.TagText("h4", "JOIN [unit] NOOVERLOAD");
	f.TagText("h4", "JOIN [unit] MERGE");
	temp = "Attempt to enter the building or fleet that the specified "
		"unit is currently inside.  This is particularly useful if "
		"you don't know what the building or fleet number will be, "
		"as is the case when a new fleet is created.";
	f.Paragraph(temp);
	temp = "If the target unit is not inside a building or fleet, then "
		"the unit issuing the JOIN command will leave any building "
		"or fleet that they happen to be inside, to be with the "
		"target.";
	f.Paragraph(temp);
	temp = "If the NOOVERLOAD flag is specified, and the target unit ends "
		"up on board a fleet, then the unit issuing the JOIN command "
		"will only attempt to board the fleet if the fleet would be "
		"able to sail with the issuing units' weight loaded on board.";
	f.Paragraph(temp);
	temp = "The MERGE flag may only be used by the owner of a fleet, "
		"and will cause the entire fleet they command to join the "
		"fleet owned by the specified unit - the units on board "
		"will move to the other fleet, and all the ships of the "
		"fleet will be given to the target fleet. "
		"This command will fail if any unit in the fleet to be "
		"merged would be denied entry to the target fleet.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Instruct the unit to enter the building or fleet that unit 17 is in.";
	temp2 = "JOIN 17";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("leave");
	f.TagText("h4", "LEAVE");
	temp = "Leave the object you are currently in.";
	if (move_over_water) {
		temp += " If a unit is capable of swimming ";
		if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE)
			temp += "or flying ";
		temp += "then this order is usable to leave a boat while at sea, "
			"provided that it has not set ";
		temp += f.Link("#nocross", "NOCROSS") + ".";
	} else
		temp += " The order cannot be used at sea.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Leave the current object";
	temp2 = "LEAVE";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("move");
	f.TagText("h4", "MOVE [dir] ...");
	temp = "Attempt to move in the direction(s) specified.  If more than "
		"one direction is given, the unit will move multiple times, in "
		"the order specified by the MOVE order, until no more directions "
		"are given, or until one of the moves fails.  A move can fail "
		"because the unit attempts to move into the ocean (only units that "
		"can swim or fly, and have not set NOCROSS, may do so), or because "
		"guards stop it from entering a region.  If the unit is refused "
		"entry to a structure, it gets an error but carries on with the "
		"rest of its move.  If the unit runs out of movement points, the "
		"remaining moves are kept and continue next turn (see ";
	temp += f.Link("#movement_order", "Order of Movement") + ").";
	f.Paragraph(temp);
	temp = "Valid directions are:";
	f.Paragraph(temp);
	temp = "1) The compass directions North, Northwest, Southwest, South, "
		"Southeast, and Northeast.  These can be abbreviated N, NW, SW, S, "
		"SE, NE.";
	f.Paragraph(temp);
	temp = "2) A structure number.";
	f.Paragraph(temp);
	temp = "3) OUT, which will leave the structure that the unit is in.";
	f.Paragraph(temp);
	temp = "4) IN, which will move through an inner passage in the "
		"structure that the unit is currently in.";
	f.Paragraph(temp);
	temp = "5) PAUSE, which will instruct the unit to spend one movement "
		"point admiring the scenery, presumably to coordinate with "
		"slower moving companions.  This can be abbreviated P.";
	f.Paragraph(temp);
	temp = "Multiple MOVE orders given by one unit will chain together.";
	f.Paragraph(temp);
	temp = "Note that MOVE orders can lead to combat, due to hostile units "
		"meeting, or due to an advancing unit being forbidden access to a "
		"region.  Combat occurs after an entire movement phase has "
		"been completed for all regions, so units coming from different "
		"regions only fight together if they arrive in the same phase.";
	f.Paragraph(temp);
	temp = "Example 1: Units 1 and 2 are in Region A, and unit 3 is in "
		"Region B.  Units 1 and 2 are hostile to unit 3.  Both units "
		"1 and 2 move into region B, and attack unit 3.  Since combat "
		"happens after all movement has been done for the phase, "
		"they attack unit 3 at the same time, and the battle is "
		"between units 1 and 2, and unit 3.";
	f.Paragraph(temp);
	temp = "Example 2: Same as example 1, except unit 2 is in Region C, "
		"instead of region A.  Both units move into Region B, and "
		"attack unit 3.  Because combat happens after all movement "
		"has been done for the phase, they still attack unit 3 at "
		"the same time, and the battle is still between units 1 and "
		"2, and unit 3.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Move N, NE, enter structure 1 and use the passage there";
	temp2 = "MOVE N\nMOVE NE 1 IN";
	f.CommandExample(temp, temp2);
	temp = "or:";
	temp2 = "MOVE N NE 1 IN";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("name");
	f.TagText("h4", "NAME UNIT [new name]");
	f.TagText("h4", "NAME FACTION [new name]");
	f.TagText("h4", "NAME OBJECT [new name]");
	if (Globals->TOWNS_EXIST)
		f.TagText("h4", "NAME CITY [new name]");
	temp = "Change the name of the unit, or of your faction, or of "
		"the object the unit is in (of which the unit must be the owner). "
		"Names can be of any length, up to the line length your mailer "
		"can handle.  Names may not contain parentheses (square brackets "
		"can be used instead if necessary), or any control characters.";
	f.Paragraph(temp);
	if (Globals->TOWNS_EXIST) {
		temp = "In order to rename a settlement (city, town or village), "
			"the unit attempting to rename it must be the owner of a large "
			"enough structure located in the city. It requires a tower or "
			"better to rename a village, a fort or better to rename a town "
			"and a castle or a citadel to rename a city. ";
		if (Globals->CITY_RENAME_COST) {
			int c=Globals->CITY_RENAME_COST;
			temp += AString("It also costs $") + c + " to rename a village, $";
			temp += AString(2*c) + " to rename a town, and $";
			temp += AString(3*c) + " to rename a city.";
		}
		f.Paragraph(temp);
	}
	f.Paragraph("Example:");
	temp = "Name your faction \"The Merry Pranksters\".";
	temp2 = "NAME FACTION \"The Merry Pranksters\"";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("noaid");
	f.TagText("h4", "NOAID [flag]");
	temp = "NOAID 1 indicates that if the unit attacks, or is attacked, it "
		"is not to be aided by units in other hexes. NOAID status is very "
		"useful for scouts or probing units, who do not wish to drag "
		"their nearby armies into battle if they are caught. NOAID 0 "
		"cancels this.";
	f.Paragraph(temp);
	temp = "If multiple units are on one side in a battle, they must all "
		"have the NOAID flag on, or they will receive aid from other hexes.";
	if (Globals->ALLIES_NOAID) {
		temp += " If every unit of the defending faction in the region has "
			"NOAID set, its allies in the same region stay out of the battle "
			"too.";
	}
	temp += " See ";
	temp += f.Link("#com_muster", "the muster") + ".";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set a unit to receive no aid in battle.";
	temp2 = "NOAID 1";
	f.CommandExample(temp, temp2);

	if (move_over_water) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("nocross");
		f.TagText("h4", "NOCROSS [flag]");
		temp = "NOCROSS 1 indicates that if a unit attempts to cross a "
			"body of water then that unit should instead not cross it, "
			"regardless of whether the unit otherwise could do so. ";
		if (may_sail) {
			temp += "Units inside a fleet are not affected by this flag "
				"(IE, they are able to sail within the fleet). ";
		}
		temp += "This flag is useful to prevent scouts from accidentally "
			"drowning when exploring in games where movement over water "
			"is allowed. NOCROSS 0 cancels this.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Set a unit to not permit itself to cross water.";
		temp2 = "NOCROSS 1";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("option");
	f.TagText("h4", "OPTION TIMES");
	f.TagText("h4", "OPTION NOTIMES");
	f.TagText("h4", "OPTION SHOWATTITUDES");
	f.TagText("h4", "OPTION DONTSHOWATTITUDES");
	f.TagText("h4", "OPTION TEMPLATE OFF");
	f.TagText("h4", "OPTION TEMPLATE SHORT");
	f.TagText("h4", "OPTION TEMPLATE LONG");
	f.TagText("h4", "OPTION TEMPLATE MAP");
	temp = "The OPTION order is used to toggle various settings that "
		"affect your reports, and other email details. OPTION TIMES sets it "
		"so that your faction receives the times each turn (this is the "
		"default); OPTION NOTIMES sets it so that your faction is not sent "
		"the times.";
	f.Paragraph(temp);
	temp = "OPTION SHOWATTITUDES will cause units shown in your report to "
		"have a character placed before their name, which indicates "
		"your attitude towards them.  These characters are \"!\" for "
		"hostile, \"%\" for unfriendly, \"-\" for neutral, "
		"\":\" for friendly and \"=\" for allied. "
		"OPTION DONTSHOWATTITUDES turns this off again, so that all other "
		"factions' units are marked with \"-\" (this is the default). ";
	f.Paragraph(temp);
	temp = "The OPTION TEMPLATE order toggles the length of the Orders "
		"Template that appears at the bottom of a turn report.  The OFF "
		"setting eliminates the Template altogether, and the SHORT, LONG "
		"and MAP settings control how much detail the Template contains. "
		"The MAP setting will produce an ascii map of the region and "
		"surrounding regions in addition other details.";
	f.Paragraph(temp);
	temp = "For the MAP template, the region identifiers are (there might "
		"be additional symbols for unusual/special terrain):";
	f.Paragraph(temp);
	f.Enclose(1, "table");
	if (Globals->UNDERWORLD_LEVELS) {
		f.Enclose(1, "tr");
		f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
		f.PutStr("####");
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr("BLOCKED HEX (Underworld)");
		f.Enclose(0, "td");
		f.Enclose(0, "tr");
	}
	f.Enclose(1, "tr");
	f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
	f.PutStr("~~~~");
	f.Enclose(0, "td");
	f.Enclose(1, "td align=\"left\" nowrap");
	f.PutStr("OCEAN HEX");
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	f.Enclose(1, "tr");
	f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
	f.PutStr("    ");
	f.Enclose(0, "td");
	f.Enclose(1, "td align=\"left\" nowrap");
	temp = "PLAINS";
	if (Globals->UNDERWORLD_LEVELS)
		temp += "/TUNNELS";
	temp += " HEX";
	f.PutStr(temp);
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	f.Enclose(1, "tr");
	f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
	f.PutStr("^^^^");
	f.Enclose(0, "td");
	f.Enclose(1, "td align=\"left\" nowrap");
	temp = "FOREST";
	if (Globals->UNDERWORLD_LEVELS)
		temp += "/UNDERFOREST";
	temp += " HEX";
	f.PutStr(temp);
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	f.Enclose(1, "tr");
	f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
	f.PutStr("/\\/\\");
	f.Enclose(0, "td");
	f.Enclose(1, "td align=\"left\" nowrap");
	f.PutStr("MOUNTAIN HEX");
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	f.Enclose(1, "tr");
	f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
	f.PutStr("vvvv");
	f.Enclose(0, "td");
	f.Enclose(1, "td align=\"left\" nowrap");
	f.PutStr("SWAMP HEX");
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	f.Enclose(1, "tr");
	f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
	f.PutStr("@@@@");
	f.Enclose(0, "td");
	f.Enclose(1, "td align=\"left\" nowrap");
	f.PutStr("JUNGLE HEX");
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	f.Enclose(1, "tr");
	f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
	f.PutStr("....");
	f.Enclose(0, "td");
	f.Enclose(1, "td align=\"left\" nowrap");
	temp = "DESERT";
	if (Globals->UNDERWORLD_LEVELS)
		temp += "/CAVERN";
	temp += " HEX";
	f.PutStr(temp);
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	f.Enclose(1, "tr");
	f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
	f.PutStr(",,,,");
	f.Enclose(0, "td");
	f.Enclose(1, "td align=\"left\" nowrap");
	f.PutStr("TUNDRA HEX");
	f.Enclose(0, "td");
	f.Enclose(0, "tr");
	if (Globals->NEXUS_EXISTS) {
		f.Enclose(1, "tr");
		f.Enclose(1, "td align=\"left\" nowrap class=\"fixed\"");
		f.PutStr("!!!!");
		f.Enclose(0, "td");
		f.Enclose(1, "td align=\"left\" nowrap");
		f.PutStr("THE NEXUS");
		f.Enclose(0, "td");
		f.Enclose(0, "tr");
	}
	f.Enclose(0, "table");
	f.Paragraph("Example:");
	temp = "Set your faction to receive the map format order template";
	temp2 = "OPTION TEMPLATE MAP";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("password");
	f.TagText("h4", "PASSWORD [password]");
	f.TagText("h4", "PASSWORD");
	temp = "The PASSWORD order is used to set your faction's password. If "
		"you have a password set, you must specify it on your #ATLANTIS "
		"line for the game to accept your orders.  This protects you orders "
		"from being overwritten, either by accident or intentionally by "
		"other players.  PASSWORD with no password given clears out your "
		"faction's password.";
	f.Paragraph(temp);
	temp = "IMPORTANT: The PASSWORD order does not take effect until the "
		"turn is actually run.  So if you set your password, and then want "
		"to re-submit orders, you should use the old password until the "
		"turn has been run.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set the password to \"xyzzy\".";
	temp2 = "PASSWORD xyzzy";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("pillage");
	f.TagText("h4", "PILLAGE");
	temp = "Use force to extort as much money as possible from the region. "
		"Note that the ";
	temp += f.Link("#tax", "TAX") + " order and the PILLAGE order are ";
	temp += "mutually exclusive; a unit may only attempt to do one in a "
		"turn.";
	if (Globals->TAX_PILLAGE_MONTH_LONG) {
		temp += " PILLAGE is a month long order, like TAX.";
	}
	// ARegion::CanPillage: any guard of another faction blocks it, allies included; the
	// threshold is in RunPillageOrders.
	temp += " Units on guard of any other faction, even allied ones, prevent "
		"pillaging, and pillaging fails unless enough men are pillaging to "
		"tax at least half of the region's money. See the section on ";
	temp += f.Link("#economy_taxingpillaging", "taxing and pillaging") +
		" for details.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Pillage the current hex.";
	temp2 = "PILLAGE";
	f.CommandExample(temp, temp2);

	if (Globals->USE_PREPARE_COMMAND) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("prepare");
		f.TagText("h4", "PREPARE [item]");
		temp = "This command allows a mage";
		if (app_exist) {
			temp += " or ";
			temp += Globals->APPRENTICE_NAME;
		}
		temp += " to prepare a "
			"battle item (e.g. a Staff of Fire) for use in battle. ";
		if (Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_STRICT) {
			temp += " This selects the battle item which will be used, ";
		} else {
			temp += "This allows the unit to override the usual selection "
				"of battle items, ";
		}
		temp += "and also cancels any spells set via the ";
		temp += f.Link("#combat", "COMBAT") + " order.";
		// Soldier::SetupCombatItems: with a prepared item, only it (and shields) are used.
		if (Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_NORMAL) {
			temp += " If an item is prepared, it is the only battle item the "
				"unit uses, apart from shields.";
		}
		temp += " PREPARE with no item clears the setting.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Select a staff of fire as the ";
		if (!(Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_STRICT))
			temp += "preferred ";
		temp += "battle item.";
		temp2 = "PREPARE STAF";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("produce");
	f.TagText("h4", "PRODUCE [item]");
	f.TagText("h4", "PRODUCE [number] [item]");
	temp = "Spend the month producing the specified item.  If a number "
		"is given then the unit will attempt to produce exactly "
		"that number of items; if this is not possible in one month "
		"then the order will carry over to subsequent months.  If "
		"no number is given then the unit will produce as much "
		"as possible of the specified item.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Produce as much wood as possible.";
	temp2 = "PRODUCE wood";
	f.CommandExample(temp, temp2);
	temp = "Produce exactly 3 crossbows.";
	temp2 = "PRODUCE 3 crossbows";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("promote");
	f.TagText("h4", "PROMOTE [unit]");
	temp = "Promote the specified unit to owner of the object of which you "
		"are currently the owner.  The target unit must be inside the same "
		"object; its faction's attitude towards you does not matter.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Promote unit 415 to be the owner of the object that this unit "
		"owns.";
	temp2 = "PROMOTE 415";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("quit");
	f.TagText("h4", "QUIT [password]");
	temp = "Quit the game.  On issuing this order, your faction will be "
		"completely and permanently destroyed. Note that if your faction "
		"has a password, you must give it for the quit order to work; this "
		"is to provide some safety against accidentally issuing this "
		"order.";
	f.Paragraph(temp);
	temp = "Note that although this order affects the faction as a whole, "
		"it nevertheless needs to be issued by an individual unit, "
		"and so the email containing the command to quit needs to "
		"include both #atlantis and unit lines.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Quit the game for faction 27 if your password is foobar.";
	temp2 = "#atlantis 27 \"foobar\"\n";
	temp2 += "unit 1234\n";
	temp2 += "QUIT \"foobar\"\n";
	temp2 += "#end";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("restart");
	f.TagText("h4", "RESTART [password]");
	temp = "Similar to the ";
	temp += f.Link("#quit", "QUIT") + " order, this order will completely "
		"and permanently destroy your faction. However, it will begin a "
		"brand new faction for you (you will get a separate turn report for "
		"the new faction). Note that if your faction has a password, you "
		"must give it for this order to work, to provide some protection "
		"against accidentally issuing this order.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Restart faction 27 as a new faction if your password is foobar.";
	temp2 = "#atlantis 27 \"foobar\"\n";
	temp2 += "unit 1234\n";
	temp2 += "RESTART \"foobar\"\n";
	temp2 += "#end";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("reveal");
	f.TagText("h4", "REVEAL");
	f.TagText("h4", "REVEAL UNIT");
	f.TagText("h4", "REVEAL FACTION");
	temp = "Cause the unit to either show itself (REVEAL UNIT), or show "
		"itself and its faction affiliation (REVEAL FACTION), in the turn "
		"report, to all other factions in the region. ";
	if (has_stea) {
		temp += "Used to reveal high stealth scouts, should there be some "
			"reason to. ";
	}
	temp += "REVEAL is used to cancel this.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Show the unit to all factions.";
	temp2 = "REVEAL UNIT";
	f.CommandExample(temp, temp2);
	temp = "Show the unit and its affiliation to all factions.";
	temp2 = "REVEAL FACTION";
	f.CommandExample(temp, temp2);
	temp = "Cancels revealing.";
	temp2 = "REVEAL";
	f.CommandExample(temp, temp2);

	if (may_sail) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("sail");
		f.TagText("h4", "SAIL [dir] ...");
		f.TagText("h4", "SAIL");
		temp = "The first form will sail the fleet, which the unit must be "
			"the owner of, in the directions given.  The second form "
			"will cause the unit to aid in the sailing of the fleet, using "
			"the Sailing skill.  Besides the compass directions, PAUSE (or P) "
			"makes the fleet spend one movement point manoeuvring where it "
			"is.  Directions the fleet does not get to this month are kept, "
			"and it continues next turn.  See the section on movement for more "
			"information on the mechanics of sailing.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Sail north, then northwest.";
		temp2 = "SAIL N NW";
		f.CommandExample(temp, temp2);
		temp = "or:";
		temp2 = "SAIL N\nSAIL NW";
		f.CommandExample(temp, temp2);
	}

	if (Globals->TOWNS_EXIST) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("sell");
		f.TagText("h4", "SELL [quantity] [item]");
		f.TagText("h4", "SELL ALL [item]");
		temp = "Attempt to sell the amount given of the item given.  If the "
			"unit does not have as many of the item as it is trying to sell, "
			"it will attempt to sell all that it has. The first form can also "
			"sell items held by units of yours in the region that have set ";
		temp += f.Link("#share", "SHARE") + " 1. The second form will "
			"attempt to sell all of that item the unit itself has. "
			"If more of the item are on sale (by all the units in the region) "
			"than are wanted by the region, the number sold per unit will be "
			"split up in proportion to the number each unit tried to sell.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Sell 10 furs to the market.";
		temp2 = "SELL 10 furs";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("share");
	f.TagText("h4", "SHARE [flag]");
	temp = "SHARE 1 instructs the unit to share its possessions with any "
		"other unit of your faction that needs them.  Thus a unit with a "
		"supply of silver could automatically provide silver if any of "
		"your other units in the same region does not have enough to "
		"perform an action, such as ";
	temp +=	f.Link("#study", "studying");
	temp += ", ";
	temp +=	f.Link("#buy", "buying");
	temp += " or ";
	temp +=	f.Link("#produce", "producing");
	temp += ".  SHARE 0 returns a unit to its default selfish state.";
	// Unit::ConsumeShared: own items first, then sharing units of the same faction in
	// region/object/unit list order (the report order).
	temp += " A unit always uses its own possessions first; if it needs "
		"more, it takes them from your sharing units in the same region, in "
		"the order they appear in your turn report.";
	f.Paragraph(temp);
	temp = "This sharing does not extend to the heat of battle, "
		"only to economic actions.  So a unit that is sharing will provide "
		"silver for buying or studying, and resources for production "
		"(for example, if a sharing unit has wood in its inventory, and "
		"another unit is producing axes but has no wood, then the sharing "
		"unit will automatically supply wood for that production), "
		"but will not provide weapons to all units if combat occurs.";
	f.Paragraph(temp);
	temp = "Note that in the case of sharing silver, this can leave the "
		"sharing unit without enough funds to pay maintenance, so "
		"sharing is to be used with care.  You may like to make sure that "
		"there is a unit with sufficient funds for maintenance in the "
		"same region, and which is not sharing, as those funds will be "
		"shared for maintenance, but not for less important purposes.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Instruct the unit to share its possessions with other units "
			"of the same faction.";
	temp2 = "SHARE 1";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("show");
	f.TagText("h4", "SHOW SKILL [skill] [level]");
	f.TagText("h4", "SHOW ITEM [item]");
	f.TagText("h4", "SHOW OBJECT [object]");
	temp = "The first form of the order shows the skill description for a "
		"skill that your faction already possesses. The second form "
		"returns some information about an item that is not otherwise "
		"apparent on a report, such as the weight. The last form "
		"returns some information about an object (such as a ship or a "
		"building).";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Show the skill report for Mining 3 again.";
	temp2 = "SHOW SKILL Mining 3";
	f.CommandExample(temp, temp2);
	temp = "Show the item information for swords again.";
	temp2 = "SHOW ITEM sword";
	f.CommandExample(temp, temp2);
	temp = "Show the information for towers again.";
	temp2 = "SHOW OBJECT tower";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("spoils");
	f.TagText("h4", "SPOILS [type]");
	f.TagText("h4", "SPOILS");
	temp = "The SPOILS order determines which types of spoils the unit "
		"should take after a battle.  The valid values for type are "
		"'NONE', 'WALK', 'RIDE', 'FLY', 'SWIM', 'SAIL' or 'ALL'. "
		"The second form is equivalent to 'SPOILS ALL'.";
	f.Paragraph(temp);
	temp = "When this command is issued, the unit is instructed to only "
		"pick up combat spoils if they could use the chosen form "
		"of movement while carrying the spoils. Thus a unit with "
		"SPOILS FLY selected would pick up combat spoils until they "
		"reached their flying capacity.  If the spoils provide "
		"movement capacity themselves, this will be included in the "
		"decision of whether or not to take the spoils - so a unit "
		"with SPOILS RIDE would always pick up horses. "
		"SPOILS SAIL will use the capacity of the fleet the unit is "
		"in to determine whether to take spoils or not. "
		"SPOILS ALL will allow a unit to collect any spoils which "
		"are dropped regardless of weight or capacity. "
		"SPOILS NONE will instruct the unit to only collect "
		"weightless items, such as silver.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Set a unit to only pick up items that it can carry and "
			"continue to fly:";
	temp2 = "SPOILS FLY";
	f.CommandExample(temp, temp2);
	f.Paragraph("The old NOSPOILS order is no longer supported; use SPOILS "
		"instead.");

	if (has_stea) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("steal");
		f.TagText("h4", "STEAL [unit] [item]");
		// Do1Steal: one item, or for silver half the target's silver, at most 200.
		temp = "Attempt to steal the specified item from the specified unit. "
			"A successful theft takes one of the item, or, for silver, half "
			"of the target's silver, up to 200. Men and creatures cannot be "
			"stolen, and guards and monsters cannot be stolen from. The order "
			"may only be issued by a one-man unit (see ";
		temp += f.Link("#stealthobs_stealing", "Stealing") + ").";
		f.Paragraph(temp);
		temp = "A unit may only attempt to steal from a unit which is "
			"able to be seen.";
		f.Paragraph(temp);
		f.Paragraph("Examples:");
		temp = "Steal silver from unit 123.";
		temp2 = "STEAL 123 SILVER";
		f.CommandExample(temp, temp2);
		temp = "Steal wood from unit 321.";
		temp2 = "STEAL 321 wood";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("study");
	f.TagText("h4", "STUDY [skill]");
	f.TagText("h4", "STUDY [skill] [level]");
	temp = "Spend the month studying the specified skill. "
		"A level may be specified which means that study "
		"will be continued from turn to turn until the unit "
		" reaches that skill level. ";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Study horse training.";
	temp2 = "STUDY \"Horse Training\"";
	f.CommandExample(temp, temp2);
	temp = "Study combat to level 3.";
	temp2 = "STUDY combat 3";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("take");
	f.TagText("h4", "TAKE FROM [unit] [quantity] [item]");
	f.TagText("h4", "TAKE FROM [unit] ALL [item]");
	f.TagText("h4", "TAKE FROM [unit] ALL [item] EXCEPT [quantity]");
	f.TagText("h4", "TAKE FROM [unit] ALL [item class]");
	temp = "The TAKE order works just like the ";
	temp += f.Link("#give", "GIVE");
	temp += " order, except that the direction of transfer is reversed, "
		"and with the extra condition that a unit may only TAKE "
		"from another unit in the same faction. Since that makes "
		"TAKE FROM [unit] UNIT pointless, that form of the ";
	temp += f.Link("#give", "GIVE");
	temp += " order is not supported.";
	f.Paragraph(temp);
	temp = "The TAKE order is primarily intended to make automated "
		"delivery caravans less prone to generating errors, as "
		"they can use TAKE to collect the appropriate goods only "
		"when they are in the right place to collect, so the "
		"supplying unit doesn't need to keep trying to ";
	temp += f.Link("#give", "GIVE");
	temp += " to a unit that is only there some of the time. "
		"However, it can be used anywhere you wish to transfer "
		"items, ships or men, just as ";
	temp += f.Link("#give", "GIVE");
	temp += " can.";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	temp = "Take 10 swords from unit 4573.";
	temp2 = "TAKE FROM 4573 10 swords";
	f.CommandExample(temp, temp2);
	temp = "See the ";
	temp += f.Link("#turn", "TURN");
	temp += " order for an example of a caravan using TAKE FROM.";
	f.Paragraph(temp);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("tax");
	f.TagText("h4", "TAX");
	temp = "Attempt to collect taxes from the region. ";
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp += AString("Only ") + tax_factions + " may collect taxes, and then ";
	else
		temp += "Taxes may be collected ";
	temp += "only if there are no non-Friendly units on guard. Only "
		"combat-ready units may issue this order. Note that the TAX order "
		"and the ";
	temp += f.Link("#pillage", "PILLAGE") + " order are mutually exclusive; "
		"a unit may only attempt to do one in a turn.";
	if (Globals->TAX_PILLAGE_MONTH_LONG) {
		temp += " TAX is a month long order: it replaces any other month long "
			"order the unit has been given, and a later month long order "
			"replaces it. It is still carried out early in the turn, before "
			"movement.";
	}
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Attempt to collect taxes.";
	temp2 = "TAX";
	f.CommandExample(temp, temp2);

	f.ClassTagText("div", "rule", "");
	f.LinkRef("teach");
	f.TagText("h4", "TEACH [unit] ...");
	temp = "Attempt to teach the specified units whatever skill they are "
		"studying that month.  A list of several units may be specified. "
		"All units to be taught must have declared you Friendly. "
		"Subsequent TEACH orders can be used to add units to be taught.";
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Teach new unit 2 and unit 510 whatever they are studying.";
	temp2 = "TEACH NEW 2 510";
	f.CommandExample(temp, temp2);
	temp = "or:";
	temp2 = "TEACH NEW 2\nTEACH 510";
	f.CommandExample(temp, temp2);

	if (qm_exist) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("transport");
		f.TagText("h4", "TRANSPORT [unit] [num] [item]");
		f.TagText("h4", "TRANSPORT [unit] ALL [item]");
		f.TagText("h4", "TRANSPORT [unit] ALL [item] EXCEPT [amount]");
		temp = "Transport the specified items to the given target. "
			"In the second form all of the specified item is "
			"transported. "
			"In the last form, all of the specified item except "
			"for the specified amount is transported.";
		if (Globals->SHIPPING_COST > 0) {
			temp += " Long distance transportation of goods between ";
			temp += Globals->LOCAL_TRANSPORT;
			temp += AString(" and ") + Globals->NONLOCAL_TRANSPORT;
			temp += " hexes away has an associated cost.  This cost is based "
				"on the weight of the items being transported.";
			if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST) {
				temp += " At higher skill levels of the quartermaster "
					"skill, the cost for transporting goods will be less.";
			}
			if (Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST) {
				temp += " At higher skill levels of the quartermaster "
					"skill, the maximum distance goods can be transported "
					"increases over the above.";
			}
		}
		temp += " The target of the transport unit must be a unit with the "
			"quartermaster skill and must be the owner of a transport "
			"structure.";
		temp += " For long distance transport between quartermasters, the "
			"issuing unit must also be a quartermaster and be the owner of "
			"a transport structure.";
		temp += " The target's faction must have declared the issuing "
			"unit's faction Friendly.";
		// Gated like the BUILD text on BUILD_NO_TRADE: CheckTransportOrders skips the TRADE
		// ActivityCheck entirely when TRANSPORT_NO_TRADE is set (it is in NewOrigins).
		if (SomeItemsNotTransportable()) {
			temp += "  Some items cannot be transported; see the ";
			temp += f.Link("#transport_items", "list of such items") + ".";
		}
		if (!Globals->TRANSPORT_NO_TRADE) {
			temp += "  Use of this order counts as trade activity in the "
				"hex.";
		}
		f.Paragraph(temp);
		f.Paragraph("Examples:");
		temp = "Transport 10 STON to unit 1234";
		temp2 = "TRANSPORT 1234 10 STON";
		f.CommandExample(temp, temp2);
		temp = "Transport all except 10 SWOR to unit 3432";
		temp2 = "TRANSPORT 3432 ALL SWOR EXCEPT 10";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("turn");
	f.TagText("h4", "TURN");
	temp = "The TURN order may be used to delay orders by one (or more) "
		"turns. By making the TURN order repeating (via '@'), orders inside "
		"the TURN/ENDTURN construct will repeat.  Multiple TURN orders in a "
		"row will execute on successive turns, and if they all repeat, they "
		"will form a loop of orders.  Each TURN section must be ended by an "
		"ENDTURN line.";
	f.Paragraph(temp);
	// ARegion::WriteTemplate / Game::DefaultOrders: TURN blocks, queued MOVE/SAIL remainders
	// and repeating STUDY/PRODUCE orders are not stored anywhere else -- they live only as
	// lines in the next orders template. No orders (or deleted lines) means they are gone.
	temp = "TURN blocks, the remaining moves of an unfinished ";
	temp += f.Link("#move", "MOVE") + " or " + f.Link("#sail", "SAIL") +
		", and orders that carry on into the next month (such as a ";
	temp += f.Link("#study", "STUDY") + " or " + f.Link("#produce", "PRODUCE") +
		" with a target) are only kept as lines in your next orders "
		"template. If you do not send orders, or remove those lines from "
		"the orders you send, they are lost";
	if (Globals->DEFAULT_WORK_ORDER)
		temp += ", and the unit falls back to its default order";
	temp += ".";
	f.Paragraph(temp);
	f.Paragraph("Examples:");
	// With TAX_PILLAGE_MONTH_LONG, PILLAGE and ADVANCE are both month long orders, so a
	// block containing both would keep only the ADVANCE ("Overwriting previous DELAYED
	// month-long order"); the example then uses PILLAGE on its own.
	int pillage_alone = Globals->TAX_PILLAGE_MONTH_LONG;
	temp = "Study combat this month, move north next month, and then in two "
		"months, ";
	temp += pillage_alone ? "pillage the region." : "pillage and advance north.";
	temp2 = "STUDY COMB\n";
	temp2 += "TURN\n";
	temp2 += "    MOVE N\n";
	temp2 += "ENDTURN\n";
	temp2 += "TURN\n";
	temp2 += "    PILLAGE\n";
	if (!pillage_alone) temp2 += "    ADVANCE N\n";
	temp2 += "ENDTURN";
	f.CommandExample(temp, temp2);
	temp = "After the turn, the orders for that unit would look as "
		"follows in the orders template:";
	temp2 = "MOVE N\n";
	temp2 += "TURN\n";
	temp2 += "    PILLAGE\n";
	if (!pillage_alone) temp2 += "    ADVANCE N\n";
	temp2 += "ENDTURN";
	f.CommandExample(temp, temp2);
	temp = "Set up a simple cash caravan.";
	temp2 = "MOVE N\n";
	temp2 += "@TURN\n";
	temp2 += "    TAKE FROM 13794 1000 SILV\n";
	temp2 += "    MOVE S S S\n";
	temp2 += "ENDTURN\n";
	temp2 += "@TURN\n";
	temp2 += "    GIVE 13523 1000 SILV\n";
	temp2 += "    MOVE N N N\n";
	temp2 += "ENDTURN";
	f.CommandExample(temp, temp2);
	temp = "After the turn, the orders for that unit would look as "
		"follows in the orders template:";
	temp2 = "TAKE FROM 13794 1000 SILV\n";
	temp2 += "MOVE S S S\n";
	temp2 += "@TURN\n";
	temp2 += "    GIVE 13523 1000 SILV\n";
	temp2 += "    MOVE N N N\n";
	temp2 += "ENDTURN\n";
	temp2 += "@TURN\n";
	temp2 += "    TAKE FROM 13794 1000 SILV\n";
	temp2 += "    MOVE S S S\n";
	temp2 += "ENDTURN";
	f.CommandExample(temp, temp2);
	temp = "The orders in a TURN block will be inserted into the unit's orders "
			"template when there are no month long orders remaining to "
			"be executed.  In particular, if the unit does not have enough "
			"movement points to cover the full distance of a MOVE or SAIL "
			"command, the movement commands will automatically be completed "
			"over multiple turns before executing the next TURN block. In the "
			"same way, a repeating month long order (such as @WORK) keeps the "
			"next TURN block from starting.";
	f.Paragraph(temp);

	if (Globals->USE_WEAPON_ARMOR_COMMAND) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("weapon");
		f.TagText("h4", "WEAPON [item] ...");
		f.TagText("h4", "WEAPON");
		temp = "This command allows you to set a list of preferred weapons "
			"for a unit.  After searching for weapons on the preferred "
			"list, the standard weapon precedence takes effect if a weapon "
			"hasn't been set.  The second form clears the preferred weapon "
			"list.  At most four weapons can be listed, and your faction must "
			"know each of them.";
		f.Paragraph(temp);
		f.Paragraph("Examples");
		temp = "Set the unit to select double bows, then longbows then "
			"crossbows";
		temp2 = "WEAPON DBOW LBOW XBOW";
		f.CommandExample(temp, temp2);
		temp = "Clear the preferred weapon list.";
		temp2 = "WEAPON";
		f.CommandExample(temp, temp2);
	}

	if (Globals->ALLOW_WITHDRAW) {
		f.ClassTagText("div", "rule", "");
		f.LinkRef("withdraw");
		f.TagText("h4", "WITHDRAW [item]");
		f.TagText("h4", "WITHDRAW [quantity] [item]");
		temp = "Use unclaimed funds to acquire basic items that you need. "
			"If you do not have sufficient unclaimed, or if you try "
			"withdraw any other than a basic item, an error will be given. "
			"Withdraw CANNOT be used in the Nexus (to prevent building "
			"towers and such there).  The first form is the same as "
			"WITHDRAW 1 [item] in the second form.  Each item costs two and "
			"a half times its base price, paid from your unclaimed silver; "
			"silver itself cannot be withdrawn (use ";
		temp += f.Link("#claim", "CLAIM") + ").";
		f.Paragraph(temp);
		f.Paragraph("Examples:");
		temp = "Withdraw 5 stone.";
		temp2 = "WITHDRAW 5 stone";
		f.CommandExample(temp, temp2);
		temp = "Withdraw 1 iron.";
		temp2 = "WITHDRAW iron";
		f.CommandExample(temp, temp2);
	}

	f.ClassTagText("div", "rule", "");
	f.LinkRef("work");
	f.TagText("h4", "WORK");
	temp = "Spend the month performing manual work for wages.";
	if (Globals->DEFAULT_WORK_ORDER) {
		temp += " A unit that is not given any month long order works "
			"automatically";
		if (Globals->TAX_PILLAGE_MONTH_LONG) {
			temp += " (or taxes, if its ";
			temp += f.Link("#autotax", "AUTOTAX") + " flag is set)";
		}
		temp += ", except in the Nexus. To stop this, use the ";
		temp += f.Link("#idle", "IDLE") + " order.";
	}
	f.Paragraph(temp);
	f.Paragraph("Example:");
	temp = "Work all month.";
	temp2 = "WORK";
	f.CommandExample(temp, temp2);

	f.LinkRef("sequenceofevents");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Sequence of Events");
	temp = "Each turn, the following sequence of events occurs:";
	f.Paragraph(temp);
	// The order of this list mirrors Game::RunOrders (runorders.cpp), preceded by the orders
	// the parser applies while reading the orders file (parseorders.cpp) and followed by
	// PostProcessTurn. Keep it in step with RunOrders when phases are added or moved.
	f.Enclose(1, "OL");
	f.Enclose(1, "li");
	f.PutStr("Orders carried out while your orders are being read, in the "
		"order they appear in your orders (this is why, for example, silver "
		"taken with CLAIM can be used by other orders in the same turn).");
	f.Enclose(1, "ul");
	temp = f.Link("#turn", "TURN") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#form", "FORM") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#address", "ADDRESS") + ", ";
	if (Globals->USE_WEAPON_ARMOR_COMMAND)
		temp += f.Link("#armor", "ARMOR") + ", ";
	temp += f.Link("#autotax", "AUTOTAX") + ", ";
	temp += f.Link("#avoid", "AVOID") + ", ";
	temp += f.Link("#behind", "BEHIND") + ", ";
	temp += f.Link("#claim", "CLAIM") + ", ";
	temp += f.Link("#combat", "COMBAT") + ", ";
	if (Globals->FOOD_ITEMS_EXIST)
		temp += f.Link("#consume", "CONSUME") + ", ";
	temp += f.Link("#declare", "DECLARE") + ", ";
	temp += f.Link("#describe", "DESCRIBE") + ", ";
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		temp += f.Link("#faction", "FACTION") + ", ";
	temp += f.Link("#guard", "GUARD") + " 0, ";
	temp += f.Link("#hold", "HOLD") + ", ";
	temp += f.Link("#name", "NAME") + ", ";
	temp += f.Link("#noaid", "NOAID") + ", ";
	temp += f.Link("#share", "SHARE") + ", ";
	if (move_over_water)
		temp += f.Link("#nocross", "NOCROSS") + ", ";
	temp += f.Link("#option", "OPTION") + ", ";
	temp += f.Link("#password", "PASSWORD") + ", ";
	if (Globals->USE_PREPARE_COMMAND)
		temp += f.Link("#prepare", "PREPARE") + ", ";
	temp += f.Link("#reveal", "REVEAL") + ", ";
	temp += f.Link("#show", "SHOW") + ", ";
	if (!Globals->USE_WEAPON_ARMOR_COMMAND)
		temp += "and ";
	temp += f.Link("#spoils", "SPOILS");
	if (Globals->USE_WEAPON_ARMOR_COMMAND) {
		temp += ", and ";
		temp += f.Link("#weapon", "WEAPON");
	}
	temp += " orders are processed.";
	f.TagText("li", temp);
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr("Instant orders.");
	f.Enclose(1, "ul");
	temp = f.Link("#find", "FIND") + " orders are processed.";
	f.TagText("li", temp);
	// RunEnterOrders(0) handles LEAVE and ENTER in a single pass over the units.
	temp = f.Link("#leave", "LEAVE") + " and " + f.Link("#enter", "ENTER") +
		" orders are processed, unit by unit in the order the units appear "
		"in the report.";
	f.TagText("li", temp);
	temp = f.Link("#promote", "PROMOTE") + " and ";
	temp += f.Link("#evict", "EVICT");
	temp += " orders are processed.";
	f.TagText("li", temp);
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr("Combat is processed.");
	f.Enclose(1, "ul");
	temp = f.Link("#attack", "ATTACK") + " orders are processed";
	if (Globals->WANDERING_MONSTERS_EXIST) {
		// Game::DoAttackOrders also runs CheckWMonAttack for monster units.
		temp += ", and monsters decide whether to attack";
	}
	temp += ".";
	f.TagText("li", temp);
	// Game::DoAutoAttacks: every non-avoiding unit attacks units it can see and catch whose
	// faction its own faction has declared Hostile (monsters always avoid, so not them).
	temp = "Units that are not set to avoid combat attack the units they can "
		"see and catch, of factions that their faction has declared Hostile.";
	f.TagText("li", temp);
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	if (has_stea) {
		f.Enclose(1, "li");
		f.PutStr("Subterfuge orders.");
		f.Enclose(1, "ul");
		temp = f.Link("#steal", "STEAL") + " and ";
		temp += f.Link("#assassinate", "ASSASSINATE") +
			" orders are processed.";
		f.TagText("li", temp);
		f.Enclose(0, "ul");
		f.Enclose(0, "li");
	}
	f.Enclose(1, "li");
	f.PutStr("Give orders.");
	f.Enclose(1, "ul");
	temp = f.Link("#give", "GIVE") + " and " + f.Link("#take", "TAKE") +
		" orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#join", "JOIN") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#exchange", "EXCHANGE") + " orders are processed.";
	f.TagText("li", temp);
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr("Tax orders.");
	f.Enclose(1, "ul");
	temp = f.Link("#destroy", "DESTROY") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#pillage","PILLAGE") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#tax","TAX") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#guard","GUARD") + " 1 orders are processed.";
	f.TagText("li", temp);
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr("Instant Magic");
	f.Enclose(1, "ul");
	f.TagText("li", "Old spells are cancelled.");
	temp = "Spells are ";
	temp += f.Link("#cast", "CAST");
	temp += " (except for Teleportation spells).";
	f.TagText("li", temp);
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr("Market orders.");
	f.Enclose(1, "ul");
	if (Globals->TOWNS_EXIST) {
		temp = f.Link("#sell","SELL") + " orders are processed.";
		f.TagText("li", temp);
	}
	temp = f.Link("#buy","BUY") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#forget","FORGET") + " orders are processed.";
	f.TagText("li", temp);
	if (Globals->CHECK_MONSTER_CONTROL_MID_TURN) {
		// Game::MidProcessTurn -> MonsterCheck.
		f.TagText("li", "Control of summoned and controlled creatures is "
			"checked; some may escape or fade away (see the descriptions of "
			"the creatures and spells).");
	}
	temp = f.Link("#quit","QUIT") + " and ";
	temp += f.Link("#restart", "RESTART") + " orders are processed.";
	f.TagText("li", temp);
	if (Globals->ALLOW_WITHDRAW) {
		temp = f.Link("#withdraw","WITHDRAW") + " orders are processed.";
		f.TagText("li", temp);
	}
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr("Movement orders.");
	f.Enclose(1, "ul");
	temp = f.Link("#advance","ADVANCE");
	if (may_sail)
		temp += ", ";
	else
		temp += " and ";
	temp += f.Link("#move", "MOVE");
	if (may_sail) {
		temp += " and ";
		temp += f.Link("#sail","SAIL");
	}
	temp += " orders are processed phase by phase (including any combat "
		"resulting from these orders).";
	f.TagText("li", temp);
	// SinkUncrewedFleets and DrownUnits run right after RunMovementOrders.
	if (may_sail) {
		f.TagText("li", "Fleets at sea with no one aboard are lost.");
	}
	temp = "Units in an ocean region that are not aboard a fleet drown, unless "
		"they can swim";
	if (Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_UNLIMITED) {
		temp += " or fly";
	}
	temp += ".";
	f.TagText("li", temp);
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(1, "li");
	f.PutStr("Month long orders.");
	f.Enclose(1, "ul");
	temp = f.Link("#teach", "TEACH") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#idle", "IDLE") + " orders are processed.";
	f.TagText("li", temp);
	temp = f.Link("#study", "STUDY") + " orders are processed.";
	f.TagText("li", temp);
	temp = "Manufacturing ";
	temp += f.Link("#produce", "PRODUCE");
	temp += " orders (those that produce items from other items, such "
		"as using the weaponsmith skill to make swords out of iron) and ";
	temp += f.Link("#build", "BUILD") + " orders are processed together, in "
		"the order the units appear in the region.";
	f.TagText("li", temp);
	temp = "Primary ";
	temp += f.Link("#produce", "PRODUCE");
	temp += " orders (those that produce items from region resources, "
		"such as using the mining skill to produce iron) "
		"are processed.";
	f.TagText("li", temp);
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		temp = f.Link("#entertain", "ENTERTAIN") +
			" orders are processed.";
		f.TagText("li", temp);
	}
	temp = f.Link("#work", "WORK") + " orders are processed.";
	f.TagText("li", temp);
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	if (Globals->DYNAMIC_POPULATION || Globals->REGIONS_ECONOMY) {
		// Game::ProcessEconomics -> ARegion::Grow.
		temp = "The population of regions changes (see ";
		temp += f.Link("#economy_towns", "Villages, Towns, and Cities") + ").";
		f.TagText("li", temp);
	}
	temp = "Teleportation spells are ";
	temp += f.Link("#cast", "CAST") + ".";
	f.TagText("li", temp);
	if (qm_exist) {
		temp = f.Link("#transport", "TRANSPORT") + " and " +
			f.Link("#distribute", "DISTRIBUTE") + " orders are processed.";
		f.TagText("li", temp);
	}
	f.TagText("li", "Maintenance costs are assessed.");
	// Game::PostProcessTurn: victory check, ARegion::PostTurn (development, wages, markets,
	// production), AdjustCityMons, then GrowWMons/GrowLMons/GrowVMons.
	f.Enclose(1, "li");
	f.PutStr("End of the turn.");
	f.Enclose(1, "ul");
	if (!Globals->OPEN_ENDED) {
		f.TagText("li", "The victory conditions are checked.");
	}
	f.TagText("li", "Each region's development, wages, markets and "
		"production are updated for the next month.");
	if (Globals->CITY_MONSTERS_EXIST) {
		f.TagText("li", "City and town guardsmen recover their strength, "
			"or reappear.");
	}
	if (!Globals->CHECK_MONSTER_CONTROL_MID_TURN) {
		f.TagText("li", "Control of summoned and controlled creatures is "
			"checked; some may escape or fade away.");
	}
	if (Globals->WANDERING_MONSTERS_EXIST || Globals->LAIR_MONSTERS_EXIST) {
		f.TagText("li", "New monsters appear.");
	}
	f.Enclose(0, "ul");
	f.Enclose(0, "li");
	f.Enclose(0, "OL");
	temp = "Where there is no other basis for deciding in which order units "
		"will be processed within a phase, units that appear higher on the "
		"report get precedence.";
	f.Paragraph(temp);
	f.LinkRef("reportformat");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Report Format");
	// Section order and conditions mirror Faction::WriteReport (faction.cpp); a section with
	// nothing in it is not printed. There is no "Current Status" heading: the region
	// descriptions simply follow the last section.
	temp = "Your turn report contains the following parts, in this order. A "
		"section is left out when there is nothing to put in it.";
	f.Paragraph(temp);
	f.Enclose(1, "ul");
	if (Globals->FACTION_STATISTICS) {
		f.TagText("li", "A table summarising the items your faction owns "
			"(these lines start with \";\").");
	}
	f.TagText("li", "Your faction's name and type, the date, and any important "
		"messages, such as warnings.");
	if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
		f.TagText("li", "\"Faction Status\": for each limit of your faction "
			"type, how much of it you are using, with the maximum allowed in "
			"brackets.");
	}
	f.TagText("li", "\"Errors during turn\": orders that could not be carried "
		"out, and why.");
	f.TagText("li", "\"Battles during turn\": reports of any battles your units "
		"took part in.");
	f.TagText("li", "\"Events during turn\": what your units did and what "
		"happened to them.");
	f.TagText("li", "\"Skill reports\", \"Item reports\" and \"Object "
		"reports\": descriptions of skill levels, items and structures that "
		"your faction has come across for the first time.");
	f.TagText("li", "\"Declared Attitudes\": the attitudes you have declared "
		"towards other factions.");
	f.TagText("li", "\"Unclaimed silver\": your faction's unclaimed money.");
	f.TagText("li", "A description of each region in which you have units, or "
		"which you can see in some other way.");
	f.Enclose(0, "ul");
	temp = "The most important parts are the events, which list what happened "
		"last month, and the region descriptions.";
	f.Paragraph(temp);
	// Unit::WriteReport: "*" for your own units; others get "-", or with OPTION
	// SHOWATTITUDES a marker for YOUR faction's declared attitude towards theirs
	// (Object::Report passes fac->GetAttitude(u->faction->num)).
	temp = "In the region descriptions, your own units are flagged with a "
		"\"*\" character. Units belonging to other factions are flagged "
		"with a \"-\" character";
	temp += AString(", or, if you have set ") + f.Link("#option", "OPTION") +
		" SHOWATTITUDES, with a character showing your attitude towards their "
		"faction: \"=\" for Ally, \":\" for Friendly, \"-\" for Neutral, \"%\" "
		"for Unfriendly and \"!\" for Hostile. You may be informed which "
		"faction they belong to, if ";
	if (has_obse)
		temp += "you have high enough Observation skill or ";
	temp += "they are revealing that information.";
	f.Paragraph(temp);
	temp = "Objects are flagged with a \"+\" character.  The units listed "
		"under an object (if any) are inside the object.  The first unit "
		"listed under an object is its owner.";
	f.Paragraph(temp);
	temp = "If you can see a unit, you can see any large items it is "
		"carrying.  This means all items other than silver";
	if (!(ItemDefs[I_HERBS].flags & ItemType::DISABLED))
		temp += ", herbs,";
	temp += " and other small items (which are of zero size units, and are "
		"small enough to be easily concealed). Items carried by your own "
		"units of course will always be listed.";
	f.Paragraph(temp);
	temp = "At the bottom of your turn report is an Orders Template.  This "
		"template gives you a formatted orders form, with all of your "
		"units listed. You may use this to fill in your orders, or write "
		"them on your own. The ";
	temp += f.Link("#option", "OPTION") + " order gives you the option of "
		"giving more or less information in this template, or turning it "
		"off altogether. You can precede orders with an '@' sign in your "
		"orders, in which case they will appear in your template on the "
		"next turn's report.";
	f.Paragraph(temp);
	f.LinkRef("hintsfornew");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Hints for New Players");
	temp = "Make sure to use the correct #ATLANTIS and UNIT lines in your "
		"orders.";
	f.Paragraph(temp);
	temp = "Always have a month's supply of spare cash in every region in "
		"which you have units, so that even if they are deprived of "
		"income for a month (due to a mistake in your orders, for "
		"example), they will not starve to death.  It is very frustrating "
		"to have half your faction wiped out because you neglected to "
		"provide enough money for them to live on.";
	f.Paragraph(temp);
	temp = "Be conservative with your money. ";
	if (Globals->LEADERS_EXIST) {
		temp += "Leaders especially are very hard to maintain, as they "
			"cannot usually earn enough by ";
		temp += f.Link("#work", "WORK") + "ing to pay their maintenance "
			"fee. ";
	}
	temp += "Even once you have recruited men, notice that it is "
		"expensive for them to ";
	temp += f.Link("#study", "STUDY") + " (and become productive units), "
		"so be sure to save money to that end.";
	f.Paragraph(temp);
	temp = "Don't leave it until the last minute to send orders.  If "
		"there is a delay in the mailer, your orders will not arrive "
		"on time, and turns will NOT be rerun, nor will it be possible "
		"to change the data file for the benefit of players whose orders "
		"weren't there by the deadline.  If you are going to send your "
		"orders at the last minute, send a preliminary set earlier in the "
		"week so that at worst your faction will not be left with no "
		"orders at all.";
	f.Paragraph(temp);

	if (Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f.LinkRef("specialcommands");
		f.ClassTagText("div", "rule", "");
		f.TagText("h2", "Special Commands");
		temp = "These special commands have been added via the scripts "
			"processing the email to help you interact with the game "
			"and submit times and rumors. Please read over these new "
			"commands and their uses. Also note that all commands sent "
			"to the server are logged, including orders submissions, so "
			"if you have a problem, or if you attempt to abuse the system, "
			"it will get noticed and it will be tracked down.";
		f.Paragraph(temp);
		f.LinkRef("_create");
		f.ClassTagText("div", "rule", "");
		f.TagText("h4", "#create \"faction name\" \"password\"");
		temp = "This will create a new faction with the desired name and "
			"password, and it will use the player's \"from\" address as "
			"the email address of record (this, of course, can be changed "
			"from within the game).";
		f.Paragraph(temp);
		temp = "The \"\" characters are required. If they are missing, the "
			"server will not create the faction.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Join the game as the faction named \"Mighty Ducks\" with the "
			"password of \"quack\"";
		temp2="#create \"Mighty Ducks\" \"quack\"";
		f.CommandExample(temp, temp2);

		f.LinkRef("_resend");
		f.ClassTagText("div", "rule", "");
		f.TagText("h4", "#resend [faction] \"password\"");
		temp = "The faction number and your current password (if you have "
			"one) are required. The most recent turn report will be sent to "
			"the address of record.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "You are faction 999 with password \"quack\" and need another "
			"copy of the last turn (because your hard drive crashed)";
		temp2 = "#resend 999 \"quack\"";
		f.CommandExample(temp, temp2);

		f.LinkRef("_times");
		f.ClassTagText("div", "rule", "");
		f.TagText("h4", "#times [faction] \"password\"");
		f.PutStr("[body of article]");
		f.TagText("h4", "#end");
		temp = "Everything between the #times and #end lines is included "
			"in your article. Your article will be marked as being "
			"sent by your faction, so you need not include that "
			"attribution in the article.";
		if (Globals->TIMES_REWARD) {
			temp += " You will receive $";
			temp += Globals->TIMES_REWARD;
			temp += " for submitting the article.";
		}
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Faction 999 wants to declare war on everyone";
		temp2 = "#times 999 \"quack\"\n";
		temp2 += "The Mighty Ducks declare war on the world!!\n";
		temp2 += "Quack!\n";
		temp2 += "#end";
		f.CommandExample(temp, temp2);
		temp = "And it would appear something like:";
		temp2 = "---------------------------------\n";
		temp2 += "The Mighty Ducks declare war on the world!!\n";
		temp2 += "Quack!\n\n";
		temp2 += "[Article submitted by The Mighty Ducks (999)]\n";
		temp2 += "---------------------------------";
		f.CommandExample(temp, temp2);

		f.LinkRef("_rumor");
		f.ClassTagText("div", "rule", "");
		f.TagText("h4", "#rumor [faction] \"password\"");
		f.PutStr("[body of rumor]");
		f.TagText("h4", "#end");
		temp = "Submit a rumor for publication in the next news.  These "
			"articles are not attributed (unlike times articles) and will "
			"appear in the rumor section of the next news in a random order.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Start a malicious rumor";
		temp2 = "#rumor 999 \"quack\"\n";
		temp2 += "Oleg is a running-dog lackey of Azthar Spleenmonger.\n";
		temp2 += "#end";
		f.CommandExample(temp, temp2);

		f.LinkRef("_remind");
		f.ClassTagText("div", "rule", "");
		f.TagText("h4", "#remind [faction] \"password\"");
		temp = "This order will have the server find the most recent set of "
			"orders you have submitted for the current turn and mail them "
			"back to your address of record.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Remind faction 999 of its last order set.";
		temp2 = "#remind 999 \"quack\"";
		f.CommandExample(temp, temp2);

		f.LinkRef("_email");
		f.ClassTagText("div", "rule", "");
		f.TagText("h4", "#email [unit]");
		f.PutStr("[text of email]");
		temp = "This command allows you to send email to the owner of a unit "
			"even when you cannot see that unit's faction affiliation.  You "
			"will not be told who the unit belongs to, but will simply "
			"forward your email to them. When you use this command, they "
			"will receive YOUR email and can contact you if they choose. It "
			"is provided simply as a courtesy to players to help with "
			"diplomacy in first contact situations.";
		f.Paragraph(temp);
		temp = "There is no need for a \"#end\" line (such as is used in "
			"times and rumor submissions -- the entire email message you "
			"send will be forwarded to the unit's master.";
		f.Paragraph(temp);
		f.Paragraph("Example:");
		temp = "Send an email to the owner of unit 9999";
		temp2 = "#email 9999\n";
		temp2 += "Greetings.  You've entered the Kingdom of Foo.\n";
		temp2 += "Please contact us.\n\n";
		temp2 += "Lord Foo\n";
		temp2 += "foo@some.email";
		f.CommandExample(temp, temp2);
		temp = "Faction X, the owner of 9999 would receive:";
		temp2 = "From: Foo &lt;foo@some.email&gt;\n";
		temp2 += "Subject:  Greetings!\n\n";
		temp2 += "#email 9999\n";
		temp2 += "Greetings.  You've entered the Kingdom of Foo.\n";
		temp2 += "Please contact us.\n\n";
		temp2 += "Lord Foo\n";
		temp2 += "foo@some.email";
		f.CommandExample(temp, temp2);
	}
	f.LinkRef("credits");
	f.ClassTagText("div", "rule", "");
	f.TagText("h2", "Credits");
	temp = "Atlantis was originally created and programmed by Russell "
		"Wallace. Russell Wallace created Atlantis 1.0, and partially "
		"designed Atlantis 2.0 and Atlantis 3.0.";
	f.Paragraph(temp);
	temp = "Geoff Dunbar designed and programmed Atlantis 2.0, 3.0, and 4.0 "
		"up through version 4.0.4 and created the Atlantis Project to "
		"freely release and maintain the Atlantis source code.";
	f.Paragraph(temp);
	temp = "Larry Stanbery created the Atlantis 4.0.4+ derivative.";
	f.Paragraph(temp);
	temp = f.Link("mailto:jtraub@dragoncat.net", "JT Traub");
	temp += " took over the source code and merged the then forking versions "
		"of 4.0.4c and 4.0.4+ back into 4.0.5 along with modifications of his "
		"own and has been maintaining the code.";
	f.Paragraph(temp);
	temp = "Development of the code is open and there is a egroup devoted to "
		"it located at ";
	temp += f.Link("http://groups.yahoo.com/group/atlantisdev",
			"The YahooGroups AtlantisDev egroup");
	temp += ". Please join this egroup if you work on the code and share your "
		"changes back into the codebase as a whole";
	f.Paragraph(temp);
	temp = "Please see the CREDITS file in the source distribution for a "
		"complete (hopefully) list of all contributors.";
	f.Paragraph(temp);
	f.Enclose(0, "body");
	f.Enclose(0, "html");

	f.Close();

	return 1;
}

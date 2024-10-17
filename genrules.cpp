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

#include "game.h"
#include "gamedata.h"
#include "indenter.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

using namespace std;

struct tag_data { string tag; bool open;};
static inline tag_data enclose(string tag, bool open) { return { tag, open }; }

template<typename _CharT, typename _Traits>
static inline basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& os, tag_data data) {
	if (!data.open) os << indent::decr;
	os << "<" << (data.open ? "" : "/") << data.tag << ">" << '\n';
	if (data.open) os << indent::incr;
    return os;
}

struct pre_data { bool open;};
static inline pre_data pre(bool open) { return { open }; }

template<typename _CharT, typename _Traits>
static inline basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& os, pre_data data) {
	if (!data.open) {
		os << indent::pop_indent();
		os << indent::wrap(78, 70, 0);
	}
	os << enclose("pre", data.open);
	if (data.open) {
		os << indent::push_indent(0);
		os << indent::wrap(70, 30, 2);
	}
    return os;
}

static inline string class_tag(string tag, string css_class) { return tag + " class=\"" + css_class + "\""; }
static inline string anchor(string name) { return "<a name=\"" + name + "\"></a>"; }
static inline string url(string href, string text) { return "<a href=\"" + href + "\">" + text + "</a>"; }

struct example_data { string header; bool start; };
static inline example_data example_start(string header) { return { header, true }; }
static inline example_data example_end() { return { "", false }; }

template<typename _CharT, typename _Traits>
static inline basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& os, example_data data) {
	if (data.start) {
		os << enclose("p", true) << data.header << '\n' << enclose("p", false);
		os << enclose("p", true) << '\n' << enclose("p", false);
		os << pre(true);
		os << indent::suppress_format(true);
	} else {
		os << indent::suppress_format(false);
		os << pre(false);
	}
	return os;
}

static string inline num_to_word(int n) {
	static string vals [] = {
		"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
		"ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen",
		"seventeen", "eighteen", "nineteen", "twenty"
	};
	if (n > 20 || n < 0) return to_string(n);
	return vals[n];
}

static int study_rate(int days, int exp) {
	static SkillList sl;
	sl.SetDays(1,days);
	sl.SetExp(1,exp);
	return sl.GetStudyRate(1, 1);
}

inline string faction_point_usage(Faction& fac, bool verbose=true) {
	string seperator;
	stringstream ss;
	for (const auto& [key, value] : fac.type) {
		if (value <= 0) continue;
		if (verbose) {
			ss << seperator << to_string(value) << " " << plural(value, "point", "points") << " on " << key;
		} else {
			ss << seperator << key << " " << to_string(value);
		}
		seperator = (verbose ? ", " : " ");
	}
	return ss.str();
}

// This function is an utter abomination capable of summoning eldritch horrors from beyond.   This needs to
// be cleaned up, but that can wait until after this pass.  It's just too messy to fix right now.
inline string unit_tax_description() {
	string temp, temp2, temp3;
	int prev = 0, hold = 0;

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
			string temp3;
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
			string temp3;
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
	return temp;
}

string Game::FactionTypeDescription(Faction &fac) {
	stringstream buffer;
	std::vector<std::string> missingTypes;
	std::vector<std::string> types;

	int nm = AllowedMages(&fac);
	int na = AllowedApprentices(&fac);
	int nq = AllowedQuarterMasters(&fac);
	int nt = AllowedTrades(&fac);
	int nw = AllowedTaxes(&fac);
	int nma = AllowedMartial(&fac);

	for (auto &key : *FactionTypes) (fac.type[key] > 0 ? types : missingTypes).push_back(key);

	int count = 0;
	for (auto &key : types) {
		buffer << (count > 0 ? (count == (int)types.size() - 1 ? ", and " : ", ") : "");
		if (key == F_WAR) buffer << "tax " << nw << " " << plural(nw, "region", "regions");
		if (key == F_TRADE) buffer << "perform trade in " << nt << " " << plural(nt, "region", "regions");
		if (key == F_MAGIC) {
			buffer << "have " << nm << " " << plural(nm, "mage", "mages");
		}
		if (key == F_MARTIAL) {
			buffer << "perform tax " << (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL ? "or" : "and")
			       << " trade in " << nma << " " << plural(nma, "region", "regions");
		}
		count++;
	}

	// This has to be down here because at least one ruleset (Havilah) allows 1 apprentice even at magic 0
	if (Globals->APPRENTICES_EXIST && na > 0)
		buffer << ", as well have " << na << " " << Globals->APPRENTICE_NAME << (na > 1 ? "s" : "");
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT && nq > 0)
		buffer << ", and " << nq << " " << plural(nq, "quartermaster", "quartermasters");

	if (missingTypes.size() > 0) buffer << ", but ";

	count = 0;
	for (auto &fp : missingTypes) {
		buffer << (count > 0 ? (count == (int)missingTypes.size() - 1 ? ", and " : ", ") : "");
		if (fp == F_WAR) buffer << "could not perform tax in any regions";
		if (fp == F_TRADE) buffer << "could not perform trade in any regions";
		if (fp == F_MAGIC) buffer << "could not possess any mages";
		if (fp == F_MARTIAL) buffer << "could not perform tax or trade in regions";
		count++;
	}

	if (Globals->APPRENTICES_EXIST && na <= 0)
		buffer << ", as well could not possess any" << Globals->APPRENTICE_NAME << "s";
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT && nq <= 0)
		buffer << ", and could not possess any quartermasters";

	return buffer.str();
}

// LLS - converted HTML tags to lowercase
int Game::GenRules(const AString &rules, const AString &css, const AString &intro)
{
	Faction fac;
	bool found;
	SkillType *pS;

	ofstream f(rules.const_str(), ios::out|ios::trunc);
	if (!f.is_open()) return 0;

	ifstream introf(intro.const_str(), ios::in);
	if (!introf.is_open()) return 0;

	// Perform a number of ruleset sanity checks to make sure we don't output rules for something which is not truly
	// enabled in a consistent manner.
	bool may_sail = (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED)) &&
		(!(SkillDefs[S_SHIPBUILDING].flags & SkillType::DISABLED));
	bool move_over_water = (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE);
	bool has_stea = !(SkillDefs[S_STEALTH].flags & SkillType::DISABLED);
	bool has_obse = !(SkillDefs[S_OBSERVATION].flags & SkillType::DISABLED);
	bool has_annihilate = !(SkillDefs[S_ANNIHILATION].flags & SkillType::DISABLED);

	bool app_exist = (Globals->APPRENTICES_EXIST);
	bool qm_exist = (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT);

	if (qm_exist) {
		/* Make sure the S_QUARTERMASTER skill is enabled */
		if (SkillDefs[S_QUARTERMASTER].flags & SkillType::DISABLED)
			qm_exist = false;
	}
	bool found_qm_building = false;
	bool has_sacrifice = false;

	for (int i = 0; i < NOBJECTS; i++) {
		if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
		// for QM we need at least one building with the TRANSPORT flag
		if (ObjectDefs[i].flags & ObjectType::TRANSPORT) {
			found_qm_building = true;
		}
		// for SACRIFICE we need at least one building with the SACRIFICE flag
		if (ObjectDefs[i].flags & ObjectType::SACRIFICE) {
			has_sacrifice = true;
		}
	}
	if (qm_exist && !found_qm_building) qm_exist = false;

	if (app_exist) {
		found = false;
		/* Make sure we have a skill with the APPRENTICE flag */
		for (int i = 0; i < NSKILLS; i++) {
			if (SkillDefs[i].flags & SkillType::DISABLED) continue;
			if (SkillDefs[i].flags & SkillType::APPRENTICE) {
				found = true;
				break;
			}
		}
		app_exist = found;
	}

	if (!move_over_water) {
		for (int i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (ItemDefs[i].swim > 0) move_over_water = true;
		}
	}

	f << indent::wrap(78, 70, 0);

	f << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
	  << endl;
	f << enclose("html", true);

	f << enclose("head", true);
	f << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
	f << "<link type=\"text/css\" rel=\"stylesheet\" href=\"" << css << "\">\n";
	f << enclose("title", true) << Globals->RULESET_NAME << " "
	  << ATL_VER_STR(Globals->RULESET_VERSION) << " Rules\n" << enclose("title", false);
	f << enclose("head", false);

	f << enclose("body", true);
	f << enclose("center", true);
	f << enclose("h1", true) << "Rules for " << Globals->RULESET_NAME << " "
	  << ATL_VER_STR(Globals->RULESET_VERSION) << '\n' << enclose("h1", false);

	f << enclose("h1", true) << "Based on Atlantis v" << ATL_VER_STR(CURRENT_ATL_VER) << '\n' << enclose("h1", false);
	f << enclose("h2", true) << "Copyright 1996 by Geoff Dunbar\n" << enclose("h2", false);
	f << enclose("h2", true) << "Based on Russell Wallace's Draft Rules\n" << enclose("h2", false);
	f << enclose("h2", true) << "Copyright 1993 by Russell Wallace\n" << enclose("h2", false);
	auto t = time(nullptr);
	auto tm = *localtime(&t);
	f << enclose("h3", true) << "Last Change: " << put_time(&tm, "%B %d, %Y") << '\n' << enclose("h3", false);
	f << enclose("center", false);

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("p", true) << "Note: This document is subject to change, as errors are found and corrected, and "
	  << "rules sometimes change. Be sure you have the latest available copy.\n" << enclose("p", false);

	f << anchor("table_of_contents") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);

	f << enclose("h2", true) << "Table of Contents\n" << enclose("h2", false);
	f << enclose("p", true) << "Thanks to " << url("mailto:ken@satori.gso.uri.edu", "Kenneth Casey")
	  << " for putting together this table of contents.\n" << enclose("p", false);
	f << enclose("p", true) << '\n' << enclose("p", false);

	f << enclose("ul", true);
	f << enclose("li", true) << url("#intro", "Introduction") << '\n' << enclose("li", false);

	f << enclose("li", true);
	f << url("#playing", "Playing Atlantis") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#playing_factions", "Factions") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#playing_units", "Units") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#playing_turns", "Turns") << '\n' << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true);
	f << url("#world", "The World") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#world_regions", "Regions") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#region_resources", "Region Resources") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#world_structures", "Structures") << '\n' << enclose("li", false);
	if (Globals->NEXUS_EXISTS) {
		f << enclose("li", true) << url("#world_nexus", "Atlantis Nexus") << '\n' << enclose("li", false);
	}
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true);
	f << url("#movement", "Movement") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#movement_normal", "Normal Movement") << '\n' << enclose("li", false);
	if (!(SkillDefs[S_SAILING].flags & SkillType::DISABLED))
		f << enclose("li", true) << url("#movement_sailing", "Sailing") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#movement_order", "Order of Movement") << '\n' << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true);
	f << url("#skills", "Skills") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#skills_limitations", "Limitations") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#skills_studying", "Studying") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#skills_teaching", "Teaching") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#skills_skillreports", "Skill Reports") << '\n' << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true);
	f << url("#economy", "The Economy") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#economy_maintenance", "Maintenance Costs") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#economy_recruiting", "Recruiting") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#economy_items", "Items") << '\n' << enclose("li", false);
	if (Globals->TOWNS_EXIST)
		f << enclose("li", true) << url("#economy_towns", "Villages, Towns, Cities") << '\n' << enclose("li", false);
	f << enclose("li", true)
	  << url("#economy_buildings", "Buildings and Trade Structures") << '\n'
	  << enclose("li", false);
	if (!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED))
		f << enclose("li", true) << url("#economy_roads", "Roads") << '\n' << enclose("li", false);
	if (Globals->DECAY)
		f << enclose("li", true) << url("#economy_builddecay", "Building Decay") << '\n' << enclose("li", false);
	if (may_sail)
		f << enclose("li", true) << url("#economy_ships", "Ships") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#economy_advanceditems", "Advanced Items") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#economy_income", "Income") << '\n' << enclose("li", false);
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f << enclose("li", true) << url("#economy_entertainment", "Entertainment") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#economy_taxingpillaging", "Taxing/Pillaging") << '\n' << enclose("li", false);
	if (qm_exist)
		f << enclose("li", true) << url("#economy_transport", "Transporting goods") << '\n' << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true);
	f << url("#com", "Combat") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#com_attitudes", "Attitudes") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#com_attacking", "Attacking") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#com_muster", "The Muster") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#com_thebattle", "The Battle") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#com_victory", "Victory!") << '\n' << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);

	if (has_stea || has_obse) {
		f << enclose("li", true);
		f << url("#stealthobs", has_stea ? (has_obse ? "Stealth and Observation" : "Stealth") : "Observation") + '\n';
		if (has_stea) {
			f << enclose("ul", true);
			f << enclose("li", true) << url("#stealthobs_stealing", "Stealing") << '\n' << enclose("li", false);
			f << enclose("li", true) << url("#stealthobs_assassination", "Assassination") << '\n'
			  << enclose("li", false);
			f << enclose("ul", false);
		}
		f << enclose("li", false);
	}

	f << enclose("li", true);
	f << url("#magic", "Magic") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#magic_skills", "Magic Skills") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#magic_foundations", "Foundations") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#magic_furtherstudy", "Further Magic Study") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#magic_usingmagic", "Using Magic") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#magic_incombat", "Mages In Combat") << '\n' << enclose("li", false);
	if (app_exist) {
		string ref = "#magic_" + string(Globals->APPRENTICE_NAME) + "s";
		string text = string(Globals->APPRENTICE_NAME) + "s";
		text[0] = toupper(text[0]);
		f << enclose("li", true) << url(ref, text) << '\n' << enclose("li", false);
	}
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true);
	f << url("#nonplayers", "Non-Player Units") << '\n';
	f << enclose("ul", true);
	if (Globals->TOWNS_EXIST && Globals->CITY_MONSTERS_EXIST) {
		f << enclose("li", true) << url("#nonplayers_guards", "City and Town Guardsmen") << '\n'
		  << enclose("li", false);
	}
	if (Globals->WANDERING_MONSTERS_EXIST) {
		f << enclose("li", true) << url("#nonplayers_monsters", "Wandering Monsters") << '\n'
		  << enclose("li", false);
	}
	f << enclose("li", true) << url("#nonplayers_controlled", "Controlled Monsters") << '\n' << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true);
	f << url("#orders", "Orders") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#orders_abbreviations", "Abbreviations") << '\n' << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true);
	f << url("#ordersummary", "Order Summary") << '\n';
	f << enclose("ul", true);
	f << enclose("li", true) << url("#address", "address") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#advance", "advance") << '\n' << enclose("li", false);
	if (has_annihilate)
		f << enclose("li", true) << url("#annihilate", "annihilate") << '\n' << enclose("li", false);
	if (Globals->USE_WEAPON_ARMOR_COMMAND)
		f << enclose("li", true) << url("#armor", "armor") << '\n' << enclose("li", false);
	if (has_stea)
		f << enclose("li", true) << url("#assassinate", "assassinate") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#attack", "attack") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#autotax", "autotax") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#avoid", "avoid") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#behind", "behind") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#build", "build") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#buy", "buy") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#cast", "cast") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#claim", "claim") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#combat", "combat") << '\n' << enclose("li", false);
	if (Globals->FOOD_ITEMS_EXIST)
		f << enclose("li", true) << url("#consume", "consume") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#declare", "declare") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#describe", "describe") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#destroy", "destroy") << '\n' << enclose("li", false);
	if (qm_exist)
		f << enclose("li", true) << url("#distribute", "distribute") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#enter", "enter") << '\n' << enclose("li", false);
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f << enclose("li", true) << url("#entertain", "entertain") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#evict", "evict") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#exchange", "exchange") << '\n' << enclose("li", false);
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		f << enclose("li", true) << url("#faction", "faction") << '\n' << enclose("li", false);
	if (Globals->HAVE_EMAIL_SPECIAL_COMMANDS)
		f << enclose("li", true) << url("#find", "find") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#forget", "forget") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#form", "form") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#give", "give") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#guard", "guard") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#hold", "hold") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#join", "join") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#leave", "leave") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#move", "move") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#name", "name") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#noaid", "noaid") << '\n' << enclose("li", false);
	if (move_over_water)
		f << enclose("li", true) << url("#nocross", "nocross") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#option", "option") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#password", "password") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#pillage", "pillage") << '\n' << enclose("li", false);
	if (Globals->USE_PREPARE_COMMAND)
		f << enclose("li", true) << url("#prepare", "prepare") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#produce", "produce") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#promote", "promote") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#quit", "quit") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#restart", "restart") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#reveal", "reveal") << '\n' << enclose("li", false);
	if (has_sacrifice)
		f << enclose("li", true) << url("#sacrifice", "sacrifice") << '\n' << enclose("li", false);
	if (may_sail)
		f << enclose("li", true) << url("#sail", "sail") << '\n' << enclose("li", false);
	if (Globals->TOWNS_EXIST)
		f << enclose("li", true) << url("#sell", "sell") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#share", "share") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#show", "show") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#spoils", "spoils") << '\n' << enclose("li", false);
	if (has_stea)
		f << enclose("li", true) << url("#steal", "steal") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#study", "study") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#take", "take") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#tax", "tax") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#teach", "teach") << '\n' << enclose("li", false);
	if (qm_exist)
		f << enclose("li", true) << url("#transport", "transport") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#turn", "turn") << '\n' << enclose("li", false);
	if (Globals->USE_WEAPON_ARMOR_COMMAND)
		f << enclose("li", true) << url("#weapon", "weapon") << '\n' << enclose("li", false);
	if (Globals->ALLOW_WITHDRAW)
		f << enclose("li", true) << url("#withdraw", "withdraw") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#work", "work") << '\n' << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);

	f << enclose("li", true) << url("#sequenceofevents", "Sequence of Events") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#reportformat", "Report Format") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#hintsfornew", "Hints for New Players") << '\n' << enclose("li", false);
	if (Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f << enclose("li", true);
		f << url("#specialcommands", "Special Commands") << '\n';
		f << enclose("ul", true);
		f << enclose("li", true) << url("#_create", "#Create") << '\n' << enclose("li", false);
		f << enclose("li", true) << url("#_resend", "#Resend") << '\n' << enclose("li", false);
		f << enclose("li", true) << url("#_times", "#Times") << '\n' << enclose("li", false);
		f << enclose("li", true) << url("#_rumor", "#Rumor") << '\n' << enclose("li", false);
		f << enclose("li", true) << url("#_remind", "#Remind") << '\n' << enclose("li", false);
		f << enclose("li", true) << url("#_email", "#Email") << '\n' << enclose("li", false);
		f << enclose("ul", false);
		f << enclose("li", false);
	}
	f << enclose("li", true) << url("#credits", "Credits") << '\n' << enclose("li", false);
	f << enclose("ul", false);

	f << enclose("p", true) << "Index of Tables\n" << enclose("p", false);
	f << enclose("p", true) << '\n' << enclose("p", false);
	f << enclose("ul", true);
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
		f << enclose("li", true) << url("#tablefactionpoints", "Table of Faction Points") << '\n'
		  << enclose("li", false);
	f << enclose("li", true) << url("#tableitemweights", "Table of Item Weights") << '\n' << enclose("li", false);
	if (may_sail)
		f << enclose("li", true) << url("#tableshipcapacities", "Table of Ship Capacities") << '\n'
		  << enclose("li", false);
	if (Globals->RACES_EXIST)
		f << enclose("li", true) << url("#tableraces", "Table of Races") << '\n' << enclose("li", false);
	if (Globals->REQUIRED_EXPERIENCE)
		f << enclose("li", true) << url("#studyprogress", "Table of Study Progress") << '\n'
		  << enclose("li", false);
	f << enclose("li", true) << url("#tableiteminfo", "Table of Item Information") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#tablebuildings", "Table of Buildings") << '\n' << enclose("li", false);
	f << enclose("li", true) << url("#tabletradestructures", "Table of Trade Structures") << '\n'
	  << enclose("li", false);
	if (!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED))
		f << enclose("li", true) << url("#tableroadstructures", "Table of Road Structures") << '\n'
		  << enclose("li", false);
	if (may_sail)
		f << enclose("li", true) << url("#tableshipinfo", "Table of Ship Information") << '\n'
		  << enclose("li", false);
	if (Globals->LIMITED_MAGES_PER_BUILDING)
		f << enclose("li", true) << url("#tablemagebuildings", "Table of Mages/Building") << '\n'
		  << enclose("li", false);
	f << enclose("ul", false);

	f << anchor("intro") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Introduction\n" << enclose("h2", false);
	while (!introf.eof()) {
		string in;
		getline(introf >> ws, in);
		f << in << '\n';
	}

	f << anchor("playing") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Playing Atlantis\n" << enclose("h2", false);
	f << enclose("p", true) << "Atlantis (as you undoubtedly already know) is a play by email game.  When you "
	  << "sign up for Atlantis, you will be sent a turn report (via email).  Your report completely details "
	  << "your position in the game.  After going over this report, and possibly communicating with other players "
	  << "in the game, you determine your course of action, and create a file of \"orders\", which you then send "
	  << "back to the Atlantis server. Then, at a regular interval (often one week), Atlantis collects all the "
	  << "orders, runs another turn (covering one month in game time), and sends all the players another report.\n"
	  << enclose("p", false);

	f << anchor("playing_factions") << '\n';
	f << enclose("h3", true) << "Factions:\n" << enclose("h3", false);
	f << enclose("p", true) << "A player's position is called a \"faction\".  Each faction has a name and a "
	  << "number (the number is assigned by the computer, and used for entering orders). Each player is allowed "
	  << "to play one and ONLY one faction at any given time. Each faction is composed of a number of \"units\", "
	  << "each unit being a group of one or more people loyal to the faction.  You start the game with a single "
	  << "unit consisting of one character, plus a sum of money.  More people can be hired during the course of "
	  << "the game, and formed into more units.  (In these rules, the word \"character\" generally refers either "
	  << "to a unit consisting of only one person, or to a person within a larger unit.)\n"
	  << enclose("p", false);
	f << enclose("p", true) << "A faction is considered destroyed, and the player knocked out of the game, if ever "
	  << "all its people are killed or disbanded (i.e. the faction has no units left).  The program does not "
	  << "consider your starting character to be special; if your starting character gets killed, you will probably "
	  << "have been thinking of that character as the leader of your faction, so some other character can be "
	  << "regarded as having taken the dead leader's place (assuming of course that you have at least one surviving "
	  << "unit!).  As far as the computer is concerned, as long as any unit of the faction survives, the faction is "
	  << "not wiped out.  (If your faction is wiped out, you can rejoin the game with a new starting character.)\n"
	  << enclose("p", false);

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		f << enclose("p", true) << "A faction has one pre-set limit; it may not contain more than ";
		f << AllowedMages(&fac) << " mages";
		if (app_exist) {
			f << " and " << AllowedApprentices(&fac) << ' ' << Globals->APPRENTICE_NAME << 's';
		}
		f << ". Magic is a rare art, and only a few in the world can master it. Aside from that, there  is no limit "
		  << "to the number of units a faction may contain, nor to how many items can be produced or regions taxed.\n"
		  << enclose("p", false);
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f << enclose("p", true) << "Each faction has a type which is decided by the player, and determines what the "
		  << "faction may do.  The faction has " << Globals->FACTION_POINTS << " Faction Points, which may be spent on "
		  << "the Faction Areas of "
		  << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "War, Trade, and Magic" : "Martial and Magic")
		  << ".  The faction type may be changed at the beginning of each turn, so a faction can change and adapt to "
		  << "the conditions around it.  Faction Points spent on "
		  << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "War" : "Martial")
		  << " determine the number of regions in which factions can obtain income by taxing or pillaging";
		if (Globals->TACTICS_NEEDS_WAR) {
			f << ", and also determines the number of level 5 tactics leaders (tacticians) that a faction can train";
		}
		f << ". Faction Points spent on "
		  << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "Trade" : "Martial also") << " determine "
		  << "the number of regions in which a faction may conduct trade activity. Trade activity includes "
		  << "producing goods and materials";
		if (!Globals->TRANSPORT_NO_TRADE && qm_exist) {
			f << (!Globals->BUILD_NO_TRADE ? ", " : " and ") << " transporting items";
		}
		if (!Globals->BUILD_NO_TRADE) {
			f << (!Globals->TRANSPORT_NO_TRADE && qm_exist ? ", and " : " and ") << "constructing ships and buildings";
		}
		f << '.';
		if (qm_exist) {
			f << " Faction points spent on "
			  << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "Trade" : "Martial") << " also "
			  << "determine the number of quartermaster units a faction can have.";
		}
		if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL) {
			f << " It is important to remember that taxing/pillaging in a region and conducting trade activity "
			  << "in the same region  would use up two of the faction's allowed Martial regions.";
		}
		f << " Faction Points spent on Magic determine the number of mages";
		if (app_exist) {
			f << " and " << Globals->APPRENTICE_NAME << 's';
		}
		f << " the faction may have (more information on all of the faction activities is in further sections of the "
		  << "rules). Here is a chart detailing the limits on factions by Faction Points:\n"
		  << enclose("p", false);

		f << anchor("tablefactionpoints") << '\n';
		f << enclose("center", true);
		f << enclose("table border=\"1\"", true);
		f << enclose("tr", true);

		f << enclose("th", true) << "Faction Points\n" << enclose("th", false);
		for (auto &fp : *FactionTypes) {
			if (fp == F_WAR) {
				f << enclose("th", true)
				  << "War (max tax regions"
				  <<  (Globals->TACTICS_NEEDS_WAR ? " / tacticians" : "")
				  << ")\n"
				  << enclose("th", false);
			}

			if (fp == F_TRADE) {
				f << enclose("th", true)
				  << "Trade (max trade regions"
				  << (qm_exist ? " / quartermasters" : "")
				  << ")\n"
				  << enclose("th", false);
			}

			if (fp == F_MARTIAL) {
				f << enclose("th", true)
				  << "Martial (max tax "
				  << (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED ? "and" : "or")
				  << " trade regions"
				  << (qm_exist ? " / quartermasters" : "")
				  << (Globals->TACTICS_NEEDS_WAR ? " / tacticians" : "")
				  << ")\n"
				  << enclose("th", false);
			}

			if (fp == F_MAGIC) {
				f << enclose("th", true)
				  << "Magic (max mages"
				  << (app_exist ? " / " + string(Globals->APPRENTICE_NAME) + "s" : "")
				  << ")\n"
				  << enclose("th", false);
			}
		}
		f << enclose("tr", false);

		for (auto i = 0; i <= Globals->FACTION_POINTS; i++) {
			for (auto &fp : *FactionTypes) {
				fac.type[fp] = i;
			}

			f << enclose("tr", true);
			f << enclose("td align=\"center\" nowrap", true) << i << '\n' << enclose("td", false);

			for (auto &fp : *FactionTypes) {
				if (fp == F_WAR) {
					f << enclose("td align=\"center\" nowrap", true) << to_string(AllowedTaxes(&fac))
					  << (Globals->TACTICS_NEEDS_WAR ? " / " + to_string(AllowedTacticians(&fac)) : "")
					  << '\n' << enclose("td", false);
				}

				if (fp == F_TRADE) {
					f << enclose("td align=\"center\" nowrap", true) << to_string(AllowedTrades(&fac))
					  << (qm_exist ? " / " + to_string(AllowedQuarterMasters(&fac)) : "")
					  << '\n' << enclose("td", false);
				}

				if (fp == F_MARTIAL) {
					f << enclose("td align=\"center\" nowrap", true) << to_string(AllowedMartial(&fac))
					  << (qm_exist ? " / " + to_string(AllowedQuarterMasters(&fac)) : "")
					  << (Globals->TACTICS_NEEDS_WAR ? " / " + to_string(AllowedTacticians(&fac)) : "")
					  << '\n' << enclose("td", false);
				}

				if (fp == F_MAGIC) {
					f << enclose("td align=\"center\" nowrap", true) << to_string(AllowedMages(&fac))
					  << (app_exist ? " / " + to_string(AllowedApprentices(&fac)) : "")
					  << '\n' << enclose("td", false);
				}
			}
			f << enclose("tr", false);
		}

		f << enclose("table", false);
		f << enclose("center", false);
		f << "<P></P>\n";

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

		f << enclose("p", true) << "For example, a well rounded faction might spend " << faction_point_usage(fac)
		  << ". This faction's type would appear as \"" << faction_point_usage(fac, false)
		  << "\", and would be able to " << FactionTypeDescription(fac) << ".\n"
		  << enclose("p", false);

		if (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT) {
			fac.type[F_WAR] = Globals->FACTION_POINTS;
			fac.type[F_TRADE] = 0;
		} else {
			fac.type[F_MARTIAL] =  Globals->FACTION_POINTS;
		}
		fac.type[F_MAGIC] = 0;

		f << enclose("p", true) << "As another example, a specialized faction might spend all "
		  << faction_point_usage(fac) << ". This faction's type would appear as \""
		  << faction_point_usage(fac, false) << "\", and would be able to " << FactionTypeDescription(fac) << ".\n"
		  << enclose("p", false);

		if (Globals->FACTION_POINTS>3) {
			int rem = Globals->FACTION_POINTS - 3;
			f << enclose("p", true) << "Note that it is possible to have a faction type with less than "
			  << Globals->FACTION_POINTS << " points spent. In fact, a starting faction has one point spent "
			  << "on each of ";
			for (auto &fp : *FactionTypes) f << fp + ", ";
			f << "leaving " << rem << " "
			  << plural(rem, "point", "points") << " unspent.\n"
			  << enclose("p", false);
		}
	}

	f << enclose("p", true) << "When a faction starts the game, it is given a one-man unit and "
	  << Globals->START_MONEY << " silver in unclaimed money.  Unclaimed money is cash that your "
	  << "whole faction has access to, but cannot be taken away in battle (silver in a unit's possessions can be "
	  << "taken in battle).  This allows a faction to get started without presenting an enticing target for other "
	  << "factions. Units in your faction may use the " << url("#claim", "CLAIM") << " order to take this silver, "
	  << "and use it to buy goods or recruit men";

	if (Globals->ALLOW_WITHDRAW)
		f << ", or use the " << url("#withdraw", "WITHDRAW") << " order to withdraw goods directly";
	f << ".\n" << enclose("p", false);

	f << enclose("p", true) << "An example faction is shown below, consisting of a starting character, Merlin the "
	  << "Magician, who has formed two more units, Merlin's Guards and Merlin's Workers. Each unit is assigned a "
	  << "unit number by the computer (completely independent of the faction number); this is used for entering "
	  << "orders. Here, the player has chosen to give his faction the same name (\"Merlin the Magician\") as his "
	  << "starting character. Alternatively, you can call your faction something like \"The Great Northern Mining "
	  << "Company\" or whatever.\n" << enclose("p", false);
	f << enclose("p", true) << "\n" << enclose("p", false);
	f << pre(true);
	if (Globals->LEADERS_EXIST) {
		f << "* Merlin the Magician (17), Merlin (27), leader [LEAD].  Skills: none.\n";
	} else {
		f << "* Merlin the Magician (17), Merlin (27), man [MAN].  Skills: none.\n";
	}
	if (Globals->RACES_EXIST) {
		f << "* Merlin's Guards (33), Merlin (27), 20 vikings [VIKI], 20 swords [SWOR]. Skills: none.\n";
		f << "* Merlin's Workers (34), Merlin (27), 50 vikings [VIKI].  Skills: none.\n";
	} else {
		f << "* Merlin's Guards (33), Merlin (27), 20 men [MAN], 20 swords [SWOR]. Skills: none.\n";
		f << "* Merlin's Workers (34), Merlin (27), 50 men [MAN].  Skills: none.\n";
	}
	f << pre(false);
	f << anchor("playing_units") << '\n';
	f << enclose("h3", true) << "Units:\n" << enclose("h3", false);
	f << enclose("p", true) << "A unit is a grouping together of people, all loyal to the same faction. The people in "
	  << "a unit share skills and possessions, and execute the same orders each month. The reason for having units of "
	  << "many people, rather than keeping track of individuals, is to simplify the game play.  The computer does "
	  << "not keep track of individual names, possessions, or skills for people in the same unit, and all the people "
	  << "in a particular unit must be in the same place at all times.  If you want to send people in the same unit "
	  << "to different places, you must split up the unit.  Apart from this, there is little difference between "
	  << "having one unit of 50 people, or 50 units of one person each, except that the former is very much easier "
	  << "to handle.\n" << enclose("p", false);

	f << enclose("p", true);
	if (Globals->RACES_EXIST) {
		f << "There are different races that make up the population of Atlantis. (See the section on skills for "
		  << "a list of these.)";
		if (Globals->LEADERS_EXIST) {
			f << " In addition, there are \"leaders\", who are presumed to be of one of the other races, but are "
			  << "all the same in game terms.";
		}
	} else {
		f << "Units are made up of ordinary people" << (Globals->LEADERS_EXIST ? "as well as leaders" : "") << ".";
	}
	if (Globals->LEADERS_EXIST && Globals->SKILL_LIMIT_NONLEADERS) {
		f << " Units made up of normal people may only know one skill, and cannot teach other units.  Units made "
		  << "up of leaders may know as many skills as desired, and may teach other units to speed the learning "
		  << "process.";
	}
	if (Globals->LEADERS_EXIST) {
		f << " Leaders and normal people may not be mixed in the same unit. However, leaders are more expensive to "
		  << "recruit and maintain (more information is in the section on skills).";
	}
	if (Globals->RACES_EXIST) {
		f << " A unit is treated as the least common denominator of the people within it, so a unit made up of two "
		  << "races with different strengths and weaknesses will have all the weaknesses, and none of the strengths "
		  << "of either race.";
	}
	f << "\n" << enclose("p", false);
	f << anchor("playing_turns") << '\n';
	f << enclose("h3", true) << "Turns:\n" << enclose("h3", false);
	f << enclose("p", true) << "Each turn, the Atlantis server takes the orders file that you mailed to it, and "
	  << "assigns the orders to the respective units. All units in your faction are completely loyal to you, and will "
	  << "execute the orders to the best of their ability. If the unit does something unintended, it is generally "
	  << "because of incorrect orders; a unit will not purposefully betray you.\n" << enclose("p", false);

	f << enclose("p", true) << "A turn is equal to one game month.  A unit can do many actions at the start of the "
	  << "month, that only take a matter of hours, such as buying and selling commodities, or fighting an opposing "
	  << "faction.  Each unit can also do exactly one action that takes up the entire month, such as harvesting "
	  << "resources or moving from one region to another.  The orders which take an entire month are "
	  << url("#advance", "ADVANCE") << ", " << url("#build", "BUILD") << ", ";
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED))
		f << url("#entertain", "ENTERTAIN") << ", ";
	f << url("#move", "MOVE") << ", ";
	if (Globals->TAX_PILLAGE_MONTH_LONG)
		f << url("#pillage", "PILLAGE") << ", ";
	f << url("#produce", "PRODUCE") << ", ";
	if (may_sail)
		f << url("#sail", "SAIL") << ", ";
	f << url("#study", "STUDY") << ", ";
	if (Globals->TAX_PILLAGE_MONTH_LONG)
		f << url("#tax", "TAX") << ", ";
	f << url("#teach", "TEACH") << " and " << url("#work", "WORK") << ".\n" << enclose("p", false);

	f << anchor("world") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "The World\n" << enclose("h2", false);
	f << enclose("p", true) << "The Atlantis world is divided for game purposes into hexagonal regions.  Each region "
	  << "has a name, and one of the following terrain types:  Ocean, Plain, Forest, Mountain, Swamp, Jungle, Desert, "
	  << "or Tundra (there may be other types of terrain to be discovered as the game progresses). Regions can "
	  << "contain units belonging to players; they can also contain structures such as buildings";
	if (may_sail)
		f << " and fleets";
	f << ". Two units in the same region can normally interact, unless one of them is concealed in some way.  Two "
	  << "units in different regions cannot normally interact.  NOTE: Combat is an exception to this.\n"
	  << enclose("p", false);
	f << anchor("world_regions") << '\n';
	f << enclose("h3", true) << "Regions:\n" << enclose("h3", false);
	f << enclose("p", true) << "Here is a sample region, as it might appear on your turn report:\n"
	  << enclose("p", false);
	f << enclose("p", true) << "\n" << enclose("p", false);

	int manidx = -1;
	int leadidx = -1;
	for (int i = 0; i < NITEMS; i++) {
		if (!(ItemDefs[i].type & IT_MAN)) continue;
		if (ItemDefs[i].type & IT_LEADER) {
			if (leadidx == -1) leadidx = i;
		} else {
			if (manidx == -1) manidx = i;
		}
	}

	f << pre(true);
	f << "plain (172,110) in Turia, 500 peasants";
	if (Globals->RACES_EXIST)
		f << " (" + string(ItemDefs[manidx].names) + ")";
	int money = (500 * (15 - Globals->MAINTENANCE_COST));
	f << ", $" << money << ".\n";
	f << "------------------------------------------------------\n";
	f << indent::incr;
	if (Globals->WEATHER_EXISTS)
		f << "The weather was clear last month; it will be clear next month.\n";
	f << "Wages: $15 (Max: $" << (money/Globals->WORK_FRACTION) << ").\n";
	f << "Wanted: none.\n";
	f << "For Sale: 50 " << ItemDefs[manidx].names << " [" << ItemDefs[manidx].abr << "] at $";
	float ratio = ItemDefs[(Globals->RACES_EXIST?I_NOMAD:I_MAN)].baseprice/(float)Globals->BASE_MAN_COST;
	f << (int)(60*ratio);
	if (Globals->LEADERS_EXIST) {
		ratio = ItemDefs[leadidx].baseprice/(float)Globals->BASE_MAN_COST;
		f << ", 10 " << ItemDefs[leadidx].names << " [" << ItemDefs[leadidx].abr << "] at $";
		f << (int)(60*ratio);
	}
	f << ".\n";
	f << "Entertainment available: $" << (money/Globals->ENTERTAIN_FRACTION) << ".\n";
	f << "Products: " << (Globals->FOOD_ITEMS_EXIST ? "23 grain [GRAI], " : "") << "37 horses [HORS].\n";
	f << indent::decr;
	f << "\n";
	f << "Exits:\n";
	f << indent::incr;
	f << "North : ocean (172,108) in Atlantis Ocean.\n";
	f << "Northeast : ocean (173,109) in Atlantis Ocean.\n";
	f << "Southeast : ocean (173,111) in Atlantis Ocean.\n";
	f << "South : plain (172,112) in Turia.\n";
	f << "Southwest : plain (171,111) in Turia.\n";
	f << "Northwest : plain (171,109) in Turia.\n";
	f << indent::decr;
	f << "\n";

	f << "* Hans Shadowspawn (15), Merry Pranksters (14), "
	  << (Globals->LEADERS_EXIST ? "leader [LEAD]" : (Globals->RACES_EXIST ? "nomad [NOMA]" : "man [MAN]"))
	  << ", 500 silver [SILV]. Skills: none.\n";
	f << "- Vox Populi (13), "
	  << (Globals->LEADERS_EXIST ? "leader [LEAD]" : (Globals->RACES_EXIST ? "nomad [NOMA]" : "man [MAN]")) << ".\n";
	f << pre(false);
	f << enclose("p", true) << "This report gives all of the available information on this region.  The region type "
	  << "is plain, the name of the surrounding area is Turia, and the coordinates of this region are (172,110).  "
	  << "The population of this region is 500 " << (Globals->RACES_EXIST ? "nomads" : "peasants") << ", and there "
	  << "is $" << money << " of taxable income currently in this region.  Then, under the dashed line, are various "
	  << "details about items for sale, wages, etc.  Finally, there is a list of all visible units.  Units that "
	  << "belong to your faction will be so denoted by a '*', whereas other faction's units are preceded by a '-'.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Since Atlantis is made up of hexagonal regions, the coordinate system is not always "
	  << "exactly intuitive.  Here is the layout of Atlantis regions:\n"
	  << enclose("p", false);
	f << enclose("p", true) << "\n" << enclose("p", false);
	f << pre(true) << indent::wrap(78, 70, 0);
	f << "   ____        ____\n";
	f << "  /    \\      /    \\\n";
	f << " /(0,0) \\____/(2,0) \\____/\n";
	f << " \\      /    \\      /    \\     N\n";
	f << "  \\____/(1,1) \\____/(3,1) \\_   |\n";
	f << "  /    \\      /    \\      /    |\n";
	f << " /(0,2) \\____/(2,2) \\____/     |\n";
	f << " \\      /    \\      /    \\   W-O-E\n";
	f << "  \\____/(1,3) \\____/(3,3) \\_   |\n";
	f << "  /    \\      /    \\      /    S\n";
	f << " /(0,4) \\____/(2,4) \\____/\n";
	f << " \\      /    \\      /    \\\n";
	f << "  \\____/      \\____/\n";
	f << "  /    \\      /    \\\n";
	f << pre(false);

	f << enclose("p", true) << "Note that the are \"holes\" in the coordinate system; there is no region (1,2), "
	  << "for instance.  This is due to the hexagonal system of regions.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Most regions are similar to the region shown above, but the are certain "
	  << "exceptions.  Oceans, not surprisingly, have no population.";
	if (Globals->TOWNS_EXIST)
		f << " Some regions will contain villages, towns, and cities. More information on these is available in "
		  << "the section on the economy.\n";
	f << enclose("p", false);

	if (Globals->ICOSAHEDRAL_WORLD) {
		f << enclose("p", true) << "A further complication is that the world of " << Globals->WORLD_NAME
		  << " is not, as many primitive folk assume, flat, but is actually approximately spherical. However, "
		  << "rendering the surface of a three dimensional object in two dimensions is a long standing "
		  << "cartographical problem. Here is an example of rendering a different spherical world in two "
		  << "dimensional form:\n" << enclose("p", false);
		f << enclose("p", true)
		  << "<img src=\"Goode_homolosine_projection.jpg\" alt=\"Goode Homolosine Projection\">\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Observe that the map appears to have triangular chunks cut out at the poles, "
		  << "and that areas near the poles are spread apart from each other despite being close together in "
		  << "reality. These same effects are observed in the maps we have of " << Globals->WORLD_NAME
		  << ". So some regions will have exits that appear to be rather distant; this is an indicator that the "
		  << "region is on the edge of one of these triangular chunks. The intervening regions are not missing, "
		  << "but are the result of folding a three dimensional object into two dimensions."
		  << enclose("p", false);
		f << enclose("p", true) << "The apparent spacial warps are actually just relics of a two dimensional "
		  << "coordinate system being applied to a three dimensional surface. Should a unit travel over one of "
		  << "these edges, there will be a path back to their starting region, although the path back might not "
		  << "be the direction you expect it to be, as \"north\" changes relative direction when you travel "
		  << "around the pole in the polar regions, so each time you cross one of these edges, you are "
		  << "effectively turning 60 degrees as well as moving.\n"
		  << enclose("p", false);
	}

	f << anchor("region_resources") << '\n';
	f << enclose("h3", true) << "Region resources:\n" << enclose("h3", false);
	f << enclose("p", true) << "Here is list of resources you can find in regions:\n" << enclose("p", false);
	f << enclose("center", true);
	f << enclose("table border=\"1\"", true);
	f << enclose("tr", true);
	f << enclose("td colspan=\"2\"", true) << "Region type\n" << enclose("td", false);
	f << enclose("td colspan=\"4\"", true) << "Resources\n" << enclose("td", false);
	f << enclose("tr", false);

	for (int i=0; i<R_NUM; i++) {
		if (!(TerrainDefs[i].flags & TerrainType::SHOW_RULES)) continue;
		bool first = true;
		f << enclose("tr", true);
		f << enclose("td colspan=\"2\"", true) << TerrainDefs[i].name << '\n' << enclose("td", false);
		f << enclose("td colspan=\"4\"", true);
		for (unsigned int c = 0; c < sizeof(TerrainDefs[i].prods)/sizeof(TerrainDefs[i].prods[0]); c++) {
			if (TerrainDefs[i].prods[c].product == -1) continue;
			if (!first) {
				f << ", ";
			}
			f << ItemDefs[TerrainDefs[i].prods[c].product].name << " (" << TerrainDefs[i].prods[c].chance << "%)";
			first = false;
		}
		f << (first ? "none." : ".") << "\n";
		f << enclose("td", false);
		f << enclose("tr", false);
	}
	f << enclose("table", false);
	f << enclose("center", false);

	f << anchor("world_structures") << '\n';
	f << enclose("h3", true) << "Structures:\n" << enclose("h3", false);
	f << enclose("p", true) << "Regions may also contain structures, such as buildings"
	  << (may_sail ? " or fleets" : "") << ". These will appear directly below the list of units.  "
	  << "Here is a sample structure:\n" << enclose("p", false);
	f << enclose("p", true) << "\n" << enclose("p", false);
	f << pre(true);
	f << "+ Temple of Agrik [3] : Tower.\n";
	f << indent::incr;
	f << "- High Priest Chafin (9), "
	  << (Globals->LEADERS_EXIST ? "leader [LEAD]" : (Globals->RACES_EXIST ? "nomad [NOMA]" : "man [MAN]"))
	  << ", sword [SWOR]\n";
	f << "- Rowing Doom (188), 10 "
	  << (Globals->RACES_EXIST ? "nomads [NOMA]" : (Globals->LEADERS_EXIST ? "leaders [LEAD]" : "men [MAN]"))
	  << ", 10 swords [SWOR].\n";
	f << indent::decr;
	f << pre(false);

	f << enclose("p", true) << "The structure lists the name, the number, and what type of structure it is "
	  << "(more information of the types of structures can be found in the section on the economy). Following this "
	  << "is a list of units inside the structure.";
	if (has_stea)
		f << " Units within a structure are always visible, even if they would otherwise not be seen.";
	f << '\n' << enclose("p", false);
	f << enclose("p", true) << "Units inside structures are still considered to be in the region, and other units "
	  << "can interact with them; however, they may gain benefits, such as defensive bonuses in combat from being "
	  << "inside a building.  The first unit to enter an object is considered to be the owner; only this unit can "
	  << "do things such as renaming the object, or permitting other units to enter. The owner of an object can be "
	  << "identified on the turn report, as it is the first unit listed under the object.  Only units with men in "
	  << "them can be structure owners, so newly created units cannot own a structure until they contain men.\n"
	  << enclose("p", false);
	if (Globals->NEXUS_EXISTS) {
		f << anchor("world_nexus") << '\n';
		f << enclose("h3", true) << "Atlantis Nexus:\n" << enclose("h3", false);
		f << enclose("p", true) << "Note: the following section contains some details that you may wish to skip "
		  << "over until you have had a chance to read the rest of the rules, and understand the mechanics of "
		  << "Atlantis.  However, be sure to read this section before playing, as it will affect your early plans "
		  << "in Atlantis.\n" << enclose("p", false);
		f << enclose("p", true) << "When a faction first starts in Atlantis, it begins with one unit, in a special "
		  << "region called the Atlantis Nexus."
		  << (Globals->MULTI_HEX_NEXUS ? " These regions exist " : " This region exists ");
		if (!Globals->NEXUS_IS_CITY) {
			f << "outside of the normal world of Atlantis, and as such "
			  << (Globals->MULTI_HEX_NEXUS ? "have " : "has ")
			  << "no products or marketplaces; "
			  << (Globals->MULTI_HEX_NEXUS ? "they merely serve " : "it merely serves ")
			  << "as the magical entry into Atlantis.";

		} else {
			f << "inside of the normal world of Atlantis, but "
			  << (Globals->MULTI_HEX_NEXUS ? "each contains " : "contains ")
			  << "a starting city with all its benefits"
			  << (Globals->GATES_EXIST ? ", including a gate" : "") << ". "
			  << (Globals->MULTI_HEX_NEXUS ? "They also serve " : "It also serves ")
	  		  << "as the magical entry into " << Globals->WORLD_NAME << ".";
		}
		f << '\n' << enclose("p", false);

		f << enclose("p", true);
		if (!Globals->START_CITIES_EXIST) {
			f << "The Nexus contains portals that provide one-way transportation to various terrain types.  "
			  << "A unit that enters one of these portals (by entering the portal and moving IN) will "
			  << "be transported to a region of the matching terrain type.  The region chosen is somewhat "
			  << "random, but will prefer to place players in towns where no other players are present.  "
			  << "Once a unit has passed through a portal, there is no way to return to the Nexus.";
		} else if (Globals->MULTI_HEX_NEXUS) {
			f << "From the Nexus hexes, there are exits either to other Nexus hexes, or to starting cities "
			  << "in Atlantis.  Units may move through these exits as normal, but once in a starting city, there "
			  << "is no way to regain entry to the Nexus.";
		} else {
			f << "From the Atlantis Nexus, there are six exits into the starting cities of Atlantis.  Units may "
			  << "move through these exits as normal, but once through an exit, there is no return path to the Nexus.";
		}
		if (Globals->GATES_EXIST &&	(Globals->NEXUS_GATE_OUT || Globals->NEXUS_IS_CITY)) {
			f << " It is also possible to use Gate Lore to get out of the Nexus"
			  << (Globals->NEXUS_GATE_OUT && !Globals->NEXUS_IS_CITY ? " (but not to return)" : "") << ".";
		}
		if (Globals->START_CITIES_EXIST) {
			f << " The " << (!Globals->MULTI_HEX_NEXUS ? "six " : "") << "starting cities offer much to a starting "
			  << "faction; ";
			if (Globals->START_CITIES_START_UNLIMITED) {
				f << (!Globals->SAFE_START_CITIES && Globals->CITY_MONSTERS_EXIST
				      ? "until someone conquers the guardsmen, "
				      : "")
				  << "there are unlimited amounts of many materials and men (though the prices are often quite high).";
			} else {
				f << "there are materials as well as a very large supply of men (though the prices are often quite high).";
			}
			if (Globals->SAFE_START_CITIES || Globals->CITY_MONSTERS_EXIST)
				f << " In addition, ";
			if (Globals->SAFE_START_CITIES)
				f << "no battles are allowed in starting cities";
			if (Globals->CITY_MONSTERS_EXIST) {
				f << (Globals->SAFE_START_CITIES ? " and " : "")
				  << "the starting cities are guarded by strong guardsmen, keeping any units within the city "
				  << (!Globals->SAFE_START_CITIES ? "much safer " : "safe ")
				  << "from attack. See the section on Non-Player Units for more information on city guardsmen";
			}
			f << ". As a drawback, these cities tend to be extremely crowded, and most factions will wish to leave "
			  << "the starting cities when possible.\n"
			  << enclose("p", false);

			f << enclose("p", true) << "It is always possible to enter any starting city from the nexus"
			  << (!Globals->SAFE_START_CITIES
			      ? ", even if that starting city has been taken over and guarded by another faction"
				  : ""
				 )
			  << ". This is due to the transportation from the Nexus to the starting city being magical in nature."
			  << (!Globals->SAFE_START_CITIES
			      ? " Once in the starting city however, no guarantee of safety is given."
				  : ""
				 )
			  << '\n' << enclose("p", false);

			int num_methods = 1 + (Globals->GATES_EXIST ? 1 : 0) + (may_sail ? 1 : 0);
			string methods[] = {"You must go ", "The first is ", "The second is "};
			int method = 1;
			if (num_methods == 1) method = 0;
			f << enclose("p", true);
			f << "There " << plural(num_methods, "is ", "are ") << num_to_word(num_methods) << " "
			  << plural(num_methods, "method", "methods") << " of departing the starting cities. "
			  << methods[method++] << " by land, but keep in mind that the lands immediately surrounding the starting "
			  << "cities will tend to be highly populated, and possibly quite dangerous to travel.";
			if (may_sail) {
				f << " " << methods[method] << " by sea; all of the starting cities lie against an "
				  << "ocean, and a faction may easily purchase wood and construct a ship to "
				  << url("#sail", "SAIL") << " away.  Be wary of pirates seeking to prey on new "
				  << "factions, however!";
			}
			if (Globals->GATES_EXIST) {
				f << " And last, rumors of a magical Gate Lore suggest yet another way to travel from the "
				  << "starting cities.  The rumors are vague, but factions wishing to travel far from the "
				  << "starting cities, taking only a few men with them, might wish to pursue this method.";
			}
		}
		f << '\n' << enclose("p", false);
	}

	f << anchor("movement") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Movement\n" << enclose("h2", false);
	f << enclose("p", true)
	  << (may_sail
	      ? "There are two main methods of movement in Atlantis.  The first "
		  : "The main method of movement in Atlantis ")
	  << "is done using the " << url("#move", "MOVE") << " order (or the " << url("#advance", "ADVANCE")
	  << " order), and moves units individually from one region to another. ";
	if (may_sail)
		f << "The other method is done using the " << url("#sail", "SAIL") << " order, which can sail a "
		  << "fleet, including all of its occupants from one region to another. ";
	f << "Certain powerful mages may also teleport themselves, or even other units, but the knowledge of the "
	  << "workings of this magic is carefully guarded.\n"
	  << enclose("p", false);

	f << anchor("movement_normal") << '\n';
	f << enclose("h3", true) << "Normal Movement:\n" << enclose("h3", false);
	f << enclose("p", true) << "In one month, a unit can issue a single " << url("#move", "MOVE")
	  << " order, using one or more of its movement points. There are three modes of travel: walking, riding "
	  << "and flying. Walking units have " << num_to_word(ItemDefs[I_LEADERS].speed) << " movement "
	  << plural(ItemDefs[I_LEADERS].speed, "point", "points") << ", riding units have "
	  << num_to_word(ItemDefs[I_HORSE].speed) << ", and flying units have " << num_to_word(ItemDefs[I_WHORSE].speed)
	  << ". A unit will automatically use the fastest mode of travel it has available. The "
	  << url("#advance", "ADVANCE") << " order is the same as " << url("#move", "MOVE") << ", except that it "
	  << "implies attacks on units which try to forbid access; see the section on combat for details.\n"
	  << enclose("p", false);

	f << enclose("p", true) << "Note that depending on game settings certain races might be able to swim or fly "
	  << "and there are items that can enable your units to fly or walk on water.\n"
	  << enclose("p", false);

	f << enclose("p", true) << "Flying units are not initially available to starting players. A unit can ride "
	  << "provided that the carrying capacity of its horses is at least as great as the weight of its people and "
	  << "all other items. A unit can walk provided that the carrying capacity of its people";
	if (!(ItemDefs[I_HORSE].flags & ItemType::DISABLED))
		f << (!(ItemDefs[I_WAGON].flags & ItemType::DISABLED) ? ", " : " and ") << "horses";
	if (!(ItemDefs[I_WAGON].flags & ItemType::DISABLED) && (!(ItemDefs[I_HORSE].flags & ItemType::DISABLED)))
		f << ", and wagons";
	f << " is at least as great as the weight of all its other items";
	if (!(ItemDefs[I_WAGON].flags & ItemType::DISABLED) && (!(ItemDefs[I_HORSE].flags & ItemType::DISABLED))) {
		f << ", and provided that it has at least as many horses as wagons (otherwise the excess wagons count as "
		  << "weight, not capacity)";
	}
	f << ". Otherwise the unit cannot issue a " << url("#move", "MOVE") << " order. Most people weigh 10 units "
	  << "and have a capacity of 5 units; data for items is as follows:\n" << enclose("p", false);
	f << anchor("tableitemweights") << '\n';
	f << enclose("center", true);
	f << enclose("table border=\"1\"", true);
	f << enclose("tr", true);
	f << enclose("td", true) << '\n' << enclose("td", false);
	f << enclose("th", true) << "Weight\n" << enclose("th", false);
	f << enclose("th", true) << "Capacity\n" << enclose("th", false);
	f << enclose("tr", false);

	for (int i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[i].type & IT_NORMAL)) continue;
		pS = FindSkill(ItemDefs[i].pSkill);
		if (pS && (pS->flags & SkillType::DISABLED)) continue;
		int last = 0;
		for (int j = 0; j < (int) (sizeof(ItemDefs->pInput) / sizeof(ItemDefs->pInput[0])); j++) {
			int k = ItemDefs[i].pInput[j].item;
			if (k != -1 && (ItemDefs[k].flags & ItemType::DISABLED))
				last = 1;
			if (k != -1 && !(ItemDefs[k].type & IT_NORMAL)) last = 1;
		}
		if (last == 1) continue;
		f << enclose("tr", true);
		f << enclose("td align=\"left\" nowrap", true) << ItemDefs[i].name << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << ItemDefs[i].weight << '\n' << enclose("td", false);
		int cap = ItemDefs[i].walk - ItemDefs[i].weight;
		f << enclose("td align=\"left\" nowrap", true);
		if (ItemDefs[i].walk || (ItemDefs[i].hitchItem != -1)) {
			if (ItemDefs[i].hitchItem == -1)
				f << cap << '\n';
			else {
				f << (cap + ItemDefs[i].hitchwalk) << " (with "
				  << ItemDefs[ItemDefs[i].hitchItem].name << ")\n";
			}
		} else {
			f << "&nbsp;\n";
		}
		f << enclose("td", false);
		f << enclose("tr", false);
	}
	f << enclose("table", false);
	f << enclose("center", false);
	if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) {
		f << enclose("p", true) << "A unit which can fly is capable of travelling over water"
		  << (Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_MUST_LAND
		      ? ", but if the unit ends its turn over a water hex then it will drown." : ""
		     )
		  << ".\n" << enclose("p", false);
	}

	f << enclose("p", true) << "Since regions are hexagonal, each region has six neighbouring "
	  << "regions to the north, northeast, southeast, south, southwest and northwest.  Moving from one region to "
	  << "another normally takes one movement point, except that the following terrain types take two movement "
	  << "points for riding or walking units to enter: Forest, Mountain, Swamp, Jungle, and Tundra.";
	if (Globals->WEATHER_EXISTS) {
		f << " Also, during certain seasons (depending on the latitude of the region), all units (including flying "
		  << "ones) have a harder time and travel will take twice as many movement points as normal, as freezing "
		  << "weather makes travel difficult; in the tropics, seasonal hurricane winds and torrential rains have a "
		  << "similar effect.";
	}
	f << " Units may not move through ocean regions ";
	if (may_sail) {
		f << "without using the " << url("#sail", "SAIL") << " order";
	}
	if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) {
		f << " unless they are capable of flight"
		  << (Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_MUST_LAND
		      ? ", and even then, flying units must end their movement on land or else drown"
			  : ""
			 );
	}
	f  << ".\n" << enclose("p", false);
	f << enclose("p", true) << "Units may also enter or exit structures while moving.  Moving "
	  << "into or out of a structure does not use any movement points at all.  Note that a unit can also use the "
	  << url("#enter", "ENTER") << " and " << url("#leave", "LEAVE") << " orders to move in and out of "
	  << "structures, without issuing a " << url("#move", "MOVE") << " order." << " The unit can also use the "
	  << url("#move", "MOVE") << " order to enter or leave a structure.\n"
	  << enclose("p", false);
	if (Globals->UNDERWORLD_LEVELS || Globals->UNDERDEEP_LEVELS) {
		f << enclose("p", true) << "Finally, certain structures contain interior passages to "
		  << "other regions.  The " << url("#move", "MOVE") << " IN order can be used to go "
		  << "through these passages; the movement point cost is equal to the normal cost to enter the destination "
		  << "region.\n" << enclose("p", false);
	}
	int cap = ItemDefs[I_HORSE].ride - ItemDefs[I_HORSE].weight;
	int weight = ItemDefs[I_MAN].weight + ItemDefs[I_SWORD].weight + ItemDefs[I_CHAINARMOR].weight;
	int speed = ItemDefs[(cap > weight) ? I_HORSE : I_MAN].speed;
	int cost = TerrainDefs[R_PLAIN].movepoints;
	f << enclose("p", true) << "Example: One man with a horse, sword, and chain mail wants to "
	  << "move north, then northeast.  The capacity of the horse is " << cap
	  << " and the weight of the man and other items is " << weight
	  << (cap > weight ? ", so he can ride" : ", so he must walk")
	  << (Globals->WEATHER_EXISTS ? ". The month is April, so he has " : " and has ")
	  << num_to_word(speed) << " movement " << plural(speed, "point", "points") << ". He issues the order "
	  << "MOVE NORTH NORTHEAST. First he moves north, into a plain region.  This uses " << num_to_word(cost)
	  << " movement " << plural(cost, "point", "points") << ".";
	speed -= cost;
	if (speed > TerrainDefs[R_FOREST].movepoints) {
		cost = TerrainDefs[R_FOREST].movepoints;
		speed -= cost;
		f << " Then he moves northeast, into a forest region. This uses "
		  << num_to_word(TerrainDefs[R_FOREST].movepoints) << " movement "
		  << plural(TerrainDefs[R_FOREST].movepoints, "point", "points") << ", so the movement is completed with "
		  << num_to_word(speed) << " to spare.";
	} else {
		f << " He does not have the " << num_to_word(TerrainDefs[R_FOREST].movepoints)
		  << " movement " << plural(TerrainDefs[R_FOREST].movepoints, "point", "points")
		  << " needed to move into the forest region to the northeast, so the movement is halted at this point.  "
		  << "The remaining move will be added to his orders for the next turn, before any "
		  << url("#turn", "TURN") << " orders are processed.";
	}
	f << '\n' << enclose("p", false);

	if (may_sail) {
		f << anchor("movement_sailing") << '\n';
		f << enclose("h3", true) << "Sailing:\n" << enclose("h3", false);
		f << enclose("p", true) << "Movement by sea is in some ways similar. It does not use the "
		  << url("#move", "MOVE") << " order however.  Instead, the owner of a fleet must issue the "
		  << url("#sail", "SAIL") << " order, and other units wishing to help sail the fleet must also issue the "
		  << url("#sail", "SAIL") << " order. The fleet will then, if possible, make the indicated movement, "
		  << "carrying all units on the fleet with it.  Units on board the fleet, but not aiding in the sailing of "
		  << "the fleet, may execute other orders while the fleet is sailing.  A unit which does not wish to travel "
		  << "with the fleet should leave the fleet in a coastal region, before the " << url("#sail", "SAIL")
		  << " order is processed.  (A coastal region is defined as a non-ocean region with at least one adjacent "
		  << "ocean region.)\n" << enclose("p", false);
		f << enclose("p", true) << "Note that a unit on board a fleet while it is sailing may not "
		  << url("#move", "MOVE") << " later in the turn, even if he doesn't issue the " << url("#sail", "SAIL")
		  << " order; sailing is considered to take the whole month. Also, units may not remain on guard while "
		  << "on board a sailing fleet; they will have to reissue the " << url("#guard", "GUARD")
		  << " 1 order to guard a region after sailing.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Most ships get " << num_to_word(ItemDefs[I_LONGBOAT].speed)
		  << " movement point" << (ItemDefs[I_LONGBOAT].speed==1?"":"s") << " per turn.";
		if (Globals->FLEET_CREW_BOOST > 0) {
			f << " Ships get an extra movement point for each time they double the number of required crew, "
			  << "up to a maximum of " << num_to_word(Globals->FLEET_CREW_BOOST) << " extra "
			  << plural(Globals->FLEET_CREW_BOOST, "point", "points") << ".";
		}
		if (Globals->FLEET_LOAD_BOOST > 0) {
			f << " Ships get extra movement points if they are only lightly loaded. One extra point is given "
			  << "if they are at 1/2 capacity or less"
			  << (Globals->FLEET_LOAD_BOOST > 1 ? "; a second is given if they are at 1/4 capacity or less" : "");
			if (Globals->FLEET_LOAD_BOOST > 2) {
				f << ", and so on up to a maximum of " << num_to_word(Globals->FLEET_LOAD_BOOST);
			}
			f << ".";
		}
		f << " A fleet can move from an ocean region to another ocean region, or from a coastal region to an ocean "
		  << "region, or from an ocean region to a coastal region.";
		if (Globals->PREVENT_SAIL_THROUGH) {
			f << " Ships may not sail through single hex land masses and must leave via the same side they "
			  << "entered or a side adjacent to that one.";
			if (Globals->ALLOW_TRIVIAL_PORTAGE)
				f << " Ships ending their movement in a land hex may sail out along any side connecting to water.";
		}
		f << " Ships can only be constructed in coastal regions. For a fleet to enter any region only costs one "
		  << "movement point; the cost of two movement points for entering, say, a forest coastal region, does "
		  << "not apply.";
		if (Globals->WEATHER_EXISTS) {
			f << " Ships do, however, only get half movement points during the winter months (or monsoon months in "
			  << "the tropical latitudes).";
		}
		f << '\n' << enclose("p", false);
		f << enclose("p", true) << "A fleet can only move if the total weight of everything aboard does not "
		  << "exceed the fleet's capacity (the rules do not prevent an overloaded fleet from staying afloat, only "
		  << "from moving).  Also, there must be enough sailors aboard (using the " << url("#sail", "SAIL")
		  << " order), to sail the fleet, or it will not go anywhere.  Note that the sailing skill increases the "
		  << "usefulness of a unit proportionally; thus, a 1 man unit with level 5 sailing skill can sail a longboat "
		  << "alone.  (See the section on skills for further details on skills.)  The capacities (and costs in "
		  << "labor units) of the various basic ship types are as follows:\n"
		  << enclose("p", false);

		f << anchor("tableshipcapacities") << '\n';
		f << enclose("center", true);
		f << enclose("table border=\"1\"", true);
		f << enclose("tr", true);
		f << enclose("td", true) << "Class\n" << enclose("td", false);
		f << enclose("th", true) << "Capacity\n" << enclose("th", false);
		f << enclose("th", true) << "Cost\n" << enclose("th", false);
		f << enclose("th", true) << "Sailors\n" << enclose("th", false);
		f << enclose("th", true) << "Skill\n" << enclose("th", false);
		f << enclose("tr", false);
		for (int i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_SHIP)) continue;
			int pub = 1;
			for (int c = 0; c < (int) sizeof(ItemDefs->pInput)/(int) sizeof(ItemDefs->pInput[0]); c++) {
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
			f << enclose("tr", true);
			f << enclose("td align=\"center\"", true) << ItemDefs[i].name << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ItemDefs[i].swim << '\n'  << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ItemDefs[i].pMonths << '\n'  << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ItemDefs[i].weight/50 << '\n'  << enclose("td", false);
			f << enclose("td align=\"center\"", true) << slevel << '\n'  << enclose("td", false);
			f << enclose("tr", false);
		}
		for (int i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (!ObjectIsShip(i)) continue;
			if (ItemDefs[ObjectDefs[i].item].flags & ItemType::DISABLED)
				continue;
			int normal = (ItemDefs[ObjectDefs[i].item].type & IT_NORMAL);
			normal |= (ItemDefs[ObjectDefs[i].item].type & IT_TRADE);
			if (!normal) continue;
			f << enclose("tr", true);
			f << enclose("td align=\"center\"", true) << ObjectDefs[i].name << '\n'  << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ObjectDefs[i].capacity << '\n'  << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ObjectDefs[i].cost << '\n'  << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ObjectDefs[i].sailors << '\n'  << enclose("td", false);
			f << enclose("td align=\"center\"", true) << 1  << '\n' << enclose("td", false);
			f << enclose("tr", false);
		}
		f << enclose("table", false);
		f << enclose("center", false);
		f << enclose("p", true) << "The skill column is the level of shipbuilding skill required "
		  << "to build that ship type.\n" << enclose("p", false);
	}
	f << anchor("movement_order") << '\n';
	f << enclose("h3", true) << "Order of Movement:\n" << enclose("h3", false);
	f << enclose("p", true) << "This section is probably unimportant to beginning players, but "
	  << "it can be helpful for more experienced players.\n" << enclose("p", false);
	f << enclose("p", true) << "Movement in Atlantis is processed one hex of movement at a "
	  << "time, region by region. Atlantis cycles through all of the regions; for each region, "
	  << "it finds any units that are due to move, and moves them (if they can move) one hex (and only one hex). "
	  << "After processing all the regions, it conducts any battles that result from these movements. "
	  << "After it has gone through all of the regions, units will have moved at most one hex, so it goes back "
	  << "and does the whole process again. This is repeated until all units have had the opportunity to move "
	  << "their allowed distance. Units' movement is spread out over these phases proportionally to their speed, "
	  << "so a unit riding at speed 4 would move twice as often as one walking at speed 2. If the unit requires "
	  << "more than one move to enter a particular region, then it will move once it has accumulated enough "
	  << "movement points to do so. Note that these movement points can be carried over from one month to another "
	  << "if a MOVE (or ADVANCE) command did not complete in the month - for example, a unit on foot trying to move "
	  << "into a mountain region in winter would not have enough movement points to enter in one turn, but if it "
	  << "continues the same move on the next turn, it would use the accumulated points from the last month and "
	  << "manage to enter the mountains at last.\n"
	  << enclose("p", false);
	if (may_sail) {
		f << enclose("p", true) << "Sailing is handled the same way, with one minor difference: where units "
		  << "using MOVE or ADVANCE will be prevented from entering a GUARDed region (where the guards are "
		  << "unfriendly or worse to the moving unit), fleets will instead enter the region and then be stopped "
		  << "by the guards. \n" << enclose("p", false);
	}

	f << enclose("p", true) << "The following table shows when exactly units will move, given their base "
	  << "movement speed.  The \"x\"s mark the phases in which a unit of that speed will move. If you wish to "
	  << "make units of different speeds move together (for example, to coordinate an attack), you may need to "
	  << "tell the faster units to PAUSE in their movement.  See the " << url("#move", "MOVE")
	  << " order for details.\n"
	  << enclose("p", false);

	f << enclose("center", true);
	f << enclose("table border=\"1\"", true);
	f << enclose("tr", true);
	f << enclose("td colspan=\"2\" rowspan=\"2\"", true) << enclose("td", false);
	f << enclose("td colspan=\"" + to_string(Globals->MAX_SPEED) + "\"", true) << "Movement Phase\n"
	  << enclose("td", false);
	f << enclose("tr", false);
	f << enclose("tr", true);
	for (int i = 0; i < Globals->MAX_SPEED; i++) {
		f << enclose("th", true) << i + 1 << '\n' << enclose("th", false);
	}
	f << enclose("tr", false);
	for (int i = 0; i < Globals->MAX_SPEED; i++) {
		f << enclose("tr", true);
		if (!i) {
			f << enclose("td rowspan=\"" + to_string(Globals->MAX_SPEED) + "\"", true) << "Speed\n"
			  << enclose("td", false);
		}
		f << enclose("th", true) << i + 1 << '\n' << enclose("th", false);
		int k = Globals->PHASED_MOVE_OFFSET;
		for (int j = 0; j < Globals->MAX_SPEED; j++) {
			k += i + 1;
			if (k >= Globals->MAX_SPEED) {
				f << enclose("td", true) << "x\n" << enclose("td", false);
				k -= Globals->MAX_SPEED;
			} else {
				f << enclose("td", true) << "\n" << enclose("td", false);
			}
		}
		f << enclose("tr", false);
	}
	f << enclose("table", false);
	f << enclose("center", false);

	f << anchor("skills") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Skills\n" << enclose("h2", false);
	f << enclose("p", true) << "The most important thing distinguishing one character from another in Atlantis "
	  << "is skills.  The following skills are available: ";
	int comma = 0;
	found = false;
	int last = -1;
	for (int i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].flags & SkillType::DISABLED) continue;
		if (SkillDefs[i].flags & SkillType::APPRENTICE) continue;
		if (SkillDefs[i].flags & SkillType::MAGIC) continue;
		found = false;
		for (int j = 0; j < 3; j++) {
			SkillType *pS = FindSkill(SkillDefs[i].depends[j].skill);
			if (pS && !(pS->flags & SkillType::DISABLED)) {
				found = true;
				break;
			}
		}
		if (found) continue;
		if (last == -1) {
			last = i;
			continue;
		}
		f << SkillDefs[last].name << ", ";
		last = i;
		comma++;
	}
	if (last != -1) {
		f << (comma ? "and " : "") << SkillDefs[last].name;
	}

	f << ". When a unit possesses a skill, he also has a skill level to go with it.  Generally, the effectiveness of "
	  << "a skill is directly proportional to the skill level involved, so a unit with level 2 in a skill is twice "
	  << "as good as a unit with level 1 in the same skill.\n"
	  << enclose("p", false);
	f << anchor("skills_limitations") << '\n';
	f << enclose("h3", true) << "Limitations:\n" << enclose("h3", false);
	if (Globals->LEADERS_EXIST && Globals->SKILL_LIMIT_NONLEADERS) {
		f << enclose("p", true) << "A unit made up of leaders may know one or more skills; for the rest of this "
		  << "section, the word \"leader\" will refer to such a unit.  Other units, those which contain non-"
		  << "leaders, will be refered to as normal units. A normal unit may only know one skill.\n"
		  << enclose("p", false);
	}
	if (!Globals->RACES_EXIST) {
		f << (Globals->SKILL_LIMIT_NONLEADERS
		      ? "A unit may only learn one skill. "
			  : "A unit may learn as many skills as it requires. ");
		ManType *mt = FindRace("MAN");
		if (mt != NULL) {
			f << "Skills can be learned up to a maximum level of " << mt->defaultlevel << ".";
		}
		f << '\n' << enclose("p", false);
	}
	if (Globals->RACES_EXIST) {
		f << enclose("p", true) << "Skills may be learned up to a maximum level depending on the race of the "
		  << "studying unit (remembering that for units containing more than one race, the maximum is determined by "
		  << "the least common denominator).  Every race has a normal maximum skill level, and  a list of skills "
		  << "that they specialize in, and can learn up to higher level. ";
		if (Globals->LEADERS_EXIST) {
			f << "Leaders, being more powerful, can learn skills to even higher levels. ";
		}
		f << "Here is a list of the races (including leaders) and the information on normal skill levels "
		  << "and specialized skills.\n" << enclose("p", false);
		f << anchor("tableraces") << '\n';
		f << enclose("center", true);
		f << enclose("table border=\"1\"", true);
		f << enclose("tr", true);
		f << enclose("th", true) << "Race/Type\n" << enclose("th", false);
		f << enclose("th", true) << "Specilized Skills\n" << enclose("th", false);
		f << enclose("th", true) << "Max Level (specialized skills)\n" << enclose("th", false);
		f << enclose("th", true) << "Max Level (non-specialized skills)\n" << enclose("th", false);
		f << enclose("tr", false);
		for (int i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_MAN)) continue;
			ManType *mt = FindRace(ItemDefs[i].abr);

			f << enclose("tr", true);
			f << enclose("td align=\"left\" nowrap", true) << ItemDefs[i].names << '\n' << enclose("td", false);
			f << enclose("td align=\"left\" nowrap", true);
			int spec = 0;
			comma = 0;
			for (int j = 0; j < (int)(sizeof(mt->skills) / sizeof(mt->skills[0])); j++) {
				pS = FindSkill(mt->skills[j]);
				if (!pS) continue;
				if (string(pS->abbr) == "MANI" && Globals->MAGE_NONLEADERS) {
					spec = 1;
					f << (comma ? ", " : "") << "all magical skills";
					comma++;
					continue;
				}
				if (pS->flags & SkillType::DISABLED) continue;
				spec = 1;
				f << (comma ? ", " : "") << pS->name;
				comma++;
			}
			f << (spec ? "" : "None.") << "\n";
			f << enclose("td", false);
			f << enclose("td align=\"left\" nowrap", true) << (spec ? to_string(mt->speciallevel) : "--") << '\n'
			  << enclose("td", false);
			f << enclose("td align=\"left\" nowrap", true) << mt->defaultlevel << '\n' << enclose("td", false);
			f << enclose("tr", false);
		}
		f << enclose("table", false);
		f << enclose("center", false);
	}

	f << enclose("p", true) << "If units are merged together, their skills are averaged out. No rounding off is "
	  << "done; rather, the computer keeps track for each unit of how many total months of training that unit has "
	  << "in each skill. When units are split up, these months are divided as evenly as possible among the people "
	  << "in the unit; but no months are ever lost.\n"
	  << enclose("p", false);

	f << anchor("skills_studying") << '\n';
	f << enclose("h3", true) << "Studying:\n" << enclose("h3", false);
	f << enclose("p", true) << "For a unit to gain level 1 of a skill, they must gain one "
	  << "months worth of training in that skill by issuing the " << url("#study", "STUDY") << " order. ";
	if (Globals->REQUIRED_EXPERIENCE) {
		f << "Initially, a unit will gain a full months worth of training (30 day equivalents). However, high "
		  << "levels of skill also require experience, which is gained by performing actions making use of the "
		  << "skill. Any such experience is unknown even to a unit's owner, but will allow the unit to study "
		  << "at a much faster rate to higher skill levels. A unit continuing study but not acquiring experience "
		  << "will find it's study rate reduced as it progresses. \n";
		f << enclose("p", false);

		int months2 = 0;
		int months3 = 0;
		int months5 = 0;
		int mlevel = 0;
		int tries5 = 60;
		int days = 0;
		int dneeded = GetDaysByLevel(2);
		while(days < dneeded) {
			days += study_rate(days, 0);
			months2++;
			months3++;
			months5++;
		}
		dneeded = GetDaysByLevel(3);
		int tries = 60;
		while((days < dneeded) && (tries > 0)) {
			tries--;
			days += study_rate(days, 0);
			months3++;
			months5++;
		}
		dneeded = GetDaysByLevel(5);
		int rate = study_rate(days, 0);
		while((days < dneeded) && (rate > 0) && (tries5 >0)) {
			tries5--;
			rate = study_rate(days, 0);
			days += rate;
			months5++;
		}
		mlevel = GetLevelByDays(days);

		f << enclose("p", true);
		f << "To illustrate this, a unit would have to spend " << num_to_word(months2)
		  << " months studying to gain level 2 in a skill without any experience. ";
		if (tries > 0) {
			f << "In order to reach level 3 only by studying, a total of " << num_to_word(months3)
			  << " months must be spent. ";
		} else {
			f << "Said unit would for all practical purposes be unable to reach "
			  << "skill level 3 by studying without experience. ";
		}
		f << "The maximum skill level that is possible through continuous study only (i.e. no experience "
		  << "involved) is " << num_to_word(mlevel) + ", "
		  << (months5 > 36 ? "although it is hardly feasible without any experience at all, " : "")
		  << "taking " << num_to_word(months5) << " months of studying to achieve. "
		  << "Note that this assumes that the unit type is allowed to achieve this level at all "
		  << url("#skills_limitations", "(see skill limitations)") + ". ";

		/* Example with 30 experience */
		months2 = 0;
		months3 = 0;
		days = 0;
		dneeded = GetDaysByLevel(2);
		while(days < dneeded) {
			days += study_rate(days, 30);
			months2++;
			months3++;
		}
		dneeded = GetDaysByLevel(3);
		tries = 60;
		while((days < dneeded) && (tries > 0)) {
			tries--;
			days += study_rate(days, 30);
			months3++;
		}

		f << "A unit will start with 30 experience in it's race's specialized skills. In comparison, units with "
		  << "this amount of experience will reach level 2 in just " << num_to_word(months2) << " months of study, ";
		if (tries > 0) {
			f << " and in order to reach level 3 only by studying, a total of " << num_to_word(months3)
			  << " months must be spent by a unit starting with 30 experience. ";
		} else {
			f << "but even such a unit would be unable to reach skill level 3 merely through study, but will have "
			  << "to gain some experience along the way. ";
		}
		f << "The study progress is shown in the following table:\n" << enclose("p", false);

		f << anchor("studyprogress") << '\n';
		f << enclose("center", true);
		f << enclose("table border=\"1\"", true);
		f << enclose("tr", true);
		f << enclose("th", true) << "Unit type\n" << enclose("th", false);
		f << enclose("th", true) << "starts with\n" << enclose("th", false);
		f << enclose("th", true) << "1 month\n" << enclose("th", false);
		f << enclose("th", true) << "2 months\n" << enclose("th", false);
		f << enclose("th", true) << "3 months\n" << enclose("th", false);
		f << enclose("th", true) << "4 months\n" << enclose("th", false);
		f << enclose("th", true) << "5 months\n" << enclose("th", false);
		f << enclose("th", true) << "6 months\n" << enclose("th", false);
		f << enclose("th", true) << "7 months\n" << enclose("th", false);
		f << enclose("th", true) << "8 months\n" << enclose("th", false);
		f << enclose("th", true) << "9 months\n" << enclose("th", false);
		f << enclose("tr", false);
		f << enclose("tr", true);
		f << enclose("td", true) << "non-specialized\n" << enclose("td", false);
		days = 0;
		int level = 0;
		int plevel = 0;
		int next = study_rate(days, 0);
		f << enclose("td", true) << "&nbsp;" << level << "&nbsp;<font size='-1'>(" << days << "+" << next
		  << ")</font>\n" << enclose("td", false);
		for (int m = 0; m < 9; m++) {
			days += study_rate(days, 0);
			next = study_rate(days, 0);
			level = GetLevelByDays(days);
			f << enclose("td", true) << "&nbsp;";
			if (level > plevel) f << "<b>";
			f << level;
			if (level > plevel) {
				plevel = level;
				f << "</b>";
			}
			f << "&nbsp;<font size='-1'>(" << days << "+" << next << ")</font>\n" << enclose("td", false);
		}
		f << enclose("tr", false);
		f << enclose("tr", true);
		f << enclose("td", true) << "specialized\n" << enclose("td", false);
		days = 0;
		level = 0;
		plevel = 0;
		next = study_rate(days, 30);
		f << enclose("td", true) << "&nbsp;" << level << "&nbsp;<font size='-1'>(" << days << "+" << next
		  << ")</font>\n" << enclose("td", false);
		for (int m = 0; m < 9; m++) {
			days += study_rate(days, 30);
			next = study_rate(days, 30);
			level = GetLevelByDays(days);
			f << enclose("td", true) << "&nbsp;";
			if (level > plevel) f << "<b>";
			f << level;
			if (level > plevel) {
				plevel = level;
				f << "</b>";
			}
			f << "&nbsp;<font size='-1'>(" << days << "+" << next << ")</font>\n" << enclose("td", false);
		}
		f << enclose("tr", false);
		f << enclose("table", false);
		f << enclose("center", false);
		f << enclose("p", true) << "Each skill is listed with it's skill level, followed (in parentheses) by first "
		  << "the number of day equivalents already achieved, a plus sign, and the number of day equivalents "
		  << "expected to be gained with the next study order. This last amount will vary with experience "
		  << "and with progress in skill training (more training in the skill requiring considerably more experience "
		  << "in order to continue studying). This information is also listed for each unit in the report. As can "
		  << "be seen for the specialized units above, surplus experience will garner a considerable bonus to "
		  << "the study rate. \n"
		  << enclose("p", false);
		  f << enclose("p", true);
	} else {
		f << "To raise this skill level to 2, the unit must add an additional two months worth of training.  Then, "
		  << "to raise this to skill level 3 requires another three months worth of training, and so forth. ";
	}
	f << "A month of training is gained when a unit uses the " << url("#study", "STUDY") << " order.  Note that "
	  << "study months do not need to be consecutive; for a unit to go from level 1 to level 2, he can study for "
	  << "a month, do something else for a month, and then go back and complete the rest of his studies.";
	if (Globals->SKILL_PRACTICE_AMOUNT > 0) {
		f << "  A unit can also increase its level of training by using a skill.  This progress is ";
		if (Globals->SKILL_PRACTICE_AMOUNT < 11)
			f << "much slower than";
		else if (Globals->SKILL_PRACTICE_AMOUNT < 30)
			f << "slower than";
		else if (Globals->SKILL_PRACTICE_AMOUNT == 30)
			f << "the same as";
		else if (Globals->SKILL_PRACTICE_AMOUNT < 61)
			f << "faster than";
		else
			f << "much faster than";
		f << " studying.  Only one skill can be improved through practice in any month; if multiple skills are "
		  << "used, only the first will be improved.  A skill will only improve with practice if the unit has "
		  << "first studied the rudiments of the skill.";
	}
	f << '\n' << enclose("p", false);
	// XXX -- This is not as nice as it could be and could cause problems
	// if the skills are given disparate costs.   This should probably be
	// a table of all skills/costs.
	f << enclose("p", true) << "Most skills cost $" << SkillDefs[S_COMBAT].cost
	  << " per person per month to study (in addition to normal maintenance costs).  The exceptions are ";
	if (has_stea || has_obse) {
		f << (has_stea ? "Stealth" : "") << (has_stea && has_obse ? " and " : "") << (has_obse ? "Observation" : "")
		  << " (" << (has_stea && has_obse ? "both of which cost $" : "which costs $") << SkillDefs[S_STEALTH].cost
		  << "), ";
	}
	f << "Magic skills (which cost $" << SkillDefs[S_FORCE].cost << ")";
	if (!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED))
		f << ", and Tactics (which costs $" << SkillDefs[S_TACTICS].cost << ")";
	f << ".\n" << enclose("p", false);
	f << anchor("skills_teaching") << '\n';
	f << enclose("h3", true) << "Teaching:\n" << enclose("h3", false);
	f << enclose("p", true) << "A unit with a teacher can learn up to twice as fast as normal. The "
	  << url("#teach", "TEACH") << " order is used to spend the month teaching one or more other units (your own or "
	  << "another factions).  The unit doing the teaching must have a skill level greater than the unit doing the "
	  << "studying.  (Note: for all skill uses, it is skill level, not number of months of training, that counts. "
	  << "Thus, a unit with 1 month of training is effectively the same as a unit with 2 months of training, "
	  << "since both have a skill level of 1.)  The units being taught simply issue the " << url("#study", "STUDY")
	  << " order normally (also, his faction must be declared Friendly by the teaching faction).  Each person can "
	  << "only teach up to " << Globals->STUDENTS_PER_TEACHER << " "
	  << plural(Globals->STUDENTS_PER_TEACHER, "student", "students") << " in a month; additional students dilute "
	  << "the training.  Thus, if 1 teacher teaches " << (2 * Globals->STUDENTS_PER_TEACHER)
	  << " men, each man being taught will gain 1 1/2 months of training, not 2 months.\n"
	  << enclose("p", false);

	f << enclose("p", true) << "Note that it is quite possible for a single unit to teach two or more other units "
	  << "different skills in the same month, provided that the teacher has a higher skill level than each student "
	  << "in the skill that that student is studying, and that there are no more than "
	  << Globals->STUDENTS_PER_TEACHER << ' ' << plural(Globals->STUDENTS_PER_TEACHER, "student", "students")
	  << " per teacher.\n" << enclose("p", false);

	if (Globals->LEADERS_EXIST) {
		f << enclose("p", true) << "Note: Only leaders may use the " << url("#teach", "TEACH") << " order.\n"
		  << enclose("p", false);
	}
	f << anchor("skills_skillreports") << '\n';
	f << enclose("h3", true) << "Skill Reports:\n" << enclose("h3", false);
	f << enclose("p", true) << "When a faction learns a new skill level for this first time, it will be given a "
	  << "report on special abilities that a unit with this skill level has. This report can be shown again at "
	  << "any time (once a faction knows the skill), using the " << url("#show", "SHOW") << " order. For "
	  << "example, when a faction learned the skill Shoemaking level 3 for the first time, it might receive the "
	  << "following (obviously farcical) report:\n"
	  << enclose("p", false);
	f << enclose("p", true) << '\n' << enclose("p", false);
	f << pre(true);
	f << "Shoemaking [SHOE] 3: A unit with this skill may PRODUCE Sooper Dooper Air Max Winged Sandals.\n";
	f << pre(false);

	f << anchor("economy") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "The Economy\n" << enclose("h2", false);
	f << enclose("p", true) << "The unit of currency in Atlantis is the silver piece. Silver is a normal item, "
	  << "with zero weight, appearing in your unit's reports. Silver is used for such things as buying items, and "
	  << "unit's maintenance.\n" << enclose("p", false);

	f << anchor("economy_maintenance") << '\n';
	f << enclose("h3", true) << "Maintenance Costs:\n" << enclose("h3", false);
	f << enclose("p", true) << "IMPORTANT:  Each and every character in Atlantis requires a maintenance fee each "
	  << "month. Anyone who ends the month without this maintenance cost has a " << Globals->STARVE_PERCENT
	  << " percent chance of";
	if (Globals->SKILL_STARVATION != GameDefs::STARVE_NONE) {
		f << " starving, leading to the following effects:\n" << enclose("p", false);
		f << enclose("ul", true);
		f << enclose("li", true);
		if (Globals->SKILL_STARVATION == GameDefs::STARVE_MAGES)
			f << "If the unit is a mage, it";
		else if (Globals->SKILL_STARVATION == GameDefs::STARVE_LEADERS)
			f << "If the unit is a leader, it";
		else
			f << "A unit";
		f << " will lose a skill level in some of its skills.\n";
		f << enclose("li", false);
		if (Globals->SKILL_STARVATION != GameDefs::STARVE_ALL) {
			f << enclose("li", true) << "Otherwise, it will starve to death.\n" << enclose("li", false);
		}
		f << enclose("li", true) << "If a unit should forget a skill level and it knows none, it will starve to "
		  << "death.\n";
		f << enclose("li", false);
		f << enclose("ul", false);
	} else {
		f << " starving to death. ";
	}
	f << "It is up to you to make sure that your people have enough money " << (Globals->UPKEEP_MINIMUM_FOOD > 0
	  ? "and food " : "") << "available. Money " << (Globals->UPKEEP_MINIMUM_FOOD > 0 ? "and food " : "")
	  << "will be shared automatically between your units in the same region, if one is starving and another has "
	  << "more than enough; but this will not happen between units in different regions (this sharing of money "
	  << "applies only for maintenance costs, and does not occur for other purposes). If you have silver in your "
	  << "unclaimed fund, then that silver will be automatically claimed by units that would otherwise starve. ";
	if (Globals->UPKEEP_MINIMUM_FOOD && Globals->ALLOW_WITHDRAW) {
		f << "Similarly, food will automatically be " << url("#withdraw", "withdraw") << "n if needed and "
		  << "unclaimed funds are available. ";
	}
	f << "Lastly, if a faction is allied to yours, their units will provide surplus cash "
	  << (Globals->UPKEEP_MINIMUM_FOOD > 0 ? "or food " : "") << "to your units for maintenance, as a last resort.\n"
	  << enclose("p", false);

	f << enclose("p", true);
	if (Globals->MULTIPLIER_USE == GameDefs::MULT_NONE) {
		f << "This fee is generally " << Globals->MAINTENANCE_COST << " silver for a normal character";
		if (Globals->LEADERS_EXIST) {
			f << ", and " << Globals->LEADER_COST << " silver for a leader";
		}
	} else {
		if (Globals->MULTIPLIER_USE == GameDefs::MULT_MAGES) {
			f << "Mages ";
		} else if (Globals->MULTIPLIER_USE==GameDefs::MULT_LEADERS && Globals->LEADERS_EXIST) {
			f << "Leaders ";
		} else {
			f << "All units ";
		}
		f << " pay a fee based on the number of skill levels the character "
		  << "has.  This fee is the maximum of $" << Globals->MAINTENANCE_MULTIPLIER << " per skill level"
		  << " and a cost of $" << Globals->MAINTENANCE_COST << " for normal characters"
		  << " or $" << Globals->LEADER_COST << " for leaders";
		if (Globals->MULTIPLIER_USE != GameDefs::MULT_ALL) {
			f << ". All other characters pay a fee of $" << Globals->MAINTENANCE_COST << " for a normal character" ;
			if (Globals->LEADERS_EXIST) {
				f << ", and $" << Globals->LEADER_COST << " for a leader";
			}
		}
	}
	f << ".";
	if (Globals->FOOD_ITEMS_EXIST && Globals->UPKEEP_FOOD_VALUE > 0) {
		f << " Units may substitute one unit of ";
		bool first = true;
		int last = -1;
		for (int i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_FOOD)) continue;
			if (last != -1) {
				if (!first) f << ", ";
				f << ItemDefs[last].names;
				first = false;
			}
			last = i;
		}
		if (!first) f << " or ";
		f << ItemDefs[last].names << " for each " << Globals->UPKEEP_FOOD_VALUE << " silver ";
		bool evenly_divisible = Globals->MAINTENANCE_COST % Globals->UPKEEP_FOOD_VALUE == 0;
		if (!evenly_divisible) f << "(or fraction thereof) ";
		f << "of maintenance owed. ";
		if (!evenly_divisible) {
			f << "Food value for a fractional maintenance cost still consumes the entire unit of food. ";
		}

		if (Globals->UPKEEP_MINIMUM_FOOD > 0) {
			f << "A unit must be given at least " << Globals->UPKEEP_MINIMUM_FOOD
			  << " maintenance per man in the form of food. ";
		}
		if (Globals->UPKEEP_MAXIMUM_FOOD >= 0) {
			f << "At most " << Globals->UPKEEP_MAXIMUM_FOOD << " silver worth of food can be counted against each "
			  << "man's maintenance. ";
		}
		f << "A unit may use the " << url("#consume", "CONSUME") << " order to specify that it wishes to use food "
		  << "items in preference to silver.  Note that these items are worth more when sold in towns, so selling "
		  << "them and using the money is more economical than using them for maintenance.";
	};
	f << '\n' << enclose("p", false);
	f << enclose("p", true);
	f << "Maintenance costs are paid in the following order:";
	f << enclose("ol", true);
	if (Globals->FOOD_ITEMS_EXIST && Globals->UPKEEP_FOOD_VALUE > 0) {
		f << enclose("li", true) << "Food items the unit owns if the unit is set " << url("#consume", "CONSUME UNIT")
		  << "or " << url("#consume", "CONSUME FACTION") << "\n" << enclose("li", false);
		f << enclose("li", true) << "Food items from faction units in the same region if the unit is set "
		  << url("#consume", "CONSUME FACTION") << "\n" << enclose("li", false);
	}
	f << enclose("li", true) << "Silver in the unit's possession\n" << enclose("li", false);
	f << enclose("li", true) << "Silver from other faction units in the same region.\n" << enclose("li", false);
	if (Globals->FOOD_ITEMS_EXIST && Globals->UPKEEP_FOOD_VALUE > 0) {
		f << enclose("li", true) << "Food items in the unit's possession.\n" << enclose("li", false);
		f << enclose("li", true) << "Food items from faction units in the same region.\n" << enclose("li", false);
	}
	f << enclose("li", true) << "Unclaimed silver.\n" << enclose("li", false);
	f << enclose("li", true) << "Silver from allied units in the same region.\n" << enclose("li", false);
	if (Globals->FOOD_ITEMS_EXIST && Globals->UPKEEP_FOOD_VALUE > 0) {
		f << enclose("li", true) << "Food items from allied units in the same region.\n" << enclose("li", false);
	}
	f << enclose("ol", false);
	f << enclose("p", false);

	f << anchor("economy_recruiting") << '\n';
	f << enclose("h3", true) << "Recruiting:\n" << enclose("h3", false);
	f << enclose("p", true) << "People may be recruited in a region.  The total amount of recruits available per "
	  << "month in a region, and the amount that must be paid per person recruited, are shown in the region "
	  << "description. The " << url("#buy", "BUY") << " order is used to recruit new people. New recruits will "
	  << "not have any skills or items.  Note that the process of recruiting a new unit is somewhat counter-"
	  << "intuitive; it is necessary to " << url("#form", "FORM") << " an empty unit, " << url("#give", "GIVE")
	  << " the empty unit some money, and have it " << url("#buy", "BUY") << " people; see the description of the "
	  << url("#form", "FORM") << " order for further details.\n"
	  << enclose("p", false);
	f << anchor("economy_items") << '\n';
	f << enclose("h3", true) << "Items:\n" << enclose("h3", false);
	f << enclose("p", true) << "A unit may have a number of possessions, referred to as \"items\".  Some details "
	  << "were given above in the section on Movement, but many things were left out. Here is a table "
	  << "giving some information about common items in Atlantis:\n"
	  << enclose("p", false);

	f << anchor("tableiteminfo") << '\n';
	f << enclose("center", true);
	f << enclose("table border=\"1\"", true);
	f << enclose("tr", true);
	f << enclose("td", true) << "&nbsp;\n" << enclose("td", false);
	f << enclose("th", true) << "Skill (min level)\n" << enclose("th", false);
	f << enclose("th", true) << "Material\n" << enclose("th", false);
	f << enclose("th", true) << "Production time\n" << enclose("th", false);
	f << enclose("th", true) << "Weight (capacity)\n" << enclose("th", false);
	f << enclose("th", true) << "Extra Information\n" << enclose("th", false);
	f << enclose("tr", false);
	for (int i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[i].type & IT_NORMAL)) continue;
		pS = FindSkill(ItemDefs[i].pSkill);
		if (pS && (pS->flags & SkillType::DISABLED)) continue;
		last = 0;
		for (int j = 0; j < (int) (sizeof(ItemDefs->pInput) / sizeof(ItemDefs->pInput[0])); j++) {
			int k = ItemDefs[i].pInput[j].item;
			if (k != -1 &&
					!(ItemDefs[k].flags & ItemType::DISABLED) &&
					!(ItemDefs[k].type & IT_NORMAL))
				last = 1;
		}
		if (last == 1) continue;
		f << enclose("tr", true);
		f << enclose("td align=\"left\" nowrap", true) << ItemDefs[i].name << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true);
		if (pS != NULL) {
			f << pS->name << " (" << ItemDefs[i].pLevel << ")\n";
		}
		f << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true);
		comma = 0;
		if (ItemDefs[i].flags & ItemType::ORINPUTS)
			f << "Any of : ";
		for (int j = 0; j < (int) (sizeof(ItemDefs->pInput) / sizeof(ItemDefs->pInput[0])); j++) {
			int k = ItemDefs[i].pInput[j].item;
			if (k < 0 || (ItemDefs[k].flags&ItemType::DISABLED)) continue;
			f << (comma ? ", " : "") << ItemDefs[i].pInput[j].amt << " "
			  << (ItemDefs[i].pInput[j].amt > 1 ? ItemDefs[k].names : ItemDefs[k].name);
			comma = 1;
		}
		f << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true);
		if (ItemDefs[i].pMonths) {
			f << ItemDefs[i].pMonths << plural(ItemDefs[i].pMonths, " month", " months") << '\n';
		} else {
			f << "&nbsp;\n";
		}
		f << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true);
		f << ItemDefs[i].weight;
		cap = ItemDefs[i].walk - ItemDefs[i].weight;
		if (ItemDefs[i].walk || (ItemDefs[i].hitchItem != -1)) {
			f << " (";
			if (ItemDefs[i].hitchItem == -1)
				f << cap;
			else {
				f << (cap + ItemDefs[i].hitchwalk) << " with " << ItemDefs[ItemDefs[i].hitchItem].name;
			}
			f << ")";
		}
		f << '\n' << enclose("td", false);
		f << enclose("td align=\"left\"", true);
		if (ItemDefs[i].type & IT_WEAPON) {
			WeaponType *wp = FindWeapon(ItemDefs[i].abr);
			if (wp->attackBonus || wp->defenseBonus ||
					(wp->flags & WeaponType::RANGED) ||
					(wp->flags & WeaponType::NEEDSKILL)) {
				if (wp->flags & WeaponType::RANGED)
					f << "Ranged weapon";
				else
					f << "Weapon";
				f << " which gives " << (wp->attackBonus > -1 ? "+" : "")
				  << wp->attackBonus << " on attack and " << (wp->defenseBonus > -1 ? "+" : "")
				  << wp->defenseBonus << " on defense";
				if (wp->flags & WeaponType::NEEDSKILL) {
					pS = FindSkill(wp->baseSkill);
					if (pS && !(pS->flags & SkillType::DISABLED))
						f << " (needs " << pS->name;
					pS = FindSkill(wp->orSkill);
					if (pS && !(pS->flags & SkillType::DISABLED))
						f << " or " << pS->name;
					f << " skill)";
				}
				f << ".<br />";
			}
			if (wp->numAttacks < 0) {
				f << "Gives 1 attack every " << -wp->numAttacks << " rounds.<br />";
			}
		}
		if (ItemDefs[i].type & IT_MOUNT) {
			MountType *mp = FindMount(ItemDefs[i].abr);
			pS = FindSkill(mp->skill);
			if (pS && !(pS->flags & SkillType::DISABLED)) {
				f << "Gives a riding bonus with the " << pS->name << " skill.<br />";
			}
		}
		if (ItemDefs[i].type & IT_ARMOR) {
			ArmorType *at = FindArmor(ItemDefs[i].abr);
			f << "Gives a " << at->saves[SLASHING] << " in " << at->from
			  << " chance to survive a normal hit.<br />"
			  << ((at->flags & ArmorType::USEINASSASSINATE && has_stea)
			      ? "May be used during assassinations.<br />"
				  : "");
		}
		if (ItemDefs[i].type & IT_TOOL) {
			for (int j = 0; j < NITEMS; j++) {
				if (ItemDefs[j].flags & ItemType::DISABLED) continue;
				if (ItemDefs[j].mult_item != i) continue;
				if (!(ItemDefs[j].type & IT_NORMAL)) continue;
				pS = FindSkill(ItemDefs[j].pSkill);
				if (!pS || (pS->flags & SkillType::DISABLED)) continue;
				last = 0;
				for (int k = 0; k < (int) (sizeof(ItemDefs->pInput) / sizeof(ItemDefs->pInput[0])); k++) {
					int l = ItemDefs[j].pInput[k].item;
					if (l != -1 &&
							!(ItemDefs[l].flags & ItemType::DISABLED) &&
							!(ItemDefs[l].type & IT_NORMAL))
						last = 1;
				}
				if (last == 1) continue;
				f << "+" << ItemDefs[j].mult_val << " bonus when producing "
				  << ItemDefs[j].names << ".<br />";
			}
		}
		f << '\n' << enclose("td", false);
		f << enclose("tr", false);
	}
	f << enclose("table", false);
	f << enclose("center", false);

	f << enclose("p", true) << "All items except silver and trade goods are produced with the "
	  << url("#produce", "PRODUCE") << " order. Producing items will always produce as many items as "
	  << "during a month up to the limit of the supplies carried by the producing unit. The required skills "
	  << "and raw materials required to produce one output item are in the table above.\n"
	  << enclose("p", false);

	f << enclose("p", true) << "If an item requires raw materials, then the specified amount of each "
	  << "material is consumed for each item produced. The higher the skill of the unit, the more "
	  << "productive each man-month of work will be.  Thus, five men at skill level one are exactly "
	  << "equivalent to one man at skill level 5 in terms of base output. Items which require multiple "
	  << "man-months to produce will still benefit from higher skill level units, just not as "
	  << "quickly.  For example, if a unit of six level one men wanted to produce something which required "
	  << "three man-months per item, that unit could produce two of them in one month.  If their skill level "
	  << "was raised to two, then they could produce four of them in a month. At level three, they could "
	  << "then produce 6 per month.\n" << enclose("p", false);

	f << enclose("p", true) << "Some items may allow each man to produce multiple output items per raw material "
	  << "or have other differences from these basic rules.  Those items will explain their differences in the "
	  << "description of the item.\n" << enclose("p", false);

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f << enclose("p", true) << " Only "
		  << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "Trade" : "Martial")
		  << " factions can issue " << url("#produce", "PRODUCE") << " orders however, regardless of skill levels.\n"
		  << enclose("p", false);
	}

	f << enclose("p", true) << "Items which increase production may increase production of advanced items in "
	  << "addition to the basic items listed.  Some of them also increase production of other tools.  Read the skill "
	  << "descriptions for details on which tools aid which production when not noted above.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If an item does not list a raw material it may be produced directly from the land. "
	  << "Each region generally has at least one item that can be produced there.  Shown on the description of a "
	  << "region is a list of the items that can be produced, and the amount of each that can be produced per month.  "
	  << "This amount depends on the region type. "
	  << (Globals->RANDOM_ECONOMY ? "It also varies from region to region of the same type. " : "")
	  << "If the units in a region attempt to produce more of a commodity than can be produced that month, then the "
	  << "amount available is distributed among the producers\n"
	  << enclose("p", false);

	if (Globals->TOWNS_EXIST) {
		f << anchor("economy_towns") << '\n';
		f << enclose("h3", true) << "Villages, Towns, and Cities:\n" << enclose("h3", false);
		f << enclose("p", true) << "Some regions in Atlantis contain villages, towns, and cities.  Villages add to "
		  << "the wages, population, and tax income of the region they are in. "
		  << (Globals->FOOD_ITEMS_EXIST
		      ? "Also, villages will have an additional market for grain, livestock, and fish. "
			  : "")
		  << "As the village's demand for these goods is met, the population will increase. When the population "
		  << "reaches a certain threshold, the village will turn into a town.  A town will have some additional "
		  << "products that it demands, in addition to what it previously wanted.  Also a town will sell some new "
		  << "items as well. A town whose demands are being met will grow, and above another threshold it will become "
		  << "a full-blown city.  A city will have additional markets for common items, and will also have markets for "
		  << "less common, more expensive trade items.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Trade items are bought and sold only by cities, and have no other practical uses.  "
		  << "However, the profit margins on these items are usually quite high. \n"
		  << enclose("p", false);
	}
	f << anchor("economy_buildings") << '\n';
	f << enclose("h3", true) << "Buildings and Trade Structures:\n" << enclose("h3", false);
	f << enclose("p", true) << "Construction of buildings " << (may_sail ? "and ships " : "")
	  << "goes as follows: each unit of work on a building requires a unit of the required resource and a "
	  << "man-month of work by a character with the appropriate skill and level; higher skill levels allow "
	  << "work to proceed faster (still using one unit of the required resource per unit of work done). ";
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if (Globals->BUILD_NO_TRADE)
			f << "Any faction can issue ";
		else
			f << "Only " << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "Trade" : "Martial")
			  << " factions can issue ";
		f << url("#build", "BUILD") << " orders. ";
	}
	f << "Here is a table of the various building types:\n"
	  << enclose("p", false);

	f << anchor("tablebuildings") << '\n';
	f << enclose("center", true);
	f << enclose("table border=\"1\"", true);
	f << enclose("tr", true);
	f << enclose("td", true) << "\n" << enclose("td", false);
	f << enclose("th", true) << "Size\n" << enclose("th", false);
	f << enclose("th", true) << "Cost\n" << enclose("th", false);
	f << enclose("th", true) << "Material\n" << enclose("th", false);
	f << enclose("th", true) << "Skill (min level)\n" << enclose("th", false);
	if (Globals->LIMITED_MAGES_PER_BUILDING) {
		f << enclose("th", true) << "Mages\n" << enclose("th", false);
	}
	f << enclose("tr", false);
	for (int i = 0; i < NOBJECTS; i++) {
		if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
		if (!ObjectDefs[i].protect) continue;
		pS = FindSkill(ObjectDefs[i].skill);
		if (pS == NULL) continue;
		if (pS->flags & SkillType::MAGIC) continue;
		if (ObjectIsShip(i)) continue;
		int j = ObjectDefs[i].item;
		if (j == -1) continue;
		/* Need the >0 since item could be WOOD_OR_STONE (-2) */
		if (j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
		if (j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
		/* Okay, this is a valid object to build! */
		f << enclose("tr", true);
		f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].name << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].protect << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].cost << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << (j == I_WOOD_OR_STONE ? "wood or stone" : ItemDefs[j].name)
		  << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << pS->name << " (" << ObjectDefs[i].level << ")\n"
		  << enclose("td", false);
		if (Globals->LIMITED_MAGES_PER_BUILDING) {
			f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].maxMages << '\n' << enclose("td", false);
		}
		f << enclose("tr", false);
	}
	f << enclose("table", false);
	f << enclose("center", false);

	f << enclose("p", true) << "Size is the number of people that the building can shelter. Cost is both "
	  << "the number of man-months of labor and the number of units of material required to complete the building.  ";
	if (Globals->LIMITED_MAGES_PER_BUILDING) {
		f << "Mages is the number of mages that the building provides study facilities for, to enable unhindered "
		  << "study above level 2 in magical skills.  ";
	}
	f << "There are possibly other buildings which can be built that require more advanced resources, or odd "
	  << "skills to construct.   The description of a skill will include any buildings which it allows to be built.\n"
	  << enclose("p", false);

	f << enclose("p", true) << "There are other structures that increase the maximum production of certain items "
	  << "in regions";
	if (!(ObjectDefs[O_MINE].flags & ObjectType::DISABLED))
		f << "; for example, a Mine will increase the amount of iron that is available to be mined in a region";
	f << ".  To construct these structures requires a high skill level in the production skill related to the item "
	  << "that the structure will help produce. ";
	if (!(ObjectDefs[O_INN].flags & ObjectType::DISABLED)) {
		f << "(Inns are an exception to this rule, requiring the Building skill, not the Entertainment skill.) ";
	}
	f << "This bonus in production is available to any unit in the region; there is no need to be inside the "
	  << "structure.\n" << enclose("p", false);

	f << enclose("p", true) << "The first structure built in a region will increase the maximum production of the "
	  << "related product by 25%; the amount added by each additional structure will be half of the the effect of "
	  << "the previous one.  (Note that if you build enough of the same type of structure in a region, the new "
	  << "structures may not add _any_ to the production level).\n"
	  << enclose("p", false);

	f << anchor("tabletradestructures") << '\n';
	f << enclose("center", true);
	f << enclose("table border=\"1\"", true);
	f << enclose("tr", true);
	f << enclose("td", true) << "\n" << enclose("td", false);
	f << enclose("th", true) << "Cost\n" << enclose("th", false);
	f << enclose("th", true) << "Material\n" << enclose("th", false);
	f << enclose("th", true) << "Skill (level)\n" << enclose("th", false);
	f << enclose("th", true) << "Production Aided\n" << enclose("th", false);
	f << enclose("tr", false);
	for (int i = 0; i < NOBJECTS; i++) {
		if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
		if (ObjectDefs[i].protect) continue;
		if (ObjectIsShip(i)) continue;
		int j = ObjectDefs[i].productionAided;
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
		f << enclose("tr", true);
		f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].name << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].cost << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << (j == I_WOOD_OR_STONE ? "wood or stone" : ItemDefs[j].name)
		  << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << pS->name;
		if (ObjectDefs[i].level > 1)
			f << " (" << ObjectDefs[i].level << ")";
		f << '\n' << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true)
		  << (ObjectDefs[i].productionAided == I_SILVER
		      ? "entertainment"
			  : ItemDefs[ObjectDefs[i].productionAided].names)
		  << '\n' << enclose("td", false);
		f << enclose("tr", false);
	}
	f << enclose("table", false);
	f << enclose("center", false);

	f << enclose("p", true) << "Note that these structures will not increase the availability of an item in a "
	  << "region which does not already have that item available. Also, Trade structures do not offer "
	  << "defensive bonuses (which is why they do not have a size associated with them).  As with regular "
	  << "buildings, the Cost is the number of man-months of labor and also the number of units of raw "
	  << "material required to complete the structure. ";
	if (!(ItemDefs[I_WOOD].flags & ItemType::DISABLED) && !(ItemDefs[I_STONE].flags & ItemType::DISABLED))
		f << "You can use two different materials (wood or stone) to construct most trade structures. ";
	f << "It is possible that there are structures not listed above which require either advanced resources "
	  << "to build or which increase the production of advanced resources.  The skill description for a skill "
	  << "will always note if new structures may be built based on knowing that skill.\n"
	  << enclose("p", false);

	if (!(ObjectDefs[O_ROADN].flags & ObjectType::DISABLED)) {
		f << anchor("economy_roads") << '\n';
		f << enclose("h3", true) << "Roads:\n" << enclose("h3", false);
		f << enclose("p", true) << "There is a another type of structure called roads.  They do not protect units, "
		  << "nor aid in the production of resources, but do aid movement, and can improve the economy of a hex.\n"
		  << enclose("p", false);

		f << enclose("p", true) << "Roads are directional and are only considered to reach from one hexside to the "
		  << "center of the hex.  To gain a movement bonus, there must be two connecting roads, one in each adjacent "
		  << "hex.  Only one road may be built in each direction. If a road in the given direction is connected, "
		  << "units move along that road at half cost to a minimum of 1 movement point.\n"
		  << enclose("p", false);

		f << enclose("p", true) << "For example: If a unit is moving northwest, then the hex it is in must have a "
		  << "northwest road, and the hex it is moving into must have a southeast road.\n"
		  << enclose("p", false);

		f << enclose("p", true) << "To gain an economy bonus, a hex must have roads that connect to roads in at "
		  << "least two adjoining hexes.  The economy bonus for the connected roads raises the wages in the "
		  << "region by 1 point.\n"
		  << enclose("p", false);

		f << anchor("tableroadstructures") << '\n';
		f << enclose("center", true);
		f << enclose("table border=\"1\"", true);
		f << enclose("tr", true);
		f << enclose("td", true) << "\n" << enclose("td", false);
		f << enclose("th", true) << "Cost\n" << enclose("th", false);
		f << enclose("th", true) << "Material\n" << enclose("th", false);
		f << enclose("th", true) << "Skill (min level)\n" << enclose("th", false);
		f << enclose("tr", false);
		for (int i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (ObjectDefs[i].productionAided != -1) continue;
			if (ObjectDefs[i].protect) continue;
			if (ObjectIsShip(i)) continue;
			pS = FindSkill(ObjectDefs[i].skill);
			if (pS == NULL) continue;
			if (pS->flags & SkillType::MAGIC) continue;
			int j = ObjectDefs[i].item;
			if (j == -1) continue;
			/* Need the >0 since item could be WOOD_OR_STONE (-2) */
			if (j > 0 && (ItemDefs[j].flags & ItemType::DISABLED)) continue;
			if (j > 0 && !(ItemDefs[j].type & IT_NORMAL)) continue;
			/* Okay, this is a valid object to build! */
			f << enclose("tr", true);
			f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].name << '\n' << enclose("td", false);
			f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].cost << '\n' << enclose("td", false);
			f << enclose("td align=\"left\" nowrap", true)
			  << (j == I_WOOD_OR_STONE ? "wood or stone" : ItemDefs[j].name) << '\n'
			  << enclose("td", false);
			f << enclose("td align=\"left\" nowrap", true) << pS->name << " (" << ObjectDefs[i].level << ")\n"
			  << enclose("td", false);
			f << enclose("tr", false);
		}
		f << enclose("table", false);
		f << enclose("center", false);
	}
	if (Globals->DECAY) {
		f << anchor("economy_builddecay") << '\n';
		f << enclose("h3", true) << "Building Decay:\n" << enclose("h3", false);
		f << enclose("p", true) << "Some structures will decay over time if they are not maintained. "
			<< "Difficult terrain and bad weather will speed up this decay. Maintnenance involves having units "
			<< "with the appropriate level of skill expend a small amount of the material used to build the "
			<< "structure and labor on a fairly regular basis in the exactly same manner as they would work on "
			<< "the building it if it was not completed. In other words, enter the structure and issue the BUILD "
			<< "command with no parameters. If a structure will need maintenance, that information will be related "
			<< "in the object information given about the structure. If a structure is allowed to decay, it will "
			<< "not give any of its bonuses until it is repaired.\n"
			<< enclose("p", false);
	}
	if (may_sail) {
		f << anchor("economy_ships") << '\n';
		f << enclose("h3", true) << "Ships:\n" << enclose("h3", false);
		f << enclose("p", true)
			<< "Ships are constructed similarly to buildings, with a few small differences. "
			<< "Firstly, they tend to be constructed out of wood, not stone. Secondly, their "
			<< "construction tends to depend on the Shipbuilding skill, not the Building skill. "
			<< "Thirdly, while unfinished buildings appear in the region, and may be entered by "
			<< "other units, unfinished ships appear only in their builder's inventory until "
			<< "they are complete.  If the builder " << url("#move", "MOVE") << "s or they are in "
			<< "a fleet that " << url("#sail", "SAIL") << "s while they have an unfinished ship "
			<< "in their possession, the ship will be discarded and lost. Finally, ships are "
			<< "never interacted with as objects directly, but when completed are placed in fleets. "
			<< "Fleets may contain one or more ships, and may be entered like other buildings.\n"
			<< enclose("p", false);
		f << enclose("p", true);
		if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			if (Globals->BUILD_NO_TRADE)
				f << "Any faction ";
			else
				f << "Only " << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "Trade" : "Martial")
				  << " factions can issue ";
			f << "can issue " << url("#build", "BUILD") << " orders. ";
		}
		f << "Here is a table of the various ship types:\n"
		  << enclose("p", false);

		f << anchor("tableshipinfo") << '\n';
		f << enclose("center", true);
		f << enclose("table border=\"1\"", true);
		f << enclose("tr", true);
		f << enclose("td", true) << "Class\n" << enclose("td", false);
		f << enclose("th", true) << "Capacity\n" << enclose("th", false);
		f << enclose("th", true) << "Cost\n" << enclose("th", false);
		f << enclose("th", true) << "Sailors\n" << enclose("th", false);
		f << enclose("th", true) << "Skill\n" << enclose("th", false);
		f << enclose("tr", false);
		for (int i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (!(ItemDefs[i].type & IT_SHIP)) continue;
			int pub = 1;
			for (int c = 0; c < (int) sizeof(ItemDefs->pInput)/(int) sizeof(ItemDefs->pInput[0]); c++) {
				int m = ItemDefs[i].pInput[c].item;
				if (m != -1) {
					if (ItemDefs[m].flags & ItemType::DISABLED) pub = 0;
					if ((ItemDefs[m].type & IT_ADVANCED) ||	(ItemDefs[m].type & IT_MAGIC)) pub = 0;
				}
			}
			if (pub == 0) continue;
			if (ItemDefs[i].mLevel > 0) continue;
			int slevel = ItemDefs[i].pLevel;
			if (slevel > 3) continue;
			f << enclose("tr", true);
			f << enclose("td align=\"center\"", true) << ItemDefs[i].name << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ItemDefs[i].swim << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ItemDefs[i].pMonths << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ItemDefs[i].weight/50 << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << slevel << '\n' << enclose("td", false);
			f << enclose("tr", false);
		}
		for (int i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (!ObjectIsShip(i)) continue;
			if (ItemDefs[ObjectDefs[i].item].flags & ItemType::DISABLED)
				continue;
			int normal = (ItemDefs[ObjectDefs[i].item].type & IT_NORMAL);
			normal |= (ItemDefs[ObjectDefs[i].item].type & IT_TRADE);
			if (!normal) continue;
			f << enclose("tr", true);
			f << enclose("td align=\"center\"", true) << ObjectDefs[i].name << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ObjectDefs[i].capacity << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ObjectDefs[i].cost << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << ObjectDefs[i].sailors << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << 1 << '\n' << enclose("td", false);
			f << enclose("td align=\"center\"", true) << "no" << '\n' << enclose("td", false);
			f << enclose("tr", false);
		}
		f << enclose("table", false);
		f << enclose("center", false);

		f << enclose("p", true) << "The capacity of a ship is the maximum weight that the ship may have aboard "
		  << "and still move. The cost is both the man-months of labor and the number of units of material required "
		  << "to complete the ship. The sailors are the number of skill levels of the Sailing skill that must be "
		  << "aboard the ship (and issuing the " << url("#sail", "SAIL") << " order) in order for the ship to sail.\n"
		  << enclose("p", false);

		f << enclose("p", true) << "When a ship is built, if its builder is already the owner of a fleet, then "
		  << "the ship will be added to that fleet; otherwise a new fleet will be created to hold the ship.  A fleet "
		  << "has the combined capacity and sailor requirement of its constituent vessels, and moves at the speed of "
		  << "its slowest ship.\n" << enclose("p", false);
	}
	f << anchor("economy_advanceditems") << '\n';
	f << enclose("h3", true) << "Advanced Items:\n" << enclose("h3", false);
	f << enclose("p", true) << "There are also certain advanced items that highly skilled units can produce. "
	  << "These are not available to starting players, but can be discovered through study.  When a unit is skilled "
	  << "enough to produce one of these items, he will receive a skill report describing the production of this "
	  << "item. Production of advanced items is generally done in a manner similar to the normal items.\n"
	  << enclose("p", false);
	f << anchor("economy_income") << '\n';
	f << enclose("h3", true) << "Income:\n" << enclose("h3", false);
	f << enclose("p", true) << "Units can earn money with the " << url("#work", "WORK") << " order.  This means "
	  << "that the unit spends the month performing manual work for wages. The amount to be earned from this is "
	  << "usually not very high, so it is generally a last resort to be used if one is running out of money. The "
	  << "current wages are shown in the region description for each region. All units may " << url("#work", "WORK")
	  << ", regardless of skills"
	  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES ? " or faction type" : "")
	  << ".\n" << enclose("p", false);

	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		f << anchor("economy_entertainment") << '\n';
		f << enclose("h3", true) << "Entertainment:\n" << enclose("h3", false);
		f << enclose("p", true)
		  << "Units with the Entertainment skill can use it to earn money.  A unit with Entertainment level 1 will "
		  << "earn " << Globals->ENTERTAIN_INCOME << " silver per man by issuing the "
		  << url("#entertain", "ENTERTAIN") << " order.  The total amount of money that can be earned this way "
		  << "is shown in the region descriptions.  Higher levels of Entertainment skill can earn more, so a "
		  << "character with Entertainment skill 2 can earn twice as much money as one with skill 1 (and uses "
		  << "twice as much of the demand for entertainment in the region). Note that entertainment income is "
		  << "much less, per region, than the income available through working or taxing."
		  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES
		      ? " All factions may have entertainers, regardless of faction type."
			  : "")
		  << '\n' << enclose("p", false);
	}

	f << anchor("economy_taxingpillaging") << '\n';
	f << enclose("h3", true) << "Taxing/Pillaging:\n" << enclose("h3", false);
	f << enclose("p", true)
	  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES ?
	      (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "War factions" : "Martial factions") :
		  "Any faction")
	  << " may collect taxes in a region.  This is done using the " << url("#tax", "TAX")
	  << " order (which is " << (Globals->TAX_PILLAGE_MONTH_LONG ? "" : "not ") << "a full month order). "
	  << "The amount of tax money that can be collected each month in a region is shown in the region description. ";
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE) {
		f << "Any unit may " << url("#tax", "TAX");
	} else {
		f << "A unit may " << url("#tax", "TAX") << " if it " << unit_tax_description();
	}
	if (Globals->WHO_CAN_TAX &
			(GameDefs::TAX_CREATURES | GameDefs::TAX_ILLUSIONS)) {
		if (Globals->WHO_CAN_TAX & GameDefs::TAX_CREATURES) {
			f << "Summoned " << (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS ? "and illusory " : "");
		} else if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS)
			f << "Illusory ";
		f << "creatures will assist in taxation. ";
	}
	f << "Each taxing character can collect $" << Globals->TAX_BASE_INCOME << ", though if the number of "
	  << "taxers would tax more than the available tax income, the tax income is split evenly among all taxers.\n"
	  << enclose("p", false);

	f << enclose("p", true)
	  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES
		  ? (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "War factions" : "Martial factions")
		  : "Any faction")
	  << " may also pillage a region. To do this requires the faction to have enough combat ready men in the region "
	  << "to tax half of the available money in the region. The total amount of money that can be pillaged will then "
	  << "be shared out between every combat ready unit that issues the " << url("#pillage", "PILLAGE")
	  << " order. The amount of money collected is equal to twice the available tax money. However, the economy of "
	  << "the region will be seriously damaged by pillaging, and will only slowly recover over time.  Note that "
	  << url("#pillage", "PILLAGE") << " comes before " << url("#tax", "TAX") << ", so a unit performing "
	  << url("#tax", "TAX") << " will collect no money in that region that month.\n"
	  << enclose("p", false);

	f << enclose("p", true) << "It is possible to safeguard one's tax income in regions one controls.  Units"
	  << " which have the Guard flag set (using the " << url("#guard", "GUARD") << " order) will block "
	  << url("#tax", "TAX") << " orders issued by other factions in the same region, unless you have declared "
	  << "the faction in question Friendly. Units on guard will also block " << url("#pillage", "PILLAGE")
	  << " orders issued by other factions in the same region, regardless of your attitude towards the faction "
	  << "in question, and they will attempt to prevent Unfriendly units from entering the region.  Only units "
	  << "which are able to tax may be on guard.  Units on guard "
	  << (has_stea ? " are always visible regardless of Stealth skill, and " : "")
	  << "will be marked as being \"on guard\" in the region description.\n"
	  << enclose("p", false);

	if (qm_exist) {
		f << anchor("economy_transport") << '\n';
		f << enclose("H3", true) << "Transportation of goods\n" << enclose("H3", false);
		f << enclose("p", true)
		  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES
			  ? (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "Trade factions" : "Martial factions")
			  : "Any faction")
		  << " may train Quartermaster units. A Quartermaster unit may accept " << url("#transport", "TRANSPORT")
		  << "ed items from any unit within " << Globals->LOCAL_TRANSPORT << ' '
		  << plural(Globals->LOCAL_TRANSPORT, "hex", "hexes") << " distance from the hex containing the quartermaster. "
		  << "Quartermasters may also " << url("#transport", "TRANSPORT") << " items to any unit within "
		  << Globals->LOCAL_TRANSPORT << ' ' << plural(Globals->LOCAL_TRANSPORT, "hex", "hexes")
		  << " distance from the hex containing the quartermaster and may " << url("#transport", "TRANSPORT")
		  << " items to another quartermaster up to " << Globals->NONLOCAL_TRANSPORT << ' '
		  << plural(Globals->NONLOCAL_TRANSPORT, "hex", "hexes") << " distant.";
		if (Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST) {
			f << " The distance a quartermaster can " << url("#transport", "TRANSPORT") << " items to another "
			  << "quartermaster will increase with the level of skill possessed by the quartermaster unit.";
		}
		f << '\n' << enclose("p", false);

		f << enclose("p", true) << "In order to accomplish this function, a quartermaster must be the owner of a "
		  << "structure which allows transportation of items.  The structures which allow this are: ";
		last = -1;
		comma = 0;
		for (int i = 0; i < NOBJECTS; i++) {
			if (!(ObjectDefs[i].flags & ObjectType::TRANSPORT)) continue;
			if (last == -1) {
				last = i;
				continue;
			}
			f << ObjectDefs[i].name << ", ";
			comma++;
			last = i;
		}
		if (comma) f << "and ";
		f << ObjectDefs[last].name << ".\n" << enclose("p", false);

		if (Globals->SHIPPING_COST > 0) {
			f << enclose("p", true) << "The cost of transport items from one quartermaster to "
			  << "another is based on the weight of the items and costs " << Globals->SHIPPING_COST
			  << " silver per weight unit.";
			if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST) {
				f << " The cost of shipping is increased for units with a lower quartermaster skill, dropping "
				  << "to the minimum above when the unit is at the maximum skill level.";
			}
			f << '\n' << enclose("p", false);
		}

		f << enclose("p", true) << "Quartermasters must be single man units"
		  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES
		      ? ", and a faction is limited in the number of quartermasters it may have at any one time"
			  : "")
		  << ". The " << url("#transport", "TRANSPORT") << " order counts as trade activity in "
		  << "the hex of the unit issuing the order. The target unit must be at least FRIENDLY "
		  << "to the unit which issues the order.\n"
		  << enclose("p", false);

		f << enclose("p", true) << "Not all type of items can be " << url("#transport", "TRANSPORT") << "ed to "
		  << "or " << url("#transport", "TRANSPORT") << "ed by a quartermaster. Men (including Leaders), "
		  << "summoned creatures (including illusionary ones), ships, mounts, war machines, and items created "
		  << "using artifact lore need to be carried/sailed from one location to another by a unit.\n"
		  << enclose("p", false);
	}

	f << anchor("com") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Combat\n" << enclose("h2", false);
	f << enclose("p", true) << "Combat occurs when one unit attacks another.  The computer then gathers together "
	  << "all the units on the attacking side, and all the units on the defending side, and the two sides fight "
	  << "until an outcome is reached.\n" << enclose("p", false);

	f << anchor("com_attitudes") << '\n';
	f << enclose("h3", true) << "Attitudes:\n" << enclose("h3", false);
	f << enclose("p", true) << "Which side a faction's units will fight on depends on declared attitudes.  "
	  << "A faction can have one of the following attitudes towards another faction:  Ally, Friendly, Neutral, "
	  << "Unfriendly or Hostile.  Each faction has a general attitude, called the \"Default Attitude\", that it "
	  << "normally takes towards other factions; this is initially Neutral, but can be changed.  It is also "
	  << "possible to " << url("#declare", "DECLARE") << " attitudes to specific factions, e.g. "
	  << url("#declare", "DECLARE") << " 27 ALLY will declare the Ally attitude to faction 27.  (Note that this "
	  << "does not necessarily mean that faction 27 has decided to treat you as an ally.)\n" << enclose("p", false);
	f << enclose("p", true) << "Ally means that you will fight to defend units of that faction whenever they come "
	  << "under attack, if you have non-avoiding units in the region where the attack occurs. ";
	if (has_stea) {
		f << " You will also attempt to prevent any theft or assassination attempts against units of the faction";
		if (has_obse) {
			f << ", if you are capable of seeing the unit which is attempting the crime";
		}
		f << ". ";
	}
	f << "It also has the implications of the Friendly attitude.\n" << enclose("p", false);
	f << enclose("p", true) << "Friendly means that you will accept gifts from units of that faction.  This "
	  << "includes the giving of items, units of people, and the teaching of skills.  You will also admit units of "
	  << "that faction into buildings or fleets owned by one of your units, and you will permit units of that "
	  << "faction to collect taxes (but not pillage) in regions where you have units on guard.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Unfriendly means that you will not admit units of that faction into any region "
	  << "where you have units on guard.  You will not, however, automatically attack unfriendly units which are "
	  << "already present.\n" << enclose("p", false);
	f << enclose("p", true) << "Hostile means that any of your units which do not have the Avoid Combat flag set "
	  << "(using the " << url("#avoid", "AVOID") << " order) will attack any units of that faction wherever they "
	  << "find them.\n" << enclose("p", false);
	f << enclose("p", true) << "If a unit can see another unit, but "
	  << (has_obse
	      ? "does not have high enough Observation skill to determine its faction, "
		  : "it is not revealing its faction, ")
	  << "it will treat the unit using the faction's default attitude, even if the unit belongs to an Unfriendly "
	  << "or Hostile faction, because it does not know the unit's identity.  However, if your faction has declared "
	  << "an attitude of Friendly or Ally towards that unit's faction, the unit will be treated with the better "
	  << "attitude; it is assumed that the unit will produce proof of identity when relevant."
	  << (has_stea ? " (See the section on stealth for more information on when units can see each other.)" : "")
	  << '\n' << enclose("p", false);
	f << enclose("p", true) << "If a faction declares Unfriendly or Hostile as default attitude (the latter "
	  << "is a good way to die fast), it will block or attack all unidentified units, unless they belong to "
	  << "factions for which a Friendly or Ally attitude has been specifically declared."
	  << (has_stea ? " Units which cannot be seen at all cannot be directly blocked or attacked, of course." : "")
	  << '\n' << enclose("p", false);

	f << anchor("com_attacking") << '\n';
	f << enclose("h3", true) << "Attacking:\n" << enclose("h3", false);
	f << enclose("p", true) << "A unit can attack another by issuing an " << url("#attack", "ATTACK")
	  << " order. A unit that does not have Avoid Combat set will automatically attack any Hostile units it "
	  << "identifies as such.";
	if (has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		f << " When a unit issues the " << url("#attack", "ATTACK") << " order, or otherwise decides to attack "
		  << "another unit, it must first be able to attack the unit. "
		  << (has_stea && !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)
		      ? "There are two conditions for this; the first is that the"
			  : "The");
		if (has_stea) {
			f << " attacking unit must be able to see the unit that it wishes to attack. More information is "
			  << "available on this in the stealth section of the rules.";
		}
		if (!(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
			if (has_stea) {
				f << '\n' << enclose("p", false);
				f << enclose("p", true) << "Secondly, the";
			}
			f << " attacking unit must be able to catch the unit it wishes to attack.  A unit may only "
			  << "catch a unit if its effective Riding skill is greater than or equal to the target unit's "
			  << "effective Riding skill; otherwise, the target unit just rides away from the attacking unit.  "
			  << "Effective Riding is the unit's Riding skill, but with a potential maximum; if the unit can "
			  << "not ride, the effective Riding skill is 0; if the unit can ride, the maximum effective Riding "
			  << "is 3; if the unit can fly, the maximum effective Riding is 5. Note that the effective Riding "
			  << "also depends on whether the unit is attempting to attack or defend; for attack purposes, only "
			  << "one man in the unit needs to be able to ride or fly (generally, this means one of the men "
			  << "must possess a horse, or other form of transportation), whereas for defense purposes the entire "
			  << "unit needs to be able to ride or fly (usually meaning that every man in the unit must possess "
			  << "a horse or other form of speedier transportation). Also, note that for a unit to be able to use "
			  << "its defensive Riding ability to avoid attack, the unit cannot be in a building, fleet, or "
			  << "structure of any type.";
		}
	}
	f << '\n' << enclose("p", false);
	f << enclose("p", true) << "A unit which is on guard, and is Unfriendly towards a unit, will deny access to "
	  << "units using the " << url("#move", "MOVE") << " order to enter its region. ";
	if (has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		f << "Note that to deny access to a unit, at least one unit from the same faction as the unit guarding the "
		  << "hex must satisfy the above requirements. ";
	}
	f << "A unit using " << url("#advance", "ADVANCE") << " instead of " << url("#move", "MOVE")
	  << " to enter a region, will attack any units that attempt to deny it access.  If the advancing unit loses "
	  << "the battle, it will be forced to retreat to the previous region it moved through.  If the unit wins the "
	  << "battle and its army doesn't lose any men, it is allowed to continue to move, provided that it has "
	  << "enough movement points.\n"
	  << enclose("p", false);
	if (has_stea || !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
		f << enclose("p", true) << "Note that "
		  << (has_stea && !(SkillDefs[S_RIDING].flags & SkillType::DISABLED)
			  ? "these restrictions do "
			  : "this restriction does ")
		  << "not apply for sea combat, as "
		  << (has_stea ? "units within a fleet are always visible" : "");
		if (!(SkillDefs[S_RIDING].flags & SkillType::DISABLED)) {
			f << (has_stea ? ", and" : "") << " Riding does not play a part in combat on board fleets";
		}
		f << ".\n" << enclose("p", false);
	}

	f << anchor("com_muster") << '\n';
	f << enclose("h3", true) << "The Muster:\n" << enclose("h3", false);
	f << enclose("p", true) << "Once the attack has been made, the sides are gathered.  Although "
	  << "the " << url("#attack", "ATTACK") << " order takes a unit rather than "
	  << "a faction as its parameter (mainly so that unidentified units can "
	  << "be attacked), an attack is basically considered to be by an entire "
	  << "faction, against an entire faction and its allies.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "On the attacking side are all units of the attacking faction in the region where "
	  << "the fight is taking place, except those with Avoid Combat set.  A unit which has explicitly (or "
	  << "implicitly via " << url("#advance", "ADVANCE") << ") issued an " << url("#attack", "ATTACK")
	  << " order will join the fight anyway, regardless of whether Avoid Combat is set.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Also on the attacking side are all units of other factions that attacked the target "
	  << "faction (implicitly or explicitly) in the region where the fight is taking place.  In other words, if "
	  << "several factions attack one, then all their armies join together to attack at the same time (even if "
	  << "they are enemies and will later fight each other).\n" << enclose("p", false);
	f << enclose("p", true) << "On the defending side are all identifiable units belonging to the defending faction.  "
	  << "If a unit has Avoid Combat set and it belongs to the target faction, it will be uninvolved only if its "
	  << "faction cannot be identified by the attacking faction.  A unit which was explicitly attacked will be "
	  << "involved anyway, regardless of Avoid Combat. "
	  << (has_stea ? "(This means that Avoid Combat is mostly useful for high stealth scouts.) " : "")
	  << "Also, all non-avoiding units located in the target region belonging to factions allied with the defending "
	  << "unit will join in on the defending side"
	  << (Globals->ALLIES_NOAID
	      ? ", provided that at least one of the units belonging to the defending faction is not set to noaid."
		  : ".")
	  << '\n' << enclose("p", false);
	f << enclose("p", true) << "Units in adjacent regions can also become involved.  This is the exception "
	  << "to the general rule that you cannot interact with units in a different region.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If a faction has at least one unit involved in the initial region, then any units "
	  << "in adjacent regions will join the fight, if they could reach the region and do not have Avoid Combat set. "
	  << "There are a few flags that units may set to affect this; a unit with the Hold flag (set using the "
	  << url("#hold", "HOLD") << " order) will not join battles in adjacent regions.  This flag applies to both "
	  << "attacking and defending factions.  A unit with the Noaid flag (set using the " << url("#noaid", "NOAID")
	  << " order) will receive no aid from adjacent hexes when attacked, or when it issues an attack.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:  A fight starts in region A, in the initial combat phase (before any "
	  << "movement has occurred).  The defender has a unit of soldiers in adjacent region B.  They have 2 movement "
	  << "points at this stage. They will buy horses later in the turn, so that when they execute their "
	  << url("#move", "MOVE") << " order they will have 4 movement points, but right now they have 2. "
	  << (Globals->WEATHER_EXISTS ? "Region A is forest, but fortunately it is summer, " : "Region A is forest, ")
	  << "so the soldiers can join the fight.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "It is important to note that the units in nearby regions do not actually move to "
	  << "the region where the fighting happens; the computer only checks that they could move there.  (In game "
	  << "world terms, presumably they did move there to join the fight, and then moved back where they started.)  "
	  << "The computer checks for weight allowances and terrain types when determining whether a unit could reach "
	  << "the scene of the battle. Note that the use of fleets is not allowed in this virtual movement.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If you order an attack on an ally (either with the " << url("#attack", "ATTACK")
	  << " order, or if your ally has declared you Unfriendly, by attempting to " << url("#advance", "ADVANCE")
	  << " into a region which he is guarding), then your commander will decide that a mistake has occurred "
	  << "somewhere, and withdraw your troops from the fighting altogether.  Thus, your units will not attack "
	  << "that faction in that region. Note that you will always defend an ally against attack, even if it means "
	  << "that you fight against other factions that you are allied with.\n"
	  << enclose("p", false);

	f << anchor("com_thebattle") << '\n';
	f << enclose("h3", true) << "The Battle:\n" << enclose("h3", false);
	f << enclose("p", true) << "The troops having lined up, the fight begins.";
	if (!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED)) {
		f << " The computer selects the best tactician from each side; that unit is regarded as the leader of "
		  << "its side.  If two or more units on one side have the same Tactics skill, then the one with the lower "
		  << "unit number is regarded as the leader of that side.  If one side's leader has a better Tactics skill "
		  << "than the other side's, then that side gets a "
		  << (Globals->ADVANCED_TACTICS
		      ? "tactics difference bonus to their attack and defense for the first round of combat."
			  : "free round of attacks.");
	}
	f << '\n' << enclose("p", false);
	f << enclose("p", true) << "In each combat round, the combatants each get to attack once, in "
	  << "a random order. "
	  << (!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED) && !Globals->ADVANCED_TACTICS
	      ? "(In a free round of attacks, only one side's forces get to attack.) "
		  : "")
	  << "Each combatant will attempt to hit a randomly selected enemy. If he hits, and the target has no armor, "
	  << "then the target is automatically killed.  Armor may provide extra defense against otherwise successful "
	  << "attacks.\n"
	  << enclose("p", false);

	f << enclose("p", true) << "The basic skill used in battle is the Combat skill; this is used for hand to "
	  << "hand fighting.  If one soldier tries to hit another using most weapons, there is a 50% chance that the "
	  << "attacker will get an opportunity for a lethal blow.  If the attacker does get that opportunity, then "
	  << "there is a contest between his combat skill (modified by weapon attack bonus) and the defender's combat "
	  << "skill (modified by weapon defense bonus). Some weapons may not allow combat skill to affect defense (e.g. "
	  << "bows), and others may allow different skills to be used on defense (or offense).\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If the skills are equal, then there is a 1:1 (i.e. 50%) chance that the attack "
	  << "will succeed.  If the attacker's skill is 1 higher then there is a 2:1 (i.e. 66%) chance, if the "
	  << "attacker's skill is 2 higher then there is a 4:1 (i.e. 80%) chance, 3 higher means an 8:1 (i.e. 88%) "
	  << "chance, and so on. Similarly if the defender's skill is 1 higher, then there is only a 1:2 (i.e. 33%) "
	  << "chance, etc.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "There are a variety of weapons in the world which can increase a soldier's skill "
	  << "on attack or defense.  Better weapons will generally convey better bonuses, but not all weapons are as "
	  << "good in all situations.  Specifics about the bonuses conferred by specific weapons can be found both in "
	  << "these rules (for most basic weapons), and in the descriptions of the weapons themselves. Troops which "
	  << "are fighting hand-to-hand without specific weapons are assumed to be irregularly armed with makeshift "
	  << "weapons such as clubs, pitchforks, torches, etc. \n"
	  << enclose("p", false);
	f << enclose("p", true) << " Possession of a mount, and the appropriate skill to use that mount will also "
	  << "confer a bonus to the effective Combat skill. The amount of the bonus will depend on the level of the "
	  << "appropriate skill and the mount in question.  Some mounts are better than others, and may provide better "
	  << "bonus, but may also require higher levels of skill to get any bonus at all.  Some terrain might not allow "
	  << "mounts to give a combat advantage.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Certain weapons may provide different attack and defense bonuses, or have "
	  << "additional attack bonuses against mounted opponents or other special characteristics. These bonuses will "
	  << "be listed in the item descriptions in the turn reports.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Some melee weapons may be defined as Long or Short (this is relative to a normal "
	  << "weapon, e.g. the sword). A soldier wielding a longer weapon than his opponent gets a +1 bonus to his "
	  << "attack skill.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Ranged weapons are slightly different from melee weapons.  The target will "
	  << "generally not get any sort of combat bonus to defense against a ranged attack.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Some weapons, including some ranged weapons, may only attack every other round, "
	  << "or even less frequently. When a weapon is not able to attack every round, this will be specified in the "
	  << "item description.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Weapons may have one of several different attack types: Slashing, Piercing, "
	  << "Crushing, Cleaving and Armor Piercing.  Different types of armor may give different survival chances "
	  << "against a successful attack of different types.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "A soldier attacking with a ranged weapon will generally be treated as if they "
	  << "have a Combat skill of 0, even if they have an actual Combat skill.  This is the trade off for being able "
	  << "to hit from the back line of fighting.\n" << enclose("p", false);

	f << enclose("p", true) << "Being inside a building confers a bonus to defense.  This bonus is effective "
	  << "against ranged as well as melee weapons.  The number of men that a building can protect is equal to "
	  << "its size. The size of the various common buildings was listed in the "
	  << url("#tablebuildings", "Table of Buildings") + " earlier. \n"
	  << enclose("p", false);

	f << enclose("p", true) << "If there are too many units in a building to all gain protection from it, then "
	  << "those units who have been in the building longest will gain protection.  (Note that these units appear "
	  << "first on the turn report.)";

	if (!(ObjectDefs[O_FORT].flags & ObjectType::DISABLED)) {
		f << " If a unit of 200 men is inside a Fort (capacity " << ObjectDefs[O_FORT].protect << "), then the first "
		  << ObjectDefs[O_FORT].protect << " men in the unit will gain the full bonus, and the other "
		  << (200 - ObjectDefs[O_FORT].protect) << " will gain no protection.";
	}
	f << '\n' << enclose("p", false);

	f << enclose("p", true) << "Units which have the Behind flag set are at the rear and cannot be attacked by "
	  << "any means until all non-Behind units have been wiped out.  On the other hand, neither can they attack "
	  << "with melee weapons, but only with ranged weapons or magic.  Once all front-line units have been wiped "
	  << "out, then the Behind flag no longer has any effect.\n" << enclose("p", false);
	f << anchor("com_victory") << '\n';
	f << enclose("h3", true) << "Victory!\n" << enclose("h3", false);
	f << enclose("p", true) << "Combat rounds continue until one side has accrued 50% losses (or more). The "
	  << "victorious side is then awarded one free round of attacks, after which the battle is over.  If both sides "
	  << "have more than 50% losses, the battle is a draw, and neither side gets a free round.\n"
	  << enclose("p", false);

	/* XXX -- Here is where to put the ROUT information */
	if (!(SkillDefs[S_HEALING].flags & SkillType::DISABLED) && !(ItemDefs[I_HERBS].flags & SkillType::DISABLED)) {
		f << enclose("p", true) << "Units with the Healing skill have a chance of being able to heal casualties "
		  << "of the winning side, so that they recover rather than dying.  Each character with this skill can "
		  << "attempt to heal " << Globals->HEALS_PER_MAN << " casualties per skill level. Each attempt however "
		  << "requires one unit of Herbs, which is thereby used up. Each attempt has a some chance of healing one "
		  << "casualty; only one attempt at Healing may be made per casualty. Healing occurs automatically, after "
		  << "the battle is over, by any living healers on the winning side.\n"
		  << enclose("p", false);
	}

	f << enclose("p", true) << "Any items owned by dead combatants on the losing side have a 50% chance of being "
	  << "found and collected by the winning side. Each item which is recovered is picked up by one of the "
	  << "survivors able to carry it (see the " << url("#spoils", "SPOILS") << " command) at random, so the "
	  << "winners generally collect loot in proportion to their number of surviving men.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If you are expecting to fight an enemy who is carrying so much equipment that "
	  << "you would not be able to move after picking it up, and you want to move to another region later that "
	  << "month, it may be worth issuing some orders to drop items (with the " << url("#give", "GIVE")
	  << " 0 order) or to prevent yourself picking up certain types of spoils (with the " << url("#spoils", "SPOILS")
	  << " order) in case you win the battle! Also, note that if the winning side took ";
	if (Globals->BATTLE_STOP_MOVE_PERCENT > 0) {
		f << "at least " << Globals->BATTLE_STOP_MOVE_PERCENT << "% casualties ";
	} else {
		f << "any casualties ";
	}
	f << "in the battle, any units on this side will not be allowed to move, or attack again for the "
	  << " rest of the turn.\n"
	  << enclose("p", false);
	if (has_stea || has_obse) {
		f << anchor("stealthobs") << '\n';
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << enclose("h2", true)
		  << (has_stea ? "Stealth" : "")
		  << (has_stea && has_obse ? " and " : "")
		  << (has_obse ? "Observation" : "")
		  << "\n" << enclose("h2", false);
		f << enclose("p", true);
		if (has_stea && has_obse) {
			f << "The Stealth skill is used to hide units, while the Observation skill is used to see units that "
			  << "would otherwise be hidden. A unit can be seen only if you have at least one unit in the same "
			  << "region, with an Observation skill at least as high as that unit's Stealth skill. If your "
			  << "Observation skill is equal to the unit's Stealth skill, you will see the unit, but not the name "
			  << "of the owning faction. If your Observation skill is higher than the unit's Stealth skill, you "
			  << "will also see the name of the faction that owns the unit.\n";
		} else if (has_stea) {
			f << "The Stealth skill is used to hide units. A unit can be seen only if it doesn't know the Stealth "
			  << "skill and if you have at least one unit in the same region.\n";
		} else if (has_obse) {
			f << "The Observation skill is used to see information about units that would otherwise be hidden. If "
			  << "your unit knows the Observation skill, it will see the name of the faction that owns any unit in "
			  << "the same region.\n";
		}
		f << enclose("p", false);
		if (has_stea) {
			f << enclose("p", true) << "Regardless of Stealth skill, units are always visible when participating in "
			  << "combat; when guarding a region with the Guard flag; or when in a building or aboard a fleet. ";
			if (has_obse) {
				f << "However, in order to see the faction that owns the unit, you will still need a higher "
				  << "Observation skill than the unit's Stealth skill.";
			}
			f << '\n' << enclose("p", false);
			f << anchor("stealthobs_stealing") << '\n';
			f << enclose("h3", true) << "Stealing:\n" << enclose("h3", false);
			f << enclose("p", true) << "The " << url("#steal", "STEAL") << " order is a way to steal items from "
			  << "other player factions without a battle. The order can only be issued by a one-man unit. The "
			  << "order specifies a target unit; the thief will then attempt to steal the specified item from "
			  << "the target unit.\n" << enclose("p", false);
			f << enclose("p", true);
			if (has_obse) {
				f << "If the thief has higher Stealth than any of the target faction's units have Observation "
				  << "(i.e. the thief cannot be seen by the target faction), the theft will succeed. "
				  << (Globals->HARDER_ASSASSINATION
				      ? "While stealing, the thief has a -1 penalty to his Stealth Skill. "
					  : "");
			} else {
				f << "The thief must know Stealth to attempt theft. ";
			}
			f << "The target faction will be told what was stolen, but not by whom.  If the specified item is silver, "
			  << "then $200 or half the total available, whichever is less, will be stolen.  If it is any other item, "
			  << "then only one will be stolen (if available).\n"
			  << enclose("p", false);
			if (has_obse) {
				f << enclose("p", true) << "Any unit with high enough Observation to see the thief will see the "
				  << "attempt to steal, whether the attempt is successful or not.  Allies of the target unit will "
				  << "prevent the theft, if they have high enough Observation to see the unit trying to steal.\n"
				  << enclose("p", false);
			}

			f << anchor("stealthobs_assassination") << '\n';
			f << enclose("h3", true) << "Assassination:\n" << enclose("h3", false);
			f << enclose("p", true) << "The " << url("#assassinate", "ASSASSINATE") << " order is a way to kill "
			  << "another person without attacking and going through an entire battle. This order can only be "
			  << "issued by a one-man unit, and specifies a target unit.  If the target unit contains more than one "
			  << "person, then one will be singled out at random.\n"
			  << enclose("p", false);
			if (has_obse) {
				f << enclose("p", true) << "Success for assassination is determined as for theft, i.e. the "
				  << "assassin will fail if any of the target faction's units can see him.  In this case, the "
				  << "assassin will flee, and the target faction will be informed which unit made the attempt.  As "
				  << "with theft, allies of the target unit will prevent the assassination from succeeding, if "
				  << "their Observation level is high enough.";
				if (Globals->HARDER_ASSASSINATION) {
					f << " While attemping an assassination, the assassin has a "
					  << (Globals->IMPROVED_AMTS ? "-1" : "-2")
					  << " penalty to his Stealth Skill.";
				}
				f << '\n' << enclose("p", false);
			} else {
				f << "The assassin must know Stealth to attempt assassination. ";
			}
			f << enclose("p", true);
			if (has_obse) {
				f << "If the assassin has higher stealth than any of the target faction's units have Observation, "
				  << "then a one-on-one ";
			} else {
				f << " A one-on-one ";
			}
			f << "fight will take place between the assassin and the target character.  The assassin automatically "
			  << "gets a free round of attacks";
			if (Globals->MAX_ASSASSIN_FREE_ATTACKS) {
				f << ", except he is limited to " << Globals->MAX_ASSASSIN_FREE_ATTACKS
				  << " total during the free round";
			}
			f << "; after that, the battle is handled like a normal fight, with the exception that neither assassin "
			  << "nor victim can use any armor";
			last = -1;
			comma = 0;
			for (int i = 0; i < NITEMS; i++) {
				if (!(ItemDefs[i].type & IT_ARMOR)) continue;
				if (!(ItemDefs[i].type & IT_NORMAL)) continue;
				if (ItemDefs[i].flags & ItemType::DISABLED) continue;
				ArmorType *at = FindArmor(ItemDefs[i].abr);
				if (at == NULL) continue;
				if (!(at->flags & ArmorType::USEINASSASSINATE)) continue;
				if (last == -1) {
					f << " except ";
					last = i;
					continue;
				}
				f << ItemDefs[last].name << ", ";
				last = i;
				comma++;
			}
			if (comma) f << "or ";
			if (last != -1)	f << ItemDefs[last].name;
			f << ".";
			f << (last == -1 ? " Armor " : " Most armor ") << "is forbidden for the assassin because it "
			  << "would make it too hard to sneak around, and for the victim because he was caught by surprise "
			  << "with his armor off. If the assassin wins, the target faction is told merely that the victim was "
			  << "assassinated, but not by whom.  If the victim wins, then the target faction learns which unit "
			  << "made the attempt.  (Of course, this does not necessarily mean that the assassin's faction is "
			  << "known.)  The winner of the fight gets 50% of the loser's property as usual.\n"
			  << enclose("p", false);
			f << enclose("p", true) << url("#steal", "STEAL") << " and " << url("#assassinate", "ASSASSINATE")
			  << " are not full month orders, and do not interfere with other activities, but a unit can only issue "
			  << "one " << url("#steal", "STEAL") << " order or one " << url("#assassinate", "ASSASSINATE")
			  << " order in a month.\n"
			  << enclose("p", false);
		}
	}

	f << anchor("magic") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Magic\n" << enclose("h2", false);
	f << enclose("p", 2) << "A character enters the world of magic in Atlantis by beginning study on one "
	  << "of the Foundation magic skills.  Only one man units"
	  << (!Globals->MAGE_NONLEADERS && Globals->LEADERS_EXIST ? ", with the man being a leader," : "")
	  << " are permitted to study these skills. ";
	if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
		f << "The number of these units (known as \"magicians\" or \"mages\") that a faction may own is "
		  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT
		      ? "limited."
			  : "determined by the faction's type.")
		  << " Any attempt to gain more, either through study, or by transfer from another faction, will fail.  "
		  << "In addition, mages ";
	} else {
		f << "Mages ";
	}
	f << "may not " << url("#give", "GIVE") << " men at all; once a unit becomes a mage (by studying one of the "
	  << "Foundations), the unit number is fixed. (The mage may be given to another faction using the "
	  << url("#give", "GIVE") << " UNIT order.)\n"
	  << enclose("p", false);
	f << anchor("magic_skills") << '\n';
	f << enclose("h3", true) << "Magic Skills:\n" << enclose("h3", false);
	f << enclose("p", true) << "Magic skills are the same as normal skills, with a few differences.  The basic "
	  << "magic skills, called Foundations, are ";
	last = -1;
	comma = 0;
	int j = 0;
	for (int i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].flags & SkillType::DISABLED) continue;
		if (!(SkillDefs[i].flags & SkillType::FOUNDATION)) continue;
		j++;
		if (last == -1) {
			last = i;
			continue;
		}
		f << SkillDefs[last].name << ", ";
		comma++;
		last = i;
	}
	f << (comma ? "and " : "") << SkillDefs[last].name << ". To become a mage, a unit undertakes study in one of "
	  << "these Foundations.  As a unit studies the Foundations, he will be able to study deeper into the magical "
	  << "arts; the additional skills that he may study will be indicated on your turn report.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "There are two major differences between Magic skills and most normal skills. "
	  << "The first is that the ability to study Magic skills sometimes depends on lower level Magic skills. "
	  << "Magic skills cannot be learnt to a higher level than the skills they depend upon. For example, if a "
	  << "Magic skill requires Spirit 2 to begin to study, then it can never be studied to a level higher than "
	  << "the mage's Spirit skill, so in order to increase that skill to level 3, his Spirit skill would first "
	  << "have to be increased to level 3. The Magic skills that a mage may study are listed on his turn report, "
	  << "so he knows which areas he may pursue. Studying higher in the Foundation skills, and certain other "
	  << "Magic skills, will make other skills available to the mage. Secondly, study into a magic skill above "
	  << "level 2 requires that the mage be located in some sort of building which can ";
	if (!Globals->LIMITED_MAGES_PER_BUILDING) {
		f << "offer protection.  Trade structures do not count. ";
	} else {
		f << "offer specific facilities for mages.  Certain types of buildings can offer shelter and support "
		  << "and a proper environment, some more so than others. ";
	}
	f << "If the mage is not in such a structure, his study rate is cut in half, as he does not have the proper "
	  << "environment and equipment for research.\n"
	  << enclose("p", false);

	if (Globals->LIMITED_MAGES_PER_BUILDING) {
		f << enclose("p", true) << "It is possible that there are advanced buildings not listed here which also "
		  << "can support mages.  The description of a building will tell you for certain.  The common buildings "
		  << "and the mages a building of that type can support follows:\n"
		  << enclose("p", false);
		f << anchor("tablemagebuildings") << '\n';
		f << enclose("center", true);
		f << enclose("table border=\"1\"", true);
		f << enclose("tr", true);
		f << enclose("td", true) << "\n" << enclose("td", false);
		f << enclose("th", true) << "Mages\n" << enclose("th", false);
		f << enclose("tr", false);
		for (int i = 0; i < NOBJECTS; i++) {
			if (ObjectDefs[i].flags & ObjectType::DISABLED) continue;
			if (!ObjectDefs[i].maxMages) continue;
			pS = FindSkill(ObjectDefs[i].skill);
			if (pS == NULL) continue;
			if (pS->flags & SkillType::MAGIC) continue;
			int k = ObjectDefs[i].item;
			if (k == -1) continue;
			/* Need the >0 since item could be WOOD_OR_STONE (-2) */
			if (k > 0 && (ItemDefs[k].flags & ItemType::DISABLED)) continue;
			if (k > 0 && !(ItemDefs[k].type & IT_NORMAL)) continue;
			/* Okay, this is a valid object to build! */
			f << enclose("tr", true);
			f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].name << '\n' << enclose("td", false);
			f << enclose("td align=\"left\" nowrap", true) << ObjectDefs[i].maxMages << '\n' << enclose("td", false);
			f << enclose("tr", false);
		}
		f << enclose("table", false);
		f << enclose("center", false);
	}

	f << anchor("magic_foundations") << '\n';
	f << enclose("h3", true) << "Foundations:\n" << enclose("h3", false);
	f << enclose("p", true) << "The " << num_to_word(j) << " Foundation skills are called ";
	last = -1;
	comma = 0;
	for (int i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].flags & SkillType::DISABLED) continue;
		if (!(SkillDefs[i].flags & SkillType::FOUNDATION)) continue;
		if (last == -1) {
			last = i;
			continue;
		}
		f << SkillDefs[last].name << ", ";
		comma++;
		last = i;
	}
	f << (comma ? "and " : "") << SkillDefs[last].name << ".";
	/* XXX -- This needs better handling! */
	/* Add each foundation here if it exists */
	if (!(SkillDefs[S_FORCE].flags & SkillType::DISABLED)) {
		f << " Force indicates the quantity of magical energy that a mage is able to channel (a Force rating of 0 "
		  << "does not mean that the mage can channel no magical energy at all, but only a minimal amount).";
	}
	if (!(SkillDefs[S_PATTERN].flags & SkillType::DISABLED)) {
		f << " Pattern indicates ability to handle complex patterns, and is important for things like healing and "
		  << "nature spells. ";
	}
	if (!(SkillDefs[S_SPIRIT].flags & SkillType::DISABLED)) {
		f << " Spirit deals with meta-effects that lie outside the scope of the physical world.";
	}
	f << '\n' << enclose("p", false);

	f << anchor("magic_furtherstudy") << '\n';
	f << enclose("h3", true) << "Further Magic Study:\n" << enclose("h3", false);
	f << enclose("p", true) << "Once a mage has begun study of one or more Foundations, more skills that he may "
	  << "study will begin to show up on his report. These skills are the skills that give a mage his power.  As "
	  << "with normal skills, when a mage achieves a new level of a magic skill, he will be given a skill report, "
	  << "describing the new powers (if any) that the new skill confers.  The " << url("#show", "SHOW")
	  << " order may be used to show this information on future reports.\n"
	  << enclose("p", false);

	f << anchor("magic_usingmagic") << '\n';
	f << enclose("h3", true) << "Using Magic:\n" << enclose("h3", false);
	f << enclose("p", true) << "A mage may use his magical power in three different ways, depending on the type "
	  << "of spell he wants to use.  Some spells, once learned, take effect automatically and are considered always "
	  << "to be in use; these spells do not require any order to take effect.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Secondly, some spells are for use in combat. A mage may specify that he wishes to "
	  << "use a spell in combat by issuing the " << url("#combat", "COMBAT") << " order.  A combat spell specified "
	  << "in this way will only be used if the mage finds himself taking part in a battle.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The third type of spell use is for spells that take an entire month to cast.  "
	  << "These spells are cast by the mage issuing the " << url("#cast", "CAST") << " order. Because "
	  << url("#cast", "CAST") << " takes an entire month, a mage may use only one of this type of spell each "
	  << "turn. Note, however, that a " << url("#cast", "CAST") << " order is not a full month order; a mage may "
	  << "still " << url("#move", "MOVE") << ", " << url("#study", "STUDY") << ", or use any other month long "
	  << "order. The justification for this (as well as being for game balance) is that a spell drains a mage of "
	  << "his magic power for the month, but does not actually take the entire month to cast.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The description that a mage receives when he first learns a spell specifies the "
	  << "manner in which the spell is used (automatic, in combat, or by casting).\n"
	  << enclose("p", false);

	f << anchor("magic_incombat") << '\n';
	f << enclose("h3", true) << "Magic in Combat:\n" << enclose("h3", false);
	f << enclose("p", true) << "NOTE: This section is rather vague, and quite advanced.  You may want to wait "
	  << "until you have figured out other parts of Atlantis before trying to understand exactly all of the rules "
	  << "in this section.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Although the magic skills and spells are unspecified in these rules, left for the "
	  << "players to discover, the rules for combat spells' interaction are spelled out here.  There are five major "
	  << "types of attacks, and defenses: Combat, Ranged, Energy, Weather, and Spirit.  Every attack and defense "
	  << "has a type, and only the appropriate defense is effective against an attack.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Defensive spells are cast at the beginning of each round of combat, and will have "
	  << "a type of attack they deflect, and skill level (Defensive spells are generally called Shields).  Every "
	  << "time an attack is launched against an army, it must first attack the highest level Shield of the same "
	  << "type as the attack, before it may attack a soldier directly. Note that an attack only has to attack the "
	  << "highest Shield, any other Shields of the same type are ignored for that attack.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "An attack spell (and any other type of attack) also has an attack type, and attack "
	  << "level, and a number of blows it deals. When the attack spell is cast, it is matched up against the most "
	  << "powerful defensive spell of the appropriate type that the other army has cast.  If the other army has not "
	  << "cast any applicable defensive spells, the attack goes through unmolested.  Unlike normal combat however, "
	  << "men are at a disadvantage to defending against spells.   Men which are in the open (not protected by "
	  << "a building) have an effective skill of -2 unless they have a shield or some other defensive magic.  "
	  << "Some monsters have bonuses to resisting some attacks but are more susceptible to others. The skill "
	  << "level of the attack spell and the effective skill for defense are matched against each other.  The "
	  << "formula for determining the victor between a defensive and offensive spell is the same as for a contest "
	  << "of soldiers; if the levels are equal, there is a 1:1 chance of success, and so on.  If the offensive "
	  << "spell is victorious, the offensive spell deals its blows to the defending army, and the Shield in question "
	  << "is destroyed (thus, it can be useful to have more than one of the same type of Shield in effect, as the "
	  << "other Shield will take the place of the destroyed one).  Otherwise, the attack spell disperses, and "
	  << "the defending spell remains in place.\n"
	  << enclose("p", false);
	  f << enclose("p", true) << "Some spells do not actually kill enemies, but rather have some negative effect "
	    << "on them. These spells are treated the same as normal spells; if there is a Shield of the same type as "
	    << "them, they must attack the Shield before attacking the army. Physical attacks that go through a "
	    << "defensive spell also must match their skill level against that of the defensive spell in question.  "
	    << "However, they do not destroy the defensive spell when they are successful.\n"
	    << enclose("p", false);

	if (app_exist) {
		string app_name(Globals->APPRENTICE_NAME);
		app_name.append("s");
		f << anchor("magic_"+app_name) << '\n';
		app_name[0] = toupper(app_name[0]);
		f << enclose("h3", true) << app_name << ":\n" << enclose("h3", false);
		f << enclose("p", true) << app_name << " may be created by having a unit study ";
		comma = 0;
		last = -1;
		for (int i = 0; i < NSKILLS; i++) {
			if (SkillDefs[i].flags & SkillType::DISABLED) continue;
			if (!(SkillDefs[i].flags & SkillType::APPRENTICE)) continue;
			if (last == -1) {
				last = i;
				continue;
			}
			f << SkillDefs[last].name << ", ";
			comma++;
			last = i;
		}
		f << (comma ? "or " : "") << SkillDefs[last].name << ". " << "Like Mages, only one man units"
		  << (!Globals->MAGE_NONLEADERS && Globals->LEADERS_EXIST ? ", with the man being a leader, " : " ")
		  << "may become " << app_name << ". " << app_name << " may not cast spells, but may use items "
		  << "which otherwise only mages can use.\n"
		  << enclose("p", false);
	}

	f << anchor("nonplayers") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Non-Player Units\n" << enclose("h2", false);
	f << enclose("p", true) << "There are a number of units that are not controlled by players that may be "
	  << "encountered in Atlantis.  Most information about these units must be discovered in the course of the "
	  << "game, but a few basics are below.\n"
	  << enclose("p", false);
	if (Globals->TOWNS_EXIST && Globals->CITY_MONSTERS_EXIST) {
		f << anchor("nonplayers_guards") << '\n';
		f << enclose("h3", true) << "City and Town Guardsmen:\n" << enclose("h3", false);
		f << enclose("p", true) << "All cities and towns begin with guardsmen in them.  These units will defend "
		  << "any units that are attacked in the city or town, and will also prevent theft and assassination "
		  << "attempts, "
		  << (has_obse ? "if their Observation level is high enough. " : "if they can see the criminal. ")
		  << "They are on guard, and will prevent other units from taxing or pillaging. "
		  << (Globals->START_CITIES_EXIST && Globals->SAFE_START_CITIES
		      ? "Except in the starting cities, the "
			  : "The ")
		  << "guards may be killed by players, although they will form again if the city is left unguarded.\n"
		  << enclose("p", false);
		if (Globals->START_CITIES_EXIST &&
		    (Globals->SAFE_START_CITIES || Globals->START_CITY_GUARDS_PLATE || Globals->START_CITY_MAGES)) {
			f << enclose("p", true);
			if (Globals->SAFE_START_CITIES || Globals->START_CITY_GUARDS_PLATE) {
				f << "Note that the city guardsmen in the starting cities of Atlantis possess "
				  << (Globals->SAFE_START_CITIES ? "Amulets of Invincibility " : "")
				  << (Globals->START_CITY_GUARDS_PLATE && Globals->SAFE_START_CITIES ? "and " : "")
				  << (Globals->START_CITY_GUARDS_PLATE ? "plate armor ": "")
				  << "in addition to being more numerous and "
				  << (Globals->SAFE_START_CITIES ? "may not be defeated." : "are therefore harder to kill.");
			}
			if (Globals->START_CITY_MAGES) {
				f << (Globals->AMT_START_CITY_GUARDS ? " Additionally, in " : "In ")
				  << "the starting cities, Mage Guards will be found. These mages are adept at the fire spell"
				  << (!Globals->SAFE_START_CITIES
				      ? " making any attempt to control a starting city a much harder proposition"
					  : "")
				  << ".";
			}
			f << '\n' << enclose("p", false);
		}
	}

	if (Globals->WANDERING_MONSTERS_EXIST) {
		f << anchor("nonplayers_monsters") << '\n';
		f << enclose("h3", true) << "Wandering Monsters:\n" << enclose("h3", false);
		f << enclose("p", true) << "There are a number of monsters who wander free throughout Atlantis.  They "
		  << "will occasionally attack player units, so be careful when wandering through the wilderness.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Some monsters live in lairs, caves, and other structures players cannot enter. "
		  << "Such monsters will never leave their habitat and will never wander around, but they can attack player "
		  << "units present in the region. The willingness to attack is dependent on the monster's aggression level. "
		  << "It is worth reminding that monsters inside the lair will always be visible to the player regardless "
		  << "of their stealth score as any other unit in the structure. Empty lairs will spawn new monsters "
		  << "regularly if old ones are killed. Players can guard regions with lairs, and monsters will not spawn "
		  << "there.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Other monsters do not live in lairs but wander freely. Wandering monsters can "
		  << "spawn in any unguarded region regardless of whether there is a lair. Guarding will prevent monsters "
		  << "from spawning in a particular region. Their willingness to attack depends on their aggression level, "
		  << "and monsters can advance to neighboring regions while moving. Some monsters could have preferred "
		  << "terrains that they like more than others, and then they will be willing to enter such regions more "
		  << "likely than others. At the same time, some terrains could be so uncomfortable that monsters will never "
		  << "enter them. If a monster has particular terrain preferences, he will try to be close to the habitat "
		  << "he likes, and he will not go deeper into the territory that is not connected with his preferred "
		  << "terrain.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "A small tip to the players about guarding: guarding will prevent monsters from "
		  << "spawning and attacking player units, but in such a way players will not get any loot from monsters too. "
		  << "Monster hunting is a desirable activity because it is fun, and you can get a great reward like silver, "
		  << "magical items, weapons, etc.\n"
		  << enclose("p", false);

		f << enclose("h4", true) << "Monster movement probability table\n" << enclose("h4", false);
		f << enclose("table border=\"1\"", true);
		f << enclose("thead", true);
		f << enclose("tr", true);
		f << enclose("th", true) << "Directions\n" << enclose("th", false);
		f << enclose("th", true) << "Prefered\n" << enclose("th", false);
		f << enclose("th", true) << "Neutral\n" << enclose("th", false);
		f << enclose("th", true) << "Move Preferred\n" << enclose("th", false);
		f << enclose("th", true) << "Move Neutral\n" << enclose("th", false);
		f << enclose("th", true) << "Stay\n" << enclose("th", false);
		f << enclose("tr", false);
		f << enclose("thead", false);

		int matrix[3][2];
		matrix[2][0] = 4;	// stay

		f << enclose("tbody", true);
		for (int dirs = 1; dirs <= 6; dirs++) {
			for (int preferedDirs = dirs; preferedDirs >= 0; preferedDirs--) {
				const int neutralDirs = dirs - preferedDirs;

				matrix[0][0] = preferedDirs * 2;	// prefered
				matrix[1][0] = neutralDirs;			// neutral

				int totalCases = 0;
				for (int i = 0; i < 3; i++) totalCases += matrix[i][0];

				for (int i = 0; i < 3; i++) matrix[i][1] = matrix[i][0] * 100 / totalCases;

				f << enclose("tr", true);
				f << enclose("td", true) << dirs << '\n' << enclose("td", false);
				f << enclose("td", true) << preferedDirs << '\n' << enclose("td", false);
				f << enclose("td", true) << neutralDirs << '\n' << enclose("td", false);
				for (int i = 0; i < 3; i++) f << enclose("td", true) << matrix[i][1] << "%\n" << enclose("td", false);
				f << enclose("tr", false);
			}
		}
		f << enclose("tbody", false);
		f << enclose("table", false);
	}
	f << anchor("nonplayers_controlled") << '\n';
	f << enclose("h3", true) << "Controlled Monsters:\n" << enclose("h3", false);
	f << enclose("p", true) << "Through various magical methods, you may gain control of certain types of "
	  << "monsters. These monsters are just another item in a unit's inventory, with a few special rules. Monsters "
	  << "will be able to carry things at their speed of movement; use the " << url("#show", "SHOW") << " ITEM "
	  << "order to determine the carrying capacity and movement speed of a monster. Monsters will also fight for "
	  << "the controlling unit in combat; their strength can only be determined in battle. Also, note that a "
	  << "monster will always fight from the front rank, even if the controlling unit has the behind flag set. "
	  << "Whether or not you are allowed to give a monster to other units depends on the type of monster; some may "
	  << "be given freely, while others must remain with the controlling unit.";
	if (Globals->RELEASE_MONSTERS) {
		f << " All monsters may be released completely by using the " << url("#give", "GIVE")
		  << " order targetting unit 0.  When this is done, the monster will become a wandering monster.";
	}
	f << '\n' << enclose("p", false);

	f << anchor("orders") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Orders\n" << enclose("h2", false);
	f << enclose("p", true) << "To enter orders for Atlantis, you should send a mail message to the Atlantis "
	  << "server, containing the following:\n"
	  << enclose("p", false);
	f << enclose("p", true) << "\n" << enclose("p", false);
	f << pre(true);
	f << "#ATLANTIS faction-no <password>\n\n"
	  << "UNIT unit-no\n"
	  << "...orders...\n\n"
	  << "UNIT unit-no\n"
	  << "...orders...\n\n"
	  << "#END\n"
	  << pre(false);
	f << enclose("p", true) << "For example, if your faction number (shown at the top of your report) is 27, "\
	  << "your password if \"foobar\", and you have two units numbered 5 and 17:\n" // TODO if->is after all is done
	  << enclose("p", false);
	f << enclose("p", true) << "\n" << enclose("p", false);
	f << pre(true);
	f << "#ATLANTIS 27 \"foobar\"\n\n"
	  << "UNIT 5\n"
	  << "...orders...\n\n"
	  << "UNIT 17\n"
	  << "...orders...\n\n"
	  << "#END\n"
	  << pre(false);
	f << enclose("p", true) << "Thus, orders for each unit are given separately, and indicated with the UNIT "
	  << "keyword.  (In the case of an order, such as the command to rename your faction, that is not really for "
	  << "any particular unit, it does not matter which unit issues the command; but some particular unit must "
	  << "still issue it.)\n"
	  << enclose("p", false);
	f << enclose("p", true) << "IMPORTANT: You MUST use the correct #ATLANTIS line or else your orders will be "
	  << "ignored.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If you have a password set, you must specify it on you #atlantis line, or the "
	  << "game will reject your orders.  See the " << url("#password", "PASSWORD") << " order for more details.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Each type of order is designated by giving a keyword as the first non-blank item "
	  << "on a line.  Parameters are given after this, separated by spaces or tabs. Blank lines are permitted, as "
	  << "are comments; anything after a semicolon is treated as a comment (provided the semicolon is not in the "
	  << "middle of a word).\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The parser is not case sensitive, so all commands may be given in upper case, "
	  << "lower case or a mixture of the two.  However, when supplying names containing spaces, the name must be "
	  << "surrounded by double quotes, or else underscore characters must be used in place of spaces in the name.  "
	  << "(These things apply to the #ATLANTIS and #END lines as well as to order lines.)\n"
	  << enclose("p", false);
	f << enclose("p", true) << "You may precede orders with the at sign (@), in which case they will appear in "
	  << "the Template at the bottom of your report.  This is useful for orders which your units repeat for several "
	  << "months in a row.\n"
	  << enclose("p", false);
	f << anchor("orders_abbreviations") << '\n';
	f << enclose("h3", true) << "Abbreviations:\n" << enclose("h3", false);
	f << enclose("p", true) << "All common items and skills have abbreviations that can be used when giving "
	  << "orders, for brevity.  Any time you see the item on your report, it will be followed by the abbreviation.  "
	  << "Please be careful using these, as they can easily be confused.\n"
	  << enclose("p", false);

	f << anchor("ordersummary") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Order Summary\n" << enclose("h2", false);
	f << enclose("p", true) << "To specify a [unit], use the unit number.  If specifying a unit that will "
	  << "be created this turn, use the form \"NEW #\" if the unit belongs to your faction, or \"FACTION # NEW #\" "
	  << "if the unit belongs to a different faction.  See the " << url("#form", "FORM") << " order for a more "
	  << "complete description.  [faction] means that a faction number is required; [object] means that an object "
	  << "number (generally the number of a building or fleet) is required. [item] means an item (like wood or "
	  << "longbow) that a unit can have in its possession. [flag] is an argument taken by several orders, that sets "
	  << "or unsets a flag for a unit. A [flag] value must be either 1 (set the flag) or 0 (unset the flag).  Other "
	  << "parameters are generally numbers or names.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "IMPORTANT: Remember that names containing spaces (e.g., \"Plate Armor\"), must "
	  << "be surrounded by double quotes, or the spaces must be replaced with underscores \"_\" (e.g., Plate_Armor).\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Also remember that anything used in an example is just that, an example and makes "
	  << "no guarantee that such an item, structure, or skill actually exists within the game.\n"
	  << enclose("p", false);

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("address") << '\n';
	f << enclose("h4", true) << "ADDRESS [new address]\n" << enclose("h4", false);
	f << enclose("p", true) << "Change the email address to which your reports are sent.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Change your faction's email address to atlantis@rahul.net.")
	  << "ADDRESS atlantis@rahul.net\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("advance") << '\n';
	f << enclose("h4", true) << "ADVANCE [dir] ...\n" << enclose("h4", false);
	f << enclose("p", true) << "This is the same as the " << url("#move", "MOVE") << " order, except that it "
	  << "implies attacks on units which attempt to forbid access.  See the " << url("#move", "MOVE") << " order "
	  << "for details.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Move north, then northwest, attacking any units that forbid access to the regions.")
	  << "ADVANCE N NW\n"
	  << example_end();
	f << example_start(
		  "In order, move north, then enter structure number 1, move through an inner route, and finally "
	      "move southeast. Will attack any units that forbid access to any of these locations."
	     )
	  << "ADVANCE N 1 IN SE\n"
	  << example_end();

	if (has_annihilate) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("annihilate") << '\n';
		f << enclose("h4", true) << "ANNIHILATE REGION [x] [y] [z]\n" << enclose("h4", false);
		f << enclose("p", true) << "Annihilate a region and the neighboring regions.  An annihilated region will be "
		  << "converted into barren land, and all units and all structures except for shafts and anomalies in the "
		  << "regions will be destroyed.  The neighboring regions will also be annihilated.  The region to be "
		  << "annihilated must be specified by coordinates.  If the Z coordinate is not specified, it is assumed "
		  << "to be on the same level as the unit issuing the order. This order may only be issued by a unit which "
		  << "has access to the ANNIHILATE [ANNI] skill. This skill cannot target or affect regions which are "
		  << "already barren, nor can it target the Nexus.\n";
		f << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Annihilate the region located at coordinates <5, 5> on the surface.")
		  << "ANNIHILATE REGION 5 5 1\n"
		  << example_end();
	}

	if (Globals->USE_WEAPON_ARMOR_COMMAND) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("armor") << '\n';
		f << enclose("h4", true) << "ARMOR [item1] [item2] [item3] [item4]\n" << enclose("h4", false);
		f << enclose("h4", true) << "ARMOR\n" << enclose("h4", false);
		f << enclose("p", true) << "This command allows you to set a list of preferred armor for a unit.  After "
		  << "searching for armor on the preferred list, the standard armor precedence takes effect if an armor "
		  << "hasn't been set.  The second form clears the preferred armor list.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Examples\n" << enclose("p", false);
		f << example_start("Set the unit to select chain armor before plate armor.")
		  << "ARMOR CARM PARM\n"
		  << example_end();
		f << example_start("Clear the preferred armor list.")
		  << "ARMOR\n"
		  << example_end();
	}

	if (has_stea) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("assassinate") << '\n';
		f << enclose("h4", true) << "ASSASSINATE [unit]\n" << enclose("h4", false);
		f << enclose("p", true) << "Attempt to assassinate the specified unit, or one of the unit's "
		  << "people if the unit contains more than one person.  The order may only be issued by a one-man unit.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "A unit may only attempt to assassinate a unit which is able to be seen.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Assassinate unit number 177.")
		  << "ASSASSINATE 177\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("attack") << '\n';
	f << enclose("h4", true) << "ATTACK [unit] ... \n" << enclose("h4", false);
	f << enclose("p", true) << "Attack a target unit.  If multiple ATTACK orders are given, all of the targets "
	  << "will be attacked.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("To attack units 17, 431, and 985:")
	  << "ATTACK 17\n"
	  << "ATTACK 431 985\n"
	  << example_end();
	f << example_start("or:")
	  << "ATTACK 17 431 985\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("autotax") << '\n';
	f << enclose("h4", true) << "AUTOTAX [flag]\n" << enclose("h4", false);
	f << enclose("p", true) << "AUTOTAX 1 causes the unit to attempt to tax every turn (without requiring the "
	  << "TAX order) until the flag is unset. AUTOTAX 0 unsets the flag.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("To cause the unit to attempt to tax every turn.")
	  << "AUTOTAX 1\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("avoid") << '\n';
	f << enclose("h4", true) << "AVOID [flag]\n" << enclose("h4", false);
	f << enclose("p", true) << "AVOID 1 instructs the unit to avoid combat wherever possible. The unit will "
	  << "not enter combat unless it issues an ATTACK order, or the unit's faction is attacked in the unit's "
	  << "hex. AVOID 0 cancels this.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The Guard and Avoid Combat flags are mutually exclusive; setting one "
	  << "automatically cancels the other.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Set the unit to avoid combat when possible.")
	  << "AVOID 1\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("behind") << '\n';
	f << enclose("h4", true) << "BEHIND [flag]\n" << enclose("h4", false);
	f << enclose("p", true) << "BEHIND 1 sets the unit to be behind other units in combat.  BEHIND 0 cancels this.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Set the unit to be in front in combat.")
	  << "BEHIND 0\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("build") << '\n';
	f << enclose("h4", true) << "BUILD\n" << enclose("h4", false);
	f << enclose("h4", true) << "BUILD [object type]\n" << enclose("h4", false);
	f << enclose("h4", true) << "BUILD HELP [unit]\n" << enclose("h4", false);
	f << enclose("p", true) << "BUILD given with no parameters causes the unit to perform work on "
	  << (may_sail ? "an unfinished ship it possesses, or on ": "") << "the object that it is currently inside.  "
	  << "BUILD given with an [object type] (such as \"Tower\" or \"Galleon\") instructs the unit to begin work "
	  << "on a new object of the type given. The final form instructs the unit to assist the target unit in its "
	  << "current building task, even if that task was begun this same turn. This help will be rejected if the "
	  << "unit you are trying to help does not consider you to be friendly.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("To build a new tower.")
	  << "BUILD Tower\n"
	  << example_end();
	f << example_start("To help unit 5789 build a structure.")
	  << "BUILD HELP 5789\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("buy") << '\n';
	f << enclose("h4", true) << "BUY [quantity] [item]\n" << enclose("h4", false);
	f << enclose("h4", true) << "BUY ALL [item]\n" << enclose("h4", false);
	f << enclose("p", true) << "Attempt to buy a number of the given item from a city or town "
	  << "marketplace, or to buy new people in any region where people are "
	  << "available for recruiting.  If the unit can't afford as many as "
	  << "[quantity], it will attempt to buy as many as it can. If the "
	  << "demand for the item (from all units in the region) is greater "
	  << "than the number available, the available items will be split "
	  << "among the buyers in proportion to the amount each buyer attempted "
	  << "to buy. ";
	if (Globals->RACES_EXIST) {
		f << "When buying people, specify the race of the people as the [item], or you may use PEASANT or PEASANTS "
		  << "to recruit whichever race is present in the region. ";
	}
	f << "If the second form is specified, the unit will attempt to buy as many as it can afford.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example" << (Globals->RACES_EXIST ? "s" : "") << ":\n" << enclose("p", false);
	f << example_start("Buy one plate armor from the city market.")
	  << "BUY 1 \"Plate Armor\"\n"
	  << example_end();
	if (Globals->RACES_EXIST) {
		f << example_start(
				"Recruit 5 barbarians into the current unit. (This will dilute the skills that the unit has.)"
			)
		  << "BUY 5 barbarians\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("cast") << '\n';
	f << enclose("h4", true) << "CAST [skill] [arguments]\n" << enclose("h4", false);
	f << enclose("p", true) << "Cast the given spell.  Note that most spell names contain spaces; be sure to "
	  << "enclose the name in quotes!  [arguments] depends on which spell you are casting; when you are able to cast "
	  << "a spell, the skill description will tell you the syntax.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Cast the spell called \"Super Spell\".")
	  << "CAST \"Super Spell\"\n"
	  << example_end();
	f << example_start("Cast the fourth-level spell in the \"Super Magic\" skill.")
	  << "CAST Super_Magic 4\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("claim") << '\n';
	f << enclose("h4", true) << "CLAIM [amount]\n" << enclose("h4", false);
	f << enclose("p", true) << "Claim an amount of the faction's unclaimed silver, and give it to the unit issuing "
	  << "the order.  The claiming unit may then spend the silver or give it to another unit.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Claim 100 silver.")
	  << "CLAIM 100\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("combat") << '\n';
	f << enclose("h4", true) << "COMBAT [spell]\n" << enclose("h4", false);
	f << enclose("p", true) << "Set the given spell as the spell that the unit will cast in combat.  This order "
	  << "may only be given if the unit can cast the spell in question.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Instruct the unit to use the spell \"Super Spell\", when the unit is involved in a battle.")
	  << "COMBAT \"Super Spell\"\n"
	  << example_end();

	if (Globals->FOOD_ITEMS_EXIST) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("consume") << '\n';
		f << enclose("h4", true) << "CONSUME UNIT\n" << enclose("h4", false);
		f << enclose("h4", true) << "CONSUME FACTION\n" << enclose("h4", false);
		f << enclose("h4", true) << "CONSUME\n" << enclose("h4", false);
		f << enclose("p", true) << "The CONSUME order instructs the unit to use food items in preference to silver "
		  << "for maintenance costs. CONSUME UNIT tells the unit to use food items that are in that unit's possession "
		  << "before using silver. CONSUME FACTION tells the unit to use any food items that the faction owns (in the "
		  << "same region as the unit) before using silver. CONSUME tells the unit to use silver before food items "
		  << "(this is the default).\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Tell a unit to use food items in the unit's possession for maintenance costs.")
		  << "CONSUME UNIT\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("declare") << '\n';
	f << enclose("h4", true) << "DECLARE [faction] [attitude]\n" << enclose("h4", false);
	f << enclose("h4", true) << "DECLARE [faction]\n" << enclose("h4", false);
	f << enclose("h4", true) << "DECLARE DEFAULT [attitude]\n" << enclose("h4", false);
	f << enclose("p", true) << "The first form of the DECLARE order sets the attitude of your faction towards the "
	  << "given faction.  The second form cancels any attitude towards the given faction (so your faction's attitude "
	  << "towards that faction will be its default attitude).  The third form sets your faction's default attitude.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Declare your faction to be hostile to faction 15.")
	  << "DECLARE 15 hostile\n"
	  << example_end();
	f << example_start("Set your faction's attitude to faction 15 to its default attitude.")
	  << "DECLARE 15\n"
	  << example_end();
	f << example_start("Set your faction's default attitude to friendly.")
	  << "DECLARE DEFAULT friendly\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("describe") << '\n';
	f << enclose("h4", true) << "DESCRIBE UNIT [new description]\n" << enclose("h4", false);
	f << enclose("h4", true) << "DESCRIBE SHIP [new description]\n" << enclose("h4", false);
	f << enclose("h4", true) << "DESCRIBE BUILDING [new description]\n" << enclose("h4", false);
	f << enclose("h4", true) << "DESCRIBE OBJECT [new description]\n" << enclose("h4", false);
	f << enclose("h4", true) << "DESCRIBE STRUCTURE [new description]\n" << enclose("h4", false);
	f << enclose("p", true) << "Change the description of the unit, or of the object the unit is in (of which the "
	  << "unit must be the owner). Descriptions can be of any length, up to the line length your mailer can handle. "
	  << "If no description is given, the description will be cleared out. The last four are completely identical "
	  << "and serve to modify the description of the object you are currently in.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	// TODO - clean this up.
	f << example_start("Set the unit,s description to read \"Merlin's helper\".")
	  << "DESCRIBE UNIT \"Merlin's helper\"\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("destroy") << '\n';
	f << enclose("h4", true) << "DESTROY\n" << enclose("h4", false);
	f << enclose("p", true) << "Destroy the object you are in (of which you must be the owner). The order cannot "
	  << "be used at sea.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Destroy the current object")
	  << "DESTROY\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("enter") << '\n';
	f << enclose("h4", true) << "ENTER [object]\n" << enclose("h4", false);
	f << enclose("p", true) << "Attempt to enter the specified object.  If issued from inside another object, the "
	  << "unit will first leave the object it is currently in.  The order will only work if the target object is "
	  << "unoccupied, or is owned by a unit in your faction, or is owned by a faction which has declared you "
	  << "Friendly.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Enter fleet number 114.")
	  << "ENTER 114\n"
	  << example_end();

	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("entertain") << '\n';
		f << enclose("h4", true) << "ENTERTAIN\n" << enclose("h4", false);
		f << enclose("p", true) << "Spend the month entertaining the populace to earn money.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Entertain for money.")
		  << "ENTERTAIN\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("evict") << '\n';
	f << enclose("h4", true) << "EVICT [unit] ...\n" << enclose("h4", false);
	f << enclose("p", true) << "Evict the specified unit from the object of which you are currently the owner.  If "
	  << "multiple EVICT orders are given, all of the units will be evicted.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Evict units 415 and 698 from an object that this unit owns.")
	  << "EVICT 415 698\n"
	  << example_end();
	f << example_start("or")
	  << "EVICT 415\n"
	  << "EVICT 698\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("exchange") << '\n';
	f << enclose("h4", true) << "EXCHANGE [unit] [quantity given] [item given] [quantity expected] [item expected]\n"
	  << enclose("h4", false);
	f << enclose("p", true) << "This order allows any two units that can see each other, to trade items regardless "
	  << "of faction stances.  The orders given by the two units must be complementary.  If either unit involved does "
	  << "not have the items it is offering, or if the exchange orders given are not complementary, the exchange is "
	  << "aborted.  Men may not be exchanged.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Exchange 10 LBOW for 10 SWOR with unit 1310")
	  << "EXCHANGE 1310 10 LBOW 10 SWOR\n"
	  << example_end();
	f << example_start("Unit 1310 would issue (assuming the other unit is 3453)")
	  << "EXCHANGE 3453 10 SWOR 10 LBOW\n"
	  << example_end();

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)
	{
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("faction") << '\n';

		f << enclose("h4", true) << "FACTION [type] [points] ...\n" << enclose("h4", false);
		f << enclose("p", true) << "Attempt to change your faction's type.  In the order, you can specify up to "
		  << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "three" : "two")
		  << " faction types ("
		  << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "WAR, TRADE," : "MARTIAL")
		  << " and MAGIC) and the number of faction points to assign to each type; if you are assigning points to "
		  << "only one " << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "or two types" : "type")
		  << ", you may omit the " << (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "types" : "type")
		  << " that will not have any points.\n"
		  << enclose("p", false);

		f << enclose("p", true) << "Changing the number of faction points assigned to MAGIC may be tricky. "
		  << "Increasing the MAGIC points will always succeed, but if you decrease the number of points assigned "
		  << "to MAGIC, you must make sure that you have only the number of magic-skilled leaders allowed by the "
		  << "new number of MAGIC points BEFORE you change your point distribution. For example, if you have 3 "
		  << "mages (3 points assigned to MAGIC), but want to use one of those points for something else (change to "
		  << "MAGIC 2), you must first get rid of one of your mages by either giving it to another faction or "
		  << "ordering it to " << url("#forget", "FORGET") << " all its magic skills. If you have too many mages "
		  << "for the number of points you try to assign to MAGIC, the FACTION order will fail.";
		if (Globals->APPRENTICES_EXIST || qm_exist) {
			f << " Factions may have the same requirements to disband excess units before changing faction point "
			  << "allocations based on how many ";
			if (Globals->APPRENTICES_EXIST) {
				f << Globals->APPRENTICE_NAME << "s";
			}
			if (qm_exist) {
				f << (Globals->APPRENTICES_EXIST ? " or " : "") << "quartermasters";
			}
			f << " are controlled by the faction and how they are restricted.";
		}
		f << '\n' << enclose("p", false);

		f << enclose("p", true) << "Examples:\n" << enclose("p", false);

		if (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT) {
			f << example_start("Assign 2 faction points to WAR, 2 to TRADE, and 1 to MAGIC.")
		  	  << "FACTION WAR 2 TRADE 2 MAGIC 1\n"
		  	  << example_end();
		} else {
			f << example_start("Assign 4 faction points to MARTIAL, and 1 to MAGIC.")
			  << "FACTION MARTIAL 4 MAGIC 1\n"
			  << example_end();
		}

		f << example_start("Become a pure magic faction (assign all points to magic).")
		  << "FACTION MAGIC " << Globals->FACTION_POINTS << '\n'
		  << example_end();
	}

	if (Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("find") << '\n';
		f << enclose("h4", true) << "FIND [faction]\n" << enclose("h4", false);
		f << enclose("h4", true) << "FIND ALL\n" << enclose("h4", false);
		f << enclose("p", true) << "Find the email address of the specified faction or of all factions.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Find the email address of faction 4.")
		  << "FIND 4\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("forget") << '\n';
	f << enclose("h4", true) << "FORGET [skill]\n" << enclose("h4", false);
	f << enclose("p", true) << "Forget the given skill. This order is useful for "
	  << (Globals->SKILL_LIMIT_NONLEADERS
	      ? "normal units who wish to learn a new skill, but already know a different skill. It can also be used for "
		  : "")
	  << "a mage"
	  << (Globals-> APPRENTICES_EXIST ? (qm_exist ? ", " : ", or ") + string(Globals->APPRENTICE_NAME) : "")
	  << (qm_exist ? ", or quartermaster" : "")
	  << " who wish to become a normal unit. A common reason for this is to be able to change faction points.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Forget knowledge of Mining.")
	  << "FORGET Mining\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("form") << '\n';
	f << enclose("h4", true) << "FORM [alias]\n" << enclose("h4", false);
	f << enclose("p", true) << "Form a new unit.  The newly created unit will be in your faction, in the same "
	  << "region as the unit which formed it, and in the same structure if any.  It will start off, however, with "
	  << "no people or items; you should, in the same month, issue orders to transfer people into the new unit, or "
	  << "have it recruit members. The new unit will inherit its flags from the unit that forms it, such as "
	  << "avoiding, behind, revealing and sharing, with the exception of the guard and autotax flags.  If the new "
	  << "unit wants to guard or automatically tax then those flags will have to be explicitly set in its orders.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The FORM order is followed by a list of orders for the newly created unit.  This "
	  << "list is terminated by the END keyword, after which orders for the original unit resume.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The purpose of the \"alias\" parameter is so that you can refer to the new unit. "
	  << "You will not know the new unit's number until you receive the next turn report.  To refer to the new unit "
	  << "in this set of orders, pick an alias number (the only restriction on this is that it must be at least 1, "
	  << "and you should not create two units in the same region in the same month, with the same alias numbers).  "
	  << "The new unit can then be referred to as NEW <alias> in place of the regular unit number.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "You can refer to newly created units belonging to other factions, if you know what "
	  << "alias number they are, e.g. FACTION 15 NEW 2 will refer to faction 15's newly created unit with alias 2.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Note: If a unit moves out of the region in which it was formed (by the "
	  << url("#move", "MOVE") << " order, or otherwise), the alias will no longer work. This is to prevent conflicts "
	  << "with other units that may have the same alias in other regions.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If the demand for recruits in that region that month is much higher than the supply, "
	  << "it may happen that the new unit does not gain all the recruits you ordered it to buy, or it may not gain "
	  << "any recruits at all.  If a new unit gains at least one recruit, the unit will form possessing any "
	  << "unused silver and all the other items it was given.  If no recruits are gained at all, the empty unit "
	  << "will be dissolved, and the silver and any other items it was given will revert to the first unit you have "
	  << "in that region.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start(
		  "This set of orders for unit 17 would create two new units with alias numbers 1 and 2, name them Merlin's "
		  "Guards and Merlin's Workers, set the description for Merlin's Workers, have both units recruit men, and "
		  "have Merlin's Guards study combat.  Merlin's Workers will have the default order "
		  + url("#work", "WORK") + ", as all newly created units do. The unit that created these two then pays them "
		  "enough money (using the NEW keyword to refer to them by alias numbers) to cover the costs of recruitment "
		  "and the month's maintenance."
	    )
	  << "UNIT 17\n"
	  << "FORM 1\n"
	  << "    NAME UNIT \"Merlin's Guards\"\n"
	  << (Globals->RACES_EXIST ? "    BUY 5 Plainsmen\n" : "    BUY 5 men\n")
	  << "    STUDY COMBAT\n"
	  << "END\n"
	  << "FORM 2\n"
	  << "    NAME UNIT \"Merlin's Workers\"\n"
	  << "    DESCRIBE UNIT \"wearing dirty overalls\"\n"
	  << (Globals->RACES_EXIST ? "    BUY 15 Plainsmen\n" : "    BUY 15 men\n")
	  << "END\n"
	  << "CLAIM 2500\n"
	  << "GIVE NEW 1 1000 silver\n"
	  << "GIVE NEW 2 2000 silver\n\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("give") << '\n';
	f << enclose("h4", true) << "GIVE [unit] [quantity] [item]\n" << enclose("h4", false);
	f << enclose("h4", true) << "GIVE [unit] ALL [item]\n" << enclose("h4", false);
	f << enclose("h4", true) << "GIVE [unit] ALL [item] EXCEPT [quantity]\n" << enclose("h4", false);
	f << enclose("h4", true) << "GIVE [unit] ALL [item class]\n" << enclose("h4", false);
	f << enclose("h4", true) << "GIVE [unit] UNIT\n" << enclose("h4", false);
	f << enclose("p", true) << "The first form of the GIVE order gives a quantity of an item to another unit. The "
	  << "second form of the GIVE order will give all of a given item to another unit.  The third form will give all "
	  << "of an item except for a specific quantity to another unit.  The fourth form will give all items of a "
	  << "specific type to another unit.  The final form of the GIVE order gives the entire unit to the specified "
	  << "unit's faction.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The classes of items which are acceptable for the fourth form of this order are, "
	  << "NORMAL, ADVANCED, TRADE, MAN or MEN, MONSTER or MONSTERS, MAGIC, WEAPON or WEAPONS, ARMOR, MOUNT or MOUNTS, "
	  << "BATTLE, SPECIAL, TOOL or TOOLS, FOOD, SHIP or SHIPS and ITEM or ITEMS (which is the combination of all of "
	  << "the previous categories).\n"
	  << enclose("p", false);
	f << enclose("p", true) << "A unit may only give items, including silver, to a unit which it is able to see, "
	  << "unless the faction of the target unit has declared you Friendly or better.  If the target unit is not a "
	  << "member of your faction, then its faction must have declared you Friendly, with a couple of exceptions. "
	  << "First, silver may be given to any unit, regardless of factional affiliation. Second, men may not be "
	  << "given to units in other factions (you must give the entire unit); the reason for this is to prevent highly "
	  << "skilled units from being sabotaged with a " << url("#give", "GIVE") << " order.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Unfinished ships are given like other items, although a unit may only have one "
	  << "unfinished ship of a given type at a time. To give unfinished ships, add the \"UNFINISHED\" keyword to the "
	  << "beginning of the [item] specifier.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Completed ships which are part of a fleet may be given too; the owner of the fleet "
	  << "they are currently in must issue the " << url("#give", "GIVE") << " order, and give the ships to the "
	  << "owner of the fleet that should receive the ships.  If the recipient is not the owner of a fleet, then a "
	  << "new fleet will be created owned by the recipient.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "There are also a few restrictions on orders given by units who been given to another "
	  << "faction. If the receiving faction is not allied to the giving faction, the unit may not issue the "
	  << url("#advance", "ADVANCE") << " order, or issue any more " << url("#give", "GIVE") << " orders.  Both of "
	  << "these rules are to prevent unfair sabotage tactics.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If 0 is specified as the unit number, then the items are discarded.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Give 10 swords to unit 4573.")
	  << "GIVE 4573 10 swords\n"
	  << example_end();
	f << example_start("Give 5 chain armor to the new unit, alias 2, belonging to faction 14.")
	  << "GIVE FACTION 14 NEW 2 5 \"Chain armor\"\n"
	  << example_end();
	f << example_start("Give control of this unit to the faction owning unit 75.")
	  << "GIVE 75 UNIT\n"
	  << example_end();
	f << example_start("Give our unfinished Longboat to unit 95.")
	  << "GIVE 95 1 UNFINISHED Longboat\n"
	  << example_end();
	f << example_start("Transfer 2 Longboats to the fleet commanded by unit 83.")
	  << "GIVE 83 2 Longboats\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("guard") << '\n';
	f << enclose("h4", true) << "GUARD [flag]\n" << enclose("h4", false);
	f << enclose("p", true) << "GUARD 1 sets the unit issuing the order to prevent non-Friendly units from collecting "
	  << "taxes in the region, and to prevent any units not your own from pillaging the region.  Guarding units will "
	  << "also attempt to prevent Unfriendly units from entering the region.  GUARD 0 cancels Guard status.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The Guard and Avoid Combat flags are mutually exclusive; setting one automatically "
	  << "cancels the other.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Instruct the current unit to be on guard.")
	  << "GUARD 1\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("hold") << '\n';
	f << enclose("h4", true) << "HOLD [flag]\n" << enclose("h4", false);
	f << enclose("p", true) << "HOLD 1 instructs the issuing unit to never join a battle in regions the unit is not "
	  << "in.  This can be useful if the unit is in a building, and doesn't want to leave the building to join combat. "
	  << "HOLD 0 cancels holding status.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Instruct the unit to avoid combat in other regions.")
	  << "HOLD 1\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("join") << '\n';
	f << enclose("h4", true) << "JOIN [unit]\n" << enclose("h4", false);
	f << enclose("h4", true) << "JOIN [unit] NOOVERLOAD\n" << enclose("h4", false);
	f << enclose("h4", true) << "JOIN [unit] MERGE\n" << enclose("h4", false);
	f << enclose("p", true) << "Attempt to enter the building or fleet that the specified unit is currently inside.  "
	  << "This is particularly useful if you don't know what the building or fleet number will be, as is the case "
	  << "when a new fleet is created.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If the target unit is not inside a building or fleet, then the unit issuing the "
	  << "JOIN command will leave any building or fleet that they happen to be inside, to be with the target.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If the NOOVERLOAD flag is specified, and the target unit ends up on board a fleet, "
	  << "then the unit issuing the JOIN command will only attempt to board the fleet if the fleet would be able to "
	  << "sail with the issuing units' weight loaded on board.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The MERGE flag may only be used by the owner of a fleet, and will cause the entire "
	  << "fleet they command to join the fleet owned by the specified unit - the units on board will move to the "
	  << "other fleet, and all the ships of the fleet will be given to the target fleet. This command will fail if "
	  << "any unit in the fleet to be merged would be denied entry to the target fleet.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Instruct the unit to enter the building or fleet that unit 17 is in.")
	  << "JOIN 17\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("leave") << '\n';
	f << enclose("h4", true) << "LEAVE\n" << enclose("h4", false);
	f << enclose("p", true) << "Leave the object you are currently in.";
	if (move_over_water) {
		f << " If a unit is capable of swimming "
		  << (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE ? "or flying " : "")
		  << "then this order is usable to leave a boat while at sea.";
	} else
		f << " The order cannot be used at sea.";
	f << '\n' << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Leave the current object")
	  << "LEAVE\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("move") << '\n';
	f << enclose("h4", true) << "MOVE [dir] ...\n" << enclose("h4", false);
	f << enclose("p", true) << "Attempt to move in the direction(s) specified.  If more than one direction is given, "
	  << "the unit will move multiple times, in the order specified by the MOVE order, until no more directions are "
	  << "given, or until one of the moves fails.  A move can fail because the unit runs out of movement points, "
	  << "because the unit attempts to move into the ocean, or because the unit attempts to enter a structure, and "
	  << "is rejected.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Valid directions are:\n"
	  << enclose("p", false);
	f << enclose("p", true) << "1) The compass directions North, Northwest, Southwest, South, Southeast, and "
	  << "Northeast.  These can be abbreviated N, NW, SW, S, SE, NE.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "2) A structure number.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "3) OUT, which will leave the structure that the unit is in.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "4) IN, which will move through an inner passage in the structure that the unit is "
	  << "currently in.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "5) PAUSE, which will instruct the unit to spend one movement point admiring the "
	  << "scenery, presumably to coordinate with slower moving companions.  This can be abbreviated P.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Multiple MOVE orders given by one unit will chain together.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Note that MOVE orders can lead to combat, due to hostile units meeting, or due to an "
	  << "advancing unit being forbidden access to a region.  Combat occurs after an antire movement phase has been "
	  << "completed for all regions.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example 1: Units 1 and 2 are in Region A, and unit 3 is in Region B.  Units 1 and 2 "
	  << "are hostile to unit 3.  Both units 1 and 2 move into region B, and attack unit 3.  Since combat happens "
	  << "after all movement has been done for the phase, they attack unit 3 at the same time, and the battle is "
	  << "between units 1 and 2, and unit 3.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example 2: Same as example 1, except unit 2 is in Region C, instead of region A.  "
	  << "Both units move into Region B, and attack unit 3.  Because combat happens after all movement has been done "
	  << "for the phase, they still attack unit 3 at the same time, and the battle is still between units 1 and 2, "
	  << "and unit 3.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Move N, NE, enter structure 1 and use the passage there")
	  << "MOVE N\n"
	  << "MOVE NE 1 IN\n"
	  << example_end();
	f << example_start("or:")
	  << "MOVE N NE 1 IN\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("name") << '\n';
	f << enclose("h4", true) << "NAME UNIT [new name]\n" << enclose("h4", false);
	f << enclose("h4", true) << "NAME FACTION [new name]\n" << enclose("h4", false);
	f << enclose("h4", true) << "NAME OBJECT [new name]\n" << enclose("h4", false);
	if (Globals->TOWNS_EXIST)
		f << enclose("h4", true) << "NAME CITY [new name]\n" << enclose("h4", false);
	f << enclose("p", true) << "Change the name of the unit, or of your faction, or of the object the unit is in "
	  << "(of which the unit must be the owner). Names can be of any length, up to the line length your mailer can "
	  << "handle.  Names may not contain parentheses (square brackets can be used instead if necessary), or any "
	  << "control characters.\n"
	  << enclose("p", false);
	if (Globals->TOWNS_EXIST) {
		f << enclose("p", true) << "In order to rename a settlement (city, town or village), the unit attempting to "
		  << "rename it must be the owner of a large enough structure located in the city. It requires a tower or "
		  << "better to rename a village, a fort or better to rename a town and a castle or a citadel to rename a "
		  << "city. ";
		if (Globals->CITY_RENAME_COST) {
			int c=Globals->CITY_RENAME_COST;
			f << "It also costs $" << c << " to rename a village, $" << 2*c << " to rename a town, and $"
			  << 3*c << " to rename a city.";
		}
		f << '\n' << enclose("p", false);
	}
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Name your faction \"The Merry Pranksters\".")
	  << "NAME FACTION \"The Merry Pranksters\"\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("noaid") << '\n';
	f << enclose("h4", true) << "NOAID [flag]\n" << enclose("h4", false);
	f << enclose("p", true) << "NOAID 1 indicates that if the unit attacks, or is attacked, it is not to be aided by "
	  << "units in other hexes. NOAID status is very useful for scouts or probing units, who do not wish to drag "
	  << "their nearby armies into battle if they are caught. NOAID 0 cancels this.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If multiple units are on one side in a battle, they must all have the NOAID flag on, "
	  << "or they will receive aid from other hexes.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Set a unit to receive no aid in battle.")
	  << "NOAID 1\n"
	  << example_end();

	if (move_over_water) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("nocross") << '\n';
		f << enclose("h4", true) << "NOCROSS [flag]\n" << enclose("h4", false);
		f << enclose("p", true) << "NOCROSS 1 indicates that if a unit attempts to cross a body of water then that "
		  << "unit should instead not cross it, regardless of whether the unit otherwise could do so. "
		  << (may_sail
		  	  ? "Units inside a fleet are not affected by this flag (IE, they are able to sail within the fleet). "
			  : "")
		  << "This flag is useful to prevent scouts from accidentally drowning when exploring in games where "
		  << "movement over water is allowed. NOCROSS 0 cancels this.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Set a unit to not permit itself to cross water.")
		  << "NOCROSS 1\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("option") << '\n';
	f << enclose("h4", true) << "OPTION TIMES\n" << enclose("h4", false);
	f << enclose("h4", true) << "OPTION NOTIMES\n" << enclose("h4", false);
	f << enclose("h4", true) << "OPTION SHOWATTITUDES\n" << enclose("h4", false);
	f << enclose("h4", true) << "OPTION DONTSHOWATTITUDES\n" << enclose("h4", false);
	f << enclose("h4", true) << "OPTION TEMPLATE OFF\n" << enclose("h4", false);
	f << enclose("h4", true) << "OPTION TEMPLATE SHORT\n" << enclose("h4", false);
	f << enclose("h4", true) << "OPTION TEMPLATE LONG\n" << enclose("h4", false);
	f << enclose("h4", true) << "OPTION TEMPLATE MAP\n" << enclose("h4", false);
	f << enclose("p", true) << "The OPTION order is used to toggle various settings that affect your reports, and "
	  << "other email details. OPTION TIMES sets it so that your faction receives the times each week (this is the "
	  << "default); OPTION NOTIMES sets it so that your faction is not sent the times.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "OPTION SHOWATTITUDES will cause units shown in your report to have a character "
	  << "placed before their name, which indicates your attitude towards them.  These characters are \"!\" for "
	  << "hostile, \"%\" for unfriendly, \"-\" for neutral, \":\" for friendly and \"=\" for allied. "
	  << "OPTION DONTSHOWATTITUDES turns off this additional decoration. \n"
	  << enclose("p", false);
	f << enclose("p", true) << "The OPTION TEMPLATE order toggles the length of the Orders Template that appears at "
	  << "the bottom of a turn report.  The OFF setting eliminates the Template altogether, and the SHORT, LONG and "
	  << "MAP settings control how much detail the Template contains. The MAP setting will produce an ascii map of "
	  << "the region and surrounding regions in addition other details.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "For the MAP template, the region identifiers are (there might be additional symbols "
	  << "for unusual/special terrain):\n"
	  << enclose("p", false);
	f << enclose("table", true);
	if (Globals->UNDERWORLD_LEVELS) {
		f << enclose("tr", true);
		f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "####\n" << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << "BLOCKED HEX (Underworld)\n" << enclose("td", false);
		f << enclose("tr", false);
	}
	f << enclose("tr", true);
	f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "~~~~\n" << enclose("td", false);
	f << enclose("td align=\"left\" nowrap", true) << "OCEAN HEX\n" << enclose("td", false);
	f << enclose("tr", false);
	f << enclose("tr", true);
	f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "    \n" << enclose("td", false);
	f << enclose("td align=\"left\" nowrap", true)
	  << "PLAINS" << (Globals->UNDERWORLD_LEVELS ? "/TUNNELS" : "") << " HEX\n" << enclose("td", false);
	f << enclose("tr", false);
	f << enclose("tr", true);
	f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "^^^^\n" << enclose("td", false);
	f << enclose("td align=\"left\" nowrap", true)
	  << "FOREST" << (Globals->UNDERWORLD_LEVELS ? "/UNDERFOREST" : "") << " HEX\n" << enclose("td", false);
	f << enclose("tr", false);
	f << enclose("tr", true);
	f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "/\\/\\\n" << enclose("td", false);
	f << enclose("td align=\"left\" nowrap", true) << "MOUNTAIN HEX\n" << enclose("td", false);
	f << enclose("tr", false);
	f << enclose("tr", true);
	f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "vvvv\n" << enclose("td", false);
	f << enclose("td align=\"left\" nowrap", true) << "SWAMP HEX\n" << enclose("td", false);
	f << enclose("tr", false);
	f << enclose("tr", true);
	f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "@@@@\n" << enclose("td", false);
	f << enclose("td align=\"left\" nowrap", true) << "JUNGLE HEX\n" << enclose("td", false);
	f << enclose("tr", false);
	f << enclose("tr", true);
	f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "....\n" << enclose("td", false);
	f << enclose("td align=\"left\" nowrap", true)
	  << "DESERT" << (Globals->UNDERWORLD_LEVELS ? "/CAVERN" : "") << " HEX\n" << enclose("td", false);
	f << enclose("tr", false);
	f << enclose("tr", true);
	f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << ",,,,\n" << enclose("td", false);
	f << enclose("td align=\"left\" nowrap", true) << "TUNDRA HEX\n" << enclose("td", false);
	f << enclose("tr", false);
	if (Globals->NEXUS_EXISTS) {
		f << enclose("tr", true);
		f << enclose("td align=\"left\" nowrap class=\"fixed\"", true) << "!!!!\n" << enclose("td", false);
		f << enclose("td align=\"left\" nowrap", true) << "THE NEXUS\n" << enclose("td", false);
		f << enclose("tr", false);
	}
	f << enclose("table", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Set your faction to receive the map format order template")
	  << "OPTION TEMPLATE MAP\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("password") << '\n';
	f << enclose("h4", true) << "PASSWORD [password]\n" << enclose("h4", false);
	f << enclose("h4", true) << "PASSWORD\n" << enclose("h4", false);
	f << enclose("p", true) << "The PASSWORD order is used to set your faction's password. If you have a password set, "
	  << "you must specify it on your #ATLANTIS line for the game to accept your orders.  This protects you orders "
	  << "from being overwritten, either by accident or intentionally by other players.  PASSWORD with no password "
	  << "given clears out your faction's password.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "IMPORTANT: The PASSWORD order does not take effect until the turn is actually run.  "
	  << "So if you set your password, and then want to re-submit orders, you should use the old password until the "
	  << "turn has been run.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Set the password to \"xyzzy\".")
	  << "PASSWORD xyzzy\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("pillage") << '\n';
	f << enclose("h4", true) << "PILLAGE\n" << enclose("h4", false);
	f << enclose("p", true) << "Use force to extort as much money as possible from the region. Note that the "
	  << url("#tax", "TAX") << " order and the PILLAGE order are mutually exclusive; a unit may only attempt "
	  << "to do one in a turn.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Pillage the current hex.")
	  << "PILLAGE\n"
	  << example_end();

	if (Globals->USE_PREPARE_COMMAND) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("prepare") << '\n';
		f << enclose("h4", true) << "PREPARE [item]\n" << enclose("h4", false);
		f << enclose("p", true) << "This command allows a mage"
		  << (app_exist ? " or " : "")
		  << (app_exist ? Globals->APPRENTICE_NAME : "")
		  << " to prepare a battle item (e.g. a Staff of Fire) for use in battle. "
		  << (Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_STRICT
		  	  ? "This selects the battle item which will be used, "
			  : "This allows the unit to override the usual selection of battle items, ")
		  << "and also cancels any spells set via the " << url("#combat", "COMBAT") << " order.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start(
			"Select a staff of fire as the "
			+ string(!(Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_STRICT) ? "preferred " : "")
			+ "battle item.")
		  << "PREPARE STAF\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("produce") << '\n';
	f << enclose("h4", true) << "PRODUCE [item]\n" << enclose("h4", false);
	f << enclose("h4", true) << "PRODUCE [number] [item]\n" << enclose("h4", false);
	f << enclose("p", true) << "Spend the month producing the specified item.  If a number is given then the unit "
	  << "will attempt to produce exactly that number of items; if this is not possible in one month then the order "
	  << "will carry over to subsequent months.  If no number is given then the unit will produce as much as possible "
	  << "of the specified item.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Produce as much wood as possible.")
	  << "PRODUCE wood\n"
	  << example_end();
	f << example_start("Produce exactly 3 crossbows.")
	  << "PRODUCE 3 crossbows\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("promote") << '\n';
	f << enclose("h4", true) << "PROMOTE [unit]\n" << enclose("h4", false);
	f << enclose("p", true) << "Promote the specified unit to owner of the object of which you are currently the "
	  << "owner.  The target unit must have declared you Friendly.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Promote unit 415 to be the owner of the object that this unit owns.")
	  << "PROMOTE 415\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("quit") << '\n';
	f << enclose("h4", true) << "QUIT [password]\n" << enclose("h4", false);
	f << enclose("p", true) << "Quit the game.  On issuing this order, your faction will be completely and "
	  << "permanently destroyed. Note that you must give your password for the quit order to work; this is to "
	  << "provide some safety against accidentally issuing this order.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Note that although this order affects the faction as a whole, it nevertheless needs "
	  << "to be issued by an individual unit, and so the email containing the command to quit needs to include both "
	  << "#atlantis and unit lines.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Quit the game for faction 27 if your password is foobar.")
	  << "#atlantis 27 \"foobar\"\n"
	  << "unit 1234\n"
	  << "QUIT \"foobar\"\n"
	  << "#end\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("restart") << '\n';
	f << enclose("h4", true) << "RESTART [password]\n" << enclose("h4", false);
	f << enclose("p", true) << "Similar to the " << url("#quit", "QUIT") << " order, this order will completely "
	  << "and permanently destroy your faction. However, it will begin a brand new faction for you (you will get a "
	  << "separate turn report for the new faction). Note that you must give your password for this order to work, "
	  << "to provide some protection against accidentally issuing this order.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Restart faction 27 as a new faction if your password is foobar.")
	  << "#atlantis 27 \"foobar\"\n"
	  << "unit 1234\n"
	  << "RESTART \"foobar\"\n"
	  << "#end\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("reveal") << '\n';
	f << enclose("h4", true) << "REVEAL\n" << enclose("h4", false);
	f << enclose("h4", true) << "REVEAL UNIT\n" << enclose("h4", false);
	f << enclose("h4", true) << "REVEAL FACTION\n" << enclose("h4", false);
	f << enclose("p", true) << "Cause the unit to either show itself (REVEAL UNIT), or show itself and its faction "
	  << "affiliation (REVEAL FACTION), in the turn report, to all other factions in the region. "
	  << (has_stea ? "Used to reveal high stealth scouts, should there be some reason to. " : "")
	  << "REVEAL is used to cancel this.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Show the unit to all factions.")
	  << "REVEAL UNIT\n"
	  << example_end();
	f << example_start("Show the unit and it's affiliation to all factions.")
	  << "REVEAL FACTION\n"
	  << example_end();
	f << example_start("Cancels revealing.")
	  << "REVEAL\n"
	  << example_end();

	if (has_sacrifice) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("sacrifice") << '\n';
		f << enclose("h4", true) << "SACRIFICE [quantity] [item]\n" << enclose("h4", false);
		f << enclose("p", true) << "Sacrifice the given quantity of the given item.   A sacrifice will only succeed "
		  << "if there is a structure in the same region as the unit which can accept the sacrifice.  Attempting to "
		  << "sacrifice when it's not valid will have no effect.\n";
		f << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Sacrifice 15 Jewelry.")
		  << "SACRIFICE 15 JEWE\n"
		  << example_end();
	}

	if (may_sail) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("sail") << '\n';
		f << enclose("h4", true) << "SAIL [dir] ...\n" << enclose("h4", false);
		f << enclose("h4", true) << "SAIL\n" << enclose("h4", false);
		f << enclose("p", true) << "The first form will sail the fleet, which the unit must be the owner of, in the "
		  << "directions given.  The second form will cause the unit to aid in the sailing of the fleet, using the "
		  << "Sailing skill.  See the section on movement for more information on the mechanics of sailing.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Sail north, then northwest.")
		  << "SAIL N NW\n"
		  << example_end();
		f << example_start("or:")
		  << "SAIL N\n"
		  << "SAIL NW\n"
		  << example_end();
	}

	if (Globals->TOWNS_EXIST) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("sell") << '\n';
		f << enclose("h4", true) << "SELL [quantity] [item]\n" << enclose("h4", false);
		f << enclose("h4", true) << "SELL ALL [item]\n" << enclose("h4", false);
		f << enclose("p", true) << "Attempt to sell the amount given of the item given.  If the unit does not have as "
		  << "many of the item as it is trying to sell, it will attempt to sell all that it has. The second form will "
		  << "attempt to sell all of that item, regardless of how many it has. If more of the item are on sale (by all "
		  << "the units in the region) than are wanted by the region, the number sold per unit will be split up in "
		  << "proportion to the number each unit tried to sell.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Sell 10 furs to the market.")
		  << "SELL 10 furs\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("share") << '\n';
	f << enclose("h4", true) << "SHARE [flag]\n" << enclose("h4", false);
	f << enclose("p", true) << "SHARE 1 instructs the unit to share its possessions with any other unit of your "
	  << "faction that needs them.  Thus a unit with a supply of silver could automatically provide silver if any of "
	  << "your other units in the same region does not have enough to perform an action, such as "
	  << url("#study", "studying") << ", " << url("#buy", "buying") << " or " << url("#produce", "producing")
	  << ".  SHARE 0 returns a unit to its default selfish state.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "This sharing does not extend to the heat of battle, only to economic actions.  So a "
	  << "unit that is sharing will provide silver for buying or studying, and resources for production (for example, "
	  << "if a sharing unit has wood in its inventory, and another unit is producing axes but has no wood, then the "
	  << "sharing unit will automatically supply wood for that production), but will not provide weapons to all units "
	  << "if combat occurs.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Note that in the case of sharing silver, this can leave the sharing unit without "
	  << "enough funds to pay maintenance, so sharing is to be used with care.  You may like to make sure that there "
	  << "is a unit with sufficient funds for maintenance in the same region, and which is not sharing, as those "
	  << "funds will be shared for maintenance, but not for less important purposes.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Instruct the unit to share its possessions with other units of the same faction.")
	  << "SHARE 1\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("show") << '\n';
	f << enclose("h4", true) << "SHOW SKILL [skill] [level]\n" << enclose("h4", false);
	f << enclose("h4", true) << "SHOW ITEM [item]\n" << enclose("h4", false);
	f << enclose("h4", true) << "SHOW OBJECT [object]\n" << enclose("h4", false);
	f << enclose("p", true) << "The first form of the order shows the skill description for a skill that your faction "
	  << "already possesses. The second form returns some information about an item that is not otherwise apparent on "
	  << "a report, such as the weight. The last form returns some information about an object (such as a ship or a "
	  << "building).\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Show the skill report for Mining 3 again.")
	  << "SHOW SKILL Mining 3\n"
	  << example_end();
	f << example_start("Show the item information for swords again.")
	  << "SHOW ITEM sword\n"
	  << example_end();
	f << example_start("Show the information for towers again.")
	  << "SHOW OBJECT tower\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("spoils") << '\n';
	f << enclose("h4", true) << "SPOILS [type]\n" << enclose("h4", false);
	f << enclose("h4", true) << "SPOILS\n" << enclose("h4", false);
	f << enclose("p", true) << "The SPOILS order determines which types of spoils the unit should take after a battle.  "
	  << "The valid values for type are 'NONE', 'WALK', 'RIDE', 'FLY', 'SWIM', 'SAIL' or 'ALL'. The second form is "
	  << "equivalent to 'SPOILS ALL'.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "When this command is issued, the unit is instructed to only pick up combat spoils if "
	  << "they could use the chosen form of movement while carrying the spoils. Thus a unit with SPOILS FLY selected "
	  << "would pick up combat spoils until they reached their flying capacity.  If the spoils provide movement "
	  << "capacity themselves, this will be included in the decision of whether or not to take the spoils - so a unit "
	  << "with SPOILS RIDE would always pick up horses. SPOILS SAIL will use the capacity of the fleet the unit is "
	  << "in to determine whether to take spoils or not. SPOILS ALL will allow a unit to collect any spoils which are "
	  << "dropped regardless of weight or capacity. SPOILS NONE will instruct the unit to only collect weightless "
	  << "items, such as silver.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Set a unit to only pick up items that it can carry and continue to fly:")
	  << "SPOILS FLY\n"
	  << example_end();

	if (has_stea) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("steal") << '\n';
		f << enclose("h4", true) << "STEAL [unit] [item]\n" << enclose("h4", false);
		f << enclose("p", true) << "Attempt to steal as much as possible of the specified item from "
		  << "the specified unit. The order may only be issued by a one-man unit.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "A unit may only attempt to steal from a unit which is able to be seen.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Examples:\n" << enclose("p", false);
		f << example_start("Steal silver from unit 123.")
		  << "STEAL 123 SILVER\n"
		  << example_end();
		f << example_start("Steal wood from unit 321.")
		  << "STEAL 321 wood\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("study") << '\n';
	f << enclose("h4", true) << "STUDY [skill]\n" << enclose("h4", false);
	f << enclose("h4", true) << "STUDY [skill] [level]\n" << enclose("h4", false);
	f << enclose("p", true) << "Spend the month studying the specified skill. A level may be specified which means "
	  << "that study will be continued from turn to turn until the unit  reaches that skill level. \n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Study horse training.")
	  << "STUDY \"Horse Training\"\n"
	  << example_end();
	f << example_start("Study combat to level 3.")
	  << "STUDY combat 3\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("take") << '\n';
	f << enclose("h4", true) << "TAKE FROM [unit] [quantity] [item]\n" << enclose("h4", false);
	f << enclose("h4", true) << "TAKE FROM [unit] ALL [item]\n" << enclose("h4", false);
	f << enclose("h4", true) << "TAKE FROM [unit] ALL [item] EXCEPT [quantity]\n" << enclose("h4", false);
	f << enclose("h4", true) << "TAKE FROM [unit] ALL [item class]\n" << enclose("h4", false);
	f << enclose("p", true) << "The TAKE order works just like the " << url("#give", "GIVE") << " order, except that "
	  << "the direction of transfer is reversed, and with the extra condition that a unit may only TAKE from another "
	  << "unit in the same faction. Since that makes TAKE FROM [unit] UNIT pointless, that form of the "
	  << url("#give", "GIVE") << " order is not supported.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The TAKE order is primarily intended to make automated delivery caravans less "
	  << "prone to generating errors, as they can use TAKE to collect the appropriate goods only when they are "
	  << "in the right place to collect, so the supplying unit doesn't need to keep trying to "
	  << url("#give", "GIVE") << " to a unit that is only there some of the time. However, it can be used "
	  << "anywhere you wish to transfer items, ships or men, just as " << url("#give", "GIVE") << " can.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start("Take 10 swords from unit 4573.")
	  << "TAKE FROM 4573 10 swords\n"
	  << example_end();
	f << enclose("p", true) << "See the " << url("#turn", "TURN")
	  << " order for an example of a caravan using TAKE FROM.\n"
	  << enclose("p", false);

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("tax") << '\n';
	f << enclose("h4", true) << "TAX\n" << enclose("h4", false);
	f << enclose("p", true) << "Attempt to collect taxes from the region. "
	  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES
		  ? (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT ? "Only War" : "Only Martial")
		  : "All")
	  << " factions may collect taxes, but only if there are no non-Friendly units on guard. Only combat-ready "
	  << "units may issue this order. Note that the TAX order and the " << url("#pillage", "PILLAGE")
	  << " order are mutually exclusive; a unit may only attempt to do one in a turn.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Attempt to collect taxes.")
	  << "TAX\n"
	  << example_end();

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("teach") << '\n';
	f << enclose("h4", true) << "TEACH [unit] ...\n" << enclose("h4", false);
	f << enclose("p", true) << "Attempt to teach the specified units whatever skill they are studying that month.  "
	  << "A list of several units may be specified. All units to be taught must have declared you Friendly. "
	  << "Subsequent TEACH orders can be used to add units to be taught.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Teach new unit 2 and unit 510 whatever they are studying.")
	  << "TEACH NEW 2 510\n"
	  << example_end();
	f << example_start("or:")
	  << "TEACH NEW 2\n"
	  << "TEACH 510\n"
	  << example_end();

	if (qm_exist) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("transport") << '\n';
		f << enclose("h4", true) << "TRANSPORT [unit] [num] [item]\n" << enclose("h4", false);
		f << enclose("h4", true) << "TRANSPORT [unit] ALL [item]\n" << enclose("h4", false);
		f << enclose("h4", true) << "TRANSPORT [unit] ALL [item] EXCEPT [amount]\n" << enclose("h4", false);
		f << enclose("p", true) << "Transport the specified items to the given target. In the second form all "
		  << "of the specified item is transported. In the last form, all of the specified item except for the "
		  << "specified amount is transported.";
		if (Globals->SHIPPING_COST > 0) {
			f << " Long distance transportation of goods between " << Globals->LOCAL_TRANSPORT << " and "
			  << Globals->NONLOCAL_TRANSPORT << " hexes away has an associated cost.  This cost is based "
			  << "on the weight of the items being transported.";
			if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST)
				f << " At higher skill levels of the quartermaster skill, the cost for transporting goods "
				  << "will be less.";
			if (Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST)
				f << " At higher skill levels of the quartermaster skill, the maximum distance goods can be "
				  << "transported increases over the above.";
		}
		// Note this is slightly wrong for the neworgins case.  cost only applies QM -> QM.
		f << " The target of the transport unit must be a unit with the quartermaster skill and must be the "
		  << "owner of a transport structure. For long distance transport between quartermasters, the issuing "
		  << "unit must also be a quartermaster and be the owner of a transport structure.  Use of this order "
		  << "counts as trade activity in the hex.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "For historical reasons, the order DISTRIBUTE can be "
		  << "used in place of TRANSPORT and has the same meaning and syntax.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Examples:\n" << enclose("p", false);
		f << example_start("Transport 10 STON to unit 1234")
		  << "TRANSPORT 1234 10 STON\n"
		  << example_end();
		f << example_start("Transport all except 10 SWOR to unit 3432")
		  << "TRANSPORT 3432 ALL SWOR EXCEPT 10\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("turn") << '\n';
	f << enclose("h4", true) << "TURN\n" << enclose("h4", false);
	f << enclose("p", true) << "The TURN order may be used to delay orders by one (or more) turns. By making "
	  << "the TURN order repeating (via '@'), orders inside the TURN/ENDTURN construct will repeat.  Multiple "
	  << "TURN orders in a row will execute on successive turns, and if they all repeat, they will form a loop "
	  << "of orders.  Each TURN section must be ended by an ENDTURN line.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Examples:\n" << enclose("p", false);
	f << example_start(
		   "Study combat this month, move north next month, and then in two months, pillage and advance north."
		 )
	  << "STUDY COMB\n"
	  << "TURN\n"
	  << "    MOVE N\n"
	  << "ENDTURN\n"
	  << "TURN\n"
	  << "    PILLAGE\n"
	  << "    ADVANCE N\n"
	  << "ENDTURN\n"
	  << example_end();
	f << example_start("After the turn, the orders for that unit would look as follows in the orders template:")
	  << "MOVE N\n"
	  << "TURN\n"
	  << "    PILLAGE\n"
	  << "    ADVANCE N\n"
	  << "ENDTURN\n"
	  << example_end();
	f << example_start("Set up a simple cash caravan.")
	  << "MOVE N\n"
	  << "@TURN\n"
	  << "    TAKE FROM 13794 1000 SILV\n"
	  << "    MOVE S S S\n"
	  << "ENDTURN\n"
	  << "@TURN\n"
	  << "    GIVE 13523 1000 SILV\n"
	  << "    MOVE N N N\n"
	  << "ENDTURN\n"
	  << example_end();
	f << example_start("After the turn, the orders for that unit would look as follows in the orders template:")
	  << "TAKE FROM 13794 1000 SILV\n"
	  << "MOVE S S S\n"
	  << "@TURN\n"
	  << "    GIVE 13523 1000 SILV\n"
	  << "    MOVE N N N\n"
	  << "ENDTURN\n"
	  << "@TURN\n"
	  << "    TAKE FROM 13794 1000 SILV\n"
	  << "    MOVE S S S\n"
	  << "ENDTURN\n"
	  << example_end();
	f << enclose("p", true) << "The orders in a TURN block will be inserted into the unit's orders template when "
	  << "there are no month-long orders remaining to be executed.  In particular, if the unit does not have enough "
	  << "movement points to cover the full distance of a MOVE or SAIL command, the movement commands will "
	  << "automatically be completed over multiple turns before executing the next TURN block.\n"
	  << enclose("p", false);

	if (Globals->USE_WEAPON_ARMOR_COMMAND) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("weapon") << '\n';
		f << enclose("h4", true) << "WEAPON [item] ...\n" << enclose("h4", false);
		f << enclose("h4", true) << "WEAPON\n" << enclose("h4", false);
		f << enclose("p", true) << "This command allows you to set a list of preferred weapons for a unit.  After "
		  << "searching for weapons on the preferred list, the standard weapon precedence takes effect if a weapon "
		  << "hasn't been set.  The second form clears the preferred weapon list.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Examples\n" << enclose("p", false);
		f << example_start("Set the unit to select double bows, then longbows then crossbows")
		  << "WEAPON DBOW LBOW XBOW\n"
		  << example_end();
		f << example_start("Clear the preferred weapon list.")
		  << "WEAPON\n"
		  << example_end();
	}

	if (Globals->ALLOW_WITHDRAW) {
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << anchor("withdraw") << '\n';
		f << enclose("h4", true) << "WITHDRAW [item]\n" << enclose("h4", false);
		f << enclose("h4", true) << "WITHDRAW [quantity] [item]\n" << enclose("h4", false);
		f << enclose("p", true) << "Use unclaimed funds to acquire basic items that you need. If you do not have "
		  << "sufficient unclaimed, or if you try withdraw any other than a basic item, an error will be given. "
		  << "Withdraw CANNOT be used in the Nexus (to prevent building towers and such there).  The first form "
		  << "is the same as WITHDRAW 1 [item] in the second form.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Examples:\n" << enclose("p", false);
		f << example_start("Withdraw 5 stone.")
		  << "WITHDRAW 5 stone\n"
		  << example_end();
		f << example_start("Withdraw 1 iron.")
		  << "WITHDRAW iron\n"
		  << example_end();
	}

	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << anchor("work") << '\n';
	f << enclose("h4", true) << "WORK\n" << enclose("h4", false);
	f << enclose("p", true) << "Spend the month performing manual work for wages.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Example:\n" << enclose("p", false);
	f << example_start("Work all month.")
	  << "WORK\n"
	  << example_end();

	f << anchor("sequenceofevents") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Sequence of Events\n" << enclose("h2", false);
	f << enclose("p", true) << "Each turn, the following sequence of events occurs:\n"
	  << enclose("p", false);
	f << enclose("OL", true);
	f << enclose("li", true) << "Instant orders.\n";
	f << enclose("ul", true);
	f << enclose("li", true) << url("#turn", "TURN") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#form", "FORM") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#address", "ADDRESS") << ", "
	  << (Globals->USE_WEAPON_ARMOR_COMMAND ? url("#armor", "ARMOR") + ", " : "")
	  << url("#autotax", "AUTOTAX") << ", " << url("#avoid", "AVOID") << ", " << url("#behind", "BEHIND")
	  << ", " << url("#claim", "CLAIM") << ", " << url("#combat", "COMBAT") << ", "
	  << (Globals->FOOD_ITEMS_EXIST ? url("#consume", "CONSUME") + ", " : "") << url("#declare", "DECLARE")
	  << ", " << url("#describe", "DESCRIBE") << ", "
	  << (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES ? url("#faction", "FACTION") + ", " : "")
	  << url("#guard", "GUARD") << " 0, " << url("#hold", "HOLD") << ", " << url("#name", "NAME") << ", "
	  << url("#noaid", "NOAID") << ", " << url("#share", "SHARE") << ", "
	  << (move_over_water ? url("#nocross", "NOCROSS") + ", " : "") << url("#option", "OPTION") << ", "
	  << url("#password", "PASSWORD") << ", "
	  << (Globals->USE_PREPARE_COMMAND ? url("#prepare", "PREPARE") + ", " : "") << url("#reveal", "REVEAL")
	  << ", " << url("#show", "SHOW") << ", " << (!Globals->USE_WEAPON_ARMOR_COMMAND ? "and " : "")
	  << url("#spoils", "SPOILS")
	  << (Globals->USE_WEAPON_ARMOR_COMMAND ? ", and " + url("#weapon", "WEAPON") : "")
	  << " orders are processed.\n"
	  << enclose("li", false);
	f << enclose("li", true) << url("#find", "FIND") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#leave", "LEAVE") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#enter", "ENTER") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#promote", "PROMOTE") << " and " << url("#evict", "EVICT")
	  << " orders are processed.\n" << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);
	f << enclose("li", true) << "Combat is processed.\n";
	f << enclose("ul", true);
	f << enclose("li", true) << url("#attack", "ATTACK") << " orders are processed.\n" << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);
	if (has_stea) {
		f << enclose("li", true) << "Subterfuge orders.\n";
		f << enclose("ul", true);
		f << enclose("li", true) << url("#steal", "STEAL") << " and " << url("#assassinate", "ASSASSINATE")
		  << " orders are processed.\n" << enclose("li", false);
		f << enclose("ul", false);
		f << enclose("li", false);
	}
	f << enclose("li", true) << "Give orders.\n";
	f << enclose("ul", true);
	f << enclose("li", true) << url("#give", "GIVE") << " and " << url("#take", "TAKE")
	  << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#join", "JOIN") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#exchange", "EXCHANGE") << " orders are processed.\n" << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);
	f << enclose("li", true) << "Tax orders.\n";
	f << enclose("ul", true);
	f << enclose("li", true) << url("#destroy", "DESTROY") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#pillage", "PILLAGE") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#tax", "TAX") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#guard", "GUARD") << " 1 orders are processed.\n" << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);
	f << enclose("li", true) << "Instant Magic\n";
	f << enclose("ul", true);
	f << enclose("li", true) << "Old spells are cancelled.\n" << enclose("li", false);
	f << enclose("li", true) << "Spells are " << url("#cast", "CAST") << " (except for Teleportation spells).\n"
	  << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);
	f << enclose("li", true) << "Market orders.\n";
	f << enclose("ul", true);
	if (Globals->TOWNS_EXIST) {
		f << enclose("li", true) << url("#sell", "SELL") << " orders are processed.\n" << enclose("li", false);
	}
	f << enclose("li", true) << url("#buy", "BUY") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#forget", "FORGET") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#quit", "QUIT") << " and " << url("#restart", "RESTART")
	  << " orders are processed.\n" << enclose("li", false);
	if (Globals->ALLOW_WITHDRAW) {
		f << enclose("li", true) << url("#withdraw", "WITHDRAW") << " orders are processed.\n"
		  << enclose("li", false);
	}
	f << enclose("ul", false);
	f << enclose("li", false);
	if (has_sacrifice) {
		f << enclose("li", true) << url("#sacrifice", "SACRIFICE") << " orders are processed.\n"
		  << enclose("li", false);
	}
	f << enclose("li", true) << "Movement orders.\n";
	f << enclose("ul", true);
	f << enclose("li", true) << url("#advance", "ADVANCE") << (may_sail ? ", " : " and ") << url("#move", "MOVE")
	  << (may_sail ? " and " + url("#sail", "SAIL") : "") << " orders are processed phase by phase "
	  << "(including any combat resulting from these orders).\n"
	  << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);
	f << enclose("li", true) << "Month long orders.\n";
	f << enclose("ul", true);
	f << enclose("li", true) << url("#teach", "TEACH") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#study", "STUDY") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << "Manufacturing " << url("#produce", "PRODUCE")
	  << " orders (those that produce items from other items, such as using the weaponsmith skill to make swords out "
	  << "of iron) are processed.\n" << enclose("li", false);
	f << enclose("li", true) << url("#build", "BUILD") << " orders are processed.\n" << enclose("li", false);
	f << enclose("li", true) << "Primary " << url("#produce", "PRODUCE")
	  << " orders (those that produce items from region resources, such as using the mining skill to produce iron) "
	  << "are processed.\n" << enclose("li", false);
	if (!(SkillDefs[S_ENTERTAINMENT].flags & SkillType::DISABLED)) {
		f << enclose("li", true) << url("#entertain", "ENTERTAIN") << " orders are processed.\n"
		  << enclose("li", false);
	}
	f << enclose("li", true) << url("#work", "WORK") << " orders are processed.\n" << enclose("li", false);
	f << enclose("ul", false);
	f << enclose("li", false);
	f << enclose("li", true) << "Teleportation spells are " << url("#cast", "CAST") << ".\n" << enclose("li", false);

	if (qm_exist) {
		f << enclose("li", true) << url("#transport", "TRANSPORT") << " orders are processed "
		  << "in multiple phases. In each phase all units in all hexes are processed before "
		  << "starting the next phase.  Items may move in each of the phases but only once in each phase.\n";
		f << enclose("ol", true);
		f << enclose("li", true) << "Items are sent from non-quartermaster units to "
		  << "quartermaster units.\n" << enclose("li", false);
		f << enclose("li", true) << "Items are sent from one quartermaster unit to "
		  << "another quartermaster units.\n" << enclose("li", false);
		f << enclose("li", true) << "Items are sent a quartermaster unit to "
		  << "non-quartermaster units.\n" << enclose("li", false);
		f << enclose("ol", false);
		f << enclose("li", false);
	}
	if (has_annihilate) {
		f << enclose("li", true) << url("#annihilate", "ANNIHILATE") << " orders are processed.\n"
		  << enclose("li", false);
	}
	f << enclose("li", true) << "Maintenance costs are assessed.\n" << enclose("li", false);
	f << enclose("OL", false);
	f << enclose("p", true) << "Where there is no other basis for deciding in which order units will be processed "
	  << "within a phase, units that appear higher on the report get precedence.\n"
	  << enclose("p", false);

	f << anchor("reportformat") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Report Format\n" << enclose("h2", false);
	f << enclose("p", true) << "The most important sections of the turn report are the \"Events During Turn\" "
	  << "section which lists what happened last month, and the \"Current Status\" section which gives the "
	  << "description of each region in which you have units.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Your units in the Current Status section are flagged with a \"*\" character. Units "
	  << "belonging to other factions are flagged with a \"-\" character. You may be informed which faction they "
	  << "belong to, if " << (has_obse ? "you have high enough Observation skill or " : "") << "they are revealing "
	  << "that information.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Objects are flagged with a \"+\" character.  The units listed under an object (if "
	  << "any) are inside the object.  The first unit listed under an object is its owner.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "If you can see a unit, you can see any large items it is carrying.  This means all "
	  << "items other than silver" << (!(ItemDefs[I_HERBS].flags & ItemType::DISABLED) ? ", herbs," : "")
	  << " and other small items (which are of zero size units, and are small enough to be easily concealed). Items "
	  << "carried by your own units of course will always be listed.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "At the bottom of your turn report is an Orders Template.  This template gives you "
	  << "a formatted orders form, with all of your units listed. You may use this to fill in your orders, or write "
	  << "them on your own. The " << url("#option", "OPTION") << " order gives you the option of giving more or "
	  << "less information in this template, or turning it off altogether. You can precede orders with an '@' sign "
	  << "in your orders, in which case they will appear in your template on the next turn's report.\n"
	  << enclose("p", false);
	f << anchor("hintsfornew") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Hints for New Players\n" << enclose("h2", false);
	f << enclose("p", true) << "Make sure to use the correct #ATLANTIS and UNIT lines in your orders.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Always have a month's supply of spare cash in every region in which you have "
	  << "units, so that even if they are deprived of income for a month (due to a mistake in your orders, for "
	  << "example), they will not starve to death.  It is very frustrating to have half your faction wiped out "
	  << "because you neglected to provide enough money for them to live on.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Be conservative with your money. ";
	if (Globals->LEADERS_EXIST) {
		f << "Leaders especially are very hard to maintain, as they cannot usually earn enough by "
		  << url("#work", "WORK") << "ing to pay their maintenance fee. ";
	}
	f << "Even once you have recruited men, notice that it is expensive for them to " << url("#study", "STUDY")
	  << " (and become productive units), so be sure to save money to that end.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Don't leave it until the last minute to send orders.  If there is a delay in the "
	  << "mailer, your orders will not arrive on time, and turns will NOT be rerun, nor will it be possible to "
	  << "change the data file for the benefit of players whose orders weren't there by the deadline.  If you are "
	  << "going to send your orders at the last minute, send a preliminary set earlier in the week so that at worst "
	  << "your faction will not be left with no orders at all.\n"
	  << enclose("p", false);

	if (Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		f << anchor("specialcommands") << '\n';
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << enclose("h2", true) << "Special Commands\n" << enclose("h2", false);
		f << enclose("p", true) << "These special commands have been added via the scripts processing the email "
		  << "to help you interact with the game and submit times and rumors. Please read over these new commands "
		  << "and their uses. Also note that all commands sent to the server are logged, including orders "
		  << "submissions, so if you have a problem, or if you attempt to abuse the system, it will get noticed and "
		  << "it will be tracked down.\n"
		  << enclose("p", false);
		f << anchor("_create") << '\n';
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << enclose("h4", true) << "#create \"faction name\" \"password\"\n" << enclose("h4", false);
		f << enclose("p", true) << "This will create a new faction with the desired name and password, and it will "
		  << "use the player's \"from\" address as the email address of record (this, of course, can be changed from "
		  << "within the game).\n"
		  << enclose("p", false);
		f << enclose("p", true) << "The \"\" characters are required. If they are missing, the server will not "
		  << "create the faction.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Join the game as the faction named \"Mighty Ducks\" with the password of \"quack\"")
		  << "#create \"Mighty Ducks\" \"quack\"\n"
		  << example_end();

		f << anchor("_resend") << '\n';
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << enclose("h4", true) << "#resend [faction] \"password\"\n" << enclose("h4", false);
		f << enclose("p", true) << "The faction number and your current password (if you have one) are required. "
		  << "The most recent turn report will be sent to the address of record.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start(
			  "You are faction 999 with password \"quack\" and need another copy of the last turn (because "
			  "your hard drive crashed)")
		  << "#resend 999 \"quack\"\n"
		  << example_end();

		f << anchor("_times") << '\n';
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << enclose("h4", true) << "#times [faction] \"password\"\n" << enclose("h4", false)
		  << "[body of article]\n"
		  << enclose("h4", true) << "#end\n" << enclose("h4", false);
		f << enclose("p", true) << "Everything between the #times and #end lines is included in your article. Your "
		  << "article will be marked as being sent by your faction, so you need not include that attribution in the "
		  << "article.";
		if (Globals->TIMES_REWARD)
			f << " You will receive $" << Globals->TIMES_REWARD << " for submitting the article.";
		f << '\n' << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Faction 999 wants to declare war on everyone")
		  << "#times 999 \"quack\"\n"
		  << "The Mighty Ducks declare war on the world!!\n"
		  << "Quack!\n"
		  << "#end\n"
		  << example_end();
		f << example_start("And it would appear something like:")
		  << "---------------------------------\n"
		  << "The Mighty Ducks declare war on the world!!\n"
		  << "Quack!\n\n"
		  << "[Article submitted by The Mighty Ducks (999)]\n"
		  << "---------------------------------\n"
		  << example_end();

		f << anchor("_rumor") << '\n';
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << enclose("h4", true) << "#rumor [faction] \"password\"\n" << enclose("h4", false)
		  << "[body of rumor]\n"
		  << enclose("h4", true) << "#end\n" << enclose("h4", false);
		f << enclose("p", true) << "Submit a rumor for publication in the next news.  These articles are not "
		  << "attributed (unlike times articles) and will appear in the rumor section of the next news in a random "
		  << "order.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Start a malicious rumor")
		  << "#rumor 999 \"quack\"\n"
		  << "Oleg is a running-dog lackey of Azthar Spleenmonger.\n"
		  << "#end\n"
		  << example_end();

		f << anchor("_remind") << '\n';
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << enclose("h4", true) << "#remind [faction] \"password\"\n" << enclose("h4", false);
		f << enclose("p", true) << "This order will have the server find the most recent set of orders you have "
		  << "submitted for the current turn and mail them back to your address of record.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Remind faction 999 of its last order set.")
		  << "#remind 999 \"quack\"\n"
		  << example_end();

		f << anchor("_email") << '\n';
		f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
		f << enclose("h4", true) << "#email [unit]\n" << enclose("h4", false)
		  << "[text of email]\n";
		f << enclose("p", true) << "This command allows you to send email to the owner of a unit even when you "
		  << "cannot see that unit's faction affiliation.  You will not be told who the unit belongs to, but will "
		  << "simply forward your email to them. When you use this command, they will receive YOUR email and can "
		  << "contact you if they choose. It is provided simply as a courtesy to players to help with diplomacy in "
		  << "first contact situations.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "There is no need for a \"#end\" line (such as is used in times and rumor "
		  << "submissions -- the entire email message you send will be forwarded to the unit's master.\n"
		  << enclose("p", false);
		f << enclose("p", true) << "Example:\n" << enclose("p", false);
		f << example_start("Send an email to the owner of unit 9999")
		  << "#email 9999\n"
		  << "Greetings.  You've entered the Kingdom of Foo.\n"
		  << "Please contact us.\n\n"
		  << "Lord Foo\n"
		  << "foo@some.email\n"
		  << example_end();
		f << example_start("Faction X, the owner of 9999 would receive:")
		  << "From: Foo &lt;foo@some.email&gt;\n"
		  << "Subject:  Greetings!\n\n"
		  << "#email 9999\n"
		  << "Greetings.  You've entered the Kingdom of Foo.\n"
		  << "Please contact us.\n\n"
		  << "Lord Foo\n"
		  << "foo@some.email\n"
		  << example_end();
	}

	// This needs updating post 4.0.5 (with whomever did 5.0 and now with Artem).. Also should url to the
	// github repo and NOT url to the yahoo group which is, I believe, dead.
	f << anchor("credits") << '\n';
	f << enclose(class_tag("div", "rule"), true) << '\n' << enclose("div", false);
	f << enclose("h2", true) << "Credits\n" << enclose("h2", false);
	f << enclose("p", true) << "Atlantis was originally created and programmed by Russell Wallace. Russell Wallace "
	  << "created Atlantis 1.0, and partially designed Atlantis 2.0 and Atlantis 3.0.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Geoff Dunbar designed and programmed Atlantis 2.0, 3.0, and 4.0 up through version "
	  << "4.0.4 and created the Atlantis Project to freely release and maintain the Atlantis source code.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Larry Stanbery created the Atlantis 4.0.4+ derivative.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "JT Traub took over the source code and merged the then forking versions of 4.0.4c "
	  << "and 4.0.4+ back into 4.0.5 along with modifications of his own and maintained the code until v5.0\n"
	  << enclose("p", false);
	f << enclose("p", true) << "After that, Stephen Baillie took over the code and maintained it until "
	  << "the v5.2.3-v5.2.5 time frame.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Artem Trytiak took over the code maintenance at that point and has maintained it "
	  << "since along with a number of other contrubutors to the github project.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "The Atlantis source code is now hosted on github at the "
	  << url("https://github.com/Atlantis-PBEM/Atlantis", "Atlantis") << " github project. "
	  << "Additionally, Artem runs the " << url("https://atlantis-pbem.com", "New Origins")
	  << " website which hosts the New Origins games and the related " << url("https://discord.gg/HusGETf", "Discord")
	  << "server, which serves as a de-facto development community.\n"
	  << enclose("p", false);
	f << enclose("p", true) << "Please see the CREDITS file in the source distribution for a more complete, "
	  << "but still inadequate, list of contributors.\n"
	  << enclose("p", false);

	f << enclose("body", false);
	f << enclose("html", false);
	return 1;
}

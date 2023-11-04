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

#include "gamedata.h"
#include "game.h"
#include "indenter.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace std;

char const *as[] = {
	"Hostile",
	"Unfriendly",
	"Neutral",
	"Friendly",
	"Ally"
};
char const **AttitudeStrs = as;

const std::string F_WAR = "War";
const std::string F_TRADE = "Trade";
const std::string F_MAGIC = "Magic";
const std::string F_MARTIAL = "Martial";

std::vector<std::string> ft { };
std::vector<std::string> *FactionTypes = &ft;

// LLS - fix up the template strings
char const *tp[] = {
	"off",
	"short",
	"long",
	"map"
};
char const **TemplateStrs = tp;

// Quit states
const string qs[] = {
	"none",
	"quit order",
	"quit by gm"
	"quit and restart",
	"won game"
	"game over",
};
const string *QuitStrs = qs;

int ParseTemplate(AString *token)
{
	for (int i = 0; i < NTEMPLATES; i++)
		if (*token == TemplateStrs[i]) return i;
	return -1;
}

int ParseAttitude(AString *token)
{
	for (int i=0; i<NATTITUDES; i++)
		if (*token == AttitudeStrs[i]) return i;
	return -1;
}

Faction::Faction()
{
	exists = 1;
	name = 0;
	
	for (auto &ft : *FactionTypes) {
		type[ft] = 1;
	}
	
	lastchange = -6;
	address = 0;
	password = 0;
	times = 0;
	showunitattitudes = 0;
	temformat = TEMPLATE_OFF;
	quit = 0;
	defaultattitude = A_NEUTRAL;
	unclaimed = 0;
	pReg = NULL;
	pStartLoc = NULL;
	noStartLeader = 0;
	startturn = 0;
	battleLogFormat = 0;
}

Faction::Faction(int n)
{
	exists = 1;
	num = n;
	
	for (auto &ft : *FactionTypes) {
		type[ft] = 1;
	}

	lastchange = -6;
	name = new AString;
	*name = AString("Faction (") + AString(num) + AString(")");
	address = new AString("NoAddress");
	password = new AString("none");
	times = 1;
	showunitattitudes = 0;
	temformat = TEMPLATE_LONG;
	defaultattitude = A_NEUTRAL;
	quit = 0;
	unclaimed = 0;
	pReg = NULL;
	pStartLoc = NULL;
	noStartLeader = 0;
	startturn = 0;
	battleLogFormat = 0;
}

Faction::~Faction()
{
	if (name) delete name;
	if (address) delete address;
	if (password) delete password;
	attitudes.clear();
}

void Faction::Writeout(ostream& f)
{
	f << num << '\n';

	// Right now, the faction type data in the file is what determines npc/non-npc with -1 in the types
	// signifying NPCs. We should eventually persist the is_npc flag directly, but for now, let's just
	// not break things. Having it based on the faction type data is pretty bogus, but it's what we
	// have for now so if this is an NPC faction, right out -1 for all types.
	for (auto &ft : *FactionTypes) {
		f << (is_npc ? -1 : type[ft]) << '\n';
	}

	f << lastchange << '\n';
	f << lastorders << '\n';
	f << unclaimed << '\n';
	f << *name << '\n';
	f << *address << '\n';
	f << *password << '\n';
	f << times << '\n';
	f << showunitattitudes << '\n';
	f << temformat << '\n';

	skills.Writeout(f);
	items.Writeout(f);
	f << defaultattitude << '\n';
	f << attitudes.size() << '\n';
	for (const auto& attitude: attitudes) f << attitude.factionnum << '\n' << attitude.attitude << '\n';
}

void Faction::Readin(istream& f)
{
	f >> num;

	for (auto &ft : *FactionTypes) {
		f >> type[ft];
	}
	// Right now, the faction type data in the file is what determines npc/non-npc.
	// We should eventually persist this flag directly, but for now, let's just not break things.
	// Having it based on the faction type data is pretty bogus, but it's what we have for now.
	// A faction is an NPC if it has -1 for either MARTIAL or WAR (NPCs will actually have -1 for all types)
	is_npc = ((type[F_WAR] == -1) || (type[F_MARTIAL] == -1));

	f >> lastchange;
	f >> lastorders;
	f >> unclaimed;

	AString tmp;
	f >> ws >> tmp;
	name = new AString(tmp);
	f >> ws >> tmp;
	address = new AString(tmp);
	f >> ws >> tmp;
	password = new AString(tmp);
	f >> times;
	f >> showunitattitudes;
	f >> temformat;

	skills.Readin(f);
	items.Readin(f);

	f >> defaultattitude;
	int n;
	f >> n;
	for (int i = 0; i < n; i++) {
		int fnum, fattitude;
		f >> fnum >> fattitude;
		if (fnum == num) continue;
		Attitude a = { .factionnum = fnum, .attitude = fattitude };
		attitudes.push_back(a);
	}
}

void Faction::View()
{
	AString temp;
	temp = AString("Faction ") + num + AString(" : ") + *name;
	Awrite(temp);
}

void Faction::SetName(AString* s)
{
	if (s) {
		AString* newname = s->getlegal();
		delete s;
		if (!newname) return;
		delete name;
		*newname += AString(" (") + num + ")";
		name = newname;
	}
}

void Faction::SetNameNoChange(AString *s)
{
	if (s) {
		delete name;
		name = new AString(*s);
	}
}

void Faction::SetAddress(AString &strNewAddress)
{
	delete address;
	address = new AString(strNewAddress);
}

AString Faction::FactionTypeStr()
{
	AString temp;
	if (is_npc) return AString("NPC");

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED) {
		return (AString("Unlimited"));
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		return(AString("Normal"));
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		bool comma = false;

		for (auto &ft : *FactionTypes) {
			auto value = type[ft];
			if (value) {
				if (comma) {
					temp += ", ";
				} else {
					comma = true;
				}
				temp += AString(ft) + " " + value;
			}
		}
		if (!comma) return AString("none");
	}
	return temp;
}

vector<FactionStatistic> Faction::compute_faction_statistics(Game *game, size_t **citems) {
	vector<FactionStatistic> stats;
	// To allow testing, just return an empty vector if we don't have any item arrays inbound
	if (!citems) return stats;

	auto myfaction = this->num - 1; // offset by 1 since the citems array are 0-based and factions are 1-based.

	for (auto i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].type & IT_SHIP) continue;
		if(citems[myfaction][i] == 0) continue;
		size_t place = 1;
		size_t max = 0;
		size_t total = 0;

		for (int pl = 0; pl < game->factionseq; pl++) {
			if (citems[pl][i] > citems[myfaction][i]) place++;
			if (max < citems[pl][i]) max = citems[pl][i];
			total += citems[pl][i];
		}

		string name = ItemString(i, citems[myfaction][i]).const_str();
		if (ItemDefs[i].type & IT_MONSTER && ItemDefs[i].type == IT_ILLUSION) {
			name += " (illusion)";
		}
		stats.push_back({ .item_name = name, .rank = place, .max = max, .total = total });
	}
	return stats;
}

inline bool Faction::gets_gm_report(Game *game) {
	return is_npc && num == 1 && (Globals->GM_REPORT || (game->month == 0 && game->year == 1));
}

struct GmData {
	vector<ShowSkill> skills;
	vector<string> items;
	vector<string> objects;
};

static inline GmData collect_gm_data() {
	GmData data;
	for (auto i = 0; i < NSKILLS; i++)
		for (auto j = 1; j < 6; j++)
			data.skills.push_back({ .skill = i, .level = j });

	for (auto i = 0; i < NITEMS; i++) {
		AString *show = ItemDescription(i, 1);
		if (show) {
			data.items.push_back(show->const_str());
			delete show;
		}
	}

	for (auto i = 0; i < NOBJECTS; i++) {
		AString *show = ObjectDescription(i);
		if (show) {
			data.objects.push_back(show->const_str());
			delete show;
		}
	}
	return data;
}

void Faction::write_text_gm_report(ostream& f, Game *game) {
	GmData data = collect_gm_data();

	bool need_header = true;
	for (auto &skillshow : data.skills) {
		if(need_header) { f << "Skill reports:\n"; need_header = false; }
		AString *string = skillshow.Report(this);
		if (string) {
			f << '\n' << string->const_str() << '\n';
			delete string;
		}
	}
	if(!need_header) f << '\n';

	need_header = true;
	for (const auto& itemshow : data.items) {
		if(need_header) { f << "Item reports:\n"; need_header = false; }
			f << '\n' << itemshow << '\n';
	}
	if (!need_header) f << '\n';

	need_header = true;
	for (const auto& objectshow : data.objects) {
		if(need_header) { f << "Object reports:\n"; need_header = false; }
		f << '\n' << objectshow << '\n';
	}
	if (!need_header) f << '\n';

	present_regions.clear();
	forlist(&(game->regions)) {
		ARegion *reg = (ARegion *)elem;
		present_regions.push_back(reg);
	}
	for (const auto& reg: present_regions) {
		reg->write_text_report(f, this, game->month, &(game->regions));
	}
	present_regions.clear();

	errors.clear();
	events.clear();
	battles.clear();
}

void Faction::write_json_gm_report(json& j, Game *game) {
	GmData data = collect_gm_data();

	json skills = json::array();
	for (auto &skillshow : data.skills) {
		AString *string = skillshow.Report(this);
		if (string) {
			skills.push_back(string->const_str());
			delete string;
		}
	}
	j["skill_reports"] = skills;

	// These two are easier since we already have a vector of strings, we can just auto-convert to a json array.
	j["item_reports"] = data.items;
	j["object_reports"] = data.objects;

	present_regions.clear();
	forlist(&(game->regions)) {
		ARegion *reg = (ARegion *)elem;
		present_regions.push_back(reg);
	}
	json regions = json::array();
	for (const auto& reg: present_regions) {
		json region;
		reg->write_json_report(region, this, game->month, &(game->regions));
		regions.push_back(region);
	}
	j["regions"] = regions;
	present_regions.clear();

	errors.clear();
	events.clear();
	battles.clear();
}

void Faction::write_json_report(json& j, Game *game, size_t **citems) {
	if (gets_gm_report(game)) {
		write_json_gm_report(j, game);
		return;
	}

	if (Globals->FACTION_STATISTICS) {
		j["statistics"] = compute_faction_statistics(game, citems);
	}

	// This can be better, but for now..
	j["engine"] = {
		{ "version", (ATL_VER_STRING(CURRENT_ATL_VER)).const_str() },
		{ "ruleset", Globals->RULESET_NAME },
		{ "ruleset_version", (ATL_VER_STRING(Globals->RULESET_VERSION)).const_str() }
	};

	string s = name->const_str();
	j["name"] = s.substr(0, s.find(" (")); // remove the faction number from the name for json output
	j["number"] = num;
	j["email"] = address->const_str();
	j["password"] = password->const_str();
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		j["type"] = json::object();
		for (auto &ft : *FactionTypes) {
			string factype = ft;
			transform(factype.begin(), factype.end(), factype.begin(), ::tolower);
			if (type[ft]) j["type"][factype] = type[ft];
		}
	}
	j["times_sent"] = (times != 0);
	j["password_unset"] = (!password || *password == "none");
	if(Globals->MAX_INACTIVE_TURNS) {
		int cturn = game->TurnNumber() - lastorders;
		if ((cturn >= (Globals->MAX_INACTIVE_TURNS - 3)) && !is_npc) {
			cturn = Globals->MAX_INACTIVE_TURNS - cturn;
			j["administrative"]["inactivity_deletion_turns"] = cturn;
		}
	}
	if(!exists) {
		j["administrative"]["quit"] = QuitStrs[quit ? quit : QUIT_BY_ORDER];
	}
	j["date"] = {
		{ "month", MonthNames[game->month] },
		{ "year", game->year }
	};

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		j["status"]["mages"] = {
			{ "current", nummages },
			{ "allowed", game->AllowedMages(this) }
		};

		if (Globals->APPRENTICES_EXIST) {
			j["status"]["apprentices"] = {
				{ "current", numapprentices },
				{ "allowed", game->AllowedApprentices(this) }
			};
		}
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if (Globals->FACTION_ACTIVITY != FactionActivityRules::DEFAULT) {
			int currentCost = GetActivityCost(FactionActivity::TAX);
			int maxAllowedCost = game->AllowedMartial(this);
			bool isMerged = Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED;
			j["status"][(isMerged ? "regions" : "activity")] = {
				{ "current", currentCost },
				{ "allowed", maxAllowedCost }
			};
		} else {
			j["status"]["tax_regions"] = {
				{ "current", GetActivityCost(FactionActivity::TAX) },
				{ "allowed", game->AllowedTaxes(this) }
			};
			j["status"]["trade_regions"] = {
				{ "current", GetActivityCost(FactionActivity::TRADE) },
				{ "allowed", game->AllowedTrades(this) }
			};
		}
		if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
			j["status"]["quartermasters"] = {
				{ "current", numqms },
				{ "allowed", game->AllowedQuarterMasters(this) }
			};
		}
		if (Globals->TACTICS_NEEDS_WAR) {
			j["status"]["tacticians"] = {
				{ "current", numtacts },
				{ "allowed", game->AllowedTacticians(this) }
			};
		}
		j["status"]["mages"] = {
			{ "current", nummages },
			{ "allowed", game->AllowedMages(this) }
		};
		if (Globals->APPRENTICES_EXIST) {
			j["status"]["apprentices"] = {
				{ "current", numapprentices },
				{ "allowed", game->AllowedApprentices(this) }
			};
		}
	}

	if (!errors.empty()) {
		// Handle errors better.  For now, just put them into an 'errors' vector.
		// We could give nice json'y output like which unit, which region, etc.
		j["errors"] = errors;
	}

	if (!battles.empty()) {
		json jbattles = json::array();
		for (const auto& battle: battles) {
			json jbattle = json::object();
			// we will obviously want to make this into json-y output
			battle->write_json_report(jbattle, this);
			jbattles.push_back(jbattle);
		}
		j["battles"] = jbattles;
	}

	if (!events.empty()) {
		// events can also be handled as json objects rather than just strings.
		j["events"] = events;
	}

	/* Attitudes */
	j["attitudes"] = json::object();
	string defattitude = AttitudeStrs[defaultattitude];
	transform(defattitude.begin(), defattitude.end(), defattitude.begin(), ::tolower);
	j["attitudes"]["default"] = defattitude;
	for (int i=0; i<NATTITUDES; i++) {
		string attitude = AttitudeStrs[i];
		// how annoying that this is the easiest way to do this.
		transform(attitude.begin(), attitude.end(), attitude.begin(), ::tolower);
		j["attitudes"][attitude] = json::array(); // [] = json::array();
		for (const auto& a: attitudes) {
			if (a.attitude == i) {
				// Grab that faction so we can get it's number and name, and strip the " (num)" from the name for json
				Faction *fac = GetFaction(&(game->factions), a.factionnum);
				string facname = fac->name->const_str();
				facname = facname.substr(0, facname.find(" ("));
				j["attitudes"][attitude].push_back({ { "name", facname }, { "number", a.factionnum } });
			}
		}
		// the array will be empty if this faction has declared no other factions with that specific attitude.
	}
	j["unclaimed_silver"] = unclaimed;

	json skills = json::array();
	for (auto &skillshow : shows) {
		AString *string = skillshow.Report(this);
		if (string) {
			skills.push_back(string->const_str());
			delete string;
		}
	}
	j["skill_reports"] = skills;

	// These two are easier since we already have a vector of strings, we can just auto-convert to a json array.
	j["item_reports"] = itemshows;
	j["object_reports"] = objectshows;

	// regions
	json regions = json::array();
	for (const auto& reg: present_regions) {
		json region;
		reg->write_json_report(region, this, game->month, &(game->regions));
		regions.push_back(region);
	}
	j["regions"] = regions;
}

void Faction::write_text_report(ostream& f, Game *pGame, size_t ** citems)
{
	// make the output automatically wrap at 70 characters
	f << indent::wrap;

	if(gets_gm_report(pGame)) {
		write_text_gm_report(f, pGame);
		return;
	}

	if (Globals->FACTION_STATISTICS) {
		f << ";Treasury:\n";
		f << ";\n";
		f << ";Item                                      Rank  Max        Total\n";
		f << ";=====================================================================\n";
		for (const auto& stat : compute_faction_statistics(pGame, citems)) {
			f << ';' << stat << '\n';
		}
		f << '\n';
	}

	f << "Atlantis Report For:\n";
	if ((Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) ||
			(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED)) {
		f << name->const_str() << '\n';
	} else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f << name->const_str() << " (" << FactionTypeStr().const_str() << ")\n";
	}
	f << MonthNames[pGame->month] << ", Year " << pGame->year << "\n\n";

	f << "Atlantis Engine Version: " <<	ATL_VER_STRING(CURRENT_ATL_VER) << '\n';
	f << Globals->RULESET_NAME << ", Version: " << ATL_VER_STRING(Globals->RULESET_VERSION) << "\n\n";

	if (!times) {
		f << "Note: The Times is not being sent to you.\n\n";
	}

	if (!password || (*password == "none")) {
		f << "REMINDER: You have not set a password for your faction!\n\n";
	}

	if (Globals->MAX_INACTIVE_TURNS != -1) {
		int cturn = pGame->TurnNumber() - lastorders;
		if ((cturn >= (Globals->MAX_INACTIVE_TURNS - 3)) && !is_npc) {
			cturn = Globals->MAX_INACTIVE_TURNS - cturn;
			f << "WARNING: You have " << cturn
			  << " turns until your faction is automatically removed due to inactivity!\n\n";
		}
	}

	if (!exists) {
		if (quit == QUIT_AND_RESTART) {
			f << "You restarted your faction this turn. This faction "
			  << "has been removed, and a new faction has been started "
			  << "for you. (Your new faction report will come in a "
			  << "separate message.)\n";
		} else if (quit == QUIT_GAME_OVER) {
			f << "I'm sorry, the game has ended. Better luck in "
			  << "the next game you play!\n";
		} else if (quit == QUIT_WON_GAME) {
			f << "Congratulations, you have won the game!\n";
		} else {
			f << "I'm sorry, your faction has been eliminated.\n"
			  << "If you wish to restart, please let the "
			  << "Gamemaster know, and you will be restarted for "
			  << "the next available turn.\n";
		}
		f << '\n';
	}

	f << "Faction Status:\n";
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		f << "Mages: " << nummages << " (" << pGame->AllowedMages(this) << ")\n";
		
		if (Globals->APPRENTICES_EXIST) {
			string name = Globals->APPRENTICE_NAME;
			name[0] = toupper(name[0]);
			f << name << "s: " << numapprentices << " (" << pGame->AllowedApprentices(this) << ")\n";
		}
	}
	else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if (Globals->FACTION_ACTIVITY != FactionActivityRules::DEFAULT) {
			int currentCost = GetActivityCost(FactionActivity::TAX);
			int maxAllowedCost = pGame->AllowedMartial(this);

			bool isMerged = Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED;
			f << (isMerged ? "Regions: " : "Activity: ") << currentCost << " (" << maxAllowedCost << ")\n";
		} else {			
			int taxRegions = GetActivityCost(FactionActivity::TAX);
			int tradeRegions = GetActivityCost(FactionActivity::TRADE);
			f << "Tax Regions: " << taxRegions << " (" << pGame->AllowedTaxes(this) << ")\n";
			f << "Trade Regions: " << tradeRegions << " (" << pGame->AllowedTrades(this) << ")\n";
		}

		if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
			f << "Quartermasters: " << numqms << " (" << pGame->AllowedQuarterMasters(this) << ")\n";
		}

		if (Globals->TACTICS_NEEDS_WAR) {
			f << "Tacticians: " << numtacts << " (" << pGame->AllowedTacticians(this) << ")\n";
		}

		f << "Mages: " << nummages << " (" << pGame->AllowedMages(this) << ")\n";

		if (Globals->APPRENTICES_EXIST) {
			string name = Globals->APPRENTICE_NAME;
			name[0] = toupper(name[0]);
			f << name << "s: " << numapprentices << " (" << pGame->AllowedApprentices(this) << ")\n";
		}
	}
	f << '\n';

	if (!errors.empty()) {
		f << "Errors during turn:\n";
		for (const auto& error : errors) f << error << '\n';
		f << '\n';
	}

	if (!battles.empty()) {
		f << "Battles during turn:\n";
		for (const auto& battle: battles) {
			battle->write_text_report(f, this);
		}
	}

	if (!events.empty()) {
		f << "Events during turn:\n";
		for (const auto& event: events) f << event << '\n';
		f << '\n';
	}

	if (!shows.empty()) {
		f << "Skill reports:\n";
		for (const auto& skillshow : shows) {
			AString *string = skillshow.Report(this);
			if (string) {
				f << '\n' << string->const_str() << '\n';
				delete string;
			}
		}
		f << '\n';
	}

	if (!itemshows.empty()) {
		f << "Item reports:\n";
		for (const auto& itemshow : itemshows) f << '\n' << itemshow << '\n';
		f << '\n';
	}

	if (!objectshows.empty()) {
		f << "Object reports:\n";
		for (const auto& objectshow : objectshows) f << '\n' << objectshow << '\n';
		f << '\n';
	}

	/* Attitudes */
	f << "Declared Attitudes (default " << AttitudeStrs[defaultattitude] << "):\n";
	for (int i = 0; i < NATTITUDES; i++) {
		int j = 0;
		f << AttitudeStrs[i] << " : ";
		for (const auto& attitude: attitudes) {
			if (attitude.attitude == i) {
				if (j) f << ", ";
				f << GetFaction(&(pGame->factions), attitude.factionnum)->name->const_str();
				j = 1;
			}
		}
		if (!j) f << "none";
		f << ".\n";
	}
	f << '\n';

	f << "Unclaimed silver: " << unclaimed << ".\n\n";

	for (const auto& reg: present_regions) {
		reg->write_text_report(f, this, pGame->month, &(pGame->regions));
	}
	f << '\n';
}

// LLS - write order template
void Faction::WriteTemplate(ostream& f, Game *pGame)
{
	AString temp;
	if (temformat == TEMPLATE_OFF) return;
	if (is_npc) return;

	f << indent::wrap;
	f << '\n';
	switch (temformat) {
		case TEMPLATE_SHORT:
			f << "Orders Template (Short Format):\n";
			break;
		case TEMPLATE_LONG:
			f << "Orders Template (Long Format):\n";
			break;
		case TEMPLATE_MAP:
			f << "Orders Template (Map Format):\n";
			break;
	}
	f << '\n';
	f << "#atlantis " << num;
	if (!(*password == "none")) {
		f << " \"" << *password << "\"";
	}
	f << '\n';

	for (const auto& reg: present_regions) {
		reg->WriteTemplate(f, this, &(pGame->regions), pGame->month);
	}

	f << "\n#end\n\n";
}

void Faction::WriteFacInfo(ostream &f)
{
	f << "Faction: " << num << '\n';
	f << "Name: " << name->const_str() << '\n';
	f << "Email: " << address->const_str() << '\n';
	f << "Password: " << password->const_str() << '\n';
	f << "LastOrders: " << lastorders << '\n';
	f << "FirstTurn: " << startturn << '\n';
	f << "SendTimes: " << times << '\n';
	f << "Template: " << TemplateStrs[temformat] << '\n';
	f << "Battle: na\n";
	for (const auto& s: extra_player_data) {
		f << s << '\n';
	}
	extra_player_data.clear();
}

void Faction::CheckExist(ARegionList* regs)
{
	if (is_npc) return;
	exists = 0;
	forlist(regs) {
		ARegion* reg = (ARegion *) elem;
		if (reg->Present(this)) {
			exists = 1;
			return;
		}
	}
}

void Faction::error(const string& s) {
	if (is_npc) return;
	auto count = errors.size();
	if (count == 1000) errors.push_back("Too many errors!");
	if (count < 1000) errors.push_back(s);
}

void Faction::event(const string& s)
{
	if (is_npc) return;
	events.push_back(s);
}

void Faction::remove_attitude(int f) {
	attitudes.erase(
		remove_if(attitudes.begin(), attitudes.end(), [f](const Attitude& a) { return a.factionnum == f; }),
		attitudes.end()
	);
}

int Faction::get_attitude(int n)
{
	if (n == num) return A_ALLY;
	for (const auto& attitude: attitudes) {
		if (attitude.factionnum == n) return attitude.attitude;
	}
	return defaultattitude;
}

void Faction::set_attitude(int faction_id, int attitude)
{
	auto place = find_if(
		attitudes.begin(), attitudes.end(), [faction_id](const Attitude& a) { return a.factionnum == faction_id; }
	);
	if (place != attitudes.end()) {
		place->attitude = attitude;
		return;
	}
	// we didn't find it.
	Attitude a = { .factionnum = faction_id, .attitude = attitude };
	attitudes.push_back(a);
}

int Faction::CanCatch(ARegion *r, Unit *t)
{
	if (TerrainDefs[r->type].similar_type == R_OCEAN) return 1;

	int def = t->GetDefenseRiding();

	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u == t && o->type != O_DUMMY) return 1;
			if (u->faction == this && u->GetAttackRiding() >= def) return 1;
		}
	}
	return 0;
}

int Faction::CanSee(ARegion* r, Unit* u, int practice)
{
	int detfac = 0;
	if (u->faction == this) return 2;
	if (u->reveal == REVEAL_FACTION) return 2;
	int retval = 0;
	if (u->reveal == REVEAL_UNIT) retval = 1;
	if (u->guard == GUARD_GUARD) retval = 1;
	forlist((&r->objects)) {
		Object* obj = (Object *) elem;
		int dummy = 0;
		if (obj->type == O_DUMMY) dummy = 1;
		forlist((&obj->units)) {
			Unit* temp = (Unit *) elem;
			if (u == temp && dummy == 0) retval = 1;

			// penalty of 2 to stealth if assassinating and 1 if stealing
			// TODO: not sure about the reasoning behind the IMPROVED_AMTS part
			int stealpenalty = 0;
			if (Globals->HARDER_ASSASSINATION && u->stealorders){
				if (u->stealorders->type == O_STEAL) {
					stealpenalty = 1;
				} else if (u->stealorders->type == O_ASSASSINATE) {
					if (Globals->IMPROVED_AMTS){
						stealpenalty = 1;
					} else {
						stealpenalty = 2;
					}
				}
			}

			if (temp->faction == this) {
				if (temp->GetAttribute("observation") >
						u->GetAttribute("stealth") - stealpenalty) {
					if (practice) {
						temp->PracticeAttribute("observation");
						retval = 2;
					}
					else
						return 2;
				} else {
					if (temp->GetAttribute("observation") ==
							u->GetAttribute("stealth") - stealpenalty) {
						if (practice) temp->PracticeAttribute("observation");
						if (retval < 1) retval = 1;
					}
				}
				if (temp->GetSkill(S_MIND_READING) > 1) detfac = 1;
			}
		}
	}
	if (retval == 1 && detfac) return 2;
	return retval;
}

void Faction::DefaultOrders()
{
	activity.clear();
	numshows = 0;
}

void Faction::TimesReward()
{
	if (Globals->TIMES_REWARD) {
		event("Times reward of " + to_string(Globals->TIMES_REWARD) + " silver.");
		unclaimed += Globals->TIMES_REWARD;
	}
}

Faction *GetFaction(AList *facs, int n)
{
	forlist(facs)
		if (((Faction *) elem)->num == n)
			return (Faction *) elem;
	return 0;
}

Faction *GetFaction2(AList *facs, int n)
{
	forlist(facs)
		if (((FactionPtr *) elem)->ptr->num == n)
			return ((FactionPtr *) elem)->ptr;
	return 0;
}

void Faction::DiscoverItem(int item, int force, int full)
{
	int seen, skill, i;
	AString skname;

	seen = items.GetNum(item);
	if (!seen) {
		if (full) {
			items.SetNum(item, 2);
		} else {
			items.SetNum(item, 1);
		}
		force = 1;
	} else {
		if (seen == 1) {
			if (full) {
				items.SetNum(item, 2);
			}
			force = 1;
		} else {
			full = 1;
		}
	}
	if (force) {
		AString *desc = ItemDescription(item, full);
		if (desc) {
			itemshows.push_back(desc->const_str());
			delete desc;
		}
		if (!full)
			return;
		// If we've found an item that grants a skill, give a
		// report on the skill granted (if we haven't seen it
		// before)
		skname = ItemDefs[item].grantSkill;
		skill = LookupSkill(&skname);
		if (skill != -1 && !(SkillDefs[skill].flags & SkillType::DISABLED)) {
			for (i = 1; i <= ItemDefs[item].maxGrant; i++) {
				if (i > skills.GetDays(skill)) {
					skills.SetDays(skill, i);
					shows.push_back({ .skill = skill, .level = i });
				}
			}
		}
	}
}

int Faction::GetActivityCost(FactionActivity type) {
	int count = 0;
	for (auto &kv : this->activity) {
		auto regionActivity = kv.second;

		if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL) {
			// do not care on particular activity type, but each activity consumes one point
			count += regionActivity.size();
		} else if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED) {
			// Activity array item can be present due to some logic like trying to do
			// activity unsuccessfully because of different reasons
			if (regionActivity.size() > 0) {
				count++;
			}
		} else {
			// standard logic, each activity is counted separately
			if (regionActivity.find(type) != regionActivity.end()) {
				count++;
			}
		}
	}

	return count;
}

void Faction::RecordActivity(ARegion *region, FactionActivity type) {
	this->activity[region].insert(type);
}

bool Faction::IsActivityRecorded(ARegion *region, FactionActivity type) {
	auto regionActivity = this->activity[region];

	if (Globals->FACTION_ACTIVITY == FactionActivityRules::MARTIAL_MERGED) {
		if (regionActivity.size() > 0) {
			return true;
		}
	}

	return regionActivity.find(type) != std::end(regionActivity);
}

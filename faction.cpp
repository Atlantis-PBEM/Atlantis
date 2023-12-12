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
	AString *temp = name->stripnumber();
	SetName(temp);
	
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

void Faction::build_gm_json_report(json& j, Game *game) {
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
		reg->build_json_report(region, this, game->month, &(game->regions));
		regions.push_back(region);
	}
	j["regions"] = regions;
	present_regions.clear();

	errors.clear();
	events.clear();
	battles.clear();
}

void Faction::build_json_report(json& j, Game *game, size_t **citems) {
	if (gets_gm_report(game)) {
		build_gm_json_report(j, game);
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
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		j["type"] = json::object();
		for (auto &ft : *FactionTypes) {
			string factype = ft;
			std::transform(factype.begin(), factype.end(), factype.begin(), ::tolower);
			if (type[ft]) j["type"][factype] = type[ft];
		}
	}
	j["administrative"]["times_sent"] = (times != 0);
	bool password_unset = (!password || *password == "none");
	j["administrative"]["password_unset"] = password_unset;
	j["administrative"]["email"] = address->const_str();
	j["administrative"]["show_unit_attitudes"] = (showunitattitudes != 0);

	if(!password_unset) j["administrative"]["password"] = password->const_str();
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

	if ((Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT)
		|| (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)) {
		j["status"]["mages"] = {
			{ "current", nummages },
			{ "allowed", game->AllowedMages(this) }
		};

		if (Globals->APPRENTICES_EXIST) {
			string name = Globals->APPRENTICE_NAME;
			name[0] = toupper(name[0]);
			j["status"]["apprentices"] = {
				{ "current", numapprentices },
				{ "allowed", game->AllowedApprentices(this) },
				{ "name", name }
			};
		}
	}
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
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
			battle->build_json_report(jbattle, this);
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
	std::transform(defattitude.begin(), defattitude.end(), defattitude.begin(), ::tolower);
	j["attitudes"]["default"] = defattitude;
	for (int i=0; i<NATTITUDES; i++) {
		string attitude = AttitudeStrs[i];
		// how annoying that this is the easiest way to do this.
		std::transform(attitude.begin(), attitude.end(), attitude.begin(), ::tolower);
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
		reg->build_json_report(region, this, game->month, &(game->regions));
		regions.push_back(region);
	}
	j["regions"] = regions;
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

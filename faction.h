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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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
// MODIFICATIONS
// Date				Person				 Comments
// ----				------				 --------
// 2000/MAR/14 Davis Kulis		Added a new reporting Template.
// 2001/Feb/18 Joseph Traub	 Added Apprentices from Lacandon Conquest
#ifndef FACTION_CLASS
#define FACTION_CLASS

class Faction;
class Game;
struct ShowObject;

#include "gameio.h"
#include "aregion.h"
#include "unit.h"
#include "battle.h"
#include "skills.h"
#include "items.h"
#include "alist.h"
#include "astring.h"

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <sstream>

enum
{
	A_HOSTILE,
	A_UNFRIENDLY,
	A_NEUTRAL,
	A_FRIENDLY,
	A_ALLY,
	NATTITUDES
};

extern const std::string F_WAR;
extern const std::string F_TRADE;
extern const std::string F_MAGIC;
extern const std::string F_MARTIAL;

// DK
// LLS - make templates cleaner for save/restore
enum
{
	TEMPLATE_OFF,
	TEMPLATE_SHORT,
	TEMPLATE_LONG,
	TEMPLATE_MAP,
	NTEMPLATES
};

enum
{
	QUIT_NONE,
	QUIT_BY_ORDER,
	QUIT_BY_GM,
	QUIT_AND_RESTART,
	QUIT_WON_GAME,
	QUIT_GAME_OVER,
};

extern char const **AttitudeStrs;
extern std::vector<std::string> *FactionTypes;

// LLS - include strings for the template enum
extern char const **TemplateStrs;
int ParseTemplate(AString *);

int ParseAttitude(AString *);

int MagesByFacType(int);

struct Attitude
{
	int factionnum;
	int attitude;
};

enum FactionActivity
{
	TAX = 1,
	TRADE = 2
};

class FactionPtr : public AListElem
{
public:
	Faction *ptr;
};

// Collect the faction statistics for display in the report
struct FactionStatistic {
	std::string item_name;
	size_t rank;
	size_t max;
	size_t total;

	// This needs to be a friend function so that the json library can call it.  This can only work for simple
	// objects/structs that require no additional parameters to be passed in.  On the other hand, for simple
	// structures this is nice, and works with things like STL containers of structs to make the entire container
	// serializable to json.
	friend void to_json(json &j, const FactionStatistic &s) {
		j = json{{"item_name", s.item_name}, {"rank", s.rank}, {"max", s.max}, {"total", s.total}};
	};
};

struct FactionError {
	std::string message;
	Unit *unit;

	friend void to_json(json &j, const FactionError &e);
};

struct FactionEvent {
	std::string message;
	std::string category;
	Unit *unit;
	ARegion *region;

	friend void to_json(json &j, const FactionEvent &e);
};

class Faction : public AListElem
{
public:
	Faction();
	Faction(int);
	~Faction();

	void Readin(std::istream &f);
	void Writeout(std::ostream &f);
	void View();

	void SetName(AString *);
	void SetNameNoChange(AString *str);
	void SetAddress(AString &strNewAddress);

	void CheckExist(ARegionList *);
	void error(const std::string& s, Unit *u = nullptr);
	void event(const std::string& message, const std::string& category, ARegion *r = nullptr, Unit *u = nullptr);

	void build_json_report(json& j, Game *pGame, size_t **citems);

	void WriteFacInfo(std::ostream &f);

	void set_attitude(int faction_id, int attitude); // attitude -1 clears it
	int get_attitude(int faction_id);
	void remove_attitude(int faction_id);

	int CanCatch(ARegion *, Unit *);
	/* Return 1 if can see, 2 if can see faction */
	int CanSee(ARegion *, Unit *, int practice = 0);

	void DefaultOrders();
	void TimesReward();

	bool is_npc = false; // by default factions are not NPCs

	void DiscoverItem(int item, int force, int full);

	int num;

	//
	// The type is only used if Globals->FACTION_LIMIT_TYPE ==
	// FACLIM_FACTION_TYPES
	//
	std::unordered_map<std::string, int> type;

	int lastchange;
	int lastorders;
	int unclaimed;
	int bankaccount;
	int interest; // not written to game.out
	AString *name;
	AString *address;
	AString *password;
	int times;
	int showunitattitudes;
	int temformat;
	int battleLogFormat;
	char exists;
	int quit;
	int numshows;

	int nummages;
	int numapprentices;
	int numqms;
	int numtacts;
	// AList war_regions;
	// AList trade_regions;

	std::unordered_map<ARegion *, std::unordered_set<FactionActivity, std::hash<int>>> activity;
	int GetActivityCost(FactionActivity type);
	void RecordActivity(ARegion *region, FactionActivity type);
	bool IsActivityRecorded(ARegion *region, FactionActivity type);

	bool gets_gm_report(Game *game);

	/* Used when writing reports */
	std::vector<ARegion *> present_regions;

	int defaultattitude;
	// TODO: Convert this to a hashmap of <attitude, vector<factionid>>
	// For now, just making it a vector of attitudes.  More will come later.
	std::vector<Attitude> attitudes;

	// These need to not be ALists wrapped with extra behavior at some point.
	SkillList skills;
	ItemList items;

	// Extra lines/data from the players.in file for this faction.  Stored and dumped, not used.
	std::vector<std::string> extra_player_data;

	// Errors and events during turn processing
	std::vector<FactionError> errors;
	std::vector<FactionEvent> events;

	std::vector<Battle *> battles;
	std::vector<ShowSkill> shows;
	std::vector<ShowItem> itemshows;
	std::vector<ShowObject> objectshows;

	// These are used for 'granting' units to a faction via the players.in
	// file
	ARegion *pReg;
	ARegion *pStartLoc;
	int noStartLeader;
	int startturn;

private:
	vector<FactionStatistic> compute_faction_statistics(Game *game, size_t **citems);
	void gm_report_setup(Game *game);
	void build_gm_json_report(json& j, Game *game);
};

Faction *GetFaction(AList *, int);
Faction *GetFaction2(AList *, int); /*This AList is a list of FactionPtr*/

#endif

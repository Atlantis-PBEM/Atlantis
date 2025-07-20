#pragma once
#ifndef FACTION_H
#define FACTION_H

class Faction;
class Game;
struct ShowObject;

#include "logger.hpp"
#include "aregion.h"
#include "unit.h"
#include "battle.h"
#include "skills.h"
#include "items.h"
#include "string_parser.hpp"

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <sstream>

#include "scoped_enum.hpp"

enum class AttitudeType
{
    HOSTILE,
    UNFRIENDLY,
    NEUTRAL,
    FRIENDLY,
    ALLY,
    ATTITUDE_COUNT
};

extern const std::string F_WAR;
extern const std::string F_TRADE;
extern const std::string F_MAGIC;
extern const std::string F_MARTIAL;

// DK
// LLS - make templates cleaner for save/restore
enum TemplateType
{
    TEMPLATE_OFF,
    TEMPLATE_SHORT,
    TEMPLATE_LONG,
    TEMPLATE_MAP,
    NTEMPLATES
};

enum QuitReason
{
    QUIT_NONE,
    QUIT_BY_ORDER,
    QUIT_BY_GM,
    QUIT_AND_RESTART,
    QUIT_WON_GAME,
    QUIT_GAME_OVER,
};

extern const std::vector<std::string> AttitudeStrs;
extern std::vector<std::string> *FactionTypes;

// LLS - include strings for the template enum
extern const std::vector<std::string> TemplateStrs;
int parse_template_type(const parser::token& str);

std::optional<AttitudeType> parse_attitude(const parser::token& str);

int MagesByFacType(int);

struct Attitude
{
    int factionnum;
    AttitudeType attitude;
};

enum FactionActivity
{
    TAX = 1,
    TRADE = 2
};

// Collect the faction statistics for display in the report
struct FactionStatistic {
    std::string name;
    std::string tag;
    std::string plural;
    size_t amount;
    size_t rank;
    size_t max;
    size_t total;
    bool illusion;

    // This needs to be a friend function so that the json library can call it.  This can only work for simple
    // objects/structs that require no additional parameters to be passed in.  On the other hand, for simple
    // structures this is nice, and works with things like STL containers of structs to make the entire container
    // serializable to json.
    friend void to_json(json &j, const FactionStatistic &s) {
        j = json{
            {"amount", s.amount}, {"name", s.name}, {"plural", s.plural}, {"tag", s.tag},
            {"rank", s.rank}, {"max", s.max}, {"total", s.total}
        };
        if (s.illusion) j["illusion"] = true;
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

class Faction
{
public:
    Faction();
    Faction(int);
    ~Faction();

    void Readin(std::istream &f);
    void Writeout(std::ostream &f);
    void View();

    void set_name(const std::string& newname, bool canonicalize = true);
    void set_address(const std::string& strNewAddress);

    void CheckExist(ARegionList& regs);
    void error(const std::string& s, Unit *u = nullptr);
    void event(const std::string& message, const std::string& category, ARegion *r = nullptr, Unit *u = nullptr);

    void build_json_report(json& j, Game *pGame, size_t **citems);

    void WriteFacInfo(std::ostream &f);

    void set_attitude(int faction_id, AttitudeType attitude); // attitude -1 clears it
    AttitudeType get_attitude(int faction_id);
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
    std::string name;
    std::string address;
    std::string password;
    int times;
    int showunitattitudes;
    int temformat;
    int battleLogFormat;
    bool exists;
    int quit;
    int numshows;

    int nummages;
    int numapprentices;
    int numqms;
    int numtacts;

    std::unordered_map<ARegion *, std::unordered_set<FactionActivity, std::hash<int>>> activity;
    int GetActivityCost(FactionActivity type);
    void RecordActivity(ARegion *region, FactionActivity type);
    bool IsActivityRecorded(ARegion *region, FactionActivity type);

    bool gets_gm_report(Game *game);

    /* Used when writing reports */
    std::vector<ARegion *> present_regions;

    AttitudeType defaultattitude;
    // TODO: Convert this to a hashmap of <attitude, vector<factionid>>
    // For now, just making it a vector of attitudes.  More will come later.
    std::vector<Attitude> attitudes;

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
    std::vector<FactionStatistic> compute_faction_statistics(Game *game, size_t **citems);
    void gm_report_setup(Game *game);
    void build_gm_json_report(json& j, Game *game);
};

Faction *GetFaction(std::list<Faction *>& factions, int factionid);

#endif // FACTION_H

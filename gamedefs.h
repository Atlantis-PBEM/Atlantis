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
// MODIFICATONS
// Date        Person            Comments
// ----        ------            --------
// 2000/SEP/06 Joseph Traub      Added base man cost to allow races to have
//                               different base costs
// 2001/FEB/01 Joseph Traub      Added options for flying over water and
//                               easier underworld viewing and farseeing
// 2001/FEB/07 Joseph Traub      Added option to make starting cities safe
//                               or not and to control the guard numbers
//                               and to make them slightly tougher.
//                               Added option to give starting city guards
//                               mage support.
// 2001/Feb/18 Joseph Traub      Added Apprentices idea from Lacandon Conquest
// 2001/Feb/18 Joseph Traub      Added back in support for Conquest
// 2001/Feb/19 Joseph Traub      Removed the ENGINE_VERSION from the gamedef
//                               since it wasn't being used.
// 2001/Feb/21 Joseph Traub      Added a FACLIM_UNLIMITED option
// 2001/Apr/08 Joseph Traub      Added WORLD_NAME option
// 2001/Apr/28 Joseph Traub      Added MORE_PROFITABLE_TRADE_GOODS option
//

#ifndef GAME_DEFS
#define GAME_DEFS

#include "helper.h"

/* Directions */
enum {
    D_NORTH,
    D_NORTHEAST,
    D_SOUTHEAST,
    D_SOUTH,
    D_SOUTHWEST,
    D_NORTHWEST,
    NDIRS
};

extern char **DirectionStrs;

extern char **MonthNames;

class GameDefs {
public:
    char *RULESET_NAME;
    ATL_VER RULESET_VERSION;

    int FOOT_SPEED;
    int HORSE_SPEED;
    int SHIP_SPEED;
    int FLY_SPEED;
    int MAX_SPEED;
	
    int STUDENTS_PER_TEACHER;

    int MAINTENANCE_COST;
    int LEADER_COST;

	// If we use skill level multiplier then no units, all units, leaders,
	// or just mages pay X per level of skill they have per man.   The 
	// first value (MAINTENCE_MULTIPLIER) is how much is payed per skill
	// level.   The second value (MULTIPLIER_USE) is set to one of the
	// values in the given enumeration.
	// The costs listed above are used for any units NOT covered by the
	// multiplier use
	int MAINTENANCE_MULTIPLIER;

	enum {
		MULT_NONE,
		MULT_MAGES,
		MULT_LEADERS,
		MULT_ALL,
	};
	int MULTIPLIER_USE;

    int STARVE_PERCENT;

	enum {
		STARVE_NONE,
		STARVE_MAGES,
		STARVE_LEADERS,
		STARVE_ALL,
	};

	// Instead of dying, lose skill levels, but only for the types of
	// units listed below.   Any unit which should lose a skill level and
	// is unable to will die, period.
	int SKILL_STARVATION;

    int START_MONEY;
    int WORK_FRACTION;
    int ENTERTAIN_FRACTION;
    int ENTERTAIN_INCOME;
  
    int TAX_INCOME;

    int HEALS_PER_MAN;
  
    int GUARD_REGEN; /* percent */
    int CITY_GUARD;
    int GUARD_MONEY;
    int CITY_POP;
  
    int WMON_FREQUENCY;
    int LAIR_FREQUENCY;
  
    int FACTION_POINTS;

    int TURN_COST;

    int TIMES_REWARD;

    int TOWNS_EXIST;
    int LEADERS_EXIST;
    int SKILL_LIMIT_NONLEADERS;
    int MAGE_NONLEADERS;
    int RACES_EXIST;
    int GATES_EXIST;
    int FOOD_ITEMS_EXIST;
    int CITY_MONSTERS_EXIST;
    int WANDERING_MONSTERS_EXIST;
    int LAIR_MONSTERS_EXIST;
    int WEATHER_EXISTS;
    int OPEN_ENDED;
	int NEXUS_EXISTS;
	int CONQUEST_GAME;

    //
    // RANDOM_ECONOMY determines whether the economy for different regions
    // is randomized, or is always the same.
    //
    int RANDOM_ECONOMY;

    //
    // If VARIABLE_ECONOMY is set, the economy of a region is altered after
    // each turn.
    //
    int VARIABLE_ECONOMY;

    //
    // Some economy figures.
    //
    int CITY_MARKET_NORMAL_AMT;
    int CITY_MARKET_ADVANCED_AMT;
    int CITY_MARKET_TRADE_AMT;
	// If any magic items are not set NOMARKET, how many are allowed?
	int CITY_MARKET_MAGIC_AMT;
	// JLT -- Allow higher margins on trade goods.
	int MORE_PROFITABLE_TRADE_GOODS;

    // JLT -- Allow races to have differing base costs
    int BASE_MAN_COST;

	// Are the lastorders values maintained by external scripts?
	int LASTORDERS_MAINTAINED_BY_SCRIPTS;

    // How many turns to allow a faction to be inactive.
	// Set to -1 if you don't want this check performed.
    int MAX_INACTIVE_TURNS;

	// Is it easier to deal with the underworld (allows teleport and
	// farsight into the underworld)
	int EASIER_UNDERWORLD;

	// Should units with no orders perform a default 'work' order
	int DEFAULT_WORK_ORDER;

    //
    // The type of faction limits that are in effect in this game.
    //
    enum {
        FACLIM_MAGE_COUNT,
        FACLIM_FACTION_TYPES,
		FACLIM_UNLIMITED,
    };
    int FACTION_LIMIT_TYPE;

	//
	// The type of flight over water that is available.
	//
	enum {
		WFLIGHT_NONE,
		WFLIGHT_MUST_LAND,
		WFLIGHT_UNLIMITED,
	};
	int FLIGHT_OVER_WATER;

	//
	// Are starting cities safe regions?  Also controls if guards in
	// the starting cities get amulets of invulnerability.
	//
	int SAFE_START_CITIES;

	//
	// How many guards in starting cities?
	//
	int AMT_START_CITY_GUARDS;

	//
	// Should starting city guards be made tougher than normal city guards?
	// (this means giving them plate armor)
	//
	int START_CITY_GUARDS_PLATE;

	//
	// Do starting cities have a fire mages.  (0 for no mages, otherwise
	// this is the level of their fire skill)
	// 
	//
	int START_CITY_MAGES;

	// Do the starting city guards also have tactician skill (0 for
	// no tactician, otherwise this is their level of tactician)
	int START_CITY_TACTICS;

	//
	// Are we allowing apprentices?
	//
	int APPRENTICES_EXIST;

	// What is the name of the world?
	char *WORLD_NAME;

	// Does the nexus allow gating out of it
	int NEXUS_GATE_OUT;

	// Is the nexus also a city?
	int NEXUS_IS_CITY;

	// Do battle reports show factions if ANY unit on the opposing side
	// could see it.   Non-involved observers will ALSO see this
	// information.
	int BATTLE_FACTION_INFO;

	// Is the withdraw order enabled
	int ALLOW_WITHDRAW;

	// Do cities have a cost to rename them?  If this value is set,
	// the cost is the city size (1, 2, 3) * this value
	int CITY_RENAME_COST;

	// Are taxing and pillaging month-long actions?
	int TAX_PILLAGE_MONTH_LONG;

	// Are we allowing a multi-hex nexus
	int MULTI_HEX_NEXUS;

	// How many levels of the underworld do we want?
	int UNDERWORLD_LEVELS;

	// How many levels of the underdeep do we want?
	int UNDERDEEP_LEVELS;

	// Is there an abyss level?
	int ABYSS_LEVEL;

	// Does farsight make use of a mages other skills
	int IMPROVED_FARSIGHT;

	// Should the GM get a full world report every turn
	int GM_REPORT;

	// Do we allow objects to decay according to the parameters in the object
	// definition table
	int DECAY;

	// Do we limit the number of mages which can study inside of certain
	// buildings.
	int LIMITED_MAGES_PER_BUILDING;

	// Transit report options
	enum {
		REPORT_NOTHING = 0x0000,
		// Various things which can be shown
		REPORT_SHOW_PEASANTS = 0x0001,
		REPORT_SHOW_REGION_MONEY = 0x0002,
		REPORT_SHOW_WAGES = 0x0004,
		REPORT_SHOW_MARKETS = 0x0008,
		REPORT_SHOW_RESOURCES = 0x0010,
		REPORT_SHOW_ENTERTAINMENT = 0x0020,
		// Collection of the the above
		REPORT_SHOW_ECONOMY = (REPORT_SHOW_PEASANTS |
							   REPORT_SHOW_REGION_MONEY |
							   REPORT_SHOW_WAGES |
							   REPORT_SHOW_MARKETS |
							   REPORT_SHOW_RESOURCES |
							   REPORT_SHOW_ENTERTAINMENT),
		// Which type of exits to show
		REPORT_SHOW_USED_EXITS = 0x0040,
		REPORT_SHOW_ALL_EXITS = 0x0080,
		// Which types of units to show
		REPORT_SHOW_GUARDS = 0x0100,
		REPORT_SHOW_INDOOR_UNITS = 0x0200,
		REPORT_SHOW_OUTDOOR_UNITS = 0x0400,
		// Collection of the above
		REPORT_SHOW_UNITS = (REPORT_SHOW_GUARDS | REPORT_SHOW_INDOOR_UNITS |
							 REPORT_SHOW_OUTDOOR_UNITS),
		// Various types of buildings
		REPORT_SHOW_BUILDINGS = 0x0800,
		REPORT_SHOW_ROADS = 0x1000,
		REPORT_SHOW_SHIPS = 0x2000,
		// Collection of the above
		REPORT_SHOW_STRUCTURES = (REPORT_SHOW_BUILDINGS |
								  REPORT_SHOW_ROADS |
								  REPORT_SHOW_SHIPS),
		// Should the unit get to use their advanced skills?
		REPORT_USE_UNIT_SKILLS = 0x8000,

		// Some common collections
		REPORT_SHOW_REGION = (REPORT_SHOW_ECONOMY | REPORT_SHOW_ALL_EXITS),
		REPORT_SHOW_EVERYTHING = (REPORT_SHOW_REGION |
								  REPORT_SHOW_UNITS |
								  REPORT_SHOW_STRUCTURES),
	};

	// What sort of information should be shown to a unit just passing
	// through a hex?
	int TRANSIT_REPORT;

	// Should advanced items be shown in markets at all times.
	int MARKETS_SHOW_ADVANCED_ITEMS;

	// Do we require the 'ready' command to set up battle items
	int USE_PREPARE_COMMAND;

	// Monsters have the option of advancing occasionally instead of just
	// using move.
	// There are two values which control this.
	// MONSTER_ADVANCE_MIN_PERCENT is the minimum amount which monsters
	// should advance.
	// MONSTER_ADVANCE_HOSTILE_PERCENT is the percent of their hostile
	// rating which should be used to determine if they advance.
	// If you don't want monsters to EVER advance, use 0 for both.
	// If you want a flat percent, use MIN_PERCENT and set HOSTILE_PERCENT
	// to 0.  If you only want it based on the HOSTILE value, set MIN_PERCENT
	// to 0 and HOSTILE_PERCENT to what you want.
	int MONSTER_ADVANCE_MIN_PERCENT;
	int MONSTER_ADVANCE_HOSTILE_PERCENT;
};

extern GameDefs * Globals;

#endif

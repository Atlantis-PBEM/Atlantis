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
// MODIFICATIONS
// Date        Person            Comments
// ----        ------            --------
// 2001/Feb/21 Joseph Traub      Moved the item and such definitions into
//                               gamedata.cpp
//
#include "gamedata.h"
#include "gamedefs.h"

//
// Define the various globals for this game.
//
// If you change any of these, it is incumbent on you, the GM to change
// the html file containing the rules to correctly reflect the changes!
static GameDefs g = {
    "Atlantis Conquest",     // RULESET_NAME
    MAKE_ATL_VER( 1, 0, 2 ), // RULESET_VERSION

    2, /* FOOT_SPEED */
    4, /* HORSE_SPEED */
    4, /* SHIP_SPEED */
    6, /* FLY_SPEED */
    8, /* MAX_SPEED */
		   
    250, /* WAGON_CAPACITY */

    10, /* STUDENTS_PER_TEACHER */
    10, /* MAINTENANCE_COST */
    10, /* LEADER_COST */
	0,  /* MAINTAINENCE_MULTIPLIER */
	GameDefs::MULT_NONE, /* MULTIPLIER_USE */

    33, /* STARVE_PERCENT */
	GameDefs::STARVE_NONE, /* SKILL_STARVATION */

    5020, /* START_MONEY */
    5, /* WORK_FRACTION */
    20, /* ENTERTAIN_FRACTION */
    20, /* ENTERTAIN_INCOME */

    50, /* TAX_INCOME */
	
    5, /* HEALS_PER_MAN */

    20, /* GUARD_REGEN */ /* percent */
    40, /* CITY_GUARD */
    50, /* GUARD_MONEY */
    4000, /* CITY_POP */

    10, /* WMON_FREQUENCY */
    10, /* LAIR_FREQUENCY */
	
    5, /* FACTION_POINTS */

    64, /* MAX_AC_X */
    64, /* MAX_AC_Y */

    100, /* TURN_COST */

    50, /* TIMES_REWARD */

    0, // TOWNS_EXIST
    0, // LEADERS_EXIST
    0, // SKILL_LIMIT_NONLEADERS
    1, // MAGE_NONLEADERS
    0, // RACES_EXIST
    0, // GATES_EXIST
    0, // FOOD_ITEMS_EXIST
    0, // CITY_MONSTERS_EXIST
    0, // WANDERING_MONSTERS_EXIST
    0, // LAIR_MONSTERS_EXIST
    0, // WEATHER_EXISTS
    0, // OPEN_ENDED

    0, // RANDOM_ECONOMY
    0, // VARIABLE_ECONOMY

    0, // CITY_MARKET_NORMAL_AMT
    0, // CITY_MARKET_ADVANCED_AMT
    0, // CITY_MARKET_TRADE_AMT
	0, // MORE_PROFITABLE_TRADE_GOODS

	50,	// BASE_MAN_COST
	0, // LASTORDERS_MAINTAINED_BY_SCRIPTS
	10, // MAX_INACTIVE_TURNS

	// *NOTE* If this is set to 1, you need to make sure the correct
	// section in skillshows.cpp is defined to describe the X,Y,Z
	// coordinates.  If you set it to zero, make sure the original
	// skill description is enabled instead
	0, // EASIER_UNDERWORLD

	0, // DEFAULT_WORK_ORDER

    GameDefs::FACLIM_MAGE_COUNT, // FACTION_LIMIT_TYPE

	GameDefs::WFLIGHT_NONE,	// FLIGHT_OVER_WATER

	0,   // SAFE_START_CITIES
	120, // AMT_START_CITY_GUARDS
	0,   // START_CITY_GUARDS_PLATE
	0,   // START_CITY_MAGES
	0,   // START_CITY_TACTICS

	1,   // APPRENTICES_EXIST

	"Atlantis", // WORLD_NAME

	0,   // NEXUS_GATE_OUT
	0,   // NEXUS_IS_CITY
	0,	// BATTLE_FACTION_INFO
	0,	// ALLOW_WITHDRAW
	0,	// CITY_RENAME_COST
	0,	// TAX_PILLAGE_MONTH_LONG
	0,	// MULTI_HEX_NEXUS
	0,	// UNDERWORLD_LEVELS
	0,	// UNDERDEEP_LEVELS
};

GameDefs * Globals = &g;

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
#include "gamedefs.h"

//
// Define the various globals for this game.
//
// If you change any of these, it is incumbent on you, the GM to change
// the html file containing the rules to correctly reflect the changes!
//

static int am[] = { 1, 2, 3, 4, 5, 6 };
int *allowedMages = am;
int allowedMagesSize = sizeof(am) / sizeof(am[0]);

static int aa[] = { 1, 3, 5, 7, 10, 15 };
int *allowedApprentices = aa;
int allowedApprenticesSize = sizeof(aa) / sizeof(aa[0]);

static int aw[] = { 0, 15, 30, 50, 75, 100 };
int *allowedTaxes = aw;
int allowedTaxesSize = sizeof(aw) / sizeof(aw[0]);

static int at[] = { 0, 15, 30, 50, 75, 100 };
int *allowedTrades = at;
int allowedTradesSize = sizeof(at) / sizeof(at[0]);

static int aq[] = { 0, 2, 5, 9, 14, 25 };
int *allowedQuartermasters = aq;
int allowedQuartermastersSize = sizeof(aq) / sizeof(aq[0]);

// at is already taken up for allowedtaxes, so I'll use ag (allowedgenghises) ;)
static int ag[] = { 0, 1, 2, 4, 6, 10 };
int *allowedTacticians = ag;
int allowedTacticiansSize = sizeof(ag) / sizeof(ag[0]);

static GameDefs g = {
	"NewOrigins",	// RULESET_NAME
	MAKE_ATL_VER( 2, 0, 0 ),	// RULESET_VERSION

	8,	/* MAX_SPEED */
	7,	/* PHASED_MOVE_OFFSET */
	2,	/* FLEET_WIND_BOOST */
	0,	/* FLEET_CREW_BOOST */
	0,	/* FLEET_LOAD_BOOST */


	10,	/* STUDENTS_PER_TEACHER */
	10,	/* MAINTENANCE_COST */
	50,	/* LEADER_COST */

	0,	/* MAINTAINENCE_MULTIPLIER */
	GameDefs::MULT_NONE,	/* MULTIPLIER_USE */

	33,	/* STARVE_PERCENT */
	GameDefs::STARVE_NONE,	/* SKILL_STARVATION */

	10000,	/* START_MONEY */
	5,	/* WORK_FRACTION */
	20,	/* ENTERTAIN_FRACTION */
	30,	/* ENTERTAIN_INCOME */

	50,	/* TAX_BASE_INCOME */
	0,	/* TAX_BONUS_WEAPON */
	0,	/* TAX_BONUS_ARMOR */
	0,	/* TAX_BONUS_FORT */
	// WHO_CAN_TAX
	GameDefs::TAX_NORMAL
		| GameDefs::TAX_HORSE_AND_RIDING_SKILL
		| GameDefs::TAX_MAGE_DAMAGE,
	1,	// TAX_PILLAGE_MONTH_LONG

	5,	/* HEALS_PER_MAN */

	20,	/* GUARD_REGEN */ /* percent */
	40,	/* CITY_GUARD */
	50,	/* GUARD_MONEY */
	10000,	/* CITY_POP */

	30,	/* WMON_FREQUENCY */
	30,	/* LAIR_FREQUENCY */

	5,	/* FACTION_POINTS */

	200,	/* TIMES_REWARD */

	1,	// TOWNS_EXIST
	1,	// LEADERS_EXIST
	0,	// SKILL_LIMIT_NONLEADERS
	0,	// MAGE_NONLEADERS
	1,	// RACES_EXIST
	1,	// GATES_EXIST
	1,	// FOOD_ITEMS_EXIST
	0,	// COASTAL_FISH
	1,	// CITY_MONSTERS_EXIST
	1,	// WANDERING_MONSTERS_EXIST
	1,	// LAIR_MONSTERS_EXIST
	0,	// WEATHER_EXISTS
	0,	// OPEN_ENDED
	1,	// NEXUS_EXISTS
	0,	// CONQUEST_GAME

	1,	// RANDOM_ECONOMY
	1,	// VARIABLE_ECONOMY

	50,	// CITY_MARKET_NORMAL_AMT
	20,	// CITY_MARKET_ADVANCED_AMT
	50,	// CITY_MARKET_TRADE_AMT
	20,	// CITY_MARKET_MAGIC_AMT
	1, 	// MORE_PROFITABLE_TRADE_GOODS

	50,	// BASE_MAN_COST
	1,	// LASTORDERS_MAINTAINED_BY_SCRIPTS
	10,	// MAX_INACTIVE_TURNS

	1,	// EASIER_UNDERWORLD

	1,	// DEFAULT_WORK_ORDER

	GameDefs::FACLIM_FACTION_TYPES,	// FACTION_LIMIT_TYPE

	GameDefs::WFLIGHT_MUST_LAND,	// FLIGHT_OVER_WATER

	0,	// START_CITIES_EXIST
	0,	// SAFE_START_CITIES
	500,	// AMT_START_CITY_GUARDS
	1,	// START_CITY_GUARDS_PLATE
	4,	// START_CITY_MAGES
	4,	// START_CITY_TACTICS
	1,	// APPRENTICES_EXIST
	"apprentice",	// APPRENTICE_NAME

	"Atlantis",	// WORLD_NAME

	1,	// NEXUS_GATE_OUT
	0,	// NEXUS_IS_CITY
	0,	// BATTLE_FACTION_INFO
	1,	// ALLOW_WITHDRAW
	2000,	// CITY_RENAME_COST
	0,	// MULTI_HEX_NEXUS
	0,	// ICOSAHEDRAL_WORLD
	1,	// UNDERWORLD_LEVELS
	0,	// UNDERDEEP_LEVELS
	0,	// ABYSS_LEVEL
	90,	// TOWN_PROBABILITY
	100,	// TOWN_SPREAD
	100,	// TOWNS_NOT_ADJACENT
	0,	// LESS_ARCTIC_TOWNS
	55,	// OCEAN
  4,	// CONTINENT_SIZE
	4,	// TERRAIN_GRANULARITY
	1,	// LAKES
	60,	// ARCHIPELAGO
	40,	// SEVER_LAND_BRIDGES
	6,	// SEA_LIMIT	
	GameDefs::NO_EFFECT,	// LAKE_WAGE_EFFECT
	0,	// LAKESIDE_IS_COASTAL
	40,	// ODD_TERRAIN
	1,	// IMPROVED_FARSIGHT
	1,	// GM_REPORT
	0,	// DECAY
	1,	// LIMITED_MAGES_PER_BUILDING
	// TRANSIT_REPORT
	GameDefs::REPORT_SHOW_REGION |
		GameDefs::REPORT_SHOW_ROADS |
		GameDefs::REPORT_SHOW_BUILDINGS |
		GameDefs::REPORT_SHOW_GUARDS |
		GameDefs::REPORT_SHOW_INDOOR_UNITS,
	0,	// MARKETS_SHOW_ADVANCED_ITEMS
	GameDefs::PREPARE_NORMAL,	// USE_PREPARE_COMMAND
	5,	// MONSTER_ADVANCE_MIN_PERCENT
	75,	// MONSTER_ADVANCE_HOSTILE_PERCENT
	0,	// HAVE_EMAIL_SPECIAL_COMMANDS
	1,	// START_CITIES_START_UNLIMITED
	1,	// PROPORTIONAL_AMTS_USAGE
	1,	// USE_WEAPON_ARMOR_COMMAND
	4,	// MONSTER_NO_SPOILS
	4,	// MONSTER_SPOILS_RECOVERY
	0,	// MAX_ASSASSIN_FREE_ATTACKS
	0,	// RELEASE_MONSTERS
	1,	// CHECK_MONSTER_CONTROL_MID_TURN
	1,	// DETECT_GATE_NUMBERS
	GameDefs::ARMY_ROUT_FIGURES,	// ARMY_ROUT
	1,	// ONLY_ROUT_ONCE
	0,	// ADVANCED_FORTS // ?????
	1,	// FULL_TRUESEEING_BONUS !!! v1 - CHANGE
	0,	// IMPROVED_AMTS
	1,	// FULL_INVIS_ON_SELF !!! v1 - CHANGE
	0,	// MONSTER_BATTLE_REGEN
	5,	// SKILL_PRACTICE_AMOUNT
	0,	// REQUIRED_EXPERIENCE
	0,	// UPKEEP_MINIMUM_FOOD
	-1,	// UPKEEP_MAXIMUM_FOOD
	30,	// UPKEEP_FOOD_VALUE
	1,	// PREVENT_SAIL_THROUGH
	1,	// ALLOW_TRIVIAL_PORTAGE !!! v1 - CHANGE
	0,	// GATES_NOT_PERENNIAL
	0,	// START_GATES_OPEN
	0,	// SHOW_CLOSED_GATES
	1,	// TRANSPORT !!! v1 - CHANGE
	1,	// LOCAL_TRANSPORT !!! v1 - CHANGE
	3,	// NONLOCAL_TRANSPORT !!! v1 - CHANGE
	5,	// SHIPPING_COST
	0,	// FRACTIONAL_WEIGHT
	0,	// GROW_RACES
	0,	// DYNAMIC_POPULATION
	100,	// POP_GROWTH
	3,	// DELAY_MORTALITY
	6,	// DELAY_GROWTH
	100,	// TOWN_DEVELOPMENT
	0,	// TACTICS_NEEDS_WAR
	1,	// ALLIES_NOAID !!! v1 - CHANGE
	0,	// HARDER_ASSASSINATION
	1,	// DISPERSE_GATE_NUMBERS
	33,	// UNDEATH_CONTAGION
	1,   // REGIONS_ECONOMY
	1   // ADVANCED_TACTICS
};

GameDefs *Globals = &g;

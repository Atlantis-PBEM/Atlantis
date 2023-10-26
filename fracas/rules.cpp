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

static int am[] = { 0, 1, 2, 3, 5, 7 };
int *allowedMages = am;
int allowedMagesSize = sizeof(am) / sizeof(am[0]);

static int aa[] = { 0, 2, 4, 6, 10, 14 };
int *allowedApprentices = aa;
int allowedApprenticesSize = sizeof(aa) / sizeof(aa[0]);

static int aw[] = { 0, 10, 24, 40, 60, 100 };
int *allowedTaxes = aw;
int allowedTaxesSize = sizeof(aw) / sizeof(aw[0]);

static int at[] = { 0, 10, 24, 40, 60, 100 };
int *allowedTrades = at;
int allowedTradesSize = sizeof(at) / sizeof(at[0]);

static int aq[] = { 0, 2, 4, 8, 12, 20 };
int *allowedQuartermasters = aq;
int allowedQuartermastersSize = sizeof(aq) / sizeof(aq[0]);

// at is already taken up for allowedtaxes, so I'll use ag (allowedgenghises) ;)
static int ag[] = { 0, 1, 2, 4, 6, 10 };
int *allowedTacticians = ag;
int allowedTacticiansSize = sizeof(ag) / sizeof(ag[0]);

// allowed Martial activity
static int ma[] = { 0, 10, 25, 40, 60, 90 };
int *allowedMartial = ma;
int allowedMartialSize = sizeof(ma) / sizeof(ma[0]);

static GameDefs g = {
	"Fracas",				// RULESET_NAME
	MAKE_ATL_VER( 4, 0, 0 ), // RULESET_VERSION

	8, /* MAX_SPEED */
	7, /* PHASED_MOVE_OFFSET */
	2, /* FLEET_WIND_BOOST */
	0, /* FLEET_CREW_BOOST */
	0, /* FLEET_LOAD_BOOST */

	10, /* STUDENTS_PER_TEACHER */
	10, /* MAINTENANCE_COST */
	20, /* LEADER_COST */

	0,  /* MAINTAINENCE_MULTIPLIER */
	GameDefs::MULT_NONE, /* MULTIPLIER_USE */

	33, /* STARVE_PERCENT */
	GameDefs::STARVE_NONE, /* SKILL_STARVATION */

	10000, /* START_MONEY */
	3, /* WORK_FRACTION */
	20, /* ENTERTAIN_FRACTION */
	20, /* ENTERTAIN_INCOME */

	40, /* TAX_BASE_INCOME */
	 0, /* TAX_BONUS_WEAPON */
	 0, /* TAX_BONUS_ARMOR */
	 0, /* TAX_BONUS_FORT */
 	GameDefs::TAX_NORMAL, // WHO_CAN_TAX
	0,	// TAX_PILLAGE_MONTH_LONG

	5, /* HEALS_PER_MAN */

	20, /* GUARD_REGEN */ /* percent */
	40, /* CITY_GUARD */
	50, /* GUARD_MONEY */
	20000, /* CITY_POP */

	30, /* WMON_FREQUENCY */
	30, /* LAIR_FREQUENCY */

	5, /* FACTION_POINTS */

	50, /* TIMES_REWARD */

	1, // TOWNS_EXIST
	1, // LEADERS_EXIST
	1, // SKILL_LIMIT_NONLEADERS
	0, // MAGE_NONLEADERS
	1, // RACES_EXIST
	1, // GATES_EXIST
	1, // FOOD_ITEMS_EXIST
	1, // COASTAL_FISH
	1, // CITY_MONSTERS_EXIST
	1, // WANDERING_MONSTERS_EXIST
	1, // LAIR_MONSTERS_EXIST
	1, // WEATHER_EXISTS
	1, // OPEN_ENDED
	0, // NEXUS_EXISTS
	0, // CONQUEST_GAME

	1, // RANDOM_ECONOMY
	1, // VARIABLE_ECONOMY

	50, // CITY_MARKET_NORMAL_AMT
	20, // CITY_MARKET_ADVANCED_AMT
	50, // CITY_MARKET_TRADE_AMT
	20, // CITY_MARKET_MAGIC_AMT
	0,  // MORE_PROFITABLE_TRADE_GOODS

	50,	// BASE_MAN_COST
	0, // LASTORDERS_MAINTAINED_BY_SCRIPTS
	10, // MAX_INACTIVE_TURNS

	1, // EASIER_UNDERWORLD

	1, // DEFAULT_WORK_ORDER

	GameDefs::FACLIM_FACTION_TYPES, // FACTION_LIMIT_TYPE

	GameDefs::WFLIGHT_UNLIMITED,	// FLIGHT_OVER_WATER

	0,   // START_CITIES_EXIST
	0,   // SAFE_START_CITIES
	500, // AMT_START_CITY_GUARDS
	1,   // START_CITY_GUARDS_PLATE
	4,   // START_CITY_MAGES
	4,   // START_CITY_TACTICS
	0,   // APPRENTICES_EXIST
	"apprentice",   // APPRENTICE_NAME

	"Fracas", // WORLD_NAME

	1,  // NEXUS_GATE_OUT
	0,  // NEXUS_IS_CITY
	0,	// BATTLE_FACTION_INFO
	1,	// ALLOW_WITHDRAW
	0,	// CITY_RENAME_COST
	0,	// MULTI_HEX_NEXUS
	0,	// ICOSAHEDRAL_WORLD
	0,	// UNDERWORLD_LEVELS
	0,	// UNDERDEEP_LEVELS
	0,	// ABYSS_LEVEL
	100,	  // TOWN_PROBABILITY
	25,	// TOWN_SPREAD
	90,	// TOWNS_NOT_ADJACENT
	5,	// LESS_ARCTIC_TOWNS
	50, // OCEAN
	20, // CONTINENT_SIZE
	0, // TERRAIN_GRANULARITY
	0, // LAKES
	10,	// ARCHIPELAGO
	30, // SEVER_LAND_BRIDGES
	8, // SEA_LIMIT
	GameDefs::NO_EFFECT, // LAKE_WAGE_EFFECT
	0,	// LAKESIDE_IS_COASTAL
	20,	// ODD_TERRAIN
	0,	// IMPROVED_FARSIGHT
	1,	// GM_REPORT
	0,	// DECAY
	1,	// LIMITED_MAGES_PER_BUILDING
	GameDefs::REPORT_SHOW_EVERYTHING, // TRANSIT_REPORT
	//GameDefs::REPORT_NOTHING, // TRANSIT_REPORT
	0,  // MARKETS_SHOW_ADVANCED_ITEMS
	0,	// USE_PREPARE_COMMAND
	15,	// MONSTER_ADVANCE_MIN_PERCENT
	0,	// MONSTER_ADVANCE_HOSTILE_PERCENT
	// Thera/Fracas has email special commands, but they
	// don't exactly correspond
	0,	// HAVE_EMAIL_SPECIAL_COMMANDS
	1,	// START_CITIES_START_UNLIMITED
	1,	// PROPORTIONAL_AMTS_USAGE
	1,  // USE_WEAPON_ARMOR_COMMAND
	3,  // MONSTER_NO_SPOILS
	9,  // MONSTER_SPOILS_RECOVERY
	1,  // MAX_ASSASSIN_FREE_ATTACKS
	0,  // RELEASE_MONSTERS
	1,  // CHECK_MONSTER_CONTROL_MID_TURN
	0,  // DETECT_GATE_NUMBERS
	GameDefs::ARMY_ROUT_FIGURES,  // ARMY_ROUT
	0,  // ONLY_ROUT_ONCE
	0,  // ADVANCED_FORTS
	1,	// FULL_TRUESEEING_BONUS
	0,	// IMPROVED_AMTS
	0,	// FULL_INVIS_ON_SELF
	1,	// MONSTER_BATTLE_REGEN
	5,	// SKILL_PRACTICE_AMOUNT
	0, // REQUIRED_EXPERIENCE
	// Don't enable food in fracas - I've disabled it in the start up :>
	0,	// UPKEEP_MINIMUM_FOOD
	-1,	// UPKEEP_MAXIMUM_FOOD
	10,	// UPKEEP_FOOD_VALUE
	1,	// PREVENT_SAIL_THROUGH
	0,	// ALLOW_TRIVIAL_PORTAGE
	0,  // GATES_NOT_PERENNIAL
	0,  // START_GATES_OPEN
	0,  // SHOW_CLOSED_GATES
	GameDefs::ALLOW_TRANSPORT|GameDefs::QM_AFFECT_COST|GameDefs::QM_AFFECT_DIST,  // TRANSPORT
	2,	// LOCAL_TRANSPORT
	6,	// NONLOCAL_TRANSPORT
	1,	// SHIPPING_COST
	0,	// FRACTIONAL_WEIGHT
	1,	// GROW_RACES
	1,  // DYNAMIC_POPULATION
	100, // POP_GROWTH
	3, // DELAY_MORTALITY
	6, // DELAY_GROWTH
	100, // TOWN_DEVELOPMENT
	1, // TACTICS_NEEDS_WAR
	1, // ALLIES_NOAID
	0, // HARDER_ASSASSINATION
	0, //DISPERSE_GATE_NUMBERS
	0,	// UNDEATH_CONTAGION
	0,   // REGIONS_ECONOMY
	0, // ADVANCED_TACTICS
	0,	// OVERWHELMING
	0, // STRICT_GUARD
	1, // OCEAN_GUARD
	BattleLogLevel::NORMAL,	// BATTLE_LOG_LEVEL
	0, // EXTENDED_FORT_DEFENCE
	0, // SPOILS_NO_TRADE
	1,	// WORLD_EVENTS
	0,	//FACTION_STATISTICS
	0,  // BUILD_NO_TRADE
	0, // TRANSPORT_NO_TRADE
	FactionActivityRules::DEFAULT,	// FACTION_ACTIVITY
	DestroyBehavior::INSTANT,	// DESTROY_BEHAVIOR
	200,	// MIN_DESTROY_POINTS,
	34,	// MAX_DESTROY_PERCENT
	0, // HALF_RIDING_BONUS
	GameDefs::REPORT_FORMAT_TEXT,	// REPORT_FORMAT
};

GameDefs *Globals = &g;

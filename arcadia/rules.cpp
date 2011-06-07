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

static int am[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
int *allowedMages = am;
int allowedMagesSize = sizeof(am) / sizeof(am[0]);

static int aa[] = { 0, 2, 4, 6, 10, 14 };
int *allowedApprentices = aa;
int allowedApprenticesSize = sizeof(aa) / sizeof(aa[0]);

static int aw[] = { 1, 4, 7, 10, 13, 16, 19, 22, 25 };
int *allowedTaxes = aw;
int allowedTaxesSize = sizeof(aw) / sizeof(aw[0]);

static int at[] = { 1, 4, 7, 10, 13, 16, 19, 22, 25 };
int *allowedTrades = at;
int allowedTradesSize = sizeof(at) / sizeof(at[0]);

static int aq[] = { 0, 2, 4, 8, 12, 20 };
int *allowedQuartermasters = aq;
int allowedQuartermastersSize = sizeof(aq) / sizeof(aq[0]);

// at is already taken up for allowedtaxes, so I'll use ag (allowedgenghises) ;)
static int ag[] = { 0, 1, 2, 4, 6, 10 };
int *allowedTacticians = ag;
int allowedTacticiansSize = sizeof(ag) / sizeof(ag[0]);

static GameDefs g = {
	"Xanaxor",				// RULESET_NAME
	MAKE_ATL_VER( 4, 0, 0 ), // RULESET_VERSION

	2, /* FOOT_SPEED */
	4, /* HORSE_SPEED */
	8, /* SHIP_SPEED */   //This is no longer used!
	6, /* FLY_SPEED */
	16, /* MAX_SPEED */
	2, /* FLEET_WIND_BOOST */
	0, /* FLEET_CREW_BOOST */
	0, /* FLEET_LOAD_BOOST */

	10, /* STUDENTS_PER_TEACHER */
	10, /* MAINTENANCE_COST */
	20, /* LEADER_COST */

	0,  /* MAINTAINENCE_MULTIPLIER */
	GameDefs::MULT_NONE, /* MULTIPLIER_USE */

	33, /* STARVE_PERCENT */
	GameDefs::STARVE_LEADERS, /* SKILL_STARVATION */

	9100, /* START_MONEY */
	4, /* WORK_FRACTION */
	20, /* ENTERTAIN_FRACTION */
	20, /* ENTERTAIN_INCOME */

	50, /* TAX_BASE_INCOME */
	 0, /* TAX_BONUS_WEAPON */
	 0, /* TAX_BONUS_ARMOR */
	 0, /* TAX_BONUS_FORT */
	GameDefs::TAX_NORMAL | GameDefs::TAX_HORSE_AND_RIDING_SKILL_AND_MELEE_WEAPON, // WHO_CAN_TAX
	1,	// TAX_PILLAGE_MONTH_LONG

	5, /* HEALS_PER_MAN */

	60, /* GUARD_REGEN */ /* percent */
	20, /* CITY_GUARD */
	1,  /* GUARD_DEPENDS_ON_TAX */
	50, /* GUARD_MONEY */
	4000, /* CITY_POP */

	35, /* WMON_FREQUENCY */
	17, /* LAIR_FREQUENCY */

	8, /* FACTION_POINTS */

	100, /* TIMES_REWARD */
	
	0, // SEPERATE_TEMPLATES

	1, // TOWNS_EXIST
	0, // LEADERS_EXIST
	1, // SKILL_LIMIT_NONLEADERS
	1, // MAGE_NONLEADERS
	1, // RACES_EXIST
	1, // GATES_EXIST
	1, // FOOD_ITEMS_EXIST
	0, // COASTAL_FISH
	1, // CITY_MONSTERS_EXIST
	1, // WANDERING_MONSTERS_EXIST
	1, // LAIR_MONSTERS_EXIST
	1, // WEATHER_EXISTS
	0, // OPEN_ENDED
	1, // NEXUS_EXISTS
	0, // CONQUEST_GAME

	1, // RANDOM_ECONOMY
	1, // VARIABLE_ECONOMY

	50, // CITY_MARKET_NORMAL_AMT
	20, // CITY_MARKET_ADVANCED_AMT
	60, // CITY_MARKET_TRADE_AMT
	20, // CITY_MARKET_MAGIC_AMT
	1,  // MORE_PROFITABLE_TRADE_GOODS

	50,	// BASE_MAN_COST
	0, // LASTORDERS_MAINTAINED_BY_SCRIPTS
	6, // MAX_INACTIVE_TURNS

	0, // EASIER_UNDERWORLD

	1, // DEFAULT_WORK_ORDER

	GameDefs::FACLIM_FACTION_TYPES, // FACTION_LIMIT_TYPE

	GameDefs::WFLIGHT_NONE,	// FLIGHT_OVER_WATER

	1,   // START_CITIES_EXIST
	0,   // SAFE_START_CITIES
	50, // AMT_START_CITY_GUARDS
	0,   // START_CITY_GUARDS_PLATE
	3,   // START_CITY_MAGES
	0,   // START_CITY_TACTICS
	0,   // APPRENTICES_EXIST  // should be set to zero with Arcadia

	"Arcadia IV", // WORLD_NAME

	1,  // NEXUS_GATE_OUT
	0,  // NEXUS_IS_CITY
	1,	// BATTLE_FACTION_INFO
	1,	// ALLOW_WITHDRAW
	1,  // SEND_COST
	0,	// ALLOW_BANK
	0,	// BANK_FEE
	0,	// BANK_MAXUNSKILLED
	0,	// BANK_MAXSKILLPERLEVEL
	0,	// CITY_RENAME_COST
	1,	// MULTI_HEX_NEXUS
	0,	// UNDERWORLD_LEVELS
	0,	// UNDERDEEP_LEVELS
	0,	// ABYSS_LEVEL
	0,  // FLAT_WORLD
	250,	  // TOWN_PROBABILITY
	80,	// TOWN_SPREAD
	60,	// TOWNS_NOT_ADJACENT
	0,	// LESS_ARCTIC_TOWNS
	50, // OCEAN
	10, // CONTINENT_SIZE
	2, // TERRAIN_GRANULARITY
	2, // LAKES
	20,	// ARCHIPELAGO
	40, // SEVER_LAND_BRIDGES
	6, // SEA_LIMIT	
	GameDefs::NO_EFFECT, // LAKE_WAGE_EFFECT
	1,	// LAKESIDE_IS_COASTAL
	20,	// ODD_TERRAIN
	1,	// IMPROVED_FARSIGHT
	1,	// GM_REPORT
	0,	// DECAY
	0,	// LIMITED_MAGES_PER_BUILDING  // This should be set to 0 with Arcadia
	(GameDefs::REPORT_SHOW_REGION | GameDefs::REPORT_SHOW_BUILDINGS), // TRANSIT_REPORT
	0,  // MARKETS_SHOW_ADVANCED_ITEMS
	GameDefs::PREPARE_NONE,	// USE_PREPARE_COMMAND
	5,	// MONSTER_ADVANCE_MIN_PERCENT
	75,	// MONSTER_ADVANCE_HOSTILE_PERCENT
	0,	// HAVE_EMAIL_SPECIAL_COMMANDS
	0,	// START_CITIES_START_UNLIMITED
	1,	// PROPORTIONAL_AMTS_USAGE
	0,  // USE_WEAPON_ARMOR_COMMAND
	0,  // MONSTER_NO_SPOILS
	0,  // MONSTER_SPOILS_RECOVERY
	3,  // MAX_ASSASSIN_FREE_ATTACKS
	0,  // RELEASE_MONSTERS
	1,  // CHECK_MONSTER_CONTROL_MID_TURN
	1,  // DETECT_GATE_NUMBERS
	GameDefs::ARMY_ROUT_HITS_FIGURE,  // ARMY_ROUT
	1,  // ADVANCED_FORTS
	0,	// FULL_TRUESEEING_BONUS  - zero with Arcadia
	0,	// IMPROVED_AMTS 
	0,	// FULL_INVIS_ON_SELF     - zero with Arcadia
	0,	// MONSTER_BATTLE_REGEN
	0,	// SKILL_PRACTICE_AMOUNT
	0,	// UPKEEP_MINIMUM_FOOD
	-1,	// UPKEEP_MAXIMUM_FOOD
	20,	// UPKEEP_FOOD_VALUE
	0,	// PREVENT_SAIL_THROUGH
	0,	// ALLOW_TRIVIAL_PORTAGE
	9,  // GATES_NOT_PERENNIAL
	0,  // START_GATES_OPEN
	1,  // SHOW_CLOSED_GATES
	0,	// TRANSPORT               - zero with Arcadia
	1,	// LOCAL_TRANSPORT
	3,	// NONLOCAL_TRANSPORT
	5,	// SHIPPING_COST
	0,	// FRACTIONAL_WEIGHT
	0, // GROW_RACES
	0,  // PLAYER_ECONOMY
	100, // POP_GROWTH
	3, // DELAY_MORTALITY
	6, // DELAY_GROWTH
	100, // TOWN_DEVELOPMENT
	0, //TACTICS_NEEDS_WAR
	1, // ALLIES_NOAID
	1, // HEXSIDE_TERRAIN
	1, // ARCADIA_MAGIC
	0, // EARTHSEA_VICTORY
	1, // REAL_EXPERIENCE
	0, // TESTGAME_ENABLED
	20, // POP_LEVEL
	1, // FORM_TEMPLATES
	GameDefs::SHOW_AS_EVENTS, // SUPPRESS_ERRORS
	1, // LATE_TAX
	80, // MAGE_UNDEAD_INVINCIBLE
	1, // CANT_TEACH_MAGIC
	1, // ARCADIAN_CHECKER
};

GameDefs *Globals = &g;

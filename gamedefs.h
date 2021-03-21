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

extern char const **DirectionStrs;
extern char const **DirectionAbrs;

extern char const **MonthNames;

extern char const **SeasonNames;

extern int *allowedMages;
extern int allowedMagesSize;
extern int *allowedApprentices;
extern int allowedApprenticesSize;
extern int *allowedTaxes;
extern int allowedTaxesSize;
extern int *allowedTrades;
extern int allowedTradesSize;
extern int *allowedQuartermasters;
extern int allowedQuartermastersSize;
extern int *allowedTacticians;
extern int allowedTacticiansSize;

extern int NUMBATTLEITEMS;
extern int NUMARMORS;
extern int NUMWEAPONS;
extern int NUMMOUNTS;
extern int NUMMONSTERS;
extern int NUMMAN;

enum BattleLogLevel {
	NORMAL   = 0,	// Standard battle log
	DETAILED = 1,	// Will include detailed statistics after the battle
	VERBOSE  = 2	// Will include additionaly statistics for each battle round
};

class GameDefs {
public:
	char const *RULESET_NAME;
	ATL_VER RULESET_VERSION;

	int MAX_SPEED;
	int PHASED_MOVE_OFFSET;
	// Maximum speed boost for fleets from various sources
	int FLEET_WIND_BOOST; // Mages with SWIN
	int FLEET_CREW_BOOST; // Extra crew
	int FLEET_LOAD_BOOST; // Low load

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

	int START_MONEY; // standard is 5020
	int WORK_FRACTION; // standard is 3
	int ENTERTAIN_FRACTION; // standard is 20
	int ENTERTAIN_INCOME; // standard is 20

	// base tax income for ANY eligible taxer
	// AND for pillaging
	int TAX_BASE_INCOME;
	// bonus for useable weapons
	int TAX_BONUS_WEAPON;
	// bonus for armor (only in conjunction with a weapon)
	int TAX_BONUS_ARMOR;
	// bonus for manning a fortification
	int TAX_BONUS_FORT;
	
	// Options to control who is able to tax
	enum {
		TAX_ANYONE = 0x00001,
		TAX_COMBAT_SKILL = 0x00002,
		TAX_BOW_SKILL = 0x00004,
		TAX_RIDING_SKILL = 0x00008,
		TAX_USABLE_WEAPON = 0x00010, // no requirement or has required skill
		TAX_ANY_WEAPON = 0x00020, // Pay up lest I beat thee with my longbow!
		TAX_HORSE = 0x00040, // someone who HAS a horse, not the horse itself
		// Usually a weapon that requires no skill, plus COMB, but also
		// LANC (which requires RIDI) and RIDI
		TAX_MELEE_WEAPON_AND_MATCHING_SKILL = 0x00080,
		TAX_BOW_SKILL_AND_MATCHING_WEAPON = 0x00100,
		TAX_HORSE_AND_RIDING_SKILL = 0x00200,
		// Probably more petty theft than tax, but then there are those
		// who argue that taxation IS theft ;->
		TAX_STEALTH_SKILL = 0x00400,
		// Should mages be able to tax?  I'd give my tax to someone who
		// was aiming the black wind at me...
		TAX_MAGE_DAMAGE = 0x00800,
		TAX_MAGE_FEAR = 0x01000, // or able to flatten my barn?
		TAX_MAGE_OTHER = 0x02000, // or able to create magical armour?
		TAX_ANY_MAGE = 0x04000, // or who just has an impressive pointy hat?
		// Wasn't sure whether mages should be judged on whether they
		// know the spell, or their current combat spell (he's ordered
		// to cast shield in combat, but still knows how to fireball
		// unruly peasants).  Setting this uses only the current
		// spell set with the COMBAT order.
		TAX_MAGE_COMBAT_SPELL = 0x08000,
		TAX_BATTLE_ITEM = 0x10000, // Has a staff of lightning?
		TAX_USABLE_BATTLE_ITEM = 0x20000,
		TAX_CREATURES = 0x40000, // Do magical critters help to tax?
		TAX_ILLUSIONS = 0x80000, // What if they're not really there?

		// Abbreviation for "the usual"
		TAX_NORMAL = TAX_COMBAT_SKILL | TAX_USABLE_WEAPON,
	};
	int WHO_CAN_TAX;
	// Are taxing and pillaging month-long actions?
	int TAX_PILLAGE_MONTH_LONG;

	int HEALS_PER_MAN;

	int GUARD_REGEN; /* percent */
	int CITY_GUARD;
	int GUARD_MONEY;
	int CITY_POP; /* standard Atlantis is 20000 */

	int WMON_FREQUENCY;
	int LAIR_FREQUENCY;

	int FACTION_POINTS;

	int TIMES_REWARD;

	int TOWNS_EXIST;
	int LEADERS_EXIST;
	int SKILL_LIMIT_NONLEADERS;
	int MAGE_NONLEADERS;
	int RACES_EXIST;
	int GATES_EXIST;
	int FOOD_ITEMS_EXIST;
	int COASTAL_FISH;
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

	// Do starting cities exist?
	int START_CITIES_EXIST;

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
	// And what are they called?
	char const *APPRENTICE_NAME;

	// What is the name of the world?
	char const *WORLD_NAME;

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

	// Are we allowing a multi-hex nexus
	int MULTI_HEX_NEXUS;

	// Is this world icosahedral (round), as opposed to the normal flat?
	int ICOSAHEDRAL_WORLD;

	// How many levels of the underworld do we want?
	int UNDERWORLD_LEVELS;

	// How many levels of the underdeep do we want?
	int UNDERDEEP_LEVELS;

	// Is there an abyss level?
	int ABYSS_LEVEL;

	// Town probability; 100 = default
	int TOWN_PROBABILITY;

	// Raising this value will lessen the effects
	// of a region type's economy on the chance
	// for creating towns. At 100% chances will
	// be exactly equal for all types of regions,
	// at 0 the chance will be directly proportional
	// to the economy value.
	int TOWN_SPREAD;

	// Reduction (in %) of the chance for towns forming adjacent
	// to each other (0 = normal chance, 100 = no chance of towns
	// forming adjacent to each other)
	int TOWNS_NOT_ADJACENT;

	// Make settlements near the arctic less likely and smaller
	// Higher values decrease likeliness of settlements
	// suggested: 0-5
	int LESS_ARCTIC_TOWNS;
	
	// Percent of surface level covered with ocean.
	int OCEAN;
	
	// Size factor for continent creation.
	int CONTINENT_SIZE;
	
	// Granularity of terrain - default setting is 0.
	// A value of 1 means maximum variability,
	// higher values = more regular terrain spread
	int TERRAIN_GRANULARITY;
	
	// Relative frequency of LAKE terrain
	int LAKES;

	// Enable Archipelago Creation
	// = chance of creating archipelagos vs continents
	// (note that archipelagos are smaller so that
	//  the overall contribution of archipelagos to
	//  land mass will be much lower than this percentage)
	// suggested value: 10, 25, 50+ it's really a matter of taste
	int ARCHIPELAGO;
	
	// Reduce/Sever Land Bridges: this is the base chance
	// that thin strips of land will be 'cut'. This chance is
	// doubled when a region has exactly four adjacent sea
	// areas. (default is 30, 0 means no removal)
	int SEVER_LAND_BRIDGES;

	// Maximum Size of inland seas to fill in. WARNING: game
	// creation time will rise exponentially with this value.
	// Recommended: 8-12
	int SEA_LIMIT;

	// Lake Effect on Wages Options
	// Lakes will add one to adjacent regions wages if set
	enum {
		NO_EFFECT = 0x00,
		ALL = 0x01,
		TOWNS = 0x02,
		NONPLAINS = 0x04,
		NONPLAINS_TOWNS_ONLY=0x08,
		DESERT_ONLY = 0x10,
	};

	// LAKE_WAGE_EFFECT: effect on surrounding wages
	int LAKE_WAGE_EFFECT;

	// LAKESIDE_IS_COASTAL: lakeside regions count as
	// coastal for all purposes - races and such
	int LAKESIDE_IS_COASTAL;

	// ODD_TERRAIN: chance (x 0.1%) for single-hex terrain oddities
	// suggested: between 5 and 40
	int ODD_TERRAIN;

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
		REPORT_SHOW_UNITS = (REPORT_SHOW_GUARDS |
						REPORT_SHOW_INDOOR_UNITS |
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
	// If prepare is strict, then the READY command MUST be used
	// and there will be no automatic selection of battle items.
	enum {
		PREPARE_NONE = 0,
		PREPARE_NORMAL = 1,
		PREPARE_STRICT = 2,
	};
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

	// Set this to 1 if your scripts can handle the following commands
	// #create, #resend, #times, #rumor, #remind, #email
	int HAVE_EMAIL_SPECIAL_COMMANDS;

	// Should starting cities begin with unlimited markets at world
	// generation time.
	int START_CITIES_START_UNLIMITED;

	// If this is enabled, then when a unit has more men than Amulets of
	// True Seeing, and it is targetted by an assassin, then there is a
	// chance that the target picked will be one of the men carrying
	// the amulet and the attempt will be foiled.  If not enabled, the
	// attempt always succeeds.
	int PROPORTIONAL_AMTS_USAGE;

	// If this is enabled, then the ARMOR and WEAPON commands for selecting
	// armor/weapon priorities for a unit are enabled. If a preferred weapon
	// or armor isn't available, then the code will fall back to the internal
	// list.
	int USE_WEAPON_ARMOR_COMMAND;

	// Set MONSTER_NO_SPOILS > 0 to disable spoils from released monsters
	// for that many months.
	int MONSTER_NO_SPOILS;
	// Set MONSTER_SPOILS_RECOVERY > 0 to set a time in months over which
	// monster spoils are slowly regained.
	// This has no effect unles MONSTER_NO_SPOILS is also set.
	int MONSTER_SPOILS_RECOVERY;

	// Use this to limit the number of attacks an assassin gets in the free
	// round.
	int MAX_ASSASSIN_FREE_ATTACKS;

	// Set to 1 to allow 'GIVE 0 <num> <monster>' for summoned monsters
	// (dragons, wolves, eagles currently)
	int RELEASE_MONSTERS;

	// Set to 1 to check for demons breaking free and undead decaying before
	// movemeent rather than at the end of the turn.  This prevents 'balrog
	// missiles'
	int CHECK_MONSTER_CONTROL_MID_TURN;

	// Set to 1 if CAST GATE DETECT shows the actual gate numbers.
	int DETECT_GATE_NUMBERS;

	// Should army routes be basd on the number of hits lost rather than the
	// number of figures lost.
	enum {
		// Default -- rout if half of the total figures die.  All figures are
		// treated equally.
		ARMY_ROUT_FIGURES = 0,
		// Rout if half the total hits die, all hits are counted independantly
		// This means you could rout even if none of your men are dead but
		// you have just taken a lot of hits and are getting clobbered
		ARMY_ROUT_HITS_INDIVIDUAL,
		// Rout if half of the total hits die, but figures hits only stop
		// counting toward the total when the figure is fully dead.
		ARMY_ROUT_HITS_FIGURE,
	};

	int ARMY_ROUT;
	int ONLY_ROUT_ONCE;
	
	// How are fortification bonuses handled?
	// Setting this flag will enable fort bonuses to be treated as a
	// modifier to the attack chance rather than a flat bonus to the
	// defence skill. This will make forts much better, especially
	// against skilled/well-equipped attackers.
	// Chance will depend on the fort protection level:
	// 1 : 25%, 2: 25%, 3: 20%, 4: 20%, 5: 16% and so on.
	int ADVANCED_FORTS;

	// If this is set to 0 then a mage gets his TRUE_SEEING skill/2
	// rounded up as a bonus to observation.  Lacandon (and perhaps others)
	// should set this to 1.
	int FULL_TRUESEEING_BONUS;

	// If this is set to 1 then an AMTS will give 3 bonus to OBSE rather
	// than 2. This was added for Lacandon
	int IMPROVED_AMTS;

	// If this is set to 1 then a mage automatically gets his INVISIBILITY
	// skill added to his stealth.  I only recommend setting this if you
	// also set the FULL_TRUESEEING_BONUS above.  This was added for
	// Lacandon.
	int FULL_INVIS_ON_SELF;

	// Do monsters regenerate during battle?
	int MONSTER_BATTLE_REGEN;

	// Amount of skill improvement when a skill is used
	int SKILL_PRACTICE_AMOUNT;
	
	// Experience mandatory: use lower rates of study per
	// this amount of experience less than current days of
	// study. Zero disables experience system. (50 is a
	// recommended value for enabling this feature)
	int REQUIRED_EXPERIENCE;

	// Options on using food for upkeep
	// Note that all these values are in silver equivalents!
	int UPKEEP_MINIMUM_FOOD;
	// MAXIMUM_FOOD is the maximum contribution of food to upkeep
	// per man, and therefore CAN be lower than MINIMUM_FOOD
	// (although this seems a silly thing to do).
	// A negative value for this def indicates no maximum.
	int UPKEEP_MAXIMUM_FOOD;
	int UPKEEP_FOOD_VALUE;

	// Should ships be prevented from sailing through single hex landmasses
	// during the same turn. (does not prevent them from stopping in a
	// single hex one turn and sailing on through the next turn.
	int PREVENT_SAIL_THROUGH;

	// If we are preventing sail through, should we also prevent the 'easy
	// portage' that the above allows by default?
	int ALLOW_TRIVIAL_PORTAGE;
	
	// Non-Perennial Gates: setting this between 1 and 11 will cause gates
	// to open and close. Gates will be open this number of months per year.
	// Each gate will have a different period of opening (although they are
	// somewhat related to gate index). A gate will not be detectable or
	// useable while closed; EXCEPTION: a closed gate may be the TARGET for
	// a random jump (stranding the jumper until the gate opens). Nexus
	// gates (and optionally, starting city gates) are open all the time.
	int GATES_NOT_PERENNIAL;
	
	// Are gates in starting cities open all year round? Need only be set
	// if GATES_NOT_PERENNIAL is selected.
	int START_GATES_OPEN;

	// Should a message appear in the report that a closed gate exists when
	// a that is closed would normally be detected (this might affect some
	// clients with unflexible parsers badly...)
	int SHOW_CLOSED_GATES;

	// Transport and related settings
	enum {
		ALLOW_TRANSPORT = 0x01, // Do we allow transport/distribute?
		// actual cost will be * (4 - (level+1)/2), which means that 
		// the cost will be from *1 (at level 5) to *3 (at level 1)
		QM_AFFECT_COST = 0x02, // QM level affect shipping cost?
		// actual distance will be NONLOCAL_TRANSPORT + ((level + 1)/3)
		QM_AFFECT_DIST = 0x04, // QM level affect longrange dist?
	};
	int TRANSPORT;

	// Base hexes for local transport and distribute (0 to disable)
	int LOCAL_TRANSPORT;

	// Base hexes for long range transport and distribute (-1 to disable)
	// (0 to allow instant transport between structures anywhere in world)
	int NONLOCAL_TRANSPORT;

	// Base cost per weight unit to ship items.
	int SHIPPING_COST;

	// Will 0 weight items have a fractional weight so that large
	// quantities actually have some heft?
	// Leave 0 for no fractional weight.  Otherwise, X items weigh 1.
	int FRACTIONAL_WEIGHT;

	// Use grow algorithm for races?
	int GROW_RACES;

	// Enable Creation of Towns mid-game, migration and population dynamics?
	// NOTE: not implemented yet.
	int DYNAMIC_POPULATION;

	// Population growth rate modifier (100 = standard).
	int POP_GROWTH;

	// Mortality delay - in months. Minimum = 2 months. Suggested are THREE
	// months
	int DELAY_MORTALITY;

	// Growth effects delay - in months. Minimum = 2 months. Suggested are
	// SIX months.
	int DELAY_GROWTH;

	// How developed pre-existing towns are at the start of the game
	// (100 = standard).
	int TOWN_DEVELOPMENT;
	
	// Whether you need to be a war faction to have tact-5 leaders
	int TACTICS_NEEDS_WAR;
	
	// Whether allies will be affected by the noaid flag
	int ALLIES_NOAID;

	// Whether assassination has a stealth penalty of -2
	// and stealing a stealth penalty of -1
	int HARDER_ASSASSINATION;

	// Turn on to make gate numbers non-contiguous, and spread
	// over a wider range.  This option will also turn off reporting
	// of the total number of gates in the game
	int DISPERSE_GATE_NUMBERS;

	// Chance of men killed by undead coming back from the grave
	// as undead themselves
	int UNDEATH_CONTAGION;

	// Like DYNAMIC_POPULATION but without migration
	// Regions population changes depends on production in region, settlements grow
	// Enables basic economy
	int REGIONS_ECONOMY;

	// Replaces free round by combat bonus for first rounds
	int ADVANCED_TACTICS;

	// Combat overwhelming.
	//
	// It limits the BEHIND flag as too few front line troops won' t be able to defend the combatants
	// in the back line if the attacker is remarkably larger. There will be a message in the battle
	// report teeling you if one side is overwhelmed. After this has happened all units of the
	// overwhelmed side fight as if they were in the front line.
	//
	// You need as many times the number of front line troops to overwhelm an enemy as you have set
	// in OVERWHELMING varaible. Setting 0 effectively turns this featur off, but settining it to 2
	// will mean that one army front line must be 2X larger than other army front line to achieve
	// overwhelming.
	// 
	// This feature uses ARMY_ROUT setting to determine how size of the army is determined, by hits
	// or by figure count.
	int OVERWHELMING;

	// Prevents non-ally factions to set guard on guarded regions.
	int STRICT_GUARD;

	// Allows guarding in ocean
	int OCEAN_GUARD;

	// Controls how detailed battle logs are, see BattleLogLevel for more details
	BattleLogLevel BATTLE_LOG_LEVEL;

	// Extends fortifications defenses into adjacent regions
	int EXTENDED_FORT_DEFENCE;

	// Remove trade items from spoils
	int SPOILS_NO_TRADE;

	// Write into time world events like battles, city captures, etc.
	// THIS IS BOOL, NOT INT
	// Use 0 - when you don't want any world events in the times
	// Use 1 - when you want them
	int WORLD_EVENTS;
};

extern GameDefs *Globals;

#endif

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
// 2000/MAR/16 Larry Stanbery    Revised description for runesword, to give
//                               it FEAR 3 special ability.
//                               Added new items for Realms of the Arcane.
// 2000/MAR/21 Azthar Septragen  Added roads.
// 2000/MAR/25 Larry Stanbery    Corrected a few comments
// 2000/SEP/06 Joseph Traub      Added base man cost to allow races to have
//                               different base costs
// 2001/FEB/01 Joseph Traub      Added options for flying over water and
//                               easier underworld viewing and farseeing
// 2001/FEB/07 Joseph Traub      Added option to make starting cities safe
//                               or not and to control the guard numbers
//                               and to make them slightly tougher.
//                               Added option to give starting city guards
//                               mage support.

#include "rules.h"
#include "items.h"
#include "skills.h"
#include "object.h"
#include "aregion.h"

//
// Define the various globals for this game.
//
static GameDefs g = {
    "Standard Atlantis",     // RULESET_NAME
    MAKE_ATL_VER( 4, 0, 4 ), // RULESET_VERSION
    MAKE_ATL_VER( 4, 0, 4 ), // ENGINE_VERSION

    2, /* FOOT_SPEED */
    4, /* HORSE_SPEED */
    4, /* SHIP_SPEED */
    6, /* FLY_SPEED */
    8, /* MAX_SPEED */
		   
    250, /* WAGON_CAPACITY */

    10, /* STUDENTS_PER_TEACHER */
    10, /* MAINTENANCE_COST */
    20, /* LEADER_COST */
    33, /* STARVE_PERCENT */
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

    1, // TOWNS_EXIST
    1, // LEADERS_EXIST
    1, // SKILL_LIMIT_NONLEADERS
    0, // MAGE_NONLEADERS
    1, // RACES_EXIST
    1, // GATES_EXIST
    1, // FOOD_ITEMS_EXIST
    1, // CITY_MONSTERS_EXIST
    1, // WANDERING_MONSTERS_EXIST
    1, // LAIR_MONSTERS_EXIST
    1, // WEATHER_EXISTS
    1, // OPEN_ENDED

    1, // RANDOM_ECONOMY
    1, // VARIABLE_ECONOMY

    50, // CITY_MARKET_NORMAL_AMT
    20, // CITY_MARKET_ADVANCED_AMT
    50, // CITY_MARKET_TRADE_AMT

	50,	// BASE_MAN_COST
	10, // MAX_INACTIVE_TURNS

	// *NOTE* If this is set to 1, you need to make sure the correct
	// section in skillshows.cpp is defined to describe the X,Y,Z
	// coordinates.  If you set it to zero, make sure the original
	// skill description is enabled instead
	0, // EASIER_UNDERWORLD

	0, // DEFAULT_WORK_ORDER

    GameDefs::FACLIM_FACTION_TYPES, // FACTION_LIMIT_TYPE

	GameDefs::WFLIGHT_NONE,	// FLIGHT_OVER_WATER

	1,   // SAFE_START_CITIES
	120, // AMT_START_CITY_GUARDS
	0,   // START_CITY_GUARDS_PLATE
	0,   // START_CITY_MAGES
};

GameDefs * Globals = &g;

//
// Table of items
//
// name, plural,abbr, flags, skill, level, input, number, weight, type,
// base price, combat, monster, walk,ride,fly,swim, mult_item, mult_val
//
ItemType id[] =
{
    {"leader","leaders","LEAD",0,-1,0,-1,0,10,IT_MAN,
     100,1,MAN_LEADER,15,0,0,0,-1,0},
    {"viking","vikings","VIKI",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_VIKING,15,0,0,0,-1,0},
    {"barbarian","barbarians","BARB",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_BARBARIAN,15,0,0,0,-1,0},
    {"plainsman","plainsmen","PLAI",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_PLAINSMAN,15,0,0,0,-1,0},
    {"eskimo","eskimos","ESKI",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_ESKIMO,15,0,0,0,-1,0},
    {"nomad","nomads","NOMA",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_NOMAD,15,0,0,0,-1,0},
    {"tribesman","tribesmen","TMAN",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_TRIBESMAN,15,0,0,0,-1,0},
    {"darkman","darkmen","DMAN",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_DARKMAN,15,0,0,0,-1,0},
    {"wood elf","wood elves","WELF",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_WOODELF,15,0,0,0,-1,0},
    {"sea elf","sea elves","SELF",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_SEAELF,15,0,0,0,-1,0},
    {"high elf","high elves","HELF",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_HIGHELF,15,0,0,0,-1,0},
    {"tribal elf","tribal elves","TELF",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_TRIBALELF,15,0,0,0,-1,0},
    {"ice dwarf","ice dwarves","IDWA",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_ICEDWARF,15,0,0,0,-1,0},
    {"hill dwarf","hill dwarves","HDWA",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_HILLDWARF,15,0,0,0,-1,0},
    {"under dwarf","under dwarves","UDWA",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_UNDERDWARF,15,0,0,0,-1,0},
    {"desert dwarf","desert dwarves","DDWA",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_DESERTDWARF,15,0,0,0,-1,0},
    {"orc","orcs","ORC",0,-1,0,-1,0,10,IT_MAN,
     50,1,MAN_ORC,15,0,0,0,-1,0},
    {"silver","silver","SILV",0,-1,0,-1,0,0,IT_NORMAL,
     1,0,0,0,0,0,0,-1,0},
    {"grain","grain","GRAI",0,S_FARMING,1,-1,0,5,IT_NORMAL,
     15,0,0,0,0,0,0,I_BAG,2},
    {"livestock","livestock","LIVE",0,S_RANCHING,1,-1,0,50,IT_NORMAL,
     15,0,0,50,0,0,0,I_LASSO,1},
    {"iron","iron","IRON",0,S_MINING,1,-1,0,5,IT_NORMAL,
     30,0,0,0,0,0,0,I_PICK,1},
    {"wood","wood","WOOD",0,S_LUMBERJACK,1,-1,0,5,IT_NORMAL,
     30,0,0,0,0,0,0,I_AXE,1},
    {"stone","stone","STON",0,S_QUARRYING,1,-1,0,50,IT_NORMAL,
     30,0,0,0,0,0,0,I_PICK,1},
    {"fur","furs","FUR",0,S_HUNTING,1,-1,0,1,IT_NORMAL,
     30,0,0,0,0,0,0,I_SPEAR,1},
    {"fish","fish","FISH",0,S_FISHING,1,-1,0,1,IT_NORMAL,
     15,0,0,0,0,0,0,I_NET,2},
    {"herb","herbs","HERB",0,S_HERBLORE,1,-1,0,0,IT_NORMAL,
     30,0,0,0,0,0,0,I_BAG,2},
    {"horse","horses","HORS",0,S_HORSETRAINING,1,-1,0,50,
     IT_NORMAL | IT_MOUNT,
     30,1,MOUNT_HORSE,70,70,0,0,I_LASSO,1},
    {"sword","swords","SWOR",0,S_WEAPONSMITH,1,I_IRON,1,1,
     IT_NORMAL | IT_WEAPON,
     60,1,WEAPON_SWORD,0,0,0,0,I_HAMMER,1},
    {"crossbow","crossbows","XBOW",0,S_WEAPONSMITH,1,I_WOOD,1,1,
     IT_NORMAL | IT_WEAPON,
     60,1,WEAPON_CROSSBOW,0,0,0,0,I_AXE,1},
    {"longbow","longbows","LBOW",0,S_WEAPONSMITH,1,I_WOOD,1,1,
     IT_NORMAL | IT_WEAPON,
     60,1,WEAPON_LONGBOW,0,0,0,0,I_AXE,1},
    {"chain armor","chain armor","CARM",0,S_ARMORER,1,I_IRON,1,1,
     IT_NORMAL | IT_ARMOR,
     60,1,ARMOR_CHAINARMOR,0,0,0,0,I_HAMMER,1},
    {"plate armor","plate armor","PARM",0,S_ARMORER,3,I_IRON,3,3,
     IT_NORMAL | IT_ARMOR,
     250,1,ARMOR_PLATEARMOR,0,0,0,0,I_HAMMER,1},
    {"wagon","wagons","WAGO",0,
     S_CARPENTER,1,I_WOOD,1,50,IT_NORMAL,
     100,0,0,0,0,0,0,I_AXE,1},
    {"mithril","mithril","MITH",0,
     S_MINING,3,-1,0,10,IT_ADVANCED,
     100,0,0,0,0,0,0,I_PICK,1},
    {"ironwood","ironwood","IRWD",0,
     S_LUMBERJACK,3,-1,0,10,IT_ADVANCED,
     100,0,0,0,0,0,0,I_AXE,1},
    {"winged horse","winged horses","WING",0,
     S_HORSETRAINING,5,-1,0,50,
     IT_ADVANCED | IT_MOUNT,
     100,1,MOUNT_WHORSE,70,70,70,0,I_LASSO,1},
    {"floater hide","floater hides","FLOA",0,
     S_HUNTING,3,-1,0,1,IT_ADVANCED,
     100,0,0,0,0,0,0,I_SPEAR,1},
    {"rootstone","rootstone","ROOT",0,
     S_QUARRYING,3,-1,0,50,IT_ADVANCED,
     100,0,0,0,0,0,0,I_PICK,1},
    {"yew","yew","YEW",0,S_LUMBERJACK,
     5,-1,0,5,IT_ADVANCED,
     100,0,0,0,0,0,0,I_AXE,1},
    {"mithril sword","mithril swords","MSWO",0,
     S_WEAPONSMITH,3,I_MITHRIL,1,1,IT_ADVANCED | IT_WEAPON,
     200,1,WEAPON_MSWORD,0,0,0,0,I_HAMMER,1},
    {"mithril armor","mithril armor","MARM",0,
     S_ARMORER,5,I_MITHRIL,1,1,IT_ADVANCED | IT_ARMOR,
     500,1,ARMOR_MARMOR,0,0,0,0,I_HAMMER,1},
    {"double bow","double bows","DBOW",0,
     S_WEAPONSMITH,5,I_YEW,1,1,IT_ADVANCED | IT_WEAPON,
     200,1,WEAPON_DOUBLEBOW,0,0,0,0,I_AXE,1},
    {"ivory","ivory","IVOR",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"pearls","pearls","PEAR",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"jewelry","jewelry","JEWE",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"figurines","figurines","FIGU",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"tarot cards","tarot cards","TARO",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"caviar","caviar","CAVI",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"wine","wine","WINE",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"spices","spices","SPIC",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"chocolate","chocolate","CHOC",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"truffles","truffles","TRUF",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"vodka","vodka","VODK",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"roses","roses","ROSE",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"perfume","perfume","PERF",0,
     -1,0,-1,0,1,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"silk","silk","SILK",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"velvet","velvet","VELV",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"mink","mink","MINK",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"cashmere","cashmere","CASH",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"cotton","cotton","COTT",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"dye","dye","DYE",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"wool","wool","WOOL",0,
     -1,0,-1,0,5,IT_TRADE,
     60,0,0,0,0,0,0,-1,0},
    {"lion","lions","LION",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_LION,15,0,0,0,-1,0},
    {"wolf","wolves","WOLF",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_WOLF,15,15,0,0,-1,0},
    {"grizzly bear","grizzly bears","GRIZ",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_GBEAR,60,0,0,0,-1,0},
    {"crocodile","crocodiles","CROC",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_CROCODILE,15,0,0,0,-1,0},
    {"anaconda","anacondas","ANAC",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_ANACONDA,60,0,0,0,-1,0},
    {"giant scorpion","giant scorpions","SCOR",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_SCORPION,15,0,0,0,-1,0},
    {"polar bear","polar bears","POLA",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_PBEAR,60,0,0,0,-1,0},
    {"giant rat","giant rats","GRAT",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_GRAT,15,0,0,0,-1,0},
    {"giant spider","giant spiders","GSPI",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_SPIDER,60,0,0,0,-1,0},
    {"giant lizard","giant lizards","GLIZ",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_LIZARD,60,0,0,0,-1,0},
    {"trent","trents","TREN",ItemType::CANTGIVE,
     -1,0,-1,0,250,IT_MONSTER,
     50,1,MONSTER_TRENT,300,0,0,0,-1,0},
    {"roc","rocs","ROC",ItemType::CANTGIVE,
     -1,0,-1,0,250,IT_MONSTER,
     50,1,MONSTER_ROC,300,300,300,0,-1,0},
    {"bog thing","bog things","BOGT",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_BTHING,60,0,0,0,-1,0},
    {"kong","kongs","KONG",ItemType::CANTGIVE,
     -1,0,-1,0,250,IT_MONSTER,
     50,1,MONSTER_KONG,300,0,0,0,-1,0},
    {"sphinx","sphinxes","SPHI",ItemType::CANTGIVE,
     -1,0,-1,0,250,IT_MONSTER,
     50,1,MONSTER_SPHINX,300,300,0,0,-1,0},
    {"ice wurm","ice wurms","ICEW",ItemType::CANTGIVE,
     -1,0,-1,0,250,IT_MONSTER,
     50,1,MONSTER_IWURM,300,0,0,0,-1,0},
    {"dragon","dragons","DRAG",ItemType::CANTGIVE,
     -1,0,-1,0,250,IT_MONSTER,
     50,1,MONSTER_DRAGON,300,300,300,0,-1,0},
    {"centaur","centaurs","CENT",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_CENTAUR,60,60,0,0,-1,0},
    {"kobold","kobolds","KOBO",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_KOBOLD,15,0,0,0,-1,0},
    {"ogre","ogres","OGRE",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_OGRE,60,0,0,0,-1,0},
    {"lizard man","lizard men","LMAN",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_LMAN,15,0,0,0,-1,0},
    {"wild man","wild men","WMAN",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_WMAN,15,0,0,0,-1,0},
    {"sandling","sandlings","SAND",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_SANDLING,15,0,0,0,-1,0},
    {"yeti","yeti","YETI",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_YETI,60,0,0,0,-1,0},
    {"goblin","goblins","GOBL",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_GOBLIN,15,0,0,0,-1,0},
    {"troll","trolls","TROL",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_TROLL,60,0,0,0,-1,0},
    {"ettin","ettins","ETTI",ItemType::CANTGIVE,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_ETTIN,60,0,0,0,-1,0},
    {"skeleton","skeletons","SKEL",0,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_SKELETON,15,0,0,0,-1,0},
    {"undead","undead","UNDE",0,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_UNDEAD,15,0,0,0,-1,0},
    {"lich","liches","LICH",0,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_LICH,15,0,0,0,-1,0},
    {"imp","imps","IMP",0,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_IMP,15,0,0,0,-1,0},
    {"demon","demons","DEMO",0,
     -1,0,-1,0,50,IT_MONSTER,
     50,1,MONSTER_DEMON,60,60,0,0,-1,0},
    {"balrog","balrogs","BALR",0,
     -1,0,-1,0,250,IT_MONSTER,
     50,1,MONSTER_BALROG,300,300,300,0,-1,0},
    {"eagle","eagles","EAGL",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_EAGLE,15,15,15,0,-1,0},
    {"amulet of invulnerability","amulets of invulnerability","XXXX",0,
     -1,0,-1,0,0,IT_MAGIC | IT_BATTLE | IT_SPECIAL,
     1000000,1,BATTLE_AOFI,0,0,0,0,-1,0},
    {"ring of invisibility","rings of invisibility","RING",0,
     -1,0,-1,0,0,IT_MAGIC,
     4000,1,0,0,0,0,0,-1,0},
    {"cloak of invulnerability","cloaks of invulnerability","CLOA",0,
     -1,0,-1,0,0,IT_MAGIC | IT_ARMOR,
     8000,1,ARMOR_CLOAKOFI,0,0,0,0,-1,0},
    {"staff of fire","staves of fire","STAF",0,
     -1,0,-1,0,0,IT_MAGIC | IT_BATTLE,
     4000,1,BATTLE_STAFFOFF,0,0,0,0,-1,0},
    {"staff of lightning","staves of lightning","STAL",0,
     -1,0,-1,0,0,IT_MAGIC | IT_BATTLE,
     16000,1,BATTLE_STAFFOFL,0,0,0,0,-1,0},
    {"amulet of true seeing","amulets of true seeing","AMTS",0,
     -1,0,-1,0,0,IT_MAGIC,
     4000,0,0,0,0,0,0,-1,0},
    {"amulet of protection","amulets of protection","AMPR",0,
     -1,0,-1,0,0,IT_MAGIC | IT_BATTLE,
     1000,1,BATTLE_AMULETOFP,0,0,0,0,-1,0},
    {"runesword","runeswords","RUNE",0,
     -1,0,-1,0,1,IT_MAGIC | IT_WEAPON | IT_BATTLE,
     8000,1, WEAPON_RUNESWORD,0,0,0,0,-1,0},
    {"shieldstone","shieldstones","SHST",0,
     -1,0,-1,0,0,IT_MAGIC | IT_BATTLE, 
     4000,1,BATTLE_SHIELDSTONE,0,0,0,0,-1,0},
    {"magic carpet","magic carpets","CARP",0,
     -1,0,-1,0,0,IT_MAGIC,
     2000,0,0,15,15,15,0,-1,0},
    {"wolf","wolves","WOLF",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"eagle","eagles","EAGL",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"dragon","dragons","DRAG",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"skeleton","skeletons","SKEL",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"undead","undead","UNDE",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"lich","liches","LICH",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"imp","imps","IMP",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"demon","demons","DEMO",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"balrog","balrogs","BALR",ItemType::CANTGIVE,
     -1,0,-1,0,1,IT_MONSTER,
     1,1,MONSTER_ILLUSION,1,1,1,0,-1,0},
    {"portal","portals","PORT",0,
     -1,0,-1,0,1,IT_MAGIC | IT_SPECIAL,
     1000,0,0,0,0,0,0,-1,0},
    {"peasant","peasants","PEAS",0,
     -1,0,-1,0,10,IT_MAN,
     50,1,0,15,0,0,0,-1,0},
    {"pick","picks","PICK",0,S_WEAPONSMITH,1,I_IRON,
     1,1,IT_NORMAL | IT_WEAPON,60,1,WEAPON_PICK,
     0,0,0,0,I_HAMMER,1},
    {"spear","spears","SPEA",0,S_WEAPONSMITH,1,I_WOOD,
     1,1,IT_NORMAL | IT_WEAPON,60,1,WEAPON_SPEAR,
     0,0,0,0,I_AXE,1},
    {"axe","axes","AXE",0,S_WEAPONSMITH,1,I_WOOD,
     1,1,IT_NORMAL | IT_WEAPON,60,1,WEAPON_AXE,
     0,0,0,0,I_HAMMER,1},
    {"hammer","hammers","HAMM",0,S_WEAPONSMITH,1,I_IRON,
     1,1,IT_NORMAL | IT_WEAPON,60,1,WEAPON_HAMMER,
     0,0,0,0,I_HAMMER,1},
    {"magic crossbow","magic crossbows","MXBO",0,
     S_WEAPONSMITH,4,I_IRONWOOD,1,1,IT_ADVANCED | IT_WEAPON,
     200,1,WEAPON_MCROSSBOW,0,0,0,0,I_AXE,1},
    {"magic wagon","magic wagons","MWAG",0,
     S_CARPENTER,3,I_IRONWOOD,1,50,IT_ADVANCED,
     200,1,-1,250,250,0,0,I_AXE,1},
    {"glider","gliders","GLID",0,
     S_CARPENTER,5,I_FLOATER,2,5,IT_ADVANCED,
     400,1,-1,0,0,15,0,I_AXE,1},
    {"net","nets","NET",0,
     S_FISHING,1,I_HERBS,1,1,IT_NORMAL,
     60,1,-1,0,0,0,0,I_SPINNING,2},
    {"lasso","lassoes","LASS",0,
     S_HERBLORE,1,I_HERBS,1,1,IT_NORMAL,
     60,1,-1,0,0,0,0,I_SPINNING,2},
    {"bag","bag","BAG",0,
     S_HERBLORE,1,I_HERBS,1,1,IT_NORMAL,
     60,1,-1,0,0,0,0,-1,2},
    {"spinning wheel","spinning wheels","SPIN",0,
     S_CARPENTER,1,I_WOOD,1,1,IT_NORMAL,
     60,1,-1,0,0,0,0,I_AXE,1},
    {"leather armor","leather armor","LARM",0,
     S_ARMORER,1,I_FUR,1,1,IT_NORMAL | IT_ARMOR,
     45,1,ARMOR_LEATHERARMOR,0,0,0,0,I_SPINNING,2},
    {"cloth armor","cloth armor","CLAR",0,
     S_ARMORER,1,I_HERBS,1,1,IT_NORMAL | IT_ARMOR,
     40,1,ARMOR_CLOTHARMOR,0,0,0,0,I_SPINNING,2},
    {"boots of levitation","boots of levitation","BOOT",0,
     -1,0,-1,0,0,IT_MAGIC,
     1000,0,0,0,0,0,15,-1,1},
    {"pirates","pirates","PIRA",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_PIRATES,15,0,0,15,-1,0},
    {"kraken","kraken","KRAK",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_KRAKEN,300,0,0,300,-1,0},
    {"merfolk","merfolk","MERF",ItemType::CANTGIVE,
     -1,0,-1,0,10,IT_MONSTER,
     50,1,MONSTER_MERFOLK,0,0,0,15,-1,0},
    {"elemental","elementals","ELEM",ItemType::CANTGIVE,
     -1,0,-1,0,250,IT_MONSTER,
     50,1,MONSTER_ELEMENTAL,300,0,0,0,-1,0},
};

ItemType * ItemDefs = id;

//
// Table of men
//
ManType mt[] = {
    //
    // Special level, default, 1st skill, 2nd, 3rd, 4th
    //
  { 0,0,0,0,0,0 }, /*  MAN_NONE */
  { 5,5,-1,-1,-1,-1 }, /* MAN_LEADER */
  { 3,2,S_SHIPBUILDING,S_SAILING,S_LUMBERJACK,S_COMBAT }, /* MAN_VIKING */
  { 3,2,S_MINING,S_HUNTING,S_WEAPONSMITH,S_COMBAT }, /* MAN_BARBARIAN */
  { 3,2,S_HORSETRAINING,S_FARMING,S_CARPENTER,
    S_ENTERTAINMENT }, /* MAN_PLAINSMAN */
  { 3,2,S_HERBLORE,S_FISHING,S_HUNTING,S_HEALING }, /* MAN_ESKIMO */
  { 3,2,S_HORSETRAINING,S_RANCHING,S_CROSSBOW,-1 }, /* MAN_NOMAD */
  { 3,2,S_HERBLORE,S_HEALING,S_FARMING,S_LUMBERJACK }, /* MAN_TRIBESMAN */
  { 3,2,S_QUARRYING,S_BUILDING,S_MINING,S_ARMORER }, /* MAN_DARKMAN */
  { 3,2,S_LUMBERJACK,S_CARPENTER,S_LONGBOW,S_ENTERTAINMENT }, /* MAN_WOODELF */
  { 3,2,S_SHIPBUILDING,S_SAILING,S_FISHING,S_LONGBOW }, /* MAN_SEAELF */
  { 3,2,S_HEALING,S_FARMING,S_ENTERTAINMENT,
    S_HORSETRAINING }, /* MAN_HIGHELF */
  { 3,2,S_HERBLORE,S_HEALING,S_RANCHING,S_LONGBOW }, /* MAN_TRIBALELF */
  { 3,2,S_FISHING,S_BUILDING,S_CROSSBOW,S_SHIPBUILDING }, /* MAN_ICEDWARF */
  { 3,2,S_MINING,S_WEAPONSMITH,S_ARMORER,S_COMBAT }, /* MAN_HILLDWARF */
  { 3,2,S_MINING,S_QUARRYING,S_CROSSBOW,S_ARMORER }, /* MAN_UNDERDWARF */
  { 3,2,S_QUARRYING,S_BUILDING,S_ARMORER,S_CROSSBOW }, /* MAN_DESERTDWARF */
  { 4,1,S_COMBAT,-1,-1,-1 }, /* MAN_ORC */
};

ManType * ManDefs = mt;

//
// Table of monsters.
//
MonType md[] = {
    //
    // skill,hits,tactics,special,stealth,obs,
    // silver,spoiltype,hostile,number,name
    //
  {0,0,0,-1,0,0,
   0,-1,0,0,"None"}, /* none */
  {3,2,3,0,2,2,
   100,-1,10,3,"Pride of Lions"},
  {2,1,0,0,0,0,
   40,-1,10,10,"Wolf Pack"},
  {2,6,0,0,0,2,
   250,-1,10,3,"Grizzly Bears"},
  {3,1,1,0,1,1,
   50,-1,10,6,"Crocodiles"},
  {3,1,2,0,2,0,
   50,-1,10,5,"Anacondas"},
  {2,2,2,0,3,0,
   80,-1,10,8,"Giant Scorpions"},
  {2,6,0,0,0,2,
   250,-1,10,3,"Polar Bears"},
  {1,1,0,0,2,0,
   20,-1,10,30,"Pack of Rats"},
  {4,2,0,0,2,2,
   200,-1,10,4,"Giant Spiders"},
  {3,5,0,0,0,0,
   400,-1,25,3,"Giant Lizards"},
  {2,10,2,0,3,1,
   500,IT_ADVANCED,25,7,"Living Trees"},
  {4,15,4,0,2,4,
   1500,IT_ADVANCED,25,2,"Giant Birds"},
  {4,20,3,0,1,3,
   2000,IT_ADVANCED,25,2,"Swamp Creatures"},
  {4,25,0,0,2,2,
   2500,IT_ADVANCED,25,2,"Great Apes"},
  {4,50,0,0,0,4,
   5000,IT_ADVANCED,25,1,"Sphinx"},
  {3,6,2,0,4,1,
   500,IT_ADVANCED,25,8,"Ice Wurms"},
  {6,50,4,SPECIAL_FIREBALL,0,3,
   10000,IT_MAGIC,50,1,"Dragon"},
  {4,2,0,0,1,1,
   150,IT_NORMAL,10,8,"Tribe of Centaurs"},
  {2,1,0,0,1,0,
   40,IT_NORMAL,10,20,"Kobold Pack"},
  {3,10,0,0,0,0,
   600,IT_NORMAL,10,2,"Family of Ogres"},
  {3,1,0,0,0,1,
   50,IT_NORMAL,10,10,"Lizard Men"},
  {3,1,0,0,2,0,
   50,IT_NORMAL,10,10,"Clan of Wild Men"},
  {3,1,1,0,2,1,
   50,IT_NORMAL,10,10,"Sandlings"},
  {4,2,0,0,3,1,
   150,IT_NORMAL,10,5,"Yeti"},
  {2,1,0,0,0,0,
   40,IT_NORMAL,10,40,"Goblin Horde"},
  {3,4,1,0,0,2,
   250,IT_NORMAL,10,8,"Troll Pack"},
  {2,32,2,0,0,0,
   1200,IT_NORMAL,10,2,"Ettins"},
  {2,1,0,0,0,0,
   40,IT_NORMAL,10,100,"Skeletons"},
  {3,6,0,0,0,0,
   400,IT_ADVANCED,25,10,"Undead"},
  {4,40,3,SPECIAL_CAUSEFEAR,0,3,
   4000,IT_MAGIC,50,1,"Lich"},
  {3,1,0,0,3,0,
   50,IT_NORMAL,10,50,"Imp"},
  {4,10,0,0,0,0,
   1000,IT_ADVANCED,25,10,"Demon"},
  {6,200,5,SPECIAL_HELLFIRE,5,5,
   50000,IT_MAGIC,100,1,"Balrog"},
  {2,1,2,0,3,3,
   20,-1,10,1,"Eagle"},
  {3,1,1,0,1,1,
   400,IT_NORMAL,50,20,"Pirates"},
  {5,50,1,SPECIAL_CAUSEFEAR,1,2,
   1000,IT_MAGIC,50,1,"Kraken"},
  {2,1,2,0,2,3,
   1000,IT_ADVANCED,10,100,"Merfolk"},
  {2,10,2,0,3,1,
   500,IT_ADVANCED,25,7,"Living Water"},
  {-5,0,0,0,5,0,
   0,-1,0,0,"Illusion"}
/* skill,hits,tactics,special,stealth,obs,
   silver,spoiltype,hostile,number,name */
};

MonType * MonDefs = md;

//
// Table of weapons.
//
WeaponType wepd[] = {
    // WEAPON_NONE
    { -1, -1, 0, 0, 0 },
    // LLS
    // Rearranged priority, to make Runesword more important.
    // Added new items.
    // WEAPON_RUNESWORD
    { -1, -1, 0, 4, 1 },
    // WEAPON_DOUBLEBOW
    { S_LONGBOW, S_CROSSBOW,
      WeaponType::NEEDSKILL | WeaponType::RANGED |
      WeaponType::NODEFENSE | WeaponType::GOODARMOR, 
      0, WeaponType::NUM_ATTACKS_SKILL },
    // WEAPON_MCROSSBOW
    { S_CROSSBOW, -1,
      WeaponType::NEEDSKILL | WeaponType::RANGED | WeaponType::NODEFENSE |
      WeaponType::GOODARMOR,
      0, 1 },
    // WEAPON_LONGBOW
    { S_LONGBOW, -1,
      WeaponType::NEEDSKILL | WeaponType::RANGED | WeaponType::NODEFENSE,
      -2, 1 },
    // WEAPON_CROSSBOW
    { S_CROSSBOW, -1,
      WeaponType::NEEDSKILL | WeaponType::RANGED | WeaponType::NODEFENSE |
      WeaponType::GOODARMOR,
      0, -2 },
    // WEAPON_MSWORD
    { -1, -1, 0, 4, 1 },
    // WEAPON_SWORD
    { -1, -1, 0, 2, 1 },
    // WEAPON_PICK
    { -1, -1, 0, 1, 1 },
    // WEAPON_SPEAR
    { -1, -1, 0, 1, 1 },
    // WEAPON_AXE
    { -1, -1, 0, 1, 1 },
    // WEAPON_HAMMER
    { -1, -1, 0, 1, 1 },
};

WeaponType *WeaponDefs = wepd;

//
// Table of armors.
//
// LLS
// Added new items.
ArmorType armd[] = {
    // ARMOR_NONE
    { 0, 0, 0, 0, 0 },
    // ARMOR_CLOAKOFI
    { 0, 99, 100, 99, 100 },
    // ARMOR_MARMOR
    { 0, 9, 10, 2, 3 },
    // ARMOR_PLATEARMOR
    { 0, 2, 3, 0, 1 },
    // ARMOR_CHAINARMOR
    { 0, 1, 3, 0, 1 },
    // ARMOR_LEATHERARMOR
    { 0, 1, 4, 0, 1 },
    // ARMOR_CLOTHARMOR
    { ArmorType::USEINASS, 1, 6, 0, 1 },
};

ArmorType *ArmorDefs = armd;

//
// Table of mounts
//
MountType mountd[] = {
    // MOUNT_NONE
    { 0, 0, 0 },
    // MOUNT_WHORSE
    { S_RIDING, 3, 5 },
    // MOUNT_HORSE
    { S_RIDING, 1, 3 },
};

MountType *MountDefs = mountd;

//
// Table of other battle items
//
BattleItemType bitd[] = {
    // BATTLE_NONE
    { 0, 0, 0, 0 },
    // LLS
    // Made Runesword match order in WeaponDefs, so that its special
    // is mapped correctly.
    // BATTLE_RUNESWORD
    { BattleItemType::SPECIAL, I_RUNESWORD, SPECIAL_CAUSEFEAR, 3 },
    // BATTLE_STAFFOFL
    { BattleItemType::MAGEONLY | BattleItemType::SPECIAL,
      I_STAFFOFL, SPECIAL_LSTRIKE, 3 },
    // BATTLE_STAFFOFF
    { BattleItemType::MAGEONLY | BattleItemType::SPECIAL,
      I_STAFFOFF, SPECIAL_FIREBALL, 3 },
    // BATTLE_AOFI
    { BattleItemType::SHIELD,
      I_AMULETOFI, NUM_ATTACK_TYPES, 5 },
    // BATTLE_AMULETOFP
    { BattleItemType::SHIELD,
      I_AMULETOFP, ATTACK_SPIRIT, 3 },
    // BATTLE_SHIELDSTONE
    { BattleItemType::SHIELD,
      I_SHIELDSTONE, ATTACK_ENERGY, 3 },
};

BattleItemType *BattleItemDefs = bitd;

//
// Table of skills.
//
static SkillType sd[] = {
    //
    // name, abbr, cost, flags, special
    // depend1, level1, depend2, level2, depend3, level3
    //
    {"mining","MINI",10,0,0,-1,0,-1,0,-1,0},
    {"lumberjack","LUMB",10,0,0,-1,0,-1,0,-1,0},
    {"quarrying","QUAR",10,0,0,-1,0,-1,0,-1,0},
    {"hunting","HUNT",10,0,0,-1,0,-1,0,-1,0},
    {"fishing","FISH",10,0,0,-1,0,-1,0,-1,0},
    {"herb lore","HERB",10,0,0,-1,0,-1,0,-1,0},
    {"horse training","HORS",10,0,0,-1,0,-1,0,-1,0},
    {"weaponsmith","WEAP",10,0,0,-1,0,-1,0,-1,0},
    {"armorer","ARMO",10,0,0,-1,0,-1,0,-1,0},
    {"carpenter","CARP",10,0,0,-1,0,-1,0,-1,0},
    {"building","BUIL",10,0,0,-1,0,-1,0,-1,0},
    {"shipbuilding","SHIP",10,0,0,-1,0,-1,0,-1,0},
    {"entertainment","ENTE",10,0,0,-1,0,-1,0,-1,0},
    {"tactics","TACT",200,0,0,-1,0,-1,0,-1,0},
    {"combat","COMB",10,0,0,-1,0,-1,0,-1,0},
    {"riding","RIDI",10,0,0,-1,0,-1,0,-1,0},
    {"crossbow","XBOW",10,0,0,-1,0,-1,0,-1,0},
    {"longbow","LBOW",10,0,0,-1,0,-1,0,-1,0},
    {"stealth","STEA",50,0,0,-1,0,-1,0,-1,0},
    {"observation","OBSE",50,0,0,-1,0,-1,0,-1,0},
    {"healing","HEAL",10,0,0,-1,0,-1,0,-1,0},
    {"sailing","SAIL",10,0,0,-1,0,-1,0,-1,0},
    {"farming","FARM",10,0,0,-1,0,-1,0,-1,0},
    {"ranching","RANC",10,0,0,-1,0,-1,0,-1,0},
    {"force","FORC",100, SkillType::MAGIC | SkillType::FOUNDATION,
     0,-1,0,-1,0,-1,0},
    {"pattern","PATT",100, SkillType::MAGIC | SkillType::FOUNDATION,
     0,-1,0,-1,0,-1,0},
    {"spirit","SPIR",100, SkillType::MAGIC | SkillType::FOUNDATION,
     0,-1,0,-1,0,-1,0},
    {"fire","FIRE",100, SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_FIREBALL, S_FORCE,1,-1,0,-1,0},
    {"earthquake","EQUA",100, SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_EARTHQUAKE, S_FORCE,1,S_PATTERN,1,-1,0},
    {"force shield","FSHI",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_FORCE_SHIELD, S_FORCE,1,-1,0,-1,0},
    {"energy shield","ESHI",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_ENERGY_SHIELD, S_FORCE,1,-1,0,-1,0},
    {"spirit shield","SSHI",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_SPIRIT_SHIELD, S_SPIRIT,1,S_FORCE,1,-1,0},
    {"magical healing","MHEA",100,SkillType::MAGIC,
     0,S_PATTERN,1,-1,0,-1,0},
    {"gate lore","GATE",100, SkillType::MAGIC | SkillType::CAST,
     0,S_PATTERN,1,S_SPIRIT,1,-1,0},
    {"farsight","FARS",100,SkillType::MAGIC | SkillType::CAST,
     0,S_PATTERN,1,S_SPIRIT,1,-1,0},
    {"teleportation","TELE",100,SkillType::MAGIC | SkillType::CAST,
     0,S_GATE_LORE,1,S_FARSIGHT,3,-1,0},
    {"portal lore","PORT",100,SkillType::MAGIC | SkillType::CAST,
     0,S_GATE_LORE,3,S_FARSIGHT,1,-1,0},
    {"mind reading","MIND",100,SkillType::MAGIC | SkillType::CAST,
     0,S_PATTERN,1,-1,0,-1,0},
    {"weather lore","WEAT",100,SkillType::MAGIC,
     0,S_FORCE,1,S_PATTERN,1,-1,0},
    {"summon wind","SWIN",100,SkillType::MAGIC,
     0,S_WEATHER_LORE,1,-1,0,-1,0},
    {"summon storm","SSTO",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_SUMMON_STORM, S_WEATHER_LORE,1,-1,0,-1,0},
    {"summon tornado","STOR",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_TORNADO, S_WEATHER_LORE,3,-1,0,-1,0},
    {"call lightning","CALL",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_LSTRIKE, S_WEATHER_LORE,5,-1,0,-1,0},
    {"clear skies","CLEA",100,
     SkillType::MAGIC | SkillType::COMBAT | SkillType::CAST,
     SPECIAL_CLEAR_SKIES, S_WEATHER_LORE,1,-1,0,-1,0},
    {"earth lore","EART",100,SkillType::MAGIC | SkillType::CAST,
     0,S_PATTERN,1,-1,0,-1,0},
    {"wolf lore","WOLF",100,SkillType::MAGIC | SkillType::CAST,
     0,S_EARTH_LORE,1,-1,0,-1,0},
    {"bird lore","BIRD",100,SkillType::MAGIC | SkillType::CAST,
     0,S_EARTH_LORE,1,-1,0,-1,0},
    {"dragon lore","DRAG",100,SkillType::MAGIC | SkillType::CAST,
     0,S_BIRD_LORE,3,-1,0,-1,0},
    {"necromancy","NECR",100,SkillType::MAGIC,
     0,S_FORCE,1,S_SPIRIT,1,-1,0},
    {"summon skeletons","SUSK",100,SkillType::MAGIC | SkillType::CAST,
     0,S_NECROMANCY,1,-1,0,-1,0},
    {"raise undead","RAIS",100,SkillType::MAGIC | SkillType::CAST,
     0,S_NECROMANCY,3,-1,0,-1,0},
    {"summon lich","SULI",100,SkillType::MAGIC | SkillType::CAST,
     0,S_NECROMANCY,5,-1,0,-1,0},
    {"create aura of fear","FEAR",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_CAUSEFEAR, S_NECROMANCY,1,-1,0,-1,0},
    {"summon black wind","SBLA",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_BLACK_WIND, S_NECROMANCY,5,-1,0,-1,0},
    {"banish undead","BUND",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_BANISH_UNDEAD, S_NECROMANCY,1,-1,0,-1,0},
    {"demon lore","DEMO",100,SkillType::MAGIC,
     0,S_FORCE,1,S_SPIRIT,1,-1,0},
    {"summon imps","SUIM",100,SkillType::MAGIC | SkillType::CAST,
     0,S_DEMON_LORE,1,-1,0,-1,0},
    {"summon demon","SUDE",100,SkillType::MAGIC | SkillType::CAST,
     0,S_SUMMON_IMPS,3,-1,0,-1,0},
    {"summon balrog","SUBA",100,SkillType::MAGIC | SkillType::CAST,
     0,S_SUMMON_DEMON,3,-1,0,-1,0},
    {"banish demons","BDEM",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_BANISH_DEMONS, S_DEMON_LORE,1,-1,0,-1,0},
    {"illusion","ILLU",100,SkillType::MAGIC,
     0,S_FORCE,1,S_PATTERN,1,-1,0},
    {"phantasmal entertainment","PHEN",100,SkillType::MAGIC,
     0,S_ILLUSION,1,-1,0,-1,0},
    {"create phantasmal beasts","PHBE",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ILLUSION,1,-1,0,-1,0},
    {"create phantasmal undead","PHUN",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ILLUSION,1,-1,0,-1,0},
    {"create phantasmal demons","PHDE",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ILLUSION,1,-1,0,-1,0},
    {"invisibility","INVI",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ILLUSION,3,-1,0,-1,0},
    {"true seeing","TRUE",100,SkillType::MAGIC,
     0,S_ILLUSION,3,-1,0,-1,0},
    {"dispel illusions","DISP",100,SkillType::MAGIC | SkillType::COMBAT,
     SPECIAL_DISPEL_ILLUSIONS, S_ILLUSION,1,-1,0,-1,0},
    {"artifact lore","ARTI",100,SkillType::MAGIC,
     0,S_FORCE,1,S_PATTERN,1,S_SPIRIT,1},
    {"create ring of invisibility","CRRI",100,
     SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,3,S_INVISIBILITY,3,-1,0},
    {"create cloak of invulnerability","CRCL",100,
     SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,4, S_FORCE_SHIELD,3,-1,0},
    {"create staff of fire","CRSF",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,3, S_FIRE,3,-1,0},
    {"create staff of lightning","CRSL",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,5, S_CALL_LIGHTNING,3,-1,0},
    {"create amulet of true seeing","CRTA",100,
     SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,3, S_TRUE_SEEING,3,-1,0},
    {"create amulet of protection","CRPA",100,
     SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,1,S_SPIRIT_SHIELD,3,-1,0},
    {"create runesword","CRRU",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,4, S_CREATE_AURA_OF_FEAR,3,-1,0},
    {"create shieldstone","CRSH",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,3, S_ENERGY_SHIELD,3,-1,0},
    {"create magic carpet","CRMA",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,2, S_WEATHER_LORE,3,-1,0},
    {"engrave runes of warding","ENGR",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,3, S_ENERGY_SHIELD,3,S_SPIRIT_SHIELD,3},
    {"construct gate","CGAT",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,3,S_GATE_LORE,3,-1,0},
    {"enchant swords","ESWO",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,1,-1,0,-1,0},
    {"enchant armor","EARM",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,1,-1,0,-1,0},
    {"construct portal","CPOR",100,SkillType::MAGIC | SkillType::CAST,
     0,S_ARTIFACT_LORE,3, S_PORTAL_LORE,3,-1,0}
};

SkillType * SkillDefs = sd;

//
// Table of objects.
//
static ObjectType ot[] =
{
    //
    // name,protect,capacity,item,cost,level,skill,sailors,canenter,monster,
    // production aided.
    //
    {"None",0,0,-1,-1,0,-1,0,1,-1,-1},
    {"Longboat",0,200,I_WOOD,25,1,S_SHIPBUILDING,5,1,-1,-1},
    {"Clipper",0,800,I_WOOD,50,1,S_SHIPBUILDING,10,1,-1,-1},
    {"Galleon",0,1800,I_WOOD,75,1,S_SHIPBUILDING,15,1,-1,-1},
    {"Tower",10,0,I_STONE,10,1,S_BUILDING,0,1,-1,-1},
    {"Fort",50,0,I_STONE,40,1,S_BUILDING,0,1,-1,-1},
    {"Castle",250,0,I_STONE,160,1,S_BUILDING,0,1,-1,-1},
    {"Citadel",1250,0,I_STONE,640,1,S_BUILDING,0,1,-1,-1},
    {"Shaft",0,0,-1,-1,0,-1,0,1,-1,-1},
    {"Lair",0,0,-1,-1,0,-1,0,0,I_TRENT,-1},
    {"Ruin",0,0,-1,-1,0,-1,0,0,I_CENTAUR,-1},
    {"Cave",0,0,-1,-1,0,-1,0,0,I_DRAGON,-1},
    {"Demon Pit",0,0,-1,-1,0,-1,0,0,I_IMP,-1},
    {"Crypt",0,0,-1,-1,0,-1,0,0,I_SKELETON,-1},
    {"Balloon",0,800,I_FLOATER,50,5,S_SHIPBUILDING,10,1,-1,-1},
    {"Armored Galleon",200,1800,I_IRONWOOD,75,3,S_SHIPBUILDING,15,1,-1,-1},
    {"Magical Fortress",250,0,I_ROOTSTONE,160,3,S_BUILDING,0,1,-1,-1},
    {"Mine",0,0,I_WOOD_OR_STONE,10,3,S_MINING,0,1,-1,I_IRON},
    {"Farm",0,0,I_WOOD_OR_STONE,10,3,S_FARMING,0,1,-1,I_GRAIN},
    {"Ranch",0,0,I_WOOD_OR_STONE,10,3,S_RANCHING,0,1,-1,I_LIVESTOCK},
    {"Timber Yard",0,0,I_WOOD_OR_STONE,10,3,S_LUMBERJACK,0,1,-1,I_WOOD},
    {"Inn",0,0,I_WOOD_OR_STONE,10,3,S_BUILDING,0,1,-1,I_SILVER},
    {"Quarry",0,0,I_WOOD_OR_STONE,10,3,S_QUARRYING,0,1,-1,I_STONE},
    // LLS
    // Added ocean lairs.
    {"Island",0,0,-1,-1,0,-1,0,0,I_PIRATES,-1},
    {"Derelict Ship",0,0,-1,-1,0,-1,0,0,I_KRAKEN,-1},
    {"Cavern",0,0,-1,-1,0,-1,0,0,I_MERFOLK,-1},
    {"Whirlpool",0,0,-1,-1,0,-1,0,0,I_ELEMENTAL,-1},
	// JT
	// Added Abyss Lair
	{"Black Keep",0,0,-1,-1,0,-1,0,0,-1,-1},
    // AS
    {"Road N",0,0,I_STONE,75,3,S_BUILDING,0,1,-1,-1},
    {"Road NW",0,0,I_STONE,75,3,S_BUILDING,0,1,-1,-1},
    {"Road NE",0,0,I_STONE,75,3,S_BUILDING,0,1,-1,-1},
    {"Road SW",0,0,I_STONE,75,3,S_BUILDING,0,1,-1,-1},
    {"Road SE",0,0,I_STONE,75,3,S_BUILDING,0,1,-1,-1},
    {"Road S",0,0,I_STONE,75,3,S_BUILDING,0,1,-1,-1},
#ifdef EXTRA_STRUCTURES
	{"Temple",0,0,I_STONE,10,3,S_BUILDING,0,1,-1,I_HERBS},
	{"Mystic Quarry",0, 0, I_ROOTSTONE,20,3,S_QUARRYING,0,1,-1,I_ROOTSTONE},
	{"Arcane Mine",0,0,I_MITHRIL,20,3,S_MINING,0,1,-1,I_MITHRIL},
	{"Forest Preserve",0,0,I_IRONWOOD,20,3,S_LUMBERJACK,0,1,-1,I_IRONWOOD},
	{"Sacred Grove",0,0,I_YEW,30,5,S_LUMBERJACK,0,1,-1,I_YEW},
#endif
};

ObjectType * ObjectDefs = ot;

//
// Table of terrain types.
//
static TerrainType td[] = {
    //
    // name, flags,
    // pop, wages, economy, movepoints, prod1, chance1, amt1,
    // prod2, chance2, amt2, prod3, chance3, amt3,
    // prod4, chance4, amt4, prod5, chance5, amt5,
    // race1, race2, race3, coastalrace1, coastalrace2,
    // wmonfreq, smallmon, bigmon, humanoid,
    // lairChance, lair1, lair2, lair3, lair4
    //
    {"ocean",0,
     0,0,0,1,I_FISH,100,20,
     -1,0,0,-1,0,0,
     -1,0,0,-1,0,0,
     -1,-1,-1,-1,-1,
     1,I_PIRATES,I_KRAKEN,I_MERFOLK,
     1,O_ISLE,O_DERELICT,O_OCAVE,O_WHIRL},
    {"plain", TerrainType::RIDINGMOUNTS | TerrainType::FLYINGMOUNTS,
     800,14,40,1,I_HORSE,100,20,
     I_WHORSE,25,5,-1,0,0,
     -1,0,0,-1,0,0,
     I_PLAINSMAN,I_NOMAD,I_HIGHELF,I_VIKING,I_SEAELF,
     1,I_LION,-1,I_CENTAUR,
     3,O_RUIN,O_CRYPT,-1,-1},
    {"forest", TerrainType::FLYINGMOUNTS,
     400,12,20,2,I_WOOD,100,20,
     I_FUR,100,10,I_HERBS,100,10,
     I_IRONWOOD,25,5,I_YEW,25,5,
     I_WOODELF,I_VIKING,-1,I_SEAELF,-1,
     2,I_WOLF,I_TRENT,I_KOBOLD,
     3,O_LAIR,O_RUIN,O_CRYPT,-1},
    {"mountain", TerrainType::FLYINGMOUNTS,
     400,12,20,2,I_IRON,100,20,
     I_STONE,100,10,I_MITHRIL,25,5,
     I_ROOTSTONE,25,5,-1,0,0,
     I_BARBARIAN,I_HILLDWARF,I_ORC,I_VIKING,I_SEAELF,
     2,I_GBEAR,I_ROC,I_OGRE,
     3, O_LAIR, O_RUIN, O_CAVE, O_CRYPT },
    {"swamp", TerrainType::FLYINGMOUNTS,
     200,11,10,2,I_WOOD,100,10,
     I_FLOATER,25,10,I_HERBS,100,10,
     -1,0,0,-1,0,0,
     I_TRIBESMAN,I_TRIBALELF,-1,I_VIKING,I_SEAELF,
     2,I_CROCODILE,I_BTHING,I_LMEN,
     3, O_LAIR, O_RUIN, O_CRYPT, -1 },
    {"jungle", TerrainType::FLYINGMOUNTS,
     200,11,20,2,I_WOOD,100,10,
     I_HERBS,100,20,-1,0,0,
     -1,0,0,-1,0,0,
     I_TRIBESMAN,I_TRIBALELF,I_WOODELF,I_SEAELF,-1,
     2,I_ANACONDA,I_KONG,I_WMEN,
     3, O_LAIR, O_RUIN, O_CRYPT, -1 },
    {"desert", TerrainType::RIDINGMOUNTS | TerrainType::FLYINGMOUNTS,
     200,11,10,1,I_IRON,100,10,
     I_STONE,100,10,I_ROOTSTONE,25,5,
     -1,0,0,-1,0,0,
     I_NOMAD,I_DESERTDWARF,-1,I_SEAELF,I_VIKING,
     2,I_SCORPION,I_SPHINX,I_SANDLING,
     3, O_LAIR, O_RUIN, O_CRYPT, -1 },
    {"tundra", TerrainType::RIDINGMOUNTS | TerrainType::FLYINGMOUNTS,
     200,11,10,2,I_FUR,100,10,
     I_HERBS,100,10,-1,0,0,
     -1,0,0,-1,0,0,
     I_ESKIMO,I_ICEDWARF,-1,I_SEAELF,I_VIKING,
     2,I_PBEAR,I_IWURM,I_YETI,
     3, O_LAIR, O_RUIN, O_CRYPT, -1 },
    {"cavern", TerrainType::FLYINGMOUNTS,
     100,11,10,1,I_IRON,100,20,
     I_STONE,100,20,I_MITHRIL,25,5,
     I_ROOTSTONE,25,5,-1,0,0,
     I_DARKMAN,I_UNDERDWARF,I_ORC,-1,-1,
     3,I_RAT,I_DRAGON,I_GOBLIN,
     5, O_LAIR, O_RUIN, O_CAVE, -1 },
    {"underforest", TerrainType::FLYINGMOUNTS,
     100,11,10,2,I_WOOD,100,10,
     I_STONE,100,10,I_IRON,100,10,
     -1,0,0,-1,0,0,
     I_DARKMAN,I_UNDERDWARF,I_ORC,-1,-1,
     3,I_SPIDER,I_DRAGON,I_TROLL,
     5, O_LAIR, O_RUIN, O_CAVE, -1 },
    {"tunnels", 0,
     0,0,0,2,I_IRON,100,20,
     I_STONE,100,20,I_MITHRIL,25,5,
     I_ROOTSTONE,25,5,-1,0,0,
     -1,-1,-1,-1,-1,
     5,I_LIZARD,I_DRAGON,I_ETTIN,
     5, O_LAIR, O_RUIN, O_CAVE, O_DEMONPIT },
    {"nexus",TerrainType::RIDINGMOUNTS | TerrainType::FLYINGMOUNTS,
     0,0,0,1,-1,0,0,
     -1,0,0,-1,0,0,
     -1,0,0,-1,0,0,
     -1,-1,-1,-1,-1,
     0,-1,-1,-1,
     0,-1,-1,-1,-1}
};

TerrainType * TerrainDefs = td;


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
// 2000/MAR/16 Larry Stanbery    Added new items for Realms of the Arcane.
//                               Corrected Runesword bug.
// 2000/MAR/21 Azthar Septragen  Added roads.
#ifndef RULES_H
#define RULES_H

//
// The items
//
enum {
    I_LEADERS,
    I_VIKING,
    I_BARBARIAN,
    I_PLAINSMAN,
    I_ESKIMO,
    I_NOMAD,
    I_TRIBESMAN,
    I_DARKMAN,
    I_WOODELF,
    I_SEAELF,
    I_HIGHELF,
    I_TRIBALELF,
    I_ICEDWARF,
    I_HILLDWARF,
    I_UNDERDWARF,
    I_DESERTDWARF,
    I_ORC,
    I_SILVER,
    I_GRAIN,
    I_LIVESTOCK,
    I_IRON,
    I_WOOD,
    I_STONE,
    I_FUR,
    I_FISH,
    I_HERBS,
    I_HORSE,
    I_SWORD,
    I_CROSSBOW,
    I_LONGBOW,
    I_CHAINARMOR,
    I_PLATEARMOR,
    I_WAGON,
    I_MITHRIL,
    I_IRONWOOD,
    I_WHORSE,
    I_FLOATER,
    I_ROOTSTONE,
    I_YEW,
    I_MSWORD,
    I_MPLATE,
    I_DOUBLEBOW,
    I_IVORY,
    I_PEARL,
    I_JEWELRY,
    I_FIGURINES,
    I_TAROTCARDS,
    I_CAVIAR,
    I_WINE,
    I_SPICES,
    I_CHOCOLATE,
    I_TRUFFLES,
    I_VODKA,
    I_ROSES,
    I_PERFUME,
    I_SILK,
    I_VELVET,
    I_MINK,
    I_CASHMERE,
    I_COTTON,
    I_DYES,
    I_WOOL,
    I_LION,
    I_WOLF,
    I_GBEAR,
    I_CROCODILE,
    I_ANACONDA,
    I_SCORPION,
    I_PBEAR,
    I_RAT,
    I_SPIDER,
    I_LIZARD,
    I_TRENT,
    I_ROC,
    I_BTHING,
    I_KONG,
    I_SPHINX,
    I_IWURM,
    I_DRAGON,
    I_CENTAUR,
    I_KOBOLD,
    I_OGRE,
    I_LMEN,
    I_WMEN,
    I_SANDLING,
    I_YETI,
    I_GOBLIN,
    I_TROLL,
    I_ETTIN,
    I_SKELETON,
    I_UNDEAD,
    I_LICH,
    I_IMP,
    I_DEMON,
    I_BALROG,
    I_EAGLE,
    I_AMULETOFI,
    I_RINGOFI,
    I_CLOAKOFI,
    I_STAFFOFF,
    I_STAFFOFL,
    I_AMULETOFTS,
    I_AMULETOFP,
    I_RUNESWORD,
    I_SHIELDSTONE,
    I_MCARPET,
    I_IWOLF,
    I_IEAGLE,
    I_IDRAGON,
    I_ISKELETON,
    I_IUNDEAD,
    I_ILICH,
    I_IIMP,
    I_IDEMON,
    I_IBALROG,
    I_PORTAL,
    I_PEASANT,
    // LLS
    I_PICK,
    I_SPEAR,
    I_AXE,
    I_HAMMER,
    I_MCROSSBOW,
    I_MWAGON,
    I_GLIDER,
    I_NET,
    I_LASSO,
    I_BAG,
    I_SPINNING,
    I_LEATHERARMOR,
    I_CLOTHARMOR,
    I_BOOTS,
    I_PIRATES,
    I_KRAKEN,
    I_MERFOLK,
    I_ELEMENTAL,
    NITEMS
};

//
// Types of men.
//
enum {
    MAN_NONE,
    MAN_LEADER,
    MAN_VIKING,
    MAN_BARBARIAN,
    MAN_PLAINSMAN,
    MAN_ESKIMO,
    MAN_NOMAD,
    MAN_TRIBESMAN,
    MAN_DARKMAN,
    MAN_WOODELF,
    MAN_SEAELF,
    MAN_HIGHELF,
    MAN_TRIBALELF,
    MAN_ICEDWARF,
    MAN_HILLDWARF,
    MAN_UNDERDWARF,
    MAN_DESERTDWARF,
    MAN_ORC,
    NUMMAN
};

//
// Types of monsters
//
enum {
    MONSTER_NONE,
    MONSTER_LION,
    MONSTER_WOLF,
    MONSTER_GBEAR,
    MONSTER_CROCODILE,
    MONSTER_ANACONDA,
    MONSTER_SCORPION,
    MONSTER_PBEAR,
    MONSTER_GRAT,
    MONSTER_SPIDER,
    MONSTER_LIZARD,
    MONSTER_TRENT,
    MONSTER_ROC,
    MONSTER_BTHING,
    MONSTER_KONG,
    MONSTER_SPHINX,
    MONSTER_IWURM,
    MONSTER_DRAGON,
    MONSTER_CENTAUR,
    MONSTER_KOBOLD,
    MONSTER_OGRE,
    MONSTER_LMAN,
    MONSTER_WMAN,
    MONSTER_SANDLING,
    MONSTER_YETI,
    MONSTER_GOBLIN,
    MONSTER_TROLL,
    MONSTER_ETTIN,
    MONSTER_SKELETON,
    MONSTER_UNDEAD,
    MONSTER_LICH,
    MONSTER_IMP,
    MONSTER_DEMON,
    MONSTER_BALROG,
    MONSTER_EAGLE,
    // LLS
    MONSTER_PIRATES,
    MONSTER_KRAKEN,
    MONSTER_MERFOLK,
    MONSTER_ELEMENTAL,
    MONSTER_ILLUSION,
    NUMMONSTERS
};

//
// Types of weapons
//
// LLS
// Changed priority of weapons.
// Added new items.
enum {
    WEAPON_NONE,
    WEAPON_RUNESWORD,
    WEAPON_DOUBLEBOW,
    WEAPON_MCROSSBOW,
    WEAPON_LONGBOW,
    WEAPON_CROSSBOW,
    WEAPON_MSWORD,
    WEAPON_SWORD,
    WEAPON_PICK,
    WEAPON_SPEAR,
    WEAPON_AXE,
    WEAPON_HAMMER,
    NUMWEAPONS
};

//
// Types of armor
//
// LLS
// Added new items.
enum {
    ARMOR_NONE,
    ARMOR_CLOAKOFI,
    ARMOR_MARMOR,
    ARMOR_PLATEARMOR,
    ARMOR_CHAINARMOR,
    ARMOR_LEATHERARMOR,
    ARMOR_CLOTHARMOR,
    NUMARMORS
};

//
// Types of mounts
//
enum {
    MOUNT_NONE,
    MOUNT_WHORSE,
    MOUNT_HORSE,
    NUMMOUNTS
};

//
// Other battle items
//
// LLS
// Changed order of battle items to match Runesword
enum {
    BATTLE_NONE,
    BATTLE_RUNESWORD,
    BATTLE_STAFFOFL,
    BATTLE_STAFFOFF,
    BATTLE_AOFI,
    BATTLE_AMULETOFP,
    BATTLE_SHIELDSTONE,
    NUMBATTLEITEMS
};

//
// Types of skills.
//
enum {
    S_MINING,
    S_LUMBERJACK,
    S_QUARRYING,
    S_HUNTING,
    S_FISHING,
    S_HERBLORE,
    S_HORSETRAINING,
    S_WEAPONSMITH,
    S_ARMORER,
    S_CARPENTER,
    S_BUILDING,
    S_SHIPBUILDING,
    S_ENTERTAINMENT,
    S_TACTICS,
    S_COMBAT,
    S_RIDING,
    S_CROSSBOW,
    S_LONGBOW,
    S_STEALTH,
    S_OBSERVATION,
    S_HEALING,
    S_SAILING,
    S_FARMING,
    S_RANCHING,
    S_FORCE,
    S_PATTERN,
    S_SPIRIT,
    S_FIRE,
    S_EARTHQUAKE,
    S_FORCE_SHIELD,
    S_ENERGY_SHIELD,
    S_SPIRIT_SHIELD,
    S_MAGICAL_HEALING,
    S_GATE_LORE,
    S_FARSIGHT,
    S_TELEPORTATION,
    S_PORTAL_LORE,
    S_MIND_READING,
    S_WEATHER_LORE,
    S_SUMMON_WIND,
    S_SUMMON_STORM,
    S_SUMMON_TORNADO,
    S_CALL_LIGHTNING,
    S_CLEAR_SKIES,
    S_EARTH_LORE,
    S_WOLF_LORE,
    S_BIRD_LORE,
    S_DRAGON_LORE,
    S_NECROMANCY,
    S_SUMMON_SKELETONS,
    S_RAISE_UNDEAD,
    S_SUMMON_LICH,
    S_CREATE_AURA_OF_FEAR,
    S_SUMMON_BLACK_WIND,
    S_BANISH_UNDEAD,
    S_DEMON_LORE,
    S_SUMMON_IMPS,
    S_SUMMON_DEMON,
    S_SUMMON_BALROG,
    S_BANISH_DEMONS,
    S_ILLUSION,
    S_PHANTASMAL_ENTERTAINMENT,
    S_CREATE_PHANTASMAL_BEASTS,
    S_CREATE_PHANTASMAL_UNDEAD,
    S_CREATE_PHANTASMAL_DEMONS,
    S_INVISIBILITY,
    S_TRUE_SEEING,
    S_DISPEL_ILLUSIONS,
    S_ARTIFACT_LORE,
    S_CREATE_RING_OF_INVISIBILITY,
    S_CREATE_CLOAK_OF_INVULNERABILITY,
    S_CREATE_STAFF_OF_FIRE,
    S_CREATE_STAFF_OF_LIGHTNING,
    S_CREATE_AMULET_OF_TRUE_SEEING,
    S_CREATE_AMULET_OF_PROTECTION,
    S_CREATE_RUNESWORD,
    S_CREATE_SHIELDSTONE,
    S_CREATE_MAGIC_CARPET,
    S_ENGRAVE_RUNES_OF_WARDING,
    S_CONSTRUCT_GATE,
    S_ENCHANT_SWORDS,
    S_ENCHANT_ARMOR,
    S_CONSTRUCT_PORTAL,
    NSKILLS
};

//
// Types of special attacks
//
enum {
    SPECIAL_NONE_DUMMY,
    SPECIAL_FIREBALL,
    SPECIAL_HELLFIRE,
    SPECIAL_CAUSEFEAR,
    SPECIAL_LSTRIKE,
    SPECIAL_MINDBLAST,
    SPECIAL_EARTHQUAKE,
    SPECIAL_FORCE_SHIELD,
    SPECIAL_ENERGY_SHIELD,
    SPECIAL_SPIRIT_SHIELD,
    SPECIAL_DISPEL_ILLUSIONS,
    SPECIAL_SUMMON_STORM,
    SPECIAL_TORNADO,
    SPECIAL_CLEAR_SKIES,
    SPECIAL_BLACK_WIND,
    SPECIAL_BANISH_UNDEAD,
    SPECIAL_BANISH_DEMONS,
    NUMSPECIALS
};

//
// Types of objects.
//
enum {
    O_DUMMY,
    O_LONGBOAT,
    O_CLIPPER,
    O_GALLEON,
    O_TOWER,
    O_FORT,
    O_CASTLE,
    O_CITADEL,
    O_SHAFT,
    O_LAIR,
    O_RUIN,
    O_CAVE,
    O_DEMONPIT,
    O_CRYPT,
    O_BALLOON,
    O_AGALLEON,
    O_MFORTRESS,
    O_MINE,
    O_FARM,
    O_RANCH,
    O_TIMBERYARD,
    O_INN,
    O_QUARRY,
    // LLS
    // New ocean lairs
    O_ISLE,
    O_DERELICT,
    O_OCAVE,
    O_WHIRL,
	// JT
	// Abyss Lair
	O_BKEEP,
    // AS
    O_ROADN,
    O_ROADNW,
    O_ROADNE,
    O_ROADSW,
    O_ROADSE,
    O_ROADS,
#ifdef EXTRA_STRUCTURES
	O_TEMPLE,
	O_MQUARRY,
	O_AMINE,
	O_PRESERVE,
    O_SACGROVE,
#endif
    NOBJECTS
};

//
// Types of terrain
//
/* ARegion Types */
enum {
    R_OCEAN,
    R_PLAIN,
    R_FOREST,
    R_MOUNTAIN,
    R_SWAMP,
    R_JUNGLE,
    R_DESERT,
    R_TUNDRA,
    R_CAVERN,
    R_UFOREST,
    R_TUNNELS,
    R_NEXUS,
    R_NUM
};


#endif

#pragma once
#ifndef ITEMS_H
#define ITEMS_H

class Item;
class ItemType;

#include "gamedefs.h"
#include "strings_util.hpp"
#include "string_parser.hpp"
#include <vector>
#include <list>
#include <optional>

enum {
    ATTACK_COMBAT,
    ATTACK_ENERGY,
    ATTACK_SPIRIT,
    ATTACK_WEATHER,
    ATTACK_RIDING,
    ATTACK_RANGED,
    NUM_ATTACK_TYPES    // non resistable attack
};

enum {
    IT_NORMAL = 0x0001,
    IT_ADVANCED = 0x0002,
    IT_TRADE = 0x0004,
    IT_MAN = 0x0008,
    IT_MONSTER = 0x0010,
    IT_MAGIC = 0x0020,
    IT_WEAPON = 0x0040,
    IT_ARMOR = 0x0080,
    IT_MOUNT = 0x0100,
    IT_BATTLE = 0x0200,
    IT_SPECIAL = 0x0400,
    IT_TOOL = 0x0800,
    IT_FOOD = 0x1000,
    IT_ILLUSION = 0x2000,
    IT_UNDEAD = 0x4000,
    IT_DEMON = 0x8000,
    IT_LEADER = 0x10000,
    IT_MONEY = 0x20000,
    IT_ANIMAL = 0x40000,
    IT_SHIP = 0x80000,
    IT_MAGEONLY = 0x100000,
    IT_ALWAYS_SPOIL = 0x200000,
    IT_NEVER_SPOIL = 0x400000
};

struct Materials
{
    int item;
    int amt;
};

struct ShowItem {
    int item;
    bool full;

    std::string display_name();
    std::string display_tag();
};

class ItemType
{
    public:
        std::string name;
        std::string names;
        std::string abr;

        enum {
            CANTGIVE = 0x1,
            DISABLED = 0x2,
            NOMARKET = 0x4,
            // This item requires ANY of its inputs, not ALL
            // of them
            ORINPUTS = 0x8,
            // A number of items are produced equal to the
            // producer's skill, based on a fixed number of
            // inputs
            SKILLOUT = 0x10,
            // This item cannot be transported.
            NOTRANSPORT = 0x20,
            // Produced monsters
            MANPRODUCE = 0x40,
            SKILLOUT_HALF = 0x80,
            NOSTEALTH = 0x100, // this item makes the unit non-stealthy
            MAINTENANCE = 0x200, // this item has a maintenance cost (it uses the items base cost)
            SEEK_ALTAR = 0x400, // this item wants to move towards the nearest O_RITUAL_ALTAR (NO7 specific)
            NO_SHAFT = 0x800, // the unit carrying this item transit a shaft
        };
        int flags;

        char const *pSkill; // production skill
        int pLevel; // production skill level
        int pMonths; // Man months required for production
        int pOut; // How many of the item we get
        Materials pInput[4];

        char const *mSkill; // magical production skill
        int mLevel; // magical production skill level
        int mOut; // How many of the item are conjured
        Materials mInput[4];

        int weight;
        int type;
        int baseprice;
        int combat;

        int walk;
        int ride;
        int fly;
        int swim;
        int speed;

        int hitchItem;
        int hitchwalk;
        // LLS
        int mult_item;
        int mult_val;

        int max_inventory; // if non-zero, amount allowed in inventory.

        enum {
            // LOSE_LINKED only works with ESC_LEV_*
            LOSE_LINKED = 0x01,     // All items of same type will be lost.
            // The rest of these are mutually exclusive
            HAS_SKILL = 0x02,       // Check skill, if exists at level, no loss
            ESC_LEV_LINEAR = 0x04,      // bottom of formula based on level
            ESC_LEV_SQUARE = 0x08,      // bottom of formula based on level squared
            ESC_LEV_CUBE = 0x10,        // bottom of formula based on level cubed
            ESC_LEV_QUAD = 0x20,        // bottom of formula based on level ^ 4
            LOSS_CHANCE = 0x40,     // flat chance of escape.
            // escape chances increase quadratically with number of monsters
            ESC_NUM_SQUARE = 0x80,
        };
        int escape;
        char const *esc_skill;
        int esc_val; // level for has_skill, constant for all others

        char const *grantSkill;
        char const *fromSkills[4];
        int minGrant, maxGrant;
};

extern std::vector<ItemType> ItemDefs;


enum Ethnicity {
    NONE,
    VIKING,
    BARBARIAN,
    MAN,
    ESKIMO,
    NOMAD,
    TRIBESMAN,
    HIGHELF,
    ELF,
    DWARF,
    ORC,
    LIZARDMAN,
    DROW,
    TITAN,
    HERO
};

class ManType
{
    public:
        const std::string abbr;
        int terrain;
        int speciallevel;
        int defaultlevel;
        std::optional<std::string> skills[6];
        Ethnicity ethnicity;

        bool CanProduce(int);
        bool CanUse(int);
};

extern std::vector<ManType> ManDefs;

class MonType
{
    public:
        int attackLevel;
        int defense[NUM_ATTACK_TYPES];

        int numAttacks;
        int hits;
        int regen;

        int tactics;
        int stealth;
        int obs;

        char const *special;
        int specialLevel;

        int silver;
        int spoiltype;
        int hostile; /* Percent */
        int number;
        std::string name;
        std::string abbr;

        int hitDamage;

        // Terrain types which monster like to be in.
        // When the list is left empty, it will mean all terrains are possible.
        std::vector<int> preferredTerrain;

        // Terrain types into which monster will never try to enter.
        std::vector<int> forbiddenTerrain;

        /*
        The general algorithm of the monster movement:

        There are three categories of terrain: 1) what monster likes, 2) what monster is neutral, and 3) what monster dislikes.

        Monster will freely move through the terrain he likes, and there will be a standard chance to move into the terrain he likes.

        The monster will never enter the terrain he dislikes and will have a 2x lower chance to enter neutral terrain.

        The monster can enter into the region of neutral terrain only if that particular region has at least one neighbor of the terrain
        he likes.

        This forbids monsters from going deeper into the unusual terrain but leaves regions on borders affected by "uncommon" monsters.
        */

        const int getAggression();
};

extern std::vector<MonType> MonDefs;

enum {
    SLASHING,       // e.g. sword attack (This is default)
    PIERCING,       // e.g. spear or arrow attack
    CRUSHING,       // e.g. mace attack
    CLEAVING,       // e.g. axe attack
    ARMORPIERCING,      // e.g. crossbow double bow
    MAGIC_ENERGY,       // e.g. fire, dragon breath
    MAGIC_SPIRIT,       // e.g. black wind
    MAGIC_WEATHER,      // e.g. tornado
    NUM_WEAPON_CLASSES
};


#define MAX_WEAPON_BM_TARGETS   4

// Describes bonus/mauls against another weapon
class WeaponBonusMalus {
    public:
        char const *weaponAbbr; // weapon abbreviation
        int attackModifer;      // how much increase/decrase attack versus this weapon
        int defenseModifer;     // how much increase/decrase defense versus this weapon
};

class WeaponType
{
    public:
        const std::string abbr;

        enum {
            NEEDSKILL = 0x1, // No bonus or use unless skilled
            ALWAYSREADY = 0x2, // Ignore the 50% chance to attack
            NODEFENSE = 0x4, // No combat defense against this weapon
            NOFOOT = 0x8, // Weapon cannot be used on foot (e.g. lance)
            NOMOUNT = 0x10, // Weapon cannot be used mounted (e.g. pike)
            SHORT = 0x20, // Short melee weapon (e.g. shortsword, hatchet)
            LONG = 0x40, // Long melee weapon (e.g. lance, pike)
            RANGED = 0x80, // Missile weapon
            NOATTACKERSKILL = 0x100, // Attacker gets no combat/skill defense.
            RIDINGBONUS = 0x200, // Unit gets riding bonus on att and def.
            RIDINGBONUSDEFENSE = 0x400, // Unit gets riding bonus on def only.
        };
        int flags;

        char const *baseSkill;
        char const *orSkill;

        int weapClass;  // SLASHING, PIERCING, CRUSHING, CLEAVING, ARMORPIERCING, MAGIC_ENERGY, MAGIC_SPIRIT, MAGIC_WEATHER
        int attackType; // ATTACK_COMBAT, ATTACK_ENERGY, ATTACK_SPIRIT, ATTACK_WEATHER, ATTACK_RIDING, ATTACK_RANGED, NUM_ATTACK_TYPES (non resistable attack)
        //
        // For numAttacks:
        // - A positive number is the number of attacks per round.
        // - A negative number is the number of rounds per attack.
        // - NUM_ATTACKS_HALF_SKILL indicates that the weapon gives as many
        //   attacks as the skill of the user divided by 2, rounded up.
        // - NUM_ATTACKS_HALF_SKILL+1 indicates that the weapon gives an extra
        //   attack above that, etc.
        // - NUM_ATTACKS_SKILL indicates the the weapon gives as many attacks
        //   as the skill of the user.
        // - NUM_ATTACKS_SKILL+1 indicates the the weapon gives as many
        //   attacks as the skill of the user + 1, etc.
        //
        enum {
            NUM_ATTACKS_HALF_SKILL = 50,
            NUM_ATTACKS_SKILL = 100,
        };
        int numAttacks;

        int attackBonus;
        int defenseBonus;
        int mountBonus;

        //
        // For hitDamage:
        // - A positive number is the number of damage per attack.
        // - A negative number is the number of rounds per attack.
        // - NUM_DAMAGE_HALF_SKILL indicates that the weapon gives as many
        //   damage as the skill of the user divided by 2, rounded up.
        // - NUM_DAMAGE_HALF_SKILL+1 indicates that the weapon gives an extra
        //   damage above that, etc.
        // - NUM_DAMAGE_SKILL indicates the the weapon gives as many damage
        //   as the skill of the user.
        // - NUM_DAMAGE_SKILL+1 indicates the the weapon gives as many
        //   damage as the skill of the user + 1, etc.
        //
        enum {
            NUM_DAMAGE_HALF_SKILL = 500,
            NUM_DAMAGE_SKILL = 1000,
        };
        int hitDamage;

        WeaponBonusMalus bonusMalus[MAX_WEAPON_BM_TARGETS];
};

extern std::vector<WeaponType> WeaponDefs;

class ArmorType
{
    public:
        const std::string abbr;

        enum {
            USEINASSASSINATE = 0x1,
        };

        int flags;
        //
        // Against attacks, the chance of the armor protecting the wearer
        // is: <type>Chance / from
        //
        int from;
        int saves[NUM_WEAPON_CLASSES];
};

extern std::vector<ArmorType> ArmorDefs;

class MountType
{
    public:
        const std::string abbr;

        //
        // This is the skill needed to use this mount.
        //
        char const *skill;

        //
        // This is the minimum bonus (and minimal skill level) for this mount.
        //
        int minBonus;

        //
        // This is the maximum bonus this mount will grant.
        //
        int maxBonus;

        //
        // This is the max bonus a mount will grant if it can normally fly
        // but the region doesn't allow flying mounts
        int maxHamperedBonus;

        // If the mount has a special effect it generates when ridden in
        // combat
        char const *mountSpecial;
        int specialLev;
};

extern std::vector<MountType> MountDefs;

class BattleItemType
{
    public:
        const std::string abbr;

        enum {
            MAGEONLY = 0x1,
            SPECIAL = 0x2,
            SHIELD = 0x4,
            EXCLUSIVE = 0x8,
        };

        int flags;
        char const *special;
        int skillLevel;

        int hitDamage;
};

extern std::vector<BattleItemType> BattleItemDefs;

extern int parse_all_items(const parser::token& token, int flags = 0);
extern int parse_enabled_item(const parser::token& token, int flags = 0);
extern int parse_giveable_item(const parser::token& token, int flags = 0);
extern int parse_transportable_item(const parser::token& token, int flags = 0);
extern std::optional<int> parse_item_category(const parser::token& token);

extern int lookup_item(const strings::ci_string& name);

extern std::optional<std::reference_wrapper<BattleItemType>> find_battle_item(const strings::ci_string& abbr);
extern std::optional<std::reference_wrapper<ItemType>> find_item(const strings::ci_string& abbr);
extern std::optional<std::reference_wrapper<ArmorType>> find_armor(const strings::ci_string& abbr);
extern std::optional<std::reference_wrapper<WeaponType>> find_weapon(const strings::ci_string& abbr);
extern std::optional<std::reference_wrapper<MountType>> find_mount(const strings::ci_string& abbr);
extern std::optional<std::reference_wrapper<MonType>> find_monster(const strings::ci_string& abbr, int illusion);
extern std::optional<std::reference_wrapper<ManType>> find_race(const strings::ci_string& abbr);
extern std::string attack_type(int atype);

enum {
    FULLNUM = 0x01,
    ALWAYSPLURAL = 0x02
};
extern std::string item_string(int type, int num, int flags = 0);
extern std::string item_description(int item, int full);

extern int IsSoldier(int);

class Item
{
    public:
        Item();
        ~Item();

        void Readin(std::istream& f);
        void Writeout(std::ostream& f);

        std::string report(bool see_illusions);

        int type;
        int num;
        int selling;
        int checked; // flag whether item has been reported, counted etc.
};

class ItemList
{
    std::list<Item *> items;

    public:
        using iterator = typename std::list<Item *>::iterator;

        void Readin(std::istream& f);
        void Writeout(std::ostream& f);

        std::string report(int obs, int seeillusions, int nofirstcomma);
        std::string battle_report();
        std::string report_by_type(int type, int obs, int seeillusions, int nofirstcomma);

        int Weight();
        int GetNum(int);
        void SetNum(int, int); /* type, number */
        int CanSell(int);
        void Selling(int, int); /* type, number */
        void UncheckAll(); // re-set checked flag for all

        inline iterator begin() { return items.begin(); }
        inline iterator end() { return items.end(); }
        inline iterator erase(iterator it) { return items.erase(it); }
        inline size_t size() { return items.size(); }
        inline void clear() { items.clear(); }
        inline Item *front() { return items.front(); }

};

extern std::string show_special(const std::string& special, int level, int expandLevel, int fromItem);

#endif // ITEMS_H

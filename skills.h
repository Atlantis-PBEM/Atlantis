#pragma once
#ifndef SKILL_H
#define SKILL_H

class Faction;
class Skill;
class SkillList;

#include "gamedefs.h"
#include <list>
#include "string_parser.hpp"

/* For dependencies:
  A value of depend == -1 indicates no more dependencies.
  If depend is set to a skill, to study this skill, you must know
  the depended skill at level equal to (at least) the level in the
  structure, or at the level you are trying to study to.

  Example:
  SANDLE has depends[0].skill = SHOE and depends[0].level = 3.

  To study: requires:
  SANDLE 1  SHOE 3
  SANDLE 2  SHOE 3
  SANDLE 3  SHOE 3
  SANDLE 4  SHOE 4
  SANDLE 5  SHOE 5
*/

struct SkillDepend
{
    char const *skill;
    int level;
};

class SkillType
{
    public:
        std::string name;
        std::string abbr;
        int cost;

        enum {
            MAGIC = 0x1,
            COMBAT = 0x2,
            CAST = 0x4,
            FOUNDATION = 0x8,
            APPRENTICE = 0x10,
            DISABLED = 0x20,
            SLOWSTUDY = 0x40,
            BATTLEREP = 0x80,
            NOTIFY = 0x100,
            DAMAGE = 0x200,
            FEAR = 0x400,
            MAGEOTHER=0x800,
            NOSTUDY=0x1000,
            NOTEACH=0x2000,
            NOEXP=0x4000,
            GRANTED=0x8000,
        };
        int flags;

        //
        // special for combat spells only
        //
        std::optional<std::string> special;

        // range class for ranged skills
        std::optional<std::string> range;

        SkillDepend depends[3];

        bool operator==(const SkillType& other) const {
            return name == other.name && abbr == other.abbr && flags == other.flags;
        }
};
extern std::vector<SkillType> SkillDefs;

std::optional<std::reference_wrapper<SkillType>> FindSkill(char const *skname);
int lookup_skill(const parser::token& token);
int parse_skill(const parser::token& token);
std::string SkillStrs(int skill);
std::string SkillStrs(const SkillType& skill);

int SkillCost(int);
int SkillMax(char const *,int); /* skill, race */
int GetLevelByDays(int);
int GetDaysByLevel(int);
int StudyRateAdjustment(int, int); /* days, exp */

struct ShowSkill {
    int skill;
    int level;

    const std::string Report(Faction *faction) const;
};

class Skill {
public:
    void Readin(std::istream& f);
    void Writeout(std::ostream &f) const;
    Skill *Split(int total, int split);

    int type;
    unsigned int days;
    unsigned int exp;
};

class SkillList {
    std::list<Skill *> skills;

public:
    using iterator = typename std::list<Skill *>::iterator;

    int GetDays(int sk);
    int GetExp(int sk);
    void SetDays(int sk, int days);
    void SetExp(int sk,int exp);
    void Combine(SkillList *skl);
    int GetStudyRate(int sk, int men);
    SkillList *Split(int total, int split);
    std::string report(int men);
    void Readin(std::istream& f);
    void Writeout(std::ostream& f);
    inline int size() { return skills.size(); }
    inline Skill *front() { return skills.front(); }

    inline iterator begin(){ return skills.begin(); }
    inline iterator end(){ return skills.end(); }
    inline size_t erase(Skill *s) { return std::erase(skills, s); }
};

class HealType {
    public:
        int num;
        int rate;
};
extern std::vector<HealType> HealDefs;
extern std::vector<HealType> MagicHealDefs;

class DamageType {
    public:
        int type;
        int minnum;
        int value;
        int flags;
        int dclass;
        char const *effect;
        int hitDamage;
};

class ShieldType {
    public:
        int type;
        int value;
}
;
class DefenseMod {
    public:
        int type;
        int val;
};

#define SPECIAL_BUILDINGS   5

class SpecialType {
    public:
        const std::string key;
        const std::string specialname;

        enum {
            HIT_BUILDINGIF      = 0x001,    /* mutually exclusive (1) */
            HIT_BUILDINGEXCEPT  = 0x002,    /* mutually exclusive (1) */
            HIT_SOLDIERIF       = 0x004,    /* mutually exclusive (2) */
            HIT_SOLDIEREXCEPT   = 0x008,    /* mutually exclusive (2) */
            HIT_MOUNTIF         = 0x010,    /* mutually exclusive (2) */
            HIT_MOUNTEXCEPT     = 0x020,    /* mutually exclusive (2) */
            HIT_EFFECTIF        = 0x040,    /* mutually exclusive (3) */
            HIT_EFFECTEXCEPT    = 0x080,    /* mutually exclusive (3) */
            HIT_ILLUSION        = 0x100,
            HIT_NOMONSTER       = 0x200,
        };
        int targflags;

        int buildings[SPECIAL_BUILDINGS];
        int targets[7];
        char const *effects[3];

        enum {
            FX_SHIELD   =   0x01,
            FX_DAMAGE   =   0x02,
            FX_USE_LEV  =   0x04,
            FX_DEFBONUS =   0x08,
            FX_NOBUILDING = 0x10,
            FX_DONT_COMBINE=0x20,
        };
        int effectflags;

        int shield[4];
        DefenseMod defs[4];
        char const *shielddesc;

        DamageType damage[4];
        char const *spelldesc;
        char const *spelldesc2;
        char const *spelltarget;
};
extern std::vector<SpecialType> SpecialDefs;

extern std::optional<std::reference_wrapper<SpecialType>> find_special(const strings::ci_string& key);

class EffectType {
    public:
        const std::string name;
        int attackVal;
        DefenseMod defMods[4];
        char const *cancelEffect;

        enum {
            EFF_ONESHOT = 0x001,
            EFF_NOSET = 0x002,
        };
        int flags;
};
extern std::vector<EffectType> EffectDefs;

extern std::optional<std::reference_wrapper<EffectType>> FindEffect(char const *effect);

class RangeType {
    public:
        const std::string key;
        enum {
            RNG_NEXUS_TARGET = 0x0001,  // Can cast *to* Nexus
            RNG_NEXUS_SOURCE = 0x0002,  // Can cast *from* Nexus
            RNG_CROSS_LEVELS = 0x0004,  // Spell can cross levels
            RNG_SURFACE_ONLY = 0x0008,  // Target region must be on surface
        };
        int flags;

        enum {
            RNG_ABSOLUTE = 0,   // Range is not based on skill
            RNG_LEVEL,          // Range is based on skill
            RNG_LEVEL2,         // Range is based on skill level squared
            RNG_LEVEL3,         // Range is based on skill level cubed
            NUMRANGECLASSES
        };
        int rangeClass;

        int rangeMult;

        int crossLevelPenalty;  // How much extra distance to cross levels?
};
extern std::vector<RangeType> RangeDefs;

extern std::optional<std::reference_wrapper<RangeType>> find_range(strings::ci_string range);

class AttribModItem {
    public:
        enum {
            SKILL = 0x0001,
            ITEM = 0x0002,
            FLAGGED = 0x0004,
            NOT = 0x0100,
            CUMULATIVE = 0x0200,
            PERMAN = 0x400
        };
        int flags;

        std::string ident;

        enum {
            CONSTANT,
            UNIT_LEVEL,
            UNIT_LEVEL_HALF,
            FORCECONSTANT,
            NUMMODTYPE,
        };
        int modtype;

        int val;
};

class AttribModType {
    public:
        const std::string key;

        enum {
            CHECK_MONSTERS = 0x01,
            USE_WORST = 0x02,
        };
        int flags;

        AttribModItem mods[5];
};

extern std::vector<AttribModType> AttribDefs;

extern std::optional<std::reference_wrapper<AttribModType>> FindAttrib(char const *attrib);

#endif // SKILL_H

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
// Date        Person          Comment
// ----        ------          -------
// 2001/Feb/18 Joseph Traub    Added apprentice support from Lacandon Conquest
// 2001/Feb/18 Joseph Traub    Added support for conquest
//
#ifndef SKILL_CLASS
#define SKILL_CLASS

class Faction;
class Skill;
class SkillList;

#include "fileio.h"
#include "astring.h"
#include "gamedefs.h"
#include "alist.h"

/* For dependencies:
  A value of depend == -1 indicates no more dependencies.
  If depend is set to a skill, to study this skill, you must know
  the depended skill at level equal to (at least) the level in the
  structure, or at the level you are trying to study to.

  Example:
  SANDLE has depends[0].skill = SHOE and depends[0].level = 3.

  To study:	requires:
  SANDLE 1	SHOE 3
  SANDLE 2	SHOE 3
  SANDLE 3	SHOE 3
  SANDLE 4	SHOE 4
  SANDLE 5	SHOE 5
*/

struct SkillDepend
{
	char const *skill;
	int level;
};

class SkillType
{
	public:
		char const * name;
		char const * abbr;
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
		};
		int flags;

		//
		// special for combat spells only
		//
		char const *special;

		// range class for ranged skills (-1 for all others)
		char const *range;

		SkillDepend depends[3];
};
extern SkillType *SkillDefs;

SkillType *FindSkill(char const *skname);
int LookupSkill(AString *);
int ParseSkill(AString *);
AString SkillStrs(int);
AString SkillStrs(SkillType *);

class ShowType {
	public:
		int skill;
		int level;
		char const * desc;
};
extern ShowType * ShowDefs;

int SkillCost(int);
int SkillMax(char const *,int); /* skill, race */
int GetLevelByDays(int);
int GetDaysByLevel(int);
int StudyRateAdjustment(int, int); /* days, exp */

class ShowSkill : public AListElem {
	public:
		ShowSkill(int,int);

		AString * Report(Faction *);

		int skill;
		int level;
};

class Skill : public AListElem {
	public:
		void Readin(Ainfile *);
		void Writeout(Aoutfile *);

		Skill * Split(int,int); /* total num, num leaving */

		int type;
		unsigned int days;
		unsigned int exp;
};

class SkillList : public AList {
	public:
		int GetDays(int); /* Skill */
		int GetExp(int); /* Skill */
		void SetDays(int,int); /* Skill, days */
		void SetExp(int,int); /* Skill, exp */
		void Combine(SkillList *);
		int GetStudyRate(int, int); /* Skill, num of men */
		SkillList * Split(int,int); /* total men, num to split */
		AString Report(int); /* Number of men */
		void Readin(Ainfile *);
		void Writeout(Aoutfile *);
};

class HealType {
	public:
		int num;
		int rate;
};
extern HealType * HealDefs;
extern HealType * MagicHealDefs;

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

#define SPECIAL_BUILDINGS	5

class SpecialType {
	public:
		char const *key;
		char const *specialname;

		enum {
			HIT_BUILDINGIF		= 0x001,	/* mutually exclusive (1) */
			HIT_BUILDINGEXCEPT	= 0x002,	/* mutually exclusive (1) */
			HIT_SOLDIERIF		= 0x004,	/* mutually exclusive (2) */
			HIT_SOLDIEREXCEPT	= 0x008,	/* mutually exclusive (2) */
			HIT_MOUNTIF			= 0x010,	/* mutually exclusive (2) */
			HIT_MOUNTEXCEPT		= 0x020,	/* mutually exclusive (2) */
			HIT_EFFECTIF		= 0x040,	/* mutually exclusive (3) */
			HIT_EFFECTEXCEPT	= 0x080,	/* mutually exclusive (3) */
			HIT_ILLUSION		= 0x100,
			HIT_NOMONSTER		= 0x200,
		};
		int targflags;

		int buildings[SPECIAL_BUILDINGS];
		int targets[7];
		char const *effects[3];

		enum {
			FX_SHIELD	=	0x01,
			FX_DAMAGE	=	0x02,
			FX_USE_LEV	=	0x04,
			FX_DEFBONUS	=	0x08,
			FX_NOBUILDING =	0x10,
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
extern SpecialType *SpecialDefs;
extern int NUMSPECIALS;

extern SpecialType *FindSpecial(char const *key);

class EffectType {
	public:
		char const *name;
		int attackVal;
		DefenseMod defMods[4];
		char const *cancelEffect;

		enum {
			EFF_ONESHOT	= 0x001,
			EFF_NOSET = 0x002,
		};
		int flags;
};
extern EffectType *EffectDefs;
extern int NUMEFFECTS;

extern EffectType *FindEffect(char const *effect);

class RangeType {
	public:
		char const *key;
		enum {
			RNG_NEXUS_TARGET = 0x0001,	// Can cast *to* Nexus
			RNG_NEXUS_SOURCE = 0x0002,	// Can cast *from* Nexus
			RNG_CROSS_LEVELS = 0x0004,	// Spell can cross levels
			RNG_SURFACE_ONLY = 0x0008,	// Target region must be on surface
		};
		int flags;

		enum {
			RNG_ABSOLUTE = 0,	// Range is not based on skill
			RNG_LEVEL,			// Range is based on skill
			RNG_LEVEL2,			// Range is based on skill level squared
			RNG_LEVEL3,			// Range is based on skill level cubed
			NUMRANGECLASSES
		};
		int rangeClass;

		int rangeMult;

		int crossLevelPenalty;	// How much extra distance to cross levels?
};
extern RangeType *RangeDefs;
extern int NUMRANGES;

extern RangeType *FindRange(char const *range);

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

		char const *ident;

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
		char const *key;

		enum {
			CHECK_MONSTERS = 0x01,
			USE_WORST = 0x02,
		};
		int flags;

		AttribModItem mods[5];
};

extern AttribModType *AttribDefs;
extern int NUMATTRIBMODS;

extern AttribModType *FindAttrib(char const *attrib);

#endif

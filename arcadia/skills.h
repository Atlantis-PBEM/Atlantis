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

  To study:   requires:
  SANDLE 1    SHOE 3
  SANDLE 2    SHOE 3
  SANDLE 3    SHOE 3
  SANDLE 4    SHOE 4
  SANDLE 5    SHOE 5
*/

struct SkillDepend
{
	char *skill;
	int level;
};

class SkillType
{
	public:
		char * name;
		char * abbr;
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
			FEAR = 0x400,        //only used in tax calculations (can mages with fear spell tax)
			MAGEOTHER=0x800,     //not sure what the purpose of this is. Only used in taxing
			NOSTUDY=0x1000,
			NOTEACH=0x2000,
			COSTVARIES=0x4000,   //makes the energy cost of casting a spell decrease with the spell level
			STUDYOR=0x8000,
			UPGRADE=0x10000,
		};
		int flags;

		//
		// special for combat spells only
		//
		char *special;

		// range class for ranged skills (-1 for all others)
		char *range;

		int baseskill;

		SkillDepend depends[3];

		int cast_cost;
		int combat_first;
		int combat_cost;
};
extern SkillType *SkillDefs;

SkillType *FindSkill(char *skname);
int LookupSkill(AString *);
int ParseSkill(AString *);
AString SkillStrs(int);
AString SkillStrs(SkillType *);

class ShowType {
	public:
		int skill;
		int level;
		char * desc;
};
extern ShowType * ShowDefs;

int SkillCost(int); /* skill */
int IsSpeciality(char *,int); /* skill, race */
int SkillMax(char *,int); /* skill, race */
int SkillExperMax(char *,int); /* skill, race */
int GetLevelByDays(int);
int GetLevelByDays(int, int);
int GetDaysByLevel(int);

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
		unsigned int experience; /* REAL_EXPERIENCE Patch */
};

class SkillList : public AList {
	public:
		int GetDays(int); /* Skill */
		int GetExper(int); /* Skill */
		void SetDays(int,int); /* Skill, days */
		void SetDays(int,int,int); /* Skill, days, exper */		
		void SetExper(int,int); /* Skill, exper */		
		void Combine(SkillList *);
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

class DamageType {
	public:
		int type;
		int minnum;
		int value;
		int flags;
		int dclass;
		char *effect;
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

class SpecialType {
	public:
		char *key;
		char *specialname;

		enum {
			HIT_BUILDINGIF		= 0x0001,	/* mutually exclusive (1) */
			HIT_BUILDINGEXCEPT	= 0x0002,	/* mutually exclusive (1) */
			HIT_SOLDIERIF		= 0x0004,	/* mutually exclusive (2) */
			HIT_SOLDIEREXCEPT	= 0x0008,	/* mutually exclusive (2) */
			HIT_MOUNTIF			= 0x0010,	/* mutually exclusive (2) */
			HIT_MOUNTEXCEPT		= 0x0020,	/* mutually exclusive (2) */
			HIT_EFFECTIF		= 0x0040,	/* mutually exclusive (3) */
			HIT_EFFECTEXCEPT	= 0x0080,	/* mutually exclusive (3) */
			HIT_ILLUSION		= 0x0100,   //currently all illusions are monsters, so to hit_nomonsters will not hit illusions. This may change, in which case some changes need to be done.
			HIT_NOILLUSION      = 0x0200,
			HIT_NOMONSTER		= 0x0400,
			HIT_MONSTEREXCEPT   = 0x0800,    //not used yet, planned for transfiguration
			HIT_OWN_ARMY        = 0x1000,    //spell designed to hit own army
		};
		int targflags;

		int buildings[3];
		int targets[7];
		char *effects[3];

		enum {
			FX_SHIELD	=	0x001,
			FX_DAMAGE	=	0x002,   // Needed to use during combat round (eg not shields)
			FX_USE_LEV	=	0x004,   // multiplies damage by skill level
			FX_DEFBONUS	=	0x008,   // gives def bonus to caster (?) 
			FX_NOBUILDING =	0x010,   // no def bonus from building?
			FX_DONT_COMBINE=0x020,   // horrible report format!
			FX_FOG      =   0x040,   // spell which affects taccontrol.  Mututally exclusive with FX_SHIELD (changed?)
			FX_ARMYBONUS  = 0x080,   // spell which affects armies. Mutually exclusive with FX_DEFBONUS (changed?)
			FX_DUAL     =   0x100,   // affects both armies equally (fog spells only)
		};
		int effectflags;

		int shield[4];
		DefenseMod defs[4];
		char *shielddesc;

		DamageType damage[4];
		char *spelldesc;
		char *spelldesc2;
		char *spelltarget;
};
extern SpecialType *SpecialDefs;
extern int NUMSPECIALS;

extern SpecialType *FindSpecial(char *key);

class EffectType {
	public:
	    int effectnum; //hack because mapping of effects in battle was not working in modified combat code - don't know why. These need to numbered sequentially from zero, and must be less or equal effects than size of effect array in soldiers.h
		char *name;
		int attackVal;
		DefenseMod defMods[4];
		char *cancelEffect;

		enum {
			EFF_ONESHOT	= 0x001,  //gets cleared at the end of combat round (Arcadian system only)
			EFF_NOSET = 0x002,    //soldier gets penalty, but is not flagged as having that effect
			EFF_TRANSFIGURE = 0x004,  //special spell, transfigures down to rats. Not tested on 'men', only monsters.
		};
		int flags;
		int monster;
};
extern EffectType *EffectDefs;
extern int NUMEFFECTS;

extern EffectType *FindEffect(char *effect);

class RangeType {
	public:
		char *key;
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

extern RangeType *FindRange(char *range);

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

		char *ident;

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
		char *key;

		enum {
			CHECK_MONSTERS = 0x01,
			USE_WORST = 0x02,
		};
		int flags;

		AttribModItem mods[6];  // BS mod to expand stealth
};

extern AttribModType *AttribDefs;
extern int NUMATTRIBMODS;

extern AttribModType *FindAttrib(char *attrib);

#endif

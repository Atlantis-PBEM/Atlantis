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
  SANDLE has depend1 = SHOE and level1 = 3.

  To study:   requires:
  SANDLE 1    SHOE 3
  SANDLE 2    SHOE 3
  SANDLE 3    SHOE 3
  SANDLE 4    SHOE 4
  SANDLE 5    SHOE 5
*/
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
    };
    int flags;

    //
    // special for combat spells only
    //
    int special;

    int depend1;
    int level1;
    int depend2;
    int level2;
    int depend3;
    int level3;
};

extern SkillType * SkillDefs;

int ParseSkill(AString *);

AString SkillStrs(int);

class ShowType { 
public:
  int skill;
  int level;
  char * desc;
};

extern ShowType * ShowDefs;

int SkillCost(int);
int SkillMax(int,int); /* skill, race */

int GetLevelByDays(int);
int GetDaysByLevel(int);

class ShowSkill : public AListElem {
public:
  ShowSkill(int,int);
  
  AString * Report();
  
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
};

class SkillList : public AList {
public:
  int GetDays(int); /* Skill */
  void SetDays(int,int); /* Skill, days */
  void Combine(SkillList *);
  SkillList * Split(int,int); /* total men, num to split */
  AString Report(int); /* Number of men */
  void Readin(Ainfile *);
  void Writeout(Aoutfile *);
};

class HealType
{	
public:
	int num;
	int rate;
};

extern HealType * HealDefs;

#endif

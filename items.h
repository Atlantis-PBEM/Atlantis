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
// Date        Person          Comments
// ----        ------          --------
// 2000/MAR/14 Larry Stanbery  Added enhanced production capability.
#ifndef ITEMS_CLASS
#define ITEMS_CLASS

class Item;
class ItemType;

#include "fileio.h"
#include "gamedefs.h"
#include "alist.h"
#include "astring.h"

enum {
    IT_NORMAL = 0x01,
    IT_ADVANCED = 0x02,
    IT_TRADE = 0x04,
    IT_MAN = 0x08,
    IT_MONSTER = 0x10,
    IT_MAGIC = 0x20,
    IT_WEAPON = 0x40,
    IT_ARMOR = 0x80,
    IT_MOUNT = 0x100,
    IT_BATTLE = 0x200,
	IT_SPECIAL = 0x400,
	IT_TOOL = 0x800,
};

class ItemType
{
public:
    char *name;
    char *names;
    char *abr;

    enum {
        CANTGIVE = 0x1,
		DISABLED = 0x2,
    };
    int flags;

    int skill;
    int level;
    int input;
    int number;
	int input2;
	int number2;
    int weight;
    int type;
    int baseprice;
    int combat;
    int index;
    int walk;
    int ride;
    int fly;
    int swim;
    // LLS
    int mult_item;
    int mult_val;
};

extern ItemType * ItemDefs;

class ManType
{
public:
    int speciallevel;
    int defaultlevel;
    int firstskill;
    int secondskill;
    int thirdskill;
    int fourthskill;
};

extern ManType * ManDefs;

class MonType
{
public:
    int skill;
    int hits;
    int tactics;
    int special;
    int stealth;
    int obs;
    int silver;
    int spoiltype;
    int hostile; /* Percent */
    int number;
    char *name;
};

extern MonType * MonDefs;

class WeaponType
{
public:
    enum {
        NEEDSKILL = 0x1,
        RANGED = 0x4,
        NODEFENSE = 0x8,
        GOODARMOR = 0x10,
        NEVERMISS = 0x20,
    };
        
    int skill1;
    int skill2;
    int flags;
    int skillBonus;

    //
    // For numAttacks:
    // - A positive number is the number of attacks per round.
    // - A negative number is the number of rounds per attack.
    // - NUM_ATTACKS_SKILL indicates the the weapon gives as many attacks
    //   as the skill of the user.
    //
    enum {
        NUM_ATTACKS_SKILL = 100,
    };
    int numAttacks;

};

extern WeaponType *WeaponDefs;

class ArmorType
{
public:
    enum {
        USEINASS = 0x1,
    };

    int flags;

    //
    // Against normal attacks, the chance of the armor protecting the wearer
    // is: normalChance / normalFrom
    //
    int normalChance;
    int normalFrom;

    //
    // Same thing, but against "good" attacks.
    //
    int goodChance;
    int goodFrom;
};

extern ArmorType *ArmorDefs;

class MountType
{
public:
    //
    // This is the skill needed to use this mount.
    //
    int skill;

    //
    // This is the minimum bonus (and minimal skill level) for this mount.
    //
    int minBonus;

    //
    // This is the maximum bonus this mount will grant.
    //
    int maxBonus;

};

extern MountType *MountDefs;

class BattleItemType
{
public:
    enum {
        MAGEONLY = 0x1,
        SPECIAL = 0x2,
        SHIELD = 0x4,
    };

    int flags;
    int itemNum;
    int index;
    int skillLevel;

};

extern BattleItemType *BattleItemDefs;

int ParseItem(AString *);

AString ItemString(int,int); /* type, num */
AString ItemDescription(int);

int IsSoldier(int);

class Item : public AListElem
{
public:
    Item();
    ~Item();
    
    void Readin(Ainfile *);
    void Writeout(Aoutfile *);
    
    AString Report(int);
    
    int type;
    int num;
};

class ItemList : public AList
{
public:
    void Readin(Ainfile *);
    void Writeout(Aoutfile *);
	
    AString Report(int,int,int);
    AString BattleReport();
    
    int Weight();
    int GetNum(int);
    void SetNum(int,int); /* type, number */
};

#endif
	



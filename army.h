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
#ifndef ARMY_CLASS
#define ARMY_CLASS

class Soldier;
class Army;

#include "unit.h"
#include "alist.h"
#include "items.h"
#include "object.h"
#include "shields.h"
#include "helper.h"

#define SPECIAL_NONE 0

#define EFFECT_DAZZLE 1
#define EFFECT_FEAR 2
#define EFFECT_STORM 4
#define EFFECT_CAMEL_FEAR 8

#define SPECIAL_FLAGS ( WeaponType::NEVERMISS | WeaponType::GOODARMOR )

enum {
    ATTACK_COMBAT,
    ATTACK_ENERGY,
    ATTACK_SPIRIT,
    ATTACK_WEATHER,
    ATTACK_RIDING,
    ATTACK_RANGED,
    NUM_ATTACK_TYPES
};

enum {
    HEAL_NONE,
    HEAL_THREE,
    HEAL_TWO,
    HEAL_ONE
};

class Soldier {
public:
    Soldier( Unit *unit, 
             Object *object,
             int regType,
             int race,
             int ass = 0 );

    void SetupSpell();
    void SetupCombatItems();

    //
    // SetupHealing is actually game-specific, and appears in specials.cpp
    //
    void SetupHealing();
  
    int HasEffect(int);
    void SetEffect(int);
    void ClearEffect(int);	
    int ArmorProtect( int weaponFlags );

    void RestoreItems();
    void Alive(int);
    void Dead();

    /* Unit info */
	AString name;
    Unit * unit;
    int race;
    int riding;
    int building;
    
    /* Healing information */
    int healing;
    int healtype;
    int healitem;
    int canbehealed;

    /* Attack info */
    int weapon;
    int attacktype;
    int askill;
    int attacks;
    int special;
    int slevel;

    /* Defense info */
    int dskill[NUM_ATTACK_TYPES];
    int armor;
    int hits;

    BITFIELD battleItems;
    int amuletofi;
    
    /* Effects */
    int effects;
};

typedef Soldier * SoldierPtr;

class Army
{
public:
    Army(Unit *,AList *,int,int = 0);
	~Army();
  
    void WriteLosses(Battle *);
    void Lose(Battle *,ItemList *);
    void Win(Battle *,ItemList *);
    void Tie(Battle *);
    int CanBeHealed();
    void DoHeal(Battle *);
    void DoHealLevel(Battle *,int,int, int useItems );
	
    void GetMonSpoils(ItemList *,int);

    int Broken();
    int NumAlive();
	int NumSpoilers();
    int CanAttack();
    int NumFront();
    Soldier *GetAttacker( int, int & );
    int GetEffectNum( int effect );
    int GetTargetNum( int = SPECIAL_NONE );
    Soldier *GetTarget( int );
    int RemoveEffects( int num, int effect );
    int DoAnAttack( int special, int numAttacks, int attackType,
                    int attackLevel, int flags, int effect );
    void Kill(int);
    void Reset();

    //
    // These funcs are rules specific, and are in specials.cpp
    //
    int CheckSpecialTarget(int,int);

    SoldierPtr * soldiers;
    Unit * leader;
    ShieldList shields;
    int round;
    int tac;
    int canfront;
    int canbehind;
    int notfront;
    int notbehind;
    int count;
};

#endif

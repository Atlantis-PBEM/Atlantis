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
#ifndef SOLDIER_CLASS
#define SOLDIER_CLASS

#include <functional>
#include <map>
using namespace std;

class Soldier;

#include "unit.h"
#include "alist.h"
#include "items.h"
#include "object.h"
#include "shields.h"
#include "helper.h"

#ifndef DEBUG
//#define DEBUG
#endif

enum {
	WIN_MOVE,
	WIN_NO_MOVE,
	LOSS
};

class Soldier
{
	public:
		Soldier(Unit *unit, Object *object, int regType, int race, int ass=0);
		~Soldier();

		void SetupSpell();
		void SetupCombatItems();

		//
		// SetupHealing is actually game-specific, and appears in specials.cpp
		//
		void SetupHealing();

		int ArmorProtect(int weaponClass );

		void RestoreItems();
		void Alive(int);
		void Dead();

		int HasEffect(char *);
		void SetEffect(char *, int form, Army *army);
		void ClearEffect(char *);
		void ClearOneTimeEffects(void);
		
		int DoSpellCost(int round, Battle *b);
		void DoSpellCheck(int round, Battle *b);
		
		/* Unit info */
		AString name;
		Unit * unit;
		int race;
		int riding;
		int ridingbonus;
		int building;

		/* Healing information */
		int healing;
		int healtype;
		int healitem;
		int canbehealed;
		int canberesurrected;
		int regen;
		
		/* Attack info */
		int weapon;
		int attacktype;
		int askill;
		int attacks;
		char *special;
		int slevel;
		int exhausted;
		int candragontalk;

		/* Defense info */
		int dskill[NUM_ATTACK_TYPES];
		int protection[NUM_ATTACK_TYPES];
		int armor;
		int hits;
		int maxhits;
		int damage;
		int illusion;

		int isdead;
		
		/* Formation info */
		int inform;
		int defaultform;
		
		BITFIELD battleItems;
		int amuletofi;

		/* Effects */
//		map< char *, int > effects;
        int effects[5];
};

typedef Soldier * SoldierPtr;

#endif

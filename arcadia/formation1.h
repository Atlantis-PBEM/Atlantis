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

#ifndef FORMATION_CLASS
#define FORMATION_CLASS

#include <functional>
#include <map>
using namespace std;

class Formation;

#include "soldier1.h"
#include "unit.h"
#include "alist.h"
#include "items.h"
#include "object.h"
#include "helper.h"

enum {
	FORM_FOOT,
	FORM_RIDE,
	FORM_FLY,
	FORM_ANY
};

enum {
    ENGAGED_NONE,
    ENGAGED_CONDITIONAL,
    ENGAGED_CANATTACK,
    ENGAGED_TRYATTACK,
    ENGAGED_ENGAGED,
    ENGAGED_ANY
};

enum {
    FORM_FRONT,
    FORM_FLANKING,
    FORM_FLANKED,
    FORM_DEAD
};

class Formation
{
	public:
		Formation();
		~Formation();

		void SetupFormation(int formnum, int count);

		int bonus;      // modifies the attack chance from 50% to 67% (+1) or 33% (-1) etc.
		int tempbonus;  // this only applies to the formation currently attacking
		
		//access methods		
	    int GetNumMen() const { return nummen; }
	    int GetSize() const { return size; }
	    void AddSize(int addsize) {size += addsize; }
	    int Type() const { return type; }
	    int Behind() const { return behind; }
	    int Flank() const { return flank; }
	    int Reserve() const { return reserve; }
	    int IsConcealed() const { return concealed; }
	    void SetConcealed(int state) { concealed = state; }
	    int CanAttack() const { return canattack; }
	    void SetTemporaryBonus(int b);
	    void RemoveTemporaryBonus();
	    Soldier * GetSoldier(int soldiernum) const;
	    Soldier * GetAttacker(int soldiernum);
	    int GetNonIllusionSize() const;
	    int CountMages() const;
	    Soldier * GetMage(int);
	    
	    
	    //modification methods
	    void AddSoldier(Soldier * pSoldier);
	    void AddCanAttackSoldier(Soldier * pSoldier);
	    Soldier * RemoveSoldier(int soldiernum);
	    void TransferSoldier(int, Formation * pToForm);
	    void MoveSoldiers(Formation * pToForm);
	    int MoveSoldiers(Formation * pToForm, int sizetomove, int condition=0);
	    void Kill(int soldiernum, Army *itsarmy);

		void Reset();
		void ResetHeal() const;
		void Regenerate(Battle *b) const;

		//Magic Targetting:
		int CheckSpecialTarget(char *, int soldiernum) const; //This is in specials.cpp
		
		//Formation Phase methods
		int Engaged(Army *itsarmy) const;
		void Sort(Army *itsarmy, Battle *b, int regtype);
		
		int NumMeleeAttacks() const;
		int NumRangedAttacks() const;
		float MeleeAttackLevel() const;
		float MeleeDefenceLevel() const;
		
		
  		
	private:	
	//ultimately, type, behind, flank and reserve could all be determined from number,
	//so could be got rid of to save 16*19*2 = 608 bytes of memory ;)
		SoldierPtr * pSoldiers;
		int nummen;
        int size;  //number of hits in the formation (including illusions).
		int lastkilled; //don't think this is used yet?
        int type; //should be constant after creation
        int behind; //should be constant after creation
        int canattack;
		int flank; //should be constant after creation
		int concealed; // whether formation is concealed (for flanking maneuvres only).
		int reserve;
		int number; //should be constant after creation. Don't think this is needed?
};
#endif

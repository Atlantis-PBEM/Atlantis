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

#include <functional>
#include <map>
using namespace std;

class Army;

#include "soldier1.h"
#include "formation1.h"
#include "unit.h"
#include "alist.h"
#include "items.h"
#include "object.h"
#include "shields.h"
#include "helper.h"

class Army
{
	public:
		Army(Unit *,AList *,int,int = 0);
		~Army();

		int NumAlive();  //includes illusions
		int NumNonIllusionsAlive();
		Soldier * GetSoldier(int soldiernum) const; //soldiernum should be from 0 to count-1.
		
		int Broken();
		void Reset();
		void ResetEngagements();
		int NumSpoilers() const;

		int CanAttack();
		Soldier * GetAttacker(int attackernum); //attackernum should be from 0 to CanAttack()-1. 
                                      //For some reason this can't be set constant - may want to check this if problems.
        
        int ShieldIsUseful(char const *special) const;
        int IsSpecialTarget(char const *special) const;
        int GetTarget(Army *attackers, int formation, int attackType, int *targetform, char const * special, Battle *b);                                      
		int DoAnAttack(char const *special, int numAttacks, int race, int attackType, int attackLevel, 
                  int flags, int weaponClass, char const *effect, int mountBonus, Army *attackers, int formation, Battle *b,
                  int strength = 1);

        AList armytext;
		void AddLine(const AString &);

		void WriteLosses(Battle *);
		int Lose(Battle *,ItemList *, int ass = 0);
		void Tie(Battle *);
		void Win(Battle *,ItemList *, int enemydead);
		int CanBeHealed();
		void DoHeal(Battle *, int enemydead);
		void DoHealLevel(Battle *,int,int useItems );
		void DoResurrect(Battle *);
		void AssassinationResurrect();
        void DoNecromancy(Battle *, int enemydead);
		void Regenerate(Battle *);
		void DoExperience(int enemydead = 0);

		void GetMonSpoils(ItemList *,int, int);

        void DowngradeShield(Shield *hi); //This is in shields.cpp
        void DoBindingAttack(Soldier *pAtt, Battle *b); //This is in specials.cpp
        void DoDragonBindingAttack(Soldier *pAtt, Battle *b, Army *atts);

		//Formation Engagement Details
		void CombineEngagements(int formfrom, int formto, Army *enemy);
		int GetMidRoundTarget(int formation, Army *enemy, Battle *b); ///returns formnumber of newly engaged enemy formation, or -1 if none.
		int GetMidRoundRangedTarget(int formation, Army *enemy); //returns formnumber of a randomg newly tryattacked enemy formation, or -1 if none.

		//Formation Phase
		void PenaltyToHit(int penalty);
		void SortFormations(Battle *b, int regtype);
		void MirrorEngagements(Army *enemy);
		void FlankFlankers(Battle *b, Army *enemy);
		void AssignFrontTargets(Battle *b, Army *enemy, int ass = 0);
		void SplitOverwhelmingFrontFormations(Battle *b, Army *enemy);
		void SplitOverwhelmingFlankingFormations(Battle *b, Army *enemy, int regtype);
		void SetCanAttack(Army *enemy);
		void ReservesIntercept(Army *enemy);
		void AssignRangedTargets(Army *enemy);

		int SizeFrontFliers();
		int SizeFrontRiders();
		int SizeBehind();

		float KillChance(float attack, float defence);
		void FlyingReservesMayFlank(Battle *b, Army *enemy);
		void RidingReservesMayFlank(Battle *b, Army *enemy);
		int GetFlankedTarget(Battle *b, Army *enemy, int formnum);
		void FlankersEngage(Battle *b, Army *enemy);
		
		void ClearEmptyEngagements();
		void SetTemporaryBonuses(Army *enemy);
		void DoEthnicMoraleEffects(Battle *b);
  		
		Unit * pLeader;
		ShieldList shields;
		int round;
		int tac;
		int taccontrol;
		int misassigned;
        int canride;	
		int count;
		int nonillusioncount; //used in win conditions
		int rangedbonus; //as for bonus in formations, but only affects ranged attacks made by army.
		int concealment;

		int hitstotal; // Number of hits at start of battle, excluding illusions.

	
		/* Formations Stuff */
		#define NUMFORMS 18
		Formation formations[NUMFORMS+1];
		int engagements[NUMFORMS][NUMFORMS];
        int sortedformations;
        
        #ifdef DEBUG
        void WriteEngagements(Battle *b);
        #endif
};

#endif

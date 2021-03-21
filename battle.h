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
#ifndef BATTLE_CLASS
#define BATTLE_CLASS

class Battle;

#include "astring.h"
#include "alist.h"
#include "fileio.h"
#include "army.h"
#include "items.h"
#include <vector>

enum {
	ASS_NONE,
	ASS_SUCC,
	ASS_FAIL
};

enum {
	BATTLE_IMPOSSIBLE,
	BATTLE_LOST,
	BATTLE_WON,
	BATTLE_DRAW
};

class BattlePtr : public AListElem
{
	public:
		Battle * ptr;
};

class Battle : public AListElem
{
	public:
		Battle();
		~Battle();

		void Report(Areport *,Faction *);
		void AddLine(const AString &);

		int Run(ARegion *, Unit *, AList *, Unit *, AList *, int ass,
				ARegionList *pRegs);
		void FreeRound(Army *,Army *, int ass = 0);
		void NormalRound(int,Army *,Army *);
		void DoAttack(int round, Soldier *a, Army *attackers, Army *def,
				int behind, int ass = 0, bool canAttackBehind = false, bool canAttackFromBehind = false);

		void GetSpoils(AList *,ItemList *, int);

		//
		// These functions should be implemented in specials.cpp
		//
		void UpdateShields(Army *);
		void DoSpecialAttack( int round, Soldier *a, Army *attackers,
				Army *def, int behind, int canattackback);

		void WriteSides(ARegion *,Unit *,Unit *,AList *,AList *,int,
				ARegionList *pRegs );

		// void WriteBattleStats(ArmyStats *);

		int assassination;
		Faction * attacker; /* Only matters in the case of an assassination */
		AString * asstext;
		AList text;
};

#endif

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
#include "army1.h"
#include "items.h"

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
	    BattlePtr();
		Battle * ptr;
};

class Battle : public AListElem
{
	public:
		Battle(ARegion *);
		~Battle();

		int Run(ARegion *, Unit *, AList *, Unit *, AList *, int ass,
				ARegionList *pRegs);
		void NormalRound(int round,Army *,Army *,int regtype, int bias = 0, int ambush=0, int assassination=0);
		void DoAttack(int round, Soldier *a, Army *attackers, Army *def,
				int ass = 0);
		void FormationsPhase(Army * a, Army * b, int regtype, int bias, int ambush, int ass); ///

		void GetSpoils(AList *,ItemList *, int);

		//
		// These functions should be implemented in specials.cpp
		//
		void UpdateShields(Army *a, Army *enemy);
		void DoBinding(Army *att, Army *def);
        void UpdateRoundSpells(Army * a, Army * b);		
        int GetRoundSpellLevel(Army *a, Army *b, int type, int spell, int antispell);
		void DoSpecialAttack( int round, Soldier *a, Army *attackers, Army *def);

		void WriteSides(ARegion *,Unit *,Unit *,AList *,AList *,int,
				ARegionList *pRegs );

        void WriteTerrainMessage(int regtype);
        void WriteAggressionMessage(Army *a, Army *b);
        void TransferMessages(Army *a, Army *b);
		void AddLine(const AString &);
		void Report(Areport *,Faction *);
		
		void WriteBattleSituation(Army *a, Army *b);
		AString WriteBattleFormation(char f, char g, int size);

		int assassination;
		Faction * attacker; /* Only matters in the case of an assassination */
		AString * asstext;
		AList text;
		
		//Used for statistics / times reports
		ARegion *region;
		int casualties;
		
};

#endif

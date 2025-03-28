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
#include "army.h"
#include "items.h"
#include "events.h"
#include <vector>

class Location;

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

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

class Battle
{
	public:
		Battle();
		~Battle();

		void build_json_report(json &j, Faction *fac);
		void AddLine(const AString &);

		int Run(
			Events* events, ARegion *region, Unit *att, std::list<Location *>& atts,
			Unit *tar, std::list<Location *>& defs, int ass
		);
		void FreeRound(Army *,Army *, int ass = 0);
		void NormalRound(int,Army *,Army *);
		void DoAttack(int round, Soldier *a, Army *attackers, Army *def,
				int behind, int ass = 0, bool canAttackBehind = false, bool canAttackFromBehind = false);

		void GetSpoils(std::list<Location *>& losers, ItemList& spoils, int ass);

		//
		// These functions should be implemented in specials.cpp
		//
		void UpdateShields(Army *);
		void DoSpecialAttack( int round, Soldier *a, Army *attackers,
				Army *def, int behind, int canattackback);

		void WriteSides(
			ARegion * r, Unit * att, Unit * tar, std::list<Location *>& atts, std::list<Location *>& defs, int ass
		);

		// void WriteBattleStats(ArmyStats *);

		int assassination;
		Faction * attacker; /* Only matters in the case of an assassination */
		std::string asstext;
		std::vector<std::string> text;
};

#endif

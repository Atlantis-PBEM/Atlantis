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

#include "production.h"
#include "gameio.h"
#include "items.h"
#include "skills.h"
#include "gamedata.h"

using namespace std;

Production::Production()
{
	itemtype = -1;
	amount = 0;
	baseamount = 0;
	productivity = 0;
	skill = -1;
}

Production::Production(int it, int maxamt)
{
	itemtype = it;
	amount = maxamt;
	if (Globals->RANDOM_ECONOMY)
		amount += getrandom(maxamt);
	baseamount = amount;
	productivity = 10;
	AString skname = ItemDefs[it].pSkill;
	skill = LookupSkill(&skname);
}

void Production::write_out(ostream& f)
{
	f << (itemtype == -1 ? "NO_ITEM" : ItemDefs[itemtype].abr) << '\n';	
	f << amount << '\n';
	f << baseamount << '\n';
	if (itemtype == I_SILVER) {
		f << (skill == -1 ? "NO_SKILL" : SkillDefs[skill].abbr) << '\n';
	}
	f << productivity << '\n';
}

void Production::read_in(istream& f)
{
	AString temp;

	f >> ws >> temp;
	itemtype = LookupItem(&temp);

	f >> amount;
	f >> baseamount;

	if (itemtype == I_SILVER)
		f >> ws >> temp;
	else
		temp = AString(ItemDefs[itemtype].pSkill);

	skill = LookupSkill(&temp);

	f >> productivity;
}

string Production::write_report() {
	return ItemString(itemtype, amount);
}

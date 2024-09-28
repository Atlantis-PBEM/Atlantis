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

#include "market.h"
#include "items.h"
#include "gameio.h"
#include "gamedata.h"

void Market::post_turn(int population, int wages)
{
	// Nothing to do to the markets.
	if (!(Globals->VARIABLE_ECONOMY)) return;

	//
	// Unlimited, unchanging market.
	//
	if (amount == -1) return;

	if (ItemDefs[item].type & IT_MAN) {
		float ratio = ItemDefs[item].baseprice / (10 * (float)Globals->BASE_MAN_COST);
		// hack: included new wage factor of ten in float assignment above
		price = (int)((float) wages * 4 * ratio);
		if (ItemDefs[item].type & IT_LEADER)
			amount = population / 125;
		else
			amount = population / 25;
		return;
	}

	int tarprice = price;
	if (amount) {
		int fluctuation = (baseprice * activity)/amount;
		if (type == M_BUY)
			tarprice = (2 * baseprice + fluctuation) / 2;
		else
			tarprice = (3 * baseprice - fluctuation) / 2;
	}
	price = price + (tarprice - price) / 5;

	if (population <= minpop)
		amount = minamt;
	else if (population >= maxpop)
		amount = maxamt;
	else {
		amount = minamt + ((maxamt - minamt) * (population - minpop)) / (maxpop - minpop);
	}
}

void Market::write_out(std::ostream& f)
{
	f << type << '\n';
	f << (item == -1 ? "NO_ITEM" : ItemDefs[item].abr) << '\n';
	f << price << '\n';
	f << amount << '\n';
	f << minpop << '\n';
	f << maxpop << '\n';
	f << minamt << '\n';
	f << maxamt << '\n';
	f << baseprice << '\n';
}

void Market::read_in(std::istream& f)
{
	AString temp;

	int t;
	f >> t;

	type = MarketType{t};

	f >> std::ws >> temp;
	item = LookupItem(&temp);

	f >> price;
	f >> amount;
	f >> minpop;
	f >> maxpop;
	f >> minamt;
	f >> maxamt;
	f >> baseprice;
}

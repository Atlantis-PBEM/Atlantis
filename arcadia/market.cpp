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

Market::Market()
{
	activity = 0;
}

Market::Market(int a, int b, int c, int d, int e, int f, int g, int h)
{
	type = a;
	item = b;
	price = c;
	amount = d;
	minpop = e;
	maxpop = f;
	minamt = g;
	maxamt = h;
	baseprice = price;
	activity = 0;
}

void Market::PostTurn(int population, int wages, int race)
{
	// Nothing to do to the markets.
	if(!(Globals->VARIABLE_ECONOMY)) return;

	//
	// Unlimited, unchanging market.
	//
	if (amount == -1) return;

	if (ItemDefs[item].type & IT_MAN) {
		float ratio = ItemDefs[item].baseprice /
			(float)Globals->BASE_MAN_COST;
		if(wages < 10) wages = 10;   //minimum cost of $40 for normal race.
		price = (int)(wages * 4 * ratio);
		if (ItemDefs[item].type & IT_LEADER)
			amount = population / (25*Globals->POP_LEVEL);
		else {
			amount = population / (5*Globals->POP_LEVEL);
			item = race; //allows for race of region to change.
		}
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
	else {
		if (population >= maxpop)
			amount = maxamt;
		else {
			amount = minamt + ((maxamt - minamt) *
					(population - minpop)) /
				(maxpop - minpop);
		}
	}
}

void Market::Writeout(Aoutfile *f)
{
	f->PutInt(type);
	if (item != -1) f->PutStr(ItemDefs[item].abr);
	else f->PutStr("NO_ITEM");
	f->PutInt(price);
	f->PutInt(amount);
	f->PutInt(minpop);
	f->PutInt(maxpop);
	f->PutInt(minamt);
	f->PutInt(maxamt);
	f->PutInt(baseprice);
}

void Market::Readin(Ainfile *f)
{
	AString *temp;
	type = f->GetInt();

	temp = f->GetStr();
	item = LookupItem(temp);
	delete temp;

	price = f->GetInt();
	amount = f->GetInt();
	minpop = f->GetInt();
	maxpop = f->GetInt();
	minamt = f->GetInt();
	maxamt = f->GetInt();
	baseprice = f->GetInt();
}

AString Market::Report()
{
	AString temp;
	temp += ItemString(item, amount) + " at $" + price;
	return temp;
}

void MarketList::PostTurn(int population, int wages, int race)
{
	forlist(this) {
		((Market *) elem)->PostTurn(population, wages, race);
	}
}

void MarketList::Writeout(Aoutfile *f)
{
	f->PutInt(Num());
	forlist (this) ((Market *) elem)->Writeout(f);
}

void MarketList::Readin(Ainfile *f)
{
	int n = f->GetInt();
	for (int i=0; i<n; i++) {
		Market *m = new Market;
		m->Readin(f);
		Add(m);
	}
}

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
// MODIFICATIONS
// Date        Person            Comments
// ----        ------            --------
// 2000/SEP/06 Joseph Traub      Added base man cost to allow races to have
//                               different base costs
#include "market.h"
#include "items.h"
#include "gameio.h"
#include "rules.h"

Market::Market() {
  activity = 0;
}

Market::Market(int a,int b,int c,int d,int e,int f,int g,int h) {
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

void Market::PostTurn(int population,int wages)
{
    if( !( Globals->VARIABLE_ECONOMY ))
    {
        //
        // Nothing to do to the markets.
        //
        return;
    }

    //
    // Unlimited, unchanging market.
    //
    if (amount == -1) return;

    if (ItemDefs[item].type & IT_MAN)
    {
		float ratio = ItemDefs[item].baseprice / (float)Globals->BASE_MAN_COST;
        price = (int)(wages * 4 * ratio);
        if (item == I_LEADERS)
            amount = population / 25;
        else
            amount = population / 5;
        return;
    }

    int tarprice = price;
    if (amount)
    {
        if (type == M_BUY)
        {
            tarprice = (2 * baseprice + (baseprice * activity) / amount) / 2;
        } 
        else
        {
            tarprice = (3 * baseprice - (baseprice * activity) / amount) / 2;
        }
    }
    price = price + (tarprice - price) / 5;

    if (population <= minpop)
    {
        amount = minamt;
    } 
    else
    {
        if (population >= maxpop)
        {
            amount = maxamt;
        }
        else
        {
            amount = minamt +
                ((maxamt - minamt) * (population - minpop)) /
                (maxpop - minpop);
        }
    }
}

void Market::Writeout(Aoutfile * f) {
#ifdef DEBUG_GAME
  f->PutStr("Market");
#endif
  f->PutInt(type);
  f->PutInt(item);
  f->PutInt(price);
  f->PutInt(amount);
  f->PutInt(minpop);
  f->PutInt(maxpop);
  f->PutInt(minamt);
  f->PutInt(maxamt);
  f->PutInt(baseprice);
}

void Market::Readin(Ainfile * f) {
#ifdef DEBUG_GAME
  delete f->GetStr();
#endif
  type = f->GetInt();
  item = f->GetInt();
  price = f->GetInt();
  amount = f->GetInt();
  minpop = f->GetInt();
  maxpop = f->GetInt();
  minamt = f->GetInt();
  maxamt = f->GetInt();
  baseprice = f->GetInt();
/* XXXXX - Take this out!!! */
#ifdef NEVER
  if (ItemDefs[item].type & IT_TRADE && amount >= 0) {
    minamt *= 10;
    maxamt *= 10;
    amount *= 10;
  }
#endif
}

AString Market::Report() {
  AString temp;
  temp += ItemString(item,amount) + " at $" + price;
  return temp;
}

void MarketList::PostTurn(int population,int wages) {
  forlist(this) {
    ((Market *) elem)->PostTurn(population,wages);
  }
}

void MarketList::Writeout(Aoutfile * f) {
#ifdef DEBUG_GAME
  f->PutStr("MarketList");
#endif
  f->PutInt(Num());
  forlist (this)
    ((Market *) elem)->Writeout(f);
}

void MarketList::Readin(Ainfile * f) {
#ifdef DEBUG_GAME
  delete f->GetStr();
#endif
  int n = f->GetInt();
  for (int i=0; i<n; i++) {
    Market * m = new Market;
    m->Readin(f);
    Add(m);
  }
}

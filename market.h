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
#ifndef MARKET_CLASS
#define MARKET_CLASS

#include "alist.h"
#include "fileio.h"

enum {
  M_BUY,
  M_SELL
};

class Market : public AListElem {
public:
  Market();

  /* type, item, price, amount, minpop, maxpop, minamt, maxamt */
  Market(int,int,int,int,int,int,int,int);

  int type;
  int item;
  int price;
  int amount;
  
  int minpop;
  int maxpop;
  int minamt;
  int maxamt;

  int baseprice;
  int activity;

  void PostTurn(int,int);
  void Writeout(Aoutfile * f);
  void Readin(Ainfile * f);
  AString Report();
};

class MarketList : public AList {
public:
  void PostTurn(int,int);
  void Writeout(Aoutfile * f);
  void Readin(Ainfile * f);
};

#endif

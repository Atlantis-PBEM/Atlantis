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
#include "items.h"
#include "skills.h"
#include "gamedata.h"

int ParseItem(AString * token)
{
    for (int i=0; i<NITEMS; i++)
    {
        if (ItemDefs[i].type & IT_MONSTER &&
            ItemDefs[i].index == MONSTER_ILLUSION) { 
            if (*token == AString("i") + ItemDefs[i].name) return i;
            if (*token == AString("i") + ItemDefs[i].names) return i;
            if (*token == AString("i") + ItemDefs[i].abr) return i;
        } else {
            if (*token == ItemDefs[i].name) return i;
            if (*token == ItemDefs[i].names) return i;
            if (*token == ItemDefs[i].abr) return i;
        }
    }
    return -1;
}

AString ItemString(int type, int num) {
  AString temp;
  if (num == 1) {
    temp += AString(ItemDefs[type].name) + " [" + ItemDefs[type].abr + "]";
  } else {
    if (num == -1) {
      temp += AString("unlimited ") + ItemDefs[type].names + " [" +
      ItemDefs[type].abr + "]";
    } else {
      temp += AString(num) + " " + ItemDefs[type].names + " [" +
	ItemDefs[type].abr + "]";
    }
  }
  return temp;
}	

AString ItemDescription(int item) {
  AString temp = AString(ItemDefs[item].name) + " [" +
    ItemDefs[item].abr + "], weight " + ItemDefs[item].weight;

  if (ItemDefs[item].walk) {
    temp += AString(", capacity ") + (ItemDefs[item].walk
				      - ItemDefs[item].weight);
  }

  if (ItemDefs[item].fly) {
    temp += ", can fly";
  } else {
    if (ItemDefs[item].ride) temp += ", can ride";
  }

  if (ItemDefs[item].type & IT_MAN) temp += ", is a man";
  if (ItemDefs[item].type & IT_MONSTER) temp += ", is a creature";

  temp += ".";
  return temp;
}

int IsSoldier(int item) {
  if (ItemDefs[item].type & IT_MAN || ItemDefs[item].type & IT_MONSTER)
    return 1;
  return 0;
}

Item::Item() {
}

Item::~Item() {
}

AString Item::Report(int seeillusions)
{
    AString ret = ItemString(type,num);
    if (seeillusions && ItemDefs[type].type & IT_MONSTER &&
        ItemDefs[type].index == MONSTER_ILLUSION) {
        ret = ret + " (illusion)";
    }
    return ret;
}

void Item::Writeout(Aoutfile * f) {
  f->PutInt(type);
  f->PutInt(num);
}

void Item::Readin(Ainfile * f) {
  type = f->GetInt();
  num = f->GetInt();
}

void ItemList::Writeout(Aoutfile * f) {
  f->PutInt(Num());
  forlist (this)
    ((Item *) elem)->Writeout(f);
}

void ItemList::Readin(Ainfile * f) {
  int i = f->GetInt();
  for (int j=0; j<i; j++) {
    Item * temp = new Item;
    temp->Readin(f);
    if (temp->num < 1) {
      delete temp;
    } else {
      Add(temp);
    }
  }
}

int ItemList::GetNum(int t) {
  forlist(this) {
    Item * i = (Item *) elem;
    if (i->type == t) return i->num;
  }
  return 0;
}

int ItemList::Weight() {
  int wt = 0;
  forlist(this) {
    Item * i = (Item *) elem;
    wt += ItemDefs[i->type].weight * i->num;
  }
  return wt;
}

AString ItemList::Report(int obs,int seeillusions,int nofirstcomma)
{
    AString temp;
    forlist(this) {
        Item * i = (Item *) elem;
        if (obs == 2)
        {
            if (nofirstcomma)
            {
                nofirstcomma = 0;
            } 
            else
            {
                temp += ", ";
            }
            temp += i->Report(seeillusions);
        } 
        else 
        {
            if (ItemDefs[i->type].weight)
            {
                if (nofirstcomma) 
                {
                    nofirstcomma = 0;
                } 
                else
                {
                    temp += ", ";
                }
                temp += i->Report(seeillusions);
            }
        }
    }
    return temp;
}

AString ItemList::BattleReport()
{
    AString temp;
    forlist(this) {
        Item * i = (Item *) elem;
        if (ItemDefs[i->type].combat) {
            temp += ", ";
            temp += i->Report(0);
            if (ItemDefs[i->type].type & IT_MONSTER) {
                MonType & mondef = MonDefs[ItemDefs[i->type].index];
                temp += AString(" (Combat ") + mondef.attackLevel + 
					"/" + mondef.combatDefense + ", Attacks " +
					mondef.numAttacks + ", Hits " + mondef.hits +
					", Tactics " + mondef.tactics + ")";
            }
        }
    }
    return temp;
}

void ItemList::SetNum(int t,int n)
{
    if (n)
    {
        forlist(this) {
            Item * i = (Item *) elem;
            if (i->type == t)
            {
                i->num = n;
                return;
            }
        }
        Item * i = new Item;
        i->type = t;
        i->num = n;
        Add(i);
    } 
    else
    {
        forlist(this) {
            Item * i = (Item *) elem;
            if (i->type == t)
            {
                Remove(i);
                delete i;
                return;
            }
        }
    }
}

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
#include "alist.h"

AListElem::~AListElem() {
}

AList::AList() {
  list = 0;
  lastelem = 0;
  num = 0;
}

AList::~AList() {
  DeleteAll();
}

void AList::DeleteAll() {
  AListElem * temp;
  while (list) {
    temp = list->next;
    delete list;
    list = temp;
  }
  lastelem = 0;
  num = 0;
}

void AList::Empty() {
  AListElem * temp;
  while (list) {
    temp = list->next;
    list->next = 0;
    list = temp;
  }
  lastelem = 0;
  num = 0;
}

void AList::Insert(AListElem * e) {
  num ++;
  e->next = list;
  list = e;
  if (!lastelem) lastelem = list;
}

void AList::Add(AListElem * e) {
  num ++;
  if (list) {
    lastelem->next = e;
    e->next = 0;
    lastelem = e;
  } else {
    list = e;
    e->next = 0;
    lastelem = list;
  }
}

AListElem * AList::Next(AListElem * e) {
  if (!e) return 0;
  return e->next;
}

AListElem * AList::First() {
  return list;
}

AListElem * AList::Get(AListElem * e) {
  AListElem * temp = list;
  while (temp) {
    if (temp == e) return temp;
    temp = temp->next;
  }
  return 0;
}

char AList::Remove(AListElem * e) {
  AListElem * temp = list;
  if (!e) return 0;
  if (!list) return 0;
  if (list==e) {
    list=list->next;
    num--;
    if (lastelem == e) lastelem = 0;
    return 1;
  }
  
  AListElem * p = list;
  temp = list->next;
  while (temp) {
    if (temp==e) {
      p->next = temp->next;
      num--;
      if (p->next == 0) lastelem = p;
      return 1;
    }
    p=temp;
    temp=temp->next;
  }
  return 0;
}
		
int AList::Num() {
  return num;
}

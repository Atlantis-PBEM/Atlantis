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
#ifndef ALIST_CLASS
#define ALIST_CLASS

class AListElem;
class AList;

class AListElem {
public:
  virtual ~AListElem();
  
  AListElem * next;
};

class AList {
public:
  AList();
  ~AList();
  void DeleteAll();
  void Empty(); /* Clears the list without deleting members */
  
  AListElem * Get(AListElem *);
  char Remove(AListElem *);
  void Insert(AListElem *); /* into the front */
  void Add(AListElem *); /* to the back */
  AListElem * Next(AListElem *);
  AListElem * First();
  int Num();
private:
  AListElem *list;
  AListElem *lastelem;
  int num;
};

#define forlist(l)	AListElem * elem, * _elem2; \
      for ( elem=(l)->First(), \
      _elem2 = (elem ? (l)->Next(elem) : 0); \
      elem; \
      elem = _elem2, \
      _elem2 = (_elem2 ? ((l)->Next(_elem2)) : 0))

#endif

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

/// This represents an element of a list.
/** 
AListElem is a virtual class, which will be set to whatever the right type is.
It has a 'next' variable, and all the rest of the usual list stuff.
*/
class AListElem {
	public:
		virtual ~AListElem();

		AListElem * next;	///< The next element in the list
};

/// A standard list
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

		/* Helper function for forlist_safe */
		int NextLive(AListElem **copy, int size, int pos);

	private:
		AListElem *list;		///< The first element of the list
		AListElem *lastelem;	///< The last element of the list
		int num;
};

/// Iterate over a list
#define forlist(l) \
	AListElem * elem, * _elem2; \
	for (elem=(l)->First(), \
			_elem2 = (elem ? (l)->Next(elem) : 0); \
			elem; \
			elem = _elem2, \
			_elem2 = (_elem2 ? ((l)->Next(_elem2)) : 0))

/// Iterate over a list, if we've already done so.
#define forlist_reuse(l) \
	for (elem=(l)->First(), \
			_elem2 = (elem ? (l)->Next(elem) : 0); \
			elem; \
			elem = _elem2, \
			_elem2 = (_elem2 ? ((l)->Next(_elem2)) : 0))

/// Iterate over a list (without messing it up?)
#define forlist_safe(l) \
	int size = (l)->Num(); \
	AListElem **copy = new AListElem*[size]; \
	AListElem *elem; \
	int pos; \
	for (pos = 0, elem = (l)->First(); elem; elem = elem->next, pos++) { \
		copy[pos] = elem; \
	} \
	for (pos = 0; \
			pos < size ? (elem = copy[pos], 1) : (delete [] copy, 0); \
			pos = (l)->NextLive(copy, size, pos))

#endif

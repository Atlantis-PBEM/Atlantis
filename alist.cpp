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

/// A destructor for a list elem.
/** I think this will be in addition to whatever destructors apply for the element.
*/
AListElem::~AListElem()
{
}

/// A constructor for the AList class
AList::AList()
{
	list = 0;
	lastelem = 0;
	num = 0;
}

/// A destructor for the Alist class
/* This just calls the DeleteAll class */
AList::~AList()
{
	DeleteAll();
}

/// Delete every element in a list.
/* Start at the start, and delete every element, then set num and lastelement to 0

*/
void AList::DeleteAll()
{
	AListElem * temp; ///< A temporary variable to contain the current list element.
								 ///  Otherwise, when we delete list, we lose list->next too!
	while (list) {
		temp = list->next;
		delete list;
		list = temp;
	}
	lastelem = 0;
	num = 0;
}

/// Set every element in a list to 0
/** Note that these are lists of things which may be managed in a separate area
(eg. Units) so we don't want to delete the list contents, just where they point to.
*/
void AList::Empty()
{
	AListElem * temp; ///< A temporary variable to contain the current list element.
								 ///  Otherwise, when we delete list, we lose list->next too!
	while (list) {
		temp = list->next;
		list->next = 0;
		list = temp;
	}
	lastelem = 0;
	num = 0;
}

/// Insert an AListElem at the start of the list.
void AList::Insert(AListElem * e)
{
	num ++;		///<Increment the number of elements in the list
	e->next = list;
	list = e;
	if (!lastelem) lastelem = list;
}

/// Add an AListElem to the end of the list
void AList::Add(AListElem * e)
{
	num ++;
	if (list) {	// if list isn't empty
		lastelem->next = e;
		e->next = 0;
		lastelem = e;
	} else {	// if list is empty
		list = e;
		e->next = 0;
		lastelem = list;
	}
}

/// Get the next AListElem from this one.
AListElem * AList::Next(AListElem * e)
{
	if (!e) return 0;
	return e->next;
}

/// Return the first item in the list
AListElem * AList::First()
{
	return list;
}

/// Return an item from the list, or return zero if it isn't in there.
AListElem * AList::Get(AListElem * e)
{
	AListElem * temp = list;
	while (temp) {
		if (temp == e) return temp;
		temp = temp->next;
	}
	return 0;
}

/// Remove an element from the list... I think
/**This one looks like deep voodoo, and I can't follow it at all :\
*/
char AList::Remove(AListElem * e)
{
	if (!e) return 0;
	if (!e->next) lastelem = 0;

	for (AListElem **pp = &list; *pp; pp = &((*pp)->next)) {
		if (*pp == e) {
			*pp = e->next;
			num--;
			return 1;
		}
		if (!e->next) lastelem = *pp;
	}
	return 0;
}

/// Return the number of elements in the list
int AList::Num()
{
	return num;
}

/// No idea what this one does.
int AList::NextLive(AListElem **copy, int size, int pos)
{
	while (++pos < size) {
		for (AListElem *elem = First(); elem; elem = elem->next) {
			if (elem == copy[pos]) return pos;
		}
	}
	return pos;
}

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
#ifndef ASTRING_CLASS
#define ASTRING_CLASS

#include <iostream>
#include "alist.h"

using namespace std;

class AString : public AListElem {
	friend ostream & operator <<(ostream &os, const AString &);
	friend istream & operator >>(istream &is, AString &);
public:

	AString();
	AString(char *);
	AString(const char *);
	AString(int);
	AString(unsigned int);
	AString(char);
	AString(const AString &);
	~AString();

	int operator==(const AString &);
	int operator==(char *);
	int operator==(const char *);
	int CheckPrefix(const AString &);
	AString operator+(const AString &);
	AString & operator+=(const AString &);

	AString & operator=(const AString &);
	AString & operator=(const char *);

	char *Str();
	int Len();

	AString *gettoken();
	int getat();
	AString *getlegal();
	AString *Trunc(int, int back=30);
	int value();
	int strict_value();
	AString *StripWhite();

private:

	int len;
	char *str;
	int isEqual(const char *);
};

#endif

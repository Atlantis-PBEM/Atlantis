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
#include <iostream>
#include <fstream>

using namespace std;

#include "gameio.h"

#define ENDLINE '\n'
char buf[256];

void cleartoendl()
{
	char ch = ' ';
	while (!(cin.eof()) && (ch != ENDLINE))
	{
		ch = cin.get();
	}
}

int clamp(int imin, int ivalue, int imax) {
	return max(imin, min(ivalue, imax));
}

int Agetint()
{
	int x;
	cin >> x;
	cleartoendl();
	return x;
}

void Awrite(const AString & s)
{
	cout << s << ENDLINE;
}

void Adot()
{
	cout << ".";
}


AString * getfilename(const AString & s)
{
	cout << s;
	return( AGetString() );
}

AString *AGetString()
{
	cin.getline( buf, 256, ENDLINE );
	return( new AString( buf ));
}

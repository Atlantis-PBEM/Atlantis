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
#include "gamedefs.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "i_rand.h"

static randctx isaac_ctx;

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

void initIO()
{
	seedrandom( 1783 );
}

void doneIO()
{
}

int getrandom(int range)
{
	int neg = (range < 0);
	if (!range) return 0;
	int ret = 0;
	if (neg) range = -range;
	unsigned long i = isaac_rand( &isaac_ctx );
	i = i % range;
	if (neg) ret = (int)(i*-1);
	else ret = (int)i;
	return ret;
}

void seedrandom(int num)
{
	ub4 i;
	isaac_ctx.randa = isaac_ctx.randb = isaac_ctx.randc = (ub4)0;
	for (i=0; i<256; ++i)
	{
		isaac_ctx.randrsl[i]=(ub4)num+i;
	}
	randinit( &isaac_ctx, TRUE );
}

void seedrandomrandom()
{
	seedrandom( time( 0 ) );
}

int makeRoll(int rolls, int sides) {
	int result = 0;
	for (int i = 0; i < rolls; i++) {
		result += getrandom(sides) + 1;
	}

	return result;
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

void message(char * c)
{
	cout << c << ENDLINE;
	morewait();
}

void morewait()
{
	cout << ENDLINE;
	cin.getline(buf,256,ENDLINE);
	cout << ENDLINE;
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

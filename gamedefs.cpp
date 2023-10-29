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
#include "gamedefs.h"
#include <string>

char const *dr[] = {
	"North",
	"Northeast",
	"Southeast",
	"South",
	"Southwest",
	"Northwest"
};

char const ** DirectionStrs = dr;

char const *da[] = {
	"N",
	"NE",
	"SE",
	"S",
	"SW",
	"NW"
};

char const ** DirectionAbrs = da;

char const *mn[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

char const ** MonthNames = mn;

std::string SeasonNames[] = {
	"clear",
	"winter",
	"monsoon season",
	"blizzard"
};

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
#ifndef PRODUCTION_CLASS
#define PRODUCTION_CLASS

#include <iostream>

class Production {
public:
	Production(int type, int amount);
    Production() {};

	void write_out(std::ostream& f);
	void read_in(std::istream& f);
	std::string write_report();

	int itemtype = -1;
	int baseamount = 0;
	int amount = 0;
	int skill = -1;
	int productivity = 0;
	int activity = 0;
};

#endif

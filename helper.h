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
#ifndef HELPER
#define HELPER

//
// Version macros
//
typedef unsigned int ATL_VER;

#define MAKE_ATL_VER(x, y, z) ((x << 0x10) + (y << 0x8) + z)

#define ATL_VER_MAJOR(x) ((x >> 0x10) % 0x100)
#define ATL_VER_MINOR(x) ((x >> 0x8) % 0x100)
#define ATL_VER_PATCH(x) (x % 0x100)

#define ATL_MJR_STR(x) (std::to_string(ATL_VER_MAJOR(x)))
#define ATL_MNR_STR(x) (std::to_string(ATL_VER_MINOR(x)))
#define ATL_PTCH_STR(x) (std::to_string(ATL_VER_PATCH(x)))
#define ATL_VER_STR(x) ATL_MJR_STR(x) + "." + ATL_MNR_STR(x) + "." + ATL_PTCH_STR(x)
#define ATL_VER_STRING(x) ATL_VER_STR(x) + (ATL_VER_MINOR(x) % 2 ? "" : " (beta)")

#endif

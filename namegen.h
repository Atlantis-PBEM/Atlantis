// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 2022 Valdis Zobēla
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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

#pragma once

#include "object.h"
#include "items.h"

#include <string>

std::string getAbstractName(const int seed);
std::string getEthnicName(const int seed, const Ethnicity etnos);
std::string getObjectName(const int seed, const int typeIndex, const ObjectType& type);
std::string getRegionName(const int seed, const Ethnicity etnos, const int type, const int size, const bool island);
std::string getRiverName(const int seed, const int size, const int min, const int max);

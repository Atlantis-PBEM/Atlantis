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
// MODIFICATIONS
// Date        Person            Comments
// ----        ------            --------
// 2000/MAR/21 Azthar Septragen  Added roads.
#ifndef OBJECT_CLASS
#define OBJECT_CLASS

class Object;

#include "alist.h"
#include "fileio.h"
#include "gamedefs.h"
#include "faction.h"

#define I_WOOD_OR_STONE -2

class ObjectType {
public:
  char * name;
  int protect;
  int capacity;
  int item;
  int cost;
  int level;
  int skill;
  int sailors;
  int canenter;
  int monster;
  int production;
};

extern ObjectType * ObjectDefs;

int ParseObject(AString *);

int ObjectIsShip(int);

class Object : public AListElem
{
public:
    Object( ARegion *region );
    ~Object();
  
    void Readin( Ainfile *f, AList *, ATL_VER v );
    void Writeout( Aoutfile *f );
    void Report(Areport *,Faction *,int,int,int);
  
    void SetName(AString *);
    void SetDescribe(AString *);
  
    Unit *GetUnit(int);
    Unit *GetUnitAlias(int,int); /* alias, faction number */
    Unit *GetUnitId(UnitId *,int);

    // AS
    int IsRoad();
    int IsRoadUsable();
    int IsRoadDecaying();
    AString DoDecayWarning();
    AString DoMaintenanceWarning();

    int IsBoat();
    int IsBuilding();
    int CanModify();
    int CanEnter(ARegion *,Unit *);
    Unit *ForbiddenBy(ARegion *, Unit *);
    Unit *GetOwner();

    void MoveObject( ARegion *toreg );

    AString * name;
    AString * describe;
    ARegion *region;
    int inner;
    int num;
    int type;
    int incomplete;
    int capacity;
    int runes;
    AList units;
};

#endif

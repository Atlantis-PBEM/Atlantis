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

#ifndef OBJECT_CLASS
#define OBJECT_CLASS

class Hexside;
class Object;

#include "../alist.h"
#include "fileio.h"
#include "gamedefs.h"
#include "faction.h"
#include "items.h"

#define I_WOOD_OR_STONE -2

class HexsideType {
	public:
		char *name;
		enum {
			DISABLED	= 0x001,
			CANMODIFY	= 0x020
		};
		int flags;

		int item;
		int cost;
		char *skill;
		int level;	

		int sailable;
		int movementmultiplier;
		int blockeffect;
		int stealthpen;
		int advancepen;
		
		int exclusive;
};		

extern HexsideType *HexsideDefs;

AString *HexsideDescription(int type);

int ParseHexsideDir(AString *);
int ParseHexside(AString *);

class Hexside
{
	public:
		Hexside();
		~Hexside();

		void Readin(Ainfile *f);
		void Writeout(Aoutfile *f);

		int type;   //exclusive types, cannot be built. Value indicates type which is present.
		int bridge; //non-exclusive types. Value of -1 indicates finished, 0 not present, positive value is amount needed to finish.
		int road;   //non-exclusive types.
		int harbour; //exclusive type, so if harbour gets completed, then is reset to zero and type = harbour. can only be built on a beach.
//currently the program is not writing out / reading in harbours.
};


class ObjectType {
	public:
		char *name;
		enum {
			DISABLED	= 0x001,
			NOMONSTERGROWTH	= 0x002,
			NEVERDECAY	= 0x004,
			CANENTER	= 0x008,
			CANMODIFY	= 0x020,
			TRANSPORT	= 0x040,
			OCEANBUILD  = 0x080,
			SAILOVERLAND = 0x100
		};
		int flags;

		int protect;
		int capacity;
		int sailors;
		int maxMages;
		int speed;

		int item;
		int cost;
		char *skill;
		int level;

		int maxMaintenance;
		int maxMonthlyDecay;
		int maintFactor;

		int monster;

		int productionAided;
		int defenceArray[NUM_ATTACK_TYPES];
		
		int hexside;
		int sailable;
		int oceanbonus;
};

extern ObjectType *ObjectDefs;

AString *ObjectDescription(int obj);

int ParseObject(AString *);

int ObjectIsShip(int);

class Object : public AListElem
{
	public:
		Object(ARegion *region);
		~Object();

		void Readin(Ainfile *f, AList *, ATL_VER v);
		void Writeout(Aoutfile *f);
		void Report(Areport *, Faction *, int, int, int, int, int, int, int, int);

		void SetName(AString *);
		void SetDescribe(AString *);

		Unit *GetUnit(int);
		Unit *GetUnitAlias(int, int); /* alias, faction number */
		Unit *GetUnitId(UnitId *, int);

		// AS
		int IsRoad();

		int IsBoat();
		int IsBuilding();
		int CanModify();
		int CanEnter(ARegion *, Unit *);
		int GetPopulation();
		int Weight();
		Unit *ForbiddenBy(ARegion *, Unit *);
		Unit *GetOwner();

		void SetPrevDir(int);
		void MoveObject(ARegion *toreg);

		AString *name;
		AString *describe;
		ARegion *region;
		int inner;
		int num;
		int type;
		int incomplete;
		int capacity;
		int runes;
		int hexside;
		int prevdir;
		int mages;
		/* The following are needed only for arcadia portals */
		int mageowner; //needs to be saved
		int speedbonus;
		
		AList units;
};

#endif

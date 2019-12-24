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

class Object;

#include "alist.h"
#include "fileio.h"
#include "gamedefs.h"
#include "faction.h"
#include "items.h"
#include <map>

#define I_WOOD_OR_STONE -2

class ObjectType {
	public:
		char const *name;
		enum {
			DISABLED	= 0x001,
			NOMONSTERGROWTH	= 0x002,
			NEVERDECAY	= 0x004,
			CANENTER	= 0x008,
			CANMODIFY	= 0x020,
			TRANSPORT	= 0x040,
			GROUP		= 0x080
		};
		int flags;

		int protect;
		int capacity;
		int sailors;
		int maxMages;

		int item;
		int cost;
		char const *skill;
		int level;

		int maxMaintenance;
		int maxMonthlyDecay;
		int maintFactor;

		int monster;

		int productionAided;
		int defenceArray[NUM_ATTACK_TYPES];
};

extern ObjectType *ObjectDefs;

AString *ObjectDescription(int obj);

int LookupObject(AString *token);

int ParseObject(AString *, int ships);

int ObjectIsShip(int);

class Object : public AListElem
{
	public:
		Object(ARegion *region);
		~Object();

		void Readin(Ainfile *f, AList *, ATL_VER v);
		void Writeout(Aoutfile *f);
		void Report(Areport *, Faction *, int, int, int, int, int, int, int);

		void SetName(AString *);
		void SetDescribe(AString *);

		Unit *GetUnit(int);
		Unit *GetUnitAlias(int, int); /* alias, faction number */
		Unit *GetUnitId(UnitId *, int);

		// AS
		int IsRoad();

		int IsFleet();
		int IsBuilding();
		int CanModify();
		int CanEnter(ARegion *, Unit *);
		Unit *ForbiddenBy(ARegion *, Unit *);
		Unit *GetOwner();

		void SetPrevDir(int);
		void MoveObject(ARegion *toreg);
		
		// Fleets
		void ReadinFleet(Ainfile *f);
		void WriteoutFleet(Aoutfile *f);
		int CheckShip(int);
		int GetNumShips(int);
		void SetNumShips(int, int);
		void AddShip(int);
		AString FleetDefinition();
		int FleetCapacity();
		int FleetLoad();
		int SailThroughCheck(int dir);
		int FleetSailingSkill(int);
		int GetFleetSize();
		int GetFleetSpeed(int);
		
		AString *name;
		AString *describe;
		ARegion *region;
		int inner;
		int num;
		int type;
		int incomplete;
		int capacity;
		int flying;
		int load;
		int runes;
		int prevdir;
		int mages;
		int shipno;
		int movepoints;
		AList units;
		AList ships;
};

#endif

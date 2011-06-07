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

#include <stdio.h>
#include <string.h>
#include "game.h"
#include "gamedata.h"

Location *GetUnit(AList *list, int n)
{
	forlist(list) {
		Location *l = (Location *) elem;
		if (l->unit->num == n) return l;
	}
	return 0;
}

ARegionPtr *GetRegion(AList *l, int n)
{
	forlist(l) {
		ARegionPtr *p = (ARegionPtr *) elem;
		if (p->ptr->num == n) return p;
	}
	return 0;
}

Farsight::Farsight()
{
	faction = 0;
	unit = 0;
	level = 0;
	observation = 0;
	for (int i = 0; i < NDIRS; i++)
		exits_used[i] = 0;
}

Farsight *GetFarsight(AList *l, Faction *fac)
{
	forlist(l) {
		Farsight *f = (Farsight *) elem;
		if (f->faction == fac) return f;
	}
	return 0;
}

AString TownString(int i)
{
	switch (i) {
	case TOWN_VILLAGE:
		return "village";
	case TOWN_TOWN:
		return "town";
	case TOWN_CITY:
		return "city";
	}
	return "huh?";
}

TownInfo::TownInfo()
{
	name = 0;
	pop = 0;
	activity = 0;
	hab = 0;
}

TownInfo::~TownInfo()
{
	if (name) delete name;
}

void TownInfo::Readin(Ainfile *f, ATL_VER &v)
{
	name = f->GetStr();
	pop = f->GetInt();
	hab = f->GetInt();
}

void TownInfo::Writeout(Aoutfile *f)
{
	f->PutStr(*name);
	f->PutInt(pop);
	f->PutInt(hab);
}

ARegion::ARegion()
{
	name = new AString("Region");
	xloc = 0;
	yloc = 0;
	buildingseq = 1;
	gate = 0;
	gatemonth = 0;
	gateopen = 1;
	town = 0;
	development = 0;
	habitat = 0;
	immigrants = 0;
	emigrants = 0;
	improvement = 0;
	clearskies = 0;
	earthlore = 0;
	for (int i=0; i<NDIRS; i++)
		neighbors[i] = 0;
	visited = 0;
}

ARegion::~ARegion()
{
	if (name) delete name;
	if (town) delete town;
}

void ARegion::ZeroNeighbors()
{
	for (int i=0; i<NDIRS; i++) {
		neighbors[i] = 0;
	}
}

void ARegion::SetName(char const *c)
{
	if (name) delete name;
	name = new AString(c);
}


int ARegion::IsNativeRace(int item)
{
	TerrainType *typer = &(TerrainDefs[type]);
	int coastal = sizeof(typer->coastal_races)/sizeof(int);
	int noncoastal = sizeof(typer->races)/sizeof(int);
	if (IsCoastal()) {
		for (int i=0; i<coastal; i++) {
			if (item == typer->coastal_races[i]) return 1;
		}
	}
	for (int i=0; i<noncoastal; i++) {
		if (item == typer->races[i]) return 1;
	}
	return 0;
}

int ARegion::GetNearestProd(int item)
{
	AList regs, regs2;
	AList *rptr = &regs;
	AList *r2ptr = &regs2;
	AList *temp;
	ARegionPtr *p = new ARegionPtr;
	p->ptr = this;
	regs.Add(p);

	for (int i=0; i<5; i++) {
		forlist(rptr) {
			ARegion *r = ((ARegionPtr *) elem)->ptr;
			AString skname = ItemDefs[item].pSkill;
			int sk = LookupSkill(&skname);
			if (r->products.GetProd(item, sk)) {
				regs.DeleteAll();
				regs2.DeleteAll();
				return i;
			}
			for (int j=0; j<NDIRS; j++) {
				if (neighbors[j]) {
					p = new ARegionPtr;
					p->ptr = neighbors[j];
					r2ptr->Add(p);
				}
			}
			rptr->DeleteAll();
			temp = rptr;
			rptr = r2ptr;
			r2ptr = temp;
		}
	}
	regs.DeleteAll();
	regs2.DeleteAll();
	return 5;
}


void ARegion::LairCheck()
{
	// No lair if town in region
	if (town) return;


	TerrainType *tt = &TerrainDefs[type];

	if (!tt->lairChance) return;

	int check = getrandom(100);
	if (check >= tt->lairChance) return;

	int count = 0;
	unsigned int c;
	for (c = 0; c < sizeof(tt->lairs)/sizeof(int); c++) {
		if (tt->lairs[c] != -1) {
			if (!(ObjectDefs[tt->lairs[c]].flags & ObjectType::DISABLED)) {
				count++;
			}
		}
	}
	count = getrandom(count);

	int lair = -1;
	for (c = 0; c < sizeof(tt->lairs)/sizeof(int); c++) {
		if (tt->lairs[c] != -1) {
			if (!(ObjectDefs[tt->lairs[c]].flags & ObjectType::DISABLED)) {
				if (!count) {
					lair = tt->lairs[c];
					break;
				}
				count--;
			}
		}
	}

	if (lair != -1) {
		MakeLair(lair);
		return;
	}
}

void ARegion::MakeLair(int t)
{
	Object *o = new Object(this);
	o->num = buildingseq++;
	o->name = new AString(AString(ObjectDefs[t].name) + " [" + o->num + "]");
	o->type = t;
	o->incomplete = 0;
	o->inner = -1;
	objects.Add(o);
}

int ARegion::GetPoleDistance(int dir)
{
	int ct = 1;
	ARegion *nreg = neighbors[dir];
	while (nreg) {
		ct++;
		nreg = nreg->neighbors[dir];
	}
	return ct;
}

void ARegion::Setup()
{
	//
	// type and location have been setup, do everything else
	SetupProds();

	SetupPop();

	//
	// Make the dummy object
	//
	Object *obj = new Object(this);
	objects.Add(obj);

	if (Globals->LAIR_MONSTERS_EXIST)
		LairCheck();
}

int ARegion::TraceConnectedRoad(int dir, int sum, AList *con, int range, int dev)
{
	ARegionPtr *rn = new ARegionPtr();
	rn->ptr = this;
	int isnew = 1;
	forlist(con) {
		ARegionPtr *reg = (ARegionPtr *) elem;
		if ((reg) && (reg->ptr)) if (reg->ptr == this) isnew = 0;
	}
	if (isnew == 0) return sum;
	con->Add(rn);
	// Add bonus for connecting town
	if (town) sum++;
	// Add bonus if development is higher
	if (development > dev + 9) sum++;
	if (development * 2 > dev * 5) sum++;
	// Check further along road
	if (range > 0) {
		for (int d=0; d<NDIRS; d++) {
			if (!HasExitRoad(d)) continue;
			ARegion *r = neighbors[d];
			if (!r) continue;
			if (dir == r->GetRealDirComp(d)) continue;
			if (r->HasConnectingRoad(d)) sum = r->TraceConnectedRoad(d, sum, con, range-1, dev+2);
		}
	}
	return sum;
}

int ARegion::RoadDevelopmentBonus(int range, int dev)
{
	int bonus = 0;
	AList *con = new AList();
	ARegionPtr *rp = new ARegionPtr();
	rp->ptr = this;
	con->Add(rp);
	for (int d=0; d<NDIRS; d++) {
		if (!HasExitRoad(d)) continue;
		ARegion *r = neighbors[d];
		if (!r) continue;
		if (r->HasConnectingRoad(d)) bonus = r->TraceConnectedRoad(d, bonus, con, range-1, dev);
	}
	return bonus;	
}

// AS
void ARegion::DoDecayCheck(ARegionList *pRegs)
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		if (!(ObjectDefs[o->type].flags & ObjectType::NEVERDECAY)) {
			DoDecayClicks(o, pRegs);
		}
	}
}

// AS
void ARegion::DoDecayClicks(Object *o, ARegionList *pRegs)
{
	if (ObjectDefs[o->type].flags & ObjectType::NEVERDECAY) return;

	int clicks = getrandom(GetMaxClicks());
	clicks += PillageCheck();

	if (clicks > ObjectDefs[o->type].maxMonthlyDecay)
		clicks = ObjectDefs[o->type].maxMonthlyDecay;

	o->incomplete += clicks;

	if (o->incomplete > 0) {
		// trigger decay event
		RunDecayEvent(o, pRegs);
	}
}

// AS
void ARegion::RunDecayEvent(Object *o, ARegionList *pRegs)
{
	AList *pFactions;
	pFactions = PresentFactions();
	forlist (pFactions) {
		Faction *f = ((FactionPtr *) elem)->ptr;
		f->Event(GetDecayFlavor() + *o->name + " " +
				ObjectDefs[o->type].name + " in " +
				ShortPrint(pRegs));
	}
}

// AS
AString ARegion::GetDecayFlavor()
{
	AString flavor;
	int badWeather = 0;
	if (weather != W_NORMAL && !clearskies) badWeather = 1;
	if (!Globals->WEATHER_EXISTS) badWeather = 0;
	switch (type) {
		case R_PLAIN:
		case R_ISLAND_PLAIN:
		case R_CERAN_PLAIN1:
		case R_CERAN_PLAIN2:
		case R_CERAN_PLAIN3:
		case R_CERAN_LAKE:
			flavor = AString("Floods have damaged ");
			break;
		case R_DESERT:
		case R_CERAN_DESERT1:
		case R_CERAN_DESERT2:
		case R_CERAN_DESERT3:
			flavor = AString("Flashfloods have damaged ");
			break;
		case R_CERAN_WASTELAND:
		case R_CERAN_WASTELAND1:
			flavor = AString("Magical radiation has damaged ");
			break;
		case R_TUNDRA:
		case R_CERAN_TUNDRA1:
		case R_CERAN_TUNDRA2:
		case R_CERAN_TUNDRA3:
			if (badWeather) {
				flavor = AString("Ground freezing has damaged ");
			} else {
				flavor = AString("Ground thaw has damaged ");
			}
			break;
		case R_MOUNTAIN:
		case R_ISLAND_MOUNTAIN:
		case R_CERAN_MOUNTAIN1:
		case R_CERAN_MOUNTAIN2:
		case R_CERAN_MOUNTAIN3:
			if (badWeather) {
				flavor = AString("Avalanches have damaged ");
			} else {
				flavor = AString("Rockslides have damaged ");
			}
			break;
		case R_CERAN_HILL:
		case R_CERAN_HILL1:
		case R_CERAN_HILL2:
			flavor = AString("Quakes have damaged ");
			break;
		case R_FOREST:
		case R_SWAMP:
		case R_ISLAND_SWAMP:
		case R_JUNGLE:
		case R_CERAN_FOREST1:
		case R_CERAN_FOREST2:
		case R_CERAN_FOREST3:
		case R_CERAN_MYSTFOREST:
		case R_CERAN_MYSTFOREST1:
		case R_CERAN_MYSTFOREST2:
		case R_CERAN_SWAMP1:
		case R_CERAN_SWAMP2:
		case R_CERAN_SWAMP3:
		case R_CERAN_JUNGLE1:
		case R_CERAN_JUNGLE2:
		case R_CERAN_JUNGLE3:
			flavor = AString("Encroaching vegetation has damaged ");
			break;
		case R_CAVERN:
		case R_UFOREST:
		case R_TUNNELS:
		case R_CERAN_CAVERN1:
		case R_CERAN_CAVERN2:
		case R_CERAN_CAVERN3:
		case R_CERAN_UFOREST1:
		case R_CERAN_UFOREST2:
		case R_CERAN_UFOREST3:
		case R_CERAN_TUNNELS1:
		case R_CERAN_TUNNELS2:
		case R_CHASM:
		case R_CERAN_CHASM1:
		case R_GROTTO:
		case R_CERAN_GROTTO1:
		case R_DFOREST:
		case R_CERAN_DFOREST1:
			if (badWeather) {
				flavor = AString("Lava flows have damaged ");
			} else {
				flavor = AString("Quakes have damaged ");
			}
			break;
		default:
			flavor = AString("Unexplained phenomena have damaged ");
			break;
	}
	return flavor;
}

// AS
int ARegion::GetMaxClicks()
{
	int terrainAdd = 0;
	int terrainMult = 1;
	int weatherAdd = 0;
	int badWeather = 0;
	int maxClicks;
	if (weather != W_NORMAL && !clearskies) badWeather = 1;
	if (!Globals->WEATHER_EXISTS) badWeather = 0;
	switch (type) {
		case R_PLAIN:
		case R_ISLAND_PLAIN:
		case R_TUNDRA:
		case R_CERAN_PLAIN1:
		case R_CERAN_PLAIN2:
		case R_CERAN_PLAIN3:
		case R_CERAN_LAKE:
		case R_CERAN_TUNDRA1:
		case R_CERAN_TUNDRA2:
		case R_CERAN_TUNDRA3:
			terrainAdd = -1;
			if (badWeather) weatherAdd = 4;
			break;
		case R_MOUNTAIN:
		case R_ISLAND_MOUNTAIN:
		case R_CERAN_MOUNTAIN1:
		case R_CERAN_MOUNTAIN2:
		case R_CERAN_MOUNTAIN3:
		case R_CERAN_HILL:
		case R_CERAN_HILL1:
		case R_CERAN_HILL2:
			terrainMult = 2;
			if (badWeather) weatherAdd = 4;
			break;
		case R_FOREST:
		case R_SWAMP:
		case R_ISLAND_SWAMP:
		case R_JUNGLE:
		case R_CERAN_FOREST1:
		case R_CERAN_FOREST2:
		case R_CERAN_FOREST3:
		case R_CERAN_MYSTFOREST:
		case R_CERAN_MYSTFOREST1:
		case R_CERAN_MYSTFOREST2:
		case R_CERAN_SWAMP1:
		case R_CERAN_SWAMP2:
		case R_CERAN_SWAMP3:
		case R_CERAN_JUNGLE1:
		case R_CERAN_JUNGLE2:
		case R_CERAN_JUNGLE3:
			terrainAdd = -1;
			terrainMult = 2;
			if (badWeather) weatherAdd = 1;
			break;
		case R_DESERT:
		case R_CERAN_DESERT1:
		case R_CERAN_DESERT2:
		case R_CERAN_DESERT3:
			terrainAdd = -1;
			if (badWeather) weatherAdd = 5;
		case R_CAVERN:
		case R_UFOREST:
		case R_TUNNELS:
		case R_CERAN_CAVERN1:
		case R_CERAN_CAVERN2:
		case R_CERAN_CAVERN3:
		case R_CERAN_UFOREST1:
		case R_CERAN_UFOREST2:
		case R_CERAN_UFOREST3:
		case R_CERAN_TUNNELS1:
		case R_CERAN_TUNNELS2:
		case R_CHASM:
		case R_CERAN_CHASM1:
		case R_GROTTO:
		case R_CERAN_GROTTO1:
		case R_DFOREST:
		case R_CERAN_DFOREST1:
			terrainAdd = 1;
			terrainMult = 2;
			if (badWeather) weatherAdd = 6;
			break;
		default:
			if (badWeather) weatherAdd = 4;
			break;
	}
	maxClicks = terrainMult * (terrainAdd + 2) + (weatherAdd + 1);
	return maxClicks;
}

// AS
int ARegion::PillageCheck()
{
	int pillageAdd = maxwages - wages;
	if (pillageAdd > 0) return pillageAdd;
	return 0;
}

// AS
int ARegion::HasRoad()
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		if (o->IsRoad() && o->incomplete < 1) return 1;
	}
	return 0;
}

// AS
int ARegion::HasExitRoad(int realDirection)
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		if (o->IsRoad() && o->incomplete < 1) {
			if (o->type == GetRoadDirection(realDirection)) return 1;
		}
	}
	return 0;
}

// AS
int ARegion::CountConnectingRoads()
{
	int connections = 0;
	for (int i = 0; i < NDIRS; i++) {
		if (HasExitRoad(i) && neighbors[i] &&
				HasConnectingRoad(i))
			connections ++;
	}
	return connections;
}

// AS
int ARegion::HasConnectingRoad(int realDirection)
{
	int opposite = GetRealDirComp(realDirection);

	if (neighbors[realDirection]
			&& neighbors[realDirection]->HasExitRoad(opposite))
		return 1;

	return 0;
}

// AS
int ARegion::GetRoadDirection(int realDirection)
{
	int roadDirection = 0;
	switch (realDirection) {
		case D_NORTH:
			roadDirection = O_ROADN;
			break;
		case D_NORTHEAST:
			roadDirection = O_ROADNE;
			break;
		case D_NORTHWEST:
			roadDirection = O_ROADNW;
			break;
		case D_SOUTH:
			roadDirection = O_ROADS;
			break;
		case D_SOUTHEAST:
			roadDirection = O_ROADSE;
			break;
		case D_SOUTHWEST:
			roadDirection = O_ROADSW;
			break;
	}
	return roadDirection;
}

// AS
int ARegion::GetRealDirComp(int realDirection)
{
	int complementDirection = 0;

	if (neighbors[realDirection]) {
		ARegion *n = neighbors[realDirection];
		for (int i = 0; i < NDIRS; i++)
			if (n->neighbors[i] == this)
				return i;
	}

	switch (realDirection) {
		case D_NORTH:
			complementDirection = D_SOUTH;
			break;
		case D_NORTHEAST:
			complementDirection = D_SOUTHWEST;
			break;
		case D_NORTHWEST:
			complementDirection = D_SOUTHEAST;
			break;
		case D_SOUTH:
			complementDirection = D_NORTH;
			break;
		case D_SOUTHEAST:
			complementDirection = D_NORTHWEST;
			break;
		case D_SOUTHWEST:
			complementDirection = D_NORTHEAST;
			break;
	}
	return complementDirection;
}

AString ARegion::ShortPrint(ARegionList *pRegs)
{
	AString temp = TerrainDefs[type].name;

	temp += AString(" (") + xloc + "," + yloc;

	ARegionArray *pArr = pRegs->pRegionArrays[zloc];
	if (pArr->strName) {
		temp += ",";
		if (Globals->EASIER_UNDERWORLD &&
		   (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS > 1)) {
			temp += AString("") + zloc + " <";
		} else {
			// add less explicit multilevel information about the underworld
			if (zloc > 2 && zloc < Globals->UNDERWORLD_LEVELS+2) {
				for (int i = zloc; i > 3; i--) {
					temp += "very ";
				}
				temp += "deep ";
			} else if ((zloc > Globals->UNDERWORLD_LEVELS+2) &&
					  (zloc < Globals->UNDERWORLD_LEVELS +
					   Globals->UNDERDEEP_LEVELS + 2)) {
				for (int i = zloc; i > Globals->UNDERWORLD_LEVELS + 3; i--) {
					temp += "very ";
				}
				temp += "deep ";
			}
		}
		temp += *pArr->strName;
		if (Globals->EASIER_UNDERWORLD &&
		   (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS > 1)) {
			temp += ">";
		}
	}
	temp += ")";

	temp += AString(" in ") + *name;
	return temp;
}

AString ARegion::Print(ARegionList *pRegs)
{
	AString temp = ShortPrint(pRegs);
	if (town) {
		temp += AString(", contains ") + *(town->name) + " [" +
			TownString(town->TownType()) + "]";
	}
	return temp;
}

void ARegion::SetLoc(int x, int y, int z)
{
	xloc = x;
	yloc = y;
	zloc = z;
}

void ARegion::SetGateStatus(int month)
{
	if ((type == R_NEXUS) || (Globals->START_GATES_OPEN && IsStartingCity())) {
		gateopen = 1;
		return;
	}
	gateopen = 0;
	for (int i = 0; i < Globals->GATES_NOT_PERENNIAL; i++) {
		int dmon = gatemonth + i;
		if (dmon > 11) dmon = dmon - 12;
		if (dmon == month) gateopen = 1;
	}
}

void ARegion::Kill(Unit *u)
{
	Unit *first = 0;
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		if (obj) {
			forlist((&obj->units)) {
				if (((Unit *) elem)->faction->num == u->faction->num &&
					((Unit *) elem) != u) {
					first = (Unit *) elem;
					break;
				}
			}
		}
		if (first) break;
	}

	if (first) {
		// give u's stuff to first
		forlist(&u->items) {
			Item *i = (Item *) elem;
			if (!IsSoldier(i->type)) {
				first->items.SetNum(i->type, first->items.GetNum(i->type) +
									i->num);
				// If we're in ocean and not in a structure, make sure that
				// the first unit can actually hold the stuff and not drown
				// If the item would cause them to drown then they won't
				// pick it up.
				if (TerrainDefs[type].similar_type == R_OCEAN) {
					if (first->object->type == O_DUMMY) {
						if (!first->CanReallySwim()) {
							first->items.SetNum(i->type,
									first->items.GetNum(i->type) - i->num);
						}
					}
				}
			}
			u->items.SetNum(i->type, 0);
		}
	}

	u->MoveUnit(0);
	hell.Add(u);
}

void ARegion::ClearHell()
{
	hell.DeleteAll();
}

Object *ARegion::GetObject(int num)
{
	forlist(&objects) {
		Object *o = (Object *) elem;
		if (o->num == num) return o;
	}
	return 0;
}

Object *ARegion::GetDummy()
{
	forlist(&objects) {
		Object *o = (Object *) elem;
		if (o->type == O_DUMMY) return o;
	}
	return 0;
}

/* Checks all fleets to see if they are empty.
 * Moves all units out of an empty fleet into the
 * dummy object.
 */
void ARegion::CheckFleets()
{
	forlist(&objects) {
		Object *o = (Object *) elem;
		if (o->IsFleet()) {
			int bail = 0;
			if (o->FleetCapacity() < 1) bail = 1;
			int alive = 0;
			forlist(&o->units) {
				Unit * unit = (Unit *) elem;
				if (unit->IsAlive()) alive = 1;
				if (bail > 0) unit->MoveUnit(GetDummy());
			}
			// don't remove fleets when no living units are
			// aboard when they're not at sea.
			if (TerrainDefs[type].similar_type == R_OCEAN) alive = 1;
			if ((alive == 0) || (bail == 1)) {
				objects.Remove(o);
				delete o;
			}
		}
	}
}

Unit *ARegion::GetUnit(int num)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		Unit *u = obj->GetUnit(num);
		if (u) {
			return(u);
		}
	}
	return 0;
}

Location *ARegion::GetLocation(UnitId *id, int faction)
{
	Unit *retval = 0;
	forlist(&objects) {
		Object *o = (Object *) elem;
		retval = o->GetUnitId(id, faction);
		if (retval) {
			Location *l = new Location;
			l->region = this;
			l->obj = o;
			l->unit = retval;
			return l;
		}
	}
	return 0;
}

Unit *ARegion::GetUnitAlias(int alias, int faction)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		Unit *u = obj->GetUnitAlias(alias, faction);
		if (u) {
			return(u);
		}
	}
	return 0;
}

Unit *ARegion::GetUnitId(UnitId *id, int faction)
{
	Unit *retval = 0;
	forlist(&objects) {
		Object *o = (Object *) elem;
		retval = o->GetUnitId(id, faction);
		if (retval) return retval;
	}
	return retval;
}

Location *ARegionList::GetUnitId(UnitId *id, int faction, ARegion *cur)
{
	Location *retval = NULL;
	// Check current region first
	retval = cur->GetLocation(id, faction);
	if (retval) return retval;

	// No? We must be looking for an existing unit.
	if (!id->unitnum) return NULL;

	return this->FindUnit(id->unitnum);
}

int ARegion::Present(Faction *f)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units))
			if (((Unit *) elem)->faction == f) return 1;
	}
	return 0;
}

AList *ARegion::PresentFactions()
{
	AList *facs = new AList;
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (!GetFaction2(facs, u->faction->num)) {
				FactionPtr *p = new FactionPtr;
				p->ptr = u->faction;
				facs->Add(p);
			}
		}
	}
	return facs;
}

void ARegion::Writeout(Aoutfile *f)
{
	f->PutStr(*name);
	f->PutInt(num);
	if (type != -1) f->PutStr(TerrainDefs[type].type);
	else f->PutStr("NO_TERRAIN");
	f->PutInt(buildingseq);
	f->PutInt(gate);
	if (gate > 0) f->PutInt(gatemonth);
	if (race != -1) f->PutStr(ItemDefs[race].abr);
	else f->PutStr("NO_RACE");
	f->PutInt(population);
	f->PutInt(basepopulation);
	f->PutInt(wages);
	f->PutInt(maxwages);
	f->PutInt(wealth);

	f->PutInt(elevation);
	f->PutInt(humidity);
	f->PutInt(temperature);
	f->PutInt(vegetation);
	f->PutInt(culture);

	f->PutInt(habitat);
	f->PutInt(development);

	if (town) {
		f->PutInt(1);
		town->Writeout(f);
	} else {
		f->PutInt(0);
	}

	f->PutInt(xloc);
	f->PutInt(yloc);
	f->PutInt(zloc);
	f->PutInt(visited);

	products.Writeout(f);
	markets.Writeout(f);

	f->PutInt(objects.Num());
	forlist ((&objects)) ((Object *) elem)->Writeout(f);
}

int LookupRegionType(AString *token)
{
	for (int i = 0; i < R_NUM; i++) {
		if (*token == TerrainDefs[i].type) return i;
	}
	return -1;
}

void ARegion::Readin(Ainfile *f, AList *facs, ATL_VER v)
{
	AString *temp;

	name = f->GetStr();

	num = f->GetInt();
	temp = f->GetStr();
	type = LookupRegionType(temp);
	delete temp;
	buildingseq = f->GetInt();
	gate = f->GetInt();
	if (gate > 0) gatemonth = f->GetInt();

	temp = f->GetStr();
	race = LookupItem(temp);
	delete temp;

	population = f->GetInt();
	basepopulation = f->GetInt();
	wages = f->GetInt();
	maxwages = f->GetInt();
	wealth = f->GetInt();
	
	elevation = f->GetInt();
	humidity = f->GetInt();
	temperature = f->GetInt();
	vegetation = f->GetInt();
	culture = f->GetInt();

	habitat = f->GetInt();
	development = f->GetInt();

	if (f->GetInt()) {
		town = new TownInfo;
		town->Readin(f, v);
		town->dev = TownDevelopment();
	} else {
		town = 0;
	}

	xloc = f->GetInt();
	yloc = f->GetInt();
	zloc = f->GetInt();
	visited = f->GetInt();

	products.Readin(f);
	markets.Readin(f);

	int i = f->GetInt();
	for (int j=0; j<i; j++) {
		Object *temp = new Object(this);
		temp->Readin(f, facs, v);
		objects.Add(temp);
	}
	fleetalias = 1;
	newfleets.clear();
}

int ARegion::CanMakeAdv(Faction *fac, int item)
{
	AString skname;
	int sk;
	Farsight *f;

	if (Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			f = (Farsight *)elem;
			if (f && f->faction == fac && f->unit) {
				skname = ItemDefs[item].pSkill;
				sk = LookupSkill(&skname);
				if (f->unit->GetSkill(sk) >= ItemDefs[item].pLevel)
					return 1;
			}
		}
	}

	if ((Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_RESOURCES)) {
		forlist(&passers) {
			f = (Farsight *)elem;
			if (f && f->faction == fac && f->unit) {
				skname = ItemDefs[item].pSkill;
				sk = LookupSkill(&skname);
				if (f->unit->GetSkill(sk) >= ItemDefs[item].pLevel)
					return 1;
			}
		}
	}

	forlist(&objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->faction == fac) {
				skname = ItemDefs[item].pSkill;
				sk = LookupSkill(&skname);
				if (u->GetSkill(sk) >= ItemDefs[item].pLevel)
					return 1;
			}
		}
	}
	return 0;
}

void ARegion::WriteProducts(Areport *f, Faction *fac, int present)
{
	AString temp = "Products: ";
	int has = 0;
	forlist((&products)) {
		Production *p = ((Production *) elem);
		if (ItemDefs[p->itemtype].type & IT_ADVANCED) {
			if (CanMakeAdv(fac, p->itemtype) || (fac->IsNPC())) {
				if (has) {
					temp += AString(", ") + p->WriteReport();
				} else {
					has = 1;
					temp += p->WriteReport();
				}
			}
		} else {
			if (p->itemtype == I_SILVER) {
				if (p->skill == S_ENTERTAINMENT) {
					if ((Globals->TRANSIT_REPORT &
							GameDefs::REPORT_SHOW_ENTERTAINMENT) || present) {
						f->PutStr(AString("Entertainment available: $") +
								p->amount + ".");
					} else {
						f->PutStr(AString("Entertainment available: $0."));
					}
				}
			} else {
				if (!present &&
				   !(Globals->TRANSIT_REPORT &
					   GameDefs::REPORT_SHOW_RESOURCES))
					continue;
				if (has) {
					temp += AString(", ") + p->WriteReport();
				} else {
					has = 1;
					temp += p->WriteReport();
				}
			}
		}
	}

	if (has==0) temp += "none";
	temp += ".";
	f->PutStr(temp);
}

int ARegion::HasItem(Faction *fac, int item)
{
	forlist(&objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->faction == fac) {
				if (u->items.GetNum(item)) return 1;
			}
		}
	}
	return 0;
}

void ARegion::WriteMarkets(Areport *f, Faction *fac, int present)
{
	AString temp = "Wanted: ";
	int has = 0;
	forlist(&markets) {
		Market *m = (Market *) elem;
		if (!m->amount) continue;
		if (!present &&
		   !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_MARKETS))
			continue;
		if (m->type == M_SELL) {
			if (ItemDefs[m->item].type & IT_ADVANCED) {
				if (!Globals->MARKETS_SHOW_ADVANCED_ITEMS) {
					if (!HasItem(fac, m->item)) {
						continue;
					}
				}
			}
			if (has) {
				temp += ", ";
			} else {
				has = 1;
			}
			temp += m->Report();
		}
	}
	if (!has) temp += "none";
	temp += ".";
	f->PutStr(temp);

	temp = "For Sale: ";
	has = 0;
	{
		forlist(&markets) {
			Market *m = (Market *) elem;
			if (!m->amount) continue;
			if (!present &&
			   !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_MARKETS))
				continue;
			if (m->type == M_BUY) {
				if (has) {
					temp += ", ";
				} else {
					has = 1;
				}
				temp += m->Report();
			}
		}
	}
	if (!has) temp += "none";
	temp += ".";
	f->PutStr(temp);
}

void ARegion::WriteEconomy(Areport *f, Faction *fac, int present)
{
	f->AddTab();

	if ((Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_WAGES) || present) {
		f->PutStr(AString("Wages: ") + WagesForReport() + ".");
	} else {
		f->PutStr(AString("Wages: $0."));
	}

	WriteMarkets(f, fac, present);

	WriteProducts(f, fac, present);

	f->EndLine();
	f->DropTab();
}

void ARegion::WriteExits(Areport *f, ARegionList *pRegs, int *exits_seen)
{
	f->PutStr("Exits:");
	f->AddTab();
	int y = 0;
	for (int i=0; i<NDIRS; i++) {
		ARegion *r = neighbors[i];
		if (r && exits_seen[i]) {
			f->PutStr(AString(DirectionStrs[i]) + " : " +
					r->Print(pRegs) + ".");
			y = 1;
		}
	}
	if (!y) f->PutStr("none");
	f->DropTab();
	f->EndLine();
}

#define AC_STRING "%s Nexus is a magical place; the entryway " \
"to the world of %s. Enjoy your stay; the city guards should " \
"keep you safe as long as you should choose to stay. However, rumor " \
"has it that once you have left the Nexus, you can never return."

void ARegion::WriteReport(Areport *f, Faction *fac, int month,
		ARegionList *pRegions)
{
	Farsight *farsight = GetFarsight(&farsees, fac);
	Farsight *passer = GetFarsight(&passers, fac);
	int present = Present(fac) || fac->IsNPC();

	if (farsight || passer || present)  {
		AString temp = Print(pRegions);
		if (Population() &&
			(present || farsight ||
			 (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_PEASANTS))) {
			temp += AString(", ") + Population() + " peasants";
			if (Globals->RACES_EXIST) {
				temp += AString(" (") + ItemDefs[race].names + ")";
			}
			if (present || farsight ||
			   Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_REGION_MONEY) {
				temp += AString(", $") + wealth;
			} else {
				temp += AString(", $0");
			}
		}
		temp += ".";
		f->PutStr(temp);
		f->PutStr("-------------------------------------------------"
				"-----------");

		f->AddTab();
		if (Globals->WEATHER_EXISTS) {
			temp = "It was ";
			if (clearskies) temp += "unnaturally clear ";
			else {
				if (weather == W_BLIZZARD) temp = "There was an unnatural ";
				else if (weather == W_NORMAL) temp = "The weather was ";
				temp += SeasonNames[weather];
			}
			temp += " last month; ";
			int nxtweather = pRegions->GetWeather(this, (month + 1) % 12);
			temp += "it will be ";
			temp += SeasonNames[nxtweather];
			temp += " next month.";
			f->PutStr(temp);
		}
		
#if 0
		f->PutStr("");
		temp = "Elevation is ";
		f->PutStr(temp + elevation);
		temp = "Humidity is ";
		f->PutStr(temp + humidity);
		temp = "Temperature is ";
		f->PutStr(temp + temperature);
#endif

		if (type == R_NEXUS) {
			int len = strlen(AC_STRING)+2*strlen(Globals->WORLD_NAME);
			char *nexus_desc = new char[len];
			sprintf(nexus_desc, AC_STRING, Globals->WORLD_NAME,
					Globals->WORLD_NAME);
			f->PutStr("");
			f->PutStr(nexus_desc);
			f->PutStr("");
			delete [] nexus_desc;
		}

		f->DropTab();

		WriteEconomy(f, fac, present || farsight);

		int exits_seen[NDIRS];
		if (present || farsight ||
		   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_ALL_EXITS)) {
			for (int i = 0; i < NDIRS; i++)
				exits_seen[i] = 1;
		} else {
			// This is just a transit report and we're not showing all
			// exits.   See if we are showing used exits.

			// Show none by default.
			int i;
			for (i = 0; i < NDIRS; i++)
				exits_seen[i] = 0;
			// Now, if we should, show the ones actually used.
			if (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_USED_EXITS) {
				forlist(&passers) {
					Farsight *p = (Farsight *)elem;
					if (p->faction == fac) {
						for (i = 0; i < NDIRS; i++) {
							exits_seen[i] |= p->exits_used[i];
						}
					}
				}
			}
		}

		WriteExits(f, pRegions, exits_seen);

		if (Globals->GATES_EXIST && gate && gate != -1) {
			int sawgate = 0;
			if (fac->IsNPC())
				sawgate = 1;
			if (Globals->IMPROVED_FARSIGHT && farsight) {
				forlist(&farsees) {
					Farsight *watcher = (Farsight *)elem;
					if (watcher && watcher->faction == fac && watcher->unit) {
						if (watcher->unit->GetSkill(S_GATE_LORE)) {
							sawgate = 1;
						}
					}
				}
			}
			if (Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) {
				forlist(&passers) {
					Farsight *watcher = (Farsight *)elem;
					if (watcher && watcher->faction == fac && watcher->unit) {
						if (watcher->unit->GetSkill(S_GATE_LORE)) {
							sawgate = 1;
						}
					}
				}
			}
			forlist(&objects) {
				Object *o = (Object *) elem;
				forlist(&o->units) {
					Unit *u = (Unit *) elem;
					if (!sawgate &&
							((u->faction == fac) &&
							 u->GetSkill(S_GATE_LORE))) {
						sawgate = 1;
					}
				}
			}
			if (sawgate) {
				if (gateopen) {
					AString temp;
					temp = "There is a Gate here (Gate ";
					temp += gate;
					if (!Globals->DISPERSE_GATE_NUMBERS) {
						temp += " of " + (pRegions->numberofgates);
					}
					temp += ").";
					f->PutStr(temp);
					f->PutStr("");
				} else if (Globals->SHOW_CLOSED_GATES) {
					f->PutStr(AString("There is a closed Gate here."));
					f->PutStr("");
				}
			}
		}

		int obs = GetObservation(fac, 0);
		int truesight = GetTrueSight(fac, 0);
		int detfac = 0;

		int passobs = GetObservation(fac, 1);
		int passtrue = GetTrueSight(fac, 1);
		int passdetfac = detfac;

		if (fac->IsNPC()) {
			obs = 10;
			passobs = 10;
		}

		forlist (&objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == fac && u->GetSkill(S_MIND_READING) > 2) {
					detfac = 1;
				}
			}
		}
		if (Globals->IMPROVED_FARSIGHT && farsight) {
			forlist(&farsees) {
				Farsight *watcher = (Farsight *)elem;
				if (watcher && watcher->faction == fac && watcher->unit) {
					if (watcher->unit->GetSkill(S_MIND_READING) > 2) {
						detfac = 1;
					}
				}
			}
		}

		if ((Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
		   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
			forlist(&passers) {
				Farsight *watcher = (Farsight *)elem;
				if (watcher && watcher->faction == fac && watcher->unit) {
					if (watcher->unit->GetSkill(S_MIND_READING) > 2) {
						passdetfac = 1;
					}
				}
			}
		}

		{
			forlist (&objects) {
				((Object *) elem)->Report(f, fac, obs, truesight, detfac,
										  passobs, passtrue, passdetfac,
										  present || farsight);
			}
			f->EndLine();
		}
	}
}

// DK
void ARegion::WriteTemplate(Areport *f, Faction *fac,
		ARegionList *pRegs, int month)
{
	int header = 0;

	forlist (&objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->faction == fac) {
				if (!header) {
					// DK
					if (fac->temformat == TEMPLATE_MAP) {
						WriteTemplateHeader(f, fac, pRegs, month);
					} else {
						f->PutStr("");
						f->PutStr(AString("*** ") + Print(pRegs) + " ***", 1);
					}
					header = 1;
				}

				f->PutStr("");
				f->PutStr(AString("unit ") + u->num);
				// DK
				if (fac->temformat == TEMPLATE_LONG ||
						fac->temformat == TEMPLATE_MAP) {
					f->PutStr(u->TemplateReport(), 1);
				}
				forlist(&(u->oldorders)) {
					f->PutStr(*((AString *) elem));
				}
				u->oldorders.DeleteAll();

				if (u->turnorders.First()) {
					int first = 1;
					TurnOrder *tOrder;
					forlist(&u->turnorders) {
						tOrder = (TurnOrder *)elem;
						if (first) {
							forlist(&tOrder->turnOrders) {
								f->PutStr(*((AString *) elem));
							}
							first = 0;
						} else {
							if (tOrder->repeating)
								f->PutStr(AString("@TURN"));
							else
								f->PutStr(AString("TURN"));

							forlist(&tOrder->turnOrders) {
								f->PutStr(*((AString *) elem));
							}
							f->PutStr(AString("ENDTURN"));
						}
					}
					tOrder = (TurnOrder *) u->turnorders.First();
					if (tOrder->repeating) {
						f->PutStr(AString("@TURN"));
						forlist(&tOrder->turnOrders) {
							f->PutStr(*((AString *) elem));
						}
						f->PutStr(AString("ENDTURN"));
					}
				}
				u->turnorders.DeleteAll();
			}
		}
	}
}

int ARegion::GetTrueSight(Faction *f, int usepassers)
{
	int truesight = 0;

	if (Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			Farsight *farsight = (Farsight *)elem;
			if (farsight && farsight->faction == f && farsight->unit) {
				int t = farsight->unit->GetSkill(S_TRUE_SEEING);
				if (t > truesight) truesight = t;
			}
		}
	}

	if (usepassers &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
		forlist(&passers) {
			Farsight *farsight = (Farsight *)elem;
			if (farsight && farsight->faction == f && farsight->unit) {
				int t = farsight->unit->GetSkill(S_TRUE_SEEING);
				if (t > truesight) truesight = t;
			}
		}
	}

	forlist ((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == f) {
				int temp = u->GetSkill(S_TRUE_SEEING);
				if (temp>truesight) truesight = temp;
			}
		}
	}
	return truesight;
}

int ARegion::GetObservation(Faction *f, int usepassers)
{
	int obs = 0;

	if (Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			Farsight *farsight = (Farsight *)elem;
			if (farsight && farsight->faction == f && farsight->unit) {
				int o = farsight->observation;
				if (o > obs) obs = o;
			}
		}
	}

	if (usepassers &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
		forlist(&passers) {
			Farsight *farsight = (Farsight *)elem;
			if (farsight && farsight->faction == f && farsight->unit) {
				int o = farsight->observation;
				if (o > obs) obs = o;
			}
		}
	}

	forlist ((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == f) {
				int temp = u->GetAttribute("observation");
				if (temp>obs) obs = temp;
			}
		}
	}
	return obs;
}

void ARegion::SetWeather(int newWeather)
{
	weather = newWeather;
}

int ARegion::IsCoastal()
{
	if ((type != R_LAKE) && (TerrainDefs[type].similar_type == R_OCEAN)) return 1;
	int seacount = 0;
	for (int i=0; i<NDIRS; i++) {
		if (neighbors[i] && TerrainDefs[neighbors[i]->type].similar_type == R_OCEAN) {
		if (!Globals->LAKESIDE_IS_COASTAL && neighbors[i]->type == R_LAKE) continue;
		seacount++;
	}
	}
	return seacount;
}

int ARegion::IsCoastalOrLakeside()
{
	if ((type != R_LAKE) && (TerrainDefs[type].similar_type == R_OCEAN)) return 1;
	int seacount = 0;
	for (int i=0; i<NDIRS; i++) {
		if (neighbors[i] && TerrainDefs[neighbors[i]->type].similar_type == R_OCEAN) {
		seacount++;
	}
	}
	return seacount;
}

int ARegion::MoveCost(int movetype, ARegion *fromRegion, int dir, AString *road)
{
	int cost = 1;
	if (Globals->WEATHER_EXISTS) {
		cost = 2;
		if (weather == W_BLIZZARD) return 10;
		if (weather == W_NORMAL || clearskies) cost = 1;
	}
	if (movetype == M_WALK || movetype == M_RIDE) {
		cost = (TerrainDefs[type].movepoints * cost);
		if (fromRegion->HasExitRoad(dir) && fromRegion->HasConnectingRoad(dir)) {
			cost -= cost/2;
			if (road)
				*road = "on a road ";
		}
	}
	if (cost < 1) cost = 1;
	return cost;
}

Unit *ARegion::Forbidden(Unit *u)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u2 = (Unit *) elem;
			if (u2->Forbids(this, u)) return u2;
		}
	}
	return 0;
}

Unit *ARegion::ForbiddenByAlly(Unit *u)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u2 = (Unit *) elem;
			if (u->faction->GetAttitude(u2->faction->num) == A_ALLY &&
				u2->Forbids(this, u)) return u2;
		}
	}
	return 0;
}

int ARegion::HasCityGuard()
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->type == U_GUARD && u->GetSoldiers() &&
				u->guard == GUARD_GUARD) {
				return 1;
			}
		}
	}
	return 0;
}

int ARegion::NotifySpell(Unit *caster, char const *spell, ARegionList *pRegs)
{
	AList flist;
	unsigned int i;

	SkillType *pS = FindSkill(spell);

	if (!(pS->flags & SkillType::NOTIFY)) {
		// Okay, we aren't notifyable, check our prerequisites
		for (i = 0; i < sizeof(pS->depends)/sizeof(SkillDepend); i++) {
			if (pS->depends[i].skill == NULL) break;
			if (NotifySpell(caster, pS->depends[i].skill, pRegs)) return 1;
		}
		return 0;
	}

	AString skname = spell;
	int sp = LookupSkill(&skname);
	forlist((&objects)) {
		Object *o = (Object *) elem;
		forlist ((&o->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == caster->faction) continue;
			if (u->GetSkill(sp)) {
				if (!GetFaction2(&flist, u->faction->num)) {
					FactionPtr *fp = new FactionPtr;
					fp->ptr = u->faction;
					flist.Add(fp);
				}
			}
		}
	}

	forlist_reuse (&flist) {
		FactionPtr *fp = (FactionPtr *) elem;
		fp->ptr->Event(AString(*(caster->name)) + " uses " + SkillStrs(sp) +
				" in " + Print(pRegs) + ".");
	}
	return 1;
}

// ALT, 26-Jul-2000
// Procedure to notify all units in city about city name change
void ARegion::NotifyCity(Unit *caster, AString& oldname, AString& newname)
{
	AList flist;
	forlist((&objects)) {
		Object *o = (Object *) elem;
		forlist ((&o->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == caster->faction) continue;
			if (!GetFaction2(&flist, u->faction->num)) {
				FactionPtr *fp = new FactionPtr;
				fp->ptr = u->faction;
				flist.Add(fp);
			}
		}
	}
	{
		forlist(&flist) {
			FactionPtr *fp = (FactionPtr *) elem;
			fp->ptr->Event(AString(*(caster->name)) + " renames " +
					oldname + " to " + newname + ".");
		}
	}
}

int ARegion::CanTax(Unit *u)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u2 = (Unit *) elem;
			if (u2->guard == GUARD_GUARD && u2->IsAlive())
				if (u2->GetAttitude(this, u) <= A_NEUTRAL)
					return 0;
		}
	}
	return 1;
}

int ARegion::CanPillage(Unit *u)
{
	forlist(&objects) {
		Object *obj = (Object *)elem;
		forlist (&obj->units) {
			Unit *u2 = (Unit *)elem;
			if (u2->guard == GUARD_GUARD && u2->IsAlive() &&
					u2->faction != u->faction)
				return 0;
		}
	}
	return 1;
}

int ARegion::ForbiddenShip(Object *ship)
{
	forlist(&ship->units) {
		Unit *u = (Unit *) elem;
		if (Forbidden(u)) return 1;
	}
	return 0;
}

void ARegion::DefaultOrders()
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units))
			((Unit *) elem)->DefaultOrders(obj);
	}
}

//
// This is just used for mapping; just check if there is an inner region.
//
int ARegion::HasShaft()
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		if (o->inner != -1) return 1;
	}
	return 0;
}

int ARegion::IsGuarded()
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		forlist (&o->units) {
			Unit *u = (Unit *) elem;
			if (u->guard == GUARD_GUARD) return 1;
		}
	}
	return 0;
}

int ARegion::CountWMons()
{
	int count = 0;
	forlist (&objects) {
		Object *o = (Object *) elem;
		forlist (&o->units) {
			Unit *u = (Unit *) elem;
			if (u->type == U_WMON) {
				count ++;
			}
		}
	}
	return count;
}

/* New Fleet objects are stored in the newfleets
 * map for resolving aliases in the Enter NEW phase.
 */
void ARegion::AddFleet(Object * fleet)
{
	objects.Add(fleet);
	//Awrite(AString("Setting up fleet alias #") + fleetalias + ": " + fleet->num);
	newfleets.insert(make_pair(fleetalias++, fleet->num));
	
}

int ARegion::ResolveFleetAlias(int alias)
{
	map<int, int>::iterator f;
	f = newfleets.find(alias);
	//Awrite(AString("Resolving Fleet Alias #") + alias + ": " + f->second);
	if (f == newfleets.end()) return -1;
	return f->second;
}

ARegionList::ARegionList()
{
	pRegionArrays = 0;
	numLevels = 0;
	numberofgates = 0;
}

ARegionList::~ARegionList()
{
	if (pRegionArrays) {
		int i;
		for (i = 0; i < numLevels; i++) {
			delete pRegionArrays[i];
		}

		delete pRegionArrays;
	}
}

void ARegionList::WriteRegions(Aoutfile *f)
{
	f->PutInt(Num());

	f->PutInt(numLevels);
	int i;
	for (i = 0; i < numLevels; i++) {
		ARegionArray *pRegs = pRegionArrays[i];
		f->PutInt(pRegs->x);
		f->PutInt(pRegs->y);
		if (pRegs->strName) {
			f->PutStr(*pRegs->strName);
		} else {
			f->PutStr("none");
		}
		f->PutInt(pRegs->levelType);
	}

	f->PutInt(numberofgates);
	forlist(this) ((ARegion *) elem)->Writeout(f);
	{
		f->PutStr("Neighbors");
		forlist(this) {
			ARegion *reg = (ARegion *) elem;
			for (i = 0; i < NDIRS; i++) {
				if (reg->neighbors[i]) {
					f->PutInt(reg->neighbors[i]->num);
				} else {
					f->PutInt(-1);
				}
			}
		}
	}
}

int ARegionList::ReadRegions(Ainfile *f, AList *factions, ATL_VER v)
{
	int num = f->GetInt();

	numLevels = f->GetInt();
	CreateLevels(numLevels);
	int i;
	for (i = 0; i < numLevels; i++) {
		int curX = f->GetInt();
		int curY = f->GetInt();
		AString *name = f->GetStr();
		ARegionArray *pRegs = new ARegionArray(curX, curY);
		if (*name == "none") {
			pRegs->strName = 0;
			delete name;
		} else {
			pRegs->strName = name;
		}
		pRegs->levelType = f->GetInt();
		pRegionArrays[i] = pRegs;
	}

	numberofgates = f->GetInt();

	ARegionFlatArray fa(num);

	Awrite("Reading the regions...");
	for (i = 0; i < num; i++) {
		ARegion *temp = new ARegion;
		temp->Readin(f, factions, v);
		fa.SetRegion(temp->num, temp);
		Add(temp);

		pRegionArrays[temp->zloc]->SetRegion(temp->xloc, temp->yloc,
												temp);
	}

	Awrite("Setting up the neighbors...");
	{
		delete f->GetStr();
		forlist(this) {
			ARegion *reg = (ARegion *) elem;
			for (i = 0; i < NDIRS; i++) {
				int j = f->GetInt();
				if (j != -1) {
					reg->neighbors[i] = fa.GetRegion(j);
				} else {
					reg->neighbors[i] = 0;
				}
			}
		}
	}
	return 1;
}

ARegion *ARegionList::GetRegion(int n)
{
	forlist(this) {
		if (((ARegion *) elem)->num == n) return ((ARegion *) elem);
	}
	return 0;
}

ARegion *ARegionList::GetRegion(int x, int y, int z)
{

	if (z >= numLevels) return NULL;

	ARegionArray *arr = pRegionArrays[z];

	x = (x + arr->x) % arr->x;
	y = (y + arr->y) % arr->y;

	return(arr->GetRegion(x, y));
}

Location *ARegionList::FindUnit(int i)
{
	forlist(this) {
		ARegion *reg = (ARegion *) elem;
		forlist((&reg->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->num == i) {
					Location *retval = new Location;
					retval->unit = u;
					retval->region = reg;
					retval->obj = obj;
					return retval;
				}
			}
		}
	}
	return 0;
}

void ARegionList::NeighSetup(ARegion *r, ARegionArray *ar)
{
	r->ZeroNeighbors();

	if (r->yloc != 0 && r->yloc != 1) {
		r->neighbors[D_NORTH] = ar->GetRegion(r->xloc, r->yloc - 2);
	}
	if (r->yloc != 0) {
		r->neighbors[D_NORTHEAST] = ar->GetRegion(r->xloc + 1, r->yloc - 1);
		r->neighbors[D_NORTHWEST] = ar->GetRegion(r->xloc - 1, r->yloc - 1);
	}
	if (r->yloc != ar->y - 1) {
		r->neighbors[D_SOUTHEAST] = ar->GetRegion(r->xloc + 1, r->yloc + 1);
		r->neighbors[D_SOUTHWEST] = ar->GetRegion(r->xloc - 1, r->yloc + 1);
	}
	if (r->yloc != ar->y - 1 && r->yloc != ar->y - 2) {
		r->neighbors[D_SOUTH] = ar->GetRegion(r->xloc, r->yloc + 2);
	}
}

void ARegionList::IcosahedralNeighSetup(ARegion *r, ARegionArray *ar)
{
	int scale, x, y, x2, y2, x3, neighX, neighY;

	scale = ar->x / 10;

	r->ZeroNeighbors();

	y = r->yloc;
	x = r->xloc;
	// x2 is the x-coord of this hex inside its "wedge"
	if (y < 5 * scale)
		x2 = x % (2 * scale);
	else
		x2 = (x + 1) % (2 * scale);
	// x3 is the distance of this hex from the right side of its "wedge"
	x3 = (2 * scale - x2) % (2 * scale);
	// y2 is the distance from the SOUTH pole
	y2 = 10 * scale - 1 - y;
	// Always try to connect in the standard way...
	if (y > 1) {
		r->neighbors[D_NORTH] = ar->GetRegion(x, y - 2);
	}
	// but if that fails, use the special icosahedral connections:
	if (!r->neighbors[D_NORTH]) {
		if (y > 0 & y < 3 * scale)
		{
			if (y == 2) {
				neighX = 0;
				neighY = 0;
			}
			else if (y == 3 * x2) {
				neighX = x + 2 * (scale - x2) + 1;
				neighY = y - 1;
			}
			else {
				neighX = x + 2 * (scale - x2);
				neighY = y - 2;
			}
			neighX %= (scale * 10);
			r->neighbors[D_NORTH] = ar->GetRegion(neighX, neighY);
		}
	}
	if (y > 0) {
		neighX = x + 1;
		neighY = y - 1;
		neighX %= (scale * 10);
		r->neighbors[D_NORTHEAST] = ar->GetRegion(neighX, neighY);
	}
	if (!r->neighbors[D_NORTHEAST]) {
		if (y == 0) {
			neighX = 4 * scale;
			neighY = 2;
		}
		else if (y < 3 * scale) {
			if (y == 3 * x2) {
				neighX = x + 2 * (scale - x2) + 1;
				neighY = y + 1;
			}
			else {
				neighX = x + 2 * (scale - x2);
				neighY = y;
			}
		}
		else if (y2 < 1) {
			neighX = x + 2 * scale;
			neighY = y - 2;
		}
		else if (y2 < 3 * scale) {
			neighX = x + 2 * (scale - x2);
			neighY = y - 2;
		}
		neighX %= (scale * 10);
		r->neighbors[D_NORTHEAST] = ar->GetRegion(neighX, neighY);
	}
	if (y2 > 0) {
		neighX = x + 1;
		neighY = y + 1;
		neighX %= (scale * 10);
		r->neighbors[D_SOUTHEAST] = ar->GetRegion(neighX, neighY);
	}
	if (!r->neighbors[D_SOUTHEAST]) {
		if (y == 0) {
			neighX = 2 * scale;
			neighY = 2;
		}
		else if (y2 < 1) {
			neighX = x + 4 * scale;
			neighY = y - 2;
		}
		else if (y2 < 3 * scale) {
			if (y2 == 3 * x2) {
				neighX = x + 2 * (scale - x2) + 1;
				neighY = y - 1;
			}
			else {
				neighX = x + 2 * (scale - x2);
				neighY = y;
			}
		}
		else if (y < 3 * scale) {
			neighX = x + 2 * (scale - x2);
			neighY = y + 2;
		}
		neighX %= (scale * 10);
		r->neighbors[D_SOUTHEAST] = ar->GetRegion(neighX, neighY);
	}
	if (y2 > 1) {
		r->neighbors[D_SOUTH] = ar->GetRegion(x, y + 2);
	}
	if (!r->neighbors[D_SOUTH]) {
		if (y2 > 0 & y2 < 3 * scale)
		{
			if (y2 == 2) {
				neighX = 10 * scale - 1;
				neighY = y + 2;
			}
			else if (y2 == 3 * x2) {
				neighX = x + 2 * (scale - x2) + 1;
				neighY = y + 1;
			}
			else {
				neighX = x + 2 * (scale - x2);
				neighY = y + 2;
			}
			neighX = (neighX + scale * 10) % (scale * 10);
			r->neighbors[D_SOUTH] = ar->GetRegion(neighX, neighY);
		}
	}
	if (y2 > 0) {
		neighX = x - 1;
		neighY = y + 1;
		neighX = (neighX + scale * 10) % (scale * 10);
		r->neighbors[D_SOUTHWEST] = ar->GetRegion(neighX, neighY);
	}
	if (!r->neighbors[D_SOUTHWEST]) {
		if (y == 0) {
			neighX = 8 * scale;
			neighY = 2;
		}
		else if (y2 < 1) {
			neighX = x + 6 * scale;
			neighY = y - 2;
		}
		else if (y2 < 3 * scale) {
			if (y2 == 3 * x3 + 4) {
				neighX = x + 2 * (x3 - scale) + 1;
				neighY = y + 1;
			}
			else {
				neighX = x + 2 * (x3 - scale);
				neighY = y;
			}
		}
		else if (y < 3 * scale) {
			neighX = x - 2 * (scale - x3) + 1;
			neighY = y + 1;
		}
		neighX = (neighX + scale * 10) % (scale * 10);
		r->neighbors[D_SOUTHWEST] = ar->GetRegion(neighX, neighY);
	}
	if (y > 0) {
		neighX = x - 1;
		neighY = y - 1;
		neighX = (neighX + scale * 10) % (scale * 10);
		r->neighbors[D_NORTHWEST] = ar->GetRegion(neighX, neighY);
	}
	if (!r->neighbors[D_NORTHWEST]) {
		if (y == 0) {
			neighX = 6 * scale;
			neighY = 2;
		}
		else if (y < 3 * scale) {
			if (y == 3 * x3 + 4) {
				neighX = x + 2 * (x3 - scale) + 1;
				neighY = y - 1;
			}
			else {
				neighX = x + 2 * (x3 - scale);
				neighY = y;
			}
		}
		else if (y2 < 1) {
			neighX = x + 8 * scale;
			neighY = y - 2;
		}
		else if (y2 < 3 * scale) {
			neighX = x - 2 * (scale - x3) + 1;
			neighY = y - 1;
		}
		neighX = (neighX + scale * 10) % (scale * 10);
		r->neighbors[D_NORTHWEST] = ar->GetRegion(neighX, neighY);
	}
}

void ARegionList::CalcDensities()
{
	Awrite("Densities:");
	int arr[R_NUM];
	int i;
	for (i=0; i<R_NUM; i++)
		arr[i] = 0;
	forlist(this) {
		ARegion *reg = ((ARegion *) elem);
		arr[reg->type]++;
	}
	for (i=0; i<R_NUM; i++)
		if (arr[i]) Awrite(AString(TerrainDefs[i].name) + " " + arr[i]);

	Awrite("");
}

void ARegionList::TownStatistics()
{
	int villages = 0;
	int towns = 0;
	int cities = 0;
	forlist(this) {
		ARegion *reg = ((ARegion *) elem);
		if (reg->town) {
			switch(reg->town->TownType()) {
				case TOWN_VILLAGE:
					villages++;
					break;
				case TOWN_TOWN:
					towns++;
					break;
				case TOWN_CITY:
					cities++;
			}	
		}
	}
	int tot = villages + towns + cities;
	int perv = villages * 100 / tot;
	int pert = towns * 100 / tot;
	int perc = cities * 100 / tot;
	Awrite(AString("Settlements: ") + tot);
	Awrite(AString("Villages: ") + villages + " (" + perv + "%)");
	Awrite(AString("Towns   : ") + towns + " (" + pert + "%)");
	Awrite(AString("Cities  : ") + cities + " (" + perc + "%)");
	Awrite("");
}

ARegion *ARegionList::FindGate(int x)
{
	if (x == -1) {
		int count = 0;

		forlist(this) {
			ARegion *r = (ARegion *) elem;
			if (r->gate)
				count++;
		}
		count = getrandom(count);
		forlist_reuse(this) {
			ARegion *r = (ARegion *) elem;
			if (r->gate) {
				if (!count)
					return r;
				count--;
			}
		}

		return 0;
	}
	forlist(this) {
		ARegion *r = (ARegion *) elem;
		if (r->gate == x) return r;
	}
	return 0;
}

int ARegionList::GetPlanarDistance(ARegion *one, ARegion *two,
		int penalty, int maxdist)
{
	if (one->zloc == ARegionArray::LEVEL_NEXUS ||
			two->zloc == ARegionArray::LEVEL_NEXUS)
		return 10000000;

	if (Globals->ABYSS_LEVEL) {
		// make sure you cannot teleport into or from the abyss
		int ablevel = Globals->UNDERWORLD_LEVELS +
			Globals->UNDERDEEP_LEVELS + 2;
		if (one->zloc == ablevel || two->zloc == ablevel)
			return 10000000;
	}

	int one_x, one_y, two_x, two_y;
	int maxy;
	ARegionArray *pArr=pRegionArrays[ARegionArray::LEVEL_SURFACE];

	one_x = one->xloc * GetLevelXScale(one->zloc);
	one_y = one->yloc * GetLevelYScale(one->zloc);

	two_x = two->xloc * GetLevelXScale(two->zloc);
	two_y = two->yloc * GetLevelYScale(two->zloc);

	if (Globals->ICOSAHEDRAL_WORLD) {
		int zdist, i;
		ARegion *start, *target, *queue;

		start = pArr->GetRegion(one_x, one_y);
		if (start == 0) {
			one_x += GetLevelXScale(one->zloc) - 1;
			one_y += GetLevelYScale(one->zloc) - 1;
			start = pArr->GetRegion(one_x, one_y);
		}

		target = pArr->GetRegion(two_x, two_y);
		if (target == 0) {
			two_x += GetLevelXScale(two->zloc) - 1;
			two_y += GetLevelYScale(two->zloc) - 1;
			target = pArr->GetRegion(two_x, two_y);
		}

		if (start == 0 || target == 0) {
			// couldn't find equivalent locations on
			// the surface (this should never happen)
			Awrite(AString("Unable to find ends pathing from (") +
				one->xloc + "," +
				one->yloc + "," +
				one->zloc + ") to (" +
				two->xloc + "," +
				two->yloc + "," +
				two->zloc + ")!");
			return 10000000;
		}

		forlist(this) {
			ARegion *r = (ARegion *) elem;
			r->distance = -1;
			r->next = 0;
		}
		
		zdist = (one->zloc - two->zloc);
		if (zdist < 0) zdist = -zdist;
		start->distance = zdist * penalty;
		queue = start;
		while (maxdist == -1 || start->distance <= maxdist) {
			if (start->xloc == two_x && start->yloc == two_y) {
				// found our target within range
				return start->distance;
			}
			// add neighbours to the search list
			for (i = 0; i < NDIRS; i++)
				if (start->neighbors[i] &&
					start->neighbors[i]->distance == -1) {
					queue->next = start->neighbors[i];
					queue = queue->next;
					queue->distance = start->distance + 1;
				}
			start = start->next;
			if (start == 0)
			{
				// ran out of hexes to search
				// (this should never happen)
				Awrite(AString("Unable to find path from (") +
					one->xloc + "," +
					one->yloc + "," +
					one->zloc + ") to (" +
					two->xloc + "," +
					two->yloc + "," +
					two->zloc + ")!");
				return 10000000;
			}
		}
		// didn't find the target within range
		return start->distance;
	} else {
		maxy = one_y - two_y;
		if (maxy < 0) maxy=-maxy;

		int maxx = one_x - two_x;
		if (maxx < 0) maxx = -maxx;

		int max2 = one_x + pArr->x - two_x;
		if (max2 < 0) max2 = -max2;
		if (max2 < maxx) maxx = max2;

		max2 = one_x - (two_x + pArr->x);
		if (max2 < 0) max2 = -max2;
		if (max2 < maxx) maxx = max2;

		if (maxy > maxx) maxx = (maxx+maxy)/2;

		if (one->zloc != two->zloc) {
			int zdist = (one->zloc - two->zloc);
			if ((two->zloc - one->zloc) > zdist)
				zdist = two->zloc - one->zloc;
			maxx += (penalty * zdist);
		}

		return maxx;
	}
}

ARegionArray *ARegionList::GetRegionArray(int level)
{
	return(pRegionArrays[level]);
}

void ARegionList::CreateLevels(int n)
{
	numLevels = n;
	pRegionArrays = new ARegionArray *[n];
}

ARegionArray::ARegionArray(int xx, int yy)
{
	x = xx;
	y = yy;
	regions = new ARegion *[x * y / 2 + 1];
	strName = 0;

	int i;
	for (i = 0; i < x * y / 2; i++) regions[i] = 0;
}

ARegionArray::~ARegionArray()
{
	if (strName) delete strName;
	delete [] regions;
}

void ARegionArray::SetRegion(int xx, int yy, ARegion *r)
{
	regions[xx / 2 + yy * x / 2] = r;
}

ARegion *ARegionArray::GetRegion(int xx, int yy)
{
	xx = (xx + x) % x;
	yy = (yy + y) % y;
	if ((xx + yy) % 2) return(0);
	return(regions[xx / 2 + yy * x / 2]);
}

void ARegionArray::SetName(char const *name)
{
	if (name) {
		strName = new AString(name);
	} else {
		delete strName;
		strName = 0;
	}
}

ARegionFlatArray::ARegionFlatArray(int s)
{
	size = s;
	regions = new ARegion *[s];
}

ARegionFlatArray::~ARegionFlatArray()
{
	if (regions) delete regions;
}

void ARegionFlatArray::SetRegion(int x, ARegion *r) {
	regions[x] = r;
}

ARegion *ARegionFlatArray::GetRegion(int x) {
	return regions[x];
}

int ParseTerrain(AString *token)
{
	for (int i = 0; i < R_NUM; i++) {
		if (*token == TerrainDefs[i].type) return i;
	}
	
	for (int i = 0; i < R_NUM; i++) {
		if (*token == TerrainDefs[i].name) return i;
	}
	
	return (-1);
}

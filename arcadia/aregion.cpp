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
	for(int i = 0; i < NDIRS; i++)
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
	basepop = 0;
	activity = 0;
	growth = 0;
	mortality = 0;
}

TownInfo::~TownInfo()
{
	if (name) delete name;
}

void TownInfo::Readin(Ainfile *f, ATL_VER &v)
{
	name = f->GetStr();
	pop = f->GetInt();
	basepop = f->GetInt();
	growth = f->GetInt();
	mortality = f->GetInt();
}

void TownInfo::Writeout(Aoutfile *f)
{
	f->PutStr(*name);
	f->PutInt(pop);
	f->PutInt(basepop);
	f->PutInt(growth);
	f->PutInt(mortality);
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
	willsink = 0;
	town = 0;
	development = 0;
	habitat = 0;
	mortality = 0;
	growth = 0;
	migration = 0;
	culture = 0;
	clearskies = 0;
	earthlore = 0;
	fog = 0;
	flagpole = FL_NULL;
	timesmarker = 0;
	dynamicexits = 0;
	for (int i=0; i<NDIRS; i++)
		neighbors[i] = 0;
	for (int i=0; i<NDIRS; i++)
		hexside[i] = 0;
}

ARegion::~ARegion()
{
	if (name) delete name;
	if (town) delete town;
	events.DeleteAll(); //why is this needed? Is it needed?
}

void ARegion::ZeroNeighbors()
{
	for (int i=0; i<NDIRS; i++) {
		neighbors[i] = 0;
	}
}

void ARegion::SetName(char *c)
{
	if(name) delete name;
	name = new AString(c);
}



int ARegion::IsNativeRace(int item)
{
	TerrainType *typer = &(TerrainDefs[type]);
	int coastal = sizeof(typer->coastal_races)/sizeof(int);
	int noncoastal = sizeof(typer->races)/sizeof(int);
	if(IsCoastal()) {
		for(int i=0; i<coastal; i++) {
			if (item == typer->coastal_races[i]) return 1;
		}
	}
	for(int i=0; i<noncoastal; i++) {
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

	if(!tt->lairChance) return;

	int check = getrandom(100);
	if(check >= tt->lairChance) return;

	int count = 0;
	unsigned int c;
	for(c = 0; c < sizeof(tt->lairs)/sizeof(int); c++) {
		if(tt->lairs[c] != -1) {
			if(!(ObjectDefs[tt->lairs[c]].flags & ObjectType::DISABLED)) {
				count++;
			}
		}
	}
	count = getrandom(count);

	int lair = -1;
	for(c = 0; c < sizeof(tt->lairs)/sizeof(int); c++) {
		if(tt->lairs[c] != -1) {
			if(!(ObjectDefs[tt->lairs[c]].flags & ObjectType::DISABLED)) {
				if(!count) {
					lair = tt->lairs[c];
					break;
				}
				count--;
			}
		}
	}

	if(lair != -1) {
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
cout << ".";
	SetupPop();
cout << ".";
	//
	// Make the dummy object
	//
	Object *obj = new Object(this);
	objects.Add(obj);
cout << ".";
	if(Globals->LAIR_MONSTERS_EXIST)
		LairCheck();
}

int ARegion::TraceConnectedRoad(int dir, int sum, AList *con, int range)
{
	ARegionPtr *rn = new ARegionPtr();
	rn->ptr = this;
	int isnew = 1;
	forlist(con) {
		ARegionPtr *reg = (ARegionPtr *) elem;
		if ((reg) && (reg->ptr)) if(reg->ptr == this) isnew = 0;
	}
	if(isnew == 0) return sum;
	con->Add(rn);
#if 0
	Awrite(AString(" -> ") + *name + "(" + xloc + ", " + yloc + ")");
	Awrite(AString("   +") + (town->TownType()+1));
#endif
	if(town) sum += town->TownType() + 1;
	if(range > 0) {
		for(int d=0; d<NDIRS; d++) {
			if(!HasExitRoad(d)) continue;
			ARegion *r = neighbors[d];
			if(!r) continue;
			if(dir == r->GetRealDirComp(d)) continue;
			if(r->HasConnectingRoad(d)) sum = r->TraceConnectedRoad(d, sum, con, range-1);
		}
	}
	return sum;
}

int ARegion::CountRoadConnectedTowns(int range)
{
	int townsum = 0;
	AList *con = new AList();
	ARegionPtr *rp = new ARegionPtr();
	rp->ptr = this;
	con->Add(rp);
	for(int d=0; d<NDIRS; d++) {
		if(!HasExitRoad(d)) continue;
		ARegion *r = neighbors[d];
		if(!r) continue;
		if(r->HasConnectingRoad(d)) townsum = r->TraceConnectedRoad(d, townsum, con, range-1);
	}
	return townsum;	
}

// AS
void ARegion::DoDecayCheck(ARegionList *pRegs)
{
	forlist (&objects) {
		Object *o = (Object *) elem;
		if(!(ObjectDefs[o->type].flags & ObjectType::NEVERDECAY)) {
			DoDecayClicks(o, pRegs);
		}
	}
}

// AS
void ARegion::DoDecayClicks(Object *o, ARegionList *pRegs)
{
	if(ObjectDefs[o->type].flags & ObjectType::NEVERDECAY) return;

	int clicks = getrandom(GetMaxClicks());
	clicks += PillageCheck();

	if(clicks > ObjectDefs[o->type].maxMonthlyDecay)
		clicks = ObjectDefs[o->type].maxMonthlyDecay;

	o->incomplete += clicks;

	if(o->incomplete > 0) {
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
/* Hex Patch Dec '03 */
    if(!Globals->HEXSIDE_TERRAIN) {
    	forlist (&objects) {
    		Object *o = (Object *) elem;
    		if(o->IsRoad() && o->incomplete < 1) return 1;
    	}
	}
	else {
	    for(int i=0;i<6;i++) {
    	    if(hexside[i]->road == -1) return 1;
        }
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
//BS - This will stuff up on really small maps where some hexes may have multiple connections
//to another hex - this may return the wrong connection.
//Also, this seems to return zero (=D_NORTH) by default. Is this wise?
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

void ARegion::UpdateProducts()
{
	forlist (&products) {
		Production *prod = (Production *) elem;
		int lastbonus = prod->baseamount / 2;
		int bonus = 0;

		if (prod->itemtype == I_SILVER && prod->skill == -1) continue;

		forlist (&objects) {
			Object *o = (Object *) elem;
			if (o->incomplete < 1 &&
					ObjectDefs[o->type].productionAided == prod->itemtype) {
				lastbonus /= 2;
				bonus += lastbonus;
			}
		}
		prod->amount = prod->baseamount + bonus;

		if (prod->itemtype == I_GRAIN || prod->itemtype == I_LIVESTOCK) {
			if(!Globals->ARCADIA_MAGIC) prod->amount += ((earthlore + clearskies) * 40) / prod->baseamount;
			else prod->amount += ((2*earthlore + clearskies) * prod->baseamount) / 6;
		}
		if (prod->itemtype == I_FISH) prod->amount += clearskies * prod->baseamount / 2;
	}
}

AString ARegion::ShortPrint(ARegionList *pRegs)
{
	AString temp = TerrainDefs[type].name;
	temp += AString(" (") + xloc + "," + yloc;

	ARegionArray *pArr = pRegs->pRegionArrays[zloc];
	if(pArr->strName) {
		temp += ",";
		if(Globals->EASIER_UNDERWORLD &&
		   (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS > 1)) {
			temp += AString("") + zloc + " <";
		} else {
			// add less explicit multilevel information about the underworld
			if(zloc > 2 && zloc < Globals->UNDERWORLD_LEVELS+2) {
				for(int i = zloc; i > 3; i--) {
					temp += "very ";
				}
				temp += "deep ";
			} else if((zloc > Globals->UNDERWORLD_LEVELS+2) &&
					  (zloc < Globals->UNDERWORLD_LEVELS +
					   Globals->UNDERDEEP_LEVELS + 2)) {
				for(int i = zloc; i > Globals->UNDERWORLD_LEVELS + 3; i--) {
					temp += "very ";
				}
				temp += "deep ";
			}
		}
		temp += *pArr->strName;
		if(Globals->EASIER_UNDERWORLD &&
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
    //edit for times reporting of dead guards.
    if(u->type == U_GUARD || u->type == U_GUARDMAGE) {
        timesmarker = 1;    
    }

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
			    u->items.SetNum(i->type, 0);
				// If we're in ocean and not in a structure, make sure that
				// the first unit can actually hold the stuff and not drown
				// If the item would cause them to drown then they won't
				// pick it up.
				if(TerrainDefs[type].similar_type == R_OCEAN) {
					if(first->object->type == O_DUMMY) {
						if(!first->CanReallySwim()) {
							first->items.SetNum(i->type,
									first->items.GetNum(i->type) - i->num);
						}
					}
				}
			}
		}
	}

//BS mod - units at this stage still have men in them! Shouldn't matter ... I hope! (needed for mages to rise again!)
    if(u->type != U_MAGE || !Globals->ARCADIA_MAGIC || !u->IsAlive()) {
    	u->MoveUnit(0);
    	hell.Add(u);
	} else {
	//mage under arcadia
	    u->MoveUnit(GetDummy());
	    u->dead = u->faction->num;
	    u->canattack = 0;
	    //cannot clear any orders which may have led to the battle, or might later double destruct the order
	    u->SafeClearOrders();
	    //faction changed in post-turn processing, or in Game::KillDead if died in battle.
	}
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

Unit *ARegion::GetUnit(int num)
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		Unit *u = obj->GetUnit(num);
		if(u) {
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
		if(u) {
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
//    if(!id || !cur) return NULL;   //for safety
    
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
    if(!Globals->ARCADIA_MAGIC) {
    	forlist((&objects)) {
    		Object *obj = (Object *) elem;
    		forlist((&obj->units))
    			if (((Unit *) elem)->faction == f) return 1;
    	}
	} else {
    	forlist((&objects)) {
    		Object *obj = (Object *) elem;
    		forlist((&obj->units))
    			if ((((Unit *) elem)->faction == f) && (((Unit *) elem)->dead == 0)) return 1;
    	}
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
	if(gate > 0) f->PutInt(gatemonth);
	f->PutInt(willsink);
	f->PutInt(flagpole);
	
	if (race != -1) f->PutStr(ItemDefs[race].abr);
	else f->PutStr("NO_RACE");
	f->PutInt(population);
	f->PutInt(basepopulation);
	f->PutInt(migration);
	f->PutInt(wages);
	f->PutInt(maxwages);
	f->PutInt(money);

	f->PutInt(elevation);
	f->PutInt(humidity);
	f->PutInt(temperature);
	f->PutInt(vegetation);
	f->PutInt(culture);

	f->PutInt(habitat);
	f->PutInt(development);
	f->PutInt(growth);
	f->PutInt(mortality);

	if (town) {
		f->PutInt(1);
		town->Writeout(f);
	} else {
		f->PutInt(0);
	}

	f->PutInt(xloc);
	f->PutInt(yloc);
	f->PutInt(zloc);

	products.Writeout(f);
	markets.Writeout(f);

	for(int i=0; i<6; i++) {
	    if(neighbors[i] && (neighbors[i]->num < num)) continue;
	    else 
            {
	        f->PutInt(i);
#ifdef DEBUG
	        if(!hexside[i]) {
                Awrite("bugger!");
                system("PAUSE");
                Awrite(AString("missing hexside ") + i);
                f->PutInt(0);
                f->PutInt(0);
                f->PutInt(0);
            } else
#endif        
            hexside[i]->Writeout(f);
            delete hexside[i];            //otherwise they seem to never get cleared.
        }
	}
	f->PutInt(-1);

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
	if(gate > 0) gatemonth = f->GetInt();
	willsink = f->GetInt();
	flagpole = f->GetInt();
	
	temp = f->GetStr();
	race = LookupItem(temp);
	delete temp;

	population = f->GetInt();
	basepopulation = f->GetInt();
	migration = f->GetInt();
	wages = f->GetInt();
	maxwages = f->GetInt();
	money = f->GetInt();
	
	elevation = f->GetInt();
	humidity = f->GetInt();
	temperature = f->GetInt();
	vegetation = f->GetInt();
	culture = f->GetInt();

	habitat = f->GetInt();
	development = f->GetInt();
	growth = f->GetInt();
	mortality = f->GetInt();

	if (f->GetInt()) {
		town = new TownInfo;
		town->Readin(f, v);
	} else {
		town = 0;
	}

	xloc = f->GetInt();
	yloc = f->GetInt();
	zloc = f->GetInt();

	products.Readin(f);
	markets.Readin(f);

	int done = 0;
	while(!done) {
	    int side = f->GetInt();
	    if(side == -1) done = 1;
	    else {
	        Hexside *temp = new Hexside();
	        temp->Readin(f);
	        hexside[side] = temp;
	    }
    }

	int i = f->GetInt();
	for (int j=0; j<i; j++) {
		Object *temp = new Object(this);
		temp->Readin(f, facs, v);
		objects.Add(temp);
	}
}

int ARegion::CanMakeAdv(Faction *fac, int item)
//currently only called for report writing, so really "can see advanced"
{
	AString skname;
	int sk;
	Farsight *f;
	
	if(ItemDefs[item].flags & ItemType::ALWAYSSEE) return 1;

	if(Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			f = (Farsight *)elem;
			if(f && f->faction == fac && f->unit) {
				skname = ItemDefs[item].pSkill;
				sk = LookupSkill(&skname);
				if(f->unit->GetSkill(sk) >= ItemDefs[item].pLevel)
					return 1;
			}
		}
	}

	if((Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_RESOURCES)) {
		forlist(&passers) {
			f = (Farsight *)elem;
			if(f && f->faction == fac && f->unit) {
				skname = ItemDefs[item].pSkill;
				sk = LookupSkill(&skname);
				if(f->unit->GetSkill(sk) >= ItemDefs[item].pLevel)
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
				if(u->GetSkill(S_BASE_CHARISMA) && Population()) {   //hardcoded charisma effect
				    int charis = u->GetSkill(S_BASE_CHARISMA);
				    while(charis--) {
				        if(getrandom(2)) {
				            u->Event(AString("Learns about the presence of ") + ItemDefs[item].name + " in the local region.");
                            return 1;
                        }
				    }
				}
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
					if((Globals->TRANSIT_REPORT &
							GameDefs::REPORT_SHOW_ENTERTAINMENT) || present) {
						f->PutStr(AString("Entertainment available: $") +
								p->amount + ".");
					} else {
						f->PutStr(AString("Entertainment available: $0."));
					}
				}
			} else {
				if(!present &&
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
		if(!present &&
		   !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_MARKETS))
			continue;
		if (m->type == M_SELL) {
			if (ItemDefs[m->item].type & IT_ADVANCED) {
				if(!Globals->MARKETS_SHOW_ADVANCED_ITEMS) {
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
			if(!present &&
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

	if((Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_WAGES) || present) {
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
	/* Hex Patch Dec '03*/
	AString temp = "";
	int y = 0;
	for (int i=0; i<NDIRS; i++) {
		ARegion *r = neighbors[i];
		if (r && exits_seen[i]) {
		    temp = AString(DirectionStrs[i]) + " : " + r->Print(pRegs);

		    Hexside *h = hexside[i];
		    if(h && h->type) temp += AString(", ") + HexsideDefs[h->type].name;
		    if(h && h->road != 0) {
                temp += AString(", ") + HexsideDefs[H_ROAD].name;
		        if(h->road > 0) temp += AString(" (needs ") + h->road + ")";
            }
		    if(h && h->bridge != 0) {
                temp += AString(", ") + HexsideDefs[H_BRIDGE].name;
		        if(h->bridge > 0) temp += AString(" (needs ") + h->bridge + ")";
            }
		    if(h && h->harbour != 0) {
                temp += AString(", ") + HexsideDefs[H_HARBOUR].name;
		        if(h->harbour > 0) temp += AString(" (needs ") + h->harbour + ")";
            }
            
		    temp += ".";
		    
			f->PutStr(temp);
			y = 1;
		}
		else if (exits_seen[i]) {
		    if(hexside[i]->type) {
    		    Hexside *h = hexside[i];
    		    temp = AString(DirectionStrs[i]) + " : ";
    		    if(h && h->type) temp += HexsideDefs[h->type].name;
    		    
		        temp += ".";
		    
			    f->PutStr(temp);
			    y = 1;
		    }
		}
	}
	if (!y) f->PutStr("none");
	f->DropTab();
	f->EndLine();
}
/*
#define AC_STRING "%s Nexus is a magical place; the entryway " \
"to the world of %s. Enjoy your stay, the city guards should " \
"keep you safe as long as you should choose to stay. However, rumour " \
"has it that once you have left the Nexus, you can never return."
*/


#define AC_STRING "%s Nexus is a magical place; the purgatory " \
"of the world of %s. If you have ended up here, it is because " \
"something has gone wrong. Most likely, too many factions have " \
"already joined the world, and there was no space for your own."


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
			if(Globals->RACES_EXIST) {
				temp += AString(" (") + ItemDefs[race].names + ")";
			}
			if(present || farsight ||
			   Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_REGION_MONEY) {
				temp += AString(", $") + money;
			} else {
				temp += AString(", $0");
			}
		}
		temp += ".";
		f->PutStr(temp);
		f->PutStr("-------------------------------------------------"
				"-----------");

		f->AddTab();
		if(Globals->WEATHER_EXISTS) {
			temp = "It was ";
			if (clearskies) temp += "unnaturally clear ";
			else {
				if(weather == W_BLIZZARD) temp = "There was an unnatural ";
				else if(weather == W_NORMAL) temp = "The weather was ";
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

//write events
		f->PutStr("Events:");
		f->AddTab();
		
		int done = 0;
		if(fog) {
		    done = 1;
		    f->PutStr("There is an unnatural fog over the region.");
		}
		
		
		if(events.First()) {
	        done = 1;
		    forlist(&events) {
		        AString *temp = (AString *) elem;
		        f->PutStr(*temp);
            }
		}

		if(willsink>0) {
            if(willsink>1) temp = AString("This region will sink beneath the seas in ") + willsink + " months";
            else if(willsink == 1) temp = "This region will sink beneath the seas next month.";
            f->PutStr(temp);
            done = 1;
        }
        if (!done) f->PutStr("none");
	    f->DropTab();
	    f->EndLine();
	    
	    int exits_seen[NDIRS];
		if(present || farsight ||
		   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_ALL_EXITS)) {
			for(int i = 0; i < NDIRS; i++)
				exits_seen[i] = 1;
		} else {
			// This is just a transit report and we're not showing all
			// exits.   See if we are showing used exits.

			// Show none by default.
			int i;
			for(i = 0; i < NDIRS; i++)
				exits_seen[i] = 0;
			// Now, if we should, show the ones actually used.
			if(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_USED_EXITS) {
				forlist(&passers) {
					Farsight *p = (Farsight *)elem;
					if(p->faction == fac) {
						for(i = 0; i < NDIRS; i++) {
							exits_seen[i] |= p->exits_used[i];
						}
					}
				}
			}
		}

		WriteExits(f, pRegions, exits_seen);
	    
	    
		if(Globals->GATES_EXIST && gate && gate != -1) {
			int sawgate = 0;
			if(fac->IsNPC())
				sawgate = 1;
			if(Globals->IMPROVED_FARSIGHT && farsight) {
				forlist(&farsees) {
					Farsight *watcher = (Farsight *)elem;
					if(watcher && watcher->faction == fac && watcher->unit) {
						if(watcher->unit->GetSkill(S_GATE_LORE)) {
							sawgate = 1;
						}
					}
				}
			}
			if(Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) {
				forlist(&passers) {
					Farsight *watcher = (Farsight *)elem;
					if(watcher && watcher->faction == fac && watcher->unit) {
						if(watcher->unit->GetSkill(S_GATE_LORE)) {
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
			if(sawgate) {
				if(gateopen) {
					f->PutStr(AString("There is a Gate here (Gate ") + gate +
							" of " + (pRegions->numberofgates) + ").");
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
		if(fac->IsNPC()) {
			obs = 10;
			passobs = 10;
		}
		forlist (&objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == fac && (u->GetSkill(S_MIND_READING) > 2 || (Globals->ARCADIA_MAGIC && u->GetSkill(S_MIND_READING) > 1)) ) {
					detfac = 1;
				}
			}
		}
		if(Globals->IMPROVED_FARSIGHT && farsight) {
			forlist(&farsees) {
				Farsight *watcher = (Farsight *)elem;
				if(watcher && watcher->faction == fac && watcher->unit) {
					if(watcher->unit->GetSkill(S_MIND_READING) > 2 || (Globals->ARCADIA_MAGIC && watcher->unit->GetSkill(S_MIND_READING) > 1) ) {
						detfac = 1;
					}
				}
			}
		}

		if((Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
		   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
			forlist(&passers) {
				Farsight *watcher = (Farsight *)elem;
				if(watcher && watcher->faction == fac && watcher->unit) {
					if(watcher->unit->GetSkill(S_MIND_READING) > 2 || (Globals->ARCADIA_MAGIC && watcher->unit->GetSkill(S_MIND_READING) > 1)) {
						passdetfac = 1;
					}
				}
			}
		}
		{
			forlist (&objects) {
				((Object *) elem)->Report(f, fac, obs, truesight, fog, detfac,
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

	if(Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			Farsight *farsight = (Farsight *)elem;
			if(farsight && farsight->faction == f && farsight->unit) {
				int t = farsight->unit->GetSkill(S_TRUE_SEEING);
				if(t > truesight) truesight = t;
			}
		}
	}

	if(usepassers &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
		forlist(&passers) {
			Farsight *farsight = (Farsight *)elem;
			if(farsight && farsight->faction == f && farsight->unit) {
				int t = farsight->unit->GetSkill(S_TRUE_SEEING);
				if(t > truesight) truesight = t;
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

	if(Globals->IMPROVED_FARSIGHT) {
		forlist(&farsees) {
			Farsight *farsight = (Farsight *)elem;
			if(farsight && farsight->faction == f && farsight->unit) {
				int o = farsight->unit->GetAttribute("observation");				
				if(o > obs) obs = o;
			}
		}
	}
	if(usepassers &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
	   (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
		forlist(&passers) {
			Farsight *farsight = (Farsight *)elem;
			if(farsight && farsight->faction == f && farsight->unit) {
				int o = farsight->unit->GetAttribute("observation");
				if(o > obs) obs = o;
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
	if (TerrainDefs[type].similar_type == R_OCEAN || TerrainDefs[type].similar_type == R_FAKE) return 1;
	int seacount = 0;
	for (int i=0; i<NDIRS; i++) {
		if (neighbors[i] && (TerrainDefs[neighbors[i]->type].similar_type == R_OCEAN || TerrainDefs[neighbors[i]->type].similar_type == R_FAKE) ) {
		    if (!Globals->LAKESIDE_IS_COASTAL && neighbors[i]->type == R_LAKE) continue;
		    seacount++;
	    }
	}
	return seacount;
}

int ARegion::IsCoastalOrLakeside()
{
/* BS Sailing Patch */
	if ((type != R_LAKE) && (TerrainDefs[type].similar_type == R_OCEAN || TerrainDefs[type].similar_type == R_FAKE) ) return 1;
	int seacount = 0;
	for (int i=0; i<NDIRS; i++) {
		if (neighbors[i] && (TerrainDefs[neighbors[i]->type].similar_type == R_OCEAN || TerrainDefs[neighbors[i]->type].similar_type == R_FAKE) ) {
		    seacount++;
	    }
	}
	return seacount;
}

int ARegion::MoveCost(int movetype, ARegion *fromRegion, int dir, AString *road)
{
	int cost = 1;
	if(Globals->WEATHER_EXISTS) {
		cost = 2;
		if (weather == W_BLIZZARD && !clearskies) return 10;
		if (weather == W_NORMAL || clearskies) cost = 1;
	}
	if (movetype != M_FLY) { //ie walk, ride, swim* or none   *: don't think swim gets sent here
		cost = (TerrainDefs[type].movepoints * cost);

		/* Hex Patch Dec '03 */
    	if(!(Globals->HEXSIDE_TERRAIN)) {
    		if(fromRegion->HasExitRoad(dir) && fromRegion->HasConnectingRoad(dir)) {
    			cost -= cost/2;
    			if (road)
    				*road = "on a road ";
    		}
        }
    	if(Globals->HEXSIDE_TERRAIN && dir < NDIRS) {        //NDIRS condition in case of MOVE_IN.
    		int block = 0;
    		/* Hex Patch Dec '03 */
    		Hexside *h = fromRegion->hexside[dir];
    		if(h->road < 0) {
    		    cost = (cost * (HexsideDefs[H_ROAD].movementmultiplier + 2) + 1) /2 ;
    		    block += HexsideDefs[H_ROAD].blockeffect;
    		}
    		if(h->bridge < 0) {
    		    cost = (cost * (HexsideDefs[H_BRIDGE].movementmultiplier + 2) + 1) /2;
    		    block += HexsideDefs[H_BRIDGE].blockeffect;
    		}
    		if(h->type) {
    		    cost = (cost * (HexsideDefs[h->type].movementmultiplier + 2) + 1) /2;
    		    block += HexsideDefs[h->type].blockeffect;
    		}
     			
   			if(block>0) return -1; // note this only applies to walking and riding				
         }
		
	}
	if(cost < 1) cost = 1;
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

int ARegion::NotifySpell(Unit *caster, char *spell, ARegionList *pRegs)
{
	AList flist;
	unsigned int i;

	SkillType *pS = FindSkill(spell);

	if (!(pS->flags & SkillType::NOTIFY)) {
		// Okay, we aren't notifyable, check our prerequisites
		for(i = 0; i < sizeof(pS->depends)/sizeof(SkillDepend); i++) {
			if (pS->depends[i].skill == NULL) break;
			if(NotifySpell(caster, pS->depends[i].skill, pRegs)) return 1;
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
			if (u2->guard == GUARD_GUARD && u2->IsReallyAlive())
			    if(u2->type == U_GUARD || u2->type == U_GUARDMAGE) {
			        if(town) return 0; //guards prevent taxing in towns
			        if(u2->faction->ethnicity != u->faction->ethnicity) return 0; //guards prevent other ethnicities taxing
				} else if (u2->GetAttitude(this, u) <= A_NEUTRAL) return 0;
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
			if(u2->guard == GUARD_GUARD && u2->IsReallyAlive() &&
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

void ARegion::DefaultOrders(int peasantfaction)
//peasantfaction is carried for ARCADIA MAGIC, it has no effect if ARCADIA_MAGIC is disabled.
{
	forlist((&objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units))
			((Unit *) elem)->DefaultOrders(obj, peasantfaction);
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

ARegionList::ARegionList()
{
	pRegionArrays = 0;
	numLevels = 0;
	numberofgates = 0;
}

ARegionList::~ARegionList()
{
	if(pRegionArrays) {
		int i;
		for(i = 0; i < numLevels; i++) {
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
	for(i = 0; i < numLevels; i++) {
		ARegionArray *pRegs = pRegionArrays[i];
		f->PutInt(pRegs->x);
		f->PutInt(pRegs->y);
		if(pRegs->strName) {
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
			for(i = 0; i < NDIRS; i++) {
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
	for(i = 0; i < numLevels; i++) {
		int curX = f->GetInt();
		int curY = f->GetInt();
		AString *name = f->GetStr();
		ARegionArray *pRegs = new ARegionArray(curX, curY);
		if(*name == "none") {
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
	for(i = 0; i < num; i++) {
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
			for(i = 0; i < NDIRS; i++) {
				int j = f->GetInt();
				if (j != -1) {
					reg->neighbors[i] = fa.GetRegion(j);
				} else {
					reg->neighbors[i] = 0;
				}
			}
		}
	}
	Awrite("Setting up the hexsides...");
	{
	    forlist(this) {
	        ARegion *reg = (ARegion *) elem;
	        for(i = 0; i < NDIRS; i++) {
	            if(!reg->hexside[i]) {
	                if(i<3 && reg->neighbors[i]->hexside[i+3]) reg->hexside[i] = reg->neighbors[i]->hexside[i+3];
	                else if(i>2 && reg->neighbors[i]->hexside[i-3]) reg->hexside[i] = reg->neighbors[i]->hexside[i-3];
	                else {
	                    Awrite("bugger!");
	                    #ifdef DEBUG
	                    system("pause");
	                    #endif
	                    Hexside *temp = new Hexside;
	                    reg->hexside[i] = temp;
                    }
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
	if(!Globals->FLAT_WORLD) {
    	x = (x + arr->x) % arr->x;
    	y = (y + arr->y) % arr->y;
	} else {
	    if(x < 0 || x >= arr->x) return 0;
	    if(y < 0 || y >= arr->y) return 0;
	}
	
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

void ARegionList::EditNeighSetup(ARegion *r, ARegionArray *ar)
{
    NeighSetup(r,ar);
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
		if(arr[i]) Awrite(AString(TerrainDefs[i].name) + " " + arr[i]);

	Awrite("");
}

ARegion *ARegionList::FindGate(int x)
{
	if (!x) return 0;
	forlist(this) {
		ARegion *r = (ARegion *) elem;
		if (r->gate == x) return r;
	}
	return 0;
}

int ARegionList::GetPlanarDistance(ARegion *one, ARegion *two, int penalty)
{
	if(one->zloc == ARegionArray::LEVEL_NEXUS ||
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

	maxy = one_y - two_y;
	if(maxy < 0) maxy=-maxy;

	int maxx = one_x - two_x;
	if(maxx < 0) maxx = -maxx;
	
	if(!Globals->FLAT_WORLD) {
    	int max2 = one_x + pArr->x - two_x;
    	if(max2 < 0) max2 = -max2;
    	if(max2 < maxx) maxx = max2;
    
    	max2 = one_x - (two_x + pArr->x);
    	if(max2 < 0) max2 = -max2;
    	if(max2 < maxx) maxx = max2;
	}

	if(maxy > maxx) maxx = (maxx+maxy)/2;

	if(one->zloc != two->zloc) {
		int zdist = (one->zloc - two->zloc);
		if ((two->zloc - one->zloc) > zdist)
			zdist = two->zloc - one->zloc;
		maxx += (penalty * zdist);
	}

	return maxx;
}

int ARegionList::GetDistance(ARegion *one, ARegion *two)
{
	if(one->zloc != two->zloc) return(10000000);

	ARegionArray *pArr = pRegionArrays[one->zloc];

	int maxy;
	maxy = one->yloc - two->yloc;
	if (maxy < 0) maxy = -maxy;

	int maxx = one->xloc - two->xloc;
	if (maxx < 0) maxx = -maxx;
	
	if(!Globals->FLAT_WORLD) {
    	int max2 = one->xloc + pArr->x - two->xloc;
    	if (max2 < 0) max2 = -max2;
    	if (max2 < maxx) maxx = max2;
    
    	max2 = one->xloc - (two->xloc + pArr->x);
    	if (max2 < 0) max2 = -max2;
    	if (max2 < maxx) maxx = max2;
	}
	
	if (maxy > maxx) return (maxx + maxy) / 2;
	return maxx;
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
	for(i = 0; i < x * y / 2; i++) regions[i] = 0;
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
    if(!Globals->FLAT_WORLD) {
    	xx = (xx + x) % x;
    	yy = (yy + y) % y;
	} else {
	    if(xx<0 || xx>=x) return 0;
	    if(yy<0 || yy>=y) return 0;
	}
	if((xx + yy) % 2) return(0);
	return(regions[xx / 2 + yy * x / 2]);
}

void ARegionArray::SetName(char *name)
{
	if(name) {
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

void ARegion::Fake()
{
    type = R_FAKE;
    if(Globals->HEXSIDE_TERRAIN) {
        for (int i=0; i<NDIRS; i++) {
    		hexside[i]->type = H_DUMMY;
    		hexside[i]->road = 0;
    		hexside[i]->bridge = 0;
    		hexside[i]->harbour = 0;
  		}
    }
    
    //gates left so the gate spell doesn't crash
    willsink = 0;
    untaxed = 0;
    if(town) delete town;
    town = 0;
    //economics stuff
    development = 0;
	habitat = 0;
	mortality = 0;
	growth = 0;
	migration = 0;
	culture = 0;
	clearskies = 0;
	earthlore = 0;
	fog = 0;
	flagpole = FL_NULL;

	race = -1;

	population = 0;
	basepopulation = 0;
	wages = 0;
	maxwages = 0;
	money = 0;
	
	elevation = 0;
	humidity = 0;
	temperature = 0;
	vegetation = 0;

	products.DeleteAll();
	markets.DeleteAll();
}

void ARegion::SinkRegion(ARegionList *pRegs)
{
    if(TerrainDefs[type].similar_type == R_OCEAN) return;
    if(IsCoastal()) type = R_OCEAN;
    else type = R_LAKE;
    
    AString ocean_name = Globals->WORLD_NAME;
	ocean_name += " Ocean";
	SetName(ocean_name.Str());
   
    forlist(&objects) {
       	Object *obj = (Object *) elem;
       	if(obj->IsBoat()) {
       	    if(Globals->HEXSIDE_TERRAIN) obj->hexside = -1;
       	} else {
    		forlist((&obj->units)) {
    			Unit *u = (Unit *) elem;
				int drown = 0;
				if(u->type == U_WMON) {
					 // Make sure flying monsters only drown if we
					 // are in WFLIGHT_NONE mode
					if(Globals->FLIGHT_OVER_WATER==GameDefs::WFLIGHT_NONE)
						drown = !(u->CanReallySwim());
					else
						drown = !(u->CanSwim());
				} else {
					switch(Globals->FLIGHT_OVER_WATER) {
						case GameDefs::WFLIGHT_UNLIMITED:
							drown = !(u->CanSwim());
							break;
						case GameDefs::WFLIGHT_MUST_LAND:
							drown = !(u->CanReallySwim() || u->leftShip);
							u->leftShip = 0;
							break;
						case GameDefs::WFLIGHT_NONE:
							drown = !(u->CanReallySwim());
							break;
						default: // Should never happen
							drown = 1;
							break;
					}
				}
				if (drown) {
					Kill(u);
					u->Event("Drowns in the ocean.");
				}
            }
            if(obj->type != O_DUMMY) {
            //Move non-drowning units out
		        Object *dest = GetDummy();
		        forlist(&obj->units) {
					Unit *u = (Unit *) elem;
					u->MoveUnit(dest);
			    }
			//if inner object, destroy link (if any) at other end.
                if(obj->inner > -1) {
                    ARegion *newreg = pRegs->GetRegion(obj->inner);
                    forlist(&newreg->objects) {
                        Object *ob = (Object *) elem;
                        if(ob->inner == num) {    //if it's inner and pointing here, it dies, even if it's mirror is something else
                            Object *dest = newreg->GetDummy();
		                    forlist(&obj->units) {
					            Unit *u = (Unit *) elem;
					            u->MoveUnit(dest);
				            }
                            newreg->objects.Remove(ob);
                            delete ob;
                        }
                    }
                }
			//destroy object
                objects.Remove(obj);
                delete obj;
            }
        }
    }
    buildingseq = 1;
// add beaches.
/* Hex Patch Dec '03 */
    if (Globals->HEXSIDE_TERRAIN) {
        for(int k=0; k<6; k++) {
            if(!neighbors[k]) continue;
            if(neighbors[k]->type == R_LAKE) neighbors[k]->type = R_OCEAN;
            if(TerrainDefs[neighbors[k]->type].similar_type == R_OCEAN) {
                hexside[k]->type = H_DUMMY;
                hexside[k]->bridge = 0;
                hexside[k]->road = 0;        
                continue;
            }
    
    		int coasttype = H_DUMMY;
    	    if(!(HexsideDefs[H_BEACH].flags & HexsideType::DISABLED)) coasttype = H_BEACH;
    		if(!(HexsideDefs[H_HARBOUR].flags & HexsideType::DISABLED)) {
                if(coasttype == H_DUMMY || getrandom(100) < 10) coasttype = H_HARBOUR;
            }
            if(!(HexsideDefs[H_ROCKS].flags & HexsideType::DISABLED)) {
                if(coasttype == H_DUMMY || getrandom(100) < 5) coasttype = H_ROCKS;
            }
    		
    		hexside[k]->type = coasttype;
    		//destroy bridges & roads
    		hexside[k]->bridge = 0;
    		hexside[k]->road = 0;
        }
    }
    
// convert neighbour lakes to ocean
    for(int k=0; k<6; k++) {
        if(!neighbors[k]) continue;
        if((TerrainDefs[neighbors[k]->type].similar_type == R_OCEAN) && (neighbors[k]->type != R_OCEAN)) neighbors[k]->type = R_OCEAN; //ie if it's water but not ocean ...
   	}


// redo economy
    if(town) {
        //move peasants out to a neighbouring region, if there is one.
        int emptyregions = 0;
        int townregions = 0;
        int newdir = -1;
        for(int i=0; i<NDIRS; i++) {
            if(neighbors[i] && TerrainDefs[neighbors[i]->type].similar_type != R_OCEAN) {
                if(neighbors[i]->town) townregions++;
                else {
                    emptyregions++;
                    if(!getrandom(emptyregions)) newdir = i;  //getrandom(1) is always 0.
                }
            }
        }
        
        if(newdir != -1) {
            neighbors[newdir]->town = town;
            town = NULL;
        } else {
            if(townregions) {
                //we can't move the whole town. Move the peasants into neighbouring towns if possible.
                int peasants = town->pop;
                int basepeasants = town->basepop;
                for(int i=0; i<NDIRS; i++) {
                    if(neighbors[i] && neighbors[i]->town) {
                        neighbors[i]->town->pop += peasants/townregions;
                        neighbors[i]->town->basepop += basepeasants/townregions;
                        peasants -= peasants/townregions;
                        basepeasants -= basepeasants/townregions;
                        townregions--;
                        if(neighbors[i]->town->pop > 6000) {
                            //new town too big, remove some people.
                            peasants += neighbors[i]->town->pop - 6000;
                            basepeasants += neighbors[i]->town->pop - 6000;
                            neighbors[i]->town->basepop -= neighbors[i]->town->pop - 6000;
                            neighbors[i]->town->pop = 6000;
                        }
                    }
                    if(peasants) {
                        //still got some peasants. 
                        //well, bugger that. 6000 people in one town means they're rich anyway.
                    }
                }
            }
            delete town;
            town = NULL;        
        }
    }

    products.DeleteAll();
    SetupProds();

    markets.DeleteAll();

    SetupEditRegion();
	UpdateEditRegion();
}

void ARegion::OceanToLand()
// exact land type will be set later
// economy to be reset later
{
int lastocean = -1;

    //have to set this to land to generate lakes properly later
    if(TerrainDefs[type].similar_type == R_OCEAN) type = R_MOUNTAIN;

    if(Globals->HEXSIDE_TERRAIN) {
    
        for(int k=0; k<6; k++) {
            if(neighbors[k]) {
                if(TerrainDefs[neighbors[k]->type].similar_type == R_OCEAN) lastocean = k;
            }
        }

        forlist(&objects) {
           	Object *obj = (Object *) elem;
           	if(obj->IsBoat()) {
           	    if(Globals->HEXSIDE_TERRAIN && ObjectDefs[obj->type].hexside) obj->hexside = lastocean;  // note this may place ocean-only ships on beaches!
       	    } else if(obj->type != O_DUMMY) {
		        Object *dest = GetDummy();
		        forlist(&obj->units) {
					Unit *u = (Unit *) elem;
					u->MoveUnit(dest);
			    }
			    //destroy object
                objects.Remove(obj);
                delete obj;
            }       	    
       	    
       	}

// add/remove beaches.
        for(int k=0; k<6; k++) {
            if(!neighbors[k]) continue;
            if(!TerrainDefs[neighbors[k]->type].similar_type == R_OCEAN) {
                hexside[k]->type = H_DUMMY; //gets rid of beaches
                continue;
            }
    		int coasttype = H_DUMMY;
    	    if(!(HexsideDefs[H_BEACH].flags & HexsideType::DISABLED)) coasttype = H_BEACH;
    		if(!(HexsideDefs[H_HARBOUR].flags & HexsideType::DISABLED)) {
                if(coasttype == H_DUMMY || getrandom(100) < 10) coasttype = H_HARBOUR;
            }
            if(!(HexsideDefs[H_ROCKS].flags & HexsideType::DISABLED)) {
                if(coasttype == H_DUMMY || getrandom(100) < 5) coasttype = H_ROCKS;
            }
    		
    		hexside[k]->type = coasttype;
    		//destroy bridges & roads
    		hexside[k]->bridge = 0;
    		hexside[k]->road = 0;
        }
    }

// convert neighbours to lakes if necessary
    for(int k=0; k<6; k++) {
        if(!neighbors[k]) continue;
        if(TerrainDefs[neighbors[k]->type].similar_type != R_OCEAN) continue;
        int lake = 1;
        for(int j=0; j<6; j++) {
            if(!neighbors[k]->neighbors[j]) continue;
            if(TerrainDefs[neighbors[k]->neighbors[j]->type].similar_type == R_OCEAN) {
                lake = 0;
                break; //if I stick a break in here will it break one loop (good) or both loops (bad) ?
            }
    	}
    	if(lake) neighbors[k]->type = R_LAKE;
   	}
   	#define NUMREGIONNAMES 1000
   	
   	AString tempname = AString(AGetNameString(NUMREGIONNAMES + getrandom(NUMREGIONNAMES)));
   	
   	for(int j=0; j<6; j++) {
   	    if(!neighbors[j]) continue;
   	    if(TerrainDefs[neighbors[j]->type].similar_type == R_OCEAN) continue;
   	    tempname = *(neighbors[j]->name);
   	}
   	
 	SetName(tempname.Str());
}

void ARegion::Event(const AString &s)
{
	AString *temp = new AString(s);
	events.Add(temp);
}

AString *TerrainDescription(int type)
{
	if(TerrainDefs[type].flags & TerrainType::DISABLED)
		return NULL;

	TerrainType *t = &TerrainDefs[type];
	AString *temp = new AString;
	*temp += AString(t->name) + ":";

	if(t->similar_type == R_OCEAN) {
        *temp += " This terrain allows sailing of deep water ships.";
        *temp += AString(" This terrain takes ") + t->movepoints + " move points for a swimming or sailing unit to enter.";
    } else *temp += AString(" This terrain takes ") + t->movepoints + " move points for a walking or riding unit to enter.";
    
    int canfly = (t->flags & TerrainType::FLYINGMOUNTS);
    int canride = (t->flags & TerrainType::RIDINGMOUNTS);
    if(canfly || canride) {
        if(!canfly) *temp += " Riding";
        else if(!canride) *temp += " Flying";
        else *temp += " Flying and riding";
        *temp += " mounts may be used here in battle";
        
        int limfly = (t->flags & TerrainType::FLYINGLIMITED);
        int limride = (t->flags & TerrainType::RIDINGLIMITED);
        if(limfly || limride) {
            if(limfly && canride) *temp += ", although the combat skill bonus will not be added for flying mounts.";
            if(limride && canfly) *temp += ", although the combat skill bonus will not be added for riding mounts.";
            else *temp += ", although the combat skill bonus will not be added.";
        } else *temp += ".";
    }
    
    if(t->flags & TerrainType::RESTRICTEDFOOT) *temp += " This terrain prevents foot troops flanking around outnumbered opponents during battle.";
    if(t->flags & TerrainType::RESTRICTEDRANGED) *temp += " During battle, magic and ranged attacks will suffer a penalty of 1 to their "
                                    "chance-to-attack and attack skill values.";
    
    //put in some if statements for random economy.
    if(t->pop) {
        *temp += AString(" This terrain supports a base population of between ") + (t->pop/2) + " and " + (t->pop-1)
                 + " peasants, with wages from " + t->wages + " to " + (t->wages+2) + " silver. You may find ";
        int comma = 0;
        int last = -1;
        for(unsigned int i=0; i<sizeof(t->races)/sizeof(int); i++) {
            if(t->races[i] != -1) {
                if(last != -1) {
                    if(comma) *temp += ", ";
                    *temp += ItemDefs[last].names;
                    comma = 1;
                }
                last = t->races[i];
            }
        }
        if(last != -1) {
            if(comma) *temp += " or ";
            *temp += ItemDefs[last].names;
        }
        *temp += " here, as well as ";
        comma = 0;
        last = -1;
        for(unsigned int i=0; i<sizeof(t->coastal_races)/sizeof(int); i++) {
            if(t->coastal_races[i] != -1) {
                if(last != -1) {
                    if(comma) *temp += ", ";
                    *temp += ItemDefs[last].names;
                    comma = 1;
                }
                last = t->coastal_races[i];
            }
        }
        if(last != -1) {
            if(comma) *temp += " or ";
            *temp += ItemDefs[last].names;
        }
        *temp += " if the region is adjacent to an ocean, lake or river.";
    }
    else *temp += " No peasants live in this terrain.";

    *temp += " This terrain will always produce ";
    int comma = 0;
    int last = -1;
    if(t->economy) {
        *temp += AString(t->economy) + "-" + (2*t->economy-1) + " livestock or grain";
        comma = 1;
    }    
    for(unsigned int i=0; i<sizeof(t->prods)/sizeof(Product); i++) {
        if(t->prods[i].product != -1 && t->prods[i].chance >= 100) {
            if(last != -1) {
                if(comma) *temp += ", ";
                *temp += AString(t->prods[last].amount) + "-" + (2*t->prods[last].amount-1) + " " + ItemDefs[t->prods[last].product].names;
                comma = 1;
            }
            last = i;
        }
    }
    if(last != -1) {
        if(comma) *temp += " and ";
        *temp += AString(t->prods[last].amount) + "-" + (2*t->prods[last].amount-1) + " " + ItemDefs[t->prods[last].product].names;
    }
    *temp += ", and may also produce ";
    comma = 0;
    last = -1;
    for(unsigned int i=0; i<sizeof(t->prods)/sizeof(Product); i++) {
        if(t->prods[i].product != -1 && t->prods[i].chance < 100) {
            if(last != -1) {
                if(comma) *temp += ", ";
                *temp += AString(t->prods[last].amount) + "-" + (2*t->prods[last].amount-1) + " " + ItemDefs[t->prods[last].product].names;
                comma = 1;
            }
            last = i;
        }
    }
    if(last != -1) {
        if(comma) *temp += " and ";
        *temp += AString(t->prods[last].amount) + "-" + (2*t->prods[last].amount-1) + " " + ItemDefs[t->prods[last].product].names;
    }
    *temp += ".";
	return temp;
}

int ARegion::GetEthnicity()
{
    if(race < 0) return RA_NA;
    ManType *mt = FindRace(ItemDefs[race].abr);
    if(!mt) return RA_NA;
    return mt->ethnicity;
}

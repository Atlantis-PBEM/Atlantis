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
// 13/06/04 changed Object class to handle fleets (ravanrooke)

#include "object.h"
#include "items.h"
#include "skills.h"
#include "gamedata.h"
#include "unit.h"

int LookupObject(AString *token)
{
	for (int i = 0; i < NOBJECTS; i++) {
		if (*token == ObjectDefs[i].name) return i;
	}
	return -1;
}

/* ParseObject checks for matching Object types AND
 * for matching ship-type items (which are also
 * produced using the build order) if the ships
 * argument is given.
 */
int ParseObject(AString *token, int ships)
{
	int r = -1;
	for (int i=O_DUMMY+1; i<NOBJECTS; i++) {
		if (*token == ObjectDefs[i].name) {
			if(ObjectDefs[i].flags & ObjectType::DISABLED) return -1;
			r = i;
			break;
		}
	}
	// Check for ship-type items:
	if((r == -1) && (ships > 0)) {
		for (int i=0; i<NITEMS; i++) {
			if(ItemDefs[i].type & IT_SHIP) {
				if ((*token == ItemDefs[i].name) ||
					(*token == ItemDefs[i].abr)) {
						if(ItemDefs[i].flags & ItemType::DISABLED) return -1;
						r = -(i+1);
						break;
				}
			}
		}
	}

	return r;
}

int ObjectIsShip(int ot)
{
	if (ObjectDefs[ot].capacity) return 1;
	return 0;
}

Object::Object(ARegion *reg)
{
	num = 0;
	type = O_DUMMY;
	name = new AString("Dummy");
	incomplete = 0;
	describe = 0;
	capacity = 0;
	mages = 0;
	inner = -1;
	runes = 0;
	region = reg;
	prevdir = -1;
	flying = 0;
	ships.Empty();
}

Object::~Object()
{
	if (name) delete name;
	if (describe) delete describe;
	region = (ARegion *)NULL;
}

void Object::Writeout(Aoutfile *f)
{
	f->PutInt(num);
	if (type != -1) f->PutStr(ObjectDefs[type].name);
	else f->PutStr("NO_OBJECT");
	f->PutInt(incomplete);
	f->PutStr(*name);
	if (describe) {
		f->PutStr(*describe);
	} else {
		f->PutStr("none");
	}
	f->PutInt(inner);
	if (Globals->PREVENT_SAIL_THROUGH && !Globals->ALLOW_TRIVIAL_PORTAGE)
		f->PutInt(prevdir);
	else
		f->PutInt(-1);
	f->PutInt(runes);
	f->PutInt(units.Num());
	forlist ((&units))
		((Unit *) elem)->Writeout(f);
	WriteoutFleet(f);
}

void Object::Readin(Ainfile *f, AList *facs, ATL_VER v)
{
	AString *temp;

	num = f->GetInt();

	temp = f->GetStr();
	type = LookupObject(temp);
	delete temp;

	incomplete = f->GetInt();

	if (name) delete name;
	name = f->GetStr();
	describe = f->GetStr();
	if (*describe == "none") {
		delete describe;
		describe = 0;
	}
	inner = f->GetInt();
	prevdir = f->GetInt();
	runes = f->GetInt();

	// Now, fix up a save file if ALLOW_TRIVIAL_PORTAGE is allowed, just
	// in case it wasn't when the save file was made.
	if (Globals->ALLOW_TRIVIAL_PORTAGE) prevdir = -1;
	int i = f->GetInt();
	for (int j=0; j<i; j++) {
		Unit *temp = new Unit;
		temp->Readin(f, facs, v);
		temp->MoveUnit(this);
	}
	mages = ObjectDefs[type].maxMages;
	ReadinFleet(f);
}

void Object::SetName(AString *s)
{
	if (s && (CanModify())) {
		AString *newname = s->getlegal();
		if(!newname) {
			delete s;
			return;
		}
		delete s;
		delete name;
		*newname += AString(" [") + num + "]";
		name = newname;
	}
}

void Object::SetDescribe(AString *s)
{
	if (CanModify()) {
		if (describe) delete describe;
		if (s) {
			AString *newname = s->getlegal();
			delete s;
			describe = newname;
		} else describe = 0;
	}
}

int Object::IsFleet()
{
	if(type == O_FLEET) return 1;
	return 0;
}

int Object::IsBuilding()
{
	if (ObjectDefs[type].protect)
		return 1;
	return 0;
}

int Object::CanModify()
{
	return (ObjectDefs[type].flags & ObjectType::CANMODIFY);
}

Unit *Object::GetUnit(int num)
{
	forlist((&units))
		if (((Unit *) elem)->num == num)
			return ((Unit *) elem);
	return 0;
}

Unit *Object::GetUnitAlias(int alias, int faction)
{
	// First search for units with the 'formfaction'
	forlist((&units)) {
		if(((Unit *)elem)->alias == alias &&
				((Unit *)elem)->formfaction->num == faction)
			return ((Unit *)elem);
	}
	// Now search against their current faction
	{
		forlist((&units)) {
			if (((Unit *) elem)->alias == alias &&
					((Unit *) elem)->faction->num == faction)
				return ((Unit *) elem);
		}
	}
	return 0;
}

Unit *Object::GetUnitId(UnitId *id, int faction)
{
	if (id == 0) return 0;
	if (id->unitnum) {
		return GetUnit(id->unitnum);
	} else {
		if (id->faction) {
			return GetUnitAlias(id->alias, id->faction);
		} else {
			return GetUnitAlias(id->alias, faction);
		}
	}
}

int Object::CanEnter(ARegion *reg, Unit *u)
{
	if(!(ObjectDefs[type].flags & ObjectType::CANENTER) &&
			(u->type == U_MAGE || u->type == U_NORMAL ||
			 u->type == U_APPRENTICE)) {
		return 0;
	}
	return 1;
}

Unit *Object::ForbiddenBy(ARegion *reg, Unit *u)
{
	Unit *owner = GetOwner();
	if(!owner) {
		return(0);
	}

	if(owner->GetAttitude(reg, u) < A_FRIENDLY) {
		return owner;
	}
	return 0;
}

Unit *Object::GetOwner()
{
	Unit *owner = (Unit *) units.First();
	while(owner && !owner->GetMen()) {
		owner = (Unit *) units.Next(owner);
	}
	return(owner);
}

void Object::Report(Areport *f, Faction *fac, int obs, int truesight,
		int detfac, int passobs, int passtrue, int passdetfac, int present)
{
	ObjectType *ob = &ObjectDefs[type];

	if((type != O_DUMMY) && !present) {
		if(IsBuilding() &&
		   !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_BUILDINGS)) {
			// This is a building and we don't see buildings in transit
			return;
		}
		if(IsFleet() &&
		   !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_SHIPS)) {
			// This is a ship and we don't see ships in transit
			return;
		}
		if(IsRoad() &&
		   !(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_ROADS)) {
			// This is a road and we don't see roads in transit
			return;
		}
	}

	/* Fleet Report */
	if(IsFleet()) {
		AString temp = AString("+ ") + *name + " : " + ob->name;
		// report ships:
		for(int item=0; item<NITEMS; item++) {
			int num = GetNumShips(item);
			if(num > 0) {
				if(num > 1) {
					temp += AString(", ") + num + " " + ItemDefs[item].names;
				} else {
					temp += AString(", ") + num + " " +ItemDefs[item].name;
				}
			}
		}
		if((fac == GetOwner()->faction) || (obs > 9)){
			temp += ";";
			if (incomplete > 0) {
				temp += AString(" ") + incomplete + "% damaged;";
			}
			temp += AString(" Load: ") + FleetLoad() + "/" + FleetCapacity() + ";";
			temp += AString(" Sailors: ") + FleetSailingSkill(1) + "/" + GetFleetSize() + ";";
			temp += AString(" MaxSpeed: ") + GetFleetSpeed(1);
		}
		temp += ".";
		f->PutStr(temp);
		f->AddTab();
	} else if (type != O_DUMMY) {
		AString temp = AString("+ ") + *name + " : " + ob->name;
		if (incomplete > 0) {
			temp += AString(", needs ") + incomplete;
		} else if(Globals->DECAY &&
				!(ob->flags & ObjectType::NEVERDECAY) && incomplete < 1) {
			if(incomplete > (0 - ob->maxMonthlyDecay)) {
				temp += ", about to decay";
			} else if(incomplete > (0 - ob->maxMaintenance/2)) {
				temp += ", needs maintenance";
			}
		}
		if (inner != -1) {
			temp += ", contains an inner location";
		}
		if (runes) {
			temp += ", engraved with Runes of Warding";
		}
		if (describe) {
			temp += AString("; ") + *describe;
		}
		if (!(ob->flags & ObjectType::CANENTER)) {
			temp += ", closed to player units";
		}
		temp += ".";
		f->PutStr(temp);
		f->AddTab();
	}

	forlist ((&units)) {
		Unit *u = (Unit *) elem;
		int attitude = fac->GetAttitude(u->faction->num);
		if (u->faction == fac) {
			u->WriteReport(f, -1, 1, 1, 1, attitude);
		} else {
			if(present) {
				u->WriteReport(f, obs, truesight, detfac, type != O_DUMMY, attitude);
			} else {
				if(((type == O_DUMMY) &&
					(Globals->TRANSIT_REPORT &
					 GameDefs::REPORT_SHOW_OUTDOOR_UNITS)) ||
				   ((type != O_DUMMY) &&
					(Globals->TRANSIT_REPORT &
					 GameDefs::REPORT_SHOW_INDOOR_UNITS)) ||
				   ((u->guard == GUARD_GUARD) &&
					(Globals->TRANSIT_REPORT &
					 GameDefs::REPORT_SHOW_GUARDS))) {
					u->WriteReport(f, passobs, passtrue, passdetfac,
							type != O_DUMMY, attitude);
				}
			}
		}
	}
	f->EndLine();
	if (type != O_DUMMY) {
		f->DropTab();
	}
}

void Object::SetPrevDir(int newdir)
{
	prevdir = newdir;
}

void Object::MoveObject(ARegion *toreg)
{
	region->objects.Remove(this);
	region = toreg;
	toreg->objects.Add(this);
}

int Object::IsRoad()
{
	if (type >= O_ROADN && type <= O_ROADS) return 1;
	return 0;
}

/* Performs a basic check on items for ship-types.
 * (note: always fails for non-Fleet Objects.)
 */
int Object::CheckShip(int item)
{
	if(type != O_FLEET) return 0;
	if(!(ItemDefs[item].flags & ItemType::DISABLED)
		&& (ItemDefs[item].type & IT_SHIP)) return 1;
	return 0;
}

void Object::WriteoutFleet(Aoutfile *f)
{
	if(type != O_FLEET) return;
	int nships = (int) ships.Num();
	f->PutInt(nships);
	forlist(&ships)
		((Item *) elem)->Writeout(f);
}

void Object::ReadinFleet(Ainfile *f)
{
	if(type != O_FLEET) return;
	int nships = f->GetInt();
	for(int i=0; i<nships; i++) {
		Item *ship = new Item;
		ship->Readin(f);
		ships.Add(ship);
	}
}

/* Returns the number of component ships of a given
 * type.
 */
int Object::GetNumShips(int type)
{
	if(CheckShip(type) != 0) {
		forlist(&ships) {
			Item *ship = (Item *) elem;
			if(ship->type == type) {
				return ship->num;
			}
		}
	}
	return 0;
}

/* Erases possible previous entries for ship type
 * and resets the number of ships.
 */
void Object::SetNumShips(int type, int num)
{
	if(CheckShip(type) != 0) {
		if(num > 0) {
			forlist(&ships) {
				Item *ship = (Item *) elem;
				if(ship->type == type) {
					Awrite(AString("Setting ") + ItemDefs[type].names + " to " + num + ".");
					ship->num = num;
					return;
				}
			}
			Item *ship = new Item;
			ship->type = type;
			ship->num = num;
			ships.Add(ship);
			Awrite(AString("Setting ") + ItemDefs[type].names + " to " + num + ".");
		} else {
			forlist(&ships) {
				Item *ship = (Item *) elem;
				if(ship->type == type) {
					ships.Remove(ship);
					delete ship;
					return;
				}
			}
		}
	}
}

/* Adds one ship of the given type.
 */
void Object::AddShip(int type)
{	
	if(CheckShip(type) == 0) return;
	int num = GetNumShips(type);
	num++;
	SetNumShips(type, num);	
}

/* Sets a fleet's sailing capacity.
 */
int Object::FleetCapacity()
{
	capacity = 0;
	if(type != O_FLEET) return 0;
	for(int item=0; item<NITEMS; item++) {
		int num = GetNumShips(item);
		// It is assumed that flying and swimming
		// "ships" are never built or given
		// into the same fleet! (i.e. the build
		// and give code checks for that)
		if (ItemDefs[item].fly > 0) {
			capacity += num * ItemDefs[item].fly;
			flying = 1;
		} else {
			capacity += num * ItemDefs[item].swim;
			flying = 0;
		}
	}
	return capacity;
}

/* Returns a fleet's load or -1 for non-fleet objects.
 */
int Object::FleetLoad()
{
	int load = -1;
	int wgt = 0;
	if(type == O_FLEET) {
		forlist(&units) {
			Unit * unit = (Unit *) elem;
			wgt += unit->Weight();
		}
		load = wgt;
	}
	return load;
}

/* Returns the total skill level of all sailors.
 * If report is not 0, returns the total skill level of all
 * units regardless if they have sail orders (for report
 * purposes).
 */
int Object::FleetSailingSkill(int report)
{
	int skill = -1;
	int slvl = 0;
	if(type == O_FLEET) {
		forlist(&units) {
			Unit * unit = (Unit *) elem;
			if ((report != 0) ||
				(unit->monthorders && unit->monthorders->type == O_SAIL)) {
				slvl += unit->GetSkill(S_SAILING) * unit->GetMen();
			}
		}
		skill = slvl;
	}
	return skill;
}

/* Returns fleet size - which is the total of
 * sailors needed to move the fleet.
 */
int Object::GetFleetSize()
{
	if(type != O_FLEET) return 0;
	int inertia = 0;
	for(int item=0; item<NITEMS; item++) {
		int num = GetNumShips(item);
		if (num > 0) inertia += num * ItemDefs[item].pMonths;
	}
	return (inertia / 5);
}

/* Returns the fleet speed - theoretical if report
 * argument is greater than zero (which means all
 * potential sailors issued a SAIL command). The
 * latter is mainly for report purposes. Game
 * functions for moving the fleet provide a zero
 * argument.
 */
int Object::GetFleetSpeed(int report)
{
	if(type != O_FLEET) return 0;
	int tskill = FleetSailingSkill(report);
	int speed = Globals->SHIP_SPEED;
	int inertia = 0;
	int skillreq = 0;
	for(int item=0; item<NITEMS; item++) {
		int num = GetNumShips(item);
		if (num > 0) {
			int diff = num * ItemDefs[item].pMonths;
			skillreq += diff * ItemDefs[item].pLevel;
			inertia += diff;
		}
	}
	if (inertia < 1) return 0;
	if(inertia > 0) skillreq = skillreq / inertia;
	// check for sufficient sailing skill!
	if(tskill < (inertia / 5)) return 0;
	// check for wind mages & sailor bonus
	int windbonus = 0;
	int sailexp = 0;
	int sailors = 0;
	forlist(&units) {
		Unit * unit = (Unit *) elem;
		int wb = unit->GetAttribute("wind");
		if(wb > 0) windbonus += (wb+1) * (wb+1) * 8;
		int sb = unit->GetSkill(S_SAILING);
		if(sb > 0) {
			sailors += unit->GetMen();
			sailexp += unit->GetMen() * sb;
		}
	}
	sailexp = sailexp / sailors;
	// up to 50% speed gain through wind:
	speed += (Globals->SHIP_SPEED / 2) * windbonus / inertia;
	// less than required skill: speed = 0!
	int minskill = skillreq-2;
	if(minskill < 1) minskill = 1;
	if (sailexp < minskill) return 0;
	int boost = 125 * (skillreq - 1);
	int bonus1 = 1000 + boost * (sailexp-1);
	int bonus2 = 1000 * tskill / (inertia / 5) + boost * 2;
	if (bonus2 < bonus1) bonus1 = bonus2;
	speed += (Globals->SHIP_SPEED / 4) * bonus1 / 1000;	
	return speed;
}

AString *ObjectDescription(int obj)
{
	if(ObjectDefs[obj].flags & ObjectType::DISABLED)
		return NULL;

	ObjectType *o = &ObjectDefs[obj];
	AString *temp = new AString;
	*temp += AString(o->name) + ": ";
	if(o->capacity) {
		*temp += "This is a ship.";
	} else {
		*temp += "This is a building.";
	}

	if(Globals->LAIR_MONSTERS_EXIST && (o->monster != -1)) {
		*temp += " Monsters can potentially lair in this structure.";
		if(o->flags & ObjectType::NOMONSTERGROWTH) {
			*temp += " Monsters in this structures will never regenerate.";
		}
	}

	if(o->flags & ObjectType::CANENTER) {
		*temp += " Units may enter this structure.";
	}

	if(o->protect) {
		*temp += AString(" This structure provides defense to the first ") +
			o->protect + " men inside it.";
		// Now do the defences. First, figure out how many to do.
		int totaldef = 0; 
		for (int i=0; i<NUM_ATTACK_TYPES; i++) {
			totaldef += (o->defenceArray[i] != 0);
		}
	// Now add the description to temp
		*temp += AString(" This structure gives a defensive bonus of ");
		for (int i=0; i<NUM_ATTACK_TYPES; i++) {
			if (o->defenceArray[i]) {
				totaldef--;
				*temp += AString(o->defenceArray[i]) + " against " +
					AttType(i) + AString(" attacks");
				
				if (totaldef >= 2) {
					*temp += AString(", ");
				} else {
					if (totaldef == 1) {	// penultimate bonus
						*temp += AString(" and ");
					} else {	// last bonus
						*temp += AString(".");
					}
				} // end if 
			}
		} // end for
	}

	/*
	 * Handle all the specials
	 */
	for(int i = 0; i < NUMSPECIALS; i++) {
		SpecialType *spd = &SpecialDefs[i];
		AString effect = "are";
		int match = 0;
		if(!(spd->targflags & SpecialType::HIT_BUILDINGIF) &&
				!(spd->targflags & SpecialType::HIT_BUILDINGEXCEPT)) {
			continue;
		}
		for(int j = 0; j < 3; j++)
			if(spd->buildings[j] == obj) match = 1;
		if(!match) continue;
		if(spd->targflags & SpecialType::HIT_BUILDINGEXCEPT) {
			effect += " not";
		}
		*temp += " Units in this structure ";
		*temp += effect + " affected by " + spd->specialname + ".";
	}

	if(o->sailors) {
		*temp += AString(" This ship requires ") + o->sailors +
			" total levels of sailing skill to sail.";
	}
	if(o->maxMages && Globals->LIMITED_MAGES_PER_BUILDING) {
		*temp += AString(" This structure will allow up to ") + o->maxMages +
			" mages to study above level 2.";
	}
	int buildable = 1;
	SkillType *pS = NULL;
	if(o->item == -1 || o->skill == NULL) buildable = 0;
	if (o->skill != NULL) pS = FindSkill(o->skill);
	if (pS && (pS->flags & SkillType::DISABLED)) buildable = 0;
	if(o->item != I_WOOD_OR_STONE &&
			(ItemDefs[o->item].flags & ItemType::DISABLED))
		buildable = 0;
	if(o->item == I_WOOD_OR_STONE &&
			(ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
			(ItemDefs[I_STONE].flags & ItemType::DISABLED))
		buildable = 0;
	if(!buildable) {
		*temp += " This structure cannot be built by players.";
	} else {
		*temp += AString(" This structure is built using ") +
			SkillStrs(pS) + " " + o->level + " and requires " +
			o->cost + " ";
		if(o->item == I_WOOD_OR_STONE) {
			*temp += "wood or stone";
		} else {
			*temp += ItemDefs[o->item].name;
		}
		*temp += " to build.";
	}

	if(o->productionAided != -1 &&
			!(ItemDefs[o->productionAided].flags & ItemType::DISABLED)) {
		*temp += " This trade structure increases the amount of ";
		if(o->productionAided == I_SILVER) {
			*temp += "entertainment";
		} else {
			*temp += ItemDefs[o->productionAided].names;
		}
		*temp += " available in the region.";
	}

	if(Globals->DECAY) {
		if(o->flags & ObjectType::NEVERDECAY) {
			*temp += " This structure will never decay.";
		} else {
			*temp += AString(" This structure can take ") + o->maxMaintenance +
				" units of damage before it begins to decay.";
			*temp += AString(" Damage can occur at a maximum rate of ") +
				o->maxMonthlyDecay + " units per month.";
			if(buildable) {
				*temp += AString(" Repair of damage is accomplished at ") +
					"a rate of " + o->maintFactor + " damage units per " +
					"unit of ";
				if(o->item == I_WOOD_OR_STONE) {
					*temp += "wood or stone.";
				} else {
					*temp += ItemDefs[o->item].name;
				}
			}
		}
	}

	return temp;
}

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
// MODIFICATONS
// Date        Person            Comments
// ----        ------            --------
// 2000/MAR/21 Azthar Septragen  Added roads.
#include "object.h"
#include "items.h"
#include "skills.h"
#include "gamedata.h"
#include "unit.h"

int ParseObject(AString * token)
{
	int r = -1;
	for (int i=O_DUMMY+1; i<NOBJECTS; i++) {
		if (*token == ObjectDefs[i].name) {
			r = i;
			break;
		}
	}
	if(r != -1) {
		if(ObjectDefs[r].flags & ObjectType::DISABLED) r = -1;
	}
	return r;
}

int ObjectIsShip(int ot)
{
	if (ObjectDefs[ot].capacity) return 1;
	return 0;
}

Object::Object( ARegion *reg )
{
	num = 0;
	type = O_DUMMY;
	name = new AString("Dummy");
	incomplete = 0;
	describe = 0;
	capacity = 0;
	inner = -1;
	runes = 0;
	region = reg;
}

Object::~Object()
{
	if (name) delete name;
	if (describe) delete describe;
	region = (ARegion *)NULL;
}

void Object::Writeout( Aoutfile *f )
{
	f->PutInt(num);
	f->PutInt(type);
	f->PutInt(incomplete);
	f->PutStr(*name);
	if (describe) {
		f->PutStr(*describe);
	} else {
		f->PutStr("none");
	}
	f->PutInt(inner);
	f->PutInt(runes);
	f->PutInt(units.Num());
	forlist ((&units))
		((Unit *) elem)->Writeout( f );
}

void Object::Readin(Ainfile * f,AList * facs,ATL_VER v)
{
	num = f->GetInt();
	type = f->GetInt();
	incomplete = f->GetInt();

	if (name) delete name;
	name = f->GetStr();
	describe = f->GetStr();
	if (*describe == "none") {
		delete describe;
		describe = 0;
	}
	inner = f->GetInt();
	runes = f->GetInt();
	int i = f->GetInt();
	for (int j=0; j<i; j++) {
		Unit * temp = new Unit;
		temp->Readin(f,facs,v);
		temp->MoveUnit( this );
	}
}

void Object::SetName(AString * s)
{
	if (s && (CanModify())) {
		AString * newname = s->getlegal();
		if( !newname ) {
			delete s;
			return;
		}
		delete s;
		delete name;
		*newname += AString(" [") + num + "]";
		name = newname;
	}
}

void Object::SetDescribe(AString * s)
{
	if (CanModify()) {
		if (describe) delete describe;
		if (s) {
			AString * newname = s->getlegal();
			delete s;
			describe = newname;
		} else describe = 0;
	}
}

int Object::IsBoat()
{
	if (ObjectDefs[type].capacity)
		return 1;
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
	if (type == O_DUMMY) return 0;
	if (type == O_SHAFT) return 0;
	/* XXX -- This check is not really correct */
	if (ObjectDefs[type].monster == -1) return 1;
	return 0;
} 

Unit * Object::GetUnit(int num)
{
	forlist((&units))
		if (((Unit *) elem)->num == num)
			return ((Unit *) elem);
	return 0;
}

Unit * Object::GetUnitAlias(int alias,int faction)
{
	forlist((&units))
		if (((Unit *) elem)->alias == alias &&
				((Unit *) elem)->faction->num == faction)
			return ((Unit *) elem);
	return 0;
}

Unit * Object::GetUnitId(UnitId * id,int faction)
{
	if (id == 0) return 0;
	if (id->unitnum) {
		return GetUnit(id->unitnum);
	} else {
		if (id->faction) {
			return GetUnitAlias(id->alias,id->faction);
		} else {
			return GetUnitAlias(id->alias,faction);
		}
	}
}

int Object::CanEnter(ARegion * reg,Unit * u)
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
	if( !owner ) {
		return( 0 );
	}

	if( owner->GetAttitude( reg, u ) < A_FRIENDLY ) {
		return owner;
	}
	return 0;
}

Unit *Object::GetOwner()
{
	Unit *owner = (Unit *) units.First();
	while( owner && !owner->GetMen() ) {
		owner = (Unit *) units.Next( owner );
	}
	return( owner );
}

void Object::Report(Areport *f, Faction *fac, int obs, int truesight,
		int detfac)
{
	ObjectType *ob = &ObjectDefs[type];
	if (type != O_DUMMY) {
		AString temp = AString("+ ") + *name + " : " + ob->name;
		if (incomplete > 0) {
			temp += AString(", needs ") + incomplete;
		} else if(Globals->DECAY &&
				!(ob->flags & ObjectType::NEVERDECAY) && incomplete < 1) {
			if(incomplete > (0 - ob->maxMonthlyDecay)) {
				temp += DoDecayWarning();
			} else if(incomplete > (0 - ob->maxMaintenance/2)) {
				temp += DoMaintenanceWarning();
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
		Unit * u = (Unit *) elem;
		if (u->faction == fac) {
			u->WriteReport(f,-1,1,1,1);
		} else {
			u->WriteReport(f,obs,truesight,detfac,type != O_DUMMY);
		}
	}
	f->EndLine();
	if (type != O_DUMMY) {
		f->DropTab();
	}
}

void Object::MoveObject( ARegion *toreg )
{
	region->objects.Remove( this );
	region = toreg;
	toreg->objects.Add( this );
}

int Object::IsRoad()
{
	if (type >= O_ROADN && type <= O_ROADS) return 1;
	return 0;
}

AString Object::DoDecayWarning()
{
	return AString(". This object is about to decay.");
}

AString Object::DoMaintenanceWarning()
{
	return AString(". This object needs maintenance.");
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
	}
	if(o->flags & ObjectType::NOEARTHQUAKE) {
		*temp += " Units in this structure are not affected by earthquakes.";
	}
	if(o->sailors) {
		*temp += AString(" This ship requires ") + o->sailors +
			" total levels of sailing skill to sail.";
	}
	int buildable = 1;
	if(o->item == -1 || o->skill == -1) buildable = 0;
	if(SkillDefs[o->skill].flags & SkillType::DISABLED) buildable = 0;
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
			SkillStrs(o->skill) + " " + o->level + " and requires " +
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

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
#include "rules.h"
#include "unit.h"

int ParseObject(AString * token)
{
    for (int i=O_DUMMY+1; i<NOBJECTS; i++)
    {
        if (*token == ObjectDefs[i].name) return i;
    }
    return -1;
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
}

void Object::Writeout( Aoutfile *f )
{
#ifdef DEBUG_GAME
    f->PutStr("Object");
#endif
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
#ifdef DEBUG_GAME
    f->PutStr("Num of Units");
#endif
    f->PutInt(units.Num());
    forlist ((&units))
        ((Unit *) elem)->Writeout( f );
}

void Object::Readin(Ainfile * f,AList * facs,ATL_VER v)
{
#ifdef DEBUG_GAME
    delete f->GetStr();
#endif
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
#ifdef DEBUG_GAME
    delete f->GetStr();
#endif
    int i = f->GetInt();
    for (int j=0; j<i; j++) {
        Unit * temp = new Unit;
        temp->Readin(f,facs,v);
        temp->MoveUnit( this );
    }
}

void Object::SetName(AString * s) {
  if (s && (CanModify())) {
    AString * newname = s->getlegal();
    if( !newname )
    {
        delete s;
        return;
    }
    delete s;
    delete name;
    *newname += AString(" [") + num + "]";
    name = newname;
  }
}

void Object::SetDescribe(AString * s) {
  if (CanModify()) {
    if (describe) delete describe;
    if (s) {
      AString * newname = s->getlegal();
      delete s;
      describe = newname;
    } else describe = 0;
  }
}

int Object::IsBoat() {
  if (ObjectDefs[type].capacity)
    return 1;
  return 0;
}

int Object::IsBuilding() {
  if (ObjectDefs[type].protect)
    return 1;
  return 0;
}

int Object::CanModify() {
  if (type == O_DUMMY) return 0;
  if (type == O_SHAFT) return 0;
  if (ObjectDefs[type].monster == -1) return 1;
  return 0;
} 

Unit * Object::GetUnit(int num) {
  forlist((&units))
  {
    if (((Unit *) elem)->num == num) return ((Unit *) elem);
  }
  return 0;
}

Unit * Object::GetUnitAlias(int alias,int faction) {
  forlist((&units))
    if (((Unit *) elem)->alias == alias &&
	((Unit *) elem)->faction->num == faction)
      return ((Unit *) elem);
  return 0;
}

Unit * Object::GetUnitId(UnitId * id,int faction) {
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
    if( !ObjectDefs[type].canenter &&
        ( u->type == U_MAGE || u->type == U_NORMAL ))
    {
        return 0;
    }
    return 1;
}

Unit *Object::ForbiddenBy(ARegion *reg, Unit *u)
{
    Unit *owner = GetOwner();
    if( !owner )
    {
        return( 0 );
    }

    if( owner->GetAttitude( reg, u ) < A_FRIENDLY )
    {
        return owner;
    }
    return 0;
}

Unit *Object::GetOwner()
{
    Unit *owner = (Unit *) units.First();
    while( owner && !owner->GetMen() )
    {
        owner = (Unit *) units.Next( owner );
    }
    return( owner );
}

void Object::Report( Areport * f,Faction * fac,int obs,int truesight,
                     int detfac)
{
    if (type != O_DUMMY) {
        AString temp = AString("+ ") + *name + " : " +
            ObjectDefs[type].name;
        if (incomplete > 0) {
            temp += AString(", needs ") + incomplete;
        }
        if (IsRoad())
        {
            if (incomplete == 0)
            {
//                fac->Event(*name + " " + ObjectDefs[type].name +
//                    " in " + (AString) TerrainDefs[region->type].name +
//                    " (" + region->xloc + "," + region->yloc + ") in " +
//                    *region->name + DoDecayWarning());
                temp += DoDecayWarning();
            }
            if (incomplete < 0 &&
                incomplete > (0 - (region->GetMaxClicks() +
                region->PillageCheck())))
            {
//                fac->Event(*name + " " + ObjectDefs[type].name + " in " +
//                    (AString) TerrainDefs[region->type].name + " (" +
//                    region->xloc + "," + region->yloc + ") in " +
//                    *region->name + DoMaintenanceWarning());
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
        if (!ObjectDefs[type].canenter) {
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

int Object::IsRoadUsable()
{
    if (IsRoad() && incomplete < 1) return 1;
    return 0;
}

int Object::IsRoadDecaying()
{
    if (IsRoad() && incomplete > 0) return 1;
    return 0;
}

AString Object::DoDecayWarning()
{
  return AString(". This road is about to decay.");
}

AString Object::DoMaintenanceWarning()
{
  return AString(". This road needs maintenance.");
}

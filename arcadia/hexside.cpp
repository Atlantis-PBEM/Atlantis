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

#include "object.h"
#include "gamedata.h"
#include "game.h"

int LookupHexside(AString *token)
{
	for (int i = 0; i < NHEXSIDES; i++) {
		if (*token == HexsideDefs[i].name) return i;
	}
	return -1;
}

int ParseHexside(AString *token)
{
	int r = -1;
	for (int i=H_DUMMY+1; i<NHEXSIDES; i++) {
		if (*token == HexsideDefs[i].name) {
			r = i;
			break;
		}
	}
	if(r != -1) {
		if(HexsideDefs[r].flags & HexsideType::DISABLED) r = -1;
	}
	return r;
}

Hexside::Hexside()
{
	type = H_DUMMY;
	bridge = 0;
	road = 0;
	harbour = 0;
}

Hexside::~Hexside()
{
}

void Hexside::Writeout(Aoutfile *f)
{
	f->PutStr(HexsideDefs[type].name);
	f->PutInt(bridge);
	f->PutStr(road);
	f->PutInt(harbour);
}

void Hexside::Readin(Ainfile *f)
{
    AString *temp;
    
	temp = f->GetStr();
	type = LookupHexside(temp);
	delete temp;
	
	bridge = f->GetInt();
	road = f->GetInt();  /* Hexside Patch 030825 BS */
	harbour = f->GetInt();
}

AString *HexsideDescription(int type)
{
	if(HexsideDefs[type].flags & ObjectType::DISABLED)
		return NULL;

	HexsideType *h = &HexsideDefs[type];
	AString *temp = new AString;
	*temp += AString(h->name) + ": ";
	*temp += "This is a terrain edge feature.";

	if(h->sailable) {
	    switch(h->sailable) {
	        case 3: *temp += " All ships";
	            break;
	        case 2: *temp += " Deep water ships";
	            break;
	        case 1: *temp += " Shallow water ships";
	        default:
	            break;
	    }
	    *temp += " can sail here.";
	}

	if(h->movementmultiplier) {
	    *temp += " The movement cost to walk or ride across this edge is";
	    if(h->movementmultiplier < 0) {
	        *temp += AString(" decreased by ") + (-50*h->movementmultiplier) + "%, then rounded up.";
	    } else {
	        *temp += AString(" increased by ") + (50*h->movementmultiplier) + "%, rounded up.";
	    }
	}

	if(h->blockeffect) {
	    if(h->blockeffect == 1) {
	        *temp += " This edge cannot be crossed by walking or riding units, unless a bridge is present.";
	    } else if(h->blockeffect > 1) {
	        *temp += " This edge cannot be crossed by walking or riding units.";
	    } else {
	        *temp += " This feature allows walking and riding units to cross a ";
	        int comma = 0;
	        int last = 0;
	        for(int i=0; i<NHEXSIDES; i++) {
	            if(HexsideDefs[i].flags & HexsideType::DISABLED) continue;
	            if(HexsideDefs[i].blockeffect == 1) {
	                if(last) {
	                    if(comma) *temp += ", ";
	                    comma = 1;
	                    *temp += HexsideDefs[last].name;
	                }
	                last = i;
	            }
	        }
	        if(comma) *temp += " or ";
	        if(last) *temp += HexsideDefs[last].name;
	        *temp += ".";
	    }
	}

	if(h->stealthpen) {
	    *temp += " Units which cross this feature will suffer a malus of 2 to "
                 "their stealth rating until the next magic round.";
	}

	if(h->advancepen) {
	    *temp += " Walking and riding units which cross this feature to fight a battle (while advancing or aiding others) "
                "will suffer a malus of ";
	    *temp += AString(h->advancepen) + " to their attack and defence skills.";
	}

	int buildable = 1;
	SkillType *pS = NULL;
	if(h->item == -1 || h->skill == NULL) buildable = 0;
	if (buildable) pS = FindSkill(h->skill);
	if (pS && (pS->flags & SkillType::DISABLED)) buildable = 0;
	if(h->item != I_WOOD_OR_STONE &&
			(ItemDefs[h->item].flags & ItemType::DISABLED))
		buildable = 0;
	if(h->item == I_WOOD_OR_STONE &&
			(ItemDefs[I_WOOD].flags & ItemType::DISABLED) &&
			(ItemDefs[I_STONE].flags & ItemType::DISABLED))
		buildable = 0;
	if(!buildable) {
		*temp += " This feature cannot be built by players.";
	} else {
		*temp += AString(" This feature is built using ") +
			SkillStrs(pS) + " " + h->level + " and requires " +
			h->cost + " ";
		if(h->item == I_WOOD_OR_STONE) {
			*temp += "wood or stone";
		} else {
			*temp += ItemDefs[h->item].name;
		}
		*temp += " to build.";
		if(type == H_ROAD) *temp += " This feature may be built on any edge between two land regions.";
		if(type == H_BRIDGE) *temp += " This feature may be built on any edge containing a feature which may be bridged.";
		if(type == H_HARBOUR) *temp += " This feature may only be built on a beach, and will replace the beach when completed.";
	}

	return temp;
}


void Unit::CrossHexside(ARegion *fromreg, ARegion *toreg)
{
    if(!Globals->HEXSIDE_TERRAIN) return;
    int dir = -1;
    for(int i=0; i<NDIRS; i++) {
        if(fromreg->neighbors[i] == toreg) dir = i;
    }
    if(dir == -1) return;
    Hexside *h = fromreg->hexside[dir];
    if(HexsideDefs[h->type].stealthpen) SetFlag(FLAG_VISIB,1);
    else if(h->road < 0 && HexsideDefs[H_ROAD].stealthpen) SetFlag(FLAG_VISIB,1);
    else if(h->road < 0 && HexsideDefs[H_BRIDGE].stealthpen) SetFlag(FLAG_VISIB,1);
}


/*
void Game::SetupObjectMirrors()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			if (o->mirrornum>-1 && !(o->mirror) && r->neighbors[o->hexside]) {
                forlist(&r->neighbors[o->hexside]->objects) {
                    Object *obj = (Object *) elem;
                    if(obj->num == o->mirrornum && o->num == obj->mirrornum) {
                        obj->mirror = o;
                        o->mirror = obj;
                    }
                }
			}
		}
	}

}
*/


/*
/* Hexside Patch 030825 BS *//*
void Game::HexsideCompatibility(ARegion *r, Object *obj)
{
// If building a hexside object should remove any other hexside object, do it here.
/* Harbour Removes Beach *//*
    if(obj->type==O_HARBOUR) {
        forlist(&r->objects) {
		    Object *o = (Object *) elem;        
		    if (o->hexside==obj->hexside && o->type == O_BEACH) {
		        if (o->mirror) o->mirror->region->objects.Remove(o->mirror);
		        r->objects.Remove(o);
		    }
        }
    }
}



/* Hexside Patch 030825 BS *//*
int Game::HexsideCanGoThere(ARegion * r,Object * obj,Unit * u)
{
    int dir = obj->hexside;
    if(!r->neighbors[dir]) return 0;

// No hexside can have two of the same object except ships.
    if(!obj->IsBoat()) {
        forlist (&r->objects) {
            Object *o = (Object *) elem;
            if (o->type == obj->type) {
                if (o->hexside == obj->hexside) {
                    if (o->incomplete != ObjectDefs[o->type].cost) {     // is ok if structure has not been started
                        if (o->num != obj->num) return 0;                // is ok if structure is this structure
                    }
                }
            }
        }   
    }
/* Object Specific Stuff. May code into gamedata.cpp at some stage */
/* Ship may be built on sailable hexside object of correct depth, or ocean ships on ocean edge*//*
    if (obj->IsBoat()) {
        if(TerrainDefs[r->neighbors[dir]->type].similar_type == R_OCEAN && ObjectDefs[obj->type].sailable > 1) return 1;
    	forlist (&r->objects) {
    	    Object *o = (Object *) elem;
    	    if (o->hexside == dir) {
                if (!o->IsBoat() && ObjectDefs[o->type].sailable == 3) return 1;
                if (!o->IsBoat() && ObjectDefs[o->type].sailable == ObjectDefs[obj->type].sailable) return 1;
                if (!o->IsBoat() && ObjectDefs[o->type].sailable && ObjectDefs[obj->type].sailable == 3) return 1;
            }
        }
        return 0;
    }



/* Bridge cannot go to ocean and must go over river or ravine *//*
   if (obj->type == O_BRIDGE) {
        if(TerrainDefs[r->neighbors[dir]->type].similar_type == R_OCEAN) return 0;
    	forlist (&r->objects) {
    	    Object *o = (Object *) elem;
    	    if (o->type == O_RIVER || O_RAVINE) {
                 if (o->hexside == dir) return 1;
            }
        }
   return 0;
   }
/* Road cannot go to ocean *//*
   if (obj->type == O_ROAD) {
        if(TerrainDefs[r->neighbors[dir]->type].similar_type == R_OCEAN) return 0;
   return 1;
   }
/* Wall cannot go to river edge. To do: Should not go to river mouth */   /*
   if (obj->type == O_IWALL) {
    	forlist (&r->objects) {
    	    Object *o = (Object *) elem;
    	    if (o->type == O_RIVER) {
                 if (o->hexside == dir) return 0;
            }
        }
   return 1;
   }
/* Harbour must go on beach */   /*
   if (obj->type == O_HARBOUR) {
    	forlist (&r->objects) {
    	    Object *o = (Object *) elem;
    	    if (o->type == O_BEACH) {
                 if (o->hexside == dir) return 1;
            }
        }
   return 0;
   }   
/* Anything else should not be built */   /*
return 0;
}



*/

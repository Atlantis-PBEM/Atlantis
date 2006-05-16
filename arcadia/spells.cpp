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
#include "game.h"
#include "gamedata.h"
#include "unit.h"

#ifndef DEBUG
//#define DEBUG
#endif

void Game::ProcessCastOrder(Unit * u,AString * o, OrdersCheck *pCheck, int isquiet)
{
    AString * token = o->gettoken();
    if (!token) {
        ParseError( pCheck, u, 0, "CAST: No skill given.");
        return;
    }

    int sk = ParseSkill(token);
    delete token;
    if (sk==-1) {
        ParseError( pCheck, u, 0, "CAST: Invalid skill.");
        return;
    }

    if( !( SkillDefs[sk].flags & SkillType::MAGIC )) {
        ParseError( pCheck, u, 0, "CAST: That is not a magic skill.");
        return;
    }
    if( !( SkillDefs[sk].flags & SkillType::CAST )) {
        ParseError( pCheck, u, 0, "CAST: That skill cannot be CAST.");
        return;
    }

	RangeType *rt = NULL;
    if( !pCheck ) {
        //
        // XXX -- should be error checking spells
        //
        switch(sk) {
			case S_MIND_READING:
				ProcessMindReading(u,o, pCheck, isquiet );
				break;
			case S_CONSTRUCT_PORTAL: // Disabled in Earthsea
			case S_ENCHANT_SWORDS:
			case S_ENCHANT_ARMOR:
			case S_CONSTRUCT_GATE:
			case S_ENGRAVE_RUNES_OF_WARDING:
			case S_SUMMON_BALROG:
			case S_SUMMON_SKELETONS:
			case S_SUMMON_LICH:
			case S_DRAGON_LORE: // Disabled in Arcadia
			case S_GRYFFIN_LORE: // Summons gryffins
			case S_CREATE_RING_OF_INVISIBILITY:
			case S_CREATE_CLOAK_OF_INVULNERABILITY:
			case S_CREATE_STAFF_OF_FIRE:
			case S_CREATE_STAFF_OF_LIGHTNING:
			case S_CREATE_AMULET_OF_TRUE_SEEING:
			case S_CREATE_AMULET_OF_PROTECTION: // in artifact_lore in Earthsea
			case S_CREATE_RUNESWORD:
			case S_CREATE_SHIELDSTONE: // in artifact_lore in Earthsea
			case S_CREATE_MAGIC_CARPET:
			case S_CREATE_FOOD:
			//Earthsea spells
			case S_PHANTASMAL_ENTERTAINMENT: // castable in earthsea mod.
   			case S_CONCEALMENT:	 // Adds to stealth of friendly units which do not attack/steal/assassinate
				ProcessGenericSpell(u,sk, pCheck, isquiet );
				break;
			case S_WOLF_LORE:
			case S_SUMMON_IMPS:
			case S_SUMMON_DEMON:
			case S_RAISE_UNDEAD:
			    if(Globals->ARCADIA_MAGIC) ProcessSummonCreaturesSpell(u,o, sk, pCheck, isquiet );
			    else ProcessGenericSpell(u,sk, pCheck, isquiet );
			    break;
			case S_FOG:  // Blanks out region for others
                ProcessFogSpell(u,o, pCheck, isquiet);
			case S_CLEAR_SKIES:
			case S_EARTH_LORE:
				rt = FindRange(SkillDefs[sk].range);
				if (rt == NULL && !Globals->ARCADIA_MAGIC) {
				    ProcessGenericSpell(u, sk, pCheck, isquiet);
				} else
					ProcessRegionSpell(u, o, sk, pCheck, isquiet); //large option
				break;
			case S_FARSIGHT: // small/large
			case S_TELEPORTATION:
			case S_WEATHER_LORE:
			case S_DIVERSION: //modifies rivers
			case S_BLIZZARD:   //summons small/large blizzard around target region
			case S_CREATE_PORTAL: //earthsea - creates a portal, may be linked to another.
			case S_SEAWARD: // delays region sinking.
				ProcessRegionSpell(u, o, sk, pCheck, isquiet); //"large" a possibility now for all region spells. Used for blizzard, clear skies
				break;
			case S_BIRD_LORE:
				ProcessBirdLore(u,o, pCheck, isquiet );
				break;
			case S_INVISIBILITY:
				ProcessUnitsSpell(u,o, sk, pCheck, isquiet );
				break;
			case S_GATE_LORE:
				ProcessCastGateLore(u,o, pCheck, isquiet );
				break;
			case S_PORTAL_LORE:
				ProcessCastPortalLore(u,o, pCheck, isquiet );
				break;
			case S_CREATE_PHANTASMAL_BEASTS:
				ProcessPhanBeasts(u,o, pCheck, isquiet );
				break;
			case S_CREATE_PHANTASMAL_UNDEAD:
				ProcessPhanUndead(u,o, pCheck, isquiet );
				break;
			case S_CREATE_PHANTASMAL_DEMONS:
				ProcessPhanDemons(u,o, pCheck, isquiet );
				break;
			//Earthsea spells:
			case S_ILLUSORY_CREATURES: // summons illusory creatures of specified type
			    ProcessPhanCreatures(u,o, pCheck, isquiet );
                break;
			case S_SUMMON_MEN: // summons men of particular race for $50 each
                ProcessSummonMen(u,o,pCheck, isquiet);
                break;
			case S_SPIRIT_OF_DEAD: //cast: summons long dead mage
				ProcessUnitsSpell(u,o, sk, pCheck, isquiet );                
                break;
			case S_TRANSMUTATION: //turns 2 pieces of one item type into one piece of another
                ProcessChangeSpell(u,o, sk, pCheck, isquiet );
                break;
			case S_MODIFICATION: //modifies region product
                ProcessModificationSpell(u,o,pCheck, isquiet);
                break;
			case S_REJUVENATION: //modifies region terrain. High levels: includes ocean
                ProcessRejuvenationSpell(u,o,pCheck, isquiet);
                break;
			case S_HYPNOSIS: //hypnotises target non-mage unit(s). Maximum men limit. Can give work/tax/produce/build orders. Higher level can give move/pillage/advance orders
                ProcessHypnosisSpell(u,o,pCheck, isquiet);
                break;		
			case S_ARTIFACT_LORE: //creates shieldstone or AMPR in EarthSea.
			case S_BASE_ARTIFACTLORE:
			    ProcessArtifactSpell(u,o,sk,pCheck, isquiet);
                break;
            default:
                u->Error("CAST: That spell does not seem to have process code. Please contact your GM.");     	
		}
	}
}

void Unit::AddCastOrder(CastOrder *order)
{
     if(!Globals->ARCADIA_MAGIC) castlistorders.DeleteAll(); //no more than one spell allowed
     else {
         forlist(&castlistorders) {
            CastOrder *ord = (CastOrder *) elem;
            if(ord->spell == order->spell) { 
                castlistorders.Remove(ord);
    		    delete ord;
    		}
        }
    }
    castlistorders.Add(order);
}

void Game::ProcessMindReading(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
    UnitId *id = ParseUnit(o);

    if (!id) {
        u->Error("CAST: No unit specified.", 0);
        return;
    }

    CastMindOrder *order = new CastMindOrder;
    order->id = id;
    order->spell = S_MIND_READING;
    order->level = 1;
    order->quiet = isquiet;

    u->AddCastOrder(order);
}

void Game::ProcessBirdLore(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
    AString *token = o->gettoken();

    if (!token) {
        u->Error("CAST: Missing arguments.", 0);
        return;
    }

    if (*token == "eagle") {
        CastIntOrder *order = new CastIntOrder;
        order->spell = S_BIRD_LORE;
        order->quiet = isquiet;
		order->level = 3;
		order->target = -1;  //Earthsea mod, does nothing in non-Earthsea.
		delete token;
		token = o->gettoken();
		if(token) {
		    order->target = token->value();
		    delete token;
		}
        u->AddCastOrder(order);
        return;
    }

/*    if (*token == "large") {
    //ARC-III only
		delete token;
        CastIntOrder *order = new CastIntOrder;
        order->spell = S_BIRD_LORE;
        order->quiet = isquiet;
		order->level = 4;
		order->target = 0; //just for safety.
        u->AddCastOrder(order);
        return;
    }*/

    if (*token == "direction") {
        delete token;
        token = o->gettoken();

        if (!token) {
            u->Error("CAST: Missing arguments.", 0);
            return;
        }

        int dir = ParseDir(token);
        delete token;
        if (dir == -1 || dir > NDIRS) {
            u->Error("CAST: Invalid direction.", 0);
            return;
        }

        CastIntOrder *order = new CastIntOrder;
        order->spell = S_BIRD_LORE;
        order->quiet = isquiet;
        order->level = 1;
        order->target = dir;
        u->AddCastOrder(order);

        return;
    }

    u->Error("CAST: Invalid arguments.", 0);
    delete token;
}

void Game::ProcessFogSpell(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
    AString *token = o->gettoken();

    int large = 0;
    int dir = -1;
    if (token && *token == "large") {
        large = 1;
        delete token;
        token = o->gettoken();
        if(token) {
            dir = ParseDir(token); // return -1 if doesn't make sense
            delete token;
            if(dir<0 || dir>5) dir = -1;
        }
    }

    CastIntOrder *order = new CastIntOrder;
    order->spell = S_FOG;
    order->quiet = isquiet;
    order->level = 1+3*large;
    order->target = dir;
    u->AddCastOrder(order);
    return;
}

void Game::ProcessUnitsSpell(Unit *u,AString *o, int spell, OrdersCheck *pCheck, int isquiet )
{
    AString *token = o->gettoken();
    
    if (!token || (!(*token == "units") && !(*token == "unit")) ) {
        u->Error("CAST: Must specify units.", 0);
        return;
    }
    delete token;

    CastUnitsOrder *order = 0;
    //option of adding units over multiple lines
    
    
    forlist(&u->castlistorders) {
        CastOrder *ord = (CastOrder *) elem;
        if(ord->spell == spell) {
            order = (CastUnitsOrder *) ord;
            if(order->quiet && !isquiet) order->quiet = isquiet; //if earlier line wasn't quiet, don't make it quiet now
        }
    }

    if(!order) {
        order = new CastUnitsOrder;
        order->spell = spell;
        order->quiet = isquiet;
        order->level = 1;
        u->AddCastOrder(order);
    }

    UnitId *id = ParseUnit(o);
    while (id) {
        order->units.Add(id);
        id = ParseUnit(o);
    }
}

void Game::ProcessChangeSpell(Unit *u,AString *o, int spell, OrdersCheck *pCheck, int isquiet )
{
    AString *token = o->gettoken();

    if (!token) {
        u->Error("CAST: Arguments missing.", 0);
        return;
    }
    
    int toitem = ParseEnabledItem(token);
	delete token;
    if(toitem<0) {
		u->Error("CAST: Invalid item.", 0);
		return;
    }
    
    token = o->gettoken();
    int fromitem;
    if (!token) {
        fromitem = -1;
    } else {
        fromitem = ParseEnabledItem(token);
    	delete token;
        if(fromitem<0) {
    		u->Error("CAST: Invalid item.", 0);
    		return;
        }
    }
    if(toitem == fromitem) {
        u->Error("CAST: to item is same as from item.", 0);
        return;
    }
    
    int unitlist = 0;
    token = o->gettoken();    
    if(token && ( *token == "units" || *token == "unit" ) ) {
        delete token;
        unitlist = 1;
    }
    
    CastChangeOrder *order;
    order = new CastChangeOrder;
    order->spell = spell;
    order->quiet = isquiet;
    order->level = 1;
    order->toitem = toitem;
    order->fromitem = fromitem;
    u->AddCastOrder(order);
    
    if(unitlist) {
        UnitId *id = ParseUnit(o);
        while (id) {
            order->units.Add(id);
            id = ParseUnit(o);
        }
    }
}

void Game::ProcessModificationSpell(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
    AString *token = o->gettoken();

    int type;

    if (*token == "Increase") {
        type = 1;
    } else if (*token == "Decrease") {
        type = 3;
    } else {
		u->Error("CAST: Invalid argument.", 0);
		return;
    }
    delete token;
    token = o->gettoken();
    if(!token) {
        u->Error("CAST: Insufficient arguments", 0);
        return;
    }   
    int firstitem = ParseEnabledItem(token);
	delete token;     
    if(firstitem<0) {
		u->Error("CAST: Invalid item.", 0);
		return;
    }

    token = o->gettoken();
    if(!token) {
        u->Error("CAST: Insufficient arguments", 0);
        return;
    }   
    int seconditem = ParseEnabledItem(token);
    delete token;
    if(seconditem<0) {
        u->Error("CAST: Invalid item.", 0);
    	return;
    }

    token = o->gettoken();
	int x = -1;
	int y = -1;
	int z = -1;
    if(token && *token == "region") {
	    delete token;
		token = o->gettoken();
		if(!token) {
			u->Error("CAST: Region X coordinate not specified.", 0);
			return;
		}
		x = token->value();
		delete token;

		token = o->gettoken();
		if(!token) {
			u->Error("CAST: Region Y coordinate not specified.", 0);
			return;
		}
		y = token->value();
		delete token;
		
		RangeType *range = FindRange(SkillDefs[S_MODIFICATION].range);
		if(range && (range->flags & RangeType::RNG_CROSS_LEVELS)) {
			token = o->gettoken();
			if(token) {
				z = token->value();
				delete token;
				if(z < 0 || (z >= Globals->UNDERWORLD_LEVELS +
							Globals->UNDERDEEP_LEVELS +
							Globals->ABYSS_LEVEL + 2)) {
					u->Error("CAST: Invalid Z coordinate specified.", 0);
					return;
				}
			}
		}
    } else {
		if(token) delete token;
	}
	if(x == -1) x = u->object->region->xloc;
	if(y == -1) y = u->object->region->yloc;
	if(z == -1) z = u->object->region->zloc;
    
    CastModifyOrder *order;
    order = new CastModifyOrder;
    order->spell = S_MODIFICATION;
    order->quiet = isquiet;
    order->level = type;
    order->xloc = x;
    order->yloc = y;
    order->zloc = z;
    if(type == 3) {
        order->toitem = seconditem;
        order->fromitem = firstitem;
    } else {
        order->toitem = firstitem;
        order->fromitem = seconditem;
    }
    u->AddCastOrder(order);

}

void Game::ProcessRejuvenationSpell(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
    AString *token = o->gettoken();
    if(!token) {
        u->Error("CAST: No arguments supplied", 0);
        return;
    }   

    int type = ParseTerrain(token);
    delete token;

    if(type >= R_CAVERN) {
        u->Error("CAST: Cannot rejuvenate a region to non-surface terrain.", 0);
        return;
    }

    token = o->gettoken();
	int x = -1;
	int y = -1;
	int z = -1;
    if(token && *token == "region") {
	    delete token;
		token = o->gettoken();
		if(!token) {
			u->Error("CAST: Region X coordinate not specified.", 0);
			return;
		}
		x = token->value();
		delete token;

		token = o->gettoken();
		if(!token) {
			u->Error("CAST: Region Y coordinate not specified.", 0);
			return;
		}
		y = token->value();
		delete token;
		
		RangeType *range = FindRange(SkillDefs[S_MODIFICATION].range);
		if(range && (range->flags & RangeType::RNG_CROSS_LEVELS)) {
			token = o->gettoken();
			if(token) {
				z = token->value();
				delete token;
				if(z < 0 || (z >= Globals->UNDERWORLD_LEVELS +
							Globals->UNDERDEEP_LEVELS +
							Globals->ABYSS_LEVEL + 2)) {
					u->Error("CAST: Invalid Z coordinate specified.", 0);
					return;
				}
			}
		}
    } else {
		if(token) delete token;
	}
	if(x == -1) x = u->object->region->xloc;
	if(y == -1) y = u->object->region->yloc;
	if(z == -1) z = u->object->region->zloc;
    
    CastModifyOrder *order;
    order = new CastModifyOrder;
    order->spell = S_REJUVENATION;
    order->quiet = isquiet;
    order->level = 1;
    order->xloc = x;
    order->yloc = y;
    order->zloc = z;
    order->toitem = type;
    order->fromitem = -1;

    u->AddCastOrder(order);
}

void Game::ProcessPhanDemons(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
    CastIntOrder *order = new CastIntOrder;
    order->spell = S_CREATE_PHANTASMAL_DEMONS;
    order->quiet = isquiet;
    order->level = 0;
    order->target = 1;

    AString *token = o->gettoken();

    if (!token) {
        u->Error("CAST: Illusion to summon must be given.", 0);
        delete order;
        return;
    }

    if (*token == "imp" || *token == "imps") {
        order->level = 1;
    }

    if (*token == "demon" || *token == "demons") {
        order->level = 3;
    }

    if (*token == "balrog" || *token == "balrogs") {
        order->level = 5;
    }

    delete token;

    if (!order->level) {
        u->Error("CAST: Can't summon that illusion.", 0);
        delete order;
        return;
    }

    token = o->gettoken();

    if (!token) {
        order->target = 1;
    } else {
        order->target = token->value();
        delete token;
    }

    u->AddCastOrder(order);
}

void Game::ProcessPhanUndead(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet)
{
	CastIntOrder *order = new CastIntOrder;
	order->spell = S_CREATE_PHANTASMAL_UNDEAD;
    order->quiet = isquiet;
	order->level = 0;
	order->target = 1;

	AString *token = o->gettoken();

	if (!token) {
		u->Error("CAST: Must specify which illusion to summon.", 0);
		delete order;
		return;
	}

	if (*token == "skeleton" || *token == "skeletons") {
		order->level = 1;
	}

	if (*token == "undead") {
		order->level = 3;
	}

	if (*token == "lich" || *token == "liches") {
		order->level = 5;
	}

	delete token;

	if (!order->level) {
		u->Error("CAST: Must specify which illusion to summon.", 0);
		delete order;
		return;
	}

	token = o->gettoken();

	if (token) {
		order->target = token->value();
		delete token;
	} else {
		order->target = 1;
	}

	u->AddCastOrder(order);
}

void Game::ProcessPhanBeasts(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
	CastIntOrder *order = new CastIntOrder;
	order->spell = S_CREATE_PHANTASMAL_BEASTS;
    order->quiet = isquiet;
	order->level = 0;
	order->target = 1;

	AString *token = o->gettoken();

	if (!token) {
		u->Error("CAST: Must specify which illusion to summon.", 0);
		delete order;
		return;
	}

	if (*token == "wolf" || *token == "wolves") {
		order->level = 1;
	}
	if (*token == "eagle" || *token == "eagles") {
		order->level = 3;
	}
	if (*token == "dragon" || *token == "dragon") {
		order->level = 5;
	}

	delete token;
	if (!order->level) {
		delete order;
		u->Error("CAST: Must specify which illusion to summon.", 0);
		return;
	}

	token = o->gettoken();
	if (token) {
		order->target = token->value();
		delete token;
	}

	u->AddCastOrder(order);
}

void Game::ProcessPhanCreatures(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
	CastMenOrder *order = new CastMenOrder;
	order->spell = S_ILLUSORY_CREATURES;
    order->quiet = isquiet;
	order->level = 0;
	order->men = 1;

	AString *token = o->gettoken();

	if (!token) {
		u->Error("CAST: Must specify which illusion to summon.", 0);
		delete order;
		return;
	}

	if (*token == "wolf" || *token == "wolves") {
		order->level = 1;
		order->race = I_IWOLF;
	}
	if (*token == "eagle" || *token == "eagles" || *token == "eagl") {
		order->level = 2;
		order->race = I_IEAGLE;
	}
	if (*token == "gryffin" || *token == "gryffins" || *token == "gryf") {
		order->level = 3;
		order->race = I_IGRYFFIN;
	}
 	if (*token == "dragon" || *token == "dragon" || *token == "drag") {
		order->level = 4;
		order->race = I_IDRAGON;
	}
	if (*token == "skeleton" || *token == "skeletons" || *token == "skel") {
		order->level = 1;
		order->race = I_ISKELETON;
	}
	if (*token == "undead" || *token == "unde") {
		order->level = 2;
		order->race = I_IUNDEAD;
	}
	if (*token == "lich" || *token == "liches") {
		order->level = 3;
		order->race = I_ILICH;
	}
    if (*token == "imp" || *token == "imps") {
        order->level = 1;
		order->race = I_IIMP;
    }
    if (*token == "demon" || *token == "demons" || *token == "demo") {
        order->level = 2;
		order->race = I_IDEMON;
    }
    if (*token == "balrog" || *token == "balrogs" || *token == "balr") {
        order->level = 3;
		order->race = I_IBALROG;
    }

	delete token;
	if (!order->level) {
		delete order;
		u->Error("CAST: Must specify which illusion to summon.", 0);
		return;
	}

	token = o->gettoken();
	if (token) {
		order->men = token->value();
		delete token;
	}
	u->AddCastOrder(order);
}

void Game::ProcessGenericSpell(Unit *u,int spell, OrdersCheck *pCheck, int isquiet )
{
	CastOrder *order = new CastOrder;
	order->spell = spell;
    order->quiet = isquiet;
	order->level = 1;
	u->AddCastOrder(order);
}

void Game::ProcessSummonCreaturesSpell(Unit *u, AString *o, int spell, OrdersCheck *pCheck, int isquiet )
{
	AString *token = o->gettoken();
	int number = -1;
    if(token) {
        number = token->value();
    }
	CastIntOrder *order = new CastIntOrder;
	order->spell = spell;
    order->quiet = isquiet;
	order->level = 1;
	order->target = number;
	u->AddCastOrder(order);
}

void Game::ProcessArtifactSpell(Unit *u, AString *o, int sk, OrdersCheck *pCheck, int isquiet )
{
    if(!Globals->ARCADIA_MAGIC) return;
	AString *token = o->gettoken();
	int type = -1;
    if(token) {
        if(*token == "protection" || *token == "ampr" || *token == "protections") {
            type = 1;
        } else if(*token == "shieldstone" || *token == "shst" || *token == "shieldstones") {
            type = 3;
        }
        delete token;
    } 
    if(type<0) {
        u->Error("CAST: Invalid argument", 0);
        return;
    }
    
	CastOrder *order = new CastOrder;
	order->spell = sk;
    order->quiet = isquiet;
	order->level = type;
	u->AddCastOrder(order);
}



void Game::ProcessRegionSpell(Unit *u, AString *o, int spell,
		OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
	int x = -1;
	int y = -1;
	int z = -1;
	int size = 0;
	RangeType *range = FindRange(SkillDefs[spell].range);

	// Edit to allow large region casting - Earthsea
    if(token && *token == "large") {
         size = 3;
         delete token;
         token = o->gettoken();
    }	

	if(token) {
		if(*token == "region") {
			delete token;
			token = o->gettoken();
			if(!token) {
				u->Error("CAST: Region X coordinate not specified.", 0);
				return;
			}
			x = token->value();
			delete token;

			token = o->gettoken();
			if(!token) {
				u->Error("CAST: Region Y coordinate not specified.", 0);
				return;
			}
			y = token->value();
			delete token;

			if(range && (range->flags & RangeType::RNG_CROSS_LEVELS)) {
				token = o->gettoken();
				if(token) {
					z = token->value();
					delete token;
					if(z < 0 || (z >= Globals->UNDERWORLD_LEVELS +
								Globals->UNDERDEEP_LEVELS +
								Globals->ABYSS_LEVEL + 2)) {
						u->Error("CAST: Invalid Z coordinate specified.", 0);
						return;
					}
				}
			}
		} else {
			delete token;
		}
	}
	if(x == -1) x = u->object->region->xloc;
	if(y == -1) y = u->object->region->yloc;
	if(z == -1) z = u->object->region->zloc;

	CastRegionOrder *order;
	if(spell == S_TELEPORTATION)
		order = new TeleportOrder;
	else
		order = new CastRegionOrder;
	order->spell = spell;
    order->quiet = isquiet;
	order->level = 1+size; //Earthsea mod. sets level to 4 if "large"
	order->xloc = x;
	order->yloc = y;
	order->zloc = z;

	
	/* Teleports happen late in the turn! */
	if(spell == S_TELEPORTATION) {
	    u->ClearTeleportOrders();
		u->teleportorders = (TeleportOrder *)order;
	} else {
	    u->AddCastOrder(order);
	}
}

void Game::ProcessSummonMen(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
	if (!token) {
		u->Error("CAST: No arguments submitted.", 0);
		return;
	}
	int number = token->value();
	delete token; 	
	if(number<1) {
		u->Error("CAST: Specify number of men to summon.", 0);
		return;
	}
	token = o->gettoken(); 
	if (!token) {
		u->Error("CAST: No race specified.", 0);
		return;
	} 
    int race = ParseEnabledItem(token);
	delete token;     
    if(!(ItemDefs[race].type & IT_MAN)) {
		u->Error("CAST: Invalid Race.", 0);
		return;
    }
 	token = o->gettoken(); 
	if (!token || (!(*token == "unit") && !(*token == "units")) ) {
		u->Error("CAST: Unit not specified.", 0);
		delete token;
		return;
	}
	delete token;  
  	 
	CastMenOrder *order;
	order = new CastMenOrder;
	order->spell = S_SUMMON_MEN; //spell;
    order->quiet = isquiet;
	order->level = 1;
	order->men = number;
	order->race = race;

	UnitId *id = ParseUnit(o);
	//For summon men only the first unit will be used.
	while(id) {
		order->units.Add(id);
		id = ParseUnit(o);
	}
	u->AddCastOrder(order);
}

void Game::ProcessCastPortalLore(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
	AString *token = o->gettoken();
	if (!token) {
		u->Error("CAST: Requires a target mage.", 0);
		return;
	}
	int gate = token->value();
	delete token;
	token = o->gettoken();

	if (!token) {
		u->Error("CAST: No units to teleport.", 0);
		return;
	}

	if (!(*token == "units") && !(*token == "unit") ) {
		u->Error("CAST: No units to teleport.", 0);
		delete token;
		return;
	}

	TeleportOrder *order;

	if (u->teleportorders && u->teleportorders->spell == S_PORTAL_LORE) {
		order = u->teleportorders;
	} else {
		order = new TeleportOrder;
		u->ClearTeleportOrders();
		u->teleportorders = order;
	}

	order->gate = gate;
	order->spell = S_PORTAL_LORE;
    order->quiet = isquiet;
	order->level = 1;

	UnitId *id = ParseUnit(o);
	while(id) {
		order->units.Add(id);
		id = ParseUnit(o);
	}
}

void Game::ProcessCastGateLore(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
	AString *token = o->gettoken();

	if (!token) {
		u->Error("CAST: Missing argument.", 0);
		return;
	}

	if ((*token) == "gate") {
		delete token;
		token = o->gettoken();

		if (!token) {
			u->Error("CAST: Requires a target gate.", 0);
			return;
		}

		TeleportOrder *order;

		if (u->teleportorders && u->teleportorders->spell == S_GATE_LORE &&
				u->teleportorders->gate == token->value()) {
			order = u->teleportorders;
		} else {
			order = new TeleportOrder;
			u->ClearTeleportOrders();
			u->teleportorders = order;
		}

		order->gate = token->value();
		order->spell = S_GATE_LORE;
        order->quiet = isquiet;
		order->level = 3;

		delete token;

		token = o->gettoken();

		if (!token) return;
		if (!(*token == "units") && !(*token == "unit")) {
			delete token;
			return;
		}

		UnitId *id = ParseUnit(o);
		while(id) {
			order->units.Add(id);
			id = ParseUnit(o);
		}
		return;
	}

	if ((*token) == "random") {
		TeleportOrder *order;

		if (u->teleportorders && u->teleportorders->spell == S_GATE_LORE &&
				u->teleportorders->gate == -1 ) {
			order = u->teleportorders;
		} else {
			order = new TeleportOrder;
			u->ClearTeleportOrders();
			u->teleportorders = order;
		}

		order->gate = -1;
		order->spell = S_GATE_LORE;
        order->quiet = isquiet;
		order->level = 1;

		delete token;

		token = o->gettoken();

		if (!token) return;
		if (!(*token == "units") && !(*token == "unit")) {
			delete token;
			return;
		}

		UnitId *id = ParseUnit(o);
		while(id) {
			order->units.Add(id);
			id = ParseUnit(o);
		}
		return;
	}

	if ((*token) == "detect") {
		delete token;
		CastOrder *to = new CastOrder;
		to->spell = S_GATE_LORE;
		to->level = 2;
        to->quiet = isquiet;
		token = o->gettoken();
		if(token) {
		    if(*token == "large") to->level = 5;
		    delete token;
		}
		u->AddCastOrder(to);
		return;
	}

	delete token;
	u->Error("CAST: Invalid argument.", 0);
}

void Game::ProcessHypnosisSpell(Unit *u,AString *o, OrdersCheck *pCheck, int isquiet )
{
/* Am going to have to move cast before tax! */
	AString *token = o->gettoken();
	int level = 1;
	Order * monthorders = 0;
	int tax = TAX_NONE;
	
	if (!token) {
		u->Error("CAST: Missing argument.", 0);
		return;
	}

	if(*token == "work") {
	    delete token;
	    token = o->gettoken();
	    
	   	ProduceOrder *p = new ProduceOrder;
		p->skill = -1;
	    p->item = I_SILVER;
	    monthorders = p;
    } else if(*token == "produce") {
        delete token;
        token = o->gettoken();
	    if (!token) {
		    ParseError(pCheck, u, 0, "CAST: No item given to produce.");
		    return;
	    }
	    int it = ParseEnabledItem(token);
		delete token;
		token = o->gettoken();
		
	    if (it == -1 || ItemDefs[it].flags & ItemType::DISABLED) {
		    ParseError(pCheck, u, 0, "CAST: Can't produce that.");
		    return;
	    }
	    
        ProduceOrder *p = new ProduceOrder;
	    p->item = it;
	    AString skname = ItemDefs[it].pSkill;
	    p->skill = LookupSkill(&skname);
	    monthorders = p;
    } else if(*token == "sail") {
        delete token;
//        token = o->gettoken();
        level = 5;
    
    	SailOrder *m = new SailOrder;

        int done = 0;
        int atleastone = 0;
    	while (!done) {
    		token = o->gettoken();
    		if(!token) done = 1;
    		else if (*token == "units" || *token == "unit" ) done = 1;
    		else {
    		    int d = ParseDir(token);
    		    delete token;
    		    if (d!=-1) {
    			    MoveDir *x = new MoveDir;
    			    x->dir = d;
    			    m->dirs.Add(x);
    			    atleastone = 1;
    		    } else {
    		        u->Error("CAST: Bad move direction.", 0);
    		        return;
    		    }
		    }
    	}
    	if(!atleastone) {
    	    u->Error("CAST: No directions specified");
    	    return;
    	}
        monthorders = m;

    } else if(*token == "tax") {
        delete token;
        token = o->gettoken();
        tax = TAX_TAX;
    } else if(*token == "pillage") {
        delete token;
        token = o->gettoken();
        level = 5;
        tax = TAX_PILLAGE;
    } else if(*token == "move") {
        delete token;
        level = 3;
        
        MoveOrder *m = new MoveOrder;
        m->advancing = 0;
                
        int done = 0;
        int atleastone = 0;
    	while (!done) {
    		token = o->gettoken();
    		if(!token) done = 1;
    		else if (*token == "units" || *token == "unit") done = 1;
    		else {
    		    int d = ParseDir(token);
    		    delete token;
    		    if (d != -1) {
    			    MoveDir *x = new MoveDir;
    			    x->dir = d;
    			    m->dirs.Add(x);
    			    atleastone = 1;
    		    } else {
    		        u->Error("CAST: Bad move direction.", 0);
    		        return;
    		    }
		    }
    	}
    	if(!atleastone) {
    	    u->Error("CAST: No directions specified");
    	    return;
    	}
        monthorders = m;
        
    } else if(*token == "advance") {
        delete token;
        level = 4;
        
        MoveOrder *m = new MoveOrder;
        m->advancing = 1;
        m->type = O_ADVANCE;
        
        int done = 0;
        int atleastone = 0;
    	while (!done) {
    		token = o->gettoken();
    		if(!token) done = 1;
    		else if (*token == "units" || *token == "unit") done = 1;
    		else {
    		    int d = ParseDir(token);
    		    delete token;
    		    if (d!=-1) {
    			    MoveDir *x = new MoveDir;
    			    x->dir = d;
    			    m->dirs.Add(x);
    			    atleastone = 1;
    		    } else {
    		        u->Error("CAST: Bad move direction.", 0);
    		        return;
    		    }
		    }
    	}
    	if(!atleastone) {
    	    u->Error("CAST: No directions specified");
    	    return;
    	}

        monthorders = m;

	} else if(*token == "study") {
        delete token;
        level = 2;
        token = o->gettoken();

    	if (!token) {
    		u->Error("CAST: No skill given.", 0);
    		return;
    	}
    	int sk = ParseSkill(token);
    	delete token;
    	token = o->gettoken();
    	
    	if (sk==-1 || (SkillDefs[sk].flags & SkillType::DISABLED)) {
    		u->Error("CAST: Invalid skill.", 0);
    		return;
    	}
    	
    	if((SkillDefs[sk].flags & SkillType::APPRENTICE) || 
                (SkillDefs[sk].flags & SkillType::APPRENTICE)) {
    		u->Error("CAST: Cannot hypnotise with magic or apprentice skills.", 0);
    		return;
    	}
    
    	StudyOrder *m = new StudyOrder;
    	m->skill = sk;
    	m->days = 0;
    
        monthorders = m;
        
	} else {
	    delete token;
	    u->Error("CAST: Invalid month order", 0);
	    return;
    }

	if (!token) {
		u->Error("CAST: No units specified to hypnotise.", 0);
		return;
	}
	if(!(*token == "units") && !(*token == "unit")) {
	    delete token;
	    u->Error("CAST: Must specify units to hypnotise.", 0);
	    return;
	}
    delete token;

	CastHypnosisOrder *order;
	order = new CastHypnosisOrder;
	order->spell = S_HYPNOSIS; //spell;
    order->quiet = isquiet;
	order->level = level;  // level required.
	order->monthorder = monthorders;
	order->taxing = tax;

	UnitId *id = ParseUnit(o);
	while(id) {
		order->units.Add(id);
		id = ParseUnit(o);
	}
	
	u->AddCastOrder(order);
}

void Game::RunACastOrder(ARegion * r,Object *o,Unit * u, CastOrder *order)
{
	int val;
	if (u->type != U_MAGE) {
		u->Error("CAST: Unit is not a mage.", order->quiet);
		return;
	}

	if (order->level == 0) {
		order->level = u->GetSkill(order->spell);
	}
	if (u->GetSkill(order->spell) < order->level ||
			order->level == 0) {
		u->Error("CAST: Skill level isn't that high.", order->quiet);
		return;
	}
	u->activecastorder = order;
	int sk = order->spell;
	switch (sk) {
		case S_MIND_READING:
			val = RunMindReading(r,u);
			break;
		case S_ENCHANT_ARMOR:
			val = RunEnchantArmor(r,u);
			break;
		case S_ENCHANT_SWORDS:
			val = RunEnchantSwords(r,u);
			break;
		case S_CONSTRUCT_GATE:
			val = RunConstructGate(r,u,sk,order->quiet);
			break;
		case S_ENGRAVE_RUNES_OF_WARDING:
			val = RunEngraveRunes(r,o,u,order->quiet);
			break;
		case S_CONSTRUCT_PORTAL:
			val = RunCreateArtifact(r,u,sk,I_PORTAL);
			break;
		case S_CREATE_RING_OF_INVISIBILITY:
			val = RunCreateArtifact(r,u,sk,I_RINGOFI);
			break;
		case S_CREATE_CLOAK_OF_INVULNERABILITY:
			val = RunCreateArtifact(r,u,sk,I_CLOAKOFI);
			break;
		case S_CREATE_STAFF_OF_FIRE:
			val = RunCreateArtifact(r,u,sk,I_STAFFOFF);
			break;
		case S_CREATE_STAFF_OF_LIGHTNING:
			val = RunCreateArtifact(r,u,sk,I_STAFFOFL);
			break;
		case S_CREATE_AMULET_OF_TRUE_SEEING:
			val = RunCreateArtifact(r,u,sk,I_AMULETOFTS);
			break;
		case S_CREATE_AMULET_OF_PROTECTION:
			val = RunCreateArtifact(r,u,sk,I_AMULETOFP);
			break;
		case S_CREATE_RUNESWORD:
			val = RunCreateArtifact(r,u,sk,I_RUNESWORD);
			break;
		case S_CREATE_SHIELDSTONE:
			val = RunCreateArtifact(r,u,sk,I_SHIELDSTONE);
			break;
		case S_CREATE_MAGIC_CARPET:
			val = RunCreateArtifact(r,u,sk,I_MCARPET);
			break;
		case S_CREATE_FLAMING_SWORD:
			val = RunCreateArtifact(r,u,sk,I_FSWORD);
			break;
		case S_SUMMON_IMPS:
		    if(Globals->ARCADIA_MAGIC) val = RunSummonCreatures(r,u,sk,I_IMP,-1);
			else val = RunSummonImps(r,u);
			break;
		case S_SUMMON_DEMON:
		    if(Globals->ARCADIA_MAGIC) val = RunSummonCreatures(r,u,sk,I_DEMON,-1);
			else val = RunSummonDemon(r,u);
			break;
		case S_SUMMON_BALROG:
		    if(Globals->ARCADIA_MAGIC) val = RunSummonHigherCreature(r,u,sk,I_BALROG);
		    else val = RunSummonBalrog(r,u,order->quiet);
			break;
		case S_SUMMON_LICH:
			if(Globals->ARCADIA_MAGIC) val = RunSummonHigherCreature(r,u,sk,I_LICH);
		    else val = RunSummonLich(r,u);
			break;
		case S_RAISE_UNDEAD:
		    if(Globals->ARCADIA_MAGIC) val = RunSummonCreatures(r,u,sk,I_UNDEAD,-1);
			else val = RunRaiseUndead(r,u);
			break;
		case S_SUMMON_SKELETONS:
			val = RunSummonSkeletons(r,u);
			break;
		case S_DRAGON_LORE:
			val = RunDragonLore(r,u,order->quiet);
			break;
		case S_BIRD_LORE:
			val = RunBirdLore(r,u);
			break;
		case S_WOLF_LORE:
		    if(Globals->ARCADIA_MAGIC && SkillDefs[sk].cast_cost > 0) val = RunSummonCreatures(r,u,sk,I_WOLF,8);
			else val = RunWolfLore(r,u,order->quiet);
			break;
		case S_INVISIBILITY:
			val = RunInvisibility(r,u);
			break;
		case S_CREATE_PHANTASMAL_DEMONS:
			val = RunPhanDemons(r,u);
			break;
		case S_CREATE_PHANTASMAL_UNDEAD:
			val = RunPhanUndead(r,u);
			break;
		case S_CREATE_PHANTASMAL_BEASTS:
			val = RunPhanBeasts(r,u);
			break;
		case S_GATE_LORE:
			val = RunDetectGates(r,o,u);
			break;
		case S_FARSIGHT:
			val = RunFarsight(r,u); // "large" edit!
			break;
		case S_EARTH_LORE:
			val = RunEarthLore(r,u);
			break;
		case S_WEATHER_LORE:
			val = RunWeatherLore(r, u);
			break;
		case S_CLEAR_SKIES:
			val = RunClearSkies(r,u);
			break;
		case S_CREATE_FOOD:
			val = RunCreateFood(r, u);
			break;
/* Earthsea Spells */
        case S_PHANTASMAL_ENTERTAINMENT:
            val = RunPhanEntertainment(r,u);
            break;
		case S_ARTIFACT_LORE:
		case S_BASE_ARTIFACTLORE:
            if( (order->level > 1) && (u->GetSkill(sk) > 2) ) val = RunCreateArtifact(r,u,sk,I_SHIELDSTONE);
			else val = RunCreateArtifact(r,u,sk,I_AMULETOFP);
			break;
		case S_BLIZZARD:
		    val = RunBlizzard(r,u);
		    break;
		case S_FOG:                  // Difficult: need to get final mage region!
		    val = RunFog(r,u);
		    break;
		case S_ILLUSORY_CREATURES:
		    val = RunPhanCreatures(r,u);
		    break;
		case S_SUMMON_MEN:
		    val = RunSummonMen(r,u);
		    break;
		case S_SPIRIT_OF_DEAD:
		    val = RunSpiritOfDead(r,u);
		    break;
		case S_TRANSMUTATION:
		    val = RunTransmutation(r,u);
		    break;
		case S_MODIFICATION:
		    val = RunModification(r,u);
		    break;
		case S_REJUVENATION:
		    val = RunRejuvenation(r,u);
		    break;
		case S_DIVERSION:
		    val = RunDiversion(r,u);
		    break;
		case S_GRYFFIN_LORE:
		    val = RunSummonHigherCreature(r,u,sk,I_GRYFFIN);
		    break;
		case S_SEAWARD:
		    val = RunSeaward(r,u);
		    break;
		case S_HYPNOSIS:
		    val = RunHypnosis(r,u);
		    break;
		case S_CREATE_PORTAL:
		    val = RunCreatePortal(r,u);
		    break;
	}
	if (val) {
		u->Practice(sk);
		r->NotifySpell(u, SkillDefs[sk].abbr, &regions);
	}
    u->activecastorder = 0;

}

int Game::GetRegionInRange(ARegion *r, ARegion *tar, Unit *u, int spell, int quiet, int penalty)
{
	int level = u->GetSkill(spell);
	if(!level) {
		u->Error("CAST: You don't know that spell.", quiet);
		return 0;
	}

	RangeType *range = FindRange(SkillDefs[spell].range);
	if (range == NULL) {
		u->Error("CAST: Spell is not castable at range.", quiet);
		return 0;
	}

	if(penalty) level -= penalty;

	int rtype = regions.GetRegionArray(r->zloc)->levelType;
	if((rtype == ARegionArray::LEVEL_NEXUS) &&
			!(range->flags & RangeType::RNG_NEXUS_SOURCE)) {
		u->Error("CAST: Spell does not work from the Nexus.", quiet);
		return 0;
	}

	if(!tar) {
		u->Error("CAST: No such region.", quiet);
		return 0;
	}

	rtype = regions.GetRegionArray(tar->zloc)->levelType;
	if((rtype == ARegionArray::LEVEL_NEXUS) &&
			!(range->flags & RangeType::RNG_NEXUS_TARGET)) {
		u->Error("CAST: Spell does not work to the Nexus.", quiet);
		return 0;
	}

	if((rtype != ARegionArray::LEVEL_SURFACE) &&
			(range->flags & RangeType::RNG_SURFACE_ONLY)) {
		u->Error("CAST: Spell can only target regions on the surface.", quiet);
		return 0;
	}
	if(!(range->flags&RangeType::RNG_CROSS_LEVELS) && (r->zloc != tar->zloc)) {
		u->Error("CAST: Spell is not able to work across levels.", quiet);
		return 0;
	}

	int maxdist;
	switch(range->rangeClass) {
		default:
		case RangeType::RNG_ABSOLUTE:
			maxdist = 1;
			break;
		case RangeType::RNG_LEVEL:
			maxdist = level;
			break;
		case RangeType::RNG_LEVEL2:
			maxdist = level * level;
			break;
		case RangeType::RNG_LEVEL3:
			maxdist = level * level * level;
			break;
	}
	maxdist *= range->rangeMult;

	int dist;
	if(range->flags & RangeType::RNG_CROSS_LEVELS)
		dist = regions.GetPlanarDistance(tar, r, range->crossLevelPenalty);
	else
		dist = regions.GetDistance(tar, r);
	if(dist > maxdist) {
		u->Error("CAST: Target region out of range.", quiet);
		return 0;
	}
	return 1;
}

int Game::RunMindReading(ARegion *r,Unit *u)
{
	CastMindOrder *order = (CastMindOrder *) u->activecastorder;
	int level = u->GetSkill(S_MIND_READING);

	Unit *tar = r->GetUnitId(order->id,u->faction->num);
	if (!tar) {
		u->Error("No such unit.", order->quiet);
		return 0;
	}
	
	//free spell to use.
	
	u->Experience(S_MIND_READING, 10);
	
	int falsefaction = 0;
	#ifdef FIZZLES
	int mevent = u->MysticEvent();
	AString temp2;
    switch(mevent) {
        case 4:
            u->Error("Attempts to read minds, but the spell is reflected.", order->quiet);
        	temp2 = AString("Gets a vision of: ") + *(u->name) + ", " +
        		*(u->faction->name) + ".";
        	temp2 += u->items.Report(2,5,0,u->type) + ". Skills: ";
        	temp2 += u->skills.Report(u->GetMen()) + ".";
        	tar->Event(temp2);
        	return 1;
        case 3:
            falsefaction = 1;
            break;
        case 2:
        case 1:
            u->Error("Attempts to read minds, but the spell fizzles.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif

	AString temp = AString("Casts Mind Reading: ") + *(tar->name) + ", ";
    if(!falsefaction) temp += *(tar->faction->name);
    else {
        int num = 0;
        Faction *falsefaction = 0;
        forlist(&factions) {
            Faction *f = (Faction *) elem;
            if(f->IsNPC()) continue;
            if(f == u->faction) continue; //don't want it appearing as the mage's faction.
            if(num && f == tar->faction) continue; //don't want the real faction if there's an alternative
            if( getrandom(++num) ) continue; //random faction selection
            falsefaction = f;
        }
        if(falsefaction) temp += *(falsefaction->name);
    }

	if (level < 3 || (!Globals->ARCADIA_MAGIC && level < 5)) {
		u->Event(temp + ".");
		return 1;
	}

	temp += tar->items.Report(2,5,0,tar->type) + ". Skills: ";
	temp += tar->skills.Report(tar->GetMen()) + ".";

	u->Event(temp);
	return 1;
}

int Game::RunEnchantArmor(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_ENCHANT_ARMOR);
	int max = ItemDefs[I_MPLATE].mOut * level;
//	int count = 0;
	unsigned int c;
//	int found;

	CastOrder *order = u->activecastorder;

	int cost = u->GetCastCost(S_ENCHANT_ARMOR,order->extracost,max);
	while(u->energy < cost) {
        cost = u->GetCastCost(S_ENCHANT_ARMOR,order->extracost,--max);
    }
    if(!max) {
        u->Error("CAST: Not enough energy to cast that.", order->quiet);
        return 0;
    }


	for(c = 0; c < sizeof(ItemDefs->mInput)/sizeof(Materials); c++) {
		int i = ItemDefs[I_MPLATE].mInput[c].item;
		if(i != -1) {
			int amt = u->GetSharedNum(i);
			if(amt/ItemDefs[I_MPLATE].mInput[c].amt < max) {
				max = amt/ItemDefs[I_MPLATE].mInput[c].amt;
			}
		}
	}

	if (max == 0) {
        u->Error("CAST: Do not have the required item.", order->quiet);
        return 0;
    }

	// Deduct the items used
	for(c = 0; c < sizeof(ItemDefs->mInput)/sizeof(Materials); c++) {
		int i = ItemDefs[I_MPLATE].mInput[c].item;
		int a = ItemDefs[I_MPLATE].mInput[c].amt;
		if(i != -1) {
			u->ConsumeShared(i, max*a);
		}
	}

/*

	// Figure out how many components there are
	for(c=0; c<sizeof(ItemDefs[I_MPLATE].mInput)/sizeof(Materials); c++) {
		if(ItemDefs[I_MPLATE].mInput[c].item != -1) count++;
	}

	while(max) {
		int i, a;
		found = 0;
		// See if we have enough of all items
		for(c=0; c<sizeof(ItemDefs[I_MPLATE].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_MPLATE].mInput[c].item;
			a = ItemDefs[I_MPLATE].mInput[c].amt;
			if(i != -1) {
				if(u->items.GetNum(i) >= a) found++;
			}
		}
		// We do not, break.
		if(found != count) break;

		// Decrement our inputs
		for(c=0; c<sizeof(ItemDefs[I_MPLATE].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_MPLATE].mInput[c].item;
			a = ItemDefs[I_MPLATE].mInput[c].amt;
			if(i != -1) {
				u->items.SetNum(i, u->items.GetNum(i) - a);
			}
		}
		// We've made one.
		num++;
		max--;
	}
*/


	u->energy -= u->GetCastCost(S_ENCHANT_ARMOR,order->extracost,max);
	int experience = 4*max / level;
	if(experience > 10) experience = 10;
    u->Experience(S_ENCHANT_ARMOR,experience);

	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
            u->Error("Attempts to enchant armour, but the spell goes wrong, producing chain armour.", order->quiet);
            u->items.SetNum(I_CHAINARMOR,u->items.GetNum(I_CHAINARMOR) + max);
	        u->Event(AString("Enchants ") + num + " chain armour.");
            return 1;
        case 3:
            u->Error("Attempts to enchant armour, but the spell goes wrong, producing pearl plate armour.", order->quiet);
            u->items.SetNum(I_PPLATE,u->items.GetNum(I_PPLATE) + max);
	        u->Event(AString("Enchants ") + num + " pearl plate armour.");
            return 1;
        case 2:
        case 1:
            u->Error("Attempts to enchant armour, but the spell fizzles and they disappear.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif

	u->items.SetNum(I_MPLATE,u->items.GetNum(I_MPLATE) + max);
	u->Event(AString("Enchants ") + max + " mithril armour.");
	return 1;
}

int Game::RunEnchantSwords(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_ENCHANT_SWORDS);
	int max = ItemDefs[I_MSWORD].mOut * level;
//	int count = 0;
	unsigned int c;
//	int found;

	CastOrder *order = u->activecastorder;
	int cost = u->GetCastCost(S_ENCHANT_SWORDS,order->extracost,max);
	while(u->energy < cost) {
        cost = u->GetCastCost(S_ENCHANT_SWORDS,order->extracost,--max);
    }
    if(!max) {
        u->Error("CAST: Not enough energy to cast that.", order->quiet);
        return 0;
    }
    
    for(c = 0; c < sizeof(ItemDefs->mInput)/sizeof(Materials); c++) {
		int i = ItemDefs[I_MSWORD].mInput[c].item;
		if(i != -1) {
			int amt = u->GetSharedNum(i);
			if(amt/ItemDefs[I_MSWORD].mInput[c].amt < max) {
				max = amt/ItemDefs[I_MSWORD].mInput[c].amt;
			}
		}
	}

	if (max == 0) {
        u->Error("CAST: Do not have the required item.", order->quiet);
        return 0;
    }

	// Deduct the items used
	for(c = 0; c < sizeof(ItemDefs->mInput)/sizeof(Materials); c++) {
		int i = ItemDefs[I_MSWORD].mInput[c].item;
		int a = ItemDefs[I_MSWORD].mInput[c].amt;
		if(i != -1) {
			u->ConsumeShared(i, max*a);
		}
	}
    
/*
	// Figure out how many components there are
	for(c=0; c<sizeof(ItemDefs[I_MSWORD].mInput)/sizeof(Materials); c++) {
		if(ItemDefs[I_MSWORD].mInput[c].item != -1) count++;
	}

	while(max) {
		int i, a;
		found = 0;
		// See if we have enough of all items
		for(c=0; c<sizeof(ItemDefs[I_MSWORD].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_MSWORD].mInput[c].item;
			a = ItemDefs[I_MSWORD].mInput[c].amt;
			if(i != -1) {
				if(u->items.GetNum(i) >= a) found++;
			}
		}
		// We do not, break.
		if(found != count) break;

		// Decrement our inputs
		for(c=0; c<sizeof(ItemDefs[I_MSWORD].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_MSWORD].mInput[c].item;
			a = ItemDefs[I_MSWORD].mInput[c].amt;
			if(i != -1) {
				u->items.SetNum(i, u->items.GetNum(i) - a);
			}
		}
		// We've made one.
		num++;
		max--;
	}
*/
	u->energy -= u->GetCastCost(S_ENCHANT_SWORDS,order->extracost,max);
	int experience = 4*max / level;
	if(experience > 10) experience = 10;
    u->Experience(S_ENCHANT_SWORDS,experience);

	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
        case 3:
        case 2:
        case 1:
            u->Error("Attempts to enchant swords, but the spell fizzles and they disappear.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif

	u->items.SetNum(I_MSWORD,u->items.GetNum(I_MSWORD) + max);
	u->Event(AString("Enchants ") + max + " mithril swords.");
	return 1;
}

int Game::RunCreateFood(ARegion *r,Unit *u)
//Not adjusted for item sharing
{
	int level = u->GetSkill(S_CREATE_FOOD);
	int max = ItemDefs[I_FOOD].mOut * level;
	int num = 0;
	int count = 0;
	unsigned int c;
	int found;

	// Figure out how many components there are
	for(c=0; c<sizeof(ItemDefs[I_FOOD].mInput)/sizeof(Materials); c++) {
		if(ItemDefs[I_FOOD].mInput[c].item != -1) count++;
	}

	while(max) {
		int i, a;
		found = 0;
		// See if we have enough of all items
		for(c=0; c<sizeof(ItemDefs[I_FOOD].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_FOOD].mInput[c].item;
			a = ItemDefs[I_FOOD].mInput[c].amt;
			if(i != -1) {
				if(u->items.GetNum(i) >= a) found++;
			}
		}
		// We do not, break.
		if(found != count) break;

		// Decrement our inputs
		for(c=0; c<sizeof(ItemDefs[I_FOOD].mInput)/sizeof(Materials); c++) {
			i = ItemDefs[I_FOOD].mInput[c].item;
			a = ItemDefs[I_FOOD].mInput[c].amt;
			if(i != -1) {
				u->items.SetNum(i, u->items.GetNum(i) - a);
			}
		}
		// We've made one.
		num++;
		max--;
	}

	u->items.SetNum(I_FOOD,u->items.GetNum(I_FOOD) + num);
	u->Event(AString("Creates ") + num + " food.");
	if (num == 0) return 0;
	return 1;
}

int Game::RunConstructGate(ARegion *r,Unit *u, int spell, int isquiet)
{
	if (TerrainDefs[r->type].similar_type == R_OCEAN) {
		u->Error("Gates may not be constructed at sea.", isquiet);
		return 0;
	}

	if (r->gate) {
		u->Error("There is already a gate in that region.", isquiet);
		return 0;
	}

	if(!Globals->ARCADIA_MAGIC) {
    	if (!u->GetSharedMoney(1000)) {
    		u->Error("Can't afford to construct a Gate.", isquiet);
    		return 0;
    	}
    
    	u->ConsumeSharedMoney(1000);
    
    	int level = u->GetSkill(spell);
    	int chance = level * 20;
    	if (getrandom(100) >= chance) {
    		u->Event("Attempts to construct a gate, but fails.");
    		return 0;
    	}
    
    	u->Event(AString("Constructs a Gate in ")+r->ShortPrint( &regions )+".");
    	regions.numberofgates++;
    	r->gate = regions.numberofgates;
    	if(Globals->GATES_NOT_PERENNIAL) {
    		int dm = Globals->GATES_NOT_PERENNIAL / 2;
    		int gm = month + 1 - getrandom(dm) - getrandom(dm) - getrandom(Globals->GATES_NOT_PERENNIAL % 2);
    		while(gm < 0) gm += 12;
    		r->gatemonth = gm;
    	}
    	return 1;
	}
	
//	int level = u->GetSkill(spell); //only contruct_gate ever seems to be called here ... ?!
	CastOrder *order = u->activecastorder;
	
	int cost = u->GetCastCost(spell,order->extracost,1);
	if(u->energy < cost) {
       u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
       return 0;
    }
    u->energy -= cost;

    u->Experience(spell,20);  // unusual spell to cast.

	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
            if(!u->teleportorders) {
                TeleportOrder *torder;
       			torder = new TeleportOrder;
       			u->teleportorders = torder;
        
        		torder->gate = -1;
        		torder->spell = S_GATE_LORE;
        		torder->level = 1;
        		u->Error("Attempts to construct a gate, but the spell backfires into a random gate jump instead.", order->quiet);
        		return 1;
    		}
        case 3:
        case 2:
        case 1:
            u->Error("Attempts to construct a gate, but the spell fizzles.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif
    
	u->Event(AString("Constructs a Gate in ") + r->ShortPrint( &regions ) + ".");
	regions.numberofgates++;
	r->gate = regions.numberofgates;
	if(Globals->GATES_NOT_PERENNIAL) {
		int dm = Globals->GATES_NOT_PERENNIAL / 2;
		int gm = month + 1 - getrandom(dm) - getrandom(dm) - getrandom(Globals->GATES_NOT_PERENNIAL % 2);
		while(gm < 0) gm += 12;
		r->gatemonth = gm;
	}
	return 1;
}

int Game::RunEngraveRunes(ARegion *r,Object *o,Unit *u, int isquiet)
{
	if (!o->IsBuilding()) {
		u->Error("Runes of Warding may only be engraved on a building.", isquiet);
		return 0;
	}

	if (o->incomplete > 0) {
		u->Error( "Runes of Warding may only be engraved on a completed "
				"building.", isquiet);
		return 0;
	}

	int level = u->GetSkill(S_ENGRAVE_RUNES_OF_WARDING);
	
	if(!Globals->ARCADIA_MAGIC) {
    	switch (level) {
    		case 5:
    			if (o->type == O_MFORTRESS) break;
    		case 4:
    			if (o->type == O_CITADEL) break;
    		case 3:
    			if (o->type == O_CASTLE) break;
    		case 2:
    			if (o->type == O_FORT) break;
    			if (o->type == O_MTOWER) break;
    		case 1:
    			if (o->type == O_TOWER) break;
    		default:
    			u->Error("Not high enough level to engrave Runes of Warding on "
    					"that building.", isquiet);
    			return 0;
    	}
    
    	if (!u->GetSharedMoney(600)) {
    		u->Error("Can't afford to engrave Runes of Warding.", isquiet);
    		return 0;
    	}
    
    	u->ConsumeSharedMoney(600);
    	if( o->type == O_MFORTRESS ) {
    		o->runes = 5;
    	} else if(o->type == O_MTOWER) {
    		o->runes = 4;
    	} else {
    		o->runes = 3;
    	}
    	u->Event(AString("Engraves Runes of Warding on ") + *(o->name) + ".");
    	return 1;
	}
	
	int size = 1;
	if(ObjectDefs[o->type].protect > 39) size++;
	if(ObjectDefs[o->type].protect > 199) size++;
	if(ObjectDefs[o->type].protect > 999) size++;
	
	if(size > level) {
	    u->Error("CAST: Insufficient level to engrave runes on that building.", isquiet);
	    return 0;
	}
	
	
    if(!(ObjectDefs[o->type].flags & ObjectType::CANMODIFY)) {
        u->Error("CAST: Cannot engrave runes on that building type.", isquiet);
        return 0;    
    }
	
	CastOrder * order = (CastOrder *) u->activecastorder;
	
	int cost = u->GetCastCost(S_ENGRAVE_RUNES_OF_WARDING,order->extracost,size);
	if(u->energy < cost) {
       u->Error("CAST: Not enough energy to cast that spell.", isquiet);
       return 0;
    }
    u->energy -= cost;
    
    int experience = (30*size)/level;
    if(experience > 16) experience = 16;
    u->Experience(S_ENGRAVE_RUNES_OF_WARDING,experience);

	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
            if(o->runes == 0 && level <= 4) {
                u->Error("While engraving runes, the spell escapes the mage's control, engraving runes two levels higher than usual.", isquiet);
                level += 2;
                break;
            }
        case 3:
            if(o->runes) {
                u->Error("Attempts to engrave runes, but the spell backfires, leaving the building without engravings.", isquiet);
                o->runes = 0;
                return 1;
            }
        case 2:
        case 1:
            u->Error("Attempts to engrave runes, but the spell fizzles.", isquiet);
            return 1;
        default:
            break;
    }
	#endif
	
	o->runes = level;
	u->Event(AString("Engraves Runes of Warding on ") + *(o->name) + ".");
	return 1;
}

int Game::RunSummonBalrog(ARegion *r,Unit *u, int isquiet)
{
	int level = u->GetSkill(S_SUMMON_BALROG);
	if(!Globals->ARCADIA_MAGIC) {
    	if (u->items.GetNum(I_BALROG) >= ItemDefs[I_BALROG].max_inventory) {
    		u->Error("Can't control any more balrogs.", isquiet);
    		return 0;
    	}
    
    	int num = (level * 20 + getrandom(100)) / 100;
    
    	u->items.SetNum(I_BALROG,u->items.GetNum(I_BALROG) + num);
    	u->Event(AString("Summons ") + ItemString(I_BALROG,num) + ".");
    	return 1;
	}
    if (u->items.GetNum(I_BALROG) >= level) {
    	u->Error("Can't control any more balrogs.", isquiet);
    	return 0;
    }
    return 0;
}

int Game::RunSummonDemon(ARegion *r,Unit *u)
{
	u->items.SetNum(I_DEMON,u->items.GetNum(I_DEMON) + 1);
	u->Event(AString("Summons ") + ItemString(I_DEMON,1) + ".");
	return 1;
}

int Game::RunSummonImps(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_SUMMON_IMPS);

	u->items.SetNum(I_IMP,u->items.GetNum(I_IMP) + level);
	u->Event(AString("Summons ") + ItemString(I_IMP,level) + ".");
	return 1;
}

int Game::RunCreateArtifact(ARegion *r,Unit *u,int skill,int item)
{
    CastOrder *order = (CastOrder *) u->activecastorder;

	int level = u->GetSkill(skill);
	unsigned int c;

	if(Globals->ARCADIA_MAGIC) {
	    int num;	
        if(SkillDefs[skill].flags & SkillType::COSTVARIES) num = 1;    //always produce one
        else num = (level * level * ItemDefs[item].mOut)/100;                  //produce level^2 * fixed value. This should give at least 2 for level=2, ie .mOut needs to be at least 50, typically 100.
        int max = num;
        
	    int cost = u->GetCastCost(skill,order->extracost,1);  //cost to make one if num = 1, or level otherwise.
	    if(num > 1) cost = (cost + level - 1) / level;        //cost to make one. Final cost is recalculated below to account for rounding.
		if(cost > u->energy) {
		    u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
		    return 0;
		}

    	for(c = 0; c < sizeof(ItemDefs[item].mInput)/sizeof(Materials); c++) {
    		if(ItemDefs[item].mInput[c].item == -1) continue;
    		int amt = u->GetSharedNum(ItemDefs[item].mInput[c].item);
    		int itemcost = ItemDefs[item].mInput[c].amt;
    		if(amt < itemcost) {
    			u->Error(AString("CAST: Doesn't have sufficient ") +
    					ItemDefs[ItemDefs[item].mInput[c].item].name +
    					" to create that.", order->quiet);
    			return 0;
    		}
    		if(amt < itemcost*num) num = amt / itemcost;     //reduce num if we don't have enough items.
    	}
		
		if(num > 1) {
		    cost = u->GetCastCost(skill,order->extracost,num);  //cost to make num*level
		    cost = (cost + level - 1) / level;                  //cost to make num.
		    if( cost > u->energy) {
                num = (num*u->energy)/cost;               //reduce num if we don't have the energy.
		        cost = u->GetCastCost(skill, order->extracost, num);
		        cost = (cost + level - 1) / level;                  //cost to make num.
            }
		}
        u->energy -= cost;

        int exper = (20*num + max - 1)/ max;
        if(exper > 10) exper = 10;
        
		u->Experience(skill,exper);
    	// Deduct the costs
    	for(c = 0; c < sizeof(ItemDefs[item].mInput)/sizeof(Materials); c++) {
    		if(ItemDefs[item].mInput[c].item == -1) continue;
    		int cost = ItemDefs[item].mInput[c].amt;
    		u->ConsumeShared(ItemDefs[item].mInput[c].item, num*cost);
    	}


	#ifdef FIZZLES
        int mevent = u->MysticEvent();
        switch(mevent) {
            case 4:
            case 3:
            case 2:
            case 1:
                u->Error(AString("Attempts to create an ") + ItemDefs[item].name + ", but the spell fizzles.", order->quiet);
                return 1;
            default:
                break;
        }
	#endif
        
    	u->items.SetNum(item,u->items.GetNum(item) + num);
    	u->Event(AString("Creates ") + ItemString(item,num) + ".");
    	if (num == 0) return 0;
    	return 1;
	}
	// non-Earthsea:
	
    //This got corrupted, and needs to be replaced if ARCADIA_MAGIC is turned off
    return 0;
}

int Game::RunSummonLich(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_SUMMON_LICH);

	int num = ((2 * level * level) + getrandom(100))/100;

	u->items.SetNum(I_LICH,u->items.GetNum(I_LICH) + num);
	u->Event(AString("Summons ") + ItemString(I_LICH,num) + ".");
	if (num == 0) return 0;
	return 1;
}

int Game::RunRaiseUndead(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_RAISE_UNDEAD);

	int num = ((10 * level * level) + getrandom(100))/100;

	u->items.SetNum(I_UNDEAD,u->items.GetNum(I_UNDEAD) + num);
	u->Event(AString("Raises ") + ItemString(I_UNDEAD,num) + ".");
	if (num == 0) return 0;
	return 1;
}

int Game::RunSummonSkeletons(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_SUMMON_SKELETONS);

	int num = ((40 * level * level) + getrandom(100))/100;

	u->items.SetNum(I_SKELETON,u->items.GetNum(I_SKELETON) + num);
	u->Event(AString("Summons ") + ItemString(I_SKELETON,num) + ".");
	if (num == 0) return 0;
	return 1;
}

int Game::RunDragonLore(ARegion *r, Unit *u, int isquiet)
{
	int level = u->GetSkill(S_DRAGON_LORE);

	int num = u->items.GetNum(I_DRAGON);
	if (num >= level) {
		u->Error("Mage may not summon more dragons.", isquiet);
		return 0;
	}

	int chance = level * level * 4;
	if (getrandom(100) < chance) {
		u->items.SetNum(I_DRAGON,num + 1);
		u->Event("Summons a dragon.");
		num = 1;
	} else {
		u->Event("Attempts to summon a dragon, but fails.");
		num = 0;
	}
	if (num == 0) return 0;
	return 1;
}

int Game::RunBirdLore(ARegion *r,Unit *u)
{
	CastIntOrder *order = (CastIntOrder *) u->activecastorder;
	int type = regions.GetRegionArray(r->zloc)->levelType;

	if(type != ARegionArray::LEVEL_SURFACE) {
		AString error = "CAST: Bird Lore may only be cast on the surface of ";
		error += Globals->WORLD_NAME;
		error += ".";
		u->Error(error.Str(), order->quiet);
		return 0;
	}

	if (order->level < 3) {
	    int cost = u->GetCastCost(S_BIRD_LORE,order->extracost);
		if(cost > u->energy) {
		    u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
		    return 0;
		} else u->energy -= cost;
		
		int level = u->GetSkill(S_BIRD_LORE);
		u->Experience(S_BIRD_LORE,5);

		#ifdef FIZZLES
        int mevent = u->MysticEvent();
        int newdir;
        switch(mevent) {
            case 4:
            case 3:
                u->Error("Summoned birds get confused, and visit the wrong region.", order->quiet);
                do{
                    newdir = getrandom(6);
                } while (newdir == order->target);
                order->target = newdir;
                break;
            case 2:
            case 1:
                u->Error("Attempts to send birds to a neighbouring region, but they fail to return.", order->quiet);
                return 1;
            default:
                break;
        }
        #endif


		int dir = order->target;
		int count = 0;
		ARegion *tempReg = r;
		
		while(level > 0) {
    		level--;
    		ARegion *tar = tempReg->neighbors[dir];
    		if (!tar) {
    			if(!count) u->Error("CAST: No such region.", order->quiet);
    			return 0;
    		}
    		count++;
    		tempReg = tar;
    		
    		Farsight *f = new Farsight;
    		f->faction = u->faction;
    		f->level = u->GetSkill(S_BIRD_LORE);
    		tar->farsees.Add(f);
    		u->Event(AString("Sends birds to spy on ") +
    				tar->Print( &regions ) + ".");
		}
		return 1;
	}/* else if(order->level == 4) {
	
	    int cost = u->GetCastCost(S_BIRD_LORE,order->extracost,4); //4 times normal cost
		if(cost > u->energy) {
		    u->Error("CAST: Not enough energy to cast that spell.");
		    return 0;
		} else u->energy -= cost;
		
		u->Experience(S_BIRD_LORE,20);

		#ifdef FIZZLES
        int mevent = u->MysticEvent();
        switch(mevent) {
            case 4:
            case 3:
            case 2:
            case 1:
                u->Error("Attempts to send birds to neighbouring regions, but they fail to return.");
                return 1;
            default:
                break;
        }
	    #endif

	    for(int i=0; i<6; i++) {
	        ARegion *tar = r->neighbors[i];
    		if (!tar) continue;
    		Farsight *f = new Farsight;
    		f->faction = u->faction;
    		f->level = u->GetSkill(S_BIRD_LORE);
    		tar->farsees.Add(f);
	    }
   		u->Event("Sends birds to spy on all neighbouring regions.");
   		return 1;
	}*/

    int level = u->GetSkill(S_BIRD_LORE);
    int currentnum = u->items.GetNum(I_EAGLE);
    

 	int cost = u->GetCastCost(S_BIRD_LORE,order->extracost,10); //cost of 10 creatures !
 	int num = 1;   //if free energy, only summon one at once
 	if(cost > 0) {
 	num = (10*u->energy) / cost;
     	if(num < 1) {
     	    u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
     	    return 0;
        }
    }
 	if( (num + currentnum) > (level-2)*(level-2) ) num = (level-2)*(level-2) - currentnum;
 	if(num < 1) {
 	    u->Error("CAST: Already have the maximum number of eagles.", order->quiet);
 	    return 0;
    }
 	if(order->target > 0 && order->target < num) num = order->target;
 	
 	cost = u->GetCastCost(S_BIRD_LORE,order->extracost,num);
 	u->energy -= cost;
 	
 	int experience = num*4;
 	if(experience > 20) experience = 20;
 	u->Experience(S_BIRD_LORE,experience);
 	
	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
            if(currentnum>0) {
                u->Error(AString("Attempts to summon ") + ItemDefs[I_EAGLE].names + ", but all of those creatures magically disappear.", order->quiet);
                u->items.SetNum(I_EAGLE,0);
                return 1;
            }
            //fall thru
        case 3:
        case 2:
        case 1:
            u->Error(AString("Attempts to cast ") + SkillDefs[S_BIRD_LORE].name + ", but talking parrots appear instead.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif

	u->items.SetNum(I_EAGLE,currentnum + num);
	u->Event(AString("Summons ") + ItemString(I_EAGLE,num));
	return 1;
}

int Game::RunWolfLore(ARegion *r,Unit *u, int isquiet)
{ //in Arcadia, SummonCreatures is called instead unless there is no energy cost.
	if (TerrainDefs[r->type].similar_type != R_MOUNTAIN &&
		TerrainDefs[r->type].similar_type != R_FOREST) {
		u->Error("CAST: Can only summon wolves in mountain and "
				 "forest regions.", isquiet);
		return 0;
	}

	int level = u->GetSkill(S_WOLF_LORE);
	int max = level * level * 4;

	int num = u->items.GetNum(I_WOLF);
	int summon = max - num;
	if (summon > level) summon = level;
	if (summon < 0) summon = 0;
	
    u->Experience(S_WOLF_LORE,summon); //typically 'level' experience.

	u->Event(AString("Casts Wolf Lore, summoning ") +
			ItemString(I_WOLF,summon) + ".");
	u->items.SetNum(I_WOLF,num + summon);
	if (summon == 0) return 0;
	return 1;
}

int Game::RunInvisibility(ARegion *r,Unit *u)
{
	CastUnitsOrder *order = (CastUnitsOrder *) u->activecastorder;
	int level = u->GetSkill(S_INVISIBILITY);
	int max = level * level;
	int energymax = (u->energy * level) / u->GetCastCost(S_INVISIBILITY,order->extracost,1);

	int num = 0;
	forlist (&(order->units)) {
		Unit *tar = r->GetUnitId((UnitId *) elem,u->faction->num);
		if (!tar) continue;
		if (tar->GetAttitude(r,u) < A_FRIENDLY) continue;
		num += tar->GetRealSoldiers();
	}

	if (!num) {
		u->Error("CAST: No valid targets to turn invisible.", order->quiet);
		return 0;
	}
	
	if (num > max) {
		u->Error("CAST: Can't render that many men or creatures invisible.", order->quiet);
	}
	if (num > energymax) {
	    u->Error("CAST: Not enough energy to render that many men or creatures invisible.", order->quiet);
    }
    if(energymax < max) max = energymax; //max is now the smaller of the two limits.

    int backfire = 0;
	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
            u->Error("Attempts to cast invisibility, but the spell backfires, decreasing the stealth of the mage and targets by 2.", order->quiet);
            backfire = 2;
            u->SetFlag(FLAG_VISIB,1);
            break;
        case 3:
            u->Error("Attempts to cast invisibility, but the spell backfires, decreasing the stealth of the targets by 2.", order->quiet);
            backfire = 2;
            break;
        case 2:
        case 1:
            u->Error("Attempts to cast invisibility, but the spell fizzles.", order->quiet);
            backfire = 1; //continue, because we have to count targets to know how much energy to deduct.
            break;
        default:
            break;
    }
	#endif

    int men = 0;
	forlist_reuse(&(order->units)) {
		Unit *tar = r->GetUnitId((UnitId *) elem,u->faction->num);
		if (!tar) continue;
		if (tar->GetAttitude(r,u) < A_FRIENDLY) continue;
		if ( (men + tar->GetSoldiers()) > max ) continue;
		men += tar->GetSoldiers();
		if(!backfire) {
    		tar->SetFlag(FLAG_INVIS,1);
    		tar->Event(AString("Is rendered invisible by ") +
    				*(u->name) + ".");
		} else if(backfire == 2) {
    		tar->SetFlag(FLAG_VISIB,1);
    		tar->Event(AString("Is rendered visible by ") +
    				*(u->name) + ".");
		}
	}

    int cost = u->GetCastCost(S_INVISIBILITY,order->extracost,men);
    cost = (cost + level - 1) / level;
    u->energy -= cost;
    int experience = (20 * men) / (level*level);
    if(experience > 10) experience = 10;
    u->Experience(S_INVISIBILITY, experience);

	u->Event("Casts invisibility.");
	return 1;
}

int Game::RunPhanDemons(ARegion *r,Unit *u)
{
	CastIntOrder *order = (CastIntOrder *) u->activecastorder;
	int level = u->GetSkill(S_CREATE_PHANTASMAL_DEMONS);
	int create,max;

	if (order->level < 3) {
		create = I_IIMP;
		max = level * level * 4;
	} else {
		if (order->level < 5) {
			create = I_IDEMON;
			max = (level - 2) * (level - 2);
		} else {
			create = I_IBALROG;
			max = 1;
		}
	}

	if (order->target > max || order->target <= 0) {
		u->Error("CAST: Can't create that many Phantasmal Demons.", order->quiet);
		return 0;
	}

	u->items.SetNum(create,order->target);
	u->Event("Casts Create Phantasmal Demons.");
	return 1;
}

int Game::RunPhanUndead(ARegion *r,Unit *u)
{
	CastIntOrder *order = (CastIntOrder *) u->activecastorder;
	int level = u->GetSkill(S_CREATE_PHANTASMAL_UNDEAD);
	int create,max;

	if (order->level < 3) {
		create = I_ISKELETON;
		max = level * level * 4;
	} else {
		if (order->level < 5) {
			create = I_IUNDEAD;
			max = (level - 2) * (level - 2);
		} else {
			create = I_ILICH;
			max = 1;
		}
	}

	if (order->target > max || order->target <= 0) {
		u->Error("CAST: Can't create that many Phantasmal Undead.", order->quiet);
		return 0;
	}

	u->items.SetNum(create,order->target);
	u->Event("Casts Create Phantasmal Undead.");
	return 1;
}

int Game::RunPhanBeasts(ARegion *r,Unit *u)
{
	CastIntOrder *order = (CastIntOrder *) u->activecastorder;
	int level = u->GetSkill(S_CREATE_PHANTASMAL_BEASTS);
	int create,max;

	if (order->level < 3) {
		create = I_IWOLF;
		max = level * level * 4;
	} else {
		if (order->level < 5) {
			create = I_IEAGLE;
			max = (level - 2) * (level - 2);
		} else {
			create = I_IDRAGON;
			max = 1;
		}
	}

	if (order->target > max || order->target <= 0) {
		u->Error("CAST: Can't create that many Phantasmal Beasts.", order->quiet);
		return 0;
	}

	u->items.SetNum(create,order->target);
	u->Event("Casts Create Phantasmal Beasts.");
	return 1;
}

int Game::RunPhanCreatures(ARegion *r,Unit *u)
{
	CastMenOrder *order = (CastMenOrder *) u->activecastorder;
	int level = u->GetSkill(S_ILLUSORY_CREATURES);

	if(order->level > level) {
		u->Error("CAST: Insufficient level to create that illusion.", order->quiet);
		return 0;
  	}

	int cost = u->GetCastCost(S_ILLUSORY_CREATURES,order->extracost,10); // this is the cost to summon ten balrogs,
	//or 4 demons, or 40 imps.
  	int change = order->men - u->items.GetNum(order->race);
  	if(!change) {
  	    u->Error("CAST: The mage already has that many of that creature.", order->quiet);
  	    return 1;
  	}
  	
  	cost *= change;
  	if(cost <= 0) cost = 0;
  	else {
  	    switch(order->race) {
  	        case I_IWOLF:
  	        case I_ISKELETON:
  	        case I_IIMP:
  	            cost = (cost+399)/400;
  	            break;
            case I_IEAGLE:
            case I_IDEMON:
            case I_IUNDEAD:
                cost = (cost+39)/40;
                break;
            case I_IGRYFFIN:
            case I_IBALROG:
            case I_ILICH:
                cost = cost/10;
                break;
            case I_IDRAGON:
                cost = cost/5;
                break;
            default:
                u->Error("CAST: That does not seem to be an illusionary creature. This error should not occur, contact your GM.", 0);
                return 1;
  	    }
  	}

  	if(cost > u->energy) {
  	    u->Error("CAST: Not enough energy to create that many illusions.", order->quiet);
  	    return 0;
  	}
  	u->energy -= cost;
  	
	#ifdef FIZZLES
  	int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
            u->Error("Attempts to create illusionary creatures, but the spell backfires, and all his illusory creatures are lost!", order->quiet);
            for(int i=0; i<NITEMS; i++) if(ItemDefs[i].type & IT_ILLUSION) u->items.SetNum(i,0);
            return 0;
        case 3:
            u->Error("Attempts to create illusionary creatures, but the spell backfires, and illusory cockroaches "
                "swarm over him all month long, despite his efforts to get rid of them.", order->quiet);
            u->energy -= 4;
            if(u->energy < 0) u->energy = 0;
            u->Experience(S_ILLUSORY_CREATURES,10);
            return 1;
        case 2:
        case 1:
            u->Error("Attempts to create illusionary creatures, but the spell fizzles, producing only illusory roast dinners.", order->quiet);
            u->Experience(S_ILLUSORY_CREATURES,10);
            return 1;
        default:
        break;
    }
	#endif

  	int experience = 5 * cost;
  	if(experience > 30) experience = 30;
  	u->Experience(S_ILLUSORY_CREATURES, experience);
  	
	u->items.SetNum(order->race,order->men);
	u->Event("Casts Create Illusory Creatures.");
	return 1;
}

int Game::RunPhanEntertainment(ARegion *r,Unit *u)
//castable spell in Arc-III only.
{
    CastOrder *order = (CastOrder *) u->activecastorder;
    int level = u->GetSkill(S_PHANTASMAL_ENTERTAINMENT);
    
    if(!r->town || r->town->TownType() != TOWN_CITY) {
        u->Error("CAST: That spell can only be cast in cities.", order->quiet);
        return 0;
    }

	int cost = u->GetCastCost(S_PHANTASMAL_ENTERTAINMENT,order->extracost);
	if(cost > u->energy) {
	    u->Error("CAST: Not enough energy to cast that spell", order->quiet);
	    return 0;
    }
    u->energy -= cost;

	#ifdef FIZZLES
    int mevent = u->MysticEvent();
//    int numunits = 0;
//    Unit *tar = u;
    
    switch(mevent) {
        case 4:
        case 3:
            u->Error("Attempts to entertain the nobility, and succeeds so well that jealous husbands leave him penniless, "
                "while their wives leave him exhausted!", order->quiet);
            u->energy = 0;
            u->items.SetNum(I_SILVER,0);
            u->Experience(S_PHANTASMAL_ENTERTAINMENT,10);
            u->Experience(S_ENTERTAINMENT,90);
            return 1;
        case 2:
            u->Error("Attempts to entertain the nobility, but they are a humourless bunch, and he is forced to weave "
                "a web of illusions to escape unharmed.", order->quiet);
            u->energy -= cost;
            if(u->energy < 0) u->energy = 0;
            u->Experience(S_PHANTASMAL_ENTERTAINMENT,20);
            u->Experience(S_ILLUSION,40);
            return 1;
        case 1:
            u->Error("Attempts to entertain the nobility, but his spells fizzle, and he gets covered in rotten tomato juice.", order->quiet);
            u->Experience(S_PHANTASMAL_ENTERTAINMENT,10);
            //unfortunately there is not yet any fruit-dodging skill to practise.
            return 1;
        default:
        break;
    }
	#endif

    u->items.SetNum(I_SILVER,u->items.GetNum(I_SILVER) + 100 * level);
    u->Experience(S_PHANTASMAL_ENTERTAINMENT,10);
    u->Event(AString("Casts Phantasmal Entertainment, raising ") + (100*level) + " silver from the nobility.");
    return 1;
}

int Game::RunEarthLore(ARegion *r,Unit *u)
{
	int level = u->GetSkill(S_EARTH_LORE);

	CastRegionOrder *order = (CastRegionOrder *)u->activecastorder;
	int size = order->level - 1; // 3 if large, 0 otherwise
	int penalty = 0;
	if(size>2) penalty = 2; //2 is max penalty
	AString temp = "Casts Earth Lore";
	ARegion *tar = r;
	int val;

	RangeType *range = FindRange(SkillDefs[S_EARTH_LORE].range);
	if(range != NULL) {
		tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
		val = GetRegionInRange(r, tar, u, S_EARTH_LORE,order->quiet,penalty);
		if(!val) return 0;
		temp += " on ";
		temp += tar->ShortPrint(&regions);
	}

	if(TerrainDefs[tar->type].similar_type == R_OCEAN) {
	    u->Error("CAST: Cannot cast Earth Lore at sea.", order->quiet);
	    return 0;
	}

	int cost = u->GetCastCost(S_EARTH_LORE,order->extracost, 1);
	int largecost = u->GetCastCost(S_EARTH_LORE,order->extracost, 4);
	
	if(cost > u->energy) {   //if large and not enough energy for large casting, we default to small, so don't exit
	    u->Error("CAST: Not enough energy to cast that spell", order->quiet);
	    return 0;
    }

	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
        case 3:
        case 2:
        case 1:
            u->Error("Attempts to cast earth lore, but the spell fizzles.", order->quiet);
            u->Experience(S_EARTH_LORE,10);
            if(!size) u->energy -= cost;
            else u->energy -= largecost;
            return 1;
        default:
            break;
    }
	#endif

	if (level > tar->earthlore) r->earthlore = level;
	int amt = tar->Wages() * level * 4;

    int count = 0;
	if (size && Globals->ARCADIA_MAGIC) {
	    if(level < 4) {
	        u->Error("CAST: Not high enough level for large casting.", order->quiet);
	    } else if(largecost <= u->energy) {
    	    for (int i = 0; i<6; i++) {
    	        if(tar->neighbors[i] && TerrainDefs[tar->neighbors[i]->type].similar_type != R_OCEAN) {
    	            count++;
    	            if (level > tar->neighbors[i]->earthlore) tar->neighbors[i]->earthlore = level;
    	            amt += tar->neighbors[i]->Wages() * level * 4;
                }
            }
            if(range == NULL) temp += AString(" on ") + tar->ShortPrint(&regions);
            temp += AString(" and ") + count + " neighbouring regions";
        } else {
            u->Error("CAST: Not enough energy for large casting.", order->quiet);
        }
	}
	temp += AString(", raising ") + amt + " silver.";

	if(count) {
        u->energy -= largecost;
        u->Experience(S_EARTH_LORE,20);
    } else {
        u->energy -= cost;
        u->Experience(S_EARTH_LORE,10);
    }
	
	u->items.SetNum(I_SILVER,u->items.GetNum(I_SILVER) + amt);
	u->Event(temp);
	
	return 1;
}

int Game::RunClearSkies(ARegion *r, Unit *u)
{
	ARegion *tar = r;
	AString temp = "Casts Clear Skies";
	int val;

	CastRegionOrder *order = (CastRegionOrder *)u->activecastorder;
	int size = order->level - 1;
	int penalty = 0;
	if(size) penalty = 2;

	RangeType *range = FindRange(SkillDefs[S_CLEAR_SKIES].range);
	if(range != NULL) {
		tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
		val = GetRegionInRange(r, tar, u, S_CLEAR_SKIES, order->quiet, penalty);
		if(!val) return 0;
		temp += " on ";
		temp += tar->ShortPrint(&regions);
	}

	int level = u->GetSkill(S_CLEAR_SKIES);

    if(Globals->ARCADIA_MAGIC) {
        if(size && level < 4) {
            u->Error("CAST: Not high enough level for large casting.", order->quiet);
            return 0;
        }
        if(penalty < 1) penalty = 1;
     	int cost = u->GetCastCost(S_CLEAR_SKIES,order->extracost,penalty*penalty); //*4 if large
     	if(u->energy < cost) {
     	    u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
     	    return 0;
     	}
        u->energy -= cost;
     	
     	u->Experience(S_CLEAR_SKIES,10*penalty);
     	
     	#ifdef FIZZLES
        int mevent = u->MysticEvent();
        int xx;
        int yy;
        switch(mevent) {
            case 4:
                u->Error("Attempts to cast clearskies, but a terrible blizzard appears instead.", order->quiet);
                tar->weather = W_BLIZZARD;
                return 1;
            case 3:
            case 2:
                xx = getrandom(regions.GetRegionArray(1)->x);
                do {
                    yy = getrandom(regions.GetRegionArray(1)->y);
                } while ( (xx+yy)%2 );
                tar = regions.GetRegion(xx,yy,1);
                if(!tar) {
                    u->Error("Attempts to cast clearskies, but the spell fizzles.", order->quiet);
                    return 1;
                }
                u->Error(AString("Attempts to cast clearskies, but the spell is misaimed, hitting ") + tar->ShortPrint(&regions), order->quiet);
                break;
            case 1:
                u->Error("Attempts to cast clearskies, but the spell fizzles, and the mage gets rained on.", order->quiet);
                return 1;
            default:
                break;
        }
        #endif
    }

	if (level > tar->clearskies) tar->clearskies = level;

	if (size && Globals->ARCADIA_MAGIC) {
	    int count = 0;
	    for (int i = 0; i<6; i++) {
	        if(tar->neighbors[i]) {
	            count++;
	            if (level > tar->neighbors[i]->clearskies) tar->neighbors[i]->clearskies = level;
            }
        }
        temp += AString(" and ") + count + " neighbouring regions";
	}	

	temp += ".";	
	u->Event(temp);
	return 1;
}

int Game::RunWeatherLore(ARegion *r, Unit *u)
{
	ARegion *tar;
	int val, i;

	CastRegionOrder *order = (CastRegionOrder *)u->activecastorder;

	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_WEATHER_LORE, order->quiet, 0);
	if(!val) return 0;

	int level = u->GetSkill(S_WEATHER_LORE);
	int months = 3;
	if(level >= 5) months = 12;
	else if (level >= 3) months = 6;

	AString temp = "Casts Weather Lore on ";
	temp += tar->ShortPrint(&regions);
	temp += ". It will be ";
	int weather, futuremonth;
	for(i = 0; i <= months; i++) {
		futuremonth = (month + i)%12;
		weather=regions.GetWeather(tar, futuremonth);
		temp += SeasonNames[weather];
		temp += " in ";
		temp += MonthNames[futuremonth];
		if(i < (months-1))
			temp += ", ";
		else if(i == (months-1))
			temp += " and ";
		else
			temp += ".";
	}
	u->Event(temp);
	return 1;
}

int Game::RunFarsight(ARegion *r,Unit *u)
{
	ARegion *tar;
	int val;

	CastRegionOrder *order = (CastRegionOrder *)u->activecastorder;

	int size = order->level - 1;
	int penalty = 0;
	if(size) penalty = 2;

	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_FARSIGHT, order->quiet, penalty);
	if(!val) return 0;

	int level = u->GetSkill(S_FARSIGHT);

	if(Globals->ARCADIA_MAGIC) {
        if(size && level < 4) {
            u->Error("CAST: Not high enough level for large casting.", order->quiet);
            return 0;
        }
        
        if(penalty<1) penalty=1;
     	int cost = u->GetCastCost(S_FARSIGHT, order->extracost, penalty*penalty); //*4 if large
     	if(u->energy < cost) {
     	    u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
     	    return 0;
     	}
        u->energy -= cost;
     	
     	u->Experience(S_FARSIGHT,10*penalty);
     	
	    #ifdef FIZZLES
        int mevent = u->MysticEvent();
        int xx;
        int yy;
        switch(mevent) {
            case 4:
                u->Error("Attempts to cast farsight, but it is reflected, displaying the mage's region for all in the "
                    "target region to see.", order->quiet);
                    //not programmed
                return 1;
            case 3:
            case 2:
                xx = getrandom(regions.GetRegionArray(1)->x);
                do {
                    yy = getrandom(regions.GetRegionArray(1)->y);
                } while ( (xx+yy)%2 );
                tar = regions.GetRegion(xx,yy,1);
                if(!tar) {
                    u->Error("Attempts to cast farsight, but the spell fizzles.", order->quiet);
                    return 1;
                }
                u->Error(AString("Attempts to cast farsight, but the spell is misaimed, hitting ") + tar->ShortPrint(&regions), order->quiet);
                break;
            case 1:
                u->Error("Attempts to cast farsight, but the spell fizzles, and the mage gets two black eyes.", order->quiet);
                return 1;
            default:
                break;
        }
	    #endif
	}
	
	Farsight *f = new Farsight;
	f->faction = u->faction;
	f->level = u->GetSkill(S_FARSIGHT);
	f->unit = u;
	tar->farsees.Add(f);
	AString temp = "Casts Farsight on ";
	temp += tar->ShortPrint(&regions);

	if (size && Globals->ARCADIA_MAGIC) {
	    int count = 0;
	    for (int i = 0; i<6; i++) {
	        if(tar->neighbors[i]) {
		        Farsight *f = new Farsight;
		        f->faction = u->faction;
		        f->level = u->GetSkill(S_FARSIGHT);
		        f->unit = u;
		        tar->neighbors[i]->farsees.Add(f);
                count++;
            }
        }
        temp += AString(" and ") + count + " neighbouring regions";
	}	
	
	temp += ".";
	u->Event(temp);
	return 1;
}

int Game::RunDetectGates(ARegion *r,Object *o,Unit *u)
{
	CastOrder *order = (CastOrder *)u->activecastorder;
	int distance = 1;
	if(order->level == 5) distance = 2;

	int level = u->GetSkill(S_GATE_LORE);

	if (level == 1) {
		u->Error("CAST: Insufficient level to DETECT gates.",order->quiet);
		return 0;
	}
	
	if(Globals->ARCADIA_MAGIC) {
        
     	int cost = u->GetCastCost(S_GATE_LORE, order->extracost, distance*distance);
     	if(u->energy < cost) {
     	    u->Error("CAST: Not enough energy to cast that spell.",order->quiet);
     	    return 0;
     	}
        u->energy -= cost;
     	u->Experience(S_GATE_LORE,6*distance); //low experience, most experience comes from gating around.

     	#ifdef FIZZLES
        int mevent = u->MysticEvent();
        switch(mevent) {
            case 4:
            case 3:
            case 2:
            case 1:
                u->Error("Attempts to cast detect gates, but the spell fizzles, leaving the mage with a headache.");
                return 1;
            default:
                break;
        }
	    #endif
	}


	u->Event("Casts Gate Lore, detecting nearby Gates:");
	int found = 0;
	if ((r->gate) && (!r->gateopen)) {
	    u->Event(AString("Identified local gate number ") + (r->gate) +
		" in " + r->ShortPrint(&regions) + ".");
	}
	for (int i=0; i<NDIRS; i++) {
		ARegion *tar = r->neighbors[i];
		if (tar) {
			if (tar->gate) {
				if(Globals->DETECT_GATE_NUMBERS) {
					u->Event(tar->Print(&regions) +
					    " contains Gate " + tar->gate +
					    ".");
				} else {
					u->Event(tar->Print(&regions) +
					    " contains a Gate.");
				}
				found = 1;
			}
		}
	}
	if(distance == 2) {
	    //check regions distance two away
	    //first, clear markers on those regions.
        for(int i=0; i<NDIRS; i++) {
            ARegion *adj = r->neighbors[i];
            if(adj) {
                for(int j=0; j<NDIRS; j++) {
                    ARegion *tar = adj->neighbors[j];
                    if(tar) tar->marker = 0;
                }
            }
        }
        //second, mark adjacent regions.
        for(int i=0; i<NDIRS; i++) {
            ARegion *adj = r->neighbors[i];
            if(adj) adj->marker = 1;
        }
        //third, cycle through dist 2 regions with marker = 0.
        for(int i=0; i<NDIRS; i++) {
            ARegion *adj = r->neighbors[i];
            if(adj) {
                for(int j=0; j<NDIRS; j++) {
                    ARegion *tar = adj->neighbors[j];
                    if(tar->marker == 0) {
                        tar->marker = 1;
            			if (tar->gate) {
            				if(Globals->DETECT_GATE_NUMBERS) {
            					u->Event(tar->Print(&regions) +
            					    " contains Gate " + tar->gate +
            					    ".");
            				} else {
            					u->Event(tar->Print(&regions) +
            					    " contains a Gate.");
            				}
            				found = 1;
            			}
                    }
                }
            }
        }
    }
	
	if (!found)
		u->Event("There are no nearby Gates.");
	return 1;
}

int Game::RunTeleport(ARegion *r,Object *o,Unit *u)
{
	ARegion *tar;
	int val;

	CastRegionOrder *order = (CastRegionOrder *)u->teleportorders;

	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_TELEPORTATION, order->quiet, 0);
	if(!val) return 0;

	int level = u->GetSkill(S_TELEPORTATION);
	int maxweight = level * 50;
	if(Globals->ARCADIA_MAGIC) maxweight *= level;	

	if (u->Weight() > maxweight) {
		u->Error("CAST: Can't carry that much when teleporting.", order->quiet);
		return 0;
	}

	// Presume they had to open the portal to see if target is ocean
	if (TerrainDefs[tar->type].similar_type == R_OCEAN) {
		u->Error(AString("CAST: ") + tar->Print(&regions) +
		    " is an ocean.",order->quiet);
		return 1;
	}

    if(Globals->ARCADIA_MAGIC) {
       
     	int cost = u->GetCastCost(S_TELEPORTATION,order->extracost,(u->Weight()/level)); //This is 50 times the cost.
     	cost /= 50;
     	if(u->energy < cost) {
     	    u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
     	    return 0;
     	}
        u->energy -= cost;
     	
     	int experience = (40 * u->Weight()) / (50*level*level);
     	if(experience > 15) experience = 15;
     	
     	u->Experience(S_TELEPORTATION,experience);
     	
     	#ifdef FIZZLES
        int mevent = u->MysticEvent();
        int xx;
        int yy;
        switch(mevent) {
            case 4:
            case 3:
                do {
                    tar = 0;
                    xx = getrandom(regions.GetRegionArray(1)->x);
                    yy = getrandom(regions.GetRegionArray(1)->y);
                    if(!(xx+yy)%2) tar = regions.GetRegion(xx,yy,1);
                } while ( tar == 0 || TerrainDefs[tar->type].similar_type == R_OCEAN );
                
                if(!tar) {
                    u->Error("Attempts to cast teleport, but the spell fizzles.", order->quiet);
                    return 1;
                }
                u->Error(AString("Attempts to teleport, but the spell is misaimed, teleporting the mage to ") + tar->ShortPrint(&regions), order->quiet);
                break;
            case 2:
            case 1:
                u->Error("Attempts to cast teleport, but the spell fizzles.", order->quiet);
                return 1;
            default:
                break;
        }
        #endif
    }

	u->Event(AString("Teleports to ") + tar->Print(&regions) + ".");
	u->MoveUnit(tar->GetDummy());
	return 1;
}

int Game::RunGateJump(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_GATE_LORE);
	int nexgate = 0;

	TeleportOrder *order = u->teleportorders;
	
	if( !level ) {
		u->Error( "CAST: Unit doesn't have that skill.", order->quiet );
		return 0;
	}

	if (order->gate != -1 && level < 3) {
		u->Error("CAST: Unit Doesn't know Gate Lore at that level.", order->quiet);
		return 0;
	}

	nexgate = Globals->NEXUS_GATE_OUT &&
		(TerrainDefs[r->type].similar_type == R_NEXUS);
	if (!r->gate && !nexgate) {
		u->Error("CAST: There is no gate in that region.", order->quiet);
		return 0;
	}

	if (!r->gateopen) {
	    u->Error("CAST: Gate not open at this time of year.", order->quiet);
	    return 0;
	}

	int maxweight = 10;
	if (order->gate != -1) level -= 2; //subtract two levels if targetting a particular gate
	if(Globals->ARCADIA_MAGIC) {
	    maxweight = (18*u->GetEnergy())/(u->GetCastCost(S_GATE_LORE,order->extracost,1));
	    maxweight *= level*level;   //can carry 3*level^2 weight per energy.
	} else {
    	switch (level) {
    		case 1:
    			maxweight = 15;
    			break;
    		case 2:
    			maxweight = 100;
    			break;
    		case 3:
    		case 4:
    		case 5:
    			maxweight = 1000;
    			break;
    	}
	}
	int weight = u->Weight();

	forlist (&(order->units)) {
		Unit *taru = r->GetUnitId((UnitId *) elem,u->faction->num);
		if (taru && taru != u) weight += taru->Weight();
	}

	if (weight > maxweight) {
		u->Error( "CAST: That mage cannot carry that much weight "
		    "through a Gate.", order->quiet);
		return 0;
	}
	
	if(Globals->ARCADIA_MAGIC) {
    	int cost = u->GetCastCost(S_GATE_LORE,order->extracost,weight);
    	cost = (cost + 18*level*level - 1) / (18*level*level);
    	u->energy -= cost;
    	int experience = weight / (level*level*level);
    	if(experience > 10) experience = 10;
    	u->Experience(S_GATE_LORE, experience); //max experience takes 10*level*level*level weight.

    	#ifdef FIZZLES
        int mevent = u->MysticEvent();
//        int xx;
//        int yy;
        switch(mevent) {
            case 4:
/*                u->Error("Attempts to cast gate jump, but the mage emerges at a random location.");
                ARegion *tar = 0;
                do {
                    tar = 0;
                    xx = getrandom(regions.GetRegionArray(r->zloc)->x);
                    yy = getrandom(regions.GetRegionArray(1)->y);
                    if(!(xx+yy)%2) tar = regions.GetRegion(xx,yy,1);
                } while ( tar == 0 || TerrainDefs[tar->type].similar_type == R_OCEAN );
*/                
            case 3:
            case 2:
                if(order->gate != -1) {
                    order->gate = -1;
                    u->Error("Attempts to jump to a particular gate, but the spell backfires, sending the mage to a random gate.", order->quiet);
                    break;
                }
            case 1:
                u->Error("Attempts to cast gate jump, but the spell fizzles.", order->quiet);
                return 1;
            default:
                break;
        }
        #endif
        
    }

	ARegion *tar;
	if (order->gate == -1) {
		int good = 0;
		int gatenum = getrandom(regions.numberofgates);
		tar = regions.FindGate(gatenum+1);

		if(tar && tar->zloc == r->zloc) good = 1;
		if(tar && nexgate && tar->zloc == ARegionArray::LEVEL_SURFACE)
			good = 1;

		while( !good ) {
			gatenum = getrandom(regions.numberofgates);
			tar = regions.FindGate(gatenum+1);
			if(tar && tar->zloc == r->zloc) good = 1;
			if(tar && nexgate && tar->zloc == ARegionArray::LEVEL_SURFACE)
				good = 1;
		}

		u->Event("Casts Random Gate Jump.");
	} else {
		if (order->gate < 1 || order->gate > regions.numberofgates) {
			u->Error("CAST: No such target gate.", order->quiet);
			return 0;
		}

		tar = regions.FindGate(order->gate);
		if (!tar) {
			u->Error("CAST: No such target gate.", order->quiet);
			return 0;
		}
		if(!tar->gateopen) {
		    u->Error("CAST: Target gate not open at this time of year.", order->quiet);
		    return 0;
		}

		u->Event("Casts Gate Jump.");
	}

	int comma = 0;
	AString unitlist; {
		forlist(&(order->units)) {
			Location *loc = r->GetLocation((UnitId *) elem,u->faction->num);
			if (loc) {
				/* Don't do the casting unit yet */
				if (loc->unit == u) {
					delete loc;
					continue;
				}

				if (loc->unit->GetAttitude(r,u) < A_ALLY) {
					u->Error("CAST: Unit is not allied.", order->quiet);
				} else {
					if (comma) {
						unitlist += AString(", ") + AString(loc->unit->num);
					} else {
						unitlist += AString(loc->unit->num);
						comma = 1;
					}

					loc->unit->Event(AString("Is teleported through a ") +
							"Gate to " + tar->Print(&regions) + " by " +
							*u->name + ".");
					loc->unit->MoveUnit( tar->GetDummy() );
					if (loc->unit != u) loc->unit->ClearCastOrder();
				}
				delete loc;
			} else {
				u->Error("CAST: No such unit.", order->quiet);
			}
		}
	}

	u->Event(AString("Jumps through a Gate to ") +
			tar->Print( &regions ) + ".");
	if (comma) {
		u->Event(unitlist + " follow through the Gate.");
	}
	u->MoveUnit( tar->GetDummy() );
	return 1;
}

int Game::RunPortalLore(ARegion *r,Object *o,Unit *u)
{
	int level = u->GetSkill(S_PORTAL_LORE);
	TeleportOrder *order = u->teleportorders;

	if (!level) {
		u->Error("CAST: Doesn't know Portal Lore.", order->quiet);
		return 0;
	}

	if (!u->items.GetNum(I_PORTAL)) {
		u->Error("CAST: Unit doesn't have a Portal.", order->quiet);
		return 0;
	}

	int maxweight = 50 * level;
	int weight = 0;
	forlist (&(order->units)) {
		Unit *taru = r->GetUnitId((UnitId *) elem,u->faction->num);
		if (taru) weight += taru->Weight();
	}

    if (weight > maxweight) {
		u->Error("CAST: That mage cannot teleport that much weight through a "
				"Portal.", order->quiet);
		return 0;
	}

	Location *tar = regions.FindUnit(order->gate);
	if (!tar) {
		u->Error("CAST: No such target mage.", order->quiet);
		return 0;
	}

	if (tar->unit->faction->GetAttitude(u->faction->num) < A_FRIENDLY) {
		u->Error("CAST: Target mage is not friendly.", order->quiet);
		return 0;
	}

	if (tar->unit->type != U_MAGE) {
		u->Error("CAST: Target is not a mage.", order->quiet);
		return 0;
	}

	if (!tar->unit->items.GetNum(I_PORTAL)) {
		u->Error("CAST: Target does not have a Portal.", order->quiet);
		return 0;
	}

	if (!GetRegionInRange(r, tar->region, u, S_PORTAL_LORE, order->quiet, 0)) return 0;

	u->Event("Casts Portal Jump.");

	{
		forlist(&(order->units)) {
			Location *loc = r->GetLocation((UnitId *) elem,u->faction->num);
			if (loc) {
				if (loc->unit->GetAttitude(r,u) < A_ALLY) {
					u->Error("CAST: Unit is not allied.", order->quiet);
				} else {
					loc->unit->Event(AString("Is teleported to ") +
							tar->region->Print( &regions ) +
							" by " + *u->name + ".");
					loc->unit->MoveUnit( tar->obj );
					if (loc->unit != u) loc->unit->ClearCastOrder();
				}
				delete loc;
			} else {
				u->Error("CAST: No such unit.", order->quiet);
			}
		}
	}

	delete tar;
	return 1;
}

void Game::RunTeleportOrders()
{
	int val = 1;
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			int foundone = 1;
			while (foundone) {
				foundone = 0;
				forlist(&o->units) {
					Unit * u = (Unit *) elem;
					if (u->teleportorders) {
						foundone = 1;
						switch (u->teleportorders->spell) {
							case S_GATE_LORE:
								val = RunGateJump(r,o,u);
								break;
							case S_TELEPORTATION:
								val = RunTeleport(r,o,u);
								break;
							case S_PORTAL_LORE:
								val = RunPortalLore(r,o,u);
								break;
						}
						if (val)
							u->Practice(u->teleportorders->spell);
						delete u->teleportorders;
						u->teleportorders = 0;
						break;
					}
				}
			}
		}
	}
}

int Game::RunBlizzard(ARegion *r, Unit *u)
// Earthsea spell
{
	ARegion *tar;
	int val;

	CastRegionOrder *order = (CastRegionOrder *)u->activecastorder;

	int size = order->level - 1;
	int penalty = 0;
	if(size) penalty = 2;
	int level = u->GetSkill(S_BLIZZARD);
	
	if(size && level < 4) {
	    u->Error("CAST: Not high enough level for large casting.", order->quiet);
	    return 0;
	}
	
	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_BLIZZARD, order->quiet, penalty);
	if(!val) return 0;

	if(penalty<1) penalty = 1;
 	int cost = u->GetCastCost(S_BLIZZARD,order->extracost, penalty*penalty); // 4 times cost for large casting.
	if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
	}
	u->energy -= cost;
	
	#ifdef FIZZLES
	int mevent = u->MysticEvent();
	int xx;
	int yy;
    switch(mevent) {
        case 4:
        case 3:
            u->Error("Attempts to cast a blizzard, but the spell is reflected back on the caster!", order->quiet);
            tar = r;
            //get experience normally!
            break;
        case 2:
            xx = getrandom(regions.GetRegionArray(1)->x);
            do {
                yy = getrandom(regions.GetRegionArray(1)->y);
            } while ( (xx+yy)%2 );
            tar = regions.GetRegion(xx,yy,1);
            if(!tar) {
                u->Error("Attempts to cast a blizzard, but the spell fizzles.", order->quiet);
                u->Experience(S_BLIZZARD,10);
                return 1;
            }
            u->Error(AString("Attempts to cast a blizzard, but the spell is misaimed, hitting ") + tar->ShortPrint(&regions), order->quiet);
            //get experience normally!
            break;
        case 1:
            u->Error("Attempts to cast a blizzard, but the spell fizzles, and the mage is seen shivering all month long.", order->quiet);
            u->Experience(S_BLIZZARD,10);
            return 1;
        default:
            break;
    }
	#endif
	

	AString temp = "Casts Blizzard on ";
	temp += tar->ShortPrint(&regions);
	if (size) {
	    int count = 0;
	    for (int i = 0; i<6; i++) {
	        if(tar->neighbors[i]) {
                tar->neighbors[i]->weather = W_BLIZZARD;
                count++;
            }
        }
        temp += AString(" and ") + count + " neighbouring regions";
	}	
	temp += ".";
	tar->weather = W_BLIZZARD;
	u->Event(temp);
	if(size) u->Experience(S_BLIZZARD,20);
	else u->Experience(S_BLIZZARD,10);
	return 1;
}

int Game::RunSeaward(ARegion *r, Unit *u)
// Arcadia spell
{
	ARegion *tar;
	int val;

	CastRegionOrder *order = (CastRegionOrder *)u->activecastorder;

	int size = order->level - 1;
	int penalty = 0;
	if(size) penalty = 2;
	int level = u->GetSkill(S_SEAWARD);
	int time = level/2 + level%2;

	if(size && level < 4) {
	    u->Error("CAST: Not high enough level for large casting.", order->quiet);
	    return 0;
	}
	
	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_SEAWARD, order->quiet, penalty);

	// should have something here for reducing range of large spells.
	if(!val) return 0;

 	int cost = u->GetCastCost(S_SEAWARD,order->extracost); // cost of normal size
 	if(size) cost = u->GetCastCost(S_SEAWARD,order->extracost,3); //note, only 3, not usual 4, because it is highly unlikely all six neighbouring regions can be affected.
	if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
	}
	//subtraction of energy done at the end.

	if(!size && (tar->willsink == 0 || tar->willsink >= time)) {
	    u->Error("CAST: That region is not sinking.", order->quiet); //check so it doesn't have a chance to fizzle in single cast.
	    return 0;
    }
	
//	int destroyed = 0;
	int permanent = 0;
	#ifdef FIZZLES
	int mevent = u->MysticEvent();
    switch(mevent) {
        case 4:
            u->Error("Attempts to ward off the sea, but the spell backfires, attracting the ocean to "
                "the target's neighbouring regions as well.", order->quiet);
    	    for (int i = 0; i<6; i++) {
    	        if(tar->neighbors[i]) {
                    if(TerrainDefs[tar->neighbors[i]->type].similar_type == R_OCEAN) continue; //do not try to sink ocean regions!
                    if(tar->neighbors[i]->willsink != 1) {
                        tar->neighbors[i]->willsink = 2;
                    }
                }
            }
            u->Experience(S_SEAWARD,20);
            return 1;
        case 3:
            if(tar->willsink < 1) break;
            u->Error("Mystic energies cause a sea ward to have incredible effectiveness.", order->quiet);
            permanent = 1;
            break;
        case 2:
        //this case could cause problems if casting large on regions with nothing sinking.
            forlist(&r->objects) {
                Object *obj = (Object *) elem;
                if(obj->type != O_DUMMY && obj->inner == -1) {
                //Move units out
    		        Object *dest = r->GetDummy();
    		        forlist(&obj->units) {
    					Unit *u = (Unit *) elem;
    					u->MoveUnit(dest);
    			    }
    			//destroy object
                    r->objects.Remove(obj);
                    delete obj;
                    destroyed++;
                }
            }
            if(destroyed) u->Error(AString("While warding off the sea, high winds destroy ") + destroyed + " structures in the region.", order->quiet);
            break;
        case 1:
            u->Error("Attempts to ward off the sea, but the spell fizzles, and a freak storm of fish land on the mage.", order->quiet);
            u->Experience(S_SEAWARD,10);
            u->energy -= cost;
            if(!(ItemDefs[I_FISH].flags & ItemType::DISABLED)) u->items.SetNum(I_FISH,u->items.GetNum(I_FISH) + 20);
            
            if(u->energy < 0) u->energy = 0;
            return 1;
        default:
            break;
    }
	#endif


	int done = 0;
	AString temp = "Wards the sea away from ";
	temp += tar->ShortPrint(&regions);
	if (size && Globals->ARCADIA_MAGIC) {
	    for (int i = 0; i<6; i++) {
	        if(tar->neighbors[i]) {
                if(tar->neighbors[i]->willsink > 0 && tar->neighbors[i]->willsink <= time) {
                    tar->neighbors[i]->willsink = time + 1;
                    done = 1;
                }
            }
        }
        temp += " and all neighbouring regions";
	}	
	temp += AString(", holding the sea at bay for ") + time + " month";
	if(time>1) temp += "s";
	temp += ".";

	if(tar->willsink > 0 && tar->willsink <= time) {
	    tar->willsink = time+1;
	    done = 1;
    }
    if(tar->willsink && permanent) {
        tar->willsink = 0; //never sinks
        done = 1;
    }

	if(!done) {
	    u->Error("CAST: That region is not sinking.", order->quiet);
        return 0;
    }

    u->energy -= cost;
    if(size) u->Experience(S_SEAWARD,30); //highish experience because unusual spell to cast.
    else u->Experience(S_SEAWARD,15);

	if(!permanent) u->Event(temp);
	return 1;
}

int Game::RunDiversion(ARegion *r, Unit *u)
// Arcadia spell
// Complicated 3 case code. I cannot see a simpler solution however.
{
// If rivers disabled, return! (else will crash below)
    if((HexsideDefs[H_RIVER].flags & HexsideType::DISABLED) || !Globals->HEXSIDE_TERRAIN) return 0;

	ARegion *tar;
	int val;

	CastRegionOrder *order = (CastRegionOrder *)u->activecastorder;

	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_DIVERSION, order->quiet, 0);
	if(!val) return 0;
	
 	int cost = u->GetCastCost(S_DIVERSION,order->extracost);
	if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
	}
	u->energy -= cost;	
	u->Experience(S_DIVERSION, 15);
	
	int rivers = 0;
	#ifdef FIZZLES
	int mevent = u->MysticEvent();
	int xx;
	int yy;
	int tries = 0;
    switch(mevent) {
        case 4:
        case 3:
            xx = getrandom(regions.GetRegionArray(1)->x);
            do {
                tar = 0;
                yy = getrandom(regions.GetRegionArray(1)->y);
                if(!(xx+yy)%2) tar = regions.GetRegion(xx,yy,1);
                if(tar) {
                    for(int i=0; i<6; i++) {
                        Hexside *h = tar->hexside[i];
                        if(h && h->type == H_RIVER) rivers++;
                    }
                }
            } while ( (tar == 0 || rivers == 0 ) && tries++ < 1000);
            if(!tar) {
                u->Error("Attempts to cast diversion, but the spell fizzles.", order->quiet);
                return 1;
            }
            u->Error(AString("Attempts to cast diversion, but the spell is misaimed, hitting ") + tar->ShortPrint(&regions), order->quiet);
            break;
        case 2:
        case 1:
            u->Error("Attempts to cast diversion, but the spell fizzles, dragging the mage into a nearby well.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif

// If no river in region, return (do not create a river "ring")
    rivers = 0;
    for(int i=0; i<6; i++) {
        if(tar->hexside[i]->type == H_RIVER) rivers++;
    }
    if(!rivers) {
        u->Error("CAST: No rivers in that region.", order->quiet);
        return 0;
    }
    if(rivers == 6) {
        u->Error("CAST: Nowhere to divert river to.", order->quiet);
        return 0;
    }

// Currently this code can leave hanging rivers if cast on a 
// river junction. Must deal with this somehow.  -Fixed?

    int inflows[6];
    for (int i=0; i<6; i++) inflows[i] = 0;
    
    for (int k=0; k<6; k++) {
        if(!tar->neighbors[k]) continue;
        if(tar->neighbors[k]->hexside[(k+2)%6]->type == H_RIVER) {
            inflows[k] += 1;
        }
        if(tar->neighbors[k]->hexside[(k+4)%6]->type == H_RIVER) {
            inflows[(k+5)%6] += 1;
        }
    }
    int total = 0;
    for (int j=0; j<6; j++) {
        if(inflows[j]) total++;
    }

    //only continue if inflows < 3. Otherwise, divide river piecemeal.
    if(total < 3) {
	
    	for (int i=0; i<6; i++) {
    	    if(tar->hexside[i]->type == H_RIVER) {
                tar->hexside[i]->type = H_DUMMY;
//                tar->hexside[i]->bridge = 0;
                continue;
            }
            
            if(tar->neighbors[i] && TerrainDefs[tar->neighbors[i]->type].similar_type == R_OCEAN) continue;
            //no river, non coastal. Create river.
            tar->hexside[i]->type = H_RIVER;
    	}
	} else {
	    // 3+ inflows
	    int nonseg = 0;  //counting number of strips of non-beach/river (could be up to 3)
	    for(int k=0; k<6; k++) {
	        if (tar->hexside[k]->type == H_RIVER) continue;
	        if (tar->neighbors[k] && (TerrainDefs[tar->neighbors[k]->type].similar_type == R_OCEAN) ) continue;
	        //not a beach or river.
	        
	        if (tar->hexside[(k+5)%6]->type == H_RIVER) {
                nonseg++;
                continue;
            }
            if (tar->neighbors[(k+5)%6] && (TerrainDefs[tar->neighbors[(k+5)%6]->type].similar_type == R_OCEAN) ) {
                if (tar->hexside[(k+4)%6]->type == H_RIVER) nonseg++;
                else break; //three continuous non-rivers -> break
            }
	    }

        if((nonseg > 1) && (total == 4)) {
            //two non-river segments, 4+ inflows:
            for(int k=0; k<6; k++) {
                if(tar->hexside[k]->type == H_RIVER) {
                    tar->hexside[k]->type = H_DUMMY;
//                    tar->hexside[k]->bridge = 0;
                    continue;
                }
                if(tar->neighbors[k] && (TerrainDefs[tar->neighbors[k]->type].similar_type == R_OCEAN)) continue;
                //non river, non coastal. Create river.
                tar->hexside[k]->type = H_RIVER;            
            }
        } else {
            //one non-river segment, 3+ inflows, or two non-river segments and 5+ inflows:
    
    	    int foundnon = 0; //found non-river segment
    	    int foundriv = 0; //found river segment
    	    int first = 1;
    	    for(int i=0; i<12; i++) {
    	        int j=i%6;
    	        int river = 0;
    	        if(tar->hexside[j]->type == H_RIVER) river = 1;
    	        
    	        //we have to find a river segment first
    	        if(!foundriv) {
    	            if(!river) continue;
    	            else foundriv = 1;
                }
                //then we have to find the non-river segment
                if(!foundnon) {
                    if(river) continue;
                    else foundnon = 1;
                }
                
                //then we treat the non-river segment(s)
                if(!river && first) {
                    //if coastal, cycle
                    if(tar->neighbors[j]) {
                        if(TerrainDefs[tar->neighbors[j]->type].similar_type == R_OCEAN) continue;
                    }
                    //otherwise form river
                    tar->hexside[j]->type = H_RIVER;
                //then we treat the river segment(s)
                } else {
                    //we are now on the first or later edge of the first river segment
                    if(first) {
                        first = 0;
                        tar->hexside[j]->type = H_DUMMY;
//                        tar->hexside[j]->bridge = 0;
                    } else {
                    //if not first, break if there is an inflow - but make sure on a river.
                        if(inflows[(i-1)%6]) {
                            tar->hexside[(i%6)]->type = H_RIVER;
                            break;
                        }
                        else {
                            tar->hexside[j]->type = H_DUMMY;
//                            tar->hexside[j]->bridge = 0;
                        }
                    }
                }
    	    }
	    
	    }
	
	
	}

   	AString temp = "Casts Diversion on ";
   	temp += tar->ShortPrint(&regions);	
	u->Event(temp);
	SpecialError(tar, AString("Rivers in ") + tar->ShortPrint(&regions) + " change their course.", u->faction);
	tar->Event("Local rivers were diverted from their courses last month.");
	return 1;
}

int Game::RunSummonCreatures(ARegion *r, Unit *u, int skill, int item, int max)
//if no max, set max to -1.
{
    CastIntOrder *order = (CastIntOrder *) u->activecastorder;
    
    int level = u->GetSkill(skill);  //wolf, imp, demon, undead (eagles handled in bird lore)
    int currentnum = u->items.GetNum(item);

    if(item == I_WOLF && TerrainDefs[r->type].similar_type != R_MOUNTAIN &&
		TerrainDefs[r->type].similar_type != R_FOREST) {
		u->Error("CAST: Can only summon wolves in mountain and "
				 "forest regions.", order->quiet);
		return 0;
	}

 	int cost = u->GetCastCost(skill,order->extracost); //cost of 10 creatures !
 	int num = (10*u->energy) / cost;
 	if(num < 1) {
 	    u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
 	    return 0;
    }
  	if(max > 0 && (num + currentnum) > max*level*level ) num = max*level*level - currentnum;
 	if(num < 1) {
 	    u->Error("CAST: Already have the maximum number of that creature.", order->quiet);
 	    return 0;
    }
 	if(order->target > 0 && order->target < num) num = order->target;
 	
 	cost = u->GetCastCost(skill,order->extracost,num); //this gives 10*cost
 	cost = (cost+9)/10;
 	u->energy -= cost;
 	
 	int experience = num/2+cost;
 	if(experience > 20) experience = 20;
 	u->Experience(skill,experience);
 	
	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    int newitem;
    switch(mevent) {
        case 4:
            if(currentnum>0) {
                u->Error(AString("Attempts to summon ") + ItemDefs[item].names + ", but all of those creatures magically disappear.", order->quiet);
                u->items.SetNum(item,0);
                return 1;
            }
            //fall thru
        case 3:
            if(item == I_DEMON) newitem = I_IMP;
            else if(item == I_UNDEAD) newitem = I_SKELETON;
            else break;
            u->Error(AString("Attempts to summon ") + ItemDefs[item].names + ", but something goes wrong and " + ItemDefs[newitem].names + " appear instead.", order->quiet);
            item = newitem;
            currentnum = u->items.GetNum(item);
            break;
        case 2:
        case 1:
            u->Error(AString("Attempts to cast ") + SkillDefs[skill].name + ", but the spell fizzles.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif

	u->items.SetNum(item,currentnum + num);
	u->Event(AString("Summons ") + num + " " + ItemDefs[item].names);

	return 1;
}

int Game::RunSummonHigherCreature(ARegion *r, Unit *u, int skill, int item)
//Arcadia code for summoning balrogs, gryffins, liches
{
    CastOrder *order = (CastOrder *) u->activecastorder;

	int level = u->GetSkill(skill);  //balrog, lich, S_GRYFFIN_LORE
	int maxnum = (level+1)/2;

	int num = u->items.GetNum(item);
	if(item == I_LICH) num = 0; //special case, no limit for liches
	if (num >= maxnum) {
		u->Error(AString("Mage may not control more ") + ItemDefs[item].names + ".", order->quiet);
		return 0;
	}

 	int cost = u->GetCastCost(skill,order->extracost, 1, 2*num);
	if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
	}
	
	if(!cost) {
	    //free spell to cast; eg gryffin lore in Arc IV. Let's make a 10% summon chance per level, since it's free.
        u->Experience(skill,5);
	    if(getrandom(100) >= 10*(level - 2*num)) {
            u->Event("Attempts to summon a gryffin, but fails.");
	        return 1;
        }
    } else {
        u->energy -= cost;
    	u->Experience(skill,24); //this skill would usually only be cast 3 times, and is energy intensive.
	}
	
	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    int newitem;
        
    switch(mevent) {
        case 4:
            if(num>0) {
                u->Error(AString("Attempts to summon a ") + ItemDefs[item].name + ", but all of those creatures magically disappear.", order->quiet);
                u->items.SetNum(item,0);
                return 1;
            }
            //fall thru
        case 3:
            if(item == I_BALROG) newitem = I_DEMON;
            else if(item == I_GRYFFIN) newitem = I_EAGLE;
            else if(item == I_LICH) newitem = I_UNDEAD;
            u->Error(AString("Attempts to summon a ") + ItemDefs[item].name + ", but something goes wrong and a " + ItemDefs[newitem].name + " appears instead.", order->quiet);
            item = newitem;
            num = u->items.GetNum(item);
            break;
        case 2:
        case 1:
            u->Error(AString("Attempts to cast ") + SkillDefs[skill].name + ", but the spell fizzles.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif

	u->items.SetNum(item,u->items.GetNum(item) + 1); //can't use (num+1) because of lich special case.
	u->Event(AString("Summons a ") + ItemDefs[item].name);

	return 1;
}


int Game::RunFog(ARegion *r, Unit *u)
// Arcadia spell
{
	CastIntOrder *order = (CastIntOrder *)u->activecastorder;

/* Fog must follow the mage around, so must be a property of the mage, 
not of the region - at least until after the sail and move phases. */
    int large = 0;
    if(order->level > 1) large = 1;
    int dir = order->target;
    
    int skill = u->GetSkill(S_FOG);
    if(large && skill < 4) {
        u->Error("CAST: Not high enough skill level to cast large fog", order->quiet);
        return 0;
    }

    int marker = 1;  //marks unit as foggy
    if(dir < -1) dir = -1;
    if(large) marker += 2 + dir; //if marker > 1, then large. If marker == 2, then centred on mage, else on direction 'marker-3'

 	int cost = u->GetCastCost(S_FOG,order->extracost); // cost of normal sized fog
 	if(large) cost = u->GetCastCost(S_FOG,order->extracost,4);
	if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
	}
	u->energy -= cost;
	if(large) u->Experience(S_FOG,16);
	else u->Experience(S_FOG,10);
	
    Unit *tar = u;
	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    int numunits = 0;
    
    switch(mevent) {
        case 4:
            u->Error("Attempts to cast fog, but the spell backfires, "
                "clearing the region of all bad weather and making the mage a local hero.", order->quiet);
            if(r->clearskies == 0) r->clearskies = 1;
            return 1;
        case 3:
            forlist(&r->objects) {
                Object *o = (Object *) elem;
                forlist(&o->units) {
                    Unit *unit = (Unit *) elem;
                    if(unit != u) {
                        if(getrandom(++numunits)) tar = unit;
                    }
                }
            }
            if(tar == u) break;
            u->Error(AString("Attempts to cast fog, but the spell follows ")
                + *(tar->name) + " instead.", order->quiet);
            break;
        case 2:
        case 1:
            u->Error("Attempts to cast fog, but the spell fizzles.", order->quiet);
            return 1;
        default:
            break;
    }
	#endif
	

    tar->foggy = marker;
    
	return 1;
}

int Game::RunSummonMen(ARegion *r, Unit *u)
// Arcadia spell
{
	int level = u->GetSkill(S_SUMMON_MEN);
	CastMenOrder *order = (CastMenOrder *)u->activecastorder;

//checks: race must be a man and not a leader:
    if(!(ItemDefs[order->race].type & IT_MAN) || (ItemDefs[order->race].flags & ItemType::DISABLED)) {
        u->Error(AString("CAST: Incorrect race"), order->quiet);
        return 0;
    }
    if(ItemDefs[order->race].type & IT_LEADER) {
        u->Error(AString("CAST: Cannot summon leaders"), order->quiet);
        return 0;
    }

// work out cost of men
    int cost = ItemDefs[order->race].baseprice; //This is usually $50.
    int amt = u->GetSharedNum(I_SILVER);

	if(amt < cost) {
		u->Error(AString("CAST: Doesn't have sufficient silver to summon that."), order->quiet);
		return 0;
	}

	int num = amt/cost;
	if (num > order->men) num = order->men;
	if (num > 10*level*level) num = 10*level*level;

	int energycost = u->GetCastCost(S_SUMMON_MEN,order->extracost); //cost per 10 men.
	if( (num * energycost) / 10 > u->energy) num = (u->energy * 10) / energycost;

    if(!num) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
    }

	Unit *tar = r->GetUnitId((UnitId *) order->units.First(),u->faction->num);
	if (!tar) {
	    u->Error("CAST: Invalid unit number.", order->quiet);
	    return 0;
    }
	if (tar->faction != u->faction) {
	    u->Error("CAST: Target unit is not from your faction.", order->quiet);
	    return 0;
	}
	if(tar->type != U_NORMAL) {
        u->Error("CAST: Can only summon men into normal units.", order->quiet);
        return 0;
    }

	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    //cases 4&3: 
    //case 2: change to a different race
    int i = 0;
    int numraces = 0;
    int race = -1;
    
    switch(mevent) {
        case 4:
            u->Error("Attempts to cast summon men, but he summons women instead. The lure of temptation is too great, and the mage ends the month exhausted.", order->quiet);
            u->energy = 0;
            u->Experience(S_SUMMON_MEN,10);
            return 1;
        case 3:
        case 2:
            for(i=0; i<NITEMS; i++) {
                if(ItemDefs[i].flags & ItemType::DISABLED) continue;
                if(!(ItemDefs[i].type & IT_MAN)) continue;
                if(ItemDefs[i].type & IT_LEADER) continue;
                if(i == order->race) continue;
                if(!getrandom(++numraces)) race = i;
            }
            if(race == -1) break;
            u->Error("Attempts to cast summon men, but the spell backfires, summoning the wrong race.", order->quiet);
            order->race = race;
            break;
        case 1:
            u->Error("Attempts to cast summon men, but the spell fizzles, producing only a frog.", order->quiet);
            u->Experience(S_SUMMON_MEN,10);
            u->energy -= cost;
            if(u->energy < 0) u->energy = 0;
            return 1;
        default:
            break;
    }
	#endif


    u->ConsumeShared(I_SILVER, num*cost);
	tar->items.SetNum(order->race,tar->items.GetNum(order->race) + num);

	u->Event(AString("Summons ") + num + " " + ItemDefs[order->race].names
      + " into " + *tar->name + ".");
      
    u->energy -= (num * energycost) / 10;
    if((num * energycost)%10) u->energy -= 1;
    
    int experience = (2 * num) / level;
    if(experience > 10) experience = 10;
    
    u->Experience(S_SUMMON_MEN,experience);

	return 1;
}

int Game::RunTransmutation(ARegion *r, Unit *u)
// Arcadia spell
{
	int level = u->GetSkill(S_TRANSMUTATION);
	CastChangeOrder *order = (CastChangeOrder *) u->activecastorder;
	int fromitem = order->fromitem;
	int toitem = order->toitem;
   	if(level<5 && fromitem<0) {
	    u->Error("CAST: Must specify item to transmute.", order->quiet);
	    return 0;
	}
	if(!IsPrimary(toitem)) {
	    if(level<2) {
	        u->Error("CAST: To item is not a primary item.", order->quiet);
	        return 0;
	    }
	    if(IsPrimary(fromitem)) {
	        u->Error("CAST: Cannot transmute a primary item into a manufactured item.", order->quiet);
	        return 0;
	    }
	}
	
	if( (fromitem >= 0) && ItemDefs[toitem].baseprice >= 2*ItemDefs[fromitem].baseprice) {
	        u->Error("CAST: To item is too valuable to be transmuted from that.", order->quiet);
	        return 0;
	}
	
	int ratio = 1;
	if(!(ItemDefs[toitem].type & IT_NORMAL) ) {
	    if(level<5) {
	        u->Error("CAST: Can only create normal items.", order->quiet);
	        return 0;
	    }
	    if(toitem == I_MITHRIL && !(ItemDefs[I_IRON].flags & ItemType::DISABLED)) 
            fromitem = I_IRON;
	    else if(toitem == I_IRONWOOD && !(ItemDefs[I_WOOD].flags & ItemType::DISABLED)) 
            fromitem = I_WOOD;
	    else if(toitem == I_YEW && !(ItemDefs[I_WOOD].flags & ItemType::DISABLED)) 
            fromitem = I_WOOD;
	    else if(toitem == I_PEARL && !(ItemDefs[I_FISH].flags & ItemType::DISABLED)) 
            fromitem = I_FISH;
	    else if(toitem == I_ROOTSTONE && !(ItemDefs[I_STONE].flags & ItemType::DISABLED)) 
            fromitem = I_STONE;
	    else if(toitem == I_MUSHROOM && !(ItemDefs[I_HERBS].flags & ItemType::DISABLED)) 
            fromitem = I_HERBS;
	    else if(toitem == I_FLOATER && !(ItemDefs[I_FUR].flags & ItemType::DISABLED)) 
            fromitem = I_FUR;
	    else if(toitem == I_WHORSE && !(ItemDefs[I_HORSE].flags & ItemType::DISABLED)) 
            fromitem = I_HORSE;
        else {
            u->Error("CAST: Cannot create that item.", order->quiet);
            return 0;
        }
	    ratio = 2;
	}
	
	if(fromitem<0) {
	    u->Error("CAST: Must specify item to transmute.", order->quiet);
	    return 0;
	}	
	
/*	if(!(SkillDefs[S_TRANSFIGURE].flags & SkillType::DISABLED)) { //only have this limit if transfigure is also in play
    	if(toitem == I_LIVESTOCK || toitem == I_HORSE || toitem == I_CAMEL ||
          fromitem == I_LIVESTOCK || fromitem == I_HORSE || 
          fromitem == I_CAMEL || fromitem == I_WHORSE) {
    	    u->Error("CAST: Cannot transmute living items.");
    	    return 0;
	}*/

	
	if(ItemDefs[toitem].type & IT_MAN || ItemDefs[fromitem].type & IT_MAN ) {
	    u->Error("CAST: Cannot transmute men.", order->quiet);
	    return 0;
	}
	
	if(ItemDefs[toitem].type & IT_MONSTER || ItemDefs[fromitem].type & IT_MONSTER ) {
	    u->Error("CAST: Cannot transmute creatures.", order->quiet);
	    return 0;
	}
	
	if(toitem == I_PLATEARMOR) {
	//should generalise this to anything needing more than 1 input to make.
	    u->Error("CAST: Cannot transmute into items which require more than one input to manufacture.", order->quiet);
	    return 0;
	}
	
	int num = u->GetSharedNum(fromitem);
	if(!num) {
        u->Error("CAST: Not enough of that item to transmute.", order->quiet);
        return 0;
	}
	if(num > 4*level*level) num = 4*level*level; // max items to transmute
	num /= ratio; // advanced items produce half as many.
 	
 	int cost = u->GetCastCost(S_TRANSMUTATION,order->extracost, ratio*ratio); // cost of 8 items transmuted
	// ratio: advanced resources cost 4 times more energy!
	// but only 3 times more resources!
	if(ratio == 2) ratio = 3;

	if(u->energy < 0) u->energy = 0; //status check to prevent silly negatives.
	if(num > (8 * u->energy) / cost) num = (8 * u->energy) / cost;
	if(!num) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
	}
	
//    int i = 0;
//    int numitems = 0;
//    int item = -1;
    int nooutput = 0;
    int noinput = 0;
	#ifdef FIZZLES
    int mevent = u->MysticEvent();
    //cases 4&3: 
    //case 2: change to a different resource
    
    switch(mevent) {
        case 4:
            noinput = 1;
            u->Error("Transmutation spell produces resources from nothing!", order->quiet);
            break;
        case 3:
            nooutput = 1;
            u->Error("Attempts to cast transmutation, but it fizzles, and the resources vanish. ", order->quiet);
            break;
        case 2:
            for(i=0; i<NITEMS; i++) {
                if(ItemDefs[i].flags & ItemType::DISABLED) continue;
                if(!(ItemDefs[i].type & IT_NORMAL)) continue;
                if(ItemDefs[i].type & IT_MAN) continue;
                if(ItemDefs[i].type & IT_MOUNT) continue;
                if(i == I_PLATEARMOR || i == I_LIVESTOCK) continue;
                if(IsPrimary(i) && !IsPrimary(fromitem)) continue;
                if(i == toitem) continue;
                if(!getrandom(++numitems)) item = i;
            }
            if(item == -1) break;
            u->Error("Attempts to cast transmutation, but the spell backfires, producing the wrong output.", order->quiet);
            toitem = item;
            break;
        case 1:
            u->Error("Attempts to cast transmutation, but the spell fizzles, and the mage's hat disappears.", order->quiet);
            if(u->describe) *u->describe = AString("Hatless. ") + *u->describe;  // crossing my fingers this will not overwrite data.
            else u->describe = new AString("Hatless");
            u->Experience(S_TRANSMUTATION,8);
            u->energy -= cost;
            if(u->energy < 0) u->energy = 0;
            return 1;
        default:
            break;
    }
	#endif
	
	u->energy -= (num * cost) / 8;
	if((num*cost)%8) u->energy -= 1; //cost gets rounded up.
    if(u->energy < 0) u->energy = 0;

    if(!noinput) u->ConsumeShared(fromitem, num*ratio);
	if(!nooutput) u->items.SetNum(toitem,u->items.GetNum(toitem) + num);

    if(!noinput && !nooutput) u->Event(AString("Transmutes ") + num*ratio + " " + ItemDefs[fromitem].names
      + " to " + num + " " + ItemDefs[toitem].names); //should rewrite this using the itemstring function.
      
    int experience = (ratio * num * 20) / (4*level*level);
    if(experience>10) experience = 10;
    u->Experience(S_TRANSMUTATION, experience);
    
	return 1;
}

int Game::RunModification(ARegion *r, Unit *u)
{
	int level = u->GetSkill(S_MODIFICATION);
	CastModifyOrder *order = (CastModifyOrder *) u->activecastorder;
	int fromitem = order->fromitem;
	int toitem = order->toitem;

	ARegion *tar;
	int val;
	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_MODIFICATION, order->quiet, 0);
	if(!val) return 0;

	int increase = 1;
	if(order->level == 3) increase = 0;
	// increase = 1 to go up, 0 to decrease
	
	if(!increase && level<3) {
	    u->Error("CAST: Not high enough level to decrease resources.", order->quiet);
	    return 0;	
	}

		// check the level is high enough to do what is needed
	int advanced = 0;
    if(!(ItemDefs[toitem].type & IT_NORMAL)) {
        if(increase) {
            if(level<4) {
                u->Error("CAST: Not high enough level to increase advanced resources.", order->quiet);
                return 0;
            }
        } else { //ie if decreasing
            if(level<5) {
                u->Error("CAST: Not high enough level to decrease advanced resources.", order->quiet);
                return 0;
            }
        }
        advanced = 1;
    }
	
    if(!(ItemDefs[fromitem].type & IT_NORMAL)) {
        if(level<5) {
            u->Error("CAST: Not high enough level to modify advanced resources.", order->quiet);
            return 0;
        }
        advanced = 1;
    }
 
   	if(advanced) level -= 2;

	if(toitem == I_SILVER || fromitem == I_SILVER) {
	    u->Error("CAST: Cannot modify wages.", order->quiet);
	    return 0;	
	}

	if(toitem == fromitem) {
	    u->Error("CAST: Item increasing and item decreasing are the same.", order->quiet);
	    return 0;
	}

	//check the region has the resources!
	Production *toprod = 0;
	Production *fromprod = 0;
	
	forlist(&tar->products) {
	    Production *p = (Production *) elem;
	    if(p->itemtype == toitem) toprod = p;
	    if(p->itemtype == fromitem) fromprod = p;
    }

    if(!toprod || !fromprod) {
        u->Error("CAST: Product not present in region.", order->quiet);
        return 0;
	}

	int toamount = 0;
	int fromamount = 0;
	for(unsigned int i=0; i<(sizeof(TerrainDefs[tar->type].prods)/sizeof(Product)); i++) {
	    if(TerrainDefs[tar->type].prods[i].product == toitem) toamount = TerrainDefs[tar->type].prods[i].amount;
	    if(TerrainDefs[tar->type].prods[i].product == fromitem) fromamount = TerrainDefs[tar->type].prods[i].amount;
	}
	//food items (livestock, grain, fish) may be present in the region but not listed in the terrain table. Set them from
	//the economy table.
	if(!toamount && (ItemDefs[toitem].type & IT_FOOD) ) toamount = TerrainDefs[tar->type].economy;
	if(!fromamount && (ItemDefs[fromitem].type & IT_FOOD) ) fromamount = TerrainDefs[tar->type].economy;

	if(!toamount || !fromamount) {
	//This should never occur.
	    u->Error("CAST: Product does not naturally occur in target region type.", order->quiet);
	    return 0;
    }
	
	int cost = u->GetCastCost(S_MODIFICATION,order->extracost); //cost per resource changed up/down.
	if(advanced) cost = u->GetCastCost(S_MODIFICATION,order->extracost,4); //4 times cost.
    if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
    }
    
    #ifdef FIZZLES
    int mevent = u->MysticEvent();
    //cases 4&3: add or remove one of the mentioned resource
    //case 2: change a different resource
    switch(mevent) {
        case 4:
        case 3:
        case 2:
        case 1:
            u->Error("Attempts to cast modification, but the spell fizzles.", order->quiet);
            u->Experience(S_MODIFICATION,10);
            u->energy -= 4*cost;
            if(u->energy < 0) u->energy = 0;
            return 1;
        default:
            break;
    }
    #endif

    int change;
    int decrease;

	if(increase) {
	    change = toamount + toamount * level / 2 - toprod->baseamount;
		if(change > level * 6) change = level*6;
		if(change <= 0) {
		    u->Error("CAST: Mage is not skilled enough to increase the quantity of that resource", order->quiet);
		    return 0;
        }
        if(change*cost > u->energy) change = u->energy / cost;
        
		decrease = (change * 5 + level + 1 ) / (level + 4); //ie change*5/(l+4) rounded up
		if(decrease >= fromprod->baseamount) {
		    decrease = fromprod->baseamount - 1;
		    change = decrease*(level+2) / 3;
		}
        u->energy -= change * cost;
		toprod->amount += change;
		toprod->baseamount += change;
		fromprod->amount -= decrease;
		fromprod->baseamount -= decrease;
		
        u->Event(AString("Casts modification on ") + tar->ShortPrint(&regions) + " increasing the amount of " + ItemDefs[toitem].name + " by " + change);
        SpecialError(tar, AString("Resources present in ") + tar->ShortPrint(&regions) + " are magically altered.", u->faction);
	}
	else {
	    decrease = toprod->baseamount - 1;
		if(decrease > level * 6) decrease = level*6;
		if(decrease*cost > u->energy) decrease = u->energy / cost;
		
		change = (decrease * 3) / level; //ie decrease*3/(l) rounded up
		if(change + fromprod->baseamount > fromamount + fromamount * level / 2) {
		    change = fromamount + fromamount * level / 2 - fromprod->baseamount;
		    decrease = change * level / 3;
		}
		u->energy -= decrease * cost;
		toprod->amount -= decrease;
		toprod->baseamount -= decrease;
		fromprod->amount += change;
		fromprod->baseamount += change;

        u->Event(AString("Casts modification on ") + tar->ShortPrint(&regions) + " decreasing the amount of " + ItemDefs[toitem].name + " by " + decrease);		
        SpecialError(tar, AString("Resources present in ") + tar->ShortPrint(&regions) + " are magically altered.", u->faction);
	}
	
	//max 10 experience, but you have to change at least 50% of max capable of to get it.
	int experience = 60;
 	if(increase) experience *= change;
	else experience *= decrease;
	experience /= level * 6;
	if(experience > 30) experience = 30;
	if(!advanced) experience /= 3;
	u->Experience(S_MODIFICATION,experience);
	
    return 1;
}

int Game::RunRejuvenation(ARegion *r, Unit *u)
{
//this had better be a surface only spell, or wierd things will happen!

	int level = u->GetSkill(S_REJUVENATION);
	CastModifyOrder *order = (CastModifyOrder *) u->activecastorder;
	int regtype = order->toitem;

	ARegion *tar;
	int val;
	tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
	val = GetRegionInRange(r, tar, u, S_REJUVENATION, order->quiet, 0);
	if(!val) return 0;

	if(TerrainDefs[regtype].similar_type == TerrainDefs[tar->type].similar_type) {
	    u->Error("CAST: Region is already of that type.", order->quiet);
	    return 0;	
	}

	if(TerrainDefs[tar->type].similar_type == R_OCEAN && level < 3) {
	    u->Error("CAST: Not high enough level to rejuvenate oceans or lakes.", order->quiet);
	    return 0;
    }

	if(TerrainDefs[regtype].similar_type == R_OCEAN && level < 5) {
	    u->Error("CAST: Not high enough level to create oceans or lakes.", order->quiet);
	    return 0;
    }

    int cost = u->GetCastCost(S_REJUVENATION,order->extracost);
    if(TerrainDefs[regtype].similar_type == R_OCEAN) cost = u->GetCastCost(S_REJUVENATION,order->extracost, 3, 4);
    else if(TerrainDefs[tar->type].similar_type == R_OCEAN) cost = u->GetCastCost(S_REJUVENATION,order->extracost, 2, 2);
    if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
    }
    u->energy -= cost;

    int large = 0;
    #ifdef FIZZLES
    int mevent = u->MysticEvent();
    int xx;
    int yy;
    int tries = 0;
    //case 4: reflects to random type, large
    //case 3: hits random location
    //case 2: reflects to random type
    //case 1: fizzles
    switch(mevent) {
        case 4:
            u->Error("Attempts to cast rejuvenation, but the spell backfires, rejuvenating nearby regions.", order->quiet);
            large = 1;
            tar = r;
            do {
                regtype = getrandom(R_CAVERN);
            } while((TerrainDefs[regtype].similar_type == TerrainDefs[tar->type].similar_type) && tries++ < 20);
            //get experience normally, plus some additional compensation!
            u->Experience(S_REJUVENATION,20);
            break;
        case 3:
            xx = getrandom(regions.GetRegionArray(1)->x);
            do {
                yy = getrandom(regions.GetRegionArray(1)->y);
            } while ( (xx+yy)%2 );
            tar = regions.GetRegion(xx,yy,1);
            if(!tar) {
                u->Error("Attempts to cast rejuvenation, but the spell fizzles.", order->quiet);
                u->Experience(S_REJUVENATION,10);
                return 1;
            }
            u->Error(AString("Attempts to cast rejuvenation, but the spell is misaimed, rejuvenating ") + tar->ShortPrint(&regions), order->quiet);
            //get experience normally!
            break;
        case 2:
            u->Error("Attempts to cast rejuvenation, but the spell backfires, rejuvenating the local region.", order->quiet);
            tar = r;
            do {
                regtype = getrandom(R_CAVERN);
            } while((TerrainDefs[regtype].similar_type == TerrainDefs[tar->type].similar_type) && tries++ < 20);
            //get experience normally!
            break;
        case 1:
            u->Error("Attempts to cast rejuvenation, but the spell fizzles.", order->quiet);
            u->Experience(S_REJUVENATION,10);
            return 1;
        default:
            break;
    }
    #endif
    
    
// if creating oceans or lakes, set tosink and quit
	if(TerrainDefs[regtype].similar_type == R_OCEAN) {
	    u->Experience(S_REJUVENATION, 30);
	    if(tar->willsink != 1) {
            tar->willsink = 2;
	        u->Event(AString("Casts rejuvenation. ") + tar->ShortPrint(&regions) + " will sink below the waves next month.");
	    } else {
	        u->Event(AString("Casts rejuvenation. ") + tar->ShortPrint(&regions) + " has will sink below the waves.");
	    }

	    if(large)
	    for(int i=0; i<6; i++) {
	         ARegion *tar2 = tar->neighbors[i];
	         if(!tar2) continue;
	         if(tar2->willsink != 1) tar2->willsink = 2; //poor sod who cast this and gets his seven region sunk! Maybe check if mage is on ship in ocean ... and do something equally horrible
	    }

	    return 1;
    }

    int redopop = 0;

// if converting from ocean to land, adjust beaches and ships    
    if(TerrainDefs[tar->type].similar_type == R_OCEAN) {
        tar->OceanToLand();
        u->Experience(S_REJUVENATION, 20);
        redopop = 1;
    } else u->Experience(S_REJUVENATION, 10);

    SpecialError(tar, tar->ShortPrint(&regions) + " is magically changed to " + TerrainDefs[regtype].name + ".", u->faction);
    tar->Event(AString("Is magically changed from ") + TerrainDefs[tar->type].name);
    tar->type = regtype;
    u->Event(AString("Casts rejuvenation on ") + tar->ShortPrint(&regions));

    if(!redopop) {
        forlist((&tar->products)) {
            Production * p = ((Production *) elem);
            if(p->itemtype != I_SILVER) {
                tar->products.Remove(p);
                delete p;
            }
        }
        tar->SetupProds();

        //this assumes NOT using the new economy system.
        int oldwages = tar->maxwages;
        int mw = TerrainDefs[regtype].wages;
        if(Globals->RANDOM_ECONOMY) mw += getrandom(3);
        tar->maxwages = mw;
        tar->wages += mw - oldwages;
        tar->UpdateEditRegion();
    } else {
    //redo the economy
        if(tar->town) delete tar->town;
        tar->town = NULL;
    
        tar->products.DeleteAll();
        tar->SetupProds();
                    
        tar->markets.DeleteAll();
    
        tar->SetupEditRegion(0);
    	tar->UpdateEditRegion();
	}
	// region event!
	if(!large) return 1;
	
	for(int i=0; i<6; i++) {
	    ARegion *tar2 = tar->neighbors[i];
	    if(!tar2) continue;

        if(TerrainDefs[tar2->type].similar_type == R_OCEAN) {
        //redo the economy
            tar2->OceanToLand();
            SpecialError(tar2, tar2->ShortPrint(&regions) + " is magically changed to " + TerrainDefs[regtype].name + ".");
            tar2->Event(AString("Is magically changed from ") + TerrainDefs[tar->type].name);
            tar2->type = regtype;
            if(tar2->town) delete tar->town;
            tar2->town = NULL;
        
            tar2->products.DeleteAll();
            tar2->SetupProds();
                        
            tar2->markets.DeleteAll();
        
            tar2->SetupEditRegion(0);
        	tar2->UpdateEditRegion();
            
            
        } else {
            tar2->type = regtype;
            forlist((&tar2->products)) {
                Production * p = ((Production *) elem);
                if(p->itemtype != I_SILVER) {
                    tar2->products.Remove(p);
                    delete p;
                }
            }
            tar2->SetupProds();

            //this assumes NOT using the new economy system.
            int oldwages = tar2->maxwages;
            int mw = TerrainDefs[regtype].wages;
            if(Globals->RANDOM_ECONOMY) mw += getrandom(3);
            tar2->maxwages = mw;
            tar2->wages += mw - oldwages;
            tar2->UpdateEditRegion();
        }
	}	
	return 1;
}

int Game::RunSpiritOfDead(ARegion *r, Unit *u)
{
	int level = u->GetSkill(S_SPIRIT_OF_DEAD);
	CastUnitsOrder *order = (CastUnitsOrder *) u->activecastorder;
	
	Unit *tar;
	if(order->units.First()) {
    	tar = r->GetUnitId((UnitId *) order->units.First(),u->faction->num);
    	if (!tar) {
    	    u->Error("CAST: Cannot find that unit.", order->quiet);
    	    return 0;
    	}
	} else {
	    // list shades in the region.
	    int cost = (u->GetCastCost(S_SPIRIT_OF_DEAD,order->extracost)+4)/5;
	    if(cost > u->energy) {
	         u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
	         return 0;
	    }
	    u->energy -= cost;
	    
	    
        #ifdef FIZZLES
	    if(u->MysticEvent()) {
	         u->Error("Attempts to search for shades, but the spell fizzles.", order->quiet);
	         u->Experience(S_SPIRIT_OF_DEAD,2);
	         return 0;
	    }
        #endif
	    u->Event("The following shades are present in this region:");
	    int shades = 0;
	    forlist(&r->objects) {
	        Object *obj = (Object *) elem;
	        forlist(&obj->units) {
	            Unit *un = (Unit *) elem;
	            if(un->dead) {
	                shades++;
	                u->Event(*un->name);
	            }
            }
        }
        if(!shades) u->Event("none.");
        u->Experience(S_SPIRIT_OF_DEAD,10);
        return 1;
	}
	// we have a target. Make sure it satisfies all conditions
	if(!tar->dead) {
	    u->Error("CAST: That unit is not dead.", order->quiet);
	    return 0;
	}
	
	if(!tar->faction->num == ghostfaction) {
	    u->Error("CAST: That unit appears to be dead, but not a ghost. Please contact your GM.", 0);
	    return 1;
	}
	
	if(!tar->type == U_MAGE) {
	    u->Error("CAST: That unit appears to be dead, but not a mage. Please contact your GM.", 0);
	    return 1;
	}	

	if((tar->dead != 1) && (tar->dead != u->faction->num)) {
	//This mage is loyal to another faction.
	    u->Error("CAST: That mage is still loyal to another.", order->quiet);
	    return 0;	
	}

	int cost = u->GetCastCost(S_SPIRIT_OF_DEAD,order->extracost);	
    if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
    }
    u->energy -= cost;
    
    #ifdef FIZZLES
    int mevent = u->MysticEvent();
    Faction *gf = 0;
    
    switch(mevent) {
        case 4:
            u->Error("Attempts to summon a spirit of the dead, but the spell backfires, sucking the mage "
                        "into the spirit world.", order->quiet);
            u->MoveUnit(r->GetDummy());
            u->dead = u->faction->num;
            gf = GetFaction(&factions, ghostfaction);
            if(!gf) {
                u->MoveUnit(0);
                r->hell.Add(u);
                return 0;
            }
            u->faction = gf;
            u->Experience(S_SPIRIT_OF_DEAD,180);
            return 0;
        case 3: //could give spirit mage to another faction - have to find another faction & check for too many mages.
        case 2:
            u->Error("Attempts to summon a spirit of the dead, but the spell backfires, sucking the mage's "
                        "energy into the spirit world.", order->quiet);
            u->energy = 0;
            u->Experience(S_SPIRIT_OF_DEAD,10);
            return 1;
        case 1:
            u->Error("Attempts to summon a spirit of the dead, but the spell fizzles. The mage spends the month hearing tortured whispers in his head.", order->quiet);
            u->Experience(S_SPIRIT_OF_DEAD,10);
            return 1;
        default:
            break;
    }
    #endif

	if(getrandom(6) >= level) {
	    u->Event(AString("Summons the spirit of ") + *tar->name + " who refuses to join your faction.");
	    u->Experience(S_SPIRIT_OF_DEAD,20);
	    return 1;
	}
	
	if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
		if (CountMages(u->faction) >= AllowedMages(u->faction)) {
			u->Error("CAST: Faction has too many mages.", order->quiet);
			u->Event(AString("Summons the spirit of ") + *tar->name + " who is unable to join your faction due to lack of magic faction points.");
			u->Experience(S_SPIRIT_OF_DEAD,20);
			return 1;
		}
	}
	
	tar->resurrects++;
	tar->faction = u->faction;
	tar->energy = tar->MaxEnergy()/5;
	tar->dead = 0;
	
	u->Event(AString("Summons the spirit of ") + *tar->name + " who joins your faction.");
	u->Experience(S_SPIRIT_OF_DEAD,40);
	return 1;
}

int Game::RunHypnosis(ARegion *r, Unit *u)
{
	int level = u->GetSkill(S_HYPNOSIS);
	CastHypnosisOrder *order = (CastHypnosisOrder *) u->activecastorder;

	if(level < order->level) {
	    u->Error("CAST: Insufficient level for that month order.", order->quiet);
	    return 0;
    }

    if(!u->energy) {
        u->Error("CAST: Insufficient energy to cast that spell.", order->quiet);
        return 0;
    }

    int cost = u->GetCastCost(S_HYPNOSIS,order->extracost); //this is the cost to hypnotise 10*level men.
    int max = u->energy * 10 * level / cost;

    int max2 = level * level * 10;
    if (max2 < max) max = max2;

    //mystic events:
    //1: fizzle 40%
    //2: work 30%
    //3: hypnotise self 20%
    //4: they attack you 10% ??
    
    #ifdef FIZZLES
    int mevent = u->MysticEvent();
    ProduceOrder *p = 0;   
   
    switch(mevent) {
        case 4:
        case 3:
            u->Error("Attempts to hypnotise, but the spell is reflected onto himself.", order->quiet);
            delete u->monthorders;
            u->monthorders = order->monthorder;
            u->taxing = order->taxing;
            order->monthorder = 0;
            u->energy -= 1;
            u->Experience(S_HYPNOSIS, 20);
            return 1;
        case 2:
            u->Error("Attempts to hypnotise, but cannot convince them to do more than work.", order->quiet);
            delete order->monthorder;
            p = new ProduceOrder;
            p->skill = -1;
            p->item = I_SILVER;
            order->monthorder = p;
            break;
        case 1:
            u->Error("Attempts to hypnotise, but the spell fizzles.", order->quiet);
            u->energy -= cost;
            if(u->energy < 0) u->energy = 0;
            u->Experience(S_HYPNOSIS,10);
            return 1;
        default:
            break;
    }
    #endif

	int num = 0;
	int mageerror = 0;
	int numberserror = 0;
	int emptyerror = 0;
	forlist (&(order->units)) {
//cout << "Move is: " << O_MOVE << "Hypno is: " << order->monthorder->type << endl;

		Unit *tar = r->GetUnitId((UnitId *) elem,u->faction->num);
		if (!tar) continue;
		if (tar->faction != u->faction) {
    		if (!tar->IsMage()) {
        		if (tar->GetMen()) {
                    if( (num + tar->GetMen()) <= max) {
                        num += tar->GetMen();
                        delete tar->monthorders;
                        tar->monthorders = 0;
                        
                        tar->taxing = order->taxing;
                        
                        if(order->monthorder) {
                            //not using a switch as have to initialise orders in here - else becomes a nightmare :(.
                            if(order->monthorder->type == O_PRODUCE) {
                                //this includes "WORK" orders
                                    ProduceOrder *p = new ProduceOrder;
                                    p->item = ((ProduceOrder *) order->monthorder)->item;
                                    p->skill = ((ProduceOrder *) order->monthorder)->skill;
                                    tar->monthorders = p;
                            } else if(order->monthorder->type == O_SAIL) {
                                    SailOrder *m = new SailOrder;
                                    forlist(&((SailOrder *) order->monthorder)->dirs) {
                                        MoveDir *old = (MoveDir *) elem;
                                        MoveDir *toadd = new MoveDir;
                                        toadd->dir = old->dir;
                                        m->dirs.Add(toadd);
                                    }
                                    tar->monthorders = m;
                            } else if(order->monthorder->type == O_MOVE || order->monthorder->type == O_ADVANCE) {
                                    MoveOrder *m = new MoveOrder;
                                    m->advancing = ((MoveOrder *) order->monthorder)->advancing;
                                    m->type = ((MoveOrder *) order->monthorder)->type;
    //cout << "Directions ";                                
                                    forlist(&((MoveOrder *) order->monthorder)->dirs) {
    //cout << "in list ";
                                        MoveDir *old = (MoveDir *) elem;
                                        MoveDir *toadd = new MoveDir;
    //cout << old->dir << " ";
                                        toadd->dir = old->dir;
                                        m->dirs.Add(toadd);
                                    }
    //cout << endl;
                                    tar->monthorders = m;
                            } else if(order->monthorder->type == O_STUDY) {
                                    StudyOrder *m = new StudyOrder;
                                    m->skill = ((StudyOrder *) order->monthorder)->skill;
                                    m->days = 0;
                                    m->level = 0;
                                    tar->monthorders = m;
                            } else {
                                   u->Error("Something went wrong with the HYPNOSIS order. Please contact your GM.");
                                   if(tar->taxing == TAX_NONE) {
                                       ProduceOrder *p = new ProduceOrder;
                                       p->item = I_SILVER;
                                       p->skill = -1;
                                       tar->monthorders = p;
                                       u->Error("PS: Hypnosis defaulted to 'WORK'.");
                                   }
                            }
                        }
                       
    //                    tar->monthorders = order->monthorder;        //this isn't going to work for unit #2! Need to clone it.
    
                        tar->Event(AString("Is hypnotised by ") + *(u->name) + ".");
                        u->Event(AString("Hypnotises ") + *(tar->name) + ".");
                    } else numberserror = 1;
                } else {
                    emptyerror = 1;
                    forlist(&tar->gavemento) {
                		UnitId *id = new UnitId;
                		id->unitnum = ((UnitId *) elem)->unitnum;
                		id->alias = 0;
                		id->faction = ((UnitId *) elem)->faction;
                		order->units.Add(id);
                    }
                }
            } else mageerror = 1;
        }
	}

    delete order->monthorder;
    order->monthorder = 0;                       

	if(mageerror) {
	    u->Error("CAST: Can't hypnotise heroes.", order->quiet);
	}
	
	if(numberserror) {
	    if(num) u->Error("CAST: Can't hypnotise that many men. Hypnotising as many as able.", order->quiet);
	    else {
		if(!mageerror) u->Error("CAST: Can't hypnotise that many men.", order->quiet);
		return 0;	        
	    }
	} 
	if(emptyerror) {
	    u->Error("CAST: Trying to hypnotise an empty unit", order->quiet);
	} 


	cost = cost * num / (10 * level);
	if((cost*num)%(10*level)) cost++; //round up
	u->energy -= cost;

	//max 20 experience because it's a rare spell to use, but have to hypnotise at least 67% of your max to get it.
	int experience = 30 * num / max2;
	if(experience > 20) experience = 20;
	u->Experience(S_HYPNOSIS, experience);

	u->Event(AString("Hypnotises ") + num + " men.");
	return 1;
}

int Game::RunCreatePortal(ARegion *r, Unit *u)
{

//	int level = u->GetSkill(S_CREATE_PORTAL);
	CastRegionOrder *order = (CastRegionOrder *) u->activecastorder;
	
    if(TerrainDefs[r->type].similar_type == R_OCEAN) {
        u->Error("CAST: Cannot create a portal in the ocean.", order->quiet);
        return 0;    
    }

	RangeType *range = FindRange(SkillDefs[S_CREATE_PORTAL].range);
	ARegion *tar;
	int val;
	
	if(range != NULL) {
		tar = regions.GetRegion(order->xloc, order->yloc, order->zloc);
		val = GetRegionInRange(r, tar, u, S_CREATE_PORTAL, order->quiet, 0);
		if(!val) return 0;
	} else {
	    u->Error("CAST: Create Portal Range class does not exist. Contact your GM.", 0);
	    return 0;
	}

	int cost = u->GetCastCost(S_CREATE_PORTAL,order->extracost);	
    if(cost > u->energy) {
        u->Error("CAST: Not enough energy to cast that spell.", order->quiet);
        return 0;
    }
    //energy subtracted at end.

    #ifdef FIZZLES
    int mevent = u->MysticEvent();
    int xx;
    int yy;
    switch(mevent) {
        case 4:
            u->Error("Attempts to create a portal, but the spell backfires, sucking away the mage's energy.", order->quiet);
            u->energy = 0;
            u->Experience(S_CREATE_PORTAL,15);
            return 1;
        case 3:
            if(tar != r) {
                u->Error(AString("Attempts to create a linked portal, but the spell backfires, teleporting the mage to ") + tar->ShortPrint(&regions), order->quiet);
                u->energy -= cost;
                u->MoveUnit(tar->GetDummy());
                u->Experience(S_TELEPORTATION,10);
                return 0;
            } else {
                u->energy -= cost;
                xx = getrandom(regions.GetRegionArray(1)->x);
                do {
                    yy = getrandom(regions.GetRegionArray(1)->y);
                } while ( (xx+yy)%2 );
                tar = regions.GetRegion(xx,yy,1);
                if(!tar) break;
                u->Error(AString("Attempts to create a linked portal, but the spell backfires, teleporting the mage to ") + tar->ShortPrint(&regions), order->quiet);
                u->MoveUnit(tar->GetDummy());
                u->Experience(S_TELEPORTATION,10);
                return 0;
            }
        case 2:
        case 1:
            u->Error("Attempts to create a portal, but the spell fizzles.", order->quiet);
            u->energy -= cost;
            u->Experience(S_CREATE_PORTAL,10);
            return 1;
        default:
            break;
    }
    #endif

	//create portal
	Object *o = new Object(r);
	o->num = r->buildingseq++;
	o->type = O_ESEAPORTAL;
	o->name = new AString(AString(ObjectDefs[o->type].name) + " [" + o->num + "]");
	o->incomplete = 0;
	o->inner = -1;
	o->mageowner = u->num;
    r->objects.Add(o);

    AString temp;
    temp = "Creates an unlinked portal.";

	if(tar != r) {
	    //link portal
	    int done = 0;
	    forlist(&tar->objects) {
	        Object *obj = (Object *) elem;
	        if(obj->type == O_ESEAPORTAL && obj->inner < 0 && obj->mageowner == u->num) {
	            done = 1;
	            //link
	            obj->inner = r->num;
	            o->inner = tar->num;
	            temp = AString("Links portals between ") + r->ShortPrint(&regions) + " and " + tar->ShortPrint(&regions);
	        }
     	}
     	if(!done) {
     	    u->Error("CAST: No unlinked portal belongs to this mage in the target region.", order->quiet);
     	    r->objects.Remove(o);
     	    delete o;
     	    r->buildingseq--;
     	    return 0;
 	    }
	}
	u->Experience(S_CREATE_PORTAL, 15);
	u->energy -= cost;
	u->Event(temp);
	return 1;
}

void Game::DoMerchantBuy(Unit *u, BuyOrder *o)
{
    int level = u->GetSkill(S_MERCHANTRY);
    if(level < 4) {
        u->Error("BUY: Insufficient Merchantry level to BUY", o->quiet);
        return;
    }
    if((ItemDefs[o->item].type & IT_ADVANCED) && level < 6) {
        u->Error("BUY: Insufficient Merchantry level to buy advanced items", o->quiet);
        return;
    }

    if(ItemDefs[o->item].type & IT_MAN || ItemDefs[o->item].type & IT_MONSTER || ItemDefs[o->item].type & IT_MAGIC || ItemDefs[o->item].type & IT_SPECIAL || ItemDefs[o->item].type & IT_TRADE || ItemDefs[o->item].type & IT_ILLUSION) {
        u->Error(AString("BUY: Cannot merchant buy ") + ItemDefs[o->item].names, o->quiet);
        return;
    }

    if(!(ItemDefs[o->item].type & IT_NORMAL) && !(ItemDefs[o->item].type & IT_ADVANCED)) {
        u->Error(AString("BUY: Item problem. Please alert your GM. Cannot merchant buy ") + ItemDefs[o->item].names, 0);
        Awrite("Merchant BUY problem!");
        return;
    }

    int cost = ItemDefs[o->item].baseprice * (17-level);
    if(!(ItemDefs[o->item].type & IT_ADVANCED)) cost = (cost+3)/4;    //rounded up!  3.25, 3, 2.75 times baseprice, or 130,120,110% of withdraw cost.
    else cost = ItemDefs[o->item].baseprice * 4;
    int silver = u->GetSharedMoney();
    if(o->num < 0 || o->num > silver/cost) o->num = silver/cost;
    if(u->ConsumeSharedMoney(o->num*cost)) {
        u->items.SetNum(o->item, u->items.GetNum(o->item) + o->num);
    	u->Event(AString("Buys ") + ItemString(o->item, o->num)
            + " at $" + cost + " each.");
    }
    u->nummerchanted += o->num*cost;
}

void Game::DoMerchantSell(Unit *u, SellOrder *o)
{
    int level = u->GetSkill(S_MERCHANTRY);
    
    if(ItemDefs[o->item].type & IT_MAN || ItemDefs[o->item].type & IT_MONSTER || ItemDefs[o->item].type & IT_SPECIAL || ItemDefs[o->item].type & IT_ILLUSION) {
        u->Error(AString("SELL: Cannot merchant sell ") + ItemDefs[o->item].names, o->quiet);
    }
    
        if(!(ItemDefs[o->item].type & IT_NORMAL) && !(ItemDefs[o->item].type & IT_ADVANCED) && !(ItemDefs[o->item].type & IT_MAGIC) && !(ItemDefs[o->item].type & IT_TRADE)) {
        u->Error(AString("SELL: Item problem. Please alert your GM. Cannot merchant sell ") + ItemDefs[o->item].names, o->quiet);
        Awrite("Merchant SELL problem!");
        return;
    }

    int price = ItemDefs[o->item].baseprice * (6+level);
    price /= 8;    //rounded down!    This is 7/8, 1, 9/8, 5/4, 11/8, 3/2 of bp, or 35, 40, 45, 50, 55, 60% of withdraw cost
    if(o->num < 0) o->num = u->items.GetNum(o->item);
    else if(!u->GetSharedNum(o->item,o->num)) o->num = u->GetSharedNum(o->item);

    if(u->ConsumeShared(o->item,o->num)) {
        u->SetMoney(u->GetMoney() + o->num*price);
        u->Event(AString("Sells ") + ItemString(o->item, o->num)
			+ " at $" + price + " each.");
    }
    u->nummerchanted += o->num*price*2;  //double effect due to poor sell prices
}


/*
Todo:

non urgent:
Hexside updates - walls? generalise to int array rather than bridge, road etc.
Square defence and fort combat?
renaming when terrain changes.
*/

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
// 2000/MAR/14 Larry Stanbery    Added production enhancement.
// 2000/MAR/21 Azthar Septragen  Added roads.
// 2001/Feb/21 Joseph Traub      Added FACLIM_UNLIMITED
#include "game.h"
#include "gamedata.h"

void Game::RunSailOrders()
{
	// ALT 28-Jul-2000
	// Fixes to prevent sailing of incomplete ships
	int tmpError = 0;

    forlist(&regions) {
        ARegion * r = (ARegion *) elem;
        AList regs;
        forlist(&r->objects) {
            Object * o = (Object *) elem;
            Unit * u = o->GetOwner();
            if (u && u->monthorders && u->monthorders->type == O_SAIL &&
                o->IsBoat())
            {
				if(o->incomplete == 0) {
					ARegionPtr * p = new ARegionPtr;
					p->ptr = Do1SailOrder(r,o,u);
					regs.Add(p);
				} else {
					tmpError = 1;
				}
            } else {
				tmpError = 2;
			}

			if(tmpError) {
                forlist(&o->units) {
                    Unit * u2 = (Unit *) elem;
                    if (u2->monthorders &&
                        u2->monthorders->type == O_SAIL)
                    {
                        delete u2->monthorders;
                        u2->monthorders = 0;
						switch(tmpError) {
							case 1:
								u2->Error("SAIL: Ship is not finished.");
								break;
							case 2:
								u2->Error("SAIL: Owner must sail ship.");
								break;
						}
                    }
                }
            }
        }
        {
            forlist(&regs) {
                ARegion * r2 = ((ARegionPtr *) elem)->ptr;
                DoAutoAttacksRegion(r2);
            }
        }
    }
}

ARegion * Game::Do1SailOrder(ARegion * reg,Object * ship,Unit * cap)
{
    SailOrder * o = (SailOrder *) cap->monthorders;
    int movepoints = Globals->SHIP_SPEED;
    int moveok = 0;
    
    AList facs;
    int wgt = 0;
    int slr = 0;
    forlist(&ship->units) {
        Unit * unit = (Unit *) elem;
        if (unit->guard == GUARD_GUARD) unit->guard = GUARD_NONE;
        if (!GetFaction2(&facs,unit->faction->num)) {
            FactionPtr * p = new FactionPtr;
            p->ptr = unit->faction;
            facs.Add(p);
        }
        wgt += unit->Weight();
        if (unit->monthorders && unit->monthorders->type == O_SAIL) {
            slr += unit->GetSkill(S_SAILING) * unit->GetMen();
        }

        // xxxxx - sheesh... gotta do something about this.        
        int windlevel = unit->GetSkill(S_SUMMON_WIND);
        if (windlevel) {
            switch (ship->type) {
            case O_LONGBOAT:
                movepoints = Globals->SHIP_SPEED + 2;
                unit->Event("Casts Summon Wind to aid the ship's progress.");
                break;
            case O_CLIPPER:
            case O_BALLOON:
                if (windlevel > 1) {
                    movepoints = Globals->SHIP_SPEED + 2;
                    unit->Event("Casts Summon Wind to aid the ship's progress.");
                }
                break;
            default:
                if (windlevel > 2) {
                    movepoints = Globals->SHIP_SPEED + 2;
                    unit->Event("Casts Summon Wind to aid the ship's progress.");
                }
                break;
            }
        }
    }
    
    if (wgt > ObjectDefs[ship->type].capacity) {
        cap->Error("SAIL: Ship is overloaded.");
        moveok = 1;
    } else {
        if (slr < ObjectDefs[ship->type].sailors) {
            cap->Error("SAIL: Not enough sailors.");
            moveok = 1;
        } else {
            while (o->dirs.Num()) {
                MoveDir * x = (MoveDir *) o->dirs.First();
                o->dirs.Remove(x);
                int i = x->dir;
                delete x;
                ARegion * newreg = reg->neighbors[i];
                if (!newreg) {
                    cap->Error("SAIL: Can't sail that way.");
                    break;
                }
                int cost = 1;
                if( Globals->WEATHER_EXISTS )
                {
                    if (newreg->weather != W_NORMAL) cost = 2;
                }
                
                if (ship->type != O_BALLOON && !newreg->IsCoastal()) {
                    cap->Error("SAIL: Can't sail inland.");
                    break;
                }
                
                if (ship->type != O_BALLOON && reg->type != R_OCEAN &&
                    newreg->type != R_OCEAN) {
                    cap->Error("SAIL: Can't sail inland.");
                    break;
                }
                
                if (movepoints < cost) {
                    cap->Error("SAIL: Can't sail that far.");
                    break;
                }
                
                movepoints -= cost;
                ship->MoveObject( newreg );
                forlist(&facs) {
                    Faction * f = ((FactionPtr *) elem)->ptr;
                    f->Event(*ship->name + AString(" sails from ") +
                             reg->ShortPrint( &regions ) + AString(" to ") +
                             newreg->ShortPrint( &regions ) + AString("."));
                }
                reg = newreg;
                if (newreg->ForbiddenShip(ship)) break;
            }
        }
    }
    
    /* Clear out everyone's orders */
    {
        forlist(&ship->units) {
            Unit * unit = (Unit *) elem;
            if (!moveok) {
                unit->alias = 0;
            }
            
            if (unit->monthorders) {
                if ((!moveok && unit->monthorders->type == O_MOVE) ||
                    unit->monthorders->type == O_SAIL) {
                    delete unit->monthorders;
                    unit->monthorders = 0;
                }
            }
        }
    }
    
    return reg;
}

void Game::RunTeachOrders()
{
	forlist((&regions)) {
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->monthorders) {
					if (u->monthorders->type == O_TEACH) {
						Do1TeachOrder(r,u);
						delete u->monthorders;
						u->monthorders = 0;
					}
				}
			}
		}
	}
}

void Game::Do1TeachOrder(ARegion * reg,Unit * unit)
{
    /* First pass, find how many to teach */
    if( Globals->LEADERS_EXIST && !unit->IsLeader() ) {
		/* small change to handle Ceran's mercs */
		if(!unit->GetMen(I_MERC)) {
			// Mercs can teach even though they are not leaders.
			// They cannot however improve their own skills
			unit->Error("TEACH: Only leaders can teach.");
			return;
		}
	}
    
    int students = 0;
    TeachOrder * order = (TeachOrder *) unit->monthorders;
    forlist(&order->targets) {
        UnitId * id = (UnitId *) elem;
        Unit * target = reg->GetUnitId(id,unit->faction->num);
        if (!target)
        {
            order->targets.Remove(id);
            unit->Error("TEACH: No such unit.");
            delete id;
        }
        else
        {
            if (target->faction->GetAttitude(unit->faction->num) <
                A_FRIENDLY)
            {
                unit->Error(AString("TEACH: ") + *(target->name) +
                            " is not a member of a friendly faction.");
                order->targets.Remove(id);
                delete id;
            }
            else
            {
                if (!target->monthorders ||
                    target->monthorders->type != O_STUDY)
                {
                    unit->Error(AString("TEACH: ") + *(target->name) +
                                " is not studying.");
                    order->targets.Remove(id);
                    delete id;
                } 
                else
                {
                    int sk = ((StudyOrder *) target->monthorders)->skill;
                    if (unit->GetRealSkill(sk) <= target->GetRealSkill(sk))
                    {
                        unit->Error(AString("TEACH: ") +
                                    *(target->name) + " is not studying "
                                    "a skill you can teach.");
                        order->targets.Remove(id);
                        delete id;
                    }
                    else
                    {
                        students += target->GetMen();
                    }
                }
            }
        }
    }
    if (!students) return;
    
    int days = (30 * unit->GetMen() * Globals->STUDENTS_PER_TEACHER);
    
    /* We now have a list of valid targets */
    {
        forlist(&order->targets) {
            UnitId * id = (UnitId *) elem;
            Unit * u = reg->GetUnitId(id,unit->faction->num);
            
            int umen = u->GetMen();
            int tempdays = (umen * days) / students;
            if (tempdays > 30 * umen) tempdays = 30 * umen;
            days -= tempdays;
            students -= umen;
            
            StudyOrder * o = (StudyOrder *) u->monthorders;
            o->days += tempdays;
            if (o->days > 30 * umen)
            {
                days += o->days - 30 * umen;
                o->days = 30 * umen;
            }
            unit->Event(AString("Teaches ") + SkillDefs[o->skill].name +
                        " to " + *u->name + ".");
        }
    }
}

// AS
#define MAX_ROAD_STATE -24

void Game::Run1BuildOrder(ARegion * r,Object * obj,Unit * u)
{
    if (!TradeCheck( r, u->faction ))
    {
        u->Error("BUILD: Faction can't produce in that many regions.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }
  
    int sk = ObjectDefs[obj->type].skill;
    if (sk == -1)
    {
        u->Error("BUILD: Can't build that.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }
    
    int usk = u->GetSkill(sk);
    if (usk < ObjectDefs[obj->type].level) {
        u->Error("BUILD: Can't build that.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }
    
    int needed = obj->incomplete;
    // AS
    if (needed == 0 && !obj->IsRoad()) {
        u->Error("BUILD: Object is finished.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }

    // AS
    if (needed <= MAX_ROAD_STATE && obj->IsRoad())
    {
        u->Error("BUILD: Road does not yet require maintenance.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }
    
    int it = ObjectDefs[obj->type].item;
    int itn;
    if (it == I_WOOD_OR_STONE) {
        itn = u->items.GetNum(I_WOOD) + u->items.GetNum(I_STONE);
    } else {
        itn = u->items.GetNum(it);
    }
  
    if (itn == 0) {
        u->Error("BUILD: Don't have the required item.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }

    int num = u->GetMen() * usk;

	// JLT
	if(obj->incomplete == ObjectDefs[obj->type].cost) {
		if(ObjectIsShip(obj->type)) {
			obj->num = shipseq++;
			obj->SetName(new AString("Ship"));
		} else {
			obj->num = u->object->region->buildingseq++;
			obj->SetName(new AString("Building"));
		}
	}
	// Hack to fix bogus ship numbers
	if(ObjectIsShip(obj->type) && obj->num < 100) {
		obj->num = shipseq++;
		obj->SetName(new AString("Ship"));
	}

    // AS
    AString job;
    if (obj->IsRoadUsable())
    {
        int maintenanceMax = (-MAX_ROAD_STATE + 3 + obj->incomplete) / 4;
        if (num > maintenanceMax) num = maintenanceMax;
        if (itn < num) num = itn;
        job = " maintenance ";
        obj->incomplete -= (num * 4);
        if (obj->incomplete < MAX_ROAD_STATE)
            obj->incomplete = MAX_ROAD_STATE;
    }
    if (!obj->IsRoad() || obj->IsRoadDecaying())
    {
        if (num > needed) num = needed;
        if (itn < num) num = itn;
        job = " construction ";
        obj->incomplete -= num;
        if (obj->IsRoad() && obj->incomplete == 0)
        {
            obj->incomplete = MAX_ROAD_STATE;
        }
    }

    /* Perform the build */

    if (it == I_WOOD_OR_STONE) {
        if (num > u->items.GetNum(I_STONE)) {
            num -= u->items.GetNum(I_STONE);
            u->items.SetNum(I_STONE,0);
            u->items.SetNum(I_WOOD,u->items.GetNum(I_WOOD) - num);
        } else {
            u->items.SetNum(I_STONE,u->items.GetNum(I_STONE) - num);
        }
    } else {
        u->items.SetNum(it,itn - num);
    }

    // AS
    u->Event(AString("Performs") + job + "on " + *(obj->name));
  
    delete u->monthorders;
    u->monthorders = 0;
}

void Game::RunMonthOrders() {
  forlist(&regions) {
    ARegion * r = (ARegion *) elem;
    RunStudyOrders(r);
    RunProduceOrders(r);
  }
}

void Game::RunUnitProduce(ARegion * r,Unit * u)
{
    ProduceOrder * o = (ProduceOrder *) u->monthorders;
  
    if (o->item == I_SILVER) {
        u->Error("Can't do that in this region.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }

    int input = ItemDefs[o->item].input;
    if (input == -1) {
        u->Error("PRODUCE: Can't produce that.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }
  
    int level = u->GetSkill(o->skill);
    if (level < ItemDefs[o->item].level) {
        u->Error("PRODUCE: Can't produce that.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }

    // LLS
    int number = u->GetMen() * level + u->GetProductionBonus(o->item);
  
    if (!TradeCheck( r, u->faction ))
    {
        u->Error("PRODUCE: Faction can't produce in that many regions.");
        delete u->monthorders;
        u->monthorders = 0;
        return;
    }
  
    int inputno = ItemDefs[o->item].number;
	int maxproduced;

	int input2 = ItemDefs[o->item].input2;
	int inputno2 = ItemDefs[o->item].number2;
	if(input2 != -1) {
		maxproduced = number/(inputno + inputno2);
	} else {
		maxproduced = number/inputno;
	}
   
    if (maxproduced * inputno > u->items.GetNum(input))
        maxproduced = u->items.GetNum(input) / inputno;

	if((input2 != -1) && (maxproduced * inputno > u->items.GetNum(input2))) {
		int max2 = u->items.GetNum(input2)/inputno2;
		if(max2 < maxproduced) maxproduced = max2;
	}

    u->items.SetNum(input,u->items.GetNum(input) - maxproduced * inputno);
	if(input2 != -1) {
		u->items.SetNum(input2, u->items.GetNum(input2)-maxproduced*inputno2);
	}
    u->items.SetNum(o->item,u->items.GetNum(o->item) + maxproduced);
    u->Event(AString("Produces ") + ItemString(o->item,maxproduced) + " in " +
             r->ShortPrint( &regions ) + ".");
    delete u->monthorders;
    u->monthorders = 0;
}

void Game::RunProduceOrders(ARegion * r) {
  {
    forlist ((&r->products))
      RunAProduction(r,(Production *) elem);
  }
  {
    forlist((&r->objects)) {
      Object * obj = (Object *) elem;
      forlist ((&obj->units)) {
	Unit * u = (Unit *) elem;
	if (u->monthorders) {
	  if (u->monthorders->type == O_PRODUCE) {
	    RunUnitProduce(r,u);
	  } else
	    if (u->monthorders->type == O_BUILD) {
	      Run1BuildOrder(r,obj,u);
	    }	
	}
      }
    }
  }
}

int Game::ValidProd(Unit * u,ARegion * r,Production * p)
{
    if (u->monthorders->type != O_PRODUCE) return 0;

    ProduceOrder * po = (ProduceOrder *) u->monthorders;
    if (p->itemtype == po->item && p->skill == po->skill) {
        if (p->skill == -1) {
            po->productivity = u->GetMen() * p->productivity;
            return po->productivity;
        }
        int level = u->GetSkill(p->skill);
        //    if (level < p->level) {
        if (level < ItemDefs[p->itemtype].level) {
            u->Error("PRODUCE: Unit isn't skilled enough.");
            delete u->monthorders;
            u->monthorders = 0;
            return 0;
        }
		
        //
        // Check faction limits on production. If the item is silver, then the
        // unit is entertaining or working, and the limit does not apply
        //
        if (p->itemtype != I_SILVER && !TradeCheck( r, u->faction ))
        {
            u->Error("PRODUCE: Faction can't produce in that many regions.");
            delete u->monthorders;
            u->monthorders = 0;
            return 0;
        }
    
        /* check for bonus production */
        // LLS
        int bonus = u->GetProductionBonus(p->itemtype);
        po->productivity = u->GetMen() * level * p->productivity + bonus;
        return po->productivity;
    }
    return 0;
}

int Game::FindAttemptedProd(ARegion * r,Production * p) {
  int attempted = 0;
  forlist((&r->objects)) {
    Object * obj = (Object *) elem;
    forlist((&obj->units)) {
      Unit * u = (Unit *) elem;
      if (u->monthorders) {
	attempted += ValidProd(u,r,p);
      }
    }
  }
  return attempted;
}

void Game::RunAProduction(ARegion * r,Production * p)
{
    p->activity = 0;
    if (p->amount == 0) return;
    
    /* First, see how many units are trying to work */
    int attempted = FindAttemptedProd(r,p);
    int amt = p->amount;
    if (attempted < amt) attempted = amt;
    forlist((&r->objects)) {
        Object * obj = (Object *) elem;
        forlist((&obj->units)) {
            Unit * u = (Unit *) elem;
            if( !u->monthorders || u->monthorders->type != O_PRODUCE)
            {
                continue;
            }

            ProduceOrder * po = (ProduceOrder *) u->monthorders;
            if (po->skill != p->skill || po->item != p->itemtype)
            {
                continue;
            }

            /* We need to implement a hack to avoid overflowing */
            int uatt, ubucks;
            
            uatt = po->productivity;
            if (uatt && amt && attempted)
            {
                double dUbucks = ((double) amt) * ((double) uatt)
                    / ((double) attempted);
                ubucks = (int) dUbucks;
            }
            else
            {
                ubucks = 0;
            }
            
            amt -= ubucks;
            attempted -= uatt;
            u->items.SetNum(po->item,u->items.GetNum(po->item)
                            + ubucks);
            p->activity += ubucks;
            
            /* Show in unit's events section */
            if (po->item == I_SILVER)
            {
                //
                // WORK
                //
                if (po->skill == -1)
                {
                    u->Event(AString("Earns ") + ubucks + " silver working in "
                             + r->ShortPrint( &regions ) + ".");
                }
                else
                {
                    //
                    // ENTERTAIN
                    //
                    u->Event(AString("Earns ") + ubucks
                             + " silver entertaining in " +
                             r->ShortPrint( &regions )
                             + ".");
                }
            }
            else
            {
                /* Everything else */
                u->Event(AString("Produces ") + ItemString(po->item,ubucks) + 
                         " in " + r->ShortPrint( &regions ) + ".");
            }
            delete u->monthorders;
            u->monthorders = 0;
        }
    }
}

void Game::RunStudyOrders(ARegion * r)
{
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			if (u->monthorders) {
				if (u->monthorders->type == O_STUDY) {
					Do1StudyOrder(u,obj);
					delete u->monthorders;
					u->monthorders = 0;
				}
			}
		}
	}
}

void Game::Do1StudyOrder(Unit *u,Object *obj)
{
    StudyOrder * o = (StudyOrder *) u->monthorders;
    int sk = o->skill;
    int cost = SkillCost(sk) * u->GetMen();
	int reset_man = -1;
    if (cost > u->GetMoney())
    {
        u->Error("STUDY: Not enough funds.");
        return;
    }

	// Small patch for Ceran Mercs
	if(u->GetMen(I_MERC)) {
		u->Error("STUDY: Mercenaries are not allowed to study.");
		return;
	}
	
    if( ( SkillDefs[sk].flags & SkillType::MAGIC ) && u->type != U_MAGE)
    {
		if(u->type == U_APPRENTICE) {
			u->Error("STUDY: An apprentice cannot be made into an mage.");
			return;
		}
		if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
			if (CountMages(u->faction) >= AllowedMages( u->faction )) {
				u->Error("STUDY: Can't have another magician.");
				return;
			}
		}
        if (u->GetMen() != 1)
        {
            u->Error("STUDY: Only 1-man units can be magicians.");
            return;
        }
        if( !( Globals->MAGE_NONLEADERS ))
        {
            if (u->GetMen(I_LEADERS) != 1)
            {
                u->Error("STUDY: Only leaders may study magic.");
                return;
            }
        }
		reset_man = u->type;
        u->type = U_MAGE;
    }

	if((SkillDefs[sk].flags&SkillType::APPRENTICE) && u->type != U_APPRENTICE){
		if(u->type == U_MAGE) {
			u->Error("STUDY: A mage cannot be made into an apprentice.");
			return;
		}

		if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
			if(CountApprentices(u->faction)>=AllowedApprentices(u->faction)) {
				u->Error("STUDY: Can't have another apprentice.");
				return;
			}
		}
		if(u->GetMen() != 1) {
			u->Error("STUDY: Only 1-man units can be apprentices.");
			return;
		}
		if(!(Globals->MAGE_NONLEADERS)) {
			if(u->GetMen(I_LEADERS) != 1) {
				u->Error("STUDY: Only leaders may be apprentices.");
				return;
			}
		}
		reset_man = u->type;
		u->type = U_APPRENTICE;
	}
  
    int days = 30 * u->GetMen() + o->days;
    
    if( ( SkillDefs[sk].flags & SkillType::MAGIC ) && u->GetSkill(sk) >= 2 &&
        (!ObjectDefs[obj->type].protect || obj->incomplete) )
    {
        u->Error("Warning: Magic study rate outside of a tower cut in half "
                 "above level 2.");
        days /= 2;
    }

    if (u->Study(sk,days))
    {
        u->SetMoney(u->GetMoney() - cost);
        u->Event(AString("Studies ") + SkillDefs[sk].name + ".");
    }
	else
	{
		// if we just tried to become a mage or apprentice, but
		// were unable to study, reset unit to whatever it was before.
		if(reset_man != -1)
			u->type = reset_man;
	}
}

void Game::RunMoveOrders()
{
	for (int phase = 0; phase < Globals->MAX_SPEED; phase++) {
		{
			forlist((&regions)) {
				ARegion * region = (ARegion *) elem;
				forlist((&region->objects)) {
					Object * obj = (Object *) elem;
					forlist(&obj->units) {
						Unit * unit = (Unit *) elem;
						Object *tempobj = obj;
						DoMoveEnter(unit,region,&tempobj);
					}
				}
			}
		}

		AList * locs = new AList;
		forlist((&regions)) {
			ARegion * region = (ARegion *) elem;
			forlist((&region->objects)) {
				Object * obj = (Object *) elem;
				forlist(&obj->units) {
					Unit * unit = (Unit *) elem;
					if (phase == unit->movepoints && unit->monthorders &&
						unit->monthorders->type == O_MOVE && !unit->nomove) {
						locs->Add(DoAMoveOrder(unit,region,obj));
					}
				}
			}
/*
      DoAdvanceAttacks(locs);
      locs->DeleteAll();
*/
		}
		DoAdvanceAttacks(locs);
		locs->DeleteAll();
	}
}

void Game::DoMoveEnter(Unit * unit,ARegion * region,Object **obj)
{
    MoveOrder * o;
    if (!unit->monthorders || (unit->monthorders->type != O_MOVE)) return;
    o = (MoveOrder *) unit->monthorders;
    
    while (o->dirs.Num()) {
        MoveDir * x = (MoveDir *) o->dirs.First();
        int i = x->dir;
        if (i != MOVE_OUT && i < MOVE_ENTER) return;
        o->dirs.Remove(x);
        delete x;
        
        if (i >= MOVE_ENTER) {
            Object * to = region->GetObject(i - MOVE_ENTER);
            if (!to) {
                unit->Error("MOVE: Can't find object.");
                continue;
            }
            
            if (!to->CanEnter(region,unit)) {
                unit->Error("ENTER: Can't enter that.");
                continue;
            }
            
            Unit *forbid = to->ForbiddenBy(region, unit);
            if (forbid && !o->advancing) {
                unit->Error("ENTER: Is refused entry.");
                continue;
            }
            
            if( forbid && region->IsSafeRegion() )
            {
                unit->Error( "ENTER: No battles allowed in safe regions." );
                continue;
            }

            int done = 0;
            while (forbid)
            {
                RunBattle(region, unit, forbid, 0, 0);
                if (!unit->IsAlive() || !unit->canattack) {
                    done = 1;
                    break;
                }
                forbid = to->ForbiddenBy(region, unit);
            }
            if (done) continue;
            
            unit->MoveUnit( to );
            unit->Event(AString("Enters ") + *(to->name) + ".");
            *obj = to;
        } else {
            if (i == MOVE_OUT) {
				if( region->type == R_OCEAN &&
						(!unit->CanSwim() ||
						 unit->GetFlag(FLAG_NOCROSS_WATER)) )
				{
                    unit->Error("MOVE: Can't leave ship.");
                    continue;
                }

                Object * to = region->GetDummy();
                unit->MoveUnit( to );
                *obj = to;
            }
        }
    }
}

Location * Game::DoAMoveOrder(Unit * unit,ARegion * region,Object * obj)
{
    Location * loc = new Location;
    MoveOrder * o = (MoveOrder *) unit->monthorders;
    int movetype = unit->MoveType();
    
    if (unit->guard == GUARD_GUARD) unit->guard = GUARD_NONE;
    if (o->advancing) unit->guard = GUARD_ADVANCE;
    
    /* Ok, now we can move a region */
    if (o->dirs.Num()) {
        MoveDir * x = (MoveDir *) o->dirs.First();
        o->dirs.Remove(x);
        int i = x->dir;
		int startmove = 0;
        delete x;
        
        /* Setup region to move to */
        ARegion * newreg;
        if (i == MOVE_IN) {
            if (obj->inner == -1) {
                unit->Error("MOVE: Can't move IN there.");
                goto done_moving;
            }
            newreg = regions.GetRegion(obj->inner);
        } else {
            newreg = region->neighbors[i];
        }
        
        if (!newreg) {
            unit->Error(AString("MOVE: Can't move that direction."));
            goto done_moving;
        }

		if(region->type == R_NEXUS && newreg->IsStartingCity())
			startmove = 1;
        
        if( region->type == R_OCEAN &&
		   (!unit->CanSwim() || unit->GetFlag(FLAG_NOCROSS_WATER)) )
        {
            unit->Error( AString("MOVE: Can't move while in the ocean.") );
            goto done_moving;
        }
        
        int cost = newreg->MoveCost(movetype);
        // AS
        if (cost > 1 && movetype != M_FLY && region->HasExitRoad(i) &&
            newreg->HasConnectingRoad(i))
        {
            cost /= 2;
        }
        if (region->type != R_NEXUS &&
            unit->CalcMovePoints() - unit->movepoints < cost) 
        {
            unit->Error(AString("MOVE: Unit does not have enough movement points."));
            goto done_moving;
        }
        
        if(newreg->type == R_OCEAN &&
		   (!unit->CanSwim() || unit->GetFlag(FLAG_NOCROSS_WATER)) ) {
            unit->Event(AString("Discovers that ") + 
                        newreg->ShortPrint( &regions ) +
                        AString(" is ocean."));
            goto done_moving;
        }
        
        if (unit->type == U_WMON && newreg->town && newreg->IsGuarded()) {
            unit->Event("Monsters don't move into guarded towns.");
            goto done_moving;
        }
        
        if (unit->guard == GUARD_ADVANCE) {
            Unit *ally = newreg->ForbiddenByAlly(unit);
            if (ally && !startmove) {
                unit->Event(AString("Can't ADVANCE: ") + *(newreg->name) +
                            " is guarded by " + *(ally->name) + ", an ally.");
                goto done_moving;
            }
        }
        
        Unit * forbid = newreg->Forbidden(unit);
        if (forbid && !startmove && unit->guard != GUARD_ADVANCE) {
			int obs = unit->GetObservation();
			unit->Event(AString("Is forbidden entry to ") +
                        newreg->ShortPrint( &regions ) + " by " +
						forbid->GetName(obs) + ".");
			obs = forbid->GetObservation();
			forbid->Event(AString("Forbids entry to ") +
						  unit->GetName(obs) + ".");
            goto done_moving;
        }
        
        /* Clear the unit's alias out, so as not to interfere with TEACH */
        unit->alias = 0;
        
        unit->movepoints += cost;
        unit->MoveUnit( newreg->GetDummy() );

        AString temp;
        switch (movetype) {
        case M_WALK:
            temp = "Walks ";
            break;
        case M_RIDE:
            temp = "Rides ";
            break;
        case M_FLY:
            temp = "Flies ";
            break;
        }
        unit->Event(temp + AString("from ") + region->ShortPrint( &regions )
                    + AString(" to ") + newreg->ShortPrint( &regions ) +
                    AString("."));
        if (forbid) {
            unit->advancefrom = region;
        }
        region = newreg;
    }
    
    loc->unit = unit;
    loc->region = region;
    loc->obj = obj;
    return loc;
    
done_moving:
    delete o;
    unit->monthorders = 0;
    loc->unit = unit;
    loc->region = region;
    loc->obj = obj;
    return loc;
}

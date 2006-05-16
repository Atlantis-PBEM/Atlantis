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
// Date			Person				Comments
// ----			------				--------
// 2000/MAR/14	Larry Stanbery		Added production enhancement.
// 2000/MAR/21	Azthar Septragen	Added roads.
// 2001/Feb/21	Joseph Traub		Added FACLIM_UNLIMITED
#ifndef DEBUG
//#define DEBUG
#endif


#include "game.h"
#include "gamedata.h"

void Game::SailShips(ARegion *r, int phase, AList * regs)
{
    //created 10 March 2004 to allow ships to move one region at a time
    //during the move cycle
	int tmpError = 0;

	forlist(&r->objects) {
		Object * o = (Object *) elem;
		Unit * u = o->GetOwner();
		if (u && u->movepoints == phase && u->monthorders &&
				u->monthorders->type == O_SAIL &&
				o->IsBoat()) {
			if(o->incomplete < 1) {
//				locations->Add(DoASailOrder(r,o,u));
				ARegionPtr * p = new ARegionPtr;
				p->ptr = DoASailOrder(r,o,u);
				regs->Add(p);
			} else {
				tmpError = 1;
			}
		} else {
			tmpError = 2;
		}

		if(tmpError && phase == 0 && u && u->movepoints == 0) {  //ie no errors here on later phases, or in first phase if ship has already sailed into this region.
			forlist(&o->units) {
				Unit * u2 = (Unit *) elem;
				if (u2->monthorders && u2->monthorders->type == O_SAIL) {
					switch(tmpError) {
						case 1:
							u2->Error("SAIL: Ship is not finished.", u2->monthorders->quiet);
							break;
						case 2:
                            if(o->IsBoat()) u2->Error("SAIL: Owner must sail ship.", u2->monthorders->quiet);
                            else u2->Error("SAIL: That unit is not in a ship.", u2->monthorders->quiet);
							break;
					}
					delete u2->monthorders;
					u2->monthorders = 0;
				}
			}
		}
	}
}

ARegion * Game::DoASailOrder(ARegion *reg, Object *ship, Unit *captain)
{
	AList facs;
	int wgt = 0;
	int slr = 0;
	int movepoints = ObjectDefs[ship->type].speed;
	int moved = 1;
	
	Unit * windmage = 0;
	int windlevel = 0;

	int firstmove = 0;
	if (captain->movepoints == 0) firstmove = 1;

	//get move direction
	int dir;
	int quiet;
	if(captain->monthorders->type == O_SAIL) {
    	SailOrder * o = (SailOrder *) captain->monthorders;
    	quiet = o->quiet;
    	if (o->dirs.Num()) {
    		MoveDir * x = (MoveDir *) o->dirs.First();
    		o->dirs.Remove(x);
    		dir = x->dir;
		    delete x;
    	} else goto done_sailing;
	} else if(captain->monthorders->type == O_FOLLOW) {
	    FollowOrder * o = (FollowOrder *) captain->monthorders;
	    dir = o->dir;
    	quiet = o->quiet;
	} else goto done_sailing; //this should never occur.


	{
        int shipclass = 1;
        if(ObjectDefs[ship->type].capacity > 499) shipclass++;
        if(ObjectDefs[ship->type].capacity > 1200) shipclass++;

		//check weight onboard (need to do this every move, because someone might get on with a moveenter command). Also check SWIN mages
    	forlist(&ship->units) {
    		Unit * unit = (Unit *) elem;
    		if (unit->guard == GUARD_GUARD) unit->guard = GUARD_NONE;
    		if (!GetFaction2(&facs,unit->faction->num)) {
    			FactionPtr * p = new FactionPtr;
    			p->ptr = unit->faction;
    			facs.Add(p);
    		}
    		wgt += unit->Weight();
    		if (unit == captain || (unit->monthorders && unit->monthorders->type == O_SAIL) ) {  //captain may not have orders to sail if he is FOLLOWing.
    			slr += unit->GetSkill(S_SAILING) * unit->GetMen();
    		}
    		if(ship->speedbonus == 0 && unit->GetSkill(S_SUMMON_WIND) > windlevel && (!Globals->ARCADIA_MAGIC || unit->energy >= unit->GetCastCost(S_SUMMON_WIND, 1, shipclass) ) ) {
    		    windlevel = unit->GetSkill(S_SUMMON_WIND);
    		    windmage = unit;
		    }
   		}

        if(windlevel >= shipclass) {
        	if(Globals->ARCADIA_MAGIC) {
        	    //summon_wind speed bonus for ARCADIA
            	ship->speedbonus = (windlevel*movepoints)/5;
            	if (ship->speedbonus == 0) ship->speedbonus = 1; //minimum bonus of 1. If don't want this, must prevent casting, else will cast every ship move
            	if(windmage) {
                    windmage->energy -= windmage->GetCastCost(S_SUMMON_WIND, 1, shipclass);
                    windmage->Event("Casts Summon Wind to aid the ship's "
    				"progress.");
    			    windmage->Practice(S_SUMMON_WIND);
    			    windmage->Experience(S_SUMMON_WIND,10);
  				}
  				#ifdef DEBUG
  				if (windlevel & !windmage) {
  				    Awrite("Windlevel but no windmage");
  				    system("pause");
  				}
  				#endif
        	} else {
        	    //non-Arcadia coding
    			movepoints += 2;
    			if(windmage && captain->movepoints == 0) {
                    windmage->Event("Casts Summon Wind to aid the ship's "
    				"progress.");
    			    windmage->Practice(S_SUMMON_WIND);
    			    windmage->Experience(S_SUMMON_WIND,10);
    			}
        	}
    	}
    	
    	movepoints += ship->speedbonus; //have to do it this way so bonus carries over to subsequent phases (but only if the mage had the energy to cast SWIN originally)

    	if (wgt > ObjectDefs[ship->type].capacity) {
    		captain->Error("SAIL: Ship is overloaded.", quiet);
    		moved = 0;
    	} else if (slr < ObjectDefs[ship->type].sailors) {
			captain->Error("SAIL: Not enough sailors.", quiet);
			moved = 0;
		} else {
		    //guts of the sailing code, and movepoints check.
			/* Hexside Patch 030825 BS */
			int cost = 1;
			int newhexside = -1;
			int waterdepth = 0;


			if(ship->hexside < 0) {    //ie starts from the middle of a hex.
				ARegion * newreg = reg->neighbors[dir];
				if (!newreg) {
					captain->Error("SAIL: Can't sail that way.", quiet);   //ARCADIAN_CHECKER: This will give info away in underground caverns unless blocks are removed
					goto done_sailing;
				}
				if(Globals->WEATHER_EXISTS) {
					if (newreg->weather != W_NORMAL && !ship->speedbonus && !newreg->clearskies) cost *= 2;
					if (newreg->weather == W_BLIZZARD && !ship->speedbonus && !newreg->clearskies) cost *= 5;
				}

				if (!(ObjectDefs[ship->type].flags & ObjectType::SAILOVERLAND) && !newreg->IsCoastal()) {
					captain->Error("SAIL: Can't sail inland.", quiet);
					goto done_sailing;
				}

				if (!(ObjectDefs[ship->type].flags & ObjectType::SAILOVERLAND) &&        //remember this is for ships starting in the centre of the hex, so if
					(TerrainDefs[reg->type].similar_type != R_OCEAN && TerrainDefs[reg->type].similar_type != R_FAKE) &&           //if the hex is not ocean, we are clearly NOT using hexside terrain rules.
					(TerrainDefs[newreg->type].similar_type != R_OCEAN && TerrainDefs[newreg->type].similar_type != R_FAKE)) {
					captain->Error("SAIL: Can't sail inland.", quiet);
					goto done_sailing;
				}

				// Check to see if sailing THROUGH land!
				// always allow retracing steps
				//This section was not modified by BS in changing the sailing rules March '04
				//This section not made compatible with ARCADIAN_CHECKER patch
				if (Globals->PREVENT_SAIL_THROUGH &&
						(TerrainDefs[reg->type].similar_type != R_OCEAN) &&
						!(ObjectDefs[ship->type].flags & ObjectType::SAILOVERLAND) &&
						(ship->prevdir != -1) &&
						(ship->prevdir != dir) && !Globals->HEXSIDE_TERRAIN) {
					int blocked1 = 0;
					int blocked2 = 0;
					int d1 = ship->prevdir;
					int d2 = dir;
					if (d1 > d2) {
						int tmp = d1;
						d1 = d2;
						d2 = tmp;
					}
					for (int k = d1+1; k < d2; k++) {
						ARegion *land1 = reg->neighbors[k];
						if ((!land1) ||
								(TerrainDefs[land1->type].similar_type !=
								 R_OCEAN))
							blocked1 = 1;
					}
					int sides = NDIRS - 2 - (d2 - d1 - 1);
					for (int l = d2+1; l <= d2 + sides; l++) {
						int dl = l;
						if (dl >= NDIRS) dl -= NDIRS;
						ARegion *land2 = reg->neighbors[dl];
						if ((!land2) ||
								(TerrainDefs[land2->type].similar_type !=
								 R_OCEAN))
							blocked2 = 1;
					}
					if ((blocked1) && (blocked2))
					{
						captain->Error(AString("SAIL: Could not sail ") +
								DirectionStrs[dir] + AString(" from ") +
								reg->ShortPrint(&regions) +
								". Cannot sail through land.", quiet);
					    goto done_sailing;
					}
				}


			    /* Hexside Patch 030825 BS */
				// set hexside value for hexside ships
				newhexside = -1;
				if((TerrainDefs[newreg->type].similar_type != R_OCEAN && TerrainDefs[newreg->type].similar_type != R_FAKE) && ObjectDefs[ship->type].hexside && Globals->HEXSIDE_TERRAIN) {
				    //check if the ship can land here. Since it is at sea, it cannot be river-only [Changed: it could be shallow only on a lake]. Check if the terrain forbids ocean-only ships.
				    waterdepth = HexsideDefs[newreg->hexside[reg->GetRealDirComp(dir)]->type].sailable;
				    cost = (cost+1) / 2;  //ie 1 in winter, 3 in blizzard

                    if(waterdepth==1 && ObjectDefs[ship->type].sailable==2) {
                        captain->Error(AString("SAIL: Water is too shallow for ship to approach ") + newreg->ShortPrint(&regions), quiet);
                        goto done_sailing;
                    }
                    if(waterdepth==2 && ObjectDefs[ship->type].sailable==1) {  //currently no hexside terrain with depth 2, but put in just in case of shallow ship sailing from a lake to such terrain
                        captain->Error(AString("SAIL: Water is too deep for ship to approach. NB This should never occur in Arcadia, contact your GM!") + newreg->ShortPrint(&regions), 0);
                        goto done_sailing;
                    }
                    if(waterdepth==0) {
                        captain->Error(AString("SAIL: Terrain prevents ship sailing to ") + newreg->ShortPrint(&regions), quiet);
                        goto done_sailing;
                    }                        
                    newhexside = reg->GetRealDirComp(dir);   //this is the hexside we land at.
                }            
				// else hexside value remains -1. Eg if hexside rules are turned off.
				// for shallow ships on a lake, we are relying on lakes never being adjacent to oceans. Otherwise we need a depth check here also.
				
				if (movepoints - captain->movepoints < cost) {
				    if(captain->monthorders->type == O_SAIL) {
				        SailOrder *o = (SailOrder *) captain->monthorders;
    					captain->Error("SAIL: Can't sail that far;"
    						" remaining moves queued.", quiet);
    					TurnOrder *tOrder = new TurnOrder;
    					tOrder->repeating = 0;
    					AString order = "SAIL ";
    					order += DirectionAbrs[dir];
    					forlist(&o->dirs) {
    						MoveDir *move = (MoveDir *) elem;
    						order += " ";
    						order += DirectionAbrs[move->dir];
    					}
    					tOrder->turnOrders.Add(new AString(order));
    					captain->turnorders.Insert(tOrder);
   					} else {
			            captain->Error("SAIL: Unit has insufficient movement points to continue following.", quiet);
   					}
  					goto done_sailing;
				}

				captain->movepoints += cost;                  //at the moment, captain of ship cannot change mid-sail (well, except for unlikely event of him being killed, in which case if replacement had sail orders they'll be phase 0 so never called). If ever a unit is allowed to sail then move, then need to update all shipboard unit's movepoints at this step.
				ship->MoveObject(newreg);
				ship->SetPrevDir(reg->GetRealDirComp(dir));
				ship->hexside = newhexside;
				forlist(&facs) {                        //it might be more convenient to give messages for each unit who helps with sailing. However, this is more concise on the report.
					Faction * f = ((FactionPtr *) elem)->ptr;
					if(newhexside>-1) f->Event(*ship->name + AString(" sails from ") +
							reg->ShortPrint(&regions) + AString(" to ") +
							newreg->ShortPrint(&regions) + " (" + DirectionStrs[newhexside] + AString(")."));
					else f->Event(*ship->name + AString(" sails from ") +
							reg->ShortPrint(&regions) + AString(" to ") +
							newreg->ShortPrint(&regions) + AString("."));		
				}
				if(Globals->TRANSIT_REPORT != GameDefs::REPORT_NOTHING) {
					forlist(&ship->units) {
						// Everyone onboard gets to see the sights
						Unit *unit = (Unit *)elem;
						Farsight *f;
						// Note the hex being left
						forlist(&reg->passers) {
							f = (Farsight *)elem;
							if(f->unit == unit) {
								// We moved into here this turn
								f->exits_used[dir] = 1;
							}
						}
						// And mark the hex being entered
						f = new Farsight;
						f->faction = unit->faction;
						f->level = 0;
						f->unit = unit;
						f->exits_used[reg->GetRealDirComp(dir)] = 1;
						newreg->passers.Add(f);
					}
				}
				reg = newreg; //updates for region returned.
				if (newreg->ForbiddenShip(ship) && !Globals->HEXSIDE_TERRAIN) { //with hexside rules, we are on a beach, so guards are NOT going to stop us.
					captain->faction->Event(*ship->name +
							AString(" is stopped by guards in ") +
							newreg->ShortPrint(&regions) + AString("."));
					goto done_sailing;
				}
			}

			else {   //ie (ship->hexside (originally) > -1)
			    //Thus, we are clearly using hexside sailing rules ;)
			    newhexside = -1;
			    ARegion * newreg = reg;

			    //if sailing is in same direction as hexside, cross river or enter sea
			    if(dir==ship->hexside) {
				    newreg = reg->neighbors[dir];
    				    if (!newreg) {
        					captain->Error("SAIL: Can't sail that way - no region present.", quiet);
        					goto done_sailing;
    					}
				    newhexside = (ship->hexside+3)%6;                 //ie opposite edge. Could also use GetCompDir here I think.			    
			    }
			    
			    //if sailing to the same hex
			    else if(dir == (ship->hexside+2)%6 || dir == (ship->hexside+4)%6) {
				    newreg = reg;
				    if(dir == (ship->hexside+2)%6) {
                        newhexside = (ship->hexside+1)%6; //eg if on N hexside sailing SE, move to NE hexside of same region.                        
				    }
				    if(dir == (ship->hexside+4)%6 ) {
                        newhexside = (ship->hexside+5)%6; //eg if on N hexside sailing SW, move to NW hexside of same region.
				    }
			    }
			    
			    //if sailing to a new river section & new hex
			    else if(dir == (ship->hexside+1)%6 || dir == (ship->hexside+5)%6 ) {
				    newreg = reg->neighbors[dir];
    				    if (!newreg) {
        					captain->Error("SAIL: Can't sail that way - no region present.", quiet);
        					goto done_sailing;
    					}
				    if(dir == (ship->hexside+1)%6 ) {
    				    newhexside = (ship->hexside+5)%6; //eg if on N hexside sailing NE, move to NW hexside of NE region.			    
                    }
				    if(dir == (ship->hexside+5)%6 ) {
    				    newhexside = (ship->hexside+1)%6; //eg if on N hexside sailing NW, move to NE hexside of NW region
				    }
			    }
			    
			    //if sailing towards hex centre
			    else if(dir == (ship->hexside+3)%6 ) {
					captain->Error("SAIL: Can't sail across land", quiet);	
        			goto done_sailing;
			    }
			    else {
					captain->Error("SAIL: This sailing error should never be called. Contact your GM.", 0);		
        			goto done_sailing;
			    }
			    
			    if(newhexside<0 || newhexside>5) {
					captain->Error("SAIL: This hexside error should never be called. Contact your GM", 0);	
        			goto done_sailing;
			    }
			    //we now have the new region and new hexside!
			    //check that the new region/hexside can be sailed to!
			    cost = 1;
                waterdepth = 0;
			    if(TerrainDefs[newreg->type].similar_type == R_OCEAN || TerrainDefs[newreg->type].similar_type == R_FAKE) {
                    newhexside=-1;
                    if(newreg->type == R_LAKE) waterdepth=3; //anything can sail onto a lake.
                    else waterdepth=2;
                }
                else if(!Globals->HEXSIDE_TERRAIN) {   // This could get called if a game goes through rule changes. Otherwise it should never be.
                    newhexside = -1;
                }
			    else {
				    waterdepth = HexsideDefs[newreg->hexside[newhexside]->type].sailable;

                    //if have found no sailable object, check if it is a beach with no beach object from bad world generation
/*                    if(!waterdepth) {
                        if(TerrainDefs[newreg->neighbors[newhexside]->type].similar_type == R_OCEAN) waterdepth = 2; //This looks strange, but is checking the hexside the boat is on to see if it is adjacent to ocean/lake, and saying that in the absence of a beach we will treat it like ocean.
                    } */                        //removed as this was allowing sailing onto rocks
			    }
			    if(!waterdepth) {
				    captain->Error("SAIL: That location cannot be sailed to.", quiet);	
        			goto done_sailing;
			    }
			    if(ObjectDefs[ship->type].sailable != waterdepth && ObjectDefs[ship->type].sailable != 3 && waterdepth != 3) {
			        captain->Error("SAIL: That ship type cannot sail there.", quiet);	
        			goto done_sailing;
			    }
			    
				if(Globals->WEATHER_EXISTS) {
				    if (newreg->weather == W_BLIZZARD && !ship->speedbonus && !newreg->clearskies) cost = cost * 5;
					else if (newreg->weather != W_NORMAL && !ship->speedbonus && !newreg->clearskies) cost = cost * 2;
				}
				if(newhexside != -1) cost = (cost+1)/2;  //ie half cost for moving to hexsides (rounded up).
				
				// if cannot move
				if (movepoints - captain->movepoints < cost) {
				    if(captain->monthorders->type == O_SAIL) {
				        SailOrder *o = (SailOrder *) captain->monthorders;
    					captain->Error("SAIL: Can't sail that far;"
    						" remaining moves queued.", quiet);
    					TurnOrder *tOrder = new TurnOrder;
    					tOrder->repeating = 0;
    					AString order = "SAIL ";
    					order += DirectionAbrs[dir];
    					forlist(&o->dirs) {
    						MoveDir *move = (MoveDir *) elem;
    						order += " ";
    						order += DirectionAbrs[move->dir];
    					}
    					tOrder->turnOrders.Add(new AString(order));
    					captain->turnorders.Insert(tOrder);
   					} else {
			            captain->Error("SAIL: Unit has insufficient movement points to continue following.", quiet);
   					}
  					goto done_sailing;
				}
				//move the ship
				captain->movepoints += cost;
				ship->MoveObject(newreg);
				ship->SetPrevDir(dir);  //This is obsolete with hexside stuff. Could just as well discard it.
				forlist(&facs) {
					Faction * f = ((FactionPtr *) elem)->ptr;
					if(newhexside>-1) f->Event(*ship->name + AString(" sails from ") +
							reg->ShortPrint(&regions) + " (" + DirectionStrs[ship->hexside] + AString(") to ") +
							newreg->ShortPrint(&regions) + " (" + DirectionStrs[newhexside] + AString(")."));
					else f->Event(*ship->name + AString(" sails from ") +
							reg->ShortPrint(&regions) + " (" + DirectionStrs[ship->hexside] + AString(") to ") +
							newreg->ShortPrint(&regions) + AString("."));		
				}
  				ship->hexside = newhexside;
			    
			    //do post-move stuff
				if(Globals->TRANSIT_REPORT != GameDefs::REPORT_NOTHING && newreg != reg) {
					forlist(&ship->units) {
						// Everyone onboard gets to see the sights
						Unit *unit = (Unit *)elem;
						Farsight *f;
						// Note the hex being left
						forlist(&reg->passers) {
							f = (Farsight *)elem;
							if(f->unit == unit) {
								// We moved into here this turn
								f->exits_used[dir] = 1;
							}
						}
						// And mark the hex being entered
						f = new Farsight;
						f->faction = unit->faction;
						f->level = 0;
						f->unit = unit;
						f->exits_used[newreg->GetRealDirComp(dir)] = 1;
						newreg->passers.Add(f);
					}
				}
				reg = newreg; //updates for region returned.
				if (newreg->ForbiddenShip(ship)) {      //ship can be stopped once on each edge, as long as that edge is not a coastal edge (ie adjacent to sea) since in that case guards could hardly stop the ship.
				    if(!newreg->neighbors[newhexside] || TerrainDefs[newreg->neighbors[newhexside]->type].similar_type != R_OCEAN) {
    					captain->faction->Event(*ship->name +
    							AString(" is stopped by guards in ") +
    							newreg->ShortPrint(&regions) + AString("."));
    					goto done_sailing;
					}
   				}
			//end of the hexside ships stuff
			}
		}
    }


	/* Clear out move orders */
	forlist(&ship->units) {
		Unit * unit = (Unit *) elem;
		if (moved) {
            unit->alias = 0;
            if (firstmove && (unit == captain || (unit->monthorders && unit->monthorders->type == O_SAIL))) {     //captain might have FOLLOW orders
		    	unit->Practice(S_SAILING);
		    	unit->Experience(S_SAILING,10);
	    	}
    		if (unit->monthorders && unit->monthorders->type == O_MOVE) {
				delete unit->monthorders;
				unit->monthorders = 0;
			}
		}
	}
	return reg;

done_sailing:
    {
    	if(captain->movepoints > 0) moved = 1;  //we might not have moved this phase, but we moved in an earlier phase.
    	/* Clear out everyone's orders */
    	forlist(&ship->units) {
    		Unit * unit = (Unit *) elem;
    		if (moved) {
    			unit->alias = 0;
    		}
    
    		if (unit->monthorders) {
/*    		    if ((unit == captain || unit->monthorders->type == O_SAIL) && moved) {     //captain might have FOLLOW orders
    		    	unit->Practice(S_SAILING);
    		    	unit->Experience(S_SAILING,10);
    	    	}*/
    		    if ((moved && unit->monthorders->type == O_MOVE) ||        //I don't think a unit here could still have move orders, but just in case. Well, why not? MOVE N 123 N
    				unit->monthorders->type == O_SAIL) {
    				delete unit->monthorders;
    				unit->monthorders = 0;
    			}
    		}
    	}
    
    	return reg;  //why do we need to return a location if we haven't moved?
    }
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
	TeachOrder * order = (TeachOrder *) unit->monthorders;
	int quiet = order->quiet;
	
	/* First pass, find how many to teach */
	if(unit->IsNormal()) {
		/* small change to handle Ceran's mercs */
		if(!unit->GetMen(I_MERC)) {
			// Mercs can teach even though they are not leaders.
			// They cannot however improve their own skills
			unit->Error("TEACH: Normal units cannot teach.", quiet);
			return;
		}
	}

	int students = 0;
	forlist(&order->targets) {
		UnitId * id = (UnitId *) elem;
		Unit * target = reg->GetUnitId(id,unit->faction->num);
		if (!target) {
			order->targets.Remove(id);
			unit->Error("TEACH: No such unit.", quiet);
			delete id;
		} else {
			if (target->faction->GetAttitude(unit->faction->num) < A_FRIENDLY) {
				unit->Error(AString("TEACH: ") + *(target->name) +
							" is not a member of a friendly faction.", quiet);
				order->targets.Remove(id);
				delete id;
			} else {
				if ((!target->monthorders ||
					target->monthorders->type != O_STUDY ) && (!Globals->ARCADIA_MAGIC || !target->IsMage() || !target->herostudyorders) ) {
					unit->Error(AString("TEACH: ") + *(target->name) +
								" is not studying.", quiet);
					order->targets.Remove(id);
					delete id;
				} else {
                    int sk;
                    if(Globals->ARCADIA_MAGIC && target->IsMage() && target->herostudyorders) {
                        sk = ((StudyOrder *) target->herostudyorders)->skill;
                    } else sk = ((StudyOrder *) target->monthorders)->skill;
					if (unit->GetRealSkill(sk) <= target->GetRealSkill(sk) && unit->GetDaysSkill(sk) <= target->GetDaysSkill(sk)) {
					/* REAL_EXPERIENCE Patch: Can teach units with lower total skill OR lower days skill */
						unit->Error(AString("TEACH: ") +
									*(target->name) + " is not studying "
									"a skill you can teach.", quiet);
						order->targets.Remove(id);
						delete id;
					} else {
						// Check whether it's a valid skill to teach
						if ( (SkillDefs[sk].flags & SkillType::NOTEACH) || 
                            ((SkillDefs[sk].flags & SkillType::MAGIC) && Globals->CANT_TEACH_MAGIC) ) {
							unit->Error(AString("TEACH: ") + 
									AString(SkillDefs[sk].name) + 
									" cannot be taught.", quiet);
							return;
						} else {
							students += target->GetMen();
						}
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

			//int sk = ((StudyOrder *) u->monthorders)->skill;
            int sk;
            if(Globals->ARCADIA_MAGIC && u->IsMage() && u->herostudyorders) {
               sk = ((StudyOrder *) u->herostudyorders)->skill;
            } else sk = ((StudyOrder *) u->monthorders)->skill;

			int umen = u->GetMen();
			int tempdays = (umen * days) / students;
			if (tempdays > 30 * umen) tempdays = 30 * umen;

			/* REAL_EXPERIENCE Patch*/
/*
//removed this limitation (cannot be taught beyond teacher level) for Nylandor. Besides, it wasn't working properly. Check it out sometime, I think the problem is in the third line.

			int teacherexp = unit->GetExperSkill(sk) - u->GetExperSkill(sk); //if have higher experience, can teach student beyond own study level.
			if (teacherexp < 0 ) teacherexp = 0; //this is 0,1,2,3 - ie the level difference
			int daysaboveteacher = u->skills.GetDays(sk) + tempdays - umen * GetDaysByLevel(unit->GetDaysSkill(sk) + teacherexp);
			if(daysaboveteacher > 0) tempdays -= daysaboveteacher;
*/			
			if(tempdays < 0) tempdays = 0; // shouldn't be necessary, but just in case.

			days -= tempdays;  //reducing the teacher attributes of how many people he can teach!
			students -= umen;

			StudyOrder * o = (StudyOrder *) u->monthorders;
			if(Globals->ARCADIA_MAGIC && u->herostudyorders) o = (StudyOrder *) u->herostudyorders;
			
			o->days += tempdays;
			if (o->days > 30 * umen) // this bit returns any spare days to the teacher
			{
				days += o->days - 30 * umen;
				o->days = 30 * umen;
			}
			unit->Event(AString("Teaches ") + SkillDefs[o->skill].name +
						" to " + *u->name + ".");
			// The TEACHER may learn something in this process!
			unit->Practice(o->skill);		
		}
	//Recycle any spare teaching ability to stop multi-teaching only giving 29 days due to rounding down errors
	    if(days > 0) {
    		forlist_reuse(&order->targets) {
    		    if(days < 1) continue; //we've allocated it all, break out.
    		    
    			UnitId * id = (UnitId *) elem;
    			Unit * u = reg->GetUnitId(id,unit->faction->num);

    			//int sk = ((StudyOrder *) u->monthorders)->skill;
                int sk;
                if(Globals->ARCADIA_MAGIC && u->IsMage() && u->herostudyorders) {
                   sk = ((StudyOrder *) u->herostudyorders)->skill;
                } else sk = ((StudyOrder *) u->monthorders)->skill;
    
    			int umen = u->GetMen();
    
    			StudyOrder * o = (StudyOrder *) u->monthorders;
    			if(Globals->ARCADIA_MAGIC && u->herostudyorders) o = (StudyOrder *) u->herostudyorders;

    			if (o->days < 30 * umen) // this unit can still be taught
    			{
    				o->days += days;
    				days = 0;
    			}
    			//check if we've given too much:
    			if (o->days > 30 * umen) // this bit returns any spare days to the teacher
    			{
    				days += o->days - 30 * umen;
    				o->days = 30 * umen;
    			}
    		}
	    }
	}
	
}

/* Hexside Patch 030825 BS */
int Game::HexsideCanGoThere(ARegion * r,Object * obj,Unit * u)
{
    int dir = obj->hexside;
    if(dir < 0 || dir > 5) return 0;

/* Ship may be built on sailable hexside of correct depth */
    int depth = HexsideDefs[r->hexside[dir]->type].sailable;
    if(depth < 1) return 0;
    int shiptype = ObjectDefs[obj->type].sailable;
    if(depth == shiptype || depth == 3 || shiptype == 3) return 1;

    return 0;
}

void Game::Run1BuildOrder(ARegion * r,Object * obj,Unit * u)
{
    int quiet = u->monthorders->quiet;

	if (!TradeCheck(r, u->faction)) {
		u->Error("BUILD: Faction can't produce in that many regions.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	AString skname = ObjectDefs[obj->type].skill;
	int sk = LookupSkill(&skname);
	if (sk == -1) {
		u->Error("BUILD: Can't build that.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int usk = u->GetSkill(sk);
	if (usk < ObjectDefs[obj->type].level) {
		u->Error("BUILD: Can't build that.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}
	if ((Globals->ALLOW_BANK & GameDefs::BANK_ENABLED) && (obj->type == O_OBANK)) { // trying to build a bank ?
		if ((Globals->ALLOW_BANK & GameDefs::BANK_NOTONGUARD) && !(r->CanTax(u))) {
			u->Error("BUILD: Cannot build banks if there are guarding units.", quiet);
			delete u->monthorders;
			u->monthorders = 0;
			return;
		}
		if (!r->town && (Globals->ALLOW_BANK & GameDefs::BANK_INSETTLEMENT)) {
			u->Error("BUILD: Cannot build banks outside settlements.", quiet);
			delete u->monthorders;
			u->monthorders = 0;
			return;
		}
		if ((Globals->ALLOW_BANK & GameDefs::BANK_SKILLTOBUILD) && (SkillDefs[S_BANKING].flags & SkillType::DISABLED)) {
			// GM error - requested banking skill to build but skill is disabled
			u->Error("BUILD: Impossible to build banks due to missing skill.", quiet);
			delete u->monthorders;
			u->monthorders = 0;
			return;
		}
		if ((Globals->ALLOW_BANK & GameDefs::BANK_SKILLTOBUILD) && (!u->GetSkill(S_BANKING))) {
			u->Error("BUILD: Can't build that.", quiet);
			delete u->monthorders;
			u->monthorders = 0;
			return;
		}
	} else if (obj->type == O_OBANK) { // This is only if a dumb GM enables banks but not the gamedef
		u->Error("BUILD: Bank ? What is that ?", quiet); // maybe give same error "Can't build that." ?
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}


	int needed = obj->incomplete;
	int type = obj->type;
	// AS
	if(((ObjectDefs[type].flags&ObjectType::NEVERDECAY) || !Globals->DECAY) &&
			needed < 1) {
		u->Error("BUILD: Object is finished.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	// AS
	if(needed <= -(ObjectDefs[type].maxMaintenance)) {
		u->Error("BUILD: Object does not yet require maintenance.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}


/* Hexside Patch 030825 BS */
   	if(Globals->HEXSIDE_TERRAIN) {
    	if(ObjectDefs[type].hexside && obj->hexside==-1) {
    	u->Error("BUILD: Problem type 1 with hexside terrain, please contact your GM", 0);
		delete u->monthorders;
		u->monthorders = 0;
		return;
    	}
    	if(!ObjectDefs[type].hexside && obj->hexside>-1) {
    	u->Error("BUILD: Problem type 2 with hexside terrain, please contact your GM", 0);
		delete u->monthorders;
		u->monthorders = 0;
		return;
    	}
    	if(ObjectDefs[type].hexside) {
    	    if (!HexsideCanGoThere(r,obj,u)) {
    	    u->Error("Build: That structure cannot not be built there.", quiet);
    		delete u->monthorders;
    		u->monthorders = 0;
    		return;
    	    }
    	}
	}
	int it = ObjectDefs[type].item;
	int itn;
	if (it == I_WOOD_OR_STONE) {
		itn = u->GetSharedNum(I_WOOD) + u->GetSharedNum(I_STONE);
	} else {
		itn = u->GetSharedNum(it);
	}

	if (itn == 0) {
		u->Error("BUILD: Don't have the required item.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int num = u->GetMen() * usk;

	// JLT
	if(obj->incomplete == ObjectDefs[type].cost) {
		if(ObjectIsShip(type)) {
			obj->num = shipseq++;
			obj->SetName(new AString("Ship"));
		} else {
			obj->num = u->object->region->buildingseq++;
			obj->SetName(new AString("Building"));
		}
	}

	// Hack to fix bogus ship numbers
	if(ObjectIsShip(type) && obj->num < 100) {
		obj->num = shipseq++;
		obj->SetName(new AString("Ship"));
	}

	// AS
	AString job;
	if (needed < 1) {
		// This looks wrong, but isn't.
		// If a building has a maxMaintainence of 75 and the road is at
		// -70 (ie, 5 from max) then we want the value of maintMax to be
		// 5 here.  Then we divide by maintFactor (some things are easier
		// to refix than others) to get how many items we need to fix it.
		// Then we fix it by that many items * maintFactor
		int maintMax = ObjectDefs[type].maxMaintenance + needed;
		maintMax /= ObjectDefs[type].maintFactor;
		if (num > maintMax) num = maintMax;
		if (itn < num) num = itn;
		job = " maintenance ";
		obj->incomplete -= (num * ObjectDefs[type].maintFactor);
		if (obj->incomplete < -(ObjectDefs[type].maxMaintenance))
			obj->incomplete = -(ObjectDefs[type].maxMaintenance);
	} else if(needed > 0) {
		if (num > needed) num = needed;
		if (itn < num) num = itn;
		job = " construction ";
		obj->incomplete -= num;
		if (obj->incomplete == 0) {
			obj->incomplete = -(ObjectDefs[type].maxMaintenance);
		}
	}

	/* Perform the build */
	u->MoveUnit(obj);   //BS: I don't think this should be there. obj has been imported as the object the unit is in, so it does not need to be moved there!

	if (it == I_WOOD_OR_STONE) {
		if (num > u->items.GetNum(I_STONE)) {
			num -= u->items.GetNum(I_STONE);
			u->items.SetNum(I_STONE,0);
		} else {
			u->items.SetNum(I_STONE,u->items.GetNum(I_STONE) - num);
			num = 0;
		}
		if (num > u->items.GetNum(I_WOOD)) {
			num -= u->items.GetNum(I_WOOD);
			u->items.SetNum(I_WOOD,0);
		} else {
			u->items.SetNum(I_WOOD,u->items.GetNum(I_WOOD) - num);
			num = 0;
		}
		int sharedstone = u->GetSharedNum(I_STONE);
		if (num > sharedstone) {
			num -= sharedstone;
			u->ConsumeShared(I_STONE, sharedstone);
			u->ConsumeShared(I_WOOD, num);
		} else {
			u->ConsumeShared(I_STONE, num);
		}
	} else {
		u->ConsumeShared(it,num);
	}

	// AS
	u->Event(AString("Performs") + job + "on " + *(obj->name) + ".");
	u->Practice(sk);
	u->Experience(sk,10);

	delete u->monthorders;
	u->monthorders = 0;
}

void Game::Run1BuildHexsideOrder(ARegion * r,Object * obj,Unit * u)
{
    int quiet = u->monthorders->quiet;

    if(!Globals->HEXSIDE_TERRAIN) return;

	if (!TradeCheck(r, u->faction)) {
		u->Error("BUILD: Faction can't produce in that many regions.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	BuildHexsideOrder *o = (BuildHexsideOrder *)u->monthorders;
	AString skname = HexsideDefs[o->terrain].skill;
	int sk = LookupSkill(&skname);
	if (sk == -1) {
		u->Error("BUILD: Can't build that.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int usk = u->GetSkill(sk);
	if (usk < HexsideDefs[o->terrain].level) {
		u->Error("BUILD: Can't build that.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	Hexside *h = r->hexside[o->direction];
	if(!h) {
	    //This should never occur.
	    u->Error("BUILD: Hexside does not exist. Contact your GM.", 0);
		delete u->monthorders;
		u->monthorders = 0;	    
	    return;
    }

	int needed = 0;
	// If hexside terrain gets expanded to other types, have to redo this!
	if(o->terrain == H_ROAD) {
	    if(h->road < 0) {
	        u->Error("BUILD: Road is finished.", quiet);
	        delete u->monthorders;
	        u->monthorders = 0;
	        return;
        }
        if(!r->neighbors[o->direction] || TerrainDefs[r->neighbors[o->direction]->type].similar_type == R_OCEAN) {
            u->Error("BUILD: A road cannot be built there.", quiet);
            delete u->monthorders;
            u->monthorders = 0;
            return;
        }
        if(h->road == 0) h->road = HexsideDefs[o->terrain].cost;
        needed = h->road;
    }
	else if(o->terrain == H_BRIDGE) {
	    //since bridge blockeffect = -1
	    if(HexsideDefs[h->type].blockeffect != 1 && h->bridge <= 0) {   //allow finishing a bridge if partially completed then dive moved the river.
	        u->Error("BUILD: Nothing to bridge.", quiet);
	        delete u->monthorders;
	        u->monthorders = 0;
	        return;
	    }
	    if(h->bridge < 0) {
	        u->Error("BUILD: Bridge is finished.", quiet);
	        delete u->monthorders;
	        u->monthorders = 0;
	        return;
        }
        if(h->bridge == 0) h->bridge = HexsideDefs[o->terrain].cost;
        needed = h->bridge;
    }
	else if(o->terrain == H_HARBOUR) {
	    if(h->type != H_BEACH) {
	        u->Error("BUILD: Harbours can only be built on beaches.", quiet);
	        delete u->monthorders;
	        u->monthorders = 0;
	        return;
	    }
	    if(h->harbour < 0) {
	    //this should never occur
	        u->Error("BUILD: Harbour appears to be finished. Contact your GM.", 0);
	        delete u->monthorders;
	        u->monthorders = 0;
	        return;
        }
        if(h->harbour == 0) h->harbour = HexsideDefs[o->terrain].cost;
        needed = h->harbour;
    }
    else {
        u->Error("BUILD: Cannot BUILD that!", quiet);
	        delete u->monthorders;
	        u->monthorders = 0;
	        return;
    }

	int it = HexsideDefs[o->terrain].item;
	int itn;
	if (it == I_WOOD_OR_STONE) {
		itn = u->GetSharedNum(I_WOOD) + u->GetSharedNum(I_STONE);
	} else {
		itn = u->GetSharedNum(it);
	}

	if (itn == 0) {
		u->Error("BUILD: Don't have the required item.", quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int num = u->GetMen() * usk;

	if (num > needed) num = needed;
	if (itn < num) num = itn;
    if(o->terrain == H_ROAD) {
        h->road -= num;
        if(h->road == 0) h->road = -1;
        if(h->road >= HexsideDefs[o->terrain].cost) h->road = 0;
    } else if(o->terrain == H_BRIDGE) {
        h->bridge -= num;
        if(h->bridge == 0) h->bridge = -1;
        if(h->bridge >= HexsideDefs[o->terrain].cost) h->bridge = 0;        
    } else if(o->terrain == H_HARBOUR) {
        h->harbour -= num;
        if(h->harbour >= HexsideDefs[o->terrain].cost) h->harbour = 0;  
        if(h->harbour <= 0) {
        //harbour is finished.
            h->harbour = 0;
            h->type = H_HARBOUR;
        }
    }

	/* Deduct the items */
	
	u->MoveUnit(r->GetDummy());

	if (it == I_WOOD_OR_STONE) {

		if (num > u->items.GetNum(I_STONE)) {
			num -= u->items.GetNum(I_STONE);
			u->items.SetNum(I_STONE,0);
		} else {
			u->items.SetNum(I_STONE,u->items.GetNum(I_STONE) - num);
			num = 0;
		}
		if (num > u->items.GetNum(I_WOOD)) {
			num -= u->items.GetNum(I_WOOD);
			u->items.SetNum(I_WOOD,0);
		} else {
			u->items.SetNum(I_WOOD,u->items.GetNum(I_WOOD) - num);
			num = 0;
		}
		int sharedstone = u->GetSharedNum(I_STONE);
		if (num > sharedstone) {
			num -= sharedstone;
			u->ConsumeShared(I_STONE, sharedstone);
			u->ConsumeShared(I_WOOD, num);
		} else {
			u->ConsumeShared(I_STONE, num);
		}
	} else {
		u->ConsumeShared(it,num);
	}

	// AS
	u->Event(AString("Performs construction on ") + (HexsideDefs[o->terrain].name) + ".");
	u->Practice(sk);
    u->Experience(sk,10);

	delete u->monthorders;
	u->monthorders = 0;
}

void Game::RunBuildHelpers(ARegion *r)
{
	forlist((&r->objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->monthorders) {
				if (u->monthorders->type == O_BUILD) {
					BuildOrder *o = (BuildOrder *)u->monthorders;
					Object *tarobj = NULL;
					if(o->target) {
						Unit *target = r->GetUnitId(o->target,u->faction->num);
						if(!target) {
							u->Error("BUILD: No such unit to help.", o->quiet);
							delete u->monthorders;
							u->monthorders = 0;
							continue;
						}
						// Make sure that unit is building
						if (target->monthorders &&
								target->monthorders->type != O_BUILD) {
							u->Error("BUILD: Unit isn't building.", o->quiet);
							delete u->monthorders;
							u->monthorders = 0;
							continue;
						}
						// Make sure that unit considers you friendly!
						if(target->faction->GetAttitude(u->faction->num) <
								A_FRIENDLY) {
							u->Error("BUILD: Unit you are helping rejects "
									"your help.", o->quiet);
							delete u->monthorders;
							u->monthorders = 0;
							continue;
						}
						tarobj = target->build;
						if (tarobj == NULL) tarobj = target->object;
						if((tarobj != NULL) && (u->object != tarobj))
							u->MoveUnit(tarobj);
					} else if (u->build != NULL && u->build != u->object) {
						u->MoveUnit(u->build);
					}
				}
			}
		}
	}
}


void Game::RunMonthOrders()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;

		RunIdleOrders(r);
		RunStudyOrders(r);
		RunBuildHelpers(r);
		RunProduceOrders(r);
	}
	if(Globals->ARCADIA_MAGIC) RunMasterOrders();
}

void Game::RunUnitProduce(ARegion * r,Unit * u)
{
	ProduceOrder * o = (ProduceOrder *) u->monthorders;

	if (o->item == I_SILVER) {
		u->Error("Can't do that in this region.", o->quiet);
//u->Error(AString("Can't do that in this region. Error Code: ") + o->skill + " " + o->productivity, o->quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int input = ItemDefs[o->item].pInput[0].item;
	if (input == -1) {
		u->Error("PRODUCE: Can't produce that.", o->quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int level = u->GetSkill(o->skill);
	if (level < ItemDefs[o->item].pLevel) {
		u->Error("PRODUCE: Can't produce that.", o->quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	if (!TradeCheck(r, u->faction)) {
		u->Error("PRODUCE: Faction can't produce in that many regions.", o->quiet);
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	// LLS
	int number = u->GetMen() * level + u->GetProductionBonus(o->item);

	// find the max we can possibly produce based on man-months of labor
	int maxproduced;
	if (ItemDefs[o->item].flags & ItemType::SKILLOUT)
		maxproduced = u->GetMen();
	else
		maxproduced = number/ItemDefs[o->item].pMonths;

	if (ItemDefs[o->item].flags & ItemType::ORINPUTS) {
		// Figure out the max we can produce based on the inputs
		int count = 0;
		unsigned int c;
		for(c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			if(i != -1)
				count += u->GetSharedNum(i) / ItemDefs[o->item].pInput[c].amt;
		}
		if (maxproduced > count)
			maxproduced = count;
		count = maxproduced;
		
		// Deduct the items spent
		for(c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			int a = ItemDefs[o->item].pInput[c].amt;
			if(i != -1) {
				int amt = u->items.GetNum(i);
				if (count > amt / a) {
					count -= amt / a;
					u->items.SetNum(i, amt-(amt/a)*a);
				} else {
					u->items.SetNum(i, amt - count * a);
					count = 0;
				}
			}
		}
		// Recycle with sharing if necessary
		for(c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			int a = ItemDefs[o->item].pInput[c].amt;
			if(count && i != -1) {
				int amt = u->GetSharedNum(i);
				if (count > amt / a) {
					count -= amt / a;
					u->ConsumeShared(i, (amt/a)*a);
				} else {
					u->ConsumeShared(i, count * a);
					count = 0;
				}
			}
		}
	} else {
		// Figure out the max we can produce based on the inputs
		unsigned int c;
		for(c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			if(i != -1) {
				int amt = u->GetSharedNum(i);
				if(amt/ItemDefs[o->item].pInput[c].amt < maxproduced) {
					maxproduced = amt/ItemDefs[o->item].pInput[c].amt;
				}
			}
		}
		
		// Deduct the items spent
		for(c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			int a = ItemDefs[o->item].pInput[c].amt;
			if(i != -1) {
				u->ConsumeShared(i, maxproduced*a);
			}
		}
	}

	// Now give the items produced
	int output = maxproduced * ItemDefs[o->item].pOut;
	if (ItemDefs[o->item].flags & ItemType::SKILLOUT)
		output *= level;
	//u->items.SetNum(o->item,u->items.GetNum(o->item) + output);
	//mod for Xanaxor games. Produced items go into list credited later to prevent SHARE reusing them for secondary prod:
	u->itemsintransit.SetNum(o->item, u->itemsintransit.GetNum(o->item) + output);
	
	u->Event(AString("Produces ") + ItemString(o->item,output) + " in " +
			r->ShortPrint(&regions) + ".");
	u->Practice(o->skill);
	u->Experience(o->skill,10);
	delete u->monthorders;
	u->monthorders = 0;
}

void Game::RunProduceOrders(ARegion * r)
{
	forlist ((&r->products))
		RunAProduction(r,(Production *) elem);
	{
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist ((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->monthorders) {
					if (u->monthorders->type == O_PRODUCE) RunUnitProduce(r,u);
					else if (u->monthorders->type == O_BUILD) Run1BuildOrder(r,obj,u);
					else if (u->monthorders->type == O_BUILDHEXSIDE) Run1BuildHexsideOrder(r,obj,u);
				}
			}
		}
	}
}

int Game::ValidProd(Unit * u,ARegion * r, Production * p)
{
	if (u->monthorders->type != O_PRODUCE) return 0;

	ProduceOrder * po = (ProduceOrder *) u->monthorders;
	if (p->itemtype == po->item && p->skill == po->skill) {
		if (p->skill == -1) {
			po->productivity = u->GetMen() * p->productivity;
			return po->productivity;
		}
		int level = u->GetSkill(p->skill);
		//	if (level < p->level) {
		if (level < ItemDefs[p->itemtype].pLevel) {
			u->Error("PRODUCE: Unit isn't skilled enough.", po->quiet);
			delete u->monthorders;
			u->monthorders = 0;
			return 0;
		}

		//
		// Check faction limits on production. If the item is silver, then the
		// unit is entertaining or working, and the limit does not apply
		//
		if (p->itemtype != I_SILVER && !TradeCheck(r, u->faction)) {
			u->Error("PRODUCE: Faction can't produce in that many regions.", po->quiet);
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

int Game::FindAttemptedProd(ARegion * r,Production * p)
{
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
	int attempted = FindAttemptedProd(r,p); //This eliminates 'illegal' production orders
	int amt = p->amount;
	if (attempted < amt) attempted = amt;
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			if(!u->monthorders || u->monthorders->type != O_PRODUCE)
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
			u->itemsintransit.SetNum(po->item, u->itemsintransit.GetNum(po->item) + ubucks);
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
							 + r->ShortPrint(&regions) + ".");
				}
				else
				{
					//
					// ENTERTAIN
					//
					u->Event(AString("Earns ") + ubucks
							 + " silver entertaining in " +
							 r->ShortPrint(&regions)
							 + ".");
					u->Practice(S_ENTERTAINMENT);
					if(ubucks) u->Experience(S_ENTERTAINMENT,10);
					if(u->GetSkill(S_PHANTASMAL_ENTERTAINMENT)) {
    					u->Practice(S_PHANTASMAL_ENTERTAINMENT);
    					if(ubucks) u->Experience(S_PHANTASMAL_ENTERTAINMENT,10);
					}
				}
			}
			else
			{
				/* Everything else */
				u->Event(AString("Produces ") + ItemString(po->item,ubucks) +
						 " in " + r->ShortPrint(&regions) + ".");
				u->Practice(po->skill);
				if(ubucks) u->Experience(po->skill,10);
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
			if(Globals->ARCADIA_MAGIC && u->IsMage() && u->herostudyorders) {
                Do1StudyOrder(u,obj);
                delete u->herostudyorders;
                u->herostudyorders = 0;
            }
		}
	}
}

void Game::RunIdleOrders(ARegion *r)
{
	forlist((&r->objects)) {
		Object *obj = (Object *)elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *)elem;
			if (u->monthorders && u->monthorders->type == O_IDLE) {
				u->Event("Sits idle.");
				delete u->monthorders;
				u->monthorders = 0;
			}
		}
	}
}

void Game::Do1StudyOrder(Unit *u,Object *obj)
{
	StudyOrder * o;
    if(u->monthorders && u->monthorders->type == O_STUDY) o = (StudyOrder *) u->monthorders;
    else o = u->herostudyorders; //ARCADIA_MAGIC MOD  
    
	int sk = o->skill;
	int cost = SkillCost(sk) * u->GetMen();
	if (!u->GetSharedMoney(cost)) {
		u->Error("STUDY: Not enough funds.", o->quiet);
		return;
	}

	// Check that the skill can be studied
	if (SkillDefs[sk].flags & SkillType::NOSTUDY) {
		u->Error( AString("STUDY: ") + AString(SkillDefs[sk].name) + " cannot be studied.", o->quiet);
		return;
	}
	
	// Small patch for Ceran Mercs
	if(u->GetMen(I_MERC)) {
		u->Error("STUDY: Mercenaries are not allowed to study.", o->quiet);
		return;
	}

	if((SkillDefs[sk].flags & SkillType::MAGIC) && u->type != U_MAGE) {
		u->Error("STUDY: That skill can only be studied by heroes.", o->quiet);
	}
	
	if(sk == S_HEROSHIP && Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
		if (CountMages(u->faction) >= AllowedMages(u->faction)) {
		    u->Error("STUDY: Faction has too many heroes.", o->quiet);
	        return;
        }
    }

	if((SkillDefs[sk].flags&SkillType::APPRENTICE) && u->type != U_APPRENTICE) {
		u->Error("STUDY: That skill can only be studied by apprentices.", o->quiet);
	}

//TO CHECK:
	if ((Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) &&
			(sk == S_QUARTERMASTER) && (u->GetRealSkill(S_QUARTERMASTER) == 0) &&
			(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)) {
			if (CountQuarterMasters(u->faction) >=
					AllowedQuarterMasters(u->faction)) {
				u->Error("STUDY: Can't have another quartermaster.", o->quiet);
				return;
			}
			if(u->GetMen() != 1) {
				u->Error("STUDY: Only 1-man units can be quartermasters.", o->quiet);
				return;
			}
	}

	// If TACTICS_NEEDS_WAR is enabled, and the unit is trying to study to tact-5,
	// check that there's still space...
	if (Globals->TACTICS_NEEDS_WAR && sk == S_TACTICS && 
			u->GetRealSkill(sk) == 4 && u->skills.GetDays(sk)/u->GetMen() >= 390) {
		
		if (CountTacticians(u->faction) >=
				AllowedTacticians(u->faction)) {
			u->Error("STUDY: Can't have another level 5 tactics leader.", o->quiet);
			return;
		}

		if (u->GetMen() != 1) {
			u->Error("STUDY: Only 1-man units can study to level 5 in tactics.", o->quiet);
			return;
		}
		
	} // end tactics check

	int days = 30 * u->GetMen();
	int taughtdays = o->days;

	if((SkillDefs[sk].flags & SkillType::MAGIC) && u->GetRealSkill(sk) >= 2 && !Globals->ARCADIA_MAGIC) {
		if(Globals->LIMITED_MAGES_PER_BUILDING) {
			if (obj->incomplete > 0 || obj->type == O_DUMMY) {
				u->Error("Warning: Magic study rate outside of a building "
						"cut in half above level 2.", o->quiet);
				days /= 2;
				taughtdays /= 2;
			} else if(obj->mages == 0) {
				u->Error("Warning: Magic rate cut in half above level 2 due "
						"to number of mages studying in structure.", o->quiet);
				days /= 2;
				taughtdays /= 2;
			} else {
				obj->mages--;
			}
		} else if(!(ObjectDefs[obj->type].protect) || (obj->incomplete > 0)) {
			u->Error("Warning: Magic study rate outside of a building cut in "
					"half above level 2.", o->quiet);
			days /= 2;
				taughtdays /= 2;
		}
	}

	if(SkillDefs[sk].flags & SkillType::SLOWSTUDY) {
		days /= 2;
		taughtdays /= 2;
	}


	if (u->Study(sk,days,o->quiet)) {
	    if(taughtdays) u->Study(sk,taughtdays,0,0);  //the second 0 means any taught knowledge does not overflow to experience
		u->ConsumeSharedMoney(cost);                         //We checked earlier that we can get this
		AString str("Studies ");
		str += SkillDefs[sk].name;
		int teachdays = o->days/u->GetMen();
		if (teachdays) {
			str += " and was taught for ";
			str += teachdays;
			str += " days";
		}
		str += ".";
		u->Event(str);
	} else return; //return if could not study.
	
	if(o->level && u->GetRealSkill(sk) < o->level) { //STUDY order mod
    	TurnOrder *tOrder = new TurnOrder;
    	AString order;
    	tOrder->repeating = 0;
    	order = AString("STUDY ") + SkillDefs[sk].abbr + " " + o->level;
    	tOrder->turnOrders.Add(new AString(order));
    	u->turnorders.Insert(tOrder);
	}
}

void Game::ClearCombatMovementMaluses()
{
	forlist((&regions)) {
		ARegion * region = (ARegion *) elem;
		forlist((&region->objects)) {
			Object * obj = (Object *) elem;
			forlist(&obj->units) {
				Unit * unit = (Unit *) elem;
				unit->movementmalus = 0;
			}
		}
	}
}

void Game::RunMoveOrders()
{
// This follow code is very messy. I don't recommend using it unless, like me,
// you are pushed for time. A better method would be to link all followers to
// the unit they are following, and get them to move immediately after
// the target moves (or is a unit on a ship that sails).


	for (int phase = 0; phase < Globals->MAX_SPEED; phase++) {

	    ClearCombatMovementMaluses();

		forlist((&regions)) {
			ARegion * region = (ARegion *) elem;
			forlist((&region->objects)) {
				Object * obj = (Object *) elem;
				forlist(&obj->units) {
					Unit * unit = (Unit *) elem;
					unit->marker = 0; //used in setupfollowers.
					Object *tempobj = obj;
					if(phase == unit->movepoints) DoMoveEnter(unit,region,&tempobj);  //the phase check is added on for sail during move, to avoid gaining movepoints by boarding a ship
				}
			}
		}
		SetupFollowers(phase); //this assigns a unit for 'followers' to follow, and ensures that no loops of followers creep in.
		AList * locations = new AList;
		AList * shipping = new AList;
		forlist_reuse((&regions)) {
			ARegion * region = (ARegion *) elem;
			forlist((&region->objects)) {
				Object * obj = (Object *) elem;
				forlist(&obj->units) {
					Unit * unit = (Unit *) elem;
					if (phase == unit->movepoints && unit->monthorders &&
						(unit->monthorders->type == O_MOVE ||
						 unit->monthorders->type == O_ADVANCE) &&
						!unit->nomove) {
						locations->Add(DoAMoveOrder(unit,region,obj));
					}
				}
			}
			SailShips(region, phase, shipping);
		    //now, have to move all followers!
			int numfollowers;
			do {
			    numfollowers = 0;
    			forlist((&region->objects)) {
    				Object * obj = (Object *) elem;
    				forlist(&obj->units) {
    					Unit * unit = (Unit *) elem;
    					if(phase >= unit->movepoints && unit->monthorders && 
                                unit->monthorders->type == O_FOLLOW &&
                                ((FollowOrder *)unit->monthorders)->target && 
                                !unit->nomove ) {
                            //this unit has follow orders.
                            FollowOrder *o = (FollowOrder *) unit->monthorders;
                            if(o->target->monthorders && o->target->monthorders->type == O_FOLLOW &&
                                ((FollowOrder *)o->target->monthorders)->target) numfollowers++; //this unit's target has yet to try and follow it's own target, so don't process this unit's follow order yet.
                            else if(o->target->object->region == unit->object->region) {
                                //The target unit has not moved, so clear our follow order.
                                o->target = 0;
                            } else {
                                //This unit's target has moved, so we want to follow it with a MOVE or SAIL order.
                                int initialpoints = unit->movepoints;
                                unit->movepoints = phase; //cannot proceed 'before' the unit it is following. This may lead to
                                        //situations where a following unit cannot join battles even though it does not move.
                                if(obj->IsBoat() && obj->GetOwner() == unit) {
                                    //sail order
                                    if(obj->incomplete < 1) {
                        				ARegionPtr * p = new ARegionPtr;
                        				p->ptr = DoASailOrder(region,obj,unit);
                        				shipping->Add(p);
                        				o->target = 0; //since we have moved.
                    				} else {
                    				    unit->Error("SAIL: Ship is not finished.", o->quiet);
                    				    delete o;
                    				    unit->monthorders = 0;
                    				    o = 0;
                    				}
                                } else {
                                    //move order
                                    locations->Add(DoAMoveOrder(unit,region,obj));
                                    o->target = 0;
                                }
                                if(unit->movepoints == phase) unit->movepoints = initialpoints;
                            }
					    } else if(unit->monthorders && unit->monthorders->type == O_FOLLOW) {
					        if(unit->nomove) {
					            FollowOrder *o = (FollowOrder *) unit->monthorders;
					            delete o;
					            unit->monthorders = 0;
					        } else {
					            FollowOrder *o = (FollowOrder *) unit->monthorders;
					            o->target = 0;
					        }
					    }
				    }
			    }
			} while (numfollowers);
		}

		DoAdvanceAttacks(locations);
		locations->DeleteAll();

		forlist_reuse(shipping) {
			ARegion * r2 = ((ARegionPtr *) elem)->ptr;
			DoAutoAttacksRegion(r2);
		}
		shipping->DeleteAll();
	}
	
	forlist((&regions)) {
		ARegion * region = (ARegion *) elem;
		forlist((&region->objects)) {
			Object * obj = (Object *) elem;
			forlist(&obj->units) {
				Unit * unit = (Unit *) elem;
                if(unit->monthorders && unit->monthorders->type == O_FOLLOW) {
                    FollowOrder *o = (FollowOrder *) unit->monthorders;
                    delete o;
                    unit->monthorders = 0;
                }
			}
		}
	}
}

void Game::SetupFollowers(int phase)
{
	forlist(&regions) {
	    ARegion *region = (ARegion *) elem;
	    forlist(&region->objects) {
			Object * obj = (Object *) elem;
			forlist(&obj->units) {
			    Unit *unit = (Unit *) elem;
			    if(unit->monthorders && unit->monthorders->type == O_FOLLOW) {
    			    if(phase >= unit->movepoints && !unit->nomove) {
    			    
    			        FollowOrder *o = (FollowOrder *) unit->monthorders;
    			        o->advancing = 0;
    			        Unit *tar = 0;
    			        if(o->ship) {
    			            Object *pship = region->GetObject(o->ship);
    			            if(pship) {
    			                forlist(&pship->units) {
    			                    Unit *u = (Unit *) elem;
    			                    //skip all units which don't have men, or have orders to move
    			                    if(!u->GetMen()) continue;
    			                    if(u->monthorders && (u->monthorders->type == O_MOVE || u->monthorders->type == O_ADVANCE)) continue;
    			                    //this unit is our only possible captain to sail without battles - since he's not moving out.
    			                    if(u->monthorders && (u->monthorders->type == O_SAIL || u->monthorders->type == O_FOLLOW)) {
    			                        tar = u;
    			                    }
    			                    break;
    			                }
    			            }
    			        } else {
                            tar = region->GetUnitId(o->targetid, unit->faction->num);
                            if(tar && !o->targetid->unitnum) o->targetid->unitnum = tar->num; //updates "NEW" targets so we can follow them beyond one hex.
                            if(!tar && o->targetid && o->targetid->unitnum) {
                                int done;
                                do {
                                    done = 0;
                                    forlist(&region->hell) {
                                        Unit *deadu = (Unit *) elem;
                                        if(!done && deadu->num == o->targetid->unitnum) {
                                            if(deadu->gavemento.First()) {
                                                o->targetid->unitnum = ((UnitId *) deadu->gavemento.First())->unitnum;
                                                done = 1;
                                            }
                                        }
                                    }
                                    if(done) tar = region->GetUnitId(o->targetid, unit->faction->num);
                                } while(done && !tar);
                            }
                        }
/*                        if(tar && ObjectIsShip(tar->object->type) && tar->object != unit->object && (!tar->monthorders || (tar->monthorders->type != O_MOVE && tar->monthorders->type != O_ADVANCE && tar->monthorders->type != O_SAIL && tar->monthorders->type != O_FOLLOW))) {
                            //this target unit is not moving, but the ship he is on may be.
                            forlist(&tar->object->units) {
			                    Unit *u = (Unit *) elem;
			                    //skip all units which don't have men, or have orders to move
			                    if(!u->GetMen()) continue;
			                    if(u->monthorders && (u->monthorders->type == O_MOVE || u->monthorders->type == O_ADVANCE)) continue;
			                    //this unit is our only possible captain to sail without battles - since he's not moving out.
			                    if(u->monthorders && u->monthorders->type == O_SAIL) {
			                        tar = u;
			                    }
			                    break;
                            }
                        }*/
                        
                        if(tar && tar->monthorders && !tar->nomove) {
                            o->target = tar;  //set the unit to follow the unit specified.
                            Unit * finaltar = tar;
                            while(finaltar && finaltar->monthorders && finaltar->monthorders->type == O_FOLLOW && !finaltar->nomove) {
                                if( ((FollowOrder *)finaltar->monthorders)->ship ) {
            			            Object *pship = region->GetObject(((FollowOrder *)finaltar->monthorders)->ship);
            			            finaltar = 0;
            			            if(pship) {
            			                forlist(&pship->units) {
            			                    Unit *u = (Unit *) elem;
            			                    if(!u->GetMen()) continue;
            			                    if(u->monthorders && (u->monthorders->type == O_MOVE || u->monthorders->type == O_ADVANCE)) continue;
            			                    if(u->monthorders && (u->monthorders->type == O_SAIL || u->monthorders->type == O_FOLLOW)) finaltar = u;
            			                    break;
            			                }
            			            }
                                } else finaltar = region->GetUnitId(((FollowOrder *)finaltar->monthorders)->targetid, finaltar->faction->num);
                                if(finaltar == unit) finaltar = 0; //if get a loop of follow orders, then no-one moves.
                                if(finaltar) {
                                    if(finaltar->marker == unit->num) finaltar = 0;
                                    else finaltar->marker = unit->num; //mark each unit, if we get back to it we have looped.
                                }
                            }
                            //if we are not a ship captain, try to enter the object the ultimate target is in, but only if all intermediary targets can do so also.
                            //I am assuming here that there is no drowning issue with moving out of a ship over water. If there is, someone else can code it!
                            //Ship captains try to follow their target by sailing not moving.
                            int iscaptain = 0;
                            if(unit->object->IsBoat() && unit->object->GetOwner() == unit) iscaptain = 1;
                            int shouldadvance = 1;
                            if(!iscaptain && finaltar) {
                                int shouldfollow = 1;
                                Object *to = finaltar->object;
                                Unit *oldtar;
                                tar = unit;
                                while(tar != finaltar && (shouldfollow || shouldadvance)) {
                                    if(tar->object != finaltar->object &&
                                        (!to->CanEnter(region,tar) || to->ForbiddenBy(region, tar)) )
                                            shouldfollow = 0; //we don't start battles to enter objects when following.
                                    oldtar = tar;
                                    if( ((FollowOrder *)finaltar->monthorders)->ship ) {
                                        tar = finaltar; //if there's a ship involved, no follow, no advance.
                                        shouldfollow = 0;
                                        shouldadvance = 0;
                                    } else tar = region->GetUnitId(((FollowOrder *)tar->monthorders)->targetid, tar->faction->num);
                                    if (oldtar->faction->GetAttitude(tar->faction->num) < A_ALLY) shouldadvance = 0;
                                }
                                if(finaltar->object != unit->object && shouldfollow) {
                                	unit->MoveUnit(to);
                         			unit->Event(AString("Enters ") + *(to->name) + ".");
                                }
                                if(finaltar->object != unit->object && ObjectIsShip(unit->object->type)) {
                                    //follower might get sailed away!
                                    unit->MoveUnit(region->GetDummy());
                                }
                                if(finaltar->monthorders && finaltar->monthorders->type == O_ADVANCE && shouldadvance) o->advancing = 1;  //ie if all units in between are advancing and allied
                            }
                            tar = finaltar; //tar is now the 'ultimate' target.
                            
                            //we now have the 'ultimate' target which this unit, and maybe some others, are following.
                            if(tar && phase == tar->movepoints && !tar->nomove && tar->monthorders && (tar->monthorders->type == O_MOVE || tar->monthorders->type == O_ADVANCE ||
                                tar->monthorders->type == O_SAIL)) {
                                //set the follow direction to the next direction this unit is moving.
                                MoveDir * x = 0;
                                if(tar->monthorders->type == O_SAIL && ((SailOrder *)tar->monthorders)->dirs.Num() ) {
                                    x = (MoveDir *) ((SailOrder *)tar->monthorders)->dirs.First();
                                    o->advancing = 0;
                                } else if ( ((MoveOrder *)tar->monthorders)->dirs.Num()) {
                                    x = (MoveDir *) ((MoveOrder *)tar->monthorders)->dirs.First();
                                    if(shouldadvance) o->advancing = ((MoveOrder *)tar->monthorders)->advancing;
                                    else o->advancing = 0; //only advance after an ally.
                                }
                                if(x) {
                                    if(x->dir == MOVE_IN && unit->object != tar->object) {
                                        //they are moving IN, and are in a different object to us. Thus, we cannot follow them :(.
                                        o->target = 0;
                                        o->dir = -1;
                                    } else o->dir = x->dir; //somehow have to deal with moveenters (& thus move IN) At the moment will not enter into objects after the target unit.
                                } else {
                                    o->target = 0; //ultimate target is not moving.
                                    o->dir = -1;
                                }
                            } else {
                                o->target = 0; //the target unit is not moving, so nor are we, so don't bother looking for it later.
                                o->dir = -1;
                            }
                        } else {
                            o->target = 0; //there is no target unit.
                            o->dir = -1;
                        }
                    } else {
                        FollowOrder *o = (FollowOrder *) unit->monthorders;
                        if(unit->nomove) {
                            delete o;
                            unit->monthorders = 0;
                        } else {
                            o->target = 0;
                            o->dir = -1;
                        }
                    }
                }
			}    
        }	
	}
}

void Game::DoMoveEnter(Unit * unit,ARegion * region,Object **obj)
{
	MoveOrder * o;
	if (!unit->monthorders ||
			((unit->monthorders->type != O_MOVE) &&
			 (unit->monthorders->type != O_ADVANCE)))
		return;
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
				unit->Error("MOVE: Can't find object.", o->quiet);
				continue;
			}

			if (!to->CanEnter(region,unit)) {
				unit->Error("ENTER: Can't enter that.", o->quiet);
				continue;
			}

			Unit *forbid = to->ForbiddenBy(region, unit);
			if (forbid && !o->advancing) {
				unit->Error("ENTER: Is refused entry.", o->quiet);
				continue;
			}

			if(forbid && region->IsSafeRegion())
			{
				unit->Error("ENTER: No battles allowed in safe regions.", o->quiet);
				continue;
			}

			if (forbid && !(unit->canattack && unit->IsReallyAlive())) {
				unit->Error(AString("ENTER: Unable to attack ") +
						*(forbid->name), o->quiet);
				continue;
			}

			int done = 0;
			while (forbid)
			{
				int result = RunBattle(region, unit, forbid, 0, 0);
#ifdef DEBUG
cout << "Returned to DoMoveEnter" << endl;
#endif
				if(result == BATTLE_IMPOSSIBLE) {
					unit->Error(AString("ENTER: Unable to attack ")+
							*(forbid->name), o->quiet);
					done = 1;
					break;
				}
				if (!unit->canattack || !unit->IsReallyAlive()) {
				  done = 1;
				  break;
				}
				forbid = to->ForbiddenBy(region, unit);
			}
#ifdef DEBUG
cout << "Resolved forbidding" << endl;
#endif
			if (done) continue;

			unit->MoveUnit(to);
			unit->Event(AString("Enters ") + *(to->name) + ".");
			*obj = to;
		} else {
			if (i == MOVE_OUT) {
				if(TerrainDefs[region->type].similar_type == R_OCEAN &&
						(!unit->CanSwim() ||
						 unit->GetFlag(FLAG_NOCROSS_WATER)))
				{
					unit->Error("MOVE: Can't leave ship.", o->quiet);
					continue;
				}

				Object * to = region->GetDummy();
				unit->MoveUnit(to);
				*obj = to;
			}
		}
	}
}

Location * Game::DoAMoveOrder(Unit * unit, ARegion * region, Object * obj)
{
	Location * loc = new Location;	
//	int movetype = unit->MoveType(); //dummy movetype until we know where we are going.
	AString road;

	if (unit->guard == GUARD_GUARD) unit->guard = GUARD_NONE;

	int i;
	int quiet;
	if(unit->monthorders->type == O_MOVE || unit->monthorders->type == O_ADVANCE) {
    	MoveOrder * o = (MoveOrder *) unit->monthorders;
    	quiet = o->quiet;
    	if (o->advancing) unit->guard = GUARD_ADVANCE;
    	if (o->dirs.Num()) {
    		MoveDir * x = (MoveDir *) o->dirs.First();
    		o->dirs.Remove(x);
    		i = x->dir;
		    delete x;
    	} else goto done_moving;
	} else if(unit->monthorders->type == O_FOLLOW) {
	    FollowOrder * o = (FollowOrder *) unit->monthorders;
    	quiet = o->quiet;
	    if(o->advancing) unit->guard = GUARD_ADVANCE;
	    i = o->dir;
	} else goto done_moving; //this should never occur.
	
	/* Ok, now we can move a region */
	if(region->dynamicexits) ResolveExits(region,unit);	
	
	{
		int startmove = 0;
		/* Setup region to move to */
		ARegion * newreg;
		int portalwgt = 0;
		if (i == MOVE_IN) {
			if (obj->inner == -1) {
				unit->Error("MOVE: Can't move IN there.", quiet);
				goto done_moving;
			} else if(obj->type == O_ESEAPORTAL) {
			    portalwgt += unit->Weight();  // ARCADIA_MAGIC Patch (portals)
			}
			newreg = regions.GetRegion(obj->inner);
		} else {
			newreg = region->neighbors[i];
		}

		if (!newreg) {
			unit->Error(AString("MOVE: Can't move that direction."), quiet);
			goto done_moving;
		}

		if(region->type == R_NEXUS && newreg->IsStartingCity())
			startmove = 1;

		if((TerrainDefs[region->type].similar_type == R_OCEAN) &&
		   (!unit->CanSwim() || unit->GetFlag(FLAG_NOCROSS_WATER))) {
			unit->Error(AString("MOVE: Can't move while in the ocean."), quiet);
			goto done_moving;
		}
		
		//BS edit to prevent merfolk beaching themselves (and any other swimming monster with no walk capacity)
		if(unit->type == U_WMON && (TerrainDefs[region->type].similar_type == R_OCEAN) &&
		   (TerrainDefs[newreg->type].similar_type != R_OCEAN) && !unit->CanWalk(unit->items.Weight())) {
			unit->Error(AString("MOVE: Can't move out of ocean."), quiet);
			goto done_moving;
		}

		if(Globals->ARCADIA_MAGIC) {
            if(unit->type == U_WMON && newreg->willsink > 0 && newreg->willsink < region->willsink && !unit->CanSwim() ) {
                unit->Error("MOVE: Monsters don't move into sinking regions.", quiet);
                goto done_moving;
            }
        }


// Arcadia portal stuff, deducting travel energy cost.	If cost is more than mage can bear, collapse the portal.
// Otherwise, do nothing, and we'll subtrace the cost later. The cost here must be the same as the cost in
// unit::EnergyRecharge() or strange things will happen.
        
        Location *locmage = 0;
		if(portalwgt) {
		    locmage = regions.FindUnit(obj->mageowner); //this creates a 'new' location, so should be deleted later.

		    if(!locmage || (locmage->unit->GetEnergy(portalwgt) < 0) ) {
		          //cannot pass through
		        unit->Error(AString("MOVE: The portal collapses as unit enters."), quiet);
		        if(locmage) {
		            locmage->unit->Error(AString("Not enough energy to hold a portal open, which promptly collapses"), quiet);
		        }
		        if(locmage) delete locmage;
		        locmage = 0;
		        Object *dest = region->GetDummy();
		        forlist(&obj->units) {
		            Unit *u = (Unit *) elem;
		            u->MoveUnit(dest);
			    }
			//inner object, so destroy link at other end.
                ARegion *newreg = regions.GetRegion(obj->inner);
                forlist_reuse(&newreg->objects) {
                    Object *ob = (Object *) elem;
                    if(ob->type == O_ESEAPORTAL && ob->mageowner == obj->mageowner && ob->inner == region->num) {
                        Object *dest2 = newreg->GetDummy();
	                    forlist(&obj->units) {
				            Unit *u = (Unit *) elem;
				            u->MoveUnit(dest2);
			            }
                        newreg->objects.Remove(ob);
                        delete ob;
                    }
                }
                region->objects.Remove(obj);
                delete obj;
                obj = dest; //for when we access it later, this is where our unit now is.
                goto done_moving;
		    }
		    //otherwise, this portal can be used.
		}

	    int movetype = unit->MoveType(newreg);

		road = "";
		int cost = newreg->MoveCost(movetype, region, i, &road);
		if (region->type != R_NEXUS &&
				unit->CalcMovePoints(newreg) - unit->movepoints < cost) {
				//we don't have enough movement points to move!
			if(unit->MoveType(newreg) == M_NONE) {
				unit->Error("MOVE: Unit is overloaded and cannot move.", quiet);
			} else if(unit->monthorders->type == O_FOLLOW) {
			    unit->Error("MOVE: Unit has insufficient movement points to continue following.", quiet);
			} else {
				unit->Error("MOVE: Unit has insufficient movement points;"
						" remaining moves queued.", quiet);
				MoveOrder * o = (MoveOrder *) unit->monthorders;
				TurnOrder *tOrder = new TurnOrder;
				AString order;
				tOrder->repeating = 0;
				if (o->advancing) order = "ADVANCE ";
				else order = "MOVE ";
				if (i < NDIRS) order += DirectionAbrs[i];
				else if (i == MOVE_IN) order += "IN";
				else if (i == MOVE_OUT) order += "OUT";
				else order += i - MOVE_ENTER;
				forlist(&o->dirs) {
					MoveDir *move = (MoveDir *) elem;
					order += " ";
					if (move->dir < NDIRS) order += DirectionAbrs[move->dir];
					else if (move->dir == MOVE_IN) order += "IN";
					else if (move->dir == MOVE_OUT) order += "OUT";
					else order += move->dir - MOVE_ENTER;
				}
				tOrder->turnOrders.Add(new AString(order));
				unit->turnorders.Insert(tOrder);
			}
			goto done_moving;
		}
		if(cost < 0) {
		    unit->Event(AString("MOVE: Unit is prevented from moving to ") + newreg->ShortPrint(&regions) + " by intervening terrain.");
		    goto done_moving;
		}

		if((TerrainDefs[newreg->type].similar_type == R_OCEAN) &&
		   (!unit->CanSwim() || unit->GetFlag(FLAG_NOCROSS_WATER))) {
			unit->Event(AString("Discovers that ") +
						newreg->ShortPrint(&regions) + " is " +
						TerrainDefs[newreg->type].name + ".");
			goto done_moving;
		}

		if (unit->type == U_WMON && newreg->town && newreg->IsGuarded()) {
		    //monsters don't move into towns ... unless it is Arcadia and they are dragons!
		    if(!Globals->ARCADIA_MAGIC || unit->items.GetNum(I_DRAGON) == 0) {
    			unit->Event("Monsters don't move into guarded towns.");
    			goto done_moving;
			}
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
			int obs = unit->GetAttribute("observation");
			unit->Event(AString("Is forbidden entry to ") +
						newreg->ShortPrint(&regions) + " by " +
						forbid->GetName(obs) + ".");
			obs = forbid->GetAttribute("observation");
			forbid->Event(AString("Forbids entry to ") +
						  unit->GetName(obs) + ".");
			goto done_moving;
		}

		/* Clear the unit's alias out, so as not to interfere with TEACH */
		unit->alias = 0;

		unit->movepoints += cost;
		unit->MoveUnit(newreg->GetDummy());
		if(movetype != M_FLY) unit->CrossHexside(region, newreg);

		AString temp;
		switch (movetype) {
		case M_WALK:
			temp = AString("Walks ") + road;
			if(TerrainDefs[newreg->type].similar_type == R_OCEAN)
				temp = "Swims ";
			break;
		case M_RIDE:
			temp = AString("Rides ") + road;
			unit->Experience(S_RIDING,cost);
			if(unit->GetSkill(S_SWIFTNESS)) unit->Experience(S_SWIFTNESS,cost);
			break;
		case M_FLY:
			temp = "Flies ";
			unit->Experience(S_RIDING,cost);
			if(unit->GetSkill(S_SWIFTNESS)) unit->Experience(S_SWIFTNESS,cost);
			break;
		}
		unit->Event(temp + AString("from ") + region->ShortPrint(&regions)
					+ AString(" to ") + newreg->ShortPrint(&regions) +
					AString("."));
		if (forbid) {
			unit->advancefrom = region;
		}

		//Arcadia portal stuff
        #ifdef DEBUG
	    if(portalwgt && !locmage || portalwgt < 0) {
	        Awrite("portal error; locmage is zero after movement or portalwgt is negative");
	        system("pause");
        }
        #endif

		if(locmage) {
            locmage->unit->transferred += portalwgt;
            delete locmage;
		}

		if(Globals->TRANSIT_REPORT != GameDefs::REPORT_NOTHING) {
			// Update our visit record in the region we are leaving.
			Farsight *f;
			forlist(&region->passers) {
				f = (Farsight *)elem;
				if(f->unit == unit) {
					// We moved into here this turn
					if(i < MOVE_IN) {
						f->exits_used[i] = 1;
					}
				}
			}
			// And mark the hex being entered
			f = new Farsight;
			f->faction = unit->faction;
			f->level = 0;
			f->unit = unit;
			if(i < MOVE_IN) {
				f->exits_used[region->GetRealDirComp(i)] = 1;
			}
			newreg->passers.Add(f);
		}
		region = newreg;   //region updated here
	}
	loc->unit = unit;
	loc->region = region;
	loc->obj = region->GetDummy(); //This is an Arcadia change, to allow checking of the object during attacks. Before the object was the starting object, not the finishing one.
	return loc;

done_moving:
    if(unit->monthorders->type == O_MOVE || unit->monthorders->type == O_ADVANCE) {
        MoveOrder *o = (MoveOrder *) unit->monthorders;
	    delete o;
	    unit->monthorders = 0;
    }
	loc->unit = unit;
	loc->region = region;
	loc->obj = obj;
	return loc;
}

void Game::RunMasterOrders()
{
    //Need to recode with new system. See Nylandor code for details
}

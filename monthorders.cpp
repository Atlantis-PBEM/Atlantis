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

#include <stdlib.h>

#include "game.h"
#include "gamedata.h"
#include "quests.h"

void Game::RunMovementOrders()
{
	int phase, error;
	ARegion *r;
	Object *o;
	Unit *u;
	AList locs;
	Location *l;
	MoveOrder *mo;
	SailOrder *so;
	MoveDir *d;
	AString order, *tOrder;

	for (phase = 0; phase < Globals->MAX_SPEED; phase++) {
		forlist(&regions) {
			r = (ARegion *) elem;
			forlist(&r->objects) {
				o = (Object *) elem;
				forlist(&o->units) {
					u = (Unit *) elem;
					Object *tempobj = o;
					DoMoveEnter(u, r, &tempobj);
				}
			}
		}
		forlist_reuse(&regions) {
			r = (ARegion *) elem;
			forlist(&r->objects) {
				o = (Object *) elem;
				error = 1;
				if (o->IsFleet()) {
					u = o->GetOwner();
					if (!u)
						continue;
					if (u->phase >= phase)
						continue;
					if (!u->nomove &&
							u->monthorders &&
							u->monthorders->type == O_SAIL)  {
						u->phase = phase;
						if (o->incomplete < 50) {
							l = Do1SailOrder(r, o, u);
							if (l) locs.Add(l);
							error = 0;
						} else
							error = 3;
					} else
						error = 2;
				}
				if (error > 0) {
					forlist(&o->units) {
						u = (Unit *) elem;
						if (u && u->monthorders &&
								u->monthorders->type == O_SAIL) {
							switch (error) {
								case 1:
									u->Error("SAIL: Must be on a ship.");
									break;
								case 2:
									u->Error("SAIL: Owner must issue fleet directions.");
									break;
								case 3:
									u->Error("SAIL: Fleet is too damaged to sail.");
									break;
							}
							delete u->monthorders;
							u->monthorders = 0;
						}
					}
				}
			}
		}
		forlist_reuse(&regions) {
			r = (ARegion *) elem;
			forlist(&r->objects) {
				o = (Object *) elem;
				forlist(&o->units) {
					u = (Unit *) elem;
					if (u->phase >= phase)
						continue;
					u->phase = phase;
					if (u && !u->nomove && u->monthorders &&
							(u->monthorders->type == O_MOVE ||
							u->monthorders->type == O_ADVANCE)) {
						l = DoAMoveOrder(u, r, o);
						if (l) locs.Add(l);
					}
				}
			}
		}
		DoMovementAttacks(&locs);
		locs.DeleteAll();
	}

	// Do a final round of Enters after the phased movement is done,
	// in case such a thing is at the end of a move chain
	forlist(&regions) {
		r = (ARegion *) elem;
		forlist(&r->objects) {
			o = (Object *) elem;
			forlist(&o->units) {
				u = (Unit *) elem;
				Object *tempobj = o;
				DoMoveEnter(u, r, &tempobj);
			}
		}
	}

	// Queue remaining moves
	forlist_reuse(&regions) {
		r = (ARegion *) elem;
		forlist(&r->objects) {
			o = (Object *) elem;
			forlist(&o->units) {
				u = (Unit *) elem;
				mo = (MoveOrder *) u->monthorders;
				if (!u->nomove &&
						u->monthorders &&
						(u->monthorders->type == O_MOVE ||
						u->monthorders->type == O_ADVANCE) &&
						mo->dirs.Num() > 0) {
					d = (MoveDir *) mo->dirs.First();
					if (u->savedmovedir != d->dir)
						u->savedmovement = 0;
					u->savedmovement += u->movepoints / Globals->MAX_SPEED;
					u->savedmovedir = d->dir;
				} else {
					u->savedmovement = 0;
					u->savedmovedir = -1;
				}
				if (u->monthorders && 
						(u->monthorders->type == O_MOVE ||
						u->monthorders->type == O_ADVANCE)) {
					mo = (MoveOrder *) u->monthorders;
					if (mo->dirs.Num() > 0) {
						tOrder = new AString;
						if (mo->advancing)
							*tOrder = "ADVANCE";
						else
							*tOrder = "MOVE";
						u->Event(*tOrder + ": Unit has insufficient movement points;"
								" remaining moves queued.");
						forlist(&mo->dirs) {
							d = (MoveDir *) elem;
							*tOrder += " ";
							if (d->dir < NDIRS) *tOrder += DirectionAbrs[d->dir];
							else if (d->dir == MOVE_IN) *tOrder += "IN";
							else if (d->dir == MOVE_OUT) *tOrder += "OUT";
							else if (d->dir == MOVE_PAUSE) *tOrder += "P";
							else *tOrder += d->dir - MOVE_ENTER;
						}
						u->oldorders.Insert(tOrder);
					}
				}
			}
			u = o->GetOwner();
			if (o->IsFleet() && u && !u->nomove &&
					u->monthorders && 
					u->monthorders->type == O_SAIL) {
				so = (SailOrder *) u->monthorders;
				if (so->dirs.Num() > 0) {
					u->Event("SAIL: Can't sail that far;"
						" remaining moves queued.");
					tOrder = new AString("SAIL");
					forlist(&so->dirs) {
						d = (MoveDir *) elem;
						*tOrder += " ";
						if (d->dir == MOVE_PAUSE)
							*tOrder += "P";
						else
							*tOrder += DirectionAbrs[d->dir];
					}
					u->oldorders.Insert(tOrder);
				}
			}
		}
	}
}

Location *Game::Do1SailOrder(ARegion *reg, Object *fleet, Unit *cap)
{
	SailOrder *o = (SailOrder *) cap->monthorders;
	int stop, wgt, slr, nomove, cost;
	AList facs;
	ARegion *newreg;
	MoveDir *x;
	Unit *unit;
	Location *loc;

	fleet->movepoints += fleet->GetFleetSpeed(0);
	stop = 0;
	wgt = 0;
	slr = 0;
	nomove = 0;
	forlist(&fleet->units) {
		unit = (Unit *) elem;
		if (!GetFaction2(&facs,unit->faction->num)) {
			FactionPtr * p = new FactionPtr;
			p->ptr = unit->faction;
			facs.Add(p);
		}
		wgt += unit->Weight();
		if (unit->nomove) {
			// If any unit on-board was in a fight (and
			// suffered > 5% casualties), then halt movement
			nomove = 1;
		}
		if (unit->monthorders && unit->monthorders->type == O_SAIL) {
			slr += unit->GetSkill(S_SAILING) * unit->GetMen();
		}
	}

	if (nomove) {
		stop = 1;
	} else if (wgt > fleet->FleetCapacity()) {
		cap->Error("SAIL: Fleet is overloaded.");
		stop = 1;
	} else if (slr < fleet->GetFleetSize()) {
		cap->Error("SAIL: Not enough sailors.");
		stop = 1;
	} else if (!o->dirs.Num()) {
		// no more moves?
		stop = 1;
	} else {
		x = (MoveDir *) o->dirs.First();
		if (x->dir == MOVE_PAUSE) {
			newreg = reg;
		} else {
			newreg = reg->neighbors[x->dir];
		}
		cost = 1;
		if (Globals->WEATHER_EXISTS) {
			if (newreg && newreg->weather != W_NORMAL &&
					!newreg->clearskies)
				cost = 2;
		}
		if (x->dir == MOVE_PAUSE) {
			cost = 1;
		}
		// We probably shouldn't see terrain-based errors until
		// we accumulate enough movement points to get there
		if (fleet->movepoints < cost * Globals->MAX_SPEED)
			return 0;
		if (!newreg) {
			cap->Error("SAIL: Can't sail that way.");
			stop = 1;
		} else if (x->dir == MOVE_PAUSE) {
			// Can always do maneuvers
		} else if (fleet->flying < 1 && !newreg->IsCoastalOrLakeside()) {
			cap->Error("SAIL: Can't sail inland.");
			stop = 1;
		} else if ((fleet->flying < 1) &&
			(TerrainDefs[reg->type].similar_type != R_OCEAN) &&
			(TerrainDefs[newreg->type].similar_type != R_OCEAN)) {
			cap->Error("SAIL: Can't sail inland.");
			stop = 1;
		} else if (fleet->SailThroughCheck(x->dir) < 1) {
			cap->Error(AString("SAIL: Could not sail ") +
					DirectionStrs[x->dir] + AString(" from ") +
					reg->ShortPrint(&regions) +
					". Cannot sail through land.");
			stop = 1;
		}

		if (!stop) {
			fleet->movepoints -= cost * Globals->MAX_SPEED;
			if (x->dir != MOVE_PAUSE) {
				fleet->MoveObject(newreg);
				fleet->SetPrevDir(reg->GetRealDirComp(x->dir));
			}
			forlist(&fleet->units) {
				unit = (Unit *) elem;
				unit->moved += cost;
				if (unit->guard == GUARD_GUARD)
					unit->guard = GUARD_NONE;
				unit->alias = 0;
				unit->PracticeAttribute("wind");
				if (unit->monthorders) {
					if (unit->monthorders->type == O_SAIL)
						unit->Practice(S_SAILING);
					if (unit->monthorders->type == O_MOVE) {
						delete unit->monthorders;
						unit->monthorders = 0;
					}
				}
				unit->DiscardUnfinishedShips();
				if (!GetFaction2(&facs, unit->faction->num)) {
					FactionPtr *p = new FactionPtr;
					p->ptr = unit->faction;
					facs.Add(p);
				}
			}

			forlist_reuse(&facs) {
				Faction * f = ((FactionPtr *) elem)->ptr;
				if (x->dir == MOVE_PAUSE) {
					f->Event(*fleet->name +
						AString(" performs maneuvers in ") +
						reg->ShortPrint(&regions) +
						AString("."));
				} else {
					f->Event(*fleet->name +
						AString(" sails from ") +
						reg->ShortPrint(&regions) +
						AString(" to ") +
						newreg->ShortPrint(&regions) +
						AString("."));
				}
			}
			if (Globals->TRANSIT_REPORT != GameDefs::REPORT_NOTHING &&
					x->dir != MOVE_PAUSE) {
				if (!(cap->faction->IsNPC())) newreg->visited = 1;
				forlist(&fleet->units) {
					// Everyone onboard gets to see the sights
					unit = (Unit *) elem;
					
					Farsight *f;
					// Note the hex being left
					forlist(&reg->passers) {
						f = (Farsight *)elem;
						if (f->unit == unit) {
							// We moved into here this turn
							f->exits_used[x->dir] = 1;
						}
					}
					// And mark the hex being entered
					f = new Farsight;
					f->faction = unit->faction;
					f->level = 0;
					f->unit = unit;
					f->exits_used[reg->GetRealDirComp(x->dir)] = 1;
					newreg->passers.Add(f);
				}
			}
			reg = newreg;
			if (newreg->ForbiddenShip(fleet)) {
				cap->faction->Event(*fleet->name +
					AString(" is stopped by guards in ") +
					newreg->ShortPrint(&regions) + 
					AString("."));
				stop = 1;
			}
			o->dirs.Remove(x);
			delete x;
		}
	}

	if (stop) {
		// Clear out everyone's orders
		forlist(&fleet->units) {
			Unit *unit = (Unit *) elem;

			if (unit->monthorders &&
					unit->monthorders->type == O_SAIL) {
				delete unit->monthorders;
				unit->monthorders = 0;
			}
		}
	}

	loc = new Location;
	loc->unit = cap;
	loc->region = reg;
	loc->obj = fleet;
	return loc;
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
	if (Globals->LEADERS_EXIST && !unit->IsLeader()) {
		/* small change to handle Ceran's mercs */
		if (!unit->GetMen(I_MERC)) {
			// Mercs can teach even though they are not leaders.
			// They cannot however improve their own skills
			unit->Error("TEACH: Only leaders can teach.");
			return;
		}
	}

	int students = 0;
	TeachOrder * order = (TeachOrder *) unit->monthorders;
	reg->DeduplicateUnitList(&order->targets, unit->faction->num);
	forlist(&order->targets) {
		UnitId * id = (UnitId *) elem;
		Unit * target = reg->GetUnitId(id,unit->faction->num);
		if (!target) {
			order->targets.Remove(id);
			unit->Error("TEACH: No such unit.");
			delete id;
		} else {
			if (target->faction->GetAttitude(unit->faction->num) < A_FRIENDLY) {
				unit->Error(AString("TEACH: ") + *(target->name) +
							" is not a member of a friendly faction.");
				order->targets.Remove(id);
				delete id;
			} else {
				if (!target->monthorders ||
					target->monthorders->type != O_STUDY) {
					unit->Error(AString("TEACH: ") + *(target->name) +
								" is not studying.");
					order->targets.Remove(id);
					delete id;
				} else {
					int sk = ((StudyOrder *) target->monthorders)->skill;
					if (unit->GetRealSkill(sk) <= target->GetRealSkill(sk)) {
						unit->Error(AString("TEACH: ") +
									*(target->name) + " is not studying "
									"a skill you can teach.");
						order->targets.Remove(id);
						delete id;
					} else {
						// Check whether it's a valid skill to teach
						if (SkillDefs[sk].flags & SkillType::NOTEACH) {
							unit->Error(AString("TEACH: ") + 
									AString(SkillDefs[sk].name) + 
									" cannot be taught.");
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
			// The TEACHER may learn something in this process!
			unit->Practice(o->skill);
		}
	}
}

void Game::Run1BuildOrder(ARegion *r, Object *obj, Unit *u)
{
	Object *buildobj;
	int questcomplete = 0;
	AString quest_rewards;

	if (!Globals->BUILD_NO_TRADE && !ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
		u->Error("BUILD: Faction can't produce in that many regions.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	buildobj = r->GetObject(u->build);
	// plain "BUILD" order needs to check that the unit is in something
	// that can be built AFTER enter/leave orders have executed
	if (!buildobj || buildobj->type == O_DUMMY) {
		buildobj = obj;
	}
	if (!buildobj || buildobj->type == O_DUMMY) {
		u->Error("BUILD: Nothing to build.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}
	AString skname = ObjectDefs[buildobj->type].skill;
	int sk = LookupSkill(&skname);
	if (sk == -1) {
		u->Error("BUILD: Can't build that.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int usk = u->GetSkill(sk);
	if (usk < ObjectDefs[buildobj->type].level) {
		u->Error("BUILD: Can't build that.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}
	
	int needed = buildobj->incomplete;
	int type = buildobj->type;
	// AS
	if (((ObjectDefs[type].flags & ObjectType::NEVERDECAY) || !Globals->DECAY) &&
			needed < 1) {
		u->Error("BUILD: Object is finished.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	// AS
	if (needed <= -(ObjectDefs[type].maxMaintenance)) {
		u->Error("BUILD: Object does not yet require maintenance.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int it = ObjectDefs[type].item;
	int itn;
	if (it == I_WOOD_OR_STONE) {
		itn = u->GetSharedNum(I_WOOD) + u->GetSharedNum(I_STONE);
	} else {
		itn = u->GetSharedNum(it);
	}

	if (itn == 0) {
		u->Error("BUILD: Don't have the required materials.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int num = u->GetMen() * usk;

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
		job = "Performs maintenance on ";
		buildobj->incomplete -= (num * ObjectDefs[type].maintFactor);
		if (buildobj->incomplete < -(ObjectDefs[type].maxMaintenance))
			buildobj->incomplete = -(ObjectDefs[type].maxMaintenance);
	} else if (needed > 0) {
		if (num > needed) num = needed;
		if (itn < num) num = itn;
		job = "Performs construction on ";
		buildobj->incomplete -= num;
		if (buildobj->incomplete == 0) {
			job = "Completes construction of ";
			buildobj->incomplete = -(ObjectDefs[type].maxMaintenance);
			if (quests.CheckQuestBuildTarget(r, type, u, &quest_rewards)) {
				questcomplete = 1;
			}
		}
	}

	/* Perform the build */
	
	if (obj != buildobj)
		u->MoveUnit(buildobj);

	if (it == I_WOOD_OR_STONE) {
		if (num > u->GetSharedNum(I_STONE)) {
			num -= u->GetSharedNum(I_STONE);
			u->ConsumeShared(I_STONE, u->GetSharedNum(I_STONE));
			u->ConsumeShared(I_WOOD, num);
		} else {
			u->ConsumeShared(I_STONE, num);
		}
	} else {
		u->ConsumeShared(it, num);
	}
	
	/* Regional economic improvement */
	r->improvement += num;

	// AS
	u->Event(job + *(buildobj->name));
	if (questcomplete) {
		u->Event(AString("You have completed a quest! ") + quest_rewards);
	}
	u->Practice(sk);

	delete u->monthorders;
	u->monthorders = 0;
}

/* Alternate processing for building item-type ship
 * objects and instantiating fleets.
 */
void Game::RunBuildShipOrder(ARegion * r,Object * obj,Unit * u)
{
	int ship, skill, level, maxbuild, unfinished, output, percent;
	AString skname;

	ship = abs(u->build);
	skname = ItemDefs[ship].pSkill;
	skill = LookupSkill(&skname);
	level = u->GetSkill(skill);

	if (skill == -1) {
		u->Error("BUILD: Can't build that.");
		return;
	}

	// get needed to complete
	maxbuild = 0;
	if ((u->monthorders) && 
		(u->monthorders->type == O_BUILD)) {
			BuildOrder *border = (BuildOrder *) u->monthorders;
			maxbuild = border->needtocomplete;
	}
	if (maxbuild < 1) {
		// Our helpers have already finished the hard work, so
		// just put the finishing touches on the new vessel
		unfinished = 0;
	} else {
		output = ShipConstruction(r, u, u, level, maxbuild, ship);
		
		if (output < 1) return;
		
		// are there unfinished ship items of the given type?
		unfinished = u->items.GetNum(ship);
		
		if (unfinished == 0) {
			// Start a new ship's construction from scratch
			unfinished = ItemDefs[ship].pMonths;
			u->items.SetNum(ship, unfinished);	
		}

		// Now reduce unfinished by produced amount
		unfinished -= output;
		if (unfinished < 0)
			unfinished = 0;
	}
	u->items.SetNum(ship, unfinished);

	// practice
	u->Practice(skill);

	if (unfinished == 0) {
		u->Event(AString("Finishes building a ") + ItemDefs[ship].name + " in " +
			r->ShortPrint(&regions) + ".");
		CreateShip(r, u, ship);
	} else {
		percent = 100 * output / ItemDefs[ship].pMonths;
		u->Event(AString("Performs construction work on a ") + 
			ItemDefs[ship].name + " (" + percent + "%) in " +
			r->ShortPrint(&regions) + ".");
	}

	delete u->monthorders;
	u->monthorders = 0;
}

void Game::AddNewBuildings(ARegion *r)
{
	int i;
	forlist((&r->objects)) {
		Object *obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->monthorders) {
				if (u->monthorders->type == O_BUILD) {
					BuildOrder *o = (BuildOrder *)u->monthorders;

					// If BUILD order was marked for creating new building
					// in parse phase, it is time to create one now.
					if (o->new_building != -1) {
						for (i = 1; i < 100; i++) {
							if (!r->GetObject(i)) {
								break;
							}
						}
						if (i < 100) {
							Object * obj = new Object(r);
							obj->type = o->new_building;
							obj->incomplete = ObjectDefs[obj->type].cost;
							obj->num = i;
							obj->SetName(new AString("Building"));
							u->build = obj->num;
							r->objects.Add(obj);

							// This moves unit to a new building.
							// This unit might be processed again but from new object.
							u->MoveUnit(obj);
							// This why we need to unset new_building so it will not
							// try to create new object again.
							o->new_building = -1;
						} else {
							u->Error("BUILD: The region is full.");
						}
					}
				}
			}
		}
	}
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
					if (o->target) {
						Unit *target = r->GetUnitId(o->target,u->faction->num);
						if (!target) {
							u->Error("BUILD: No such unit to help.");
							delete u->monthorders;
							u->monthorders = 0;
							continue;
						}
						// Make sure that unit is building
						if (!target->monthorders ||
								target->monthorders->type != O_BUILD) {
							u->Error("BUILD: Unit isn't building.");
							delete u->monthorders;
							u->monthorders = 0;
							continue;
						}
						// Make sure that unit considers you friendly!
						if (target->faction->GetAttitude(u->faction->num) <
								A_FRIENDLY) {
							u->Error("BUILD: Unit you are helping rejects "
									"your help.");
							delete u->monthorders;
							u->monthorders = 0;
							continue;
						}
						if (target->build == 0) {
							// Help with whatever building the target is in
							tarobj = target->object;
							u->build = tarobj->num;
						} else if (target->build > 0) {
							u->build = target->build;
							tarobj = r->GetObject(target->build);
						} else {
							// help build ships
							int ship = abs(target->build);
							AString skname = ItemDefs[ship].pSkill;
							int skill = LookupSkill(&skname);
							int level = u->GetSkill(skill);
							int needed = 0;
							if ((target->monthorders) && 
									(target->monthorders->type == O_BUILD)) {
										BuildOrder *border = (BuildOrder *) target->monthorders;
										needed = border->needtocomplete;
							}
							if (needed < 1) {
								u->Error("BUILD: Construction is already complete.");
								delete u->monthorders;
								u->monthorders = 0;
								continue;
							}
							int output = ShipConstruction(r, u, target, level, needed, ship);
							if (output < 1) continue;
							
							int unfinished = target->items.GetNum(ship);
							if (unfinished == 0) {
								// Start construction on a new ship
								unfinished = ItemDefs[ship].pMonths;
								target->items.SetNum(ship, unfinished);	
							}
							unfinished -= output;
							
							// practice
							u->Practice(skill);
							
							if (unfinished > 0) {
								target->items.SetNum(ship, unfinished);
								if ((target->monthorders) && 
									(target->monthorders->type == O_BUILD)) {
										BuildOrder *border = (BuildOrder *) target->monthorders;
										border->needtocomplete = unfinished;
								}
							} else {
								// CreateShip(r, target, ship);
								// don't create the ship yet; leave that for the unit we're helping
								target->items.SetNum(ship, 1);
								if ((target->monthorders) && 
									(target->monthorders->type == O_BUILD)) {
										BuildOrder *border = (BuildOrder *) target->monthorders;
										border->needtocomplete = 0;
								}
							} 
							int percent = 100 * output / ItemDefs[ship].pMonths;
							u->Event(AString("Helps ") +
								*(target->name) + " with construction of a " + 
								ItemDefs[ship].name + " (" + percent + "%) in " +
								r->ShortPrint(&regions) + ".");							
						}
						// no need to move unit if item-type ships
						// are being built. (leave this commented out)
						// if (tarobj == NULL) tarobj = target->object;
						if ((tarobj != NULL) && (u->object != tarobj))
							u->MoveUnit(tarobj);
					} else {
						Object *buildobj;
						if (u->build > 0) {
							buildobj = r->GetObject(u->build);
							if (buildobj && 
									buildobj != r->GetDummy() &&
									buildobj != u->object)
							{
								u->MoveUnit(buildobj);
							}
						}
					}
				}
			}
		}
	}
}

/* Creates a new ship - either by adding it to a 
 * compatible fleet object or creating a new fleet
 * object with Unit u as owner consisting of exactly
 * ONE ship of the given type.
 */
void Game::CreateShip(ARegion *r, Unit * u, int ship)
{
	Object * obj = u->object;
	// Do we need to create a new fleet?
	int newfleet = 1;
	if (u->object->IsFleet()) {
		newfleet = 0;
		int flying = obj->flying;
		// are the fleets compatible?
		if ((flying > 0) && (ItemDefs[ship].fly < 1)) newfleet = 1;
		if ((flying < 1) && (ItemDefs[ship].fly > 0)) newfleet = 1;
	}
	if (newfleet != 0) {
		// create a new fleet
		Object * fleet = new Object(r);
		fleet->type = O_FLEET;
		fleet->num = shipseq++;
		fleet->name = new AString(AString("Ship [") + fleet->num + "]");
		fleet->AddShip(ship);
		u->object->region->objects.Add(fleet);
		u->MoveUnit(fleet);
	} else {
		obj->AddShip(ship);
	}
}

/* Checks and returns the amount of ship construction,
 * handles material use and practice for both the main
 * shipbuilders and the helpers.
 */
int Game::ShipConstruction(ARegion *r, Unit *u, Unit *target, int level, int needed, int ship)
{
	if (!Globals->BUILD_NO_TRADE && !ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
		u->Error("BUILD: Faction can't produce in that many regions.");
		delete u->monthorders;
		u->monthorders = 0;
		return 0;
	}

	if (level < ItemDefs[ship].pLevel) {
		u->Error("BUILD: Can't build that.");
		delete u->monthorders;
		u->monthorders = 0;
		return 0;
	}

	// are there unfinished ship items of the given type?
	int unfinished = target->items.GetNum(ship);

	int number = u->GetMen() * level + u->GetProductionBonus(ship);

	// find the max we can possibly produce based on man-months of labor
	int maxproduced;
	if (ItemDefs[ship].flags & ItemType::SKILLOUT)
		maxproduced = u->GetMen();
	else
		// don't adjust for pMonths
		// - pMonths represents total requirement
		maxproduced = number;
	
	// adjust maxproduced for items needed until completion
	if (needed < maxproduced) maxproduced = needed;
		
	// adjust maxproduced for unfinished ships
	if ((unfinished > 0) && (maxproduced > unfinished))
		maxproduced = unfinished;

	if (ItemDefs[ship].flags & ItemType::ORINPUTS) {
		// Figure out the max we can produce based on the inputs
		int count = 0;
		unsigned int c;
		for (c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[ship].pInput[c].item;
			if (i != -1)
				count += u->GetSharedNum(i) / ItemDefs[ship].pInput[c].amt;
		}
		if (maxproduced > count)
			maxproduced = count;
		count = maxproduced;
		
		// no required materials?
		if (count < 1) {
			u->Error("BUILD: Don't have the required materials.");
			delete u->monthorders;
			u->monthorders = 0;
			return 0;
		}
		
		/* regional economic improvement */
		r->improvement += count;

		// Deduct the items spent
		for (c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[ship].pInput[c].item;
			int a = ItemDefs[ship].pInput[c].amt;
			if (i != -1) {
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
	}
	else {
		// Figure out the max we can produce based on the inputs
		unsigned int c;
		for (c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[ship].pInput[c].item;
			if (i != -1) {
				int amt = u->GetSharedNum(i);
				if (amt/ItemDefs[ship].pInput[c].amt < maxproduced) {
					maxproduced = amt/ItemDefs[ship].pInput[c].amt;
				}
			}
		}
		
		// no required materials?
		if (maxproduced < 1) {
			u->Error("BUILD: Don't have the required materials.");
			delete u->monthorders;
			u->monthorders = 0;
			return 0;
		}
		
		/* regional economic improvement */
		r->improvement += maxproduced;
		
		// Deduct the items spent
		for (c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[ship].pInput[c].item;
			int a = ItemDefs[ship].pInput[c].amt;
			if (i != -1) {
				u->ConsumeShared(i, maxproduced*a);
			}
		}
	}
	int output = maxproduced * ItemDefs[ship].pOut;
	if (ItemDefs[ship].flags & ItemType::SKILLOUT)
		output *= level;

	delete u->monthorders;
	u->monthorders = 0;
	
	return output;
}

void Game::RunMonthOrders()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		RunIdleOrders(r);
		RunStudyOrders(r);
		AddNewBuildings(r);
		RunBuildHelpers(r);
		RunProduceOrders(r);
	}
}

void Game::RunUnitProduce(ARegion * r,Unit * u)
{
	Production *p;
	ProduceOrder *o = (ProduceOrder *) u->monthorders;

	forlist(&r->products) {
		p = (Production *) elem;
		// PRODUCE orders for producing goods from the land
		// are shared among factions, and therefore handled
		// specially by the RunAProduction() function
		if (o->skill == p->skill && o->item == p->itemtype)
			return;
	}

	if (o->item == I_SILVER) {
		if (!o->quiet)
			u->Error("Can't do that in this region.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	if (o->item == -1 || ItemDefs[o->item].flags & ItemType::DISABLED) {
		u->Error("PRODUCE: Can't produce that.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int input = ItemDefs[o->item].pInput[0].item;
	if (input == -1) {
		u->Error("PRODUCE: Can't produce that.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	int level = u->GetSkill(o->skill);
	if (level < ItemDefs[o->item].pLevel) {
		u->Error("PRODUCE: Can't produce that.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	// LLS
	int number = u->GetMen() * level + u->GetProductionBonus(o->item);

	if (!ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
		u->Error("PRODUCE: Faction can't produce in that many regions.");
		delete u->monthorders;
		u->monthorders = 0;
		return;
	}

	// find the max we can possibly produce based on man-months of labor
	int maxproduced;
	if (ItemDefs[o->item].flags & ItemType::SKILLOUT)
		maxproduced = u->GetMen();
	else
		maxproduced = number/ItemDefs[o->item].pMonths;

	if (o->target > 0 && maxproduced > o->target)
		maxproduced = o->target;

	if (ItemDefs[o->item].flags & ItemType::ORINPUTS) {
		// Figure out the max we can produce based on the inputs
		int count = 0;
		unsigned int c;
		for (c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			if (i != -1)
				count += u->GetSharedNum(i) / ItemDefs[o->item].pInput[c].amt;
		}
		if (maxproduced > count)
			maxproduced = count;
		count = maxproduced;
		
		/* regional economic improvement */
		r->improvement += count;

		// Deduct the items spent
		for (c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			int a = ItemDefs[o->item].pInput[c].amt;
			if (i != -1) {
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
	}
	else {
		// Figure out the max we can produce based on the inputs
		unsigned int c;
		for (c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			if (i != -1) {
				int amt = u->GetSharedNum(i);
				if (amt/ItemDefs[o->item].pInput[c].amt < maxproduced) {
					maxproduced = amt/ItemDefs[o->item].pInput[c].amt;
				}
			}
		}
		
		/* regional economic improvement */
		r->improvement += maxproduced;
		
		// Deduct the items spent
		for (c = 0; c < sizeof(ItemDefs->pInput)/sizeof(Materials); c++) {
			int i = ItemDefs[o->item].pInput[c].item;
			int a = ItemDefs[o->item].pInput[c].amt;
			if (i != -1) {
				u->ConsumeShared(i, maxproduced*a);
			}
		}
	}

	// Now give the items produced
	int output = maxproduced * ItemDefs[o->item].pOut;
	if (ItemDefs[o->item].flags & ItemType::SKILLOUT)
		output *= level;
	u->items.SetNum(o->item,u->items.GetNum(o->item) + output);
	u->Event(AString("Produces ") + ItemString(o->item,output) + " in " +
			r->ShortPrint(&regions) + ".");
	u->Practice(o->skill);
	o->target -= output;
	if (o->target > 0) {
		TurnOrder *tOrder = new TurnOrder;
		AString order;
		tOrder->repeating = 0;
		order = "PRODUCE ";
		order += o->target;
		order += " ";
		order += ItemDefs[o->item].abr;
		tOrder->turnOrders.Add(new AString(order));
		u->turnorders.Insert(tOrder);
	}
	delete u->monthorders;
	u->monthorders = 0;
}

void Game::RunProduceOrders(ARegion * r)
{
	forlist(&r->objects) {
		Object * obj = (Object *) elem;
		forlist ((&obj->units)) {
			Unit * u = (Unit *) elem;
			if (u->monthorders) {
				if (u->monthorders->type == O_PRODUCE) {
					RunUnitProduce(r,u);
				} else {
					if (u->monthorders->type == O_BUILD) {
						if (u->build >= 0) {
							Run1BuildOrder(r,obj,u);
						} else {
							RunBuildShipOrder(r,obj,u);
						}
					}
				}
			}
		}
	}
	forlist_reuse(&r->products)
		RunAProduction(r,(Production *) elem);
}

int Game::ValidProd(Unit * u,ARegion * r, Production * p)
{
	if (u->monthorders->type != O_PRODUCE) return 0;

	ProduceOrder * po = (ProduceOrder *) u->monthorders;
	if (p->itemtype == po->item && p->skill == po->skill) {
		if (p->skill == -1) {
			/* Factor for fractional productivity: 10 */
			po->productivity = (int) ((float) (u->GetMen() * p->productivity / 10));
			return po->productivity;
		}
		int level = u->GetSkill(p->skill);
		if (level < ItemDefs[p->itemtype].pLevel) {
			u->Error("PRODUCE: Unit isn't skilled enough.");
			delete u->monthorders;
			u->monthorders = 0;
			return 0;
		}

		//
		// Check faction limits on production. If the item is silver, then the
		// unit is entertaining or working, and the limit does not apply
		//
		if (p->itemtype != I_SILVER && !ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
			u->Error("PRODUCE: Faction can't produce in that many regions.");
			delete u->monthorders;
			u->monthorders = 0;
			return 0;
		}

		/* check for bonus production */
		// LLS
		int bonus = u->GetProductionBonus(p->itemtype);
		/* Factor for fractional productivity: 10 */
		po->productivity = (int) ((float) (u->GetMen() * level * p->productivity / 10)) + bonus;
		if (po->target > 0 && po->productivity > po->target)
			po->productivity = po->target;
		return po->productivity;
	}
	return 0;
}

int Game::FindAttemptedProd(ARegion * r, Production * p)
{
	int attempted = 0;
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			if ((u->monthorders) && (u->monthorders->type == O_PRODUCE)) {
				attempted += ValidProd(u,r,p);
			}
		}
	}
	return attempted;
}

void Game::RunAProduction(ARegion * r, Production * p)
{
	int questcomplete;
	AString quest_rewards;

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
			questcomplete = 0;
			if (!u->monthorders || u->monthorders->type != O_PRODUCE)
				continue;

			ProduceOrder * po = (ProduceOrder *) u->monthorders;
			if (po->skill != p->skill || po->item != p->itemtype)
				continue;

			/* We need to implement a hack to avoid overflowing */
			int uatt, ubucks;

			uatt = po->productivity;
			if (uatt && amt && attempted)
			{
				double dUbucks = ((double) amt) * ((double) uatt)
					/ ((double) attempted);
				ubucks = (int) dUbucks;
				questcomplete = quests.CheckQuestHarvestTarget(r, po->item, ubucks, amt, u, &quest_rewards);
			}
			else
			{
				ubucks = 0;
			}

			amt -= ubucks;
			attempted -= uatt;
			u->items.SetNum(po->item,u->items.GetNum(po->item)
							+ ubucks);
			u->faction->DiscoverItem(po->item, 0, 1);
			p->activity += ubucks;
			po->target -= ubucks;
			if (po->target > 0) {
				TurnOrder *tOrder = new TurnOrder;
				AString order;
				tOrder->repeating = 0;
				order = "PRODUCE ";
				order += po->target;
				order += " ";
				order += ItemDefs[po->item].abr;
				tOrder->turnOrders.Add(new AString(order));
				u->turnorders.Insert(tOrder);
			}

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
					// If they don't have PHEN, then this will fail safely
					u->Practice(S_PHANTASMAL_ENTERTAINMENT);
					u->Practice(S_ENTERTAINMENT);
				}
			}
			else
			{
				/* Everything else */
				u->Event(AString("Produces ") + ItemString(po->item,ubucks) +
						 " in " + r->ShortPrint(&regions) + ".");
				u->Practice(po->skill);
			}
			delete u->monthorders;
			u->monthorders = 0;
			if (questcomplete) {
				u->Event(AString("You have completed a quest! ") + quest_rewards);
			}
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
	StudyOrder * o = (StudyOrder *) u->monthorders;
	int sk, cost, reset_man, skmax, taughtdays, days;
	AString str;

	reset_man = -1;
	sk = o->skill;
	if (sk == -1 || SkillDefs[sk].flags & SkillType::DISABLED ||
			(SkillDefs[sk].flags & SkillType::APPRENTICE &&
				!Globals->APPRENTICES_EXIST)) {
		u->Error("STUDY: Can't study that.");
		return;
	}

	// Check that the skill can be studied
	if (SkillDefs[sk].flags & SkillType::NOSTUDY) {
		u->Error( AString("STUDY: ") + AString(SkillDefs[sk].name) + " cannot be studied.");
		return;
	}
	
	// Small patch for Ceran Mercs
	if (u->GetMen(I_MERC)) {
		u->Error("STUDY: Mercenaries are not allowed to study.");
		return;
	}

	if (o->level != -1) {
		skmax = u->GetSkillMax(sk);
		if (skmax < o->level) {
			o->level = skmax;
			if (u->GetRealSkill(sk) >= o->level) {
				str = "STUDY: Cannot study ";
				str += SkillDefs[sk].name;
				str += " beyond level ";
				str += o->level;
				str += ".";
				u->Error(str);
				return;
			} else {
				str = "STUDY: set study goal for ";
				str += SkillDefs[sk].name;
				str += " to the maximum achievable level (";
				str += o->level;
				str += ").";
				u->Error(str);
			}
		}
		if (u->GetRealSkill(sk) >= o->level) {
			u->Error("STUDY: already reached specified level; nothing to study.");
			return;
		}
	}

	cost = SkillCost(sk) * u->GetMen();
	if (cost > u->GetSharedMoney()) {
		u->Error("STUDY: Not enough funds.");
		return;
	}

	if ((SkillDefs[sk].flags & SkillType::MAGIC) && u->type != U_MAGE) {
		if (u->type == U_APPRENTICE) {
			u->Error(AString("STUDY: An ") +
				Globals->APPRENTICE_NAME +
				" cannot be made into a mage.");
			return;
		}
		if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
			if (CountMages(u->faction) >= AllowedMages(u->faction)) {
				u->Error("STUDY: Can't have another magician.");
				return;
			}
		}
		if (u->GetMen() != 1) {
			u->Error("STUDY: Only 1-man units can be magicians.");
			return;
		}
		if (!(Globals->MAGE_NONLEADERS)) {
			if (u->GetLeaders() != 1) {
				u->Error("STUDY: Only leaders may study magic.");
				return;
			}
		}
		reset_man = u->type;
		u->type = U_MAGE;
	}

	if ((SkillDefs[sk].flags&SkillType::APPRENTICE) &&
			u->type != U_APPRENTICE) {
		if (u->type == U_MAGE) {
			u->Error(AString("STUDY: A mage cannot be made into an ") +
				Globals->APPRENTICE_NAME + ".");
			return;
		}

		if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
			if (CountApprentices(u->faction)>=AllowedApprentices(u->faction)) {
				u->Error(AString("STUDY: Can't have another ") +					Globals->APPRENTICE_NAME + ".");
				return;
			}
		}
		if (u->GetMen() != 1) {
			u->Error(AString("STUDY: Only 1-man units can be ") +
				Globals->APPRENTICE_NAME + "s.");
			return;
		}
		if (!(Globals->MAGE_NONLEADERS)) {
			if (u->GetLeaders() != 1) {
				u->Error(AString("STUDY: Only leaders may be ") +
					Globals->APPRENTICE_NAME + "s.");
				return;
			}
		}
		reset_man = u->type;
		u->type = U_APPRENTICE;
	}

	if ((Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) &&
			(sk == S_QUARTERMASTER) && (u->GetSkill(S_QUARTERMASTER) == 0) &&
			(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)) {
		if (CountQuarterMasters(u->faction) >=
				AllowedQuarterMasters(u->faction)) {
			u->Error("STUDY: Can't have another quartermaster.");
			return;
		}
		if (u->GetMen() != 1) {
			u->Error("STUDY: Only 1-man units can be quartermasters.");
			return;
		}
	}

	// If TACTICS_NEEDS_WAR is enabled, and the unit is trying to study to tact-5,
	// check that there's still space...
	if (Globals->TACTICS_NEEDS_WAR && sk == S_TACTICS && 
			u->GetSkill(sk) == 4 && u->skills.GetDays(sk)/u->GetMen() >= 300) {
		if (CountTacticians(u->faction) >=
				AllowedTacticians(u->faction)) {
			u->Error("STUDY: Can't start another level 5 tactics leader.");
			return;
		}
		if (u->GetMen() != 1) {
			u->Error("STUDY: Only 1-man units can study to level 5 in tactics.");
			return;
		}
		
	} // end tactics check
	
	// adjust teaching for study rate
	taughtdays = ((long int) o->days * u->skills.GetStudyRate(sk, u->GetMen()) / 30);

	days = u->skills.GetStudyRate(sk, u->GetMen()) * u->GetMen() + taughtdays;

	if ((SkillDefs[sk].flags & SkillType::MAGIC) && u->GetSkill(sk) >= 2) {
		if (obj->incomplete > 0 || obj->type == O_DUMMY) {
			u->Error("Warning: Magic study rate outside of a building "
					"cut in half above level 2.");
			days /= 2;
		} else if (obj->mages < 1) {
			if (!Globals->LIMITED_MAGES_PER_BUILDING ||
					(!obj->IsFleet() &&
					!ObjectDefs[obj->type].maxMages)) {
				u->Error("Warning: Magic study rate cut in half above level 2 due "
						"to unsuitable building.");
			} else {
				u->Error("Warning: Magic study rate cut in half above level 2 due "
						"to number of mages studying in structure.");
			}
			days /= 2;
		} else if (Globals->LIMITED_MAGES_PER_BUILDING) {
			obj->mages--;
		}
	}

	if (SkillDefs[sk].flags & SkillType::SLOWSTUDY) {
		days /= 2;
	}

	if (u->Study(sk,days)) {
		u->ConsumeSharedMoney(cost);
		str = "Studies ";
		str += SkillDefs[sk].name;
		taughtdays = taughtdays/u->GetMen();
		if (taughtdays) {
			str += " and was taught for ";
			str += taughtdays;
			str += " days";
		}
		str += ".";
		u->Event(str);
		// study to level order
		if (o->level != -1) {
			if (u->GetRealSkill(sk) < o->level) {
				TurnOrder *tOrder = new TurnOrder;
				AString order;
				tOrder->repeating = 0;
				order = AString("STUDY ") + SkillDefs[sk].abbr + " " + o->level;
				tOrder->turnOrders.Add(new AString(order));
				u->turnorders.Insert(tOrder);
			} else {
				AString msg("Completes study to level ");
				msg += o->level;
				msg += " in ";
				msg += SkillDefs[sk].name;
				msg += ".";
				u->Event(msg);
			}	
		}
	} else {
		// if we just tried to become a mage or apprentice, but
		// were unable to study, reset unit to whatever it was before.
		if (reset_man != -1)
			u->type = reset_man;
	}
}

void Game::DoMoveEnter(Unit *unit,ARegion *region,Object **obj)
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

			if (forbid && region->IsSafeRegion())
			{
				unit->Error("ENTER: No battles allowed in safe regions.");
				continue;
			}

			if (forbid && !(unit->canattack && unit->IsAlive())) {
				unit->Error(AString("ENTER: Unable to attack ") +
						*(forbid->name));
				continue;
			}

			int done = 0;
			while (forbid)
			{
				int result = RunBattle(region, unit, forbid, 0, 0);
				if (result == BATTLE_IMPOSSIBLE) {
					unit->Error(AString("ENTER: Unable to attack ")+
							*(forbid->name));
					done = 1;
					break;
				}
				if (!unit->canattack || !unit->IsAlive()) {
					done = 1;
					break;
				}
				forbid = to->ForbiddenBy(region, unit);
			}
			if (done) continue;

			unit->MoveUnit(to);
			unit->Event(AString("Enters ") + *(to->name) + ".");
			*obj = to;
		} else {
			if (i == MOVE_OUT) {
				if (TerrainDefs[region->type].similar_type == R_OCEAN &&
						(!unit->CanSwim() ||
						 unit->GetFlag(FLAG_NOCROSS_WATER)))
				{
					unit->Error("MOVE: Can't leave ship.");
					continue;
				}

				Object * to = region->GetDummy();
				unit->MoveUnit(to);
				*obj = to;
			}
		}
	}
}

Location *Game::DoAMoveOrder(Unit *unit, ARegion *region, Object *obj)
{
	MoveOrder *o = (MoveOrder *) unit->monthorders;
	MoveDir *x;
	ARegion *newreg;
	AString road, temp;
	int movetype, cost, startmove, weight;
	Unit *ally, *forbid;
	Location *loc;

	if (!o->dirs.Num()) {
		delete o;
		unit->monthorders = 0;
		return 0;
	}

	x = (MoveDir *) o->dirs.First();

	if (x->dir == MOVE_IN) {
		if (obj->inner == -1) {
			unit->Error("MOVE: Can't move IN there.");
			goto done_moving;
		}
		newreg = regions.GetRegion(obj->inner);
		if (obj->type == O_GATEWAY) {
			// Gateways should only exist in the nexus, and move the
			// user to a semi-random instance of the target terrain
			// type, so select where they will actually move to.
			ARegionArray *level = regions.GetRegionArray(newreg->zloc);
			// match levels to try for, in order:
			// 0 - completely empty towns
			// 1 - towns with only guardsmen
			// 2 - towns with guardsmen and other players
			// 3 - completely empty hexes
			// 4 - anywhere that matches terrain (out of options)
			int match = 0;
			int candidates = 0;
			while (!candidates && match < 5) {
				for (int x = 0; x < level->x; x++)
					for (int y = 0; y < level->y; y++) {
						ARegion *scanReg = level->GetRegion(x, y);
						if (!scanReg)
							continue;
						if (TerrainDefs[scanReg->type].similar_type != TerrainDefs[newreg->type].similar_type)
							continue;
						if (match < 3 && !scanReg->town)
							continue;
						if (match == 4) {
							candidates++;
							continue;
						}
						int guards = 0;
						int others = 0;
						forlist(&scanReg->objects) {
							Object *o = (Object *) elem;
							forlist(&o->units) {
								Unit *u = (Unit *) elem;
								if (u->faction->num == guardfaction)
									guards = 1;
								else
									others = 1;
							}
						}
						switch (match) {
							case 0:
								if (guards || others)
									continue;
								break;
							case 1:
								if (!guards || others)
									continue;
								break;
							case 2:
								if (!guards)
									continue;
								break;
							case 3:
								if (others)
									continue;
								break;
						}
						candidates++;
					}
				if (!candidates)
					match++;
			}
			if (candidates) {
				candidates = getrandom(candidates);
				for (int x = 0; x < level->x; x++)
					for (int y = 0; y < level->y; y++) {
						ARegion *scanReg = level->GetRegion(x, y);
						if (!scanReg)
							continue;
						if (TerrainDefs[scanReg->type].similar_type != TerrainDefs[newreg->type].similar_type)
							continue;
						if (match < 3 && !scanReg->town)
							continue;
						if (match == 4) {
							candidates++;
							continue;
						}
						int guards = 0;
						int others = 0;
						forlist(&scanReg->objects) {
							Object *o = (Object *) elem;
							forlist(&o->units) {
								Unit *u = (Unit *) elem;
								if (u->faction->num == guardfaction)
									guards = 1;
								else
									others = 1;
							}
						}
						switch (match) {
							case 0:
								if (guards || others)
									continue;
								break;
							case 1:
								if (!guards || others)
									continue;
								break;
							case 2:
								if (!guards)
									continue;
								break;
							case 3:
								if (others)
									continue;
								break;
						}
						if (!candidates--) {
							newreg = scanReg;
						}
					}
			}
		}
	} else if (x->dir == MOVE_PAUSE) {
		newreg = region;
	} else {
		newreg = region->neighbors[x->dir];
	}

	if (!newreg) {
		unit->Error(AString("MOVE: Can't move that direction."));
		goto done_moving;
	}

	unit->movepoints += unit->CalcMovePoints(region);

	road = "";
	startmove = 0;
	movetype = unit->MoveType(region);
	cost = newreg->MoveCost(movetype, region, x->dir, &road);
	if (x->dir == MOVE_PAUSE)
		cost = 1;
	if (region->type == R_NEXUS) {
		cost = 1;
		startmove = 1;
	}
	if ((TerrainDefs[region->type].similar_type == R_OCEAN) &&
			(!unit->CanSwim() ||
			unit->GetFlag(FLAG_NOCROSS_WATER))) {
		unit->Error("MOVE: Can't move while in the ocean.");
		goto done_moving;
	}
	weight = unit->items.Weight();
	if ((TerrainDefs[region->type].similar_type == R_OCEAN) &&
			(TerrainDefs[newreg->type].similar_type != R_OCEAN) &&
			!unit->CanWalk(weight) &&
			!unit->CanRide(weight) &&
			!unit->CanFly(weight)) {
		unit->Error("Must be able to walk to climb out of the ocean.");
		goto done_moving;
	}
	if (movetype == M_NONE) {
		unit->Error("MOVE: Unit is overloaded and cannot move.");
		goto done_moving;
	}

	// If we're moving in the same direction as last month and
	// have stored movement points, then add in those stored
	// movement points, but make sure that these are only used
	// towards entering the hex we were trying to enter
	if (!unit->moved &&
			unit->movepoints >= Globals->MAX_SPEED &&
			unit->movepoints < cost * Globals->MAX_SPEED &&
			x->dir == unit->savedmovedir) {
		while (unit->savedmovement > 0 &&
				unit->movepoints < cost * Globals->MAX_SPEED) {
			unit->movepoints += Globals->MAX_SPEED;
			unit->savedmovement--;
		}
		unit->savedmovement = 0;
		unit->savedmovedir = -1;
	}

	if (unit->movepoints < cost * Globals->MAX_SPEED)
		return 0;

	if (x->dir == MOVE_PAUSE) {
		unit->Event(AString("Pauses to admire the scenery in ") + region->ShortPrint(&regions) + ".");
		unit->movepoints -= cost * Globals->MAX_SPEED;
		unit->moved += cost;
		o->dirs.Remove(x);
		delete x;
		return 0;
	}

	if ((TerrainDefs[newreg->type].similar_type == R_OCEAN) &&
			(!unit->CanSwim() ||
			unit->GetFlag(FLAG_NOCROSS_WATER))) {
		unit->Event(AString("Discovers that ") +
				newreg->ShortPrint(&regions) + " is " +
				TerrainDefs[newreg->type].name + ".");
		goto done_moving;
	}

	if (unit->type == U_WMON && newreg->town && newreg->IsGuarded()) {
		unit->Event("Monsters don't move into guarded towns.");
		goto done_moving;
	}

	if (unit->guard == GUARD_ADVANCE) {
		ally = newreg->ForbiddenByAlly(unit);
		if (ally && !startmove) {
			unit->Event(AString("Can't ADVANCE: ") + *(newreg->name) +
						" is guarded by " + *(ally->name) + ", an ally.");
			goto done_moving;
		}
	}

	if (o->advancing) unit->guard = GUARD_ADVANCE;

	forbid = newreg->Forbidden(unit);
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

	if (unit->guard == GUARD_GUARD) unit->guard = GUARD_NONE;

	unit->alias = 0;
	unit->movepoints -= cost * Globals->MAX_SPEED;
	unit->moved += cost;
	unit->MoveUnit(newreg->GetDummy());
	unit->DiscardUnfinishedShips();

	switch (movetype) {
		case M_WALK:
		default:
			temp = AString("Walks ") + road;
			break;
		case M_RIDE:
			temp = AString("Rides ") + road;
			unit->Practice(S_RIDING);
			break;
		case M_FLY:
			temp = "Flies ";
			unit->Practice(S_SUMMON_WIND);
			unit->Practice(S_RIDING);
			break;
		case M_SWIM:
			temp = AString("Swims ");
			break;
	}
	unit->Event(temp + AString("from ") + region->ShortPrint(&regions)
			+ AString(" to ") + newreg->ShortPrint(&regions) +
			AString("."));

	if (forbid) {
		unit->advancefrom = region;
	}

	// TODO: Should we get a transit report on the starting region?
	if (Globals->TRANSIT_REPORT != GameDefs::REPORT_NOTHING) {
		if (!(unit->faction->IsNPC())) newreg->visited = 1;
		// Update our visit record in the region we are leaving.
		Farsight *f;
		forlist(&region->passers) {
			f = (Farsight *)elem;
			if (f->unit == unit) {
				// We moved into here this turn
				if (x->dir < MOVE_IN) {
					f->exits_used[x->dir] = 1;
				}
			}
		}
		// And mark the hex being entered
		f = new Farsight;
		f->faction = unit->faction;
		f->level = 0;
		f->unit = unit;
		if (x->dir < MOVE_IN) {
			f->exits_used[region->GetRealDirComp(x->dir)] = 1;
		}
		newreg->passers.Add(f);
	}

	region = newreg;

	o->dirs.Remove(x);
	delete x;

	loc = new Location;
	loc->unit = unit;
	loc->region = region;
	loc->obj = 0;
	return loc;

done_moving:
	delete o;
	unit->monthorders = 0;
	return 0;
}

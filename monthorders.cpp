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

#include <random>
#include <stdlib.h>

#include "game.h"
#include "gamedata.h"
#include "quests.h"
#include "rng.h"

using namespace std;

void Game::RunMovementOrders()
{
    int phase, error;
    Unit *u;
    std::list<Location *> locs;
    Location *l;
    AString order;

    for (phase = 0; phase < Globals->MAX_SPEED; phase++) {
        for (const auto r : regions) {
            for (const auto o : r->objects) {
                for (const auto u : o->units) { DoMoveEnter(u, r); }
            }
        }
        for (const auto r : regions) {
            for (const auto o : r->objects) {
                error = 1;
                if (o->IsFleet()) {
                    u = o->GetOwner();
                    if (!u) continue;
                    if (u->phase >= phase) continue;
                    if (u->nomove) {
                        error = 4;
                    } else if (u->monthorders && u->monthorders->type == O_SAIL) {
                        u->phase = phase;
                        if (o->incomplete < 50) {
                            l = Do1SailOrder(r, o, u);
                            if (l) locs.push_back(l);
                            error = 0;
                        } else {
                            error = 3;
                        }
                    } else {
                        error = 2;
                    }
                }
                if (error > 0) {
                    for (const auto u : o->units) {
                        if (u && u->monthorders && u->monthorders->type == O_SAIL) {
                            switch (error) {
                            case 1: u->error("SAIL: Must be on a ship."); break;
                            case 2: u->error("SAIL: Owner must issue fleet directions."); break;
                            case 3: u->error("SAIL: Fleet is too damaged to sail."); break;
                            case 4: u->error("SAIL: Unable to sail due to combat losses."); break;
                            }
                            delete u->monthorders;
                            u->monthorders = nullptr;
                        }
                    }
                }
            }
        }
        for (const auto r : regions) {
            for (auto o : r->objects) {
                for (const auto u : o->units) {
                    if (u->phase >= phase) continue;
                    u->phase = phase;
                    if (u && !u->nomove && u->monthorders &&
                        (u->monthorders->type == O_MOVE || u->monthorders->type == O_ADVANCE)) {
                        l = DoAMoveOrder(u, r, o);
                        if (l) locs.push_back(l);
                    }
                }
            }
        }
        DoMovementAttacks(locs);
        std::for_each(locs.begin(), locs.end(), [](Location *loc) { delete loc; });
        locs.clear();
    }

    // Do a final round of Enters after the phased movement is done,
    // in case such a thing is at the end of a move chain
    for (const auto r : regions) {
        for (const auto o : r->objects) {
            for (const auto u : o->units) { DoMoveEnter(u, r); }
        }
    }

    // Queue remaining moves
    for (const auto r : regions) {
        for (const auto o : r->objects) {
            for (const auto u : o->units) {
                MoveOrder *mo = dynamic_cast<MoveOrder *>(u->monthorders);
                if (!mo || (mo->type != O_MOVE && mo->type != O_ADVANCE)) {
                    u->savedmovement = 0;
                    u->savedmovedir = -1;
                    continue;
                }

                if (mo->dirs.size() > 0) {
                    if (u->nomove) {
                        u->savedmovedir = -1;
                        u->savedmovement = 0;
                    } else {
                        auto d = mo->dirs.front();
                        if (u->savedmovedir != d->dir) { u->savedmovement = 0; }
                        u->savedmovement += u->movepoints / Globals->MAX_SPEED;
                        u->savedmovedir = d->dir;
                    }

                    string tOrder = (mo->advancing ? "ADVANCE" : "MOVE");
                    string temp = tOrder + ": Unit has insufficient movement points; remaining moves queued.";
                    u->event(temp, "movement");

                    for (const auto d : mo->dirs) {
                        tOrder += " ";

                        // Add the appropriate direction token
                        if (d->dir < NDIRS)
                            tOrder += DirectionAbrs[d->dir];
                        else if (d->dir == MOVE_IN)
                            tOrder += "IN";
                        else if (d->dir == MOVE_OUT)
                            tOrder += "OUT";
                        else if (d->dir == MOVE_PAUSE)
                            tOrder += "P";
                        else
                            tOrder += to_string(d->dir - MOVE_ENTER);
                    }
                    u->oldorders.push_front(tOrder);
                }
            }
            u = o->GetOwner();
            if (o->IsFleet() && u && !u->nomove && u->monthorders && u->monthorders->type == O_SAIL) {
                SailOrder *so = dynamic_cast<SailOrder *>(u->monthorders);
                if (so->dirs.size() > 0) {
                    u->event("SAIL: Can't sail that far; remaining moves queued.", "movement");
                    string tOrder = "SAIL";
                    for (const auto d : so->dirs) {
                        tOrder += " ";
                        if (d->dir == MOVE_PAUSE)
                            tOrder += "P";
                        else
                            tOrder += DirectionAbrs[d->dir];
                    }
                    u->oldorders.push_front(tOrder);
                }
            }
        }
    }
}

Location *Game::Do1SailOrder(ARegion *reg, Object *fleet, Unit *cap)
{
    SailOrder *o = dynamic_cast<SailOrder *>(cap->monthorders);
    int stop, wgt, slr, nomove, cost;
    std::set<Faction *> facs;
    ARegion *newreg;
    Location *loc;

    fleet->movepoints += fleet->GetFleetSpeed(0);
    stop = 0;
    wgt = 0;
    slr = 0;
    nomove = 0;
    for (const auto unit : fleet->units) {
        facs.insert(unit->faction);
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
    } else if (!o->dirs.size()) {
        stop = 1;
    } else if (wgt > fleet->FleetCapacity()) {
        cap->error("SAIL: Fleet is overloaded.");
        stop = 1;
    } else if (slr < fleet->GetFleetSize()) {
        cap->error("SAIL: Not enough sailors.");
        stop = 1;
    } else {
        auto x = o->dirs.front();
        if (x->dir == MOVE_PAUSE) {
            newreg = reg;
        } else {
            newreg = reg->neighbors[x->dir];
        }
        cost = 1;

        // Blizzard effect
        if (newreg && newreg->weather == W_BLIZZARD && !newreg->clearskies) { cost = 4; }

        if (Globals->WEATHER_EXISTS) {
            if (newreg && newreg->weather != W_NORMAL && !newreg->clearskies) cost = 2;
        }
        if (x->dir == MOVE_PAUSE) { cost = 1; }
        // We probably shouldn't see terrain-based errors until
        // we accumulate enough movement points to get there
        if (fleet->movepoints < cost * Globals->MAX_SPEED) return 0;
        if (!newreg) {
            cap->error("SAIL: Can't sail that way.");
            stop = 1;
        } else if (x->dir == MOVE_PAUSE) {
            // Can always do maneuvers
        } else if (fleet->flying < 1 && !newreg->IsCoastalOrLakeside()) {
            cap->error("SAIL: Can't sail inland.");
            stop = 1;
        } else if ((fleet->flying < 1) && (TerrainDefs[reg->type].similar_type != R_OCEAN) &&
                   (TerrainDefs[newreg->type].similar_type != R_OCEAN)) {
            cap->error("SAIL: Can't sail inland.");
            stop = 1;
        } else if (fleet->SailThroughCheck(x->dir) < 1) {
            cap->error(
                "SAIL: Could not sail " + DirectionStrs[x->dir] + " from " + reg->ShortPrint().const_str() +
                ". Cannot sail through land."
            );
            stop = 1;
        }

        if (!stop) {
            // Check the new region for barriers and the fleet units for keys to the barriers
            int needed_key = -1;
            for (auto o : newreg->objects) {
                if (ObjectDefs[o->type].flags & ObjectType::KEYBARRIER) { needed_key = ObjectDefs[o->type].key_item; }
            }
            if (needed_key != -1) { // we found a barrier
                bool has_key = false;
                for (const auto u : fleet->units) {
                    if (u->items.GetNum(needed_key) > 0) {
                        has_key = true;
                        break;
                    }
                }
                if (!has_key) {
                    cap->error(
                        "SAIL: Can't sail " + DirectionStrs[x->dir] + " from " + reg->ShortPrint().const_str() +
                        " due to mystical barrier."
                    );
                    stop = 1;
                }
            }
        }

        // We could have been stopped by not having the key above.
        if (!stop) {
            fleet->movepoints -= cost * Globals->MAX_SPEED;
            if (x->dir != MOVE_PAUSE) {
                // this can invalidate the object iterator if we are in an iteration
                fleet->MoveObject(newreg);
                fleet->SetPrevDir(reg->GetRealDirComp(x->dir));
            }
            for (const auto unit : fleet->units) {
                unit->moved += cost;
                if (unit->guard == GUARD_GUARD) unit->guard = GUARD_NONE;
                unit->alias = 0;
                unit->PracticeAttribute("wind");
                if (unit->monthorders) {
                    if (unit->monthorders->type == O_SAIL) unit->Practice(S_SAILING);
                    if (unit->monthorders->type == O_MOVE) {
                        delete unit->monthorders;
                        unit->monthorders = nullptr;
                    }
                }
                unit->DiscardUnfinishedShips();
                facs.insert(unit->faction);
            }

            for (const auto f : facs) {
                string temp = fleet->name;
                temp += (x->dir == MOVE_PAUSE ? " performs maneuvers in " : " sails from ") +
                        string(reg->ShortPrint().const_str());
                if (x->dir != MOVE_PAUSE) { temp += " to " + string(newreg->ShortPrint().const_str()); }
                f->event(temp, "sail");
            }
            if (Globals->TRANSIT_REPORT != GameDefs::REPORT_NOTHING && x->dir != MOVE_PAUSE) {
                if (!(cap->faction->is_npc)) newreg->visited = 1;
                for (const auto unit : fleet->units) {
                    // Everyone onboard gets to see the sights
                    // Note the hex being left
                    for (const auto f : reg->passers) {
                        if (f->unit == unit) {
                            // We moved into here this turn
                            f->exits_used[x->dir] = 1;
                        }
                    }
                    // And mark the hex being entered
                    Farsight *f = new Farsight;
                    f->faction = unit->faction;
                    f->level = 0;
                    f->unit = unit;
                    f->exits_used[reg->GetRealDirComp(x->dir)] = 1;
                    newreg->passers.push_back(f);
                }
            }
            reg = newreg;
            if (newreg->ForbiddenShip(fleet)) {
                string temp = fleet->name + " is stopped by guards in " + newreg->ShortPrint().const_str() + ".";
                cap->faction->event(temp, "sail");
                stop = 1;
            }
            std::erase(o->dirs, x);
            delete x;
        }
    }

    if (stop) {
        // Clear out everyone's orders
        for (const auto unit : fleet->units) {
            if (unit->monthorders && unit->monthorders->type == O_SAIL) {
                delete unit->monthorders;
                unit->monthorders = nullptr;
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
    for (const auto r : regions) {
        for (const auto obj : r->objects) {
            for (const auto u : obj->units) {
                if (u->monthorders) {
                    if (u->monthorders->type == O_TEACH) {
                        Do1TeachOrder(r, u);
                        delete u->monthorders;
                        u->monthorders = nullptr;
                    }
                }
            }
        }
    }
}

void Game::Do1TeachOrder(ARegion *reg, Unit *unit)
{
    /* First pass, find how many to teach */
    if (Globals->LEADERS_EXIST && !unit->IsLeader()) {
        /* small change to handle Ceran's mercs */
        if (!unit->GetMen(I_MERC)) {
            // Mercs can teach even though they are not leaders.
            // They cannot however improve their own skills
            unit->error("TEACH: Only leaders can teach.");
            return;
        }
    }

    int students = 0;
    TeachOrder *order = dynamic_cast<TeachOrder *>(unit->monthorders);
    reg->deduplicate_unit_list(order->targets, unit->faction->num);
    for (auto it = order->targets.begin(); it != order->targets.end();) {
        UnitId *id = *it;
        Unit *target = reg->GetUnitId(id, unit->faction->num);
        if (!target) {
            unit->error("TEACH: No such unit.");
            it = order->targets.erase(it);
            delete id;
            continue;
        }
        if (target->faction->get_attitude(unit->faction->num) < A_FRIENDLY) {
            unit->error("TEACH: " + target->name + " is not a member of a friendly faction.");
            it = order->targets.erase(it);
            delete id;
            continue;
        }
        if (!target->monthorders || target->monthorders->type != O_STUDY) {
            unit->error("TEACH: " + target->name + " is not studying.");
            it = order->targets.erase(it);
            delete id;
            continue;
        }

        StudyOrder *so = dynamic_cast<StudyOrder *>(target->monthorders);
        int sk = so->skill;
        if (unit->GetRealSkill(sk) <= target->GetRealSkill(sk)) {
            unit->error("TEACH: " + target->name + " is not studying a skill you can teach.");
            it = order->targets.erase(it);
            delete id;
            continue;
        }
        // Check whether it's a valid skill to teach
        if (SkillDefs[sk].flags & SkillType::NOTEACH) {
            unit->error("TEACH: " + SkillDefs[sk].name + " cannot be taught.");
            it = order->targets.erase(it);
            delete id;
            continue;
        }
        students += target->GetMen();
        ++it;
    }

    if (!students) return;

    int days = (30 * unit->GetMen() * Globals->STUDENTS_PER_TEACHER);

    /* We now have a list of valid targets */
    for (const auto id : order->targets) {
        Unit *u = reg->GetUnitId(id, unit->faction->num);

        int umen = u->GetMen();
        int tempdays = (umen * days) / students;
        if (tempdays > 30 * umen) tempdays = 30 * umen;
        days -= tempdays;
        students -= umen;

        StudyOrder *o = dynamic_cast<StudyOrder *>(u->monthorders);
        o->days += tempdays;
        if (o->days > 30 * umen) {
            days += o->days - 30 * umen;
            o->days = 30 * umen;
        }
        unit->event("Teaches " + SkillDefs[o->skill].name + " to " + u->name + ".", "teach");
        // The TEACHER may learn something in this process!
        unit->Practice(o->skill);
    }
    std::for_each(order->targets.begin(), order->targets.end(), [](UnitId *id) { delete id; });
    order->targets.clear();
}

void Game::Run1BuildOrder(ARegion *r, Object *obj, Unit *u)
{
    Object *buildobj;
    int questcomplete = 0;
    string quest_rewards;

    if (!Globals->BUILD_NO_TRADE && !ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
        u->error("BUILD: Faction can't produce in that many regions.");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    buildobj = r->GetObject(u->build);
    // plain "BUILD" order needs to check that the unit is in something
    // that can be built AFTER enter/leave orders have executed
    if (!buildobj || buildobj->type == O_DUMMY) buildobj = obj;

    if (!buildobj || buildobj->type == O_DUMMY) {
        u->error("BUILD: Nothing to build.");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }
    int type = buildobj->type;
    int sk = lookup_skill(ObjectDefs[type].skill);
    if (sk == -1) {
        u->error("BUILD: Can't build " + string(ObjectDefs[type].name) + ".");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    int usk = u->GetSkill(sk);
    if (usk < ObjectDefs[type].level) {
        u->error("BUILD: Can't build " + string(ObjectDefs[type].name) + ".");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    int needed = buildobj->incomplete;
    // AS
    if (((ObjectDefs[type].flags & ObjectType::NEVERDECAY) || !Globals->DECAY) && needed < 1) {
        u->error("BUILD: " + string(ObjectDefs[type].name) + " is finished.");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    // AS
    if (needed <= -(ObjectDefs[type].maxMaintenance)) {
        u->error("BUILD: " + string(ObjectDefs[type].name) + " does not yet require maintenance.");
        delete u->monthorders;
        u->monthorders = nullptr;
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
        u->error("BUILD: Don't have the required materials to build " + string(ObjectDefs[type].name) + ".");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    int num = u->GetMen() * usk;

    // AS
    string job;
    if (needed < 1) {
        // This looks wrong, but isn't.
        // If a building has a maxMaintenance of 75 and the road is at
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
            if (quests.check_build_target(r, type, u, &quest_rewards)) { questcomplete = 1; }
        }
    }

    /* Perform the build */

    if (obj != buildobj) u->MoveUnit(buildobj);

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
    u->event(job + buildobj->name, "build");
    if (questcomplete) { u->event("You have completed a quest! " + quest_rewards, "quest"); }
    u->Practice(sk);
}

/* Alternate processing for building item-type ship
 * objects and instantiating fleets.
 */
void Game::RunBuildShipOrder(ARegion *r, Object *obj, Unit *u)
{
    int ship, skill, level, maxbuild, unfinished, output, percent;
    AString skname;

    ship = abs(u->build);
    skill = lookup_skill(ItemDefs[ship].pSkill);
    level = u->GetSkill(skill);

    if (skill == -1) {
        u->error("BUILD: Can't build " + string(ItemDefs[ship].name) + ".");
        return;
    }

    // get needed to complete
    maxbuild = 0;
    if ((u->monthorders) && (u->monthorders->type == O_BUILD)) {
        BuildOrder *b = dynamic_cast<BuildOrder *>(u->monthorders);
        maxbuild = b->needtocomplete;
    }
    if (maxbuild < 1) {
        // Our helpers have already finished the hard work, so
        // just put the finishing touches on the new vessel
        unfinished = 0;
        // Also clear our month orders since it's done.
        delete u->monthorders;
        u->monthorders = nullptr;
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
        if (unfinished < 0) unfinished = 0;
    }
    u->items.SetNum(ship, unfinished);

    // practice
    u->Practice(skill);

    if (unfinished == 0) {
        u->event("Finishes building a " + ItemDefs[ship].name + " in " + r->ShortPrint().const_str() + ".", "build");
        CreateShip(r, u, ship);
    } else {
        percent = 100 * output / ItemDefs[ship].pMonths;
        u->event(
            "Performs construction work on a " + ItemDefs[ship].name + " (" + to_string(percent) + "%) in " +
                r->ShortPrint().const_str() + ".",
            "build", r
        );
    }
}

void Game::AddNewBuildings(ARegion *r)
{
    int i;
    for (const auto obj : r->objects) {
        for (const auto u : obj->units) {
            if (u->monthorders && u->monthorders->type == O_BUILD) {
                BuildOrder *o = dynamic_cast<BuildOrder *>(u->monthorders);

                // If BUILD order was marked for creating new building
                // in parse phase, it is time to create one now.
                if (o->new_building != -1) {
                    if (o->new_building == u->object->type && u->object->incomplete > 0) {
                        // we have a complete for the type of building we are in and it's not finished, so no new
                        // building.
                        u->build = u->object->num; // keep the current building as the build target
                        break;
                    }

                    for (i = 1; i < 100; i++) {
                        if (!r->GetObject(i)) break;
                    }
                    if (i < 100) {
                        Object *obj = new Object(r);
                        obj->type = o->new_building;
                        obj->incomplete = ObjectDefs[obj->type].cost;
                        obj->num = i;
                        obj->set_name("Building");
                        u->build = obj->num;
                        r->objects.push_back(obj);

                        // This moves unit to a new building.
                        // This unit might be processed again but from new object.
                        u->MoveUnit(obj);
                        // This why we need to unset new_building so it will not
                        // try to create new object again.
                        o->new_building = -1;
                    } else {
                        u->error("BUILD: The region is full.");
                    }
                }
            }
        }
    }
}

void Game::RunBuildHelpers(ARegion *r)
{
    for (const auto obj : r->objects) {
        for (const auto u : obj->units) {
            if (u->monthorders && u->monthorders->type == O_BUILD) {
                BuildOrder *o = dynamic_cast<BuildOrder *>(u->monthorders);
                Object *tarobj = NULL;
                if (o->target) {
                    Unit *target = r->GetUnitId(o->target, u->faction->num);
                    if (!target) {
                        u->error("BUILD: No such unit to help.");
                        delete u->monthorders;
                        u->monthorders = nullptr;
                        continue;
                    }
                    // Make sure that unit is building
                    if (!target->monthorders || target->monthorders->type != O_BUILD) {
                        u->error("BUILD: Unit isn't building.");
                        delete u->monthorders;
                        u->monthorders = nullptr;
                        continue;
                    }
                    // Make sure that unit considers you friendly!
                    if (target->faction->get_attitude(u->faction->num) < A_FRIENDLY) {
                        u->error("BUILD: Unit you are helping rejects your help.");
                        delete u->monthorders;
                        u->monthorders = nullptr;
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
                        int skill = lookup_skill(ItemDefs[ship].pSkill);
                        int level = u->GetSkill(skill);
                        int needed = 0;
                        if (target->monthorders && (target->monthorders->type == O_BUILD)) {
                            BuildOrder *border = dynamic_cast<BuildOrder *>(target->monthorders);
                            needed = border->needtocomplete;
                        }
                        if (needed < 1) {
                            u->error("BUILD: Construction is already complete.");
                            delete u->monthorders;
                            u->monthorders = nullptr;
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
                            if (target->monthorders && (target->monthorders->type == O_BUILD)) {
                                BuildOrder *border = dynamic_cast<BuildOrder *>(target->monthorders);
                                border->needtocomplete = unfinished;
                            }
                        } else {
                            // CreateShip(r, target, ship);
                            // don't create the ship yet; leave that for the unit we're helping
                            target->items.SetNum(ship, 1);
                            if (target->monthorders && (target->monthorders->type == O_BUILD)) {
                                BuildOrder *border = dynamic_cast<BuildOrder *>(target->monthorders);
                                border->needtocomplete = 0;
                            }
                        }
                        int percent = 100 * output / ItemDefs[ship].pMonths;
                        u->event(
                            "Helps " + target->name + " with construction of a " + ItemDefs[ship].name + " (" +
                                to_string(percent) + "%) in " + r->ShortPrint().const_str() + ".",
                            "build", r
                        );
                    }
                    // no need to move unit if item-type ships
                    // are being built. (leave this commented out)
                    // if (tarobj == NULL) tarobj = target->object;
                    if ((tarobj != NULL) && (u->object != tarobj)) u->MoveUnit(tarobj);
                } else {
                    Object *buildobj;
                    if (u->build > 0) {
                        buildobj = r->GetObject(u->build);
                        if (buildobj && buildobj != r->GetDummy() && buildobj != u->object) { u->MoveUnit(buildobj); }
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
void Game::CreateShip(ARegion *r, Unit *u, int ship)
{
    Object *obj = u->object;
    // Do we need to create a new fleet?
    bool newfleet = true;
    if (u->object->IsFleet()) {
        newfleet = false;
        bool fleet_flying = obj->flying;
        bool ship_flies = ItemDefs[ship].fly > 0;

        switch (Globals->NEW_SHIP_JOINS_FLEET_BEHAVIOR) {
            case GameDefs::NewShipJoinsFleetBehavior::ALL_CROSS_JOIN:
                break;
            case GameDefs::NewShipJoinsFleetBehavior::ONLY_FLYING_CROSS_JOIN:
                if (ship_flies) break;
                newfleet = fleet_flying; // we aren't a flying ship, so only need a new fleet if fleet is flying.
                break;
            case GameDefs::NewShipJoinsFleetBehavior::NO_CROSS_JOIN:
                newfleet = (fleet_flying != ship_flies); // true if they are different, else false
                break;
            default:
                // this is an impossible case as we exhaustively check the enum
                break;
        }
    }
    if (newfleet) {
        // create a new fleet
        Object *fleet = new Object(r);
        fleet->type = O_FLEET;
        fleet->num = shipseq++;
        fleet->set_name("Ship");
        fleet->AddShip(ship);
        u->object->region->objects.push_back(fleet);
        u->MoveUnit(fleet);
        fleet->FleetCapacity();
    } else {
        obj->AddShip(ship);
        obj->FleetCapacity();
    }
}

// This is a utility function used by both ship building and unit production to correctly consume
// input items for production.  In the case of ORINPUT items it will make sure to consume items from the
// unit itself before consuming from the shared pool.
// Returns the number of items created.
int Game::consume_production_inputs(Unit *u, int item, int maxproduced)
{
    unsigned int maxInputs = sizeof(ItemDefs->pInput) / sizeof(ItemDefs->pInput[0]);

    if (ItemDefs[item].flags & ItemType::ORINPUTS) {
        // Figure out the max we can produce based on the inputs
        int count = 0;
        unsigned int c;
        for (c = 0; c < sizeof(ItemDefs->pInput) / sizeof(ItemDefs->pInput[0]); c++) {
            int i = ItemDefs[item].pInput[c].item;
            if (i != -1) count += u->GetSharedNum(i) / ItemDefs[item].pInput[c].amt;
        }
        if (maxproduced > count) maxproduced = count;
        count = maxproduced;

        if (count < 1) return 0;

        // Now consume the items from the unit itself first if possible.
        for (c = 0; c < maxInputs; c++) {
            int i = ItemDefs[item].pInput[c].item;
            int a = ItemDefs[item].pInput[c].amt;
            if (i != -1) {
                // Consume from the unit's own items first
                int amt = u->items.GetNum(i);
                if (count > amt / a) {
                    count -= amt / a;
                    u->items.SetNum(i, amt - ((amt / a) * a));
                } else {
                    u->items.SetNum(i, amt - (count * a));
                    count = 0;
                }
            }
        }

        // If we paid for everything, return now.
        if (count == 0) return maxproduced;

        // Deduct the items spent
        for (c = 0; c < maxInputs; c++) {
            int i = ItemDefs[item].pInput[c].item;
            int a = ItemDefs[item].pInput[c].amt;
            if (i != -1) {
                int amt = u->GetSharedNum(i);
                if (count > amt / a) {
                    count -= amt / a;
                    u->ConsumeShared(i, (amt / a) * a);
                } else {
                    u->ConsumeShared(i, count * a);
                    count = 0;
                }
            }
        }
    } else {
        // Figure out the max we can produce based on the inputs
        unsigned int c;
        for (c = 0; c < maxInputs; c++) {
            int i = ItemDefs[item].pInput[c].item;
            if (i != -1) {
                int amt = u->GetSharedNum(i);
                if ((amt / ItemDefs[item].pInput[c].amt) < maxproduced) {
                    maxproduced = amt / ItemDefs[item].pInput[c].amt;
                }
            }
        }

        // If we can't produce anything, return 0
        if (maxproduced < 1) return 0;

        // Deduct the items spent
        for (c = 0; c < maxInputs; c++) {
            int i = ItemDefs[item].pInput[c].item;
            int a = ItemDefs[item].pInput[c].amt;
            if (i != -1) { u->ConsumeShared(i, maxproduced * a); }
        }
    }
    return maxproduced;
}

/* Checks and returns the amount of ship construction,
 * handles material use and practice for both the main
 * shipbuilders and the helpers.
 */
int Game::ShipConstruction(ARegion *r, Unit *u, Unit *target, int level, int needed, int ship)
{
    if (!Globals->BUILD_NO_TRADE && !ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
        u->error("BUILD: Faction can't produce in that many regions.");
        delete u->monthorders;
        u->monthorders = nullptr;
        return 0;
    }

    if (level < ItemDefs[ship].pLevel) {
        u->error("BUILD: Can't build " + ItemDefs[ship].name + ".");
        delete u->monthorders;
        u->monthorders = nullptr;
        return 0;
    }

    // are there unfinished ship items of the given type?
    int unfinished = target->items.GetNum(ship);

    int number = u->GetMen() * level + u->GetProductionBonus(ship);

    // find the max we can possibly produce based on man-months of labor
    int maxproduced = (ItemDefs[ship].flags & ItemType::SKILLOUT) ? u->GetMen() : number;

    // adjust maxproduced for items needed until completion
    if (needed < maxproduced) maxproduced = needed;

    // adjust maxproduced for unfinished ships
    if ((unfinished > 0) && (maxproduced > unfinished)) maxproduced = unfinished;

    maxproduced = consume_production_inputs(u, ship, maxproduced);
    if (maxproduced < 1) {
        // We don't have enough input items to produce anything
        u->error("BUILD: Don't have the required materials to build " + ItemDefs[ship].name + ".");
        delete u->monthorders;
        u->monthorders = nullptr;
        return 0;
    }
    r->improvement += maxproduced; // regional economic improvement

    int output = maxproduced * ItemDefs[ship].pOut;
    if (ItemDefs[ship].flags & ItemType::SKILLOUT) output *= level;

    delete u->monthorders;
    u->monthorders = nullptr;

    return output;
}

void Game::RunMonthOrders()
{
    for (const auto r : regions) {
        RunIdleOrders(r);
        RunStudyOrders(r);
        AddNewBuildings(r);
        RunBuildHelpers(r);
        RunProduceOrders(r);
    }
}

void Game::RunUnitProduce(ARegion *r, Unit *u)
{
    ProduceOrder *o = (ProduceOrder *)u->monthorders;

    for (const auto& p : r->products) {
        // PRODUCE orders for producing goods from the land
        // are shared among factions, and therefore handled
        // specially by the RunAProduction() function
        if (o->skill == p->skill && o->item == p->itemtype) return;
    }

    if (o->item == I_SILVER) {
        if (!o->quiet) u->error("Can't do that in this region.");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    if (o->item == -1 || ItemDefs[o->item].flags & ItemType::DISABLED) {
        std::string name = (o->item == -1) ? "that" : ItemString(o->item, 1, ALWAYSPLURAL);
        u->error("PRODUCE: Can't produce " + name + ".");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    int input = ItemDefs[o->item].pInput[0].item;
    if (input == -1) {
        u->error("PRODUCE: Can't produce " + ItemString(o->item, 1, ALWAYSPLURAL) + ".");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    int level = u->GetSkill(o->skill);
    if (level < ItemDefs[o->item].pLevel) {
        u->error("PRODUCE: Can't produce " + ItemString(o->item, 1, ALWAYSPLURAL) + ".");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    // LLS
    int number = u->GetMen() * level + u->GetProductionBonus(o->item);

    if (!ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
        u->error("PRODUCE: Faction can't produce in that many regions.");
        delete u->monthorders;
        u->monthorders = nullptr;
        return;
    }

    // find the max we can possibly produce based on man-months of labor
    int maxproduced;
    if (ItemDefs[o->item].flags & ItemType::SKILLOUT)
        maxproduced = u->GetMen();
    else if (ItemDefs[o->item].flags & ItemType::SKILLOUT_HALF)
        maxproduced = u->GetMen();
    else
        maxproduced = number / ItemDefs[o->item].pMonths;

    if (o->target > 0 && maxproduced > o->target) maxproduced = o->target;

    maxproduced = consume_production_inputs(u, o->item, maxproduced);
    r->improvement += maxproduced; // regional economic improvement

    // Now give the items produced
    int output = maxproduced * ItemDefs[o->item].pOut;
    if (ItemDefs[o->item].flags & ItemType::SKILLOUT) output *= level;
    if (ItemDefs[o->item].flags & ItemType::SKILLOUT_HALF) { output *= (level + 1) / 2; }

    u->items.SetNum(o->item, u->items.GetNum(o->item) + output);
    u->event("Produces " + ItemString(o->item, output) + " in " + r->ShortPrint().const_str() + ".", "produce", r);
    u->Practice(o->skill);
    o->target -= output;
    if (o->target > 0) {
        TurnOrder *tOrder = new TurnOrder;
        tOrder->repeating = 0;
        std::string order = "PRODUCE " + to_string(o->target) + " " + ItemDefs[o->item].abr;
        tOrder->turnOrders.push_back(order);
        u->turnorders.push_front(tOrder);
    }
    delete u->monthorders;
    u->monthorders = nullptr;
}

void Game::RunProduceOrders(ARegion *r)
{
    for (const auto obj : r->objects) {
        for (const auto u : obj->units) {
            if (u->monthorders) {
                if (u->monthorders->type == O_PRODUCE) {
                    RunUnitProduce(r, u);
                } else {
                    if (u->monthorders->type == O_BUILD) {
                        if (u->build >= 0) {
                            Run1BuildOrder(r, obj, u);
                        } else {
                            RunBuildShipOrder(r, obj, u);
                        }
                    }
                }
            }
        }
        // Cleanup any build 'complete' orders where the object has been built or save them off for next month if not
        for (const auto u : obj->units) {
            if (u->monthorders && u->monthorders->type == O_BUILD) {
                BuildOrder *o = dynamic_cast<BuildOrder *>(u->monthorders);
                if (o->until_complete) {
                    if ((u->build > 0 || o->target) && u->object->incomplete > 0) {
                        string order = "BUILD ";
                        if (o->target) {
                            Unit *t = r->GetUnitId(o->target, u->faction->num);
                            order += "HELP " + to_string(t->num);
                        } else {
                            string name = ObjectDefs[u->object->type].name;
                            if (name.find(" ") != std::string::npos) {
                                // If the name has spaces, we need to quote it
                                order += "\"" + name + "\"";
                            } else {
                                order += name;
                            }
                        }
                        order += " COMPLETE";
                        u->oldorders.push_front(order);
                    } else if (u->build < 0 || o->target) {
                        string order = "BUILD ";
                        BuildOrder *border = nullptr;
                        Unit *t;
                        if (o->target) {
                            t = r->GetUnitId(o->target, u->faction->num);
                            if (t->monthorders && (t->monthorders->type == O_BUILD)) {
                                border = dynamic_cast<BuildOrder *>(t->monthorders);
                            }
                        } else {
                            border = dynamic_cast<BuildOrder *>(u->monthorders);
                        }
                        if (border && border->needtocomplete > 0) {
                            if (o->target) {
                                order += "HELP " + to_string(t->num);
                            } else {
                                order += ItemDefs[-(u->build)].abr;
                            }
                            order += " COMPLETE";
                            u->oldorders.push_front(order);
                        }
                    }
                }
                delete u->monthorders;
                u->monthorders = nullptr; // clear the monthorders to avoid reprocessing
            }
        }
    }
    for (const auto& p : r->products) RunAProduction(r, p);
}

int Game::ValidProd(Unit *u, ARegion *r, Production *p)
{
    if (u->monthorders->type != O_PRODUCE) return 0;

    ProduceOrder *po = dynamic_cast<ProduceOrder *>(u->monthorders);
    if (p->itemtype == po->item && p->skill == po->skill) {
        if (p->skill == -1) {
            /* Factor for fractional productivity: 10 */
            po->productivity = (int)((float)(u->GetMen() * p->productivity / 10));
            return po->productivity;
        }
        int level = u->GetSkill(p->skill);
        if (level < ItemDefs[p->itemtype].pLevel) {
            u->error("PRODUCE: Unit isn't skilled enough to produce " + ItemDefs[p->itemtype].name + ".");
            delete u->monthorders;
            u->monthorders = nullptr;
            return 0;
        }

        //
        // Check faction limits on production. If the item is silver, then the
        // unit is entertaining or working, and the limit does not apply
        //
        if (p->itemtype != I_SILVER && !ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
            u->error("PRODUCE: Faction can't produce in that many regions.");
            delete u->monthorders;
            u->monthorders = nullptr;
            return 0;
        }

        /* check for bonus production */
        // LLS
        int bonus = u->GetProductionBonus(p->itemtype);
        /* Factor for fractional productivity: 10 */
        po->productivity = (int)((float)(u->GetMen() * level * p->productivity / 10)) + bonus;
        if (po->target > 0 && po->productivity > po->target) po->productivity = po->target;
        return po->productivity;
    }
    return 0;
}

int Game::FindAttemptedProd(ARegion *r, Production *p)
{
    int attempted = 0;
    for (const auto obj : r->objects) {
        for (const auto u : obj->units) {
            if ((u->monthorders) && (u->monthorders->type == O_PRODUCE)) { attempted += ValidProd(u, r, p); }
        }
    }
    return attempted;
}

void Game::RunAProduction(ARegion *r, Production *p)
{
    int questcomplete;
    string quest_rewards;

    p->activity = 0;
    if (p->amount == 0) return;

    /* First, see how many units are trying to work */
    int attempted = FindAttemptedProd(r, p);
    int amt = p->amount;
    if (attempted < amt) attempted = amt;
    for (const auto obj : r->objects) {
        for (const auto u : obj->units) {
            questcomplete = 0;
            if (!u->monthorders || u->monthorders->type != O_PRODUCE) continue;

            ProduceOrder *po = dynamic_cast<ProduceOrder *>(u->monthorders);
            if (po->skill != p->skill || po->item != p->itemtype) continue;

            /* We need to implement a hack to avoid overflowing */
            int uatt, ubucks;

            uatt = po->productivity;
            if (uatt && amt && attempted) {
                double dUbucks = ((double)amt) * ((double)uatt) / ((double)attempted);
                ubucks = (int)dUbucks;
                questcomplete = quests.check_harvest_target(r, po->item, ubucks, amt, u, &quest_rewards);
            } else {
                ubucks = 0;
            }

            amt -= ubucks;
            attempted -= uatt;
            u->items.SetNum(po->item, u->items.GetNum(po->item) + ubucks);
            u->faction->DiscoverItem(po->item, 0, 1);
            p->activity += ubucks;
            po->target -= ubucks;
            if (po->target > 0) {
                TurnOrder *tOrder = new TurnOrder;
                tOrder->repeating = 0;
                std::string order = "PRODUCE " + to_string(po->target) + " " + ItemDefs[po->item].abr;
                tOrder->turnOrders.push_back(order);
                u->turnorders.push_front(tOrder);
            }

            /* Show in unit's events section */
            if (po->item == I_SILVER) {
                //
                // WORK
                //
                if (po->skill == -1) {
                    u->event(
                        "Earns " + to_string(ubucks) + " silver working in " + r->ShortPrint().const_str() + ".",
                        "work", r
                    );
                } else {
                    //
                    // ENTERTAIN
                    //
                    u->event(
                        "Earns " + to_string(ubucks) + " silver entertaining in " + r->ShortPrint().const_str() + ".",
                        "entertain", r
                    );
                    // If they don't have PHEN, then this will fail safely
                    u->Practice(S_PHANTASMAL_ENTERTAINMENT);
                    u->Practice(S_ENTERTAINMENT);
                }
            } else {
                /* Everything else */
                u->event(
                    "Produces " + ItemString(po->item, ubucks) + " in " + r->ShortPrint().const_str() + ".", "produce",
                    r
                );
                u->Practice(po->skill);
            }
            delete u->monthorders;
            u->monthorders = nullptr;
            if (questcomplete) { u->event("You have completed a quest! " + quest_rewards, "quest"); }
        }
    }
}

void Game::RunStudyOrders(ARegion *r)
{
    for (const auto obj : r->objects) {
        for (const auto u : obj->units) {
            if (u->monthorders) {
                if (u->monthorders->type == O_STUDY) {
                    Do1StudyOrder(u, obj);
                    delete u->monthorders;
                    u->monthorders = nullptr;
                }
            }
        }
    }
}

void Game::RunIdleOrders(ARegion *r)
{
    for (const auto obj : r->objects) {
        for (const auto u : obj->units) {
            if (u->monthorders && u->monthorders->type == O_IDLE) {
                u->event("Sits idle.", "idle");
                delete u->monthorders;
                u->monthorders = nullptr;
            }
        }
    }
}

void Game::Do1StudyOrder(Unit *u, Object *obj)
{
    StudyOrder *o = dynamic_cast<StudyOrder *>(u->monthorders);
    int sk, cost, reset_man, skmax, taughtdays, days;

    reset_man = -1;
    sk = o->skill;
    if (sk == -1 || SkillDefs[sk].flags & SkillType::DISABLED ||
        (SkillDefs[sk].flags & SkillType::APPRENTICE && !Globals->APPRENTICES_EXIST)) {
        std::string name = (sk == -1) ? "that" : SkillDefs[sk].name;
        u->error("STUDY: Can't study " + name + ".");
        return;
    }

    // Check that the skill can be studied
    if (SkillDefs[sk].flags & SkillType::NOSTUDY) {
        u->error("STUDY: " + SkillDefs[sk].name + " cannot be studied.");
        return;
    }

    // Small patch for Ceran Mercs
    if (u->GetMen(I_MERC)) {
        u->error("STUDY: Mercenaries are not allowed to study.");
        return;
    }

    if (o->level != -1) {
        skmax = u->GetSkillMax(sk);
        if (skmax < o->level) {
            o->level = skmax;
            if (u->GetRealSkill(sk) >= o->level) {
                u->error("STUDY: Cannot study " + SkillDefs[sk].name + " beyond level " + to_string(o->level) + ".");
                return;
            } else {
                u->error(
                    "STUDY: set study goal for " + SkillDefs[sk].name + " to the maximum achievable level (" +
                    to_string(o->level) + ")."
                );
            }
        }
        if (u->GetRealSkill(sk) >= o->level) {
            u->error("STUDY: already reached specified level; nothing to study.");
            return;
        }
    }

    cost = SkillCost(sk) * u->GetMen();
    if (cost > u->GetSharedMoney()) {
        u->error("STUDY: Not enough funds to study " + SkillDefs[sk].name + ".");
        return;
    }

    if ((SkillDefs[sk].flags & SkillType::MAGIC) && u->type != U_MAGE) {
        if (u->type == U_APPRENTICE) {
            u->error(string("STUDY: An ") + Globals->APPRENTICE_NAME + " cannot be made into a mage.");
            return;
        }
        if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
            if (CountMages(u->faction) >= AllowedMages(u->faction)) {
                u->error("STUDY: Can't have another magician.");
                return;
            }
        }
        if (u->GetMen() != 1) {
            u->error("STUDY: Only 1-man units can be magicians.");
            return;
        }
        if (!(Globals->MAGE_NONLEADERS)) {
            if (u->GetLeaders() != 1) {
                u->error("STUDY: Only leaders may study magic.");
                return;
            }
        }
        reset_man = u->type;
        u->type = U_MAGE;
    }

    if ((SkillDefs[sk].flags & SkillType::APPRENTICE) && u->type != U_APPRENTICE) {
        if (u->type == U_MAGE) {
            u->error(string("STUDY: A mage cannot be made into an ") + Globals->APPRENTICE_NAME + ".");
            return;
        }

        if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
            if (CountApprentices(u->faction) >= AllowedApprentices(u->faction)) {
                u->error(string("STUDY: Can't have another ") + Globals->APPRENTICE_NAME + ".");
                return;
            }
        }
        if (u->GetMen() != 1) {
            u->error(string("STUDY: Only 1-man units can be ") + Globals->APPRENTICE_NAME + "s.");
            return;
        }
        if (!(Globals->MAGE_NONLEADERS)) {
            if (u->GetLeaders() != 1) {
                u->error(string("STUDY: Only leaders may be ") + Globals->APPRENTICE_NAME + "s.");
                return;
            }
        }
        reset_man = u->type;
        u->type = U_APPRENTICE;
    }

    if ((Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) && (sk == S_QUARTERMASTER) &&
        (u->GetSkill(S_QUARTERMASTER) == 0) && (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)) {
        if (CountQuarterMasters(u->faction) >= AllowedQuarterMasters(u->faction)) {
            u->error("STUDY: Can't have another quartermaster.");
            return;
        }
        if (u->GetMen() != 1) {
            u->error("STUDY: Only 1-man units can be quartermasters.");
            return;
        }
    }

    // If TACTICS_NEEDS_WAR is enabled, and the unit is trying to study to tact-5,
    // check that there's still space...
    if (Globals->TACTICS_NEEDS_WAR && sk == S_TACTICS && u->GetSkill(sk) == 4 &&
        u->skills.GetDays(sk) / u->GetMen() >= 300) {
        if (CountTacticians(u->faction) >= AllowedTacticians(u->faction)) {
            u->error("STUDY: Can't start another level 5 tactics leader.");
            return;
        }
        if (u->GetMen() != 1) {
            u->error("STUDY: Only 1-man units can study to level 5 in tactics.");
            return;
        }

    } // end tactics check

    // adjust teaching for study rate
    taughtdays = ((long int)o->days * u->skills.GetStudyRate(sk, u->GetMen()) / 30);

    days = u->skills.GetStudyRate(sk, u->GetMen()) * u->GetMen() + taughtdays;

    if ((SkillDefs[sk].flags & SkillType::MAGIC) && u->GetRealSkill(sk) >= 2) {
        if (obj->incomplete > 0 || obj->type == O_DUMMY) {
            u->error("Warning: Magic study rate outside of a building cut in half above level 2.");
            days /= 2;
        } else if (obj->mages < 1) {
            if (!Globals->LIMITED_MAGES_PER_BUILDING || (!obj->IsFleet() && !ObjectDefs[obj->type].maxMages)) {
                u->error("Warning: Magic study rate cut in half above level 2 due to unsuitable building.");
            } else {
                u->error(
                    "Warning: Magic study rate cut in half above level 2 due to number of mages studying in structure."
                );
            }
            days /= 2;
        } else if (Globals->LIMITED_MAGES_PER_BUILDING) {
            obj->mages--;
        }
    }

    if (SkillDefs[sk].flags & SkillType::SLOWSTUDY) { days /= 2; }

    if (u->Study(sk, days)) {
        u->ConsumeSharedMoney(cost);
        string str = "Studies " + SkillDefs[sk].name;
        taughtdays = taughtdays / u->GetMen();
        if (taughtdays) { str += " and was taught for " + to_string(taughtdays) + " days"; }
        str += " at a cost of " + ItemString(I_SILVER, cost) + ".";
        u->event(str, "study");
        // study to level order
        if (o->level != -1) {
            if (u->GetRealSkill(sk) < o->level) {
                TurnOrder *tOrder = new TurnOrder;
                tOrder->repeating = 0;
                std::string order = "STUDY " + string(SkillDefs[sk].abbr) + " " + to_string(o->level);
                tOrder->turnOrders.push_back(order);
                u->turnorders.push_front(tOrder);
            } else {
                string msg = "Completes study to level " + to_string(o->level) + " in " + SkillDefs[sk].name + ".";
                u->event(msg, "study");
            }
        }
    } else {
        // if we just tried to become a mage or apprentice, but
        // were unable to study, reset unit to whatever it was before.
        if (reset_man != -1) u->type = reset_man;
    }
}

void Game::DoMoveEnter(Unit *unit, ARegion *region)
{
    if (!unit->monthorders || ((unit->monthorders->type != O_MOVE) && (unit->monthorders->type != O_ADVANCE))) return;

    MoveOrder *o = dynamic_cast<MoveOrder *>(unit->monthorders);
    while (o->dirs.size()) {
        auto x = o->dirs.front();
        int i = x->dir;
        if (i != MOVE_OUT && i < MOVE_ENTER) return;
        std::erase(o->dirs, x);
        delete x;

        if (i >= MOVE_ENTER) {
            Object *to = region->GetObject(i - MOVE_ENTER);
            if (!to) {
                unit->error("MOVE: Can't find object.");
                continue;
            }

            if (!to->CanEnter(region, unit)) {
                unit->error("ENTER: Can't enter that.");
                continue;
            }

            Unit *forbid = to->ForbiddenBy(region, unit);
            if (forbid && !o->advancing) {
                unit->error("ENTER: Is refused entry.");
                continue;
            }

            if (forbid && region->IsSafeRegion()) {
                unit->error("ENTER: No battles allowed in safe regions.");
                continue;
            }

            if (forbid && !(unit->canattack && unit->IsAlive())) {
                unit->error("ENTER: Unable to attack " + forbid->name);
                continue;
            }

            bool done = false;
            while (forbid) {
                int result = RunBattle(region, unit, forbid, 0, 0);
                if (result == BATTLE_IMPOSSIBLE) {
                    unit->error("ENTER: Unable to attack " + forbid->name);
                    done = 1;
                    break;
                }
                if (!unit->canattack || !unit->IsAlive()) {
                    done = true;
                    break;
                }
                forbid = to->ForbiddenBy(region, unit);
            }
            if (done) continue;

            unit->MoveUnit(to);
            unit->event("Enters " + to->name + ".", "movement");
        } else {
            if (i == MOVE_OUT) {
                bool isOcean = (TerrainDefs[region->type].similar_type == R_OCEAN);
                if (isOcean && (!unit->CanSwim() || unit->GetFlag(FLAG_NOCROSS_WATER))) {
                    unit->error("MOVE: Can't leave ship.");
                    continue;
                }
                Object *to = region->GetDummy();
                unit->MoveUnit(to);
            }
        }
    }
}

Location *Game::DoAMoveOrder(Unit *unit, ARegion *region, Object *obj)
{
    MoveOrder *o = dynamic_cast<MoveOrder *>(unit->monthorders);
    ARegion *newreg;
    string road, temp;

    int movetype, cost, startmove, weight;
    Unit *ally, *forbid;
    Location *loc;
    const char *prevented = nullptr;

    if (!o->dirs.size()) {
        delete o;
        unit->monthorders = nullptr;
        return 0;
    }

    auto x = o->dirs.front();

    if (x->dir == MOVE_IN) {
        if (obj->inner == -1) {
            unit->error("MOVE: Can't move IN there.");
            goto done_moving;
        }

        // Make sure that items which cannot go through a shaft don't
        for (auto i : unit->items) {
            if (ItemDefs[i->type].flags & ItemType::NO_SHAFT) {
                unit->error("MOVE: Unable to fit through the shaft.");
                goto done_moving;
            }
        }

        newreg = regions.GetRegion(obj->inner);
        if (obj->type == O_GATEWAY) {
            // Gateways should only exist in the nexus, and move the
            // user to a semi-random instance of the target terrain
            // type, so select where they will actually move to.
            ARegionArray *level = regions.GetRegionArray(newreg->zloc);
            std::vector<ARegion *> start_locations = level->get_starting_region_candidates(newreg->type);

            // match levels to try for, in order:
            // 0 - completely empty towns
            // 1 - towns with only guardsmen
            // 2 - towns with guardsmen and other players
            // 3 - completely empty hexes
            // 4 - anywhere that matches terrain (out of options)
            int match = 0;
            std::vector<ARegion *> candidates = {};
            while (candidates.empty() && match < 5) {
                for (const auto scanReg : start_locations) {
                    // ignore any region that isn't a town before match level 3
                    if (match < 3 && !scanReg->town) continue;
                    int guards = 0;
                    int others = 0;
                    for (const auto o : scanReg->objects) {
                        for (const auto u : o->units) {
                            if (u->faction->num == guardfaction)
                                guards = 1;
                            else
                                others = 1;
                        }
                    }
                    // match level 0 - ignore anything that isn't completely empty
                    if (match == 0 && (guards || others)) continue;
                    // match level 1 - ignore anything that has other players
                    if (match == 1 && (!guards || others)) continue;
                    // match level 2 - ignore anything that doesn't have guards
                    if (match == 2 && !guards) continue;
                    // match level 3 - ignore anything that has guards or others
                    if (match == 3 && (guards || others)) continue;
                    // match level 4 or we were legal by the above rules
                    candidates.push_back(scanReg);
                }
                // increment match level.  If we found anything above, we'll break at the top of the while
                match++;
            }
            if (!candidates.empty()) {
                int index = rng::rng::get_random(candidates.size());
                newreg = candidates[index];
            }
        }
    } else if (x->dir == MOVE_PAUSE) {
        newreg = region;
    } else {
        newreg = region->neighbors[x->dir];
    }

    if (!newreg) {
        unit->error("MOVE: Can't move that direction.");
        goto done_moving;
    }

    // Check for any keybarrier objects in the target region
    for (const auto o : newreg->objects) {
        if (ObjectDefs[o->type].flags & ObjectType::KEYBARRIER) {
            if (unit->items.GetNum(ObjectDefs[o->type].key_item) < 1) {
                unit->error("MOVE: A mystical barrier prevents movement in that direction.");
                goto done_moving;
            }
        }
    }

    prevented = newreg->movement_forbidden_by_ruleset(unit, region, regions);
    if (prevented != nullptr) {
        unit->error("MOVE: " + string(prevented) + " prevents movement in that direction.");
        goto done_moving;
    }

    unit->movepoints += unit->CalcMovePoints(region);

    road = "";
    startmove = 0;
    movetype = unit->MoveType(region);
    cost = newreg->MoveCost(movetype, region, x->dir, &road);
    if (x->dir == MOVE_PAUSE) cost = 1;
    if (region->type == R_NEXUS) {
        cost = 1;
        startmove = 1;
    }
    if ((TerrainDefs[region->type].similar_type == R_OCEAN) &&
        (!unit->CanSwim() || unit->GetFlag(FLAG_NOCROSS_WATER))) {
        unit->error("MOVE: Can't move while in the ocean.");
        goto done_moving;
    }
    weight = unit->items.Weight();
    if ((TerrainDefs[region->type].similar_type == R_OCEAN) && (TerrainDefs[newreg->type].similar_type != R_OCEAN) &&
        !unit->CanWalk(weight) && !unit->CanRide(weight) && !unit->CanFly(weight)) {
        unit->error("Must be able to walk to climb out of the ocean.");
        goto done_moving;
    }
    if (movetype == M_NONE) {
        unit->error("MOVE: Unit is overloaded and cannot move.");
        goto done_moving;
    }

    // If we're moving in the same direction as last month and
    // have stored movement points, then add in those stored
    // movement points, but make sure that these are only used
    // towards entering the hex we were trying to enter
    if (!unit->moved && unit->movepoints >= Globals->MAX_SPEED && unit->movepoints < cost * Globals->MAX_SPEED &&
        x->dir == unit->savedmovedir) {
        while (unit->savedmovement > 0 && unit->movepoints < cost * Globals->MAX_SPEED) {
            unit->movepoints += Globals->MAX_SPEED;
            unit->savedmovement--;
        }
        unit->savedmovement = 0;
        unit->savedmovedir = -1;
    }

    if (unit->movepoints < cost * Globals->MAX_SPEED) return 0;

    if (x->dir == MOVE_PAUSE) {
        unit->event("Pauses to admire the scenery in " + string(region->ShortPrint().const_str()) + ".", "movement");
        unit->movepoints -= cost * Globals->MAX_SPEED;
        unit->moved += cost;
        std::erase(o->dirs, x);
        delete x;
        return 0;
    }

    if ((TerrainDefs[newreg->type].similar_type == R_OCEAN) &&
        (!unit->CanSwim() || unit->GetFlag(FLAG_NOCROSS_WATER))) {
        unit->event(
            "Discovers that " + string(newreg->ShortPrint().const_str()) + " is " + TerrainDefs[newreg->type].name +
                ".",
            "movement"
        );
        goto done_moving;
    }

    if (unit->type == U_WMON && newreg->town && newreg->IsGuarded()) {
        unit->event("Monsters don't move into guarded towns.", "movement");
        goto done_moving;
    }

    if (unit->guard == GUARD_ADVANCE) {
        ally = newreg->ForbiddenByAlly(unit);
        if (ally && !startmove) {
            unit->event("Can't ADVANCE: " + newreg->name + " is guarded by " + ally->name + ", an ally.", "movement");
            goto done_moving;
        }
    }

    if (o->advancing) unit->guard = GUARD_ADVANCE;

    forbid = newreg->Forbidden(unit);
    if (forbid && !startmove && unit->guard != GUARD_ADVANCE) {
        int obs = unit->GetAttribute("observation");
        unit->event(
            std::string("Is forbidden entry to ") + newreg->ShortPrint().const_str() + " by " +
                forbid->GetName(obs).const_str() + ".",
            "movement"
        );
        obs = forbid->GetAttribute("observation");
        forbid->event(std::string("Forbids entry to ") + unit->GetName(obs).const_str() + ".", "guarding");
        goto done_moving;
    }

    if (unit->guard == GUARD_GUARD) unit->guard = GUARD_NONE;

    unit->alias = 0;
    unit->movepoints -= cost * Globals->MAX_SPEED;
    unit->moved += cost;
    unit->MoveUnit(newreg->GetDummy());
    unit->DiscardUnfinishedShips();

    // Track the initial region the unit started from for part of NO7 victory handling
    if (unit->initial_region == nullptr) { unit->initial_region = region; }

    switch (movetype) {
    case M_WALK:
    default: temp = "Walks " + road; break;
    case M_RIDE:
        temp = "Rides " + road;
        unit->Practice(S_RIDING);
        break;
    case M_FLY:
        temp = "Flies ";
        unit->Practice(S_SUMMON_WIND);
        unit->Practice(S_RIDING);
        break;
    case M_SWIM: temp = "Swims "; break;
    }
    unit->event(
        temp + "from " + string(region->ShortPrint().const_str()) + " to " + newreg->ShortPrint().const_str() + ".",
        "movement"
    );

    if (forbid) { unit->advancefrom = region; }

    // TODO: Should we get a transit report on the starting region?
    if (Globals->TRANSIT_REPORT != GameDefs::REPORT_NOTHING) {
        if (!(unit->faction->is_npc)) newreg->visited = 1;
        // Update our visit record in the region we are leaving.
        for (const auto f : region->passers) {
            if (f->unit == unit) {
                // We moved into here this turn
                if (x->dir < MOVE_IN) { f->exits_used[x->dir] = 1; }
            }
        }
        // And mark the hex being entered
        Farsight *f = new Farsight;
        f->faction = unit->faction;
        f->level = 0;
        f->unit = unit;
        if (x->dir < MOVE_IN) { f->exits_used[region->GetRealDirComp(x->dir)] = 1; }
        newreg->passers.push_back(f);
    }

    region = newreg;

    std::erase(o->dirs, x);
    delete x;

    loc = new Location;
    loc->unit = unit;
    loc->region = region;
    loc->obj = nullptr;
    return loc;

done_moving:
    delete o;
    unit->monthorders = nullptr;
    return 0;
}

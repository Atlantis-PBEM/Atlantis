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

int Unit::AgeDead()
{
//delete non-magic skills
    forlist(&skills) {
        Skill *sk = (Skill *) elem;
        if(!(SkillDefs[sk->type].flags & SkillType::MAGIC)) {
            skills.Remove(sk);
            delete sk;
        }
    }
    int decays = getrandom(skills.Num());
    forlist_reuse(&skills) {
        Skill *sk = (Skill *) elem;
        if(decays == 0) {
            if(sk->experience) {        
                if(sk->experience > 30) sk->experience -= 30; //sk->experience is an unsigned int so can't subtract 30 if less than 30.
                else sk->experience = 0;
            } else {
                if(sk->days > 30) sk->days -= 30;
                else {
                    skills.Remove(sk);
                    delete sk;
                }
            }
        }
        decays--;
    }
    
    if(dead != 1 && !getrandom(5)) dead = 1;   //this converts dead from factionnum (>1) to 1 with 20% chance per turn ... time lag of factional "loyalty"
    
    return skills.Num(); //>=1 stays undead, 0 disappears
}

int Unit::MaxEnergy()
{
    int energi = 0;
        
    forlist(&skills) {
        Skill *sk = (Skill *) elem;
        if(SkillDefs[sk->type].flags & SkillType::FOUNDATION) {
            int level = GetLevelByDays(sk->days,sk->experience) - resurrects;
            if(level <= 0) continue;
            energi += level * level;
            if(IsASpeciality(sk->type)) {
                energi += level * level;
            }
        }
    }
    //just in case:
    if(energi<0) energy = 0;
    return energi;
}

int Unit::EnergyRecharge()
{
    int energi = 0;
    
    forlist(&skills) {
        Skill *sk = (Skill *) elem;
        if(SkillDefs[sk->type].flags & SkillType::FOUNDATION) {
            int level = GetLevelByDays(sk->days,sk->experience) - resurrects;
            if(level>0) energi += level;
            if(IsASpeciality(sk->type)) energi += level;
        }
    }
    
    int inner = GetSkill(S_INNER_STRENGTH);
    if(inner) {
        energi *= (20 + inner); // 5% bonus
        energi /= 20;
        energi += inner;
        Experience(S_INNER_STRENGTH, 3); //gets divided by 2 for non-specialists in the experience() code.
    }

    if(mastery) {
        energi += (energi+4)/10; //10% rounded to nearest integer.
    }
    //hardcoded maintenance values follow.

    //first, portal maintenance.
    //note, that it is possible for a mage to use energy in a battle between the movement phase and the
    //maintenance phase, thus potentially ending with negative energy here without losing portals. To avoid this,
    //all energy checks in the battle code should use the method Unit::GetEnergy rather than accessing
    //unit->energy directly.
//for consistency, the cost here must match the cost in Unit::GetEnergy(). These could perhaps be combined into a new method.
    if(transferred > 0) {
        int skill = GetSkill(S_CREATE_PORTAL);
        if(skill < 1) skill = 1;
        int cost = (transferred + 40 * skill - 1) / (40 * skill);
        energi -= cost;
        transferred = 0;
    }

    //creature maintenance
    energi -= EnergyMaintenance(energy + energi);

    return energi;
}

int Unit::EnergyMaintenance(int maxallowed)
{
    float maxmaintenance = (float) maxallowed;
    float energycost;
    int illusions = items.GetNum(I_IRAT) + items.GetNum(I_IWOLF) + items.GetNum(I_ISKELETON) + items.GetNum(I_IIMP) + 10*(items.GetNum(I_IEAGLE) + items.GetNum(I_IDEMON) + 
        items.GetNum(I_IUNDEAD)) + 40*(items.GetNum(I_IGRYFFIN) + items.GetNum(I_ILICH) + items.GetNum(I_IBALROG)) + 80*items.GetNum(I_IDRAGON);

    energycost = (float) illusions/80;
    energycost *= (float) (11 - GetSkill(S_ILLUSORY_CREATURES))/10;
    if(energycost > maxmaintenance) {
        Event("Does not have enough energy to maintain his illusions, which are dispelled");
        energycost = 0;
        for(int i=0; i<NITEMS; i++) {
            if(ItemDefs[i].type & IT_ILLUSION) items.SetNum(i,0);
        }
    }
    int experience = (int) (2*energycost+0.99);
    if(energycost>0) Experience(S_ILLUSORY_CREATURES, experience);

    // 5 per balrog.
    int monsters = items.GetNum(I_BALROG);
    float monstercost = (float) 5 * monsters;
    monstercost *= (float) (11 - GetSkill(S_SUMMON_BALROG))/10;
    experience = (int) (monstercost + 0.99);
    if(monstercost + energycost > maxmaintenance) {
    //don't want to create a wandering monster, because it allows for aimed balrog
    //missiles. Better to just have the random escape chance, so people cannot control
    //when they escape (short of forgetting the spell).
        Event("Does not have enough energy to control his balrogs, which return whence they came.");
        items.SetNum(I_BALROG,0);
        experience = 0;
    } else energycost += monstercost;
    if(experience) Experience(S_SUMMON_BALROG, experience);
    
    // 3 per gryffin unless the spell is free to cast!
    if(SkillDefs[S_GRYFFIN_LORE].cast_cost) {
        monsters = items.GetNum(I_GRYFFIN);
        monstercost = (float) 3 * monsters;
        monstercost *= (float) (11 - GetSkill(S_GRYFFIN_LORE))/10;
        experience = (int) (monstercost + 0.99);
        if(monstercost + energycost > maxmaintenance) {
            Event("Does not have enough energy to control his gryffins, which fly back to their home.");
            items.SetNum(I_GRYFFIN,0);
            experience = 0;
        } else energycost += monstercost;
        if(experience) Experience(S_GRYFFIN_LORE, experience);
    }
    //liches/undead/skeletons/wolves/eagles/dragons are free, because their cost is built into their recruitment cost.

    // 1 per demon
    monsters = items.GetNum(I_DEMON);
    monstercost = (float) monsters;
    monstercost *= (float) (11 - GetSkill(S_SUMMON_DEMON))/10;
    experience = (int) (monstercost + 0.99);
    if(monstercost + energycost > maxmaintenance) {
        Event("Does not have enough energy to control his demons, which return whence they came.");
        items.SetNum(I_DEMON,0);
    } else energycost += monstercost;
    if(experience) Experience(S_SUMMON_DEMON, experience);
        
    // 1 per 10 imps
    monsters = items.GetNum(I_IMP);
    monstercost = (float) monsters / 10;
    monstercost *= (float) (11 - GetSkill(S_SUMMON_IMPS))/10;
    experience = (int) (2*monstercost + 0.99);
    if(monstercost + energycost > maxmaintenance) {
        Event("Does not have enough energy to control his imps, which return whence they came.");
        items.SetNum(I_IMP,0);
    } else energycost += monstercost;
    if(experience) Experience(S_SUMMON_IMPS, experience);
    
    energycost += 0.99; //round up

    if(items.GetNum(I_WOLF)) Experience(S_WOLF_LORE,getrandom(3));
    if(items.GetNum(I_EAGLE)) Experience(S_BIRD_LORE,getrandom(3));
    if(items.GetNum(I_SKELETON)) Experience(S_NECROMANCY,getrandom(3));
    if(items.GetNum(I_UNDEAD)) Experience(S_RAISE_UNDEAD,getrandom(3));
    if(items.GetNum(I_LICH)) Experience(S_SUMMON_LICH,getrandom(3));

    return (int) energycost;
}

int Unit::MysticEvent()
{
    return 0;
/*    //flat 33% chance of not getting an event (else mysticism is unattractive).
    if(!getrandom(3)) return 0;
    if(!Globals->ARCADIA_MAGIC) return 0;

    if(getrandom(MaxEnergy()) >= MysticEnergy() ) return 0;
    
    if(speciality == S_BASE_MYSTICISM && getrandom(2)) return 0;
    //mystics are less likely to get mystic events
    int mystic = GetSkill(S_BASE_MYSTICISM);
    if(getrandom(mystic+5) > 5) return 0; //0% chance at level 1 to 45% chance at level 6.

    int chance = getrandom(100);
    if(speciality != S_BASE_MYSTICISM) mystic -= 4;

    if(chance >= (60 - (mystic/2))) return 1; // most likely 38-43%
    else if(chance >= (30 - mystic)) return 2; // 28-33%
    else if(chance >= (10 - (mystic/2))) return 3; //17-22%
    return 4; // least likely 7-12%
*/    
}

void Game::RechargeMages()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if(u->type == U_MAGE) {
                    u->energy += u->EnergyRecharge();
                    int maxenergy = u->MaxEnergy();
                    if (u->energy > maxenergy) u->energy = maxenergy;
                }    
			}
		}
	}
}

void Game::SinkLandRegions()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		if (r->willsink) {
		    r->willsink--;
            if(!r->willsink) {
                AString temp = r->ShortPrint(&regions) + " has sunk beneath the ocean.";
                SpecialError(r, temp);
                r->Event("All land sunk beneath the ocean.");
                r->SinkRegion(&regions);
            }
		}
	}
}

void Game::DistributeFog()
{
//no allowance is made to check which of two casters of fog has higher level, as long as both are higher than the clearskies
//level of the region. This only needs to change if ever level 1 fog has an effect different to level 2 fog.

	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if(u->foggy) {
				    int fogskill = u->GetSkill(S_FOG);
				    if(fogskill == 0) fogskill = 1; //eg if backfire makes it follow someone else.
				    if(fogskill > r->clearskies) r->fog = fogskill - r->clearskies;
    
				    if(u->foggy>1) {
				        //large casting
				        if(u->foggy > 2 && r->neighbors[u->foggy-3]) {
				            //centred on neighbouring region
				            if(fogskill > r->neighbors[u->foggy-3]->clearskies)
				                r->neighbors[u->foggy-3]->fog = fogskill - r->neighbors[u->foggy-3]->clearskies;
				            for(int k=0; k<6; k++) {
				                if(!r->neighbors[u->foggy-3]->neighbors[k]) continue;
				                if(fogskill > r->neighbors[u->foggy-3]->neighbors[k]->clearskies)
				                    r->neighbors[u->foggy-3]->neighbors[k]->fog = fogskill - r->neighbors[u->foggy-3]->neighbors[k]->clearskies;
				            }
				        } else {
				            //centred on this region
				            for(int k=0; k<6; k++) {
				                if(!r->neighbors[k]) continue;
				                if(fogskill > r->neighbors[k]->clearskies)
				                    r->neighbors[k]->fog = fogskill - r->neighbors[k]->clearskies;
				            }
				        }
				    }
				}
			}
		}
	}
}

int Unit::GetCastCost(int skill, int extracost, int multiplier, int levelpenalty)
{
//this needs to match the cost in the skillshows.cpp file.

    float cost = (float) SkillDefs[skill].cast_cost;
    cost *= multiplier;
    if(SkillDefs[skill].flags & SkillType::COSTVARIES) {
        float leveleffect = (float) (2 + GetSkill(skill) - levelpenalty ) / 3;
        cost /= leveleffect;
    }
    
/*    if(extracost) {                    //This makes the second spell cost 50% more, the 3rd 100% more, etc.
        cost = ((2+extracost)*cost)/2;
    }*/
    
    cost += 0.99; //round up.
    
    return (int) cost;
}

int Unit::GetCombatCost(int skill, int multiplier)
{
    int cost = SkillDefs[skill].combat_cost;
    cost *= multiplier;

    return cost;
}

int Unit::GetFirstCombatCost(int skill, int multiplier)
{
    int cost = SkillDefs[skill].combat_first;
    cost *= multiplier;

    return cost;
}

void Game::GenerateVolcanic(ARegion *r)
{
    if( getrandom(10) ) return;
    if(r->zloc != 1) return;
    if(TerrainDefs[r->type].similar_type != R_OCEAN) return;    

    ARegion *centre = regions.GetRegion( (regions.GetRegionArray(1)->x)/2, (regions.GetRegionArray(1)->y)/2, 1 );
    if(regions.GetDistance(centre,r) > 5) return;
/*
    float maxx = regions.GetRegionArray(1)->x;
    float maxy = regions.GetRegionArray(1)->y;
    float top = (float) r->xloc - maxx/2;
    float radiusx = top / maxx;
    top = (float) r->yloc - maxy/2;
    float radiusy = top / maxy;
    float radius = radiusx*radiusx + radiusy*radiusy;

    if( radius > (float) 1/36 ) return;
    //we are in a circle from the centre, radius one-third of the way to the edge. In total this is about 1/10th of the map.
*/



    r->OceanToLand();
    for(int i=0; i<6; i++) {
        Hexside *h = r->hexside[i];
        if(h->type == H_BEACH || h->type == H_HARBOUR) {
            if(getrandom(100) < 15) h->type = H_ROCKS; //volcanic area has more rocks than usual.
        }
    }
    
    int type = getrandom(4);
    
    switch(type) {
        case 0:
        case 1:
            r->type = R_MOUNTAIN;
            break;
        case 2:
            r->type = R_SWAMP;
            break;
        case 3:
            r->type = R_PLAIN;
            break;
    }
    r->town = NULL;
    
    SpecialError(r, r->ShortPrint(&regions) + " rises out of the ocean.");
    r->Event("Rises out of the ocean");
    r->SetName("Volcanic Island");

    r->products.DeleteAll();
    r->SetupProds();
                
    r->markets.DeleteAll();
	
    r->population = 0;
	r->basepopulation = 0;
	r->wages = 0;
	r->maxwages = 0;
	r->money = 0;
	r->willsink = 1 + getrandom(3);
}


/* Need to incorporate energy into unit report. Need to get energy recharging each turn. 

*/

void Game::SpecialErrors(ARegion *r)
{
    int sinking = 1;
    if(r->willsink == 0 || r->willsink > 6) sinking = 0;
    int foggy = r->fog;
    if(foggy < 0) foggy = 0; //shouldn't be needed, but just in case.
    int blizzard = 0;
    if(r->weather == W_BLIZZARD) blizzard = 1;
    if(sinking + foggy + blizzard == 0) return;
    
    
    AString sinkstr = r->ShortPrint(&regions) + " will sink in " + r->willsink + " month";
    if(r->willsink > 1) sinkstr += "s";
    sinkstr += ".";

    AString fogstr = r->ShortPrint(&regions) + " is covered in fog, obscuring our view of other units.";
    AString blizstr = r->ShortPrint(&regions) + " experienced an unnatural blizzard last month.";

    forlist(&r->objects) {
        Object *o1 = (Object *) elem;
        forlist(&o1->units) {
            Unit *u1 = (Unit *) elem;
            int first = 1;
            int found = 0;
            forlist(&r->objects) {
                Object *o2 = (Object *) elem;
                forlist(&o2->units) {
                    Unit *u2 = (Unit *) elem;
                    if(u2 == u1) {
                        found = 1;
                        break;
                    } else if(u2->faction == u1->faction) {
                        found = 1;
                        first = 0;
                        break;
                    }
                }
                if(found) break;
            }
            if(first) {
                if(sinking) u1->Message(sinkstr);
                if(foggy) u1->Message(fogstr);
                if(blizzard) u1->Message(blizstr);
            }
        }
    }
}

void Game::SpecialError(ARegion *r, AString message, Faction *fac)
{
    forlist(&r->objects) {
        Object *o1 = (Object *) elem;
        forlist(&o1->units) {
            Unit *u1 = (Unit *) elem;
            if(u1->faction == fac) continue;
            int first = 1;
            int found = 0;
            forlist(&r->objects) {
                Object *o2 = (Object *) elem;
                forlist(&o2->units) {
                    Unit *u2 = (Unit *) elem;
                    if(u2 == u1) {
                        found = 1;
                        break;
                    } else if(u2->faction == u1->faction) {
                        found = 1;
                        first = 0;
                        break;
                    }
                }
                if(found) break;
            }
            if(first) u1->Message(message);
        }
    }
}

void Unit::WanderingExperience(int message)
{
    if(!Globals->ARCADIA_MAGIC) return;
    if(type != U_MAGE) return;
    if(dead) return;

    int skill = -1;
    int options = 0;
    for(int i=0; i<NSKILLS; i++) {
        if(!(SkillDefs[i].flags & SkillType::MAGIC)) continue;
        if((SkillDefs[i].flags & SkillType::DISABLED)) continue;
        if(GetRealSkill(i) || CanStudy(i) ) {
            if(GetExperSkill(i) == 3) continue; //assumes 3 is the max skill level.
            //we can experience this skill
            if(!getrandom(++options)) skill = i;
            //double chance if the skill is in our speciality.
            if(IsASpeciality(SkillDefs[i].baseskill) && !getrandom(++options)) skill = i;
        }
    }

    if(skill == -1) return;
    Experience(skill,10);
    switch(message) {
    //1 = MOVE or ADVANCE order
    //2 = WORK order. Not enabled in Arcadia.
    //nothing for TAX, STUDY, SAIL, STUDY, ENTERTAIN orders. 
    //The purpose of this is to get mages out seeing the world, not just stuck at home studying in towers.

        case 1:
            Event(AString("Gains experience in ") + SkillDefs[skill].name + " while travelling.");
            return;
        case 2:
            Event(AString("Gains experience in ") + SkillDefs[skill].name + " in his time off work.");
            return;
        default:
            Event(AString("Gains experience in ") + SkillDefs[skill].name + " in his spare time.");
            return;
    }
}

void Game::UpdateFactionAffiliations()
{
    forlist(&factions) {
        Faction *f = (Faction *) elem;
        if(f->IsNPC()) continue;
        f->ethnicity = RA_NA; //in "chaos"
    }
    
	forlist_reuse(&regions) {
		ARegion *reg = (ARegion *)elem;
		forlist(&reg->objects) {
			Object *obj = (Object *)elem;
			forlist(&obj->units) {
				Unit *u = (Unit *)elem;
				if(u->flags & FLAG_COMMANDER) {
				    if(u->faction->ethnicity == RA_NA) u->faction->ethnicity = u->GetEthnicity();
				    else u->SetFlag(FLAG_COMMANDER,0);
                }
            }
        }
    }
    
    forlist_reuse(&factions) {
        Faction *f = (Faction *) elem;
        if(f->IsNPC()) continue;
        if(f->ethnicity == RA_NA) { //in "chaos"
		    WorldEvent *event = new WorldEvent;
            event->type = WorldEvent::CONVERSION;
            event->fact1 = f->num;
            event->fact2 = f->ethnicity;
            event->reportdelay = 0;
            worldevents.Add(event);
        }
    }
}

void Game::SetupGuardsmenAttitudes()
{
    for(int i=0; i<4; i++) {
        Faction *Guardfac;
        switch(i) {
            case 0: Guardfac = GetFaction(&factions, guardfaction);
            break;
            case 1: Guardfac = GetFaction(&factions, elfguardfaction);
            break;
            case 2: Guardfac = GetFaction(&factions, dwarfguardfaction);
            break;
            case 3: Guardfac = GetFaction(&factions, independentguardfaction);
            break;
        }
        
        forlist(&factions) {
            Faction *f = (Faction *) elem;
            if(f->ethnicity != Guardfac->ethnicity) Guardfac->SetAttitude(f->num,A_UNFRIENDLY);
        }
    }
}



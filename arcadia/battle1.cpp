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
#include "battle1.h"
#include "army1.h"
#include "formation1.h"
#include "gamedefs.h"
#include "gamedata.h"

#ifndef DEBUG
//#define DEBUG
#endif
#ifndef DEBUG2
//#define DEBUG2
#endif

void Game::GetDFacs(ARegion * r,Unit * t,AList & facs)
{
    int AlliesIncluded = 0;
    
    // First, check whether allies should assist in this combat
    if (Globals->ALLIES_NOAID == 0) {
        AlliesIncluded = 1;
    } else {
        // Check whether any of the target faction's
        // units aren't set to noaid
        forlist((&r->objects)) {
            Object * obj = (Object *) elem;
            forlist((&obj->units)) {
                Unit * u = (Unit *) elem;
                if (u->IsReallyAlive()) {
                    if (u->faction == t->faction &&
                            !t->GetFlag(FLAG_NOAID)) {
                        AlliesIncluded = 1;
                        break;
                    }
                }
                if (AlliesIncluded == 1) break; // forlist(units)
            }
            if (AlliesIncluded == 1) break; // forlist (objects)
        }
    }
    
    forlist((&r->objects)) {
        Object * obj = (Object *) elem;
        forlist((&obj->units)) {
            Unit * u = (Unit *) elem;
            if (u->IsReallyAlive()) {
                if (u->faction == t->faction ||
                        (AlliesIncluded == 1 &&
                        u->guard != GUARD_AVOID &&
                        u->GetAttitude(r,t) == A_ALLY)) {
                    if (!GetFaction2(&facs,u->faction->num)) {
                        FactionPtr * p = new FactionPtr;
                        p->ptr = u->faction;
                        facs.Add(p);
                    }
                }
            }
        }
    }
}


void Game::GetAFacs(ARegion *r, Unit *att, Unit *tar, AList &dfacs,
		AList &afacs, AList &atts)
{
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			if (u->canattack && u->IsReallyAlive()) {
				int add = 0;
				if ((u->faction == att->faction ||
							u->GetAttitude(r,tar) == A_HOSTILE) &&
						(u->guard != GUARD_AVOID || u == att)) {
					add = 1;
				} else {
					if (u->guard == GUARD_ADVANCE &&
							u->GetAttitude(r,tar) != A_ALLY) {
						add = 1;
					} else {
						if (u->attackorders) {
							forlist(&(u->attackorders->targets)) {
								UnitId * id = (UnitId *) elem;
								Unit *t = r->GetUnitId(id, u->faction->num);
								if (!t) continue;
								if (t == tar) {
									u->attackorders->targets.Remove(id);
									delete id;
								}
								if(t->faction == tar->faction) add = 1;
							}
						}
					}
				}

				if (add) {
					if (!GetFaction2(&dfacs,u->faction->num)) {
						Location * l = new Location;
						l->unit = u;
						l->obj = obj;
						l->region = r;
						atts.Add(l);
						if (!GetFaction2(&afacs,u->faction->num)) {
							FactionPtr * p = new FactionPtr;
							p->ptr = u->faction;
							afacs.Add(p);
						}
					}
				}
			}
		}
	}
}

int Game::CanAttack(ARegion * r,AList * afacs,Unit * u) {
  int see = 0;
  int ride = 0;
  forlist(afacs) {
    FactionPtr * f = (FactionPtr *) elem;
    if (f->ptr->CanSee(r,u) == 2) {
      if (ride == 1) return 1;
      see = 1;
    }
    if (f->ptr->CanCatch(r,u)) {
      if (see == 1) return 1;
      ride = 1;
    }
  }
  return 0;
}

void Game::GetSides(ARegion *r, AList &afacs, AList &dfacs, AList &atts,
		AList &defs, Unit *att, Unit *tar, int ass, int adv)
{
    att->crossbridge = 0;
    tar->crossbridge = 0;

	if (ass) {
		/* Assassination attempt */
		Location * l = new Location;
		l->unit = att;
		l->obj = r->GetDummy();
		l->region = r;
		atts.Add(l);

		l = new Location;
		l->unit = tar;
		l->obj = r->GetDummy();
		l->region = r;
		defs.Add(l);

		return;
	}

	int j=NDIRS;
	int noaida = 0, noaidd = 0;
	for (int i=-1;i<j;i++) {
		ARegion * r2 = r;
		if (i>=0) {
			r2 = r->neighbors[i];
			if (!r2) continue;
			forlist(&r2->objects) {
				/* Can't get building bonus in another region */
				((Object *) elem)->capacity = 0;
			}
		} else {
			forlist(&r2->objects) {
				Object * o = (Object *) elem;
				/* Set building capacity */
				if (o->incomplete < 1 && o->IsBuilding()) {
					o->capacity = ObjectDefs[o->type].protect;
				}
			}
		}
		forlist (&r2->objects) {
			Object * o = (Object *) elem;
			forlist (&o->units) {
				Unit * u = (Unit *) elem;
				int add = 0;
				u->crossbridge = 0;
#define ADD_ATTACK 1
#define ADD_DEFENSE 2
				/* First, can the unit be involved in the battle at all? */
				if ((i==-1 || u->GetFlag(FLAG_HOLDING) == 0) && u->IsReallyAlive()) {
					if (GetFaction2(&afacs,u->faction->num)) {
						/*
						 * The unit is on the attacking side, check if the
						 * unit should be in the battle
						 */
						if (i == -1 || (!noaida)) {
							if (u->canattack &&
									(u->guard != GUARD_AVOID || u==att) &&
									u->CanMoveTo(r2,r) &&
									!::GetUnit(&atts,u->num)) {
								add = ADD_ATTACK;
							}
						}
					} else {
						/* The unit is not on the attacking side */
						/*
						 * First, check for the noaid flag; if it is set,
						 * only units from this region will join on the
						 * defensive side
						 */
						if (!(i != -1 && noaidd)) {
							if (u->type == U_GUARD) {
								/* The unit is a city guardsman */
								if (i == -1/* && adv == 0*/)
									add = ADD_DEFENSE;
							} else if(u->type == U_GUARDMAGE) {
								/* the unit is a city guard support mage */
								if(i == -1/* && adv == 0*/)
									add = ADD_DEFENSE;
							} else {
								/*
								 * The unit is not a city guardsman, check if
								 * the unit is on the defensive side
								 */
								if (GetFaction2(&dfacs,u->faction->num)) {
									if (u->guard == GUARD_AVOID) {
										/*
										 * The unit is avoiding, and doesn't
										 * want to be in the battle if he can
										 * avoid it
										 */
										if (u == tar ||
												(u->faction == tar->faction &&
												 i==-1 &&
												 CanAttack(r,&afacs,u))) {
											add = ADD_DEFENSE;
										}
									} else {
										/*
										 * The unit is not avoiding, and wants
										 * to defend, if it can
										 */
										if (u->CanMoveTo(r2,r)) {
											add = ADD_DEFENSE;
										}
									}
								}
							}
						}
					}
				}

				if (add == ADD_ATTACK) {
					Location * l = new Location;
					l->unit = u;
					l->obj = o;
					l->region = r2;
					atts.Add(l);
				} else if (add == ADD_DEFENSE) {
						Location * l = new Location;
						l->unit = u;
						l->obj = o;
						l->region = r2;
						defs.Add(l);
				}
			}
		}
		//
		// If we are in the original region, check for the noaid status of
		// the units involved
		//
		if (i == -1) {
			noaida = 1;
			forlist (&atts) {
				Location *l = (Location *) elem;
				if (!l->unit->GetFlag(FLAG_NOAID)) {
					noaida = 0;
					break;
				}
			}
		}

		noaidd = 1;
		{
			forlist (&defs) {
				Location *l = (Location *) elem;
				if (!l->unit->GetFlag(FLAG_NOAID)) {
					noaidd = 0;
					break;
				}
			}
		}
	}
}

void Game::KillDead(Location * l)
{
	if (!l->unit->IsReallyAlive()) {
		l->region->Kill(l->unit); //this deletes all non-mage units which are dead. Otherwise transfers items and moves to dummy region.
		if(l->unit->dead) {  //ie a mage.
	        //ARCADIA_MAGIC Patch
    	    Faction *fac = GetFaction(&factions, ghostfaction);
            if(fac) l->unit->faction = fac;
            else {
                l->unit->MoveUnit(0); //routine for destroying a unit, from ARegion::Kill()
        	    l->region->hell.Add(l->unit);
        	}
		}
	} else {
		if (l->unit->advancefrom) {
			l->unit->MoveUnit( l->unit->advancefrom->GetDummy() );
		}
	}
}

int Game::RunBattle(ARegion * r,Unit * attacker,Unit * target,int ass,
                     int adv)
{
	AList afacs,dfacs;
	AList atts,defs;
	FactionPtr * p;
	int result;

	if (ass) {
		if(attacker->GetAttitude(r,target) == A_ALLY) {
			attacker->Error("ASSASSINATE: Can't assassinate an ally.");
			return BATTLE_IMPOSSIBLE;
		}
		/* Assassination attempt */
		p = new FactionPtr;
		p->ptr = attacker->faction;
		afacs.Add(p);
		p = new FactionPtr;
		p->ptr = target->faction;
		dfacs.Add(p);
	} else {
		if( r->IsSafeRegion() ) {
			attacker->Error("ATTACK: No battles allowed in safe regions.");
			return BATTLE_IMPOSSIBLE;
		}
		if(attacker->GetAttitude(r,target) == A_ALLY) {
			attacker->Error("ATTACK: Can't attack an ally.");
			return BATTLE_IMPOSSIBLE;
		}
		GetDFacs(r,target,dfacs);
		if (GetFaction2(&dfacs,attacker->faction->num)) {
			attacker->Error("ATTACK: Can't attack an ally.");
			return BATTLE_IMPOSSIBLE;
		}
		GetAFacs(r,attacker,target,dfacs,afacs,atts);
	}
	GetSides(r,afacs,dfacs,atts,defs,attacker,target,ass,adv);

	if(atts.Num() <= 0) {
		// This shouldn't happen, but just in case
		Awrite(AString("Cannot find any attackers!"));
		return BATTLE_IMPOSSIBLE;
	}
	if(defs.Num() <= 0) {
		// This shouldn't happen, but just in case
		Awrite(AString("Cannot find any defenders!"));
		return BATTLE_IMPOSSIBLE;
	}

	Battle * b = new Battle;
	b->WriteSides(r,attacker,target,&atts,&defs,ass, &regions );

	battles.Add(b);
	forlist(&factions) {
		Faction * f = (Faction *) elem;
		if (GetFaction2(&afacs,f->num) || GetFaction2(&dfacs,f->num) ||
				r->Present(f)) {
			BattlePtr * p = new BattlePtr;
			p->ptr = b;
			f->battles.Add(p);
		}
	}

	result = b->Run(r,attacker,&atts,target,&defs,ass, &regions );
	/* Remove all dead units */
	{
		forlist(&atts) {
			KillDead((Location *) elem);
		}
	}
	{
		forlist(&defs) {
			KillDead((Location *) elem);
		}
	}
	return result;
}

Battle::Battle()
{
    asstext = 0;
}

Battle::~Battle()
{
    #ifdef DEBUG
    Awrite("Battle Destructor");
    #endif

    if (asstext) delete asstext;
    text.DeleteAll();

    #ifdef DEBUG
    Awrite("End of Battle Destructor");
    #endif
}

int Battle::Run( ARegion * region,
                  Unit * att,
                  AList * atts,
                  Unit * tar,
                  AList * defs,
                  int ass,
                  ARegionList *pRegs )
{
#ifdef DEBUG		
Awrite("battles 1");
#endif

    Army * armies[2];
    AString temp;
    assassination = ASS_NONE;
    attacker = att->faction;

    int amts = 0;
    if(ass > 1) {
        ass = 1; //setting it back to 1 - set to 2 earlier for proportional AMTS usage, but no longer used.
        amts = 1;
    }

    armies[0] = new Army(att,atts,region->type,2*ass); //ass = 2 for attacker, 1 for defender. This is used for armour selection.

#ifdef DEBUG		
Awrite("battles 2");
#endif

    armies[1] = new Army(tar,defs,region->type,ass);
#ifdef DEBUG2		
Awrite("battles 2.1");
#endif
    if(!ass) {
        WriteTerrainMessage(region->type);
#ifdef DEBUG2		
Awrite("battles 2.2");
#endif
        WriteAggressionMessage(armies[0],armies[1]);
#ifdef DEBUG2		
Awrite("battles 2.3");
#endif
        armies[0]->DoEthnicMoraleEffects(this);
#ifdef DEBUG2		
Awrite("battles 2.4");
#endif
        armies[1]->DoEthnicMoraleEffects(this);
    }

#ifdef DEBUG		
Awrite("battles 3");
#endif

    int round = 1;
    while (!armies[0]->Broken() && !armies[1]->Broken() && round < 26) {
        NormalRound(round++,armies[0],armies[1],region->type, 0,0,ass);
    }
    
#ifdef DEBUG		
Awrite("battles 4");
#endif

    if ((armies[0]->Broken() && !armies[1]->Broken()) ||
        (!armies[0]->NumAlive() && armies[1]->NumAlive())) {
        if (ass) assassination = ASS_FAIL;

		if (armies[0]->NumAlive()) {
		  AddLine(*(armies[0]->pLeader->name) + " is routed!");
		  NormalRound(round++,armies[0],armies[1],region->type,1);
		} else {
		  AddLine(*(armies[0]->pLeader->name) + " is destroyed!");
		}
        AddLine("Total Casualties:");
        
        ItemList *spoils = new ItemList;
        int loserdead = armies[0]->Lose(this, spoils);
        GetSpoils(atts, spoils, ass);
        if (spoils->Num()) {
            temp = AString("Spoils: ") + spoils->Report(2,0,1) + ".";
        } else {
            temp = "Spoils: none.";
        }   
#ifdef DEBUG		
Awrite("battles 5");
#endif
        armies[1]->Win(this, spoils, loserdead); //change the Win function for better spoils handling
//        AddLine("");
        AddLine(temp);
        AddLine("");
        delete spoils;
        //soldiers seem to have already been deleted in win/lose/tie methods
        delete armies[0];      
        delete armies[1];         
        return BATTLE_LOST;
    }

#ifdef DEBUG		
Awrite("battles 6");
#endif

    if ((armies[1]->Broken() && !armies[0]->Broken()) ||
        (!armies[1]->NumAlive() && armies[0]->NumAlive())) {
        if (ass) {
            assassination = ASS_SUCC;
            asstext = new AString(*(armies[1]->pLeader->name) +
                                  " is assassinated in " +
                                  region->ShortPrint( pRegs ) +
                                  "!");
        }
        if (armies[1]->NumAlive()) {
		  AddLine(*(armies[1]->pLeader->name) + " is routed!");
		  NormalRound(round++,armies[0],armies[1],region->type,2);
		} else {
		  AddLine(*(armies[1]->pLeader->name) + " is destroyed!");
		}
#ifdef DEBUG		
Awrite("battles 7");
#endif
        AddLine("Total Casualties:");
        ItemList *spoils = new ItemList;
        int loserdead = armies[1]->Lose(this, spoils, ass);  //dead soldiers are removed from units here, and skills lost.
        GetSpoils(defs, spoils, ass+amts);   //spoils allocated according to u->GetMen() and u->losses.
        if (spoils->Num()) {
            temp = AString("Spoils: ") + spoils->Report(2,0,1) + ".";
        } else {
            temp = "Spoils: none.";
        }
        armies[0]->Win(this, spoils, loserdead);
//        AddLine("");
        AddLine(temp);
        AddLine("");
            
        delete spoils;
        delete armies[0];
        delete armies[1];
        return BATTLE_WON;
    }
#ifdef DEBUG		
Awrite("battles 8");
#endif
    AddLine("The battle ends indecisively.");
    AddLine("");
    AddLine("Total Casualties:");
    armies[0]->Tie(this);
    armies[1]->Tie(this);
    temp = "Spoils: none.";
    AddLine("");
    AddLine(temp);
    AddLine("");
#ifdef DEBUG		
Awrite("battles 8.5");
#endif
    delete armies[0];
#ifdef DEBUG		
Awrite("battles 9");
#endif
    delete armies[1];
    return BATTLE_DRAW;
}

void Battle::NormalRound(int round,Army * armya,Army * armyb, int regtype, int bias, int ambush, int ass)
{
    /* Write round header */
    AddLine(AString("Round ") + round + ":");
    /* Initialize variables */
    armya->round++;
    armyb->round++;
    /* Setup Formations */
    FormationsPhase(armya,armyb,regtype,bias, ambush, ass);
    armya->Reset();
    armyb->Reset();
    int aalive = armya->NumAlive();
    int aialive = aalive;
    int balive = armyb->NumAlive();
    int bialive = balive;
    int aatt = armya->CanAttack();
    int batt = armyb->CanAttack();
    /* Run attacks until done */
    while (aalive && balive && (aatt || batt))
    {
        int num = getrandom(aatt + batt);
        if (num >= aatt)
        {
            num -= aatt;
            Soldier * s = armyb->GetAttacker(num); //this decrements the canattack, and swops the attacker into the cannot attack section of the formation.
            DoAttack(armyb->round, s, armyb, armya);
        }
        else
        {
            Soldier * s = armya->GetAttacker(num);
            DoAttack(armya->round, s, armya, armyb);
        }
        aalive = armya->NumAlive();
        balive = armyb->NumAlive();
        aatt = armya->CanAttack();
        batt = armyb->CanAttack();
    }
    /* Finish round */
	armya->Regenerate(this);
	armyb->Regenerate(this);
    aialive -= aalive;
    AddLine(*(armya->pLeader->name) + " loses " + aialive + ".");
    bialive -= balive;
    AddLine(*(armyb->pLeader->name) + " loses " + bialive + ".");
    AddLine("");
}

void Battle::DoAttack(int round, Soldier *a, Army *attackers, Army *def, int ass) //'attackers' is the army the soldier is from.
{ //ass no longer used here

	DoSpecialAttack(round, a, attackers, def);
	if (!def->NumAlive()) return;

	if (a->riding != -1) {
		MountType *pMt = FindMount(ItemDefs[a->riding].abr);
		// This does mount special attack. Adjust it so cannot behind formations cannot hit non-engaged formations (ie its not ranged)
		if(pMt->mountSpecial != NULL) {
			int i, num, tot = -1;
			SpecialType *spd = FindSpecial(pMt->mountSpecial);
			for(i = 0; i < 4; i++) {
				int times = spd->damage[i].value;
				if(spd->effectflags & SpecialType::FX_USE_LEV)
					times *= pMt->specialLev;
				int realtimes = spd->damage[i].minnum + getrandom(times) +
					getrandom(times);
				num  = def->DoAnAttack(pMt->mountSpecial, realtimes, a->race,
						spd->damage[i].type, pMt->specialLev,
						spd->damage[i].flags, spd->damage[i].dclass,
						spd->damage[i].effect, 0, attackers, a->inform, this);
				if(num != -1) {
					if(tot == -1) tot = num;
					else tot += num;
				}
			}
			if(tot != -1) {
				AddLine(a->name + " " + spd->spelldesc + ", " +
						spd->spelldesc2 + tot + spd->spelltarget + ".");
			}
		}
	}
	if(!def->NumAlive()) return;

	int numAttacks = a->attacks;
	if(a->attacks < 0) {
		if(round % ( -1 * a->attacks ) == 0)  //This used to be 1 (ie xbow attacks in round 1,3,5. Setting it to zero means xbow attacks in rounds 2,4,6)
			numAttacks = 1;
		else
			numAttacks = 0;
	} else if(ass && (Globals->MAX_ASSASSIN_FREE_ATTACKS > 0) &&
			(numAttacks > Globals->MAX_ASSASSIN_FREE_ATTACKS)) {
		numAttacks = Globals->MAX_ASSASSIN_FREE_ATTACKS;
	}

	for (int i = 0; i < numAttacks; i++) {
		WeaponType *pWep = NULL;
		if(a->weapon != -1)
			pWep = FindWeapon(ItemDefs[a->weapon].abr);

		int flags = 0;
		int attackType = a->attacktype; //this is set to weapon attack type in soldier::soldier
		int mountBonus = 0;
		int attackClass = SLASHING;
		if(pWep) {
			flags = pWep->flags;
			mountBonus = pWep->mountBonus;
			attackClass = pWep->weapClass;
		}
		def->DoAnAttack(NULL, 1, a->race, attackType, a->askill, flags, attackClass,
				NULL, mountBonus, attackers, a->inform, this);
		if (!def->NumAlive()) break;
	}
}

void Battle::FormationsPhase(Army * armya, Army * armyb, int regtype, int bias, int ambush, int ass)
{
//This section is responsible for most of the interactions between formations.
//The remaining few are handled in Army::GetMidRoundTarget based on the engagement
//table set up in this section.
//Formations have four engagement levels to other formations.
//ENGAGED_ENGAGED: Formations is actively engaged in melee combat with its target
//ENGAGED_TRYATTACK: If it becomes free, it would like to attack this target. If 
//the soldier in question is ranged, it will do so without engaging, if melee AND 
//the formation is behind 0, it will engage. This does not have relevance for
//flank 0 and 1 formations, as they will flank rather than engage a tryattack.
//ENGAGED_CANATTACK: Formation doesn't particularly want to engage this target,
//but it is able to if it runs out of enemies classified higher. This has meaning only for
//flank 2 formations and reserves, but is set anyway for all.
//ENGAGED_CONDITIONAL: If the enemy is engaged, this counts as ENGAGED_CANATTACK. Else
//it counts as ENGAGED_NONE. (ie can catch enemy only if they are already engaged).
//ENGAGED_NONE: Formation is not able to engage the target in melee combat. This has
//meaning only for flank 2 formations, but is set anyway for others.

//Formation setup reminder:
//Formation 0-5 are flank 0 (form_front). Formation 6-11 are flank 1 (form_flanking).
//Formations 12-17 are flank 2 (form_flanked). Formation 18 is dead.
//In the current setup, behind formations will not try to flank, and thus 6 formations
//will always be empty. However, they are included because it might be a nice
//future addition if behind formations can flank in order to target enemy
//behind formations in their "tryattack list".

//Basic structure: formation[0] is the foot frontlines, and will always try to engage an
//enemy. #s 2 and 4 are fron cavalry, who are in reserve.
//For now, #s 1, 3 and 5 will always stay flank 0 behind 1, these are discriminated between only
//by their ability to be caught. The players always has the FIGHTAS command to merge
//them.
//Units which try to flank will be intercepted by enemy reserves if they can be caught by
//them. Reserves will fight all flanking enemies, then fight all flanked enemies, then go
//back to doing nothing unless there are no non-behind/reserve enemies left to defend against,
//in which case they will come out of reserve and try to find a target.
//If a unit gets through enemy reserves either because (a) there are none, (b) they 
//can't catch the unit, or (c) the flanking unit is concealed, then it will become "flanked"
//and select a target to engage - usually a behind enemy. Once it is flanked, it can always
//be caught.



    // Update both army's shields
   	armya->shields.DeleteAll();
    armyb->shields.DeleteAll();
    UpdateShields(armya, armyb);
    UpdateShields(armyb, armya);

    // Reset engagement table (tryattack/canattack to zero)
    armya->ResetEngagements();
    armyb->ResetEngagements();

    //Darkness, Fog and Concealment
    UpdateRoundSpells(armya,armyb);
    
    if(ass) {
        armya->AssignFrontTargets(this, armyb, 1);
        return;
    }

    //bias is the army which is penalised.
    if(bias == 1) armya->PenaltyToHit(1);
    if(bias == 2) armyb->PenaltyToHit(1);
    
    //Sort armies if in first round
    if(armya->round == 1) armya->SortFormations(this, regtype);
    if(armyb->round == 1+ambush) armyb->SortFormations(this, regtype);

    WriteBattleSituation(armya,armyb);

    //mirror any engagements that occured last round
    armya->MirrorEngagements(armyb);
    armyb->MirrorEngagements(armya);
    
    //If formation is engaged,return.
    //If flank 1, move everyone to flank 2.
    armya->FlankFlankers(this, armyb);
    armyb->FlankFlankers(this, armya);

    //if Formation[0] unengaged: attack enemy flank 2s, then form[0], then flank (cannot catch
    //any flank 1 enemies except form 6, and enemy form 6 is empty until our form 0 is overwhelmed).
    armya->AssignFrontTargets(this, armyb);
    armyb->AssignFrontTargets(this, armya);

    //split overwhelming flank 0 formations. if concealed, go straight to flank 2
    if(!(TerrainDefs[regtype].flags & TerrainType::RESTRICTEDFOOT) ) {
        armya->SplitOverwhelmingFrontFormations(this,armyb);
        armyb->SplitOverwhelmingFrontFormations(this,armya);
    }

    //choose if reserves flank
    //flank choice
    //if concealed, flank 0 goes straight to flank 2. Otherwise, soldiers get added to
    //flank 1 formations.
    armya->FlyingReservesMayFlank(this, armyb);
    armyb->FlyingReservesMayFlank(this, armya);

    armya->RidingReservesMayFlank(this, armyb);
    armyb->RidingReservesMayFlank(this, armya);
    
    //canattack lists
    armya->SetCanAttack(armyb);
    armyb->SetCanAttack(armyb);

//armya->WriteEngagements(this);

    //reserves intercept flankers
    armya->ReservesIntercept(armyb);
    armyb->ReservesIntercept(armya);

    //split overwhelming flank 1 formations
    //any formation with sufficiently more troops than are engaging it can send some to
    //flank around the enemy. Any unengaged formation will also flank. Hence, this must
    //be placed after reserves have a chance to intercept flank 1s, and effectively pushes
    //flank 1s to flank 2 if they do not get intercepted.
    //If foot are restricted, they cannot flank here.
    armya->SplitOverwhelmingFlankingFormations(this,armyb,regtype);    //need to modify so foot don't flank in restricted terrain.
    armyb->SplitOverwhelmingFlankingFormations(this,armya,regtype);

    //flank 2s engage & get combat bonus if appropriate (as tempbonus).
    //second choice of target (if any) is set as tryattack, or first choice if no men in the formation.
    armya->FlankersEngage(this, armyb);
    armyb->FlankersEngage(this, armya);
    //assign ranged tryattack lists (can call getmidroundrangedtarget)
    armya->AssignRangedTargets(armyb);
    armyb->AssignRangedTargets(armya);
    //Clear engagement table for empty formations - if people move during battle, they'll inherit the old specifics.
    armya->ClearEmptyEngagements();
    armyb->ClearEmptyEngagements();
    
    armya->SetTemporaryBonuses(armyb);
    armyb->SetTemporaryBonuses(armya);
//armya->WriteEngagements(this);
}

void Battle::GetSpoils(AList * losers, ItemList *spoils, int ass)
{
	forlist(losers) {
		Unit * u = ((Location *) elem)->unit;
		int numalive = u->GetSoldiers();
		int numdead = u->losses;
		if(ass && numdead == 0) {
            numdead++;  //assassinations with resurrected targets still get spoils.
            numalive --;
        }
		forlist(&u->items) {
			Item * i = (Item *) elem;
			if(IsSoldier(i->type)) continue;
			// New rule:  Assassins with RINGS cannot get AMTS in spoils
			// This rule is only meaningful with Proportional AMTS usage
			// is enabled, otherwise it has no effect.
			if((ass == 2) && (i->type == I_AMULETOFTS)) continue;
			float percent = (float)numdead/(float)(numalive+numdead);
			int num = (int)(i->num * percent);
			int num2 = (num + getrandom(2))/2;
			//if some types of items should never be lost, set num2 = num here for those item types.
			if(ItemDefs[i->type].flags & ItemType::NEVERLOST) num2 = num;
			spoils->SetNum(i->type, spoils->GetNum(i->type) + num2);
			u->items.SetNum(i->type, i->num - num);
		}
	}
}

void Battle::WriteSides(ARegion * r, Unit * att, Unit * tar, AList * atts,
                          AList * defs, int ass, ARegionList *pRegs )
{
  if (ass) {
    AddLine(*att->name + " attempts to assassinate " + *tar->name
	    + " in " + r->ShortPrint( pRegs ) + "!");
  } else {
    AddLine(*att->name + " attacks " + *tar->name + " in " +
	    r->ShortPrint( pRegs ) + "!");
  }
  AddLine("");

  int dobs = 0;
  int aobs = 0;
  {
	  forlist(defs) {
		  int a = ((Location *)elem)->unit->GetAttribute("observation");
		  if(a > dobs) dobs = a;
	  }
  }

  AddLine("Attackers:");
  {
	  forlist(atts) {
		  int a = ((Location *)elem)->unit->GetAttribute("observation");
		  if(a > aobs) aobs = a;
		  AString * temp = ((Location *) elem)->unit->BattleReport(dobs);
		  AddLine(*temp);
		  delete temp;
	  }
  }
  AddLine("");
  AddLine("Defenders:");
  {
	  forlist(defs) {
		  AString * temp = ((Location *) elem)->unit->BattleReport(aobs);
		  AddLine(*temp);
		  delete temp;
	  }
  }
  AddLine("");
}

void Battle::WriteTerrainMessage(int regtype)
{
    AString temp;
	int terrainflags = TerrainDefs[regtype].flags;
 	if ((terrainflags & TerrainType::FLYINGMOUNTS) && (terrainflags & TerrainType::RIDINGMOUNTS)) {
    	if (!(terrainflags & TerrainType::FLYINGLIMITED) && !(terrainflags & TerrainType::RIDINGLIMITED))
 	        temp = "The terrain allows riding and flying.";
        else if (!(terrainflags & TerrainType::FLYINGLIMITED) && (terrainflags & TerrainType::RIDINGLIMITED))
            temp = "The terrain allows flying and limited riding. Riding soldiers do not receive a combat bonus due to their riding skill.";
        else if ((terrainflags & TerrainType::FLYINGLIMITED) && !(terrainflags & TerrainType::RIDINGLIMITED))
            temp = "The terrain allows riding and limited flying. Flying soldiers do not receive a combat bonus due to their riding skill.";
        else 
            temp = "The terrain allows limited riding and limited flying. Soldiers do not receive a combat bonus due to their riding skill.";
    } else if (terrainflags & TerrainType::FLYINGMOUNTS) {
        if (terrainflags & TerrainType::FLYINGLIMITED)
            temp = "The terrain allows limited flying. Flying soldiers do not receive a combat bonus due to their riding skill. Riding soldiers are forced to fight on foot.";
        else temp = "The terrain allows flying but not riding. All riding soldiers fight on foot.";
    } else if (terrainflags & TerrainType::RIDINGMOUNTS) {
        if (terrainflags & TerrainType::RIDINGLIMITED)
            temp = "The terrain allows limited riding. Flying soldiers are forced to ride on the ground. Mounted soldiers do not receive a combat bonus due to their riding skill.";
        else temp = "The terrain allows riding but not flying. All flying soldiers ride on the ground.";
    } else {
        temp = "The terrain does not allow riding or flying. Mounted soldiers are forced to fight on foot.";
    }
    AddLine(temp);
    AddLine("");
}

void Battle::WriteAggressionMessage(Army *a, Army *b)
{
    if(a->pLeader->tactics == TACTICS_AGGRESSIVE) AddLine(*(a->pLeader->name) + " fights aggressively.");
    if(a->pLeader->tactics == TACTICS_DEFENSIVE) AddLine(*(a->pLeader->name) + " fights defensively.");
    if(b->pLeader->tactics == TACTICS_AGGRESSIVE) AddLine(*(b->pLeader->name) + " fights aggressively.");
    if(b->pLeader->tactics == TACTICS_DEFENSIVE) AddLine(*(b->pLeader->name) + " fights defensively.");
    AddLine("");
}

void Battle::TransferMessages(Army *a, Army *b)
{
        forlist(&a->armytext) {
            AString *temp = (AString *) elem;
            AddLine(*temp);
        }
        a->armytext.DeleteAll();
        forlist_reuse(&b->armytext) {
            AString *temp = (AString *) elem;
            AddLine(*temp);
        }
        b->armytext.DeleteAll();
}

void Battle::AddLine(const AString & s) 
{
    #ifdef DEBUG
    Awrite(s);
    #endif
    AString * temp = new AString(s);
    text.Add(temp);
}

void Battle::Report(Areport * f,Faction * fac) {
  if (assassination == ASS_SUCC && fac != attacker) {
    f->PutStr(*asstext);
    f->PutStr("");
    return;
  }
  forlist(&text) {
    AString * s = (AString *) elem;
    f->PutStr(*s);
  }
}

void Battle::WriteBattleSituation(Army *armya, Army *armyb)
{
    //flanked cavalry  ranged  flanked air
    //reserve cavalry  flanked foot reserve air
    //flanking cavalry front+fl flanking air
    AddLine("The Battle Position:");
    AString temp;
    temp = *(armya->pLeader->name);
    int length = temp.Len();
    int spaces = 22 - (length/2);
    temp = "";
    while(spaces-- > 0) temp += " ";
    temp += *(armya->pLeader->name);
    AddLine(temp);

    temp = "          ";
    temp += WriteBattleFormation('C', '*', armyb->formations[14].GetSize());
    temp += WriteBattleFormation('R', '#', armya->formations[1].GetSize() +  armya->formations[3].GetSize() +  armya->formations[5].GetSize());
    temp += WriteBattleFormation('A', '*', armyb->formations[16].GetSize());
    AddLine(temp);
    temp = "          ";
    temp += WriteBattleFormation('C', '#', armya->formations[2].GetSize());
    temp += WriteBattleFormation('I', '*', armyb->formations[12].GetSize());
    temp += WriteBattleFormation('A', '#', armya->formations[4].GetSize());
    AddLine(temp);    
    temp = "          ";
    temp += WriteBattleFormation('C', '#', armya->formations[8].GetSize());
    temp += WriteBattleFormation('I', '#', armya->formations[0].GetSize() +  armya->formations[6].GetSize());
    temp += WriteBattleFormation('A', '#', armya->formations[10].GetSize());
    AddLine(temp);    
    temp = "          ";
    temp += WriteBattleFormation('C', '*', armyb->formations[8].GetSize());
    temp += WriteBattleFormation('I', '*', armyb->formations[0].GetSize() +  armyb->formations[6].GetSize());
    temp += WriteBattleFormation('A', '*', armyb->formations[10].GetSize());
    AddLine(temp);
    temp = "          ";
    temp += WriteBattleFormation('C', '*', armyb->formations[2].GetSize());
    temp += WriteBattleFormation('I', '#', armya->formations[12].GetSize());
    temp += WriteBattleFormation('A', '*', armyb->formations[4].GetSize());
    AddLine(temp);
    temp = "          ";
    temp += WriteBattleFormation('C', '#', armya->formations[14].GetSize());
    temp += WriteBattleFormation('R', '*', armyb->formations[1].GetSize() +  armyb->formations[3].GetSize() +  armyb->formations[5].GetSize());
    temp += WriteBattleFormation('A', '#', armya->formations[16].GetSize());
    AddLine(temp);
    temp = *(armyb->pLeader->name);
    length = temp.Len();
    spaces = 22 - (length/2);
    temp = "";
    while(spaces-- > 0) temp += " ";
    temp += *(armyb->pLeader->name);
    AddLine(temp);
}

AString Battle::WriteBattleFormation(char f, char g, int size)
{
    AString temp;
    if(size<1) {
        temp = "         ";
        return temp;
    }
    temp = AString(f) + g + ": " + size;
    
    while(size < 100000) {
        temp += " ";
        size *= 10;
    }
    return temp;
}


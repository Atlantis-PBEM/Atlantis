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

#ifndef DEBUG
//#define DEBUG
#endif

#ifndef DEBUG2
//#define DEBUG2
#endif

#include "army1.h"
#include "gameio.h"
#include "gamedata.h"


Army::Army(Unit * ldr,AList * locs,int regtype,int ass)
{
#ifdef DEBUG 
Awrite("Making an army");
#endif
	Unit * tactician = ldr;

	round = 0;
	tac = ldr->GetAttribute("seniority");
	count = 0;
	hitstotal = 0;
	sortedformations = 0; // Formations get sorted in first battle round after fog/darkness is cast.
    concealment = 0;
    nonillusioncount = 0;
	canride = (TerrainDefs[regtype].flags & TerrainType::RIDINGMOUNTS);

	if(TerrainDefs[regtype].flags & TerrainType::RESTRICTEDRANGED) {
	     rangedbonus = -1;
    } else if(TerrainDefs[regtype].flags & TerrainType::ENHANCEDRANGED) {
         rangedbonus = 1;
    } else rangedbonus = 0;

	
	if (ass) {
		count = 1;
		nonillusioncount = 1;
		ldr->losses = 0;
		pLeader = ldr;
	} else {
		forlist(locs) {
			Unit * u = ((Location *) elem)->unit;
			count += u->GetSoldiers();
			nonillusioncount += u->GetRealSoldiers();
			u->losses = 0;
			int temp = u->GetSeniority();
			if (temp > tac) {
				tac = temp;
				tactician = u;
			}
		}
		pLeader = tactician;
	}

    taccontrol = 100;
	
	// If TACTICS_NEEDS_WAR is enabled, we don't want to push leaders 
	// from tact-4 to tact-5! Also check that we have skills, otherwise
	// we get a nasty core dump ;)
	if(!(SkillDefs[S_TACTICS].flags & SkillType::DISABLED)) {
    	if (Globals->TACTICS_NEEDS_WAR && (tactician->skills.Num() != 0)) {
    		int currskill = tactician->skills.GetDays(S_TACTICS)/tactician->GetMen();
    		if (currskill < 450 - Globals->SKILL_PRACTICE_AMOUNT) {
    			tactician->PracticeAttribute("tactics");
    		}
    	} else { // Only Globals->TACTICS_NEEDS_WAR == 0
    		tactician->PracticeAttribute("tactics");
    	}
	}
	// This looks wrong, but isn't, as the (NUMFORMS+1)th formation contains all the dead soldiers.
	for(int i=0; i<NUMFORMS+1; i++) {
	    formations[i].SetupFormation(i, count);	
	}
	
	for(int i=0; i<NUMFORMS; i++) {
	    for(int j=0; j<NUMFORMS; j++) {
	        engagements[i][j] = ENGAGED_NONE;
        }
    }
#ifdef DEBUG 
Awrite("Adding soldiers");
#endif
	int x = 0;
	forlist(locs) {
		Unit * u = ((Location *) elem)->unit;
		Object * obj = ((Location *) elem)->obj;
#ifdef DEBUG2
cout << *u->name << endl;
#endif
		if (ass) {
			forlist(&u->items) {
				Item * it = (Item *) elem;
				if (it) {
					if(ItemDefs[ it->type ].type & IT_MAN) {
							Soldier *temp = new Soldier(u, obj, regtype,
									it->type, ass);
							formations[0].AddSoldier(temp);
							hitstotal = temp->hits;  //used only for rout conditions
							temp = 0;
							++x;
							goto finished_army;

					}
				}
			}
		} else {
			Item *it = (Item *) u->items.First();
			do {
				if(IsSoldier(it->type)) {
#ifdef DEBUG2
cout << ItemDefs[it->type].abr;
#endif
					for(int i = 0; i < it->num; i++) {
#ifdef DEBUG2
cout << ".";
#endif
						Soldier *temp = new Soldier(u, obj, regtype,
								it->type);
#ifdef DEBUG2
cout << "|";
#endif
						formations[0].AddSoldier(temp);
						if(!temp->illusion) hitstotal += temp->hits;    //used only for rout conditions, hence does not include illusions
						temp = 0;
						++x;
					}
				}
				it = (Item *) u->items.Next(it);
			} while(it);
		}
	}

finished_army:
Awrite("Finished Army Construction");
}

Army::~Army()
{
    #ifdef DEBUG
    Awrite("Army Destructor");
    #endif
}

int Army::NumAlive()
{
    int numalive = count - formations[NUMFORMS].GetNumMen();
    #ifdef DEBUG
    if(numalive < 0) {
        Awrite("negative living soldiers!");
        system("pause");
    }
    #endif
    return numalive;
}

int Army::NumNonIllusionsAlive()
{
    int na = NumAlive();
    int numalive = 0;
    
    for(int i=0; i < na; i++) {
        const Soldier *pSold = GetSoldier(i);
        if(!pSold->illusion) numalive++;
    }
    
    return numalive;    
}

Soldier * Army::GetSoldier(int soldiernum) const
//This should use an input number from 0 to count. Use caution - it may return a dead soldier 
//(this needs to be able to as it gets used in the post-battle routines).
{
    for(int i=0; i<NUMFORMS+1; i++) {
        if(soldiernum >= formations[i].GetNumMen()) soldiernum -= formations[i].GetNumMen();
        else return formations[i].GetSoldier(soldiernum);
    }
    Awrite("Invalid Soldier");
    return 0;
}

int Army::Broken()
{
	if(Globals->ARMY_ROUT == GameDefs::ARMY_ROUT_FIGURES) {
		if(NumAlive() * 2 < count) return 1;
	} else { //one of the gamedef options effectively disabled here
        int hitsalive = 0;
        for(int i=0; i<NUMFORMS; i++) hitsalive += formations[i].GetNonIllusionSize();
        if(hitsalive * 2 < hitstotal) return 1;
        if(!NumAlive()) return 1; //if everyone is dead, since if all illusions the previous line will be if(0<0), ie false.
	}
	return 0;
}

void Army::Reset()
//called at the start of a normal round. Resets bonus, concealment and canattack.
{
    for(int i=0; i<NUMFORMS; i++) {
        formations[i].Reset();
    }
}

void Army::ResetEngagements()
//called at the start of the formation phase of a normal round.
{
    for(int i=0; i<NUMFORMS; i++) {
        for(int j=0; j<NUMFORMS; j++) {
            if(engagements[i][j] != ENGAGED_ENGAGED) engagements[i][j] = ENGAGED_NONE;
        }
        formations[i].SetConcealed(0);
        formations[i].bonus = 0;
        formations[i].tempbonus = 0;
    }
}

int Army::NumSpoilers() const
// Do we need this still? If so, have to create a formation method as well to
// preserve privacy of soldierptr.
{
    return 0;
}

int Army::CanAttack()
{
    int canattack = 0;
    for(int i=0; i<NUMFORMS; i++) {
        canattack += formations[i].CanAttack();
    }
    return canattack;
}

Soldier * Army::GetAttacker(int attackernum)
//This should use an input number from 0 to (CanAttack() - 1).
//It reduces canattack of the formation by 1
{
    for(int i=0; i<NUMFORMS; i++) {
        if(attackernum >= formations[i].CanAttack()) attackernum -= formations[i].CanAttack();
        else return formations[i].GetAttacker(attackernum);
    }
    Awrite("Invalid Attacker");
    return 0;
}

int Army::GetTarget(Army *attackers, int attackerform, int attackType, int *targetform, char* special, Battle *b)
//this sequence gets an enemy soldier to attack.
//"this" is the army of the defending unit.
{
    int tarnum = -1;
    #ifdef DEBUG2
	cout << "getting new target";
	#endif
	
	SpecialType *sp = FindSpecial(special);
	if (sp && sp->targflags) {
	#ifdef DEBUG2
	cout << " special attack" << endl;
	#endif
	//this is a special which only hits certain people (other specials, which hit anyone, 
    //are dealt with as normal attacks)
		int validtargs = 0;
		if(attackers != this) {  //ie attacking enemy, so check formation stuff.
    		// Search Engaged Formations
    		for(int i=0; i<NUMFORMS; i++) {
    		    if(attackers->engagements[attackerform][i] == ENGAGED_ENGAGED) {
    		        for(int j=0; j<formations[i].GetNumMen(); j++) {
    		            if (formations[i].CheckSpecialTarget(special, j)) validtargs++;
    		        }
                }
    		}
//cout << "engaged: " << validtargs << endl;	 //testing line for effects stuff	
    		if(validtargs) {
    		    tarnum = getrandom(validtargs);
    		    for(int i=0; i<NUMFORMS; i++) {
    		        if(attackers->engagements[attackerform][i] == ENGAGED_ENGAGED) {
    		            for(int j=0; j<formations[i].GetNumMen(); j++) {
    		                if (formations[i].CheckSpecialTarget(special, j)) {
           		                if(tarnum == 0) {
    		                        *targetform = i;
    		                        return j;
    	                        } else tarnum--;
                            }
    		            }
                    }
    		    }
            }
            //No valid targets in engaged formations. However, there may be enemies 
            //still in the engaged formations, so we do not want to engage again 
            //for the sake of one mage. Since this has an effect, it is 
            //a ranged attack. Search TryAttack formations
            validtargs = 0; //Should be anyway, but just in case
    		for(int i=0; i<NUMFORMS; i++) {
    		    if(attackers->engagements[attackerform][i] == ENGAGED_TRYATTACK) {
    		        for(int j=0; j<formations[i].GetNumMen(); j++) {
    		            if (formations[i].CheckSpecialTarget(special, j)) validtargs++;
    		        }
                }
    		}
    //cout << "tries: " << validtargs << endl;	 //testing line for effects stuff	
    		if(validtargs) {
    		    tarnum = getrandom(validtargs);
    		    for(int i=0; i<NUMFORMS; i++) {
    		        if(attackers->engagements[attackerform][i] == ENGAGED_TRYATTACK) {
    		            for(int j=0; j<formations[i].GetNumMen(); j++) {
    		                if (formations[i].CheckSpecialTarget(special, j)) {
           		                if(tarnum == 0) {
    		                        *targetform = i;
    		                        return j;
    	                        } else tarnum--;
                            }
    		            }
                    }
    		    }
            }
	    }
        //Hit a random soldier
        validtargs = 0; //should be zero anyway if we get here, but just in case!
		for(int i=0; i<NUMFORMS; i++) {
	        for(int j=0; j<formations[i].GetNumMen(); j++) {
	            if (formations[i].CheckSpecialTarget(special, j)) validtargs++;
            }
		}
//cout << "randoms: " << validtargs << endl;	 //testing line for effects stuff	
		if(validtargs) {
		    tarnum = getrandom(validtargs);
		    for(int i=0; i<NUMFORMS; i++) {
	            for(int j=0; j<formations[i].GetNumMen(); j++) {
	                if (formations[i].CheckSpecialTarget(special, j)) {
   		                if(tarnum == 0) {
	                        *targetform = i;
	                        return j;
                        } else tarnum--;
                    }
	            }
		    }
        }
        
        //should get here only if there are no valid targets anywhere.
        return -1;
	} else if (attackers != this) {  //ie as long as it's not an attack or spell hitting a unit's own army.
		int tars = 0;
		int engaged = 0;
		// Search Engaged Formations
		for(int i=0; i<NUMFORMS; i++) {
		    if(attackers->engagements[attackerform][i] == ENGAGED_ENGAGED) {
		        engaged = 1;
		        tars += formations[i].GetNumMen();
            }
		}
		if(tars) {
		    tarnum = getrandom(tars);
		    for(int i=0; i<NUMFORMS; i++) {
		        if(attackers->engagements[attackerform][i] == ENGAGED_ENGAGED) {
		            if(tarnum >= formations[i].GetNumMen() ) tarnum -= formations[i].GetNumMen();
		            else {
		                *targetform = i;
                        return tarnum;
                    }
		        }
		    }
		} else if(engaged) {
		//engaged with an empty formation. Clear the engagement.
		    for(int i=0; i<NUMFORMS; i++) {
		        if(formations[i].GetNumMen() == 0) attackers->engagements[attackerform][i] = ENGAGED_NONE;
            }
		}
		//if this soldier is melee, see if we can engage a new enemy formation.
		//if we are behind 1 this will return with no target.
		if(attackType == ATTACK_COMBAT || attackType == ATTACK_RIDING) {
    		int newtarget = attackers->GetMidRoundTarget(attackerform,this, b);
    		#ifdef DEBUG
    		cout << "New target: " << newtarget << endl;
    		#endif

    		if(newtarget > -1) {
                if(formations[newtarget].GetNumMen()) {
        		    *targetform = newtarget;
        		    tars = formations[newtarget].GetNumMen();
        		    return getrandom(tars);
		        } else {
		            //we shouldn't really get to here :(
		            #ifdef DEBUG
		            cout << "New target is empty" << endl;
		            system("pause");
		            #endif
		            return -1;
		        }
    		}
		//this is a melee soldier and we could not engage a target (perhaps we are behind).
        //Return with no target.
		    return newtarget; //this is either -1 (no target found) or -2 (soldier has been moved out). Either way, quit the attack sequence.

		    //The order here could be changed to make GetMidRoundTarget only called if deleting old
		    //engagements
		}

		//This soldier is ranged, and not engaged with anyone. It wants to hit someone it
		//is trying to attack.
		tars = 0; //just in case
		engaged = 0;
		for(int i=0; i<NUMFORMS; i++) {
		    if(attackers->engagements[attackerform][i] == ENGAGED_TRYATTACK) {
		        engaged = 1;
		        tars += formations[i].GetNumMen();
            }
		}
		if(tars) {
		    tarnum = getrandom(tars);
		    for(int i=0; i<NUMFORMS; i++) {
		        if(attackers->engagements[attackerform][i] == ENGAGED_TRYATTACK) {
		            if(tarnum >= formations[i].GetNumMen() ) tarnum -= formations[i].GetNumMen();
		            else {
		                *targetform = i;
                        return tarnum;
                    }
		        }
		    }
		} else if(engaged) {
		//tryattacking empty formations only. Clear engagements with empty enemy formations.
		    for(int i=0; i<NUMFORMS; i++) {
		        if(formations[i].GetNumMen() == 0) attackers->engagements[attackerform][i] = ENGAGED_NONE;
            }
        //our old tryattack targets are empty. Get new ones. If we are a behind 0 formation will get none.
    		int newtarget = attackers->GetMidRoundRangedTarget(attackerform,this);
    		if(newtarget>-1 && formations[newtarget].GetNumMen()) {
    		    *targetform = newtarget;
    		    tars = formations[newtarget].GetNumMen();
    		    return getrandom(tars);
    		}
        
		}
		
		//This soldier is ranged, and has no-one it wants to attack. As a last resort, 
		//hit a random enemy. This will only be called when we are down to enemy reserves & behind formations
		tars = NumAlive();
		tarnum = getrandom(tars);
		for(int i=0; i<NUMFORMS; i++) {
		    if(tarnum >= formations[i].GetNumMen() ) tarnum -= formations[i].GetNumMen();
		    else {
		        *targetform = i;
		        return tarnum;
            }
        }
    } else {
        //hitting one's own army. Get a random soldier.
		tarnum = getrandom( NumAlive() );
		for(int i=0; i<NUMFORMS; i++) {
		    if(tarnum >= formations[i].GetNumMen() ) tarnum -= formations[i].GetNumMen();
		    else {
		        *targetform = i;
		        return tarnum;
            }
        }
    }
    #ifdef DEBUG
    Awrite("Fell through Target Selection");
    #endif
	return -1; // This should never get called - unless perhaps a multi-attack soldiers kills the last enemy?
}

//needed for DoAnAttack below.
int pow(int b,int p)
{
	int b2 = b;
	for(int i=1; i<p; i++) {
		b2 *= b;
	}
	return b2;
}

int Hits(int a,int d)
{
	int tohit = 1,tomiss = 1;
	if (a>d) {
		tohit = pow(2,a-d);
	} else if (d>a) {
		tomiss = pow(2,d-a);
	}
	if (getrandom(tohit+tomiss) < tohit) return 1;
	return 0;
}

int Army::DoAnAttack(char *special, int numAttacks, int race, int attackType, int attackLevel, 
                  int flags, int weaponClass, char *effect, int mountBonus, Army *attackers, int attackerform, Battle *b, int strength)
		/* The army in question is the army DEFENDING!
		   */

{
	/* 1. Check against Global effects (not sure how yet). BS: What does this even mean? */
	
	/* 2. Attack shield */
	Shield *hi;
	int combat = 0;
	int canShield = 0;
	switch(attackType) {
		case ATTACK_RANGED:
			canShield = 1;
			// fall through
		case ATTACK_COMBAT:
		case ATTACK_RIDING:
			combat = 1;
			break;
		case ATTACK_ENERGY:
		case ATTACK_WEATHER:
		case ATTACK_SPIRIT:
			canShield = 1;
			break;
	}

	if(canShield) {
		int shieldType = attackType;

		hi = shields.GetHighShield(shieldType);
		if (hi) {
			/* Check if we get through shield */
			if(!Hits(attackLevel, hi->shieldskill)) {
				return -1;
			}

			if(effect == NULL && !combat) {
				/* We got through shield... if killing spell, downgrade shield */
				DowngradeShield(hi);
			}
			if(combat && !getrandom(80/numAttacks) ) {
				/* Damaging shot, downgrade shield */
				DowngradeShield(hi);
			}
		}
	}

	// Now, loop through and do attacks
	//
	int ret = 0; //number of successful attacks!
	for(int i = 0; i < numAttacks; i++) {
		/* 3. Get the target */
		int formhit = -1;
		int tarnum = GetTarget(attackers,attackerform,attackType,&formhit, special, b);
		if(tarnum<0) return ret; //This is needed whenever no target is found 
                      //(all enemies dead, or no valid target for that attacker,
                      //or that attacker moved to a new formation).

		#ifdef DEBUG
		if(formhit<0) {
		    cout << "Invalid target formation! " << formhit << " with tarnum " << tarnum << endl;
		    system("pause");
		    return ret;
  		}
  		#endif
		
		Soldier *tar = formations[formhit].GetSoldier(tarnum);
		
		#ifdef DEBUG
		if(!tar) {
		    Awrite("Invalid target!");
		    system("pause");
		    return ret;
		}
		#endif
		
		int tarFlags = 0;
		if(tar->weapon != -1) {
			WeaponType *pw = FindWeapon(ItemDefs[tar->weapon].abr);
			tarFlags = pw->flags;
		}


		/* 4. Add in any effects, if applicable */
		int tlev = 0; //target's defence skill
		if(attackType != NUM_ATTACK_TYPES)
			tlev = tar->dskill[ attackType ];
		if(special != NULL) {
			SpecialType *sp = FindSpecial(special);
			if((sp->effectflags & SpecialType::FX_NOBUILDING) && tar->building)
				tlev -= 2; //this assumes building defence bonus is 2! Not always right.
		}

		/* 4.1 Check whether defense is allowed against this weapon */
		if((flags & WeaponType::NODEFENSE) && (tlev > 0)) tlev = 0;

		if(!(flags & WeaponType::RANGED)) {
			/* 4.2 Check relative weapon length */
			int attLen = 1;
			int defLen = 1;
			if(flags & WeaponType::LONG) attLen = 2;
			else if(flags & WeaponType::SHORT) attLen = 0;
			if(tarFlags & WeaponType::LONG) defLen = 2;
			else if(tarFlags & WeaponType::SHORT) defLen = 0;
			if(attLen > defLen) attackLevel++;
			else if(defLen > attLen) tlev++;
			
			//Arcadia lines only:
			if(attLen != 1 || defLen != 1) b->AddLine("Length weapon detected. Please contact your GM!"); //Arcadia only!
		}

		//Check whether the formation has a combat bonus/penalty
		//This also gives a bonus/penalty to %chance to attack below.
		int attackbonus = 0;
		attackbonus += attackers->formations[attackerform].bonus;
		if((attackType != ATTACK_COMBAT) && (attackType != ATTACK_RIDING)) {
		    if(attackers->rangedbonus < 0) {
		    //elves don't get ranged penalties
		        ManType *mt = FindRace(ItemDefs[race].abr);
		        if(mt->ethnicity != RA_ELF || attackType != ATTACK_RANGED) attackbonus += attackers->rangedbonus;
            } else attackbonus += attackers->rangedbonus;
		}
		
		attackLevel += attackbonus; //adding here since protection values aren't supposed to be added
		
		/* 4.3 Add bonuses versus mounted */
		if(tar->riding != -1) attackLevel += mountBonus; // this and the previous line will mess up multi-attacks.

		/* 5. Attack soldier */
		
		if(!(flags & WeaponType::ALWAYSREADY)) {
			if(Globals->ADVANCED_FORTS) {
				attackbonus -= (tar->protection[attackType]+1)/2;
			}

			//if bonus==0, 50%. if bonus = 1, 67%. if bonus = -1, 33%
			//if bonus = 2, 75%
			//getrandom(tohit+tofail) < tohit
			//eg gr(2) < 1 = 50%.  gr(3) < 1/2 = 33%/67%
			
			//this is to get a chance to attack, not the actual attack.
			int tohit = 1+attackbonus;
			int tomiss = 1-attackbonus;
			if(tohit<1) tohit = 1;
			if(tomiss<1) tomiss = 1;

			if(getrandom(tohit+tomiss) < tomiss) {
				continue;
			}
		}


		if (attackType != NUM_ATTACK_TYPES) { //this excludes dispel illusions, anti-demons, etc.
    		if (!Hits(attackLevel,tlev)) {
    			continue;
    		}
		}
		/* 6. If attack got through, apply effect, or kill */
		if (effect == NULL) {
			/* 7. Last chance... Check armor */
			if (tar->ArmorProtect(weaponClass)) {
				continue;
			}

			if(attackType != ATTACK_COMBAT && attackType != ATTACK_RIDING && getrandom(tar->unit->GetSkill(S_TOUGHNESS)+1) ) {
                continue;
            }

			/* 8. Seeya! */
			formations[formhit].Kill(tarnum, this);
			ret++;
		} else {
			if (tar->HasEffect(effect)) {
				continue;
			}
			tar->SetEffect(effect, formhit, this);
			ret++;
		}
	}
	return ret;
}

void Army::AddLine(const AString & s)
{
  AString * temp = new AString(s);
  armytext.Add(temp);
}

void Army::WriteLosses(Battle * b) {
	b->AddLine(*(pLeader->name) + " loses " + (count - NumAlive()) + ".");

	if (NumAlive() != count) {
		AList units;
		for (int i=NumAlive(); i<count; i++) {
			if (!GetUnitList(&units,GetSoldier(i)->unit)) {
				UnitPtr *u = new UnitPtr;
				u->ptr = GetSoldier(i)->unit;
				units.Add(u);
			}
		}

		int comma = 0;
		AString damaged;
		forlist (&units) {
			UnitPtr *u = (UnitPtr *) elem;
			if (comma) {
				damaged += AString(", ") + AString(u->ptr->num);
			} else {
				damaged = AString("Damaged units: ") + AString(u->ptr->num);
				comma = 1;
			}
		}

		units.DeleteAll();
		b->AddLine(damaged + ".");
	}
}

int Army::Lose(Battle *b,ItemList *spoils, int ass)
//If speed is an issue, can create Formation::Lose/Tie/Win methods and not cycle through GetSoldier repeatedly.
{
    int numdead = 0;
    DoExperience();
	WriteLosses(b);
	if(ass && Globals->ARCADIA_MAGIC) AssassinationResurrect();
	for (int i=0; i<count; i++) {
		Soldier * s = GetSoldier(i);
		if (!s->isdead) {
			s->Alive(LOSS);
		} else {
			if ((s->unit->type==U_WMON) && (ItemDefs[s->race].type&IT_MONSTER))
				GetMonSpoils(spoils,s->race,s->unit->free);
			if(!s->illusion) numdead++;
			s->Dead();
		}
		delete s;
	}
	return numdead;
}

void Army::Tie(Battle * b)
{
    DoExperience();
	WriteLosses(b);
	for(int x=0; x<count; x++) {
		Soldier * s = GetSoldier(x);
		if (!s->isdead) {
			if(round > 1) s->Alive(WIN_NO_MOVE);
			else s->Alive(WIN_MOVE);  //if only one round, can still move.
		} else {
			s->Dead();
		}
		delete s;
	}
}

void Army::Win(Battle * b,ItemList * spoils, int enemydead)
{
	int wintype;
	DoExperience(enemydead);
	DoHeal(b, enemydead);
	WriteLosses(b);
	int na = NumNonIllusionsAlive();

	if (nonillusioncount != na && round > 1) {
        wintype = WIN_NO_MOVE;  //if only one round OR no casualties, can still move!
        b->AddLine("");
        b->AddLine("The victor's units are forbidden from further movement this month.");
	} else {
        wintype = WIN_MOVE;
        b->AddLine("");
        b->AddLine("All surviving units remain able to move");
    }

#ifdef DEBUG
    if(nonillusioncount < na) {
        b->AddLine("Problem with non-illusion count, please contact your GM");
    }
#endif


    //Distribute the Spoils
    na = NumAlive();
	AList units;
	forlist(spoils) {
		Item *i = (Item *) elem;
		if(i && na) {
			Unit *u;
			UnitPtr *up;
			
            //reset marker for every new spoil
			for(int x = 0; x < na; x++) {
				GetSoldier(x)->unit->marker = 0;
			}

            int numsol = 0;

			// Make a list of units who can get this type of spoil
			for(int x = 0; x < na; x++) {
				u = GetSoldier(x)->unit;
				if(u->CanGetSpoil(i)) {
                    numsol++;                  //total number of soldiers collecting this spoil
                    if(!u->marker) { //this is the first time we've visited this unit; add it to the list
    					up = new UnitPtr;
    					up->ptr = u;
    					units.Add(up);
                    }
				}
                u->marker++;          //marks number of spoil-claiming soldiers in the unit.
			}
            //first pass through, we are as fair as possible
            
			int numunits = units.Num();
			if(numunits > 0) {              //we have some units claiming spoils
			    int initialnum = i->num;    //initial number of spoils
			    
				forlist(&units) {
					up = (UnitPtr *)elem;
					int num = up->ptr->marker * initialnum / numsol;     //soldiers in unit * num items / total soldiers. Rounded down
					int num2 = up->ptr->CanGetSpoil(i);      //total number of spoil unit is allowed to take
					if(num2 < num) num = num2;
					up->ptr->items.SetNum(i->type,
							up->ptr->items.GetNum(i->type) + num);
					i->num -= num;
					up->ptr->faction->DiscoverItem(i->type, 0, 1);
				}
                #ifdef DEBUG
                if(i->num < 0) Awrite("Item distribution is whacked!");
                #endif
                //i->num spoils remain to be divided. Some of the units may no longer be able to accept spoils
				while((i->num > 0) && units.Num()) {
                    //We are no longer caring about being fair, just give the items away ok!
                    int luckyunit = getrandom(units.Num());                       
					up = (UnitPtr *)units.First();
					while(luckyunit > 0) {
						up = (UnitPtr *)units.Next(up);
						luckyunit--;
					}
					int num = up->ptr->CanGetSpoil(i);      //total number of spoil unit is allowed to take
                    if(num > i->num) num = i->num;
					up->ptr->items.SetNum(i->type, up->ptr->items.GetNum(i->type)+num);
					i->num -= num;
					up->ptr->faction->DiscoverItem(i->type, 0, 1);
                    units.Remove(up);  //we have given up all it can take, so remove it from the list
				}
				//we have given away all we can :)
			}
			units.DeleteAll();
		}
	}

	for(int x = 0; x < count; x++) {
		Soldier * s = GetSoldier(x);
		if (!s->isdead) s->Alive(wintype);
		else s->Dead();
		delete s;
	}
}

int Army::CanBeHealed()
{
	for (int i=NumAlive(); i<count; i++) {
		Soldier * temp = GetSoldier(i);
		if (temp->canbehealed) return 1;
	}
	return 0;
}

void Army::DoHeal(Battle * b, int enemydead)
{
	// Do magical healing
	for(int i = 6; i > 0; --i)
		DoHealLevel(b, i, 0);
	// Do Normal healing
	DoHealLevel(b, 1, 1);
	// Do resurrection
    formations[NUMFORMS].ResetHeal(); //resets dead to be able to be healed again.
	DoResurrect(b);
	DoNecromancy(b, enemydead);
}

void Army::DoHealLevel(Battle *b, int type, int useItems)
{
//This is also cloned below as DoResurrectLevel()
//NB: There is no healing of illusions!
	int rate = HealDefs[type].rate;

	for (int i=0; i<NumAlive(); i++) {
		Soldier * s = GetSoldier(i);
		int healed = 0;
		int failed = 0;
		if (!CanBeHealed()) break;
		if(s->healtype <= 0) continue;
		// This should be here.. Use the best healing first
		if(s->healtype != type) continue;
		if(!s->healing) continue;
		if(useItems) {
			if(s->healitem == -1) continue;
			if (s->healitem != I_HEALPOTION) s->unit->Practice(S_HEALING);
		} else {
			if(s->healitem != -1) continue;
			s->unit->Practice(S_MAGICAL_HEALING);
    		int mevent = s->unit->MysticEvent();
    		if(mevent) {
    	        b->AddLine( *(s->unit->name) + " tries to heal, but his spells fizzle.");
    	        continue;
    		}
			int max = ( 120 * s->unit->GetEnergy() )/ s->unit->GetCombatCost(S_MAGICAL_HEALING, 1);  //cost is per 120 corpses
			if(max < 1) continue;
			if(max < s->healing) s->healing = max;
		}

		while (s->healing) {
			if (!CanBeHealed()) break;
			int j = getrandom(count - NumAlive()) + NumAlive();
			Soldier * temp = GetSoldier(j);
			if (temp->canbehealed) { //this scales as n*n so could take a long time!
				s->healing--;
				if (getrandom(100) < rate) {
					healed++;
					//return soldier to life!
					temp->isdead = 0;
					formations[NUMFORMS].TransferSoldier((j-NumAlive()), &formations[NUMFORMS-1]);
				} else {
					temp->canbehealed = 0;
					failed++;
				}
			}
		}
		if(useItems == 0) {
		    //magical healing
    		int cost = s->unit->GetCombatCost(S_MAGICAL_HEALING, failed+healed);
    		cost = (cost+119)/120;
        	s->unit->energy -= cost;
        	int exper = (30*failed) / HealDefs[type].num;
        	if(exper > 15) exper = 15;
        	s->unit->Experience(S_MAGICAL_HEALING, exper);
		} else if(s->healitem != I_HEALPOTION) s->unit->Experience(S_HEALING,healed+failed,0);

		b->AddLine(*(s->unit->name) + " heals " + healed + ".");
		
	}
}

void Army::DoResurrect(Battle *b)
{
    //self-resurrection
   	for (int i=NumAlive(); i<count; i++) {
		Soldier * s = GetSoldier(i);
        if(s->unit->type == U_MAGE && (ItemDefs[s->race].type & IT_MAN) && s->unit->GetSkill(S_RESURRECTION) >= 5 && 
              s->unit->GetEnergy() >= s->unit->GetCastCost(S_RESURRECTION, 4)) {
             //unit can resurrect himself.
             s->unit->resurrects++;
             int mevent = s->unit->MysticEvent();
             if(mevent) {
                 s->unit->resurrects++;
                 s->unit->Event("Rises from the dead, but chaotic spirits exact twice the usual cost.");
             } else  s->unit->Event("Rises from the dead.");
             s->unit->energy -= s->unit->GetEnergy(); //resurrected mage is exhausted.
             s->isdead = 0;
             formations[NUMFORMS].TransferSoldier(0, &formations[NUMFORMS-1]);
        }
    }

    //resurrection of others. NB no resurrection of illusions.
	int rate = 50;

	for (int i=0; i<NumAlive(); i++) {
		Soldier * s = GetSoldier(i);
		int raised = 0;
		int failed = 0;
		if (!CanBeHealed()) break;
		if(s->unit->type != U_MAGE || !(ItemDefs[s->race].type & IT_MAN)) continue;
		int level = s->unit->GetSkill(S_RESURRECTION);
		if(!level) continue;
		int mevent = s->unit->MysticEvent();
		if(mevent) {
	        b->AddLine( *(s->unit->name) + " tries to resurrect, but the spell fizzles.");
	        continue;
		}
		
		int max = 4 * level * level; // 4 at level 1, 144 at level 6
		int max2 = ( 3 * s->unit->GetEnergy() )/ s->unit->GetCombatCost(S_RESURRECTION, 1);  //cost is per 3 corpses
		if(max2 < max) max = max2;
		s->unit->Practice(S_RESURRECTION);
		
		while (max) {
			if (!CanBeHealed()) break;
			int j = getrandom(count - NumAlive()) + NumAlive();
			Soldier * temp = GetSoldier(j);
			if (temp->canbehealed) {
				max--;
				if (getrandom(100) < rate) {
					raised++;
					//return soldier to life!
					temp->isdead = 0;
					formations[NUMFORMS].TransferSoldier((j-NumAlive()), &formations[NUMFORMS-1]);
					if(temp->unit->type == U_MAGE) {
                        temp->unit->resurrects++;
                        temp->unit->energy -= temp->unit->GetEnergy();  //resurrected mages are exhausted
                    }
				} else {
					temp->canbehealed = 0;
					failed++;
				}
			}
		}

		int cost = s->unit->GetCombatCost(S_RESURRECTION, failed+raised);
		cost = (cost+2)/3;
    	s->unit->energy -= cost;
		
		if(raised) b->AddLine(*(s->unit->name) + " resurrects " + raised + ".");
		int exper = (failed*40)/(4*level*level);
		if(exper>20) exper = 20;
		s->unit->Experience(S_RESURRECTION,exper,0);
	}
}

void Army::AssassinationResurrect()
{
    Soldier * s = GetSoldier(0); //assassination target
    //check if the soldier can resurrect himself (soldier is always a man, not a creature belonging to a mage)
    if(s->unit->type == U_MAGE && s->unit->GetSkill(S_RESURRECTION) >= 5 && 
          s->unit->GetEnergy() >= s->unit->GetCastCost(S_RESURRECTION, 4)) {
         //unit can resurrect himself.
         s->unit->resurrects++;
         int mevent = s->unit->MysticEvent();
         if(mevent) {
             s->unit->resurrects++;
             s->unit->Event("Rises from the dead, but chaotic spirits exact twice the usual cost.");
         } else  s->unit->Event("Rises from the dead.");
         s->unit->energy -= s->unit->GetEnergy(); //resurrected mage is exhausted.
         s->isdead = 0;
         formations[NUMFORMS].TransferSoldier(0, &formations[NUMFORMS-1]);
    }


    ARegion *reg = s->unit->object->region;
    Unit * bestmage = 0;
    int skill = 0;
    forlist(&reg->objects) {
        Object *o = (Object *) elem;
        forlist(&o->units) {
            Unit *u = (Unit *) elem;
            if(u->type != U_MAGE) continue;
            if(u->faction->GetAttitude(s->unit->faction->num) < A_ALLY) continue;
            int reskill = u->GetSkill(S_RESURRECTION);
            if(reskill < 3) continue; //needs at least level 3.
            if(reskill > skill) {
                if(u->GetEnergy() >= u->GetCastCost(S_RESURRECTION, 1) ) {  //note use cast cost here, and combat cost for normal battles above.
                    skill = reskill;
                    bestmage = u;
                }
            }
        }
    }
    if(!bestmage) return;
    bestmage->energy -= bestmage->GetCastCost(S_RESURRECTION, 1);
    int mevent = bestmage->MysticEvent();
    if(mevent) {
        bestmage->Event("Tries to resurrect assassination target, but the spell fizzles.");
        return;    
    }
    int savechance = 15*skill;
    if(getrandom(100) > savechance) {
        bestmage->Event("Tries to resurrect assassination target, but is unsuccessful.");
        return;
    }
    bestmage->Event(AString("Resurrects ") + *(s->unit->name));
    s->unit->Event(AString("Is resurrected by ") + *(bestmage->name));
    if(s->unit->type == U_MAGE) {
        s->unit->resurrects++;
        s->unit->energy -= s->unit->GetEnergy();
    }
	s->isdead = 0;
    formations[NUMFORMS].TransferSoldier(0, &formations[NUMFORMS-1]);
}

void Army::DoNecromancy(Battle *b, int enemydead)
{
//NB: There is no necromancy on illusions!
	int deadmen = enemydead;
	for(int i = NumAlive(); i < count; i++) {
	    Soldier * temp = GetSoldier(i);
	    if(!temp->illusion) deadmen++;
	}
	for(int l = 6; l > 0; --l) {
	    int rate = 30 + 10*l;
    	if(!deadmen) break;
    	for (int i=0; i<NumAlive(); i++) {
    	    if(!deadmen) break;
    		Soldier * s = GetSoldier(i);
		    if(s->unit->type != U_MAGE || !(ItemDefs[s->race].type & IT_MAN)) continue;
    		int max = s->unit->GetSkill(S_NECROMANCY);
    		if(max != l) continue;
    		int mevent = s->unit->MysticEvent();
    		if(mevent) {
    	        b->AddLine( *(s->unit->name) + " tries to raise skeletons, but the spell fizzles.");
    	        continue;
    		}
    		
    		max = ( 120 * s->unit->GetEnergy() )/ s->unit->GetCombatCost(S_NECROMANCY, 1);   //cost of 120 corpses.
    		s->unit->Practice(S_NECROMANCY);
    		int raised = 0;
    		int failed = 0;
    		
    		while(max && deadmen) {
    		    max--;
    		    deadmen--;
    		    if (getrandom(100) < rate) raised++;
    		    else failed++;
    		}

    		int cost = s->unit->GetCombatCost(S_NECROMANCY, failed+raised);
    		cost = (cost+119)/120;
    		s->unit->energy -= cost;

    		//give n skeletons
    		s->unit->items.SetNum(I_SKELETON, s->unit->items.GetNum(I_SKELETON) + raised);
    		b->AddLine(*(s->unit->name) + " raises " + raised + " skeletons.");

    		failed = (failed + 1)/2;
    		if(failed > 25) failed = 25;
            s->unit->Experience(S_NECROMANCY,failed,0);
    	}
    }
}

void Army::Regenerate(Battle *b)
{
    for(int i=0; i<NUMFORMS-1; i++) { //cycles through all non-dead formations
        formations[i].Regenerate(b);
    }
}

void Army::GetMonSpoils(ItemList *spoils,int monitem, int free)
{
	if((Globals->MONSTER_NO_SPOILS > 0) &&
			(free >= Globals->MONSTER_SPOILS_RECOVERY)) {
		// This monster is in it's period of absolutely no spoils.
		return;
	}

	/* First, silver */
	MonType *mp = FindMonster(ItemDefs[monitem].abr,
			(ItemDefs[monitem].type & IT_ILLUSION));
	int silv = mp->silver;
	if((Globals->MONSTER_NO_SPOILS > 0) && (free > 0)) {
		// Adjust the spoils for length of freedom.
		silv *= (Globals->MONSTER_SPOILS_RECOVERY-free);
		silv /= Globals->MONSTER_SPOILS_RECOVERY;
	}
	spoils->SetNum(I_SILVER,spoils->GetNum(I_SILVER) + getrandom(silv));

	int thespoil = mp->spoiltype;

	if (thespoil == -1) return;
	if (thespoil == IT_NORMAL && getrandom(2) && silv >= 60) thespoil = IT_TRADE;             //note that silver is a "normal" item, so can be generated here too!
	                                                      // silv >= 60 used because otherwise no trade items are selected.
	int count = 0;
	int i;
	for (i=0; i<NITEMS; i++) {
		if ((ItemDefs[i].type & thespoil) &&
				!(ItemDefs[i].type & IT_SPECIAL) &&
				!(ItemDefs[i].flags & ItemType::DISABLED) &&
                (ItemDefs[i].baseprice <= silv) ) { //BS mod to prevent very valuable items being dropped - eg STALs. Non-Arcadians may not want this behaviour. Note that monsters with silver value less than 30 will only drop food, and less than 60 will never drop trade (eliminating half usual spoils).
			count ++;
		}
	}
	
	if(count == 0) return; //can occur if, eg, spoil value is less than 60 and trade items are selected - this fixed, but could still eg have normal item and monster spoil value < 20, etc.
	count = getrandom(count) + 1;

	for (i=0; i<NITEMS; i++) {
		if ((ItemDefs[i].type & thespoil) &&
				!(ItemDefs[i].type & IT_SPECIAL) &&
				!(ItemDefs[i].flags & ItemType::DISABLED) &&
                (ItemDefs[i].baseprice <= silv) ) {                //Second half of BS mod above.
			count--;
			if (count == 0) {
				thespoil = i;
				break;
			}
		}
	}

	int val = getrandom(mp->silver * 2);
	if((Globals->MONSTER_NO_SPOILS > 0) && (free > 0)) {
		// Adjust for length of monster freedom.
		val *= (Globals->MONSTER_SPOILS_RECOVERY-free);
		val /= Globals->MONSTER_SPOILS_RECOVERY;
	}

	spoils->SetNum(thespoil,spoils->GetNum(thespoil) +
			(val + getrandom(ItemDefs[thespoil].baseprice)) /
			ItemDefs[thespoil].baseprice);
	//The above gives val/baseprice + rand(0,1), rounded down. This makes sense in all situations I can think of.
}

void Army::CombineEngagements(int formfrom, int formto, Army *enemy)
//This should be called after moving soldiers from formfrom to formto during a battle.
{
    for(int i=0; i<NUMFORMS; i++) {
        if( engagements[formfrom][i] > engagements[formto][i] ) engagements[formto][i] = engagements[formfrom][i];
        if( enemy->engagements[i][formfrom] > engagements[i][formto] ) engagements[i][formto] = engagements[i][formfrom];
        //if old one is empty, clear engagements.
        if(!formations[formfrom].GetNumMen()) {
            engagements[formfrom][i] = ENGAGED_NONE;
            engagements[i][formfrom] = ENGAGED_NONE;
        }
    }
    //ie if could catch old formation, can catch new one. Mostly the new formation will
    //have been empty (though not always) so the new one just inherits the stats of the
    //old.
}

int Army::GetMidRoundTarget(int formnum, Army *enemy, Battle *b)
{
    //we are no longer attacking our previous target. Subtract any target-specific bonus.
    formations[formnum].RemoveTemporaryBonus();
    //if this formation is behind, it does not want to engage anything.
    if(formations[formnum].Behind() != 0) return -1;

    //Are we in reserve?
    if(formations[formnum].Reserve() != 0) {
    //needs to deal with reserve formations that don't want to engage (unless their frontline
    //is wiped out and the enemy has archers) quickly. ie if reserve and still has a frontline,
    //then return if there are no enemy flank 2s.
    
    //check there are no flank 2 enemies (flank 1 enemies should have been engaged with 
    //when they became flank 1)
        int flanktwos = -1;
        for(int i=2*NUMFORMS/3; i<NUMFORMS; i++) {
            if(enemy->formations[i].GetNumMen() != 0) {
                flanktwos = i;
                engagements[formnum][i] = ENGAGED_ENGAGED;
                //The enemy has chosen its own target, so doesn't want to engage us at this stage
            }
        }
        if(flanktwos != -1) return flanktwos;
        
        //If we are in reserve and have frontline troops, quit
        if(formations[0].GetNumMen()) return -1;
        
    //there is no frontline. If there is an enemy frontline, exit as we have to stay in
    //defence against them till the next tactical phase.
        if(enemy->formations[0].GetNumMen()) return -1;
        //the enemy does not have a frontline either, and thus must be down to reserves
        //and behind formations. Let's attack a behind formation, by falling through to 
        //treatment of non-reserves.
    }
    
    //We have a formation which is not behind and if was in reserve, wishes to move out.
    //If we are flank 0 or 1, we need to move all our soldiers to flank 1 or 2,
    //and exit gracefully. The current soldier will skip his
    //remaining attacks as a movement penalty.    
    
    //NOT DONE! Before this though, if flank 0, check if there are flank 2 enemies, and engage them!
    
    if(formations[formnum].Flank() < FORM_FLANKED) {
        //Move soldiers out, while keeping attack ability of most.
        formations[formnum].MoveSoldiers(&formations[formnum+(NUMFORMS/FORM_DEAD)]); //ie move them to formnum+6
        CombineEngagements(formnum, formnum+(NUMFORMS/FORM_DEAD), enemy);
        AString temp;
        temp = *(pLeader->name) + "'s";
        if(formations[formnum].Flank() == FORM_FLANKING) temp += " flanking";
        else if(formnum == 0) temp += " frontline";
        else temp += " reserve";
        switch(formations[formnum].Type()) {
            case FORM_FOOT:
                temp += " infantry";
                break;
            case FORM_RIDE:
                temp += " cavalry";
                break;
            case FORM_FLY:
                temp += " aerial cavalry";
                break;
        }
        
        if(formations[formnum+(NUMFORMS/FORM_DEAD)].Flank() == FORM_FLANKING) {
            //new formation is flank 1, so sure it is intercepted. The flank 2's
            //will select a target in their own time. If it cannot be intercepted,
            //it will come back here and the soldiers will become flank 2.
            temp += " flanks";
            int comma = 0;
            for(int i=0; i<NUMFORMS/FORM_DEAD; i++) {
                if(!enemy->formations[i].Reserve() ) continue;
                if(enemy->engagements[i][formnum+(NUMFORMS/FORM_DEAD)] == ENGAGED_NONE) continue; //ie cannot catch this formation.
                //'i' is an enemy reserve formation able to catch ours. Dual-engage.
                engagements[formnum+(NUMFORMS/FORM_DEAD)][i] = ENGAGED_ENGAGED;
                enemy->engagements[i][formnum+(NUMFORMS/FORM_DEAD)] = ENGAGED_ENGAGED;
                comma = 1;
                if(!comma) temp += ", but is intercepted";
            }
            temp += ".";
            b->AddLine(temp);
           return -2;            
        }
        temp += " breaks through to their enemy's backline.";
        b->AddLine(temp);
        return -2;
    }

    // We have a flanked (flank 2) formation. It should engage a new target of its choice.

    //This means finding a tryattack target
    //If no tryattack targets, select from canattack
    //Both of these are covered in:
    return GetFlankedTarget(b, enemy, formnum);
}


int Army::GetMidRoundRangedTarget(int formnum, Army *enemy)
{
    //if this formation is in front, it does not want to disrupt its (melee) 
    //tryattack listings for the sake of a ranged soldier.
    if(formations[formnum].Behind() != 1) return -1;

    int newtarget = -1;
    //tryattack any flank 2 formations with men in them
    for(int i=2*(NUMFORMS/FORM_DEAD); i<NUMFORMS; i++) {
        #ifdef DEBUG
        if(enemy->formations[i].Flank() != FORM_FLANKED) {
            Awrite("Flanked form not flanked!");
            system("pause"); }
        if(engagements[formnum][i] >= ENGAGED_TRYATTACK) {
            Awrite("Targetless unit has a target!");
            system("pause"); }
        #endif
        if(enemy->formations[i].GetNumMen()) {
            newtarget = i;
            engagements[formnum][i] = ENGAGED_TRYATTACK;
        }
    }
    if(newtarget != -1) return newtarget; //returns the highest number target just tryattacked

    //else tryattack any flank 1 formations with men in them
    for(int i=(NUMFORMS/FORM_DEAD); i<2*(NUMFORMS/FORM_DEAD); i++) {
        #ifdef DEBUG
        if(enemy->formations[i].Flank() != FORM_FLANKING) {
            Awrite("Flanking form not flanking!");
            system("pause"); }
        if(engagements[formnum][i] >= ENGAGED_TRYATTACK) {
            Awrite("Targetless unit has a target!");
            system("pause"); }
        #endif
        if(enemy->formations[i].GetNumMen()) {
            newtarget = i;
            engagements[formnum][i] = ENGAGED_TRYATTACK;
        }
    }
    if(newtarget != -1) return newtarget;
    
    //else tryattack front formation
    #ifdef DEBUG
    if(engagements[formnum][0] >= ENGAGED_TRYATTACK) {
        Awrite("Targetless unit has a target!");
        system("pause"); }
    #endif
    if(enemy->formations[0].GetNumMen()) {
        newtarget = 0;
        engagements[formnum][0] = ENGAGED_TRYATTACK;
    }
    if(newtarget != -1) return newtarget;

    //else tryattack front formations in reserve
    if(enemy->formations[2].GetNumMen()) {
        newtarget = 2;
        engagements[formnum][2] = ENGAGED_TRYATTACK;
    }
    if(enemy->formations[4].GetNumMen()) {
        newtarget = 4;
        engagements[formnum][4] = ENGAGED_TRYATTACK;
    }
    if(newtarget != -1) return newtarget;
    
    //else tryattack behind formations
    for(int i=1; i<(NUMFORMS/FORM_DEAD); i += 2) {
        #ifdef DEBUG
        if(engagements[formnum][i] >= ENGAGED_TRYATTACK) {
            Awrite("Targetless unit has a target!");
            system("pause"); }
        #endif
        if(enemy->formations[i].GetNumMen()) {
            newtarget = i;
            engagements[formnum][i] = ENGAGED_TRYATTACK;
        }
    }
    return newtarget;
}

void Army::PenaltyToHit(int penalty)
{
    for( int i=0; i<NUMFORMS; i++) {
        formations[i].bonus -= penalty;
    }
}

void Army::SortFormations(Battle *b, int regtype)
{
    formations[0].Sort(this, b, regtype);
}

void Army::MirrorEngagements(Army *enemy)
{
    for(int i=0; i<NUMFORMS; i++) {
        //if this formation is empty, clear all engagements.
        if(!formations[i].GetNumMen() ) {
            for(int j=0; j<NUMFORMS; j++) {
                engagements[i][j] = ENGAGED_NONE;
                enemy->engagements[j][i] = ENGAGED_NONE;
            }
        } else {
            //if someone engaged us, we engage them.
            for(int j=0; j<NUMFORMS; j++) {
                if(enemy->engagements[j][i] == ENGAGED_ENGAGED) engagements[i][j] = ENGAGED_ENGAGED;
            }
        }
    }
}

void Army::FlankFlankers(Battle *b, Army *enemy)
//If flanking formations are unengaged, they become flanked.
{
    //cycle through the flanking formations only
    for(int i=(NUMFORMS/FORM_DEAD); i < 2*(NUMFORMS/FORM_DEAD); i++ ) {
        if(formations[i].GetNumMen() < 1) continue;
        if(formations[i].Engaged(this)) continue;
        //unengaged, therefore move all soldiers to flank 2 formation.
        int moving = formations[i].GetNumMen();
        formations[i].MoveSoldiers(&formations[i+(NUMFORMS/FORM_DEAD)]); //ie move them to formnum+6
        CombineEngagements(i, i+(NUMFORMS/FORM_DEAD), enemy);
        AString temp;
        temp = AString(moving) + " of ";
        temp += *(pLeader->name) + "'s flanking";
        switch(formations[i].Type()) {
            case FORM_FOOT:
                temp += " infantry";
                break;
            case FORM_RIDE:
                temp += " cavalry";
                break;
            case FORM_FLY:
                temp += " aerial cavalry";
                break;
        }
        temp += " break through to their enemy's backline.";
        b->AddLine(temp);
    }
}

void Army::AssignFrontTargets(Battle *b, Army *enemy, int ass)
//if Formation[0] unengaged: attack enemy flank 2s, then form[0], then flank.
{
    if(formations[0].GetNumMen() < 1) return;
    if(formations[0].Engaged(this) ) return;
    int done = 0;
    //check if there are enemy flank 2s
    for(int i=2*(NUMFORMS/FORM_DEAD); i<NUMFORMS; i++) {    
        if( enemy->formations[i].GetNumMen() ) {
            done = 1;
            engagements[0][i] = ENGAGED_ENGAGED;
            enemy->engagements[i][0] = ENGAGED_ENGAGED;
        }
    }
    if(done) {
        b->AddLine(*(pLeader->name) + "'s frontline infantry attempts to rescue their 'behind' soldiers");
        return;
    }
    
    if(enemy->formations[0].GetNumMen() ) {
        engagements[0][0] = ENGAGED_ENGAGED;
        enemy->engagements[0][0] = ENGAGED_ENGAGED;
        if(!ass) b->AddLine("The frontlines engage");
        return;
    }
    
    //flank!
    formations[0].MoveSoldiers(&formations[(NUMFORMS/FORM_DEAD)]); //ie move them to form 6
    CombineEngagements(0, (NUMFORMS/FORM_DEAD), enemy);
    AString temp;
    temp = *(pLeader->name) + "'s frontline infantry flanks.";
    b->AddLine(temp);
}

void Army::SplitOverwhelmingFrontFormations(Battle *b, Army *enemy)
{
//reserves should never be engaged with formation 0, so there shouldn't be
//double counting between here and the later splitoverwhelmingflankingformations

    float ratio = (float) 3 * (200 + 3 * enemy->taccontrol ) / (200 + 4 * taccontrol);
    // This gives 3 for taccontrol = 0 for both sides, or 2.5 for taccontrol = 100 for both
    //it gives 7.5 if enemy=100 own=0, or 1.0 if enemy = 0 own=100.

    int canblock[NUMFORMS];
    for(int i=0; i<NUMFORMS; i++) {
        canblock[i] = (int) (enemy->formations[i].GetSize() * ratio + 0.99);
    }

    int size = formations[0].GetSize();
    int engaged = 0;
    for(int j=0; j<NUMFORMS; j++) {
        if(engagements[0][j] != ENGAGED_ENGAGED) continue;
        //we are engaged with this enemy formation.
        engaged = 1;
        if(canblock[j] == 0) continue;
        
        if(canblock[j] >= size) {
            canblock[j] -= size;
            size = 0;
            break;
        } else {
            size -= canblock[j];
            canblock[j] = 0;
        }
    }
    if(size < 1) return;
    //we have some men who can flank - they were not blocked.
    int concealedbonus = 0;
    if(getrandom(3) < concealment) concealedbonus = (NUMFORMS/FORM_DEAD);
    int moved = formations[0].MoveSoldiers(&formations[(NUMFORMS/FORM_DEAD)+concealedbonus], size, 0);
    //do not combine engagements, as these men have got past.

    if(moved) {
        AString temp;
        temp = AString(moved) + " of ";
        temp += *(pLeader->name) + "'s infantry flank";
        if(engaged) temp += " around their overwhelmed opponents";
        temp += ".";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
    }
}

void Army::SplitOverwhelmingFlankingFormations(Battle *b, Army *enemy, int regtype)
{
    //multiple formations can be engaged with each other, making counting difficult.
    //set the ratio needed before can overwhelm
    float ratio = (float) 3 * (200 + 3 * enemy->taccontrol ) / (200 + 4 * taccontrol);
    // This gives 3 for taccontrol = 0 for both sides, or 2.5 for taccontrol = 100 for both
    //it gives 7.5 if enemy=100 own=0, or 1.0 if enemy = 0 own=100.

    int canblock[NUMFORMS];
    for(int i=0; i<NUMFORMS; i++) {
        canblock[i] = (int) (enemy->formations[i].GetSize() * ratio + 0.99);
    }

    for(int i=(NUMFORMS/FORM_DEAD); i<2*(NUMFORMS/FORM_DEAD); i++) {
        if(formations[i].Type() == FORM_FOOT && (TerrainDefs[regtype].flags & TerrainType::RESTRICTEDFOOT) ) continue;
        
        int size = formations[i].GetSize();
        int engaged = 0;
        for(int j=0; j<NUMFORMS; j++) {
            if(engagements[i][j] != ENGAGED_ENGAGED) continue;
            //we are engaged with this enemy formation.
            engaged = 1;
            if(canblock[j] == 0) continue;
            
            if(canblock[j] >= size) {
                canblock[j] -= size;
                size = 0;
                break;
            } else {
                size -= canblock[j];
                canblock[j] = 0;
            }
        }
        if(size < 1) continue;
        //we have some men who can flank - they were not blocked.
        int moved = formations[i].MoveSoldiers(&formations[i+(NUMFORMS/FORM_DEAD)], size, 0);
        //do not combine engagements, as these men have got past.
        if(moved < 1) continue;
        
        AString temp;
        temp = AString(moved) + " of ";
        temp += *(pLeader->name) + "'s flanking";
        switch(formations[i].Type()) {
            case FORM_FOOT:
                temp += " infantry";
                break;
            case FORM_RIDE:
                temp += " cavalry";
                break;
            case FORM_FLY:
                temp += " aerial cavalry";
                break;
        }

        if(engaged) temp += " flank around their overwhelmed opponents and";
        temp += " reach their enemy's backline.";
        b->AddLine(temp);
    }
}

void Army::SetCanAttack(Army *enemy)
{
    for(int i=0; i<NUMFORMS; i++) {
        //who can this formation catch?
        //flat chance to catch:
        //foot: 100% foot, 40% ride, 10% fly
        //ride: 100% foot, 95% ride, 30% fly
        //fly: 100% foot, 100% ride, 50% fly
        //if cannot catch, has chance to catch conditionally:
        //foot: 30% ride, 10% fly
        //ride: 60% ride, 30% fly
        //fly: 100% ride, 50% fly
        for(int j=0; j<NUMFORMS; j++) {
            if(engagements[i][j] != ENGAGED_NONE) continue; //do not downgrade engagement status!
            //if enemy is flanked, can always at least conditionally engage him.
            if(enemy->formations[j].Flank() == FORM_FLANKED) engagements[i][j] = ENGAGED_CONDITIONAL;
            if(enemy->formations[j].Type() == FORM_FOOT) engagements[i][j] = ENGAGED_CANATTACK;
            else if(enemy->formations[j].Type() == FORM_RIDE) {
                if(formations[i].Type() == FORM_FOOT && (getrandom(100) < 40) ) engagements[i][j] = ENGAGED_CANATTACK;
                else if(formations[i].Type() == FORM_FOOT && (getrandom(100) < 30) ) engagements[i][j] = ENGAGED_CONDITIONAL;
                else if(formations[i].Type() == FORM_RIDE && (getrandom(100) < 100) ) engagements[i][j] = ENGAGED_CANATTACK;  //currently 100%!
                else if(formations[i].Type() == FORM_RIDE && (getrandom(100) < 60) ) engagements[i][j] = ENGAGED_CONDITIONAL;
                else if(formations[i].Type() == FORM_FLY) engagements[i][j] = ENGAGED_CANATTACK;
            }
            else if(enemy->formations[j].Type() == FORM_FLY) {
                if(formations[i].Type() == FORM_FOOT && (getrandom(100) < 10) ) engagements[i][j] = ENGAGED_CANATTACK;
                else if(formations[i].Type() == FORM_FOOT && (getrandom(100) < 10) ) engagements[i][j] = ENGAGED_CONDITIONAL;
                else if(formations[i].Type() == FORM_RIDE && (getrandom(100) < 30) ) engagements[i][j] = ENGAGED_CANATTACK;
                else if(formations[i].Type() == FORM_RIDE && (getrandom(100) < 30) ) engagements[i][j] = ENGAGED_CONDITIONAL;
                else if(formations[i].Type() == FORM_FLY && (getrandom(100) < 50) ) engagements[i][j] = ENGAGED_CANATTACK;
                else if(formations[i].Type() == FORM_FLY && (getrandom(100) < 50) ) engagements[i][j] = ENGAGED_CONDITIONAL;
            }

        }
    }
}

void Army::ReservesIntercept(Army *enemy)
{
    int reserves = 2;
    if(formations[2].GetNumMen() == 0) {
        reserves--;
        for(int i=0; i<NUMFORMS; i++) {
            engagements[2][i] = ENGAGED_NONE;
        }
    }
    if(formations[4].GetNumMen() == 0) {
        reserves--;
        for(int i=0; i<NUMFORMS; i++) {
            engagements[4][i] = ENGAGED_NONE;
        }
    }
    if(!reserves) return;
    //we have some reserves.
    
    for(int j=0; j<NUMFORMS; j++) {
        //we are engaging flanking formations ...
        if(enemy->formations[j].Flank() != FORM_FLANKING) continue;
        //...with men in them...
        if(enemy->formations[j].GetNumMen() == 0) continue;
        //...that we can catch.
        if(engagements[2][j] > ENGAGED_NONE) {
        //if we can attack, or if we can conditionally attack and the other reserves can attack, then engage.
            if((engagements[2][j] > ENGAGED_CONDITIONAL) || (engagements[4][j] > ENGAGED_CONDITIONAL)) {
                engagements[2][j] = ENGAGED_ENGAGED;
                enemy->engagements[j][2] = ENGAGED_ENGAGED;
            }
        }
        if(engagements[4][j] > ENGAGED_NONE) {
        //if we can attack, or if we can conditionally attack and the other reserves can attack, then engage.
            if((engagements[2][j] > ENGAGED_CONDITIONAL) || (engagements[4][j] > ENGAGED_CONDITIONAL)) {
                engagements[4][j] = ENGAGED_ENGAGED;
                enemy->engagements[j][4] = ENGAGED_ENGAGED;
            }
        }
    
    }
}

int Army::SizeFrontFliers()
{
    int size = 0;
    for(int i=0; i<NUMFORMS; i++) {
       if(formations[i].Type() != FORM_FLY) continue;
       if(formations[i].Behind() != 0) continue;
       size += formations[i].GetSize();
    }
    return size;
}

int Army::SizeFrontRiders()
{
    int size = 0;
    for(int i=0; i<NUMFORMS; i++) {
       if(formations[i].Type() != FORM_RIDE) continue;
       if(formations[i].Behind() != 0) continue;
       size += formations[i].GetSize();
    }
    return size;
}

int Army::SizeBehind()
{
    int size = 0;
    for(int i=0; i<NUMFORMS; i++) {
       if(formations[i].Behind() == 0) continue;
       size += formations[i].GetSize();
    }
    return size;
}


void Army::FlyingReservesMayFlank(Battle *b, Army *enemy)
{
    //For now, lets just look at the flyers
    if(!formations[4].GetSize()) return;
        
    float top = 0;
    float bottomone = 1;
    float bottomtwo = 0;
    float catcheffect;
    float behindscore = 1;
 
    if((enemy->SizeFrontFliers() + enemy->SizeFrontRiders()) == 0) {
        //the enemy has no cavalry to defend against. If they have any units which have not
        //flanked, lets flank!
        int backline = 0;
            for(int i=0; i<(NUMFORMS/FORM_DEAD); i++) {
                if(enemy->formations[i].GetNumMen()) backline = 1;        
            }
        if(!backline) return; //the enemy troops must have flanked, so lets intercept them as reserves.
        int moved = formations[4].GetNumMen();

        formations[4].MoveSoldiers(&formations[16]);
        //do not combine engagements
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s aerial cavalry";
        temp += " flank around the enemy unhindered.";
        b->AddLine(temp);
        return;
    }
    
    int concealedbonus = 0;
    if(getrandom(3) < concealment) concealedbonus = (NUMFORMS/FORM_DEAD);
    
    if(SizeBehind() ) {
        for(int i=0; i<NUMFORMS; i++) {
        //add score from all behind formations.
            if(formations[i].Behind() == 0) continue;
            if(formations[i].GetNumMen()) {
                if(formations[i].Type() == FORM_FLY) catcheffect = 0.7;
                else catcheffect = 1;
                
                bottomone += formations[i].NumRangedAttacks() * catcheffect;
            }
        //add score from all behind enemy formations
            if(enemy->formations[i].GetNumMen()) {
                if(enemy->formations[i].Type() == FORM_FLY) catcheffect = 0.7;
                else catcheffect = 1;
                
                top += enemy->formations[i].NumRangedAttacks() * catcheffect;
                bottomtwo += enemy->formations[i].NumMeleeAttacks() * catcheffect;
            }
        }
        float divisor = (float) enemy->SizeBehind();
        if(SizeFrontFliers() > divisor) divisor = (float) SizeFrontFliers(); //sizefrontfliers > 0
        #ifdef DEBUG
        if(divisor == 0) {
            Awrite("ReservesMayFlank dividing by zero");
            system("pause");
        } 
        #endif
        top /= divisor;
        bottomtwo /= divisor;
        divisor = (float) SizeBehind();
        if(SizeFrontFliers() > divisor) divisor = (float) SizeFrontFliers();
         #ifdef DEBUG
        if(divisor == 0) {
            Awrite("ReservesMayFlank dividing by zero");
            system("pause");
        } 
        #endif
        bottomone /= divisor;
        behindscore = top / (bottomone + bottomtwo);
    } else if (enemy->SizeBehind() ) {
        //no behind for us, but behind for them. Flank!
        int moved = formations[4].GetNumMen();
        formations[4].MoveSoldiers(&formations[10+concealedbonus]);
        //do not combine engagements
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s aerial cavalry";
        temp += " attempt to flank around the enemy.";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
        return;
    } else {
        //neither side has a behind. For sake of balance, whichever side has more fliers gets to flank.
        if(enemy->SizeFrontFliers() > SizeFrontFliers() ) return;
        //if enemy has flanked, return
        if(enemy->formations[10].GetSize() || enemy->formations[16].GetSize()) return;
        //flank all troops.
        int moved = formations[4].GetNumMen();
        formations[4].MoveSoldiers(&formations[10+concealedbonus]);
        //do not combine engagements
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s aerial cavalry";
        temp += " attempt to flank around the enemy.";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
        return;
    }
    
    //we now have a behindscore, which tells us how favourable attacking their behind
    //looks compared to defending our own (1 = neutral, >1 is good, <1 is poor). We know
    //we have fliers, and the enemy has either fliers or cavalry.
    
    if(behindscore > 1 && (enemy->SizeFrontFliers() == 0)) {
        //we want to flank and the enemy has no defending fliers. Flank!
        int moved = formations[4].GetNumMen();
        formations[4].MoveSoldiers(&formations[10+concealedbonus]);
        //do not combine engagements
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s aerial cavalry";
        temp += " attempt to flank around the enemy.";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
        return;
    }
    
    //lets get the ratio of our cavalry to the enemy's.
    float inverseratio = enemy->SizeFrontFliers() / SizeFrontFliers();
    float inversecombined = (enemy->SizeFrontFliers() + enemy->SizeFrontRiders()) / (SizeFrontFliers() + SizeFrontRiders());
    
    //if flanking is unattractive and the cavalry ratio is worse than the flying one, use the
    //cavalry ratio.
    if((behindscore <= 1) && (inversecombined > inverseratio)) inverseratio = inversecombined;
    
    //inverseratio should now be non-zero
    #ifdef DEBUG
    if(inverseratio == 0) {
        Awrite("ReservesMayFlank inverse ratio is zero");
        system("pause");
    }
    #endif
    float critical = 0.8;
    // if tactitians are able to be aggressive or defensive, adjust this number to
    // 0.4 or 1.6 respectively.
    
    if( (behindscore / inverseratio) < critical && (inverseratio > (1/3))) return; //decided not to flank, too much opposition.
    if( inverseratio > 4 ) return; //too much opposition to try and flank.
    
    //we are flanking. Split by two conditions to see how many men flank, and how many
    //stay in reserve.
    
    if(behindscore < 2) {
        int tosend = (int) ((float) (2 * behindscore * SizeFrontFliers()) / 5);
        int tostay = SizeFrontFliers() - tosend;
        if(tostay < enemy->SizeFrontFliers() / 4) tostay = 0; //this only occurs above about r = 1.5 with the default critical condition, but can occur for lower r if aggressive.
        if(tostay > 3 * enemy->SizeFrontFliers() ) tostay = 3 * enemy->SizeFrontFliers();

        //we use tostay, because tosend includes soldiers sent in earlier rounds and now
        //in formations 6 and 10. Note MoveSoldiers(,,) is robust enough to handle negative
        //numbers of soldiers to move, and will move no-one if that is the case.
        int moved = formations[4].MoveSoldiers(&formations[10+concealedbonus], (formations[4].GetSize() - tostay), 1);
        //do not combine engagements
        if(!moved) return;
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s aerial cavalry";
        temp += " attempt to flank around the enemy.";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
        return;
    } else {
        //behindscore 2 or more.
        if(behindscore * inverseratio > 1 ) {
            //very attractive target. Everyone flanks.
            int moved = formations[4].GetNumMen();
            formations[4].MoveSoldiers(&formations[10+concealedbonus]);
            //do not combine engagements
            AString temp;
            temp = AString(moved) + " of ";
            temp = *(pLeader->name) + "'s aerial cavalry";
            temp += " attempt to flank around the enemy.";
            if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
            b->AddLine(temp);
            return;
        } else {
            int tostay = SizeFrontFliers() / 4;
            if(tostay > 3 * enemy->SizeFrontFliers() ) tostay = 3 * enemy->SizeFrontFliers();
            int moved = formations[4].MoveSoldiers(&formations[10+concealedbonus], (formations[4].GetSize() - tostay), 1);
            //do not combine engagements
            AString temp;
            temp = AString(moved) + " of ";
            temp = *(pLeader->name) + "'s aerial cavalry";
            temp += " attempt to flank around the enemy.";
            if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
            b->AddLine(temp);
            return;
        }
    }
}

void Army::RidingReservesMayFlank(Battle *b, Army *enemy)
//This is almost, but not quite, a copy of the method above. Unfortunately
//the difference is big enough to make combining it into one method difficult.
{
    //Now, lets just look at the riders
    if(!formations[2].GetSize()) return;

    float top = 0;
    float bottomone = 1;
    float bottomtwo = 0;
    float catcheffect;
    float behindscore = 1;
 
    if(enemy->SizeFrontRiders() == 0) {
        //the enemy has no cavalry to defend against. If they have any units which have not
        //flanked, lets flank!
        int backline = 0;
            for(int i=0; i<(NUMFORMS/FORM_DEAD); i++) {
                if(enemy->formations[i].GetNumMen()) backline = 1;        
            }
        if(!backline) return; //the enemy troops must have flanked, so lets intercept them as reserves.
        int moved = formations[2].GetNumMen();
        formations[2].MoveSoldiers(&formations[14]);
        //do not combine engagements
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s cavalry";
        temp += " flank around the enemy unhindered.";
        b->AddLine(temp);
        return;
    }
    
    int concealedbonus = 0;
    if(getrandom(3) < concealment) concealedbonus = (NUMFORMS/FORM_DEAD);

    if(SizeBehind() ) {
        for(int i=0; i<NUMFORMS; i++) {
        //add score from all behind formations.
            if(formations[i].Behind() == 0) continue;
            if(formations[i].GetNumMen()) {
                if(formations[i].Type() == FORM_FLY) catcheffect = 0.4;
                else if(formations[i].Type() == FORM_RIDE) catcheffect = 0.8;
                else catcheffect = 1;
                
                bottomone += formations[i].NumRangedAttacks() * catcheffect;
            }
        //add score from all behind enemy formations
            if(enemy->formations[i].GetNumMen()) {
                if(formations[i].Type() == FORM_FLY) catcheffect = 0.4;
                else if(formations[i].Type() == FORM_RIDE) catcheffect = 0.8;
                else catcheffect = 1;
                
                top += enemy->formations[i].NumRangedAttacks() * catcheffect;
                bottomtwo += enemy->formations[i].NumMeleeAttacks() * catcheffect;
            }
        }
//b->AddLine(AString("top ") + (int) top);
//b->AddLine(AString("bottom ") + (int) bottomone + " " + (int) bottomtwo);
        float divisor = (float) enemy->SizeBehind();
        if(SizeFrontRiders() > divisor) divisor = (float) SizeFrontRiders(); //sizefrontfliers > 0
        #ifdef DEBUG
        if(divisor == 0) {
            Awrite("ReservesMayFlank dividing by zero");
            system("pause");
        } 
        #endif
        top /= divisor;
        bottomtwo /= divisor;
        divisor = (float) SizeBehind();
        if(SizeFrontRiders() > divisor) divisor = (float) SizeFrontRiders();
         #ifdef DEBUG
        if(divisor == 0) {
            Awrite("ReservesMayFlank dividing by zero");
            system("pause");
        } 
        #endif
        bottomone /= divisor;
        behindscore = top / (bottomone + bottomtwo);
//b->AddLine((int) behindscore);        
    } else if (enemy->SizeBehind() ) {
        //no behind for us, but behind for them. Flank!
        int moved = formations[2].GetNumMen();
        formations[2].MoveSoldiers(&formations[8+concealedbonus]);
        //do not combine engagements
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s cavalry";
        temp += " attempt to flank around the enemy.";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
        return;
    } else {
        //neither side has a behind. For sake of balance, whichever side has more fliers gets to flank.
        if(enemy->SizeFrontFliers() > SizeFrontFliers() ) return;
        //if enemy has flanked, return
        if(enemy->formations[8].GetSize() || enemy->formations[14].GetSize()) return;
        //flank all troops.
        int moved = formations[2].GetNumMen();
        formations[2].MoveSoldiers(&formations[8+concealedbonus]);
        //do not combine engagements
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s cavalry";
        temp += " attempt to flank around the enemy.";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
        return;
    }
    
    //we now have a behindscore, which tells us how favourable attacking their behind
    //looks compared to defending our own (1 = neutral, >1 is good, <1 is poor). We know
    //we have riders, and the enemy has either fliers or cavalry.

    if(behindscore > 1 && ((enemy->SizeFrontRiders() + enemy->SizeFrontFliers()) == 0)) {
        //we want to flank and the enemy has no defending cavalry. Flank!
        int moved = formations[2].GetNumMen();
        formations[2].MoveSoldiers(&formations[8+concealedbonus]);
        //do not combine engagements
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s cavalry";
        temp += " attempt to flank around the enemy.";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
        return;
    }

    //lets get the ratio of our cavalry to the enemy's.
    float inverseratio = (float) enemy->SizeFrontRiders() / SizeFrontRiders();
    float inversecombined = (float) (enemy->SizeFrontFliers() + enemy->SizeFrontRiders()) / (SizeFrontFliers() + SizeFrontRiders());
    
    //if flanking is attractive and the combined ratio is better than the flying one, use the
    //combined ratio.
    if((behindscore >= 1) && (inversecombined < inverseratio)) inverseratio = inversecombined;
    
    //inverseratio should be non-zero
//    #ifdef DEBUG
    if(inverseratio == 0) {
        Awrite("ReservesMayFlank inverse ratio is zero");
        b->AddLine("ReservesMayFlank Inverseratio is zero, please contact your GM!");
//        system("pause");
    }
//    #endif
    float critical = 0.8;
    if(pLeader->tactics == TACTICS_AGGRESSIVE) critical /= 2;
    else if(pLeader->tactics == TACTICS_DEFENSIVE) critical *= 2;
    // if tactitians are able to be aggressive or defensive, adjust this number to
    // 0.4 or 1.6 respectively.
    if( (behindscore / inverseratio) < critical && (inverseratio > (1/3))) return; //decided not to flank, too much opposition. (second part says we have less than three times the number of enemy cav)
    if( inverseratio >= 4 && pLeader->tactics != TACTICS_AGGRESSIVE) return; //too much opposition to try and flank.

    //we are flanking. Split by two conditions to see how many men flank, and how many
    //stay in reserve.
    if(behindscore < 2) {
        int tosend = (int) ((float) (2 * behindscore * SizeFrontFliers()) / 5);
        if(pLeader->tactics == TACTICS_AGGRESSIVE) tosend = (tosend*4)/3;
        int tostay = SizeFrontFliers() - tosend;
        if(tostay < enemy->SizeFrontFliers() / 4) tostay = 0; //this only occurs above about r = 1.5 with the default critical condition, but can occur for lower r if aggressive.
        if(tostay > 3 * enemy->SizeFrontFliers() ) tostay = 3 * enemy->SizeFrontFliers();

        //we use tostay, because tosend includes soldiers sent in earlier rounds and now
        //in formations 6 and 10. Note MoveSoldiers(,,) is robust enough to handle negative
        //numbers of soldiers to move, and will move no-one if that is the case.
        int moved = formations[2].MoveSoldiers(&formations[8+concealedbonus], (formations[2].GetSize() - tostay), 1);
        //do not combine engagements
        if(!moved) return;
        AString temp;
        temp = AString(moved) + " of ";
        temp = *(pLeader->name) + "'s cavalry";
        temp += " attempt to flank around the enemy.";
        if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
        b->AddLine(temp);
        return;
    } else {
        //behindscore 2 or more.
        if(behindscore * inverseratio > 1 ) {
            //very attractive target. Everyone flanks.
            int moved = formations[2].GetNumMen();
            formations[2].MoveSoldiers(&formations[8+concealedbonus]);
            //do not combine engagements
            AString temp;
            temp = AString(moved) + " of ";
            temp = *(pLeader->name) + "'s cavalry";
            temp += " attempt to flank around the enemy.";
            if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
            b->AddLine(temp);
            return;
        } else {
            int tostay = SizeFrontFliers() / 4;
            if(tostay > 3 * enemy->SizeFrontFliers() ) tostay = 3 * enemy->SizeFrontFliers();
            int moved = formations[2].MoveSoldiers(&formations[8+concealedbonus], (formations[2].GetSize() - tostay), 1);
            //do not combine engagements
            AString temp;
            temp = AString(moved) + " of ";
            temp = *(pLeader->name) + "'s cavalry";
            temp += " attempt to flank around the enemy.";
            if(concealedbonus) temp += " Their passage is concealed and they reach their enemy's backline unhindered.";
            b->AddLine(temp);
            return;
        }
    }
}

void Army::AssignRangedTargets(Army *enemy)
{
    for(int i=0; i<NUMFORMS; i++) {
        if(!formations[i].Behind()) continue;
        if(!formations[i].GetNumMen()) continue;
        if(formations[i].Engaged(this)) continue;
        
        //ranged formation, with men and no engagement.
        GetMidRoundRangedTarget(i,enemy);
    }
}

float Army::KillChance(float attack, float defence)
//this is approximate only, because the numbers coming in are averaged anyway.
{
//if attack>>defence, return 1
//if attack<<defence, return 0.05 (min value corresponds to 1:19 or level diff of 4.2)    
    float tohit = 1;
    float tomiss = 1;
    float diff = (attack - defence);
    if (diff > 4.2) return 0.95;
    if (diff < -4.2) return 0.05;
    while(diff > 0.3) { // max 14 cycles ... not too time intensive.
        diff -= 0.3;
        tohit *= 1.231;
    }
    while(diff < -0.3) {
        diff += 0.3;
        tomiss *= 1.231;
    }
    return (tohit / (tohit + tomiss));
}

int Army::GetFlankedTarget(Battle *b, Army *enemy, int formnum)
{
//return of -1 means already engaged.
//return of -2 means cannot find any target.

    //timesaving: if we have no men, return
    if(!formations[formnum].GetNumMen()) return -2;
//this assumes we are a melee formation. If we ever allow ranged formations to flank, will have
//to write a new section for them.
    int besttarget = -1;
    //if we are already engaged, return.
    for(int j=0; j<NUMFORMS; j++) 
        if(engagements[formnum][j] == ENGAGED_ENGAGED) return j;

    //if we are trying to attack someone, return them.
    for(int j=0; j<NUMFORMS; j++) {
        if(engagements[formnum][j] == ENGAGED_TRYATTACK) { //we should probably check there are still men in this enemy formation!
            if(enemy->formations[j].GetSize()) {
                #ifdef DEBUG
                cout << "Attacking a tryattack formation";
                #endif
                engagements[formnum][j] = ENGAGED_ENGAGED;
                if(enemy->formations[j].Flank() != FORM_FLANKED ) enemy->engagements[j][formnum] = ENGAGED_ENGAGED;   //they are forced to engage us ... but NOT if they are a flank 2 formation!
                else if(enemy->engagements[j][formnum] < ENGAGED_CANATTACK) enemy->engagements[j][formnum] = ENGAGED_CANATTACK; //BUT, they should be _able_ to since we are fighting them
                besttarget = j;
            } else {
                //try attack target empty
                engagements[formnum][j] = ENGAGED_CANATTACK; //don't want to set it to none in case someone moves into this formation later
            }
        }
    }
    if(besttarget != -1) {
        AString temp = *(pLeader->name) + "'s ";
        temp += AString(formations[formnum].GetNumMen());
        switch(formations[formnum].Type()) {
            case FORM_RIDE:
                temp += " flanked cavalry";
                break;
            case FORM_FLY:
                temp += " flanked aerial cavalry";
                break;
            default:
                temp += " flanked infantry";
                break;
        }
        temp += " engage the enemy";
        if(enemy->formations[besttarget].Behind()) temp += " ranged";
        else if(besttarget == 0) formations[formnum].SetTemporaryBonus(1);
        switch(enemy->formations[besttarget].Type()) {
            case FORM_RIDE:
                temp += " cavalry.";
                break;            
            case FORM_FLY:
                temp += " aerial cavalry";
                break;    
            default:
                temp += " infantry";
                break;
        }
        if(besttarget == 0) temp += " from behind";
        temp += ".";
        b->AddLine(temp);
        return besttarget;
    }
    float bestscore = 0;
    int secondbesttarget = -1;
    float secondbestscore;
    float ourattackl = formations[formnum].MeleeAttackLevel();
    float ourdefencel = formations[formnum].MeleeDefenceLevel();
    float ourattacks = formations[formnum].NumMeleeAttacks();
    
    float score;
    
    for(int i=0; i<NUMFORMS; i++) {
        if(enemy->formations[i].GetSize() < 1) continue;
        //if we cannot catch this formation, forget about engaging it!

        if(engagements[formnum][i] == ENGAGED_NONE) continue;
        //there is some doubt here - what if a later formation engages this one? Best solution I
        //can think of is to do the flying flankers first when called from tactical phase,
        //because these are most likely to be able to engage if they choose to.
        if(engagements[formnum][i] == ENGAGED_CONDITIONAL && !enemy->formations[i].Engaged(enemy) ) continue;
        //if there are non flank-2 enemies, do not choose a flank 2 enemy. Note flank 2 enemies
        //are checked last, so this should work :).
        if(besttarget != -1 && enemy->formations[i].Flank() == FORM_FLANKED) continue;

        //these all get used regardless of whether formation is ranged or melee.
        float rangedattacks = (float) enemy->formations[i].NumRangedAttacks();
        float meleeattacks = (float) enemy->formations[i].NumMeleeAttacks();
        float defence = enemy->formations[i].MeleeDefenceLevel();
        float attack = enemy->formations[i].MeleeAttackLevel();
        float size = (float) enemy->formations[i].GetSize();
        if(enemy->formations[i].Behind() != 0) {
            //ranged formation

            //score is 'how many friendlies we save': kx - y.  k = enemy kills per size. 
            //x = our expected kills. y = extra kills their melee do.
            //k = numrangedattacks / 2 / size (assume they are skilled enough to get 50% kills)
            //x = ourattacks * chance[ourattack,theirdefence] / 2
            //y = nummeleeattacks / 2 * chance[theirattack,ourdefence] - #friendlysengaged with them (min 0).
        
            score = rangedattacks * ourattacks * KillChance(ourattackl, defence)  / (4 * size);
            
            float penalty = meleeattacks * KillChance(attack, ourdefencel) / 2;
            //NOT DONE: If we are in anti-ranged terrain, the score should be multiplied by 2/3 
            //(to bring hit chance to 1/3) and
            //the penalty should include 1/6 of their ranged attacks.
            
            for(int j=0; j<NUMFORMS; j++) {
            //subtract number they are already engaged with; forget about changing the kill chance.
                if(enemy->engagements[i][j] == ENGAGED_ENGAGED) penalty -= formations[j].GetSize();
            }
            if(penalty > 0) score -= penalty;
            //if we are agressive, we want to hit behind formations, so double this score.
            //melee scores are never negative, so if this is negative doubling has no effect on choice order.
            if(pLeader->tactics == TACTICS_AGGRESSIVE) score *= 2;
            
            //we have a target score;
            if(besttarget == -1 || score > bestscore) {
                secondbesttarget = besttarget;
                secondbestscore = bestscore;
                besttarget = i;
                bestscore = score;
            } else if(secondbesttarget == -1 || score > secondbestscore) {
                secondbesttarget = i;
                secondbestscore = score;
            }
        } else {
            //melee formation
            //score is 'how many friendlies we save': kx.
            // k = enemy kills per man (when directed at us), x = our expected kills.
            //Because flanking formations will tend to be of high attack score,
            //the fact that k is kills directed at us (not at whoever they're fighting)
            //means that melee formations will be artificially reduced in score, ie flankers
            //will usually prefer to fight ranged formations. This is probably as it should be,
            //but just to balance it out we'll include the +1 combat bonus here in both our kills
            //AND their kills (even though they actually get a -1 bonus in the fight).
            // k = numranged / 2 + nummelee / 2 * chance[theirattack+1,ourdefence] / size
            // x = ourattacks * chance[ourattack+1,theirdefence] / 2
            
            score = (rangedattacks + meleeattacks * KillChance(attack + 1, ourdefencel) ) / (4 * size);
            score *= ourattacks * KillChance(ourattackl + 1, defence);
            
            //if this formation is flanking or in reserve, penalise it by factor of 2 (score is always positive).
            if(i != 0) score /= 2;
            
            //we have a target score;
            if(besttarget == -1 || score > bestscore) {
                secondbesttarget = besttarget;
                secondbestscore = bestscore;
                besttarget = i;
                bestscore = score;
            } else if(secondbesttarget == -1 || score > secondbestscore) {
                secondbesttarget = i;
                secondbestscore = score;
            }            
        }
    }
    //by now we hopefully have the two best targets.
    if(besttarget == -1) {
        //no best target!
        //this means we are not going to fight anyone!
        return -2;
    }
    engagements[formnum][besttarget] = ENGAGED_ENGAGED;
    if(enemy->formations[besttarget].Flank() != FORM_FLANKED ) enemy->engagements[besttarget][formnum] = ENGAGED_ENGAGED; //they don't necessarily engage us back if they are flank 2.
    else if(enemy->engagements[besttarget][formnum] < ENGAGED_CANATTACK) enemy->engagements[besttarget][formnum] = ENGAGED_CANATTACK; //BUT, they should be able to since we are fighting them
    if(secondbesttarget != -1) engagements[formnum][secondbesttarget] = ENGAGED_TRYATTACK;
    
    AString temp = *(pLeader->name) + "'s ";
    temp += AString(formations[formnum].GetNumMen());
    switch(formations[formnum].Type()) {
        case FORM_RIDE:
            temp += " flanked cavalry";
            break;            
        case FORM_FLY:
            temp += " flanked aerial cavalry";
            break;    
        default:
            temp += " flanked infantry";
            break;
    }
    temp += " engage the enemy";
    if(enemy->formations[besttarget].Behind()) temp += " ranged";
    else if(besttarget == 0) formations[formnum].SetTemporaryBonus(1);
    switch(enemy->formations[besttarget].Type()) {
        case FORM_RIDE:
            temp += " cavalry.";
            break;            
        case FORM_FLY:
            temp += " aerial cavalry";
            break;    
        default:
            temp += " infantry";
            break;
    }
    if(besttarget == 0) temp += " from behind";
    temp += ".";
    b->AddLine(temp);
    return besttarget;
}

void Army::FlankersEngage(Battle *b, Army *enemy)
{
    //search through flank 2 formations
    for(int i=NUMFORMS-1; i >= 2*(NUMFORMS/FORM_DEAD); i--) {
        //search backwards, so flying formations engage first
        GetFlankedTarget(b,enemy,i);
    }
}

void Army::ClearEmptyEngagements()
{
    for(int i=0; i<NUMFORMS; i++) {
        if(formations[i].GetNumMen()) continue;
        for(int j=0; j<NUMFORMS; j++) engagements[i][j] = ENGAGED_NONE;
    }
}

void Army::SetTemporaryBonuses(Army *enemy)
{
    for(int i=0; i<NUMFORMS; i++) formations[i].RemoveTemporaryBonus();
    
    int frontengaged = 0;
    for(int i=0; i<NUMFORMS/FORM_DEAD; i++)
        if(engagements[i][0] == ENGAGED_ENGAGED) frontengaged = 1;
    if(!frontengaged) return;
    if(enemy->formations[0].GetSize() < 1) return;
    //one of our frontline units has engaged the enemy frontline. Any of our flanked
    //units which have engaged the enemy frontline get a +1 temp bonus
    //TODO: Add in a check that these units haven't engaged others also - if so, they shouldn't get the bonus.
    for(int i=2*(NUMFORMS/FORM_DEAD); i<NUMFORMS; i++) if(engagements[i][0] == ENGAGED_ENGAGED) formations[i].SetTemporaryBonus(1);
}

void Army::DoExperience(int enemydead)
{
    //experience based on own losses
    float dead = (float) (count - NumAlive())/count;   //including illusions here - if illusions drop all around you, it's stressful!
    float exp = ( 300 * dead / (3 - dead) ) + 0.5;  // max value 150. 60xp for 50% losses
    //small battles -> less experience:
    if (count < 50) exp *= (float) (50+count)/100;   //scaled down for small battles.

    //small experience for victor based on enemies killed. Not yet dependent on who killed them. Not yet dependent on who dies (ie wolf or dragon count the same)
    float exp2 = (float) (30 * enemydead) / count;
    int divisor = 1;
    while(exp2 > 20) {
        exp += (float) 20/divisor;
        divisor++;   //20, then 10, then 7, then 5
        exp2 -= 20;
    }
    exp += (float) exp2/divisor;

	for(int x = 0; x < count; x++) {
		Soldier * s = GetSoldier(x);
		if(!(ItemDefs[s->race].type & IT_MAN)) continue;
		int exper = (int) exp;
		//if riding leader, give 2/3 of normal bonus to riding skill & normal skill

        if(s->unit->GetSkill(S_FRENZY)) s->unit->Experience(S_FRENZY,(2*exper/3),0);
        if(s->unit->GetSkill(S_BASE_BATTLETRAINING)) s->unit->Experience(S_BASE_BATTLETRAINING,(2*exper/3),0);

		if(s->riding != -1) {
		    //has riding skill.
		    if(!s->unit->IsNormal()) {
		        exper = (int) (2*exp/3);
		        s->unit->Experience(S_RIDING,exper,0);
		    } else s->unit->Experience(S_RIDING,exper,0);
		}

		if(s->weapon != -1) {
            WeaponType *pWep = FindWeapon(ItemDefs[s->weapon].abr);
            AString skname;
            skname = pWep->baseSkill;
            int skill = LookupSkill(&skname);
            skname = pWep->orSkill;
            int orskill = LookupSkill(&skname);
            if(orskill != -1) {
                if(s->unit->GetSkill(orskill) > s->unit->GetSkill(skill) ) skill = orskill;
            }
            if(skill == -1) skill = S_COMBAT;
            //we want to practise skill
            s->unit->Experience(skill,exper,0);
            
        } else if(!s->special) s->unit->Experience(S_COMBAT,exper,0);
        //practise special if we are a mage.
        if(s->special && s->unit->IsMage() && s->unit->combat != -1) s->unit->Experience(s->unit->combat,(int) (exp/3), 0);
	}
}

#ifdef DEBUG
void Army::WriteEngagements(Battle *b)
{
    for(int i=0; i<NUMFORMS; i++) {
        AString temp = "";
        for(int j=0; j<NUMFORMS; j++) temp += AString(engagements[i][j]) + " ";
        temp += AString("  ") + formations[i].Type();
        b->AddLine(temp);
    }
    b->AddLine("");
}
#endif

int RoundRandom(float input)
//rounds a positive float to one of the adjacent integers while preserving overall mean. eg 2.2 would have a 20% chance of rounding to 3, and 80% chance of rounding to 2.
{
    if(input < 0) return 0;
    int min = (int) input;
    input -= min;
    //input is now between 0 and 1.
    input = input*10000 - 0.5;
    int val = (int) input;
    if(getrandom(10000) < val) return (min+1);
    return min;
}

void Army::DoEthnicMoraleEffects(Battle *b)
{
    int racecount[RA_NA];
    for(int i=0; i<RA_NA; i++) racecount[i] = 0;
    int unity = 0;
    int unityhelps = 0;
    
    for(int i=0; i<count; i++) {
        Soldier *s = GetSoldier(i);
        if(s->special == SkillDefs[S_UNITY].special) unity += s->slevel*s->slevel;
        ManType *mt = FindRace(ItemDefs[s->race].abr);
        if(mt) racecount[mt->ethnicity]++;
        else racecount[RA_OTHER]++;
    }
#ifdef DEBUG2
cout << " !";
#endif
    unity *= 50; //unity 'heals' 50 men * lvl * lvl.

    float racepenalty[RA_NA];
    for(int i=0; i<RA_NA; i++) racepenalty[i] = 2*(count - racecount[i]);
    for(int i=0; i<RA_NA; i++) racepenalty[i] /= count;
    // penalties are now between 0 and 2, depending on racial fractions in the army.
    for(int i=0; i<RA_NA; i++) racecount[i] = 0;
    
    int max = unity;
#ifdef DEBUG2
cout << "@";
#endif
    for(int i=0; i<count; i++) {
        Soldier *s = GetSoldier(i);
        ManType *mt = FindRace(ItemDefs[s->race].abr);
        int ethnic;
        if(mt) ethnic = mt->ethnicity;
        else ethnic = RA_OTHER;
        int penalty = RoundRandom(racepenalty[ethnic]);
        if(penalty && unity > 0) {
            unity--;
            if(getrandom(2)) {
                unityhelps++;
                penalty = 0;
            }
        }
        if(penalty) racecount[ethnic]++;
        s->askill -= penalty;
        s->dskill[ATTACK_COMBAT] -= penalty;
    }
    int total = 0;
    for(int i=0; i<RA_NA; i++) total += racecount[i];
#ifdef DEBUG2
cout << "#";
#endif

    AString temp = AString("Unity calms ") + unityhelps + " of " + *pLeader->name + "'s soldiers.";
    if(unityhelps) b->AddLine(temp);
    
    if(max > unity) {
        for(int i=0; i<count; i++) {
            Soldier *s = GetSoldier(i);
            if(s->special == SkillDefs[S_UNITY].special) s->unit->Experience(S_UNITY,(10*(max-unity)+max/2)/max,0);
        }
    }       
    
    temp = AString(total) + " of " + *pLeader->name + "'s soldiers suffer a morale penalty due to the presence of soldiers from different ethnic groups.";
    if(total) b->AddLine(temp);
}

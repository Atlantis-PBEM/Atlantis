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
#include "battle1.h"
#include "gamedata.h"

void Soldier::SetupHealing()
{
    if (unit->type == U_MAGE) {
        healtype = unit->GetSkill(S_MAGICAL_HEALING);
		if (healtype > 6) healtype = 6;
		if (healtype > 0) {
			healing = HealDefs[healtype].num;
            healitem = -1;
            return;
        }
    }

	if(unit->items.GetNum(I_HEALPOTION)) {
		healtype = 1;
		unit->items.SetNum(I_HEALPOTION, unit->items.GetNum(I_HEALPOTION)-1);
		healing = Globals->HEALS_PER_MAN; // Previously 10, too powerful.
		healitem = I_HEALPOTION;
	} else {
		healing = unit->GetSkill(S_HEALING) * Globals->HEALS_PER_MAN;
		if (healing) {
			healtype = 1;
			int herbs = unit->items.GetNum(I_HERBS);
			if (herbs < healing) healing = herbs;
			unit->items.SetNum(I_HERBS,herbs - healing);
			healitem = I_HERBS;
		}
    }
}

int Army::IsSpecialTarget(char *special) const
{
	// Search All Formations
	for(int i=0; i<NUMFORMS; i++) {
        for(int j=0; j<formations[i].GetNumMen(); j++) {
            if (formations[i].CheckSpecialTarget(special, j)) return 1;
        }
    }
    return 0;
}

int Formation::CheckSpecialTarget(char *special, int soldiernum) const
{
	SpecialType *spd = FindSpecial(special);
	int i;
	int match = 0;

	if(spd->targflags & SpecialType::HIT_BUILDINGIF) {
		match = 0;
		if(!pSoldiers[soldiernum]->building) return 0;
		for(i = 0; i < 3; i++) {
			if (pSoldiers[soldiernum]->building &&
					(spd->buildings[i] == pSoldiers[soldiernum]->building)) match = 1;
		}
		if(!match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_BUILDINGEXCEPT) {
		match = 0;
		if(!pSoldiers[soldiernum]->building) return 0;
		for(i = 0; i < 3; i++) {
			if (pSoldiers[soldiernum]->building &&
					(spd->buildings[i] == pSoldiers[soldiernum]->building)) match = 1;
		}
		if(match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_SOLDIERIF) {
		match = 0;
		if (pSoldiers[soldiernum]->race == -1) return 0;
		for(i = 0; i < 7; i++) {
			if(pSoldiers[soldiernum]->race == spd->targets[i]) match = 1;
		}
		if(!match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_SOLDIEREXCEPT) {
		match = 0;
		if (pSoldiers[soldiernum]->race == -1) return 0;
		for(i = 0; i < 7; i++) {
			if(pSoldiers[soldiernum]->race == spd->targets[i]) match = 1;
		}
		if(match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_EFFECTIF) {
		match = 0;
		for(i = 0; i < 3; i++) {
			if(pSoldiers[soldiernum]->HasEffect(spd->effects[i])) match = 1;
		}
		if(!match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_EFFECTEXCEPT) {
		for(i = 0; i < 3; i++) {
			if(pSoldiers[soldiernum]->HasEffect(spd->effects[i])) return 0;
		}
	}

	if(spd->targflags & SpecialType::HIT_MOUNTIF) {
		match = 0;
		if (pSoldiers[soldiernum]->riding == -1) return 0;
		for(i = 0; i < 7; i++) {
			if(pSoldiers[soldiernum]->riding == spd->targets[i]) match = 1;
		}
		if(!match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_MOUNTEXCEPT) {
		match = 0;
		if (pSoldiers[soldiernum]->riding == -1) return 0;
		for(i = 0; i < 7; i++) {
			if(pSoldiers[soldiernum]->riding == spd->targets[i]) match = 1;
		}
		if(match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_ILLUSION) {
		// All illusions are of type monster, so lets make sure we get it
		// right.  If we ever have other types of illusions, we can change
		// this.
		if(!(ItemDefs[pSoldiers[soldiernum]->race].type & IT_MONSTER))
			return 0;
		if(!(ItemDefs[pSoldiers[soldiernum]->race].type & IT_ILLUSION))
			return 0;
	}
	
	if(spd->targflags & SpecialType::HIT_NOILLUSION) {
		if(ItemDefs[pSoldiers[soldiernum]->race].type & IT_ILLUSION) return 0;
	}
	
	if(spd->targflags & SpecialType::HIT_NOMONSTER) {
		if(ItemDefs[pSoldiers[soldiernum]->race].type & IT_MONSTER)
			return 0;
	}
	
	if(spd->targflags & SpecialType::HIT_MONSTEREXCEPT) {
		if (!(ItemDefs[pSoldiers[soldiernum]->race].type & IT_MONSTER)) return 0; //only hits monsters
		for(i = 0; i < 7; i++) {
			if(pSoldiers[soldiernum]->race == spd->targets[i]) return 0; //a match - so can't hit
		}
	}
	
    return 1;
}

int Army::ShieldIsUseful(char *special) const
{
	SpecialType *shi;
	shi = FindSpecial(special);

	// Search All Formations
	for(int i=0; i<NUMFORMS; i++) {
        for(int j=0; j<formations[i].GetNumMen(); j++) {
            Soldier *pSold = formations[i].GetSoldier(j);
            if(pSold->attacks > 0 || (pSold->attacks != 0 && (round % ( -1 * pSold->attacks ) == 1)) ) {
    			for(int shtype = 0; shtype < 4; shtype++) {
    				if(shi->shield[shtype] == pSold->attacktype) return 1;
    			}
            }
            if(pSold->special) {
            //limitations: doesn't check the spell has a target, doesn't check the caster has energy.
            	SpecialType *spd = FindSpecial(pSold->special);
                if(spd->effectflags & SpecialType::FX_DAMAGE) {
                	for(i = 0; i < 4; i++) {
            			for(int shtype = 0; shtype < 4; shtype++) {
            				if(shi->shield[shtype] == spd->damage[i].type) return 1;
            			}
                    }
                }
            }
            
        }
    }
    return 0;
}

void Battle::UpdateShields(Army *a, Army *enemy)
{
	for (int i=0; i<a->NumAlive(); i++) {
		int shtype = -1;
		SpecialType *spd;

		if(a->GetSoldier(i)->special == NULL) continue;
		Soldier *pSold = a->GetSoldier(i);
		spd = FindSpecial(pSold->special);

		if(!(spd->effectflags & SpecialType::FX_SHIELD) &&
				!(spd->effectflags & SpecialType::FX_DEFBONUS)) continue;

        if(!enemy->ShieldIsUseful(pSold->special)) {
    		AddLine(*(pSold->unit->name) + " chooses not to cast a shield.");
            continue; //should shields be cast when not useful?
        }

    	//energy cost for mages using their combat spell only
    	if(!pSold->DoSpellCost(a->round, this)) continue;

		if(spd->effectflags & SpecialType::FX_SHIELD) {
			for(shtype = 0; shtype < 4; shtype++) {
				if(spd->shield[shtype] == -1) continue;
				Shield *sh = new Shield;
				sh->shieldtype = spd->shield[shtype];
				sh->shieldskill = pSold->slevel;
				if(Globals->ARCADIA_MAGIC) sh->shieldskill++; //+1 bonus to shield strength
				sh->pCaster = pSold;
				a->shields.Add(sh);
			}
		}

		if(spd->effectflags & SpecialType::FX_DEFBONUS && a->round == 1) {  //first round is round 1, not 0.
			for(shtype = 0; shtype < 4; shtype++) {
				if(spd->defs[shtype].type == -1) continue;
				int bonus = spd->defs[shtype].val;
				if(spd->effectflags & SpecialType::FX_USE_LEV)
					bonus *= pSold->slevel;
				pSold->dskill[spd->defs[shtype].type] += bonus;
			}
		}

		AddLine(*(pSold->unit->name) + " casts " +
				spd->shielddesc + ".");
	}
}

void Battle::DoBinding(Army *atts, Army *defs)
{
    int na = atts->NumAlive();
    //get max spell level in this army
    for(int i=0; i<na; i++) {
        Soldier *pSold = atts->GetSoldier(i);
	    if(pSold->special == NULL) continue;
	    if(pSold->special == SkillDefs[S_BINDING].special) {
	        if(pSold->slevel) defs->DoBindingAttack(pSold, this);
	    } else if(pSold->special == SkillDefs[S_DRAGON_TALK].special) {
	        if(pSold->slevel) defs->DoDragonBindingAttack(pSold, this, atts);
	    }
    }
}

void Army::DoBindingAttack(Soldier *pAtt, Battle *b)
//'this' army is the defending army
{
    int mages = 0;
    for(int i=0; i<NUMFORMS; i++) mages += formations[i].CountMages();
    if(!mages) return;
    int target = getrandom(mages);
    Soldier *defender = 0;
    
    for(int i=0; i<NUMFORMS; i++) {
        if(formations[i].CountMages() > target) defender = formations[i].GetMage(target);
        else target -= formations[i].CountMages();
    }
    if(!defender) return;
    
    if(!pAtt->DoSpellCost(round, b)) return;
    if(pAtt->exhausted) {
        pAtt->special = NULL;
        pAtt->slevel = 0;
        return; //binding is an unusual spell - cannot be cast when exhausted.
    }
    //defend & attack with better of binding skill & dragontalk skill.    
    int defenceskill = defender->unit->GetSkill(S_BINDING);
    if(defender->unit->GetSkill(S_DRAGON_TALK) > defender->unit->GetSkill(S_BINDING) ) defenceskill = defender->unit->GetSkill(S_DRAGON_TALK);
    if(pAtt->unit->GetSkill(S_DRAGON_TALK) > pAtt->slevel) pAtt->slevel = pAtt->unit->GetSkill(S_DRAGON_TALK); //the mage is not exhausted, so this is valid.
    int defence = defender->unit->MaxEnergy() * (3 + defenceskill/2);
    int attack = pAtt->slevel * pAtt->unit->MaxEnergy();

    if(getrandom(attack+defence) < attack) {
        defender->slevel = 0;
        defender->special = NULL;
        AString temp = *(pAtt->unit->name) + " binds " + *(defender->unit->name) + " from spellcasting.";
        b->AddLine(temp);
    }    
}

void Army::DoDragonBindingAttack(Soldier *pAtt, Battle *b, Army *atts)
//'this' army is the defending army
{
    int dragons = 0;
    int na = NumAlive();
    Soldier *pDragon = 0;
    Soldier *pSold;
    for(int i=0; i<na; i++) {
        pSold = GetSoldier(i);
        if(pSold->candragontalk && pSold->race == I_DRAGON) { //each dragon can only have one attempt to convert or bind them.
            if(!getrandom(++dragons)) pDragon = pSold;        
        }    
    }

    //if no dragons, do a regular binding attack.
    if(!pDragon) {
        //set the soldier's special to binding here to allow different spell costs
        SkillType *pST = &SkillDefs[S_BINDING];
        pAtt->special = pST->special;
        pAtt->slevel = pAtt->unit->GetSkill(S_BINDING);
        DoBindingAttack(pAtt, b);
        return;
    }

    #ifdef DEBUG
    Awrite("Doing dragon binding");
    #endif

    //defender is our randomly picked dragon.
    if(!pAtt->DoSpellCost(round, b)) return;
    if(pAtt->exhausted) {
        pAtt->special = NULL;
        pAtt->slevel = 0;
        return; //dragonbinding is an unusual spell - cannot be cast when exhausted.
    }

    pDragon->candragontalk = 0; //cannot dragontalk with this dragon again.

    //some stuff to determine result.
    //chance to:
    //(a) tie down the dragon's special
    //(b) convince the dragon to leave the fight
    //(c) convince the dragon to join us
    //set 
    
    int result = 0;
    //result 0: fails
    //result 1: dragon's special removed
    //result 2: dragon leaves
    //result 3: dragon joins us
    
    int attack = (pAtt->slevel + 2) * pAtt->unit->MaxEnergy();
    //typical scores at lvl 2 are 4*80 (this is a mysticism mage) ~ 320. Max expected score
    //would be 8*150 ~ 1200.
    if( getrandom(300+attack) < attack ) {
        result = 1;
        attack = pAtt->slevel * pAtt->unit->MaxEnergy();
        //typical scores at lvl 3 are 3*100
        if(getrandom(300+attack) < attack ) {
            result = 2;
            attack = (pAtt->slevel - 3) * pAtt->unit->MaxEnergy();
            if(attack>0 && (getrandom(300+attack) < attack) ) result = 3;
        }
    }

    AString temp = "";
    Soldier * pDragon2 = 0;
    int dragonnum;
    switch(result) {
        case 0:
            if(pDragon->unit->GetSoldiers() == 1) temp = "A dragon from ";
            if(getrandom(2)) b->AddLine(temp + *(pDragon->unit->name) + " ignores the pleas of " + *(pAtt->unit->name) );
            else b->AddLine(temp + *(pDragon->unit->name) + " listens to the words of " + *(pAtt->unit->name) + " but is not swayed to leave this battle.");
            return;
        case 1:
            if(pDragon->unit->GetSoldiers() == 1) temp = "a dragon from ";
            b->AddLine(*(pAtt->unit->name) + " cannot convince " + temp + *(pDragon->unit->name) + " to leave this battle, but manages to bind him from spellcasting.");
            pDragon->slevel = 0;
            pDragon->special = NULL;
            return;
        case 3:
            //if joining us:
            //remove a dragon
            pDragon->unit->items.SetNum(I_DRAGON, pDragon->unit->items.GetNum(I_DRAGON) - 1);
            //add a dragon
            pAtt->unit->items.SetNum(I_DRAGON, pAtt->unit->items.GetNum(I_DRAGON) + 1);
            
            if(pDragon->unit->GetSoldiers() == 1) b->AddLine( *(pDragon->unit->name) + " agrees to quit this battle and join " + *(pAtt->unit->name) );
            else b->AddLine(AString("A dragon from ") + *(pDragon->unit->name) + " agrees to quit this battle and join " + *(pAtt->unit->name) );
            //fall through...
        case 2:
            //if leaving this fight:
            dragonnum = -1;
            for(int i=0; i<formations[pDragon->inform].GetNumMen(); i++) {
                const Soldier *temp = formations[pDragon->inform].GetSoldier(i);
                if (temp == pDragon) {
                    dragonnum = i;
                    break;
                }
            }
            if(dragonnum == -1) {
                #ifdef DEBUG
                Awrite("Could not find bound dragon in its formation!");
                system("pause");
                #endif
                return;
            }
            
            pDragon2 = formations[pDragon->inform].RemoveSoldier(dragonnum);
            #ifdef DEBUG
            if(pDragon != pDragon2) {
                Awrite(AString("Removed dragon is not the dragon it should be. It is of race ") + pDragon2->race);
                system("pause");
            }
            #endif
            if(result == 2) {
                if(pDragon->unit->GetSoldiers() == 1) b->AddLine( *(pAtt->unit->name) + " convinces " + *(pDragon->unit->name) + " to quit this battle." );
                else b->AddLine( *(pAtt->unit->name) + " convinces a dragon from " + *(pDragon->unit->name) + " to quit this battle." );
            }
            count--;
            delete pDragon2;
            return;
        default:
            return;
    }
}

void Battle::UpdateRoundSpells(Army *armya, Army *armyb)
{

    if(getrandom(2)) {
        DoBinding(armya,armyb);
        DoBinding(armyb,armya);
    } else {
        DoBinding(armyb,armya);
        DoBinding(armya,armyb);
    }
    
    int darkness = GetRoundSpellLevel(armya, armyb, 2, S_DARKNESS, S_LIGHT);
    int foga = GetRoundSpellLevel(armya, armyb, 1, S_FOG, S_CLEAR_SKIES) + darkness;
    int fogb = GetRoundSpellLevel(armyb, armya, 1, S_FOG, S_CLEAR_SKIES) + darkness;
    int conceala = GetRoundSpellLevel(armya, armyb, 1, S_CONCEALMENT, S_DISPEL_ILLUSIONS);
    int concealb = GetRoundSpellLevel(armyb, armya, 1, S_CONCEALMENT, S_DISPEL_ILLUSIONS);

//Do taccontrol for army a, then army b.
    float taca = 100;
    while(fogb > 0) {
        fogb--;
        taca *= 0.7;
    }
    armya->taccontrol = (int) taca;

    float tacb = 100;
    while(foga > 0) {
        foga--;
        tacb *= 0.7;
    }
    armyb->taccontrol = (int) tacb;
    
    armya->concealment = conceala;
    armyb->concealment = concealb;
    
    //Do 'bonus' penalty for darkness.
    for( int i=0; i<NUMFORMS; i++) {
        armya->formations[i].bonus -= darkness;
        armyb->formations[i].bonus -= darkness;
    }
}

int Battle::GetRoundSpellLevel(Army *armya, Army *armyb, int type, int spell, int antispell)
//type can be 1 or 2 - 1 looks only at this army, 2 looks through both armies.
{
    Soldier *caster = 0;
    int spelllevel = 0;
    Soldier *anticaster = 0;
    int antispelllevel = 0;
    
    int na = armya->NumAlive();
    //get max spell level in this army
    for(int i=0; i<na; i++) {
        Soldier *pSold = armya->GetSoldier(i);
	    if(pSold->special == NULL) continue;
	    if(pSold->special == SkillDefs[spell].special) {
	        pSold->DoSpellCheck(armya->round, this); //this checks if the mage should become exhausted
	        if(pSold->slevel > spelllevel) {
                spelllevel = pSold->slevel;
                caster = pSold;
            }
	    }
	    if((type == 2) && pSold->special == SkillDefs[antispell].special) {
	        pSold->DoSpellCheck(armya->round, this); //this checks if the mage should become exhausted
	        if(pSold->slevel > antispelllevel) {
                antispelllevel = pSold->slevel;
                anticaster = pSold;
            }
	    }
    }

    na = armyb->NumAlive();
    for(int i=0; i<na; i++) {
        Soldier *pSold = armyb->GetSoldier(i);
	    if(pSold->special == NULL) continue;

	    if((type == 2) && pSold->special == SkillDefs[spell].special) {
	    	pSold->DoSpellCheck(armyb->round, this); //this checks if the mage should become exhausted
	        if(pSold->slevel > spelllevel) {
                spelllevel = pSold->slevel;
                caster = pSold;
            }
	    }
	    if(pSold->special == SkillDefs[antispell].special) {
	        pSold->DoSpellCheck(armyb->round, this); //this checks if the mage should become exhausted
	        if(pSold->slevel > antispelllevel) {
                antispelllevel = pSold->slevel;
                anticaster = pSold;
            }
	    }
    }

    if(spelllevel == 0) return 0;

    if( caster->DoSpellCost(armya->round, this) == 0) return 0; //round should be same for both armies, so this works for darkness also.

    AString temp;
    temp = *(caster->unit->name) + " casts ";
  
    SpecialType *spd = FindSpecial(SkillDefs[spell].special);
    temp += spd->spelldesc;
    
    if(antispelllevel && anticaster->DoSpellCost(armyb->round, this)) {
    //anticaster will get used twice - dispel illusions, light and clear skies all have another purpose. Well, tough - they get double experience also!
        if(spelllevel > antispelllevel) {
            temp += ", but it's effectiveness is reduced by ";
        } else temp += ", but the spell is rendered ineffective by ";
        temp += *(anticaster->unit->name);
    } else temp += ".";
    AddLine(temp);

    if(spell > antispell) return (spelllevel - antispelllevel);
    else return 0;
}

void Battle::DoSpecialAttack(int round, Soldier *a, Army *attackers, Army *def)
{
	SpecialType *spd;
	int i, num, tot = -1;
	AString results[4];
	int dam = 0;

	if(a->special == NULL) return;
	spd = FindSpecial(a->special);

	if(!(spd->effectflags & SpecialType::FX_DAMAGE)) return;

	int hitself = 0;
	if(spd->targflags & SpecialType::HIT_OWN_ARMY) hitself = 1;

	if(spd->targflags && ( (!hitself && (def->IsSpecialTarget(a->special) < 1)) || 
          (hitself && (attackers->IsSpecialTarget(a->special) < 1)) ) ) {
	    if(a->unit->type == U_MAGE) {
	        AddLine(a->name + " cannot find a spell target, and saves his energy.");
	    }
        return; //no targets to hit.
    }

	//energy cost for mages using their combat spell only (ie not for monsters, battle items etc)
	int cancast = a->DoSpellCost(round, this); //this also includes fizzle chance.
	if(!cancast) return;
	if(cancast == 2) {
	    hitself = (hitself+1)%2; //swop army to hit.
	}

	for(i = 0; i < 4; i++) {
		if(spd->damage[i].type == -1) continue;
		int times = spd->damage[i].value;
		if(spd->effectflags & SpecialType::FX_USE_LEV)
			times *= a->slevel;
		int realtimes = spd->damage[i].minnum + getrandom(times) +
			getrandom(times);
        if(!hitself) num = def->DoAnAttack(a->special, realtimes, a->race,
				spd->damage[i].type, a->slevel,
				spd->damage[i].flags, spd->damage[i].dclass,
				spd->damage[i].effect, 0, attackers, a->inform, this);
		else num = attackers->DoAnAttack(a->special, realtimes, a->race,        //hitting own army
				spd->damage[i].type, a->slevel,
				spd->damage[i].flags, spd->damage[i].dclass,
				spd->damage[i].effect, 0, attackers, a->inform, this); //'attackers' are still attackers; this is used to trigger random target selection rather than formation-specific.
		if(spd->effectflags & SpecialType::FX_DONT_COMBINE && num != -1) {
			if(spd->damage[i].effect == NULL) {
				results[dam] = AString("killing ") + num;
				dam++;
			} else {
				results[dam] = AString(spd->spelldesc2) + num;
			}
		}
		if(num != -1) {
			if(tot == -1) tot = num;
			else tot += num;
		}
	}
	if(tot == -1) {
		AddLine(a->name + " " + spd->spelldesc + ", but it is deflected.");
	} else {
		if(spd->effectflags & SpecialType::FX_DONT_COMBINE) {
			AString temp = a->name + " " + spd->spelldesc;
			for(i = 0; i < dam; i++) {
				if(i) temp += ", ";
				if(i == dam-1) temp += " and ";
				temp += results[dam];
			}
			temp += AString(spd->spelltarget) + ".";
			AddLine(temp);
		} else {
			AddLine(a->name + " " + spd->spelldesc + ", " + spd->spelldesc2 +
					tot + spd->spelltarget + ".");
		}
	}
}


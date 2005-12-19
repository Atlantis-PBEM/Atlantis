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
#include "army1.h"
#include "formation1.h"
#include "gameio.h"
#include "gamedata.h"


Formation::Formation()
{
	reserve = 0;
	flank = 0;
	concealed = 0;
	bonus = 0;
	tempbonus = 0;
	nummen = 0;
    size = 0;
	lastkilled = 0;
    type = FORM_FOOT;
    behind = 0;	
    canattack = 0;
}

Formation::~Formation()
{
    #ifdef DEBUG
    Awrite("Formation Destructor");
    #endif

    delete [] pSoldiers;

    #ifdef DEBUG
    Awrite("End of Formation Destructor");
    #endif
}

void Formation::SetupFormation(int formnum, int count)
{
    type = (formnum%6)/2;   //0=foot, 1=ride, 2=fly
    behind = formnum%2;     //0=front, 1=behind
    flank = formnum/6;              //0=normal, 1 = trying to flank, 2 = has flanked
    number = formnum;
    if(flank == FORM_FRONT && type != FORM_FOOT) reserve = 1;
    
    pSoldiers = new SoldierPtr[count];
    for(int i=0; i<count; i++) {
        pSoldiers[i] = 0;
    }
}

void Formation::SetTemporaryBonus(int b)
{
    tempbonus = b;
    bonus += b;
}

void Formation::RemoveTemporaryBonus()
{
    bonus -= tempbonus;
    tempbonus = 0;
}

Soldier * Formation::GetSoldier(int soldiernum) const
//Get a pointer to a particular soldier
{

    if(soldiernum > nummen-1 || soldiernum < 0) {
        Awrite(AString("Illegal Get Soldier Access ") + soldiernum + "/" + (nummen-1) );
        return 0;
    }

    return pSoldiers[soldiernum];
}

Soldier * Formation::GetAttacker(int soldiernum)
//Get a pointer to a particular soldier, and move that soldier out of the canattack section of the soldierarray
//The soldier number should be from 0 to canattack-1.
{
    if(soldiernum > canattack-1 || soldiernum < 0) {
        Awrite("Illegal Get Attacker Access");
        return 0;
    }
    Soldier *temp = pSoldiers[soldiernum];
    //swop temp and the last soldier who can attack
    pSoldiers[soldiernum] = pSoldiers[canattack-1];
    pSoldiers[canattack-1] = temp;
    canattack--;
    return temp;
}

int Formation::GetNonIllusionSize() const
{
    int hits = 0;
    for(int i=0; i<nummen; i++) {
        if(pSoldiers[i]->illusion) continue;
        hits += pSoldiers[i]->maxhits;
    }
    return hits;
}

int Formation::CountMages() const
{
    int mages = 0;
    for(int i=0; i<nummen; i++) {
        if(!pSoldiers[i]->slevel) continue;
        if(pSoldiers[i]->unit->type != U_MAGE && pSoldiers[i]->unit->type != U_GUARDMAGE) continue;
        if(!(ItemDefs[pSoldiers[i]->race].type & IT_MAN) ) continue;
        mages++;
    }
    return mages;
}

Soldier * Formation::GetMage(int mage)
{
    for(int i=0; i<nummen; i++) {
        if(!pSoldiers[i]->slevel) continue;
        if(pSoldiers[i]->unit->type != U_MAGE && pSoldiers[i]->unit->type != U_GUARDMAGE) continue;
        if(!(ItemDefs[pSoldiers[i]->race].type & IT_MAN) ) continue;
        if(!mage) return pSoldiers[i];
        mage--;
    }
    return 0;
}



void Formation::AddSoldier(Soldier * pNewsoldier)
//Add a soldier to a formation
//This routine adjust nummen, but does not adjust canattack - ie if it
//is called during a combat round the soldier should not be able
//to attack (eg is dead!).
{
    #ifdef DEBUG
    if(nummen < 0) {
        Awrite("Number of men out of bounds");
        system("pause");
    }
    #endif

    size += pNewsoldier->maxhits;
    pSoldiers[nummen++] = pNewsoldier;
    pNewsoldier->inform = number;
}

void Formation::AddCanAttackSoldier(Soldier * pNewsoldier)
//Add a soldier to the front (canattack part) of a formation
//This routine adjust nummen, and canattack - it should only be
//called for soldiers who can attack.
{
    #ifdef DEBUG
    if(nummen < 0) {
        Awrite("Number of men out of bounds");
        system("pause");
    }
    #endif

    size += pNewsoldier->maxhits;
    pSoldiers[nummen++] = pSoldiers[canattack];
    pSoldiers[canattack++] = pNewsoldier;
    pNewsoldier->inform = number;
}


Soldier * Formation::RemoveSoldier(int soldiernum)
//Remove a soldier from a formation
//soldiernum should be a value from 0 to nummen-1.
//This routine adjusts nummen and canattack, so can be called
//anytime, but is slightly quicker if canattack == 0.
{
    if(soldiernum >= nummen || soldiernum < 0) {
        Awrite("Illegal Remove Soldier Access");
        return 0;
    }    

    //temp is the soldier we want to remove
    Soldier * temp = pSoldiers[soldiernum];

    if(soldiernum < canattack) {
    //move the last soldier who can attack into temp's old position.
        pSoldiers[soldiernum] = pSoldiers[canattack-1];
    //move the last soldier into the newly vacated spot
        pSoldiers[canattack-1] = pSoldiers[nummen-1];
    //decrease canattack, marking the last filled spot as cannot attack.
        canattack--;
    } else {
    //move the last soldier into temp's old position.
        pSoldiers[soldiernum] = pSoldiers[nummen-1];
    }

    //empty old last soldier's position for safety.
    pSoldiers[nummen-1] = 0;

    //decrease number of men and return.
    nummen--;
    size -= temp->maxhits;
    return temp;
}

void Formation::TransferSoldier(int soldiernum, Formation * pToForm)
//Transfer a soldier from this formation to another. This preserves the
//canattack status of the soldier.
{
    //Remove soldier from this formation
    Soldier * emmigrant = RemoveSoldier(soldiernum);
    //Add soldier to new formation
    //Check if the soldier canattack or not.
    if(soldiernum <= canattack) pToForm->AddCanAttackSoldier(emmigrant); 
    //remember than canattack just got decreased by 1, hence the '<=' above.
    else pToForm->AddSoldier(emmigrant);
}

void Formation::MoveSoldiers(Formation *pToForm)
//Move all soldiers to target formation. This preserves the canattack
//status of the soldiers.
{
    while(nummen) {
        TransferSoldier(0, pToForm);
    }
}

int Formation::MoveSoldiers(Formation *pToForm, int sizetomove, int condition)
//If soldiers are >1 hit, size moved may not be exactly 'sizetomove'. In this
//situation, 'condition' gets tested:
//condition 0: moves equal or less than wanted
//condition 1: moves equal or more than wanted
//return number of men (not size!) moved.
//if sizetomove is negative, will return gracefully.
{
    int nummoved = 0;
    while(sizetomove > 0 && GetNumMen()) {
        int moving = getrandom(nummen);
        if(condition || pSoldiers[moving]->maxhits <= sizetomove) {
            sizetomove -= pSoldiers[moving]->maxhits;
            TransferSoldier(moving, pToForm);
            nummoved++;
        } else sizetomove = 0;
    }

    return nummoved;
}

void Formation::Kill(int soldiernum, Army * itsarmy, int numhits)
//This adjusts nummen and canattack, so can be called during
//a combat round.
{
    if(soldiernum >= nummen || soldiernum < 0) {
        Awrite("Illegal Soldier Access");
        return;
    }
    
    if(numhits < 0) return;

    if (pSoldiers[soldiernum]->amuletofi) return;
    
    if(numhits == 1 || pSoldiers[soldiernum]->hits == 1) {
        pSoldiers[soldiernum]->damage++;
        pSoldiers[soldiernum]->hits--;
        //return if soldier is alive
        if(pSoldiers[soldiernum]->hits) return; //return if still alive
    } else {
        //doing more than one hit
        if(numhits >= pSoldiers[soldiernum]->hits) {
            pSoldiers[soldiernum]->damage += pSoldiers[soldiernum]->hits;
            pSoldiers[soldiernum]->hits = 0;
        } else {
            pSoldiers[soldiernum]->damage += numhits;
            pSoldiers[soldiernum]->hits -= numhits;
            return; //still alive
        }
    }

    //soldier is dead!
    pSoldiers[soldiernum]->isdead = 1;
    pSoldiers[soldiernum]->unit->losses++;

    //transfer soldier to the "dead" formation.    
    TransferSoldier(soldiernum, &itsarmy->formations[NUMFORMS]);
}

void Formation::Reset()
//called at the start of a normal round after formation phase
{
    canattack = nummen;
    for(int i=0; i<nummen; i++) {
        pSoldiers[i]->ClearOneTimeEffects();
    }
}

void Formation::ResetHeal() const
{
    for(int i=0; i<nummen; i++) {
        if(!pSoldiers[i]->illusion) pSoldiers[i]->canbehealed = 1;
    }
}

void Formation::Regenerate(Battle * b) const
{
    for(int i=0; i<nummen; i++) {
        Soldier *s = pSoldiers[i];
        int diff = s->maxhits - s->hits;
		if (diff > 0) {
			AString aName = s->name;

			if (s->damage != 0) {
				b->AddLine(aName + " takes " + s->damage +
						" hits bringing it to " + s->hits + "/" +
						s->maxhits + ".");
				s->damage = 0;
			} else {
				b->AddLine(aName + " takes no hits leaving it at " +
						s->hits + "/" + s->maxhits + ".");
			}
			if (s->regen) {
				int regen = s->regen;
				if (regen > diff) regen = diff;
				s->hits += regen;
				b->AddLine(aName + " regenerates " + regen +
						" hits bringing it to " + s->hits + "/" +
						s->maxhits + ".");
			}
		}
    }        
}

int Formation::Engaged(Army *itsarmy) const
{
    if(GetNumMen() == 0) return 0;
    for(int i=0; i<NUMFORMS; i++) {
        if(itsarmy->engagements[number][i] == ENGAGED_ENGAGED) return 1;
    }
    return 0;
}

void Formation::Sort(Army *itsarmy, Battle *b, int regtype)
{
    int i=0;
    int missort=0;
//    int terrainlimited = 0;
//    int tacerror = 0; //1 = one downgrade, 2 = two downgrades

    while(i<nummen) {
        if(pSoldiers[i]->defaultform == 0) i++;
        else if( getrandom(100) < itsarmy->taccontrol) {
        //Sorted correctly, move to its default formation.
            TransferSoldier(i,&itsarmy->formations[pSoldiers[i]->defaultform]);
        } else {
            int form = pSoldiers[i]->defaultform;
            missort++;
            int fallthru = 1;
            while(fallthru && (form>0) ) {
            //does it fall through again?
                if(getrandom(100) < itsarmy->taccontrol) fallthru = 0;
            //if behind, 50% chance to fall either way.
                if(form%2 && getrandom(2) ) form -= 1;
                else form -= 2;
            }
            if(form < 1) i++;
            else TransferSoldier(i,&itsarmy->formations[form]);
        }
    }
    
    if(itsarmy->taccontrol == 100) b->AddLine(AString(*(itsarmy->pLeader->name)) + " has full control of his army. All soldiers are correctly assigned to their place.");
    else {
        AString temp;
        if(itsarmy->taccontrol > 70) temp = "slightly.";
        else if(itsarmy->taccontrol > 30) temp = "considerably.";
        b->AddLine(AString(*(itsarmy->pLeader->name)) + " has " + itsarmy->taccontrol + "% control his army. " + missort + " soldiers are misassigned, and maneuvering is impaired " + temp);
    }
}

int Formation::NumMeleeAttacks() const
{
    int attacks = 0;
    for(int i=0; i<nummen; i++) {
        if(pSoldiers[i]->attacktype == ATTACK_COMBAT) {
            if(pSoldiers[i]->attacks>0) attacks += pSoldiers[i]->attacks;
            if(pSoldiers[i]->attacks<0) if(!getrandom(-pSoldiers[i]->attacks)) attacks++;
        }
    }
    return attacks;
}

int Formation::NumRangedAttacks() const
//this is only used for selecting best target. Could maybe include tactics skill etc. here.
{
    int attacks = 0;
    int heroes = 0;
    for(int i=0; i<nummen; i++) {
        if(pSoldiers[i]->attacktype == ATTACK_RANGED) {
            if(pSoldiers[i]->attacks>0) attacks += pSoldiers[i]->attacks;
            if(pSoldiers[i]->attacks<0 && !(getrandom(-pSoldiers[i]->attacks))) attacks++;
        }
        if(pSoldiers[i]->special) {
            SpecialType *sp = FindSpecial(pSoldiers[i]->special);
            int damage = 0;
            for(int j = 0; j < 4; j++) {
		        if(sp->damage[j].type == -1) continue;
		        int times = sp->damage[j].value;
				if(sp->effectflags & SpecialType::FX_USE_LEV) times *= pSoldiers[j]->slevel;
				if(sp->damage[j].effect == NULL) times *= 2; //ie if it is a killing spell *2, else *1 for fear/storm.
				attacks += times;
				damage = 1;
            }
            if(!damage) attacks += pSoldiers[i]->slevel * 20; //some bonus for other spells,
                                                          //eg shields, concealment, etc.
        }
        if(pSoldiers[i]->unit->type == U_LEADER) {
            attacks += 1;          //increased value for leaders
        }
        if(pSoldiers[i]->unit->type == U_MAGE) {
			forlist(&pSoldiers[i]->unit->skills) {
			    Skill *sk = (Skill *)elem;
                attacks += (sk->days + sk->experience) / 30;     //increased value for heroes
            }
            heroes++;
        }
    }
    
    //each hero gives an additional multiplicative bonus to the values
    attacks = (attacks * (6+heroes))/6;
    return attacks;
}

float Formation::MeleeAttackLevel() const
{
    int level = 0;
    int totalattacks = 0;
    for(int i=0; i<nummen; i++) {
        if(pSoldiers[i]->attacktype == ATTACK_COMBAT) {
            int attacks = 0;
            if(pSoldiers[i]->attacks>0) attacks = pSoldiers[i]->attacks;
            if(pSoldiers[i]->attacks<0 && !getrandom(-pSoldiers[i]->attacks)) attacks = 1;
            totalattacks += attacks;
            level += pSoldiers[i]->askill * attacks;
        }
    }
    float averagelevel = (float) level / totalattacks;
    return averagelevel;
}

float Formation::MeleeDefenceLevel() const
{
    int level = 0;
    for(int i=0; i<nummen; i++) {
        level += pSoldiers[i]->dskill[ATTACK_COMBAT];
    }
    float averagelevel = (float) level / nummen;
    return averagelevel;
}

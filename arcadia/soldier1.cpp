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
#include "formation1.h"
#include "gameio.h"
#include "gamedata.h"

Soldier::Soldier(Unit * u,Object * o,int regtype,int r,int ass)
{
    inform = 0;
    defaultform = 0;
    
	race = r;
	unit = u;
	building = 0;

	healing = 0;
	healtype = 0;
	healitem = -1;
	canbehealed = 1;
	canberesurrected = 1;
	regen = 0;
	isdead = 0;
	exhausted = 0;
	illusion = 0;
	candragontalk = 0;
	if(r == I_DRAGON) candragontalk = 1;
	
	for(int i=0; i<NUMEFFECTS; i++) {
	    effects[i] = 0;	
	}
	
	armor = -1;
	riding = -1;
	weapon = -1;

	attacks = 1;
	attacktype = ATTACK_COMBAT;

	special = NULL;
	slevel = 0;

	int def = - (u->crossbridge); //crossing bridges/walls mod - penalty of 1 usually. Set to zero for others. (Hexside Terrain).
	if(TerrainDefs[regtype].similar_type == R_OCEAN) def += ObjectDefs[o->type].oceanbonus;


	askill = def;

	dskill[ATTACK_COMBAT] = def;
	dskill[ATTACK_ENERGY] = def;
	dskill[ATTACK_SPIRIT] = def;
	dskill[ATTACK_WEATHER] = def;
	dskill[ATTACK_RIDING] = def;
	dskill[ATTACK_RANGED] = def;
	for(int i=0; i<NUM_ATTACK_TYPES; i++)
		protection[i] = 0;
	damage = 0;
	hits = 1;
	maxhits = 1;
	amuletofi = 0;
	battleItems = 0;
	
	int terrainflags = TerrainDefs[regtype].flags;
 	
 	
#ifdef DEBUG2
cout << "@";
#endif
	/* BS 030825 Multiple Hit Men */
    if (ItemDefs[r].type & IT_MAN) {
        ManType *mt = FindRace(ItemDefs[r].abr);    
        hits = mt->hits;
        maxhits = mt->hits;
        if(unit->GetSkill(S_TOUGHNESS)) {
            int level = unit->GetSkill(S_TOUGHNESS);
            int gain = 0;
            while(level > 0) {                      //2,4,7,11,16,22
                gain += level--;
            }
            hits += gain;
            maxhits += gain;
        }
    }
    
    if (ItemDefs[r].type & IT_ILLUSION) {
        illusion = 1;
        canbehealed = 0;
    }
    
	/* Building bonus */
	if (o->capacity) {
		building = o->type;
		//should the runes spell be a base or a bonus? Set to a bonus here
		for (int i=0; i<NUM_ATTACK_TYPES; i++) {
			if(Globals->ADVANCED_FORTS) {
				protection[i] += ObjectDefs[o->type].defenceArray[i];
			} else
				dskill[i] += ObjectDefs[o->type].defenceArray[i];
		}
		if (o->runes) {
			dskill[ATTACK_ENERGY] += o->runes;
			dskill[ATTACK_SPIRIT] += o->runes;
			dskill[ATTACK_WEATHER] += o->runes;
		}
		o->capacity--;
	}
	/* Is this a monster? */
	if (ItemDefs[r].type & IT_MONSTER) {
		MonType *mp = FindMonster(ItemDefs[r].abr,
				(ItemDefs[r].type & IT_ILLUSION));
		int illurace = r;
		if(ItemDefs[r].type & IT_ILLUSION) {
		    AString *temp = new AString;
		    *temp = ItemDefs[r].abr;
		    illurace = LookupItem(temp);
		    if(illurace == -1) illurace = r;
		    delete temp;
		}
		
		if(u->type == U_WMON)
			name = AString(mp->name) + " in " + *(unit->name);
		else
			name = AString(mp->name) + " controlled by " + *(unit->name);
		askill += mp->attackLevel;
		dskill[ATTACK_COMBAT] += mp->defense[ATTACK_COMBAT];
		dskill[ATTACK_ENERGY] += mp->defense[ATTACK_ENERGY];
		dskill[ATTACK_SPIRIT] += mp->defense[ATTACK_SPIRIT];
		dskill[ATTACK_WEATHER] += mp->defense[ATTACK_WEATHER];
		dskill[ATTACK_RIDING] += mp->defense[ATTACK_RIDING];
		dskill[ATTACK_RANGED] += mp->defense[ATTACK_RANGED];
		
		if(ItemDefs[illurace].fly && (terrainflags & TerrainType::FLYINGMOUNTS))
		    defaultform = 4;
		else if(ItemDefs[illurace].ride && (terrainflags & TerrainType::RIDINGMOUNTS))
		    defaultform = 2;
        else defaultform = 0;
        
		damage = 0;
		hits = mp->hits;
		if (hits < 1) hits = 1;       //even illusions get 1 hit. But why list them with 0 in gamedata?
		maxhits = hits;
		attacks = mp->numAttacks;
		if(attacks < 0) attacks = 0; //allow zero attacks for illusions.
		special = mp->special;
		slevel = mp->specialLevel;
		if (Globals->MONSTER_BATTLE_REGEN) {
			regen = mp->regen;
			if (regen < 0) regen = 0;
		}		
		return;
	}

	name = *(unit->name);

	SetupHealing(); //in specials.cpp
	SetupSpell();
	SetupCombatItems();
 	
#ifdef DEBUG2
cout << "#";
#endif
	// Set up armor
	AString abbr;
	int i, item, armorType;
	for(i = 0; i < MAX_READY; i++) {
		// Check preferred armor first.
		item = unit->readyArmor[i];
		if(item == -1) break;
		abbr = ItemDefs[item].abr;
		item = unit->GetArmor(abbr, ass);
		if(item != -1) {
			armor = item;
			break;
		}
	}
	if(armor == -1) {
		for(armorType = 1; armorType < NUMARMORS; armorType++) {
			abbr = ArmorDefs[armorType].abbr;
			item = unit->GetArmor(abbr, ass);
			if(item != -1) {
				armor = item;
				break;
			}
		}
	}
	
	//need to check if this works properly!
	ArmorType *pa = FindArmor(ItemDefs[armor].abr);
	if(pa && pa->flags & ArmorType::ONLYONEHIT) {
	    hits = 1;
	    maxhits = 1;
    }

	//
	// Check if this unit is mounted
	//
	int canFly = (terrainflags & TerrainType::FLYINGMOUNTS);
	int canRide = (terrainflags & TerrainType::RIDINGMOUNTS);
	ridingbonus = 0;
	int formtype = 0;
	if(canFly || canRide) {
		//
		// Mounts of some type _are_ allowed in this region
		//
		int mountType;
		for(mountType = 1; mountType < NUMMOUNTS; mountType++) {
			abbr = MountDefs[mountType].abbr;
			item = unit->GetMount(abbr, canFly, canRide, ridingbonus, formtype);
			if(item == -1) continue;
			// Defer adding the combat bonus until we know if the weapon
			// allows it.  The defense bonus for riding can be added now
			// however.
			/* What about buildings? Riding in buildings?! BS */
			if(formtype == 4 && (terrainflags & TerrainType::FLYINGLIMITED)) {
			    ridingbonus = 0;
			}
			if(formtype == 2 && (terrainflags & TerrainType::RIDINGLIMITED)) {
			    ridingbonus = 0;
			}
			dskill[ATTACK_RIDING] += ridingbonus;
			riding = item;		
			break;
		}
	}

	// Assign correct formtype for soldier
    if(u->GetFlag(FLAG_FIGHTASFOOT)) formtype = 0;
    if(formtype>3 && u->GetFlag(FLAG_FIGHTASRIDE)) formtype = 2;

    if(u->GetFlag(FLAG_BEHIND)) formtype += 1;
    defaultform = formtype;

	//
	// Find the correct weapon for this soldier.
	//
	int weaponType;
	int attackBonus = 0;
	int defenseBonus = 0;
	int numAttacks = 1;
	for(i = 0; i < MAX_READY; i++) {
		// Check the preferred weapon first.
		item = unit->readyWeapon[i];
		if(item == -1) break;
		abbr = ItemDefs[item].abr;
		item = unit->GetWeapon(abbr, riding, ridingbonus, attackBonus,
				defenseBonus, numAttacks);
		if(item != -1) {
			weapon = item;
			break;
		}
	}

	if(weapon == -1) {
		for(weaponType = 1; weaponType < NUMWEAPONS; weaponType++) {
			abbr = WeaponDefs[weaponType].abbr;
			item = unit->GetWeapon(abbr, riding, ridingbonus, attackBonus,
					defenseBonus, numAttacks);
			if(item != -1) {
				weapon = item;
				break;
			}
		}
	}
#ifdef DEBUG2
cout << "$";
#endif
	// If we did not get a weapon, set attack and defense bonuses to
	// combat skill (and riding bonus if applicable).
	if(weapon == -1) {
		attackBonus = unit->GetAttribute("combat") + ridingbonus;
		defenseBonus = attackBonus;
		numAttacks = 1;
	} else {
		// Okay.  We got a weapon.  If this weapon also has a special
		// and we don't have a special set, use that special.
		// Weapons (like Runeswords) which are both weapons and battle
		// items will be skipped in the battle items setup and handled
		// here.
		if ((ItemDefs[weapon].type & IT_BATTLE)) {
			BattleItemType *pBat = FindBattleItem(ItemDefs[weapon].abr);
			 //BS mod - only do so if battleitem is set to make a special; if ever have one which makes a shield, then add the shield coding here.
			if(special == NULL && pBat->flags & BattleItemType::SPECIAL) {
    			special = pBat->special;
    			slevel = pBat->skillLevel;
			}
			//BS - BattleItemType::ENERGY is dealt with in DoSpellCost below
		}
		//set attacktype to weapontype
		WeaponType *pWep = FindWeapon(ItemDefs[weapon].abr);
		attacktype = pWep->attackType;
	}

	unit->PracticeAttribute("combat");

	// Set the attack and defense skills
	// These will include the riding bonus if they should be included.
	askill += attackBonus;
	dskill[ATTACK_COMBAT] += defenseBonus;
	
	if(unit->GetSkill(S_BASE_BATTLETRAINING)) {
	    int gain = unit->GetSkill(S_BASE_BATTLETRAINING);
    	askill += gain;
    	dskill[ATTACK_COMBAT] += gain;
    	dskill[ATTACK_ENERGY] += gain;
    	dskill[ATTACK_SPIRIT] += gain;
    	dskill[ATTACK_WEATHER] += gain;
    	dskill[ATTACK_RIDING] += gain;
    	dskill[ATTACK_RANGED] += gain;
	}
	
	attacks = numAttacks;
	
    if(unit->GetSkill(S_FRENZY)) {
        int level = unit->GetSkill(S_FRENZY);
        int gain = 0;
        while(level > 0) {                      //+1,3,6,10,15,21
            gain += level--;
        }
        if(attacks < 0 && attacks+gain > -2) gain += 2;
        attacks += gain;
    }
}

Soldier::~Soldier()
{
    #ifdef DEBUG
    Awrite("Soldier Destructor");
    #endif
}


void Soldier::SetupSpell()
{
	if (unit->type != U_MAGE && unit->type != U_GUARDMAGE) return;

	if (unit->combat != -1) {
		slevel = unit->GetSkill(unit->combat);
		if(!slevel) {
			//
			// The unit can't cast this spell!
			//
			unit->combat = -1;
			return;
		}

		SkillType *pST = &SkillDefs[unit->combat];
		if(!(pST->flags & SkillType::COMBAT)) {
			//
			// This isn't a combat spell!
			//
			unit->combat = -1;
			return;
		}

		special = pST->special;
		unit->Practice(unit->combat);
	}
}

void Soldier::SetupCombatItems()
{
	int battleType;
	for(battleType = 1; battleType < NUMBATTLEITEMS; battleType++) {
		BattleItemType *pBat = &BattleItemDefs[battleType];

		AString abbr = pBat->abbr;
		int item = unit->GetBattleItem(abbr);
		if(item == -1) continue;

		// If we are using the ready command, skip this item unless
		// it's the right one, or unless it is a shield which doesn't
		// need preparing.
		if(!Globals->USE_PREPARE_COMMAND ||
				((unit->readyItem == -1) &&
				 (Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_NORMAL)) ||
				(item == unit->readyItem) ||
				(pBat->flags & BattleItemType::SHIELD)) {
			if((pBat->flags & BattleItemType::SPECIAL) && special != NULL) {
				// This unit already has a special attack so give the item
				// back to the unit as they aren't going to use it.
				unit->items.SetNum(item, unit->items.GetNum(item)+1);
				continue;
			}
			if(pBat->flags & BattleItemType::MAGEONLY &&
			   unit->type != U_MAGE && unit->type != U_GUARDMAGE &&
			   unit->type != U_APPRENTICE) {
				// Only mages/apprentices can use this item so give the
				// item back to the unit as they aren't going to use it.
				unit->items.SetNum(item, unit->items.GetNum(item)+1);
				continue;
			}

			/* Make sure amulets of invulnerability are marked */
			if(item == I_AMULETOFI) {
				amuletofi = 1;
			}

			SET_BIT(battleItems, battleType);

			if(pBat->flags & BattleItemType::SPECIAL) {
				special = pBat->special;
				slevel = pBat->skillLevel;
			}

			if(pBat->flags & BattleItemType::SHIELD) {
				SpecialType *sp = FindSpecial(pBat->special);
				/* we have a shield item with no shield FX */
				if(!(sp->effectflags & SpecialType::FX_SHIELD)) {
					continue;
				}
				for(int i = 0; i < 4; i++) {
					if(sp->shield[i] == NUM_ATTACK_TYPES) {
						for(int j = 0; j < NUM_ATTACK_TYPES; j++) {
//							if(dskill[j] < pBat->skillLevel)      //BS mod, so bonus is cumulative, not constant.
								dskill[j] += pBat->skillLevel;
						}
					} else if(sp->shield[i] >= 0) {
//						if(dskill[sp->shield[i]] < pBat->skillLevel)
							dskill[sp->shield[i]] += pBat->skillLevel;
					}
				}
			}
		} else {
			// We are using prepared items and this item is NOT the one
			// we have prepared, so give it back to the unit as they won't
			// use it.
			unit->items.SetNum(item, unit->items.GetNum(item)+1);
			continue;
		}
	}
}

int Soldier::ArmorProtect(int weaponClass)
// Returns 1 if the armour is successful
{
    if(weaponClass == NUM_WEAPON_CLASSES) return 0;
    
	ArmorType *pArm = NULL;
	if(armor > 0) pArm = FindArmor(ItemDefs[armor].abr);
	if (pArm == NULL) return 0;
	int chance = pArm->saves[weaponClass];

	if(chance <= 0) return 0;
	if(chance > getrandom(pArm->from)) return 1;

	return 0;
}

void Soldier::RestoreItems()
{
	if (healing && healitem != -1) {
		if(healitem == I_HERBS) {
			unit->items.SetNum(healitem,
					unit->items.GetNum(healitem) + healing);
		} else if(healitem == I_HEALPOTION) {
			unit->items.SetNum(healitem,
					unit->items.GetNum(healitem)+healing/5);
		}
	}
	if (weapon != -1)
		unit->items.SetNum(weapon,unit->items.GetNum(weapon) + 1);
	if(armor != -1)
		unit->items.SetNum(armor,unit->items.GetNum(armor) + 1);
	if(riding != -1)
		unit->items.SetNum(riding,unit->items.GetNum(riding) + 1);

	int battleType;
	for(battleType = 1; battleType < NUMBATTLEITEMS; battleType++) {
		BattleItemType *pBat = &BattleItemDefs[ battleType ];

		if(GET_BIT(battleItems, battleType)) {
			AString itm(pBat->abbr);
			int item = LookupItem(&itm);
			unit->items.SetNum(item, unit->items.GetNum(item) + 1);
		}
	}
}

void Soldier::Alive(int state)
{
	RestoreItems();
	if(maxhits > hits && (ItemDefs[race].type & IT_MAN)) {
        if(unit->GetSkill(S_TOUGHNESS)) unit->Experience(S_TOUGHNESS,50*(maxhits-hits)/maxhits);
    }

	if (state == LOSS) {
		unit->canattack = 0;
		/* Guards with amuletofi will not go off guard */
		if (!amuletofi &&
			(unit->guard == GUARD_GUARD || unit->guard == GUARD_SET)) {
			unit->guard = GUARD_NONE;
		}
	} else {
		unit->advancefrom = 0;
	}

	if (state == WIN_NO_MOVE) {
		unit->canattack = 0;
		unit->nomove = 1;
	}
}

void Soldier::Dead()
{
	RestoreItems();

	//if it's not a mage, or if it's a non-man from a mage unit, remove it.
	if(unit->type != U_MAGE || !(ItemDefs[race].type & IT_MAN)) unit->SetMen(race,unit->GetMen(race) - 1);
	//setmen changes the experience/skills.
	else unit->dead = unit->faction->num; //ARCADIA_MAGIC mod. Mages don't disappear, they become spirits of the dead.
}

int Soldier::HasEffect(char *eff)
//imported direct from old code.
{
	if(eff == NULL) return 0;
    EffectType *e = FindEffect(eff);
	return effects[e->effectnum];
}

void Soldier::SetEffect(char *eff, int form, Army *army)
{
	if(eff == NULL) return;
	int i;

	EffectType *e = FindEffect(eff);
	if (e == NULL) return;

	askill += e->attackVal;
	for(i = 0; i < 4; i++) {
		if(e->defMods[i].type != -1)
			dskill[e->defMods[i].type] += e->defMods[i].val;
	}

	if(e->cancelEffect != NULL) ClearEffect(e->cancelEffect);

	if(!(e->flags & EffectType::EFF_NOSET)) effects[e->effectnum] = 1;

	if(e->flags & EffectType::EFF_TRANSFIGURE) {
	    MonType *mpold = FindMonster(ItemDefs[race].abr, (ItemDefs[race].type & IT_ILLUSION));
	    int finalrace = e->monster;
	    if(race == finalrace) return;
	    int newrace = -1;
	    if(maxhits <= 2) {
            newrace = finalrace;
            if(illusion) newrace = I_IRAT; //Arcadia hack.
	    } else {
	        //something to get newrace
	        //not done in time for Nylandor :(.
	    }
	    if(newrace < 0) return;
	    MonType *mp = FindMonster(ItemDefs[newrace].abr, (ItemDefs[newrace].type & IT_ILLUSION));
	    if(!mp || !mpold) return;
	    unit->items.SetNum(race, unit->items.GetNum(race) - 1);
	    race = newrace; //change soldier's race.
	    unit->items.SetNum(race, unit->items.GetNum(race) + 1);
	    int oldhits = maxhits;
	    maxhits = mp->hits;
	    if(!illusion) army->hitstotal += maxhits - oldhits;
	    army->formations[form].AddSize(maxhits - oldhits);
	    if(hits > maxhits) hits = maxhits;

		askill += mp->attackLevel - mpold->attackLevel;
		dskill[ATTACK_COMBAT] += mp->defense[ATTACK_COMBAT] - mpold->defense[ATTACK_COMBAT];
		dskill[ATTACK_ENERGY] += mp->defense[ATTACK_ENERGY] - mpold->defense[ATTACK_ENERGY];
		dskill[ATTACK_SPIRIT] += mp->defense[ATTACK_SPIRIT] - mpold->defense[ATTACK_SPIRIT];
		dskill[ATTACK_WEATHER] += mp->defense[ATTACK_WEATHER] - mpold->defense[ATTACK_WEATHER];
		dskill[ATTACK_RIDING] += mp->defense[ATTACK_RIDING] - mpold->defense[ATTACK_RIDING];
		dskill[ATTACK_RANGED] += mp->defense[ATTACK_RANGED] - mpold->defense[ATTACK_RANGED];
		
		if(unit->type == U_WMON)
			name = AString(mp->name) + " in " + *(unit->name);
		else
			name = AString(mp->name) + " controlled by " + *(unit->name);
        //if needed, transfer soldier to new formation.
	}
}

void Soldier::ClearEffect(char *eff)
{
	if(eff == NULL) return;
	int i;

	EffectType *e = FindEffect(eff);
	if (e == NULL) return;

	if(effects[e->effectnum] == 0) return;

	askill -= e->attackVal;

	for(i = 0; i < 4; i++) {
		if(e->defMods[i].type != -1)
			dskill[e->defMods[i].type] -= e->defMods[i].val;
	}

	effects[e->effectnum] = 0;
}

//this is called at the end of formations phase (from Formation::Reset from Army::Reset)
void Soldier::ClearOneTimeEffects(void)
{
	for(int i = 0; i < NUMEFFECTS; i++) {
		if(EffectDefs[i].flags & EffectType::EFF_ONESHOT) ClearEffect(EffectDefs[i].name);
	}
}


int Soldier::DoSpellCost(int round, Battle *b)
//returns 1 if can cast the spell, 0 if cannot.
{
    if(!Globals->ARCADIA_MAGIC) return 1;
    
    //staff of yew coding
    if ((ItemDefs[weapon].type & IT_BATTLE)) {
		BattleItemType *pBat = FindBattleItem(ItemDefs[weapon].abr);
		if(pBat->flags & BattleItemType::ENERGY) {
        //can cast, energy cost of zero.
            int exper = 8 - round; //7, 6, then 5, then 4 ... sum of 27 in 6 rounds, then 1 per round.
	        if(exper < 1) exper = 1;
            unit->Experience(unit->combat, exper, 0);
            return 1;
		}
	}


	if(!exhausted && unit->type == U_MAGE && unit->combat != -1) {
	    int cost;
	    if(round == 1) cost = unit->GetFirstCombatCost(unit->combat);
	    else cost = unit->GetCombatCost(unit->combat);
	    if(cost > unit->GetEnergy() ) {
            exhausted = 1;
	        slevel /= 2;
	        if(!slevel) {
                special = NULL;
                b->AddLine( *(unit->name) + " is exhausted, and cannot cast any more spells.");
                return 0;
            } else {
                b->AddLine( *(unit->name) + " is exhausted, and will cast with reduced effectiveness.");
                unit->energy -= unit->GetEnergy(); //do not want to reduce energy below this, because some might have already been "used" by portalling.
            }
	    } else {
            unit->energy -= cost;
	        int exper = 8 - round; //7, 6, then 5, then 4 ... sum of 27 in 6 rounds, then 1 per round.
	        if(exper < 1) exper = 1;
	        unit->Experience(unit->combat, exper, 0);
        }
	    //no experience after running out of energy.
	} else if(exhausted && unit->type == U_MAGE && unit->combat != -1 && slevel) {
	    unit->energy -= unit->GetEnergy(); //just in case wasn't set to zero earlier, eg in dospellcheck
	}
	
	int mevent = unit->MysticEvent();
	switch(mevent) {
	    case 4:
	        b->AddLine( *(unit->name) + "'s spell backfires, hitting the wrong army.");
	        return 2;
	    case 3:
	    case 2:
	    case 1:
	        b->AddLine( *(unit->name) + "'s spell fizzles mid-cast.");
	        return 0;
        default:
            break;
	}
	return 1;
}

void Soldier::DoSpellCheck(int round, Battle *b)
{
	if(!exhausted && unit->type == U_MAGE && unit->combat != -1) {
	    int cost;
	    if(round == 1) cost = unit->GetFirstCombatCost(unit->combat);
	    else cost = unit->GetCombatCost(unit->combat);
	    if(cost > unit->GetEnergy() ) {
	        exhausted = 1;
	        slevel /= 2;
	        if(!slevel) {
                special = NULL;
                b->AddLine( *(unit->name) + "is exhausted, and cannot cast any more spells.");
            } else b->AddLine( *(unit->name) + "is exhausted, and will cast with reduced effectiveness.");
	    }
	}
	return;
}

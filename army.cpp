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
// Date        Person          Change
// ----        ------          ------
// 2000/MAR/16 Larry Stanbery  Corrected bug in Soldier constructor.
//                             Fixed "assassination bug" reported on
//                             DejaNews.
// 2001/Feb/18 Joseph Traub    Added Apprentices concept from Lacondon Conquest
#include "army.h"
#include "gameio.h"
#include "gamedata.h"

enum {
  WIN_NO_DEAD,
  WIN_DEAD,
  LOSS
};

Soldier::Soldier(Unit * u,Object * o,int regtype,int r,int ass)
{
	race = r;
	unit = u;
	building = 0;

	healing = 0;
	healtype = 0;
	healitem = -1;
	canbehealed = 1;

	armor = -1;
	riding = -1;
	weapon = -1;

	attacks = 1;
	attacktype = ATTACK_COMBAT;

	special = -1;
	slevel = 0;

	askill = 0;

	dskill[ATTACK_COMBAT] = 0;
	dskill[ATTACK_ENERGY] = -2;
	dskill[ATTACK_SPIRIT] = -2;
	dskill[ATTACK_WEATHER] = -2;
	dskill[ATTACK_RIDING] = 0;
	dskill[ATTACK_RANGED] = 0;
	hits = 1;
	amuletofi = 0;
	battleItems = 0;

	effects = 0;

	/* Building bonus */
	if (o->capacity) {
		building = o->type;
		for (int i=0; i<NUM_ATTACK_TYPES; i++) {
			dskill[i] += 2;
		}
		if (o->runes) {
			dskill[ATTACK_ENERGY] = o->runes;
			dskill[ATTACK_SPIRIT] = o->runes;
		}
		o->capacity--;
	}

	/* Is this a monster? */
	if (ItemDefs[r].type & IT_MONSTER) {
		int mon = ItemDefs[r].index;
		if(u->type == U_WMON)
			name = AString(MonDefs[mon].name);
		else
			name = AString(MonDefs[mon].name) +
				AString(" controlled by ") + *(unit->name);
		askill = MonDefs[mon].attackLevel;
		dskill[ATTACK_COMBAT] += MonDefs[mon].defense[ATTACK_COMBAT];
		if (MonDefs[mon].defense[ATTACK_ENERGY] > dskill[ATTACK_ENERGY]) {
			dskill[ATTACK_ENERGY] = MonDefs[mon].defense[ATTACK_ENERGY];
		}
		if (MonDefs[mon].defense[ATTACK_SPIRIT] > dskill[ATTACK_SPIRIT]) {
			dskill[ATTACK_SPIRIT] = MonDefs[mon].defense[ATTACK_SPIRIT];
		}
		if(MonDefs[mon].defense[ATTACK_WEATHER] > dskill[ATTACK_WEATHER]) {
			dskill[ATTACK_WEATHER] = MonDefs[mon].defense[ATTACK_WEATHER];
		}
		dskill[ATTACK_RIDING] += MonDefs[mon].defense[ATTACK_RIDING];
		dskill[ATTACK_RANGED] += MonDefs[mon].defense[ATTACK_RANGED];
		hits = MonDefs[mon].hits;
		if (!hits) hits = 1;
		attacks = MonDefs[mon].numAttacks;
		if(!attacks) attacks = 1;
		special = MonDefs[mon].special;
		slevel = MonDefs[mon].specialLevel;
		return;
	}

	name = *(unit->name);

	SetupHealing();

	SetupSpell();
	SetupCombatItems();

	// Set up armor
	int i, item, armorType;
	for(i = 0; i < MAX_READY; i++) {
		// Check preferred armor first.
		item = unit->readyArmor[i];
		if(item == -1) break;
		armorType = ItemDefs[item].index;
		item = unit->GetArmor(armorType, ass);
		if(item != -1) {
			armor = item;
			break;
		}
	}
	if(armor == -1) {
		for( armorType = 1; armorType < NUMARMORS; armorType++ ) {
			item = unit->GetArmor(armorType, ass);
			if(item != -1) {
				armor = item;
				break;
			}
		}
	}

	//
	// Check if this unit is mounted
	//
	int terrainflags = TerrainDefs[regtype].flags;
	int canFly = (terrainflags & TerrainType::FLYINGMOUNTS);
	int canRide = (terrainflags & TerrainType::RIDINGMOUNTS);
	if(canFly || canRide) {
		//
		// Mounts of some type _are_ allowed in this region
		//
		int mountType;
		for(mountType = 1; mountType < NUMMOUNTS; mountType++ ) {
			int bonus = 0;
			item = unit->GetMount(mountType, canFly, canRide, bonus);
			if(item == -1) continue;
			askill += bonus;
			dskill[ATTACK_COMBAT] += bonus;
			dskill[ATTACK_RIDING] += bonus;
			riding = item;
			break;
		}
	}

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
		weaponType = ItemDefs[item].index;
		item = unit->GetWeapon(weaponType, riding, attackBonus, defenseBonus,
				numAttacks);
		if(item != -1) {
			weapon = item;
			break;
		}
	}
	if(weapon == -1) {
		for( weaponType = 1; weaponType < NUMWEAPONS; weaponType++ ) {
			item = unit->GetWeapon(weaponType, riding, attackBonus,
					defenseBonus, numAttacks);
			if(item != -1) {
				weapon = item;
				break;
			}
		}
	}
	// If we did not get a weapon, set attack and defense bonuses to
	// combat skill.
	if( weapon == -1 ) {
		attackBonus = unit->GetSkill(S_COMBAT);
		defenseBonus = attackBonus;
		numAttacks = 1;
	}

	// Set the attack and defense skills
	askill += attackBonus;
	dskill[ATTACK_COMBAT] += defenseBonus;
	attacks = numAttacks;
}

void Soldier::SetupSpell()
{
    if (unit->type != U_MAGE && unit->type != U_GUARDMAGE) return;

    if (unit->combat != -1)
    {
        slevel = unit->GetSkill( unit->combat );
        if( !slevel )
        {
            //
            // The unit can't cast this spell!
            //
            unit->combat = -1;
            return;
        }

        SkillType *pST = &SkillDefs[ unit->combat ];
        if( !( pST->flags & SkillType::COMBAT ))
        {
            //
            // This isn't a combat spell!
            //
            unit->combat = -1;
            return;
        }

        special = pST->special;
    }
}

void Soldier::SetupCombatItems()
{
    int battleType;
    for(battleType = 1; battleType < NUMBATTLEITEMS; battleType++) {
        BattleItemType *pBat = &BattleItemDefs[ battleType ];

		int item = unit->GetBattleItem(battleType);
		if(item == -1) continue;

		// If we are using the ready command, skip this item unless
		// it's the right one.
		if(!Globals->USE_PREPARE_COMMAND || (pBat->itemNum==unit->readyItem)) {
			if(( pBat->flags & BattleItemType::SPECIAL ) && special != -1 ) {
				// This unit already has a special attack
				continue;
			}
			if(pBat->flags & BattleItemType::MAGEONLY &&
			   unit->type != U_MAGE && unit->type != U_GUARDMAGE &&
			   unit->type != U_APPRENTICE) {
				// Only mages/apprentices can use this item
				continue;
			}

			/* Make sure amulets of invulnerability are marked */
			if( item == I_AMULETOFI ) {
				amuletofi = 1;
			}

			SET_BIT( battleItems, battleType );

			if( pBat->flags & BattleItemType::SPECIAL ) {
				special = pBat->index;
				slevel = pBat->skillLevel;
			}
		}

		if( pBat->flags & BattleItemType::SHIELD ) {
			SpecialType *sp = &SpecialDefs[pBat->index];
			/* we have a shield item with no shield FX */
			if(!(sp->effectflags & SpecialType::FX_SHIELD)) {
				continue;
			}
			for(int i = 0; i < 4; i++) {
				if(sp->shield[i] == NUM_ATTACK_TYPES) {
					for(int j = 0; j < NUM_ATTACK_TYPES; j++) {
						if(dskill[j] < pBat->skillLevel)
							dskill[j] = pBat->skillLevel;
					}
				} else if(sp->shield[i] >= 0) {
					if(dskill[sp->shield[i]] < pBat->skillLevel)
						dskill[sp->shield[i]] = pBat->skillLevel;
				}
			}
		}
	}
}

int Soldier::HasEffect(int eff)
{
	if(eff < 0) return 0;
	int n = 1 << eff;
	return (effects & n);
}

void Soldier::SetEffect(int eff)
{
	if(eff < 0) return;
	int n = 1 << eff;
	int i;

	EffectType *e = &EffectDefs[eff];

	askill += e->attackVal;

	for(i = 0; i < 4; i++) {
		if(e->defMods[i].type != -1) {
			dskill[e->defMods[i].type] += e->defMods[i].val;
		}
	}

	if(e->cancelEffect != -1) {
		ClearEffect(e->cancelEffect);
	}

	if(!(e->flags & EffectType::EFF_NOSET)) {
		effects = effects | n;
	}
}

void Soldier::ClearEffect(int eff)
{
	if(eff < 0) return;
	int n = 1 << eff;
	int i;

	EffectType *e = &EffectDefs[eff];

	askill -= e->attackVal;

	for(i = 0; i < 4; i++) {
		if(e->defMods[i].type != -1) {
			dskill[e->defMods[i].type] -= e->defMods[i].val;
		}
	}
	effects &= ~n;
}

void Soldier::ClearOneTimeEffects(void)
{
	for(int i = 0; i < NUMEFFECTS; i++) {
		if(HasEffect(i) && (EffectDefs[i].flags & EffectType::EFF_ONESHOT))
			ClearEffect(i);
	}
}

int Soldier::ArmorProtect(int weaponClass)
{
    //
    // Return 1 if the armor is successful
    //
	ArmorType *pArm;
	int armorType = ARMOR_NONE;
	if(armor > 0) armorType = ItemDefs[armor].index;
	if(armorType < ARMOR_NONE || armorType >= NUMARMORS)
		armorType = ARMOR_NONE;
	pArm = &ArmorDefs[armorType];
	int chance = pArm->saves[weaponClass];

	if(chance <= 0) return 0;
	if(chance > getrandom(pArm->from)) return 1;

    return 0;
}

void Soldier::RestoreItems()
{
    if (healing && healitem != -1 )
    {
		if(healitem == I_HERBS) {
			unit->items.SetNum( healitem,
					unit->items.GetNum( healitem ) + healing);
		} else if(healitem == I_HEALPOTION) {
			unit->items.SetNum(healitem,
					unit->items.GetNum(healitem)+healing/10);
		}
    }
    if (weapon != -1)
    {
        unit->items.SetNum(weapon,unit->items.GetNum(weapon) + 1);
    }
    if( armor != -1 )
    {
        unit->items.SetNum(armor,unit->items.GetNum(armor) + 1);
    }
    if( riding != -1 )
    {
        unit->items.SetNum(riding,unit->items.GetNum(riding) + 1);
    }

    int battleType;
    for( battleType = 1; battleType < NUMBATTLEITEMS; battleType++ )
    {
        BattleItemType *pBat = &BattleItemDefs[ battleType ];

        if( GET_BIT( battleItems, battleType ))
        {
            int item = pBat->itemNum;
            unit->items.SetNum( item, unit->items.GetNum( item ) + 1 );
        }
    }
}

void Soldier::Alive(int state)
{
    RestoreItems();

    if (state == LOSS)
    {
        unit->canattack = 0;
        /* Guards with amuletofi will not go off guard */
        if (!amuletofi &&
            (unit->guard == GUARD_GUARD || unit->guard == GUARD_SET)) {
            unit->guard = GUARD_NONE;
        }
    }
    else
    {
        unit->advancefrom = 0;
    }

    if (state == WIN_DEAD)
    {
        unit->canattack = 0;
        unit->nomove = 1;
    }
}

void Soldier::Dead()
{
    RestoreItems();

    unit->SetMen(race,unit->GetMen(race) - 1);
}

Army::Army(Unit * ldr,AList * locs,int regtype,int ass)
{
    leader = ldr;
    round = 0;
    tac = 0;
    count = 0;

    if (ass)
    {
        count = 1;
        ldr->losses = 0;
    }
    else
    {
        forlist(locs) {
            Unit * u = ((Location *) elem)->unit;
            count += u->GetSoldiers();
            u->losses = 0;
        }
    }

    soldiers = new SoldierPtr[count];
    int x = 0;
    int y = count;
    int tacspell = 0;

    {
        forlist(locs) {
            Unit * u = ((Location *) elem)->unit;
            Object * obj = ((Location *) elem)->obj;
            int temp = u->GetSkill(S_TACTICS);
            if (temp > tac) tac = temp;
            if (ass)
            {
                forlist(&u->items) {
                    Item * it = (Item *) elem;
                    if (it)
                    {
                        if( ItemDefs[ it->type ].type & IT_MAN )
                        {
                            soldiers[x++] = new Soldier( u,
                                                         obj,
                                                         regtype,
                                                         it->type,
                                                         ass );
                            goto finished_army;
                        }
                    }
                }
            }
            else
            {
                Item *it = (Item *) u->items.First();
                do
                {
                    if( IsSoldier( it->type ) )
                    {
                        for( int i = 0; i < it->num; i++ )
                        {
                            if( ItemDefs[ it->type ].type & IT_MAN &&
                                u->GetFlag( FLAG_BEHIND ) )
                            {
                                soldiers[--y] = new Soldier( u,
                                                             obj,
                                                             regtype,
                                                             it->type );
                            }
                            else
                            {
                                soldiers[x++] = new Soldier( u,
                                                             obj,
                                                             regtype,
                                                             it->type);
                            }
                        }
                    }

                    it = (Item *) u->items.Next( it );
                }
                while( it );
            }
        }
    }

finished_army:
    tac = tac + tacspell;

    canfront = x;
    canbehind = count;
    notfront = count;
    notbehind = count;

    if (!NumFront())
    {
        canfront = canbehind;
        notfront = notbehind;
    }
}

Army::~Army()
{
	delete [] soldiers;
}

void Army::Reset() {
    canfront = notfront;
    canbehind = notbehind;
    notfront = notbehind;
}

void Army::WriteLosses(Battle * b) {
    b->AddLine(*(leader->name) + " loses " + (count - NumAlive()) + ".");

    if (notbehind != count) {
        AList units;
        for (int i=notbehind; i<count; i++) {
            if (!GetUnitList(&units,soldiers[i]->unit)) {
                UnitPtr *u = new UnitPtr;
                u->ptr = soldiers[i]->unit;
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

void Army::GetMonSpoils(ItemList *spoils,int monitem)
{
    /* First, silver */
    spoils->SetNum(I_SILVER,spoils->GetNum(I_SILVER) +
                   getrandom(MonDefs[ItemDefs[monitem].index].silver));

    int thespoil = MonDefs[ItemDefs[monitem].index].spoiltype;

    if (thespoil == -1) return;
    if (thespoil == IT_NORMAL && getrandom(2))
    {
        thespoil = IT_TRADE;
    }

    int count = 0;
    int i;
    for (i=0; i<NITEMS; i++)
    {
        if ((ItemDefs[i].type & thespoil) &&
			!(ItemDefs[i].type & IT_SPECIAL) &&
			!(ItemDefs[i].flags & ItemType::DISABLED))
        {
            count ++;
        }
    }

    count = getrandom(count) + 1;

    for (i=0; i<NITEMS; i++) {
        if ((ItemDefs[i].type & thespoil) &&
			!(ItemDefs[i].type & IT_SPECIAL) &&
			!(ItemDefs[i].flags & ItemType::DISABLED))
        {
            count--;
            if (count == 0)
            {
                thespoil = i;
                break;
            }
        }
    }

    int val = getrandom(MonDefs[ItemDefs[monitem].index].silver * 2);

    spoils->SetNum(thespoil,spoils->GetNum(thespoil) +
                   (val + getrandom(ItemDefs[thespoil].baseprice))
                   / ItemDefs[thespoil].baseprice);
}

void Army::Lose(Battle *b,ItemList *spoils)
{
	WriteLosses(b);
	for (int i=0; i<count; i++) {
		Soldier * s = soldiers[i];
		if (i<notbehind) {
			s->Alive(LOSS);
		} else {
			if (s->unit->type==U_WMON && (ItemDefs[s->race].type&IT_MONSTER)) {
				GetMonSpoils(spoils,s->race);
			}
			s->Dead();
		}
		delete s;
	}
}

void Army::Tie(Battle * b)
{
	WriteLosses(b);
	for(int x=0; x<count; x++) {
		Soldier * s = soldiers[x];
		if (x<NumAlive()) {
			s->Alive(WIN_DEAD);
		} else {
			s->Dead();
		}
		delete s;
	}
}

int Army::CanBeHealed()
{
    for (int i=notbehind; i<count; i++)
    {
        Soldier * temp = soldiers[i];
        if (temp->canbehealed) return 1;
    }
    return 0;
}

void Army::DoHeal(Battle * b)
{
	// Do magical healing
	for(int i = 5; i > 0; --i)
    {
		DoHealLevel(b, i, 0);
    }
	// Do Normal healing
	DoHealLevel(b, 1, 1);
}

void Army::DoHealLevel( Battle *b, int type, int useItems )
{
	int rate = HealDefs[type].rate;

    for (int i=0; i<NumAlive(); i++)
    {
        Soldier * s = soldiers[i];
        int n = 0;
        if (!CanBeHealed()) break;
        if( s->healtype <= 0)
        {
            continue;
        }
		// This should be here.. Use the best healing first
		if(s->healtype != type) {
			continue;
		}
        if( !s->healing )
        {
            continue;
        }
        if( useItems )
        {
            if( s->healitem == -1 )
            {
                continue;
            }
        }
        else
        {
            if( s->healitem != -1 )
            {
                continue;
            }
        }

        while (s->healing)
        {
            if (!CanBeHealed()) break;
            int j = getrandom(count - NumAlive()) + notbehind;
            Soldier * temp = soldiers[j];
            if (temp->canbehealed)
            {
                s->healing--;
                if (getrandom(100) < rate)
                {
                    n++;
                    soldiers[j] = soldiers[notbehind];
                    soldiers[notbehind] = temp;
                    notbehind++;
                }
                else
                {
                    temp->canbehealed = 0;
                }
            }
        }
        b->AddLine(*(s->unit->name) + " heals " + n + ".");
    }
}

void Army::Win(Battle * b,ItemList * spoils)
{
	int wintype;
	if (count - NumAlive()) wintype = WIN_DEAD;
	else wintype = WIN_NO_DEAD;

	DoHeal(b);

	WriteLosses(b);
	int na = NumAlive();
	AList units;

	forlist(spoils) {
		Item *i = (Item *) elem;
		if(i && na) {
			Unit *u;
			UnitPtr *up;

			// Make a list of units who can get this type of spoil
			for(int x = 0; x < na; x++) {
				u = soldiers[x]->unit;
				if(u->CanGetSpoil(i)) {
					up = new UnitPtr;
					up->ptr = u;
					units.Add(up);
				}
			}

			int ns = units.Num();
			if(ns > 0) {
				int n = i->num/ns; // Divide spoils equally
				if(n >= 1) {
					forlist(&units) {
						up = (UnitPtr *)elem;
						up->ptr->items.SetNum(i->type,
								up->ptr->items.GetNum(i->type)+n);
						up->ptr->faction->DiscoverItem(i->type, 0, 1);
					}
				}
				n = i->num % ns; // allocate the remainder
				if(n) {
					for(int x = 0; x < n; x++) {
						int t = getrandom(ns);
						up = (UnitPtr *)units.First();
						if(up) {
							UnitPtr *p;
							while(t > 0) {
								p = (UnitPtr *)units.Next(up);
								if(p) up = p;
								else break;
								--t;
							}
							up->ptr->items.SetNum(i->type,
									up->ptr->items.GetNum(i->type)+1);
							up->ptr->faction->DiscoverItem(i->type, 0, 1);
						}
					}
				}
			}
			units.DeleteAll();
		}
	}

	for(int x = 0; x < count; x++) {
		Soldier * s = soldiers[x];
		if (x<NumAlive()) s->Alive(wintype);
		else s->Dead();
		delete s;
	}
}

int Army::Broken()
{
	if ((NumAlive() * 2 / count) >= 1) return 0;
	return 1;
}

int Army::NumSpoilers()
{
	int na = NumAlive();
	int count = 0;
	for(int x=0; x<na; x++) {
		Unit * u = soldiers[x]->unit;
		if(!(u->flags & FLAG_NOSPOILS)) count++;
	}
	return count;
}

int Army::NumAlive()
{
	return notbehind;
}

int Army::CanAttack()
{
	return canbehind;
}

int Army::NumFront()
{
	return (canfront + notfront - canbehind);
}

Soldier * Army::GetAttacker(int i,int &behind)
{
	Soldier * retval = soldiers[i];
	if (i<canfront) {
		soldiers[i] = soldiers[canfront-1];
		soldiers[canfront-1] = soldiers[canbehind-1];
		soldiers[canbehind-1] = retval;
		canfront--;
		canbehind--;
		behind = 0;
		return retval;
	}
	soldiers[i] = soldiers[canbehind-1];
	soldiers[canbehind-1] = soldiers[notfront-1];
	soldiers[notfront-1] = retval;
	canbehind--;
	notfront--;
	behind = 1;
	return retval;
}

int Army::GetTargetNum(int special)
{
	int tars = NumFront();
	if (tars == 0) {
		canfront = canbehind;
		notfront = notbehind;
		tars = NumFront();
		if (tars == 0) return -1;
	}

	/* For certain attacks, we need to do retries */
	for (int retries = 0; retries < 10; retries++) {
		int i = getrandom(tars);
		if (i<canfront) {
			if (CheckSpecialTarget(special,i)) return i;
			continue;
		}
		i += canbehind - canfront;
		if (CheckSpecialTarget(special,i)) return i;
		continue;
	}

	return -1;
}

int Army::GetEffectNum( int effect )
{
	int tars = NumAlive();
	int retries;
	for( retries = 0; retries < 10; retries++ ) {
		int i = getrandom( tars );
		if( soldiers[ i ]->HasEffect( effect )) {
			return( i );
		}
	}
	return( -1 );
}

Soldier * Army::GetTarget(int i)
{
	return soldiers[i];
}

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

int Army::RemoveEffects( int num, int effect )
{
	int ret = 0;
	for( int i = 0; i < num; i++ ) {
		//
		// Try to find a target unit.
		//
		int tarnum = GetEffectNum( effect );
		if (tarnum == -1) continue;
		Soldier *tar = GetTarget( tarnum );

		//
		// Remove the effect
		//
		tar->ClearEffect( effect );
		ret++;
	}
	return( ret );
}

int Army::DoAnAttack( int special, int numAttacks, int attackType,
		int attackLevel, int flags, int weaponClass, int effect,
		int mountBonus )
{
	/* 1. Check against Global effects (not sure how yet) */
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
			if( !Hits( attackLevel, hi->shieldskill )) {
				return -1;
			}

			if(!effect && !combat) {
				/* We got through shield... if killing spell, destroy shield */
				shields.Remove(hi);
				delete hi;
			}
		}
	}

	//
	// Now, loop through and do attacks
	//
	int ret = 0;
	for( int i = 0; i < numAttacks; i++ ) {
		/* 3. Get the target */
		int tarnum = GetTargetNum(special);
		if (tarnum == -1) continue;
		Soldier * tar = GetTarget(tarnum);
		int tarFlags = 0;
		if(tar->weapon != -1) {
			tarFlags = WeaponDefs[ItemDefs[tar->weapon].index].flags;
		}

		/* 4. Add in any effects, if applicable */
		int tlev = 0;
		if(attackType != NUM_ATTACK_TYPES)
			tlev = tar->dskill[ attackType ];
		if(special > 0) {
			if((SpecialDefs[special].effectflags&SpecialType::FX_NOBUILDING) &&
					tar->building) {
				tlev -= 2;
			}
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
		}

		/* 4.3 Add bonuses versus mounted */
		if(tar->riding != -1) attackLevel += mountBonus;

		/* 5. Attack soldier */
		if ( attackType != NUM_ATTACK_TYPES) {
			if( !( flags & WeaponType::ALWAYSREADY)) {
				if( getrandom( 2 )) {
					continue;
				}
            }

			if (!Hits( attackLevel,tlev)) {
				continue;
			}
		}

		/* 6. If attack got through, apply effect, or kill */
		if (!effect) {
			/* 7. Last chance... Check armor */
			if (tar->ArmorProtect(weaponClass)) {
				continue;
			}

			/* 8. Seeya! */
			Kill(tarnum);
			ret++;
		} else {
			if (tar->HasEffect(effect)) {
				continue;
			}
			tar->SetEffect(effect);
			ret++;
		}
	}
	return ret;
}

void Army::Kill(int killed)
{
    Soldier * temp = soldiers[killed];

    if (temp->amuletofi) return;

    temp->hits--;
    if (temp->hits) return;
    temp->unit->losses++;

    if (killed < canfront)
    {
        soldiers[killed] = soldiers[canfront-1];
        soldiers[canfront-1] = temp;
        killed = canfront - 1;
        canfront--;
    }

    if (killed < canbehind)
    {
        soldiers[killed] = soldiers[canbehind-1];
        soldiers[canbehind-1] = temp;
        killed = canbehind-1;
        canbehind--;
    }

    if (killed < notfront)
    {
        soldiers[killed] = soldiers[notfront-1];
        soldiers[notfront-1] = temp;
        killed = notfront-1;
        notfront--;
    }

    soldiers[killed] = soldiers[notbehind-1];
    soldiers[notbehind-1] = temp;
    notbehind--;
}

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
#include "battle.h"
#include "gamedata.h"

void Soldier::SetupHealing()
{
    if (unit->type == U_MAGE) {
        healtype = unit->GetSkill(S_MAGICAL_HEALING);
		if (healtype > 5) healtype = 5;
		if (healtype > 0) {
			healing = HealDefs[healtype].num;
            healitem = -1;
            return;
        }
    }

	if(unit->items.GetNum(I_HEALPOTION)) {
		healtype = 1;
		unit->items.SetNum(I_HEALPOTION, unit->items.GetNum(I_HEALPOTION)-1);
		healing = 10;
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

int Army::CheckSpecialTarget(int special,int tar)
{
	SpecialType *spd = &SpecialDefs[special];
	int i;
	int match = 0;

	if(spd->targflags & SpecialType::HIT_BUILDINGIF) {
		match = 0;
		for(i = 0; i < 3; i++) {
			if (soldiers[tar]->building &&
					(spd->buildings[i] == soldiers[tar]->building)) match = 1;
		}
		if(!match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_BUILDINGEXCEPT) {
		match = 0;
		for(i = 0; i < 3; i++) {
			if (soldiers[tar]->building &&
					(spd->buildings[i] == soldiers[tar]->building)) match = 1;
		}
		if(match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_SOLDIERIF) {
		match = 0;
		for(i = 0; i < 7; i++) {
			if(soldiers[tar]->race == spd->targets[i]) match = 1;
		}
		if(!match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_SOLDIEREXCEPT) {
		match = 0;
		for(i = 0; i < 7; i++) {
			if(soldiers[tar]->race == spd->targets[i]) match = 1;
		}
		if(match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_EFFECTIF) {
		match = 0;
		for(i = 0; i < 3; i++) {
			if(soldiers[tar]->HasEffect(spd->effects[i])) match = 1;
		}
		if(!match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_EFFECTEXCEPT) {
		match = 0;
		for(i = 0; i < 3; i++) {
			if(soldiers[tar]->HasEffect(spd->effects[i])) match = 1;
		}
		if(match) return 0;
	}

	if(spd->targflags & SpecialType::HIT_ILLUSION) {
		// All illusions are of type monster, so lets make sure we get it
		// right.  If we ever have other types of illusions, we can change
		// this.
		if(!(ItemDefs[soldiers[tar]->race].type & IT_MONSTER))
			return 0;
		if(ItemDefs[soldiers[tar]->race].index != MONSTER_ILLUSION)
			return 0;
	}

	if(spd->targflags & SpecialType::HIT_NOMONSTER) {
		if(ItemDefs[soldiers[tar]->race].type & IT_MONSTER)
			return 0;
	}
    return 1;
}

void Battle::UpdateShields(Army *a)
{
	for (int i=0; i<a->notbehind; i++) {
		int shtype = -1;
		SpecialType *spd;

		if(a->soldiers[i]->special == -1) continue;
		spd = &SpecialDefs[a->soldiers[i]->special];

		if(!(spd->effectflags & FX_SHIELD)) continue;

		for(shtype = 0; shtype < 4; shtype++) {
			if(spd->shield[shtype].type == -1) continue;
			if(spd->effectflags & FX_DEFBONUS) {
				int bonus = spd->shield[shtype].value;
				if(spd->effectflags & FX_USE_LEV)
					bonus *= a->soldiers[i]->slevel;
				if(a->round == 0)
					a->soldiers[i]->dskill[spd->shield[shtype].type] += bonus;
			}

			Shield *sh = new Shield;
			sh->shieldtype = spd->shield[shtype].type;
			sh->shieldskill = a->soldiers[i]->slevel;
			a->shield.Add(sh);
            AddLine(*(a->soldiers[i]->unit->name) + " casts " +
					spd->shielddesc + ".");
		}
	}
}

void Battle::DoSpecialAttack(int round, Soldier *a, Army *attackers,
		Army *def, int behind)
{
	int num,num2;
	switch (a->special) {
		case -1:
			break;

		case SPECIAL_TORNADO:
        num = def->DoAnAttack( a->special,
                               getrandom(a->slevel * 25)
                               + getrandom(a->slevel * 25) + 2,
                               ATTACK_WEATHER,
                               a->slevel,
                               SPECIAL_FLAGS, SPECIAL_CLASS,
                               0, 0);
        if (num == -1) {
            AddLine(a->name + " summons a wild tornado, but it is "
                    "deflected.");
        } else {
            AddLine(a->name + " summons a wild tornado, killing " +
                    num + ".");
        }
        break;
        
    case SPECIAL_SUMMON_STORM:
        num = def->DoAnAttack(a->special,
                              getrandom(25 * a->slevel)
                              + getrandom(25 * a->slevel) + 2,
                              ATTACK_WEATHER,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              EFFECT_STORM, 0);
        if (num == -1) {
            AddLine(a->name + " summons a terrible storm, "
                    "but it is deflected.");
        } else {
            AddLine(a->name + " summons a terrible storm, "
                    "reducing the effectiveness of " + num + " troops.");
        }
        break;
        
    case SPECIAL_DISPEL_ILLUSIONS:
        num = def->DoAnAttack(a->special,
                              getrandom(50 * a->slevel)
                              + getrandom(50 * a->slevel) + 2,
                              NUM_ATTACK_TYPES,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              0, 0);
        AddLine(a->name + " casts Dispel Illusions, dispelling " +
                num + " illusions.");
        break;
        
    case SPECIAL_BANISH_UNDEAD:
        num = def->DoAnAttack(a->special,
                              getrandom(25 * a->slevel)
                              + getrandom(25 * a->slevel) + 2,
                              NUM_ATTACK_TYPES,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              0, 0);
        AddLine(a->name + " casts Banish Undead, banishing " +
                num + " undead.");
        break;
        
    case SPECIAL_BANISH_DEMONS:
        num = def->DoAnAttack(a->special,
                              getrandom(25 * a->slevel)
                              + getrandom(25 * a->slevel) + 2,
                              NUM_ATTACK_TYPES,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              0, 0);
        AddLine(a->name + " casts Banish Demons, banishing " +
                num + " demons.");
        break;
        
    case SPECIAL_EARTHQUAKE:
        num = def->DoAnAttack(a->special,
                              getrandom(50 * a->slevel) +
                              getrandom(50 * a->slevel) + 2,
                              ATTACK_COMBAT,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              0, 0);
        if (num == -1) {
            AddLine(a->name + " invokes a mighty Earthquake, "
                    "but it is deflected.");
        } else {
            AddLine(a->name + " invokes a mighty Earthquake, "
                    "killing " + num + ".");
        }
        break;

    case SPECIAL_LSTRIKE:
        num = def->DoAnAttack(a->special,
                              getrandom(30 * a->slevel)
                              + getrandom(30 * a->slevel) + 2,
                              ATTACK_WEATHER,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              0, 0);
        num2 = def->DoAnAttack(a->special,
                               getrandom(30 * a->slevel)
                               + getrandom(30 * a->slevel) + 2,
                               ATTACK_ENERGY,
                               a->slevel,
                               SPECIAL_FLAGS, SPECIAL_CLASS,
                               0, 0);
        if (num == -1 && num2 == -1) {
            AddLine(a->name + " unleashes a mighty lightning "
                    "strike, but it is deflected.");
        } else {
            if( num == -1 || num2 == -1 )
            {
                num++;
            }
            AddLine(a->name + " unleashes a mighty lightning "
                    "strike, killing " + (num + num2) + ".");
        }
        break;
       
		/* FOO */
    case SPECIAL_FIREBALL:
        num = def->DoAnAttack(a->special,
                              getrandom(a->slevel * 5) +
                              getrandom(a->slevel * 5) + 2,
                              ATTACK_ENERGY,
                              a->slevel,
                              SPECIAL_FLAGS,  SPECIAL_CLASS,
                              0, 0);
        if (num == -1) {
            AddLine(a->name + " shoots a Fireball, but it is "
                    "deflected.");
        } else {
            AddLine(a->name + " shoots a Fireball, killing " +
                    num + ".");
        }
        break;
	case SPECIAL_FIREBREATH:
		num = def->DoAnAttack(a->special,
				getrandom(a->slevel * 5) + getrandom(a->slevel * 5) + 2,
				ATTACK_ENERGY, a->slevel, SPECIAL_FLAGS, SPECIAL_CLASS,
				0, 0);
		if (num == -1) {
			AddLine(a->name + " breathes Fire, but it is deflected.");
		} else {
			AddLine(a->name + " breathes Fire, killing " + num + ".");
		}
		break;
	case SPECIAL_ICEBREATH:
		num = def->DoAnAttack(a->special,
				getrandom(a->slevel * 5) + getrandom(a->slevel * 5) + 2,
				ATTACK_ENERGY, a->slevel, SPECIAL_FLAGS, SPECIAL_CLASS,
				0, 0);
		if (num == -1) {
			AddLine(a->name + " breathes Ice, but it is deflected.");
		} else {
			AddLine(a->name + " breathes Ice, killing " + num + ".");
		}
		break;
    case SPECIAL_HELLFIRE:
        num = def->DoAnAttack(a->special,
                              getrandom(a->slevel * 25) +
                              getrandom(a->slevel * 25) + 2,
                              ATTACK_ENERGY,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              0, 0);
        if (num == -1) {
            AddLine(a->name + " blasts the enemy with Hellfire, but it is "
                    "deflected.");
        } else {
            AddLine(a->name + " blasts the enemy with Hellfire, killing " +
                    num + ".");
        }
        break;
        
    case SPECIAL_CAUSEFEAR:
        num = def->DoAnAttack(a->special,
                              getrandom(a->slevel * 10)
                              + getrandom(a->slevel * 10) + 2,
                              ATTACK_SPIRIT,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              EFFECT_FEAR, 0);
        if (num == -1) {
            AddLine(a->name + " attempts to strike fear into the "
                    "enemy, but the spell is deflected.");
        } else {
            AddLine(a->name + " strikes fear into the hearts "
                    "of " + num + " men.");
        }
        break;
        
    case SPECIAL_BLACK_WIND:
        num = def->DoAnAttack(a->special,
                              getrandom(a->slevel * 100) +
                              getrandom(a->slevel * 100) + 2,
                              ATTACK_SPIRIT,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              0, 0);
        if (num == -1) {
            AddLine(a->name + " attempts to summon the black wind, "
                    "but the spell is deflected.");
        } else {
            AddLine(a->name + " summons the black wind, killing " + num +
                    ".");
        }
        break;
        
    case SPECIAL_MINDBLAST:
        num = def->DoAnAttack(a->special,
                              getrandom(125) + getrandom(125) + 2,
                              ATTACK_SPIRIT,
                              a->slevel,
                              SPECIAL_FLAGS, SPECIAL_CLASS,
                              0, 0);
        if (num == -1) {
            AddLine(a->name + " attempts to blast the minds of the "
                    "enemy, but the spell is deflected.");
        } else {
            AddLine(a->name + " mind blasts " + num +
                    " to death.");
        }
        break;
        
    default:
        /* Probably a shield spell */
        break;
    }
}


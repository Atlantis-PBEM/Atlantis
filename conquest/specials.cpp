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
#include "rules.h"

void Soldier::SetupHealing()
{
    if (unit->type == U_MAGE)
    {
        int maglvl = unit->GetSkill(S_MAGICAL_HEALING);
        if (maglvl > 4)
        {
            healtype = HEAL_THREE;
            healing = 100;
            healitem = -1;
            return;
        }
        if (maglvl > 2)
        {
            healtype = HEAL_TWO;
            healing = 25;
            healitem = -1;
            return;
        }
        if (maglvl > 0)
        {
            healtype = HEAL_ONE;
            healing = 10;
            healitem = -1;
            return;
        }
    }

    healing = unit->GetSkill(S_HEALING) * Globals->HEALS_PER_MAN;
    if (healing)
    {
        healtype = HEAL_ONE;
        int herbs = unit->items.GetNum(I_HERBS);
        if (herbs < healing) healing = herbs;
        unit->items.SetNum(I_HERBS,herbs - healing);
        healitem = I_HERBS;
    }
}

int Army::CheckSpecialTarget(int special,int tar)
{
    switch (special)
    {
    case SPECIAL_EARTHQUAKE:
        if (soldiers[tar]->building && 
            soldiers[tar]->building != O_MFORTRESS) return 1;
        return 0;
    case SPECIAL_DISPEL_ILLUSIONS:
        if (ItemDefs[soldiers[tar]->race].type & IT_MONSTER &&
            ItemDefs[soldiers[tar]->race].index == MONSTER_ILLUSION)
        {
            return 1;
        }
        return 0;
    case SPECIAL_BANISH_DEMONS:
        {
            int itemtype = soldiers[tar]->race;
            if (itemtype == I_IMP || itemtype == I_DEMON ||
                itemtype == I_BALROG) return 1;
            return 0;
        }
    case SPECIAL_BANISH_UNDEAD:
        {
            int itemtype = soldiers[tar]->race;
            if (itemtype == I_SKELETON || itemtype == I_UNDEAD ||
                itemtype == I_LICH) return 1;
            return 0;
        }
    case SPECIAL_SUMMON_STORM:
        if (soldiers[tar]->HasEffect(EFFECT_STORM)) return 0;
        return 1;
    case SPECIAL_CAUSEFEAR:
        if (soldiers[tar]->HasEffect(EFFECT_FEAR)) return 0;
        if (ItemDefs[soldiers[tar]->race].type & IT_MONSTER) return 0;
        return 1;
    case SPECIAL_BLACK_WIND:
        {
            int itemtype = soldiers[tar]->race;
            if (itemtype == I_SKELETON || itemtype == I_UNDEAD ||
                itemtype == I_LICH || itemtype == I_IMP ||
                itemtype == I_DEMON || itemtype == I_BALROG)
            {
                return 0;
            }
            return 1;
        }
    }
    return 1;
}

void Battle::UpdateShields(Army *a)
{
    for (int i=0; i<a->notbehind; i++)
    {
        int shtype = -1;
        AString shdesc;
        if (a->soldiers[i]->special == SPECIAL_FORCE_SHIELD)
        {
            shtype = ATTACK_COMBAT;
            shdesc = "Force Shield";

            //
            // Note: Force shield gives a defensive bonus to the caster.
            // Be sure to only apply this bonus once!
            //
            if( a->round == 0 )
            {
                a->soldiers[ i ]->dskill[ ATTACK_COMBAT ] += 
                    a->soldiers[ i ]->slevel;
            }
        }
        if (a->soldiers[i]->special == SPECIAL_ENERGY_SHIELD)
        {
            shtype = ATTACK_ENERGY;
            shdesc = "Energy Shield";
        }
        if (a->soldiers[i]->special == SPECIAL_SPIRIT_SHIELD)
        {
            shtype = ATTACK_SPIRIT;
            shdesc = "Spirit Shield";
        }
        if (a->soldiers[i]->special == SPECIAL_CLEAR_SKIES)
        {
            shtype = ATTACK_WEATHER;
            shdesc = "Clear Skies";
        }

        if (shtype != -1)
        {
            Shield *sh = new Shield;
            sh->shieldtype = shtype;
            sh->shieldskill = a->soldiers[i]->slevel;
            a->shields.Add(sh);
            AddLine(*(a->soldiers[i]->unit->name) + " casts " + shdesc + ".");
        }
    }
}

void Battle::DoSpecialAttack( int round, 
                              Soldier *a,
                              Army *attackers,
                              Army *def, 
                              int behind )
{
    int num,num2;
    switch (a->special)
    {
    case -1:
        break;

    case SPECIAL_TORNADO:
        num = def->DoAnAttack( a->special,
                               getrandom(a->slevel * 25)
                               + getrandom(a->slevel * 25) + 2,
                               ATTACK_WEATHER,
                               a->slevel,
                               SPECIAL_FLAGS,
                               0);
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
                              SPECIAL_FLAGS,
                              EFFECT_STORM);
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
                              SPECIAL_FLAGS,
                              0);
        AddLine(a->name + " casts Dispel Illusions, dispelling " +
                num + " illusions.");
        break;
        
    case SPECIAL_BANISH_UNDEAD:
        num = def->DoAnAttack(a->special,
                              getrandom(25 * a->slevel)
                              + getrandom(25 * a->slevel) + 2,
                              NUM_ATTACK_TYPES,
                              a->slevel,
                              SPECIAL_FLAGS,
                              0);
        AddLine(a->name + " casts Banish Undead, banishing " +
                num + " undead.");
        break;
        
    case SPECIAL_BANISH_DEMONS:
        num = def->DoAnAttack(a->special,
                              getrandom(25 * a->slevel)
                              + getrandom(25 * a->slevel) + 2,
                              NUM_ATTACK_TYPES,
                              a->slevel,
                              SPECIAL_FLAGS,
                              0);
        AddLine(a->name + " casts Banish Demons, banishing " +
                num + " demons.");
        break;
        
    case SPECIAL_EARTHQUAKE:
        num = def->DoAnAttack(a->special,
                              getrandom(50 * a->slevel) +
                              getrandom(50 * a->slevel) + 2,
                              ATTACK_COMBAT,
                              a->slevel,
                              SPECIAL_FLAGS,
                              0);
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
                              SPECIAL_FLAGS,
                              0);
        num2 = def->DoAnAttack(a->special,
                               getrandom(30 * a->slevel)
                               + getrandom(30 * a->slevel) + 2,
                               ATTACK_ENERGY,
                               a->slevel,
                               SPECIAL_FLAGS,
                               0);
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
        
    case SPECIAL_FIREBALL:
        num = def->DoAnAttack(a->special,
                              getrandom(a->slevel * 5) +
                              getrandom(a->slevel * 5) + 2,
                              ATTACK_ENERGY,
                              a->slevel,
                              SPECIAL_FLAGS, 
                              0);
        if (num == -1) {
            AddLine(a->name + " shoots a Fireball, but it is "
                    "deflected.");
        } else {
            AddLine(a->name + " shoots a Fireball, killing " +
                    num + ".");
        }
        break;
        
    case SPECIAL_HELLFIRE:
        num = def->DoAnAttack(a->special,
                              getrandom(a->slevel * 25) +
                              getrandom(a->slevel * 25) + 2,
                              ATTACK_ENERGY,
                              a->slevel,
                              SPECIAL_FLAGS,
                              0);
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
                              SPECIAL_FLAGS,
                              EFFECT_FEAR);
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
                              SPECIAL_FLAGS, 
                              0);
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
                              SPECIAL_FLAGS,
                              0);
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


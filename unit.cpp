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
// Date        Person          Comments
// ----        ------          --------
// 2000/MAR/14 Larry Stanbery  Corrected logical flaw in creation of mages.
//                             Replaced specific skill bonus functions with
//                             generic function.
// 2001/Feb/18 Joseph Traub    Added support for Apprentices
// 2001/Feb/25 Joseph Traub    Added a flag preventing units from crossing
//                             water.
//

#include "unit.h"
#include "gamedata.h"

UnitId::UnitId() {
}

UnitId::~UnitId() {
}

AString UnitId::Print()
{
    if (unitnum) {
        return AString(unitnum);
    } else {
        if (faction) {
            return AString("faction ") + AString(faction) + " new " +
                AString(alias);
        } else {
            return AString("new ") + AString(alias);
        }
    }
}

UnitPtr *GetUnitList(AList *list, Unit *u) {
  forlist (list) {
    UnitPtr *p = (UnitPtr *) elem;
    if (p->ptr == u) return p;
  }
  return 0;
}

Unit::Unit()
{
    name = 0;
    describe = 0;
    num = 0;
    type = U_NORMAL;
    faction = 0;
    ClearOrders();
    alias = 0;
    guard = GUARD_NONE;
    reveal = REVEAL_NONE;
    flags = 0;
    combat = -1;
    object = 0;
}

Unit::Unit(int seq,Faction * f,int a)
{
    num = seq;
    type = U_NORMAL;
    name = new AString;
    describe = 0;
    *name = AString("Unit (") + num + ")";
    faction = f;
    alias = a;
    guard = 0;
    reveal = REVEAL_NONE;
    flags = 0;
    combat = -1;
    ClearOrders();
    object = 0;
}

Unit::~Unit() {
  if (monthorders) delete monthorders;
  if (attackorders) delete attackorders;
  if (stealorders) delete stealorders;
  if (name) delete name;
  if (describe) delete describe;
}

void Unit::SetMonFlags() {
  guard = GUARD_AVOID;
  SetFlag(FLAG_HOLDING,1);
}

void Unit::MakeWMon(char * monname,int mon,int num)
{
    AString * temp = new AString(monname);
    SetName(temp);
    
    type = U_WMON;
    items.SetNum(mon,num);
    SetMonFlags();
}

void Unit::Writeout( Aoutfile *s )
{
    s->PutStr(*name);
    if (describe) {
        s->PutStr(*describe);
    } else {
        s->PutStr("none");
    }
    s->PutInt(num);
    s->PutInt(type);
    s->PutInt(faction->num);
    s->PutInt(guard);
    s->PutInt(reveal);
    s->PutInt(flags);
    items.Writeout(s);
    skills.Writeout(s);
    s->PutInt(combat);
}

void Unit::Readin( Ainfile *s, AList *facs, ATL_VER v )
{
    name = s->GetStr();
    describe = s->GetStr();
    if (*describe == "none") {
        delete describe;
        describe = 0;
    }
    num = s->GetInt();
    type = s->GetInt();
    int i = s->GetInt();
    faction = GetFaction(facs,i);
    guard = s->GetInt();
    reveal = s->GetInt();
    flags = s->GetInt();
    items.Readin(s);
    skills.Readin(s);
    combat = s->GetInt();
}

AString Unit::MageReport()
{
	AString temp;

	if (combat != -1) {
		temp = AString(". Combat spell: ") + SkillStrs(combat);
	}
	return temp;
}

AString Unit::StudyableSkills()
{
	AString temp;
	int j=0;

	for (int i=0; i<NSKILLS; i++) {
		if(SkillDefs[i].depends[0].skill != -1) {
			if (CanStudy(i)) {
				if (j) {
					temp += ", ";
				} else {
					temp += ". Can Study: ";
					j=1;
				}
				temp += SkillStrs(i);
			}
		}
	}
	return temp;
}

AString Unit::GetName(int obs)
{
	AString ret = *name;
	int stealth = GetSkill(S_STEALTH);
	if(reveal == REVEAL_FACTION || obs > stealth) {
		ret += ", ";
		ret += *faction->name;
	}
    return ret;
}

void Unit::WriteReport(Areport * f,int obs,int truesight,int detfac,
		       int autosee)
{
    int stealth = GetSkill(S_STEALTH);
	if (obs==-1) {
		/* The unit belongs to the Faction writing the report */
		obs = 2;
	} else {
		if (obs < stealth) {
			/* The unit cannot be seen */
			if (reveal == REVEAL_FACTION) {
				obs = 1;
			} else {
				if (guard == GUARD_GUARD || reveal == REVEAL_UNIT || autosee) {
					obs = 0;
				} else {
					return;
				}
			}
		} else {
			if (obs == stealth) {
				/* Can see unit, but not Faction */
				if (reveal == REVEAL_FACTION) {
					obs = 1;
				} else {
					obs = 0;
				}
			} else { 
				/* Can see unit and Faction */
				obs = 1;
			}
		}
	}

    /* Setup True Sight */
    if (obs == 2) {
        truesight = 1;
    } else {
        if (GetSkill(S_ILLUSION) > truesight) {
            truesight = 0;
        } else {
            truesight = 1;
        }
    }

    if (detfac && obs != 2) obs = 1;
  
  /* Write the report */
    AString temp;
    if (obs == 2) {
        temp += AString("* ") + *name;
    } else {
        temp += AString("- ") + *name;
    }
  
    if (guard == GUARD_GUARD) temp += ", on guard";
    if (obs > 0) {
        temp += AString(", ") + *faction->name;
        if (guard == GUARD_AVOID) temp += ", avoiding";
        if (GetFlag(FLAG_BEHIND)) temp += ", behind";
    }
  
    if (obs == 2) {
        if (reveal == REVEAL_UNIT) temp += ", revealing unit";
        if (reveal == REVEAL_FACTION) temp += ", revealing faction";
        if (GetFlag(FLAG_HOLDING)) temp += ", holding";
        if (GetFlag(FLAG_AUTOTAX)) temp += ", taxing";
        if (GetFlag(FLAG_NOAID)) temp += ", receiving no aid";
        if (GetFlag(FLAG_CONSUMING_UNIT)) temp += ", consuming unit's food";
        if (GetFlag(FLAG_CONSUMING_FACTION)) 
            temp += ", consuming faction's food";
		if (GetFlag(FLAG_NOCROSS_WATER)) temp += ", won't cross water";
		if (GetFlag(FLAG_NOSPOILS)) temp += ", no battle spoils";
    }
  
    temp += items.Report(obs,truesight,0);
    if (obs == 2) {
        temp += ". Skills: ";
        temp += skills.Report(GetMen());
    }
  
    if (obs == 2 && (type == U_MAGE || type == U_GUARDMAGE)) {
        temp += MageReport();
    }
	if(obs == 2) {
		temp += StudyableSkills();
	}
  
    if (describe) {
        temp += AString("; ") + *describe;
    }
    temp += ".";
    f->PutStr(temp);
}

AString Unit::TemplateReport() {
  /* Write the report */
  AString temp;
  temp = *name;
  
  if (guard == GUARD_GUARD) temp += ", on guard";
  if (guard == GUARD_AVOID) temp += ", avoiding";
  if (GetFlag(FLAG_BEHIND)) temp += ", behind";
  if (reveal == REVEAL_UNIT) temp += ", revealing unit";
  if (reveal == REVEAL_FACTION) temp += ", revealing faction";
  if (GetFlag(FLAG_HOLDING)) temp += ", holding";
  if (GetFlag(FLAG_AUTOTAX)) temp += ", taxing";
  if (GetFlag(FLAG_NOAID)) temp += ", receiving no aid";
  if (GetFlag(FLAG_CONSUMING_UNIT)) temp += ", consuming unit's food";
  if (GetFlag(FLAG_CONSUMING_FACTION)) 
    temp += ", consuming faction's food";
  if (GetFlag(FLAG_NOCROSS_WATER)) temp += ", won't cross water";
  if (GetFlag(FLAG_NOSPOILS)) temp += ", no battle spoils";
  
  temp += items.Report(2,1,0);
  temp += ". Skills: ";
  temp += skills.Report(GetMen());
  
  if (type == U_MAGE || type == U_GUARDMAGE) {
    temp += MageReport();
  }
  temp += StudyableSkills();
  
  if (describe) {
    temp += AString("; ") + *describe;
  }
  temp += ".";
  return temp;
}

AString * Unit::BattleReport(int obs)
{
  AString * temp = new AString("");
  if(Globals->BATTLE_FACTION_INFO)
	  *temp += GetName(obs);
  else
	  *temp += *name;

  if (GetFlag(FLAG_BEHIND)) *temp += ", behind";
  
  *temp += items.BattleReport();
  
  int lvl;
  lvl = GetRealSkill(S_TACTICS);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_TACTICS].name;
    *temp += " ";
    *temp += lvl;
  }
	
  lvl = GetRealSkill(S_COMBAT);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_COMBAT].name;
    *temp += " ";
    *temp += lvl;
  }
	
  lvl = GetRealSkill(S_LONGBOW);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_LONGBOW].name;
    *temp += " ";
    *temp += lvl;
  }
	
  lvl = GetRealSkill(S_CROSSBOW);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_CROSSBOW].name;
    *temp += " ";
    *temp += lvl;
  }
	
  lvl = GetRealSkill(S_RIDING);
  if (lvl) {
    *temp += ", ";
    *temp += SkillDefs[S_RIDING].name;
    *temp += " ";
    *temp += lvl;
  }
  
  if (describe) {
    *temp += "; ";
    *temp += *describe;
  }
	
  *temp += ".";
  return temp;
}

void Unit::ClearOrders() {
  canattack = 1;
  nomove = 0;
  enter = 0;
  destroy = 0;
  attackorders = 0;
  stealorders = 0;
  promote = 0;
  taxing = TAX_NONE;
  advancefrom = 0;
  movepoints = 0;
  monthorders = 0;
  castorders = 0;
  teleportorders = 0;
}

void Unit::ClearCastOrders() {
  if (castorders) delete castorders;
  castorders = 0;
  if (teleportorders) delete teleportorders;
  teleportorders = 0;
}

void Unit::DefaultOrders(Object * obj)
{
	ClearOrders();
	if (type == U_WMON) {
		if (ObjectDefs[obj->type].monster == -1) {
			MoveOrder * o = new MoveOrder;
			MoveDir * d = new MoveDir;
			d->dir = getrandom(NDIRS);
			o->dirs.Add(d);
			monthorders = o;
		}
	} else if(type == U_GUARD) {
		if (guard != GUARD_GUARD)
			guard = GUARD_SET;
	} else if(type == U_GUARDMAGE) {
		combat = S_FIRE;
	} else{
		if(obj->region->type != R_NEXUS) {
			if(GetFlag(FLAG_AUTOTAX) && Globals->TAX_PILLAGE_MONTH_LONG &&
					Taxers()) {
				taxing = TAX_AUTO;
			} else {
				ProduceOrder * order = new ProduceOrder;
				order->skill = -1;
				order->item = I_SILVER;
				monthorders = order;
			}
		}
	}
}

void Unit::PostTurn(ARegion *r)
{
    if (type == U_WMON)
    {
        forlist(&items) {
            Item *i = (Item *) elem;
            if( !( ItemDefs[i->type].type & IT_MONSTER ))
            {
                items.Remove(i);
                delete i;
            }
        }
    }
}

void Unit::SetName(AString *s) {
  if (s) {
    AString * newname = s->getlegal();
    if (!newname) {
      delete s;
      return;
    }
    *newname += AString(" (") + num + ")";
    delete s;
    delete name;
    name = newname;
  }
}

void Unit::SetDescribe(AString * s) {
  if (describe) delete describe;
  if (s) {
    AString * newname = s->getlegal();
    delete s;
    describe = newname;
  } else
    describe = 0;
}

int Unit::IsAlive()
{
    if( type == U_MAGE || type == U_APPRENTICE )
    {
        return( GetMen() );
    }
    else
    {
        forlist(&items) {
            Item * i = (Item *) elem;
            if (IsSoldier(i->type) && i->num > 0)
                return 1;
        }
    }
    return 0;
}

void Unit::SetMen(int t,int n)
{
	if (ItemDefs[t].type & IT_MAN) {
		int oldmen = GetMen();
		items.SetNum(t,n);
		int newmen = GetMen();
		if (newmen < oldmen) {
			delete skills.Split(oldmen, oldmen - newmen);
		}
	} else {
		/* This is probably a monster in this case */
		items.SetNum(t,n);
	}
}

int Unit::GetMen(int t)
{
	return items.GetNum(t);
}

int Unit::GetMons()
{
	int n=0;
	forlist(&items) {
		Item * i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			n += i->num;
		}
	}
	return n;
}

int Unit::GetMen()
{
	int n = 0;
	forlist(&items) {
		Item * i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MAN) {
			n += i->num;
		}
	}
	return n;
}

int Unit::GetSoldiers()
{
	int n = 0;
	forlist(&items) {
		Item * i = (Item *) elem;
		if (IsSoldier(i->type)) n+=i->num;
	}

	return n;
}

void Unit::SetMoney(int n)
{
	items.SetNum(I_SILVER,n);
}

int Unit::GetMoney()
{
	return items.GetNum(I_SILVER);
}

int Unit::GetTactics()
{
	int retval = GetRealSkill(S_TACTICS);

	forlist(&items) {
		Item * i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			int temp = MonDefs[(ItemDefs[i->type].index)].tactics;
			if (temp > retval) retval = temp;
		}
	}

	return retval;
}

int Unit::GetObservation()
{
	int retval = GetRealSkill(S_OBSERVATION);
	// LLS
	int bonus = GetSkillBonus(S_OBSERVATION);
	retval += bonus;

	forlist(&items) {
		Item * i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			int temp = MonDefs[ItemDefs[i->type].index].obs;
			if (temp > retval) retval = temp;
		}
	}

	return retval;
}

int Unit::GetAttackRiding()
{
	int riding = 0;
	if (type == U_WMON) {
		forlist(&items) {
			Item *i = (Item *) elem;
			if (ItemDefs[i->type].type & IT_MONSTER) {
				if (ItemDefs[i->type].fly) {
					return 5;
				}
				if (ItemDefs[i->type].ride) riding = 3;
			}
		}
		return riding;
	} else {
		riding = GetSkill(S_RIDING);
		int lowriding = 0;
		forlist(&items) {
			Item *i = (Item *) elem;
			if (ItemDefs[i->type].fly - ItemDefs[i->type].weight >= 10) {
				return riding;
			}
			if (ItemDefs[i->type].ride - ItemDefs[i->type].weight) {
				if (riding <= 3) return riding;
				lowriding = 3;
			}
		}
		return lowriding;
	}
}

int Unit::GetDefenseRiding() 
{
	if (guard == GUARD_GUARD) return 0;

	int riding = 0;
	int weight = Weight();

	if (CanFly(weight)) {
		riding = 5;
	} else {
		if (CanRide(weight)) riding = 3;
	}

	if (GetMen()) {
		int manriding = GetSkill(S_RIDING);
		if (manriding < riding) return manriding;
	}

	return riding;
}

int Unit::GetStealth()
{
	int monstealth = 100;
	int manstealth = 100;

	if (guard == GUARD_GUARD) return 0;

	forlist(&items) {
		Item * i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			int temp = MonDefs[ItemDefs[i->type].index].stealth;
			if (temp < monstealth) monstealth = temp;
		} else {
			if (ItemDefs[i->type].type & IT_MAN) {
				if (manstealth == 100) {
					manstealth = GetRealSkill(S_STEALTH);
				}
			}
		}
	}

	// LLS
	manstealth += GetSkillBonus(S_STEALTH);

	if (monstealth < manstealth) return monstealth;
	return manstealth;
}

int Unit::GetEntertainment()
{
	int level = GetRealSkill(S_ENTERTAINMENT);
	int level2 = 5 * GetRealSkill(S_PHANTASMAL_ENTERTAINMENT);
	return (level > level2 ? level : level2);
}

int Unit::GetSkill(int sk)
{
	if (sk == S_TACTICS) return GetTactics();
	if (sk == S_STEALTH) return GetStealth();
	if (sk == S_OBSERVATION) return GetObservation();
	if (sk == S_ENTERTAINMENT) return GetEntertainment();
	int retval = GetRealSkill(sk);
	return retval;
}

void Unit::SetSkill(int sk,int level)
{
	skills.SetDays(sk,GetDaysByLevel(level) * GetMen());
}

int Unit::GetRealSkill(int sk)
{
	if (GetMen()) {
		return GetLevelByDays(skills.GetDays(sk)/GetMen());
	} else {
		return 0;
	}
}

void Unit::ForgetSkill(int sk)
{
	skills.SetDays(sk,0);
	if (type == U_MAGE) {
		forlist(&skills) {
			Skill * s = (Skill *) elem;
			if( SkillDefs[s->type].flags & SkillType::MAGIC ) {
				return;
			}
		}
		type = U_NORMAL;
	}
	if(type == U_APPRENTICE) {
		forlist(&skills) {
			Skill * s = (Skill *) elem;
			if( SkillDefs[s->type].flags & SkillType::APPRENTICE) {
				return;
			}
		}
		type = U_NORMAL;
	}
}

int Unit::CheckDepend(int lev, SkillDepend &dep)
{
	int temp = GetRealSkill(dep.skill);
	if(temp < dep.level) return 0;
	if(lev >= temp) return 0;
	return 1;
}

int Unit::CanStudy(int sk)
{
	int curlev = GetRealSkill(sk);

	if(SkillDefs[sk].flags & SkillType::DISABLED) return 0;

	unsigned int c;
	for(c = 0; c < sizeof(SkillDefs[sk].depends)/sizeof(SkillDepend); c++) {
		if(SkillDefs[sk].depends[c].skill == -1) return 1;
		if(SkillDefs[SkillDefs[sk].depends[c].skill].flags&SkillType::DISABLED)
			continue;
		if(!CheckDepend(curlev, SkillDefs[sk].depends[c])) return 0;
	}
	return 1;
}

int Unit::Study(int sk,int days)
{
	if( Globals->SKILL_LIMIT_NONLEADERS && !IsLeader() ) {
		if (skills.Num()) {
			Skill * s = (Skill *) skills.First();
			if (s->type != sk) {
				Error("Can know only 1 skill.");
				return 0;
			}
		}
	}
	if (!CanStudy(sk)) {
		Error("Doesn't have the pre-requisite skills to study that.");
		return 0;
	}

	skills.SetDays(sk,skills.GetDays(sk) + days);
	AdjustSkills();

	/* Check to see if we need to show a skill report */
	int lvl = GetRealSkill(sk);
	if (lvl > faction->skills.GetDays(sk)) {
		faction->skills.SetDays(sk,lvl);
		faction->shows.Add(new ShowSkill(sk,lvl));
	}
	return 1;
}

int Unit::IsLeader()
{
	if (GetMen(I_LEADERS)) return 1;
	return 0;
}

int Unit::IsNormal()
{
	if (GetMen() && !IsLeader()) return 1;
	return 0;
}

void Unit::AdjustSkills()
{
	//
	// First, is the unit a leader?
	//
	if( IsLeader() ) {
		//
		// Unit is all leaders: Make sure no skills are > max
		//
		forlist(&skills) {
			Skill * s = (Skill *) elem;
			if (GetRealSkill(s->type) >= SkillMax(s->type,I_LEADERS)) {
				s->days = GetDaysByLevel(SkillMax(s->type,I_LEADERS)) *
					GetMen();
			}
		}
	} else {
		if( Globals->SKILL_LIMIT_NONLEADERS ) {
			//
			// Not a leader, can only know 1 skill
			//
			if (skills.Num() > 1) {
				//
				// Find highest skill, eliminate others
				//
				unsigned int max = 0;
				Skill * maxskill = 0;
				forlist(&skills) {
					Skill * s = (Skill *) elem;
					if (s->days > max) {
						max = s->days;
						maxskill = s;
					}
				}
				{
					forlist(&skills) {
						Skill * s = (Skill *) elem;
						if (s != maxskill) {
							skills.Remove(s);
							delete s;
						}
					}
				}
			}
		}

		//
		// Limit remaining skills to max
		//
		forlist( &skills ) {
			Skill *theskill = (Skill *) elem;
			int max = 100;
			forlist(&items) {
				Item * i = (Item *) elem;
				if (ItemDefs[i->type].type & IT_MAN) {
					if (SkillMax(theskill->type,i->type) < max) {
						max = SkillMax(theskill->type,i->type);
					}
				}
			}
			if (GetRealSkill(theskill->type) >= max) {
				theskill->days = GetDaysByLevel(max) * GetMen();
			}
		}
	}
}

int Unit::MaintCost()
{
    int retval = 0;
	int levels = 0;
    if (type == U_WMON || type == U_GUARD || type == U_GUARDMAGE) return 0;

	int leaders = GetMen(I_LEADERS);
	if(leaders < 0) leaders = 0;
    int nonleaders = GetMen() - leaders;
    if (nonleaders < 0) nonleaders = 0;

	switch(Globals->MULTIPLIER_USE) {
		case GameDefs::MULT_NONE:
			retval += Globals->MAINTENANCE_COST * nonleaders;
			retval += Globals->LEADER_COST * leaders;
			break;
		case GameDefs::MULT_MAGES:
			if(type == U_MAGE) {
				retval += SkillLevels() * Globals->MAINTENANCE_MULTIPLIER;
			} else {
				retval += Globals->MAINTENANCE_COST * nonleaders;
				retval += Globals->LEADER_COST * leaders;
			}
			break;
		case GameDefs::MULT_LEADERS:
			levels = SkillLevels();
			if(levels)
				retval += levels * Globals->MAINTENANCE_MULTIPLIER * leaders;
			else
				retval += Globals->LEADER_COST * leaders;
			retval += Globals->MAINTENANCE_COST * nonleaders;
			break;
		case GameDefs::MULT_ALL:
			levels = SkillLevels();
			if(levels) {
				retval += levels*Globals->MAINTENANCE_MULTIPLIER*leaders;
				retval += levels*Globals->MAINTENANCE_MULTIPLIER*nonleaders;
			} else {
				retval += Globals->LEADER_COST * leaders;
				retval += Globals->MAINTENANCE_COST * nonleaders;
			}
	}
  
    return retval;
}

void Unit::Short(int needed)
{
    int n = 0;

	switch(Globals->SKILL_STARVATION) {
		case GameDefs::STARVE_MAGES:
			if(type == U_MAGE)
				SkillStarvation();
			return;
		case GameDefs::STARVE_LEADERS:
			if(GetMen(I_LEADERS))
				SkillStarvation();
			return;
		case GameDefs::STARVE_ALL:
			SkillStarvation();
			return;
	}

	if(!needed) return;
	
    for (int i = 0; i<= NITEMS; i++)
    {
        if( !( ItemDefs[ i ].type & IT_MAN ))
        {
            //
            // Only men need sustenance.
            //
            continue;
        }

        if( i == I_LEADERS )
        {
            //
            // Don't starve leaders just yet.
            //
            continue;
        }

        while (GetMen(i))
        {
            if (getrandom(100) < Globals->STARVE_PERCENT)
            {
                SetMen(i,GetMen(i) - 1);
                n++;
            }
            needed -= Globals->MAINTENANCE_COST;
            if (needed <= 0)
            {
                if (n)
                {
                    Error(AString(n) + " starve to death.");
                }
                return;
            }
        }
    }
	
    while (GetMen(I_LEADERS))
    {
        if (getrandom(100) < Globals->STARVE_PERCENT)
        {
            SetMen(I_LEADERS,GetMen(I_LEADERS) - 1);
            n++;
        }
        needed -= Globals->LEADER_COST;
        if (needed <= 0)
        {
            if (n)
            {
                Error(AString(n) + " starve to death.");
            }
            return;
        }
    }
}

int Unit::Weight() {
  int retval = items.Weight();
  return retval;
}

int Unit::CanFly(int weight)
{
    int cap = 0;
    forlist(&items) {
        Item * i = (Item *) elem;
        if (i->type == I_LEADERS) {
        }
        cap += ItemDefs[i->type].fly * i->num;
    }
    
    if (cap >= weight) return 1;
    return 0;
}

int Unit::CanReallySwim() {
  int cap = 0;
  forlist(&items) {
    Item * i = (Item *) elem;
    cap += ItemDefs[i->type].swim * i->num;
  }
	
  if (cap >= items.Weight()) return 1;
  return 0;
}

int Unit::CanSwim()
{
	if(this->CanReallySwim())
		return 1;
	if((Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) && this->CanFly())
		return 1;
	return 0;
}

int Unit::CanFly() {
  int weight = items.Weight();
  return CanFly(weight);
}

int Unit::CanRide(int weight) {
  int cap = 0;
  forlist(&items) {
    Item * i = (Item *) elem;
    cap += ItemDefs[i->type].ride * i->num;
  }
  
  if (cap >= weight) return 1;
  return 0;
}

int Unit::CanWalk(int weight)
{
    int cap = 0;
    forlist(&items) {
        Item * i = (Item *) elem;
        cap += ItemDefs[i->type].walk * i->num;
		if(ItemDefs[i->type].hitchItem != -1) {
			int hitch = ItemDefs[i->type].hitchItem;
			if(!(ItemDefs[hitch].flags & ItemType::DISABLED)) {
				int hitches = items.GetNum(hitch);
				int hitched = i->num;
				if(i->num > hitches ) hitched = hitches;
				cap += hitches * ItemDefs[i->type].hitchwalk;
			}
		}
    }
	
    if (cap >= weight) return 1;
    return 0;
}
	
int Unit::MoveType() {
  int weight = items.Weight();
  if (CanFly(weight)) return M_FLY;
  if (CanRide(weight)) return M_RIDE;
  if (CanWalk(weight)) return M_WALK;
  return M_NONE;
}

int Unit::CalcMovePoints()
{
	switch (MoveType()) {
		case M_NONE:
			return 0;
		case M_WALK:
			return Globals->FOOT_SPEED;
		case M_RIDE:
			return Globals->HORSE_SPEED;
		case M_FLY:
			if (GetSkill(S_SUMMON_WIND)) return Globals->FLY_SPEED + 2;
			return Globals->FLY_SPEED;
	}
	return 0;
}

int Unit::CanMoveTo(ARegion * r1,ARegion * r2)
{
    if (r1 == r2) return 1;

    int exit = 0;	
    int i;
    for ( i=0; i<NDIRS; i++) {
        if (r1->neighbors[i] == r2) {
            exit = 1;
            break;
        }
    }
    if (!exit) return 0;
    exit = 0;
    for (i=0; i<NDIRS; i++) {
        if (r2->neighbors[i] == r1) {
            exit = 1;
            break;
        }
    }
    if (!exit) return 0;
    
    int mt = MoveType();
    if ((r1->type == R_OCEAN || r2->type == R_OCEAN) &&
		(!CanSwim() || GetFlag(FLAG_NOCROSS_WATER)))
        return 0;
    int mp = CalcMovePoints() - movepoints;
    if (mp < (r2->MoveCost(mt))) return 0;
    return 1;
}

int Unit::CanCatch(ARegion *r,Unit *u) {
  return faction->CanCatch(r,u);
}

int Unit::CanSee(ARegion * r,Unit * u) {
  return faction->CanSee(r,u);
}

int Unit::GetAttitude(ARegion * r,Unit * u) {
  if (faction == u->faction) return A_ALLY;
  int att = faction->GetAttitude(u->faction->num);
  if (att >= A_FRIENDLY && att >= faction->defaultattitude) return att;
  
  if (CanSee(r,u) == 2) {
    return att;
  } else {
    return faction->defaultattitude;
  }
}

int Unit::Hostile()
{
	if (type != U_WMON) return 0;
	int retval = 0;
	forlist(&items) {
		Item * i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			int hos = MonDefs[ItemDefs[i->type].index].hostile;
			if (hos > retval) retval = hos;
		}
	}
	return retval;
}

int Unit::Forbids(ARegion * r,Unit * u) {
  if (guard != GUARD_GUARD) return 0;
  if (!IsAlive()) return 0;
  if (!CanSee(r,u)) return 0;
  if (!CanCatch(r,u)) return 0;
  if (GetAttitude(r,u) < A_NEUTRAL) return 1;
  return 0;
}

int Unit::Taxers()
{
	int totalMen = GetMen();

	if( !totalMen ) {
		return( 0 );
	}

	if (GetSkill(S_COMBAT)) {
		return( totalMen );
	}

	int numNoWeapons = totalMen;
	int numNoFootWeapons = totalMen;
	int numMounts = 0;
	int numMountedWeapons = 0;
	int numUsableMounted = 0;
	int weaponType;
	for( weaponType = 1; weaponType < NUMWEAPONS; weaponType++ ) {
		WeaponType *pWep = &( WeaponDefs[ weaponType ]);

		//
		// Here's a weapon to look for
		//
		forlist( &items ) {
			Item *pItem = (Item *) elem;
			if( !pItem->num ) {
				continue;
			}

			if( ItemDefs[ pItem->type ].type & IT_MOUNT ) {
				// don't bother checking skill, that's done later
				numMounts += pItem->num;
				numUsableMounted = numMountedWeapons;
				if (numUsableMounted > numMounts) numUsableMounted = numMounts;
				numNoWeapons = numNoFootWeapons - numUsableMounted;
				continue;
			}

			if( !( ItemDefs[ pItem->type ].type & IT_WEAPON )) {
				continue;
			}

			if( ItemDefs[ pItem->type ].index != weaponType ) {
				continue;
			}

			if (CanUseWeapon(pWep) == -1) continue;
			//
			// OK, the unit has the skill to use this weapon,
			// or no skill is required
			//

			if (pWep->flags & WeaponType::NOFOOT) {
				// Check for mounted only weapons
				numMountedWeapons += pItem->num;
				numUsableMounted = numMountedWeapons;
				if (numUsableMounted > numMounts) numUsableMounted = numMounts;
				numNoWeapons = numNoFootWeapons - numUsableMounted;
				if( numNoWeapons <= 0) {
					return( totalMen );
				}
				continue;
			}

			numNoFootWeapons -= pItem->num;
			numNoWeapons = numNoFootWeapons - numUsableMounted;
			if( numNoWeapons <= 0 ) {
				return( totalMen );
			}
		}
	}

	return( totalMen - numNoWeapons );
}

int Unit::GetFlag(int x)
{
	return (flags & x);
}

void Unit::SetFlag(int x,int val)
{
	if (val) {
		flags = flags | x;
	} else {
		if (flags & x) flags -= x;
	}
}

void Unit::CopyFlags(Unit * x)
{
	flags = x->flags;
	if (x->guard != GUARD_SET && x->guard != GUARD_ADVANCE) {
		guard = x->guard;
	} else {
		guard = GUARD_NONE;
	}
	reveal = x->reveal;
}

int Unit::GetBattleItem( int batType, int index )
{
    forlist( &items ) {
        Item *pItem = (Item *) elem;
        if( !pItem->num )
        {
            continue;
        }

        if( ItemDefs[ pItem->type ].type & batType
            && ItemDefs[ pItem->type ].index == index ) {
            //
            // We found it
            //
            int retval = pItem->type;
            // Let's do this the "right" way
            items.SetNum(retval, pItem->num-1);
            return( retval );
        }
    }

    return( -1 );
}

void Unit::MoveUnit( Object *toobj )
{
	if( object ) {
		object->units.Remove( this );
    }
    object = toobj;
    if( object ) {
        object->units.Add( this );
    }
}

void Unit::Event(const AString & s)
{
	AString temp = *name + ": " + s;
	faction->Event(temp);
}

void Unit::Error(const AString & s)
{
	AString temp = *name + ": " + s;
	faction->Error(temp);
}

int Unit::GetSkillBonus( int sk )
{
	int bonus = 0;
	switch( sk ) {
		case S_OBSERVATION:
			if (GetMen()) {
				bonus = (GetSkill(S_TRUE_SEEING) + 1) / 2;
			}
			if ((bonus != 3) && GetMen() && items.GetNum(I_AMULETOFTS)) {
				bonus = 2;
			}
		   	break;
		case S_STEALTH:
			if (GetFlag(FLAG_INVIS) || GetMen() <= items.GetNum(I_RINGOFI)) {
				bonus = 3;
			}
			break;
		default:
			bonus = 0;
			break;
	}
	return bonus;
}

int Unit::GetProductionBonus( int item )
{
	int bonus = 0;
	if (ItemDefs[item].mult_item != -1) {
		bonus = items.GetNum(ItemDefs[item].mult_item);
	} else {
		bonus = GetMen();
	}
	if (bonus > GetMen()) bonus = GetMen();
	return bonus * ItemDefs[item].mult_val;
}

int Unit::SkillLevels()
{
	int levels = 0;
	forlist(&skills) {
		Skill *s = (Skill *)elem;
		levels += GetLevelByDays(s->days);
	}
	return levels;
}

Skill *Unit::GetSkillObject(int sk)
{
	forlist(&skills) {
		Skill *s = (Skill *)elem;
		if(s->type == sk)
			return s;
	}
	return NULL;
}

void Unit::SkillStarvation()
{
	int can_forget[NSKILLS];
	int count = 0;
	int i;
	for(i = 0; i < NSKILLS; i++) {
		if(SkillDefs[i].flags & SkillType::DISABLED) {
			can_forget[i] = 0;
			continue;
		}
		if(GetSkillObject(i)) {
			can_forget[i] = 1;
			count++;
		} else {
			can_forget[i] = 0;
		}
	}
	for(i = 0; i < NSKILLS; i++) {
		if(SkillDefs[i].flags & SkillType::DISABLED) continue;
		Skill *si = GetSkillObject(i);
		for(int j=0; j < NSKILLS; j++) {
			if(SkillDefs[j].flags & SkillType::DISABLED) continue;
			Skill *sj = GetSkillObject(j);
			int dependancy_level = 0;
			unsigned int c;
			for(c=0;c < sizeof(SkillDefs[i].depends)/sizeof(SkillDepend);c++) {
				if(SkillDefs[i].depends[c].skill == j) {
					dependancy_level = SkillDefs[i].depends[c].level;
					break;
				}
			}
			if(dependancy_level > 0) {
				if(GetLevelByDays(sj->days) == GetLevelByDays(si->days)) {
					can_forget[j] = 0;
					count--;
				}
			}
		}
	}
	if(!count) {
		forlist(&items) {
			Item *i = (Item *)elem;
			if(ItemDefs[i->type].type & IT_MAN) {
				count += items.GetNum(i->type);
				items.SetNum(i->type, 0);
			}
		}
		AString temp = AString(count) + " starve to death.";
		Error(temp);
		return;
	}
	count = getrandom(count)+1;
	for(i = 0; i < NSKILLS; i++) {
		if(can_forget[i]) {
			if(--count == 0) {
				Skill *s = GetSkillObject(i);
				AString temp = AString("Starves and forgets one level of ")+
					SkillDefs[i].name + ".";
				Error(temp);
				switch(GetLevelByDays(s->days)) {
					case 1:
						s->days -= 30;
						if(s->days <= 0)
							ForgetSkill(i);
						break;
					case 2:
						s->days -= 60;
						break;
					case 3:
						s->days -= 90;
						break;
					case 4:
						s->days -= 120;
						break;
					case 5:
						s->days -= 150;
						break;
				}
			}
		}
	}
	return;
}

int Unit::CanUseWeapon(WeaponType *pWep, int riding)
{
	if (riding == -1) {
		if( pWep->flags & WeaponType::NOFOOT) return -1;
	} else {
		if( pWep->flags & WeaponType::NOMOUNT) return -1;
	}
	return CanUseWeapon(pWep);
}

int Unit::CanUseWeapon(WeaponType *pWep)
{
	// we don't care in this case, their combat skill will be used.
	if ( !(pWep->flags & WeaponType::NEEDSKILL) ) return 0;

	int baseSkillLevel = 0;
	int tempSkillLevel = 0;
	if( pWep->baseSkill != -1 ) {
		baseSkillLevel = GetSkill( pWep->baseSkill );
	}

	if( pWep->orSkill != -1 ) {
		tempSkillLevel = GetSkill( pWep->orSkill );
	}

	if( tempSkillLevel > baseSkillLevel ) {
		baseSkillLevel = tempSkillLevel;
	}

	if( pWep->flags & WeaponType::NEEDSKILL && !baseSkillLevel ) return -1;

	return baseSkillLevel;
}

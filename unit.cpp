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

#include "unit.h"
#include "gamedata.h"

UnitId::UnitId()
{
}

UnitId::~UnitId()
{
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

UnitPtr *GetUnitList(AList *list, Unit *u)
{
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
	formfaction = 0;
	alias = 0;
	guard = GUARD_NONE;
	reveal = REVEAL_NONE;
	flags = FLAG_NOCROSS_WATER;
	combat = -1;
	for(int i = 0; i < MAX_READY; i++) {
		readyWeapon[i] = -1;
		readyArmor[i] = -1;
	}
	readyItem = -1;
	object = 0;
	attackorders = NULL;
	evictorders = NULL;
	stealorders = NULL;
	monthorders = NULL;
	castorders = NULL;
	teleportorders = NULL;
	inTurnBlock = 0;
	presentTaxing = 0;
	presentMonthOrders = NULL;
	former = NULL;
	free = 0;
	practiced = 0;
	ClearOrders();
}

Unit::Unit(int seq, Faction *f, int a)
{
	num = seq;
	type = U_NORMAL;
	name = new AString;
	describe = 0;
	*name = AString("Unit (") + num + ")";
	faction = f;
	formfaction = f;
	alias = a;
	guard = 0;
	reveal = REVEAL_NONE;
	flags = FLAG_NOCROSS_WATER;
	combat = -1;
	for(int i = 0; i < MAX_READY; i++) {
		readyWeapon[i] = -1;
		readyArmor[i] = -1;
	}
	readyItem = -1;
	object = 0;
	attackorders = NULL;
	evictorders = NULL;
	stealorders = NULL;
	monthorders = NULL;
	castorders = NULL;
	teleportorders = NULL;
	inTurnBlock = 0;
	presentTaxing = 0;
	presentMonthOrders = NULL;
	former = NULL;
	free = 0;
	ClearOrders();
}

Unit::~Unit()
{
	if (monthorders) delete monthorders;
	if (presentMonthOrders) delete presentMonthOrders;
	if (attackorders) delete attackorders;
	if (stealorders) delete stealorders;
	if (name) delete name;
	if (describe) delete describe;
}

void Unit::SetMonFlags()
{
	guard = GUARD_AVOID;
	SetFlag(FLAG_HOLDING, 1);
}

void Unit::MakeWMon(char *monname, int mon, int num)
{
	AString *temp = new AString(monname);
	SetName(temp);

	type = U_WMON;
	items.SetNum(mon, num);
	SetMonFlags();
}

void Unit::Writeout(Aoutfile *s)
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
	s->PutInt(free);
	if (readyItem != -1) s->PutStr(ItemDefs[readyItem].abr);
	else s->PutStr("NO_ITEM");
	for(int i = 0; i < MAX_READY; ++i) {
		if (readyWeapon[i] != -1)
			s->PutStr(ItemDefs[readyWeapon[i]].abr);
		else s->PutStr("NO_ITEM");
		if (readyArmor[i] != -1)
			s->PutStr(ItemDefs[readyArmor[i]].abr);
		else s->PutStr("NO_ITEM");
	}
	s->PutInt(flags);
	items.Writeout(s);
	skills.Writeout(s);
	if (combat != -1) s->PutStr(SkillDefs[combat].abbr);
	else s->PutStr("NO_SKILL");
}

void Unit::Readin(Ainfile *s, AList *facs, ATL_VER v)
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
	faction = GetFaction(facs, i);
	guard = s->GetInt();
	if(guard == GUARD_ADVANCE) guard = GUARD_NONE;
	if(guard == GUARD_SET) guard = GUARD_GUARD;
	reveal = s->GetInt();

	/* Handle the new 'ready item', ready weapons/armor, and free */
	free = 0;
	readyItem = -1;
	for(i = 0; i < MAX_READY; i++) {
		readyWeapon[i] = -1;
		readyArmor[i] = -1;
	}

	free = s->GetInt();
	AString *temp;
	temp = s->GetStr();
	readyItem = LookupItem(temp);
	delete temp;
	for(i = 0; i < MAX_READY; i++) {
		temp = s->GetStr();
		readyWeapon[i] = LookupItem(temp);
		delete temp;
		temp = s->GetStr();
		readyArmor[i] = LookupItem(temp);
		delete temp;
	}
	flags = s->GetInt();

	items.Readin(s);
	skills.Readin(s);
	temp = s->GetStr();
	combat = LookupSkill(temp);
	delete temp;
}

AString Unit::MageReport()
{
	AString temp;

	if (combat != -1) {
		temp = AString(". Combat spell: ") + SkillStrs(combat);
	}
	return temp;
}

AString Unit::ReadyItem()
{
	AString temp, weaponstr, armorstr, battlestr;
	int weapon, armor, item, i, ready;

	item = 0;
	for(i = 0; i < MAX_READY; ++i) {
		ready = readyWeapon[i];
		if(ready != -1) {
			if(item) weaponstr += ", ";
			weaponstr += ItemString(ready, 1);
			++item;
		}
	}
	if(item > 0)
		weaponstr = AString("Ready weapon") + (item == 1?"":"s") + ": " +
			weaponstr;
	weapon = item;

	item = 0;
	for(i = 0; i < MAX_READY; ++i) {
		ready = readyArmor[i];
		if(ready != -1) {
			if(item) armorstr += ", ";
			armorstr += ItemString(ready, 1);
			++item;
		}
	}
	if(item > 0)
		armorstr = AString("Ready armor: ") + armorstr;
	armor = item;

	if(readyItem != -1) {
		battlestr = AString("Ready item: ") + ItemString(readyItem, 1);
		item = 1;
	} else
		item = 0;

	if(weapon || armor || item) {
		temp += AString(". ");
		if(weapon) temp += weaponstr;
		if(armor) {
			if(weapon) temp += ". ";
			temp += armorstr;
		}
		if(item) {
			if(armor || weapon) temp += ". ";
			temp += battlestr;
		}
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

int Unit::CanGetSpoil(Item *i)
{
	if(!i) return 0;
	int weight = ItemDefs[i->type].weight;
	if(!weight) return 1; // any unit can carry 0 weight spoils

	int fly = ItemDefs[i->type].fly;
	int ride = ItemDefs[i->type].ride;
	int walk = ItemDefs[i->type].walk;

	if(flags & FLAG_NOSPOILS) return 0;
	if((flags & FLAG_FLYSPOILS) && fly < weight) return 0; // only flying
	if((flags & FLAG_WALKSPOILS) && walk < weight) return 0; // only walking
	if((flags & FLAG_RIDESPOILS) && ride < weight) return 0; // only riding
	return 1; // all spoils
}

AString Unit::SpoilsReport() {
	AString temp;
	if(GetFlag(FLAG_NOSPOILS)) temp = ", weightless battle spoils";
	else if(GetFlag(FLAG_FLYSPOILS)) temp = ", flying battle spoils";
	else if(GetFlag(FLAG_WALKSPOILS)) temp = ", walking battle spoils";
	else if(GetFlag(FLAG_RIDESPOILS)) temp = ", riding battle spoils";
	return temp;
}

void Unit::WriteReport(Areport *f, int obs, int truesight, int detfac,
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
		temp += SpoilsReport();
	}

	temp += items.Report(obs, truesight, 0);

	if (obs == 2) {
		temp += ". Weight: ";
		temp += AString(items.Weight());
		temp += ". Capacity: ";
		temp += AString(FlyingCapacity());
		temp += "/";
		temp += AString(RidingCapacity());
		temp += "/";
		temp += AString(WalkingCapacity());
		temp += "/";
		temp += AString(SwimmingCapacity());
		temp += ". Skills: ";
		temp += skills.Report(GetMen());
	}

	if (obs == 2 && (type == U_MAGE || type == U_GUARDMAGE)) {
		temp += MageReport();
	}

	if(obs == 2) {
		temp += ReadyItem();
		temp += StudyableSkills();
	}

	if (describe) {
		temp += AString("; ") + *describe;
	}
	temp += ".";
	f->PutStr(temp);
}

AString Unit::TemplateReport()
{
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
	if (GetFlag(FLAG_CONSUMING_FACTION)) temp += ", consuming faction's food";
	if (GetFlag(FLAG_NOCROSS_WATER)) temp += ", won't cross water";
	temp += SpoilsReport();

	temp += items.Report(2, 1, 0);
	temp += ". Weight: ";
	temp += AString(items.Weight());
	temp += ". Capacity: ";
	temp += AString(FlyingCapacity());
	temp += "/";
	temp += AString(RidingCapacity());
	temp += "/";
	temp += AString(WalkingCapacity());
	temp += "/";
	temp += AString(SwimmingCapacity());
	temp += ". Skills: ";
	temp += skills.Report(GetMen());

	if (type == U_MAGE || type == U_GUARDMAGE) {
		temp += MageReport();
	}
	temp += ReadyItem();
	temp += StudyableSkills();

	if (describe) {
		temp += AString("; ") + *describe;
	}
	temp += ".";
	return temp;
}

AString *Unit::BattleReport(int obs)
{
  AString *temp = new AString("");
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

void Unit::ClearOrders()
{
	canattack = 1;
	nomove = 0;
	enter = 0;
	leftShip = 0;
	destroy = 0;
	if (attackorders) delete attackorders;
	attackorders = 0;
	if (evictorders) delete evictorders;
	evictorders = 0;
	if (stealorders) delete stealorders;
	stealorders = 0;
	promote = 0;
	taxing = TAX_NONE;
	advancefrom = 0;
	movepoints = 0;
	if (monthorders) delete monthorders;
	monthorders = 0;
	inTurnBlock = 0;
	presentTaxing = 0;
	if (presentMonthOrders) delete presentMonthOrders;
	presentMonthOrders = 0;
	if (castorders) delete castorders;
	castorders = 0;
	if (teleportorders) delete teleportorders;
	teleportorders = 0;
}

void Unit::ClearCastOrders()
{
	if (castorders) delete castorders;
	castorders = 0;
	if (teleportorders) delete teleportorders;
	teleportorders = 0;
}

void Unit::DefaultOrders(Object *obj)
{
	ClearOrders();
	if (type == U_WMON) {
		if (ObjectDefs[obj->type].monster == -1) {
			MoveOrder *o = new MoveOrder;
			o->advancing = 0;
			int aper = Hostile();
			aper *= Globals->MONSTER_ADVANCE_HOSTILE_PERCENT;
			aper /= 100;

			if(aper < Globals->MONSTER_ADVANCE_MIN_PERCENT)
				aper = Globals->MONSTER_ADVANCE_MIN_PERCENT;

			int n = getrandom(100);
			if(n < aper) o->advancing = 1;
			MoveDir *d = new MoveDir;
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
		/* Set up default orders for factions which submit none */
		if(obj->region->type != R_NEXUS) {
			if(GetFlag(FLAG_AUTOTAX) &&
					Globals->TAX_PILLAGE_MONTH_LONG && Taxers()) {
				taxing = TAX_AUTO;
			} else {
				ProduceOrder *order = new ProduceOrder;
				order->skill = -1;
				order->item = I_SILVER;
				monthorders = order;
			}
		}
	}
}

void Unit::PostTurn(ARegion *r)
{
	if (type == U_WMON) {
		forlist(&items) {
			Item *i = (Item *) elem;
			if(!(ItemDefs[i->type].type & IT_MONSTER)) {
				items.Remove(i);
				delete i;
			}
		}
		if(free > 0) --free;
	}
}

void Unit::SetName(AString *s)
{
	if (s) {
		AString *newname = s->getlegal();
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

void Unit::SetDescribe(AString *s)
{
	if (describe) delete describe;
	if (s) {
		AString *newname = s->getlegal();
		delete s;
		describe = newname;
	} else
		describe = 0;
}

int Unit::IsAlive()
{
	if(type == U_MAGE || type == U_APPRENTICE) {
		return(GetMen());
	} else {
		forlist(&items) {
			Item *i = (Item *) elem;
			if (IsSoldier(i->type) && i->num > 0)
				return 1;
		}
	}
	return 0;
}

void Unit::SetMen(int t, int n)
{
	if (ItemDefs[t].type & IT_MAN) {
		int oldmen = GetMen();
		items.SetNum(t, n);
		int newmen = GetMen();
		if (newmen < oldmen) {
			delete skills.Split(oldmen, oldmen - newmen);
		}
	} else {
		/* This is probably a monster in this case */
		items.SetNum(t, n);
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
		Item *i = (Item *) elem;
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
		Item *i = (Item *) elem;
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
		Item *i = (Item *) elem;
		if (IsSoldier(i->type)) n+=i->num;
	}

	return n;
}

void Unit::SetMoney(int n)
{
	items.SetNum(I_SILVER, n);
}

int Unit::GetMoney()
{
	return items.GetNum(I_SILVER);
}

int Unit::GetTactics()
{
	int retval = GetRealSkill(S_TACTICS);

	forlist(&items) {
		Item *i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			MonType *mp = FindMonster(ItemDefs[i->type].abr,
					(ItemDefs[i->type].type & IT_ILLUSION));
			int temp = mp->tactics;
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
		Item *i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			MonType *mp = FindMonster(ItemDefs[i->type].abr,
					(ItemDefs[i->type].type & IT_ILLUSION));
			int temp = mp->obs;
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
			/* XXX -- Fix this -- not all men weigh the same */
			/* XXX --			 Use the least weight man in the unit */
			if (ItemDefs[i->type].fly - ItemDefs[i->type].weight >= 10) {
				return riding;
			}
			/* XXX -- Fix this -- Should also be able to carry the man */
			if (ItemDefs[i->type].ride - ItemDefs[i->type].weight >= 10) {
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
		Item *i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			MonType *mp = FindMonster(ItemDefs[i->type].abr,
					(ItemDefs[i->type].type & IT_ILLUSION));
			int temp = mp->stealth;
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
	/* XXX -- hack to adjust for invisible monsters */
	/* XXX -- This bonus should not be hard coded */
	if (GetFlag(FLAG_INVIS)) monstealth += 3;

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

void Unit::SetSkill(int sk, int level)
{
	skills.SetDays(sk, GetDaysByLevel(level) * GetMen());
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
	skills.SetDays(sk, 0);
	if (type == U_MAGE) {
		forlist(&skills) {
			Skill *s = (Skill *) elem;
			if(SkillDefs[s->type].flags & SkillType::MAGIC) {
				return;
			}
		}
		type = U_NORMAL;
	}
	if(type == U_APPRENTICE) {
		forlist(&skills) {
			Skill *s = (Skill *) elem;
			if(SkillDefs[s->type].flags & SkillType::APPRENTICE) {
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
		if(SkillDefs[SkillDefs[sk].depends[c].skill].flags &
			SkillType::DISABLED)
			continue;
		if(!CheckDepend(curlev, SkillDefs[sk].depends[c])) return 0;
	}
	return 1;
}

int Unit::Study(int sk, int days)
{
	Skill *s;

	if(Globals->SKILL_LIMIT_NONLEADERS && !IsLeader()) {
		if (skills.Num()) {
			s = (Skill *) skills.First();
			if (s->type != sk) {
				Error("STUDY: Can know only 1 skill.");
				return 0;
			}
		}
	}
	int max = GetSkillMax(sk);
	if (GetRealSkill(sk) >= max) {
		Error("STUDY: Maximum level for skill reached.");
		return 0;
	}

	if (!CanStudy(sk)) {
		Error("STUDY: Doesn't have the pre-requisite skills to study that.");
		return 0;
	}

	skills.SetDays(sk, skills.GetDays(sk) + days);
	AdjustSkills();

	/* Check to see if we need to show a skill report */
	int lvl = GetRealSkill(sk);
	if (lvl > faction->skills.GetDays(sk)) {
		faction->skills.SetDays(sk, lvl);
		faction->shows.Add(new ShowSkill(sk, lvl));
	}
	return 1;
}

int Unit::GetSkillMax(int sk)
{
	int max = 0;

	if (SkillDefs[sk].flags & SkillType::DISABLED) return 0;

	forlist (&items) {
		Item *i = (Item *)elem;
		if (ItemDefs[i->type].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[i->type].type & IT_MAN)) continue;
		int m = SkillMax(sk, i->type);
		if ((max == 0 && m > max) || (m < max)) max = m;
	}
	return max;
}

int Unit::Practice(int sk)
{
	int bonus, men, curlev, reqsk, reqlev;
	unsigned int i;

	bonus = Globals->SKILL_PRACTICE_AMOUNT;
	if (practiced || (bonus < 1)) return 1;
	days = skills.GetDays(sk);
	men = GetMen();

	if (men < 1 || days < 1) return 0;

	/*
	 * Let's do this check for max level correctly.. Non-leader units
	 * won't ever be able to get to 450 days like the original code checked
	 * for.
	 */
	int max = GetSkillMax(sk);
	curlev = GetRealSkill(sk);
	if (curlev >= max) return 0;

	for(i = 0; i < sizeof(SkillDefs[sk].depends)/sizeof(SkillDepend); i++) {
		reqsk = SkillDefs[sk].depends[i].skill;
		if (reqsk == -1) break;
		if (SkillDefs[reqsk].flags & SkillType::DISABLED) continue;
		reqlev = GetRealSkill(reqsk);
		if (reqlev <= curlev) {
			if (Practice(reqsk))
				return 1;
			// We don't meet the reqs, and can't practice that
			// req, but we still need to check the other reqs.
			bonus = 0;
		}
	}

	if (bonus) {
		Study(sk, men * bonus);
		practiced = 1;
	}

	return bonus;
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
	if(IsLeader()) {
		//
		// Unit is all leaders: Make sure no skills are > max
		//
		forlist(&skills) {
			Skill *s = (Skill *) elem;
			if (GetRealSkill(s->type) >= SkillMax(s->type, I_LEADERS)) {
				s->days = GetDaysByLevel(SkillMax(s->type, I_LEADERS)) *
					GetMen();
			}
		}
	} else {
		if(Globals->SKILL_LIMIT_NONLEADERS) {
			//
			// Not a leader, can only know 1 skill
			//
			if (skills.Num() > 1) {
				//
				// Find highest skill, eliminate others
				//
				unsigned int max = 0;
				Skill *maxskill = 0;
				forlist(&skills) {
					Skill *s = (Skill *) elem;
					if (s->days > max) {
						max = s->days;
						maxskill = s;
					}
				}
				{
					forlist(&skills) {
						Skill *s = (Skill *) elem;
						if (s != maxskill) {
							skills.Remove(s);
							delete s;
						}
					}
				}
			}
		}

		//
		// Limit remaining skill to max
		//
		forlist(&skills) {
			Skill *theskill = (Skill *) elem;
			int max = GetSkillMax(theskill->type);
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

void Unit::Short(int needed, int hunger)
{
	int n = 0;

	if (faction->IsNPC())
		return; // Don't starve monsters and the city guard!

	if (needed < 1 && hunger < 1) return;

	switch(Globals->SKILL_STARVATION) {
		case GameDefs::STARVE_MAGES:
			if(type == U_MAGE) SkillStarvation();
			return;
		case GameDefs::STARVE_LEADERS:
			if(GetMen(I_LEADERS)) SkillStarvation();
			return;
		case GameDefs::STARVE_ALL:
			SkillStarvation();
			return;
	}

	for (int i = 0; i<= NITEMS; i++) {
		if(!(ItemDefs[ i ].type & IT_MAN)) {
			// Only men need sustenance.
			continue;
		}

		if(i == I_LEADERS) {
			// Don't starve leaders just yet.
			continue;
		}

		while (GetMen(i)) {
			if (getrandom(100) < Globals->STARVE_PERCENT) {
				SetMen(i, GetMen(i) - 1);
				n++;
			}
			needed -= Globals->MAINTENANCE_COST;
			hunger -= Globals->UPKEEP_MINIMUM_FOOD;
			if (needed < 1 && hunger < 1) {
				if (n) Error(AString(n) + " starve to death.");
				return;
			}
		}
	}

	while (GetMen(I_LEADERS)) {
		if (getrandom(100) < Globals->STARVE_PERCENT) {
			SetMen(I_LEADERS, GetMen(I_LEADERS) - 1);
			n++;
		}
		needed -= Globals->LEADER_COST;
		hunger -= Globals->UPKEEP_MINIMUM_FOOD;
		if (needed < 1 && hunger < 1) {
			if (n) Error(AString(n) + " starve to death.");
			return;
		}
	}
}

int Unit::Weight()
{
	int retval = items.Weight();
	return retval;
}

int Unit::FlyingCapacity()
{
	int cap = 0;
	forlist(&items) {
		Item *i = (Item *) elem;
		cap += ItemDefs[i->type].fly * i->num;
	}
   
	return cap;
}

int Unit::RidingCapacity()
{
	int cap = 0;
	forlist(&items) {
		Item *i = (Item *) elem;
		cap += ItemDefs[i->type].ride * i->num;
	}

	return cap;
}

int Unit::SwimmingCapacity()
{
	int cap = 0;
	forlist(&items) {
		Item *i = (Item *) elem;
		cap += ItemDefs[i->type].swim * i->num;
	}

	return cap;
}

int Unit::WalkingCapacity()
{
	int cap = 0;
	forlist(&items) {
		Item *i = (Item *) elem;
		cap += ItemDefs[i->type].walk * i->num;
		if(ItemDefs[i->type].hitchItem != -1) {
			int hitch = ItemDefs[i->type].hitchItem;
			if(!(ItemDefs[hitch].flags & ItemType::DISABLED)) {
				int hitches = items.GetNum(hitch);
				int hitched = i->num;
				if(hitched > hitches) hitched = hitches;
				cap += hitched * ItemDefs[i->type].hitchwalk;
			}
		}
	}

	return cap;
}



int Unit::CanFly(int weight)
{
	if (FlyingCapacity() >= weight) return 1;
	return 0;
}

int Unit::CanReallySwim()
{
	if (SwimmingCapacity() >= items.Weight()) return 1;
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

int Unit::CanFly()
{
	int weight = items.Weight();
	return CanFly(weight);
}

int Unit::CanRide(int weight)
{
	if (RidingCapacity() >= weight) return 1;
	return 0;
}

int Unit::CanWalk(int weight)
{
	if (WalkingCapacity() >= weight) return 1;
	return 0;
}

int Unit::MoveType()
{
	int weight = items.Weight();
	if (CanFly(weight)) return M_FLY;
	if (CanRide(weight)) return M_RIDE;
	/* Check if we should be able to 'swim' */
	/* This should become it's own M_TYPE sometime */
	if(TerrainDefs[object->region->type].similar_type == R_OCEAN)
		if(CanSwim()) return M_WALK;
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

int Unit::CanMoveTo(ARegion *r1, ARegion *r2)
{
	if (r1 == r2) return 1;

	int exit = 1;
	int i;
	int dir;

	for (i=0; i<NDIRS; i++) {
		if (r1->neighbors[i] == r2) {
			exit = 0;
			dir = i;
			break;
		}
	}
	if (exit) return 0;
	exit = 1;
	for (i=0; i<NDIRS; i++) {
		if (r2->neighbors[i] == r1) {
			exit = 0;
			break;
		}
	}
	if (exit) return 0;

	int mt = MoveType();
	if (((TerrainDefs[r1->type].similar_type == R_OCEAN) ||
				(TerrainDefs[r2->type].similar_type == R_OCEAN)) &&
			(!CanSwim() || GetFlag(FLAG_NOCROSS_WATER)))
		return 0;
	int mp = CalcMovePoints() - movepoints;
	if (mp < (r2->MoveCost(mt, r1, dir))) return 0;
	return 1;
}

int Unit::CanCatch(ARegion *r, Unit *u)
{
	return faction->CanCatch(r, u);
}

int Unit::CanSee(ARegion *r, Unit *u, int practice)
{
	return faction->CanSee(r, u, practice);
}

int Unit::AmtsPreventCrime(Unit *u)
{
	if(!u) return 0;

	int amulets = items.GetNum(I_AMULETOFTS);
	if((u->items.GetNum(I_RINGOFI) < 1) || (amulets < 1)) return 0;
	int men = GetMen();
	if(men <= amulets) return 1;
	if(!Globals->PROPORTIONAL_AMTS_USAGE) return 0;
	if(getrandom(men) < amulets) return 1;
	return 0;
}

int Unit::GetAttitude(ARegion *r, Unit *u)
{
	if (faction == u->faction) return A_ALLY;
	int att = faction->GetAttitude(u->faction->num);
	if (att >= A_FRIENDLY && att >= faction->defaultattitude) return att;

	if (CanSee(r, u) == 2)
		return att;
	else
		return faction->defaultattitude;
}

int Unit::Hostile()
{
	if (type != U_WMON) return 0;
	int retval = 0;
	forlist(&items) {
		Item *i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MONSTER) {
			MonType *mp = FindMonster(ItemDefs[i->type].abr,
					(ItemDefs[i->type].type & IT_ILLUSION));
			int hos = mp->hostile;
			if (hos > retval) retval = hos;
		}
	}
	return retval;
}

int Unit::Forbids(ARegion *r, Unit *u)
{
	if (guard != GUARD_GUARD) return 0;
	if (!IsAlive()) return 0;
	if (!CanSee(r, u, Globals->SKILL_PRACTICE_AMOUNT > 0)) return 0;
	if (!CanCatch(r, u)) return 0;
	if (GetAttitude(r, u) < A_NEUTRAL) return 1;
	return 0;
}

int Unit::Taxers()
{
	int totalMen = GetMen();
	int illusions = 0;
	int creatures = 0;
	int taxers = 0;

	if ((Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_COMBAT_SKILL) &&
		 GetSkill(S_COMBAT)) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_BOW_SKILL) &&
		 (GetSkill(S_CROSSBOW) || GetSkill(S_LONGBOW))) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_RIDING_SKILL) &&
		 GetSkill(S_RIDING)) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_STEALTH_SKILL) &&
		 GetSkill(S_STEALTH))) {
		taxers = totalMen;
	} else {
		// check out items
		int numMelee= 0;
		int numUsableMelee = 0;
		int numBows = 0;
		int numUsableBows = 0;
		int numMounted= 0;
		int numUsableMounted = 0;
		int numMounts = 0;
		int numUsableMounts = 0;
		int numBattle = 0;
		int numUsableBattle = 0;

		forlist (&items) {
			Item *pItem = (Item *) elem;
			BattleItemType *pBat = NULL;

			if ((ItemDefs[pItem->type].type & IT_BATTLE) &&
				((pBat = FindBattleItem(ItemDefs[pItem->type].abr)) != NULL) &&
				(pBat->flags & BattleItemType::SPECIAL)) {
				// Only consider offensive items
				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_BATTLE_ITEM) &&
					(!(pBat->flags & BattleItemType::MAGEONLY) ||
					 type == U_MAGE || type == U_APPRENTICE)) {
					numUsableBattle += pItem->num;
					numBattle += pItem->num;
					continue; // Don't count this as a weapon as well!
				}
				if (Globals->WHO_CAN_TAX & GameDefs::TAX_BATTLE_ITEM) {
					numBattle += pItem->num;
					continue; // Don't count this as a weapon as well!
				}
			}

			if (ItemDefs[pItem->type].type & IT_WEAPON) {
				WeaponType *pWep = FindWeapon(ItemDefs[pItem->type].abr);
				if (!(pWep->flags & WeaponType::NEEDSKILL)) {
					// A melee weapon
					if (GetSkill(S_COMBAT)) numUsableMelee += pItem->num;
					numMelee += pItem->num;
				} else if (pWep->baseSkill == S_RIDING) {
					// A mounted weapon
					if (GetSkill(S_RIDING)) numUsableMounted += pItem->num;
					numMounted += pItem->num;
				} else {
					// Presume that anything else is a bow!
					if (GetSkill(pWep->baseSkill) ||
						(pWep->orSkill != -1 && GetSkill(pWep->orSkill)))
						numUsableBows += pItem->num;
					numBows += pItem->num;
				}
			}

			if (ItemDefs[pItem->type].type & IT_MOUNT) {
				MountType *pm = FindMount(ItemDefs[pItem->type].abr);
				if (pm->minBonus <= GetSkill(S_RIDING))
					numUsableMounts += pItem->num;
				numMounts += pItem->num;
			}

			if (ItemDefs[pItem->type].type & IT_MONSTER) {
				if (ItemDefs[pItem->type].type & IT_ILLUSION)
					illusions += pItem->num;
				else
					creatures += pItem->num;
			}
		}

		// Ok, now process the counts!
		if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_WEAPON) {
			if (numUsableMounted > numUsableMounts) {
				taxers = numUsableMounts;
				numMounts -= numUsableMounts;
				numUsableMounts = 0;
			} else {
				taxers = numUsableMounted;
				numMounts -= numUsableMounted;
				numUsableMounts -= numUsableMounted;
			}
			taxers += numMelee + numUsableBows;
		} else if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANY_WEAPON) {
			taxers = numMelee + numBows + numMounted;
		} else {
			if (Globals->WHO_CAN_TAX &
					GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL) {
				if (numUsableMounted > numUsableMounts) {
					taxers += numUsableMounts;
					numMounts -= numUsableMounts;
					numUsableMounts = 0;
				} else {
					taxers += numUsableMounted;
					numMounts -= numUsableMounted;
					numUsableMounts -= numUsableMounted;
				}
				taxers += numUsableMelee;
			}
			if (Globals->WHO_CAN_TAX &
					GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON) {
				taxers += numUsableBows;
			}
		}

		if (Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE)
			taxers += numMounts;
		else if (Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE_AND_RIDING_SKILL)
			taxers += numUsableMounts;

		if (Globals->WHO_CAN_TAX & GameDefs::TAX_BATTLE_ITEM)
			taxers += numBattle;
		else if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_BATTLE_ITEM)
			taxers += numUsableBattle;
	}

	// Ok, all the items categories done - check for mages taxing
	if (type == U_MAGE) {
		if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANY_MAGE)
			taxers = totalMen;
		else {
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_COMBAT_SPELL) {
				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_DAMAGE) &&
						(combat == S_FIRE || combat == S_EARTHQUAKE ||
						 combat == S_SUMMON_TORNADO ||
						 combat == S_CALL_LIGHTNING ||
						 combat == S_SUMMON_BLACK_WIND))
					taxers = totalMen;

				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_FEAR) &&
						(combat == S_SUMMON_STORM ||
						 combat == S_CREATE_AURA_OF_FEAR))
					taxers = totalMen;

				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_OTHER) &&
						(combat == S_FORCE_SHIELD ||
						 combat == S_ENERGY_SHIELD ||
						 combat == S_SPIRIT_SHIELD ||
						 combat == S_CLEAR_SKIES))
					taxers = totalMen;
			} else {
				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_DAMAGE) &&
						(GetSkill(S_FIRE) || GetSkill(S_EARTHQUAKE) ||
						 GetSkill(S_SUMMON_TORNADO) ||
						 GetSkill(S_CALL_LIGHTNING) ||
						 GetSkill(S_SUMMON_BLACK_WIND)))
					taxers = totalMen;

				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_FEAR) &&
						(GetSkill(S_SUMMON_STORM) ||
						 GetSkill(S_CREATE_AURA_OF_FEAR)))
					taxers = totalMen;

				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_OTHER) &&
						(GetSkill(S_FORCE_SHIELD) ||
						 GetSkill(S_ENERGY_SHIELD) ||
						 GetSkill(S_SPIRIT_SHIELD) ||
						 GetSkill(S_CLEAR_SKIES)))
					taxers = totalMen;
			}
		}
	}

	// Now check for an overabundance of tax enabling objects
	if (taxers > totalMen) taxers = totalMen;

	// And finally for creatures
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_CREATURES)
		taxers += creatures;
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS)
		taxers += illusions;

	return(taxers);
}

int Unit::GetFlag(int x)
{
	return (flags & x);
}

void Unit::SetFlag(int x, int val)
{
	if (val)
		flags = flags | x;
	else
		if (flags & x) flags -= x;
}

void Unit::CopyFlags(Unit *x)
{
	flags = x->flags;
	guard = GUARD_NONE;
	if (x->Taxers()) {
		if (x->guard != GUARD_SET && x->guard != GUARD_ADVANCE)
			guard = x->guard;
	} else {
		SetFlag(FLAG_AUTOTAX, 0);
	}
	reveal = x->reveal;
}

int Unit::GetBattleItem(AString &itm)
{
	int item = LookupItem(&itm);
	if (item == -1) return -1;

	int num = items.GetNum(item);
	if (num < 1) return -1;

	if (!(ItemDefs[item].type & IT_BATTLE)) return -1;
	// Exclude weapons.  They will be handled later.
	if (ItemDefs[item].type & IT_WEAPON) return -1;
	items.SetNum(item, num - 1);
	return item;
}

int Unit::GetArmor(AString &itm, int ass)
{
	int item = LookupItem(&itm);
	ArmorType *pa = FindArmor(itm.Str());

	if (pa == NULL) return -1;
	if (ass && !(pa->flags & ArmorType::USEINASSASSINATE)) return -1;

	int num = items.GetNum(item);
	if (num < 1) return -1;

	if (!(ItemDefs[item].type & IT_ARMOR)) return -1;
	items.SetNum(item, num - 1);
	return item;
}

int Unit::GetMount(AString &itm, int canFly, int canRide, int &bonus)
{
	bonus = 0;

	// This region doesn't allow riding or flying, so no mounts, bail
	if(!canFly && !canRide) return -1;

	int item = LookupItem(&itm);
	MountType *pMnt = FindMount(itm.Str());

	int num = items.GetNum(item);
	if (num < 1) return -1;

	if (canFly) {
		// If the mount cannot fly, and the region doesn't allow
		// riding mounts, bail
		if (!ItemDefs[item].fly && !canRide) return -1;
	} else {
		// This region allows riding mounts, so if the mount
		// can not carry at a riding level, bail
		if (!ItemDefs[item].ride) return -1;
	}

	bonus = GetSkill(pMnt->skill);
	if(bonus < pMnt->minBonus) {
		// Unit isn't skilled enough for this mount
		bonus = 0;
		return -1;
	}
	// Limit to max mount bonus;
	if(bonus > pMnt->maxBonus) bonus = pMnt->maxBonus;
	// If the mount can fly and the terrain doesn't allow
	// flying mounts, limit the bonus to the maximum hampered
	// bonus allowed by the mount
	if(ItemDefs[item].fly && !canFly) {
		if(bonus > pMnt->maxHamperedBonus)
			bonus = pMnt->maxHamperedBonus;
	}
	// Get the mount
	items.SetNum(item, num - 1);
	return item;
}

int Unit::GetWeapon(AString &itm, int riding, int ridingBonus,
		int &attackBonus, int &defenseBonus, int &attacks)
{
	int item = LookupItem(&itm);
	WeaponType *pWep = FindWeapon(itm.Str());

	if (pWep == NULL) return -1;

	int num = items.GetNum(item);
	if (num < 1) return -1;

	if (!(ItemDefs[item].type & IT_WEAPON)) return -1;

	attackBonus = 0;
	defenseBonus = 0;
	attacks = 1;
	int combatSkill = GetSkill(S_COMBAT);

	// Found a weapon, check flags and skills
	int baseSkillLevel = CanUseWeapon(pWep, riding);
	// returns -1 if weapon cannot be used, else the usable skill level
	if(baseSkillLevel == -1) return -1;

	// Attack and defense skill
	if(!(pWep->flags & WeaponType::NEEDSKILL)) baseSkillLevel = combatSkill;
	attackBonus = baseSkillLevel + pWep->attackBonus;
	if(pWep->flags & WeaponType::NOATTACKERSKILL)
		defenseBonus = pWep->defenseBonus;
	else
		defenseBonus = baseSkillLevel + pWep->defenseBonus;
	// Riding bonus
	if(pWep->flags & WeaponType::RIDINGBONUS) attackBonus += ridingBonus;
	if(pWep->flags & (WeaponType::RIDINGBONUSDEFENSE|WeaponType::RIDINGBONUS))
		defenseBonus += ridingBonus;
	// Number of attacks
	attacks = pWep->numAttacks;
	// Note: NUM_ATTACKS_SKILL must be > NUM_ATTACKS_HALF_SKILL
	if(attacks >= WeaponType::NUM_ATTACKS_SKILL)
		attacks += baseSkillLevel - WeaponType::NUM_ATTACKS_SKILL;
	else if(attacks >= WeaponType::NUM_ATTACKS_HALF_SKILL)
		attacks += (baseSkillLevel +1)/2 - WeaponType::NUM_ATTACKS_HALF_SKILL;
	// Sanity check
	if(attacks == 0) attacks = 1;

	// get the weapon
	items.SetNum(item, num-1);
	return item;
}

void Unit::MoveUnit(Object *toobj)
{
	if(object) object->units.Remove(this);
	object = toobj;
	if(object) object->units.Add(this);
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

int Unit::GetSkillBonus(int sk)
{
	int bonus = 0;
	int men = GetMen();
	/* XXX -- skill bonuses should be part of the skill structure!! */
	switch(sk) {
		case S_OBSERVATION:
			if(!men) break;
			if(Globals->FULL_TRUESEEING_BONUS) {
				bonus = GetSkill(S_TRUE_SEEING);
			} else {
				bonus = (GetSkill(S_TRUE_SEEING)+1)/2;
			}
			if ((bonus < (2 + Globals->IMPROVED_AMTS)) &&
					items.GetNum(I_AMULETOFTS)) {
				bonus = 2 + Globals->IMPROVED_AMTS;
			}
			break;
		case S_STEALTH:
			if(men == 1 && Globals->FULL_INVIS_ON_SELF) {
				bonus = GetSkill(S_INVISIBILITY);
			}
			if((bonus < 3) &&
					(GetFlag(FLAG_INVIS) || men <= items.GetNum(I_RINGOFI))) {
				bonus = 3;
			}
			break;
		default:
			break;
	}
	return bonus;
}

int Unit::GetProductionBonus(int item)
{
	int bonus = 0;
	if (ItemDefs[item].mult_item != -1)
		bonus = items.GetNum(ItemDefs[item].mult_item);
	else
		bonus = GetMen();
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
		if (!can_forget[i]) continue;
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
	if (riding == -1)
		if(pWep->flags & WeaponType::NOFOOT) return -1;
	else
		if(pWep->flags & WeaponType::NOMOUNT) return -1;
	return CanUseWeapon(pWep);
}

int Unit::CanUseWeapon(WeaponType *pWep)
{
	// we don't care in this case, their combat skill will be used.
	if (!(pWep->flags & WeaponType::NEEDSKILL)) {
		Practice(S_COMBAT);
		return 0;
	}

	int baseSkillLevel = 0;
	int tempSkillLevel = 0;
	if(pWep->baseSkill != -1) baseSkillLevel = GetSkill(pWep->baseSkill);

	if(pWep->orSkill != -1) tempSkillLevel = GetSkill(pWep->orSkill);

	if(tempSkillLevel > baseSkillLevel) {
		baseSkillLevel = tempSkillLevel;
		Practice(pWep->orSkill);
	} else
		Practice(pWep->baseSkill);

	if(pWep->flags & WeaponType::NEEDSKILL && !baseSkillLevel) return -1;

	return baseSkillLevel;
}

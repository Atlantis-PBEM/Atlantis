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
	movepoints = Globals->PHASED_MOVE_OFFSET % Globals->MAX_SPEED;
	combat = -1;
	for (int i = 0; i < MAX_READY; i++) {
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
	joinorders = NULL;
	inTurnBlock = 0;
	presentTaxing = 0;
	presentMonthOrders = NULL;
	former = NULL;
	format = 0;
	free = 0;
	practiced = 0;
	moved = 0;
	phase = -1;
	savedmovement = 0;
	savedmovedir = -1;
	ClearOrders();
	raised = 0;
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
	movepoints = Globals->PHASED_MOVE_OFFSET % Globals->MAX_SPEED;
	combat = -1;
	for (int i = 0; i < MAX_READY; i++) {
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
	joinorders = NULL;
	inTurnBlock = 0;
	presentTaxing = 0;
	presentMonthOrders = NULL;
	former = NULL;
	format = 0;
	free = 0;
	practiced = 0;
	moved = 0;
	phase = -1;
	savedmovement = 0;
	savedmovedir = -1;
	ClearOrders();
	raised = 0;
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

void Unit::MakeWMon(char const *monname, int mon, int num)
{
	AString *temp = new AString(monname);
	SetName(temp);

	type = U_WMON;
	items.SetNum(mon, num);
	SetMonFlags();
}

void Unit::Writeout(Aoutfile *s)
{
	set<string>::iterator it;

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
	for (int i = 0; i < MAX_READY; ++i) {
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
	s->PutInt(savedmovement);
	s->PutInt(savedmovedir);
	s->PutInt(visited.size());
	for (it = visited.begin();
			it != visited.end();
			it++) {
		s->PutStr(it->c_str());
	}
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
	if (guard == GUARD_ADVANCE) guard = GUARD_NONE;
	if (guard == GUARD_SET) guard = GUARD_GUARD;
	reveal = s->GetInt();

	/* Handle the new 'ready item', ready weapons/armor, and free */
	free = 0;
	readyItem = -1;
	for (i = 0; i < MAX_READY; i++) {
		readyWeapon[i] = -1;
		readyArmor[i] = -1;
	}

	free = s->GetInt();
	AString *temp;
	temp = s->GetStr();
	readyItem = LookupItem(temp);
	delete temp;
	for (i = 0; i < MAX_READY; i++) {
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
	savedmovement = s->GetInt();
	savedmovedir = s->GetInt();
	i = s->GetInt();
	while (i-- > 0) {
		temp = s->GetStr();
		visited.insert(temp->Str());
		delete temp;
	}
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
	for (i = 0; i < MAX_READY; ++i) {
		ready = readyWeapon[i];
		if (ready != -1) {
			if (item) weaponstr += ", ";
			weaponstr += ItemString(ready, 1);
			++item;
		}
	}
	if (item > 0)
		weaponstr = AString("Ready weapon") + (item == 1?"":"s") + ": " +
			weaponstr;
	weapon = item;

	item = 0;
	for (i = 0; i < MAX_READY; ++i) {
		ready = readyArmor[i];
		if (ready != -1) {
			if (item) armorstr += ", ";
			armorstr += ItemString(ready, 1);
			++item;
		}
	}
	if (item > 0)
		armorstr = AString("Ready armor: ") + armorstr;
	armor = item;

	if (readyItem != -1) {
		battlestr = AString("Ready item: ") + ItemString(readyItem, 1);
		item = 1;
	} else
		item = 0;

	if (weapon || armor || item) {
		temp += AString(". ");
		if (weapon) temp += weaponstr;
		if (armor) {
			if (weapon) temp += ". ";
			temp += armorstr;
		}
		if (item) {
			if (armor || weapon) temp += ". ";
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
		if (SkillDefs[i].depends[0].skill != NULL) {
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
	int stealth = GetAttribute("stealth");
	if (reveal == REVEAL_FACTION || obs > stealth) {
		ret += ", ";
		ret += *faction->name;
	}
	return ret;
}

int Unit::CanGetSpoil(Item *i)
{
	int weight, load, capacity;

	if (!i) return 0;
	if (ItemDefs[i->type].type & IT_SHIP) {
		// Don't pick up an incomplete ship if we already have one
		if (items.GetNum(i->type) > 0) return 0;
	}
	weight = ItemDefs[i->type].weight;
	if (!weight) return 1; // any unit can carry 0 weight spoils

	if (flags & FLAG_NOSPOILS)
		return 0;

	load = items.Weight();
	
	if (flags & FLAG_FLYSPOILS) {
		capacity = ItemDefs[i->type].fly;
		if (FlyingCapacity() + capacity < load + weight)
			return 0;
	}

	if (flags & FLAG_RIDESPOILS) {
		capacity = ItemDefs[i->type].ride;
		if (RidingCapacity() + capacity < load + weight)
			return 0;
	}

	if (flags & FLAG_WALKSPOILS) {
		capacity = ItemDefs[i->type].walk;
		if (ItemDefs[i->type].hitchItem) {
			if (items.GetNum(ItemDefs[i->type].hitchItem) >
					items.GetNum(i->type))
				capacity = ItemDefs[i->type].hitchwalk;
		}
		if (WalkingCapacity() + capacity < load + weight)
			return 0;
	}

	if (flags & FLAG_SWIMSPOILS) {
		capacity = ItemDefs[i->type].swim;
		if (ItemDefs[i->type].type & IT_SHIP)
			capacity = 0;
		if (SwimmingCapacity() + capacity < load + weight)
			return 0;
	}

	if ((flags & FLAG_SAILSPOILS) && object && object->IsFleet()) {
		load = object->FleetLoad();
		if (object->FleetCapacity() < load + weight)
			return 0;
	}

	return 1; // all spoils
}

AString Unit::SpoilsReport() {
	AString temp;
	if (GetFlag(FLAG_NOSPOILS)) temp = ", weightless battle spoils";
	else if (GetFlag(FLAG_FLYSPOILS)) temp = ", flying battle spoils";
	else if (GetFlag(FLAG_WALKSPOILS)) temp = ", walking battle spoils";
	else if (GetFlag(FLAG_RIDESPOILS)) temp = ", riding battle spoils";
	else if (GetFlag(FLAG_SAILSPOILS)) temp = ", sailing battle spoils";
	return temp;
}

void Unit::WriteReport(Areport *f, int obs, int truesight, int detfac,
				int autosee, int attitude, int showattitudes)
{
	int stealth = GetAttribute("stealth");
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
		if (showattitudes) {
			switch (attitude) {
			case A_ALLY: 
				temp += AString("= ") +*name;
				break;
			case A_FRIENDLY: 
				temp += AString(": ") +*name;
				break;
			case A_NEUTRAL: 
				temp += AString("- ") +*name;
				break;
			case A_UNFRIENDLY: 
				temp += AString("% ") +*name;
				break;
			case A_HOSTILE: 
				temp += AString("! ") +*name;
				break;
			}
		} else {
			temp += AString("- ") + *name;
		}
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
		if (GetFlag(FLAG_SHARING)) temp += ", sharing";
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

	if (obs == 2) {
		temp += ReadyItem();
		temp += StudyableSkills();
		if (visited.size() > 0) {
			set<string>::iterator it;
			unsigned int count;

			count = 0;
			temp += ". Has visited ";
			for (it = visited.begin();
					it != visited.end();
					it++) {
				count++;
				if (count > 1) {
					if (count == visited.size())
						temp += " and ";
					else
						temp += ", ";
				}
				temp += it->c_str();
			}
		}
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
	if (GetFlag(FLAG_SHARING)) temp += ", sharing";
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
	if (visited.size() > 0) {
		set<string>::iterator it;
		unsigned int count;

		count = 0;
		temp += ". Has visited ";
		for (it = visited.begin();
				it != visited.end();
				it++) {
			count++;
			if (count > 1) {
				if (count == visited.size())
					temp += " and ";
				else
					temp += ", ";
			}
			temp += it->c_str();
		}
	}

	if (describe) {
		temp += AString("; ") + *describe;
	}
	temp += ".";
	return temp;
}

AString *Unit::BattleReport(int obs)
{
	AString *temp = new AString("");
	if (Globals->BATTLE_FACTION_INFO)
		*temp += GetName(obs);
	else
		*temp += *name;

	if (GetFlag(FLAG_BEHIND)) *temp += ", behind";

	*temp += items.BattleReport();

	forlist (&skills) {
		Skill *s = (Skill *)elem;
		if (SkillDefs[s->type].flags & SkillType::BATTLEREP) {
			int lvl = GetAvailSkill(s->type);
			if (lvl) {
				*temp += ", ";
				*temp += SkillDefs[s->type].name;
				*temp += " ";
				*temp += lvl;
			}
		}
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
	routed = 0;
	enter = 0;
	build = 0;
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
	int count, weight, i;
	ARegion *r, *n;

	ClearOrders();
	if (type == U_WMON) {
		if (ObjectDefs[obj->type].monster == -1) {
			// count starts at 2 to give a 2 / (available dirs + 2)
			// chance of a wandering monster not moving
			count = 2;
			weight = items.Weight();
			r = obj->region;
			for (i = 0; i < NDIRS; i++) {
				n = r->neighbors[i];
				if (!n)
					continue;
				if (TerrainDefs[n->type].similar_type == R_OCEAN &&
						!CanReallySwim() &&
						!(CanFly(weight) &&
							Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_UNLIMITED))
					continue;
				if (TerrainDefs[n->type].similar_type != R_OCEAN &&
						!CanWalk(weight) &&
						!CanRide(weight) &&
						!CanFly(weight))
					continue;
				count++;

			}
			count = getrandom(count);
			for (i = 0; i < NDIRS; i++) {
				n = r->neighbors[i];
				if (!n)
					continue;
				if (TerrainDefs[n->type].similar_type == R_OCEAN &&
						!CanReallySwim() &&
						!(CanFly(weight) &&
							Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_UNLIMITED))
					continue;
				if (TerrainDefs[n->type].similar_type != R_OCEAN &&
						!CanWalk(weight) &&
						!CanRide(weight) &&
						!CanFly(weight))
					continue;
				if (!count--) {
					MoveOrder *o = new MoveOrder;
					o->advancing = 0;
					int aper = Hostile();
					aper *= Globals->MONSTER_ADVANCE_HOSTILE_PERCENT;
					aper /= 100;
					if (aper < Globals->MONSTER_ADVANCE_MIN_PERCENT)
						aper = Globals->MONSTER_ADVANCE_MIN_PERCENT;
					if (getrandom(100) < aper)
						o->advancing = 1;
					MoveDir *d = new MoveDir;
					d->dir = i;
					o->dirs.Add(d);
					monthorders = o;
				}
			}
		}
	} else if (type == U_GUARD) {
		if (guard != GUARD_GUARD)
			guard = GUARD_SET;
	} else if (type == U_GUARDMAGE) {
		combat = S_FIRE;
	} else{
		/* Set up default orders for factions which submit none */
		if (obj->region->type != R_NEXUS) {
			if (GetFlag(FLAG_AUTOTAX) &&
					Globals->TAX_PILLAGE_MONTH_LONG && Taxers(1)) {
				taxing = TAX_AUTO;
			} else {
				ProduceOrder *order = new ProduceOrder;
				order->skill = -1;
				order->item = I_SILVER;
				order->target = 0;
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
			if (!(ItemDefs[i->type].type & IT_MONSTER)) {
				items.Remove(i);
				delete i;
			}
		}
		if (free > 0) --free;
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
	if (type == U_MAGE || type == U_APPRENTICE) {
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

int Unit::GetLeaders()
{
	int n = 0;
	forlist (&items) {
		Item *i = (Item *)elem;
		if (ItemDefs[i->type].type & IT_LEADER) {
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

int Unit::GetSharedNum(int item)
{
	int count = 0;

	if (ItemDefs[item].type & IT_MAN)
		return items.GetNum(item);

	forlist((&object->region->objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			if ((u->num == num) || 
			(u->faction == faction && u->GetFlag(FLAG_SHARING)))
				count += u->items.GetNum(item);
		}
	}

	return count;
}

void Unit::ConsumeShared(int item, int n)
{
	if (items.GetNum(item) >= n) {
		// This unit doesn't need to use shared resources
		items.SetNum(item, items.GetNum(item) - n);
		return;
	}

	// Use up items carried by the using unit first
	n -= items.GetNum(item);
	items.SetNum(item, 0);

	forlist((&object->region->objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			if (u->faction == faction && u->GetFlag(FLAG_SHARING)) {
				if (u->items.GetNum(item) < 1)
					continue;
				if (u->items.GetNum(item) >= n) {
					u->items.SetNum(item, u->items.GetNum(item) - n);
					u->Event(*(u->name) + " shares " + ItemString(item, n) +
							" with " + *name + ".");
					return;
				}
				u->Event(*(u->name) + " shares " +
						ItemString(item, u->items.GetNum(item)) +
						" with " + *name + ".");
				n -= u->items.GetNum(item);
				u->items.SetNum(item, 0);
			}
		}
	}
}

int Unit::GetSharedMoney()
{
	return GetSharedNum(I_SILVER);
}

void Unit::ConsumeSharedMoney(int n)
{
	return ConsumeShared(I_SILVER, n);
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
		int minweight = 10000;
		forlist(&items) {
			Item *i = (Item *)elem;
			if (ItemDefs[i->type].type & IT_MAN)
				if (ItemDefs[i->type].weight < minweight)
					minweight = ItemDefs[i->type].weight;
		}
		forlist_reuse (&items) {
			Item *i = (Item *)elem;
			if (ItemDefs[i->type].fly - ItemDefs[i->type].weight >= minweight)
				return riding;
			if (ItemDefs[i->type].ride-ItemDefs[i->type].weight >= minweight) {
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

	if (CanFly(weight)) riding = 5;
	else if (CanRide(weight)) riding = 3;

	if (GetMen()) {
		int manriding = GetSkill(S_RIDING);
		if (manriding < riding) return manriding;
	}

	return riding;
}

int Unit::GetSkill(int sk)
{
	if (sk == S_TACTICS) return GetAttribute("tactics");
	if (sk == S_STEALTH) return GetAttribute("stealth");
	if (sk == S_OBSERVATION) return GetAttribute("observation");
	if (sk == S_ENTERTAINMENT) return GetAttribute("entertainment");
	int retval = GetAvailSkill(sk);
	return retval;
}

void Unit::SetSkill(int sk, int level)
{
	skills.SetDays(sk, GetDaysByLevel(level) * GetMen());
	skills.SetExp(sk, 0);
}

int Unit::GetAvailSkill(int sk)
{
	AString str;
	int retval = GetRealSkill(sk);

	forlist (&items) {
		Item *i = (Item *)elem;
		if (ItemDefs[i->type].flags & ItemType::DISABLED) continue;
		if (ItemDefs[i->type].type & IT_MAGEONLY
				&& type != U_MAGE
				&& type != U_APPRENTICE
				&& type != U_GUARDMAGE)
			continue;
		if ((SkillDefs[sk].flags & SkillType::MAGIC)
				&& type != U_MAGE
				&& type != U_APPRENTICE
				&& type != U_GUARDMAGE)
			continue;
		if (i->num < GetMen())
			continue;
		str = ItemDefs[i->type].grantSkill;
		if (ItemDefs[i->type].grantSkill && LookupSkill(&str) == sk) {
			int grant = 0;
			for (unsigned j = 0; j < sizeof(ItemDefs[0].fromSkills)
								 / sizeof(ItemDefs[0].fromSkills[0]); j++) {
				if (ItemDefs[i->type].fromSkills[j]) {
					int fromSkill;

					str = ItemDefs[i->type].fromSkills[j];

					fromSkill = LookupSkill(&str);
					if (fromSkill != -1) {
						/*
							Should this use GetRealSkill or GetAvailSkill?
							GetAvailSkill could cause unbounded recursion,
							but only if the GM sets up items stupidly...
						*/
						if (grant < GetRealSkill(fromSkill))
							grant = GetRealSkill(fromSkill);
					}
				}
			}
			if (grant < ItemDefs[i->type].minGrant)
				grant = ItemDefs[i->type].minGrant;
			if (grant > ItemDefs[i->type].maxGrant)
				grant = ItemDefs[i->type].maxGrant;
			
			if (grant > retval)
				retval = grant;
		}
	}

	return retval;
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
			if (SkillDefs[s->type].flags & SkillType::MAGIC) {
				return;
			}
		}
		type = U_NORMAL;
	}
	if (type == U_APPRENTICE) {
		forlist(&skills) {
			Skill *s = (Skill *) elem;
			if (SkillDefs[s->type].flags & SkillType::APPRENTICE) {
				return;
			}
		}
		type = U_NORMAL;
	}
}

int Unit::CheckDepend(int lev, SkillDepend &dep)
{
	AString skname = dep.skill;
	int sk = LookupSkill(&skname);
	if (sk == -1) return 0;
	int temp = GetRealSkill(sk);
	if (temp < dep.level) return 0;
	if (lev >= temp) return 0;
	return 1;
}

int Unit::CanStudy(int sk)
{
	if (skills.GetStudyRate(sk, GetMen()) < 1) return 0;

	if (Globals->SKILL_LIMIT_NONLEADERS &&
		IsNormal() &&
		skills.GetDays(sk) < 1 &&
		skills.Num() > 0) {
		if (!Globals->MAGE_NONLEADERS ||
			!(SkillDefs[sk].flags & SkillType::MAGIC))
		return 0;
	}
	
	int curlev = GetRealSkill(sk);

	if (SkillDefs[sk].flags & SkillType::DISABLED) return 0;

	unsigned int c;
	for (c = 0; c < sizeof(SkillDefs[sk].depends)/sizeof(SkillDepend); c++) {
		if (SkillDefs[sk].depends[c].skill == NULL) return 1;
		SkillType *pS = FindSkill(SkillDefs[sk].depends[c].skill);
		if (pS && (pS->flags & SkillType::DISABLED)) continue;
		if (!CheckDepend(curlev, SkillDefs[sk].depends[c])) return 0;
	}
	return 1;
}

int Unit::Study(int sk, int days)
{
	Skill *s;

	if (Globals->SKILL_LIMIT_NONLEADERS && !IsLeader()) {
		if (SkillDefs[sk].flags & SkillType::MAGIC) {
			forlist(&skills) {
				s = (Skill *) elem;
				if (!(SkillDefs[s->type].flags & SkillType::MAGIC)) {
					Error("STUDY: Non-leader mages cannot possess non-magical skills.");
					return 0;
				}
			}
		} else if (skills.Num()) {
			s = (Skill *) skills.First();
			if ((s->type != sk) && (s->days > 0)) {
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
		if (GetRealSkill(sk) > 0)
			Error("STUDY: Doesn't have the pre-requisite skills to study that.");
		else
			Error("STUDY: Can't study that.");
		return 0;
	}

	skills.SetDays(sk, skills.GetDays(sk) + days);
	AdjustSkills();

	/* Check to see if we need to show a skill report */
	int lvl = GetRealSkill(sk);
	int shown = faction->skills.GetDays(sk);
	while (lvl > shown) {
		shown++;
		faction->skills.SetDays(sk, shown);
		faction->shows.Add(new ShowSkill(sk, shown));
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
		int m = SkillMax(SkillDefs[sk].abbr, i->type);
		if ((max == 0 && m > max) || (m < max)) max = m;
	}
	return max;
}

int Unit::Practice(int sk)
{
	int bonus, men, curlev, reqsk, reqlev, days;
	unsigned int i;

	bonus = Globals->SKILL_PRACTICE_AMOUNT;
	if (bonus == 0) bonus = Globals->REQUIRED_EXPERIENCE / 8;
	if (practiced || (bonus < 1)) return 1;
	days = skills.GetDays(sk);
	men = GetMen();

	if (GetAvailSkill(sk) > GetRealSkill(sk)) {
		// This is a skill granted by an item, so try to practice
		// the skills it depends on (if any)
		AString str;

		reqlev = 0;

		forlist (&items) {
			Item *it = (Item *)elem;
			if (ItemDefs[it->type].flags & ItemType::DISABLED) continue;
			if (ItemDefs[it->type].type & IT_MAGEONLY
					&& type != U_MAGE
					&& type != U_APPRENTICE
					&& type != U_GUARDMAGE)
				continue;
			if ((SkillDefs[sk].flags & SkillType::MAGIC)
					&& type != U_MAGE
					&& type != U_APPRENTICE
					&& type != U_GUARDMAGE)
				continue;
			if (it->num < GetMen())
				continue;
			str = ItemDefs[it->type].grantSkill;
			if (ItemDefs[it->type].grantSkill && LookupSkill(&str) == sk) {
				for (unsigned j = 0; j < sizeof(ItemDefs[0].fromSkills)
									 / sizeof(ItemDefs[0].fromSkills[0]); j++) {
					if (ItemDefs[it->type].fromSkills[j]) {
						int fromSkill;

						str = ItemDefs[it->type].fromSkills[j];

						fromSkill = LookupSkill(&str);
						if (fromSkill != -1 && GetRealSkill(fromSkill) > reqlev) {
							reqsk = fromSkill;
							reqlev = GetRealSkill(fromSkill);
						}
					}
				}
			}
		}

		if (reqlev > 0) {
			// Since granting items use the highest contributing
			// skill, practice that skill.
			Practice(reqsk);
			return 1;
		}
	}

	if (men < 1 || ((days < 1) && (!Globals->REQUIRED_EXPERIENCE))) return 0;

	int max = GetSkillMax(sk);
	curlev = GetRealSkill(sk);
	if (curlev >= max) return 0;

	for (i = 0; i < sizeof(SkillDefs[sk].depends)/sizeof(SkillDepend); i++) {
		AString skname = SkillDefs[sk].depends[i].skill;
		reqsk = LookupSkill(&skname);
		if (reqsk == -1) break;
		if (SkillDefs[reqsk].flags & SkillType::DISABLED) continue;
		if (SkillDefs[reqsk].flags & SkillType::NOEXP) continue;
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
		if (!Globals->REQUIRED_EXPERIENCE) {
			Study(sk, men * bonus);
		} else {
			Skill *s;
			// check if it's a nonleader and this is not it's
			// only skill
			if (Globals->SKILL_LIMIT_NONLEADERS && !IsLeader()) {
				forlist(&skills) {
					s = (Skill *) elem;
					if ((s->days > 0) && (s->type != sk)) {
						return 0;
					}
				}
			}
			// don't raise exp above the maximum days for
			// that unit
			int max = men * GetDaysByLevel(GetSkillMax(sk));
			int exp = skills.GetExp(sk);
			exp += men * bonus;
			if (exp > max) exp = max;
			skills.SetExp(sk, exp);
		}
		practiced = 1;
	}

	return bonus;
}

int Unit::IsLeader()
{
	if (GetLeaders()) return 1;
	return 0;
}

int Unit::IsNormal()
{
	if (GetMen() && !IsLeader()) return 1;
	return 0;
}

void Unit::AdjustSkills()
{
	if (!IsLeader() && Globals->SKILL_LIMIT_NONLEADERS) {
		//
		// Not a leader: can only know 1 skill
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
			forlist_reuse(&skills) {
				Skill *s = (Skill *) elem;
				if (s != maxskill) {
					// Allow multiple skills if they're all
					// magical ones
					if ((SkillDefs[maxskill->type].flags & SkillType::MAGIC) &&
							(SkillDefs[s->type].flags & SkillType::MAGIC) )
						continue;
					if ((Globals->REQUIRED_EXPERIENCE) && (s->exp > 0)) continue;
					skills.Remove(s);
					delete s;
				}
			}
		}
	}

	// Everyone: limit all skills to their maximum level
	forlist(&skills) {
		Skill *theskill = (Skill *) elem;
		int max = GetSkillMax(theskill->type);
		if (GetRealSkill(theskill->type) >= max) {
			theskill->days = GetDaysByLevel(max) * GetMen();
		}
	}
}

int Unit::MaintCost()
{
	int retval = 0;
	int i;
	if (type == U_WMON || type == U_GUARD || type == U_GUARDMAGE) return 0;

	int leaders = GetLeaders();
	if (leaders < 0) leaders = 0;
	int nonleaders = GetMen() - leaders;
	if (nonleaders < 0) nonleaders = 0;

	// Handle leaders
	// Leaders are counted at maintenance_multiplier * skills in all except
	// the case where it's not being used (mages, leaders, all)
	if (Globals->MULTIPLIER_USE != GameDefs::MULT_NONE) {
		i = leaders * SkillLevels() * Globals->MAINTENANCE_MULTIPLIER;
		if (i < (leaders * Globals->LEADER_COST))
			i = leaders * Globals->LEADER_COST;
	} else
		i = leaders * Globals->LEADER_COST;
	retval += i;

	// Handle non-leaders
	// Non leaders are counted at maintenance_multiplier * skills only if
	// all characters pay that way.
	if (Globals->MULTIPLIER_USE == GameDefs::MULT_ALL) {
		i = nonleaders * SkillLevels() * Globals->MAINTENANCE_MULTIPLIER;
		if (i < (nonleaders * Globals->MAINTENANCE_COST))
			i = nonleaders * Globals->MAINTENANCE_COST;
	} else
		i = nonleaders * Globals->MAINTENANCE_COST;
	retval += i;

	return retval;
}

void Unit::Short(int needed, int hunger)
{
	int i, n = 0, levels;

	if (faction->IsNPC())
		return; // Don't starve monsters and the city guard!

	if (needed < 1 && hunger < 1) return;

	switch(Globals->SKILL_STARVATION) {
		case GameDefs::STARVE_MAGES:
			if (type == U_MAGE) SkillStarvation();
			return;
		case GameDefs::STARVE_LEADERS:
			if (GetLeaders()) SkillStarvation();
			return;
		case GameDefs::STARVE_ALL:
			SkillStarvation();
			return;
	}

	for (i = 0; i<= NITEMS; i++) {
		if (!(ItemDefs[ i ].type & IT_MAN)) {
			// Only men need sustenance.
			continue;
		}

		if (ItemDefs[i].type & IT_LEADER) {
			// Don't starve leaders just yet.
			continue;
		}

		while (GetMen(i)) {
			if (getrandom(100) < Globals->STARVE_PERCENT) {
				SetMen(i, GetMen(i) - 1);
				n++;
			}
			if (Globals->MULTIPLIER_USE == GameDefs::MULT_ALL) {
				levels = SkillLevels();
				i = levels * Globals->MAINTENANCE_MULTIPLIER;
				if (i < Globals->MAINTENANCE_COST)
					i = Globals->MAINTENANCE_COST;
				needed -= i;
			} else
				needed -= Globals->MAINTENANCE_COST;
			hunger -= Globals->UPKEEP_MINIMUM_FOOD;
			if (needed < 1 && hunger < 1) {
				if (n) Error(AString(n) + " starve to death.");
				return;
			}
		}
	}

	// Now starve leaders
	for (int i = 0; i<= NITEMS; i++) {
		if (!(ItemDefs[ i ].type & IT_MAN)) {
			// Only men need sustenance.
			continue;
		}

		if (!(ItemDefs[i].type & IT_LEADER)) {
			// now we're doing leaders
			continue;
		}

		while (GetMen(i)) {
			if (getrandom(100) < Globals->STARVE_PERCENT) {
				SetMen(i, GetMen(i) - 1);
				n++;
			}
			if (Globals->MULTIPLIER_USE != GameDefs::MULT_NONE) {
				levels = SkillLevels();
				i = levels * Globals->MAINTENANCE_MULTIPLIER;
				if (i < Globals->LEADER_COST)
					i = Globals->LEADER_COST;
				needed -= i;
			} else
				needed -= Globals->LEADER_COST;
			hunger -= Globals->UPKEEP_MINIMUM_FOOD;
			if (needed < 1 && hunger < 1) {
				if (n) Error(AString(n) + " starve to death.");
				return;
			}
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
		// except ship items
		if (ItemDefs[i->type].type & IT_SHIP) continue;
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
		// except ship items
		if (ItemDefs[i->type].type & IT_SHIP) continue;
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
		if (ItemDefs[i->type].hitchItem != -1) {
			int hitch = ItemDefs[i->type].hitchItem;
			if (!(ItemDefs[hitch].flags & ItemType::DISABLED)) {
				int hitches = items.GetNum(hitch);
				int hitched = i->num;
				if (hitched > hitches) hitched = hitches;
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
	if (IsAlive() && (SwimmingCapacity() >= items.Weight())) return 1;
	return 0;
}

int Unit::CanSwim()
{
	if (this->CanReallySwim())
		return 1;
	if ((Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) && this->CanFly())
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

int Unit::MoveType(ARegion *r)
{
	int weight;

	if (!r)
		r = object->region;
	weight = items.Weight();
	if (!weight)
		return M_NONE;
	if (CanFly(weight))
		return M_FLY;
	if (TerrainDefs[r->type].similar_type != R_OCEAN) {
		if (CanRide(weight))
			return M_RIDE;
		if (CanWalk(weight))
			return M_WALK;
	} else {
		/* Check if we should be able to 'swim' */
		/* This should become it's own M_TYPE sometime */
		if (CanSwim())
			return M_SWIM;
	}
	if (r->type == R_NEXUS)
		return M_WALK;
	return M_NONE;
}

static int ContributesToMovement(int movetype, int item)
{
	switch(movetype) {
		case M_WALK:
			if (ItemDefs[item].walk > 0)
				return ItemDefs[item].walk;
			break;
		case M_RIDE:
			if (ItemDefs[item].ride > 0)
				return ItemDefs[item].ride;
			break;
		case M_FLY:
			if (ItemDefs[item].fly > 0)
				return ItemDefs[item].fly;
			break;
		case M_SWIM:
			// incomplete ship items do have a "swimming"
			// capacity given, but don't help us to swim
			if (ItemDefs[item].type & IT_SHIP)
				return 0;
			if (ItemDefs[item].swim > 0)
				return ItemDefs[item].swim;
			break;
	}
	
	return 0;
}

int Unit::CalcMovePoints(ARegion *r)
{
	int movetype, speed, weight, cap, hitches;
	Item *i;

	movetype = MoveType(r);
	speed = 0;
	if (movetype == M_NONE)
		return 0;

	forlist(&items) {
		i = (Item *) elem;
		if (ContributesToMovement(movetype, i->type)) {
			if (ItemDefs[i->type].speed > speed)
				speed = ItemDefs[i->type].speed;
		}
	}
	weight = items.Weight();
	while (weight > 0 && speed > 0) {
		forlist(&items) {
			i = (Item *) elem;
			cap = ContributesToMovement(movetype, i->type);
			if (ItemDefs[i->type].speed == speed) {
				if (cap > 0)
					weight -= cap * i->num;
				else if (ItemDefs[i->type].hitchItem != -1) {
					hitches = items.GetNum(ItemDefs[i->type].hitchItem);
					if (i->num < hitches)
						hitches = i->num;
					weight -= hitches * ItemDefs[i->type].hitchwalk;
				}
			}
		}
		if (weight > 0) {
			// Hm, can't move at max speed.  There must be
			// items with different speeds, and we have to
			// use some of the slower ones...
			speed--;
		}
	}

	if (weight > 0)
		return 0; // not that this should be possible!

	if (movetype == M_FLY) {
		if (GetAttribute("wind") > 0)
			speed += Globals->FLEET_WIND_BOOST;
	}

	if (speed > Globals->MAX_SPEED)
		speed = Globals->MAX_SPEED;

	return speed;
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
	int mp = CalcMovePoints() - moved;
	if (mp < (r2->MoveCost(mt, r1, dir, 0))) return 0;
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
	if (!u) return 0;

	int amulets = items.GetNum(I_AMULETOFTS);
	if ((u->items.GetNum(I_RINGOFI) < 1) || (amulets < 1)) return 0;
	int men = GetMen();
	if (men <= amulets) return 1;
	if (!Globals->PROPORTIONAL_AMTS_USAGE) return 0;
	if (getrandom(men) < amulets) return 1;
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

/* This function was modified to either return the amount of
   taxes this unit is eligible for (numtaxers == 0) or the
   number of taxing men (numtaxers > 0).
*/
int Unit::Taxers(int numtaxers)
{
	int totalMen = GetMen();
	int illusions = 0;
	int creatures = 0;
	int taxers = 0;
	int basetax = 0;
	int weapontax = 0;
	int armortax = 0;
	
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
	int numArmor = 0;

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
			int num = pItem->num;
			int basesk = 0;
			AString skname = pWep->baseSkill;
			int sk = LookupSkill(&skname);
			if (sk != -1) basesk = GetSkill(sk);
			if (basesk == 0) {
				skname = pWep->orSkill;
				sk = LookupSkill(&skname);
				if (sk != -1) basesk = GetSkill(sk);
			}
			if (!(pWep->flags & WeaponType::NEEDSKILL)) {
				if (basesk) {
					numUsableMelee += num;
				}
				numMelee += num;
			} else if (pWep->flags & WeaponType::NOFOOT) {
				if (basesk) {
					numUsableMounted += num;
				}
				numMounted += num;
			} else {
				if (pWep->flags & WeaponType::RANGED) {
					if (basesk) {
						numUsableBows += num;
					}
					numBows += num;
				} else {
					if (basesk) {
						numUsableMelee += num;
					}
					numMelee += num;
				}
			}
		}

		if (ItemDefs[pItem->type].type & IT_MOUNT) {
			MountType *pm = FindMount(ItemDefs[pItem->type].abr);
			if (pm->skill) {
				AString skname = pm->skill;
				int sk = LookupSkill(&skname);
				if (pm->minBonus <= GetSkill(sk))
					numUsableMounts += pItem->num;
			} else
				numUsableMounts += pItem->num;
			numMounts += pItem->num;
		}

		if (ItemDefs[pItem->type].type & IT_MONSTER) {
			if (ItemDefs[pItem->type].type & IT_ILLUSION)
				illusions += pItem->num;
			else
				creatures += pItem->num;
		}
		
		if (ItemDefs[pItem->type].type & IT_ARMOR) {
			numArmor += pItem->num;
		}
	}


	// Ok, now process the counts!
	if ((Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_COMBAT_SKILL) &&
		 GetSkill(S_COMBAT)) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_BOW_SKILL) &&
		 (GetSkill(S_CROSSBOW) || GetSkill(S_LONGBOW))) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_RIDING_SKILL) &&
		 GetSkill(S_RIDING)) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_STEALTH_SKILL) &&
		 GetSkill(S_STEALTH))) {
		basetax = totalMen;
		taxers = totalMen;
		
		// Weapon tax bonus
		if ((Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_COMBAT_SKILL) &&
		 GetSkill(S_COMBAT)) ||
		((Globals->WHO_CAN_TAX & GameDefs::TAX_STEALTH_SKILL) &&
		 GetSkill(S_STEALTH))) {
		 	if (numUsableMounted > numUsableMounts) {
		 		weapontax = numUsableMounts;
		 	} else {
		 		weapontax = numUsableMounted;
		 	}
		 	weapontax += numMelee;
		 }
		 
		if (((Globals->WHO_CAN_TAX & GameDefs::TAX_BOW_SKILL) &&
		 (GetSkill(S_CROSSBOW) || GetSkill(S_LONGBOW)))) {
		 	weapontax += numUsableBows;
		 }
		if ((Globals->WHO_CAN_TAX & GameDefs::TAX_RIDING_SKILL) &&
		 GetSkill(S_RIDING)) {
		 	if (weapontax < numUsableMounts) weapontax = numUsableMounts;
		 }
		
	} else {

		if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_WEAPON) {
			if (numUsableMounted > numUsableMounts) {
				weapontax = numUsableMounts;
				taxers = numUsableMounts;
				numMounts -= numUsableMounts;
				numUsableMounts = 0;
			} else {
				weapontax = numUsableMounted;
				taxers = numUsableMounted;
				numMounts -= numUsableMounted;
				numUsableMounts -= numUsableMounted;
			}
			weapontax += numMelee + numUsableBows;
			taxers += numMelee + numUsableBows;
		} else if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANY_WEAPON) {
			weapontax = numMelee + numBows + numMounted;
			taxers = numMelee + numBows + numMounted;
		} else {
			if (Globals->WHO_CAN_TAX &
					GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL) {
				if (numUsableMounted > numUsableMounts) {
					weapontax += numUsableMounts;
					taxers += numUsableMounts;
					numMounts -= numUsableMounts;
					numUsableMounts = 0;
				} else {
					weapontax += numUsableMounted;
					taxers += numUsableMounted;
					numMounts -= numUsableMounted;
					numUsableMounts -= numUsableMounted;
				}
				weapontax += numUsableMelee;
				taxers += numUsableMelee;
			}
			if (Globals->WHO_CAN_TAX &
					GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON) {
				weapontax += numUsableBows;
				taxers += numUsableBows;
			}
		}

		if (Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE) {
			weapontax += numMounts;
			taxers += numMounts;
		}
		else if (Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE_AND_RIDING_SKILL) {
			weapontax += numUsableMounts;
			taxers += numUsableMounts;
		}

		if (Globals->WHO_CAN_TAX & GameDefs::TAX_BATTLE_ITEM) {
			weapontax += numBattle;
			taxers += numBattle;
		}
		else if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_BATTLE_ITEM) {
			weapontax += numUsableBattle;
			taxers += numUsableBattle;
		}
		
	}

	// Ok, all the items categories done - check for mages taxing
	if (type == U_MAGE) {
		if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANY_MAGE) {
			basetax = totalMen;
			taxers = totalMen;
		}
		else {
			if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_COMBAT_SPELL) {
				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_DAMAGE) &&
						SkillDefs[combat].flags & SkillType::DAMAGE) {
					basetax = totalMen;
					taxers = totalMen;
				}

				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_FEAR) &&
						SkillDefs[combat].flags & SkillType::FEAR) {
					basetax = totalMen;
					taxers = totalMen;
				}

				if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_OTHER) &&
						SkillDefs[combat].flags & SkillType::MAGEOTHER) {
					basetax = totalMen;
					taxers = totalMen;
				}
			} else {
				forlist(&skills) {
					Skill *s = (Skill *)elem;
					if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_DAMAGE) &&
							SkillDefs[s->type].flags & SkillType::DAMAGE) {
						basetax = totalMen;
						taxers = totalMen;
						break;
					}
					if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_FEAR) &&
							SkillDefs[s->type].flags & SkillType::FEAR) {
						basetax = totalMen;
						taxers = totalMen;
						break;
					}
					if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_OTHER) &&
							SkillDefs[s->type].flags & SkillType::MAGEOTHER) {
						basetax = totalMen;
						taxers = totalMen;
						break;
					}
				}
			}
		}
	}
	
	armortax = numArmor;
	
	// Check for overabundance
	if (weapontax > totalMen) weapontax = totalMen;
	if (armortax > weapontax) armortax = weapontax;
	
	// Adjust basetax in case of weapon taxation
	if (basetax < weapontax) basetax = weapontax;

	// Now check for an overabundance of tax enabling objects
	if (taxers > totalMen) taxers = totalMen;

	// And finally for creatures
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_CREATURES)
		basetax += creatures;
		taxers += creatures;
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS)
		basetax += illusions;
		taxers += illusions;
	
	if (numtaxers) return(taxers);

	int taxes = Globals->TAX_BASE_INCOME * basetax
		+ Globals->TAX_BONUS_WEAPON * weapontax
		+ Globals->TAX_BONUS_ARMOR * armortax;
	return(taxes);
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
	if (Taxers(1)) {
		if (x->guard != GUARD_SET && x->guard != GUARD_ADVANCE)
			guard = x->guard;
	} else {
		if (x->guard == GUARD_AVOID)
			guard = GUARD_AVOID;
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
	if (!canFly && !canRide) return -1;

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

	if (pMnt->skill) {
		AString skname = pMnt->skill;
		int sk = LookupSkill(&skname);
		bonus = GetSkill(sk);
		if (bonus < pMnt->minBonus) {
			// Unit isn't skilled enough for this mount
			bonus = 0;
			return -1;
		}
		// Limit to max mount bonus;
		if (bonus > pMnt->maxBonus) bonus = pMnt->maxBonus;
		// If the mount can fly and the terrain doesn't allow
		// flying mounts, limit the bonus to the maximum hampered
		// bonus allowed by the mount
		if (ItemDefs[item].fly && !canFly) {
			if (bonus > pMnt->maxHamperedBonus)
				bonus = pMnt->maxHamperedBonus;
		}

		// Practice the mount's skill
		Practice(sk);
	}

	// Remove the mount from the unit to attach it to the soldier
	// UNLESS it IS the soldier (looking at you, Centaurs)
	if (!(ItemDefs[item].type & IT_MAN))
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

	// Found a weapon, check flags and skills
	int baseSkillLevel = CanUseWeapon(pWep, riding);
	// returns -1 if weapon cannot be used, else the usable skill level
	if (baseSkillLevel == -1) return -1;

	// Attack and defense skill
	attackBonus = baseSkillLevel + pWep->attackBonus;
	if (pWep->flags & WeaponType::NOATTACKERSKILL)
		defenseBonus = pWep->defenseBonus;
	else
		defenseBonus = baseSkillLevel + pWep->defenseBonus;
	// Riding bonus
	if (pWep->flags & WeaponType::RIDINGBONUS) attackBonus += ridingBonus;
	if (pWep->flags & (WeaponType::RIDINGBONUSDEFENSE|WeaponType::RIDINGBONUS))
		defenseBonus += ridingBonus;
	// Number of attacks
	attacks = pWep->numAttacks;
	// Note: NUM_ATTACKS_SKILL must be > NUM_ATTACKS_HALF_SKILL
	if (attacks >= WeaponType::NUM_ATTACKS_SKILL)
		attacks += baseSkillLevel - WeaponType::NUM_ATTACKS_SKILL;
	else if (attacks >= WeaponType::NUM_ATTACKS_HALF_SKILL)
		attacks += (baseSkillLevel +1)/2 - WeaponType::NUM_ATTACKS_HALF_SKILL;
	// Sanity check
	if (attacks == 0) attacks = 1;

	// get the weapon
	items.SetNum(item, num-1);
	return item;
}

void Unit::MoveUnit(Object *toobj)
{
	if (object) object->units.Remove(this);
	object = toobj;
	if (object) {
		object->units.Add(this);
	}
}

void Unit::DiscardUnfinishedShips() {
	int discard = 0;
	// remove all unfinished ship-type items
	for (int i=0; i<NITEMS; i++) {
		if (ItemDefs[i].type & IT_SHIP) {
			if (items.GetNum(i) > 0) discard = 1;
			items.SetNum(i,0);
		}
	}
	if (discard > 0) Event("discards all unfinished ships.");	
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

int Unit::GetAttribute(char const *attrib)
{
	AttribModType *ap = FindAttrib(attrib);
	if (ap == NULL) return 0;
	AString temp;
	int base = 0;
	int bonus = 0;
	int monbase = -1;
	int monbonus = 0;

	if (ap->flags & AttribModType::CHECK_MONSTERS) {
		forlist (&items) {
			Item *i = (Item *) elem;
			if (ItemDefs[i->type].type & IT_MONSTER) {
				MonType *mp = FindMonster(ItemDefs[i->type].abr,
						(ItemDefs[i->type].type & IT_ILLUSION));
				int val = 0;
				temp = attrib;
				if (temp == "observation") val = mp->obs;
				else if (temp == "stealth") val = mp->stealth;
				else if (temp == "tactics") val = mp->tactics;
				else continue;
				if (monbase == -1) monbase = val;
				else if (ap->flags & AttribModType::USE_WORST)
					monbase = (val < monbase) ? val : monbase;
				else
					monbase = (val > monbase) ? val : monbase;
			}
		}
	}

	for (int index = 0; index < 5; index++) {
		int val = 0;
		if (ap->mods[index].flags & AttribModItem::SKILL) {
			temp = ap->mods[index].ident;
			int sk = LookupSkill(&temp);
			val = GetAvailSkill(sk);
			if (ap->mods[index].modtype == AttribModItem::UNIT_LEVEL_HALF) {
				val = ((val + 1)/2) * ap->mods[index].val;
			} else if (ap->mods[index].modtype == AttribModItem::CONSTANT) {
				val = ap->mods[index].val;
			} else {
				val *= ap->mods[index].val;
			}
		} else if (ap->mods[index].flags & AttribModItem::ITEM) {
			val = 0;
			temp = ap->mods[index].ident;
			int item = LookupItem(&temp);
			if (item != -1) {
				if (ItemDefs[item].type & IT_MAGEONLY
					&& type != U_MAGE
					&& type != U_APPRENTICE
					&& type != U_GUARDMAGE) {
					// Ignore mage only items for non-mages
				} else if (ap->mods[index].flags & AttribModItem::PERMAN) {
					int men = GetMen();
					if (men <= items.GetNum(item))
						val = ap->mods[index].val;
				} else {
					if (items.GetNum(item) > 0)
						val = ap->mods[index].val;
				}
			}
		} else if (ap->mods[index].flags & AttribModItem::FLAGGED) {
			temp = ap->mods[index].ident;
			if (temp == "invis")
				val = (GetFlag(FLAG_INVIS) ? ap->mods[index].val : 0);
			if (temp == "guard")
				val = (guard == GUARD_GUARD ? ap->mods[index].val : 0);

		}
		if (ap->mods[index].flags & AttribModItem::NOT)
			val = ((val == 0) ? ap->mods[index].val : 0);
		if (val && ap->mods[index].modtype == AttribModItem::FORCECONSTANT)
			return val;
		// Only flags can add to monster bonuses
		if (ap->mods[index].flags & AttribModItem::FLAGGED) {
			if (ap->flags & AttribModType::CHECK_MONSTERS) monbonus += val;
		}
		if (ap->mods[index].flags & AttribModItem::CUMULATIVE)
			base += val;
		else if (val > bonus) bonus = val;
	}

	base += bonus;

	if (monbase != -1) {
		monbase += monbonus;
		if (GetMen() > 0) {
			if (ap->flags & AttribModType::USE_WORST)
				base = (monbase < base) ? monbase : base;
			else
				base = (monbase > base) ? monbase : base;
		}
		else
			base = monbase; // monster units have no men
	}	
	return base;
}

int Unit::PracticeAttribute(char const *attrib)
{
	AttribModType *ap = FindAttrib(attrib);
	if (ap == NULL) return 0;
	for (int index = 0; index < 5; index++) {
		if (ap->mods[index].flags & AttribModItem::SKILL) {
			AString temp = ap->mods[index].ident;
			int sk = LookupSkill(&temp);
			if (sk != -1)
				if (Practice(sk)) return 1;
		}
	}
	return 0;
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
		levels += GetLevelByDays(s->days/GetMen());
	}
	return levels;
}

Skill *Unit::GetSkillObject(int sk)
{
	forlist(&skills) {
		Skill *s = (Skill *)elem;
		if (s->type == sk)
			return s;
	}
	return NULL;
}

void Unit::SkillStarvation()
{
	int can_forget[NSKILLS];
	int count = 0;
	int i;
	for (i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].flags & SkillType::DISABLED) {
			can_forget[i] = 0;
			continue;
		}
		if (GetSkillObject(i)) {
			can_forget[i] = 1;
			count++;
		} else {
			can_forget[i] = 0;
		}
	}
	for (i = 0; i < NSKILLS; i++) {
		if (!can_forget[i]) continue;
		Skill *si = GetSkillObject(i);
		for (int j=0; j < NSKILLS; j++) {
			if (SkillDefs[j].flags & SkillType::DISABLED) continue;
			Skill *sj = GetSkillObject(j);
			int dependancy_level = 0;
			unsigned int c;
			for (c=0;c < sizeof(SkillDefs[i].depends)/sizeof(SkillDepend);c++) {
				AString skname = SkillDefs[i].depends[c].skill;
				if (skname == SkillDefs[j].abbr) {
					dependancy_level = SkillDefs[i].depends[c].level;
					break;
				}
			}
			if (dependancy_level > 0) {
				if (GetLevelByDays(sj->days) == GetLevelByDays(si->days)) {
					can_forget[j] = 0;
					count--;
				}
			}
		}
	}
	if (!count) {
		forlist(&items) {
			Item *i = (Item *)elem;
			if (ItemDefs[i->type].type & IT_MAN) {
				count += items.GetNum(i->type);
				items.SetNum(i->type, 0);
			}
		}
		AString temp = AString(count) + " starve to death.";
		Error(temp);
		return;
	}
	count = getrandom(count)+1;
	for (i = 0; i < NSKILLS; i++) {
		if (can_forget[i]) {
			if (--count == 0) {
				Skill *s = GetSkillObject(i);
				AString temp = AString("Starves and forgets one level of ")+
					SkillDefs[i].name + ".";
				Error(temp);
				switch(GetLevelByDays(s->days)) {
					case 1:
						s->days -= 30;
						if (s->days <= 0)
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
		if (pWep->flags & WeaponType::NOFOOT) return -1;
	}
	else if (pWep->flags & WeaponType::NOMOUNT) return -1;
	return CanUseWeapon(pWep);
}

int Unit::CanUseWeapon(WeaponType *pWep)
{
	int baseSkillLevel = 0;
	int tempSkillLevel = 0;

	int bsk, orsk;
	AString skname;
	if (pWep->baseSkill != NULL) {
		skname = pWep->baseSkill;
		bsk = LookupSkill(&skname);
		if (bsk != -1) baseSkillLevel = GetSkill(bsk);
	}

	if (pWep->orSkill != NULL) {
		skname = pWep->orSkill;
		orsk = LookupSkill(&skname);
		if (orsk != -1) tempSkillLevel = GetSkill(orsk);
	}

	if (tempSkillLevel > baseSkillLevel) {
		baseSkillLevel = tempSkillLevel;
		Practice(orsk);
	} else
		Practice(bsk);

	if (pWep->flags & WeaponType::NEEDSKILL && !baseSkillLevel) return -1;

	return baseSkillLevel;
}

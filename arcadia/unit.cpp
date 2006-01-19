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
	label = 0;
	num = 0;
	type = U_NORMAL;
	dead = 0;
	energy = 0;
	resurrects = 0;
	mastery = 0;
	transferred = 0;
	foggy = 0;
	faction = 0;
	formfaction = 0;
	alias = 0;
	guard = GUARD_NONE;
	reveal = REVEAL_NONE;
	tactics = TACTICS_NONE;
	if(Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_NONE) flags = 0;
	else flags = FLAG_NOCROSS_WATER;
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
	herostudyorders = NULL;
	activecastorder = NULL;
	teleportorders = NULL;
	inTurnBlock = 0;
	presentTaxing = 0;
	presentMonthOrders = NULL;
	former = NULL;
	free = 0;
	practiced = 0;
	numtraded = 0;
	nummerchanted = 0;
	numquartermastered = 0;
	ClearOrders();
}

Unit::Unit(int seq, Faction *f, int a)
{
	num = seq;
	type = U_NORMAL;
	name = new AString;
	describe = 0;
	label = 0;
	*name = AString("Unit (") + num + ")";

	dead = 0;
	energy = 0;
	resurrects = 0;
	mastery = 0;
	transferred = 0;
	foggy = 0;

	faction = f;
	formfaction = f;
	alias = a;
	guard = 0;
	reveal = REVEAL_NONE;
	tactics = TACTICS_NONE;
	if(Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_NONE) flags = 0;
	else flags = FLAG_NOCROSS_WATER;
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
	herostudyorders = NULL;
	activecastorder = NULL;
	teleportorders = NULL;
	inTurnBlock = 0;
	presentTaxing = 0;
	presentMonthOrders = NULL;
	former = NULL;
	free = 0;
	practiced = 0;
	numtraded = 0;
	nummerchanted = 0;
	numquartermastered = 0;
	ClearOrders();
}

Unit::~Unit()
{
	if (monthorders) delete monthorders;
	if (herostudyorders) delete herostudyorders;
	if (presentMonthOrders) delete presentMonthOrders;
	if (attackorders) delete attackorders;
	if (stealorders) delete stealorders;
	if (name) delete name;
	if (describe) delete describe;
	if (label) delete label;
#ifdef DEBUG
Awrite("Deleting Unit");
#endif
}

void Unit::SetMonFlags()
{
	guard = GUARD_AVOID;
	SetFlag(FLAG_HOLDING, 1);
//	reveal = REVEAL_FACTION; // BS Edit - monsters should be obviously monsters. But this would make them always visible regardless of stealth :(.
	tactics = TACTICS_AGGRESSIVE;
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
	if (label) {
		s->PutStr(*label);
	} else {
		s->PutStr("none");
	}
	s->PutInt(num);
	s->PutInt(type);
	if (type == U_MAGE && Globals->ARCADIA_MAGIC) {
	    s->PutInt(energy);
	    s->PutInt(mastery);
	    s->PutInt(resurrects);
	    s->PutInt(dead);
	}
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
	label = s->GetStr();
	if (*label == "none") {
		delete label;
		label = 0;
	}
	num = s->GetInt();
	type = s->GetInt();
	if(type == U_MAGE && Globals->ARCADIA_MAGIC) {
	    energy = s->GetInt();
	    mastery = s->GetInt();
	    resurrects = s->GetInt();
	    dead = s->GetInt();
	}
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
	AString temp = AString("");
	
	if (combat != -1) {
		temp += AString(". Combat spell: ") + SkillStrs(combat);
	} else temp += ". Combat spell: none";
	
	if (Globals->ARCADIA_MAGIC) { //not relevant for guardmages.
	    temp += AString(". Energy: ") + energy + "/" + MaxEnergy();	
	    temp += AString(". Recharge: ") + EnergyRecharge();
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
		if(SkillDefs[i].depends[0].skill != NULL) {
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
	if(reveal == REVEAL_FACTION || obs > stealth) {
		ret += ", ";
		ret += *faction->name;
	}
	return ret;
}

int Unit::CanGetSpoil(Item *i)
{
    //if can carry unlimited items, we return (i->num+1) as this always exceeds what is available ...
	if(!i) return 0;
	int itemweight = ItemDefs[i->type].weight;
	if(!itemweight) return (i->num+1); // any unit can carry 0 weight spoils

    int num;

	if(flags & FLAG_NOSPOILS) return 0; //no spoils!
	if(flags & FLAG_FLYSPOILS) {
        itemweight -= ItemDefs[i->type].fly;
        if(itemweight < 1) return (i->num+1);
        num = (FlyingCapacity() - Weight())/itemweight; //we know itemweight is not zero
        if(num < 1) return 0;
        else return num;
    }
	if(flags & FLAG_RIDESPOILS) {
        itemweight -= ItemDefs[i->type].ride;
        if(itemweight < 1) return (i->num+1);
        num = (RidingCapacity() - Weight())/itemweight; //we know itemweight is not zero
        if(num < 1) return 0;
        else return num;
    }
	if(flags & FLAG_WALKSPOILS) {
        itemweight -= ItemDefs[i->type].walk;
        if(itemweight < 1) return (i->num+1);
        num = (WalkingCapacity() - Weight())/itemweight; //we know itemweight is not zero
        if(num < 1) return 0;
        else return num;
    }
	if(flags & FLAG_SWIMSPOILS) {
        itemweight -= ItemDefs[i->type].swim;
        if(itemweight < 1) return (i->num+1);
        num = (SwimmingCapacity() - Weight())/itemweight; //we know itemweight is not zero
        if(num < 1) return 0;
        else return num;
    }
	if(flags & FLAG_SAILSPOILS) {
        if(itemweight < 1) return (i->num+1); //this should not be needed, but hey, why not!
        if(!object->IsBoat()) return (i->num+1); //if we're not on a boat, consider this to be spoils all
        //we need num to be (spare sailing capacity) divided by itemweight
        num = (ObjectDefs[object->type].capacity - object->Weight())/itemweight; //we know itemweight is not zero
        if(num < 1) return 0;
        else return num;
    }
	return (i->num+1); // all spoils
}

AString Unit::SpoilsReport() {
	AString temp;
	if(GetFlag(FLAG_NOSPOILS)) temp = ", weightless battle spoils";
	else if(GetFlag(FLAG_FLYSPOILS)) temp = ", flying battle spoils";
	else if(GetFlag(FLAG_WALKSPOILS)) temp = ", walking battle spoils";
	else if(GetFlag(FLAG_RIDESPOILS)) temp = ", riding battle spoils";
	else if(GetFlag(FLAG_SWIMSPOILS)) temp = ", swimming battle spoils";
	else if(GetFlag(FLAG_SAILSPOILS)) temp = ", sailing battle spoils";
	return temp;
}

void Unit::WriteReport(Areport *f, int obs, int truesight, int detfac,
			   int autosee, int attitude)
{

    if(dead) return; // Do not see dead units

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
		if (!Globals->ARCADIA_MAGIC && (GetSkill(S_ILLUSION) > truesight) ) {
			truesight = 0;
		} else if (Globals->ARCADIA_MAGIC && (GetSkill(S_BASE_ILLUSION) > truesight)) {
		    truesight = 0;
        } else if (truesight != 0) {  //BS mod: if no illusion skill, eg monsters, still need TRUE 1 to see they are illusions!
			truesight = 1;
		}
	}

	if (detfac && obs != 2) obs = 1;

	/* Write the report */
	AString temp;
	if (obs == 2) {
		temp += AString("* ") + *name;
	} else {
		if (faction->showunitattitudes) {
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
	if (obs > 0) temp += AString(", ") + *faction->name + " [" + EthnicityString(faction->ethnicity) + "]";
	if(obs == 2 && label) temp += AString(", ") + *label;
	if (obs > 0) {		
		if (guard == GUARD_AVOID) temp += ", avoiding";
		if (GetFlag(FLAG_BEHIND)) temp += ", behind";
	}

	if (obs == 2) {
		if (GetFlag(FLAG_COMMANDER)) temp += ", commanding faction";
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
		if (GetFlag(FLAG_FIGHTASFOOT)) temp += ", fighting as foot";
		if (GetFlag(FLAG_FIGHTASRIDE)) temp += ", fighting as cavalry";
		if (tactics == TACTICS_AGGRESSIVE) temp += ", fighting aggressively";
		if (tactics == TACTICS_DEFENSIVE) temp += ", fighting defensively";
		temp += SpoilsReport();
	}

	temp += items.Report(obs, truesight, 0, type);

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
	if (label) temp += AString(", ") + *label;
	if (guard == GUARD_AVOID) temp += ", avoiding";
	if (GetFlag(FLAG_BEHIND)) temp += ", behind";
	if (GetFlag(FLAG_COMMANDER)) temp += ", commanding faction";
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

	temp += items.Report(2, 1, 0, type);
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
  if (GetFlag(FLAG_FIGHTASFOOT)) *temp += ", fighting as foot";
  if (GetFlag(FLAG_FIGHTASRIDE)) *temp += ", fighting as cavalry";

  *temp += items.BattleReport(type);

  forlist (&skills) {
	  Skill *s = (Skill *)elem;
	  if (SkillDefs[s->type].flags & SkillType::BATTLEREP) {
		  int lvl = GetSkill(s->type);
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
	enter = 0;
	build = NULL;
	leftShip = 0;
	destroy = 0;
	if (attackorders) delete attackorders;
	attackorders = 0;
	if (evictorders) delete evictorders;
	evictorders = 0;
	if (stealorders) delete stealorders;
	stealorders = 0;
	promote = 0;
	promotequiet = 0;
	taxing = TAX_NONE;
	advancefrom = 0;
	movepoints = 0;
	if (monthorders) delete monthorders;
	monthorders = 0;
	if (herostudyorders) delete herostudyorders;
	herostudyorders = 0;
	inTurnBlock = 0;
	presentTaxing = 0;
	if (presentMonthOrders) delete presentMonthOrders;
	presentMonthOrders = 0;
	if (teleportorders) delete teleportorders;
	teleportorders = 0;
	typeorders.DeleteAll();
	giveorders.DeleteAll();
	sendorders.DeleteAll();
	withdraworders.DeleteAll();
	wishdraworders.DeleteAll();
	wishskillorders.DeleteAll();
	bankorders.DeleteAll();
	buyorders.DeleteAll();
	sellorders.DeleteAll();
	forgetorders.DeleteAll();
	exchangeorders.DeleteAll();
	transportorders.DeleteAll();	
}


void Unit::SafeClearOrders()
{
//Called when a unit dies, and becomes a spirit of the dead. Cannot delete
//any orders which may have led to the battle, as these are likely to be deleted
//later.
//Instead, simply leave them and make sure they cannot lead to any further game
//effects (ie unit cannot attack & steal orders are hardcoded so dead units can't steal).
//Since they are not reset to zero, should be no memory leaks.

//don't let it attack further
	canattack = 0;
	nomove = 1;
	enter = 0;
	build = NULL;
	leftShip = 0;
	destroy = 0;
//don't delete attack order
	if (evictorders) delete evictorders;
	evictorders = 0;
//don't delete steal orders
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
	activecastorder = 0;
	if (teleportorders) delete teleportorders;
	teleportorders = 0;
	castlistorders.DeleteAll();
	giveorders.DeleteAll();
	sendorders.DeleteAll();
	withdraworders.DeleteAll();
	wishdraworders.DeleteAll();
	wishskillorders.DeleteAll();
	bankorders.DeleteAll();
	buyorders.DeleteAll();
	sellorders.DeleteAll();
	forgetorders.DeleteAll();
	exchangeorders.DeleteAll();
	transportorders.DeleteAll();
}

void Unit::ClearCastOrder()
{
    castlistorders.DeleteAll();
}

void Unit::ClearTeleportOrders()
{
    if(!Globals->ARCADIA_MAGIC) {
	    castlistorders.DeleteAll(); //can't have more than one cast order
    }
	if (teleportorders) delete teleportorders;
	teleportorders = 0;
}

void Unit::DefaultOrders(Object *obj, int peasantfac)
{
	ClearOrders();
	if (type == U_WMON) {
		if (ObjectDefs[obj->type].monster == -1) {
		    guard = GUARD_AVOID; //resetting, gets lost (when advancing?)
		    reveal = REVEAL_NONE;  //Arcadia mod only, to fix earlier reveal faction mistake.
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
			int tries = 0;            //BS mod to make monsters move sensible directions.
			int direc = -1;
#define MAXTRIES 10
			while(tries < MAXTRIES) {
			    direc = getrandom(NDIRS);
			    if(obj->region->neighbors[direc]) {
			        if( CanSwim() ) tries = MAXTRIES;
			        else if( (TerrainDefs[obj->region->neighbors[direc]->type].similar_type != R_OCEAN) &&
                             (obj->region->neighbors[direc]->willsink == 0 || 
                             (obj->region->willsink != 0 && obj->region->neighbors[direc]->willsink >= obj->region->willsink)) ) tries = MAXTRIES;
                    else direc = -1;
			    }
			    tries++;
			}
			if(direc > -1) {
    			d->dir = direc;
    			o->dirs.Add(d);
    			monthorders = o;
			} else {
			    //no move orders
			    delete d;
			    delete o;
			}
		}
	} else if(type == U_GUARD) {
		if (guard != GUARD_GUARD)
			guard = GUARD_SET;
	} else if(type == U_GUARDMAGE) {
//		combat = S_FIRE;  combat skill stored and set.
//wandering peasants:   // maybe have some chance of them settling in the region ...
    } else if(faction->num == peasantfac) {
        reveal = REVEAL_FACTION;
        if(type == U_NORMAL && getrandom(2)) {
			ProduceOrder *order = new ProduceOrder;
			order->skill = -1;
			order->item = I_SILVER;
			monthorders = order;
			return;
        }
		MoveOrder *o = new MoveOrder;
		o->advancing = 0;
		MoveDir *d = new MoveDir;
		int tries = 0;
		int direc = -1;
		while(tries < MAXTRIES) {
		    direc = getrandom(NDIRS);
		    if(obj->region->neighbors[direc]) {
		        if( CanMoveTo(obj->region, obj->region->neighbors[direc]) ) tries = MAXTRIES;
		    }
		    tries++;
		}
		d->dir = direc;
		o->dirs.Add(d);
		monthorders = o;
	} else {
		/* Set up default orders for factions which submit none */
		if(obj->region->type != R_NEXUS) {
			if(GetFlag(FLAG_AUTOTAX) &&
					Globals->TAX_PILLAGE_MONTH_LONG && Taxers(1)) {
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
			if(!(ItemDefs[i->type].type & IT_MONSTER) && !getrandom(3)) { //Nylandor patch, 67% chance of monsters keeping spoils per turn.
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

void Unit::SetLabel(AString *s)
{
	if (label) delete label;
	if (s) {
		AString *newname = s->getlegal();
		delete s;
		label = newname;
	} else
		label = 0;
}

int Unit::IsAlive()
{
//note if a mage is dead under Arcadia, it can be returned here as still alive - this is to prevent them being discarded midturn
//or trashed in the :kill() routine.
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

int Unit::IsReallyAlive()
{
    if(dead) return 0;
    else return IsAlive();
}

//This returns zero unless the unit is DEFINITELY stationary for the rest of the turn.
//This does not yet take into account enter orders, but should be generalised to
//do so if it is ever called before enter orders take place
int Unit::IsStationary()
{
    if(monthorders && (monthorders->type == O_MOVE || 
              monthorders->type == O_ADVANCE ||
              monthorders->type == O_FOLLOW ||
              monthorders->type == O_SAIL)) return 0;
    if(object->IsBoat()) {
        forlist(&object->units) {
            Unit *u = (Unit *) elem;
            if(u->monthorders && u->monthorders->type == O_SAIL) return 0;
        }
    }
    return 1;
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
/*
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
*/
int Unit::GetSoldiers()
{
	int n = 0;
	forlist(&items) {
		Item *i = (Item *) elem;
		if (IsSoldier(i->type)) n+=i->num;
	}

	return n;
}

int Unit::GetRealSoldiers()
{
	int n = 0;
	forlist(&items) {
		Item *i = (Item *) elem;
		if (IsSoldier(i->type) && !(ItemDefs[i->type].type & IT_ILLUSION) ) n += i->num;
	}

	return n;
}

int Unit::GetAttackRiding()
{
	int riding = 0;
	if (type == U_WMON) {
		forlist(&items) {
			Item *i = (Item *) elem;
			if (ItemDefs[i->type].type & IT_MONSTER) {
				if (ItemDefs[i->type].fly) {
                    if(!Globals->REAL_EXPERIENCE) return 5;
                    else return 6;
				}
				if (ItemDefs[i->type].ride) riding = 3;
			}
		}
		return riding;
	} else {
		riding = GetSkill(S_RIDING);
		int bonusriding = GetSkill(S_SWIFTNESS);
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
				return (riding+bonusriding);
			if (ItemDefs[i->type].ride-ItemDefs[i->type].weight >= minweight) {
				if (riding <= 3 && bonusriding <= 3) return (riding+bonusriding);
				lowriding = 3;
			}
		}
		if(bonusriding > lowriding) bonusriding = lowriding;
		return (lowriding+bonusriding);
	}
}

int Unit::GetDefenseRiding()
{
	if (guard == GUARD_GUARD) return 0;

	int riding = 0;
	int weight = Weight();

	if (CanFly(weight)) {
        if(!Globals->REAL_EXPERIENCE) riding = 5;
        else riding = 6;
    }
	else if (CanRide(weight)) riding = 3;

	if (GetMen()) {
		int manriding = GetSkill(S_RIDING);
		int bonusriding = GetSkill(S_SWIFTNESS);
		if (riding < bonusriding) bonusriding = riding;
		if (manriding < riding) return (manriding+bonusriding);
		else return (riding+bonusriding);
	} else if (type == U_WMON && riding) riding = riding/2 + 1; //make monsters easier to catch. 6->4, 3->2

	return riding;
}

int Unit::GetSkill(int sk)
{
	if (sk == S_TACTICS) return GetAttribute("tactics");
	if (sk == S_STEALTH) return GetAttribute("stealth");
	if (sk == S_OBSERVATION) return GetAttribute("observation");
	if (sk == S_ENTERTAINMENT) return GetAttribute("entertainment");
    if(skills.IsDisabled(sk)) return 0;    //either don't have the skill, or it's been disabled.
	int retval = GetRealSkill(sk);
	return retval;
}

void Unit::SetSkill(int sk, int level)
{
	skills.SetDays(sk, GetDaysByLevel(level) * GetMen());
}

void Unit::SetSkill(int sk, int level, int experlevel)
{
	if(Globals->REAL_EXPERIENCE) skills.SetDays(sk, GetDaysByLevel(level) * GetMen(), GetDaysByLevel(experlevel) * GetMen());
	else skills.SetDays(sk, GetDaysByLevel(level+experlevel) * GetMen());
}

int Unit::GetRealSkill(int sk)
{
	if (GetMen()) {
		return GetLevelByDays(skills.GetDays(sk)/GetMen(),skills.GetExper(sk)/GetMen());
	} else {
		return 0;
	}
}

int Unit::GetDaysSkill(int sk)
{
	if (GetMen()) {
		return GetLevelByDays(skills.GetDays(sk)/GetMen());
	} else {
		return 0;
	}
}

int Unit::GetExperSkill(int sk)
{
	if (GetMen()) {
		return GetLevelByDays(skills.GetExper(sk)/GetMen());
	} else {
		return 0;
	}
}


void Unit::ForgetSkill(int sk)
{
	skills.SetDays(sk, 0, 0); //REAL_EXPERIENCE Patch
		
    if(combat == sk) {
		combat = -1;
		Event("Combat spell set to none.");
	}
	
	if(!Globals->ARCADIA_MAGIC) {
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
	} else {
	    if (sk == S_HEROSHIP && type == U_MAGE) {
            type = U_LEADER;
            AdjustSkills(0);
        } else if(sk == S_LEADERSHIP && type == U_LEADER) {
            type = U_NORMAL;
            AdjustSkills(0);
        }
	}
}

int Unit::CheckDepend(int lev, SkillDepend &dep)
{
	AString skname = dep.skill;
	int sk = LookupSkill(&skname);
	if (sk == -1) return 0;
	int temp = GetRealSkill(sk);
	if(temp < dep.level) return 0;
	if(lev >= temp) return 0;
	return 1;
}

int Unit::CanStudy(int sk)
{
	int curlev = GetDaysSkill(sk);
	if(curlev >= GetSkillKnowledgeMax(sk) && GetExperSkill(sk) >= GetSkillExperMax(sk) ) return 0;  //for REAL_EXPERIENCE only (should ifglobal it), this means that if reached max exper level, cannot keep studying.

	if(SkillDefs[sk].flags & SkillType::DISABLED) return 0;

	unsigned int c;
	for(c = 0; c < sizeof(SkillDefs[sk].depends)/sizeof(SkillDepend); c++) {
		if(SkillDefs[sk].depends[c].skill == NULL) return 1;
		SkillType *pS = FindSkill(SkillDefs[sk].depends[c].skill);
		if (pS && (pS->flags & SkillType::DISABLED)) continue;
		if(!CheckDepend(curlev, SkillDefs[sk].depends[c])) return 0;
	}
	return 1;
}

int Unit::Study(int sk, int days, int quiet, int overflow)
{
    //Upgrading gets handled differently to other skills.
	if(SkillDefs[sk].flags & SkillType::UPGRADE) {
	    switch(sk) {
	        case S_LEADERSHIP:
	            if(type != U_NORMAL) {
	                Error("STUDY: Only normal units can study leadership", quiet);
	                return 0;
	            } else type = U_LEADER;
	            return 1;
            case S_HEROSHIP:
                if(type != U_LEADER || GetMen() != 1) {
	                Error("STUDY: Only one man leader units can study heroship", quiet);
	                return 0;
	            }
                type = U_MAGE;
	            return 1;
            default:
                break;
	    }
	}

	Skill *s;

	if(Globals->SKILL_LIMIT_NONLEADERS && IsNormal()) {
		if (skills.Num()) {
			s = (Skill *) skills.First();
			if (s->type != sk) {
			    //Real experience patch
			    if(s->days >= 30*GetMen() || s->experience >= 30*GetMen()) {
    				Error("STUDY: Can know only 1 skill.", quiet);
    				return 0;
				}
			}
		}
	}
	int max = GetSkillKnowledgeMax(sk);
	if (GetDaysSkill(sk) >= max) {
	    if(!Globals->REAL_EXPERIENCE) {
    		Error("STUDY: Maximum level for skill reached.", quiet);
    		return 0;
   		} else if (!overflow) {
   		    Error("STUDY: Maximum knowledge level for skill reached, teaching has no effect.", quiet);
   		    return 0;
		} else if (GetExperSkill(sk) >= GetSkillExperMax(sk) ) {
    		Error("STUDY: Maximum level for skill reached.", quiet);
    		return 0;
		}
	}

	if (!CanStudy(sk) && overflow) {   //overflow == 0 is only called for teaching, when to stick to old behaviour we don't want this checked. For Arcadia IV, review this.
		Error("STUDY: Doesn't have the pre-requisite skills to study that.", quiet);
		return 0;
	}

	skills.SetDays(sk, skills.GetDays(sk) + days);
	AdjustSkills(overflow);

	/* Check to see if we need to show a skill report */
	int lvl = GetRealSkill(sk);
	if (lvl > faction->skills.GetDays(sk)) {
		faction->skills.SetDays(sk, lvl);
		faction->shows.Add(new ShowSkill(sk, lvl));
	}
	return 1;
}

void Unit::Experience(int sk, int experience, int group, int dividenonspecials) //group =1, dividenonspecials =1, by default.
{
    if(!Globals->REAL_EXPERIENCE) return;
    if(experience < 1) return;
    
	Skill *s;

	if(Globals->SKILL_LIMIT_NONLEADERS && IsNormal()) {
	//if can know only one skill, and first skill is not the one to experience, do not get experience.
		if (skills.Num()) {
			s = (Skill *) skills.First();
			if (s->type != sk) {
				return;
			}
		}
	}
	
	if(GetSkillExperMax(sk) == 0) return;
	    
	if(dividenonspecials) {
	//if non-specialist skill, divide experience gain by 2 (from 10 to 5 in most cases).
	    if(!IsASpeciality(sk)) experience /= 2;
	}

    if(experience < 1) return;

	if(SkillDefs[sk].baseskill != -1 && SkillDefs[sk].baseskill != sk) {
	//check to make sure some silly GM hasn't given a baseskill a different baseskill (cascading experiences and possibly infernal loops)
	    if(SkillDefs[SkillDefs[sk].baseskill].baseskill == -1 || 
           SkillDefs[SkillDefs[sk].baseskill].baseskill == SkillDefs[sk].baseskill) {
            int baseexp = (experience)/2;
            if(baseexp>4) baseexp = 4;
            Experience(SkillDefs[sk].baseskill, baseexp, group, 0); //don't divide non-specials again!
        }
	}
	
	if(group) experience *= GetMen();
	
	skills.SetExper(sk, skills.GetExper(sk) + experience);
	AdjustSkills(1);

	int lvl = GetRealSkill(sk);
	if (lvl > faction->skills.GetDays(sk)) {
		faction->skills.SetDays(sk, lvl);
		faction->shows.Add(new ShowSkill(sk, lvl));
	}

}

int Unit::IsASpeciality(int sk)
{
	if (SkillDefs[sk].flags & SkillType::DISABLED) return 0;

	forlist (&items) {
		Item *i = (Item *)elem;
		if (ItemDefs[i->type].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[i->type].type & IT_MAN)) continue;
		if(!(IsSpeciality(SkillDefs[sk].abbr, i->type)) && (SkillDefs[sk].baseskill < 0 || !(IsSpeciality(SkillDefs[SkillDefs[sk].baseskill].abbr, i->type)))) return 0;
	}
	return 1;	
}

int Unit::GetSkillKnowledgeMax(int sk)
/* REAL_EXPERIENCE Patch */
//returns maximum knowledge level of a skill (not number of days study!)
{
    if((SkillDefs[sk].flags & SkillType::MAGIC) && !IsMage()) return 0;

	int max = -2;

	if (SkillDefs[sk].flags & SkillType::DISABLED) return 0;

	forlist (&items) {
		Item *i = (Item *)elem;
		if (ItemDefs[i->type].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[i->type].type & IT_MAN)) continue;
		int m = SkillMax(SkillDefs[sk].abbr, i->type);
		if ((max == -2 && m > max) || (m < max)) max = m;    //need this to return 0 or 1 for hero skills, as this gets added up to 2 or 3 below.
	}
	
	if(max < 0) max = 0;
	
	switch(type) {
	    case U_NORMAL:
	    case U_GUARD:
	        return max;           //2 or 1
        case U_SPECIALIST:
        case U_LEADER:
        case U_APPRENTICE:
            return (max+1);         //3 or 2
        case U_MAGE:
        case U_GUARDMAGE:
            return (max+2);            //4 or 3
        default:
            return max;
	}
}

int Unit::GetSkillExperMax(int sk)
/* REAL_EXPERIENCE Patch */
//returns maximum experience level of a skill
{
    int max = GetSkillKnowledgeMax(sk);    //set experience and knowledge maxes equal.
    //add in level of appropriate glamour spell - doesn't exist yet
    return max;
/*
	int max = -1;

	if (SkillDefs[sk].flags & SkillType::DISABLED) return 0;

	forlist (&items) {
		Item *i = (Item *)elem;
		if (ItemDefs[i->type].flags & ItemType::DISABLED) continue;
		if (!(ItemDefs[i->type].type & IT_MAN)) continue;
		int m = SkillExperMax(SkillDefs[sk].abbr, i->type);
		if ((max == -1 && m > max) || (m < max)) max = m;
	}
	
	if(max < 0) max = 0;

	switch(type) {
	    case U_NORMAL:
	        return max;
        case U_SPECIALIST:
        case U_LEADER:
            return (max+1);
        case U_MAGE:
            return (max+2);
        default:
            return max;
	}
	*/
}

int Unit::Practice(int sk)
{
    if(Globals->REAL_EXPERIENCE) return 1;
    
	int bonus, men, curlev, reqsk, reqlev, days;
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
	int max = GetSkillKnowledgeMax(sk);
	curlev = GetDaysSkill(sk);
	if (curlev >= max) return 0;

	for(i = 0; i < sizeof(SkillDefs[sk].depends)/sizeof(SkillDepend); i++) {
		AString skname = SkillDefs[sk].depends[i].skill;
		reqsk = LookupSkill(&skname);
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
		Study(sk, men * bonus, 1);
		practiced = 1;
	}

	return bonus;
}

int Unit::IsMage()
{
    if(type == U_MAGE || type == U_GUARDMAGE) return 1;
	return 0;
}

int Unit::IsLeader()
{
//	if (GetLeaders()) return 1;
    if(type == U_LEADER) return 1;
	return 0;
}

int Unit::IsNormal()
{
	if (GetMen() && !IsLeader() && !IsMage()) return 1;
	return 0;
}

void Unit::AdjustSkills(int overflow)
//overflow allows for movement of skills between experience and knowledge
{
    if(!Globals->REAL_EXPERIENCE) overflow = 0;
    
	// First, is the unit a leader?
	//
	if(!IsNormal()) {   //ie is a mage or leader
		//
		// Unit is all leaders: Make sure no skills are > max
		//
		forlist(&skills) {
			Skill *theskill = (Skill *) elem;
			int max = GetSkillKnowledgeMax(theskill->type);
			int expermax = GetSkillExperMax(theskill->type);
			if (GetDaysSkill(theskill->type) >= max) {
			    if(overflow) {
			        int extra = theskill->days - GetMen() * GetDaysByLevel(max);
			        if(extra<0) extra = 0; //should never get called
			        if (SkillDefs[theskill->type].flags & SkillType::MAGIC) {
			            if(IsASpeciality(SkillDefs[theskill->type].baseskill)) theskill->experience += extra / 3;
			            else theskill->experience += extra / 6; //halved for non-specials.
			        } else theskill->experience += extra / 3; //full roll over whether speciality or not?
			    }
				theskill->days = GetDaysByLevel(max) * GetMen();
			}
			if (GetExperSkill(theskill->type) >= expermax) {
			    if(overflow) {
			        int extra = theskill->experience - GetMen() * GetDaysByLevel(expermax);
			        if(extra<0) extra = 0; //should never get called
			        if (CanStudy(theskill->type)) {
			            theskill->days += extra / 3;
                    }
			    }
				theskill->experience = GetDaysByLevel(expermax) * GetMen();
			}
			if (overflow && GetDaysSkill(theskill->type) >= max) {
				theskill->days = GetDaysByLevel(max) * GetMen();
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
				int maxlevel = 0;
				//highest skill is defined by days+experience, with the exception that a level 1 skill is
				//always counted as higher than a level 0 skill.
				
				Skill *maxskill = 0;
				forlist(&skills) {
					Skill *s = (Skill *) elem;
					if (s->days + s->experience > max) {
					    if(s->days >= 30 || s->experience >= 30 || maxlevel == 0) 
						max = s->days + s->experience;
						if(s->days >= 30 || s->experience >= 30) maxlevel = 1;
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
			int max = GetSkillKnowledgeMax(theskill->type);
			int expermax = GetSkillExperMax(theskill->type);
			if (GetDaysSkill(theskill->type) >= max) {
			    if(overflow) {
			        int extra = theskill->days - GetMen() * GetDaysByLevel(max);
			        if(extra<0) extra = 0; //should never get called
			        
			        if(IsASpeciality(theskill->type)) theskill->experience += extra / 3;
			        else theskill->experience += extra / 6;
			    }
				theskill->days = GetDaysByLevel(max) * GetMen();
			}
			if (GetExperSkill(theskill->type) >= expermax) {
			    if(overflow) {
			        int extra = theskill->experience - GetMen() * GetDaysByLevel(expermax);
			        if(extra<0) extra = 0; //should never get called
			        theskill->days += extra / 3;
			    }
				theskill->experience = GetDaysByLevel(expermax) * GetMen();
			}
			if (overflow && GetDaysSkill(theskill->type) >= max) {
				theskill->days = GetDaysByLevel(max) * GetMen();
			}
		}
	}
}

int Unit::MaintCost()
{
//	int retval = 0;
	int i;
	if (type == U_WMON || type == U_GUARD || type == U_GUARDMAGE) return 0;

	int men = GetMen();
	int okethnic = 0;
	
	if(Globals->ARCADIA_MAGIC) {	    
	    int type = GetEthnicity();
	    if(type == RA_MIXED) {
	        men = 0;
	        forlist(&items) {
        		Item *i = (Item *) elem;
        		if (ItemDefs[i->type].type & IT_MAN) {
        		    ManType *mt = FindRace(ItemDefs[i->type].abr);
        			if(mt->ethnicity == faction->ethnicity) men += i->num;
        			else men += 2*i->num;                //double maintenance cost for non-ethnic men.
        		}
        	}
	    } else if(type != faction->ethnicity) men *= 2;  //double maintenance cost for non-ethnic men.
	    else okethnic = 1;
	}
	
	if(type == U_LEADER || type == U_MAGE) {
        if(Globals->ARCADIA_MAGIC && type == U_MAGE) {
            men *= 5; //$100 or $200
        }
    	// Handle leaders
    	// Leaders are counted at maintenance_multiplier * skills in all except
    	// the case where it's not being used (mages, leaders, all)
    	if (Globals->MULTIPLIER_USE != GameDefs::MULT_NONE) {
    		i = men * SkillLevels() * Globals->MAINTENANCE_MULTIPLIER;
    		if (i < (men * Globals->LEADER_COST))
    			i = men * Globals->LEADER_COST;
    	} else
    		i = men * Globals->LEADER_COST;
    	return i;
	
	} else {

    	// Handle non-leaders
    	// Non leaders are counted at maintenance_multiplier * skills only if
    	// all characters pay that way.
    	if (Globals->MULTIPLIER_USE == GameDefs::MULT_ALL) {
    		i = men * SkillLevels() * Globals->MAINTENANCE_MULTIPLIER;
    		if (i < (men * Globals->MAINTENANCE_COST))
    			i = men * Globals->MAINTENANCE_COST;
    	} else
    		i = men * Globals->MAINTENANCE_COST;
    	return i;
	
	}

	return 0;
}

void Unit::Short(int needed, int hunger)
{
	int i, n = 0, levels;

	if (faction->IsNPC())
		return; // Don't starve monsters and the city guard!

	if (needed < 1 && hunger < 1) return;

	switch(Globals->SKILL_STARVATION) {
		case GameDefs::STARVE_MAGES:
			if(type == U_MAGE) SkillStarvation();
			return;
		case GameDefs::STARVE_LEADERS:
			if(IsLeader()) SkillStarvation();
			return;
		case GameDefs::STARVE_ALL:
			SkillStarvation();
			return;
	}

	for (i = 0; i<= NITEMS; i++) {
		if(!(ItemDefs[ i ].type & IT_MAN)) {
			// Only men need sustenance.
			continue;
		}

		if(ItemDefs[i].type & IT_LEADER) {
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
				if (n) Error(AString(n) + " starve to death.", 0);
				return;
			}
		}
	}

	// Now starve leaders
	for (int i = 0; i<= NITEMS; i++) {
		if(!(ItemDefs[ i ].type & IT_MAN)) {
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
				if (n) Error(AString(n) + " starve to death.", 0);
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
	if (FlyingCapacity() > 0 && FlyingCapacity() >= weight) return 1;     //empty units don't fly
	return 0;
}

int Unit::CanReallySwim()
{
	if (SwimmingCapacity() > 0 && SwimmingCapacity() >= items.Weight()) return 1;
	if (dead) return 1; //ARCADIA_MAGIC Patch (ghosts can remain after regions sink!)
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

int Unit::TryToSwim()
//returns 1 if can swim, return 0 otherwise
{
    //In progress!

	switch(Globals->FLIGHT_OVER_WATER) {
		case GameDefs::WFLIGHT_UNLIMITED:
			if(CanSwim()) return 1;
		    //this is old behaviour, someone else can code new!
			break;
			
		case GameDefs::WFLIGHT_MUST_LAND:
            if(type == U_WMON && CanSwim()) return 1; //monsters that fly don't drown
			if(leftShip) {
			    leftShip = 0;  //no idea what this does
                return 1;
            }
            //fall thru!
		case GameDefs::WFLIGHT_NONE:
			if(CanReallySwim()) return 1;
			//we can't swim yet ...
			if(SwimmingCapacity()) {
			    //but maybe we will be able to!
			    int tries = items.Num();
			    while(tries-- > 0) {
				    int maxweight = 0;
				    int type = -1;
				    //pick the heaviest item to discard first
				    forlist(&items) {
		                Item *i = (Item *) elem;
		                if((ItemDefs[i->type].weight - ItemDefs[i->type].swim) > maxweight) {
                            maxweight = ItemDefs[i->type].weight - ItemDefs[i->type].swim;
                            type = i->type;
                        }
	                }
	                if(type < 0) break;
				    int tolose = (items.Weight() - SwimmingCapacity() + maxweight - 1)/maxweight; //this is the number of our heaviest item we need to discard (need to round up).
				    //careful ... what if this is a man?
				    //Warning: if there are no swimming men in the unit this code might leave items but no men. Perhaps should discard all items before any men ... hmm.
				    if((ItemDefs[type].type & IT_MAN)) {
                        if(items.GetNum(type) <= tolose) {
                            tolose = items.GetNum(type);
                            SetMen(type,0);
                        } else SetMen(type, items.GetNum(type) - tolose);
                        Event(AString("Disbands ") + ItemString(type, tolose) + ".");
                    } else {
                        if(items.GetNum(type) <= tolose) {
                            tolose = items.GetNum(type);
                            items.SetNum(type,0);
                        } else items.SetNum(type, items.GetNum(type) - tolose);
                        Event(AString("Discards ") + ItemString(type, tolose) + ".");
                    }

				    if(CanReallySwim()) return 1;
                }
            }
            if(CanReallySwim()) return 1; //this shouldn't be needed; just being overly thorough
			break;
		default: // Should never happen
			break;
	}
	return 0;
}

int Unit::CanDolphinRide()
{
    //this is a specific alternative to introducing a fifth movement class 'riding-at-sea'
	int cap = 0;
	cap = ItemDefs[I_DOLPHIN].swim * items.GetNum(I_DOLPHIN);
	if(cap >= items.Weight()) return 1;
	
	return 0;
}

int Unit::MoveType(ARegion *regionto)
{
	/* Check if we should be able to 'swim' */
	/* This should become it's own M_TYPE sometime */
	//we can swim if we are going to ocean from anywhere, or if we are going to land FROM ocean.
	//likewise, if we are going to OR from ocean, and cannot swim, we cannot move even if we can walk and are going to land.
	if((regionto && TerrainDefs[regionto->type].similar_type == R_OCEAN) || 
        TerrainDefs[object->region->type].similar_type == R_OCEAN) {
        
        if((Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) && this->CanFly()) return M_FLY;
        
		if(CanReallySwim()) {
		    if(CanDolphinRide()) return M_RIDE;
            return M_WALK;
        }
		else return M_NONE;
	}
		
	int weight = items.Weight();
	if (CanFly(weight)) return M_FLY;
	if (CanRide(weight)) return M_RIDE;
	if (CanWalk(weight)) return M_WALK;
	return M_NONE;
}

int Unit::CalcMovePoints(ARegion *regionto)
{
	switch (MoveType(regionto)) {
		case M_NONE:
			return 0;
		case M_WALK:
			return Globals->FOOT_SPEED;
		case M_RIDE:
			return Globals->HORSE_SPEED;
		case M_FLY:
			return Globals->FLY_SPEED + GetAttribute("flying");
	}
	return 0;
}

int Unit::CanMoveTo(ARegion *r1, ARegion *r2)
{
//moving to r1 from r2

	if (r1 == r2) {
	//hexside terrain mod. If the unit has advanced here, then subtract any modifiers due to edge terrain.
        if(Globals->HEXSIDE_TERRAIN && advancefrom) {
            int dir = -1;
            for(int i=0; i<NDIRS; i++) {
                if(advancefrom->neighbors[i] == r1) dir = i;
            }
            if(dir == -1) return 1;
            Hexside *h = advancefrom->hexside[dir];
            crossbridge += HexsideDefs[h->type].advancepen;
            if(h->road < 0) crossbridge += HexsideDefs[H_ROAD].advancepen;
    		if(h->bridge < 0) crossbridge += HexsideDefs[H_BRIDGE].advancepen;   //this is going to suffer from the same problems described in atlantisdev post 6274, relating to movements with more than one advance command issued.
        }
        return 1;
    }

	int exit = 1;
	int i;
	int dir;
//we seem to be checking the connection was 2-way ... but shouldn't we be subtracting the penalty even if one way?
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
			if(!CanFly()) {
    			Hexside *h = r2->hexside[i];
    			crossbridge += HexsideDefs[h->type].advancepen;
                if(h->road < 0) crossbridge += HexsideDefs[H_ROAD].advancepen;
    			if(h->bridge < 0) crossbridge += HexsideDefs[H_BRIDGE].advancepen;
			}
			break;
		}
	}
	if (exit) return 0;

	int mt = MoveType(r2);
	if (((TerrainDefs[r1->type].similar_type == R_OCEAN) ||
				(TerrainDefs[r2->type].similar_type == R_OCEAN)) &&
			(!CanSwim() || GetFlag(FLAG_NOCROSS_WATER)))
		return 0;
	int mp = CalcMovePoints(r1) - movepoints;
	int mc = r2->MoveCost(mt, r1, dir, 0);
	if (mp < mc || mc < 0) return 0;   //mc less than 0 means blocked from moving there.
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
	if (!IsReallyAlive()) return 0;
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
	
	//added by BS
	int spareMelee = 0;

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
			int numUse = 0;
			int basesk = 0;
			AString skname = pWep->baseSkill;
			int sk = LookupSkill(&skname);
			if (sk != -1) basesk = GetSkill(sk);
			if (basesk == 0) {
				skname = pWep->orSkill;
				sk = LookupSkill(&skname);
				if(sk != -1) basesk = GetSkill(sk);
			}
			/*
			//use this bit if weapons that don't need a skill for combat shouldn't need one for taxing
			if (!(pWep->flags & WeaponType::NEEDSKILL)) {
				numUsableMelee += numUse;
				numMelee += num;
			}			
			*/
			//added by BS
			if (!(pWep->flags & WeaponType::NEEDSKILL)) {
				spareMelee += num;   //for BS mod these, with a horse and riding, allow taxation.
			}	
			
			if (basesk) {
				numUse = num;
				if (!(pWep->flags & WeaponType::NEEDSKILL)) {
					numUsableMelee += numUse;
					numMelee += num;
					spareMelee -= num;     //to avoid double counting; if here, these weapons are usable with the units skills already
				} else if (pWep->flags & WeaponType::NOFOOT) {
					numUsableMounted += numUse;
					numMounted += num;
				} else {
					// Presume that anything else is a bow!
					numUsableBows += pItem->num;
					numBows += pItem->num;
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
		basetax = totalMen;        //ie in this case, everyone in the unit can tax. Not sure the bonus are right in complex situations though! -BS
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
		 
		if(((Globals->WHO_CAN_TAX & GameDefs::TAX_BOW_SKILL) &&
		 (GetSkill(S_CROSSBOW) || GetSkill(S_LONGBOW)))) {
		 	weapontax += numUsableBows;
		 }
		if((Globals->WHO_CAN_TAX & GameDefs::TAX_RIDING_SKILL) &&
		 GetSkill(S_RIDING)) {
		 	if(weapontax < numUsableMounts) weapontax = numUsableMounts;
		 }
		
	} else {

		if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_WEAPON) {
			if (numUsableMounted > numUsableMounts) {     //numusablemounted is a weapon type
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
			weapontax += numMelee + numUsableBows;      //shouldn't this be numUsableMelee? Not that it matters since in the current incarnation numMelee = numUsableMelee, but still ...
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
			weapontax += numMounts;
			taxers += numUsableMounts;
		}
		//BS mod
		else if (Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE_AND_RIDING_SKILL_AND_MELEE_WEAPON) {
		    //don't understand how the numMounts / numUsableMounts system works (or doesn't), so just doing this:
		    if(numUsableMounts > spareMelee) {
		        weapontax += spareMelee;
		        taxers += spareMelee;
		    } else {
		        weapontax += numUsableMounts;
		        taxers += numUsableMounts;
		    }
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
	if(weapontax > totalMen) weapontax = totalMen;
	if(armortax > weapontax) armortax = weapontax;

	//added by BS
	if(basetax > totalMen) basetax = totalMen;
	
	// Adjust basetax in case of weapon taxation
	if(basetax < weapontax) basetax = weapontax;

	// Now check for an overabundance of tax enabling objects
	if (taxers > totalMen) taxers = totalMen;

	// And finally for creatures
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_CREATURES)
		basetax += creatures;
		taxers += creatures;
	if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS)
		basetax += illusions;
		taxers += illusions;
		

	if(numtaxers) return(taxers);

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
//This is currently used only for FORM orders. May have to modify it to use for anything else. -BS
{
	flags = x->flags;
	if(x->guard == GUARD_AVOID) guard = GUARD_AVOID; //Arcadia mod; guard and autotax do not get carried over.
	else guard = GUARD_NONE;
	SetFlag(FLAG_AUTOTAX, 0);
	//BS mod:
	SetFlag(FLAG_VISIB, 0);
	SetFlag(FLAG_INVIS, 0);
	SetFlag(FLAG_COMMANDER, 0);
/*	if (x->Taxers(1)) {
		if (x->guard != GUARD_SET && x->guard != GUARD_ADVANCE)
			guard = x->guard;
	} else {
		SetFlag(FLAG_AUTOTAX, 0);
	}*/
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
	//if assassination, and armour is not useinassassinate, and unit is attacker or armour is no definassassinate  (BS mod)
	if (ass && !(pa->flags & ArmorType::USEINASSASSINATE) && (ass == 2 || !(pa->flags & ArmorType::DEFINASSASSINATE) ) ) return -1;

	int num = items.GetNum(item);
	if (num < 1) return -1;

	if (!(ItemDefs[item].type & IT_ARMOR)) return -1;
	items.SetNum(item, num - 1);
	return item;
}

int Unit::GetMount(AString &itm, int canFly, int canRide, int ocean, int &bonus, int &type)
{

	bonus = 0;

	int item = LookupItem(&itm);
	MountType *pMnt = FindMount(itm.Str());
	
	if(ocean) {
	    //if this mount can swim in ocean, we can ride here
	    if(ItemDefs[item].swim) canRide = 1;
	    //otherwise; normal behaviour (ie no riding in Xan)
	} else {
	    //if we can't ride, we can't ride.
	    if(!ItemDefs[item].ride) canRide = 0;
	    //otherwise, normal behaviour
	}

	// This region doesn't allow riding or flying, so no mounts, bail
	if(!canFly && !canRide) return -1;

	//we can either fly or swim
	int num = items.GetNum(item);
	if (num < 1) return -1;

	if (canFly) {
		// If the mount cannot fly, and the region doesn't allow
		// riding mounts, bail
		if (!ItemDefs[item].fly && !canRide) return -1;
	} /*else {
		// This region allows riding mounts, so if the mount
		// can not carry at a riding level, bail
		if (!ItemDefs[item].ride && !ItemDefs[item].swim) return -1;
	}*/

	if (pMnt->skill) {
		AString skname = pMnt->skill;
		int sk = LookupSkill(&skname);
		bonus = GetSkill(sk);
		if(bonus < pMnt->minBonus) {
			// Unit isn't skilled enough for this mount
			bonus = 0;
			return -1;
		}
		// Limit to max mount bonus;
		if(bonus > pMnt->maxBonus) bonus = pMnt->maxBonus;
		
		if(ItemDefs[item].fly) type = 4;
        else type = 2;
        
		// If the mount can fly and the terrain doesn't allow
		// flying mounts, limit the bonus to the maximum hampered
		// bonus allowed by the mount		
		if(ItemDefs[item].fly && !canFly) {
			if(bonus > pMnt->maxHamperedBonus)
				bonus = pMnt->maxHamperedBonus;
				type = 2;
		}

		// Practice the mount's skill
		Practice(sk);
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

	// Found a weapon, check flags and skills
	int baseSkillLevel = CanUseWeapon(pWep, riding);
	// returns -1 if weapon cannot be used, else the usable skill level
	if(baseSkillLevel == -1) return -1;

	// Attack and defense skill
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

void Unit::Error(const AString & s, int quiet)
{
    if(quiet) {
        if(Globals->SUPPRESS_ERRORS == GameDefs::SHOW_AS_EVENTS) {
            Event(s);
            return;
        } else if(Globals->SUPPRESS_ERRORS == GameDefs::SUPPRESS_ALL) return;
    }
	AString temp = *name + ": " + s;
	faction->Error(temp);
}

void Unit::Message(const AString & s)
{
	faction->Message(s);
}

int Unit::GetAttribute(char *attrib)
{
	AttribModType *ap = FindAttrib(attrib);
	if(ap == NULL) return 0;
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
	for(int index = 0; index < 6; index++) {  //BS mod 5 to 6 to expand stealth.
		int val = 0;
		if (ap->mods[index].flags & AttribModItem::SKILL) {
			temp = ap->mods[index].ident;
			int sk = LookupSkill(&temp);
			val = GetRealSkill(sk);
			if(val > 0) {
    			if (ap->mods[index].modtype == AttribModItem::UNIT_LEVEL_HALF) {
    				val = ((val + 1)/2) * ap->mods[index].val;
    			} else if (ap->mods[index].modtype == AttribModItem::CONSTANT) {
    				val = ap->mods[index].val;
    			} else {
    				val *= ap->mods[index].val;
    			}
			}
		} else if (ap->mods[index].flags & AttribModItem::ITEM) {
			val = 0;
			temp = ap->mods[index].ident;
			int item = LookupItem(&temp);
			if (item != -1) {
				if (ap->mods[index].flags & AttribModItem::PERMAN) {
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
			if (temp == "visib")
				val = (GetFlag(FLAG_VISIB) ? ap->mods[index].val : 0);
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
		if (ap->flags & AttribModType::USE_WORST)
			base = (monbase < base) ? monbase : base;
		else
			base = (monbase > base) ? monbase : base;
	}	
	return base;
}

int Unit::PracticeAttribute(char *attrib)
{
	AttribModType *ap = FindAttrib(attrib);
	if(ap == NULL) return 0;
	for(int index = 0; index < 5; index++) {
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
		levels += GetLevelByDays(s->days/GetMen(),s->experience/GetMen());
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
			if(!sj) continue; // prevents a crash if a unit doesn't know all the prerequisites.
			int dependancy_level = 0;
			unsigned int c;
			for(c=0;c < sizeof(SkillDefs[i].depends)/sizeof(SkillDepend);c++) {
				AString skname = SkillDefs[i].depends[c].skill;
				if (skname == SkillDefs[j].abbr) {
					dependancy_level = SkillDefs[i].depends[c].level;
					break;
				}
			}
			if(dependancy_level > 0) {
				if(GetLevelByDays(sj->days) == GetLevelByDays(si->days)) { //REAL_EXPERIENCE Patch: No change because go by study days only here. No starvation of experience?
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
		Error(temp, 0);
		return;
	}
	count = getrandom(count)+1;
	for(i = 0; i < NSKILLS; i++) {
		if(can_forget[i]) {
			if(--count == 0) {
				Skill *s = GetSkillObject(i);
				AString temp = AString("Starves and forgets one level of ")+
					SkillDefs[i].name + ".";
				Error(temp, 0);
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
		if(pWep->flags & WeaponType::NOFOOT) return -1;
	} else {
		if(pWep->flags & WeaponType::NOMOUNT) return -1;
	}
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

	if(tempSkillLevel > baseSkillLevel) {
		baseSkillLevel = tempSkillLevel;
		Practice(orsk);
	} else
		Practice(bsk);

	if(pWep->flags & WeaponType::NEEDSKILL && !baseSkillLevel) return -1;

	return baseSkillLevel;
}

int Unit::GetEnergy(int transferring)
{
//this should be used when checking a mages energy after the move phase but before the maintenance phase - eg during combat or teleportation.
    if(transferred == 0 && transferring == 0) return energy;
    transferring += transferred;
//for consistency, the cost here must match the cost in Unit::EnergyRecharge(). These could perhaps be combined into a new method.
    if(dead) return -1;
    int skill = GetSkill(S_CREATE_PORTAL);
    if(skill < 1) skill = 1;
    int cost = (transferring + 40 * skill - 1) / (40 * skill);
    
    return (energy - cost);
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
    if(item < 0 || item > NITEMS) return 0;
	int count = 0;

	if (ItemDefs[item].type & IT_MAN)
		return items.GetNum(item);

	forlist((&object->region->objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			if ((u == this) || 
			(u->faction == faction && u->GetFlag(FLAG_SHARING)))
				count += u->items.GetNum(item);
		}
	}

	return count;
}

int Unit::GetSharedNum(int item, int quantity)
{
    if(item < 0 || item > NITEMS) return 0;
    //speed optimiser:
    if(items.GetNum(item) >= quantity) return 1;
    //else
    if(GetSharedNum(item) >= quantity) return 1;
    return 0;
}

int Unit::ConsumeShared(int item, int n)
{
    if(item < 0 || item > NITEMS) return 0;
    if(!GetSharedNum(item,n)) return 0;
    
	if (items.GetNum(item) >= n) {
		// This unit doesn't need to use shared resources
		items.SetNum(item, items.GetNum(item) - n);
		return 1;
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
					u->Event(AString("Shares ") + ItemString(item, n) +
							" with " + *name + ".");
					return 1;
				}
				u->Event(AString("Shares ") +
						ItemString(item, u->items.GetNum(item)) +
						" with " + *name + ".");
				n -= u->items.GetNum(item);
				u->items.SetNum(item, 0);
			}
		}
	}
	return 1;
}

int Unit::GetSharedMoney()
{
	return GetSharedNum(I_SILVER);
}

int Unit::GetSharedMoney(int quantity)
{
	return GetSharedNum(I_SILVER);
}

int Unit::ConsumeSharedMoney(int n)
{
	return ConsumeShared(I_SILVER, n);
}

int Unit::GetEthnicity()
{
    int racetype = -1;

	forlist(&items) {
		Item *i = (Item *) elem;
		if (ItemDefs[i->type].type & IT_MAN) {
		    ManType *mt = FindRace(ItemDefs[i->type].abr);
			if(racetype < 0) racetype = mt->ethnicity;
			else if(racetype != mt->ethnicity) return RA_MIXED;
		}
	}
	if(racetype < 0) return RA_NA;
	
	return racetype;
}

int Unit::GetSeniority()
{
    //no functional effect, just decides who leads an army
    int score = 0;
    switch(type) {
        case U_MAGE:
        case U_GUARDMAGE:
            score += 30;
            break;
        case U_APPRENTICE:
            score += 20;
            break;
        case U_LEADER:
        case U_GUARD:
            score += 10;
            break;
        case U_SPECIALIST:
            score += 5;
            break;
        default:
            break;
    }
    if(flags & FLAG_COMMANDER) score += 40;
    score += GetSkill(S_COMBAT) + GetSkill(S_LONGBOW) + GetSkill(S_CROSSBOW) + 
             GetSkill(S_HEALING) + 2*GetSkill(S_BASE_BATTLETRAINING) + 
             2*GetSkill(S_TOUGHNESS) + 2*GetSkill(S_FRENZY) + 
             GetSkill(S_OBSERVATION) + GetSkill(S_STEALTH);
    if (combat != -1) score += 3*GetSkill(combat);
    score += getrandom(2+score/5);
    return score;
}

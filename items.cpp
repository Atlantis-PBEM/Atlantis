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
#include "items.h"
#include "skills.h"
#include "gamedata.h"

int ParseItem(AString * token)
{
	int r = -1;
	for (int i=0; i<NITEMS; i++) {
		if ((ItemDefs[i].type & IT_MONSTER) &&
				ItemDefs[i].index == MONSTER_ILLUSION) {
			if ((*token == (AString("i") + ItemDefs[i].name)) ||
				(*token == (AString("i") + ItemDefs[i].names)) ||
				(*token == (AString("i") + ItemDefs[i].abr))) {
				r = i;
				break;
			}
		} else {
			if ((*token == ItemDefs[i].name) ||
				(*token == ItemDefs[i].names) ||
				(*token == ItemDefs[i].abr)) {
				r = i;
				break;
			}
		}
	}
	if(r != -1) {
		if(ItemDefs[r].flags & ItemType::DISABLED) r = -1;
	}
	return r;
}

int ParseBattleItem(int item)
{
	for(int i = 0; i < NUMBATTLEITEMS; i++) {
		if(item == BattleItemDefs[i].itemNum) return i;
	}
	return -1;
}

AString ItemString(int type, int num)
{
	AString temp;
	if (num == 1) {
		temp += AString(ItemDefs[type].name) + " [" + ItemDefs[type].abr + "]";
	} else {
		if (num == -1) {
			temp += AString("unlimited ") + ItemDefs[type].names + " [" +
				ItemDefs[type].abr + "]";
		} else {
			temp += AString(num) + " " + ItemDefs[type].names + " [" +
				ItemDefs[type].abr + "]";
		}
	}
	return temp;
}

static AString ItemSpecial(int special, int level)
{
	AString temp = "Can cast ";
	if(special < 0 || special > (NUMSPECIALS-1)) special = 0;
	temp += SpecialDefs[special].specialname;
	temp += AString(" in battle at level ") + level + ".";
	// XXX -- Handle the effect data.
	return temp;
}

static AString MonResist(int type, int val, int full)
{
	AString temp = "This monster ";
	if(full) {
		temp += AString("has a resistance of ") + val;
	} else {
		temp += "is ";
		if(val < 1) temp += "very susceptible";
		else if(val == 1) temp += "susceptible";
		else if(val > 1 && val < 3) temp += "typically resistant";
		else if(val > 2 && val < 5) temp += "slightly resistant";
		else temp += "very resistant";
	}
	temp += " to ";
	switch(type) {
		case ATTACK_COMBAT: temp += "physical"; break;
		case ATTACK_ENERGY: temp += "energy"; break;
		case ATTACK_SPIRIT: temp += "spiritual"; break;
		case ATTACK_WEATHER: temp += "weather-based"; break;
		case ATTACK_RIDING: temp += "riding"; break;
		case ATTACK_RANGED: temp += "ranged"; break;
		default: temp += "unknown";
	}
	temp += " attacks.";
    return temp;
}

static AString WeapClass(int wclass) 
{
	switch(wclass) {
		case SLASHING: return AString("slashing");
		case PIERCING: return AString("piercing");
		case CRUSHING: return AString("crushing");
		case CLEAVING: return AString("cleaving");
		case ARMORPIERCING: return AString("armorpiercing");
		default: return AString("unknown");
	}
}

static AString AttType(int atype)
{
	switch(atype) {
		case ATTACK_COMBAT: return AString("melee");
		case ATTACK_ENERGY: return AString("energy");
		case ATTACK_SPIRIT: return AString("spirit");
		case ATTACK_WEATHER: return AString("weather");
		case ATTACK_RIDING: return AString("riding");
		case ATTACK_RANGED: return AString("ranged");
		case NUM_ATTACK_TYPES: return AString("all");
		default: return AString("unknown");
	}
}

AString *ItemDescription(int item, int full)
{
	if(ItemDefs[item].flags & ItemType::DISABLED)
		return NULL;

	AString *temp = new AString;
	int illusion = ((ItemDefs[item].type & IT_MONSTER) &&
			(ItemDefs[item].index == MONSTER_ILLUSION));

	*temp += AString(illusion?"illusory ":"")+ ItemDefs[item].name + " [" +
		(illusion?"I":"") + ItemDefs[item].abr + "], weight " +
		ItemDefs[item].weight;

	if (ItemDefs[item].walk) {
		int cap = ItemDefs[item].walk - ItemDefs[item].weight;
		if(cap) {
			*temp += AString(", walking capacity ") + cap;
		} else {
			*temp += ", can walk";
		}
	}
	if((ItemDefs[item].hitchItem != -1 )&&
			!(ItemDefs[ItemDefs[item].hitchItem].flags & ItemType::DISABLED)) {
		int cap = ItemDefs[item].walk - ItemDefs[item].weight +
			ItemDefs[item].hitchwalk;
		if(cap) {
			*temp += AString(", walking capacity ") + cap +
				" when hitched to a " +
				ItemDefs[ItemDefs[item].hitchItem].name;
		}
	}
	if (ItemDefs[item].ride) {
		int cap = ItemDefs[item].ride - ItemDefs[item].weight;
		if(cap) {
			*temp += AString(", riding capacity ") + cap;
		} else {
			*temp += ", can ride";
		}
	}
	if (ItemDefs[item].swim) {
		int cap = ItemDefs[item].swim - ItemDefs[item].weight;
		if(cap) {
			*temp += AString(", swimming capacity ") + cap;
		} else {
			*temp += ", can swim";
		}
	}
	if (ItemDefs[item].fly) {
		int cap = ItemDefs[item].fly - ItemDefs[item].weight;
		if(cap) {
			*temp += AString(", flying capacity ") + cap;
		} else {
			*temp += ", can fly";
		}
	}

	if(Globals->ALLOW_WITHDRAW) {
		if(ItemDefs[item].type & IT_NORMAL && item != I_SILVER) {
			*temp += AString(", costs ") + (ItemDefs[item].baseprice*5/2) +
				" silver to withdraw";
		}
	}
	*temp += ".";

	if(ItemDefs[item].type & IT_MAN) {
		int man = ItemDefs[item].index;
		int found = 0;
		*temp += " This race may study ";
		unsigned int c;
		unsigned int len = sizeof(ManDefs[man].skills)/sizeof(int);
		for(c = 0; c < len; c++) {
			if(ManDefs[man].skills[c] != -1) {
				if(found) *temp += ", ";
				if(found && c == len - 1) *temp += "and ";
				found = 1;
				*temp += SkillStrs(ManDefs[man].skills[c]);
			}
		}
		if(found) {
			*temp += AString(" to level ") + ManDefs[man].speciallevel +
			   	" and all others to level " + ManDefs[man].defaultlevel;
		} else {
			*temp += AString("all skills to level ") +
				ManDefs[man].defaultlevel;
		}
	}
	if(ItemDefs[item].type & IT_MONSTER) {
		*temp += " This is a monster.";
		int mon = ItemDefs[item].index;
		*temp += AString(" This monster attacks with a combat skill of ") +
			MonDefs[mon].attackLevel + ".";
		for(int c = 0; c < NUM_ATTACK_TYPES; c++) {
			*temp += AString(" ") + MonResist(c,MonDefs[mon].defense[c], full);
		}
		if(MonDefs[mon].special && MonDefs[mon].special != -1) {
			*temp += AString(" ") +
				ItemSpecial(MonDefs[mon].special, MonDefs[mon].specialLevel);
		}
		if(full) {
			int hits = MonDefs[mon].hits;
			int atts = MonDefs[mon].numAttacks;
			if(!hits) hits = 1;
			if(!atts) atts = 1;
			*temp += AString(" This monster has ") + atts + " " +
				((atts > 1)?"attacks":"attack") + " per round and takes " +
				hits + " " + ((hits > 1)?"hits":"hit") + " to kill.";
			*temp += AString(" This monster has a tactics score of ") +
				MonDefs[mon].tactics + ", a stealth score of " +
				MonDefs[mon].stealth + ", and an observation score of " +
				MonDefs[mon].obs + ".";
		}
		*temp += " This monster drops ";
		if(MonDefs[mon].spoiltype != -1) {
			if(MonDefs[mon].spoiltype & IT_MAGIC) {
				*temp += "magic items and ";
			} else if(MonDefs[mon].spoiltype & IT_ADVANCED) {
				*temp += "advanced items and ";
			} else if(MonDefs[mon].spoiltype & IT_NORMAL) {
				*temp += "normal or trade items and ";
			}
		}
		*temp += "silver as treasure.";
	}
	if(ItemDefs[item].type & IT_WEAPON) {
		*temp += " This is a weapon.";
		int wep = ItemDefs[item].index;
		WeaponType *pW = &WeaponDefs[wep];
		if(pW->flags & WeaponType::NEEDSKILL) {
			*temp += AString(" Knowledge of ") + SkillStrs(pW->baseSkill);
			if(pW->orSkill != -1) {
				*temp += AString(" or ") + SkillStrs(pW->orSkill);
			}
			*temp += " is needed to wield this weapon.";
		} else {
			if(pW->baseSkill == -1 && pW->orSkill == -1) {
				*temp += " No skill is needed to wield this weapon.";
			}
		}
		*temp += AString(" This is a ") + WeapClass(pW->weapClass) + 
			" weapon.";
		if(pW->flags & WeaponType::LONG) {
			*temp += " This weapon has a long reach.";
		} else if(pW->flags & WeaponType::SHORT) {
			*temp += " This weapon has a short reach.";
		} else if(pW->flags & WeaponType::RANGED) {
			*temp += " This is a ranged weapon.";
		} else {
			*temp += " This weapon has a normal reach.";
		}

		if(pW->flags & WeaponType::NOFOOT) {
			*temp += " Only mounted troops may use this weapon.";
		} else if(pW->flags & WeaponType::NOMOUNT) {
			*temp += " Only foot troops may use this weapon.";
		}
		if(pW->flags & WeaponType::NODEFENSE) {
			*temp += " Defenders are treated as if they have an "
				"effective combat skill of 0.";
		}
		if(pW->flags & WeaponType::NOATTACKERSKILL) {
			*temp += " Attackers do not get skill bonus on defense.";
		}
		if(pW->flags & WeaponType::ALWAYSREADY) {
			*temp += " Wielders of this weapon never miss a round to ready "
				"their weapon.";
		}

		if(full) {
			*temp += AString(" This weapon attacks versus the target's ") +
				"defense against " + AttType(pW->attackType) + " attacks.";
			*temp += AString(" This weapon allows ");
			if(pW->numAttacks > 0) {
				if(pW->numAttacks >= WeaponType::NUM_ATTACKS_SKILL) {
					*temp += " a number of attacks equal to skill level";
					if(pW->numAttacks > WeaponType::NUM_ATTACKS_SKILL) {
						int val = pW->numAttacks -
							WeaponType::NUM_ATTACKS_SKILL;
						*temp += AString("+") + val;
					}
				} else {
					*temp += AString(pW->numAttacks) + " attacks";
				}
				*temp += " per round.";
			} else {
				*temp += AString("1 attack every ") + -pW->numAttacks +
					" rounds.";
			}
		}
		*temp += AString(" This weapon grants ") +
			((pW->attackBonus>=0)?"+":"") + pW->attackBonus + " on attack " +
			"and " + ((pW->defenseBonus>=0)?"+":"") + pW->defenseBonus +
			" on defense.";
		if(full) {
			if(pW->mountBonus) {
				*temp += AString(" This weapon grants ") +
					((pW->mountBonus>0)?"+":"") + pW->mountBonus +
					" versus mounted opponents.";
			}
		}
	}
	if(ItemDefs[item].type & IT_ARMOR) {
		*temp += " This is an armor.";
		int arm = ItemDefs[item].index;
		ArmorType *pA = &ArmorDefs[arm];
		*temp += " This armor will protect its wearer ";
		for(int i = 0; i < NUM_WEAPON_CLASSES; i++) {
			if(i == NUM_WEAPON_CLASSES - 1) {
				*temp += ", and ";
			} else if(i > 0) {
				*temp += ", ";
			}
			*temp += AString(pA->saves[i]) + "/" + pA->from +
				" of the time versus " + WeapClass(i) + " attacks";
		}
		*temp += ".";
		if(full) {
			if(pA->flags & ArmorType::USEINASSASSINATE) {
				*temp += " This armor may be worn by assassins.";
			}
		}
	}

	if(ItemDefs[item].type & IT_TOOL) {
		int comma = 0;
		int last = -1;
		*temp += " This is a tool.";
		*temp += " This item increases the production of ";
		for(int i = NITEMS - 1; i > 0; i--) {
			if(ItemDefs[i].flags & ItemType::DISABLED) continue;
			if(ItemDefs[i].mult_item == item) {
				last = i;
				break;
			}
		}
		for(int i = 0; i < NITEMS; i++) {
		   if(ItemDefs[i].flags & ItemType::DISABLED) continue;
		   if(ItemDefs[i].mult_item == item) {
			   if(comma) {
				   if(last == i) {
					   if(comma > 1) *temp += ",";
					   *temp += " and ";
				   } else {
					   *temp += ", ";
				   }
			   }
			   comma++;
			   *temp += AString(ItemDefs[i].names) + " by " +
				   ItemDefs[i].mult_val;
		   }
		}
		*temp += ".";
	}

	if(ItemDefs[item].type & IT_TRADE) {
		*temp += " This is a trade good.";
		if(full) {
			if(Globals->RANDOM_ECONOMY) {
				int maxbuy, minbuy, maxsell, minsell;
				if(Globals->MORE_PROFITABLE_TRADE_GOODS) {
					minsell = (ItemDefs[item].baseprice*250)/100;
					maxsell = (ItemDefs[item].baseprice*350)/100;
					minbuy = (ItemDefs[item].baseprice*100)/100;
					maxbuy = (ItemDefs[item].baseprice*190)/100;
				} else {
					minsell = (ItemDefs[item].baseprice*150)/100;
					maxsell = (ItemDefs[item].baseprice*200)/100;
					minbuy = (ItemDefs[item].baseprice*100)/100;
					maxbuy = (ItemDefs[item].baseprice*150)/100;
				}
				*temp += AString(" This item can be bought for between ") +
					minbuy + " and " + maxbuy + " silver.";
				*temp += AString(" This item can be sold for between ") +
					minsell+ " and " + maxsell+ " silver.";
			} else {
				*temp += AString(" This item can be bought and sold for ") +
					ItemDefs[item].baseprice + " silver.";
			}
		}
	}

	if(ItemDefs[item].type & IT_MOUNT) {
		*temp += " This is a mount.";
		int mnt = ItemDefs[item].index;
		MountType *pM = &MountDefs[mnt];
		if(pM->skill == -1) {
			*temp += " No skill is required to use this mount.";
		} else if(SkillDefs[pM->skill].flags & SkillType::DISABLED) {
			*temp += " This mount is unridable.";
		} else {
			*temp += AString(" This mount requires ") + SkillStrs(pM->skill) +
				" of at least level " + pM->minBonus + " to ride in combat.";
		}
		*temp += AString(" This mount gives a minimum bonus of +") +
			pM->minBonus + " when ridden into combat.";
		*temp += AString(" This mount gives a maximum bonus of +") +
			pM->maxBonus + " when ridden into combat.";
		if(full) {
			if(ItemDefs[item].fly) {
				*temp += AString(" This mount gives a maximum bonus of +") +
					pM->maxHamperedBonus + " when ridden into combat in " +
					"terrain which allows ridden mounts but not flying "+
					"mounts.";
			}
			/* XXX -- handle mount special */
		}
	}

	if(ItemDefs[item].pSkill != -1 &&
			!(SkillDefs[ItemDefs[item].pSkill].flags & SkillType::DISABLED)) {
		unsigned int c;
		unsigned int len;
		*temp += AString(" Units with ") + SkillStrs(ItemDefs[item].pSkill) +
			" " + ItemDefs[item].pLevel + " may PRODUCE this item";
		len = sizeof(ItemDefs[item].pInput)/sizeof(Materials);
		int count = 0;
		int tot = len;
		for(c = 0; c < len; c++) {
			int itm = ItemDefs[item].pInput[c].item;
			int amt = ItemDefs[item].pInput[c].amt;
			if(itm == -1 || ItemDefs[itm].flags & ItemType::DISABLED) {
				tot--;
				continue;
			}
			if(count == 0) {
				*temp += " from ";
			} else if (count == tot) {
				if(c > 1) *temp += ",";
				*temp += " and ";
			} else {
				*temp += ", ";
			}
			count++;
			*temp += AString(amt) + " " + ItemDefs[itm].names;
		}
		if(ItemDefs[item].pOut) {
			*temp += AString(" at a rate of ") + ItemDefs[item].pOut;
			if(ItemDefs[item].pMonths) {
				if(ItemDefs[item].pMonths == 1) {
					*temp += " per man-month.";
				} else {
					*temp += AString(" per ") + ItemDefs[item].pMonths +
						" man-months.";
				}
			}
		}
	}
	if(ItemDefs[item].mSkill != -1 &&
			!(SkillDefs[ItemDefs[item].mSkill].flags & SkillType::DISABLED)) {
		unsigned int c;
		unsigned int len;
		*temp += AString(" Units with ") + SkillStrs(ItemDefs[item].mSkill) +
			" " + ItemDefs[item].mLevel + " may create this item via magic";
		len = sizeof(ItemDefs[item].mInput)/sizeof(Materials);
		int count = 0;
		int tot = len;
		for(c = 0; c < len; c++) {
			int itm = ItemDefs[item].mInput[c].item;
			int amt = ItemDefs[item].mInput[c].amt;
			if(itm == -1 || ItemDefs[itm].flags & ItemType::DISABLED) {
				tot--;
				continue;
			}
			if(count == 0) {
				*temp += " from ";
			} else if (count == tot) {
				if(c > 1) *temp += ",";
				*temp += " and ";
			} else {
				*temp += ", ";
			}
			count++;
			*temp += AString(amt) + " " + ItemDefs[itm].names;
		}
		*temp += ".";
	}

	if((ItemDefs[item].type & IT_BATTLE) && full) {
		*temp += " This item is a miscellaneous combat item.";
		for(int i = 0; i < NUMBATTLEITEMS; i++) {
			if(BattleItemDefs[i].itemNum == item) {
				if(BattleItemDefs[i].flags & BattleItemType::MAGEONLY) {
					*temp += " This item may only be used by a mage";
					if(Globals->APPRENTICES_EXIST) {
						*temp += " or an apprentice";
					}
					*temp += ".";
				}
				if(BattleItemDefs[i].flags & BattleItemType::SPECIAL) {
					*temp += AString(" ") +
						ItemSpecial(BattleItemDefs[i].index,
								BattleItemDefs[i].skillLevel);
				} else if(BattleItemDefs[i].flags & BattleItemType::SHIELD) {
					*temp += AString("Can cast a shield against ") +
						AttType(BattleItemDefs[i].index) +
						" attacks at level " + BattleItemDefs[i].skillLevel +
						".";
				}
			}
		}
	}
	if((ItemDefs[item].flags & ItemType::CANTGIVE) && full) {
		*temp += " This item cannot be given to other units.";
	}

	return temp;
}

int IsSoldier(int item)
{
	if (ItemDefs[item].type & IT_MAN || ItemDefs[item].type & IT_MONSTER)
		return 1;
	return 0;
}

Item::Item()
{
}

Item::~Item()
{
}

AString Item::Report(int seeillusions)
{
	AString ret = ItemString(type,num);
	if (seeillusions && ItemDefs[type].type & IT_MONSTER &&
			ItemDefs[type].index == MONSTER_ILLUSION) {
		ret = ret + " (illusion)";
	}
	return ret;
}

void Item::Writeout(Aoutfile * f)
{
	f->PutInt(type);
	f->PutInt(num);
}

void Item::Readin(Ainfile * f)
{
	type = f->GetInt();
	num = f->GetInt();
}

void ItemList::Writeout(Aoutfile * f)
{
	f->PutInt(Num());
	forlist (this)
		((Item *) elem)->Writeout(f);
}

void ItemList::Readin(Ainfile * f)
{
	int i = f->GetInt();
	for (int j=0; j<i; j++) {
		Item * temp = new Item;
		temp->Readin(f);
		if (temp->num < 1) {
			delete temp;
		} else {
			Add(temp);
		}
	}
}

int ItemList::GetNum(int t)
{
	forlist(this) {
		Item * i = (Item *) elem;
		if (i->type == t) return i->num;
	}
	return 0;
}

int ItemList::Weight()
{
	int wt = 0;
	forlist(this) {
		Item * i = (Item *) elem;
		wt += ItemDefs[i->type].weight * i->num;
	}
	return wt;
}

AString ItemList::Report(int obs,int seeillusions,int nofirstcomma)
{
	AString temp;
	forlist(this) {
		Item * i = (Item *) elem;
		if (obs == 2) {
			if (nofirstcomma) {
				nofirstcomma = 0;
			} else {
				temp += ", ";
			}
			temp += i->Report(seeillusions);
		} else {
			if (ItemDefs[i->type].weight) {
				if (nofirstcomma) {
					nofirstcomma = 0;
				} else {
					temp += ", ";
				}
				temp += i->Report(seeillusions);
			}
		}
	}
	return temp;
}

AString ItemList::BattleReport()
{
	AString temp;
	forlist(this) {
		Item * i = (Item *) elem;
		if (ItemDefs[i->type].combat) {
			temp += ", ";
			temp += i->Report(0);
			if (ItemDefs[i->type].type & IT_MONSTER) {
				MonType & mondef = MonDefs[ItemDefs[i->type].index];
				temp += AString(" (Combat ") + mondef.attackLevel +
					"/" + mondef.defense[ATTACK_COMBAT] + ", Attacks " +
					mondef.numAttacks + ", Hits " + mondef.hits +
					", Tactics " + mondef.tactics + ")";
			}
		}
	}
	return temp;
}

void ItemList::SetNum(int t,int n)
{
	if (n) {
		forlist(this) {
			Item * i = (Item *) elem;
			if (i->type == t) {
				i->num = n;
				return;
			}
		}
		Item * i = new Item;
		i->type = t;
		i->num = n;
		Add(i);
	} else {
		forlist(this) {
			Item * i = (Item *) elem;
			if (i->type == t) {
				Remove(i);
				delete i;
				return;
			}
		}
	}
}

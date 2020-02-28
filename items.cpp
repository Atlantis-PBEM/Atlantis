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

#include <stdlib.h>
#include "items.h"
#include "skills.h"
#include "object.h"
#include "gamedata.h"

BattleItemType *FindBattleItem(char const *abbr)
{
	if (abbr == NULL) return NULL;
	for (int i = 0; i < NUMBATTLEITEMS; i++) {
		if (BattleItemDefs[i].abbr == NULL) continue;
		if (AString(abbr) == BattleItemDefs[i].abbr)
			return &BattleItemDefs[i];
	}
	return NULL;
}

ArmorType *FindArmor(char const *abbr)
{
	if (abbr == NULL) return NULL;
	for (int i = 0; i < NUMARMORS; i++) {
		if (ArmorDefs[i].abbr == NULL) continue;
		if (AString(abbr) == ArmorDefs[i].abbr)
			return &ArmorDefs[i];
	}
	return NULL;
}

WeaponType *FindWeapon(char const *abbr)
{
	if (abbr == NULL) return NULL;
	for (int i = 0; i < NUMWEAPONS; i++) {
		if (WeaponDefs[i].abbr == NULL) continue;
		if (AString(abbr) == WeaponDefs[i].abbr)
			return &WeaponDefs[i];
	}
	return NULL;
}

MountType *FindMount(char const *abbr)
{
	if (abbr == NULL) return NULL;
	for (int i = 0; i < NUMMOUNTS; i++) {
		if (MountDefs[i].abbr == NULL) continue;
		if (AString(abbr) == MountDefs[i].abbr)
			return &MountDefs[i];
	}
	return NULL;
}

MonType *FindMonster(char const *abbr, int illusion)
{
	if (abbr == NULL) return NULL;
	AString tag = (illusion ? "i" : "");
	tag += abbr;

	for (int i = 0; i < NUMMONSTERS; i++) {
		if (MonDefs[i].abbr == NULL) continue;
		if (tag == MonDefs[i].abbr)
			return &MonDefs[i];
	}
	return NULL;
}

ManType *FindRace(char const *abbr)
{
	if (abbr == NULL) return NULL;
	for (int i = 0; i < NUMMAN; i++) {
		if (ManDefs[i].abbr == NULL) continue;
		if (AString(abbr) == ManDefs[i].abbr)
			return &ManDefs[i];
	}
	return NULL;
}

AString AttType(int atype)
{
	switch(atype) {
		case ATTACK_COMBAT: return AString("melee");
		case ATTACK_ENERGY: return AString("energy");
		case ATTACK_SPIRIT: return AString("spirit");
		case ATTACK_WEATHER: return AString("weather");
		case ATTACK_RIDING: return AString("riding");
		case ATTACK_RANGED: return AString("ranged");
		case NUM_ATTACK_TYPES: return AString("non-resistable");
		default: return AString("unknown");
	}
}

static AString DefType(int atype)
{
	if (atype == NUM_ATTACK_TYPES) return AString("all");
	return AttType(atype);
}

int LookupItem(AString *token)
{
	for (int i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].type & IT_ILLUSION) {
			if (*token == (AString("i") + ItemDefs[i].abr)) return i;
		} else {
			if (*token == ItemDefs[i].abr) return i;
		}
	}
	return -1;
}

int ParseAllItems(AString *token)
{
	int r = -1;
	for (int i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].type & IT_ILLUSION) {
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
	return r;
}

int ParseEnabledItem(AString *token)
{
	int r = -1;
	for (int i=0; i<NITEMS; i++) {
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (ItemDefs[i].type & IT_ILLUSION) {
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
	if (r != -1) {
		if (ItemDefs[r].flags & ItemType::DISABLED) r = -1;
	}
	return r;
}

int ParseGiveableItem(AString *token)
{
	int r = -1;
	for (int i=0; i<NITEMS; i++) {
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (ItemDefs[i].flags & ItemType::CANTGIVE) continue;
		if (ItemDefs[i].type & IT_ILLUSION) {
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
	if (r != -1) {
		if (ItemDefs[r].flags & ItemType::DISABLED) r = -1;
	}
	return r;
}

int ParseTransportableItem(AString *token)
{
	int r = -1;
	for (int i=0; i<NITEMS; i++) {
		if (ItemDefs[i].flags & ItemType::DISABLED) continue;
		if (ItemDefs[i].flags & ItemType::NOTRANSPORT) continue;
		if (ItemDefs[i].flags & ItemType::CANTGIVE) continue;
		if (ItemDefs[i].type & IT_ILLUSION) {
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
	if (r != -1) {
		if (ItemDefs[r].flags & ItemType::DISABLED) r = -1;
	}
	return r;
}

AString ItemString(int type, int num, int flags)
{
	AString temp;
	if (num == 1) {
		if (flags & FULLNUM)
			temp += AString(num) + " ";
		temp +=
			AString((flags & ALWAYSPLURAL) ?
				ItemDefs[type].names: ItemDefs[type].name) +
			" [" + ItemDefs[type].abr + "]";
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

static AString EffectStr(char const *effect)
{
	AString temp, temp2;
	int comma = 0;
	int i;

	EffectType *ep = FindEffect(effect);

	temp += ep->name;

	if (ep->attackVal) {
		temp2 += AString(ep->attackVal) + " to attack";
		comma = 1;
	}

	for (i = 0; i < 4; i++) {
		if (ep->defMods[i].type == -1) continue;
		if (comma) temp2 += ", ";
		temp2 += AString(ep->defMods[i].val) + " versus " +
			DefType(ep->defMods[i].type) + " attacks";
		comma = 1;
	}

	if (comma) {
		temp += AString(" (") + temp2 + ")";
	}

	if (comma) {
		if (ep->flags & EffectType::EFF_ONESHOT) {
			temp += " for their next attack";
		} else {
			temp += " for the rest of the battle";
		}
	}
	temp += ".";

	if (ep->cancelEffect != NULL) {
		if (comma) temp += " ";
		EffectType *up = FindEffect(ep->cancelEffect);
		temp += AString("This effect cancels out the effects of ") +
			up->name + ".";
	}
	return temp;
}

AString ShowSpecial(char const *special, int level, int expandLevel, int fromItem)
{
	AString temp;
	int comma = 0;
	int i;
	int last = -1;
	int val;

	SpecialType *spd = FindSpecial(special);
	temp += spd->specialname;
	temp += AString(" in battle");
	if (expandLevel)
		temp += AString(" at a skill level of ") + level;
	temp += ".";

	if ((spd->targflags & SpecialType::HIT_BUILDINGIF) ||
			(spd->targflags & SpecialType::HIT_BUILDINGEXCEPT)) {
		temp += " This ability will ";
		if (spd->targflags & SpecialType::HIT_BUILDINGEXCEPT) {
			temp += "only target units inside structures, with the exception of";
		} else {
			temp += "only target units which are inside";
		}

		temp += " the following structures: ";
		for (i = 0; i < SPECIAL_BUILDINGS; i++) {
			if (spd->buildings[i] == -1) continue;
			if (ObjectDefs[spd->buildings[i]].flags & ObjectType::DISABLED)
				continue;
			if (last == -1) {
				last = i;
				continue;
			}
			temp += AString(ObjectDefs[spd->buildings[last]].name) + ", ";
			last = i;
			comma++;
		}
		if (comma) {
			temp += "or ";
		}
		temp += AString(ObjectDefs[spd->buildings[last]].name) + ".";
	}
	if ((spd->targflags & SpecialType::HIT_SOLDIERIF) ||
			(spd->targflags & SpecialType::HIT_SOLDIEREXCEPT) ||
			(spd->targflags & SpecialType::HIT_MOUNTIF) ||
			(spd->targflags & SpecialType::HIT_MOUNTEXCEPT)) {
		temp += " This ability will ";
		if ((spd->targflags & SpecialType::HIT_SOLDIEREXCEPT) ||
				(spd->targflags & SpecialType::HIT_MOUNTEXCEPT)) {
			temp += "not ";
		} else {
			temp += "only ";
		}
		temp += "target ";
		if ((spd->targflags & SpecialType::HIT_MOUNTIF) ||
				(spd->targflags & SpecialType::HIT_MOUNTEXCEPT)) {
			temp += "units mounted on ";
		}
		comma = 0;
		last = -1;
		for (i = 0; i < 7; i++) {
			if (spd->targets[i] == -1) continue;
			if (ItemDefs[spd->targets[i]].flags & ItemType::DISABLED) continue;
			if (last == -1) {
				last = i;
				continue;
			}
			temp += ItemString(spd->targets[last], 1, ALWAYSPLURAL) + ", ";
			last = i;
			comma++;
		}
		if (comma) {
			temp += "or ";
		}
		temp += ItemString(spd->targets[last], 1, ALWAYSPLURAL) + ".";
	}
	if ((spd->targflags & SpecialType::HIT_EFFECTIF) ||
			(spd->targflags & SpecialType::HIT_EFFECTEXCEPT)) {
		temp += " This ability will ";
		if (spd->targflags & SpecialType::HIT_EFFECTEXCEPT) {
			temp += "not ";
		} else {
			temp += "only ";
		}
		temp += "target creatures which are currently affected by ";
		EffectType *ep;
		for (i = 0; i < 3; i++) {
			if (spd->effects[i] == NULL) continue;
			if (last == -1) {
				last = i;
				continue;
			}
			ep = FindEffect(spd->effects[last]);
			temp += AString(ep->name) + ", ";
			last = i;
			comma++;
		}
		if (comma) {
			temp += "or ";
		}
		ep = FindEffect(spd->effects[last]);
		temp += AString(ep->name) + ".";
	}
	if (spd->targflags & SpecialType::HIT_ILLUSION) {
		temp += " This ability will only target illusions.";
	}
	if (spd->targflags & SpecialType::HIT_NOMONSTER) {
		temp += " This ability cannot target monsters.";
	}
	if (spd->effectflags & SpecialType::FX_NOBUILDING) {
		temp += AString(" The bonus given to units inside buildings is ") +
			"not effective against this ability.";
	}

	if (spd->effectflags & SpecialType::FX_SHIELD) {
		if (!fromItem)
			temp += " This spell provides a shield against all ";
		else
			temp += AString(" This ability provides the wielder with a defence bonus of ") + level + " against all ";
		comma = 0;
		last = -1;
		for (i = 0; i < 4; i++) {
			if (spd->shield[i] == -1) continue;
			if (last == -1) {
				last = i;
				continue;
			}
			temp += DefType(spd->shield[last]) + ", ";
			last = i;
			comma++;
		}
		if (comma) {
			temp += "and ";
		}
		if (fromItem)
			temp += DefType(spd->shield[last]) + " attacks.";
		else {
			temp += DefType(spd->shield[last]) + " attacks against the entire" +
				" army at a level equal to the skill level of the ability.";
		}
	}
	if (spd->effectflags & SpecialType::FX_DEFBONUS) {
		temp += " This ";
		if (fromItem)
			temp += "ability";
		else
			temp += "spell";
		temp += " provides ";
		comma = 0;
		last = -1;
		for (i = 0; i < 4; i++) {
			if (spd->defs[i].type == -1) continue;
			if (last == -1) {
				last = i;
				continue;
			}
			val = spd->defs[last].val;
			if (expandLevel) {
				if (spd->effectflags & SpecialType::FX_USE_LEV)
					val *= level;
			}

			temp += AString("a defensive bonus of ") + val;
			if (!expandLevel) {
				temp += " per skill level";
			}
			temp += AString(" versus ") + DefType(spd->defs[last].type) +
				" attacks, ";
			last = i;
			comma++;
		}
		if (comma) {
			temp += "and ";
		}
		val = spd->defs[last].val;
		if (expandLevel) {
			if (spd->effectflags & SpecialType::FX_USE_LEV)
				val *= level;
		}
		temp += AString("a defensive bonus of ") + val;
		if (!expandLevel) {
			temp += " per skill level";
		}
		temp += AString(" versus ") + DefType(spd->defs[last].type) +
			" attacks";
		temp += " to the user.";
	}

	/* Now the damages */
	for (i = 0; i < 4; i++) {
		if (spd->damage[i].type == -1) continue;
		temp += AString(" This ability does between ") +
			spd->damage[i].minnum + " and ";
		val = spd->damage[i].value * 2;
		if (expandLevel) {
			if (spd->effectflags & SpecialType::FX_USE_LEV)
				val *= level;
		}
		temp += AString(val);
		if (!expandLevel) {
			temp += " times the skill level of the mage";
		}
		temp += AString(" ") + AttType(spd->damage[i].type) + " attacks.";
		if (spd->damage[i].effect) {
			temp += " Each attack causes the target to be effected by ";
			temp += EffectStr(spd->damage[i].effect);
		}
	}

	return temp;
}

static AString MonResist(int type, int val, int full)
{
	AString temp = "This monster ";
	if (full) {
		temp += AString("has a resistance of ") + val;
	} else {
		temp += "is ";
		if (val < 1) temp += "very susceptible";
		else if (val == 1) temp += "susceptible";
		else if (val > 1 && val < 3) temp += "typically resistant";
		else if (val > 2 && val < 5) temp += "slightly resistant";
		else temp += "very resistant";
	}
	temp += " to ";
	temp += AttType(type);
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
		case ARMORPIERCING: return AString("armor-piercing");
		case MAGIC_ENERGY: return AString("energy");
		case MAGIC_SPIRIT: return AString("spirit");
		case MAGIC_WEATHER: return AString("weather");
		default: return AString("unknown");
	}
}

static AString WeapType(int flags, int wclass)
{
	AString type;
	if (flags & WeaponType::RANGED) type = "ranged ";
	if (flags & WeaponType::LONG) type = "long ";
	if (flags & WeaponType::SHORT) type = "short ";
	type += WeapClass(wclass);
	return type;
}

AString *ItemDescription(int item, int full)
{
	int i;
	AString skname;
	SkillType *pS;

	if (ItemDefs[item].flags & ItemType::DISABLED)
		return NULL;

	AString *temp = new AString;
	int illusion = (ItemDefs[item].type & IT_ILLUSION);
	
	*temp += AString(illusion?"illusory ":"")+ ItemDefs[item].name + " [" +
			(illusion?"I":"") + ItemDefs[item].abr + "]";
		
	/* new ship items */
	if (ItemDefs[item].type & IT_SHIP) {
		if (ItemDefs[item].swim > 0) {
			*temp += AString(". This is a ship with a capacity of ") + ItemDefs[item].swim;
		}
		if (ItemDefs[item].fly > 0) {
			*temp += AString(". This is a flying 'ship' with a capacity of ") + ItemDefs[item].fly;
		}
		*temp += " and a speed of ";
		*temp += ItemDefs[item].speed;
		*temp += " hex";
		if (ItemDefs[item].speed != 1)
			*temp += "es";
		*temp += " per month";
		*temp += AString(". This ship requires a total of ") + ItemDefs[item].weight/50 + " levels of sailing skill to sail";
		AString abbr = ItemDefs[item].name;
		int objectno = LookupObject(&abbr);
		if (objectno >= 0) {
			if (ObjectDefs[objectno].protect > 0) {
				*temp += ". This ship provides defense to the first ";
				*temp += ObjectDefs[objectno].protect;
				*temp += " men inside it, ";
				int totaldef = 0;
				for (int i=0; i<NUM_ATTACK_TYPES; i++) {
					totaldef += (ObjectDefs[objectno].defenceArray[i] != 0);
				}
				// Now add the description to temp
				*temp += AString("giving a defensive bonus of ");
				for (int i=0; i<NUM_ATTACK_TYPES; i++) {
					if (ObjectDefs[objectno].defenceArray[i]) {
						totaldef--;
						*temp += AString(ObjectDefs[objectno].defenceArray[i]) + " against " +
							AttType(i) + AString(" attacks");

						if (totaldef >= 2) {
							*temp += AString(", ");
						} else {
							if (totaldef == 1) {	// penultimate bonus
								*temp += AString(" and ");
							} else {	// last bonus
							}
						} // end if
					}
				} // end for
			}
			if (Globals->LIMITED_MAGES_PER_BUILDING && ObjectDefs[objectno].maxMages > 0) {
				*temp += ". This ship will allow ";
				if (ObjectDefs[objectno].maxMages > 1) {
					*temp += "up to ";
					*temp += ObjectDefs[objectno].maxMages;
					*temp += " mages";
				} else {
					*temp += "one mage";
				}
				*temp += " to study above level 2";
			}
		}
	} else {
		
		*temp += AString(", weight ") + ItemDefs[item].weight;

		if (ItemDefs[item].walk) {
			int cap = ItemDefs[item].walk - ItemDefs[item].weight;
			if (cap) {
				*temp += AString(", walking capacity ") + cap;
			} else {
				*temp += ", can walk";
			}
		}
		if ((ItemDefs[item].hitchItem != -1 )&&
				!(ItemDefs[ItemDefs[item].hitchItem].flags & ItemType::DISABLED)) {
			int cap = ItemDefs[item].walk - ItemDefs[item].weight +
				ItemDefs[item].hitchwalk;
			if (cap) {
				*temp += AString(", walking capacity ") + cap +
					" when hitched to a " +
					ItemDefs[ItemDefs[item].hitchItem].name;
			}
		}
		if (ItemDefs[item].ride) {
			int cap = ItemDefs[item].ride - ItemDefs[item].weight;
			if (cap) {
				*temp += AString(", riding capacity ") + cap;
			} else {
				*temp += ", can ride";
			}
		}
		if (ItemDefs[item].swim) {
			int cap = ItemDefs[item].swim - ItemDefs[item].weight;
			if (cap) {
				*temp += AString(", swimming capacity ") + cap;
			} else {
				*temp += ", can swim";
			}
		}
		if (ItemDefs[item].fly) {
			int cap = ItemDefs[item].fly - ItemDefs[item].weight;
			if (cap) {
				*temp += AString(", flying capacity ") + cap;
			} else {
				*temp += ", can fly";
			}
		}
		if (ItemDefs[item].speed) {
			*temp += ", moves ";
			*temp += ItemDefs[item].speed;
			*temp += " hex";
			if (ItemDefs[item].speed != 1)
				*temp += "es";
			*temp += " per month";
		}
	}

	if (Globals->ALLOW_WITHDRAW) {
		if (ItemDefs[item].type & IT_NORMAL && item != I_SILVER) {
			*temp += AString(", costs ") + (ItemDefs[item].baseprice*5/2) +
				" silver to withdraw";
		}
	}
	*temp += ".";

	if (ItemDefs[item].type & IT_MAN) {
		ManType *mt = FindRace(ItemDefs[item].abr);
		AString mani = "MANI";
		AString last = "";
		int found = 0;
		*temp += " This race may study ";
		unsigned int c;
		unsigned int len = sizeof(mt->skills) / sizeof(mt->skills[0]);
		for (c = 0; c < len; c++) {
			pS = FindSkill(mt->skills[c]);
			if (!pS) continue;
			if (mani == pS->abbr && Globals->MAGE_NONLEADERS) {
				if (!(last == "")) {
					if (found)
						*temp += ", ";
					*temp += last;
					found = 1;
				}
				last = "all magical skills";
				continue;
			}
			if (pS->flags & SkillType::DISABLED) continue;
			if (!(last == "")) {
				if (found)
					*temp += ", ";
				*temp += last;
				found = 1;
			}
			last = SkillStrs(pS);
		}
		if (!(last == "")) {
			if (found)
				*temp += " and ";
			*temp += last;
			found = 1;
		}
		if (found) {
			*temp += AString(" to level ") + mt->speciallevel +
				" and all other skills to level " +
				mt->defaultlevel + ".";
		} else {
			*temp += AString("all skills to level ") + mt->defaultlevel + ".";
		}
	}
	if (ItemDefs[item].type & IT_MONSTER) {
		*temp += " This is a monster.";
		MonType *mp = FindMonster(ItemDefs[item].abr,
				(ItemDefs[item].type & IT_ILLUSION));
		*temp += AString(" This monster attacks with a combat skill of ") +
			mp->attackLevel + ".";
		for (int c = 0; c < NUM_ATTACK_TYPES; c++) {
			*temp += AString(" ") + MonResist(c,mp->defense[c], full);
		}
		if (mp->special && mp->special != NULL) {
			*temp += AString(" ") +
				"Monster can cast " +
				ShowSpecial(mp->special, mp->specialLevel, 1, 0);
		}
		if (full) {
			int hits = mp->hits;
			int atts = mp->numAttacks;
			int regen = mp->regen;
			if (!hits) hits = 1;
			if (!atts) atts = 1;
			*temp += AString(" This monster has ") + atts + " melee " +
				((atts > 1)?"attacks":"attack") + " per round and takes " +
				hits + " " + ((hits > 1)?"hits":"hit") + " to kill.";
			if (regen > 0) {
				*temp += AString(" This monsters regenerates ") + regen +
					" hits per round of battle.";
			}
			*temp += AString(" This monster has a tactics score of ") +
				mp->tactics + ", a stealth score of " + mp->stealth +
				", and an observation score of " + mp->obs + ".";
		}
		*temp += " This monster might have ";
		if (mp->spoiltype != -1) {
			if (mp->spoiltype & IT_MAGIC) {
				*temp += "magic items and ";
			} else if (mp->spoiltype & IT_ADVANCED) {
				*temp += "advanced items and ";
			} else if (mp->spoiltype & IT_NORMAL) {
				*temp += "normal or trade items and ";
			}
		}
		*temp += "silver as treasure.";
	}

	if (ItemDefs[item].type & IT_WEAPON) {
		WeaponType *pW = FindWeapon(ItemDefs[item].abr);
		*temp += " This is a ";
		*temp += WeapType(pW->flags, pW->weapClass) + " weapon.";
		if (pW->flags & WeaponType::NEEDSKILL) {
			pS = FindSkill(pW->baseSkill);
			if (pS) {
				*temp += AString(" Knowledge of ") + SkillStrs(pS);
				pS = FindSkill(pW->orSkill);
				if (pS)
					*temp += AString(" or ") + SkillStrs(pS);
				*temp += " is needed to wield this weapon.";
			}
		} else
			*temp += " No skill is needed to wield this weapon.";

		int flag = 0;
		if (pW->attackBonus != 0) {
			*temp += " This weapon grants a ";
			*temp += ((pW->attackBonus > 0) ? "bonus of " : "penalty of ");
			*temp += abs(pW->attackBonus);
			*temp += " on attack";
			flag = 1;
		}
		if (pW->defenseBonus != 0) {
			if (flag) {
				if (pW->attackBonus == pW->defenseBonus) {
					*temp += " and defense.";
					flag = 0;
				} else {
					*temp += " and a ";
				}
			} else {
				*temp += " This weapon grants a ";
				flag = 1;
			}
			if (flag) {
				*temp += ((pW->defenseBonus > 0)?"bonus of ":"penalty of ");
				*temp += abs(pW->defenseBonus);
				*temp += " on defense.";
				flag = 0;
			}
		}
		if (flag) *temp += ".";
		if (pW->mountBonus && full) {
			*temp += " This weapon ";
			if (pW->attackBonus != 0 || pW->defenseBonus != 0)
				*temp += "also ";
			*temp += "grants a ";
			*temp += ((pW->mountBonus > 0)?"bonus of ":"penalty of ");
			*temp += abs(pW->mountBonus);
			*temp += " against mounted opponents.";
		}

		if (pW->flags & WeaponType::NOFOOT)
			*temp += " Only mounted troops may use this weapon.";
		else if (pW->flags & WeaponType::NOMOUNT)
			*temp += " Only foot troops may use this weapon.";

		if (pW->flags & WeaponType::RIDINGBONUS) {
			*temp += " Wielders of this weapon, if mounted, get their riding "
				"skill bonus on combat attack and defense.";
		} else if (pW->flags & WeaponType::RIDINGBONUSDEFENSE) {
			*temp += " Wielders of this weapon, if mounted, get their riding "
				"skill bonus on combat defense.";
		}

		if (pW->flags & WeaponType::NODEFENSE) {
			*temp += " Defenders are treated as if they have an "
				"effective combat skill of 0.";
		}

		if (pW->flags & WeaponType::NOATTACKERSKILL) {
			*temp += " Attackers do not get skill bonus on defense.";
		}

		if (pW->flags & WeaponType::ALWAYSREADY) {
			*temp += " Wielders of this weapon never miss a round to ready "
				"their weapon.";
		} else {
			*temp += " There is a 50% chance that the wielder of this weapon "
				"gets a chance to attack in any given round.";
		}

		if (full) {
			int atts = pW->numAttacks;
			*temp += AString(" This weapon attacks versus the target's ") +
				"defense against " + AttType(pW->attackType) + " attacks.";
			*temp += AString(" This weapon allows ");
			if (atts > 0) {
				if (atts >= WeaponType::NUM_ATTACKS_HALF_SKILL) {
					int max = WeaponType::NUM_ATTACKS_HALF_SKILL;
					char const *attd = "half the skill level (rounded up)";
					if (atts >= WeaponType::NUM_ATTACKS_SKILL) {
						max = WeaponType::NUM_ATTACKS_SKILL;
						attd = "the skill level";
					}
					*temp += "a number of attacks equal to ";
					*temp += attd;
					*temp += " of the attacker";
					int val = atts - max;
					if (val > 0) *temp += AString(" plus ") + val;
				} else {
					*temp += AString(atts) + ((atts==1)?" attack":" attacks");
				}
				*temp += " per round.";
			} else {
				atts = -atts;
				*temp += "1 attack every ";
				if (atts == 1) *temp += "round .";
				else *temp += AString(atts) + " rounds.";
			}
		}
	}

	if (ItemDefs[item].type & IT_ARMOR) {
		*temp += " This is a type of armor.";
		ArmorType *pA = FindArmor(ItemDefs[item].abr);
		*temp += " This armor will protect its wearer ";
		for (i = 0; i < NUM_WEAPON_CLASSES; i++) {
			if (i == NUM_WEAPON_CLASSES - 1) {
				*temp += ", and ";
			} else if (i > 0) {
				*temp += ", ";
			}
			int percent = (int)(((float)pA->saves[i]*100.0) /
					(float)pA->from+0.5);
			*temp += AString(percent) + "% of the time versus " +
				WeapClass(i) + " attacks";
		}
		*temp += ".";
		if (full) {
			if (pA->flags & ArmorType::USEINASSASSINATE) {
				*temp += " This armor may be worn during assassination "
					"attempts.";
			}
		}
	}

	if (ItemDefs[item].type & IT_TOOL) {
		int comma = 0;
		int last = -1;
		*temp += " This is a tool.";
		*temp += " This item increases the production of ";
		for (i = NITEMS - 1; i > 0; i--) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (ItemDefs[i].mult_item == item) {
				last = i;
				break;
			}
		}
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (ItemDefs[i].mult_item == item) {
				if (comma) {
					if (last == i) {
						if (comma > 1) *temp += ",";
						*temp += " and ";
					} else {
						*temp += ", ";
					}
				}
				comma++;
				if (i == I_SILVER) {
					*temp += "entertainment";
				} else {
					*temp += ItemString(i, 1);
				}
				*temp += AString(" by ") + ItemDefs[i].mult_val;
			}
		}
		*temp += ".";
	}

	if (ItemDefs[item].type & IT_TRADE) {
		*temp += " This is a trade good.";
		if (full) {
			if (Globals->RANDOM_ECONOMY) {
				int maxbuy, minbuy, maxsell, minsell;
				if (Globals->MORE_PROFITABLE_TRADE_GOODS) {
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

	if (ItemDefs[item].type & IT_MOUNT) {
		*temp += " This is a mount.";
		MountType *pM = FindMount(ItemDefs[item].abr);
		if (pM->skill == NULL) {
			*temp += " No skill is required to use this mount.";
		} else {
			pS = FindSkill(pM->skill);
			if (!pS || (pS->flags & SkillType::DISABLED))
				*temp += " This mount is unridable.";
			else {
				*temp += AString(" This mount requires ") +
					SkillStrs(pS) + " of at least level " + pM->minBonus +
					" to ride in combat.";
			}
		}
		*temp += AString(" This mount gives a minimum bonus of +") +
			pM->minBonus + " when ridden into combat.";
		*temp += AString(" This mount gives a maximum bonus of +") +
			pM->maxBonus + " when ridden into combat.";
		if (full) {
			if (ItemDefs[item].fly) {
				*temp += AString(" This mount gives a maximum bonus of +") +
					pM->maxHamperedBonus + " when ridden into combat in " +
					"terrain which allows ridden mounts but not flying "+
					"mounts.";
			}
			if (pM->mountSpecial != NULL) {
				*temp += AString(" When ridden, this mount causes ") +
					ShowSpecial(pM->mountSpecial, pM->specialLev, 1, 0);
			}
		}
	} else {
		int found;

		pS = FindSkill(ItemDefs[item].pSkill);
		if (pS && !(pS->flags & SkillType::DISABLED)) {
			found = 0;
			for (i = 0; i < 4; i++)
				if (ItemDefs[item].pInput[i].item != -1)
					found = 1;
			if (!found)
				*temp += " This item is a trade resource.";
		}
	}

	if (ItemDefs[item].type & IT_MONEY) {
		*temp += " This is the currency of ";
		*temp += Globals->WORLD_NAME;
		*temp += ".";
	}

	if (!full)
		return temp;

	AttribModType *ap = FindAttrib("observation");
	if (ap) {
		for (i = 0; i < 5; i++)
			if (ap->mods[i].flags & AttribModItem::ITEM) {
				AString abbr = ItemDefs[item].abr;
				if (abbr == ap->mods[i].ident &&
						ap->mods[i].modtype == AttribModItem::CONSTANT) {
					*temp += " This item grants a ";
					*temp += ap->mods[i].val;
					*temp += " point bonus to a unit's observation skill";
					if (ap->mods[i].flags & AttribModItem::PERMAN) {
						*temp += " (note that a unit must possess one ";
						*temp += abbr;
						*temp += " for each man to gain this bonus)";
					}
					*temp += ".";
				}
			}
	}
	ap = FindAttrib("stealth");
	if (ap) {
		for (i = 0; i < 5; i++)
			if (ap->mods[i].flags & AttribModItem::ITEM) {
				AString abbr = ItemDefs[item].abr;
				if (abbr == ap->mods[i].ident &&
						ap->mods[i].modtype == AttribModItem::CONSTANT) {
					*temp += " This item grants a ";
					*temp += ap->mods[i].val;
					*temp += " point bonus to a unit's stealth skill";
					if (ap->mods[i].flags & AttribModItem::PERMAN) {
						*temp += " (note that a unit must possess one ";
						*temp += abbr;
						*temp += " for each man to gain this bonus)";
					}
					*temp += ".";
				}
			}
	}
	ap = FindAttrib("wind");
	if (ap) {
		for (i = 0; i < 5; i++)
			if (ap->mods[i].flags & AttribModItem::ITEM) {
				AString abbr = ItemDefs[item].abr;
				if (abbr == ap->mods[i].ident &&
						ap->mods[i].modtype == AttribModItem::CONSTANT) {
					if (Globals->FLEET_WIND_BOOST > 0) {
						*temp += " The possessor of this item will add ";
						*temp += Globals->FLEET_WIND_BOOST;
						*temp += " movement points to ships requiring up to ";
						*temp += ap->mods[i].val * 12;
						*temp += " sailing skill points.";
						*temp += " This bonus is not cumulative with a mage's summon wind skill.";
					}
				}
			}
	}

	// special descriptions for items whose behaviour isn't fully
	// described by the data tables.  In an ideal world there
	// wouldn't be any of these...
	switch (item) {
		case I_RINGOFI:
			if (!(ItemDefs[I_AMULETOFTS].flags & ItemType::DISABLED)) {
				*temp += " A Ring of Invisibility has one limitation; "
					"a unit possessing a RING cannot assassinate, "
					"nor steal from, a unit with an Amulet of True Seeing.";
			}
			break;
		case I_AMULETOFTS:
			if (!(ItemDefs[I_RINGOFI].flags & ItemType::DISABLED)) {
				*temp += " Also, a unit with an Amulet of True Seeing "
					"cannot be assassinated by, nor have items "
					"stolen by, a unit with a Ring of Invisibility "
					"(note that the unit must have at least one "
					"Amulet of True Seeing per man in order to repel "
					"a unit with a Ring of Invisibility).";
			}
			break;
		case I_PORTAL:
			*temp += " This item is required for mages to use the Portal Lore skill.";
			break;
		case I_STAFFOFH:
			*temp += " This item allows its possessor to magically heal "
				"units after battle, as if their skill in Magical Healing "
				"was the highest of their manipulation, pattern, force and "
				"spirit skills, up to a maximum of level 3.";
			break;
		case I_RELICOFGRACE:
			*temp += " This token was given to you by the Maker in recognition "
				"of you turning away from your rebellion.  Complete more "
				"quests as they are offered if you wish to continue to work "
				"toward restoring your relationship.";
			break;
		case I_HEALPOTION:
			*temp += " This item allows its possessor to heal wounded units after battle."
				" No skill is necessary to use this item; it will be used automatically"
				" when the possessor is involved in a battle. It can heal up to 10"
				" casualties, with a 50 percent success rate. Healing consumes an item.";
			break;
		default:
			break;
	}

	pS = FindSkill(ItemDefs[item].grantSkill);
	if (pS && pS->flags & SkillType::CAST) {
		*temp += " This item allows its possessor to CAST the ";
		*temp += pS->name;
		*temp += " spell as if their skill in ";
		*temp += pS->name;
		*temp += " was ";
		if (ItemDefs[item].minGrant < ItemDefs[item].maxGrant) {
			int count, found;
			count = 0;
			for (i = 0; i < 4; i++) {
				pS = FindSkill(ItemDefs[item].fromSkills[i]);
				if (pS && !(pS->flags & SkillType::DISABLED))
					count++;
			}
			if (count > 1)
				*temp += "the highest of ";
			else
				*temp += "that of ";
			*temp += "their ";
			found = 0;
			for (i = 0; i < 4; i++) {
				pS = FindSkill(ItemDefs[item].fromSkills[i]);
				if (!pS || (pS->flags & SkillType::DISABLED))
					continue;
				if (found > 0) {
					if ((found + 1) == count)
						*temp += " and ";
					else
						*temp += ", ";
				}
				*temp += pS->name;
				found++;
			}
			*temp += " skill";
			if (count > 1)
				*temp += "s";
			*temp += ", up to a maximum of";
		}
		*temp += " level ";
		*temp += ItemDefs[item].maxGrant;
		*temp += ".";
		if (ItemDefs[item].minGrant > 1 &&
				ItemDefs[item].minGrant < ItemDefs[item].maxGrant) {
			*temp += " A skill level of at least ";
			*temp += ItemDefs[item].minGrant;
			*temp += " will always be granted.";
		}
	}

	if (ItemDefs[item].type & IT_BATTLE) {
		*temp += " This item is a miscellaneous combat item.";
		BattleItemType *bt = FindBattleItem(ItemDefs[item].abr);
		if (bt != NULL) {
			if (bt->flags & BattleItemType::MAGEONLY) {
				*temp += " This item may only be used by a mage";
				if (Globals->APPRENTICES_EXIST) {
					*temp += " or an ";
					*temp += Globals->APPRENTICE_NAME;
				}
				*temp += ".";
			}
			if (bt->flags & BattleItemType::SHIELD)
				*temp += AString(" ") + "This item provides ";
			else
				*temp += AString(" ") + "This item can cast ";
			*temp += ShowSpecial(bt->special, bt->skillLevel, 1, bt->flags & BattleItemType::SHIELD);
		}
	} else if (ItemDefs[item].type & IT_MAGEONLY) {
		*temp += " This item may only be used by a mage";
		if (Globals->APPRENTICES_EXIST) {
			*temp += " or an ";
			*temp += Globals->APPRENTICE_NAME;
		}
		*temp += ".";
	}
	if (ItemDefs[item].type & IT_FOOD) {
		*temp += " This item can be eaten to provide ";
		*temp += Globals->UPKEEP_FOOD_VALUE;
		*temp += " silver towards a unit's maintenance cost.";
	}
	if (ItemDefs[item].flags & ItemType::CANTGIVE) {
		*temp += " This item cannot be given to other units.";
	}

	if (ItemDefs[item].max_inventory) {
		*temp += AString(" A unit may have at most ") +
			ItemString(item, ItemDefs[item].max_inventory, FULLNUM) + ".";
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
	selling = 0;
	checked = 0;
}

Item::~Item()
{
}

AString Item::Report(int seeillusions)
{
	AString ret = "";
	// special handling of the unfinished ship items
	if (ItemDefs[type].type & IT_SHIP) {
		ret += AString("unfinished ") + ItemDefs[type].name +
			" [" + ItemDefs[type].abr + "] (needs " + num + ")";
	} else ret += ItemString(type,num);
	if (seeillusions && (ItemDefs[type].type & IT_ILLUSION)) {
		ret = ret + " (illusion)";
	}
	return ret;
}

void Item::Writeout(Aoutfile *f)
{
	AString temp;
	if (type != -1) {
		temp = AString(num) + " ";
		if (ItemDefs[type].type & IT_ILLUSION) temp += "i";
		temp += ItemDefs[type].abr;
	}
	else temp = "-1 NO_ITEM";
	f->PutStr(temp);
}

void Item::Readin(Ainfile *f)
{
	AString *temp = f->GetStr();
	AString *token = temp->gettoken();
	num = token->value();
	delete token;
	token = temp->gettoken();
	type = LookupItem(token);
	delete token;
	delete temp;
}

void ItemList::Writeout(Aoutfile *f)
{
	f->PutInt(Num());
	forlist (this) ((Item *) elem)->Writeout(f);
}

void ItemList::Readin(Ainfile *f)
{
	int i = f->GetInt();
	for (int j=0; j<i; j++) {
		Item *temp = new Item;
		temp->Readin(f);
		if (temp->type < 0 || temp->num < 1 ||
				ItemDefs[temp->type].flags & ItemType::DISABLED)
			delete temp;
		else
			Add(temp);
	}
}

int ItemList::GetNum(int t)
{
	forlist(this) {
		Item *i = (Item *) elem;
		if (i->type == t) return i->num;
	}
	return 0;
}

int ItemList::Weight()
{
	int wt = 0;
	int frac = 0;
	forlist(this) {
		Item *i = (Item *) elem;
		// Except unfinished ships from weight calculations:
		// these just get removed when the unit moves.
		if (ItemDefs[i->type].type & IT_SHIP) continue;
		if (ItemDefs[i->type].weight == 0) frac += i->num;
		else wt += ItemDefs[i->type].weight * i->num;
	}
	if (Globals->FRACTIONAL_WEIGHT > 0 && frac != 0)
		wt += (frac/Globals->FRACTIONAL_WEIGHT);
	return wt;
}

int ItemList::CanSell(int t)
{
	forlist(this) {
		Item *i = (Item *)elem;
		if (i->type == t) return i->num - i->selling;
	}
	return 0;
}

void ItemList::Selling(int t, int n)
{
	forlist(this) {
		Item *i = (Item *)elem;
		if (i->type == t) i->selling += n;
	}
}

void ItemList::UncheckAll()
{
	forlist(this) {
		Item *i = (Item *)elem;
		i->checked = 0;
	}
}

AString ItemList::Report(int obs,int seeillusions,int nofirstcomma)
{
	UncheckAll();
	AString temp;
	for (int s = 0; s < 7; s++) {
		temp += ReportByType(s, obs, seeillusions, nofirstcomma);
		if (temp.Len()) nofirstcomma = 0;
	}
	return temp;
}

AString ItemList::BattleReport()
{
	AString temp;
	forlist(this) {
		Item *i = (Item *) elem;
		if (ItemDefs[i->type].combat) {
			temp += ", ";
			temp += i->Report(0);
			if (ItemDefs[i->type].type & IT_MONSTER) {
				MonType *mp = FindMonster(ItemDefs[i->type].abr,
						(ItemDefs[i->type].type & IT_ILLUSION));
				temp += AString(" (Combat ") + mp->attackLevel +
					"/" + mp->defense[ATTACK_COMBAT] + ", Attacks " +
					mp->numAttacks + ", Hits " + mp->hits +
					", Tactics " + mp->tactics + ")";
			}
		}
	}
	return temp;
}

AString ItemList::ReportByType(int type, int obs, int seeillusions,
		int nofirstcomma)
{
	AString temp;
	forlist(this) {
		int report = 0;
		Item *i = (Item *) elem;
		if (i->checked) continue;
		switch (type) {
			case 0:
				if (ItemDefs[i->type].type & IT_MAN)
					report = 1;
				break;
			case 1:
				if (ItemDefs[i->type].type & IT_MONSTER)
					report = 1;
				break;
			case 2:
				if ((ItemDefs[i->type].type & IT_WEAPON) ||
						(ItemDefs[i->type].type & IT_BATTLE) ||
						(ItemDefs[i->type].type & IT_ARMOR) ||
						(ItemDefs[i->type].type & IT_MAGIC))
					report = 1;
				break;
			case 3:
				if (ItemDefs[i->type].type & IT_MOUNT)
					report = 1;
				break;
			case 4:
				if ((i->type == I_WAGON) || (i->type == I_MWAGON))
					report = 1;
				break;
			case 5:
				report = 1;
				if (ItemDefs[i->type].type & IT_MAN)
					report = 0;
				if (ItemDefs[i->type].type & IT_MONSTER)
					report = 0;
				if (i->type == I_SILVER)
					report = 0;
				if ((ItemDefs[i->type].type & IT_WEAPON) ||
						(ItemDefs[i->type].type & IT_BATTLE) ||
						(ItemDefs[i->type].type & IT_ARMOR) ||
						(ItemDefs[i->type].type & IT_MAGIC))
					report = 0;
				if (ItemDefs[i->type].type & IT_MOUNT)
					report = 0;
				if ((i->type == I_WAGON) ||
						(i->type == I_MWAGON))
					report = 0;
				break;
			case 6:
				if (i->type == I_SILVER)
					report = 1;
		}
		if (report) {
			if (obs == 2) {
				if (nofirstcomma) nofirstcomma = 0;
				else temp += ", ";
				temp += i->Report(seeillusions);
			} else {
				if (ItemDefs[i->type].weight) {
					if (nofirstcomma) nofirstcomma = 0;
					else temp += ", ";
					temp += i->Report(seeillusions);
				}
			}
			i->checked = 1;
		}
	}
	return temp;
}

void ItemList::SetNum(int t,int n)
{
	// sanity check: does this item type exist?
	if ((t<0) || (t>=NITEMS)) return;
	if (n) {
		forlist(this) {
			Item *i = (Item *) elem;
			if (i->type == t) {
				i->num = n;
				return;
			}
		}
		Item *i = new Item;
		i->type = t;
		i->num = n;
		Add(i);
	} else {
		forlist(this) {
			Item *i = (Item *) elem;
			if (i->type == t) {
				Remove(i);
				delete i;
				return;
			}
		}
	}
}

int ManType::CanProduce(int item)
{
	if (ItemDefs[item].flags & ItemType::DISABLED) return 0;
	for (unsigned int i=0; i<(sizeof(skills)/sizeof(skills[0])); i++) {
		if (skills[i] == NULL) continue;
		if (ItemDefs[item].pSkill == skills[i]) return 1;
	}
	return 0;
}

int ManType::CanUse(int item)
{
	if (ItemDefs[item].flags & ItemType::DISABLED) return 0;
	
	// Check if the item is a mount
	if (ItemDefs[item].type & IT_MOUNT) return 1;

	// Check if the item is a weapon
	if (ItemDefs[item].type & IT_WEAPON) return 1;
	
	// Check if the item is a tool
	if (ItemDefs[item].type & IT_TOOL) return 1;
	
	// Check if the item is an armor
	if (ItemDefs[item].type & IT_ARMOR) {
		ArmorType *armor = FindArmor(ItemDefs[item].abr);
		int p = armor->from / armor->saves[3];
		if (p > 4) {
			// puny armor not used by combative races
			int mayWearArmor = 1;
			for (unsigned int i=0; i<(sizeof(skills)/sizeof(skills[0])); i++) {
				if (skills[i] == NULL) continue;
				if (FindSkill(skills[i]) == FindSkill("COMB"))
						mayWearArmor = 0;
			}
			if (mayWearArmor) return 1;
		} else
		if (p > 3) return 1;
		else {
			// heavy armor not be worn by sailors and sneaky races
			int mayWearArmor = 1;
			for (unsigned int i=0; i<(sizeof(skills)/sizeof(skills[0])); i++) {
				if (skills[i] == NULL) continue;
				if ((FindSkill(skills[i]) == FindSkill("SAIL"))
					|| (FindSkill(skills[i]) == FindSkill("HUNT"))
					|| (FindSkill(skills[i]) == FindSkill("STEA"))
					|| (FindSkill(skills[i]) == FindSkill("LBOW")))
						mayWearArmor = 0;
			}
			if (mayWearArmor) return 1;
		}
	}
	
	return 0;
}

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
	AString abbr;
	int i, item, armorType;

	race = r;
	unit = u;
	building = 0;

	healing = 0;
	healtype = 0;
	healitem = -1;
	canbehealed = 1;
	regen = 0;

	armor = -1;
	riding = -1;
	weapon = -1;

	attacks = 1;
	attacktype = ATTACK_COMBAT;

	special = NULL;
	slevel = 0;

	askill = 0;

	dskill[ATTACK_COMBAT] = 0;
	dskill[ATTACK_ENERGY] = -2;
	dskill[ATTACK_SPIRIT] = -2;
	dskill[ATTACK_WEATHER] = -2;
	dskill[ATTACK_RIDING] = 0;
	dskill[ATTACK_RANGED] = 0;
	for (int i=0; i<NUM_ATTACK_TYPES; i++)
		protection[i] = 0;
	damage = 0;
	hits = unit->GetAttribute("toughness");
	if (hits < 1) hits = 1;
	maxhits = hits;
	amuletofi = 0;
	battleItems = 0;

	/* Special case to allow protection from ships */
	if (o->IsFleet() && o->capacity < 1 && o->shipno < o->ships.Num()) {
		int objectno;

		i = 0;
		forlist(&o->ships) {
			Item *ship = (Item *) elem;
			if (o->shipno == i) {
				abbr = ItemDefs[ship->type].name;
				objectno = LookupObject(&abbr);
				if (objectno >= 0 && ObjectDefs[objectno].protect > 0) {
					o->capacity = ObjectDefs[objectno].protect * ship->num;
					o->type = objectno;
				}
				o->shipno++;
			}
			i++;
			if (o->capacity > 0) break;
		}
	}
	/* Building bonus */
	if (o->capacity) {
		building = o->type;
		//should the runes spell be a base or a bonus?
		for (int i=0; i<NUM_ATTACK_TYPES; i++) {
			if (Globals->ADVANCED_FORTS) {
				protection[i] += ObjectDefs[o->type].defenceArray[i];
			} else
				dskill[i] += ObjectDefs[o->type].defenceArray[i];
		}
		if (o->runes) {
			dskill[ATTACK_ENERGY] = max(dskill[ATTACK_ENERGY], o->runes);
			dskill[ATTACK_SPIRIT] = max(dskill[ATTACK_SPIRIT], o->runes);
		}
		o->capacity--;
	}

	/* Is this a monster? */
	if (ItemDefs[r].type & IT_MONSTER) {
		MonType *mp = FindMonster(ItemDefs[r].abr,
				(ItemDefs[r].type & IT_ILLUSION));
		if (u->type == U_WMON)
			name = AString(mp->name) + " in " + *(unit->name);
		else
			name = AString(mp->name) + " controlled by " + *(unit->name);
		askill = mp->attackLevel;
		dskill[ATTACK_COMBAT] += mp->defense[ATTACK_COMBAT];
		if (mp->defense[ATTACK_ENERGY] > dskill[ATTACK_ENERGY]) {
			dskill[ATTACK_ENERGY] = mp->defense[ATTACK_ENERGY];
		}
		if (mp->defense[ATTACK_SPIRIT] > dskill[ATTACK_SPIRIT]) {
			dskill[ATTACK_SPIRIT] = mp->defense[ATTACK_SPIRIT];
		}
		if (mp->defense[ATTACK_WEATHER] > dskill[ATTACK_WEATHER]) {
			dskill[ATTACK_WEATHER] = mp->defense[ATTACK_WEATHER];
		}
		dskill[ATTACK_RIDING] += mp->defense[ATTACK_RIDING];
		dskill[ATTACK_RANGED] += mp->defense[ATTACK_RANGED];
		damage = 0;
		hits = mp->hits;
		if (hits < 1) hits = 1;
		maxhits = hits;
		attacks = mp->numAttacks;
		if (!attacks) attacks = 1;
		special = mp->special;
		slevel = mp->specialLevel;
		if (Globals->MONSTER_BATTLE_REGEN) {
			regen = mp->regen;
			if (regen < 0) regen = 0;
		}
		return;
	}

	name = *(unit->name);

	SetupHealing();

	SetupSpell();
	SetupCombatItems();

	// Set up armor
	for (i = 0; i < MAX_READY; i++) {
		// Check preferred armor first.
		item = unit->readyArmor[i];
		if (item == -1) break;
		abbr = ItemDefs[item].abr;
		item = unit->GetArmor(abbr, ass);
		if (item != -1) {
			armor = item;
			break;
		}
	}
	if (armor == -1) {
		for (armorType = 1; armorType < NUMARMORS; armorType++) {
			abbr = ArmorDefs[armorType].abbr;
			item = unit->GetArmor(abbr, ass);
			if (item != -1) {
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
	int ridingBonus = 0;
	if (canFly || canRide) {
		//
		// Mounts of some type _are_ allowed in this region
		//
		int mountType;
		if (ItemDefs[race].type & IT_MOUNT) {
			// If the man is a mount (Centaurs), then the only option
			// they have for riding is the built-in one
			abbr = ItemDefs[race].abr;
			item = unit->GetMount(abbr, canFly, canRide, ridingBonus);
		} else {
			for (mountType = 1; mountType < NUMMOUNTS; mountType++) {
				abbr = MountDefs[mountType].abbr;
				// See if this mount is an option
				item = unit->GetMount(abbr, canFly, canRide, ridingBonus);
				if (item == -1) continue;
				// No riding other men in combat
				if (ItemDefs[item].type & IT_MAN) {
					item = -1;
					ridingBonus = 0;
					continue;
				}
				break;
			}
		}
		// Defer adding the combat bonus until we know if the weapon
		// allows it.  The defense bonus for riding can be added now
		// however.
		dskill[ATTACK_RIDING] += ridingBonus;
		riding = item;
	}

	//
	// Find the correct weapon for this soldier.
	//
	int weaponType;
	int attackBonus = 0;
	int defenseBonus = 0;
	int numAttacks = 1;
	for (i = 0; i < MAX_READY; i++) {
		// Check the preferred weapon first.
		item = unit->readyWeapon[i];
		if (item == -1) break;
		abbr = ItemDefs[item].abr;
		item = unit->GetWeapon(abbr, riding, ridingBonus, attackBonus,
				defenseBonus, numAttacks);
		if (item != -1) {
			weapon = item;
			break;
		}
	}
	if (weapon == -1) {
		for (weaponType = 1; weaponType < NUMWEAPONS; weaponType++) {
			abbr = WeaponDefs[weaponType].abbr;
			item = unit->GetWeapon(abbr, riding, ridingBonus, attackBonus,
					defenseBonus, numAttacks);
			if (item != -1) {
				weapon = item;
				break;
			}
		}
	}
	// If we did not get a weapon, set attack and defense bonuses to
	// combat skill (and riding bonus if applicable).
	if (weapon == -1) {
		attackBonus = unit->GetAttribute("combat") + ridingBonus;
		defenseBonus = attackBonus;
		numAttacks = 1;
	} else {
		// Okay.  We got a weapon.  If this weapon also has a special
		// and we don't have a special set, use that special.
		// Weapons (like Runeswords) which are both weapons and battle
		// items will be skipped in the battle items setup and handled
		// here.
		if ((ItemDefs[weapon].type & IT_BATTLE) && special == NULL) {
			BattleItemType *pBat = FindBattleItem(ItemDefs[weapon].abr);
			special = pBat->special;
			slevel = pBat->skillLevel;
		}
	}

	unit->PracticeAttribute("combat");

	// Set the attack and defense skills
	// These will include the riding bonus if they should be included.
	askill += attackBonus;
	dskill[ATTACK_COMBAT] += defenseBonus;
	attacks = numAttacks;
}

void Soldier::SetupSpell()
{
	if (unit->type != U_MAGE && unit->type != U_GUARDMAGE) return;

	if (unit->combat != -1) {
		slevel = unit->GetSkill(unit->combat);
		if (!slevel) {
			//
			// The unit can't cast this spell!
			//
			unit->combat = -1;
			return;
		}

		SkillType *pST = &SkillDefs[unit->combat];
		if (!(pST->flags & SkillType::COMBAT)) {
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
	int exclusive = 0;

	for (battleType = 1; battleType < NUMBATTLEITEMS; battleType++) {
		BattleItemType *pBat = &BattleItemDefs[battleType];

		AString abbr = pBat->abbr;
		int item = unit->GetBattleItem(abbr);
		if (item == -1) continue;

		// If we are using the ready command, skip this item unless
		// it's the right one, or unless it is a shield which doesn't
		// need preparing.
		if (!Globals->USE_PREPARE_COMMAND ||
				((unit->readyItem == -1) &&
				 (Globals->USE_PREPARE_COMMAND == GameDefs::PREPARE_NORMAL)) ||
				(item == unit->readyItem) ||
				(pBat->flags & BattleItemType::SHIELD)) {
			if ((pBat->flags & BattleItemType::SPECIAL) && special != NULL) {
				// This unit already has a special attack so give the item
				// back to the unit as they aren't going to use it.
				unit->items.SetNum(item, unit->items.GetNum(item)+1);
				continue;
			}
			if (pBat->flags & BattleItemType::MAGEONLY &&
					unit->type != U_MAGE && unit->type != U_GUARDMAGE &&
					unit->type != U_APPRENTICE) {
				// Only mages/apprentices can use this item so give the
				// item back to the unit as they aren't going to use it.
				unit->items.SetNum(item, unit->items.GetNum(item)+1);
				continue;
			}

			if (pBat->flags & BattleItemType::EXCLUSIVE) {
				if (exclusive) {
					// Can only use one exclusive item, and we already
					// have one, so give the extras back.
					unit->items.SetNum(item, unit->items.GetNum(item)+1);
					continue;
				}
				exclusive = 1;
			}

			if (pBat->flags & BattleItemType::MAGEONLY) {
				unit->Practice(S_MANIPULATE);
			}

			/* Make sure amulets of invulnerability are marked */
			if (item == I_AMULETOFI) {
				amuletofi = 1;
			}

			SET_BIT(battleItems, battleType);

			if (pBat->flags & BattleItemType::SPECIAL) {
				special = pBat->special;
				slevel = pBat->skillLevel;
			}

			if (pBat->flags & BattleItemType::SHIELD) {
				SpecialType *sp = FindSpecial(pBat->special);
				for (int i = 0; i < 4; i++) {
					if (sp->shield[i] == NUM_ATTACK_TYPES) {
						for (int j = 0; j < NUM_ATTACK_TYPES; j++) {
							if (dskill[j] < pBat->skillLevel)
								dskill[j] = pBat->skillLevel;
						}
					} else if (sp->shield[i] >= 0) {
						if (dskill[sp->shield[i]] < pBat->skillLevel)
							dskill[sp->shield[i]] = pBat->skillLevel;
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

int Soldier::HasEffect(char const *eff)
{
	if (eff == NULL) return 0;

	return effects[eff];
}

void Soldier::SetEffect(char const *eff)
{
	if (eff == NULL) return;
	int i;

	EffectType *e = FindEffect(eff);
	if (e == NULL) return;

	askill += e->attackVal;

	for (i = 0; i < 4; i++) {
		if (e->defMods[i].type != -1)
			dskill[e->defMods[i].type] += e->defMods[i].val;
	}

	if (e->cancelEffect != NULL) ClearEffect(e->cancelEffect);

	if (!(e->flags & EffectType::EFF_NOSET)) effects[eff] = 1;
}

void Soldier::ClearEffect(char const *eff)
{
	if (eff == NULL) return;
	int i;

	EffectType *e = FindEffect(eff);
	if (e == NULL) return;

	askill -= e->attackVal;

	for (i = 0; i < 4; i++) {
		if (e->defMods[i].type != -1)
			dskill[e->defMods[i].type] -= e->defMods[i].val;
	}

	effects[eff] = 0;
}

void Soldier::ClearOneTimeEffects(void)
{
	for (int i = 0; i < NUMEFFECTS; i++) {
		if (HasEffect(EffectDefs[i].name) &&
				(EffectDefs[i].flags & EffectType::EFF_ONESHOT))
			ClearEffect(EffectDefs[i].name);
	}
}

int Soldier::ArmorProtect(int weaponClass)
{
	//
	// Return 1 if the armor is successful
	//
	ArmorType *pArm = NULL;
	if (armor > 0) pArm = FindArmor(ItemDefs[armor].abr);
	if (pArm == NULL) return 0;
	int chance = pArm->saves[weaponClass];

	if (chance <= 0) return 0;
	if (chance > getrandom(pArm->from)) return 1;

	return 0;
}

void Soldier::RestoreItems()
{
	if (healing && healitem != -1) {
		if (healitem == I_HERBS) {
			unit->items.SetNum(healitem,
					unit->items.GetNum(healitem) + healing);
		} else if (healitem == I_HEALPOTION) {
			unit->items.SetNum(healitem,
					unit->items.GetNum(healitem)+healing/10);
		}
	}
	if (weapon != -1)
		unit->items.SetNum(weapon,unit->items.GetNum(weapon) + 1);
	if (armor != -1)
		unit->items.SetNum(armor,unit->items.GetNum(armor) + 1);
	if (riding != -1 && !(ItemDefs[riding].type & IT_MAN))
		unit->items.SetNum(riding,unit->items.GetNum(riding) + 1);

	int battleType;
	for (battleType = 1; battleType < NUMBATTLEITEMS; battleType++) {
		BattleItemType *pBat = &BattleItemDefs[ battleType ];

		if (GET_BIT(battleItems, battleType)) {
			AString itm(pBat->abbr);
			int item = LookupItem(&itm);
			unit->items.SetNum(item, unit->items.GetNum(item) + 1);
		}
	}
}

void Soldier::Alive(int state)
{
	RestoreItems();

	if (state == LOSS) {
		unit->canattack = 0;
		unit->routed = 1;
		/* Guards with amuletofi will not go off guard */
		if (!amuletofi &&
			(unit->guard == GUARD_GUARD || unit->guard == GUARD_SET)) {
			unit->guard = GUARD_NONE;
		}
	} else {
		unit->advancefrom = 0;
	}

	if (state == WIN_DEAD) {
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
	int tacspell = 0;
	Unit * tactician = ldr;

	leader = ldr;
	round = 0;
	tac = ldr->GetAttribute("tactics");
	count = 0;
	hitstotal = 0;
	tactics_bonus = 0;

	if (ass) {
		count = 1;
		ldr->losses = 0;
	} else {
		forlist(locs) {
			Unit * u = ((Location *) elem)->unit;
			count += u->GetSoldiers();
			u->losses = 0;
			int temp = u->GetAttribute("tactics");
			if (temp > tac) {
				tac = temp;
				tactician = u;
			}
		}
	}
	// If TACTICS_NEEDS_WAR is enabled, we don't want to push leaders 
	// from tact-4 to tact-5! Also check that we have skills, otherwise
	// we get a nasty core dump ;)
	if (Globals->TACTICS_NEEDS_WAR && (tactician->skills.Num() != 0)) {
		int currskill = tactician->skills.GetDays(S_TACTICS)/tactician->GetMen();
		if (currskill < 450 - Globals->SKILL_PRACTICE_AMOUNT) {
			tactician->PracticeAttribute("tactics");
		}
	} else { // Only Globals->TACTICS_NEEDS_WAR == 0
		tactician->PracticeAttribute("tactics");
	}
	soldiers = new SoldierPtr[count];
	int x = 0;
	int y = count;

	forlist(locs) {
		Unit * u = ((Location *) elem)->unit;
		Object * obj = ((Location *) elem)->obj;
		if (ass) {
			forlist(&u->items) {
				Item * it = (Item *) elem;
				if (it) {
					if (ItemDefs[ it->type ].type & IT_MAN) {
							soldiers[x] = new Soldier(u, obj, regtype,
									it->type, ass);
							hitstotal = soldiers[x]->hits;
							++x;
							goto finished_army;
					}
				}
			}
		} else {
			Item *it = (Item *) u->items.First();
			do {
				if (IsSoldier(it->type)) {
					for (int i = 0; i < it->num; i++) {
						if ((ItemDefs[ it->type ].type & IT_MAN) &&
								u->GetFlag(FLAG_BEHIND)) {
							--y;
							soldiers[y] = new Soldier(u, obj, regtype,
									it->type);
							hitstotal += soldiers[y]->hits;
						} else {
							soldiers[x] = new Soldier(u, obj, regtype,
									it->type);
							hitstotal += soldiers[x]->hits;
							++x;
						}
					}
				}
				it = (Item *) u->items.Next(it);
			} while(it);
		}
	}

finished_army:
	tac = tac + tacspell;

	canfront = x;
	canbehind = count;
	notfront = count;
	notbehind = count;

	hitsalive = hitstotal;

	if (!NumFront()) {
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
	tactics_bonus = 0;
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

void Army::GetMonSpoils(ItemList *spoils,int monitem, int free)
{
	if ((Globals->MONSTER_NO_SPOILS > 0) &&
			(free >= Globals->MONSTER_SPOILS_RECOVERY)) {
		// This monster is in it's period of absolutely no spoils.
		return;
	}

	/* First, silver */
	MonType *mp = FindMonster(ItemDefs[monitem].abr,
			(ItemDefs[monitem].type & IT_ILLUSION));
	int silv = mp->silver;
	if ((Globals->MONSTER_NO_SPOILS > 0) && (free > 0)) {
		// Adjust the spoils for length of freedom.
		silv *= (Globals->MONSTER_SPOILS_RECOVERY-free);
		silv /= Globals->MONSTER_SPOILS_RECOVERY;
	}
	spoils->SetNum(I_SILVER,spoils->GetNum(I_SILVER) + getrandom(silv));

	int thespoil = mp->spoiltype;

	if (thespoil == -1) return;
	if (thespoil == IT_NORMAL && getrandom(2)) thespoil = IT_TRADE;

	int count = 0;
	int i;
	for (i=0; i<NITEMS; i++) {
		if ((ItemDefs[i].type & thespoil) &&
				!(ItemDefs[i].type & IT_SPECIAL) &&
				!(ItemDefs[i].type & IT_SHIP) &&
				(ItemDefs[i].baseprice <= mp->silver) &&
				!(ItemDefs[i].flags & ItemType::DISABLED)) {
			count ++;
		}
	}
	if (count == 0) return;
	count = getrandom(count) + 1;

	for (i=0; i<NITEMS; i++) {
		if ((ItemDefs[i].type & thespoil) &&
				!(ItemDefs[i].type & IT_SPECIAL) &&
				!(ItemDefs[i].type & IT_SHIP) &&
				(ItemDefs[i].baseprice <= mp->silver) &&
				!(ItemDefs[i].flags & ItemType::DISABLED)) {
			count--;
			if (count == 0) {
				thespoil = i;
				break;
			}
		}
	}

	int val = getrandom(mp->silver * 2);
	if ((Globals->MONSTER_NO_SPOILS > 0) && (free > 0)) {
		// Adjust for length of monster freedom.
		val *= (Globals->MONSTER_SPOILS_RECOVERY-free);
		val /= Globals->MONSTER_SPOILS_RECOVERY;
	}

	spoils->SetNum(thespoil,spoils->GetNum(thespoil) +
			(val + getrandom(ItemDefs[thespoil].baseprice)) /
			ItemDefs[thespoil].baseprice);
}

void Army::Regenerate(Battle *b)
{
	for (int i = 0; i < count; i++) {
		Soldier *s = soldiers[i];
		if (i<notbehind) {
			int diff = s->maxhits - s->hits;
			if (diff > 0) {
				AString aName = s->name;

				if (s->damage != 0) {
					b->AddLine(aName + " takes " + s->damage +
							" hits bringing it to " + s->hits + "/" +
							s->maxhits + ".");
					s->damage = 0;
				} else {
					b->AddLine(aName + " takes no hits leaving it at " +
							s->hits + "/" + s->maxhits + ".");
				}
				if (s->regen) {
					int regen = s->regen;
					if (regen > diff) regen = diff;
					s->hits += regen;
					b->AddLine(aName + " regenerates " + regen +
							" hits bringing it to " + s->hits + "/" +
							s->maxhits + ".");
				}
			}
		}
	}
}

void Army::Lose(Battle *b,ItemList *spoils)
{
	WriteLosses(b);
	for (int i=0; i<count; i++) {
		Soldier * s = soldiers[i];
		if (i<notbehind) {
			s->Alive(LOSS);
		} else {
			if ((s->unit->type==U_WMON) && (ItemDefs[s->race].type&IT_MONSTER))
				GetMonSpoils(spoils,s->race,s->unit->free);
			s->Dead();
		}
		delete s;
	}
}

void Army::Tie(Battle * b)
{
	WriteLosses(b);
	for (int x=0; x<count; x++) {
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
	for (int i=notbehind; i<count; i++) {
		Soldier * temp = soldiers[i];
		if (temp->canbehealed) return 1;
	}
	return 0;
}

void Army::DoHeal(Battle * b)
{
	// Do magical healing
	for (int i = 5; i > 0; --i)
		DoHealLevel(b, i, 0);
	// Do Normal healing
	DoHealLevel(b, 1, 1);
}

void Army::DoHealLevel(Battle *b, int type, int useItems)
{
	int rate = HealDefs[type].rate;

	for (int i=0; i<NumAlive(); i++) {
		Soldier * s = soldiers[i];
		int n = 0;
		if (!CanBeHealed()) break;
		if (s->healtype <= 0) continue;
		// This should be here.. Use the best healing first
		if (s->healtype != type) continue;
		if (!s->healing) continue;
		if (useItems) {
			if (s->healitem == -1) continue;
			if (s->healitem != I_HEALPOTION) s->unit->Practice(S_HEALING);
		} else {
			if (s->healitem != -1) continue;
			s->unit->Practice(S_MAGICAL_HEALING);
		}

		while (s->healing) {
			if (!CanBeHealed()) break;
			int j = getrandom(count - NumAlive()) + notbehind;
			Soldier * temp = soldiers[j];
			if (temp->canbehealed) {
				s->healing--;
				if (getrandom(100) < rate) {
					n++;
					soldiers[j] = soldiers[notbehind];
					soldiers[notbehind] = temp;
					notbehind++;
				} else
					temp->canbehealed = 0;
			}
		}
		b->AddLine(*(s->unit->name) + " heals " + n + ".");
	}
}

void Army::Win(Battle * b,ItemList * spoils)
{
	int wintype;

	DoHeal(b);

	WriteLosses(b);

	int na = NumAlive();

	if (count - na) wintype = WIN_DEAD;
	else wintype = WIN_NO_DEAD;

	AList units;

	for (int x = 0; x < count; x++) {
		Soldier * s = soldiers[x];
		if (x<NumAlive()) s->Alive(wintype);
		else s->Dead();
	}

	forlist(spoils) {
		Item *i = (Item *) elem;
		if (i && na) {
			Unit *u;
			UnitPtr *up;
			int ns;

			do {
				units.DeleteAll();
				// Make a list of units who can get this type of spoil
				for (int x = 0; x < na; x++) {
					u = soldiers[x]->unit;
					if (u->CanGetSpoil(i)) {
						up = new UnitPtr;
						up->ptr = u;
						units.Add(up);
					}
				}

				ns = units.Num();
				if (ItemDefs[i->type].type & IT_SHIP) {
					int t = getrandom(ns);
					up = (UnitPtr *) units.First();
					while (t-- > 0)
						up = (UnitPtr *) up->next;
					if (up && up->ptr->CanGetSpoil(i)) {
						up->ptr->items.SetNum(i->type, i->num);
						up->ptr->faction->DiscoverItem(i->type, 0, 1);
						i->num = 0;
					}
					break;
				}
				while (ns > 0 && i->num >= ns) {
					int chunk = 1;
					if (!ItemDefs[i->type].weight) {
						chunk = i->num / ns;
					}
					forlist(&units) {
						up = (UnitPtr *) elem;
						if (up->ptr->CanGetSpoil(i)) {
							up->ptr->items.SetNum(i->type,
									up->ptr->items.GetNum(i->type) + chunk);
							up->ptr->faction->DiscoverItem(i->type, 0, 1);
							i->num -= chunk;
						} else {
							units.Remove(up);
							ns--;
						}
					}
				}
				while (ns > 0 && i->num > 0) {
					int t = getrandom(ns);
					up = (UnitPtr *)units.First();
					while (t-- > 0)
						up = (UnitPtr *) up->next;
					if (up && up->ptr->CanGetSpoil(i)) {
						up->ptr->items.SetNum(i->type,
								up->ptr->items.GetNum(i->type) + 1);
						up->ptr->faction->DiscoverItem(i->type, 0, 1);
						i->num--;
					} else {
						units.Remove(up);
						ns--;
					}
				}
				units.DeleteAll();
			} while (ns > 0 && i->num > 0);
		}
	}

	for (int x = 0; x < count; x++) {
		Soldier * s = soldiers[x];
		delete s;
	}
}

int Army::Broken()
{
	if (Globals->ARMY_ROUT == GameDefs::ARMY_ROUT_FIGURES) {
		if ((NumAlive() << 1) < count) return 1;
	} else {
		if ((hitsalive << 1) < hitstotal) return 1;
	}
	return 0;
}

int Army::NumSpoilers()
{
	int na = NumAlive();
	int count = 0;
	for (int x=0; x<na; x++) {
		Unit * u = soldiers[x]->unit;
		if (!(u->flags & FLAG_NOSPOILS)) count++;
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

int Army::GetTargetNum(char const *special)
{
	int tars = NumFront();
	if (tars == 0) {
		canfront = canbehind;
		notfront = notbehind;
		tars = NumFront();
		if (tars == 0) return -1;
	}

	SpecialType *sp = FindSpecial(special);

	if (sp && sp->targflags) {
		int validtargs = 0;
		int i, start = -1;

		for (i = 0; i < canfront; i++) {
			if (CheckSpecialTarget(special, i)) {
				validtargs++;
				// slight scan optimisation - skip empty initial sequences
				if (start == -1) start = i;
			}
		}
		for (i = canbehind; i < notfront; i++) {
			if (CheckSpecialTarget(special, i)) {
				validtargs++;
				// slight scan optimisation - skip empty initial sequences
				if (start == -1) start = i;
			}
		}
		if (validtargs) {
			int targ = getrandom(validtargs);
			for (i = start; i < notfront; i++) {
				if (i == canfront) i = canbehind;
				if (CheckSpecialTarget(special, i)) {
					if (!targ--) return i;
				}
			}
		}
	} else {
		int i = getrandom(tars);
		if (i<canfront) return i;
		return i + canbehind - canfront;
	}

	return -1;
}

int Army::GetEffectNum(char const *effect)
{
	int validtargs = 0;
	int i, start = -1;

	for (i = 0; i < canfront; i++) {
		if (soldiers[i]->HasEffect(effect)) {
			validtargs++;
			// slight scan optimisation - skip empty initial sequences
			if (start == -1) start = i;
		}
	}
	for (i = canbehind; i < notfront; i++) {
		if (soldiers[i]->HasEffect(effect)) {
			validtargs++;
			// slight scan optimisation - skip empty initial sequences
			if (start == -1) start = i;
		}
	}
	if (validtargs) {
		int targ = getrandom(validtargs);
		for (i = start; i < notfront; i++) {
			if (i == canfront) i = canbehind;
			if (soldiers[i]->HasEffect(effect)) {
				if (!targ--) return i;
			}
		}
	}
	return -1;
}

Soldier * Army::GetTarget(int i)
{
	return soldiers[i];
}

int pow(int b,int p)
{
	int b2 = b;
	for (int i=1; i<p; i++) {
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

int Army::RemoveEffects(int num, char const *effect)
{
	int ret = 0;
	for (int i = 0; i < num; i++) {
		//
		// Try to find a target unit.
		//
		int tarnum = GetEffectNum(effect);
		if (tarnum == -1) continue;
		Soldier *tar = GetTarget(tarnum);

		//
		// Remove the effect
		//
		tar->ClearEffect(effect);
		ret++;
	}
	return(ret);
}

int Army::DoAnAttack(Battle * b, char const *special, int numAttacks, int attackType,
		int attackLevel, int flags, int weaponClass, char const *effect,
		int mountBonus, Soldier *attacker, Army *attackers)
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

	if (canShield) {
		int shieldType = attackType;

		hi = shields.GetHighShield(shieldType);
		if (hi) {
			/* Check if we get through shield */
			if (!Hits(attackLevel, hi->shieldskill)) {
				return -1;
			}

			if (effect != NULL && !combat) {
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
	for (int i = 0; i < numAttacks; i++) {
		/* 3. Get the target */
		int tarnum = GetTargetNum(special);
		if (tarnum == -1) continue;
		Soldier * tar = GetTarget(tarnum);
		int tarFlags = 0;
		if (tar->weapon != -1) {
			WeaponType *pw = FindWeapon(ItemDefs[tar->weapon].abr);
			tarFlags = pw->flags;
		}

		/* 4. Add in any effects, if applicable */
		int tlev = 0;
		if (attackType != NUM_ATTACK_TYPES)
			tlev = tar->dskill[ attackType ];
		if (special != NULL) {
			SpecialType *sp = FindSpecial(special);
			if ((sp->effectflags & SpecialType::FX_NOBUILDING) && tar->building)
				tlev -= 2;
		}

		/* 4.1 Check whether defense is allowed against this weapon */
		if ((flags & WeaponType::NODEFENSE) && (tlev > 0)) tlev = 0;

		if (!(flags & WeaponType::RANGED)) {
			/* 4.2 Check relative weapon length */
			int attLen = 1;
			int defLen = 1;
			if (flags & WeaponType::LONG) attLen = 2;
			else if (flags & WeaponType::SHORT) attLen = 0;
			if (tarFlags & WeaponType::LONG) defLen = 2;
			else if (tarFlags & WeaponType::SHORT) defLen = 0;
			if (attLen > defLen) attackLevel++;
			else if (defLen > attLen) tlev++;
		}

		/* 4.3 Add bonuses versus mounted */
		if (tar->riding != -1) attackLevel += mountBonus;

		// TODO: debug only, remove later
		// b->AddLine(attacker->name + AString(" attack level: ") + attackLevel + "(+" + attackers->tactics_bonus + ").");
		// b->AddLine(tar->name + AString(" defence level: ") + tlev + "(+" + tactics_bonus + ").");

		/* 5. Attack soldier */
		if (attackType != NUM_ATTACK_TYPES) {
			if (!(flags & WeaponType::ALWAYSREADY)) {
				int failchance = 2;
				if (Globals->ADVANCED_FORTS) {
					failchance += (tar->protection[attackType]+1)/2;
				}
				if (getrandom(failchance)) {
					continue;
				}
			}

			/* 4.4 Add advanced tactics bonus */
			if (!Hits(attackLevel + attackers->tactics_bonus, tlev + tactics_bonus)) {
				continue;
			}
		}

		/* 6. If attack got through, apply effect, or kill */
		if (effect == NULL) {
			/* 7. Last chance... Check armor */
			if (tar->ArmorProtect(weaponClass)) {
				continue;
			}

			/* 8. Seeya! */
			Kill(tarnum);
			ret++;
			if ((ItemDefs[tar->race].type & IT_MAN) &&
				(ItemDefs[attacker->race].type & IT_UNDEAD)) {
				if (getrandom(100) < Globals->UNDEATH_CONTAGION) {
					attacker->unit->raised++;
					tar->canbehealed = 0;
				}
			}
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
	Soldier *temp = soldiers[killed];

	if (temp->amuletofi) return;

	if (Globals->ARMY_ROUT == GameDefs::ARMY_ROUT_HITS_INDIVIDUAL)
		hitsalive--;
	temp->hits--;
	temp->damage++;
	if (temp->hits > 0) return;
	temp->unit->losses++;
	if (Globals->ARMY_ROUT == GameDefs::ARMY_ROUT_HITS_FIGURE) {
		if (ItemDefs[temp->race].type & IT_MONSTER) {
			MonType *mp = FindMonster(ItemDefs[temp->race].abr,
					(ItemDefs[temp->race].type & IT_ILLUSION));
			hitsalive -= mp->hits;
		} else {
			// Assume everything that is a solder and isn't a monster is a
			// man.
			hitsalive--;
		}
	}

	if (killed < canfront) {
		soldiers[killed] = soldiers[canfront-1];
		soldiers[canfront-1] = temp;
		killed = canfront - 1;
		canfront--;
	}

	if (killed < canbehind) {
		soldiers[killed] = soldiers[canbehind-1];
		soldiers[canbehind-1] = temp;
		killed = canbehind-1;
		canbehind--;
	}

	if (killed < notfront) {
		soldiers[killed] = soldiers[notfront-1];
		soldiers[notfront-1] = temp;
		killed = notfront-1;
		notfront--;
	}

	soldiers[killed] = soldiers[notbehind-1];
	soldiers[notbehind-1] = temp;
	notbehind--;
}

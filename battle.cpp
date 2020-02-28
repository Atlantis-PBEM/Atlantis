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
#include "game.h"
#include "battle.h"
#include "army.h"
#include "gamedefs.h"
#include "gamedata.h"
#include "quests.h"

Battle::Battle()
{
	asstext = 0;
}

Battle::~Battle()
{
	if (asstext)
	{
		delete asstext;
	}
}

void Battle::FreeRound(Army * att,Army * def, int ass)
{
	/* Write header */
	AddLine(*(att->leader->name) + " gets a free round of attacks.");

	/* Update both army's shields */
	att->shields.DeleteAll();
	UpdateShields(att);

	def->shields.DeleteAll();
	UpdateShields(def);

	//
	// Update the attacking armies round counter
	//
	att->round++;

	/* Run attacks until done */
	int alv = def->NumAlive();
	while (att->CanAttack() && def->NumAlive()) {
		int num = getrandom(att->CanAttack());
		int behind;
		Soldier * a = att->GetAttacker(num, behind);
		DoAttack(att->round, a, att, def, behind, ass);
	}

	/* Write losses */
	def->Regenerate(this);
	alv -= def->NumAlive();
	AddLine(*(def->leader->name) + " loses " + alv + ".");
	AddLine("");
	att->Reset();
}

void Battle::DoAttack(int round, Soldier *a, Army *attackers, Army *def,
		int behind, int ass)
{
	DoSpecialAttack(round, a, attackers, def, behind);
	if (!def->NumAlive()) return;

	if (!behind && (a->riding != -1)) {
		MountType *pMt = FindMount(ItemDefs[a->riding].abr);
		if (pMt->mountSpecial != NULL) {
			int i, num, tot = -1;
			SpecialType *spd = FindSpecial(pMt->mountSpecial);
			for (i = 0; i < 4; i++) {
				int times = spd->damage[i].value;
				if (spd->effectflags & SpecialType::FX_USE_LEV)
					times *= pMt->specialLev;
				int realtimes = spd->damage[i].minnum + getrandom(times) +
					getrandom(times);
				num = def->DoAnAttack(this, pMt->mountSpecial, realtimes,
						spd->damage[i].type, pMt->specialLev,
						spd->damage[i].flags, spd->damage[i].dclass,
						spd->damage[i].effect, 0, a, attackers);
				if (num != -1) {
					if (tot == -1) tot = num;
					else tot += num;
				}
			}
			if (tot != -1) {
				AddLine(a->name + " " + spd->spelldesc + ", " +
						spd->spelldesc2 + tot + spd->spelltarget + ".");
			}
		}
	}
	if (!def->NumAlive()) return;

	int numAttacks = a->attacks;
	if (a->attacks < 0) {
		if (round % ( -1 * a->attacks ) == 1)
			numAttacks = 1;
		else
			numAttacks = 0;
	} else if (ass && (Globals->MAX_ASSASSIN_FREE_ATTACKS > 0) &&
			(numAttacks > Globals->MAX_ASSASSIN_FREE_ATTACKS)) {
		numAttacks = Globals->MAX_ASSASSIN_FREE_ATTACKS;
	}

	for (int i = 0; i < numAttacks; i++) {
		WeaponType *pWep = NULL;
		if (a->weapon != -1)
			pWep = FindWeapon(ItemDefs[a->weapon].abr);

		if (behind) {
			if (!pWep) break;
			if (!( pWep->flags & WeaponType::RANGED)) break;
		}

		int flags = 0;
		int attackType = ATTACK_COMBAT;
		int mountBonus = 0;
		int attackClass = SLASHING;
		if (pWep) {
			flags = pWep->flags;
			attackType = pWep->attackType;
			mountBonus = pWep->mountBonus;
			attackClass = pWep->weapClass;
		}
		//
		def->DoAnAttack(this, NULL, 1, attackType, a->askill, flags, attackClass,
				NULL, mountBonus, a, attackers);
		if (!def->NumAlive()) break;
	}

	a->ClearOneTimeEffects();
}

void Battle::NormalRound(int round,Army * a,Army * b)
{
	/* Write round header */
	AddLine(AString("Round ") + round + ":");

	if (a->tactics_bonus > b->tactics_bonus) {
		AddLine(*(a->leader->name) + " tactics bonus " + a->tactics_bonus + ".");	
	}
	if (b->tactics_bonus > a->tactics_bonus) {
		AddLine(*(b->leader->name) + " tactics bonus " + b->tactics_bonus + ".");	
	}

	/* Update both army's shields */
	UpdateShields(a);
	UpdateShields(b);

	/* Initialize variables */
	a->round++;
	b->round++;
	int aalive = a->NumAlive();
	int aialive = aalive;
	int balive = b->NumAlive();
	int bialive = balive;
	int aatt = a->CanAttack();
	int batt = b->CanAttack();

	/* Run attacks until done */
	while (aalive && balive && (aatt || batt))
	{
		int num = getrandom(aatt + batt);
		int behind;
		if (num >= aatt)
		{
			num -= aatt;
			Soldier * s = b->GetAttacker(num, behind);
			DoAttack(b->round, s, b, a, behind);
		}
		else
		{
			Soldier * s = a->GetAttacker(num, behind);
			DoAttack(a->round, s, a, b, behind);
			// ---
		}
		aalive = a->NumAlive();
		balive = b->NumAlive();
		aatt = a->CanAttack();
		batt = b->CanAttack();
	}

	/* Finish round */
	a->Regenerate(this);
	b->Regenerate(this);
	aialive -= aalive;
	AddLine(*(a->leader->name) + " loses " + aialive + ".");
	bialive -= balive;
	AddLine(*(b->leader->name) + " loses " + bialive + ".");
	AddLine("");
	a->Reset();
	b->Reset();
}

void Battle::GetSpoils(AList *losers, ItemList *spoils, int ass)
{
	ItemList *ships = new ItemList;
	AString quest_rewards;

	forlist(losers) {
		Unit * u = ((Location *) elem)->unit;
		int numalive = u->GetSoldiers();
		int numdead = u->losses;
		if (!numalive) {
			if (quests.CheckQuestKillTarget(u, spoils, &quest_rewards)) {
				AddLine(AString("Quest completed! ") + quest_rewards);
			}
		}
		forlist(&u->items) {
			Item * i = (Item *) elem;
			if (IsSoldier(i->type)) continue;
			// ignore incomplete ships
			if (ItemDefs[i->type].type & IT_SHIP) continue;
			// New rule:  Assassins with RINGS cannot get AMTS in spoils
			// This rule is only meaningful with Proportional AMTS usage
			// is enabled, otherwise it has no effect.
			if ((ass == 2) && (i->type == I_AMULETOFTS)) continue;
			float percent = (float)numdead/(float)(numalive+numdead);
			// incomplete ships:
			if (ItemDefs[i->type].type & IT_SHIP) {
				if (getrandom(100) < percent) {
					u->items.SetNum(i->type, 0);
					if (i->num < ships->GetNum(i->type))
						ships->SetNum(i->type, i->num);
				}
			} else {
				int num = (int)(i->num * percent);
				int num2 = (num + getrandom(2))/2;
				if (ItemDefs[i->type].type & IT_ALWAYS_SPOIL) {
					num2 = num;
				}
				if (ItemDefs[i->type].type & IT_NEVER_SPOIL) {
					num2 = 0;
				}
				spoils->SetNum(i->type, spoils->GetNum(i->type) + num2);
				u->items.SetNum(i->type, i->num - num);
			}
		}
	}
	// add incomplete ships to spoils...
	for (int sh = 0; sh < NITEMS; sh++) {
		if (ItemDefs[sh].type & IT_SHIP) {
			spoils->SetNum(sh, ships->GetNum(sh));
		}
	}
}

int Battle::Run( ARegion * region,
		Unit * att,
		AList * atts,
		Unit * tar,
		AList * defs,
		int ass,
		ARegionList *pRegs )
{
	Army * armies[2];
	AString temp;
	assassination = ASS_NONE;
	attacker = att->faction;

	armies[0] = new Army(att,atts,region->type,ass);
	armies[1] = new Army(tar,defs,region->type,ass);

	if (ass) {
		FreeRound(armies[0],armies[1], ass);
	} else {
		if (Globals->ADVANCED_TACTICS) {
			int tactics_bonus = 0;
			if (armies[0]->tac > armies[1]->tac) {
				tactics_bonus = armies[0]->tac - armies[1]->tac;
				if (tactics_bonus > 3) tactics_bonus = 3;
				armies[0]->tactics_bonus = tactics_bonus;
			}
			if (armies[1]->tac > armies[0]->tac) {
				tactics_bonus = armies[1]->tac - armies[0]->tac;
				if (tactics_bonus > 3) tactics_bonus = 3;
				armies[1]->tactics_bonus = tactics_bonus;
			}
		} else {
			if (armies[0]->tac > armies[1]->tac) FreeRound(armies[0],armies[1]);
			if (armies[1]->tac > armies[0]->tac) FreeRound(armies[1],armies[0]);
		}
	}

	int round = 1;
	while (!armies[0]->Broken() && !armies[1]->Broken() && round < 101) {
		NormalRound(round++,armies[0],armies[1]);
	}

	if ((armies[0]->Broken() && !armies[1]->Broken()) ||
		(!armies[0]->NumAlive() && armies[1]->NumAlive())) {
		if (ass) assassination = ASS_FAIL;

		if (armies[0]->NumAlive()) {
			AddLine(*(armies[0]->leader->name) + " is routed!");
			FreeRound(armies[1],armies[0]);
		} else {
			AddLine(*(armies[0]->leader->name) + " is destroyed!");
		}
		AddLine("Total Casualties:");
		ItemList *spoils = new ItemList;
		armies[0]->Lose(this, spoils);
		GetSpoils(atts, spoils, ass);
		if (spoils->Num()) {
			temp = AString("Spoils: ") + spoils->Report(2,0,1) + ".";
		} else {
			temp = "Spoils: none.";
		}
		armies[1]->Win(this, spoils);
		AddLine("");
		AddLine(temp);
		AddLine("");
		delete spoils;
		delete armies[0];
		delete armies[1];
		return BATTLE_LOST;
	}

	if ((armies[1]->Broken() && !armies[0]->Broken()) ||
		(!armies[1]->NumAlive() && armies[0]->NumAlive())) {
		if (ass) {
			assassination = ASS_SUCC;
			asstext = new AString(*(armies[1]->leader->name) +
						" is assassinated in " +
						region->ShortPrint( pRegs ) +
						"!");
		}
		if (armies[1]->NumAlive()) {
			AddLine(*(armies[1]->leader->name) + " is routed!");
			FreeRound(armies[0],armies[1]);
		} else {
			AddLine(*(armies[1]->leader->name) + " is destroyed!");
		}
		AddLine("Total Casualties:");
		ItemList *spoils = new ItemList;
		armies[1]->Lose(this, spoils);
		GetSpoils(defs, spoils, ass);
		if (spoils->Num()) {
			temp = AString("Spoils: ") + spoils->Report(2,0,1) + ".";
		} else {
			temp = "Spoils: none.";
		}
		armies[0]->Win(this, spoils);
		AddLine("");
		AddLine(temp);
		AddLine("");
		delete spoils;
		delete armies[0];
		delete armies[1];
		return BATTLE_WON;
	}

	AddLine("The battle ends indecisively.");
	AddLine("");
	AddLine("Total Casualties:");
	armies[0]->Tie(this);
	armies[1]->Tie(this);
	temp = "Spoils: none.";
	AddLine("");
	AddLine(temp);
	AddLine("");
	delete armies[0];
	delete armies[1];
	return BATTLE_DRAW;
}

void Battle::WriteSides(ARegion * r,
			Unit * att,
			Unit * tar,
			AList * atts,
			AList * defs,
			int ass,
			ARegionList *pRegs )
{
	if (ass) {
		AddLine(*att->name + " attempts to assassinate " + *tar->name
				+ " in " + r->ShortPrint( pRegs ) + "!");
	} else {
		AddLine(*att->name + " attacks " + *tar->name + " in " +
				r->ShortPrint( pRegs ) + "!");
	}
	AddLine("");

	int dobs = 0;
	int aobs = 0;
	{
		forlist(defs) {
			int a = ((Location *)elem)->unit->GetAttribute("observation");
			if (a > dobs) dobs = a;
		}
	}

	AddLine("Attackers:");
	{
		forlist(atts) {
			int a = ((Location *)elem)->unit->GetAttribute("observation");
			if (a > aobs) aobs = a;
			AString * temp = ((Location *) elem)->unit->BattleReport(dobs);
			AddLine(*temp);
			delete temp;
		}
	}
	AddLine("");
	AddLine("Defenders:");
	{
		forlist(defs) {
			AString * temp = ((Location *) elem)->unit->BattleReport(aobs);
			AddLine(*temp);
			delete temp;
		}
	}
	AddLine("");
}

void Battle::Report(Areport * f,Faction * fac) {
	if (assassination == ASS_SUCC && fac != attacker) {
		f->PutStr(*asstext);
		f->PutStr("");
		return;
	}
	forlist(&text) {
		AString * s = (AString *) elem;
		f->PutStr(*s);
	}
}

void Battle::AddLine(const AString & s) {
	AString * temp = new AString(s);
	text.Add(temp);
}

void Game::GetDFacs(ARegion * r,Unit * t,AList & facs)
{
	int AlliesIncluded = 0;
	
	// First, check whether allies should assist in this combat
	if (Globals->ALLIES_NOAID == 0) {
		AlliesIncluded = 1;
	} else {
		// Check whether any of the target faction's
		// units aren't set to noaid
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->IsAlive()) {
					if (u->faction == t->faction &&
						(u->GetFlag(FLAG_NOAID) == 0)) {
						AlliesIncluded = 1;
						break;
					}
				}
				if (AlliesIncluded == 1) break; // forlist(units)
			}
			if (AlliesIncluded == 1) break; // forlist (objects)
		}
		//delete obj;
		//delete u;
	}
	
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			if (u->IsAlive()) {
				if (u->faction == t->faction ||
					(AlliesIncluded == 1 && 
					 u->guard != GUARD_AVOID &&
					 u->GetAttitude(r,t) == A_ALLY) ) {

					if (!GetFaction2(&facs,u->faction->num)) {
						FactionPtr * p = new FactionPtr;
						p->ptr = u->faction;
						facs.Add(p);
					}
				}
			}
		}
	}
}

void Game::GetAFacs(ARegion *r, Unit *att, Unit *tar, AList &dfacs,
		AList &afacs, AList &atts)
{
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			if (u->canattack && u->IsAlive()) {
				int add = 0;
				if ((u->faction == att->faction ||
							u->GetAttitude(r,tar) == A_HOSTILE) &&
						(u->guard != GUARD_AVOID || u == att)) {
					add = 1;
				} else {
					if (u->guard == GUARD_ADVANCE &&
							u->GetAttitude(r,tar) != A_ALLY) {
						add = 1;
					} else {
						if (u->attackorders) {
							forlist(&(u->attackorders->targets)) {
								UnitId * id = (UnitId *) elem;
								Unit *t = r->GetUnitId(id, u->faction->num);
								if (!t) continue;
								if (t == tar) {
									u->attackorders->targets.Remove(id);
									delete id;
								}
								if (t->faction == tar->faction) add = 1;
							}
						}
					}
				}

				if (add) {
					if (!GetFaction2(&dfacs,u->faction->num)) {
						Location * l = new Location;
						l->unit = u;
						l->obj = obj;
						l->region = r;
						atts.Add(l);
						if (!GetFaction2(&afacs,u->faction->num)) {
							FactionPtr * p = new FactionPtr;
							p->ptr = u->faction;
							afacs.Add(p);
						}
					}
				}
			}
		}
	}
}

int Game::CanAttack(ARegion * r,AList * afacs,Unit * u)
{
	int see = 0;
	int ride = 0;
	forlist(afacs) {
		FactionPtr * f = (FactionPtr *) elem;
		if (f->ptr->CanSee(r,u) == 2) {
			if (ride == 1) return 1;
			see = 1;
		}
		if (f->ptr->CanCatch(r,u)) {
			if (see == 1) return 1;
			ride = 1;
		}
	}
	return 0;
}

void Game::GetSides(ARegion *r, AList &afacs, AList &dfacs, AList &atts,
		AList &defs, Unit *att, Unit *tar, int ass, int adv)
{
	if (ass) {
		/* Assassination attempt */
		Location * l = new Location;
		l->unit = att;
		l->obj = r->GetDummy();
		l->region = r;
		atts.Add(l);

		l = new Location;
		l->unit = tar;
		l->obj = r->GetDummy();
		l->region = r;
		defs.Add(l);

		return;
	}

	int j=NDIRS;
	int noaida = 0, noaidd = 0;
	for (int i=-1;i<j;i++) {
		ARegion * r2 = r;
		if (i>=0) {
			r2 = r->neighbors[i];
			if (!r2) continue;
			forlist(&r2->objects) {
				/* Can't get building bonus in another region */
				((Object *) elem)->capacity = 0;
				((Object *) elem)->shipno = ((Object *) elem)->ships.Num();
			}
		} else {
			forlist(&r2->objects) {
				Object * o = (Object *) elem;
				/* Set building capacity */
				if (o->incomplete < 1 && o->IsBuilding()) {
					o->capacity = ObjectDefs[o->type].protect;
					o->shipno = 0;
				} else if (o->IsFleet()) {
					o->capacity = 0;
					o->shipno = 0;
				}
			}
		}
		forlist (&r2->objects) {
			Object * o = (Object *) elem;
			forlist (&o->units) {
				Unit * u = (Unit *) elem;
				int add = 0;

#define ADD_ATTACK 1
#define ADD_DEFENSE 2
				/* First, can the unit be involved in the battle at all? */
				if ((i==-1 || u->GetFlag(FLAG_HOLDING) == 0) && u->IsAlive()) {
					if (GetFaction2(&afacs,u->faction->num)) {
						/*
						 * The unit is on the attacking side, check if the
						 * unit should be in the battle
						 */
						if (i == -1 || (!noaida)) {
							if (u->canattack &&
									(u->guard != GUARD_AVOID || u==att) &&
									u->CanMoveTo(r2,r) &&
									!::GetUnit(&atts,u->num)) {
								add = ADD_ATTACK;
							}
						}
					} else {
						/* The unit is not on the attacking side */
						/*
						 * First, check for the noaid flag; if it is set,
						 * only units from this region will join on the
						 * defensive side
						 */
						if (!(i != -1 && noaidd)) {
							if (u->type == U_GUARD) {
								/* The unit is a city guardsman */
								if (i == -1 && adv == 0)
									add = ADD_DEFENSE;
							} else if (u->type == U_GUARDMAGE) {
								/* the unit is a city guard support mage */
								if (i == -1 && adv == 0)
									add = ADD_DEFENSE;
							} else {
								/*
								 * The unit is not a city guardsman, check if
								 * the unit is on the defensive side
								 */
								if (GetFaction2(&dfacs,u->faction->num)) {
									if (u->guard == GUARD_AVOID) {
										/*
										 * The unit is avoiding, and doesn't
										 * want to be in the battle if he can
										 * avoid it
										 */
										if (u == tar ||
												(u->faction == tar->faction &&
												 i==-1 &&
												 CanAttack(r,&afacs,u))) {
											add = ADD_DEFENSE;
										}
									} else {
										/*
										 * The unit is not avoiding, and wants
										 * to defend, if it can
										 */
										if (u->CanMoveTo(r2,r)) {
											add = ADD_DEFENSE;
										}
									}
								}
							}
						}
					}
				}

				if (add == ADD_ATTACK) {
					Location * l = new Location;
					l->unit = u;
					l->obj = o;
					l->region = r2;
					atts.Add(l);
				} else if (add == ADD_DEFENSE) {
						Location * l = new Location;
						l->unit = u;
						l->obj = o;
						l->region = r2;
						defs.Add(l);
				}
			}
		}
		//
		// If we are in the original region, check for the noaid status of
		// the units involved
		//
		if (i == -1) {
			noaida = 1;
			forlist (&atts) {
				Location *l = (Location *) elem;
				if (!l->unit->GetFlag(FLAG_NOAID)) {
					noaida = 0;
					break;
				}
			}
		}

		noaidd = 1;
		{
			forlist (&defs) {
				Location *l = (Location *) elem;
				if (!l->unit->GetFlag(FLAG_NOAID)) {
					noaidd = 0;
					break;
				}
			}
		}
	}
}

int Game::KillDead(Location * l, Battle *b)
{
	int uncontrolled = 0;
	int skel, undead;
	AString tmp;

	if (!l->unit->IsAlive()) {
		l->region->Kill(l->unit);
		uncontrolled += l->unit->raised;
		l->unit->raised = 0;
	} else {
		if (l->unit->advancefrom) {
			l->unit->MoveUnit( l->unit->advancefrom->GetDummy() );
		}
		if (l->unit->raised > 0) {
			undead = getrandom(l->unit->raised * 2 / 3 + 1);
			skel = l->unit->raised - undead;
			tmp = ItemString(I_SKELETON, skel);
			if (undead > 0) {
				tmp += " and ";
				tmp += ItemString(I_UNDEAD, undead);
			}
			tmp += " rise";
			if ((skel + undead) == 1)
				tmp += "s";
			tmp += " from the grave to join ";
			tmp += *l->unit->name;
			tmp += ".";
			l->unit->items.SetNum(I_SKELETON, l->unit->items.GetNum(I_SKELETON) + skel);
			l->unit->items.SetNum(I_UNDEAD, l->unit->items.GetNum(I_UNDEAD) + undead);
			b->AddLine(tmp);
			b->AddLine("");
			l->unit->raised = 0;
		}
	}

	return uncontrolled;
}

int Game::RunBattle(ARegion * r,Unit * attacker,Unit * target,int ass,
					 int adv)
{
	AList afacs,dfacs;
	AList atts,defs;
	FactionPtr * p;
	int result;

	if (ass) {
		if (attacker->GetAttitude(r,target) == A_ALLY) {
			attacker->Error("ASSASSINATE: Can't assassinate an ally.");
			return BATTLE_IMPOSSIBLE;
		}
		/* Assassination attempt */
		p = new FactionPtr;
		p->ptr = attacker->faction;
		afacs.Add(p);
		p = new FactionPtr;
		p->ptr = target->faction;
		dfacs.Add(p);
	} else {
		if ( r->IsSafeRegion() ) {
			attacker->Error("ATTACK: No battles allowed in safe regions.");
			return BATTLE_IMPOSSIBLE;
		}
		if (attacker->GetAttitude(r,target) == A_ALLY) {
			attacker->Error("ATTACK: Can't attack an ally.");
			return BATTLE_IMPOSSIBLE;
		}
		GetDFacs(r,target,dfacs);
		if (GetFaction2(&dfacs,attacker->faction->num)) {
			attacker->Error("ATTACK: Can't attack an ally.");
			return BATTLE_IMPOSSIBLE;
		}
		GetAFacs(r,attacker,target,dfacs,afacs,atts);
	}

	GetSides(r,afacs,dfacs,atts,defs,attacker,target,ass,adv);

	if (atts.Num() <= 0) {
		// This shouldn't happen, but just in case
		Awrite(AString("Cannot find any attackers!"));
		return BATTLE_IMPOSSIBLE;
	}
	if (defs.Num() <= 0) {
		// This shouldn't happen, but just in case
		Awrite(AString("Cannot find any defenders!"));
		return BATTLE_IMPOSSIBLE;
	}

	Battle * b = new Battle;
	b->WriteSides(r,attacker,target,&atts,&defs,ass, &regions );

	battles.Add(b);
	{
		forlist(&factions) {
			Faction * f = (Faction *) elem;
			if (GetFaction2(&afacs,f->num) || GetFaction2(&dfacs,f->num) ||
					r->Present(f)) {
				BattlePtr * p = new BattlePtr;
				p->ptr = b;
				f->battles.Add(p);
			}
		}
	}
	result = b->Run(r,attacker,&atts,target,&defs,ass, &regions );

	/* Remove all dead units */
	int uncontrolled = 0;
	{
		forlist(&atts) {
			uncontrolled += KillDead((Location *) elem, b);
		}
	}
	{
		forlist(&defs) {
			uncontrolled += KillDead((Location *) elem, b);
		}
	}
	if (uncontrolled > 0 && monfaction > 0) {
		int undead = getrandom(uncontrolled * 2 / 3 + 1);
		int skel = uncontrolled - undead;
		AString tmp = ItemString(I_SKELETON, skel);
		if (undead > 0) {
			tmp += " and ";
			tmp += ItemString(I_UNDEAD, undead);
		}
		tmp += " rise";
		if ((skel + undead) == 1)
			tmp += "s";
		tmp += " from the grave to seek vengeance.";
		Faction *monfac = GetFaction(&factions, monfaction);
		Unit *u = GetNewUnit(monfac, 0);
		u->MakeWMon("Undead", I_SKELETON, skel);
		u->items.SetNum(I_UNDEAD, undead);
		u->MoveUnit(r->GetDummy());
		b->AddLine(tmp);
		b->AddLine("");
	}
	return result;
}

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
#include "gamedata.h"
#include "quests.h"

void Game::RunOrders()
{
	//
	// Form and instant orders are handled during parsing
	//
	Awrite("Running FIND Orders...");
	RunFindOrders();
	Awrite("Running ENTER/LEAVE Orders...");
	RunEnterOrders(0);
	Awrite("Running PROMOTE/EVICT Orders...");
	RunPromoteOrders();
	Awrite("Running Combat...");
	DoAttackOrders();
	DoAutoAttacks();
	Awrite("Running STEAL/ASSASSINATE Orders...");
	RunStealOrders();
	Awrite("Running GIVE Orders...");
	DoGiveOrders();
	Awrite("Running ENTER NEW Orders...");
	RunEnterOrders(1);
	Awrite("Running EXCHANGE Orders...");
	DoExchangeOrders();
	Awrite("Running DESTROY Orders...");
	RunDestroyOrders();
	Awrite("Running PILLAGE Orders...");
	RunPillageOrders();
	Awrite("Running TAX Orders...");
	RunTaxOrders();
	Awrite("Running GUARD 1 Orders...");
	DoGuard1Orders();
	Awrite("Running Magic Orders...");
	ClearCastEffects();
	RunCastOrders();
	Awrite("Running SELL Orders...");
	RunSellOrders();
	Awrite("Running BUY Orders...");
	RunBuyOrders();
	Awrite("Running FORGET Orders...");
	RunForgetOrders();
	Awrite("Mid-Turn Processing...");
	MidProcessTurn();
	Awrite("Running QUIT Orders...");
	RunQuitOrders();
	Awrite("Removing Empty Units...");
	DeleteEmptyUnits();
	// SinkUncrewedFleets();
	// DrownUnits();
	if (Globals->ALLOW_WITHDRAW) {
		Awrite("Running WITHDRAW Orders...");
		DoWithdrawOrders();
	}
/*
	Awrite("Running Sail Orders...");
	RunSailOrders();
	Awrite("Running Move Orders...");
	RunMoveOrders();
*/
	Awrite("Running Consolidated Movement Orders...");
	RunMovementOrders();

	SinkUncrewedFleets();
	DrownUnits();
	FindDeadFactions();
	Awrite("Running Teach Orders...");
	RunTeachOrders();
	Awrite("Running Month-long Orders...");
	RunMonthOrders();
	Awrite("Running Economics...");
	ProcessEconomics();
	Awrite("Running Teleport Orders...");
	RunTeleportOrders();
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		Awrite("Running Transport Orders...");
		CheckTransportOrders();
		RunTransportOrders();
	}
	Awrite("Assessing Maintenance costs...");
	AssessMaintenance();
	if (Globals->DYNAMIC_POPULATION) {
		Awrite("Processing Migration...");
		ProcessMigration();
	}
	Awrite("Post-Turn Processing...");
	PostProcessTurn();
	DeleteEmptyUnits();
	// EmptyHell(); moved to Game::RunGame()
	// to prevent dead mages from causing
	// a segfault with IMPROVED_FARSIGHT
	RemoveEmptyObjects();
}

void Game::ClearCastEffects()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				u->SetFlag(FLAG_INVIS, 0);
			}
		}
	}
}

void Game::RunCastOrders()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->castorders) {
					RunACastOrder(r, o, u);
					delete u->castorders;
					u->castorders = 0;
				}
			}
		}
	}
}

/* Moved to game.cpp, where it belongs...
int Game::CountMages(Faction *pFac)
{
	int i = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == pFac && u->type == U_MAGE) i++;
			}
		}
	}
	return(i);
}
*/

int Game::TaxCheck(ARegion *pReg, Faction *pFac)
{
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if (AllowedTaxes(pFac) == -1) {
			//
			// No limit.
			//
			return(1);
		}

		forlist(&(pFac->war_regions)) {
			ARegion *x = ((ARegionPtr *) elem)->ptr;
			if (x == pReg) {
				//
				// This faction already performed a tax action in this
				// region.
				//
				return 1;
			}
		}
		if (pFac->war_regions.Num() >= AllowedTaxes(pFac)) {
			//
			// Can't tax here.
			//
			return 0;
		} else {
			//
			// Add this region to the faction's tax list.
			//
			ARegionPtr *y = new ARegionPtr;
			y->ptr = pReg;
			pFac->war_regions.Add(y);
			return 1;
		}
	} else {
		//
		// No limit on taxing regions in this game.
		//
		return(1);
	}
}

int Game::TradeCheck(ARegion *pReg, Faction *pFac)
{
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if (AllowedTrades(pFac) == -1) {
			//
			// No limit on trading on this faction.
			//
			return(1);
		}

		forlist(&(pFac->trade_regions)) {
			ARegion *x = ((ARegionPtr *) elem)->ptr;
			if (x == pReg) {
				//
				// This faction has already performed a trade action in this
				// region.
				//
				return 1;
			}
		}
		if (pFac->trade_regions.Num() >= AllowedTrades(pFac)) {
			//
			// This faction is over its trade limit.
			//
			return 0;
		} else {
			//
			// Add this region to the faction's trade list, and return 1.
			//
			ARegionPtr *y = new ARegionPtr;
			y->ptr = pReg;
			pFac->trade_regions.Add(y);
			return 1;
		}
	} else {
		//
		// No limit on trade in this game.
		//
		return(1);
	}
}

void Game::RunStealOrders()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist_safe(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->stealorders) {
					if (u->stealorders->type == O_STEAL) {
						Do1Steal(r, o, u);
					} else if (u->stealorders->type == O_ASSASSINATE) {
						Do1Assassinate(r, o, u);
					}
					delete u->stealorders;
					u->stealorders = 0;
				}
			}
		}
	}
}

AList *Game::CanSeeSteal(ARegion *r, Unit *u)
{
	AList *retval = new AList;
	forlist(&factions) {
		Faction *f = (Faction *) elem;
		if (r->Present(f)) {
			if (f->CanSee(r, u, Globals->SKILL_PRACTICE_AMOUNT > 0)) {
				FactionPtr *p = new FactionPtr;
				p->ptr = f;
				retval->Add(p);
			}
		}
	}
	return retval;
}

void Game::Do1Assassinate(ARegion *r, Object *o, Unit *u)
{
	AssassinateOrder *so = (AssassinateOrder *) u->stealorders;
	Unit *tar = r->GetUnitId(so->target, u->faction->num);

	if (!tar) {
		u->Error("ASSASSINATE: Invalid unit given.");
		return;
	}
	if (!tar->IsAlive()) {
		u->Error("ASSASSINATE: Invalid unit given.");
		return;
	}

	// New rule -- You can only assassinate someone you can see
	if (!u->CanSee(r, tar)) {
		u->Error("ASSASSINATE: Invalid unit given.");
		return;
	}

	if (tar->type == U_GUARD || tar->type == U_WMON ||
			tar->type == U_GUARDMAGE) {
		u->Error("ASSASSINATE: Can only assassinate other player's "
				"units.");
		return;
	}

	if (u->GetMen() != 1) {
		u->Error("ASSASSINATE: Must be executed by a 1-man unit.");
		return;
	}

	AList *seers = CanSeeSteal(r, u);
	int succ = 1;
	forlist(seers) {
		Faction *f = ((FactionPtr *) elem)->ptr;
		if (f == tar->faction) {
			succ = 0;
			break;
		}
		if (f->GetAttitude(tar->faction->num) == A_ALLY) {
			succ = 0;
			break;
		}
		if (f->num == guardfaction) {
			succ = 0;
			break;
		}
	}
	if (!succ) {
		AString temp = *(u->name) + " is caught attempting to assassinate " +
			*(tar->name) + " in " + *(r->name) + ".";
		forlist(seers) {
			Faction *f = ((FactionPtr *) elem)->ptr;
			f->Event(temp);
		}
		// One learns from one's mistakes.  Surviving them is another matter!
		u->PracticeAttribute("stealth");
		return;
	}

	int ass = 1;
	if (u->items.GetNum(I_RINGOFI)) {
		ass = 2; // Check if assassin has a ring.
		// New rule: if a target has an amulet of true seeing they
		// cannot be assassinated by someone with a ring of invisibility
		if (tar->AmtsPreventCrime(u)) {
			tar->Event("Assassination prevented by amulet of true seeing.");
			u->Event(AString("Attempts to assassinate ") + *(tar->name) +
					", but is prevented by amulet of true seeing.");
			return;
		}
	}
	u->PracticeAttribute("stealth");
	RunBattle(r, u, tar, ass);
}

void Game::Do1Steal(ARegion *r, Object *o, Unit *u)
{
	StealOrder *so = (StealOrder *) u->stealorders;
	Unit *tar = r->GetUnitId(so->target, u->faction->num);

	if (!tar) {
		u->Error("STEAL: Invalid unit given.");
		return;
	}

	// New RULE!! You can only steal from someone you can see.
	if (!u->CanSee(r, tar)) {
		u->Error("STEAL: Invalid unit given.");
		return;
	}

	if (tar->type == U_GUARD || tar->type == U_WMON ||
			tar->type == U_GUARDMAGE) {
		u->Error("STEAL: Can only steal from other player's "
				"units.");
		return;
	}

	if (u->GetMen() != 1) {
		u->Error("STEAL: Must be executed by a 1-man unit.");
		return;
	}

	AList *seers = CanSeeSteal(r, u);
	int succ = 1;
	forlist(seers) {
		Faction *f = ((FactionPtr *) elem)->ptr;
		if (f == tar->faction) {
			succ = 0;
			break;
		}
		if (f->GetAttitude(tar->faction->num) == A_ALLY) {
			succ = 0;
			break;
		}
		if (f->num == guardfaction) {
			succ = 0;
			break;
		}
	}

	if (!succ) {
		AString temp = *(u->name) + " is caught attempting to steal from " +
			*(tar->name) + " in " + *(r->name) + ".";
		forlist(seers) {
			Faction *f = ((FactionPtr *) elem)->ptr;
			f->Event(temp);
		}
		// One learns from one's mistakes.  Surviving them is another matter!
		u->PracticeAttribute("stealth");
		return;
	}

	//
	// New rule; if a target has an amulet of true seeing they can't be
	// stolen from by someone with a ring of invisibility
	//
	if (tar->AmtsPreventCrime(u)) {
		tar->Event("Theft prevented by amulet of true seeing.");
		u->Event(AString("Attempts to steal from ") + *(tar->name) + ", but "
				"is prevented by amulet of true seeing.");
		return;
	}

	int amt = 1;
	if (so->item == I_SILVER) {
		amt = tar->GetMoney();
		if (amt < 400) {
			amt = amt / 2;
		} else {
			amt = 200;
		}
	}

	if (tar->items.GetNum(so->item) < amt) amt = 0;

	u->items.SetNum(so->item, u->items.GetNum(so->item) + amt);
	tar->items.SetNum(so->item, tar->items.GetNum(so->item) - amt);

	{
		AString temp = *(u->name) + " steals " +
			ItemString(so->item, amt) + " from " + *(tar->name) + ".";
		forlist(seers) {
			Faction *f = ((FactionPtr *) elem)->ptr;
			f->Event(temp);
		}
	}

	tar->Event(AString("Has ") + ItemString(so->item, amt) + " stolen.");
	u->PracticeAttribute("stealth");
	return;
}

void Game::DrownUnits()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		if (TerrainDefs[r->type].similar_type == R_OCEAN) {
			forlist(&r->objects) {
				Object *o = (Object *) elem;
				if (o->type != O_DUMMY) continue;
				forlist(&o->units) {
					Unit *u = (Unit *)elem;
					int drown = 0;
					switch(Globals->FLIGHT_OVER_WATER) {
						case GameDefs::WFLIGHT_UNLIMITED:
							drown = !(u->CanSwim());
							break;
						case GameDefs::WFLIGHT_MUST_LAND:
							drown = !u->CanReallySwim();
							break;
						case GameDefs::WFLIGHT_NONE:
							drown = !(u->CanReallySwim());
							break;
						default: // Should never happen
							drown = 1;
							break;
					}
					if (drown) {
						r->Kill(u);
						u->Event("Drowns in the ocean.");
					}
				}
			}
		}
	}
}

void Game::SinkUncrewedFleets()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		r->CheckFleets();
	}
}

void Game::RunForgetOrders()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				forlist(&u->forgetorders) {
					ForgetOrder *fo = (ForgetOrder *) elem;
					u->ForgetSkill(fo->skill);
					u->Event(AString("Forgets ") + SkillStrs(fo->skill) + ".");
				}
				u->forgetorders.DeleteAll();
			}
		}
	}
}

void Game::RunQuitOrders()
{
	forlist(&factions) {
		Faction *f = (Faction *) elem;
		if (f->quit)
			Do1Quit(f);
	}
}

void Game::Do1Quit(Faction *f)
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == f) {
					r->Kill(u);
				}
			}
		}
	}
}

void Game::RunDestroyOrders()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			Unit *u = o->GetOwner();
			if (u) {
				if (u->destroy) {
					Do1Destroy(r, o, u);
					continue;
				} else {
					forlist(&o->units)
						((Unit *) elem)->destroy = 0;
				}
			}
		}
	}
}

void Game::Do1Destroy(ARegion *r, Object *o, Unit *u) {
	AString quest_rewards;

	if (TerrainDefs[r->type].similar_type == R_OCEAN) {
		u->Error("DESTROY: Can't destroy a ship while at sea.");
		forlist(&o->units) {
			((Unit *) elem)->destroy = 0;
		}
		return;
	}

	if (!u->GetMen()) {
		u->Error("DESTROY: Empty units cannot destroy structures.");
		forlist(&o->units) {
			((Unit *) elem)->destroy = 0;
		}
		return;
	}

	if (o->CanModify()) {
		u->Event(AString("Destroys ") + *(o->name) + ".");
		Object *dest = r->GetDummy();
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			u->destroy = 0;
			u->MoveUnit(dest);
		}
		if (quests.CheckQuestDemolishTarget(r, o->num, u, &quest_rewards)) {
			u->Event(AString("You have completed a quest!") + quest_rewards);
		}
		r->objects.Remove(o);
		delete o;
	} else {
		u->Error("DESTROY: Can't destroy that.");
		forlist(&o->units) {
			((Unit *) elem)->destroy = 0;
		}
	}
}

void Game::RunFindOrders()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				RunFindUnit(u);
			}
		}
	}
}

void Game::RunFindUnit(Unit *u)
{
	int all = 0;
	Faction *fac;
	forlist(&u->findorders) {
		FindOrder *f = (FindOrder *) elem;
		if (f->find == 0) all = 1;
		if (!all) {
			fac = GetFaction(&factions, f->find);
			if (fac) {
				u->faction->Event(AString("The address of ") + *(fac->name) +
						" is " + *(fac->address) + ".");
			} else {
				u->Error(AString("FIND: ") + f->find + " is not a valid "
						"faction number.");
			}
		} else {
			forlist(&factions) {
				fac = (Faction *)elem;
				if (fac) {
					u->faction->Event(AString("The address of ") +
							*(fac->name) + " is " + *(fac->address) + ".");
				}
			}
		}
	}
	u->findorders.DeleteAll();
}

void Game::RunTaxOrders()
{
	forlist(&regions) {
		RunTaxRegion((ARegion *) elem);
	}
}

int Game::FortTaxBonus(Object *o, Unit *u)
{
	int protect = ObjectDefs[o->type].protect;
	int fortbonus = 0;
	forlist(&o->units) {
		Unit *unit = (Unit *) elem;
		int men = unit->GetMen();
		if (unit->num == u->num) {
			if (unit->taxing == TAX_TAX) {
				int fortbonus = men;
				int maxtax = unit->Taxers(1);
				if (fortbonus > protect) fortbonus = protect;
				if (fortbonus > maxtax) fortbonus = maxtax;
				fortbonus *= Globals->TAX_BONUS_FORT;
				return(fortbonus);
			}
		}
		protect -= men;
		if (protect < 0) protect = 0;	
	}
	return(fortbonus);
}

int Game::CountTaxes(ARegion *reg)
{
	int t = 0;
	forlist(&reg->objects) {
		Object *o = (Object *) elem;
		int protect = ObjectDefs[o->type].protect;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->GetFlag(FLAG_AUTOTAX) && !Globals->TAX_PILLAGE_MONTH_LONG)
				u->taxing = TAX_TAX;
			if (u->taxing == TAX_AUTO) u->taxing = TAX_TAX;

			if (u->taxing == TAX_TAX) {
				if (reg->Population() < 1) {
					u->Error("TAX: No population to tax.");
					u->taxing = TAX_NONE;
				} else if (!reg->CanTax(u)) {
					u->Error("TAX: A unit is on guard.");
					u->taxing = TAX_NONE;
				} else {
					int men = u->Taxers(0);
					int fortbonus = u->GetMen();
					if (fortbonus > protect) fortbonus = protect;
					protect -= u->GetMen();
					if (protect < 0) protect = 0;
					if (men) {
						if (!TaxCheck(reg, u->faction)) {
							u->Error("TAX: Faction can't tax that many "
									"regions.");
							u->taxing = TAX_NONE;
						} else {
							t += men + fortbonus * Globals->TAX_BONUS_FORT;
						}
					} else {
						u->Error("TAX: Unit cannot tax.");
						u->taxing = TAX_NONE;
						u->SetFlag(FLAG_AUTOTAX, 0);
					}
				}
			}
		}
	}
	return t;
}

void Game::RunTaxRegion(ARegion *reg)
{
	int desired = CountTaxes(reg);
	if (desired < reg->wealth) desired = reg->wealth;

	forlist(&reg->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->taxing == TAX_TAX) {
				int t = u->Taxers(0);
				t += FortTaxBonus(o, u);
				double fAmt = ((double) t) *
					((double) reg->wealth) / ((double) desired);
				int amt = (int) fAmt;
				reg->wealth -= amt;
				desired -= t;
				u->SetMoney(u->GetMoney() + amt);
				u->Event(AString("Collects $") + amt + " in taxes in " +
						reg->ShortPrint(&regions) + ".");
				u->taxing = TAX_NONE;
			}
		}
	}
}

void Game::RunPillageOrders()
{
	forlist (&regions) {
		RunPillageRegion((ARegion *) elem);
	}
}

int Game::CountPillagers(ARegion *reg)
{
	int p = 0;
	forlist(&reg->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->taxing == TAX_PILLAGE) {
				if (!reg->CanPillage(u)) {
					u->Error("PILLAGE: A unit is on guard.");
					u->taxing = TAX_NONE;
				} else {
					int men = u->Taxers(1);
					if (men) {
						if (!TaxCheck(reg, u->faction)) {
							u->Error("PILLAGE: Faction can't tax that many "
									"regions.");
							u->taxing = TAX_NONE;
						} else {
							p += men;
						}
					} else {
						u->Error("PILLAGE: Not a combat ready unit.");
						u->taxing = TAX_NONE;
					}
				}
			}
		}
	}
	return p;
}

void Game::ClearPillagers(ARegion *reg)
{
	forlist(&reg->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->taxing == TAX_PILLAGE) {
				u->Error("PILLAGE: Not enough men to pillage.");
				u->taxing = TAX_NONE;
			}
		}
	}
}

void Game::RunPillageRegion(ARegion *reg)
{
	if (TerrainDefs[reg->type].similar_type == R_OCEAN) return;
	if (reg->wealth < 1) return;
	if (reg->Wages() <= 10*Globals->MAINTENANCE_COST) return;

	/* First, count up pillagers */
	int pillagers = CountPillagers(reg);

	if (pillagers * 2 * Globals->TAX_BASE_INCOME < reg->wealth) {
		ClearPillagers(reg);
		return;
	}

	AList *facs = reg->PresentFactions();
	int amt = reg->wealth * 2;
	forlist(&reg->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->taxing == TAX_PILLAGE) {
				u->taxing = TAX_NONE;
				int num = u->Taxers(1);
				int temp = (amt * num)/pillagers;
				amt -= temp;
				pillagers -= num;
				u->SetMoney(u->GetMoney() + temp);
				u->Event(AString("Pillages $") + temp + " from " +
						reg->ShortPrint(&regions) + ".");
				forlist(facs) {
					Faction *fp = ((FactionPtr *) elem)->ptr;
					if (fp != u->faction) {
						fp->Event(*(u->name) + " pillages " +
								*(reg->name) + ".");
					}
				}
			}
		}
	}
	delete facs;

	/* Destroy economy */
	reg->Pillage();
}

void Game::RunPromoteOrders()
{
	ARegion *r;
	Object *o;
	Unit *u;

	/* First, do any promote orders */
	forlist(&regions) {
		r = (ARegion *)elem;
		forlist(&r->objects) {
			o = (Object *)elem;
			if (o->type != O_DUMMY) {
				u = o->GetOwner();
				if (u && u->promote) {
					Do1PromoteOrder(o, u);
					delete u->promote;
					u->promote = 0;
				}
			}
		}
	}
	/* Now do any evict orders */
	{
		forlist(&regions) {
			r = (ARegion *)elem;
			forlist(&r->objects) {
				o = (Object *)elem;
				if (o->type != O_DUMMY) {
					u = o->GetOwner();
					if (u && u->evictorders) {
						Do1EvictOrder(o, u);
						delete u->evictorders;
						u->evictorders = 0;
					}
				}
			}
		}
	}

	/* Then, clear out other promote/evict orders */
	{
		forlist(&regions) {
			r = (ARegion *) elem;
			forlist(&r->objects) {
				o = (Object *) elem;
				forlist(&o->units) {
					u = (Unit *) elem;
					if (u->promote) {
						if (o->type != O_DUMMY) {
							u->Error("PROMOTE: Must be owner");
							delete u->promote;
							u->promote = 0;
						} else {
							u->Error("PROMOTE: Can only promote inside structures.");
							delete u->promote;
							u->promote = 0;
						}
					}
					if (u->evictorders) {
						if (o->type != O_DUMMY) {
							u->Error("EVICT: Must be owner");
							delete u->evictorders;
							u->evictorders = 0;
						} else {
							u->Error("EVICT: Can only evict inside structures.");
							delete u->evictorders;
							u->evictorders = 0;
						}
					}
				}
			}
		}
	}
}

void Game::Do1PromoteOrder(Object *obj, Unit *u)
{
	Unit *tar = obj->GetUnitId(u->promote, u->faction->num);
	if (!tar) {
		u->Error("PROMOTE: Can't find target.");
		return;
	}
	obj->units.Remove(tar);
	obj->units.Insert(tar);
}

void Game::Do1EvictOrder(Object *obj, Unit *u)
{
	EvictOrder *ord = u->evictorders;

	obj->region->DeduplicateUnitList(&ord->targets, u->faction->num);
	while (ord && ord->targets.Num()) {
		UnitId *id = (UnitId *)ord->targets.First();
		ord->targets.Remove(id);
		Unit *tar = obj->GetUnitId(id, u->faction->num);
		delete id;
		if (!tar) continue;
		if (obj->IsFleet() &&
			(TerrainDefs[obj->region->type].similar_type == R_OCEAN) &&
			(!tar->CanReallySwim() || tar->GetFlag(FLAG_NOCROSS_WATER))) {
			u->Error("EVICT: Cannot forcibly evict units over ocean.");
			continue;
		}
		Object *to = obj->region->GetDummy();
		tar->MoveUnit(to);
		tar->Event(AString("Evicted from ") + *obj->name + " by " + *u->name);
		u->Event(AString("Evicted ") + *tar->name + " from " + *obj->name);
	}
}

/* RunEnterOrders is performed in TWO phases: one in the
 * instant orders phase for existing objects and one after
 * give orders for entering new objects (fleets).
 */
void Game::RunEnterOrders(int phase)
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				// normal enter phase or ENTER NEW / JOIN phase?
				if (phase == 0) {
					if (u->enter > 0 || u->enter == -1)
						Do1EnterOrder(r, o, u);
				} else {
					if (u->joinorders)
						Do1JoinOrder(r, o, u);
				}
			}
		}
	}
}

void Game::Do1EnterOrder(ARegion *r, Object *in, Unit *u)
{
	Object *to;
	if (u->enter == -1) {
		to = r->GetDummy();
		u->enter = 0;
		if ((TerrainDefs[r->type].similar_type == R_OCEAN) &&
				(!u->CanSwim() || u->GetFlag(FLAG_NOCROSS_WATER))) {
			u->Error("LEAVE: Can't leave a ship in the ocean.");
			return;
		}
	} else {
		int on = u->enter;
		to = r->GetObject(on);
		u->enter = 0;
		if (!to) {
			u->Error("ENTER: Can't enter that.");
			return;
		}
		if (!to->CanEnter(r, u)) {
			u->Error("ENTER: Can't enter that.");
			return;
		}
		if (to->ForbiddenBy(r, u)) {
			u->Error("ENTER: Is refused entry.");
			return;
		}
	}
	u->MoveUnit(to);
}

void Game::Do1JoinOrder(ARegion *r, Object *in, Unit *u)
{
	JoinOrder *jo;
	Unit *tar, *pass;
	Object *to, *from;
	Item *item;

	jo = (JoinOrder *) u->joinorders;
	tar = r->GetUnitId(jo->target, u->faction->num);

	if (!tar) {
		u->Error("JOIN: No such unit.");
		return;
	}

	to = tar->object;
	if (!to) {
		u->Error("JOIN: Can't enter that.");
		return;
	}

	if (u->object == to) {
		// We're already there!
		return;
	}

	if (jo->merge) {
		if (!u->object->IsFleet() ||
				u->object->GetOwner()->num != u->num) {
			u->Error("JOIN MERGE: Not fleet owner.");
			return;
		}
		if (!to->IsFleet()) {
			u->Error("JOIN MERGE: Target unit is not in a fleet.");
			return;
		}
		forlist(&u->object->units) {
			pass = (Unit *) elem;
			if (to->ForbiddenBy(r, pass)) {
				u->Error("JOIN MERGE: A unit would be refused entry.");
				return;
			}
		}
		from = u->object;
		forlist_reuse(&from->ships) {
			item = (Item *) elem;
			GiveOrder go;
			UnitId id;
			go.amount = item->num;
			go.except = 0;
			go.item = item->type;
			id.unitnum = to->GetOwner()->num;
			id.alias = 0;
			id.faction = 0;
			go.target = &id;
			go.type = O_GIVE;
			go.merge = 1;
			DoGiveOrder(r, u, &go);
			go.target = NULL;
		}
		forlist_reuse(&u->object->units) {
			pass = (Unit *) elem;
			pass->MoveUnit(to);
		}

		return;
	}

	if (to == r->GetDummy()) {
		if ((TerrainDefs[r->type].similar_type == R_OCEAN) &&
				(!u->CanSwim() || u->GetFlag(FLAG_NOCROSS_WATER))) {
			u->Error("JOIN: Can't leave a ship in the ocean.");
			return;
		}
	} else {
		if (!to->CanEnter(r, u)) {
			u->Error("JOIN: Can't enter that.");
			return;
		}
		if (to->ForbiddenBy(r, u)) {
			u->Error("JOIN: Is refused entry.");
			return;
		}
		if (to->IsFleet() &&
				!jo->overload &&
				to->FleetCapacity() < to->FleetLoad() + u->Weight()) {
			u->Event("JOIN: Fleet would be overloaded.");
			return;
		}
	}
	u->MoveUnit(to);
}

void Game::RemoveEmptyObjects()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			if ((o->IsFleet()) && 
				(TerrainDefs[r->type].similar_type != R_OCEAN)) continue;
			if (ObjectDefs[o->type].cost &&
					o->incomplete >= ObjectDefs[o->type].cost) {
				forlist(&o->units) {
					Unit *u = (Unit *) elem;
					u->MoveUnit(r->GetDummy());
				}
				r->objects.Remove(o);
				delete o;
			}
		}
	}
}

void Game::EmptyHell()
{
	forlist(&regions)
		((ARegion *) elem)->ClearHell();
}

void Game::MidProcessUnit(ARegion *r, Unit *u)
{
	MidProcessUnitExtra(r, u);
}

void Game::PostProcessUnit(ARegion *r, Unit *u)
{
	PostProcessUnitExtra(r, u);
}

void Game::EndGame(Faction *pVictor)
{
	forlist(&factions) {
		Faction *pFac = (Faction *) elem;
		pFac->exists = 0;
		if (pFac == pVictor)
			pFac->quit = QUIT_WON_GAME;
		else
			pFac->quit = QUIT_GAME_OVER;

		if (pVictor)
			pFac->Event(*(pVictor->name) + " has won the game!");
		else
			pFac->Event("The game has ended with no winner.");
	}

	gameStatus = GAME_STATUS_FINISHED;
}

void Game::MidProcessTurn()
{
	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		// r->MidTurn(); // Not yet implemented
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				MidProcessUnit(r, u);
			}
		}
	}
}

void Game::ProcessEconomics()
{
	if (!(Globals->DYNAMIC_POPULATION || Globals->REGIONS_ECONOMY)) return;

	forlist(&regions) {
		ARegion *r = (ARegion *)elem;
		// Do not process not visited regions
		if (!r->visited) continue;
		/* regional population dynamics */
		r->Grow();
	}
}

/* Process Migration if DYNAMIC_POPULATION
 * is set. */
void Game::ProcessMigration()
{
	return;
	/* process two "phases" of migration
	 * allowing a region to spread it's migration
	 * between different destinations. */
	for (int phase = 1; phase <=2; phase++) {
		forlist(&regions) {
			ARegion *r = (ARegion *) elem;
			r->FindMigrationDestination(phase);
		}
		/* should always be true, but we need a
		 * different scope for AList handling, anyway */
		if (Globals->DYNAMIC_POPULATION) {
			forlist(&regions) {
				ARegion *r = (ARegion *) elem;
				r->Migrate();
			}
		}
	}
}

void Game::PostProcessTurn()
{
	//
	// Check if there are any factions left.
	//
	int livingFacs = 0;
	forlist(&factions) {
		Faction *pFac = (Faction *) elem;
		if (pFac->exists) {
			livingFacs = 1;
			break;
		}
	}

	if (!livingFacs)
		EndGame(0);
	else if (!(Globals->OPEN_ENDED)) {
		Faction *pVictor = CheckVictory();
		if (pVictor)
			EndGame(pVictor);
	}

	forlist_reuse(&regions) {
		ARegion *r = (ARegion *) elem;
		r->PostTurn(&regions);

		if (Globals->CITY_MONSTERS_EXIST && (r->town || r->type == R_NEXUS))
			AdjustCityMons(r);

		forlist (&r->objects) {
			Object *o = (Object *) elem;
			forlist (&o->units) {
				Unit *u = (Unit *) elem;
				PostProcessUnit(r, u);
			}
		}
	}

	if (Globals->WANDERING_MONSTERS_EXIST) GrowWMons(Globals->WMON_FREQUENCY);

	if (Globals->LAIR_MONSTERS_EXIST) GrowLMons(Globals->LAIR_FREQUENCY);

	if (Globals->LAIR_MONSTERS_EXIST) GrowVMons();
}

void Game::DoAutoAttacks()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->canattack && u->IsAlive())
					DoAutoAttack(r, u);
			}
		}
	}
}

void Game::DoMovementAttacks(AList *locs)
{
	Location *l;
	Unit *u;

	forlist(locs) {
		l = (Location *) elem;
		if (l->obj) {
			forlist(&l->obj->units) {
				u = (Unit *) elem;
				DoMovementAttack(l->region, u);
			}
		} else {
			DoMovementAttack(l->region, l->unit);
		}
	}
}

void Game::DoMovementAttack(ARegion *r, Unit *u)
{
	if (u->canattack && u->IsAlive()) {
		DoAutoAttack(r, u);
		if (!u->canattack || !u->IsAlive()) {
			u->guard = GUARD_NONE;
		}
	}
	if (u->canattack && u->guard == GUARD_ADVANCE && u->IsAlive()) {
		DoAdvanceAttack(r, u);
		u->guard = GUARD_NONE;
	}
	if (u->IsAlive()) {
		DoAutoAttackOn(r, u);
		if (!u->canattack || !u->IsAlive()) {
			u->guard = GUARD_NONE;
		}
	}
}

void Game::DoAutoAttackOn(ARegion *r, Unit *t)
{
	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->guard != GUARD_AVOID &&
					(u->GetAttitude(r, t) == A_HOSTILE) && u->IsAlive() &&
					u->canattack)
				AttemptAttack(r, u, t, 1);
			if (!t->IsAlive()) return;
		}
	}
}

void Game::DoAdvanceAttack(ARegion *r, Unit *u) {
	Unit *t = r->Forbidden(u);
	while (t && u->canattack && u->IsAlive()) {
		AttemptAttack(r, u, t, 1, 1);
		t = r->Forbidden(u);
	}
}

void Game::DoAutoAttack(ARegion *r, Unit *u) {
	if (u->guard == GUARD_AVOID)
		return;
	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *t = (Unit *) elem;
			if (u->GetAttitude(r, t) == A_HOSTILE) {
				AttemptAttack(r, u, t, 1);
			}
			if (u->canattack == 0 || u->IsAlive() == 0)
				return;
		}
	}
}

int Game::CountWMonTars(ARegion *r, Unit *mon) {
	int retval = 0;
	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->type == U_NORMAL || u->type == U_MAGE ||
					u->type == U_APPRENTICE) {
				if (mon->CanSee(r, u) && mon->CanCatch(r, u)) {
					retval += u->GetMen();
				}
			}
		}
	}
	return retval;
}

Unit *Game::GetWMonTar(ARegion *r, int tarnum, Unit *mon) {
	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->type == U_NORMAL || u->type == U_MAGE ||
					u->type == U_APPRENTICE) {
				if (mon->CanSee(r, u) && mon->CanCatch(r, u)) {
					int num = u->GetMen();
					if (num && tarnum < num) return u;
					tarnum -= num;
				}
			}
		}
	}
	return 0;
}

void Game::CheckWMonAttack(ARegion *r, Unit *u) {
	int tars = CountWMonTars(r, u);
	if (!tars) return;

	int rand = 300 - tars;
	if (rand < 100) rand = 100;
	if (getrandom(rand) >= u->Hostile()) return;

	Unit *t = GetWMonTar(r, getrandom(tars), u);
	if (t) AttemptAttack(r, u, t, 1);
}

void Game::DoAttackOrders()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->type == U_WMON) {
					if (u->canattack && u->IsAlive()) {
						CheckWMonAttack(r, u);
					}
				} else {
					if (u->attackorders && u->IsAlive()) {
						AttackOrder *ord = u->attackorders;
						r->DeduplicateUnitList(&ord->targets, u->faction->num);
						while (ord->targets.Num()) {
							UnitId *id = (UnitId *) ord->targets.First();
							ord->targets.Remove(id);
							Unit *t = r->GetUnitId(id, u->faction->num);
							delete id;
							if (u->canattack && u->IsAlive()) {
								if (t) {
									AttemptAttack(r, u, t, 0);
								} else {
									u->Error("ATTACK: Non-existent unit.");
								}
							}
						}
						delete ord;
						u->attackorders = 0;
					}
				}
			}
		}
	}
}

// Presume that u is alive, can attack, and wants to attack t.
// Check that t is alive, u can see t, and u has enough riding
// skill to catch t.
//
// Return 0 if success.
// 1 if t is already dead.
// 2 if u can't see t
// 3 if u lacks the riding to catch t
void Game::AttemptAttack(ARegion *r, Unit *u, Unit *t, int silent, int adv)
{
	if (!t->IsAlive()) return;

	if (!u->CanSee(r, t)) {
		if (!silent) u->Error("ATTACK: Non-existent unit.");
		return;
	}

	if (!u->CanCatch(r, t)) {
		if (!silent) u->Error("ATTACK: Can't catch that unit.");
		return;
	}

	if (t->routed && Globals->ONLY_ROUT_ONCE) {
		if (!silent) u->Event("ATTACK: Target is already routed and scattered.");
		return;
	}

	RunBattle(r, u, t, 0, adv);
	return;
}

void Game::RunSellOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->markets)) {
			Market *m = (Market *) elem;
			if (m->type == M_SELL)
				DoSell(r, m);
		}
		{
			forlist((&r->objects)) {
				Object *obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit *u = (Unit *) elem;
					forlist((&u->sellorders)) {
						u->Error("SELL: Can't sell that.");
					}
					u->sellorders.DeleteAll();
				}
			}
		}
	}
}

int Game::GetSellAmount(ARegion *r, Market *m)
{
	int num = 0;
	forlist((&r->objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			forlist ((&u->sellorders)) {
				SellOrder *o = (SellOrder *) elem;
				if (o->item == m->item) {
					if (o->num == -1) {
						o->num = u->items.CanSell(o->item);
					}
					if (m->amount != -1 && o->num > m->amount) {
						o->num = m->amount;
					}
					if (o->num < 0) o->num = 0;
					u->items.Selling(o->item, o->num);
					num += o->num;
				}
			}
		}
	}
	return num;
}

void Game::DoSell(ARegion *r, Market *m)
{
	/* First, find the number of items being sold */
	int attempted = GetSellAmount(r, m);

	if (attempted < m->amount) attempted = m->amount;
	m->activity = 0;
	int oldamount = m->amount;
	forlist((&r->objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			forlist((&u->sellorders)) {
				SellOrder *o = (SellOrder *) elem;
				if (o->item == m->item) {
					int temp = 0;
					if (o->num > u->GetSharedNum(o->item)) {
						o->num = u->GetSharedNum(o->item);
						u->Error("SELL: Unit attempted to sell "
								"more than it had.");
					}
					if (attempted) {
						temp = (m->amount *o->num + getrandom(attempted))
							/ attempted;
						if (temp<0) temp = 0;
					}
					attempted -= o->num;
					m->amount -= temp;
					m->activity += temp;
					u->ConsumeShared(o->item, temp);
					u->SetMoney(u->GetMoney() + temp * m->price);
					u->sellorders.Remove(o);
					u->Event(AString("Sells ") + ItemString(o->item, temp)
							+ " at $" + m->price + " each.");
					delete o;
				}
			}
		}
	}
	m->amount = oldamount;
}

void Game::RunBuyOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->markets)) {
			Market *m = (Market *) elem;
			if (m->type == M_BUY)
				DoBuy(r, m);
		}
		{
			forlist((&r->objects)) {
				Object *obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit *u = (Unit *) elem;
					forlist((&u->buyorders)) {
						u->Error("BUY: Can't buy that.");
					}
					u->buyorders.DeleteAll();
				}
			}
		}
	}
}

int Game::GetBuyAmount(ARegion *r, Market *m)
{
	int num = 0;
	forlist((&r->objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			forlist ((&u->buyorders)) {
				BuyOrder *o = (BuyOrder *) elem;
				if (o->item == m->item) {
					if (ItemDefs[o->item].type & IT_MAN) {
						if (u->type == U_MAGE) {
							u->Error("BUY: Mages can't recruit more men.");
							o->num = 0;
						}
						if (u->type == U_APPRENTICE) {
							AString temp = "BUY: ";
							temp += (char) toupper(Globals->APPRENTICE_NAME[0]);
							temp += Globals->APPRENTICE_NAME + 1;
							temp += "s can't recruit more men.";
							u->Error(temp);
							o->num = 0;
						}
						// XXX: there has to be a better way
						if (u->GetSkill(S_QUARTERMASTER)) {
							u->Error("BUY: Quartermasters can't recruit more "
									"men.");
							o->num = 0;
						}
						if (Globals->TACTICS_NEEDS_WAR &&
									u->GetSkill(S_TACTICS) == 5) {
							u->Error("BUY: Tacticians can't recruit more "
									"men.");
							o->num = 0;
						}
						if (((ItemDefs[o->item].type & IT_LEADER) &&
								u->IsNormal()) ||
								(!(ItemDefs[o->item].type & IT_LEADER) &&
								 u->IsLeader())) {
							u->Error("BUY: Can't mix leaders and normal men.");
							o->num = 0;
						}
					}
					if (o->num == -1) {
						o->num = u->GetSharedMoney()/m->price;
						if (m->amount != -1 && o->num > m->amount) {
							o->num = m->amount;
						}
					}
					if (m->amount != -1 && o->num > m->amount) {
						o->num = m->amount;
						u->Error("BUY: Unit attempted to buy more than were for sale.");
					}
					if (o->num * m->price > u->GetSharedMoney()) {
						o->num = u->GetSharedMoney() / m->price;
						u->Error("BUY: Unit attempted to buy more than it "
								"could afford.");
					}
					num += o->num;
				}
				if (o->num < 1 && o->num != -1) {
					u->buyorders.Remove(o);
					delete o;
				}
			}
		}
	}
	return num;
}

void Game::DoBuy(ARegion *r, Market *m)
{
	/* First, find the number of items being purchased */
	int attempted = GetBuyAmount(r, m);

	if (m->amount != -1)
		if (attempted < m->amount) attempted = m->amount;

	m->activity = 0;
	int oldamount = m->amount;
	forlist((&r->objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			forlist((&u->buyorders)) {
				BuyOrder *o = (BuyOrder *) elem;
				if (o->item == m->item) {
					int temp = 0;
					if (m->amount == -1) {
						/* unlimited market */
						temp = o->num;
					} else {
						if (attempted) {
							temp = (m->amount * o->num +
									getrandom(attempted)) / attempted;
							if (temp < 0) temp = 0;
						}
						attempted -= o->num;
						m->amount -= temp;
						m->activity += temp;
					}
					if (ItemDefs[o->item].type & IT_MAN) {
						/* recruiting; must dilute skills */
						SkillList *sl = new SkillList;
						u->AdjustSkills();
						delete sl;
						/* Setup specialized skill experience */
						if (Globals->REQUIRED_EXPERIENCE) {
							ManType *mt = FindRace(ItemDefs[o->item].abr);
							int exp = mt->speciallevel - mt->defaultlevel;
							if (exp > 0) {
								exp = exp * temp * GetDaysByLevel(1);
								for (int ms = 0; ms < ((int) sizeof(mt->skills))/((int) sizeof(int)); ms++)
								{
									AString sname = mt->skills[ms];
									int skill = LookupSkill(&sname);
									if (skill == -1) continue;
									int curxp = u->skills.GetExp(skill);
									u->skills.SetExp(skill,exp+curxp);
								} 
							}	
						}
						/* region economy effects */
						r->Recruit(temp);
					}
					u->items.SetNum(o->item, u->items.GetNum(o->item) + temp);
					u->faction->DiscoverItem(o->item, 0, 1);
					u->ConsumeSharedMoney(temp * m->price);
					u->buyorders.Remove(o);
					u->Event(AString("Buys ") + ItemString(o->item, temp)
							+ " at $" + m->price + " each.");
					delete o;
				}
			}
		}
	}

	m->amount = oldamount;
}

void Game::CheckUnitMaintenanceItem(int item, int value, int consume)
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->needed > 0 && ((!consume) ||
								(u->GetFlag(FLAG_CONSUMING_UNIT) ||
								u->GetFlag(FLAG_CONSUMING_FACTION)))) {
					int amount = u->items.GetNum(item);
					if (amount) {
						int eat = (u->needed + value - 1) / value;
						if (eat > amount)
							eat = amount;
						if (ItemDefs[item].type & IT_FOOD) {
							if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
								eat * value > u->stomach_space) {
								eat = (u->stomach_space + value - 1) / value;
								if (eat < 0)
									eat = 0;
							}
							u->hunger -= eat * value;
							u->stomach_space -= eat * value;
							if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
								u->stomach_space < 0) {
								u->needed -= u->stomach_space;
								u->stomach_space = 0;
							}
						}
						u->needed -= eat * value;
						u->items.SetNum(item, amount - eat);
					}
				}
			}
		}
	}
}

void Game::CheckFactionMaintenanceItem(int item, int value, int consume)
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->needed > 0 && ((!consume) ||
						u->GetFlag(FLAG_CONSUMING_FACTION))) {
					/* Go through all units again */
					forlist((&r->objects)) {
						Object *obj2 = (Object *) elem;
						forlist((&obj2->units)) {
							Unit *u2 = (Unit *) elem;

							if (u->faction == u2->faction && u != u2) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->needed + value - 1) / value;
									if (eat > amount)
										eat = amount;
									if (ItemDefs[item].type & IT_FOOD) {
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
											eat * value > u->stomach_space) {
											eat = (u->stomach_space + value - 1) / value;
											if (eat < 0)
												eat = 0;
										}
										u->hunger -= eat * value;
										u->stomach_space -= eat * value;
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
											u->stomach_space < 0) {
											u->needed -= u->stomach_space;
											u->stomach_space = 0;
										}
									}
									u->needed -= eat * value;
									u2->items.SetNum(item, amount - eat);
								}
							}
						}

						if (u->needed < 1) break;
					}
				}
			}
		}
	}
}

void Game::CheckAllyMaintenanceItem(int item, int value)
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->needed > 0) {
					/* Go through all units again */
					forlist((&r->objects)) {
						Object *obj2 = (Object *) elem;
						forlist((&obj2->units)) {
							Unit *u2 = (Unit *) elem;
							if (u->faction != u2->faction &&
								u2->GetAttitude(r, u) == A_ALLY) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->needed + value - 1) / value;
									if (eat > amount)
										eat = amount;
									if (ItemDefs[item].type & IT_FOOD) {
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
											eat * value > u->stomach_space) {
											eat = (u->stomach_space + value - 1) / value;
											if (eat < 0)
												eat = 0;
										}
										u->hunger -= eat * value;
										u->stomach_space -= eat * value;
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
											u->stomach_space < 0) {
											u->needed -= u->stomach_space;
											u->stomach_space = 0;
										}
									}
									if (eat) {
										u->needed -= eat * value;
										u2->items.SetNum(item, amount - eat);
										u2->Event(*(u->name) + " borrows " +
												ItemString(item, eat) +
												" for maintenance.");
										u->Event(AString("Borrows ") +
												ItemString(item, eat) +
												" from " + *(u2->name) +
												" for maintenance.");
										u2->items.SetNum(item, amount - eat);
									}
								}
							}
						}

						if (u->needed < 1) break;
					}
				}
			}
		}
	}
}

void Game::CheckUnitHungerItem(int item, int value)
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->hunger > 0) {
					int amount = u->items.GetNum(item);
					if (amount) {
						int eat = (u->hunger + value - 1) / value;
						if (eat > amount)
							eat = amount;
						u->hunger -= eat * value;
						u->stomach_space -= eat * value;
						u->needed -= eat * value;
						if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
							u->stomach_space < 0) {
							u->needed -= u->stomach_space;
							u->stomach_space = 0;
						}
						u->items.SetNum(item, amount - eat);
					}
				}
			}
		}
	}
}

void Game::CheckFactionHungerItem(int item, int value)
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->hunger > 0) {
					/* Go through all units again */
					forlist((&r->objects)) {
						Object *obj2 = (Object *) elem;
						forlist((&obj2->units)) {
							Unit *u2 = (Unit *) elem;

							if (u->faction == u2->faction && u != u2) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->hunger + value - 1) / value;
									if (eat > amount)
										eat = amount;
									u->hunger -= eat * value;
									u->stomach_space -= eat * value;
									u->needed -= eat * value;
									if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
										u->stomach_space < 0) {
										u->needed -= u->stomach_space;
										u->stomach_space = 0;
									}
									u2->items.SetNum(item, amount - eat);
								}
							}
						}

						if (u->hunger < 1) break;
					}
				}
			}
		}
	}
}

void Game::CheckAllyHungerItem(int item, int value)
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->hunger > 0) {
					/* Go through all units again */
					forlist((&r->objects)) {
						Object *obj2 = (Object *) elem;
						forlist((&obj2->units)) {
							Unit *u2 = (Unit *) elem;
							if (u->faction != u2->faction &&
								u2->GetAttitude(r, u) == A_ALLY) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->hunger + value - 1) / value;
									if (eat > amount)
										eat = amount;
									u->hunger -= eat * value;
									u->stomach_space -= eat * value;
									u->needed -= eat * value;
									if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
										u->stomach_space < 0) {
										u->needed -= u->stomach_space;
										u->stomach_space = 0;
									}
									u2->items.SetNum(item, amount - eat);
										u2->Event(*(u->name) + " borrows " +
												ItemString(item, eat) +
												" to fend off starvation.");
										u->Event(AString("Borrows ") +
												ItemString(item, eat) +
												" from " + *(u2->name) +
												" to fend off starvation.");
										u2->items.SetNum(item, amount - eat);
								}
							}
						}

						if (u->hunger < 1) break;
					}
				}
			}
		}
	}
}

void Game::AssessMaintenance()
{
	AString quest_rewards;

	/* First pass: set needed */
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (!(u->faction->IsNPC())) {
					r->visited = 1;
					if (quests.CheckQuestVisitTarget(r, u, &quest_rewards)) {
						u->Event(AString("You have completed a pilgrimage!") + quest_rewards);
					}
				}
				u->needed = u->MaintCost();
				u->hunger = u->GetMen() * Globals->UPKEEP_MINIMUM_FOOD;
				if (Globals->UPKEEP_MAXIMUM_FOOD < 0)
					u->stomach_space = -1;
				else
					u->stomach_space = u->GetMen() *
										Globals->UPKEEP_MAXIMUM_FOOD;
			}
		}
	}

	// Assess food requirements first
	if (Globals->UPKEEP_MINIMUM_FOOD > 0) {
		CheckUnitHunger();
		CheckFactionHunger();
		if (Globals->ALLOW_WITHDRAW) {
			// Can claim food for maintenance, so find the cheapest food
			int i = -1, cost = -1;
			for (int j = 0; j < NITEMS; j++) {
				if (ItemDefs[j].flags & ItemType::DISABLED) continue;
				if (ItemDefs[j].type & IT_FOOD) {
					if (i == -1 ||
							ItemDefs[i].baseprice > ItemDefs[j].baseprice)
						i = j;
				}
			}
			if (i > 0) {
				cost = ItemDefs[i].baseprice * 5 / 2;
				forlist((&regions)) {
					ARegion *r = (ARegion *) elem;
					forlist((&r->objects)) {
						Object *obj = (Object *) elem;
						forlist((&obj->units)) {
							Unit *u = (Unit *) elem;
							if (u->hunger > 0 && u->faction->unclaimed > cost) {
								int value = Globals->UPKEEP_FOOD_VALUE;
								int eat = (u->hunger + value - 1) / value;
								/* Now see if faction has money */
								if (u->faction->unclaimed >= eat * cost) {
									u->Event(AString("Withdraws ") +
											ItemString(i, eat) +
											" for maintenance.");
									u->faction->unclaimed -= eat * cost;
									u->hunger -= eat * value;
									u->stomach_space -= eat * value;
									u->needed -= eat * value;
								} else {
									int amount = u->faction->unclaimed / cost;
									u->Event(AString("Withdraws ") +
											ItemString(i, amount) +
											" for maintenance.");
									u->faction->unclaimed -= amount * cost;
									u->hunger -= amount * value;
									u->stomach_space -= amount * value;
									u->needed -= amount * value;
								}
							}
						}
					}
				}
			}
		}
		CheckAllyHunger();
	}

	//
	// Check for CONSUMEing units.
	//
	if (Globals->FOOD_ITEMS_EXIST) {
		CheckUnitMaintenance(1);
		CheckFactionMaintenance(1);
	}

	//
	// Check the unit for money.
	//
	CheckUnitMaintenanceItem(I_SILVER, 1, 0);

	//
	// Check other units in same faction for money
	//
	CheckFactionMaintenanceItem(I_SILVER, 1, 0);

	if (Globals->FOOD_ITEMS_EXIST) {
		//
		// Check unit for possible food items.
		//
		CheckUnitMaintenance(0);

		//
		// Fourth pass; check other units in same faction for food items
		//
		CheckFactionMaintenance(0);
	}

	//
	// Check unclaimed money.
	//
	{
		forlist((&regions)) {
			ARegion *r = (ARegion *) elem;
			forlist((&r->objects)) {
				Object *obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit *u = (Unit *) elem;
					if (u->needed > 0 && u->faction->unclaimed) {
						/* Now see if faction has money */
						if (u->faction->unclaimed >= u->needed) {
							u->Event(AString("Claims ") + u->needed +
									 " silver for maintenance.");
							u->faction->unclaimed -= u->needed;
							u->needed = 0;
						} else {
							u->Event(AString("Claims ") +
									u->faction->unclaimed +
									" silver for maintenance.");
							u->needed -= u->faction->unclaimed;
							u->faction->unclaimed = 0;
						}
					}
				}
			}
		}
	}

	//
	// Check other allied factions for $$$.
	//
	CheckAllyMaintenanceItem(I_SILVER, 1);

	if (Globals->FOOD_ITEMS_EXIST) {
		//
		// Check other factions for food items.
		//
		CheckAllyMaintenance();
	}

	//
	// Last, if the unit still needs money, starve some men.
	//
	{
		forlist((&regions)) {
			ARegion *r = (ARegion *) elem;
			forlist((&r->objects)) {
				Object *obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit *u = (Unit *) elem;
					if (u->needed > 0 || u->hunger > 0)
						u->Short(u->needed, u->hunger);
				}
			}
		}
	}
}

void Game::DoWithdrawOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *)elem;
		forlist((&r->objects)) {
			Object *obj = (Object *)elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				forlist((&u->withdraworders)) {
					WithdrawOrder *o = (WithdrawOrder *)elem;
					if (DoWithdrawOrder(r, u, o)) break;
				}
				u->withdraworders.DeleteAll();
			}
		}
	}
}

int Game::DoWithdrawOrder(ARegion *r, Unit *u, WithdrawOrder *o)
{
	int itm = o->item;
	int amt = o->amount;
	int cost = (ItemDefs[itm].baseprice *5/2)*amt;

	if (r->type == R_NEXUS) {
		u->Error("WITHDRAW: Withdraw does not work in the Nexus.");
		return 1;
	}

	if (cost > u->faction->unclaimed) {
		u->Error(AString("WITHDRAW: Too little unclaimed silver to withdraw ")+
				ItemString(itm, amt)+".");
		return 0;
	}

	if (ItemDefs[itm].max_inventory) {
		int cur = u->items.GetNum(itm) + amt;
		if (cur > ItemDefs[itm].max_inventory) {
			u->Error(AString("WITHDRAW: Unit cannot have more than ")+
					ItemString(itm, ItemDefs[itm].max_inventory));
			return 0;
		}
	}

	u->faction->unclaimed -= cost;
	u->Event(AString("Withdraws ") + ItemString(o->item, amt) + ".");
	u->items.SetNum(itm, u->items.GetNum(itm) + amt);
	u->faction->DiscoverItem(itm, 0, 1);
	return 0;
}

void Game::DoGiveOrders()
{
	Item *item;
	Unit *s;
	Object *fleet;

	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				forlist((&u->giveorders)) {
					GiveOrder *o = (GiveOrder *)elem;
					if (o->item < 0) {
						if (o->amount == -1) {
							/* do 'give X unit' command */
							DoGiveOrder(r, u, o);
						} else if (o->amount == -2) {
							if (o->type == O_TAKE) {
								s = r->GetUnitId(o->target, u->faction->num);
								if (!s || !u->CanSee(r, s)) {
									u->Error(AString("TAKE: Nonexistant target (") +
											o->target->Print() + ").");
									continue;
								} else if (u->faction != s->faction) {
									u->Error(AString("TAKE: ") + o->target->Print() +
											" is not a member of your faction.");
									continue;
								}
								fleet = s->object;
							} else {
								s = u;
								fleet = obj;
							}
							/* do 'give all type' command */
							if (fleet->IsFleet() && s == fleet->GetOwner() &&
									!o->unfinished &&
									(o->item == -NITEMS || o->item == -IT_SHIP)) {
								forlist(&fleet->ships) {
									item = (Item *) elem;
									GiveOrder go;
									go.amount = item->num;
									go.except = 0;
									go.item = item->type;
									go.target = o->target;
									go.type = o->type;
									DoGiveOrder(r, u, &go);
									go.target = NULL;
								}
							}
							forlist((&s->items)) {
								item = (Item *) elem;
								if ((o->item == -NITEMS) ||
									(ItemDefs[item->type].type & (-o->item))) {
									GiveOrder go;
									go.amount = item->num;
									go.except = 0;
									go.item = item->type;
									go.target = o->target;
									go.type = o->type;
									go.unfinished = o->unfinished;
									if (ItemDefs[item->type].type & IT_SHIP) {
										if (o->item == -NITEMS) {
											go.unfinished = 1;
										}
										if (go.unfinished) {
											go.amount = 1;
										} else {
											go.amount = 0;
										}
									} else if (o->unfinished) {
										go.amount = 0;
									}
									if (go.amount) {
										DoGiveOrder(r, u, &go);
									}
									go.target = NULL;
								}
							}
						} else {
							if (o->type == O_TAKE)
								u->Error("TAKE: Invalid item.");
							else
								u->Error("GIVE: Invalid item.");
						}
					} else if (DoGiveOrder(r, u, o)) {
						break;
					}
				}
				u->giveorders.DeleteAll();
			}
		}
	}
}

void Game::DoExchangeOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				forlist((&u->exchangeorders)) {
					Order *o = (Order *) elem;
					DoExchangeOrder(r, u, (ExchangeOrder *) o);
				}
			}
		}
	}
}

void Game::DoExchangeOrder(ARegion *r, Unit *u, ExchangeOrder *o)
{
	// Check if the destination unit exists
	Unit *t = r->GetUnitId(o->target, u->faction->num);
	if (!t) {
		u->Error(AString("EXCHANGE: Nonexistant target (") +
				o->target->Print() + ").");
		u->exchangeorders.Remove(o);
		return;
	}

	// Check each Item can be given
	if (ItemDefs[o->giveItem].flags & ItemType::CANTGIVE) {
		u->Error(AString("EXCHANGE: Can't trade ") +
				ItemDefs[o->giveItem].names + ".");
		u->exchangeorders.Remove(o);
		return;
	}

	if (ItemDefs[o->expectItem].flags & ItemType::CANTGIVE) {
		u->Error(AString("EXCHANGE: Can't trade ") +
				ItemDefs[o->expectItem].names + ".");
		u->exchangeorders.Remove(o);
		return;
	}

	if (ItemDefs[o->giveItem].type & IT_MAN) {
		u->Error("EXCHANGE: Exchange aborted.  Men may not be traded.");
		u->exchangeorders.Remove(o);
		return;
	}

	if (ItemDefs[o->expectItem].type & IT_MAN) {
		u->Error("EXCHANGE: Exchange aborted. Men may not be traded.");
		u->exchangeorders.Remove(o);
		return;
	}

	// New RULE -- Must be able to see unit to give something to them!
	if (!u->CanSee(r, t)) {
		u->Error(AString("EXCHANGE: Nonexistant target (") +
				o->target->Print() + ").");
		return;
	}
	// Check other unit has enough to give
	int amtRecieve = o->expectAmount;
	if (amtRecieve > t->GetSharedNum(o->expectItem)) {
		t->Error(AString("EXCHANGE: Not giving enough. Expecting ") +
				ItemString(o->expectItem, o->expectAmount) + ".");
		u->Error(AString("EXCHANGE: Exchange aborted.  Not enough ") +
				"recieved. Expecting " +
			ItemString(o->expectItem, o->expectAmount) + ".");
		o->exchangeStatus = 0;
		return;
	}

	int exchangeOrderFound = 0;
	// Check if other unit has a reciprocal exchange order
	forlist ((&t->exchangeorders)) {
		ExchangeOrder *tOrder = (ExchangeOrder *) elem;
		Unit *ptrUnitTemp = r->GetUnitId(tOrder->target, t->faction->num);
		if (ptrUnitTemp == u) {
			if (tOrder->expectItem == o->giveItem) {
				if (tOrder->giveItem == o->expectItem) {
					exchangeOrderFound = 1;
					if (tOrder->giveAmount < o->expectAmount) {
						t->Error(AString("EXCHANGE: Not giving enough. ") +
								"Expecting " +
								ItemString(o->expectItem, o->expectAmount) +
								".");
						u->Error(AString("EXCHANGE: Exchange aborted. ") +
								"Not enough recieved. Expecting " +
								ItemString(o->expectItem, o->expectAmount) +
								".");
						tOrder->exchangeStatus = 0;
						o->exchangeStatus = 0;
						return;
					} else if (tOrder->giveAmount > o->expectAmount) {
						t->Error(AString("EXCHANGE: Exchange aborted. Too ") +
								"much given. Expecting " +
								ItemString(o->expectItem, o->expectAmount) +
								".");
						u->Error(AString("EXCHANGE: Exchange aborted. Too ") +
								"much offered. Expecting " +
								ItemString(o->expectItem, o->expectAmount) +
								".");
						tOrder->exchangeStatus = 0;
						o->exchangeStatus = 0;
					} else if (tOrder->giveAmount == o->expectAmount)
						o->exchangeStatus = 1;

					if ((o->exchangeStatus == 1) &&
							(tOrder->exchangeStatus == 1)) {
						u->Event(AString("Exchanges ") +
								ItemString(o->giveItem, o->giveAmount) +
								" with " + *t->name + " for " +
								ItemString(tOrder->giveItem,
									tOrder->giveAmount) +
								".");
						t->Event(AString("Exchanges ") +
								ItemString(tOrder->giveItem,
									tOrder->giveAmount) + " with " +
								*u->name + " for " +
								ItemString(o->giveItem, o->giveAmount) + ".");
						u->ConsumeShared(o->giveItem, o->giveAmount);
						t->items.SetNum(o->giveItem,
								t->items.GetNum(o->giveItem) + o->giveAmount);
						t->ConsumeShared(tOrder->giveItem, tOrder->giveAmount);
						u->items.SetNum(tOrder->giveItem,
								u->items.GetNum(tOrder->giveItem) +
								tOrder->giveAmount);
						u->faction->DiscoverItem(tOrder->giveItem, 0, 1);
						t->faction->DiscoverItem(o->giveItem, 0, 1);
						u->exchangeorders.Remove(o);
						t->exchangeorders.Remove(tOrder);
						return;
					} else if ((o->exchangeStatus >= 0) &&
							(tOrder->exchangeStatus >= 0)) {
						u->exchangeorders.Remove(o);
						t->exchangeorders.Remove(tOrder);
					}
				}
			}
		}
	}
	if (!exchangeOrderFound) {
		if (!u->CanSee(r, t)) {
			u->Error(AString("EXCHANGE: Nonexistant target (") +
					o->target->Print() + ").");
			u->exchangeorders.Remove(o);
			return;
		} else {
			u->Error("EXCHANGE: target unit did not issue a matching "
					"exchange order.");
			u->exchangeorders.Remove(o);
			return;
		}
	}
}

int Game::DoGiveOrder(ARegion *r, Unit *u, GiveOrder *o)
{
	int hasitem, ship, num, shipcount, amt, newfleet, cur;
	int notallied, newlvl, oldlvl;
	Item *it, *sh;
	Unit *p, *t, *s;
	Object *fleet;
	AString temp, ord;
	Skill *skill;
	SkillList *skills;

	if (o->type == O_TAKE)
		ord = "TAKE";
	else
		ord = "GIVE";

	/* Transfer/GIVE ship items: */
	if ((o->item >= 0) && (ItemDefs[o->item].type & IT_SHIP)) {
		// GIVE 0
		if (o->target->unitnum == -1) {
			hasitem = u->items.GetNum(o->item);
			temp = "Abandons ";
			// discard unfinished ships from inventory
			if (o->unfinished) {
				if (!hasitem || o->amount > 1) {
					u->Error("GIVE: not enough.");
					return 0;
				}
				ship = -1;
				forlist(&u->items) {
					it = (Item *) elem;
					if (it->type == o->item) {
						u->Event(temp + it->Report(1) + ".");
						ship = it->type;
					}
				}
				if (ship > 0) u->items.SetNum(ship,0);
				return 0;
			// abandon fleet ships
			} else if (!(u->object->IsFleet()) || 
				(u->num != u->object->GetOwner()->num)) {
				u->Error("GIVE: only fleet owner can give ships.");
				return 0;
			}

			// Check amount
			num = u->object->GetNumShips(o->item);
			if (num < 1) {
				u->Error("GIVE: no such ship in fleet.");
				return 0;
			}
			if ((num < o->amount) && (o->amount != -2)) {
				u->Error("GIVE: not enough ships.");
				o->amount = num;
			}

			// Check we're not dumping passengers in the ocean
			if (TerrainDefs[r->type].similar_type == R_OCEAN) {
				shipcount = 0;
				forlist(&(u->object->ships)) {
					sh = (Item *) elem;
					shipcount += sh->num;
				}
				if (shipcount <= o->amount) {
					forlist(&(u->object->units)) {
						p = (Unit *) elem;
						if ((!p->CanSwim() || p->GetFlag(FLAG_NOCROSS_WATER))) {
							u->Error("GIVE: Can't abandon our last ship in the ocean.");
							return 0;
						}
					}
				}
			}

			u->object->SetNumShips(o->item, num - o->amount);
			u->Event(AString(temp) + ItemString(o->item, num - o->amount) + ".");
			return 0;
		}
		// GIVE to unit:	
		t = r->GetUnitId(o->target, u->faction->num);
		if (!t) {
			u->Error(ord + ": Nonexistant target (" +
				o->target->Print() + ").");
			return 0;
		} else if (o->type == O_TAKE && u->faction != t->faction) {
			u->Error(ord + ": " + o->target->Print() +
					" is not a member of your faction.");
			return 0;
		} else if (u->faction != t->faction && t->faction->IsNPC()) {
			u->Error(ord + ": Can't give to non-player unit (" +
				o->target->Print() + ").");
			return 0;
		}
		if (u == t) {
			if (o->type == O_TAKE)
				u->Error(ord + ": Attempt to take " +
					ItemDefs[o->item].names + " from self.");
			else
				u->Error(ord + ": Attempt to give " +
					ItemDefs[o->item].names + " to self.");
			return 0;
		}
		if (!u->CanSee(r, t) &&
			(t->faction->GetAttitude(u->faction->num) < A_FRIENDLY)) {
				u->Error(ord + ": Nonexistant target (" +
					o->target->Print() + ").");
				return 0;
		}
		if (t->faction->GetAttitude(u->faction->num) < A_FRIENDLY) {
				u->Error(ord + ": Target is not a member of a friendly faction.");
				return 0;
		}
		if (ItemDefs[o->item].flags & ItemType::CANTGIVE) {
			if (o->type == O_TAKE)
				u->Error(ord + ": Can't take " +
					ItemDefs[o->item].names + ".");
			else
				u->Error(ord + ": Can't give " +
					ItemDefs[o->item].names + ".");
			return 0;
		}
		s = u;
		if (o->type == O_TAKE) {
			s = t;
			t = u;
		}
		// Check amount
		if (o->unfinished) {
			num = s->items.GetNum(o->item) > 0;
		} else if (!(s->object->IsFleet()) || 
			(s->num != s->object->GetOwner()->num)) {
			u->Error(ord + ": only fleet owner can transfer ships.");
			return 0;
		} else {
			num = s->object->GetNumShips(o->item);
			if (num < 1) {
				u->Error(ord + ": no such ship in fleet.");
				return 0;
			}
		}
		amt = o->amount;
		if (amt != -2 && amt > num) {
			u->Error(ord + ": Not enough.");
			amt = num;
		} else if (amt == -2) {
			amt = num;
			if (o->except) {
				if (o->except > amt) {
					amt = 0;
					u->Error(ord + ": EXCEPT value greater than amount on hand.");
				} else {
					amt = amt - o->except;
				}
			}
		}
		if (!amt) {
			// giving 0 things is easy
			return 0;
		}

		if (o->unfinished) {
			if (t->items.GetNum(o->item) > 0) {
				if (o->type == O_TAKE)
					u->Error(ord + ": Already have an unfinished ship of that type.");
				else
					u->Error(ord + ": Target already has an unfinished ship of that type.");
				return 0;
			}
			it = new Item();
			it->type = o->item;
			it->num = s->items.GetNum(o->item);
			if (o->type == O_TAKE) {
				u->Event(AString("Takes ") + it->Report(1) +
					" from " + *s->name + ".");
			} else {
				u->Event(AString("Gives ") + it->Report(1) +
					" to " + *t->name + ".");
				if (s->faction != t->faction) {
					t->Event(AString("Receives ") + it->Report(1) +
						" from " + *s->name + ".");
				}
			}
			s->items.SetNum(o->item, 0);
			t->items.SetNum(o->item, it->num);
			t->faction->DiscoverItem(o->item, 0, 1);
		} else {
			// Check we're not dumping passengers in the ocean
			if (TerrainDefs[r->type].similar_type == R_OCEAN &&
					!o->merge) {
				shipcount = 0;
				forlist(&(s->object->ships)) {
					sh = (Item *) elem;
					shipcount += sh->num;
				}
				if (shipcount <= amt) {
					forlist(&(s->object->units)) {
						p = (Unit *) elem;
						if ((!p->CanSwim() || p->GetFlag(FLAG_NOCROSS_WATER))) {
							u->Error(ord + ": Can't give away our last ship in the ocean.");
							return 0;
						}
					}
				}
			}

			// give into existing fleet or form new fleet?
			newfleet = 0;

			// target is not in fleet or not fleet owner
			if (!(t->object->IsFleet()) ||
				(t->num != t->object->GetOwner()->num)) newfleet = 1;

			// Set fleet variable to target fleet
			if (newfleet == 1) {
				// create a new fleet
				fleet = new Object(r);
				fleet->type = O_FLEET;
				fleet->num = shipseq++;
				fleet->name = new AString(AString("Fleet [") + fleet->num + "]");
				t->object->region->AddFleet(fleet);
				t->MoveUnit(fleet);
			}
			else {
				fleet = t->object;
			}

			if (ItemDefs[o->item].max_inventory) {
				cur = t->object->GetNumShips(o->item) + amt;
				if (cur > ItemDefs[o->item].max_inventory) {
					u->Error(ord + ": Fleets cannot have more than " +
						ItemString(o->item, ItemDefs[o->item].max_inventory) +
						".");
					return 0;
				}
			}

			// Check if fleets are compatible
			if ((Globals->PREVENT_SAIL_THROUGH) &&
					(!Globals->ALLOW_TRIVIAL_PORTAGE) &&
					// flying ships are always transferrable
					(ItemDefs[o->item].fly == 0)) {
				// if target fleet had not sailed, just copy shore from source
				if (fleet->prevdir == -1) {
					fleet->prevdir = s->object->prevdir;
				} else {
					// check that source ship is compatible with its new fleet
					if (s->object->SailThroughCheck(fleet->prevdir) == 0) {
						u->Error(ord +
								": Ships cannot be transferred through land.");
						return 0;
					}
				}
			}

			s->Event(AString("Transfers ") + ItemString(o->item, amt) + " to " +
				*t->object->name + ".");
			if (s->faction != t->faction) {
				t->Event(AString("Receives ") + ItemString(o->item, amt) +
					" from " + *s->object->name + ".");
			}
			s->object->SetNumShips(o->item, s->object->GetNumShips(o->item)-amt);
			t->object->SetNumShips(o->item, t->object->GetNumShips(o->item)+amt);
			t->faction->DiscoverItem(o->item, 0, 1);
		}
		return 0;
	}
	
	if (o->target->unitnum == -1) {
		/* Give 0 */
		// Check there is enough to give
		amt = o->amount;
		if (amt != -2 && amt > u->GetSharedNum(o->item)) {
			u->Error(ord + ": Not enough.");
			amt = u->GetSharedNum(o->item);
		} else if (amt == -2) {
			amt = u->items.GetNum(o->item);
			if (o->except) {
				if (o->except > amt) {
					amt = 0;
					u->Error("GIVE: EXCEPT value greater than amount on hand.");
				} else {
					amt = amt - o->except;
				}
			}
		}
		if (amt == -1) {
			u->Error("Can't discard a whole unit.");
			return 0;
		}

		if (amt < 0) {
			u->Error("Cannot give a negative number of items.");
			return 0;
		}

		temp = "Discards ";
		if (ItemDefs[o->item].type & IT_MAN) {
			u->SetMen(o->item, u->GetMen(o->item) - amt);
			r->DisbandInRegion(o->item, amt);
			temp = "Disbands ";
		} else if (Globals->RELEASE_MONSTERS &&
				(ItemDefs[o->item].type & IT_MONSTER)) {
			temp = "Releases ";
			u->items.SetNum(o->item, u->items.GetNum(o->item) - amt);
			if (Globals->WANDERING_MONSTERS_EXIST) {
				Faction *mfac = GetFaction(&factions, monfaction);
				Unit *mon = GetNewUnit(mfac, 0);
				MonType *mp = FindMonster(ItemDefs[o->item].abr,
						(ItemDefs[o->item].type & IT_ILLUSION));
				mon->MakeWMon(mp->name, o->item, amt);
				mon->MoveUnit(r->GetDummy());
				// This will result in 0 unless MONSTER_NO_SPOILS or
				// MONSTER_SPOILS_RECOVERY are set.
				mon->free = Globals->MONSTER_NO_SPOILS +
					Globals->MONSTER_SPOILS_RECOVERY;
			}
		} else {
			u->ConsumeShared(o->item, amt);
		}
		u->Event(temp + ItemString(o->item, amt) + ".");
		return 0;
	}

	amt = o->amount;
	t = r->GetUnitId(o->target, u->faction->num);
	if (!t) {
		u->Error(ord + ": Nonexistant target (" +
				o->target->Print() + ").");
		return 0;
	} else if (o->type == O_TAKE && u->faction != t->faction) {
		u->Error(ord + ": " + o->target->Print() +
				" is not a member of your faction.");
		return 0;
	} else if (u->faction != t->faction && t->faction->IsNPC()) {
		u->Error(ord + ": Can't give to non-player unit (" +
				o->target->Print() + ").");
		return 0;
	}
	if (amt == -1 && u->faction == t->faction) {
		u->Error(ord + ": Unit already belongs to our faction!");
		return 0;
	}
	if (u == t) {
		if (o->type == O_TAKE)
			u->Error(ord + ": Attempt to take " + 
					ItemString(o->item, amt) +
					" from self.");
		else
			u->Error(ord + ": Attempt to give " + 
					ItemString(o->item, amt) +
					" to self.");
		return 0;
	}
	// New RULE -- Must be able to see unit to give something to them!
	if (!u->CanSee(r, t) &&
			(t->faction->GetAttitude(u->faction->num) < A_FRIENDLY)) {
		u->Error(ord + ": Nonexistant target (" +
				o->target->Print() + ").");
		return 0;
	}

	s = u;
	if (o->type == O_TAKE) {
		s = t;
		t = u;
	}

	// Check there is enough to give
	if (amt != -2 && amt > s->GetSharedNum(o->item)) {
		u->Error(ord + ": Not enough.");
		amt = s->GetSharedNum(o->item);
	} else if (amt == -2) {
		amt = s->items.GetNum(o->item);
		if (o->except) {
			if (o->except > amt) {
				amt = 0;
				u->Error(ord + ": EXCEPT value greater than amount on hand.");
			} else {
				amt = amt - o->except;
			}
		}
	}

	if (o->item != I_SILVER &&
			t->faction->GetAttitude(s->faction->num) < A_FRIENDLY) {
		u->Error("GIVE: Target is not a member of a friendly faction.");
		return 0;
	}

	if (amt == -1) {
		/* Give unit */
		if (u->type == U_MAGE) {
			if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
				if (CountMages(t->faction) >= AllowedMages(t->faction)) {
					u->Error("GIVE: Faction has too many mages.");
					return 0;
				}
			}
		}
		if (u->type == U_APPRENTICE) {
			if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
				if (CountApprentices(t->faction) >=
						AllowedApprentices(t->faction)){
					temp = "GIVE: Faction has too many ";
					temp += Globals->APPRENTICE_NAME;
					temp += "s.";
					u->Error(temp);
					return 0;
				}
			}
		}

		if (u->GetSkill(S_QUARTERMASTER)) {
			if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
				if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
					if (CountQuarterMasters(t->faction) >=
							AllowedQuarterMasters(t->faction)) {
						u->Error("GIVE: Faction has too many quartermasters.");
						return 0;
					}
				}
			}
		}

		if (u->GetSkill(S_TACTICS) == 5) {
			if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
				if (Globals->TACTICS_NEEDS_WAR) {
					if (CountTacticians(t->faction) >=
							AllowedTacticians(t->faction)) {
						u->Error("GIVE: Faction has too many tacticians.");
						return 0;
					}
				}
			}
		}

		notallied = 1;
		if (t->faction->GetAttitude(u->faction->num) == A_ALLY) {
			notallied = 0;
		}

		u->Event(AString("Gives unit to ") + *(t->faction->name) + ".");
		u->faction = t->faction;
		u->Event("Is given to your faction.");

		if (notallied && u->monthorders && u->monthorders->type == O_MOVE &&
				((MoveOrder *) u->monthorders)->advancing) {
			u->Error("Unit cannot advance after being given.");
			delete u->monthorders;
			u->monthorders = 0;
		}

		/* Check if any new skill reports have to be shown */
		forlist(&(u->skills)) {
			skill = (Skill *) elem;
			newlvl = u->GetRealSkill(skill->type);
			oldlvl = u->faction->skills.GetDays(skill->type);
			if (newlvl > oldlvl) {
				for (int i=oldlvl+1; i<=newlvl; i++) {
					u->faction->shows.Add(new ShowSkill(skill->type, i));
				}
				u->faction->skills.SetDays(skill->type, newlvl);
			}
		}

		// Okay, now for each item that the unit has, tell the new faction
		// about it in case they don't know about it yet.
		forlist_reuse(&u->items) {
			it = (Item *)elem;
			u->faction->DiscoverItem(it->type, 0, 1);
		}

		return notallied;
	}

	/* If the item to be given is a man, combine skills */
	if (ItemDefs[o->item].type & IT_MAN) {
		if (s->type == U_MAGE || s->type == U_APPRENTICE ||
				t->type == U_MAGE || t->type == U_APPRENTICE) {
			u->Error(ord + ": Magicians can't transfer men.");
			return 0;
		}
		if (Globals->TACTICS_NEEDS_WAR) {
			if (s->GetSkill(S_TACTICS) == 5 ||
					t->GetSkill(S_TACTICS) == 5) {
				u->Error(ord + ": Tacticians can't transfer men.");
				return 0;
			}
		}
		if (s->GetSkill(S_QUARTERMASTER) > 0 || t->GetSkill(S_QUARTERMASTER) > 0) {
			u->Error(ord + ": Quartermasters can't transfer men.");
			return 0;
		}

		if ((ItemDefs[o->item].type & IT_LEADER) && t->IsNormal()) {
			u->Error(ord + ": Can't mix leaders and normal men.");
			return 0;
		} else {
			if (!(ItemDefs[o->item].type & IT_LEADER) && t->IsLeader()) {
				u->Error(ord + ": Can't mix leaders and normal men.");
				return 0;
			}
		}
		// Small hack for Ceran
		if (o->item == I_MERC && t->GetMen()) {
			u->Error("GIVE: Can't mix mercenaries with other men.");
			return 0;
		}

		if (u->faction != t->faction) {
			u->Error(ord + ": Can't give men to another faction.");
			return 0;
		}

		if (s->nomove) t->nomove = 1;

		skills = s->skills.Split(s->GetMen(), amt);
		t->skills.Combine(skills);
		delete skills;
	}

	if (ItemDefs[o->item].flags & ItemType::CANTGIVE) {
		u->Error(ord + ": Can't give " + ItemDefs[o->item].names + ".");
		return 0;
	}

	if (ItemDefs[o->item].max_inventory) {
		cur = t->items.GetNum(o->item) + amt;
		if (cur > ItemDefs[o->item].max_inventory) {
			u->Error(ord + ": Unit cannot have more than "+
					ItemString(o->item, ItemDefs[o->item].max_inventory));
			return 0;
		}
	}

	if (o->type == O_TAKE) {
		u->Event(AString("Takes ") + ItemString(o->item, amt) +
				" from " + *s->name + ".");
	} else {
		u->Event(AString("Gives ") + ItemString(o->item, amt) + " to " +
				*t->name + ".");
		if (s->faction != t->faction) {
			t->Event(AString("Receives ") + ItemString(o->item, amt) +
					" from " + *s->name + ".");
		}
	}
	s->ConsumeShared(o->item, amt);
	t->items.SetNum(o->item, t->items.GetNum(o->item) + amt);
	t->faction->DiscoverItem(o->item, 0, 1);

	if (ItemDefs[o->item].type & IT_MAN) {
		t->AdjustSkills();
	}
	return 0;
}

void Game::DoGuard1Orders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if (u->guard == GUARD_SET || u->guard == GUARD_GUARD) {
					if (!u->Taxers(1)) {
						u->guard = GUARD_NONE;
						u->Error("Must be combat ready to be on guard.");
						continue;
					}
					if (u->type != U_GUARD && r->HasCityGuard()) {
						u->guard = GUARD_NONE;
						u->Error("Is prevented from guarding by the "
								"Guardsmen.");
						continue;
					}
					u->guard = GUARD_GUARD;
				}
			}
		}
	}
}

void Game::FindDeadFactions()
{
	forlist((&factions)) {
		((Faction *) elem)->CheckExist(&regions);
	}
}

void Game::DeleteEmptyUnits()
{
	forlist((&regions)) {
		ARegion *region = (ARegion *) elem;
		DeleteEmptyInRegion(region);
	}
}

void Game::DeleteEmptyInRegion(ARegion *region)
{
	forlist(&region->objects) {
		Object *obj = (Object *) elem;
		forlist (&obj->units) {
			Unit *unit = (Unit *) elem;
			if (unit->IsAlive() == 0) {
				region->Kill(unit);
			}
		}
	}
}

void Game::CheckTransportOrders()
{
	if (!(Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT))
		return;

	forlist ((&regions)) {
		ARegion *r = (ARegion *)elem;
		forlist((&r->objects)) {
			Object *obj = (Object *)elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *)elem;
				forlist ((&u->transportorders)) {
					// make sure target exists
					TransportOrder *o = (TransportOrder *)elem;
					AString ordertype =
						(o->type == O_DISTRIBUTE) ? "DISTRIBUTE" :
						"TRANSPORT";
					if (!o->target || o->target->unitnum == -1) {
						u->Error(ordertype + ": Target does not exist.");
						o->type = NORDERS;
						continue;
					}

					Location *tar = regions.GetUnitId(o->target,
							u->faction->num, r);
					if (!tar) {
						u->Error(ordertype + ": Target does not exist.");
						o->type = NORDERS;
						continue;
					}

					// Make sure target isn't self
					if (tar->unit == u) {
						u->Error(ordertype + ": Target is self.");
						o->type = NORDERS;
						continue;
					}

					// Make sure the target and unit are at least friendly
					if (tar->unit->faction->GetAttitude(u->faction->num) <
							A_FRIENDLY) {
						u->Error(ordertype +
								": Target is not a member of a friendly "
								"faction.");
						o->type = NORDERS;
						continue;
					}

					// Make sure the target of a transport order is a unit
					// with the quartermaster skill who owns a transport
					// structure
					if (o->type == O_TRANSPORT) {
						if (tar->unit->GetSkill(S_QUARTERMASTER) == 0) {
							u->Error(ordertype +
									": Target is not a quartermaster");
							o->type = NORDERS;
							continue;
						}
						if ((tar->obj->GetOwner() != tar->unit) ||
								!(ObjectDefs[tar->obj->type].flags &
									ObjectType::TRANSPORT) ||
								(tar->obj->incomplete > 0)) {
							u->Error(ordertype + ": Target does not own "
									"a transport structure.");
							o->type = NORDERS;
							continue;
						}
					}

					// make sure target is in range.
					int dist, maxdist;
					if ((o->type == O_TRANSPORT) &&
						(u == obj->GetOwner()) &&
						(ObjectDefs[obj->type].flags & ObjectType::TRANSPORT)) {
						maxdist = Globals->NONLOCAL_TRANSPORT;
						if (maxdist > 0 &&
							Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST) {
							int level = u->GetSkill(S_QUARTERMASTER);
							maxdist += ((level + 1)/3);
						}
					} else
						maxdist = Globals->LOCAL_TRANSPORT;
					int penalty = 10000000;
					RangeType *rt = FindRange("rng_transport");
					if (rt) penalty = rt->crossLevelPenalty;
					if (maxdist > 0) {
						// 0 maxdist represents unlimited range
						dist = regions.GetPlanarDistance(r, tar->region, penalty, maxdist);
						if (dist > maxdist) {
							u->Error(ordertype + ": Recipient is too far away.");
							o->type = NORDERS;
							continue;
						}
					} else {
						dist = regions.GetPlanarDistance(r, tar->region, penalty, Globals->LOCAL_TRANSPORT);
					}

					// On long range transport or distribute, make sure the
					// issuer is a quartermaster and is owner of a structure
					if ((o->type == O_DISTRIBUTE) ||
						((dist > Globals->LOCAL_TRANSPORT) &&
						 (o->type == O_TRANSPORT))) {
						if (u->GetSkill(S_QUARTERMASTER == 0)) {
							u->Error(ordertype +
									": Unit is not a quartermaster");
							o->type = NORDERS;
							continue;
						}
						if ((u != obj->GetOwner()) ||
							!(ObjectDefs[obj->type].flags &
								ObjectType::TRANSPORT) ||
							(obj->incomplete > 0)) {
						u->Error(ordertype +
								": Unit does not own transport structure.");
						o->type = NORDERS;
						continue;
						}
					}

					// make sure amount is available (all handled later)
					if (o->amount > 0 && o->amount > u->GetSharedNum(o->item)) {
						u->Error(ordertype + ": Not enough.");
						o->type = NORDERS;
						continue;
					}

					if (o->amount > 0 && ItemDefs[o->item].max_inventory) {
						int cur = tar->unit->items.GetNum(o->item) + o->amount;
						if (cur > ItemDefs[o->item].max_inventory) {
							u->Error(ordertype +
									": Target cannot have more than " +
									ItemString(o->item,
										ItemDefs[o->item].max_inventory));
							o->type = NORDERS;
							continue;
						}
					}

					// Check if we have a trade hex
					if (!TradeCheck(r, u->faction)) {
						u->Error(ordertype + ": Faction cannot transport or "
								"distribute in that many hexes.");
						o->type = NORDERS;
						continue;
					}
				}
			}
		}
	}
}

void Game::RunTransportOrders()
{
	if (!(Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT))
		return;
	forlist ((&regions)) {
		ARegion *r = (ARegion *)elem;
		forlist((&r->objects)) {
			Object *obj = (Object *)elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *)elem;
				forlist ((&u->transportorders)) {
					TransportOrder *t = (TransportOrder *)elem;
					if (t->type != O_TRANSPORT && t->type != O_DISTRIBUTE)
						continue;
					Location *tar = regions.GetUnitId(t->target,
							u->faction->num, r);
					if (!tar) continue;
					AString ordertype = (t->type == O_TRANSPORT) ?
						"TRANSPORT" : "DISTRIBUTE";

					int amt = t->amount;
					if (amt < 0) {
						amt = u->items.GetNum(t->item);
						if (t->except) {
							if (t->except > amt) {
								amt = 0;
								u->Error(ordertype +
										": EXCEPT value greater than amount "
										"on hand.");
							} else {
								amt = amt - t->except;
							}
						}
					} else if (amt > u->GetSharedNum(t->item)) {
						u->Error(ordertype + ": Not enough.");
						amt = u->GetSharedNum(t->item);
					}

					if (ItemDefs[t->item].max_inventory) {
						int cur = tar->unit->items.GetNum(t->item) + amt;
						if (cur > ItemDefs[t->item].max_inventory) {
							u->Error(ordertype +
									": Target cannot have more than " +
									ItemString(t->item,
										ItemDefs[t->item].max_inventory));
							continue;
						}
					}

					u->ConsumeShared(t->item, amt);
					// now see if the unit can pay for shipping
					int penalty = 10000000;
					RangeType *rt = FindRange("rng_transport");
					if (rt) penalty = rt->crossLevelPenalty;
					int dist = regions.GetPlanarDistance(r, tar->region, penalty, Globals->LOCAL_TRANSPORT);
					int weight = ItemDefs[t->item].weight * amt;
					if (weight == 0 && Globals->FRACTIONAL_WEIGHT > 0)
						weight = (amt/Globals->FRACTIONAL_WEIGHT) + 1;
					int cost = 0;
					if (dist > Globals->LOCAL_TRANSPORT) {
						cost = Globals->SHIPPING_COST * weight;
						if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST)
							cost *= (4 - ((u->GetSkill(S_QUARTERMASTER)+1)/2));
					}

					// if not, give it back
					if (cost > u->GetSharedMoney()) {
						u->Error(ordertype + ": Cannot afford shipping cost.");
						u->items.SetNum(t->item, u->items.GetNum(t->item)+amt);
						continue;
					}
					u->ConsumeSharedMoney(cost);

					ordertype = (t->type == O_TRANSPORT) ?
						"Transports " : "Distributes ";
					u->Event(ordertype + ItemString(t->item, amt) + " to " +
							*tar->unit->name + " for $" + cost + ".");
					if (u->faction != tar->unit->faction) {
						tar->unit->Event(AString("Recieves ") +
								ItemString(t->item, amt) + " from " +
								*u->name + ".");
					}

					tar->unit->items.SetNum(t->item,
							tar->unit->items.GetNum(t->item) + amt);
					tar->unit->faction->DiscoverItem(t->item, 0, 1);

					u->Practice(S_QUARTERMASTER);

				}
				u->transportorders.DeleteAll();
			}
		}
	}
}

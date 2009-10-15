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
// Date        Person          Comment
// ----        ------          -------
// 2001/Feb/18 Joseph Traub    Added apprentice support from Lacandon Conquest
// 2001/Feb/21 Joseph Traub    Added FACLIM_UNLIMITED
//
#include "game.h"
#include "gamedata.h"

void Game::RunOrders()
{
	//
	// Form and instant orders are handled during parsing
	//
	Awrite("Running FIND Orders...");
	RunFindOrders();
	Awrite("Running ENTER/LEAVE Orders...");
	RunEnterOrders();
	Awrite("Running PROMOTE/EVICT Orders...");
	RunPromoteOrders();
	Awrite("Running Combat...");
	DoAttackOrders();
	DoAutoAttacks();
	Awrite("Running STEAL/ASSASSINATE Orders...");
	RunStealOrders();
	Awrite("Running GIVE/PAY/TRANSFER Orders...");
	DoGiveOrders();
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
	SinkUncrewedShips();
	DrownUnits();
	if(Globals->ALLOW_WITHDRAW) {
		Awrite("Running WITHDRAW Orders...");
		DoWithdrawOrders();
	}
	Awrite("Running Sail Orders...");
	RunSailOrders();
	Awrite("Running Move Orders...");
	RunMoveOrders();
	SinkUncrewedShips();
	DrownUnits();
	FindDeadFactions();
	Awrite("Running Teach Orders...");
	RunTeachOrders();
	Awrite("Running Month-long Orders...");
	RunMonthOrders();
	RunTeleportOrders();
	Awrite("Assessing Maintenance costs...");
	AssessMaintenance();
	Awrite("Post-Turn Processing...");
	PostProcessTurn();
	DeleteEmptyUnits();
	RemoveEmptyObjects();
}

void Game::ClearCastEffects()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			forlist(&o->units) {
				Unit * u = (Unit *) elem;
				u->SetFlag(FLAG_INVIS,0);
			}
		}
	}
}

void Game::RunCastOrders()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			forlist(&o->units) {
				Unit * u = (Unit *) elem;
				if (u->castorders)
				{
					RunACastOrder(r,o,u);
					delete u->castorders;
					u->castorders = 0;
				}
			}
		}
  }
}

int Game::CountMages( Faction *pFac )
{
	int i = 0;
	forlist( &regions ) {
		ARegion * r = (ARegion *) elem;
		forlist( &r->objects ) {
			Object * o = (Object *) elem;
			forlist(&o->units) {
				Unit * u = (Unit *) elem;
				if (u->faction == pFac && u->type == U_MAGE) i++;
			}
		}
	}
	return( i );
}

int Game::TaxCheck( ARegion *pReg, Faction *pFac )
{
	if( Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES )
	{
		if( AllowedTaxes( pFac ) == -1 )
		{
			//
			// No limit.
			//
			return( 1 );
		}

		forlist( &( pFac->war_regions )) {
			ARegion *x = ((ARegionPtr *) elem)->ptr;
			if( x == pReg )
			{
				//
				// This faction already performed a tax action in this
				// region.
				//
				return 1;
			}
		}
		if( pFac->war_regions.Num() >= AllowedTaxes( pFac ))
		{
			//
			// Can't tax here.
			//
			return 0;
		}
		else
		{
			//
			// Add this region to the faction's tax list.
			//
			ARegionPtr *y = new ARegionPtr;
			y->ptr = pReg;
			pFac->war_regions.Add(y);
			return 1;
		}
	}
	else
	{
		//
		// No limit on taxing regions in this game.
		//
		return( 1 );
	}
}

int Game::TradeCheck( ARegion *pReg, Faction *pFac )
{
	if( Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES )
	{
		if( AllowedTrades( pFac ) == -1 )
		{
			//
			// No limit on trading on this faction.
			//
			return( 1 );
		}

		forlist( &( pFac->trade_regions )) {
			ARegion * x = ((ARegionPtr *) elem)->ptr;
			if (x == pReg )
			{
				//
				// This faction has already performed a trade action in this
				// region.
				//
				return 1;
			}
		}
		if ( pFac->trade_regions.Num() >= AllowedTrades( pFac ))
		{
			//
			// This faction is over its trade limit.
			//
			return 0;
		}
		else
		{
			//
			// Add this region to the faction's trade list, and return 1.
			//
			ARegionPtr * y = new ARegionPtr;
			y->ptr = pReg;
			pFac->trade_regions.Add(y);
			return 1;
		}
	}
	else
	{
		//
		// No limit on trade in this game.
		//
		return( 1 );
	}
}

void Game::RunStealOrders()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			forlist_safe(&o->units) {
				Unit * u = (Unit *) elem;
				if (u->stealorders) {
					if (u->stealorders->type == O_STEAL) {
						Do1Steal(r,o,u);
					} else if (u->stealorders->type == O_ASSASSINATE) {
						Do1Assassinate(r,o,u);
					}
					delete u->stealorders;
					u->stealorders = 0;
				}
			}
		}
	}
}

AList * Game::CanSeeSteal(ARegion * r,Unit * u)
{
	AList * retval = new AList;
	forlist(&factions) {
		Faction * f = (Faction *) elem;
		if (r->Present(f)) {
			if (f->CanSee(r,u, Globals->SKILL_PRACTISE_AMOUNT > 0)) {
				FactionPtr * p = new FactionPtr;
				p->ptr = f;
				retval->Add(p);
			}
		}
	}
	return retval;
}

void Game::Do1Assassinate(ARegion * r,Object * o,Unit * u)
{
	AssassinateOrder * so = (AssassinateOrder *) u->stealorders;
	Unit * tar = r->GetUnitId(so->target,u->faction->num);

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

	AList * seers = CanSeeSteal(r,u);
	int succ = 1;
	forlist(seers) {
		Faction * f = ((FactionPtr *) elem)->ptr;
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
			Faction * f = ((FactionPtr *) elem)->ptr;
			f->Event(temp);
		}
		// One learns from one's mistakes.  Surviving them is another matter!
		u->Practise(S_STEALTH);
		return;
	}

	int ass = 1;
	if(u->items.GetNum(I_RINGOFI)) {
		ass = 2; // Check if assassin has a ring.
		// New rule: if a target has an amulet of true seeing they
		// cannot be assassinated by someone with a ring of invisibility
		if(tar->AmtsPreventCrime(u)) {
			tar->Event( "Assassination prevented by amulet of true seeing." );
			u->Event( AString( "Attempts to assassinate " ) + *(tar->name) +
					", but is prevented by amulet of true seeing." );
			return;
		}
	}
	u->Practise(S_STEALTH);
	RunBattle(r,u,tar,ass);
}

void Game::Do1Steal(ARegion * r,Object * o,Unit * u)
{
	StealOrder * so = (StealOrder *) u->stealorders;
	Unit * tar = r->GetUnitId(so->target,u->faction->num);

	if (!tar) {
		u->Error("STEAL: Invalid unit given.");
		return;
	}

	// New RULE!! You can only steal from someone you can see.
	if(!u->CanSee(r, tar)) {
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

	AList * seers = CanSeeSteal(r,u);
	int succ = 1;
	{
		forlist(seers) {
			Faction * f = ((FactionPtr *) elem)->ptr;
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
	}

	if (!succ) {
		AString temp = *(u->name) + " is caught attempting to steal from " +
			*(tar->name) + " in " + *(r->name) + ".";
		forlist(seers) {
			Faction * f = ((FactionPtr *) elem)->ptr;
			f->Event(temp);
		}
		// One learns from one's mistakes.  Surviving them is another matter!
		u->Practise(S_STEALTH);
		return;
	}

	//
	// New rule; if a target has an amulet of true seeing they can't be
	// stolen from by someone with a ring of invisibility
	//
	if(tar->AmtsPreventCrime(u)) {
		tar->Event( "Theft prevented by amulet of true seeing." );
		u->Event( AString( "Attempts to steal from " ) + *(tar->name) + ", but "
				"is prevented by amulet of true seeing." );
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

	if (tar->items.GetNum(so->item) < amt) {
		amt = 0;
	}

	u->items.SetNum(so->item,u->items.GetNum(so->item) + amt);
	tar->items.SetNum(so->item,tar->items.GetNum(so->item) - amt);

	{
		AString temp = *(u->name) + " steals " +
			ItemString(so->item,amt) + " from " + *(tar->name) + ".";
		forlist(seers) {
			Faction * f = ((FactionPtr *) elem)->ptr;
			f->Event(temp);
		}
	}

	tar->Event(AString("Has ") + ItemString(so->item,amt) + " stolen.");
	u->Practise(S_STEALTH);
	return;
}

void Game::DrownUnits()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		if (TerrainDefs[r->type].similar_type == R_OCEAN) {
			forlist(&r->objects) {
				Object * o = (Object *) elem;
				if(o->type != O_DUMMY) continue;
				forlist(&o->units) {
					Unit *u = (Unit *)elem;
					int drown = 0;
					if(u->type == U_WMON) {
						/*
						 * Make sure flying monsters only drown if we
						 * are in WFLIGHT_NONE mode
						 */
						if(Globals->FLIGHT_OVER_WATER==GameDefs::WFLIGHT_NONE)
							drown = 1;
						else
							drown = !(u->CanSwim());
					} else {
						switch(Globals->FLIGHT_OVER_WATER) {
							case GameDefs::WFLIGHT_UNLIMITED:
								drown = !(u->CanSwim());
								break;
							case GameDefs::WFLIGHT_MUST_LAND:
								drown = !(u->CanReallySwim() || u->leftShip);
								u->leftShip = 0;
								break;
							case GameDefs::WFLIGHT_NONE:
								drown = !(u->CanReallySwim());
								break;
							default: // Should never happen
								drown = 1;
								break;
						}
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

void Game::SinkUncrewedShips()
{
	Unit *u;
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		if (TerrainDefs[r->type].similar_type == R_OCEAN) {
			forlist(&r->objects) {
				Object * o = (Object *) elem;
				if(!o->IsBoat()) continue;

				int men = 0;
				forlist(&o->units) {
					u = (Unit *)elem;
					men += u->GetMen();
				}

				if(men <= 0) {
					/* No men onboard, move all units out to ocean */
					forlist(&o->units) {
						u = (Unit *)elem;
						u->MoveUnit(r->GetDummy());
					}
					/* And sink the boat */
					r->objects.Remove(o);
					delete o;
				}
			}
		}
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
		Faction * f = (Faction *) elem;
		if (f->quit)
			Do1Quit(f);
	}
}

void Game::Do1Quit(Faction * f)
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			forlist(&o->units) {
				Unit * u = (Unit *) elem;
				if (u->faction == f)
				{
					o->units.Remove(u);
					delete u;
				}
			}
		}
	}
}

void Game::RunDestroyOrders()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			Unit * u = o->GetOwner();
			if (u)
			{
				if (u->destroy)
				{
					Do1Destroy(r,o,u);
					continue;
				}
				else
				{
					forlist(&o->units)
						((Unit *) elem)->destroy = 0;
				}
			}
		}
	}
}

void Game::Do1Destroy(ARegion * r,Object * o,Unit * u) {
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
		Object * dest = r->GetDummy();
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			u->destroy = 0;
			u->MoveUnit( dest );
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
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			forlist(&o->units) {
				Unit * u = (Unit *) elem;
				RunFindUnit(u);
			}
		}
	}
}

void Game::RunFindUnit(Unit * u)
{
	int all = 0;
	Faction *fac;
	forlist(&u->findorders) {
		FindOrder * f = (FindOrder *) elem;
		if(f->find == 0) all = 1;
		if(!all) {
			fac = GetFaction(&factions,f->find);
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
				if(fac) {
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

int Game::CountTaxers(ARegion * reg)
{
	int t = 0;
	forlist(&reg->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->GetFlag(FLAG_AUTOTAX) && !Globals->TAX_PILLAGE_MONTH_LONG)
				u->taxing = TAX_TAX;
			if(u->taxing == TAX_AUTO) u->taxing = TAX_TAX;

			if (u->taxing == TAX_TAX) {
				if (!reg->CanTax(u)) {
					u->Error("TAX: A unit is on guard.");
					u->taxing = TAX_NONE;
				} else {
					int men = u->Taxers();
					if (men) {
						if( !TaxCheck(reg, u->faction )) {
							u->Error( "TAX: Faction can't tax that many "
									"regions.");
							u->taxing = TAX_NONE;
						} else {
							t += men;
						}
					} else {
						u->Error("TAX: Unit cannot tax.");
						u->taxing = TAX_NONE;
						u->SetFlag(FLAG_AUTOTAX,0);
					}
				}
			}
		}
	}
	return t;
}

void Game::RunTaxRegion(ARegion * reg)
{
	int taxers = CountTaxers(reg);
	int desired = taxers * Globals->TAX_INCOME;
	if (desired < reg->money) desired = reg->money;

	forlist(&reg->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->taxing == TAX_TAX) {
				int t = u->Taxers();
				double fAmt = ((double) t) * ((double) Globals->TAX_INCOME ) *
					((double) reg->money ) / ((double) desired );
				int amt = (int) fAmt;
				reg->money -= amt;
				desired -= t * Globals->TAX_INCOME;
				u->SetMoney(u->GetMoney() + amt);
				u->Event(AString("Collects $") + amt + " in taxes in " +
						reg->ShortPrint( &regions ) + ".");
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

int Game::CountPillagers(ARegion * reg)
{
	int p = 0;
	forlist(&reg->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->taxing == TAX_PILLAGE) {
				if (!reg->CanPillage(u)) {
					u->Error("PILLAGE: A unit is on guard.");
					u->taxing = TAX_NONE;
				} else {
					int men = u->Taxers();
					if (men) {
						if(!TaxCheck(reg, u->faction)) {
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

void Game::ClearPillagers(ARegion * reg)
{
	forlist(&reg->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->taxing == TAX_PILLAGE) {
				u->Error("PILLAGE: Not enough men to pillage.");
				u->taxing = TAX_NONE;
			}
		}
	}
}

void Game::RunPillageRegion(ARegion * reg)
{
	if (TerrainDefs[reg->type].similar_type == R_OCEAN) return;
	if (reg->money < 1) return;
	if (reg->Wages() < 11) return;

	/* First, count up pillagers */
	int pillagers = CountPillagers(reg);

	if (pillagers * 2 * Globals->TAX_INCOME < reg->money) {
		ClearPillagers(reg);
		return;
	}

	AList * facs = reg->PresentFactions();
	int amt = reg->money * 2;
	forlist(&reg->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->taxing == TAX_PILLAGE) {
				u->taxing = TAX_NONE;
				int num = u->Taxers();
				int temp = (amt * num)/pillagers;
				amt -= temp;
				pillagers -= num;
				u->SetMoney(u->GetMoney() + temp);
				u->Event(AString("Pillages $") + temp + " from " +
						reg->ShortPrint( &regions ) + ".");
				forlist(facs) {
					Faction * fp = ((FactionPtr *) elem)->ptr;
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
	reg->money = 0;
	reg->wages -= 6;
	if (reg->wages < 6) reg->wages = 6;
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
				if(u && u->promote) {
					Do1PromoteOrder(o,u);
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
						}
						else
						{
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
						}
						else
						{
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
	Unit *tar = obj->GetUnitId(u->promote,u->faction->num);
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
	Object *to = obj->region->GetDummy();

	while (ord && ord->targets.Num()) {
		UnitId *id = (UnitId *)ord->targets.First();
		ord->targets.Remove(id);
		Unit *tar = obj->GetUnitId(id, u->faction->num);
		delete id;
		if (!tar) continue;

		if(obj->IsBoat() &&
				(TerrainDefs[obj->region->type].similar_type == R_OCEAN) &&
				(!tar->CanReallySwim() || tar->GetFlag(FLAG_NOCROSS_WATER))) {
			u->Error("EVICT: Cannot forcibly evict units over ocean.");
			continue;
		}
		tar->MoveUnit(to);
		tar->Event(AString("Evicted from ") + *obj->name + " by " + *u->name);
		u->Event(AString("Evicted ") + *tar->name + " from " + *obj->name);
	}
}

void Game::RunEnterOrders()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			forlist(&o->units) {
				Unit * u = (Unit *) elem;
				if (u->enter)
					Do1EnterOrder(r,o,u);
			}
		}
	}
}

void Game::Do1EnterOrder(ARegion * r,Object * in,Unit * u)
{
	Object * to;
	if (u->enter == -1) {
		to = r->GetDummy();
		u->enter = 0;
		if((TerrainDefs[r->type].similar_type == R_OCEAN) &&
				(!u->CanSwim() || u->GetFlag(FLAG_NOCROSS_WATER))) {
			u->Error("LEAVE: Can't leave a ship in the ocean.");
			return;
		}
		if (in->IsBoat() && u->CanSwim()) u->leftShip = 1;
	} else {
		to = r->GetObject(u->enter);
		u->enter = 0;
		if (!to) {
			u->Error("ENTER: Can't enter that.");
			return;
		}
		if (!to->CanEnter(r,u)) {
			u->Error("ENTER: Can't enter that.");
			return;
		}
		if (to->ForbiddenBy(r, u)) {
			u->Error("ENTER: Is refused entry.");
			return;
		}
	}
	u->MoveUnit( to );
}

void Game::RemoveEmptyObjects()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			if (ObjectDefs[o->type].cost &&
					o->incomplete >= ObjectDefs[o->type].cost) {
				forlist(&o->units) {
					Unit * u = (Unit *) elem;
					u->MoveUnit( r->GetDummy() );
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

void Game::PostProcessUnit(ARegion *r,Unit *u)
{
	PostProcessUnitExtra(r, u);
}

void Game::EndGame(Faction *pVictor)
{
	forlist( &factions ) {
		Faction *pFac = (Faction *) elem;
		pFac->exists = 0;
		if(pFac == pVictor)
			pFac->quit = QUIT_WON_GAME;
		else
			pFac->quit = QUIT_GAME_OVER;

		if(pVictor)
			pFac->Event( *( pVictor->name ) + " has won the game!" );
		else
			pFac->Event( "The game has ended with no winner." );
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

void Game::PostProcessTurn()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		r->PostTurn(&regions);

		if(Globals->CITY_MONSTERS_EXIST && (r->town || r->type == R_NEXUS))
			AdjustCityMons( r );

		forlist (&r->objects) {
			Object *o = (Object *) elem;
			forlist (&o->units) {
				Unit *u = (Unit *) elem;
				PostProcessUnit(r,u);
			}
		}
	}

	if(Globals->WANDERING_MONSTERS_EXIST) GrowWMons(Globals->WMON_FREQUENCY);

	if(Globals->LAIR_MONSTERS_EXIST) GrowLMons(Globals->LAIR_FREQUENCY);

	if(Globals->LAIR_MONSTERS_EXIST) GrowVMons();

	//
	// Check if there are any factions left.
	//
	int livingFacs = 0;
	{
		forlist(&factions) {
			Faction *pFac = (Faction *) elem;
			if(pFac->exists) {
				livingFacs = 1;
				break;
			}
		}
	}

	if(!livingFacs)
		EndGame(0);
	else if(!(Globals->OPEN_ENDED)) {
		Faction *pVictor = CheckVictory();
		if(pVictor)
			EndGame( pVictor );
	}
}

void Game::DoAutoAttacks()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		DoAutoAttacksRegion(r);
	}
}

void Game::DoAutoAttacksRegion(ARegion * r)
{
	forlist(&r->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->IsAlive() && u->canattack) DoAutoAttack(r,u);
		}
	}
}

void Game::DoAdvanceAttacks(AList * locs)
{
	forlist(locs) {
		Location * l = (Location *) elem;
		Unit * u = l->unit;
		ARegion * r = l->region;
		if (u->IsAlive() && u->canattack) {
			DoAutoAttack(r,u);
			if(!u->IsAlive() || !u->canattack) {
				u->guard = GUARD_NONE;
			}
		}
		if (u->IsAlive() && u->canattack && u->guard == GUARD_ADVANCE) {
			DoAdvanceAttack(r,u);
			u->guard = GUARD_NONE;
		}
		if (u->IsAlive()) {
			DoAutoAttackOn(r,u);
			if(!u->IsAlive() || !u->canattack) {
				u->guard = GUARD_NONE;
			}
		}
	}
}

void Game::DoAutoAttackOn(ARegion * r,Unit * t)
{
	forlist(&r->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->guard != GUARD_AVOID &&
					(u->GetAttitude(r,t) == A_HOSTILE) && u->IsAlive() &&
					u->canattack)
				AttemptAttack(r,u,t,1);
			if (!t->IsAlive()) return;
		}
	}
}

void Game::DoAdvanceAttack(ARegion * r,Unit * u) {
	Unit * t = r->Forbidden(u);
	while (t && u->IsAlive() && u->canattack) {
		AttemptAttack(r,u,t,1,1);
		t = r->Forbidden(u);
	}
}

void Game::DoAutoAttack(ARegion * r,Unit * u) {
  forlist(&r->objects) {
	Object * o = (Object *) elem;
	forlist(&o->units) {
	  Unit * t = (Unit *) elem;
	  if (u->guard != GUARD_AVOID && (u->GetAttitude(r,t) == A_HOSTILE)) {
	AttemptAttack(r,u,t,1);
	  }
	  if (u->IsAlive() == 0 || u->canattack == 0)
	return;
	}
  }
}

int Game::CountWMonTars(ARegion * r,Unit * mon) {
	int retval = 0;
	forlist(&r->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->type == U_NORMAL || u->type == U_MAGE ||
					u->type == U_APPRENTICE) {
				if (mon->CanSee(r,u) && mon->CanCatch(r,u)) {
					retval += u->GetMen();
				}
			}
		}
	}
	return retval;
}

Unit * Game::GetWMonTar(ARegion * r,int tarnum,Unit * mon) {
	forlist(&r->objects) {
		Object * o = (Object *) elem;
		forlist(&o->units) {
			Unit * u = (Unit *) elem;
			if (u->type == U_NORMAL || u->type == U_MAGE ||
					u->type == U_APPRENTICE) {
				if (mon->CanSee(r,u) && mon->CanCatch(r,u)) {
					int num = u->GetMen();
					if (num && tarnum < num) return u;
					tarnum -= num;
				}
			}
		}
	}
	return 0;
}

void Game::CheckWMonAttack(ARegion * r,Unit * u) {
  int tars = CountWMonTars(r,u);
  if (!tars) return;

  int rand = 300 - tars;
  if (rand < 100) rand = 100;
  if (getrandom(rand) >= u->Hostile()) return;

  Unit * t = GetWMonTar(r,getrandom(tars),u);
  if (t) AttemptAttack(r,u,t,1);
}

void Game::DoAttackOrders()
{
	forlist(&regions) {
		ARegion * r = (ARegion *) elem;
		forlist(&r->objects) {
			Object * o = (Object *) elem;
			forlist(&o->units) {
				Unit * u = (Unit *) elem;
				if (u->type == U_WMON) {
					if (u->canattack && u->IsAlive()) {
						CheckWMonAttack(r,u);
					}
				} else {
					if (u->IsAlive() && u->attackorders) {
						AttackOrder * ord = u->attackorders;
						while (ord->targets.Num()) {
							UnitId * id = (UnitId *) ord->targets.First();
							ord->targets.Remove(id);
							Unit * t = r->GetUnitId(id,u->faction->num);
							delete id;
							if (u->canattack && u->IsAlive()) {
								if (t) {
									AttemptAttack(r,u,t,0);
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

/*
 * Presume that u is alive, can attack, and wants to attack t.
 * Check that t is alive, u can see t, and u has enough riding
 * skill to catch t.
 *
 * Return 0 if success.
 * 1 if t is already dead.
 * 2 if u can't see t
 * 3 if u lacks the riding to catch t
 */
void Game::AttemptAttack(ARegion * r,Unit * u,Unit * t,int silent,int adv)
{
	if (!t->IsAlive()) return;

	if (!u->CanSee(r,t))
	{
		if (!silent) u->Error("ATTACK: Non-existent unit.");
		return;
	}

	if (!u->CanCatch(r,t))
	{
		if (!silent) u->Error("ATTACK: Can't catch that unit.");
		return;
	}

	RunBattle(r,u,t,0,adv);
	return;
}

void Game::RunSellOrders()
{
	forlist((&regions)) {
		ARegion * r = (ARegion *) elem;
		forlist((&r->markets)) {
			Market * m = (Market *) elem;
			if (m->type == M_SELL)
				DoSell(r,m);
		}
		{
			forlist((&r->objects)) {
				Object * obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit * u = (Unit *) elem;
					forlist((&u->sellorders)) {
						u->Error("SELL: Can't sell that.");
					}
					u->sellorders.DeleteAll();
				}
			}
		}
	}
}

int Game::GetSellAmount(ARegion * r,Market * m)
{
	int num = 0;
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			forlist ((&u->sellorders)) {
				SellOrder * o = (SellOrder *) elem;
				if (o->item == m->item) {
					if(o->num == -1) {
						o->num = u->items.CanSell(o->item);
					}
					if (o->num > u->items.CanSell(o->item)) {
						o->num = u->items.CanSell(o->item);
						u->Error("SELL: Unit attempted to sell more than "
								"it had.");
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

void Game::DoSell(ARegion * r,Market * m)
{
	/* First, find the number of items being sold */
	int attempted = GetSellAmount(r,m);

	if (attempted < m->amount) attempted = m->amount;
	m->activity = 0;
	int oldamount = m->amount;
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			forlist((&u->sellorders)) {
				SellOrder * o = (SellOrder *) elem;
				if (o->item == m->item) {
					int temp = 0;
					if (attempted) {
						temp = (m->amount * o->num + getrandom(attempted))
							/ attempted;
						if (temp<0) temp = 0;
					}
					attempted -= o->num;
					m->amount -= temp;
					m->activity += temp;
					u->items.SetNum(o->item,u->items.GetNum(o->item) - temp);
					u->SetMoney(u->GetMoney() + temp * m->price);
					u->sellorders.Remove(o);
					u->Event(AString("Sells ") + ItemString(o->item,temp)
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
		ARegion * r = (ARegion *) elem;
		forlist((&r->markets)) {
			Market * m = (Market *) elem;
			if (m->type == M_BUY)
				DoBuy(r,m);
		}
		{
			forlist((&r->objects)) {
				Object * obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit * u = (Unit *) elem;
					forlist((&u->buyorders)) {
						u->Error("BUY: Can't buy that.");
					}
					u->buyorders.DeleteAll();
				}
			}
		}
	}
}

int Game::GetBuyAmount(ARegion * r,Market * m)
{
	int num = 0;
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			forlist ((&u->buyorders)) {
				BuyOrder * o = (BuyOrder *) elem;
				if (o->item == m->item) {
					if (ItemDefs[o->item].type & IT_MAN) {
						if (u->type == U_MAGE) {
							u->Error("BUY: Mages can't recruit more men.");
							o->num = 0;
						}
						if(u->type == U_APPRENTICE) {
							u->Error("BUY: Apprentices can't recruit more "
									"men.");
							o->num = 0;
						}
						if ((o->item == I_LEADERS && u->IsNormal()) ||
								(o->item != I_LEADERS && u->IsLeader())) {
							u->Error("BUY: Can't mix leaders and normal men.");
							o->num = 0;
						}
					}
					if (ItemDefs[o->item].type & IT_TRADE) {
						if( !TradeCheck( r, u->faction )) {
							u->Error( "BUY: Can't buy trade items in that "
									"many regions.");
							o->num = 0;
						}
					}
					if (o->num == -1) {
						o->num = u->GetMoney()/m->price;
					}
					if (o->num * m->price > u->GetMoney()) {
						o->num = u->GetMoney() / m->price;
						u->Error( "BUY: Unit attempted to buy more than it "
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

void Game::DoBuy(ARegion * r,Market * m)
{
	/* First, find the number of items being purchased */
	int attempted = GetBuyAmount(r,m);

	if (m->amount != -1)
		if (attempted < m->amount) attempted = m->amount;

	m->activity = 0;
	int oldamount = m->amount;
	forlist((&r->objects)) {
		Object * obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit * u = (Unit *) elem;
			forlist((&u->buyorders)) {
				BuyOrder * o = (BuyOrder *) elem;
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
						SkillList * sl = new SkillList;
						u->AdjustSkills();
						delete sl;
					}
					u->items.SetNum(o->item,u->items.GetNum(o->item) + temp);
					u->faction->DiscoverItem(o->item, 0, 1);
					u->SetMoney(u->GetMoney() - temp * m->price);
					u->buyorders.Remove(o);
					u->Event(AString("Buys ") + ItemString(o->item,temp)
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
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->needed > 0 && ((!consume) ||
								  (u->GetFlag(FLAG_CONSUMING_UNIT) ||
								   u->GetFlag(FLAG_CONSUMING_FACTION)))) {
					int amount = u->items.GetNum(item);
					if (amount) {
						int eat = (u->needed + value - 1) / value;
						if (eat > amount)
							eat = amount;
						if (ItemDefs[item].type & IT_FOOD)
						{
							if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
								eat * value > u->stomach_space)
							{
								eat = (u->stomach_space + value - 1) / value;
								if (eat < 0)
									eat = 0;
							}
							u->hunger -= eat * value;
							u->stomach_space -= eat * value;
							if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
								u->stomach_space < 0)
							{
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
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->needed > 0 && ((!consume) ||
								  u->GetFlag(FLAG_CONSUMING_FACTION))) {
					/* Go through all units again */
					forlist((&r->objects)) {
						Object * obj2 = (Object *) elem;
						forlist((&obj2->units)) {
							Unit * u2 = (Unit *) elem;

							if (u->faction == u2->faction && u != u2) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->needed + value - 1) / value;
									if (eat > amount)
										eat = amount;
									if (ItemDefs[item].type & IT_FOOD)
									{
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
											eat * value > u->stomach_space)
										{
											eat = (u->stomach_space + value - 1) / value;
											if (eat < 0)
												eat = 0;
										}
										u->hunger -= eat * value;
										u->stomach_space -= eat * value;
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
											u->stomach_space < 0)
										{
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
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->needed > 0) {
					/* Go through all units again */
					forlist((&r->objects)) {
						Object * obj2 = (Object *) elem;
						forlist((&obj2->units)) {
							Unit * u2 = (Unit *) elem;
							if (u->faction != u2->faction &&
								u2->GetAttitude(r,u) == A_ALLY) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->needed + value - 1) / value;
									if (eat > amount)
										eat = amount;
									if (ItemDefs[item].type & IT_FOOD)
									{
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
											eat * value > u->stomach_space)
										{
											eat = (u->stomach_space + value - 1) / value;
											if (eat < 0)
												eat = 0;
										}
										u->hunger -= eat * value;
										u->stomach_space -= eat * value;
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
											u->stomach_space < 0)
										{
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
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
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
							u->stomach_space < 0)
						{
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
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->hunger > 0) {
					/* Go through all units again */
					forlist((&r->objects)) {
						Object * obj2 = (Object *) elem;
						forlist((&obj2->units)) {
							Unit * u2 = (Unit *) elem;

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
										u->stomach_space < 0)
									{
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
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->hunger > 0) {
					/* Go through all units again */
					forlist((&r->objects)) {
						Object * obj2 = (Object *) elem;
						forlist((&obj2->units)) {
							Unit * u2 = (Unit *) elem;
							if (u->faction != u2->faction &&
								u2->GetAttitude(r,u) == A_ALLY) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->hunger + value - 1) / value;
									if (eat > amount)
										eat = amount;
									u->hunger -= eat * value;
									u->stomach_space -= eat * value;
									u->needed -= eat * value;
									if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 &&
										u->stomach_space < 0)
									{
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
	/* First pass: set needed */
	{
		forlist((&regions)) {
			ARegion * r = (ARegion *) elem;
			forlist((&r->objects)) {
				Object * obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit * u = (Unit *) elem;
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
	}

	// Assess food requirements first
	if (Globals->UPKEEP_MINIMUM_FOOD > 0) {
		CheckUnitHunger();
		CheckFactionHunger();
		if (Globals->ALLOW_WITHDRAW)
		{
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
					ARegion * r = (ARegion *) elem;
					forlist((&r->objects)) {
						Object * obj = (Object *) elem;
						forlist((&obj->units)) {
							Unit * u = (Unit *) elem;
							if (u->hunger > 0 && u->faction->unclaimed > cost)
							{
								int value = Globals->UPKEEP_FOOD_VALUE;
								int eat = (u->hunger + value - 1) / value;
								/* Now see if faction has money */
								if (u->faction->unclaimed >= eat * cost)
								{
									u->Event(AString("Withdraws ") +
											ItemString(i, eat) +
											" for maintenance.");
									u->faction->unclaimed -= eat * cost;
									u->hunger -= eat * value;
									u->stomach_space -= eat * value;
									u->needed -= eat * value;
								}
								else
								{
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
	if( Globals->FOOD_ITEMS_EXIST )
	{
		CheckUnitMaintenance( 1 );
		CheckFactionMaintenance( 1 );
	}

	//
	// Check the unit for money.
	//
	CheckUnitMaintenanceItem(I_SILVER, 1, 0 );

	//
	// Check other units in same faction for money
	//
	CheckFactionMaintenanceItem(I_SILVER, 1, 0 );

	if( Globals->FOOD_ITEMS_EXIST )
	{
		//
		// Check unit for possible food items.
		//
		CheckUnitMaintenance( 0 );

		//
		// Fourth pass; check other units in same faction for food items
		//
		CheckFactionMaintenance( 0 );
	}

	//
	// Check unclaimed money.
	//
	{
		forlist((&regions)) {
			ARegion * r = (ARegion *) elem;
			forlist((&r->objects)) {
				Object * obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit * u = (Unit *) elem;
					if (u->needed > 0 && u->faction->unclaimed)
					{
						/* Now see if faction has money */
						if (u->faction->unclaimed >= u->needed)
						{
							u->Event(AString("Claims ") + u->needed +
									 " silver for maintenance.");
							u->faction->unclaimed -= u->needed;
							u->needed = 0;
						}
						else
						{
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
	CheckAllyMaintenanceItem( I_SILVER, 1 );

	if( Globals->FOOD_ITEMS_EXIST )
	{
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
			ARegion * r = (ARegion *) elem;
			forlist((&r->objects)) {
				Object * obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit * u = (Unit *) elem;
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
					if(DoWithdrawOrder(r, u, o)) break;
				}
				u->withdraworders.DeleteAll();
			}
		}
	}
}

int Game::DoWithdrawOrder(ARegion * r,Unit * u, WithdrawOrder * o)
{
	int itm = o->item;
	int amt = o->amount;
	int cost = (ItemDefs[itm].baseprice *5/2)*amt;

	if(r->type == R_NEXUS) {
		u->Error("WITHDRAW: Withdraw does not work in the Nexus.");
		return 1;
	}

	if (cost > u->faction->unclaimed) {
		u->Error(AString("WITHDRAW: Too little unclaimed silver to withdraw ")+
				ItemString(itm,amt)+".");
		return 0;
	}
	u->faction->unclaimed -= cost;
	u->Event(AString("Withdraws ") + ItemString(o->item,amt) + ".");
	u->items.SetNum(itm,u->items.GetNum(itm) + amt);
	return 0;
}


void Game::DoGiveOrders()
{
	forlist((&regions)) {
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				forlist((&u->giveorders)) {
					GiveOrder *o = (GiveOrder *)elem;
					if(o->item < 0) {
						if(o->amount != -2) {
							u->Error("GIVE: Invalid item.");
						} else {
							forlist((&u->items)) {
								Item *item = (Item *)elem;
								if((o->item == -NITEMS) ||
									(ItemDefs[item->type].type & (-o->item))) {
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
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				forlist((&u->exchangeorders)) {
					Order * o = (Order *) elem;
					DoExchangeOrder(r,u,(ExchangeOrder *) o);
				}
			}
		}
	}
}

void Game::DoExchangeOrder(ARegion * r,Unit * u,ExchangeOrder * o)
{
	// Check if the destination unit exists
	Unit *t = r->GetUnitId(o->target,u->faction->num);
	if (!t) {
		u->Error(AString("EXCHANGE: Nonexistant target (") +
				o->target->Print() + ").");
		u->exchangeorders.Remove(o);
		return;
	}

	// Check each Item can be given
	if( ItemDefs[ o->giveItem].flags & ItemType::CANTGIVE ) {
		u->Error(AString("EXCHANGE: Can't trade ") +
				ItemDefs[o->giveItem].names + ".");
		u->exchangeorders.Remove(o);
		return;
	}

	if( ItemDefs[ o->expectItem].flags & ItemType::CANTGIVE ) {
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
	if(!u->CanSee(r, t)) {
		u->Error(AString("EXCHANGE: Nonexistant target (") +
				o->target->Print() + ").");
		return;
	}
	// Check other unit has enough to give
	int amtRecieve = o->expectAmount;
	if (amtRecieve > t->items.GetNum(o->expectItem)) {
		t->Error(AString("EXCHANGE: Not giving enough. Expecting ") +
				ItemString(o->expectItem, o->expectAmount) + ".");
		u->Error(AString("EXCHANGE: Exchange aborted.  Not enough ") +
				"recieved. Expecting " +
			ItemString(o->expectItem,o->expectAmount) + ".");
		o->exchangeStatus = 0;
		return;
	}

	int exchangeOrderFound = 0;
	// Check if other unit has a reciprocal exchange order
	forlist ((&t->exchangeorders)) {
		ExchangeOrder * tOrder = (ExchangeOrder *) elem;
		Unit * ptrUnitTemp = r->GetUnitId(tOrder->target, t->faction->num);
		if (ptrUnitTemp == u) {
			if (tOrder->expectItem == o->giveItem) {
				if (tOrder->giveItem == o->expectItem) {
					exchangeOrderFound = 1;
					if (tOrder->giveAmount < o->expectAmount) {
						t->Error(AString("EXCHANGE: Not giving enough. ") +
								"Expecting " +
								ItemString(o->expectItem,o->expectAmount) +
								".");
						u->Error(AString("EXCHANGE: Exchange aborted. ") +
								"Not enough recieved. Expecting " +
								ItemString(o->expectItem,o->expectAmount) +
								".");
						tOrder->exchangeStatus = 0;
						o->exchangeStatus = 0;
						return;
					} else if (tOrder->giveAmount > o->expectAmount) {
						t->Error(AString("EXCHANGE: Exchange aborted. Too ") +
								"much given. Expecting " +
								ItemString(o->expectItem,o->expectAmount) +
								".");
						u->Error(AString("EXCHANGE: Exchange aborted. Too ") +
								"much offered. Expecting " +
								ItemString(o->expectItem,o->expectAmount) +
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
								ItemString(o->giveItem,o->giveAmount) + ".");
						u->items.SetNum(o->giveItem,
								u->items.GetNum(o->giveItem) - o->giveAmount);
						t->items.SetNum(o->giveItem,
								t->items.GetNum(o->giveItem) + o->giveAmount);
						t->items.SetNum(tOrder->giveItem,
								t->items.GetNum(tOrder->giveItem) -
								tOrder->giveAmount);
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
		if(!u->CanSee(r, t)) {
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

int Game::DoGiveOrder(ARegion * r,Unit * u,GiveOrder * o)
{
	// Check there is enough to give
	int amt = o->amount;
	if (amt != -2 && amt > u->items.GetNum(o->item)) {
		u->Error("GIVE: Not enough.");
		amt = u->items.GetNum(o->item);
	} else if (amt == -2) {
		amt = u->items.GetNum(o->item);
		if(o->except) {
			if(o->except > amt) {
				amt = 0;
				u->Error("GIVE: EXCEPT value greater than amount on hand.");
			} else {
				amt = amt - o->except;
			}
		}
	}

	if (o->target->unitnum == -1) {
		/* Give 0 */
		if (amt == -1) {
			u->Error("Can't discard a whole unit.");
			return 0;
		}

		AString temp = "Discards ";
		if (ItemDefs[o->item].type & IT_MAN) {
			u->SetMen(o->item,u->GetMen(o->item) - amt);
			temp = "Disbands ";
		} else if(Globals->RELEASE_MONSTERS &&
				(ItemDefs[o->item].type & IT_MONSTER)) {
			temp = "Releases ";
			u->items.SetNum(o->item,u->items.GetNum(o->item) - amt);
			if(Globals->WANDERING_MONSTERS_EXIST) {
				Faction *mfac = GetFaction(&factions, monfaction);
				Unit *mon = GetNewUnit(mfac, 0);
				int mondef = ItemDefs[o->item].index;
				mon->MakeWMon(MonDefs[mondef].name, o->item, amt);
				mon->MoveUnit(r->GetDummy());
				// This will result in 0 unless MONSTER_NO_SPOILS or
				// MONSTER_SPOILS_RECOVERY are set.
				mon->free = Globals->MONSTER_NO_SPOILS +
					Globals->MONSTER_SPOILS_RECOVERY;
			}
		} else {
			u->items.SetNum(o->item,u->items.GetNum(o->item) - amt);
		}
		u->Event(temp + ItemString(o->item, amt) + ".");
		return 0;
	}

	Unit * t = r->GetUnitId(o->target,u->faction->num);
	if (!t) {
		u->Error(AString("GIVE: Nonexistant target (") + o->target->Print() +
				").");
		return 0;
	}

	if(u == t) {
		u->Error(AString("GIVE: Attempt to give ")+ItemString(o->item,amt)+
				" to self.");
		return 0;
	}

	// New RULE -- Must be able to see unit to give something to them!
	if(!u->CanSee(r, t) &&
			(t->faction->GetAttitude(u->faction->num) < A_FRIENDLY)) {
		u->Error(AString("GIVE: Nonexistant target (") + o->target->Print() +
				").");
		return 0;
	}

	if (o->item != I_SILVER &&
			t->faction->GetAttitude(u->faction->num) < A_FRIENDLY) {
		u->Error("GIVE: Target is not a member of a friendly faction.");
		return 0;
	}

	if (amt == -1) {
		/* Give unit */
		if (u->type == U_MAGE) {
			if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
				if (CountMages(t->faction) >= AllowedMages( t->faction )) {
					u->Error("GIVE: Faction has too many mages.");
					return 0;
				}
			}
		}
		if(u->type == U_APPRENTICE) {
			if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
				if(CountApprentices(t->faction) >=
						AllowedApprentices(t->faction)){
					u->Error("GIVE: Faction has too many apprentices.");
					return 0;
				}
			}
		}

		int notallied = 1;
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
			Skill * skill = (Skill *) elem;
			int newlvl = u->GetRealSkill(skill->type);
			int oldlvl = u->faction->skills.GetDays(skill->type);
			if (newlvl > oldlvl) {
				for (int i=oldlvl+1; i<=newlvl; i++) {
					u->faction->shows.Add(new ShowSkill(skill->type,i));
				}
				u->faction->skills.SetDays(skill->type,newlvl);
			}
		}

		// Okay, not for each item that the unit has, tell the new faction
		// about it in case they don't know about it yet.
		{
			forlist(&u->items) {
				Item *i = (Item *)elem;
				u->faction->DiscoverItem(i->type, 0, 1);
			}
		}

		return notallied;
	}

	/* If the item to be given is a man, combine skills */
	if (ItemDefs[o->item].type & IT_MAN) {
		if (u->type == U_MAGE || u->type == U_APPRENTICE ||
				t->type == U_MAGE || t->type == U_APPRENTICE) {
			u->Error("GIVE: Magicians can't transfer men.");
			return 0;
		}
		if (o->item == I_LEADERS && t->IsNormal()) {
			u->Error("GIVE: Can't mix leaders and normal men.");
			return 0;
		} else {
			if (o->item != I_LEADERS && t->IsLeader()) {
				u->Error("GIVE: Can't mix leaders and normal men.");
				return 0;
			}
		}
		// Small hack for Ceran
		if(o->item == I_MERC && t->GetMen()) {
			u->Error("GIVE: Can't mix mercenaries with other men.");
			return 0;
		}

		if (u->faction != t->faction) {
			u->Error("GIVE: Can't give men to another faction.");
			return 0;
		}

		if (u->nomove) t->nomove = 1;

		SkillList * temp = u->skills.Split(u->GetMen(),amt);
		t->skills.Combine(temp);
		delete temp;
	}

	if( ItemDefs[ o->item ].flags & ItemType::CANTGIVE ) {
		u->Error(AString("GIVE: Can't give ") + ItemDefs[o->item].names + ".");
		return 0;
	}

	u->Event(AString("Gives ") + ItemString(o->item,amt) + " to " +
			*t->name + ".");
	if (u->faction != t->faction) {
		t->Event(AString("Receives ") + ItemString(o->item,amt) +
				" from " + *u->name + ".");
	}
	u->items.SetNum(o->item,u->items.GetNum(o->item) - amt);
	t->items.SetNum(o->item,t->items.GetNum(o->item) + amt);
	t->faction->DiscoverItem(o->item, 0, 1);

	if (ItemDefs[o->item].type & IT_MAN) {
		t->AdjustSkills();
	}
	return 0;
}

void Game::DoGuard1Orders()
{
	forlist((&regions)) {
		ARegion * r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object * obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit * u = (Unit *) elem;
				if (u->guard == GUARD_SET || u->guard == GUARD_GUARD) {
					if (!u->Taxers()) {
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
		ARegion * region = (ARegion *) elem;
		DeleteEmptyInRegion(region);
	}
}

void Game::DeleteEmptyInRegion(ARegion * region)
{
	forlist(&region->objects) {
		Object * obj = (Object *) elem;
		forlist (&obj->units) {
			Unit * unit = (Unit *) elem;
			if (unit->IsAlive() == 0) {
				region->Kill(unit);
			}
		}
	}
}

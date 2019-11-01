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

#ifndef DEBUG
//#define DEBUG
#endif

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
	if(Globals->ARCADIA_MAGIC) { //moved forward for hypnosis, and since I couldn't think of any reason tax needed to come first.
    	Awrite("Running Magic Orders...");
    	ClearCastEffects();
    	RunCastOrders();	
	}
	if(!Globals->LATE_TAX) {
    	Awrite("Running PILLAGE Orders...");
    	RunPillageOrders();
    	Awrite("Running TAX Orders...");
    	RunTaxOrders();
	}
	Awrite("Running GUARD 1 Orders...");
	DoGuard1Orders();
	if(!Globals->ARCADIA_MAGIC) {
    	Awrite("Running Magic Orders...");
    	ClearCastEffects();
    	RunCastOrders();
	}
	Awrite("Running SELL Orders...");
	RunSellOrders();
	Awrite("Running BUY Orders...");
	RunBuyOrders();
	Awrite("Running SEND Orders...");
	DoSendOrders();
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
	if (Globals->ALLOW_BANK & GameDefs::BANK_ENABLED) {
		Awrite("Running BANK orders...");
		if (Globals->ALLOW_BANK & GameDefs::BANK_TRADEINTEREST)
			BankInterest();
		DoBankDepositOrders();
		DoBankWithdrawOrders();
	}
	if(Globals->ALLOW_WITHDRAW) {
		Awrite("Running WITHDRAW Orders...");
		DoWithdrawOrders();
	}
	if(Globals->LATE_TAX) {
    	Awrite("Running PILLAGE Orders...");
    	RunPillageOrders();
    	Awrite("Running TAX Orders...");
    	RunTaxOrders();
	}
	Awrite("Running Move Orders...");
	RunMoveOrders();
	Awrite("Sinking Ships...");
	SinkUncrewedShips();
	TransferNonShipUnits(); // BS Sailing Mod
	DrownUnits();
	Awrite("Clearing Factions...");
	FindDeadFactions();
	Awrite("Running Teach Orders...");
	RunTeachOrders();
	Awrite("Running Month-long Orders...");
	RunMonthOrders(); //Build, produce, work
	Awrite("Recieving sent goods");
	RecieveSentGoods();
	Awrite("Running Teleport Orders...");	
	RunTeleportOrders();
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		Awrite("Running Transport Orders...");
		CheckTransportOrders();
		RunTransportOrders();
	}
	if(Globals->ARCADIA_MAGIC) {
        SinkLandRegions();
	    DistributeFog();
	    RechargeMages();
    }
	Awrite("Assessing Maintenance costs...");
	AssessMaintenance();
	Awrite("Post-Turn Processing...");
	PostProcessTurn();
	DeleteEmptyUnits();
	EmptyHell();
	RemoveEmptyObjects();
}

void Game::RunCheckAllOrders()
{
	Awrite("Running FIND Orders...");
	RunFindOrders();
	
	Awrite("Running ENTER/LEAVE Orders...");
	RunEnterOrders();
	
	
	Awrite("Running PROMOTE/EVICT Orders...");
	RunPromoteOrders();
	
	Awrite("Running DESTROY Orders...");
	RunDestroyOrders();
	
	Awrite("Running GIVE/PAY/TRANSFER Orders...");
	DoGiveOrders();
	/*
	Awrite("Running EXCHANGE Orders...");
	DoExchangeOrders();
	*/
	Awrite("Running DESTROY Orders...");
	RunDestroyOrders();
	
	if(Globals->ARCADIA_MAGIC) { //moved forward for hypnosis, and since I couldn't think of any reason tax needed to come first.
    	Awrite("Running Magic Orders...");
    	ClearCastEffects();
    	RunCastOrders();	
	}
	
	if(!Globals->LATE_TAX) {
    	Awrite("Running PILLAGE Orders...");
    	RunPillageOrders();
    	Awrite("Running TAX Orders...");
    	RunTaxOrders();
	}
	
	
	Awrite("Running GUARD 1 Orders...");
	DoGuard1Orders();
	if(!Globals->ARCADIA_MAGIC) {
    	Awrite("Running Magic Orders...");
    	ClearCastEffects();
    	RunCastOrders();
	}
	
	Awrite("Running SELL Orders...");
	RunSellOrders();
	Awrite("Running BUY Orders...");
	RunBuyOrders();
	Awrite("Running SEND Orders...");
	DoSendOrders();
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
	if (Globals->ALLOW_BANK & GameDefs::BANK_ENABLED) {
		Awrite("Running BANK orders...");
		if (Globals->ALLOW_BANK & GameDefs::BANK_TRADEINTEREST)
			BankInterest();
		DoBankDepositOrders();
		DoBankWithdrawOrders();
	}
	if(Globals->ALLOW_WITHDRAW) {
		Awrite("Running WITHDRAW Orders...");
		DoWithdrawOrders();
	}
	
	if(Globals->LATE_TAX) {
    	Awrite("Running PILLAGE Orders...");
    	RunPillageOrders();
    	Awrite("Running TAX Orders...");
    	RunTaxOrders();
	}
	
	Awrite("Running Move Orders...");
	RunMoveOrders();
	Awrite("Sinking Ships...");
	SinkUncrewedShips();
	TransferNonShipUnits(); // BS Sailing Mod
	DrownUnits();
	Awrite("Clearing Factions...");
	FindDeadFactions();
	Awrite("Running Teach Orders...");
	RunTeachOrders();
	Awrite("Running Month-long Orders...");
	RunMonthOrders(); //Build, produce, work
	Awrite("Recieving sent goods");
	RecieveSentGoods();
	Awrite("Running Teleport Orders...");	
	RunTeleportOrders();
	if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
		Awrite("Running Transport Orders...");
		CheckTransportOrders();
		RunTransportOrders();
	}
	if(Globals->ARCADIA_MAGIC) {
        SinkLandRegions();
	    DistributeFog();
	    RechargeMages();
    }
	Awrite("Assessing Maintenance costs...");
	AssessMaintenance();
	Awrite("Post-Turn Processing...");
	PostProcessTurn();
	DeleteEmptyUnits();
	EmptyHell();
	RemoveEmptyObjects();
}

void Game::CountUnits()
{
//Test item. Not usually called from anywhere
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if(u->faction->num >= 3) Awrite(AString("faction ") + u->faction->num + " " + u->GetMen() + " " + u->dead);
			}
		}
	}
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
				u->SetFlag(FLAG_VISIB, 0);
			}
		}
	}
}

void Game::RunCastOrders()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *obj = (Object *) elem;
			forlist(&obj->units) {
				Unit *u = (Unit *) elem;
				forlist(&u->castlistorders) {
                    CastOrder *ord = (CastOrder *) elem;
                    RunACastOrder(r, obj, u, ord);
                }
                u->castlistorders.DeleteAll();
			}
		}
    }
}

int Game::CountMages(Faction *pFac)
{
	int i = 0;
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == pFac && u->type == U_MAGE && u->GetMen()) i++;   //get men condition added because you can have a mage unit with no leaders after the give phase.
			}
		}
	}
	return(i);
}

int Game::TaxCheck(ARegion *pReg, Faction *pFac)
{
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if(AllowedTaxes(pFac) == -1) {
			//
			// No limit.
			//
			return(1);
		}

		forlist(&(pFac->war_regions)) {
			ARegion *x = ((ARegionPtr *) elem)->ptr;
			if(x == pReg) {
				//
				// This faction already performed a tax action in this
				// region.
				//
				return 1;
			}
		}
		if(pFac->war_regions.Num() >= AllowedTaxes(pFac)) {
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
	if(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		if(AllowedTrades(pFac) == -1) {
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

void Game::Do1Assassinate(ARegion *r, Object*, Unit *u)
{
    if(u->dead) return; //Arcadia mod because assassin orders cannot be removed.

	AssassinateOrder *so = (AssassinateOrder *) u->stealorders;
	Unit *tar = r->GetUnitId(so->target, u->faction->num);

	if (!tar) {
		u->Error("ASSASSINATE: Invalid unit given.", so->quiet);
		return;
	}
	if (!tar->IsReallyAlive()) {
		u->Error("ASSASSINATE: Invalid unit given.", so->quiet);
		return;
	}

	// New rule -- You can only assassinate someone you can see
	if (!u->CanSee(r, tar)) {
		u->Error("ASSASSINATE: Invalid unit given.", so->quiet);
		return;
	}

	if (tar->type == U_GUARD || tar->type == U_WMON ||
			tar->type == U_GUARDMAGE) {
		u->Error("ASSASSINATE: Can only assassinate other player's "
				"units.", so->quiet);
		return;
	}

	if (u->GetMen() != 1) {
		u->Error("ASSASSINATE: Must be executed by a 1-man unit.", so->quiet);
		return;
	}
	
	if (u->IsMage() && tar->IsMage()) {
        u->Error("ASSASSINATE: Heros cannot assassinate other heros", so->quiet);
        return;
    }
    
    int def = tar->GetSkill(S_SECSIGHT);
    while(def > 0) {
        if(getrandom(2)) {
            u->Error("ASSASSINATE: Attempt is foiled by target's second sight", so->quiet);
            return;
        }
        def--;
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
		if (f->num == guardfaction || f->num == elfguardfaction ||
                f->num == dwarfguardfaction || f->num == independentguardfaction) {
            if(f->ethnicity == tar->faction->ethnicity) {
    			succ = 0;
    			break;
			}
		}
	}
	if (tar->type == U_MAGE && Globals->ARCADIA_MAGIC) {
	    int energy = tar->MaxEnergy();
	    if(getrandom(40+energy) > 40) succ = 0; // chance for mages to escape assassination.
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
	if(u->items.GetNum(I_RINGOFI)) {
		ass = 2; // Check if assassin has a ring.
		// New rule: if a target has an amulet of true seeing they
		// cannot be assassinated by someone with a ring of invisibility
		if(tar->AmtsPreventCrime(u)) {
			tar->Event("Assassination prevented by amulet of true seeing.");
			u->Event(AString("Attempts to assassinate ") + *(tar->name) +
					", but is prevented by amulet of true seeing.");
			return;
		}
	}
	u->PracticeAttribute("stealth");
	//assassination taking place ... experience stealth. Target experiences observation. One person dies, other gets experience
	u->Experience(S_STEALTH,10);
	tar->Experience(S_OBSERVATION,10);
	RunBattle(r, u, tar, ass);
#ifdef DEBUG
cout << "Returned to Do1Assassinate" << endl;
#endif
}

void Game::Do1Steal(ARegion *r, Object*, Unit *u)
{
    if(u->dead) return; //Arcadia mod because assassin orders cannot be removed.
    
	StealOrder *so = (StealOrder *) u->stealorders;
	Unit *tar = r->GetUnitId(so->target, u->faction->num);

	if (!tar) {
		u->Error("STEAL: Invalid unit given.", so->quiet);
		return;
	}

	// New RULE!! You can only steal from someone you can see.
	if(!u->CanSee(r, tar)) {
		u->Error("STEAL: Invalid unit given.", so->quiet);
		return;
	}

	if (tar->type == U_GUARD || tar->type == U_WMON ||
			tar->type == U_GUARDMAGE) {
		u->Error("STEAL: Can only steal from other player's "
				"units.", so->quiet);
		return;
	}

	if (u->GetMen() != 1) {
		u->Error("STEAL: Must be executed by a 1-man unit.", so->quiet);
		return;
	}
	
    int def = tar->GetSkill(S_SECSIGHT);
    while(def > 0) {
        if(getrandom(2)) {
            u->Error("ASSASSINATE: Attempt is foiled by target's second sight", so->quiet);
            return;
        }
        def--;
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
		if (f->num == guardfaction || f->num == elfguardfaction ||    
                f->num == dwarfguardfaction || f->num == independentguardfaction) {
            if(f->ethnicity == tar->faction->ethnicity) {
    			succ = 0;
    			break;
			}
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
	if(tar->AmtsPreventCrime(u)) {
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
				if(o->type != O_DUMMY) continue;
				forlist(&o->units) {
					Unit *u = (Unit *)elem;
					if (!u->TryToSwim()) {
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
		ARegion *r = (ARegion *) elem;
		if (TerrainDefs[r->type].similar_type == R_OCEAN) {
			forlist(&r->objects) {
				Object *o = (Object *) elem;
				if(!o->IsBoat()) continue;

				int men = 0;
				forlist(&o->units) {
					u = (Unit *)elem;
					men += u->GetMen();
				}

				if(men <= 0 && getrandom(100) < 10) { //BS Sailing Mod, 10% sink chance
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

void Game::TransferNonShipUnits()
{
/* BS Sailing Mod */
	Unit *u;
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		if (TerrainDefs[r->type].similar_type == R_OCEAN) {
			forlist(&r->objects) {
				Object *o = (Object *) elem;
				if(o->type == O_DUMMY) continue;				
				if(o->IsBoat()) continue;

				forlist(&o->units) {
					u = (Unit *)elem;
					if(u->faction->num != monfaction) {
			/* if no boat in the region, move units into the ocean */
					    if(!u->HasBoat(r) ) u->MoveUnit(r->GetDummy());
                    }
				}
			}
		}
	}
}

int Unit::HasBoat(ARegion *reg)
{
/* BS Sailing Mod */
    forlist(&reg->objects) {
        Object *o = (Object *) elem;
		if(!o->IsBoat()) continue;
		Unit *forbid = o->ForbiddenBy(reg, this);
		if(!forbid) return 1; //Not forbidden, ie allowed on ship
    }
    return 0; // no ship would take the unit!
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
    if(f->start != -1 && regions.GetRegion(f->start) && 
        regions.GetRegion(f->start)->flagpole == FL_USED_START_LOC) 
            regions.GetRegion(f->start)->flagpole = FL_UNUSED_START_LOC;
    
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->faction == f) {
					o->units.Remove(u);         //Remove(u), not Kill(u) ?! WHY?
					delete u;
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
		if(f->find == 0) all = 1;
		if(!all) {
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

int Game::FortTaxBonus(Object *o, Unit *u)
{
	int protect = ObjectDefs[o->type].protect;
	int fortbonus = 0;
	forlist(&o->units) {
		Unit *unit = (Unit *) elem;
		int men = unit->GetMen();
		if(unit->num == u->num) {
			if(unit->taxing == TAX_TAX) {
				int fortbonus = men;
				int maxtax = unit->Taxers(1);
				if(fortbonus > protect) fortbonus = protect;
				if(fortbonus > maxtax) fortbonus = maxtax;
				fortbonus *= Globals->TAX_BONUS_FORT;
				return(fortbonus);
			}
		}
		protect -= men;
		if(protect < 0) protect = 0;	
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
			if(u->taxing == TAX_AUTO) u->taxing = TAX_TAX;

			if (u->taxing == TAX_TAX) {
				if (!reg->CanTax(u)) {
					u->Error("TAX: A unit is on guard.");
					u->taxing = TAX_NONE;
				} else {
					int men = u->Taxers(0);
					int fortbonus = u->GetMen();
					if(fortbonus > protect) fortbonus = protect;
					protect -= u->GetMen();
					if(protect < 0) protect = 0;
					if (men) {
						if(!TaxCheck(reg, u->faction)) {
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
	reg->untaxed = reg->money - desired; //BS mod.
	if (desired < reg->money) desired = reg->money;

	forlist(&reg->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->taxing == TAX_TAX) {
				int t = u->Taxers(0);
				t += FortTaxBonus(o, u);
				double fAmt = ((double) t) *
					((double) reg->money) / ((double) desired);
				int amt = (int) fAmt;
				reg->money -= amt;
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
	if (reg->money < 1) return;
//	if (reg->Wages() < 11) return;

	/* First, count up pillagers */
	int pillagers = CountPillagers(reg);

	if (pillagers * 2 * Globals->TAX_BASE_INCOME < reg->money) {
		ClearPillagers(reg);
		return;
	}

	AList *facs = reg->PresentFactions();
	int amt = reg->money * 2;
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
	reg->money = 0;
	reg->wages -= 7;    //used to be 6, 7 for Xan because of the lower base wage for tax purposes (old breakeven point $25, with 6 would now be $20, with 7 is $26, closer to old behaviour)
	if (reg->wages < 0) reg->wages = 0;
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
//			if (o->type != O_DUMMY) {     //BS mod - allow promote in open area.
				u = o->GetOwner();
				if(u && u->promote) {
					Do1PromoteOrder(o, u);
					delete u->promote;
					u->promote = 0;
				}
//			}
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
//						if (o->type != O_DUMMY) {
							u->Error("PROMOTE: Must be owner", u->promotequiet);
							delete u->promote;
							u->promote = 0;
//						} else {
//							u->Error("PROMOTE: Can only promote inside structures.", u->promotequiet);
//							delete u->promote;
//							u->promote = 0;
//						}
					}
					if (u->evictorders) {
						if (o->type != O_DUMMY) {
							u->Error("EVICT: Must be owner",u->evictorders->quiet);
							delete u->evictorders;
							u->evictorders = 0;
						} else {
							u->Error("EVICT: Can only evict inside structures.",u->evictorders->quiet);
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
		u->Error("PROMOTE: Can't find target.", u->promotequiet);
		return;
	}
	obj->units.Remove(tar);
	obj->units.Insert(tar);
}

void Game::Do1EvictOrder(Object *obj, Unit *u)
{
	EvictOrder *ord = u->evictorders;

	while (ord && ord->targets.Num()) {
		UnitId *id = (UnitId *)ord->targets.First();
		ord->targets.Remove(id);
		Unit *tar = obj->GetUnitId(id, u->faction->num);
		delete id;
		if (!tar) continue;
		if(obj->IsBoat() &&
			(TerrainDefs[obj->region->type].similar_type == R_OCEAN) &&
			(!tar->CanReallySwim() || tar->GetFlag(FLAG_NOCROSS_WATER))) {
			u->Error("EVICT: Cannot forcibly evict units over ocean.", ord->quiet);
			continue;
		}
		Object *to = obj->region->GetDummy();
		tar->MoveUnit(to);
		tar->Event(AString("Evicted from ") + *obj->name + " by " + *u->name);
		u->Event(AString("Evicted ") + *tar->name + " from " + *obj->name);
	}
}

void Game::RunEnterOrders()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			forlist(&o->units) {
				Unit *u = (Unit *) elem;
				if (u->enter)
					Do1EnterOrder(r, o, u);
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
		if((TerrainDefs[r->type].similar_type == R_OCEAN) &&
				( (!u->CanSwim() && u->items.Weight()) || u->GetFlag(FLAG_NOCROSS_WATER))) { //this allows empty units to leave ships - which can then have men bought into them ... but otherwise, bought merfolk can't leave ships.
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

void Game::RemoveEmptyObjects()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
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
	/* ARCADIA_MAGIC Patch*/
	if(u->dead && Globals->ARCADIA_MAGIC) {
        if(!u->AgeDead()) {
	        u->MoveUnit(0);
	        r->hell.Add(u);
	    }
	    //check faction
        if(u->faction->num != ghostfaction) {
            Faction *fac = GetFaction(&factions, ghostfaction);
            if(fac) u->faction = fac;
            else {
                u->MoveUnit(0); //routine for destroying a unit, from ARegion::Kill()
    	        r->hell.Add(u);
    	    }
        }
    }
}

void Game::EndGame(Faction *pVictor, AString *victoryline)
{
	forlist(&factions) {
		Faction *pFac = (Faction *) elem;
		pFac->exists = 0;
		if(pFac == pVictor)
			pFac->quit = QUIT_WON_GAME;
		else
			pFac->quit = QUIT_GAME_OVER;

		if(pVictor)
			pFac->Event(*victoryline);
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

void Game::PostProcessTurn()
{
	// process migration before adjusting economy
	if(Globals->PLAYER_ECONOMY) ProcessMigration();

	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		r->PostTurn(&regions);

		if(Globals->CITY_MONSTERS_EXIST && (r->town || r->type == R_NEXUS))
			AdjustCityMons(r);
			
        if(Globals->ARCADIA_MAGIC) {
            SpecialErrors(r);
        }

		forlist (&r->objects) {
			Object *o = (Object *) elem;
			forlist (&o->units) {
				Unit *u = (Unit *) elem;
				PostProcessUnit(r, u);
			}
		}
	}
	
    if(Globals->ARCADIA_MAGIC) UpdateFactionAffiliations();

	if(Globals->WANDERING_MONSTERS_EXIST) GrowWMons(Globals->WMON_FREQUENCY);
	
	if(Globals->LAIR_MONSTERS_EXIST) {
        if(year == 1 && month < 2) GrowLMons(100);
        else GrowLMons(Globals->LAIR_FREQUENCY);
    }

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

	if(!livingFacs) {
	    AString *vicline = new AString("");
		EndGame(0, vicline);
		delete vicline;
	} else if(!(Globals->OPEN_ENDED)) {
	    AString *vicline = new AString("");
		Faction *pVictor = CheckVictory(vicline);          

		if(pVictor)
			EndGame(pVictor, vicline);
		delete vicline;
	}
}

void Game::DoAutoAttacks()
{
	forlist(&regions) {
		ARegion *r = (ARegion *) elem;
		DoAutoAttacksRegion(r);
	}
}

void Game::DoAutoAttacksRegion(ARegion *r)
{
	forlist(&r->objects) {
		Object *o = (Object *) elem;
		forlist(&o->units) {
			Unit *u = (Unit *) elem;
			if (u->canattack && u->IsReallyAlive()) DoAutoAttack(r, u);
		}
	}
}

void Game::DoAdvanceAttacks(AList *locs)
{
//note that region & unit are up to date, but obj used not to be because it was not updated in ARegion::DoAMoveOrder. This has been
//fixed in the Arcadia code but not yet the standard.
	forlist(locs) {
		Location *l = (Location *) elem;
		Unit *u = l->unit;
		ARegion *r = l->region;
		if (u->canattack && u->IsReallyAlive()) {
			DoAutoAttack(r, u);
			if(!u->canattack || !u->IsReallyAlive()) {
				u->guard = GUARD_NONE;
			}
		}
		if (u->canattack && u->guard == GUARD_ADVANCE && u->IsReallyAlive()) {
			DoAdvanceAttack(r, u);
			u->guard = GUARD_NONE;
		}
		if (u->IsReallyAlive()) {
			DoAutoAttackOn(r, u);
			if(!u->canattack || !u->IsReallyAlive()) {
				u->guard = GUARD_NONE;
			}
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
					(u->GetAttitude(r, t) == A_HOSTILE) && u->IsReallyAlive() &&
					u->canattack)
				AttemptAttack(r, u, t, 1);
#ifdef DEBUG
cout << "Attempted from DoAutoAttackOn" << endl;
#endif
			if (!t->IsReallyAlive()) return;
		}
	}
}

void Game::DoAdvanceAttack(ARegion *r, Unit *u) {
	Unit *t = r->Forbidden(u);
	while (t && u->canattack && u->IsReallyAlive()) {
		AttemptAttack(r, u, t, 1, 1);
#ifdef DEBUG
cout << "Attempted from DoAdvanceAttack" << endl;
#endif
		if(u->IsReallyAlive()) t = r->Forbidden(u);
	}
}

void Game::DoAutoAttack(ARegion *r, Unit *u) {
  forlist(&r->objects) {
	Object *o = (Object *) elem;
	forlist(&o->units) {
	  Unit *t = (Unit *) elem;
	  if (u->guard != GUARD_AVOID && (u->GetAttitude(r, t) == A_HOSTILE)) {
	AttemptAttack(r, u, t, 1);
#ifdef DEBUG
cout << "Attempted from DoAutoAttack" << endl;
#endif
	  }
	  if (u->canattack == 0 || u->IsReallyAlive() == 0)
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
			if (u->faction != mon->faction) {    //Nylandor mod; monsters can attack guards
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
			if (u->faction != mon->faction) {    //Nylandor mod; monsters can attack guards
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
#ifdef DEBUG
cout << "Attempted from CheckWMonAttack" << endl;
#endif
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
					if (u->canattack && u->IsReallyAlive()) {
						CheckWMonAttack(r, u);
					}
				} else {
					if (u->attackorders && u->IsReallyAlive()) {
						AttackOrder *ord = u->attackorders;
						while (ord->targets.Num()) {
							UnitId *id = (UnitId *) ord->targets.First();
							ord->targets.Remove(id);
							Unit *t = r->GetUnitId(id, u->faction->num);
							delete id;
							if (u->canattack && u->IsReallyAlive()) {
								if (t) {
									AttemptAttack(r, u, t, 0);
#ifdef DEBUG
cout << "Attempted from DoAttackOrders" << endl;
#endif
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
	if (!t->IsReallyAlive()) {
#ifdef DEBUG
cout << "Not Alive" << endl;
#endif	
        return;    
    }

	if (!u->CanSee(r, t)) {
		if (!silent) u->Error("ATTACK: Non-existent unit.");
#ifdef DEBUG
cout << "Can't See " << u->num << " " << t->num << endl;
#endif	
		return;
	}

	if (!u->CanCatch(r, t)) {
		if (!silent) u->Error("ATTACK: Can't catch that unit.");
#ifdef DEBUG
cout << "Can't Catch" << endl;
#endif	
		return;
	}

	RunBattle(r, u, t, 0, adv);
#ifdef DEBUG
cout << "Returned to AttemptAttack" << endl;
#endif	
	return;
}

void Game::RunSellOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->markets)) {
			Market *m = (Market *) elem;
			if (m->type == M_SELL && (m->amount > 0))  //second term added to prevent merchantry revealing 0-item markets (and stop them preventing merchantry from working)
				DoSell(r, m);
		}
		{
			forlist((&r->objects)) {
				Object *obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit *u = (Unit *) elem;
				    if(u->GetSkill(S_MERCHANTRY)) {
    					forlist((&u->sellorders)) {
    					    SellOrder *o = (SellOrder *) elem;
                            DoMerchantSell(u,o);        //'sell-anywhere' spell
    					}   
				    } else {
    					forlist((&u->sellorders)) {
    					    u->Error("SELL: Can't sell that.");
    					}
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
					if(o->num == -1) {
						o->num = u->items.CanSell(o->item);
					}
					if (o->num > u->items.CanSell(o->item)) {
						o->num = u->items.CanSell(o->item);
						u->Error("SELL: Unit attempted to sell more than "
								"it had.", o->quiet);
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
					//get any trading bonuses.
				    int bonus = 0;
				    Unit *trader = NULL;
				    forlist((&r->objects)) {
					    Object *obj2 = (Object *) elem;
				    	forlist((&obj2->units)) {
				    		Unit *u2 = (Unit *) elem;
				    		if(u2->GetAttitude(r,u) == A_ALLY && u2->GetSkill(S_TRADING) > bonus) {
                                bonus = u2->GetSkill(S_TRADING);
                                trader = u2;
                            }
			            }
				    }

					int temp = 0;
					if (attempted) {
						temp = (m->amount *o->num + getrandom(attempted))
							/ attempted;
						if (temp<0) temp = 0;
					}
					attempted -= o->num;
					m->amount -= temp;
					m->activity += temp;
					u->items.SetNum(o->item, u->items.GetNum(o->item) - temp);
					u->SetMoney(u->GetMoney() + (temp * m->price * (20+bonus)) / 20);
					if(trader) trader->numtraded += (temp * m->price * (20+bonus)) / 20;
					u->sellorders.Remove(o);
					u->Event(AString("Sells ") + ItemString(o->item, temp)
							+ " at $" + (m->price * (20+bonus)) / 20 + " each.");
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
			if (m->type == M_BUY && (m->amount > 0))
				DoBuy(r, m);
		}
		{
			forlist((&r->objects)) {
				Object *obj = (Object *) elem;
				forlist((&obj->units)) {
					Unit *u = (Unit *) elem;
				    if(u->GetSkill(S_MERCHANTRY)) {
    					forlist((&u->buyorders)) {
    					    BuyOrder *o = (BuyOrder *) elem;
                            DoMerchantBuy(u,o);        //'sell-anywhere' spell
    					}   
				    } else {
    					forlist((&u->buyorders)) {
    					    u->Error("BUY: Can't buy that.");
    					}
					}
					u->buyorders.DeleteAll();
				}
			}
		}
	}
}

int Game::GetBuyAmount(ARegion *r, Market *m)
{
    //cycle through and make sure each faction's orders are legal first!
    forlist(&factions) {
        Faction * fac = (Faction *) elem;
        int sharedsilver = 0;
        int shareditembuys = 0;
    	forlist((&r->objects)) {
    		Object *obj = (Object *) elem;
    		forlist((&obj->units)) {
    			Unit *u = (Unit *) elem;
    			if(u->faction != fac) continue;
    			if(!sharedsilver && u->GetFlag(FLAG_SHARING)) sharedsilver = u->GetSharedMoney();
    			forlist ((&u->buyorders)) {
    				BuyOrder *o = (BuyOrder *) elem;
    				if (o->item == m->item) {
    					if (ItemDefs[o->item].type & IT_MAN) {
    						if (u->type == U_MAGE) {
    							u->Error("BUY: Heroes can't recruit more men.", o->quiet);
    							o->num = 0;
    						}
    						if(u->type == U_APPRENTICE) {
    							u->Error("BUY: Apprentices can't recruit more "
    									"men.", o->quiet);
    							o->num = 0;
    						}
    						// XXX: there has to be a better way
    						if (u->GetRealSkill(S_QUARTERMASTER)) {
    							u->Error("BUY: Quartermasters can't recruit more "
    									"men.", o->quiet);
    							o->num = 0;
    						}
    						if (Globals->TACTICS_NEEDS_WAR && u->GetRealSkill(S_TACTICS) == 5) {
    							u->Error("BUY: Tacticians can't recruit more "
    									"men.", o->quiet);       //Why not? They wouldn't be a TACT 5 unit anymore if they did!
    							o->num = 0;
    						}
    						if (((ItemDefs[o->item].type & IT_LEADER) &&
    								u->IsNormal()) ||
    								(!(ItemDefs[o->item].type & IT_LEADER) &&
    								 u->IsLeader())) {
    							u->Error("BUY: Can't mix leaders and normal men.", o->quiet);
    							o->num = 0;
    						}
    					}
    					if (ItemDefs[o->item].type & IT_TRADE) {
    						if(!TradeCheck(r, u->faction)) {
    							u->Error("BUY: Can't buy trade items in that "
    									"many regions.", o->quiet);
    							o->num = 0;
    						}
    					}
    					
    					
    					
    					if (o->num == -1) {
    						o->num = u->GetSharedMoney()/m->price;       //here "ALL" orders draw on shared money ... should they?
    					}
    					if (o->num * m->price > u->GetSharedMoney()) {
    						o->num = u->GetSharedMoney() / m->price;
    						u->Error("BUY: Unit attempted to buy more than it "
    								"could afford.", o->quiet);
    					}
    					//we want to add the orders to faction's total shared-money orders
    					if(u->GetFlag(FLAG_SHARING)) shareditembuys += o->num*m->price; //this is the amount of the order drawing on shared money
    					else if(o->num * m->price > u->GetMoney()) shareditembuys += o->num*m->price - u->GetMoney(); //this is the amount of the order drawing on shared money
    				}
    				if (o->num < 1 && o->num != -1) {
    					u->buyorders.Remove(o);
    					delete o;
    				}
    			}
    		}
    	}
    //if we are attempting to buy more items with shared silver than we have shared silver, we need to reduce our orders so we don't try buying more than we can afford
        if(sharedsilver < shareditembuys) {
            forlist((&r->objects)) {
        		Object *obj = (Object *) elem;
        		forlist((&obj->units)) {
        			Unit *u = (Unit *) elem;
        			if(sharedsilver >= shareditembuys) continue; //we have closed the gap, no need to deduct more
        			forlist ((&u->buyorders)) {
        				BuyOrder *o = (BuyOrder *) elem;
        				if (o->item == m->item) {
                            int ownmoney = 0;
                            if(!u->GetFlag(FLAG_SHARING)) ownmoney = u->GetMoney();
                            int excess = o->num - (ownmoney/m->price);
                            //work out how much we can buy with our 'fraction' of the shared silver
                            int newamount = excess * sharedsilver / shareditembuys;
                            o->num -= (excess-newamount);
                            //count the money spent here as this unit's own money, removing both items and money from the temporary share 'pool'. However, don't claim it for real because we may not be able to buy the items in the end
                            shareditembuys -= excess;
                            sharedsilver -= (o->num*m->price - ownmoney); //subtracting the amount we need to claim
                        }
        				if (o->num < 1 && o->num != -1) {
		        			u->buyorders.Remove(o);
				        	delete o;
        				}
                    }
                }
            }
        }
    }
    
    //we now know that all factions are attempting to buy within the limits of what they can afford.
    //also, no need to check legality of purchase - that's done above
    
	int num = 0;
	forlist_reuse((&r->objects)) {
		Object *obj = (Object *) elem;
		forlist((&obj->units)) {
			Unit *u = (Unit *) elem;
			forlist ((&u->buyorders)) {
				BuyOrder *o = (BuyOrder *) elem;
				if (o->item == m->item) num += o->num;
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
				    //get any trading bonuses.
				    int bonus = 0;
				    Unit *trader = NULL;
				    forlist((&r->objects)) {
					    Object *obj2 = (Object *) elem;
				    	forlist((&obj2->units)) {
				    		Unit *u2 = (Unit *) elem;
				    		if(u2->GetAttitude(r,u) == A_ALLY && u2->GetSkill(S_TRADING) > bonus) {
                                bonus = u2->GetSkill(S_TRADING);
                                trader = u2;
                            }
			            }
				    }
				    
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
						u->AdjustSkills();      //no overflow when acquiring new men into a unit
						delete sl;
						/* region economy effects */
						r->Recruit(temp);
					}
					u->items.SetNum(o->item, u->items.GetNum(o->item) + temp);
					u->faction->DiscoverItem(o->item, 0, 1);
					u->ConsumeSharedMoney((temp * m->price * (20-bonus) + 19) / 20);
					if(trader) trader->numtraded += (temp * m->price * (20-bonus) + 19) / 20;
					u->buyorders.Remove(o);
					u->Event(AString("Buys ") + ItemString(o->item, temp)
							+ " at $" + (m->price * (20-bonus) + 19) / 20 + " each.");
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
	/* First pass: set needed */
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				u->needed = u->MaintCost();
				if(u->dead) u->needed = 0; /* ARCADIA_MAGIC Patch */
				u->hunger = u->GetMen() * Globals->UPKEEP_MINIMUM_FOOD;
				if(u->dead) u->hunger = 0; /* ARCADIA_MAGIC Patch */
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
	if(Globals->FOOD_ITEMS_EXIST) {
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

	if(Globals->FOOD_ITEMS_EXIST) {
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
	if(Globals->FOOD_ITEMS_EXIST) {
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
					if(DoWithdrawOrder(r, u, o)) break;
				}
				u->withdraworders.DeleteAll();
				
				forlist_reuse((&u->wishdraworders)) {
					WishdrawOrder *o = (WishdrawOrder *)elem;
					if(DoWishdrawOrder(r, u, o)) break;
				}
				u->wishdraworders.DeleteAll();
				
				forlist_reuse((&u->wishskillorders)) {
					WishskillOrder *o = (WishskillOrder *)elem;
					if(DoWishskillOrder(r, u, o)) break;
				}
				u->wishskillorders.DeleteAll();
			}
		}
	}
}

int Game::DoWithdrawOrder(ARegion *r, Unit *u, WithdrawOrder *o)
{
	int itm = o->item;
	int amt = o->amount;
	int cost = (ItemDefs[itm].baseprice *5/2)*amt;

	if(r->type == R_NEXUS) {
		u->Error("WITHDRAW: Withdraw does not work in the Nexus.", o->quiet);
		return 1;
	}

	if (cost > u->faction->unclaimed) {
		u->Error(AString("WITHDRAW: Too little unclaimed silver to withdraw ")+
				ItemString(itm, amt)+".", o->quiet);
		return 0;
	}

	if (ItemDefs[itm].max_inventory) {
		int cur = u->items.GetNum(itm) + amt;
		if (cur > ItemDefs[itm].max_inventory) {
			u->Error(AString("WITHDRAW: Unit cannot have more than ")+
					ItemString(itm, ItemDefs[itm].max_inventory), o->quiet);
			return 0;
		}
	}

	u->faction->unclaimed -= cost;
	u->Event(AString("Withdraws ") + ItemString(o->item, amt) + ".");
	u->items.SetNum(itm, u->items.GetNum(itm) + amt);
	u->faction->DiscoverItem(itm, 0, 1);
	return 0;
}

int Game::DoWishdrawOrder(ARegion*, Unit *u, WishdrawOrder *o)
{
	int itm = o->item;
	int amt = o->amount;
	
/* //disable this check for a testgame!
	if (!u->faction->IsNPC) {
		u->Error(AString("WISHDRAW: This faction is not a NPC."));
		return 1;
	}
*/	

	if (ItemDefs[itm].max_inventory) {
		int cur = u->items.GetNum(itm) + amt;
		if (cur > ItemDefs[itm].max_inventory) {
			u->Error(AString("WISHDRAW: Unit cannot have more than ")+
					ItemString(itm, ItemDefs[itm].max_inventory), o->quiet);
			return 0;
		}
	}

	u->Event(AString("Wishdraws ") + ItemString(o->item, amt) + ".");
	u->items.SetNum(itm, u->items.GetNum(itm) + amt);
	u->faction->DiscoverItem(itm, 0, 1);
	return 0;
}

int Game::DoWishskillOrder(ARegion*, Unit *u, WishskillOrder *o)
{
	int sk = o->skill;
	int days = o->knowledge * u->GetMen();
	int exper = o->experience * u->GetMen();

/* //disable this check for a testgame!
	if (!u->faction->IsNPC) {
		u->Error(AString("WISHDRAW: This faction is not a NPC."));
		return 1;
	}
*/	

	if((SkillDefs[sk].flags & SkillType::MAGIC) && u->type != U_MAGE) {
		if(u->type == U_APPRENTICE) {
			u->Error("WISHSKILL: An apprentice cannot be made into an mage.", o->quiet);
			return 0;
		}
		if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
			if (CountMages(u->faction) >= AllowedMages(u->faction)) {
				u->Error("WISHSKILL: Can't have another magician.", o->quiet);
				return 0;
			}
		}
		if (u->GetMen() != 1) {
			u->Error("WISHSKILL: Only 1-man units can be magicians.", o->quiet);
			return 0;
		}
		if(!(Globals->MAGE_NONLEADERS)) {
			if (u->IsLeader() != 1) {
				u->Error("WISHSKILL: Only leaders may study magic.", o->quiet);
				return 0;
			}
		}
		u->type = U_MAGE;
	}

	if((SkillDefs[sk].flags&SkillType::APPRENTICE) &&
			u->type != U_APPRENTICE) {
		if(u->type == U_MAGE) {
			u->Error("WISHSKILL: A mage cannot be made into an apprentice.", o->quiet);
			return 0;
		}

		if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
			if(CountApprentices(u->faction)>=AllowedApprentices(u->faction)) {
				u->Error("WISHSKILL: Can't have another apprentice.", o->quiet);
				return 0;
			}
		}
		if(u->GetMen() != 1) {
			u->Error("WISHSKILL: Only 1-man units can be apprentices.", o->quiet);
			return 0;
		}
		if(!(Globals->MAGE_NONLEADERS)) {
			if(u->IsLeader() != 1) {
				u->Error("WISHSKILL: Only leaders may be apprentices.", o->quiet);
				return 0;
			}
		}
		u->type = U_APPRENTICE;
	}
	if ((Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) &&
			(sk == S_QUARTERMASTER) && (u->GetRealSkill(S_QUARTERMASTER) == 0) &&
			(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES)) {
			if (CountQuarterMasters(u->faction) >=
					AllowedQuarterMasters(u->faction)) {
				u->Error("WISHSKILL: Can't have another quartermaster.", o->quiet);
				return 0;
			}
			if(u->GetMen() != 1) {
				u->Error("WISHSKILL: Only 1-man units can be quartermasters.", o->quiet);
				return 0;
			}
	}

	if(Globals->SKILL_LIMIT_NONLEADERS && u->IsNormal()) {
		if (u->skills.Num()) {
			Skill * s = (Skill *) u->skills.First();
			if (s->type != sk) {
				u->Error("WISHSKILL: Can know only 1 skill.", o->quiet);
				return 0;
			}
		}
	}

	u->skills.SetDays(sk, days, exper);
	u->AdjustSkills(0);

	/* Check to see if we need to show a skill report */
	int lvl = u->GetRealSkill(sk);
	int oldlvl = u->faction->skills.GetDays(sk);
	if (lvl > oldlvl) {
	    for(int i=oldlvl+1; i<=lvl; i++) u->faction->shows.Add(new ShowSkill(sk, i));
		u->faction->skills.SetDays(sk, lvl);
	}
	u->Event("Wishskills unit");
	return 0;
}

void Game::DoBankDepositOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *)elem;
		forlist((&r->objects)) {
			Object *obj = (Object *)elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				forlist((&u->bankorders)) {
					BankOrder *o = (BankOrder *)elem;
					if (o->what == 2) // deposit
						DoBankOrder(r, u, o);
				}
			}
		}
	}
}

void Game::DoBankWithdrawOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *)elem;
		forlist((&r->objects)) {
			Object *obj = (Object *)elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				forlist((&u->bankorders)) {
					BankOrder *o = (BankOrder *)elem;
					DoBankOrder(r, u, o);
				}
				u->bankorders.DeleteAll();
			}
		}
	}
}

void Game::DoBankOrder(ARegion *r, Unit *u, BankOrder *o)
{
	int what = o->what;
	int amt = o->amount;
	int max;// = o->max;
	int lvl;// = o->level;
	int inbank;// = o->inbank;
	int fee;

	if(r->type == R_NEXUS) {
		u->Error("BANK: does not work in the Nexus.", o->quiet);
		u->bankorders.Remove(o);
		return;
	}
	if (!(SkillDefs[S_BANKING].flags & SkillType::DISABLED)) { // banking skill ?
		lvl = u->GetSkill(S_BANKING);
	} else { // skill disabled - pretend level 5
		lvl = 5;
	}
	if (!(ObjectDefs[O_OBANK].flags & ObjectType::DISABLED)) { // banks enabled ?
		if (u->object->type != O_OBANK) // Are they in Bank ?
			inbank = 0; // No they are not
		else { // Yes they are in bank
			if (u->object->incomplete > 0) // Is it completed ?
				inbank = 0; // Not completed
			else
				inbank = 1; // Completed
		}
		if (inbank)
			max = Globals->BANK_MAXSKILLPERLEVEL * lvl * inbank;
		else
			max = Globals->BANK_MAXUNSKILLED;
	} else { // banks disabled - pretend they are in a bank
		inbank = 1;
		max = Globals->BANK_MAXSKILLPERLEVEL * lvl;
	}

	if (!r->CanTax(u) && (Globals->ALLOW_BANK & GameDefs::BANK_NOTONGUARD)) {
		if (ObjectDefs[O_OBANK].flags & ObjectType::DISABLED) {
			// if banks are disabled, inbank will be 1, so ignore it
			//FIXME
			u->Error("BANK1: A unit is on guard - banking is not allowed.", o->quiet);
			u->bankorders.Remove(o);
			return;
		} else { // pay attention to inbank
			if (!inbank) { // if a unit is in a bank, then allow nevertheless
				u->Error("BANK2: A unit is on guard - banking is not allowed.", o->quiet); //FIXME
				u->bankorders.Remove(o);
				return;
			}
		}
	}

	if(!u->object->region->town && (Globals->ALLOW_BANK & GameDefs::BANK_INSETTLEMENT)) {
		if (ObjectDefs[O_OBANK].flags & ObjectType::DISABLED) { // if banks are disabled, inbank will be 1, so ignore it
			u->Error("BANK: Unit is not in a village, town or city.", o->quiet);
			u->bankorders.Remove(o);
			return;
		} else { // pay attention to inbank
			if (!inbank) { // if a unit is in a bank, then allow nevertheless
				u->Error("BANK: Unit is not in a village, town or city.", o->quiet);
				u->bankorders.Remove(o);
				return;
			}
		}
	}

	if ((amt > u->faction->bankaccount) && (what == 1)) {
		u->Error(AString("BANK: Too little silver in the bank to withdraw."), o->quiet);
		u->bankorders.Remove(o);
		return;
	}
	if (what == 1) { // withdraw
		if (amt > max) {
			AString temp = "BANK: Withdrawal limited to ";
			temp += max;
			temp += " silver.";
			u->Error(temp, o->quiet);
			amt = max;
		}
	} else { // deposit
		if (u->items.GetNum(I_SILVER) == 0) {
			u->Error(AString("BANK: No silver available."), o->quiet);
			u->bankorders.Remove(o);
			return;
		} else {
			if (amt > max) {
				AString temp = "BANK: Deposit limited to ";
				temp += max;
				temp += " silver.";
				u->Error(temp, o->quiet);
				amt = max;
			}
			if (u->items.GetNum(I_SILVER) < amt)
				amt = u->items.GetNum(I_SILVER);
		}
	}
	if (Globals->ALLOW_BANK & GameDefs::BANK_FEES)
		fee = (amt * Globals->BANK_FEE)/100;
	else
		fee = 0;
	AString temp;
	if (what == 2)
		temp += "Deposits ";
	else
		temp += "Withdraws ";
	temp += amt - fee;
	if (what == 2)
		temp += " in";
	else
		temp += " from";
	temp += " the bank";
	if (Globals->ALLOW_BANK & GameDefs::BANK_FEES) {
		temp += " (fees ";
		temp += fee;
		temp += ")";
	}
	temp += ".";
	u->Event(temp);
	if (what == 2) {// deposit
		u->faction->bankaccount += amt - fee;
		u->items.SetNum(I_SILVER, u->items.GetNum(I_SILVER) - amt);
	} else { // withdrawal
		u->faction->bankaccount -= amt;
		u->items.SetNum(I_SILVER, u->items.GetNum(I_SILVER) + amt - fee);
	}

	u->bankorders.Remove(o);
	return;
}

void Game::DoGiveOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				forlist((&u->giveorders)) {
					GiveOrder *o = (GiveOrder *)elem;
					if(o->item < 0) {
						if (o->amount == -1) {
							/* do 'give X unit' command */
							DoGiveOrder(r, u, o);
						} else if (o->amount == -2) {
							/* do 'give all type' command */
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
						} else {
							u->Error("GIVE: Invalid item.", o->quiet);
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

void Game::DoSendOrders()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				forlist((&u->sendorders)) {
					SendOrder *o = (SendOrder *)elem;
					if(o->item < 0) {
						if (o->amount == -2) {
							/* do 'send all type' command */
							forlist((&u->items)) {
								Item *item = (Item *)elem;
								if( ((o->item == -NITEMS) ||
									(ItemDefs[item->type].type & (-o->item))) &&
                                    !(ItemDefs[item->type].type & IT_MAN) &&
                                    !(ItemDefs[item->type].type & IT_MONSTER) &&
                                    !(ItemDefs[item->type].type & IT_ILLUSION)) {
									SendOrder so;
									so.amount = item->num;
									so.except = 0;
									so.item = item->type;
									so.target = o->target;
									so.direction = o->direction;
									so.type = o->type;
									DoSendOrder(r, u, &so);
									so.target = NULL;
								}
							}
						} else {
							u->Error("SEND: Invalid item.", o->quiet);
						}
					} else if (DoSendOrder(r, u, o)) {
						break;
					}
				}
				u->sendorders.DeleteAll();
			}
		}
	}
}


void Game::RecieveSentGoods()
{
	forlist((&regions)) {
		ARegion *r = (ARegion *) elem;
		forlist((&r->objects)) {
			Object *obj = (Object *) elem;
			forlist((&obj->units)) {
				Unit *u = (Unit *) elem;
				if(u->itemsintransit.Num()) {
                    forlist(&u->itemsintransit) {
                        Item *i = (Item *) elem;
                        u->items.SetNum(i->type,u->items.GetNum(i->type) + i->num);
                    }
                    u->itemsintransit.DeleteAll();
				}
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
		u->Error(AString("EXCHANGE: Nonexistent target (") +
				o->target->Print() + ").", o->quiet);
		u->exchangeorders.Remove(o);
		return;
	}

	// Check each Item can be given
	if(ItemDefs[o->giveItem].flags & ItemType::CANTGIVE) {
		u->Error(AString("EXCHANGE: Can't trade ") +
				ItemDefs[o->giveItem].names + ".", o->quiet);
		u->exchangeorders.Remove(o);
		return;
	}

	if(ItemDefs[o->expectItem].flags & ItemType::CANTGIVE) {
		u->Error(AString("EXCHANGE: Can't trade ") +
				ItemDefs[o->expectItem].names + ".", o->quiet);
		u->exchangeorders.Remove(o);
		return;
	}

	if (ItemDefs[o->giveItem].type & IT_MAN) {
		u->Error("EXCHANGE: Exchange aborted.  Men may not be traded.", o->quiet);
		u->exchangeorders.Remove(o);
		return;
	}

	if (ItemDefs[o->expectItem].type & IT_MAN) {
		u->Error("EXCHANGE: Exchange aborted. Men may not be traded.", o->quiet);
		u->exchangeorders.Remove(o);
		return;
	}

	// New RULE -- Must be able to see unit to give something to them!
	if(!u->CanSee(r, t)) {
		u->Error(AString("EXCHANGE: Nonexistent target (") +
				o->target->Print() + ").", o->quiet);
		return;
	}
	// Check other unit has enough to give
	//This is not compatible to item-sharing because the shared items may be traded with others before the exchange is executed.
	//In fact, this applies to normal Atlantis too, as the items could be exchanged before the opposing exchange order is met - so there is a bug in the original code here
/*	int amtRecieve = o->expectAmount;
	if (amtRecieve > t->items.GetNum(o->expectItem)) {
		t->Error(AString("EXCHANGE: Not giving enough. Expecting ") +
				ItemString(o->expectItem, o->expectAmount) + ".");
		u->Error(AString("EXCHANGE: Exchange aborted.  Not enough ") +
				"recieved. Expecting " +
			ItemString(o->expectItem, o->expectAmount) + ".");
		o->exchangeStatus = 0;
		return;
	}*/
	
	//exchange status is initially -1
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
								".", o->quiet);
						u->Error(AString("EXCHANGE: Exchange aborted. ") +
								"Not enough recieved. Expecting " +
								ItemString(o->expectItem, o->expectAmount) +
								".", o->quiet);
						tOrder->exchangeStatus = 0;
						o->exchangeStatus = 0;
						return;
					} else if (tOrder->giveAmount > o->expectAmount) {
						t->Error(AString("EXCHANGE: Exchange aborted. Too ") +
								"much given. Expecting " +
								ItemString(o->expectItem, o->expectAmount) +
								".", o->quiet);
						u->Error(AString("EXCHANGE: Exchange aborted. Too ") +
								"much offered. Expecting " +
								ItemString(o->expectItem, o->expectAmount) +
								".", o->quiet);
						tOrder->exchangeStatus = 0;
						o->exchangeStatus = 0;
					} else if (tOrder->giveAmount == o->expectAmount)
						o->exchangeStatus = 1;

					if ((o->exchangeStatus == 1) &&
							(tOrder->exchangeStatus == 1) && 
                            u->GetSharedNum(o->giveItem, o->giveAmount) &&
                            t->GetSharedNum(tOrder->giveItem, tOrder->giveAmount)) {
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
						t->items.SetNum(o->giveItem, t->items.GetNum(o->giveItem) + o->giveAmount);
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
		if(!u->CanSee(r, t)) {
			u->Error(AString("EXCHANGE: Nonexistent target (") +
					o->target->Print() + ").", o->quiet);
			u->exchangeorders.Remove(o);
			return;
		} else {
			u->Error("EXCHANGE: target unit did not issue a matching "
					"exchange order.", o->quiet);
			u->exchangeorders.Remove(o);
			return;
		}
	}
}

int Game::DoGiveOrder(ARegion *r, Unit *u, GiveOrder *o)
{
	// Check there is enough to give
	int amt = o->amount;
	if (amt != -2 && amt > u->items.GetNum(o->item)) {
		u->Error("GIVE: Not enough.", o->quiet);
		amt = u->items.GetNum(o->item);
	} else if (amt == -2) {
		amt = u->items.GetNum(o->item);
		if(o->except) {
			if(o->except > amt) {
				amt = 0;
				u->Error("GIVE: EXCEPT value greater than amount on hand.", o->quiet);
			} else {
				amt = amt - o->except;
			}
		}
	}

	if (o->target->unitnum == -1) {
		/* Give 0 */
		if (amt == -1) {
            Faction *civfac = GetFaction(&factions, peasantfaction);
            if(!civfac) {
    			u->Error("Can't discard a whole unit.", o->quiet);
    			return 0;
			}
    		u->Event(AString("Gives unit to ") + *(civfac->name) + ".");
    		u->faction = civfac;
    		u->Event("Is given to your faction.");
    		u->guard = GUARD_AVOID;
    		u->reveal = REVEAL_FACTION;
            AString *temp = new AString("Peasantfolk");
    		u->SetName(temp);
    		u->ClearOrders();
    		return 1; //returning 1 means it does no further give orders.
		}

		if (amt < 0) {
			u->Error("Cannot give a negative number of items.", o->quiet);
			return 0;
		}
		
		if(ItemDefs[o->item].flags & ItemType::NEVERLOST) {
            u->Error("Cannot discard that item.", o->quiet);
            return 0;
        }

		AString temp = "Discards ";
		if (ItemDefs[o->item].type & IT_MAN) {
			u->SetMen(o->item, u->GetMen(o->item) - amt);
			r->DisbandInRegion(o->item, amt);
			temp = "Disbands ";
		} else if(Globals->RELEASE_MONSTERS &&
				(ItemDefs[o->item].type & IT_MONSTER)) {
			temp = "Releases ";
			u->items.SetNum(o->item, u->items.GetNum(o->item) - amt);
			if(Globals->WANDERING_MONSTERS_EXIST) {
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
			u->items.SetNum(o->item, u->items.GetNum(o->item) - amt);
		}
		u->Event(temp + ItemString(o->item, amt) + ".");
		return 0;
	}

	Unit *t = r->GetUnitId(o->target, u->faction->num);
	if (!t) {
		u->Error(AString("GIVE: Nonexistent target (") + o->target->Print() +
				").", o->quiet);
		return 0;
	}

	if(u == t) {
		u->Error(AString("GIVE: Attempt to give ")+ItemString(o->item, amt)+
				" to self.", o->quiet);
		return 0;
	}

	// New RULE -- Must be able to see unit to give something to them!
	if(!u->CanSee(r, t) &&
			(t->faction->GetAttitude(u->faction->num) < A_FRIENDLY)) {
		u->Error(AString("GIVE: Nonexistent target (") + o->target->Print() +
				").", o->quiet);
		return 0;
	}

	if (o->item != I_SILVER &&
			t->faction->GetAttitude(u->faction->num) < A_FRIENDLY) {
	    //This eliminates guards, monsters, ghosts and peasant factions.
		u->Error("GIVE: Target is not a member of a friendly faction.", o->quiet);
		return 0;
	}


	if (amt == -1) {
		/* Give unit */
		if(u->GetFlag(FLAG_COMMANDER)) {
		    u->Error("GIVE: Cannot GIVE your commanding unit.");
		    return 0;
		}
		if (u->type == U_MAGE) {
			if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
				if (CountMages(t->faction) >= AllowedMages(t->faction)) {
					u->Error("GIVE: Faction has too many mages.", o->quiet);
					return 0;
				}
			}
		}
		if(u->type == U_APPRENTICE) {
			if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
				if(CountApprentices(t->faction) >=
						AllowedApprentices(t->faction)){
					u->Error("GIVE: Faction has too many apprentices.", o->quiet);
					return 0;
				}
			}
		}

		if (u->GetRealSkill(S_QUARTERMASTER)) {
			if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
				if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
					if (CountQuarterMasters(t->faction) >=
							AllowedQuarterMasters(t->faction)) {
						u->Error("GIVE: Faction has too many quartermasters.", o->quiet);
						return 0;
					}
				}
			}
		}

		if (u->GetRealSkill(S_TACTICS) == 5) {
			if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
				if (Globals->TACTICS_NEEDS_WAR) {
					if (CountTacticians(t->faction) >=
							AllowedTacticians(t->faction)) {
						u->Error("GIVE: Faction has too many tacticians.", o->quiet);
						return 0;
					}
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
			u->Error("Unit cannot advance after being given.", 0);
			delete u->monthorders;
			u->monthorders = 0;
		}

		/* Check if any new skill reports have to be shown */
		forlist(&(u->skills)) {
			Skill *skill = (Skill *) elem;
			int newlvl = u->GetRealSkill(skill->type);
			int oldlvl = u->faction->skills.GetDays(skill->type);
			if (newlvl > oldlvl) {
				for (int i=oldlvl+1; i<=newlvl; i++) {
					u->faction->shows.Add(new ShowSkill(skill->type, i));
				}
				u->faction->skills.SetDays(skill->type, newlvl);
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

		return notallied;  //if it returns 1, it does no further give orders.
	}

	int newtype = -1;

	/* If the item to be given is a man, combine skills */
	if (ItemDefs[o->item].type & IT_MAN) {
		if (u->type == U_MAGE || u->type == U_APPRENTICE ||
				t->type == U_MAGE || t->type == U_APPRENTICE) {
			u->Error("GIVE: Heroes can't give or recieve men.", o->quiet);
			return 0;
		}
		if (u->type != t->type) {
		    if(t->GetMen()) {
    		    //changed for new leader handling
    		    u->Error("GIVE: Can't mix leaders and normal men.", o->quiet);
        		return 0;
    		} else {
    		    newtype = u->type;
    		}
		}
		
		// Small hack for Ceran
		if(o->item == I_MERC && t->GetMen()) {
			u->Error("GIVE: Can't mix mercenaries with other men.", o->quiet);
			return 0;
		}

		if (u->faction != t->faction) {
			u->Error("GIVE: Can't give men to another faction.", o->quiet);   //would be nice to be able to give men to a unit which you have just given to another faction
			return 0;
		}

		if (u->nomove) t->nomove = 1;

		SkillList *temp = u->skills.Split(u->GetMen(), amt);   //Is it really a good idea to do this here, when men could potentially be labelled as cantgive or max_inventory below?
		t->skills.Combine(temp);
		delete temp;
		
		//mark which units men have gone to.
        UnitId *id = new UnitId;
		id->unitnum = t->num;
		if(o->target) id->alias = o->target->alias;
		else id->alias = 0;
		id->faction = t->faction->num;
		u->gavemento.Add(id);
	}

	if(ItemDefs[o->item].flags & ItemType::CANTGIVE) {
		u->Error(AString("GIVE: Can't give ") + ItemDefs[o->item].names + ".", o->quiet);
		return 0;
	}

	if (ItemDefs[o->item].max_inventory) {
		int cur = t->items.GetNum(o->item) + amt;
		if (cur > ItemDefs[o->item].max_inventory) {
			u->Error(AString("GIVE: Unit cannot have more than ")+
					ItemString(o->item, ItemDefs[o->item].max_inventory), o->quiet);
			return 0;
		}
	}

	u->Event(AString("Gives ") + ItemString(o->item, amt) + " to " +
			*t->name + ".");
	if (u->faction != t->faction) {
		t->Event(AString("Receives ") + ItemString(o->item, amt) +
				" from " + *u->name + ".");
	}
	u->items.SetNum(o->item, u->items.GetNum(o->item) - amt);
	t->items.SetNum(o->item, t->items.GetNum(o->item) + amt);
	t->faction->DiscoverItem(o->item, 0, 1);

	if (ItemDefs[o->item].type & IT_MAN) {
	    if(newtype != -1) t->type = newtype;
		t->AdjustSkills();      //no overflow when acquiring new men into a unit
	}
	return 0;
}

int Sqrt(int input)
{
    int num = 1;
    while(num*num <= input && num < 1000) num++;
    return(num-1);
    
    //if input is 0 or less, returns 0.
}


int Game::DoSendOrder(ARegion *r, Unit *u, SendOrder *o)
{
	// Check there is enough to send
	int amt = o->amount;
	if (amt != -2 && amt > u->GetSharedNum(o->item)) {
		amt = u->GetSharedNum(o->item);
		u->Error(AString("SEND: Not enough. Sending ") + ItemString(o->item, amt) + " instead.", o->quiet);
	} else if (amt == -2) {
		amt = u->items.GetNum(o->item);  //if sending "ALL" do not check other inventories.
		if(o->except) {
			if(o->except > amt) {
				amt = 0;
				u->Error("SEND: EXCEPT value greater than amount on hand.", o->quiet);
				return 0;
			} else {
				amt = amt - o->except;
			}
		}
	}
	
	Unit *tar = NULL;
    ARegion *reg = NULL;
	if(o->direction >= 0) {
        if(o->direction < NDIRS && r->neighbors[o->direction])
            reg = r->neighbors[o->direction];
        else if(o->direction == MOVE_IN) {
            if(u->object->inner >= 0) {
                reg = regions.GetRegion(u->object->inner);
            } else {
			    u->Error("SEND: Cannot send IN there.", o->quiet);
			    return 0;
            }
        } else if(o->direction > MOVE_ENTER) {
            int inreg = r->GetObject(o->direction - MOVE_ENTER)->inner;
            if(inreg < 0) {
			    u->Error("SEND: Cannot send IN that.", o->quiet);
			    return 0;
            } else reg = regions.GetRegion(inreg);
        }
        if(!reg) {
		    u->Error("SEND: No region in that direction.", o->quiet);
		    return 0;
        }
        //we now have the specified region 'reg'.
        if(o->target) {
            tar = reg->GetUnitId(o->target, u->faction->num);
        } else {
            //no specified target unit!
            forlist(&reg->objects) {
        		Object *o = (Object *) elem;
        		if(tar) continue;
        		forlist(&o->units) {
        			Unit *unit = (Unit *) elem;
        			if (unit->faction == u->faction && unit->IsStationary()) {
                        tar = unit;
                        break;
                    }
        		}
            }
        }
        if(!tar) {
            u->Error("SEND: No unit in that direction able to recieve sent goods");
            return 0;
        }
    } else {
           //try all directions for target unit
        int i=0;
        while(i<NDIRS && !tar) {
            if(r->neighbors[i])
                tar = r->neighbors[i]->GetUnitId(o->target, u->faction->num);
            i++;
        }
        if(tar) {
            //setting up reg and direction for later
            o->direction = i-1;
            reg = r->neighbors[i-1];
        }
    }
    int cost = 0;
    Unit *bestquartermaster = NULL;
    if (o->via || (!tar && o->target) ) {
        Location *target = regions.GetUnitId(o->target, u->faction->num, r);
        Location *quartermaster = NULL;

        if(o->via) quartermaster = regions.GetUnitId(o->via, u->faction->num, r);
        int level = 0;
        if(u->GetSkill(S_ARCADIA_QUARTERMASTERY) > level) {
            level = u->GetSkill(S_ARCADIA_QUARTERMASTERY);
            bestquartermaster = u;
        }
        if(target && target->unit->GetSkill(S_ARCADIA_QUARTERMASTERY) > level) {
            level = target->unit->GetSkill(S_ARCADIA_QUARTERMASTERY);
            bestquartermaster = target->unit;
        }
        if(quartermaster && quartermaster->unit->GetSkill(S_ARCADIA_QUARTERMASTERY) > level) {
            level = quartermaster->unit->GetSkill(S_ARCADIA_QUARTERMASTERY); //is ok to use other level if greater, as that is equivalent to not using the VIA command at all ...
            bestquartermaster = quartermaster->unit;
        }
        if(!level || !target) {
            //the target is not adjacent, and no-one has a quartermastery skill.
            if(target) delete target;
            if(quartermaster) delete quartermaster;
    		u->Error(AString("SEND: Cannot find target ") + o->target->unitnum + " or target is not a member of a friendly faction", o->quiet);
    		return 0;
        }
        //we now have a SEND order using the QUARTERMASTERY skill.
        if (quartermaster && quartermaster->unit->faction->GetAttitude(u->faction->num) < A_FRIENDLY) {
    	    //This eliminates guards, monsters, ghosts and peasant factions.
    	    //These errors all have to be the same to prevent any way of players finding the location of stealth units.
    		u->Error(AString("SEND: Cannot find target  ") + o->via->unitnum + " or target is not a member of a friendly faction", o->quiet);
	        delete target;
	        if(quartermaster) delete quartermaster;
		    return 0;
	    }
	    if(quartermaster && level < 4) {
    		u->Error("SEND: Insufficient quartermastery level to use VIA", o->quiet);
	        delete target;
	        if(quartermaster) delete quartermaster;
		    return 0;
        }
	    
	    if (quartermaster && !quartermaster->unit->IsStationary()) {
		    u->Error(AString("SEND: Cannot send goods via a moving unit."), o->quiet);
	        delete target;
	        if(quartermaster) delete quartermaster;
		    return 0;
        }
	    // Need to check the ranges
		// make sure target is in range.
		int dist = regions.GetDistance(r, target->region);
		if(quartermaster) dist = regions.GetDistance(r,quartermaster->region)+regions.GetDistance(quartermaster->region,target->region);
		if (dist > level) {
			u->Error("SEND: Target is too far away.", o->quiet);
	        delete target;
	        if(quartermaster) delete quartermaster;
		    return 0;
		}

        //all done, set up for final code.
	    tar = target->unit;
	    delete target;
	    if(quartermaster) delete quartermaster;
	} else {
        //need to stick in here any checks that DO NOT apply to QUARTERMASTERY skilled transfers.       

	    //we now have the target unit 'tar', which is stationary, the item 'o->item',
        //the amount 'amt' and the target region 'reg'. We should now transfer it.
        //Check if the target hex can be walked to.
        int level = u->GetSkill(S_ARCADIA_QUARTERMASTERY);
        if(level) bestquartermaster = u;
        if( tar->GetSkill(S_ARCADIA_QUARTERMASTERY) > level) {
            level = tar->GetSkill(S_ARCADIA_QUARTERMASTERY);
            bestquartermaster = tar;
        }
        
        int multiplier = 1;
        cost = reg->MoveCost(M_WALK, r, o->direction, 0); //movement cost
        if(cost < 0 && !level) {
    		u->Error(AString("SEND: Intervening terrain blocks sending."), o->quiet);
    		return 0;
        }
    	if(cost > Globals->FOOT_SPEED) multiplier = 2;
    	if((cost > Globals->HORSE_SPEED) && !level) {
    		u->Error(AString("SEND: Target region cannot be ridden to."), o->quiet);
    		return 0;
    	}
    	
    	int weight = Sqrt(ItemDefs[o->item].weight - ItemDefs[o->item].walk);
    	if(multiplier > 1) weight = Sqrt(ItemDefs[o->item].weight - ItemDefs[o->item].ride);
    //	if(weight < 1) weight = 1;  //this makes weightless items cost money to send
    	
    	cost = amt*weight*multiplier*Globals->SEND_COST; //silver cost
    	if(level) cost = 0; //quartermasters get free transport.
    }
	if (tar->faction->GetAttitude(u->faction->num) < A_FRIENDLY) {
	    //This eliminates guards, monsters, ghosts and peasant factions.
		u->Error("SEND: Cannot find target or target is not a member of a friendly faction", o->quiet);
		return 0;
	}
	
	if (!tar->IsStationary()) {
		u->Error(AString("SEND: Cannot send goods to a moving unit."), o->quiet);
		return 0;
    }
    
    //last minute checks that item type is ok. Should never be needed, but just in case.
    if( (ItemDefs[o->item].flags & ItemType::CANTGIVE) || 
                (ItemDefs[o->item].type & IT_MAN) || 
                (ItemDefs[o->item].type & IT_MONSTER) || 
                (ItemDefs[o->item].type & IT_ILLUSION) ) {
		u->Error(AString("SEND: Can't send ") + ItemDefs[o->item].names + ".", o->quiet);
		return 0;
	}
	if (ItemDefs[o->item].max_inventory) {
		int cur = tar->items.GetNum(o->item) + tar->itemsintransit.GetNum(o->item) + amt;
		if (cur > ItemDefs[o->item].max_inventory) {
			u->Error(AString("SEND: Target unit cannot have more than ")+
					ItemString(o->item, ItemDefs[o->item].max_inventory), o->quiet);
			return 0;
		}
	}

    //cost is zero if a quartermaster transfer!    
	if(!u->GetSharedMoney(cost)) {
        u->Error(AString("SEND: Not enough silver to SEND that."), o->quiet);
        return 0;
    }   //don't forget to double count if sending silver and also paying with it:
    if(o->item == I_SILVER && cost+amt > u->GetSharedNum(I_SILVER)) {
        u->Error(AString("SEND: Not enough silver to SEND that."), o->quiet);
        return 0;
    }

    u->ConsumeSharedMoney(cost);        //pays SEND cost

	u->Event(AString("Sends ") + ItemString(o->item, amt) + " to " +
			*tar->name + " at a cost of " + cost + " silver.");
	if(u->faction != tar->faction) tar->Event(AString("Is sent ") + ItemString(o->item, amt) +
            " from " + *u->name);
	u->ConsumeShared(o->item, amt);
	tar->itemsintransit.SetNum(o->item, tar->itemsintransit.GetNum(o->item) + amt);
	tar->faction->DiscoverItem(o->item, 0, 1);

	if(bestquartermaster) bestquartermaster->numquartermastered += amt*ItemDefs[o->item].baseprice;

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
			if (unit->IsAlive() == 0) {  //don't want to discard mages here, so don't use "reallyalive"! if no men, then returns 0 anyway.
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
						u->Error(ordertype + ": Target does not exist.", o->quiet);
						o->type = NORDERS;
						continue;
					}

					Location *tar = regions.GetUnitId(o->target,
							u->faction->num, r);
					if (!tar) {
						u->Error(ordertype + ": Target does not exist.", o->quiet);
						o->type = NORDERS;
						continue;
					}

					// Make sure target isn't self
					if (tar->unit == u) {
						u->Error(ordertype + ": Target is self.", o->quiet);
						o->type = NORDERS;
						continue;
					}

					// Make sure the target and unit are at least friendly
					if (tar->unit->faction->GetAttitude(u->faction->num) <
							A_FRIENDLY) {
						u->Error(ordertype +
								": Target is not a member of a friendly "
								"faction.", o->quiet);
						o->type = NORDERS;
						continue;
					}

					// Make sure the target of a transport order is a unit
					// with the quartermaster skill who owns a transport
					// structure
					if (o->type == O_TRANSPORT) {
						if (tar->unit->GetSkill(S_QUARTERMASTER) == 0) {
							u->Error(ordertype +
									": Target is not a quartermaster", o->quiet);
							o->type = NORDERS;
							continue;
						}
						if ((tar->obj->GetOwner() != tar->unit) ||
								!(ObjectDefs[tar->obj->type].flags &
									ObjectType::TRANSPORT) ||
								(tar->obj->incomplete > 0)) {
							u->Error(ordertype + ": Target does not own "
									"a transport structure.", o->quiet);
							o->type = NORDERS;
							continue;
						}
					}

					// make sure target is in range.
					int maxdist;
					int dist = regions.GetDistance(r, tar->region);
					if ((o->type == O_TRANSPORT) &&
						(u == obj->GetOwner()) &&
						(ObjectDefs[obj->type].flags & ObjectType::TRANSPORT)) {
						maxdist = Globals->NONLOCAL_TRANSPORT;
						if (maxdist >= 0 &&
							Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST) {
							int level = u->GetSkill(S_QUARTERMASTER);
							maxdist += ((level + 1)/3);
						} else if (maxdist == 0)
							maxdist = 10000000;
					} else
						maxdist = Globals->LOCAL_TRANSPORT;
					if (dist > maxdist) {
						u->Error(ordertype + ": Recipient is too far away.", o->quiet);
						o->type = NORDERS;
						continue;
					}

					// On long range transport or distribute, make sure the
					// issuer is a quartermaster and is owner of a structure
					if ((o->type == O_DISTRIBUTE) ||
						((dist > Globals->LOCAL_TRANSPORT) &&
						 (o->type == O_TRANSPORT))) {
						if (u->GetSkill(S_QUARTERMASTER == 0)) {
							u->Error(ordertype +
									": Unit is not a quartermaster", o->quiet);
							o->type = NORDERS;
							continue;
						}
						if ((u != obj->GetOwner()) ||
							!(ObjectDefs[obj->type].flags &
								ObjectType::TRANSPORT) ||
							(obj->incomplete > 0)) {
						u->Error(ordertype +
								": Unit does not own transport structure.", o->quiet);
						o->type = NORDERS;
						continue;
						}
					}

					// make sure amount is available (all handled later)
					if (o->amount > 0 && o->amount > u->items.GetNum(o->item)) {
						u->Error(ordertype + ": Not enough.", o->quiet);
						o->type = NORDERS;
						continue;
					}

					if (o->amount > 0 && ItemDefs[o->item].max_inventory) {
						int cur = tar->unit->items.GetNum(o->item) + o->amount;
						if (cur > ItemDefs[o->item].max_inventory) {
							u->Error(ordertype +
									": Target cannot have more than " +
									ItemString(o->item,
										ItemDefs[o->item].max_inventory), o->quiet);
							o->type = NORDERS;
							continue;
						}
					}

					// Check if we have a trade hex
					if (!TradeCheck(r, u->faction)) {
						u->Error(ordertype + ": Faction cannot transport or "
								"distribute in that many hexes.", o->quiet);
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
										"on hand.", t->quiet);
							} else {
								amt = amt - t->except;
							}
						}
					} else if (amt > u->items.GetNum(t->item)) {
						u->Error(ordertype + ": Not enough.", t->quiet);
						amt = u->items.GetNum(t->item);
					}

					if (ItemDefs[t->item].max_inventory) {
						int cur = tar->unit->items.GetNum(t->item) + amt;
						if (cur > ItemDefs[t->item].max_inventory) {
							u->Error(ordertype +
									": Target cannot have more than " +
									ItemString(t->item,
										ItemDefs[t->item].max_inventory), t->quiet);
							continue;
						}
					}

					u->items.SetNum(t->item, u->items.GetNum(t->item) - amt);
					// now see if the unit can pay for shipping
					int dist = regions.GetDistance(r, tar->region);
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
					if (cost > u->items.GetNum(I_SILVER)) {
						u->Error(ordertype + ": Cannot afford shipping cost.", t->quiet);
						u->items.SetNum(t->item, u->items.GetNum(t->item)+amt);
						continue;
					}
					u->items.SetNum(I_SILVER, u->items.GetNum(I_SILVER) - cost);

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
					u->Experience(S_QUARTERMASTER,10);

				}
				u->transportorders.DeleteAll();
			}
		}
	}
}
// Shouldn't the locations be deleted?


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

using namespace std;

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
	RunStealthOrders();
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
	if (Globals->ALLOW_WITHDRAW) {
		Awrite("Running WITHDRAW Orders...");
		DoWithdrawOrders();
	}

	// Make sure we have a sacrifice enabled object before we run the orders
	for (auto ob = 0; ob < NOBJECTS; ob++) {
		ObjectType& obj = ObjectDefs[ob];
		if (obj.flags & ObjectType::DISABLED) continue;
		if (!(obj.flags & ObjectType::SACRIFICE)) continue;
		Awrite("Running SACRIFICE Orders...");
		RunSacrificeOrders();
		break;
	}

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

	if (!(SkillDefs[S_ANNIHILATION].flags & SkillType::DISABLED)) {
		Awrite("Running Annihilation Orders...");
		RunAnnihilateOrders();
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
	RemoveEmptyObjects();
}

void Game::ClearCastEffects()
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u : o->units) u->SetFlag(FLAG_INVIS, 0);
		}
	}
}

void Game::RunCastOrders()
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				if (u->castorders) {
					RunACastOrder(r, o, u);
					delete u->castorders;
					u->castorders = nullptr;
				}
			}
		}
	}
}

bool Game::ActivityCheck(ARegion *pReg, Faction *pFac, FactionActivity activity) {
	if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_FACTION_TYPES) {
		// No limit on any activity in this game.
		return true;
	}

	if (pFac->IsActivityRecorded(pReg, activity)) {
		// this activity is already performed in region
		return true;
	}

	int currentCost = pFac->GetActivityCost(activity);
	int maxAllowedCost = 0;

	if (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT) {
		if (activity == FactionActivity::TAX) {
			maxAllowedCost = AllowedTaxes(pFac);
		}

		if (activity == FactionActivity::TRADE) {
			maxAllowedCost = AllowedTrades(pFac);
		}
	}
	else {
		maxAllowedCost = AllowedMartial(pFac);
	}

	if (maxAllowedCost == -1) {
		return true;
	}

	if (currentCost >= maxAllowedCost) {
		return false;
	}

	pFac->RecordActivity(pReg, activity);

	return true;
}

void Game::RunStealthOrders()
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			// used to be a safe_list so a copy here.. consider doing that again, but we shouldn't need to.
			for(const auto u : o->units) {
				if (!u->stealthorders) continue;
				if (u->stealthorders->type == O_STEAL) {
					Do1Steal(r, o, u);
				}
				if (u->stealthorders->type == O_ASSASSINATE) {
					Do1Assassinate(r, o, u);
				}
				delete u->stealthorders;
				u->stealthorders = nullptr;
			}
		}
	}
}

std::list<Faction *> Game::CanSeeSteal(ARegion *r, Unit *u)
{
	std::list<Faction *> retval;
	for(const auto f :factions) {
		if (r->Present(f) && f->CanSee(r, u, Globals->SKILL_PRACTICE_AMOUNT > 0)) {
			retval.push_back(f);
		}
	}
	return retval;
}

void Game::Do1Assassinate(ARegion *r, Object *o, Unit *u)
{
	AssassinateOrder *so = dynamic_cast<AssassinateOrder *>(u->stealthorders);
	Unit *tar = r->GetUnitId(so->target, u->faction->num);

	if (!tar) {
		u->error("ASSASSINATE: Invalid unit given.");
		return;
	}
	if (!tar->IsAlive()) {
		u->error("ASSASSINATE: Invalid unit given.");
		return;
	}

	// New rule -- You can only assassinate someone you can see
	if (!u->CanSee(r, tar)) {
		u->error("ASSASSINATE: Invalid unit given.");
		return;
	}

	if (tar->type == U_GUARD || tar->type == U_WMON || tar->type == U_GUARDMAGE) {
		u->error("ASSASSINATE: Can only assassinate other player's units.");
		return;
	}

	if (u->GetMen() != 1) {
		u->error("ASSASSINATE: Must be executed by a 1-man unit.");
		return;
	}

	std::list<Faction *> seers(CanSeeSteal(r, u));
	int succ = 1;
	for(const auto f : seers) {
		if (f == tar->faction) {
			succ = 0;
			break;
		}
		if (f->get_attitude(tar->faction->num) == A_ALLY) {
			succ = 0;
			break;
		}
		if (f->num == guardfaction) {
			succ = 0;
			break;
		}
	}
	if (!succ) {
		string temp = u->name + " is caught attempting to assassinate " + tar->name + " in " +
			r->ShortPrint().const_str() + ".";
		for(const auto f : seers) {
			f->event(temp, "combat", r, u);
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
			tar->event("Assassination prevented by amulet of true seeing.", "combat");
			u->event("Attempts to assassinate " + tar->name + ", but is prevented by amulet of true seeing.", "combat");
			return;
		}
	}
	u->PracticeAttribute("stealth");
	RunBattle(r, u, tar, ass);
}

void Game::Do1Steal(ARegion *r, Object *o, Unit *u)
{
	StealOrder *so = dynamic_cast<StealOrder *>(u->stealthorders);
	Unit *tar = r->GetUnitId(so->target, u->faction->num);

	if (!tar) {
		u->error("STEAL: Invalid unit given.");
		return;
	}

	// New RULE!! You can only steal from someone you can see.
	if (!u->CanSee(r, tar)) {
		u->error("STEAL: Invalid unit given.");
		return;
	}

	if (tar->type == U_GUARD || tar->type == U_WMON || tar->type == U_GUARDMAGE) {
		u->error("STEAL: Can only steal from other player's units.");
		return;
	}

	if (u->GetMen() != 1) {
		u->error("STEAL: Must be executed by a 1-man unit.");
		return;
	}

	std::list<Faction *> seers(CanSeeSteal(r, u));
	int succ = 1;
	for(const auto f : seers) {
		if (f == tar->faction) {
			succ = 0;
			break;
		}
		if (f->get_attitude(tar->faction->num) == A_ALLY) {
			succ = 0;
			break;
		}
		if (f->num == guardfaction) {
			succ = 0;
			break;
		}
	}

	if (!succ) {
		string temp = u->name + " is caught attempting to steal from " + tar->name + " in " +
			r->ShortPrint().const_str() + ".";
		for(const auto f : seers) {
			f->event(temp, "theft", r, u);
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
		tar->event("Theft prevented by amulet of true seeing.", "theft");
		u->event("Attempts to steal from " + tar->name + ", but is prevented by amulet of true seeing.", "theft");
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

	string temp = u->name + " steals " + ItemString(so->item, amt) + " from " + tar->name + ".";
	for(const auto f : seers) {
		f->event(temp, "theft", r, u);
	}

	tar->event("Has " + ItemString(so->item, amt) + " stolen.", "theft");
	u->PracticeAttribute("stealth");
	return;
}

void Game::DrownUnits()
{
	for(const auto r : regions) {
		if (TerrainDefs[r->type].similar_type == R_OCEAN) {
			for(const auto o : r->objects) {
				if (o->type != O_DUMMY) continue;
				for(const auto u : o->units) {
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
						u->event("Drowns in the ocean.", "drown");
					}
				}
			}
		}
	}
}

void Game::SinkUncrewedFleets()
{
	for(const auto r : regions) {
		r->CheckFleets();
	}
}

void Game::RunForgetOrders()
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				for(const auto fo : u->forgetorders) {
					u->ForgetSkill(fo->skill);
					u->event("Forgets " + string(SkillStrs(fo->skill).const_str()) + ".", "forget");
				}
				std::for_each(u->forgetorders.begin(), u->forgetorders.end(), [](ForgetOrder *fo) { delete fo; });
				u->forgetorders.clear();
			}
		}
	}
}

void Game::RunQuitOrders()
{
	for(const auto f : factions) {
		if (f->quit) Do1Quit(f);
	}
}

void Game::Do1Quit(Faction *f)
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				if (u->faction == f) {
					r->Kill(u);
				}
			}
		}
	}
}

void Game::RunDestroyOrders()
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			Unit *u = o->GetOwner();
			if (u) {
				if (u->destroy) {
					Do1Destroy(r, o, u);
					continue;
				} else {
					for (const auto u2 : o->units) u2->destroy = 0;
				}
			}
		}
	}
}

void Game::Do1Destroy(ARegion *r, Object *o, Unit *u) {
	string quest_rewards;

	if (TerrainDefs[r->type].similar_type == R_OCEAN) {
		u->error("DESTROY: Can't destroy a ship while at sea.");
		for (const auto u2 : o->units) u2->destroy = 0;
		return;
	}

	if (!u->GetMen()) {
		u->error("DESTROY: Empty units cannot destroy structures.");
		for (const auto u2 : o->units) u2->destroy = 0;
		return;
	}

	if (o->CanModify()) {
		int maxStructurePoints;	// fully built structure points (equal to cost)
		int structurePoints;	// current structure points
		int destroyablePoints;	// how much points can be destroyed per turn for this structure type
		int destroyPower;		// how much unit can destroy during this turn
		int willDestroy;		// how muh points will be destroyed this turn

		maxStructurePoints = ObjectDefs[o->type].cost;
		structurePoints = maxStructurePoints - o->incomplete;

		if (Globals->DESTROY_BEHAVIOR == DestroyBehavior::INSTANT) {
			destroyablePoints = structurePoints;
			destroyPower = structurePoints;
		} else {
			destroyablePoints = std::max(
				Globals->MIN_DESTROY_POINTS, maxStructurePoints * Globals->MAX_DESTROY_PERCENT / 100
			);
			destroyablePoints = std::max(0, std::min(structurePoints, destroyablePoints) - o->destroyed);

			if (Globals->DESTROY_BEHAVIOR == DestroyBehavior::PER_SKILL) {
				destroyPower = std::max(1, u->GetSkill(S_BUILDING)) * u->GetMen();
			} else {
				destroyPower = u->GetMen();
			}
		}

		willDestroy = std::min(destroyablePoints, destroyPower);
		if (willDestroy == 0) {
			u->error(string("DESTROY: Can't destroy ") + o->name + " more.");
			for (const auto u2 : o->units) u2->destroy = 0;
			return;
		}

		int remainingSP = structurePoints - willDestroy;
		if (remainingSP > 0) {
			o->destroyed += willDestroy;
			o->incomplete += willDestroy;

			u->event("Destroys " + to_string(willDestroy) + " structure points from the " + o->name + ".", "destroy");
		} else {
			u->event("Destroys " + o->name + ".", "destroy");

			Object *dest = r->GetDummy();
			for(const auto u2 : o->units) {
				u2->destroy = 0;
				u2->MoveUnit(dest);
			}

			if (quests.check_demolish_target(r, o->num, u, &quest_rewards)) {
				u->event("You have completed a quest!" + quest_rewards, "quest");
			}
			r->objects.remove(o);
			delete o;
		}
	} else {
		if (o->type == O_DUMMY) {
			u->error("DESTROY: Not inside a structure.");
		} else {
			u->error(string("DESTROY: Can't destroy ") + o->name + ".");
		}
		for (const auto u2 : o->units) u2->destroy = 0;
		return;
	}
}

void Game::RunFindOrders()
{
	for(const auto r : regions) {
		for(auto o : r->objects) {
			for(const auto u : o->units) RunFindUnit(u);
		}
	}
}

void Game::RunFindUnit(Unit *u)
{
	int all = 0;
	Faction *fac;
	for(const auto f : u->findorders) {
		if (f->find == 0) all = 1;
		if (!all) {
			fac = GetFaction(factions, f->find);
			if (fac) {
				string temp = string("The address of ") + fac->name + " is " + fac->address->const_str() + ".";
				u->faction->event(temp, "find");
			} else {
				u->error(string("FIND: ") + to_string(f->find) + " is not a valid faction number.");
			}
		} else {
			for(const auto fac : factions) {
				string temp = string("The address of ") + fac->name + " is " + fac->address->const_str() + ".";
				u->faction->event(temp, "find");
			}
		}
	}
	std::for_each(u->findorders.begin(), u->findorders.end(), [](FindOrder *f) { delete f; });
	u->findorders.clear();
}

void Game::RunTaxOrders()
{
	for(const auto r : regions) RunTaxRegion(r);
}

int Game::FortTaxBonus(Object *o, Unit *u)
{
	int protect = ObjectDefs[o->type].protect;
	int fortbonus = 0;
	for(const auto unit : o->units) {
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
	for(const auto o : reg->objects) {
		int protect = ObjectDefs[o->type].protect;
		for(const auto u : o->units) {
			if (u->GetFlag(FLAG_AUTOTAX) && !Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_TAX;
			if (u->taxing == TAX_AUTO) u->taxing = TAX_TAX;

			if (u->taxing == TAX_TAX) {
				if (reg->Population() < 1) {
					u->error("TAX: No population to tax.");
					u->taxing = TAX_NONE;
				} else if (!reg->CanTax(u)) {
					u->error("TAX: A unit is on guard.");
					u->taxing = TAX_NONE;
				} else {
					int men = u->Taxers(0);
					int fortbonus = u->GetMen();
					if (fortbonus > protect) fortbonus = protect;
					protect -= u->GetMen();
					if (protect < 0) protect = 0;
					if (men) {
						if (!ActivityCheck(reg, u->faction, FactionActivity::TAX)) {
							u->error("TAX: Faction can't tax that many regions.");
							u->taxing = TAX_NONE;
						} else {
							t += men + fortbonus * Globals->TAX_BONUS_FORT;
						}
					} else {
						u->error("TAX: Unit cannot tax.");
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

	for(const auto o : reg->objects) {
		for(const auto u : o->units) {
			if (u->taxing == TAX_TAX) {
				int t = u->Taxers(0);
				t += FortTaxBonus(o, u);
				double fAmt = ((double) t) * ((double) reg->wealth) / ((double) desired);
				int amt = (int) fAmt;
				reg->wealth -= amt;
				desired -= t;
				u->SetMoney(u->GetMoney() + amt);
				u->event("Collects $" + to_string(amt) + " in taxes in " + reg->ShortPrint().const_str() + ".", "tax", reg);
				u->taxing = TAX_NONE;
			}
		}
	}
}

void Game::RunPillageOrders()
{
	for(const auto r : regions) RunPillageRegion(r);
}

int Game::CountPillagers(ARegion *reg)
{
	int p = 0;
	for(const auto o : reg->objects) {
		for(const auto u : o->units) {
			if (u->taxing == TAX_PILLAGE) {
				if (!reg->CanPillage(u)) {
					u->error("PILLAGE: A unit is on guard.");
					u->taxing = TAX_NONE;
				} else {
					int men = u->Taxers(1);
					if (men) {
						if (!ActivityCheck(reg, u->faction, FactionActivity::TAX)) {
							u->error("PILLAGE: Faction can't tax that many regions.");
							u->taxing = TAX_NONE;
						} else {
							p += men;
						}
					} else {
						u->error("PILLAGE: Not a combat ready unit.");
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
	for(const auto o : reg->objects) {
		for(const auto u : o->units) {
			if (u->taxing == TAX_PILLAGE) {
				u->error("PILLAGE: Not enough men to pillage.");
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

	std::set<Faction *> facs = reg->PresentFactions();
	int amt = reg->wealth * 2;
	for(const auto o : reg->objects) {
		for(const auto u : o->units) {
			if (u->taxing == TAX_PILLAGE) {
				u->taxing = TAX_NONE;
				int num = u->Taxers(1);
				int temp = (amt * num)/pillagers;
				amt -= temp;
				pillagers -= num;
				u->SetMoney(u->GetMoney() + temp);
				u->event("Pillages $" + to_string(temp) + " from " + reg->ShortPrint().const_str() + ".", "tax", reg);
				for(const auto f : facs) {
					if (f != u->faction) {
						string temp = u->name + " pillages " + reg->ShortPrint().const_str() + ".";
						f->event(temp, "tax", reg, u);
					}
				}
			}
		}
	}

	/* Destroy economy */
	reg->Pillage();
}

void Game::RunPromoteOrders()
{
	Unit *u;

	/* First, do any promote orders */
	for(const auto r : regions) {
		for(const auto o : r->objects) {
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
	for(const auto r : regions) {
		for(const auto o : r->objects) {
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

	/* Then, clear out other promote/evict orders */
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				if (u->promote) {
					if (o->type != O_DUMMY) {
						u->error("PROMOTE: Must be owner");
						delete u->promote;
						u->promote = 0;
					} else {
						u->error("PROMOTE: Can only promote inside structures.");
						delete u->promote;
						u->promote = 0;
					}
				}
				if (u->evictorders) {
					if (o->type != O_DUMMY) {
						u->error("EVICT: Must be owner");
						delete u->evictorders;
						u->evictorders = 0;
					} else {
						u->error("EVICT: Can only evict inside structures.");
						delete u->evictorders;
						u->evictorders = 0;
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
		u->error("PROMOTE: Can't find target.");
		return;
	}

	std::erase(obj->units, tar);
	obj->units.push_front(tar);
	ObjectType& ob = ObjectDefs[obj->type];
	if (ob.flags & ObjectType::GRANTSKILL) {
		if (tar->faction->skills.GetDays(ob.granted_skill) < ob.granted_level) {
			tar->faction->shows.push_back({ .skill = ob.granted_skill, .level = ob.granted_level });
			tar->faction->skills.SetDays(ob.granted_skill, ob.granted_level);
		}
	}
}

void Game::Do1EvictOrder(Object *obj, Unit *u)
{
	EvictOrder *ord = u->evictorders;

	if (obj->region->type == R_NEXUS) {
		u->error("Evict: Evict does not work in the Nexus.");
		return;
	}

	obj->region->deduplicate_unit_list(ord->targets, u->faction->num);
	while (ord && ord->targets.size()) {
		UnitId *id = ord->targets.front();
		std::erase(ord->targets, id);
		Unit *tar = obj->GetUnitId(id, u->faction->num);
		delete id;
		if (!tar) continue;
		if (obj->IsFleet() &&
			(TerrainDefs[obj->region->type].similar_type == R_OCEAN) &&
			(!tar->CanReallySwim() || tar->GetFlag(FLAG_NOCROSS_WATER))) {
			u->error("EVICT: Cannot forcibly evict units over ocean.");
			continue;
		}
		Object *to = obj->region->GetDummy();
		tar->MoveUnit(to);
		tar->event("Evicted from " + obj->name + " by " + u->name, "evict");
		u->event("Evicted " + tar->name + " from " + obj->name, "evict");
	}
}

/* RunEnterOrders is performed in TWO phases: one in the
 * instant orders phase for existing objects and one after
 * give orders for entering new objects (fleets).
 */
void Game::RunEnterOrders(int phase)
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u: o->units) {
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
			u->error("LEAVE: Can't leave a ship in the ocean.");
			return;
		}
	} else {
		int on = u->enter;
		to = r->GetObject(on);
		u->enter = 0;
		if (!to) {
			u->error("ENTER: Can't enter that.");
			return;
		}
		if (!to->CanEnter(r, u)) {
			u->error("ENTER: Can't enter that.");
			return;
		}
		if (to->ForbiddenBy(r, u)) {
			u->error("ENTER: Is refused entry.");
			return;
		}
	}
	u->MoveUnit(to);
}

void Game::Do1JoinOrder(ARegion *r, Object *in, Unit *u)
{
	JoinOrder *jo;
	Unit *tar;
	Object *to, *from;

	jo = (JoinOrder *) u->joinorders;
	tar = r->GetUnitId(jo->target, u->faction->num);

	if (!tar) {
		u->error("JOIN: No such unit.");
		return;
	}

	to = tar->object;
	if (!to) {
		u->error("JOIN: Can't enter that.");
		return;
	}

	if (u->object == to) {
		// We're already there!
		return;
	}

	if (jo->merge) {
		if (!u->object->IsFleet() || u->object->GetOwner()->num != u->num) {
			u->error("JOIN MERGE: Not fleet owner.");
			return;
		}
		if (!to->IsFleet()) {
			u->error("JOIN MERGE: Target unit is not in a fleet.");
			return;
		}
		for(const auto pass : u->object->units) {
			if (to->ForbiddenBy(r, pass)) {
				u->error("JOIN MERGE: A unit would be refused entry.");
				return;
			}
		}
		from = u->object;
		for(auto it = from->ships.begin(); it != from->ships.end();) {
			auto item = *it;
			// DoGiveOrder can modify the item list if the last of an item is given, so increment here.
			it++;
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
		for(const auto pass : u->object->units) {
			pass->MoveUnit(to);
		}

		return;
	}

	if (to == r->GetDummy()) {
		if ((TerrainDefs[r->type].similar_type == R_OCEAN) &&
				(!u->CanSwim() || u->GetFlag(FLAG_NOCROSS_WATER))) {
			u->error("JOIN: Can't leave a ship in the ocean.");
			return;
		}
	} else {
		if (!to->CanEnter(r, u)) {
			u->error("JOIN: Can't enter that.");
			return;
		}
		if (to->ForbiddenBy(r, u)) {
			u->error("JOIN: Is refused entry.");
			return;
		}
		if (to->IsFleet() &&
				!jo->overload &&
				to->FleetCapacity() < to->FleetLoad() + u->Weight()) {
			u->event("JOIN: Fleet would be overloaded.", "join");
			return;
		}
	}
	u->MoveUnit(to);
}

void Game::RemoveEmptyObjects()
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			if ((o->IsFleet()) && (TerrainDefs[r->type].similar_type != R_OCEAN)) continue;
			if (ObjectDefs[o->type].cost && o->incomplete >= ObjectDefs[o->type].cost) {
				for(const auto u : o->units) {
					u->MoveUnit(r->GetDummy());
				}
				std::erase(r->objects, o);
				delete o;
			}
		}
	}
}

void Game::EmptyHell()
{
	for(const auto r : regions) r->ClearHell();
}

void Game::MidProcessUnit(ARegion *r, Unit *u)
{
	MidProcessUnitExtra(r, u);
}

void Game::PostProcessUnit(ARegion *r, Unit *u)
{
	PostProcessUnitExtra(r, u);
}

void Game::EndGame(Faction *victor)
{
	for(const auto fac : factions) {
		fac->exists = false;
		if (fac == victor)
			fac->quit = QUIT_WON_GAME;
		else
			fac->quit = QUIT_GAME_OVER;

		if (victor) {
			string temp(victor->name);
			fac->event(temp + " has won the game!", "gameover");
		} else
			fac->event("The game has ended with no winner.", "gameover");
	}

	gameStatus = GAME_STATUS_FINISHED;
}

void Game::MidProcessTurn()
{
	for(const auto r : regions) {
		// r->MidTurn(); // Not yet implemented
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				MidProcessUnit(r, u);
			}
		}
	}
}

void Game::ProcessEconomics()
{
	if (!(Globals->DYNAMIC_POPULATION || Globals->REGIONS_ECONOMY)) return;

	for(const auto r : regions) {
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
		for(const auto r : regions) {
			r->FindMigrationDestination(phase);
		}
		for(const auto r : regions) {
			r->Migrate();
		}
	}
}

void Game::PostProcessTurn()
{
	//
	// Check if there are any factions left.
	//
	int livingFacs = 0;
	for(const auto fac : factions) {
		if (fac->exists) {
			livingFacs = 1;
			break;
		}
	}

	if (!livingFacs) EndGame(0);
	else if (!(Globals->OPEN_ENDED)) {
		Faction *victor = CheckVictory();
		if (victor) EndGame(victor);
	}

	for(const auto r : regions) {
		r->PostTurn();

		if (Globals->CITY_MONSTERS_EXIST && (r->town || r->type == R_NEXUS))
			AdjustCityMons(r);

		for(const auto o : r->objects) {
			for(const auto u : o->units) {
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
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				if (u->canattack && u->IsAlive()) {
					DoAutoAttack(r, u);
				}
			}
		}
	}
}

void Game::DoMovementAttacks(std::list<Location *>& locs)
{
	for(const auto l : locs) {
		if (l->obj) {
			for(const auto u : l->obj->units) {
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
	for(const auto o : r->objects) {
		for(const auto u : o->units) {
			if (u->guard != GUARD_AVOID && (u->GetAttitude(r, t) == A_HOSTILE) && u->IsAlive() && u->canattack) {
				AttemptAttack(r, u, t, 1);
			}
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
	if (u->guard == GUARD_AVOID) return;
	for(const auto o : r->objects) {
		for(const auto t : o->units) {
			if (u->GetAttitude(r, t) == A_HOSTILE) {
				AttemptAttack(r, u, t, 1);
			}
			if (u->canattack == 0 || u->IsAlive() == 0) return;
		}
	}
}

int Game::CountWMonTars(ARegion *r, Unit *mon) {
	int retval = 0;
	for(const auto o : r->objects) {
		for(const auto u : o->units) {
			if (u->type == U_NORMAL || u->type == U_MAGE || u->type == U_APPRENTICE) {
				if (mon->CanSee(r, u) && mon->CanCatch(r, u)) {
					retval += u->GetMen();
				}
			}
		}
	}
	return retval;
}

Unit *Game::GetWMonTar(ARegion *r, int tarnum, Unit *mon) {
	for(const auto o : r->objects) {
		for(const auto u : o->units) {
			if (u->type == U_NORMAL || u->type == U_MAGE || u->type == U_APPRENTICE) {
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
	if (rng::get_random(rand) >= u->Hostile()) return;

	Unit *t = GetWMonTar(r, rng::get_random(tars), u);
	if (t) AttemptAttack(r, u, t, 1);
}

void Game::DoAttackOrders()
{
	for(const auto r : regions) {
		for(const auto o : r->objects) {
			for(const auto u : o->units) {
				if (u->type == U_WMON) {
					if (u->canattack && u->IsAlive()) {
						CheckWMonAttack(r, u);
					}
				} else {
					if (u->attackorders && u->IsAlive()) {
						AttackOrder *ord = u->attackorders;
						r->deduplicate_unit_list(ord->targets, u->faction->num);
						while (ord->targets.size()) {
							UnitId *id = ord->targets.front();
							std::erase(ord->targets, id);
							Unit *t = r->GetUnitId(id, u->faction->num);
							delete id;
							if (u->canattack && u->IsAlive()) {
								if (t) {
									AttemptAttack(r, u, t, 0);
								} else {
									u->error("ATTACK: Non-existent unit.");
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
		if (!silent) u->error("ATTACK: Non-existent unit.");
		return;
	}

	if (!u->CanCatch(r, t)) {
		if (!silent) u->error("ATTACK: Can't catch that unit.");
		return;
	}

	if (t->routed && Globals->ONLY_ROUT_ONCE) {
		if (!silent) u->event("ATTACK: Target is already routed and scattered.", "combat");
		return;
	}

	RunBattle(r, u, t, 0, adv);
	return;
}

void Game::RunSellOrders()
{
	for(const auto r : regions) {
		for (const auto& m : r->markets) {
			if (m->type == Market::MarketType::M_SELL) DoSell(r, m);
		}
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				std::for_each(u->sellorders.begin(), u->sellorders.end(), [u](SellOrder* o) {
					u->error("SELL: Can't sell " + ItemString(o->item, o->num, FULLNUM) + ".");
					delete o;
				});
				u->sellorders.clear();
			}
		}
	}
}

int Game::GetSellAmount(ARegion *r, Market *m)
{
	int num = 0;
	for(const auto obj : r->objects) {
		for(const auto u : obj->units) {
			for(const auto o: u->sellorders) {
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
	for(const auto obj : r->objects) {
		for(const auto u : obj->units) {
			for(auto oit = u->sellorders.begin(); oit != u->sellorders.end();) {
				auto o = *oit;
				if (o->item == m->item) {
					int temp = 0;
					if (o->num > u->GetSharedNum(o->item)) {
						o->num = u->GetSharedNum(o->item);
						u->error("SELL: Unit attempted to sell more than it had.");
					}
					if (attempted) {
						temp = (m->amount *o->num + rng::get_random(attempted)) / attempted;
						if (temp<0) temp = 0;
					}
					attempted -= o->num;
					m->amount -= temp;
					m->activity += temp;
					u->ConsumeShared(o->item, temp);
					u->SetMoney(u->GetMoney() + temp * m->price);
					oit = u->sellorders.erase(oit);
					u->event("Sells " + ItemString(o->item, temp) + " at $" + to_string(m->price) + " each.", "sell");
					delete o;
					continue;
				}
				++oit;
			}
		}
	}
	m->amount = oldamount;
}

void Game::RunBuyOrders()
{
	for(const auto r : regions) {
		for (const auto& m : r->markets) {
			if (m->type == Market::MarketType::M_BUY) DoBuy(r, m);
		}
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				std::for_each(u->buyorders.begin(), u->buyorders.end(), [u](BuyOrder* o) {
					u->error("BUY: Can't buy " + ItemString(o->item, o->num, FULLNUM) + ".");
					delete o;
				});
				u->buyorders.clear();
			}
		}
	}
}

int Game::GetBuyAmount(ARegion *r, Market *m)
{
	int num = 0;
	for(const auto obj: r->objects) {
		for(const auto u : obj->units) {
			for(auto oit = u->buyorders.begin(); oit != u->buyorders.end();) {
				auto o = *oit;
				if (o->item == m->item) {
					if (ItemDefs[o->item].type & IT_MAN) {
						if (u->type == U_MAGE) {
							u->error("BUY: Mages can't recruit more men.");
							o->num = 0;
						}
						if (u->type == U_APPRENTICE) {
							string name = Globals->APPRENTICE_NAME;
							name[0] = toupper(name[0]);
							string temp = "BUY: " + name + "s can't recruit more men.";
							u->error(temp);
							o->num = 0;
						}
						// XXX: there has to be a better way
						if (u->GetSkill(S_QUARTERMASTER)) {
							u->error("BUY: Quartermasters can't recruit more men.");
							o->num = 0;
						}
						if (Globals->TACTICS_NEEDS_WAR && u->GetSkill(S_TACTICS) == 5) {
							u->error("BUY: Tacticians can't recruit more men.");
							o->num = 0;
						}
						if (((ItemDefs[o->item].type & IT_LEADER) && u->IsNormal()) ||
							(!(ItemDefs[o->item].type & IT_LEADER) && u->IsLeader())) {
							u->error("BUY: Can't mix leaders and normal men.");
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
						u->error("BUY: Unit attempted to buy more than were for sale.");
					}
					if (o->num * m->price > u->GetSharedMoney()) {
						o->num = u->GetSharedMoney() / m->price;
						u->error("BUY: Unit attempted to buy more than it could afford.");
					}
					num += o->num;
				}
				if (o->num < 1 && o->num != -1) {
					oit = u->buyorders.erase(oit);
					delete o;
					continue;
				}
				++oit;
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
	for(const auto obj : r->objects) {
		for(const auto u : obj->units) {
			for(auto oit = u->buyorders.begin(); oit != u->buyorders.end();) {
				auto o = *oit;
				if (o->item == m->item) {
					int temp = 0;
					if (m->amount == -1) {
						/* unlimited market */
						temp = o->num;
					} else {
						if (attempted) {
							temp = (m->amount * o->num +
									rng::get_random(attempted)) / attempted;
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
							auto mt = FindRace(ItemDefs[o->item].abr)->get();
							int exp = mt.speciallevel - mt.defaultlevel;
							if (exp > 0) {
								exp = exp * temp * GetDaysByLevel(1);
								for (auto ms : mt.skills) {
									int skill = lookup_skill(ms);
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
					oit = u->buyorders.erase(oit);
					u->event("Buys " + ItemString(o->item, temp) + " at $" + to_string(m->price) + " each.", "buy");
					delete o;
					continue;
				}
				++oit;
			}
		}
	}

	m->amount = oldamount;
}

void Game::CheckUnitMaintenanceItem(int item, int value, int consume)
{
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (u->needed > 0 &&
					((!consume) || (u->GetFlag(FLAG_CONSUMING_UNIT) || u->GetFlag(FLAG_CONSUMING_FACTION)))) {
					int amount = u->items.GetNum(item);
					if (amount) {
						int eat = (u->needed + value - 1) / value;
						if (eat > amount) eat = amount;
						if (ItemDefs[item].type & IT_FOOD) {
							if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && eat * value > u->stomach_space) {
								eat = (u->stomach_space + value - 1) / value;
								if (eat < 0) eat = 0;
							}
							u->hunger -= eat * value;
							u->stomach_space -= eat * value;
							if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && u->stomach_space < 0) {
								u->needed -= u->stomach_space;
								u->stomach_space = 0;
							}
						}
						if (eat) {
							u->needed -= eat * value;
							u->items.SetNum(item, amount - eat);

							u->event("Consumes " + ItemString(item, eat) + " for maintenance.", "maintenance");
						}
					}
				}
			}
		}
	}
}

void Game::CheckFactionMaintenanceItem(int item, int value, int consume)
{
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (u->needed > 0 && ((!consume) || u->GetFlag(FLAG_CONSUMING_FACTION))) {
					/* Go through all units again */
					for(const auto obj2 : r->objects) {
						for(const auto u2 : obj2->units) {
							if (u->faction == u2->faction && u != u2) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->needed + value - 1) / value;
									if (eat > amount) eat = amount;
									if (ItemDefs[item].type & IT_FOOD) {
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && eat * value > u->stomach_space) {
											eat = (u->stomach_space + value - 1) / value;
											if (eat < 0) eat = 0;
										}
										u->hunger -= eat * value;
										u->stomach_space -= eat * value;
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && u->stomach_space < 0) {
											u->needed -= u->stomach_space;
											u->stomach_space = 0;
										}
									}
									if (eat) {
										u->needed -= eat * value;
										u2->items.SetNum(item, amount - eat);

										u->event("Borrows " + ItemString(item, eat) + " from " + u2->name +
											" for maintenance.", "maintenance");
										u2->event(u->name + " borrows " + ItemString(item, eat) +
											" for maintenance.", "maintenance");
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

void Game::CheckAllyMaintenanceItem(int item, int value)
{
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (u->needed > 0) {
					/* Go through all units again */
					for(const auto obj2 : r->objects) {
						for(const auto u2 : obj2->units) {
							if (u->faction != u2->faction && u2->GetAttitude(r, u) == A_ALLY) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->needed + value - 1) / value;
									if (eat > amount) eat = amount;
									if (ItemDefs[item].type & IT_FOOD) {
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && eat * value > u->stomach_space) {
											eat = (u->stomach_space + value - 1) / value;
											if (eat < 0) eat = 0;
										}
										u->hunger -= eat * value;
										u->stomach_space -= eat * value;
										if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && u->stomach_space < 0) {
											u->needed -= u->stomach_space;
											u->stomach_space = 0;
										}
									}
									if (eat) {
										u->needed -= eat * value;
										u2->items.SetNum(item, amount - eat);
										u2->event(u->name + " borrows " + ItemString(item, eat) +
											" for maintenance.", "maintenance");
										u->event("Borrows " + ItemString(item, eat) + " from " +
											u2->name + " for maintenance.", "maintenance");
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
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (u->hunger > 0) {
					int amount = u->items.GetNum(item);
					if (amount) {
						int eat = (u->hunger + value - 1) / value;
						if (eat > amount) eat = amount;
						u->hunger -= eat * value;
						u->stomach_space -= eat * value;
						u->needed -= eat * value;
						if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && u->stomach_space < 0) {
							u->needed -= u->stomach_space;
							u->stomach_space = 0;
						}
						if(eat) {
							u->items.SetNum(item, amount - eat);
							u->event("Consumes " + ItemString(item, eat) + " to fend off starvation.", "maintenance");
						}
					}
				}
			}
		}
	}
}

void Game::CheckFactionHungerItem(int item, int value)
{
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (u->hunger > 0) {
					/* Go through all units again */
					for(const auto obj2 : r->objects) {
						for(const auto u2 : obj2->units) {
							if (u->faction == u2->faction && u != u2) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->hunger + value - 1) / value;
									if (eat > amount) eat = amount;
									u->hunger -= eat * value;
									u->stomach_space -= eat * value;
									u->needed -= eat * value;
									if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && u->stomach_space < 0) {
										u->needed -= u->stomach_space;
										u->stomach_space = 0;
									}
									if (eat) {
										u2->items.SetNum(item, amount - eat);
										u->event("Borrows " + ItemString(item, eat) + " from " + u2->name +
											" to fend off starvation.", "maintenance");
										u2->event(u->name + " borrows " + ItemString(item, eat) +
											" to fend off starvation.", "maintenance");
									}
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
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (u->hunger > 0) {
					/* Go through all units again */
					for(const auto obj2 : r->objects) {
						for(const auto u2 : obj2->units) {
							if (u->faction != u2->faction &&
								u2->GetAttitude(r, u) == A_ALLY) {
								int amount = u2->items.GetNum(item);
								if (amount) {
									int eat = (u->hunger + value - 1) / value;
									if (eat > amount) eat = amount;
									u->hunger -= eat * value;
									u->stomach_space -= eat * value;
									u->needed -= eat * value;
									if (Globals->UPKEEP_MAXIMUM_FOOD >= 0 && u->stomach_space < 0) {
										u->needed -= u->stomach_space;
										u->stomach_space = 0;
									}
									if (eat) {
										u2->items.SetNum(item, amount - eat);
										u2->event(u->name + " borrows " + ItemString(item, eat) +
											" to fend off starvation.", "maintenance");
										u->event("Borrows " + ItemString(item, eat) + " from " +
											u2->name + " to fend off starvation.", "maintenance");
									}
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
	string quest_rewards;

	/* First pass: set needed */
	for(const auto r : regions) {
		for(const auto obj: r->objects) {
			for(const auto u : obj->units) {
				if (!(u->faction->is_npc)) {
					r->visited = 1;
					if (quests.check_visit_target(r, u, &quest_rewards)) {
						u->event("You have completed a pilgrimage!" + quest_rewards, "quest");
					}
				}
				u->needed = u->MaintCost(regions, r);
				u->hunger = u->GetMen() * Globals->UPKEEP_MINIMUM_FOOD;
				if (Globals->UPKEEP_MAXIMUM_FOOD < 0)
					u->stomach_space = -1;
				else
					u->stomach_space = u->GetMen() * Globals->UPKEEP_MAXIMUM_FOOD;
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
					if (i == -1 || ItemDefs[i].baseprice > ItemDefs[j].baseprice) {
						i = j;
					}
				}
			}

			if (i > 0) {
				cost = ItemDefs[i].baseprice * 5 / 2;
				for(const auto r : regions) {
					for(const auto obj : r->objects) {
						for(const auto u : obj->units) {
							if (u->hunger > 0 && u->faction->unclaimed > cost) {
								int value = Globals->UPKEEP_FOOD_VALUE;
								int eat = (u->hunger + value - 1) / value;
								/* Now see if faction has money */
								if (u->faction->unclaimed >= eat * cost) {
									u->event("Withdraws " + ItemString(i, eat) + " for maintenance.", "maintenance");
									u->faction->unclaimed -= eat * cost;
									u->hunger -= eat * value;
									u->stomach_space -= eat * value;
									u->needed -= eat * value;
								} else {
									int amount = u->faction->unclaimed / cost;
									u->event("Withdraws " + ItemString(i, amount) + " for maintenance.", "maintenance");
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
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (u->needed > 0 && u->faction->unclaimed) {
					/* Now see if faction has money */
					if (u->faction->unclaimed >= u->needed) {
						u->event("Claims " + to_string(u->needed) + " silver for maintenance.", "maintenance");
						u->faction->unclaimed -= u->needed;
						u->needed = 0;
					} else {
						u->event("Claims " + to_string(u->faction->unclaimed) +	" silver for maintenance.",
							"maintenance");
						u->needed -= u->faction->unclaimed;
						u->faction->unclaimed = 0;
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
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (u->needed > 0 || u->hunger > 0)
					u->Short(u->needed, u->hunger);
			}
		}
	}
}

void Game::DoWithdrawOrders()
{
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				for(const auto o : u->withdraworders) {
					if (DoWithdrawOrder(r, u, o)) break;
				}
				std::for_each(u->withdraworders.begin(), u->withdraworders.end(), [](WithdrawOrder *o) { delete o; });
				u->withdraworders.clear();
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
		u->error("WITHDRAW: Withdraw does not work in the Nexus.");
		return 1;
	}

	if (cost > u->faction->unclaimed) {
		u->error(string("WITHDRAW: Too little unclaimed silver to withdraw ") +	ItemString(itm, amt) + ".");
		return 0;
	}

	if (ItemDefs[itm].max_inventory) {
		int cur = u->items.GetNum(itm) + amt;
		if (cur > ItemDefs[itm].max_inventory) {
			u->error(string("WITHDRAW: Unit cannot have more than ") + ItemString(itm, ItemDefs[itm].max_inventory));
			return 0;
		}
	}

	u->faction->unclaimed -= cost;
	u->event("Withdraws " + ItemString(o->item, amt) + ".", "withdraw");
	u->items.SetNum(itm, u->items.GetNum(itm) + amt);
	u->faction->DiscoverItem(itm, 0, 1);
	return 0;
}

void Game::DoGiveOrders()
{
	Item *item;
	Unit *s;
	Object *fleet;

	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				for(const auto o : u->giveorders) {
					if (o->item < 0) {
						if (o->amount == -1) {
							/* do 'give X unit' command */
							DoGiveOrder(r, u, o);
						} else if (o->amount == -2) {
							if (o->type == O_TAKE) {
								s = r->GetUnitId(o->target, u->faction->num);
								if (!s || !u->CanSee(r, s)) {
									u->error("TAKE: Nonexistant target (" +	o->target->Print() + ").");
									continue;
								} else if (u->faction != s->faction) {
									u->error("TAKE: " + o->target->Print() + " is not a member of your faction.");
									continue;
								}
								fleet = s->object;
							} else {
								s = u;
								fleet = obj;
							}
							/* do 'give all type' command */
							if (
								fleet->IsFleet() && s == fleet->GetOwner() && !o->unfinished &&
								(o->item == -NITEMS || o->item == -IT_SHIP)
							) {
								for(auto it = fleet->ships.begin(); it != fleet->ships.end();) {
									item = *it;
									// DoGiveOrder will delete the item
									it++;
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
							for(auto it = s->items.begin(); it != s->items.end();) {
								item = *it;
								it++;
								if ((o->item == -NITEMS) || (ItemDefs[item->type].type & (-o->item))) {
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
								u->error("TAKE: Invalid item.");
							else
								u->error("GIVE: Invalid item.");
						}
					} else if (DoGiveOrder(r, u, o)) {
						break;
					}
				}
				std::for_each(u->giveorders.begin(), u->giveorders.end(), [](GiveOrder *o) { delete o; });
				u->giveorders.clear();
			}
		}
	}
}

void Game::DoExchangeOrders()
{
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				for(const auto o : u->exchangeorders) {
					DoExchangeOrder(r, u, (ExchangeOrder *) o);
				}
				std::for_each(u->exchangeorders.begin(), u->exchangeorders.end(), [](ExchangeOrder *o) { delete o; });
				u->exchangeorders.clear();
			}
		}
	}
}

void Game::DoExchangeOrder(ARegion *r, Unit *u, ExchangeOrder *o)
{
	// Check if the destination unit exists
	Unit *t = r->GetUnitId(o->target, u->faction->num);
	if (!t) {
		u->error("EXCHANGE: Nonexistant target (" + o->target->Print() + ").");
		return;
	}

	// Check each Item can be given
	if (ItemDefs[o->giveItem].flags & ItemType::CANTGIVE) {
		u->error(string("EXCHANGE: Can't trade ") + ItemDefs[o->giveItem].names + ".");
		return;
	}

	if (ItemDefs[o->expectItem].flags & ItemType::CANTGIVE) {
		u->error(string("EXCHANGE: Can't trade ") +	ItemDefs[o->expectItem].names + ".");
		return;
	}

	if (ItemDefs[o->giveItem].type & IT_MAN) {
		u->error("EXCHANGE: Exchange aborted.  Men may not be traded.");
		return;
	}

	if (ItemDefs[o->expectItem].type & IT_MAN) {
		u->error("EXCHANGE: Exchange aborted. Men may not be traded.");
		return;
	}

	// New RULE -- Must be able to see unit to give something to them!
	if (!u->CanSee(r, t)) {
		u->error("EXCHANGE: Nonexistant target (" + o->target->Print() + ").");
		return;
	}
	// Check other unit has enough to give
	int amtRecieve = o->expectAmount;
	if (amtRecieve > t->GetSharedNum(o->expectItem)) {
		t->error(string("EXCHANGE: Not giving enough. Expecting ") + ItemString(o->expectItem, o->expectAmount) + ".");
		u->error(string("EXCHANGE: Exchange aborted.  Not enough recieved. Expecting ") +
			ItemString(o->expectItem, o->expectAmount) + ".");
		return;
	}

	bool exchangeOrderFound = false;
	// Check if other unit has a reciprocal exchange order
	for (auto tOrder: t->exchangeorders) {
		Unit *ptrUnitTemp = r->GetUnitId(tOrder->target, t->faction->num);
		if (ptrUnitTemp == u && tOrder->expectItem == o->giveItem && tOrder->giveItem == o->expectItem) {
			exchangeOrderFound = true;
			if (tOrder->giveAmount < o->expectAmount) {
				t->error(string("EXCHANGE: Not giving enough. Expecting ") +
					ItemString(o->expectItem, o->expectAmount) + ".");
				u->error(string("EXCHANGE: Exchange aborted. Not enough recieved. Expecting ") +
					ItemString(o->expectItem, o->expectAmount) + ".");
				tOrder->exchangeStatus = 0;
				o->exchangeStatus = 0;
				return;
			}
			if (tOrder->giveAmount > o->expectAmount) {
				t->error(string("EXCHANGE: Exchange aborted. Too much given. Expecting ") +
					ItemString(o->expectItem, o->expectAmount) + ".");
				u->error(string("EXCHANGE: Exchange aborted. Too much offered. Expecting ") +
					ItemString(o->expectItem, o->expectAmount) + ".");
				tOrder->exchangeStatus = 0;
				o->exchangeStatus = 0;
			}
			if (tOrder->giveAmount == o->expectAmount) o->exchangeStatus = 1;
			if (o->exchangeStatus == 1 && tOrder->exchangeStatus == 1) {
				u->event("Exchanges " + ItemString(o->giveItem, o->giveAmount) + " with " +	t->name +
					" for " + ItemString(tOrder->giveItem, tOrder->giveAmount) +	".", "exchange");
				t->event("Exchanges " + ItemString(tOrder->giveItem, tOrder->giveAmount) + " with " +
					u->name + " for " + ItemString(o->giveItem, o->giveAmount) + ".", "exchange");
				u->ConsumeShared(o->giveItem, o->giveAmount);
				t->items.SetNum(o->giveItem, t->items.GetNum(o->giveItem) + o->giveAmount);
				t->ConsumeShared(tOrder->giveItem, tOrder->giveAmount);
				u->items.SetNum(tOrder->giveItem, u->items.GetNum(tOrder->giveItem) + tOrder->giveAmount);
				u->faction->DiscoverItem(tOrder->giveItem, 0, 1);
				t->faction->DiscoverItem(o->giveItem, 0, 1);
				std::erase(t->exchangeorders, tOrder);
				return;
			}
			if (o->exchangeStatus >= 0 && tOrder->exchangeStatus >= 0) {
				std::erase(t->exchangeorders, tOrder);
				return;
			}
		}
	}

	if (!exchangeOrderFound) {
		u->error("EXCHANGE: target unit did not issue a matching exchange order.");
		return;
	}
}

int Game::DoGiveOrder(ARegion *r, Unit *u, GiveOrder *o)
{
	int hasitem, ship, num, shipcount, amt, newfleet, cur;
	int notallied, newlvl, oldlvl;
	Item *it;
	Unit *t, *s;
	Object *fleet;
	SkillList *skills;

	string ord = (o->type == O_TAKE) ? "TAKE" : "GIVE";
	string event_type = (o->type == O_TAKE) ? "take" : "give";

	/* Transfer/GIVE ship items: */
	if ((o->item >= 0) && (ItemDefs[o->item].type & IT_SHIP)) {
		// GIVE 0
		if (o->target->unitnum == -1) {
			hasitem = u->items.GetNum(o->item);
			// discard unfinished ships from inventory
			if (o->unfinished) {
				if (!hasitem || o->amount > 1) {
					u->error(ord + ": not enough.");
					return 0;
				}
				ship = -1;
				for(auto it : u->items) {
					if (it->type == o->item) {
						u->event("Abandons " + string(it->Report(1).const_str()) + ".", event_type);
						ship = it->type;
					}
				}
				if (ship > 0) u->items.SetNum(ship, 0);
				return 0;
			// abandon fleet ships
			} else if (!(u->object->IsFleet()) || (u->num != u->object->GetOwner()->num)) {
				u->error(ord + ": only fleet owner can give ships.");
				return 0;
			}

			// Check amount
			num = u->object->GetNumShips(o->item);
			if (num < 1) {
				u->error(ord + ": no such ship in fleet.");
				return 0;
			}
			if ((num < o->amount) && (o->amount != -2)) {
				u->error(ord + ": not enough ships.");
				o->amount = num;
			}

			// Check we're not dumping passengers in the ocean
			if (TerrainDefs[r->type].similar_type == R_OCEAN) {
				shipcount = 0;
				for(auto sh : u->object->ships) {
					shipcount += sh->num;
				}
				if (shipcount <= o->amount) {
					for(const auto p : u->object->units) {
						if ((!p->CanSwim() || p->GetFlag(FLAG_NOCROSS_WATER))) {
							u->error(ord + ": Can't abandon our last ship in the ocean.");
							return 0;
						}
					}
				}
			}

			u->object->SetNumShips(o->item, num - o->amount);
			u->event("Abandons " + ItemString(o->item, num - o->amount) + ".", event_type);
			return 0;
		}
		// GIVE to unit:
		t = r->GetUnitId(o->target, u->faction->num);
		if (!t) {
			u->error(ord + ": Nonexistant target (" + o->target->Print() + ").");
			return 0;
		} else if (o->type == O_TAKE && u->faction != t->faction) {
			u->error(ord + ": " + o->target->Print() + " is not a member of your faction.");
			return 0;
		} else if (u->faction != t->faction && t->faction->is_npc) {
			u->error(ord + ": Can't give to non-player unit (" + o->target->Print() + ").");
			return 0;
		}
		if (u == t) {
			if (o->type == O_TAKE)
				u->error(ord + ": Attempt to take " + ItemDefs[o->item].names + " from self.");
			else
				u->error(ord + ": Attempt to give " + ItemDefs[o->item].names + " to self.");
			return 0;
		}
		if (!u->CanSee(r, t) &&
			(t->faction->get_attitude(u->faction->num) < A_FRIENDLY)) {
				u->error(ord + ": Nonexistant target (" + o->target->Print() + ").");
				return 0;
		}
		if (t->faction->get_attitude(u->faction->num) < A_FRIENDLY) {
				u->error(ord + ": Target is not a member of a friendly faction.");
				return 0;
		}
		if (ItemDefs[o->item].flags & ItemType::CANTGIVE) {
			if (o->type == O_TAKE)
				u->error(ord + ": Can't take " + ItemDefs[o->item].names + ".");
			else
				u->error(ord + ": Can't give " + ItemDefs[o->item].names + ".");
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
		} else if (!(s->object->IsFleet()) || (s->num != s->object->GetOwner()->num)) {
			u->error(ord + ": only fleet owner can transfer ships.");
			return 0;
		} else {
			num = s->object->GetNumShips(o->item);
			if (num < 1) {
				u->error(ord + ": no such ship in fleet.");
				return 0;
			}
		}
		amt = o->amount;
		if (amt != -2 && amt > num) {
			u->error(ord + ": Not enough.");
			amt = num;
		} else if (amt == -2) {
			amt = num;
			if (o->except) {
				if (o->except > amt) {
					amt = 0;
					u->error(ord + ": EXCEPT value greater than amount on hand.");
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
					u->error(ord + ": Already have an unfinished ship of that type.");
				else
					u->error(ord + ": Target already has an unfinished ship of that type.");
				return 0;
			}
			it = new Item();
			it->type = o->item;
			it->num = s->items.GetNum(o->item);
			if (o->type == O_TAKE) {
				u->event("Takes " + string(it->Report(1).const_str()) + " from " + s->name + ".", event_type);
			} else {
				u->event("Gives " + string(it->Report(1).const_str()) + " to " + t->name + ".", event_type);
				if (s->faction != t->faction) {
					t->event("Receives " + string(it->Report(1).const_str()) + " from " + s->name + ".", event_type);
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
				for(auto sh : s->object->ships) {
					shipcount += sh->num;
				}
				if (shipcount <= amt) {
					for(const auto p : s->object->units) {
						if ((!p->CanSwim() || p->GetFlag(FLAG_NOCROSS_WATER))) {
							u->error(ord + ": Can't give away our last ship in the ocean.");
							return 0;
						}
					}
				}
			}

			// give into existing fleet or form new fleet?
			newfleet = 0;

			// target is not in fleet or not fleet owner
			if (!(t->object->IsFleet()) || (t->num != t->object->GetOwner()->num)) newfleet = 1;

			// Set fleet variable to target fleet
			if (newfleet == 1) {
				// create a new fleet
				fleet = new Object(r);
				fleet->type = O_FLEET;
				fleet->num = shipseq++;
				fleet->set_name("Fleet");
				t->object->region->AddFleet(fleet);
				t->MoveUnit(fleet);
			}
			else {
				fleet = t->object;
			}

			if (ItemDefs[o->item].max_inventory) {
				cur = t->object->GetNumShips(o->item) + amt;
				if (cur > ItemDefs[o->item].max_inventory) {
					u->error(ord + ": Fleets cannot have more than " +
						ItemString(o->item, ItemDefs[o->item].max_inventory) + ".");
					return 0;
				}
			}

			// Check if fleets are compatible; flying ships are always transferable.
			if ((Globals->PREVENT_SAIL_THROUGH) && (!Globals->ALLOW_TRIVIAL_PORTAGE) && (ItemDefs[o->item].fly == 0)) {
				// if target fleet had not sailed, just copy shore from source
				if (fleet->prevdir == -1) {
					fleet->prevdir = s->object->prevdir;
				} else {
					// check that source ship is compatible with its new fleet
					if (s->object->SailThroughCheck(fleet->prevdir) == 0) {
						u->error(ord + ": Ships cannot be transferred through land.");
						return 0;
					}
				}
			}

			s->event("Transfers " + ItemString(o->item, amt) + " to " + t->object->name + ".", ord);
			if (s->faction != t->faction) {
				t->event("Receives " + ItemString(o->item, amt) + " from " + s->object->name + ".", ord);
			}
			s->object->SetNumShips(o->item, s->object->GetNumShips(o->item) - amt);
			t->object->SetNumShips(o->item, t->object->GetNumShips(o->item) + amt);
			t->faction->DiscoverItem(o->item, 0, 1);
		}
		return 0;
	}

	if (o->target->unitnum == -1) {
		/* Give 0 */
		// Check there is enough to give
		amt = o->amount;
		if (amt != -2 && amt > u->GetSharedNum(o->item)) {
			u->error(ord + ": Not enough.");
			amt = u->GetSharedNum(o->item);
		} else if (amt == -2) {
			amt = u->items.GetNum(o->item);
			if (o->except) {
				if (o->except > amt) {
					amt = 0;
					u->error(ord + ": EXCEPT value greater than amount on hand.");
				} else {
					amt = amt - o->except;
				}
			}
		}
		if (amt == -1) {
			u->error("Can't discard a whole unit.");
			return 0;
		}

		if (amt < 0) {
			u->error("Cannot give a negative number of items.");
			return 0;
		}

		string temp = "Discards ";
		if (ItemDefs[o->item].type & IT_MAN) {
			u->SetMen(o->item, u->GetMen(o->item) - amt);
			r->DisbandInRegion(o->item, amt);
			temp = "Disbands ";
		} else if (Globals->RELEASE_MONSTERS &&
				(ItemDefs[o->item].type & IT_MONSTER) &&
					!(ItemDefs[o->item].flags & ItemType::MANPRODUCE)) {
			temp = "Releases ";
			u->items.SetNum(o->item, u->items.GetNum(o->item) - amt);
			if (Globals->WANDERING_MONSTERS_EXIST) {
				Faction *mfac = GetFaction(factions, monfaction);
				Unit *mon = GetNewUnit(mfac, 0);
				auto mp = FindMonster(ItemDefs[o->item].abr, (ItemDefs[o->item].type & IT_ILLUSION))->get();
				mon->MakeWMon(mp.name, o->item, amt);
				mon->MoveUnit(r->GetDummy());
				// This will result in 0 unless MONSTER_NO_SPOILS or
				// MONSTER_SPOILS_RECOVERY are set.
				mon->free = Globals->MONSTER_NO_SPOILS + Globals->MONSTER_SPOILS_RECOVERY;
			}
		} else {
			u->ConsumeShared(o->item, amt);
		}
		u->event(temp + ItemString(o->item, amt) + ".", event_type);
		return 0;
	}

	amt = o->amount;
	t = r->GetUnitId(o->target, u->faction->num);
	if (!t) {
		u->error(ord + ": Nonexistant target (" + o->target->Print() + ").");
		return 0;
	} else if (o->type == O_TAKE && u->faction != t->faction) {
		u->error(ord + ": " + o->target->Print() + " is not a member of your faction.");
		return 0;
	} else if (u->faction != t->faction && t->faction->is_npc) {
		u->error(ord + ": Can't give to non-player unit (" + o->target->Print() + ").");
		return 0;
	}
	if (amt == -1 && u->faction == t->faction) {
		u->error(ord + ": Unit already belongs to our faction!");
		return 0;
	}
	if (u == t) {
		if (o->type == O_TAKE)
			u->error(ord + ": Attempt to take " + ItemString(o->item, amt) + " from self.");
		else
			u->error(ord + ": Attempt to give " + ItemString(o->item, amt) + " to self.");
		return 0;
	}
	// New RULE -- Must be able to see unit to give something to them!
	if (!u->CanSee(r, t) && (t->faction->get_attitude(u->faction->num) < A_FRIENDLY)) {
		u->error(ord + ": Nonexistant target (" + o->target->Print() + ").");
		return 0;
	}

	s = u;
	if (o->type == O_TAKE) {
		s = t;
		t = u;
	}

	// Check there is enough to give
	if (amt != -2 && amt > s->GetSharedNum(o->item)) {
		u->error(ord + ": Not enough.");
		amt = s->GetSharedNum(o->item);
	} else if (amt == -2) {
		amt = s->items.GetNum(o->item);
		if (o->except) {
			if (o->except > amt) {
				amt = 0;
				u->error(ord + ": EXCEPT value greater than amount on hand.");
			} else {
				amt = amt - o->except;
			}
		}
	}

	if (o->item != I_SILVER && t->faction->get_attitude(s->faction->num) < A_FRIENDLY) {
		u->error(ord + ": Target is not a member of a friendly faction.");
		return 0;
	}

	if (amt == -1) {
		/* Give unit */
		if (u->type == U_MAGE) {
			if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
				if (CountMages(t->faction) >= AllowedMages(t->faction)) {
					u->error(ord + ": Faction has too many mages.");
					return 0;
				}
			}
		}
		if (u->type == U_APPRENTICE) {
			if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_UNLIMITED) {
				if (CountApprentices(t->faction) >= AllowedApprentices(t->faction)) {
					u->error(ord + ": Faction has too many " + Globals->APPRENTICE_NAME + "s.");
					return 0;
				}
			}
		}

		if (u->GetSkill(S_QUARTERMASTER)) {
			if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
				if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
					if (CountQuarterMasters(t->faction) >= AllowedQuarterMasters(t->faction)) {
						u->error(ord + ": Faction has too many quartermasters.");
						return 0;
					}
				}
			}
		}

		if (u->GetSkill(S_TACTICS) == 5) {
			if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
				if (Globals->TACTICS_NEEDS_WAR && (CountTacticians(t->faction) >= AllowedTacticians(t->faction))) {
					u->error(ord + ": Faction has too many tacticians.");
					return 0;
				}
			}
		}

		// Throw away any relics when the unit is given away
		u->items.SetNum(I_RELICOFGRACE, 0);

		notallied = 1;
		if (t->faction->get_attitude(u->faction->num) == A_ALLY) {
			notallied = 0;
		}

		u->event("Gives unit to " + t->faction->name + ".", event_type);
		u->faction = t->faction;
		u->event("Is given to your faction.", event_type);

		if (notallied && u->monthorders && u->monthorders->type == O_MOVE) {
			MoveOrder *mo = dynamic_cast<MoveOrder *>(u->monthorders);
			if (mo->advancing) {
				u->error("Unit cannot advance after being given.");
				delete u->monthorders;
				u->monthorders = nullptr;
			}
		}

		/* Check if any new skill reports have to be shown */
		for(const auto skill: u->skills) {
			newlvl = u->GetRealSkill(skill->type);
			oldlvl = u->faction->skills.GetDays(skill->type);
			if (newlvl > oldlvl) {
				for (int i=oldlvl+1; i<=newlvl; i++) {
					u->faction->shows.push_back({ .skill = skill->type, .level = i });
				}
				u->faction->skills.SetDays(skill->type, newlvl);
			}
		}

		// Okay, now for each item that the unit has, tell the new faction
		// about it in case they don't know about it yet.
		for(auto it : u->items) {
			u->faction->DiscoverItem(it->type, 0, 1);
		}

		return notallied;
	}

	/* If the item to be given is a man, combine skills */
	if (ItemDefs[o->item].type & IT_MAN) {
		if (s->type == U_MAGE || s->type == U_APPRENTICE || t->type == U_MAGE || t->type == U_APPRENTICE) {
			u->error(ord + ": Magicians can't transfer men.");
			return 0;
		}
		if (Globals->TACTICS_NEEDS_WAR) {
			if (s->GetSkill(S_TACTICS) == 5 || t->GetSkill(S_TACTICS) == 5) {
				u->error(ord + ": Tacticians can't transfer men.");
				return 0;
			}
		}
		if (s->GetSkill(S_QUARTERMASTER) > 0 || t->GetSkill(S_QUARTERMASTER) > 0) {
			u->error(ord + ": Quartermasters can't transfer men.");
			return 0;
		}

		if ((ItemDefs[o->item].type & IT_LEADER) && t->IsNormal()) {
			u->error(ord + ": Can't mix leaders and normal men.");
			return 0;
		} else {
			if (!(ItemDefs[o->item].type & IT_LEADER) && t->IsLeader()) {
				u->error(ord + ": Can't mix leaders and normal men.");
				return 0;
			}
		}
		// Small hack for Ceran
		if (o->item == I_MERC && t->GetMen()) {
			u->error(ord + ": Can't mix mercenaries with other men.");
			return 0;
		}

		if (u->faction != t->faction) {
			u->error(ord + ": Can't give men to another faction.");
			return 0;
		}

		if (s->nomove) t->nomove = 1;

		skills = s->skills.Split(s->GetMen(), amt);
		t->skills.Combine(skills);
		delete skills;
	}

	if (ItemDefs[o->item].flags & ItemType::CANTGIVE) {
		u->error(ord + ": Can't give " + ItemDefs[o->item].names + ".");
		return 0;
	}

	if (ItemDefs[o->item].max_inventory) {
		cur = t->items.GetNum(o->item) + amt;
		if (cur > ItemDefs[o->item].max_inventory) {
			u->error(ord + ": Unit cannot have more than " + ItemString(o->item, ItemDefs[o->item].max_inventory));
			return 0;
		}
	}

	if (o->type == O_TAKE) {
		u->event("Takes " + ItemString(o->item, amt) + " from " + s->name + ".", event_type);
	} else {
		u->event("Gives " + ItemString(o->item, amt) + " to " + t->name + ".", event_type);
		if (s->faction != t->faction)
			t->event("Receives " + ItemString(o->item, amt) + " from " + s->name + ".", event_type);
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
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				if (!Globals->OCEAN_GUARD &&
					(u->guard == GUARD_SET || u->guard == GUARD_GUARD) &&
					TerrainDefs[r->type].similar_type == R_OCEAN) {
					u->guard = GUARD_NONE;
					u->error("Can not guard in oceans.");
					continue;
				}

				// Only one faction and it's allies can be on guard at the same time
				if (Globals->STRICT_GUARD && u->guard == GUARD_SET && !r->CanGuard(u)) {
					u->guard = GUARD_NONE;
					u->error("Is prevented from guarding by another unit.");
					continue;
				}

				if (u->guard == GUARD_SET || u->guard == GUARD_GUARD) {
					if (!u->Taxers(1)) {
						u->guard = GUARD_NONE;
						u->error("Must be combat ready to be on guard.");
						continue;
					}
					if (u->type != U_GUARD && r->HasCityGuard()) {
						u->guard = GUARD_NONE;
						u->error("Is prevented from guarding by the "
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
	for(const auto f : factions) {
		f->CheckExist(regions);
	}
}

void Game::DeleteEmptyUnits()
{
	for(const auto r : regions) DeleteEmptyInRegion(r);
}

void Game::DeleteEmptyInRegion(ARegion *region)
{
	for(const auto obj : region->objects) {
		for(const auto unit : obj->units) {
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

	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				for(const auto o : u->transportorders) {
					if (!o->target || o->target->unitnum == -1) {
						u->error("TRANSPORT: Target does not exist.");
						o->type = NORDERS;
						continue;
					}

					Location *tar = regions.GetUnitId(o->target, u->faction->num, r);
					if (!tar) {
						u->error("TRANSPORT: Target does not exist.");
						o->type = NORDERS;
						continue;
					}

					// Make sure target isn't self
					if (tar->unit == u) {
						u->error("TRANSPORT: Target is self.");
						o->type = NORDERS;
						continue;
					}

					// Make sure the target and unit are at least friendly
					if (tar->unit->faction->get_attitude(u->faction->num) <	A_FRIENDLY) {
						u->error("TRANSPORT: Target " + tar->unit->name + " is not a member of a friendly faction.");
						o->type = NORDERS;
						continue;
					}

					// Determine the phase of the order
					bool sender_has_qm_skill = u->GetSkill(S_QUARTERMASTER) > 0;
					bool target_has_qm_skill = tar->unit->GetSkill(S_QUARTERMASTER) > 0;
					bool sender_owns_qm_building = (
						u == obj->GetOwner() &&
						!(obj->incomplete > 0) &&
						(ObjectDefs[obj->type].flags & ObjectType::TRANSPORT)
					);
					bool target_owns_qm_building = (
						tar->unit == tar->obj->GetOwner() &&
						!(tar->obj->incomplete > 0) &&
						(ObjectDefs[tar->obj->type].flags & ObjectType::TRANSPORT)
					);

					bool sender_is_valid_qm = sender_has_qm_skill && sender_owns_qm_building;
					bool target_is_valid_qm = target_has_qm_skill && target_owns_qm_building;
					if (sender_is_valid_qm) {
						o->phase = target_is_valid_qm ?
							TransportOrder::TransportPhase::INTER_QM_TRANSPORT :  // both sender and target are valid QM
							TransportOrder::TransportPhase::DISTRIBUTE_FROM_QM; // sender is a valid QM, target isn't
					} else { // sender isn't a valid QM
						if (!target_is_valid_qm) { // Non-qms or invalid qms can only send to valid QMs
							// Give a specific error message depending on why they aren't considered a quartermaster
							string temp = "TRANSPORT: Target " + tar->unit->name;
							temp += (
								target_owns_qm_building ?
								" does not own a transport structure." :
								" is not a quartermaster."
							);
							u->error(temp);
							o->type = NORDERS;
							continue;
						}
						// the target is a valid QM so the send is legal
						o->phase = TransportOrder::TransportPhase::SHIP_TO_QM;
					}

					int maxdist = Globals->LOCAL_TRANSPORT;
					if (o->phase == TransportOrder::TransportPhase::INTER_QM_TRANSPORT) {
						maxdist = Globals->NONLOCAL_TRANSPORT;
						// 0 max distance represents unlimited range
						if (maxdist > 0 && Globals->TRANSPORT & GameDefs::QM_AFFECT_DIST) {
							int level = u->GetSkill(S_QUARTERMASTER);
							maxdist += ((level + 1)/3);
						}
					}

					int dist;
					int penalty = 10000000;
					RangeType *rt = FindRange("rng_transport");
					if (rt) penalty = rt->crossLevelPenalty;
					o->distance = Globals->LOCAL_TRANSPORT;  // default to local max distance
					if (maxdist > 0) {
						// 0 maxdist represents unlimited range for QM->QM transport
						if (Globals->TRANSPORT & GameDefs::USE_CONNECTED_DISTANCES) {
							dist = regions.get_connected_distance(r, tar->region, penalty, maxdist);
						} else {
							dist = regions.GetPlanarDistance(r, tar->region, penalty, maxdist);
						}
						if (dist > maxdist) {
							u->error("TRANSPORT: Recipient " + tar->unit->name + " is too far away.");
							o->type = NORDERS;
							continue;
						}
						// Store off the distance for later use so we don't need to recompute it.
						o->distance = dist;
					}

					// We will check the amount at transport time so that if you receive items in you can tranport them
					// out without using 'all/except'
					// We will also check max inventory at the actual time of transport since things might move

					// Check if we have a trade hex available
					if (!Globals->TRANSPORT_NO_TRADE && !ActivityCheck(r, u->faction, FactionActivity::TRADE)) {
						u->error("TRANSPORT: Faction cannot transport or distribute in that many hexes.");
						o->type = NORDERS;
						continue;
					}
				}
			}
		}
	}
}

void Game::CollectInterQMTransportItems() {
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				// Move the items from the transport_items list to the unit's items list
				for(auto it : u->transport_items) {
					u->items.SetNum(it->type, u->items.GetNum(it->type) + it->num);
				}
				std::for_each(u->transport_items.begin(), u->transport_items.end(), [](Item *it) { delete it; });
				u->transport_items.clear();
			}
		}
	}
}

void Game::RunTransportOrders() {
	if (!(Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT))
		return;

	// Make sure there are no transport items on the units (shouldn't be, but just in case)
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				u->transport_items.clear();
			}
		}
	}

	// Send all items in to QMs from non-QMs
	RunTransportPhase(TransportOrder::TransportPhase::SHIP_TO_QM);
	// Ship items between QMs
	RunTransportPhase(TransportOrder::TransportPhase::INTER_QM_TRANSPORT);
	// Move items from tempororary transport storage to the QM for distribution
	CollectInterQMTransportItems();
	// Send all items out from QMs to non-QMs
	RunTransportPhase(TransportOrder::TransportPhase::DISTRIBUTE_FROM_QM);

	// erase all transport orders
	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				std::for_each(u->transportorders.begin(), u->transportorders.end(), [](TransportOrder *o) {
					delete o;
				});
				u->transportorders.clear();
			}
		}
	}

}

void Game::RunTransportPhase(TransportOrder::TransportPhase phase) {

	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				for(const auto t : u->transportorders) {
					if (t->type != O_TRANSPORT) continue;
					if (t->phase != phase) continue;

					Location *tar = regions.GetUnitId(t->target, u->faction->num, r);
					if (!tar) continue;

					int amt = t->amount;
					if (amt < 0) {
						amt = u->items.GetNum(t->item);
						if (t->except) {
							if (t->except > amt) {
								amt = 0;
								u->error("TRANSPORT: EXCEPT value greater than amount on hand.");
							} else {
								amt = amt - t->except;
							}
						}
					} else if (amt > u->GetSharedNum(t->item)) {
						amt = u->GetSharedNum(t->item);
						if (amt) {
							u->error("TRANSPORT: Do not have " + ItemString(t->item, t->amount) +
								". Transporting " + ItemString(t->item, amt) + " instead.");
						} else {
							u->error("TRANSPORT: Unable to transport. Have " + ItemString(t->item, 0) + ".");
							continue;
						}
					}

					if (ItemDefs[t->item].max_inventory) {
						int cur = tar->unit->items.GetNum(t->item) + amt;
						if (cur > ItemDefs[t->item].max_inventory) {
							u->error("TRANSPORT: Target cannot have more than " +
									ItemString(t->item, ItemDefs[t->item].max_inventory) + ".");
							continue;
						}
					}

					// now see if the unit can pay for shipping
					int dist = t->distance;
					int weight = ItemDefs[t->item].weight * amt;
					if (weight == 0 && Globals->FRACTIONAL_WEIGHT > 0)
						weight = (amt/Globals->FRACTIONAL_WEIGHT) + 1;
					int cost = 0;
					if (dist > Globals->LOCAL_TRANSPORT) {
						cost = Globals->SHIPPING_COST * weight;
						if (Globals->TRANSPORT & GameDefs::QM_AFFECT_COST)
							cost *= (4 - ((u->GetSkill(S_QUARTERMASTER)+1)/2));
					}

					// if not, don't ship
					if (cost > u->GetSharedMoney()) {
						u->error("TRANSPORT: Cannot afford shipping cost for " + ItemString(t->item, amt) + ".");
						continue;
					}
					u->ConsumeSharedMoney(cost);

					u->ConsumeShared(t->item, amt);
					u->event("Transports " + ItemString(t->item, amt) + " to " + tar->unit->name +
						" for $" + to_string(cost) + ".", "transport");
					if (u->faction != tar->unit->faction) {
						tar->unit->event("Receives " + ItemString(t->item, amt) + " from " + u->name + ".", "transport");
					}

					if (phase == TransportOrder::TransportPhase::INTER_QM_TRANSPORT) {
						tar->unit->transport_items.SetNum(t->item, tar->unit->transport_items.GetNum(t->item) + amt);
					} else {
						tar->unit->items.SetNum(t->item, tar->unit->items.GetNum(t->item) + amt);
					}
					tar->unit->faction->DiscoverItem(t->item, 0, 1);

					u->Practice(S_QUARTERMASTER);

				}
			}
		}
	}
}

void Game::RunSacrificeOrders() {
	// Check all units for sacrifice orders.  Only allow a sacrifice if there is an object in the hex which accepts
	// the item they are sacrificing.  If the sacrifice is successful, the item(s) are removed and the object is
	// destroyed/transformed/provides a reward as appropriate.
	for(const auto r : regions) {
		std::vector<Object *> destroyed_objects;
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				SacrificeOrder *o = u->sacrificeorders;
				if (o == nullptr) continue;

				bool succeeded = false;

				// Okay, this unit has a sacrifice order
				// check all objects in this region to see if any of them accept this sacfrifice
				for(const auto sacrifice_object : r->objects) {
					ObjectType sacrifice_type = ObjectDefs[sacrifice_object->type];
					if (!(sacrifice_type.flags & ObjectType::SACRIFICE)) continue;
					if (sacrifice_type.sacrifice_item != o->item) continue;
					if (sacrifice_object->incomplete >= 0) continue;

					// Make sure any sacrifice rewards are valid -- there must be at least one of an item or replaced
					// object reward.
					int reward_item = sacrifice_type.sacrifice_effect.granted_item;
					int reward_amt = sacrifice_type.sacrifice_effect.granted_amount;
					int reward_obj = sacrifice_type.sacrifice_effect.replace_object;
					bool destroy = sacrifice_type.sacrifice_effect.destroyed;

					// cannot reward a disabled item
					if (reward_item != -1 && (ItemDefs[reward_item].flags & ItemType::DISABLED)) continue;
					// Must reward at least 1 of an item if it rewards an item.
					if (reward_item != -1 && reward_amt == 0) continue;
					// Cannot be replaced by a disabled object.
					if (reward_obj != -1 && (ObjectDefs[reward_obj].flags & ObjectType::DISABLED)) continue;

					// Ok this sacrifice will be valid
					succeeded = true;

					int current = u->items.GetNum(o->item);
					if (current < o->amount) {
						u->error("SACRIFICE: You can only sacrifice up to " + ItemString(o->item, current) + ".");
						o->amount = current;
					}

					int required = -sacrifice_object->incomplete;
					int used = min(required, o->amount);

					// Okay we have a valid sacrifice.  Do it.
					sacrifice_object->incomplete += used;
					u->items.SetNum(o->item, current - used);
					u->event("Sacrifices " + ItemString(o->item, used) + ".", "sacrifice");
					// sacrifice objects store the remaining needed sacrifices as negative numbers in incomplete
					if (sacrifice_object->incomplete >= 0) {
						// was the reward an item
						if (reward_item != -1) {
							bool done = false;
							for(const auto ob : r->objects) {
								for(const auto recip : ob->units) {
									if (recip->faction == u->faction && recip->GetMen() != 0) {
										done = true;
										recip->items.SetNum(reward_item, recip->items.GetNum(reward_item) + reward_amt);
										recip->event("Gains " + ItemString(reward_item, reward_amt) +
											" from sacrifice.", "sacrifice");
										recip->faction->DiscoverItem(reward_item, 0, 1);

										break;
									}
								}
								// if we already found a reward target stop looking.
								if (done) break;
							}
						}

						if (reward_obj != -1 ) {
							sacrifice_object->type = reward_obj;
							sacrifice_object->set_name(ObjectDefs[reward_obj].name);
							u->faction->objectshows.push_back({.obj = reward_obj});
						}
						if (destroy) {
							// mark the object for destruction
							destroyed_objects.push_back(sacrifice_object);
						}
					}
				}

				// If we didn't succeed, log the error
				if (!succeeded) {
					u->error("SACRIFICE: Unable to sacrifice " + ItemString(o->item, o->amount));
				}
			}
		}

		// destroy any pending objects in this region.
		for(const auto obj: destroyed_objects) {
			// This shouldn't happen, as the sacrifice objects are not enterable, but.. just in case.
			for(const auto u : obj->units) {
				u->MoveUnit(r->GetDummy());
				u->event("Moved out of a sacrificed structure.", "sacrifice");
			}
			r->objects.remove(obj);
			delete obj;
		}
	}
}

void Game::Do1Annihilate(ARegion *reg) {
	// converts the type of the region to a barren type (either barrens or barren ocean).   When a region is
	// annihilated all units, and any city/markets/production in the region are destroyed. Shafts and anomalies are
	// unaffected.
	if (TerrainDefs[reg->type].flags & TerrainType::ANNIHILATED) return; // just a double check

	// put the annihilation in the news and tell every faction.
	string message = string(reg->Print().const_str()) + " has been utterly annihilated.";
	AnnihilationFact *fact = new AnnihilationFact();
	fact->message = message;
	events->AddFact(fact);

	// tell all factions too
	for(const auto f : factions) {
		if (f->is_npc) continue;
		f->event(message, "annihilate", reg);
	}

	if (TerrainDefs[reg->type].similar_type == R_OCEAN) {
		reg->type = R_BARRENOCEAN;
	} else {
		reg->type = R_BARREN;
	}
	// destroy all units in the region
	std::vector<Object *> destroyed_objects;
	for(const auto obj : reg->objects) {
		for(const auto u : obj->units) {
			std::for_each(u->items.begin(), u->items.end(), [](Item *it) { delete it; });
			u->items.clear();
			u->event("Is annihilated.", "annihilate", reg);
			reg->Kill(u);
		}
		// add the object to the list to be destroyed if it is able to be annihlated
		if (ObjectDefs[obj->type].flags & ObjectType::NOANNIHILATE) continue;
		destroyed_objects.push_back(obj);
	}

	// Okay, now we need to destroy all the destroyed objects
	for (const auto obj: destroyed_objects) {
		reg->objects.remove(obj);
		delete obj;
	}

	// Okay, now we clear out towns, markets, production, etc.
	for (auto& p : reg->products) delete p; // Free the allocated object
	for (auto& m : reg->markets) delete m; // Free the allocated object
	reg->markets.clear();
	reg->products.clear();
	reg->development = 0;
	reg->wages = 0;
	reg->race = -1;
	reg->population = 0;
	reg->basepopulation = 0;
	reg->maxdevelopment = 0;
	reg->maxwages = 0;
	reg->wealth = 0;
	delete reg->town;
	reg->town = nullptr;
}

void Game::RunAnnihilateOrders() {
	// Check all units for annihilate orders.  A unit my only annihilate if they have access to the annihilate skill.
	// Annihilate will destroy the target hex and all surrounding hexes.  Already annihilated regions cannot be
	// annihilated again.
	int max_annihilates = rulesetSpecificData.value("allowed_annihilates", 1);

	for(const auto r : regions) {
		for(const auto obj : r->objects) {
			for(const auto u : obj->units) {
				// How many annihilates is a unit allowed to do?
				int allowed_annihilates = max_annihilates;

				if (u->GetSkill(S_ANNIHILATION) <= 0) {
					if (u->annihilateorders.empty()) continue; // no annihilate orders
					u->error("ANNIHILATE: Unit does not have access to the annihilate skill.");
					u->annihilateorders.clear(); // clear the orders
					continue;
				}

				while(allowed_annihilates > 0 && !u->annihilateorders.empty()) {
					AnnihilateOrder *o = u->annihilateorders.front();
					u->annihilateorders.pop_front();

					// Ok we have a unit doing an annihilate order.
					ARegion *target = regions.GetRegion(o->xloc, o->yloc, o->zloc);
					if (target == nullptr) {
						u->error("ANNIHILATE: Target region does not exist.");
						continue;
					}

					// Check if the target is in range
					RangeType *rt = FindRange("rng_annihilate");
					int rtype = target->level->levelType;
					if ((rt->flags & RangeType::RNG_SURFACE_ONLY) && (rtype  != ARegionArray::LEVEL_SURFACE)) {
						u->error("ANNIHILATE: Target region is not on the surface.");
						continue;
					}

					// If the range for annihilation is changed, this code will need to be updated.  This really should
					// be made better, but not today. (right now this has a range of 1000 and a cross level penalty of 0)
					int dist = regions.GetPlanarDistance(r, target, rt->crossLevelPenalty, rt->rangeMult);
					if (dist > rt->rangeMult) {
						u->error("ANNIHILATE: Target region is out of range.");
						continue;
					}

					// Make sure the target isn't already annihilated
					if (TerrainDefs[target->type].flags & TerrainType::ANNIHILATED) {
						u->error("ANNIHILATE: Target region is already annihilated.");
						continue;
					}

					// annihilate our neigbors
					for (auto n = 0; n < NDIRS; n++) {
						ARegion *neigh = target->neighbors[n];
						if (neigh == nullptr) continue;
						Do1Annihilate(neigh);
					}
					Do1Annihilate(target);

					// lastly update that we cast one.
					allowed_annihilates--;
				}

				// If the unit didn't use all their annihilates, and we should do random ones, do them.
				while(allowed_annihilates > 0) {
					// pick a random region region to annihilate
					ARegionArray *level = nullptr;

					RangeType *rt = FindRange("rng_annihilate");
					if (rt->flags & RangeType::RNG_SURFACE_ONLY) {
						level = regions.get_first_region_array_of_type(ARegionArray::LEVEL_SURFACE);
					} else {
						int zloc = rng::get_random(regions.numLevels);
						level = regions.GetRegionArray(zloc);
						// Don't allow it to annihilate the Nexus
						if (level->levelType == ARegionArray::LEVEL_NEXUS) continue;
					}
					// now, get a random region from that level that isn't our own.
					if (level == nullptr) break;

					ARegion *target = nullptr;
					int tries = 0;
					while(!target) {
						if (tries++ > 25) break; // don't loop forever
						target = level->GetRegion(rng::get_random(level->x), rng::get_random(level->y));
						if (!target) continue; // no region there
						if (target == r) { target = nullptr; continue; } // don't pick our own region
						if (TerrainDefs[target->type].flags & TerrainType::ANNIHILATED) {
							target = nullptr; // don't pick an already annihilated region
						}
					}
					if (target == nullptr) break; // couldn't find a valid region

					// Ok, we found a valid, non-annihilated region, blow it up.
					for (auto n = 0; n < NDIRS; n++) {
						ARegion *neigh = target->neighbors[n];
						if (neigh == nullptr) continue;
						Do1Annihilate(neigh);
					}
					Do1Annihilate(target);
					// lastly update that we cast one.
					allowed_annihilates--;
				}

				// If the unit still has annihilate orders, keep them for next turn.
				while(!u->annihilateorders.empty()) {
					AnnihilateOrder *o = u->annihilateorders.front();
					u->annihilateorders.pop_front();

					TurnOrder *tOrder = new TurnOrder;
					std::string order = "ANNIHILATE " + to_string(o->xloc) + " " + to_string(o->yloc) +
						" " + to_string(o->zloc);
					tOrder->repeating = 0;
					tOrder->turnOrders.push_back(order);
					u->turnorders.push_front(tOrder);
				}
			}
		}
	}
}

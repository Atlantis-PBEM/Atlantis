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
// Date			Person			Comments
// ----			------			--------
// 2000/MAR/14	Larry Stanbery	Fixed SHOW bug.
// 2000/MAR/14	Davis Kulis		Added a new reporting Template.
// 2001/Feb/18	Joseph Traub	Added support for Apprentices
// 2001/Feb/21	Joseph Traub	Added the ablity to disable items/skills/objects
// 2001/Feb/21	Joseph Traub	Added FACLIM_UNLIMITED
#include "game.h"
#include "gameio.h"
#include "orders.h"
#include "skills.h"
#include "gamedata.h"

OrdersCheck::OrdersCheck()
{
	pCheckFile = 0;
	numshows = 0;
	dummyUnit.monthorders = 0;
}

void OrdersCheck::Error(const AString &strError)
{
	if(pCheckFile) {
		pCheckFile->PutStr("");
		pCheckFile->PutStr("");
		pCheckFile->PutStr(AString("*** Error: ") + strError + " ***");
	}
}

int Game::ParseDir(AString *token)
{
	for (int i=0; i<NDIRS; i++) {
		if (*token == DirectionStrs[i]) return i;
		if (*token == DirectionAbrs[i]) return i;
	}
	if (*token == "in") return MOVE_IN;
	if (*token == "out") return MOVE_OUT;
	int num = token->value();
	if (num) return MOVE_ENTER + num;
	return -1;
}

int ParseTF(AString *token)
{
	if (*token == "true") return 1;
	if (*token == "false") return 0;
	if (*token == "t") return 1;
	if (*token == "f") return 0;
	if (*token == "on") return 1;
	if (*token == "off") return 0;
	if (*token == "1") return 1;
	if (*token == "0") return 0;
	return -1;
}

UnitId *Game::ParseUnit(AString *s)
{
	AString *token = s->gettoken();
	if (!token) return 0;

	if (*token == "0") {
		delete token;
		UnitId *id = new UnitId;
		id->unitnum = -1;
		id->alias = 0;
		id->faction = 0;
		return id;
	}

	if (*token == "faction") {
		delete token;
		/* Get faction number */
		token = s->gettoken();
		if (!token) return 0;

		int fn = token->value();
		delete token;
		if (!fn) return 0;

		/* Next token should be "new" */
		token = s->gettoken();
		if (!token) return 0;

		if (!(*token == "new")) {
			delete token;
			return 0;
		}
		delete token;

		/* Get alias number */
		token = s->gettoken();
		if (!token) return 0;

		int un = token->value();
		delete token;
		if (!un) return 0;

		/* Return UnitId */
		UnitId *id = new UnitId;
		id->unitnum = 0;
		id->alias = un;
		id->faction = fn;
		return id;
	}

	if (*token == "new") {
		delete token;
		token = s->gettoken();
		if (!token) return 0;

		int un = token->value();
		delete token;
		if (!un) return 0;

		UnitId *id = new UnitId;
		id->unitnum = 0;
		id->alias = un;
		id->faction = 0;
		return id;
	} else {
		int un = token->value();
		delete token;
		if (!un) return 0;

		UnitId *id = new UnitId;
		id->unitnum = un;
		id->alias = 0;
		id->faction = 0;
		return id;
	}
}

int ParseFactionType(AString *o, int *type)
{
	int i;
	for (i=0; i<NFACTYPES; i++) type[i] = 0;

	AString * token = o->gettoken();
	if (!token) return -1;

	if (*token == "generic") {
		delete token;
		for (i=0; i<NFACTYPES; i++) type[i] = 1;
		return 0;
	}

	while(token) {
		int foundone = 0;
		for (i=0; i<NFACTYPES; i++) {
			if (*token == FactionStrs[i]) {
				delete token;
				token = o->gettoken();
				if (!token) return -1;
				type[i] = token->value();
				delete token;
				foundone = 1;
				break;
			}
		}
		if (!foundone) {
			delete token;
			return -1;
		}
		token = o->gettoken();
	}

	int tot = 0;
	for (i=0; i<NFACTYPES; i++) {
		tot += type[i];
	}
	if (tot > Globals->FACTION_POINTS) return -1;

	return 0;
}

void Game::ParseError(OrdersCheck *pCheck, Unit *pUnit, Faction *pFaction,
		const AString &strError)
{
	if(pCheck) pCheck->Error(strError);
	else if(pUnit) pUnit->Error(strError);
	else if(pFaction) pFaction->Error(strError);
}

void Game::ParseOrders(int faction, Aorders *f, OrdersCheck *pCheck)
{
	Faction *fac = 0;
	Unit *unit = 0;

	AString *order = f->GetLine();
	while (order) {
		AString saveorder = *order;
		int getatsign = order->getat();
		AString * token = order->gettoken();

		if (token) {
			int i = Parse1Order(token);
			switch (i) {
			case -1:
				ParseError(pCheck, unit, fac, *token+" is not a valid order.");
				break;
			case O_ATLANTIS:
				if(fac)
					ParseError(pCheck, 0, fac, "No #END statement given.");
				delete token;
				token = order->gettoken();
				if(!token) {
					ParseError(pCheck, 0, 0,
							"No faction number given on #atlantis line.");
					fac = 0;
					break;
				}
				if(pCheck) {
					fac = &(pCheck->dummyFaction);
					pCheck->numshows = 0;
				} else {
					fac = GetFaction(&factions,token->value());
				}

				if (!fac) break;

				delete token;
				token = order->gettoken();

				if(pCheck) {
					if(!token) {
						ParseError(pCheck, 0, fac,
								"Warning: No password on #atlantis line.");
						ParseError(pCheck, 0, fac,
								"If this is your first turn, ignore this "
								"error.");
					}
				} else {
					if(!(*(fac->password) == "none")) {
						if(!token || !(*(fac->password) == *token)) {
							ParseError(pCheck, 0, fac,
									"Incorrect password on #atlantis line.");
							fac = 0;
							break;
						}
					}

					if (fac->num == monfaction || fac->num == guardfaction) {
						fac = 0;
						break;
					}
					if(!Globals->LASTORDERS_MAINTAINED_BY_SCRIPTS)
						fac->lastorders = TurnNumber();
				}

				unit = 0;
				break;

			case O_END:
				while (unit) {
					Unit *former = unit->former;
					if (unit->inTurnBlock)
						ParseError(pCheck, unit, fac,"TURN: without ENDTURN");
					if (unit->former)
						ParseError(pCheck, unit, fac, "FORM: without END.");
					if(unit && pCheck) unit->ClearOrders();
					if (pCheck && former) delete unit;
					unit = former;
				}

				unit = 0;
				fac = 0;
				break;

			case O_UNIT:
				if (fac) {
					while (unit) {
						Unit *former = unit->former;
						if (unit->inTurnBlock)
							ParseError(pCheck, unit, fac,"TURN: without ENDTURN");
						if (unit->former)
							ParseError(pCheck, unit, fac, "FORM: without END.");
						if (unit && pCheck) unit->ClearOrders();
						if (pCheck && former) delete unit;
						unit = former;
					}
					unit = 0;
					delete token;

					token = order->gettoken();
					if (!token) {
						ParseError(pCheck,0,fac,"UNIT without unit number.");
						unit = 0;
						break;
					}

					if(pCheck) {
						if (!token->value()) {
							pCheck->Error("Invalid unit number.");
						} else {
							unit = &(pCheck->dummyUnit);
							unit->monthorders = 0;
						}
					} else {
						unit = GetUnit(token->value());
						if(!unit || unit->faction != fac) {
							fac->Error(*token + " is not your unit.");
							unit = 0;
						} else {
							unit->ClearOrders();
						}
					}
				}
				break;
			case O_FORM:
				if (fac) {
					if(unit) {
						if (unit->former && !unit->inTurnBlock) {
							ParseError(pCheck, unit, fac, "FORM: cannot nest.");
						}
						else {
							unit = ProcessFormOrder(unit, order, pCheck);
							if(!pCheck) {
								if(unit) unit->ClearOrders();
							}
						}
					} else {
						ParseError(pCheck, 0, fac,
								"Order given without a unit selected.");
					}
				}
				break;
			case O_ENDFORM:
				if (fac) {
					if (unit && unit->former) {
						Unit *former = unit->former;

						if (unit->inTurnBlock)
							ParseError(pCheck, unit, fac,"TURN: without ENDTURN");
						if (pCheck && former) delete unit;
						unit = former;
					} else {
						ParseError(pCheck, unit, fac, "END: without FORM.");
					}
				}
				break;
			case O_TURN:
				if (unit && unit->inTurnBlock) {
					ParseError(pCheck, unit, fac,"TURN: cannot nest");
				} else if (!unit)
					ParseError(pCheck, 0, fac, "Order given without a unit selected.");
				else {
					// faction is 0 if checking syntax only, not running turn.
					if (faction != 0) {
						AString *retval;
						retval = ProcessTurnOrder(unit, f, pCheck, getatsign);
						if (retval) {
							delete order;
							order = retval;
							continue;
						}
					} else {
						unit->inTurnBlock = 1;
						unit->presentMonthOrders = unit->monthorders;
						unit->monthorders = 0;
						unit->presentTaxing = unit->taxing;
						unit->taxing = 0;
					}
				}
				break;
			case O_ENDTURN:
				if (unit && unit->inTurnBlock) {
					if (unit->monthorders) delete unit->monthorders;
					unit->monthorders = unit->presentMonthOrders;
					unit->presentMonthOrders = 0;
					unit->taxing = unit->presentTaxing;
					unit->presentTaxing = 0;
					unit->inTurnBlock = 0;
				} else
					ParseError(pCheck, unit, fac, "ENDTURN: without TURN.");
				break;
			default:
				if (fac) {
					if (unit) {
						if(!pCheck && getatsign)
							unit->oldorders.Add(new AString(saveorder));

						ProcessOrder(i, unit, order, pCheck);
					} else {
						ParseError(pCheck, 0, fac,
								"Order given without a unit selected.");
					}
				}
			}
			SAFE_DELETE(token);
		} else {
			if(!pCheck) {
				if(getatsign && fac && unit)
					unit->oldorders.Add(new AString(saveorder));
			}
		}

		delete order;
		if(pCheck) {
			pCheck->pCheckFile->PutStr(saveorder);
		}

		order = f->GetLine();
	}

	while (unit) {
		Unit *former = unit->former;
		if (unit->inTurnBlock)
			ParseError(pCheck, 0, fac,"TURN: without ENDTURN");
		if (unit->former)
			ParseError(pCheck, 0, fac, "FORM: without END.");
		if(unit && pCheck) unit->ClearOrders();
		if (pCheck && former) delete unit;
		unit = former;
	}

	if(unit && pCheck) {
		unit->ClearOrders();
		unit = 0;
	}
}

void Game::ProcessOrder(int orderNum, Unit *unit, AString *o,
		OrdersCheck *pCheck)
{
	switch(orderNum) {
		case O_ADDRESS:
			ProcessAddressOrder(unit, o, pCheck);
			break;
		case O_ADVANCE:
			ProcessAdvanceOrder(unit, o, pCheck);
			break;
		case O_ASSASSINATE:
			ProcessAssassinateOrder(unit, o, pCheck);
			break;
		case O_ATTACK:
			ProcessAttackOrder(unit, o, pCheck);
			break;
		case O_AUTOTAX:
			ProcessAutoTaxOrder(unit, o, pCheck);
			break;
		case O_AVOID:
			ProcessAvoidOrder(unit, o, pCheck);
			break;
		case O_BEHIND:
			ProcessBehindOrder(unit, o, pCheck);
			break;
		case O_BUILD:
			ProcessBuildOrder(unit, o, pCheck);
			break;
		case O_BUY:
			ProcessBuyOrder(unit, o, pCheck);
			break;
		case O_CAST:
			ProcessCastOrder(unit, o, pCheck);
			break;
		case O_CLAIM:
			ProcessClaimOrder(unit, o, pCheck);
			break;
		case O_COMBAT:
			ProcessCombatOrder(unit, o, pCheck);
			break;
		case O_CONSUME:
			ProcessConsumeOrder(unit, o, pCheck);
			break;
		case O_DECLARE:
			ProcessDeclareOrder(unit->faction, o, pCheck);
			break;
		case O_DESCRIBE:
			ProcessDescribeOrder(unit, o, pCheck);
			break;
		case O_DESTROY:
			ProcessDestroyOrder(unit, pCheck);
		break;
		case O_ENTER:
			ProcessEnterOrder(unit, o, pCheck);
			break;
		case O_ENTERTAIN:
			ProcessEntertainOrder(unit, pCheck);
			break;
		case O_EVICT:
			ProcessEvictOrder(unit, o, pCheck);
			break;
		case O_EXCHANGE:
			ProcessExchangeOrder(unit, o, pCheck);
			break;
		case O_FACTION:
			ProcessFactionOrder(unit, o, pCheck);
			break;
		case O_FIND:
			ProcessFindOrder(unit, o, pCheck);
			break;
		case O_FORGET:
			ProcessForgetOrder(unit, o, pCheck);
			break;
		case O_WITHDRAW:
			ProcessWithdrawOrder(unit, o, pCheck);
			break;
		case O_GIVE:
			ProcessGiveOrder(unit, o, pCheck);
			break;
		case O_GUARD:
			ProcessGuardOrder(unit, o, pCheck);
			break;
		case O_HOLD:
			ProcessHoldOrder(unit, o, pCheck);
			break;
		case O_LEAVE:
			ProcessLeaveOrder(unit, pCheck);
			break;
		case O_MOVE:
			ProcessMoveOrder(unit, o, pCheck);
			break;
		case O_NAME:
			ProcessNameOrder(unit, o, pCheck);
			break;
		case O_NOAID:
			ProcessNoaidOrder(unit, o, pCheck);
			break;
		case O_NOCROSS:
			ProcessNocrossOrder(unit, o, pCheck);
			break;
		case O_NOSPOILS:
			ProcessNospoilsOrder(unit, o, pCheck);
			break;
		case O_OPTION:
			ProcessOptionOrder(unit, o, pCheck);
			break;
		case O_PASSWORD:
			ProcessPasswordOrder(unit, o, pCheck);
			break;
		case O_PILLAGE:
			ProcessPillageOrder(unit, pCheck);
			break;
		case O_PREPARE:
			ProcessPrepareOrder(unit, o, pCheck);
			break;
		case O_WEAPON:
			ProcessWeaponOrder(unit, o, pCheck);
			break;
		case O_ARMOR:
			ProcessArmorOrder(unit, o, pCheck);
			break;
		case O_PRODUCE:
			ProcessProduceOrder(unit, o, pCheck);
			break;
		case O_PROMOTE:
			ProcessPromoteOrder(unit, o, pCheck);
			break;
		case O_QUIT:
			ProcessQuitOrder(unit, o, pCheck);
			break;
		case O_RESTART:
			ProcessRestartOrder(unit, o, pCheck);
			break;
		case O_REVEAL:
			ProcessRevealOrder(unit, o, pCheck);
			break;
		case O_SAIL:
			ProcessSailOrder(unit, o, pCheck);
			break;
		case O_SELL:
			ProcessSellOrder(unit, o, pCheck);
			break;
		case O_SHOW:
			ProcessReshowOrder(unit, o, pCheck);
			break;
		case O_SPOILS:
			ProcessSpoilsOrder(unit, o, pCheck);
			break;
		case O_STEAL:
			ProcessStealOrder(unit, o, pCheck);
			break;
		case O_STUDY:
			ProcessStudyOrder(unit, o, pCheck);
			break;
		case O_TAX:
			ProcessTaxOrder(unit, pCheck);
			break;
		case O_TEACH:
			ProcessTeachOrder(unit, o, pCheck);
			break;
		case O_WORK:
			ProcessWorkOrder(unit, pCheck);
			break;
	}
}

void Game::ProcessPasswordOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	if(pCheck) return;

	AString * token = o->gettoken();
	if (u->faction->password) delete u->faction->password;
	if (token) {
		u->faction->password = token;
		u->faction->Event(AString("Password is now: ") + *token);
	} else {
		u->faction->password = new AString("none");
		u->faction->Event("Password cleared.");
	}
}

void Game::ProcessOptionOrder(Unit * u,AString *o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "OPTION: What option?");
		return;
	}

	if (*token == "times") {
		delete token;
		if(!pCheck) {
			u->faction->Event("Times will be sent to your faction.");
			u->faction->times = 1;
		}
		return;
	}

	if (*token == "notimes") {
		delete token;
		if(!pCheck) {
			u->faction->Event("Times will not be sent to your faction.");
			u->faction->times = 0;
		}
		return;
	}

	if (*token == "template") {
		delete token;

		token = o->gettoken();
		if (!token) {
			ParseError(pCheck, u, 0, "OPTION: No template type specified.");
			return;
		}

		int newformat = -1;
		if (*token == "off") {
			newformat = TEMPLATE_OFF;
		}
		if (*token == "short") {
			newformat = TEMPLATE_SHORT;
		}
		if (*token == "long") {
			newformat = TEMPLATE_LONG;
		}
		// DK
		if (*token == "map") {
			newformat = TEMPLATE_MAP;
		}
		delete token;

		if (newformat == -1) {
			ParseError(pCheck, u, 0, "OPTION: Invalid template type.");
			return;
		}

		if(!pCheck) {
			u->faction->temformat = newformat;
		}

		return;
	}

	delete token;

	ParseError(pCheck, u, 0, "OPTION: Invalid option.");
}

void Game::ProcessReshowOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		// LLS
		ParseError(pCheck, u, 0, "SHOW: Show what?");
		return;
	}

	if(pCheck) {
		if(pCheck->numshows++ > 100) {
			if(pCheck->numshows == 102) {
				pCheck->Error("Too many SHOW orders.");
			}
			return;
		}
	} else {
		if (u->faction->numshows++ > 100) {
			if (u->faction->numshows == 102) {
				u->Error("Too many SHOW orders.");
			}
			return;
		}
	}

	if (*token == "skill") {
		delete token;

		token = o->gettoken();
		if (!token) {
			ParseError(pCheck, u, 0, "SHOW: Show what skill?");
			return;
		}
		int sk = ParseSkill(token);
		delete token;

		if(sk == -1 ||
				(SkillDefs[sk].flags & SkillType::DISABLED) ||
				((SkillDefs[sk].flags & SkillType::APPRENTICE) &&
				 !Globals->APPRENTICES_EXIST)) {
			ParseError(pCheck, u, 0, "SHOW: No such skill.");
			return;
		}

		token = o->gettoken();
		if (!token) {
			ParseError(pCheck, u, 0, "SHOW: No skill level given.");
			return;
		}
		int lvl = token->value();
		delete token;

		if(!pCheck) {
			if (lvl > u->faction->skills.GetDays(sk)) {
				u->Error("SHOW: Faction doesn't have that skill.");
				return;
			}

			u->faction->shows.Add(new ShowSkill(sk,lvl));
		}
		return;
	}

	if (*token == "item") {
		delete token;
		token = o->gettoken();

		if (!token) {
			ParseError(pCheck, u, 0, "SHOW: Show which item?");
			return;
		}

		int item = ParseEnabledItem(token);
		delete token;

		if(item == -1 || (ItemDefs[item].flags & ItemType::DISABLED)) {
			ParseError(pCheck, u, 0, "SHOW: No such item.");
			return;
		}

		if(!pCheck) {
			u->faction->DiscoverItem(item, 1, 0);
		}
		return;
	}

	if (*token == "object") {
		delete token;
		token = o->gettoken();

		if(!token) {
			ParseError(pCheck, u, 0, "SHOW: Show which object?");
			return;
		}

		int obj = ParseObject(token);
		delete token;

		if(obj == -1 || (ObjectDefs[obj].flags & ObjectType::DISABLED)) {
			ParseError(pCheck, u, 0, "SHOW: No such object.");
			return;
		}

		if(!pCheck) {
			u->faction->objectshows.Add(ObjectDescription(obj));
		}
		return;
	}

	ParseError(pCheck, u, 0, "SHOW: Show what?");
}

void Game::ProcessForgetOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "FORGET: No skill given.");
		return;
	}

	int sk = ParseSkill(token);
	delete token;

	if (sk==-1) {
		ParseError(pCheck, u, 0, "FORGET: Invalid skill.");
		return;
	}

	if(!pCheck) {
		ForgetOrder *ord = new ForgetOrder;
		ord->skill = sk;
		u->forgetorders.Add(ord);
	}
}

void Game::ProcessEntertainOrder(Unit * unit, OrdersCheck *pCheck)
{
	if (unit->monthorders ||
			(Globals->TAX_PILLAGE_MONTH_LONG &&
			 ((unit->taxing == TAX_TAX) ||
			  (unit->taxing == TAX_PILLAGE)))) {
		AString err = "ENTERTAIN: Overwriting previous ";
		if (unit->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, unit, 0, err);
		if (unit->monthorders) delete unit->monthorders;
	}
	ProduceOrder * o = new ProduceOrder;
	o->item = I_SILVER;
	o->skill = S_ENTERTAINMENT;
	unit->monthorders = o;
}

void Game::ProcessCombatOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		if(!pCheck) {
			u->combat = -1;
			u->Event("Combat spell set to none.");
		}
		return;
	}
	int sk = ParseSkill(token);
	delete token;

	if (sk==-1) {
		ParseError(pCheck, u, 0, "COMBAT: Invalid skill.");
		return;
	}
	if(!(SkillDefs[sk].flags & SkillType::MAGIC)) {
		ParseError(pCheck, u, 0, "COMBAT: That is not a magic skill.");
		return;
	}
	if(!(SkillDefs[sk].flags & SkillType::COMBAT)) {
		ParseError(pCheck, u, 0,
				"COMBAT: That skill cannot be used in combat.");
		return;
	}

	if(!pCheck) {
		if (u->type != U_MAGE) {
			u->Error("COMBAT: That unit is not a mage.");
			return;
		}
		if (!u->GetSkill(sk)) {
			u->Error("COMBAT: Unit does not possess that skill.");
			return;
		}

		u->combat = sk;
		AString temp = AString("Combat spell set to ") + SkillDefs[sk].name;
		if(Globals->USE_PREPARE_COMMAND) {
			u->readyItem = -1;
			temp += " and prepared item set to none";
		}
		temp += ".";

		u->Event(temp);
	}
}

// Lacandon's prepare command
void Game::ProcessPrepareOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if(!(Globals->USE_PREPARE_COMMAND)) {
		ParseError(pCheck, u, 0, "PREPARE is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	if(!token) {
		if(!pCheck) {
			u->readyItem = -1;
			u->Event("Prepared battle item set to none.");
		}
		return;
	}
	int it = ParseEnabledItem(token);
	int bt = ParseBattleItem(it);
	delete token;

	if(bt == -1) {
		ParseError(pCheck, u, 0, "PREPARE: Invalid item.");
		return;
	}
	if(!(BattleItemDefs[bt].flags & BattleItemType::SPECIAL)) {
		ParseError(pCheck, u, 0, "PREPARE: That item cannot be prepared.");
		return;
	}

	if(!pCheck) {
		if((BattleItemDefs[bt].flags & BattleItemType::MAGEONLY) &&
				!((u->type == U_MAGE) || (u->type == U_APPRENTICE) ||
					(u->type == U_GUARDMAGE))) {
			u->Error("PREPARE: Only a mage or apprentice may use this item.");
			return;
		}
		if(!u->items.GetNum(it)) {
			u->Error("PREPARE: Unit does not possess that item.");
			return;
		}
		u->readyItem = it;
		AString temp;
		temp = AString("Prepared item set to ") + ItemDefs[it].name;
		if(u->combat != -1) {
			u->combat = -1;
			temp += " and combat spell set to none";
		}
		temp += ".";
		u->Event(temp);
	}
}

void Game::ProcessWeaponOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if(!(Globals->USE_WEAPON_ARMOR_COMMAND)) {
		ParseError(pCheck, u, 0, "WEAPON is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	int i;
	if (!token) {
		if (!pCheck) {
			for (i = 0; i < MAX_READY; ++i) u->readyWeapon[i] = -1;
			u->Event("Preferred weapons set to none.");
		}
		return;
	}
	int it;
	int items[MAX_READY];
	i = 0;
	while (token && (i < MAX_READY)) {
		it = ParseEnabledItem(token);
		delete token;
		if (it == -1) {
			ParseError(pCheck, u, 0, "WEAPON: Invalid item.");
		} else if (!(ItemDefs[it].type & IT_WEAPON)) {
			ParseError(pCheck, u, 0, "WEAPON: Item is not a weapon.");
		} else {
			if(!pCheck) items[i++] = it;
		}
		token = o->gettoken();
	}
	if (token) delete token;
	if(pCheck) return;

	while (i < MAX_READY) {
		items[i++] = -1;
	}
	if (items[0] == -1) return;
	AString temp = "Preferred weapons set to: ";
	for (i=0; i<MAX_READY;++i) {
		u->readyWeapon[i] = items[i];
		if (items[i] != -1) {
			if (i > 0) temp += ", ";
			temp += ItemDefs[items[i]].name;
		}
	}
	temp += ".";
	u->Event(temp);
}

void Game::ProcessArmorOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if(!(Globals->USE_WEAPON_ARMOR_COMMAND)) {
		ParseError(pCheck, u, 0, "ARMOR is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	int i;
	if (!token) {
		if (!pCheck) {
			for (i = 0; i < MAX_READY; ++i) u->readyArmor[i] = -1;
			u->Event("Preferred armor set to none.");
		}
		return;
	}
	int it;
	int items[MAX_READY];
	i = 0;
	while (token && (i < MAX_READY)) {
		it = ParseEnabledItem(token);
		delete token;
		if (it == -1) {
			ParseError(pCheck, u, 0, "ARMOR: Invalid item.");
		} else if (!(ItemDefs[it].type & IT_ARMOR)) {
			ParseError(pCheck, u, 0, "ARMOR: Item is not armor.");
		} else {
			if(!pCheck) items[i++] = it;
		}
		token = o->gettoken();
	}
	if (token) delete token;
	if(pCheck) return;

	while (i < MAX_READY) {
		items[i++] = -1;
	}
	if (items[0] == -1) return;
	AString temp = "Preferred armor set to: ";
	for (i=0; i<MAX_READY;++i) {
		u->readyArmor[i] = items[i];
		if (items[i] != -1) {
			if (i > 0) temp += ", ";
			temp += ItemDefs[items[i]].name;
		}
	}
	temp += ".";
	u->Event(temp);
}

void Game::ProcessClaimOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "CLAIM: No amount given.");
		return;
	}

	int value = token->value();
	delete token;
	if (!value) {
		ParseError(pCheck, u, 0, "CLAIM: No amount given.");
		return;
	}

	if(!pCheck) {
		if (value > u->faction->unclaimed) {
			u->Error("CLAIM: Don't have that much unclaimed silver.");
			value = u->faction->unclaimed;
		}
		u->faction->unclaimed -= value;
		u->SetMoney(u->GetMoney() + value);
		u->Event(AString("Claims $") + value + ".");
	}
}

void Game::ProcessFactionOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_FACTION_TYPES) {
		ParseError(pCheck, u, 0,
				"FACTION: Invalid order, no faction types in this game.");
		return;
	}

	int oldfactype[ NFACTYPES ];
	int factype[ NFACTYPES ];

	int i;
	if(!pCheck) {
		for(i = 0; i < NFACTYPES; i++) {
			oldfactype[ i ] = u->faction->type[ i ];
		}
	}

	int retval = ParseFactionType(o,factype);
	if (retval == -1) {
		ParseError(pCheck, u, 0, "FACTION: Bad faction type.");
		return;
	}

	if(!pCheck) {
		int m = CountMages(u->faction);
		int a = CountApprentices(u->faction);

		for(i = 0; i < NFACTYPES; i++) {
			u->faction->type[i] = factype[i];
		}

		if(m > AllowedMages(u->faction)) {
			u->Error(AString("FACTION: Too many mages to change to that "
							 "faction type."));

			for(i = 0; i < NFACTYPES; i++) {
				u->faction->type[ i ] = oldfactype[ i ];
			}

			return;
		}
		if(a > AllowedApprentices(u->faction)) {
			u->Error(AString("FACTION: Too many apprentices to change "
						"to that faction type."));

			for(i = 0; i < NFACTYPES; i++) {
				u->faction->type[ i ] = oldfactype[ i ];
			}

			return;
		}

		u->faction->lastchange = TurnNumber();
		u->faction->DefaultOrders();
	}
}

void Game::ProcessAssassinateOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	UnitId * id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		ParseError(pCheck, u, 0, "ASSASSINATE: No target given.");
		return;
	}
	if(!pCheck) {
		if (u->stealorders) delete u->stealorders;
		AssassinateOrder * ord = new AssassinateOrder;
		ord->target = id;
		u->stealorders = ord;
	}
}

void Game::ProcessStealOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	UnitId * id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		ParseError(pCheck, u, 0, "STEAL: No target given.");
		return;
	}
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "STEAL: No item given.");
		delete id;
		return;
	}
	int i = ParseEnabledItem(token);
	delete token;
	if (i == -1) {
		ParseError(pCheck, u, 0, "STEAL: Bad item given.");
		delete id;
		return;
	}

	if (IsSoldier(i)) {
		ParseError(pCheck, u, 0, "STEAL: Can't steal that.");
		delete id;
		return;
	}
	if(!pCheck) {
		StealOrder * ord = new StealOrder;
		ord->target = id;
		ord->item = i;
		if (u->stealorders) delete u->stealorders;
		u->stealorders = ord;
	}
}

void Game::ProcessQuitOrder(Unit *u,AString *o, OrdersCheck *pCheck)
{
	if(!pCheck) {
		if (u->faction->password && !(*(u->faction->password) == "none")) {
			AString *token = o->gettoken();
			if (!token) {
				u->faction->Error("QUIT: Must give the correct password.");
				return;
			}

			if (!(*token == *(u->faction->password))) {
				delete token;
				u->faction->Error("QUIT: Must give the correct password.");
				return;
			}

			delete token;
		}

		if (u->faction->quit != QUIT_AND_RESTART) {
			u->faction->quit = QUIT_BY_ORDER;
		}
	}
}

void Game::ProcessRestartOrder(Unit *u,AString *o, OrdersCheck *pCheck)
{
	if(!pCheck) {
		if (u->faction->password && !(*(u->faction->password) == "none")) {
			AString *token = o->gettoken();
			if (!token) {
				u->faction->Error("RESTART: Must give the correct password.");
				return;
			}

			if (!(*token == *(u->faction->password))) {
				delete token;
				u->faction->Error("RESTART: Must give the correct password.");
				return;
			}

			delete token;
		}

		if (u->faction->quit != QUIT_AND_RESTART) {
			u->faction->quit = QUIT_AND_RESTART;
			Faction *pFac = AddFaction(1);
			pFac->SetAddress(*(u->faction->address));
			AString *pass = new AString(*(u->faction->password));
			pFac->password = pass;
			AString *facstr = new AString(AString("Restarting ")
					+ *(pFac->address) + ".");
			newfactions.Add(facstr);
		}
	}
}

void Game::ProcessDestroyOrder(Unit * u, OrdersCheck *pCheck)
{
	if(!pCheck) {
		u->destroy = 1;
	}
}

void Game::ProcessFindOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "FIND: No faction number given.");
		return;
	}
	int n = token->value();
	int is_all = (*token == "all");
	delete token;
	if (n==0 && !is_all) {
		ParseError(pCheck, u, 0, "FIND: No faction number given.");
		return;
	}
	if(!pCheck) {
		FindOrder * f = new FindOrder;
		f->find = n;
		u->findorders.Add(f);
	}
}

void Game::ProcessConsumeOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (token) {
		if (*token == "unit") {
			if(!pCheck) {
				u->SetFlag(FLAG_CONSUMING_UNIT, 1);
				u->SetFlag(FLAG_CONSUMING_FACTION, 0);
			}
			delete token;
			return;
		}

		if (*token == "faction") {
			if(!pCheck) {
				u->SetFlag(FLAG_CONSUMING_UNIT, 0);
				u->SetFlag(FLAG_CONSUMING_FACTION, 1);
			}
			delete token;
			return;
		}

		if (*token == "none") {
			if(!pCheck) {
				u->SetFlag(FLAG_CONSUMING_UNIT, 0);
				u->SetFlag(FLAG_CONSUMING_FACTION, 0);
			}
			delete token;
			return;
		}

		delete token;
		ParseError(pCheck, u, 0, "CONSUME: Invalid value.");
	} else {
		if(!pCheck) {
			u->SetFlag(FLAG_CONSUMING_UNIT, 0);
			u->SetFlag(FLAG_CONSUMING_FACTION, 0);
		}
	}
}

void Game::ProcessRevealOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	if(!pCheck) {
		AString * token = o->gettoken();
		if (token) {
			if (*token == "unit") {
				u->reveal = REVEAL_UNIT;
				delete token;
				return;
			}
			if (*token == "faction") {
				delete token;
				u->reveal = REVEAL_FACTION;
				return;
			}
			if (*token == "none") {
				delete token;
				u->reveal = REVEAL_NONE;
				return;
			}
		} else {
			u->reveal = REVEAL_NONE;
		}
	}
}

void Game::ProcessTaxOrder(Unit * u, OrdersCheck *pCheck)
{
	if (u->taxing == TAX_PILLAGE) {
		ParseError(pCheck, u, 0, "TAX: The unit is already pillaging.");
		return;
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG && u->monthorders) {
		delete u->monthorders;
		u->monthorders = NULL;
		AString err = "TAX: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
	}

	u->taxing = TAX_TAX;
}

void Game::ProcessPillageOrder(Unit * u, OrdersCheck *pCheck)
{
	if (u->taxing == TAX_TAX) {
		ParseError(pCheck, u, 0, "PILLAGE: The unit is already taxing.");
		return;
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG && u->monthorders) {
		delete u->monthorders;
		u->monthorders = NULL;
		AString err = "PILLAGE: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
	}

	u->taxing = TAX_PILLAGE;
}

void Game::ProcessPromoteOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	UnitId * id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		ParseError(pCheck, u, 0, "PROMOTE: No target given.");
		return;
	}
	if(!pCheck) {
		if (u->promote) {
			delete u->promote;
		}
		u->promote = id;
	}
}

void Game::ProcessLeaveOrder(Unit * u, OrdersCheck *pCheck)
{
	if(!pCheck) {
		if (u->monthorders && u->monthorders->type == O_BUILD) return;
		if (u->enter == 0) u->enter = -1;
	}
}

void Game::ProcessEnterOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "ENTER: No object specified.");
		return;
	}
	int i = token->value();
	delete token;
	if (i) {
		if(!pCheck) {
			u->enter = i;
		}
	} else {
		ParseError(pCheck, u, 0, "ENTER: No object specified.");
	}
}

void Game::ProcessBuildOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (token) {
		if(*token == "help") {
			UnitId *targ = 0;
			delete token;
			if(!pCheck) {
				targ = ParseUnit(o);
				if(!targ) {
					unit->Error("BUILD: Non-existant unit to help.");
					return;
				}
				if(targ->unitnum == -1) {
					unit->Error("BUILD: Non-existant unit to help.");
					return;
				}
			}
			BuildOrder * order = new BuildOrder;
			order->target = targ;
			if (unit->monthorders ||
					(Globals->TAX_PILLAGE_MONTH_LONG &&
					 ((unit->taxing == TAX_TAX) ||
					  (unit->taxing == TAX_PILLAGE)))) {
				if (unit->monthorders) delete unit->monthorders;
				AString err = "BUILD: Overwriting previous ";
				if (unit->inTurnBlock) err += "DELAYED ";
				err += "month-long order.";
				ParseError(pCheck, unit, 0, err);
			}
			if(Globals->TAX_PILLAGE_MONTH_LONG) unit->taxing = TAX_NONE;
			unit->monthorders = order;
			if (unit->enter == -1) unit->enter = 0;
			return;
		}

		int ot = ParseObject(token);
		delete token;
		if (ot==-1) {
			ParseError(pCheck, unit, 0, "BUILD: Not a valid object name.");
			return;
		}
		if(ObjectDefs[ot].flags & ObjectType::DISABLED) {
			ParseError(pCheck, unit, 0, "BUILD: Not a valid object name.");
			return;
		}
		if (!pCheck) {
			ARegion *reg = unit->object->region;
			if (TerrainDefs[reg->type].similar_type == R_OCEAN){
				unit->Error("BUILD: Can't build in an ocean.");
				return;
			}

			if (ObjectIsShip(ot) && ot != O_BALLOON) {
				if (!reg->IsCoastalOrLakeside()) {
					unit->Error("BUILD: Can't build ship in "
							"non-coastal or lakeside region.");
					return;
				}
			}
			if (reg->buildingseq > 99) {
				unit->Error("BUILD: The region is full.");
				return;
			}
			Object * obj = new Object(reg);
			obj->type = ot;
			obj->incomplete = ObjectDefs[obj->type].cost;
			unit->build = obj;
			unit->object->region->objects.Add(obj);
		}
	}

	BuildOrder *order = new BuildOrder;
	order->target = NULL;
	if (unit->monthorders ||
			(Globals->TAX_PILLAGE_MONTH_LONG &&
			 ((unit->taxing == TAX_TAX) || (unit->taxing == TAX_PILLAGE)))) {
		if (unit->monthorders) delete unit->monthorders;
		AString err = "BUILD: Overwriting previous ";
		if (unit->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, unit, 0, err);
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) unit->taxing = TAX_NONE;
	unit->monthorders = order;
	if (unit->enter == -1) unit->enter = 0;
}

void Game::ProcessAttackOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	UnitId * id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		if(!pCheck) {
			if (!u->attackorders) u->attackorders = new AttackOrder;
			u->attackorders->targets.Add(id);
		}
		id = ParseUnit(o);
	}
}

void Game::ProcessSellOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "SELL: Number to sell not given.");
		return;
	}
	int num = 0;
	if(*token == "ALL") {
		num = -1;
	} else {
		num = token->value();
	}
	delete token;
	if (!num) {
		ParseError(pCheck, u, 0, "SELL: Number to sell not given.");
		return;
	}
	token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "SELL: Item not given.");
		return;
	}
	int it = ParseGiveableItem(token);
	delete token;
	if (it == -1) {
		ParseError(pCheck, u, 0, "SELL: Can't sell that.");
		return;
	}
	if(ItemDefs[it].flags & ItemType::DISABLED) {
		ParseError(pCheck, u, 0, "SELL: Can't sell that.");
		return;
	}
	if(!pCheck) {
		SellOrder * s = new SellOrder;
		s->item = it;
		s->num = num;
		u->sellorders.Add(s);
	}
}

void Game::ProcessBuyOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "BUY: Number to buy not given.");
		return;
	}
	int num = 0;
	if(*token == "ALL") {
		num = -1;
	} else {
		num = token->value();
	}
	delete token;
	if (!num) {
		ParseError(pCheck, u, 0, "BUY: Number to buy not given.");
		return;
	}
	token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "BUY: Item not given.");
		return;
	}
	int it = ParseGiveableItem(token);
	if(it == -1) {
		if(*token == "peasant" || *token == "peasants" || *token == "peas") {
			if(pCheck) {
				it = -1;
				for(int i = 0; i < NITEMS; i++) {
					if(ItemDefs[i].flags & ItemType::DISABLED) continue;
					if(i == I_LEADERS) continue;
					if(ItemDefs[i].type & IT_MAN) {
						it = i;
						break;
					}
				}
			} else {
				it = u->object->region->race;
			}
		}
	}
	delete token;
	if (it == -1) {
		ParseError(pCheck, u, 0, "BUY: Can't buy that.");
		return;
	}
	if(ItemDefs[it].flags & ItemType::DISABLED) {
		ParseError(pCheck, u, 0, "BUY: Can't buy that.");
		return;
	}

	if(!pCheck) {
		BuyOrder * b = new BuyOrder;
		b->item = it;
		b->num = num;
		u->buyorders.Add(b);
	}
}

void Game::ProcessProduceOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "PRODUCE: No item given.");
		return;
	}
	int it = ParseEnabledItem(token);
	delete token;

	if (it == -1) {
		ParseError(pCheck, u, 0, "PRODUCE: Can't produce that.");
		return;
	}
	if(ItemDefs[it].flags & ItemType::DISABLED) {
		ParseError(pCheck, u, 0, "PRODUCE: Can't produce that.");
		return;
	}

	ProduceOrder * p = new ProduceOrder;
	p->item = it;
	p->skill = ItemDefs[it].pSkill;
	if (u->monthorders ||
		(Globals->TAX_PILLAGE_MONTH_LONG &&
		 ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		if (u->monthorders) delete u->monthorders;
		AString err = "PRODUCE: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = p;
}

void Game::ProcessWorkOrder(Unit * u, OrdersCheck *pCheck)
{
	ProduceOrder * order = new ProduceOrder;
	order->skill = -1;
	order->item = I_SILVER;
	if (u->monthorders ||
		(Globals->TAX_PILLAGE_MONTH_LONG &&
		 ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		if (u->monthorders) delete u->monthorders;
		AString err = "WORK: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = order;
}

void Game::ProcessTeachOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	TeachOrder * order = 0;

	if (u->monthorders && u->monthorders->type == O_TEACH) {
		order = (TeachOrder *) u->monthorders;
	} else {
		order = new TeachOrder;
	}

	int students = 0;
	UnitId * id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		students++;
		if(order) {
			order->targets.Add(id);
		}
		id = ParseUnit(o);
	}

	if (!students) {
		ParseError(pCheck, u, 0, "TEACH: No students given.");
		return;
	}

	if ((u->monthorders && u->monthorders->type != O_TEACH) ||
		(Globals->TAX_PILLAGE_MONTH_LONG &&
		 ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		if (u->monthorders) delete u->monthorders;
		AString err = "TEACH: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = order;
}

void Game::ProcessStudyOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "STUDY: No skill given.");
		return;
	}
	int sk = ParseSkill(token);
	delete token;
	if (sk==-1) {
		ParseError(pCheck, u, 0, "STUDY: Invalid skill.");
		return;
	}

	if(SkillDefs[sk].flags & SkillType::DISABLED) {
		ParseError(pCheck, u, 0, "STUDY: Invalid skill.");
		return;
	}

	if((SkillDefs[sk].flags & SkillType::APPRENTICE) &&
			!Globals->APPRENTICES_EXIST) {
		ParseError(pCheck, u, 0, "STUDY: Invalid skill.");
		return;
	}

	StudyOrder * order = new StudyOrder;
	order->skill = sk;
	order->days = 0;
	if (u->monthorders ||
		(Globals->TAX_PILLAGE_MONTH_LONG &&
		 ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		if (u->monthorders) delete u->monthorders;
		AString err = "STUDY: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = order;
}

void Game::ProcessDeclareOrder(Faction * f,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, 0, f, "DECLARE: No faction given.");
		return;
	}
	int fac;
	if (*token == "default") {
		fac = -1;
	} else {
		fac = token->value();
	}
	delete token;

	if(!pCheck) {
		Faction * target;
		if (fac != -1) {
			target = GetFaction(&factions,fac);
			if (!target) {
				f->Error(AString("DECLARE: Non-existent faction ")+fac+".");
				return;
			}
			if (target == f) {
				f->Error(AString("DECLARE: Can't declare towards your own "
								 "faction."));
				return;
			}
		}
	}

	token = o->gettoken();
	if (!token) {
		if (fac != -1) {
			if(!pCheck) {
				f->SetAttitude(fac,-1);
			}
		}
		return;
	}

	int att = ParseAttitude(token);
	delete token;
	if (att == -1) {
		ParseError(pCheck, 0, f, "DECLARE: Invalid attitude.");
		return;
	}

	if(!pCheck) {
		if (fac == -1) {
			f->defaultattitude = att;
		} else {
			f->SetAttitude(fac,att);
		}
	}
}

void Game::ProcessWithdrawOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	if(!(Globals->ALLOW_WITHDRAW)) {
		ParseError(pCheck, unit, 0, "WITHDRAW is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	if(!token) {
		ParseError (pCheck, unit, 0, "WITHDRAW: No amount given.");
		return;
	}
	int amt = token->value();
	if(amt < 1) {
		amt = 1;
	} else {
		delete token;
		token = o->gettoken();
		if(!token) {
			ParseError(pCheck, unit, 0, "WITHDRAW: No item given.");
			return;
		}
	}
	int item = ParseGiveableItem(token);
	delete token;

	if (item == -1) {
		ParseError(pCheck, unit, 0, "WITHDRAW: Invalid item.");
		return;
	}
	if (ItemDefs[item].flags & ItemType::DISABLED) {
		ParseError(pCheck, unit, 0, "WITHDRAW: Invalid item.");
		return;
	}
	if (!(ItemDefs[item].type & IT_NORMAL)) {
		ParseError(pCheck, unit, 0, "WITHDRAW: Invalid item.");
		return;
	}
	if(item == I_SILVER) {
		ParseError(pCheck, unit, 0, "WITHDRAW: Invalid item.");
		return;
	}

	if(!pCheck) {
		WithdrawOrder *order = new WithdrawOrder;
		order->item = item;
		order->amount = amt;
		unit->withdraworders.Add(order);
	}
	return;
}

AString *Game::ProcessTurnOrder(Unit * unit, Aorders *f, OrdersCheck *pCheck,
		int repeat)
{
	int turnDepth = 1;
	int turnLast = 1;
	int formDepth = 0;
	int atsign;

	TurnOrder *tOrder = new TurnOrder;
	tOrder->repeating = repeat;

	AString *order, *token;

	while (turnDepth) {
		// get the next line
		order = f->GetLine();
		if (!order) {
			// Fake end of commands to invoke appropriate processing
			order = new AString("#end");
		}
		AString	saveorder = *order;
		token = order->gettoken();
		atsign = order->getat();

		if (token) {
			int i = Parse1Order(token);
			switch (i) {
				case O_TURN:
					if (turnLast) {
						ParseError(pCheck, unit, 0, "TURN: cannot nest.");
						break;
					}
					turnDepth++;
					tOrder->turnOrders.Add(new AString(saveorder));
					turnLast = 1;
					break;
				case O_FORM:
					if (!turnLast) {
						ParseError(pCheck, unit, 0, "FORM: cannot nest.");
						break;
					}
					turnLast = 0;
					formDepth++;
					tOrder->turnOrders.Add(new AString(saveorder));
					break;
				case O_ENDFORM:
					if (turnLast) {
						if (!(formDepth + (unit->former != 0))) {
							ParseError(pCheck, unit, 0, "END: without FORM.");
							break;
						} else {
							ParseError(pCheck, unit, 0,
									"TURN: without ENDTURN.");
							if (!--turnDepth) {
								unit->turnorders.Add(tOrder);
								return new AString(saveorder);
							}
						}
					}
					formDepth--;
					tOrder->turnOrders.Add(new AString(saveorder));
					turnLast = 1;
					break;
				case O_UNIT:
				case O_END:
					if (!turnLast)
						ParseError(pCheck, unit, 0, "FORM: without END.");
					while (--turnDepth) {
						ParseError(pCheck, unit, 0, "TURN: without ENDTURN.");
						ParseError(pCheck, unit, 0, "FORM: without END.");
					}
					ParseError(pCheck, unit, 0, "TURN: without ENDTURN.");
					unit->turnorders.Add(tOrder);
					return new AString(saveorder);
					break;
				case O_ENDTURN:
					if (!turnLast) {
						ParseError(pCheck, unit, 0, "ENDTURN: without TURN.");
					} else {
						if (--turnDepth)
							tOrder->turnOrders.Add(new AString(saveorder));
						turnLast = 0;
					}
					break;
				default:
					tOrder->turnOrders.Add(new AString(saveorder));
					break;
			}
			delete token;
		}
		delete order;
	}

	unit->turnorders.Add(tOrder);

	return NULL;
}

void Game::ProcessExchangeOrder(Unit * unit, AString * o, OrdersCheck *pCheck)
{
	UnitId * t = ParseUnit(o);
	if (!t) {
		ParseError(pCheck, unit, 0, "EXCHANGE: Invalid target.");
		return;
	}
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, unit, 0, "EXCHANGE: No amount given.");
		return;
	}
	int amtGive;
	amtGive = token->value();
	delete token;

	if(amtGive < 0) {
		ParseError(pCheck, unit, 0, "EXCHANGE: Illegal amount given.");
		return;
	}

	token = o->gettoken();
	if (!token) {
		ParseError(pCheck, unit, 0, "EXCHANGE: No item given.");
		return;
	}
	int itemGive;
	itemGive = ParseGiveableItem(token);
	delete token;

	if(itemGive == -1) {
		ParseError(pCheck, unit, 0, "EXCHANGE: Invalid item.");
		return;
	}

	token = o->gettoken();
	if (!token) {
		ParseError(pCheck, unit, 0, "EXCHANGE: No amount expected.");
		return;
	}
	int amtExpected;
	amtExpected = token->value();
	delete token;

	if(amtExpected < 0) {
		ParseError(pCheck, unit, 0, "EXCHANGE: Illegal amount given.");
		return;
	}

	token = o->gettoken();
	if (!token) {
		ParseError(pCheck, unit, 0, "EXCHANGE: No item expected.");
		return;
	}
	int itemExpected;
	itemExpected = ParseGiveableItem(token);
	delete token;

	if(itemExpected == -1) {
		ParseError(pCheck, unit, 0, "EXCHANGE: Invalid item.");
		return;
	}

	if(!pCheck) {
		ExchangeOrder * order = new ExchangeOrder;
		order->giveItem = itemGive;
		order->giveAmount = amtGive;
		order->expectAmount = amtExpected;
		order->expectItem = itemExpected;
		order->target = t;
		unit->exchangeorders.Add(order);
	}
}

void Game::ProcessGiveOrder(Unit *unit,AString * o, OrdersCheck *pCheck)
{
	UnitId * t = ParseUnit(o);
	if (!t) {
		ParseError(pCheck, unit, 0, "GIVE: Invalid target.");
		return;
	}
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, unit, 0, "GIVE: No amount given.");
		return;
	}
	int amt;
	if (*token == "unit") {
		amt = -1;
	} else if(*token == "all") {
		amt = -2;
	} else {
		amt = token->value();
	}
	delete token;
	int item = I_LEADERS;
	if (amt != -1) {
		token = o->gettoken();
		if(token) {
			if (t->unitnum == -1)
				item = ParseEnabledItem(token);
			else
				item = ParseGiveableItem(token);
			if(amt == -2) {
				int found = 0;
				if(*token == "normal") {
					item = -IT_NORMAL;
					found = 1;
				} else if(*token == "advanced") {
					item = -IT_ADVANCED;
					found = 1;
				} else if(*token == "trade") {
					item = -IT_TRADE;
					found = 1;
				} else if((*token == "man") || (*token == "men")) {
					item = -IT_MAN;
					found = 1;
				} else if((*token == "monster") || (*token == "monsters")) {
					item = -IT_MONSTER;
					found = 1;
				} else if(*token == "magic") {
					item = -IT_MAGIC;
					found = 1;
				} else if((*token == "weapon") || (*token == "weapons")) {
					item = -IT_WEAPON;
					found = 1;
				} else if(*token == "armor") {
					item = -IT_ARMOR;
					found = 1;
				} else if((*token == "mount") || (*token == "mounts")) {
					item = -IT_MOUNT;
					found = 1;
				} else if(*token == "battle") {
					item = -IT_BATTLE;
					found = 1;
				} else if(*token == "special") {
					item = -IT_SPECIAL;
					found = 1;
				} else if(*token == "food") {
					item = -IT_FOOD;
					found = 1;
				} else if((*token == "tool") || (*token == "tools")) {
					item = -IT_TOOL;
					found = 1;
				} else if((*token == "item") || (*token == "items")) {
					item = -NITEMS;
					found = 1;
				} else if(item != -1) {
					found = 1;
				}
				if(!found) {
					ParseError(pCheck, unit, 0,
							"GIVE: Invalid item or item class.");
					return;
				}
			} else if(item == -1) {
				ParseError(pCheck, unit, 0, "GIVE: Invalid item.");
				return;
			}
		} else {
			ParseError(pCheck, unit, 0, "GIVE: No item given.");
			return;
		}
		delete token;
	}

	token = o->gettoken();
	int excpt = 0;
	if(token && *token == "except") {
		if(amt == -2) {
			delete token;
			if(item < 0) {
				ParseError(pCheck, unit, 0,
						"GIVE: EXCEPT only valid with specific items.");
				return;
			}
			token = o->gettoken();
			if(!token) {
				ParseError(pCheck, unit, 0, "GIVE: EXCEPT requires a value.");
				return;
			}
			excpt = token->value();
			if(excpt <= 0) {
				ParseError(pCheck, unit, 0, "GIVE: Invalid EXCEPT value.");
				return;
			}
		} else {
			ParseError(pCheck, unit, 0, "GIVE: EXCEPT only valid with ALL");
			return;
		}
		delete token;
	}

	if(!pCheck) {
		GiveOrder * order = new GiveOrder;
		order->item = item;
		order->target = t;
		order->amount = amt;
		order->except = excpt;
		unit->giveorders.Add(order);
	}
	return;
}

void Game::ProcessDescribeOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, unit, 0, "DESCRIBE: No argument.");
		return;
	}
	if (*token == "unit") {
		delete token;
		token = o->gettoken();
		if(!pCheck) {
			unit->SetDescribe(token);
		}
		return;
	}
	if (*token == "ship" || *token == "building" || *token == "object" ||
		*token == "structure") {
		delete token;
		token = o->gettoken();
		if(!pCheck) {
			// ALT, 25-Jul-2000
			// Fix to prevent non-owner units from describing objects
			if(unit != unit->object->GetOwner()) {
				unit->Error("DESCRIBE: Unit is not owner.");
				return;
			}
			unit->object->SetDescribe(token);
		}
		return;
	}
	ParseError(pCheck, unit, 0, "DESCRIBE: Can't describe that.");
}

void Game::ProcessNameOrder(Unit *unit,AString * o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, unit, 0, "NAME: No argument.");
		return;
	}
	if (*token == "faction") {
		delete token;
		token = o->gettoken();
		if (!token) {
			ParseError(pCheck, unit, 0, "NAME: No name given.");
			return;
		}
		if(!pCheck) {
			unit->faction->SetName(token);
		}
		return;
	}

	if (*token == "unit") {
		delete token;
		token = o->gettoken();
		if (!token) {
			ParseError(pCheck, unit, 0, "NAME: No name given.");
			return;
		}
		if(!pCheck) {
			unit->SetName(token);
		}
		return;
	}

	if (*token == "building" || *token == "ship" || *token == "object" ||
		*token == "structure") {
		delete token;
		token = o->gettoken();
		if (!token) {
			ParseError(pCheck, unit, 0, "NAME: No name given.");
			return;
		}
		if(!pCheck) {
			// ALT, 25-Jul-2000
			// Fix to prevent non-owner units from renaming objects
			if(unit != unit->object->GetOwner()) {
				unit->Error("NAME: Unit is not owner.");
				return;
			}
			unit->object->SetName(token);
		}
		return;
	}

	// ALT, 26-Jul-2000
	// Allow some units to rename cities. Unit must be at least the owner
	// of tower to rename village, fort to rename town and castle to
	// rename city.
	if (*token == "village" || *token == "town" || *token == "city") {
		delete token;
		token = o->gettoken();

		if (!token) {
			ParseError(pCheck, unit, 0, "NAME: No name given.");
			return;
		}

		if (!pCheck) {
			if(!unit->object) {
				unit->Error("NAME: Unit is not in a structure.");
				return;
			}
			if(!unit->object->region->town) {
				unit->Error("NAME: Unit is not in a village, town or city.");
				return;
			}
			int cost = 0;
			int towntype = unit->object->region->town->TownType();
			AString tstring;
			switch(towntype) {
				case TOWN_VILLAGE:
					tstring = "village";
					break;
				case TOWN_TOWN:
					tstring = "town";
					break;
				case TOWN_CITY:
					tstring = "city";
					break;
			}
			if(Globals->CITY_RENAME_COST) {
				cost = (towntype+1)* Globals->CITY_RENAME_COST;
			}
			int ok = 0;
			switch(towntype) {
				case TOWN_VILLAGE:
					if(unit->object->type == O_TOWER) ok = 1;
				case TOWN_TOWN:
					if(unit->object->type == O_FORT) ok = 1;
				case TOWN_CITY:
					if(unit->object->type == O_CASTLE) ok = 1;
					if(unit->object->type == O_CITADEL) ok = 1;
					if(unit->object->type == O_MFORTRESS) ok = 1;
			}
			if(!ok) {
				unit->Error(AString("NAME: Unit is not in a large ")+
							"enough structure to rename a "+tstring+".");
				return;
			}
			if (unit != unit->object->GetOwner()) {
				unit->Error(AString("NAME: Cannot name ")+tstring+
							".  Unit is not the owner of object.");
				return;
			}
			if (unit->object->incomplete > 0) {
				unit->Error(AString("NAME: Cannot name ")+tstring+
							".  Object is not finished.");
				return;
			}

			AString * newname = token->getlegal();
			if (!newname) {
				unit->Error("NAME: Illegal name.");
				return;
			}
			if(cost) {
				int silver = unit->items.GetNum(I_SILVER);
				if(silver < cost) {
					unit->Error(AString("NAME: Unit doesn't have enough ")+
								"silver to rename a "+tstring+".");
					return;
				}
				unit->items.SetNum(I_SILVER, silver-cost);
			}

			unit->Event(AString("Renames ") +
					*(unit->object->region->town->name) + " to " +
					*newname + ".");
			unit->object->region->NotifyCity(unit,
					*(unit->object->region->town->name), *newname);
			delete unit->object->region->town->name;
			unit->object->region->town->name = newname;
		}
		return;
	}

	delete token;
	ParseError(pCheck, unit, 0, "NAME: Can't name that.");
}

void Game::ProcessGuardOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "GUARD: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "GUARD: Invalid value.");
		return;
	}
	if(!pCheck) {
		if (val==0) {
			if (u->guard != GUARD_AVOID)
				u->guard = GUARD_NONE;
		} else {
			u->guard = GUARD_SET;
		}
	}
}

void Game::ProcessBehindOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "BEHIND: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	if (val == -1) {
		ParseError(pCheck, u, 0, "BEHIND: Invalid value.");
		return;
	}
	if(!pCheck) {
		u->SetFlag(FLAG_BEHIND,val);
	}
}

void Game::ProcessNoaidOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "NOAID: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "NOAID: Invalid value.");
		return;
	}
	if(!pCheck) {
		u->SetFlag(FLAG_NOAID,val);
	}
}

void Game::ProcessSpoilsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	int flag = 0;
	int val = 1;
	if(token) {
		if(*token == "none") flag = FLAG_NOSPOILS;
		else if(*token == "walk") flag = FLAG_WALKSPOILS;
		else if(*token == "ride") flag = FLAG_RIDESPOILS;
		else if(*token == "fly") flag = FLAG_FLYSPOILS;
		else if(*token == "all") val = 0;
		else ParseError(pCheck, u, 0, "SPOILS: Bad argument.");
		delete token;
	}

	if(!pCheck) {
		/* Clear all the flags */
		u->SetFlag(FLAG_NOSPOILS, 0);
		u->SetFlag(FLAG_WALKSPOILS, 0);
		u->SetFlag(FLAG_RIDESPOILS, 0);
		u->SetFlag(FLAG_FLYSPOILS, 0);

		/* Set the flag we're trying to set */
		if(flag) {
			u->SetFlag(flag, val);
		}
	}
}

void Game::ProcessNospoilsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	ParseError(pCheck, u, 0, "NOSPOILS: This command is deprecated.  "
			"Use the 'SPOILS' command instead");

	/* Instant order */
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "NOSPOILS: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "NOSPILS: Invalid value.");
		return;
	}
	if(!pCheck) {
		u->SetFlag(FLAG_FLYSPOILS, 0);
		u->SetFlag(FLAG_RIDESPOILS, 0);
		u->SetFlag(FLAG_WALKSPOILS, 0);
		u->SetFlag(FLAG_NOSPOILS,val);
	}
}

void Game::ProcessNocrossOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	int move_over_water = 0;

	if(Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE)
		move_over_water = 1;
	if(!move_over_water) {
		int i;
		for(i = 0; i < NITEMS; i++) {
			if(ItemDefs[i].flags & ItemType::DISABLED) continue;
			if(ItemDefs[i].swim > 0) move_over_water = 1;
		}
	}
	if(!move_over_water) {
		ParseError(pCheck, u, 0, "NOCROSS is not a valid order.");
		return;
	}

	/* Instant order */
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "NOCROSS: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "NOCROSS: Invalid value.");
		return;
	}
	if(!pCheck) {
		u->SetFlag(FLAG_NOCROSS_WATER,val);
	}
}


void Game::ProcessHoldOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "HOLD: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "HOLD: Invalid value.");
		return;
	}
	if(!pCheck) {
		u->SetFlag(FLAG_HOLDING,val);
	}
}

void Game::ProcessAutoTaxOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "AUTOTAX: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "AUTOTAX: Invalid value.");
		return;
	}
	if(!pCheck) {
		u->SetFlag(FLAG_AUTOTAX,val);
	}
}

void Game::ProcessAvoidOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString * token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "AVOID: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "AVOID: Invalid value.");
		return;
	}
	if(!pCheck) {
		if (val==1) {
			u->guard = GUARD_AVOID;
		} else {
			if (u->guard == GUARD_AVOID) {
				u->guard = GUARD_NONE;
			}
		}
	}
}

Unit *Game::ProcessFormOrder(Unit *former, AString *o, OrdersCheck *pCheck)
{
	AString *t = o->gettoken();
	if (!t) {
		ParseError(pCheck, former, 0, "Must give alias in FORM order.");
		return 0;
	}

	int an = t->value();
	delete t;
	if (!an) {
		ParseError(pCheck, former, 0, "Must give alias in FORM order.");
		return 0;
	}
	if(pCheck) {
		Unit *retval = new Unit;
		retval->former = former;
		return retval;
	} else {
		if(former->object->region->GetUnitAlias(an, former->faction->num)) {
			former->Error("Alias multiply defined.");
			return 0;
		}
		Unit *temp = GetNewUnit(former->faction, an);
		temp->CopyFlags(former);
		temp->DefaultOrders(former->object);
		temp->MoveUnit(former->object);
		temp->former = former;
		return temp;
	}
}

void Game::ProcessAddressOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString * token = o->gettoken();
	if (token) {
		if(!pCheck) {
			u->faction->address = token;
		}
	} else {
		ParseError(pCheck, u, 0, "ADDRESS: No address given.");
	}
}

void Game::ProcessAdvanceOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	MoveOrder *m = 0;

	if ((u->monthorders && u->monthorders->type != O_ADVANCE) ||
		(Globals->TAX_PILLAGE_MONTH_LONG &&
		 ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		AString err = "ADVANCE: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long orders";
		ParseError(pCheck, u, 0, err);
		if (u->monthorders) delete u->monthorders;
		u->monthorders = 0;
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	if (!u->monthorders) {
		u->monthorders = new MoveOrder;
		u->monthorders->type = O_ADVANCE;
	}
	m = (MoveOrder *) u->monthorders;
	m->advancing = 1;

	for (;;) {
		AString * t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d!=-1) {
			if(!pCheck) {
				MoveDir * x = new MoveDir;
				x->dir = d;
				m->dirs.Add(x);
			}
		} else {
			ParseError(pCheck, u, 0, "ADVANCE: Warning, bad direction.");
			return;
		}
	}
}

void Game::ProcessMoveOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	MoveOrder *m = 0;

	if ((u->monthorders && u->monthorders->type != O_MOVE) ||
		(Globals->TAX_PILLAGE_MONTH_LONG &&
		 ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		AString err = "MOVE: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
		if (u->monthorders) delete u->monthorders;
		u->monthorders = 0;
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	if (!u->monthorders) {
		u->monthorders = new MoveOrder;
	}
	m = (MoveOrder *) u->monthorders;
	m->advancing = 0;

	for (;;) {
		AString * t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d!=-1) {
			if(!pCheck) {
				MoveDir * x = new MoveDir;
				x->dir = d;
				m->dirs.Add(x);
			}
		} else {
			ParseError(pCheck, u, 0, "MOVE: Warning, bad direction.");
			return;
		}
	}
}

void Game::ProcessSailOrder(Unit * u,AString * o, OrdersCheck *pCheck)
{
	SailOrder *m = 0;

	if ((u->monthorders && u->monthorders->type != O_SAIL) ||
		(Globals->TAX_PILLAGE_MONTH_LONG &&
		 ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		AString err = "SAIL: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
		if (u->monthorders) delete u->monthorders;
		u->monthorders = 0;
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	if (!u->monthorders) {
		u->monthorders = new SailOrder;
	}
	m = (SailOrder *) u->monthorders;

	for (;;) {
		AString * t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d == -1) {
			ParseError(pCheck, u, 0, "SAIL: Warning, bad direction.");
			return;
		} else {
			if (d < NDIRS) {
				if(!pCheck) {
					MoveDir * x = new MoveDir;
					x->dir = d;
					m->dirs.Add(x);
				}
			} else {
				return;
			}
		}
	}
}

void Game::ProcessEvictOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	UnitId * id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		if(!pCheck) {
			if (!u->evictorders) u->evictorders = new EvictOrder;
			u->evictorders->targets.Add(id);
		}
		id = ParseUnit(o);
	}
}

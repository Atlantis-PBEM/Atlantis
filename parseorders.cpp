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

#include "game.h"
#include "gameio.h"
#include "orders.h"
#include "skills.h"
#include "gamedata.h"

using namespace std;

void OrdersCheck::error(const string& err)
{
	pCheckFile << "\n\n*** Error: " << err << " ***\n";
	numerrors++;
}

int Game::ParseDir(AString *token)
{
	for (int i=0; i<NDIRS; i++) {
		if (*token == DirectionStrs[i]) return i;
		if (*token == DirectionAbrs[i]) return i;
	}
	if (*token == "in") return MOVE_IN;
	if (*token == "out") return MOVE_OUT;
	if (*token == "pause" || *token == "p") return MOVE_PAUSE;
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
	if (*token == "yes") return 1;
	if (*token == "no") return 0;
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

int ParseFactionType(AString *o, std::unordered_map<std::string, int> &type)
{
	for (auto &ft : *FactionTypes) {
		type[ft] = 0;
	}

	AString *token = o->gettoken();
	if (!token) return -1;

	if (*token == "generic") {
		delete token;

		for (auto &ft : *FactionTypes) {
			type[ft] = 1;
		}

		return 0;
	}

	while(token) {
		bool foundone = false;

		for (auto &ft : *FactionTypes) {
			if (*token == ft.c_str()) {
				delete token;
				token = o->gettoken();
				if (!token) return -1;
				type[ft] = token->value();
				delete token;
				foundone = true;
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
	for (auto &kv : type) {
		tot += kv.second;
	}
	if (tot > Globals->FACTION_POINTS) return -1;

	return 0;
}

void Game::parse_error(OrdersCheck *order_check, Unit *unit, Faction *faction, const string& error) {
	if (order_check) order_check->error(error);
	else if (unit) unit->error(error);
	else if (faction) faction->error(error);
}

void Game::ParseOrders(int faction, istream& f, OrdersCheck *pCheck)
{
	Faction *fac = nullptr;
	Faction *passFac = nullptr;
	Unit *unit = 0;
	int indent = 0, code, i;
	AString order, prefix;

	f >> ws >> order;
	while (!f.eof()) {
		AString saveorder = order;
		bool repeating = order.getat();
		AString *token = order.gettoken();

		if (token) {
			code = Parse1Order(token);
			switch (code) {
			case -1:
				parse_error(pCheck, unit, fac, string(token->const_str()) + " is not a valid order.");
				break;
			case O_ATLANTIS:
				if (fac)
					parse_error(pCheck, 0, fac, "No #END statement given.");
				delete token;
				token = order.gettoken();
				if (!token) {
					parse_error(pCheck, 0, 0, "No faction number given on #atlantis line.");
					fac = 0;
					break;
				}
				if (pCheck) {
					fac = &(pCheck->dummyFaction);
					pCheck->numshows = 0;
					// Even though we don't use the real faction for other things, we do want it for the password check.
					passFac = GetFaction(factions, token->value());
				} else {
					fac = GetFaction(factions, token->value());
				}

				if (!fac) break;

				delete token;
				token = order.gettoken();

				if (pCheck) {
					if (!token) {
						parse_error(pCheck, 0, fac, "Warning: No password on #atlantis line.");
						parse_error(pCheck, 0, fac, "If this is your first turn, ignore this error.");
					} else {
						// If we found their real faction above (we should have but let's not assume), then we
						// can check if they gave us the correct password.
						if (passFac) {
							bool has_password = !(*(passFac->password) == "none");
							bool wrong_password = !(*(passFac->password) == *token);
							if (has_password && wrong_password) {
								parse_error(pCheck, 0, fac, "Incorrect password on #atlantis line.");
								fac = 0;
								break;
							}
						}
					}
				} else {
					if (!(*(fac->password) == "none")) {
						if (!token || !(*(fac->password) == *token)) {
							parse_error(pCheck, 0, fac, "Incorrect password on #atlantis line.");
							fac = 0;
							break;
						}
					}

					if (fac->num == monfaction || fac->num == guardfaction) {
						fac = 0;
						break;
					}
					if (!Globals->LASTORDERS_MAINTAINED_BY_SCRIPTS)
						fac->lastorders = TurnNumber();
				}

				unit = 0;
				break;

			case O_END:
				indent = 0;
				while (unit) {
					Unit *former = unit->former;
					if (unit->inTurnBlock)
						parse_error(pCheck, unit, fac, "TURN: without ENDTURN");
					if (unit->former)
						parse_error(pCheck, unit, fac, "FORM: without END.");
					if (unit && pCheck) unit->ClearOrders();
					if (pCheck && former) delete unit;
					unit = former;
				}

				unit = 0;
				fac = 0;
				break;

			case O_UNIT:
				indent = 0;
				if (fac) {
					while (unit) {
						Unit *former = unit->former;
						if (unit->inTurnBlock)
							parse_error(pCheck, unit, fac, "TURN: without ENDTURN");
						if (unit->former)
							parse_error(pCheck, unit, fac, "FORM: without END.");
						if (unit && pCheck) unit->ClearOrders();
						if (pCheck && former) delete unit;
						unit = former;
					}
					unit = 0;
					delete token;

					token = order.gettoken();
					if (!token) {
						parse_error(pCheck, 0, fac, "UNIT without unit number.");
						unit = 0;
						break;
					}

					if (pCheck) {
						if (!token->value()) {
							parse_error(pCheck, 0, fac, "Invalid unit number.");
						} else {
							unit = &(pCheck->dummyUnit);
							unit->monthorders = nullptr;
						}
					} else {
						unit = GetUnit(token->value());
						if (!unit || unit->faction != fac) {
							// as we get rid of more uses of astring, these will become unnecessary
							string tmp(token->const_str());
							fac->error(tmp + " is not your unit.");
							unit = 0;
						} else {
							unit->ClearOrders();
						}
					}
				}
				break;
			case O_FORM:
				if (fac) {
					if (unit) {
						if (unit->former && !unit->inTurnBlock) {
							parse_error(pCheck, unit, fac, "FORM: cannot nest.");
						}
						else {
							unit = ProcessFormOrder(unit, &order, pCheck, repeating);
							if (!pCheck && unit && unit->former && unit->former->form_repeated)
								unit->former->oldorders.push_back(saveorder.const_str());
							if (!pCheck) {
								if (unit) unit->ClearOrders();
							}
						}
					} else {
						parse_error(pCheck, 0, fac,
								"Order given without a unit selected.");
					}
				}
				break;
			case O_ENDFORM:
				if (fac) {
					if (unit && unit->former) {
						Unit *former = unit->former;

						if (unit->inTurnBlock)
							parse_error(pCheck, unit, fac, "TURN: without ENDTURN");

						if (!pCheck && unit->former && unit->former->form_repeated)
							unit->former->oldorders.push_back(saveorder.const_str());

						if (pCheck && former) delete unit;
						unit = former;
					} else {
						parse_error(pCheck, unit, fac, "END: without FORM.");
					}
				}
				break;
			case O_TURN:
				if (unit && unit->inTurnBlock) {
					parse_error(pCheck, unit, fac, "TURN: cannot nest");
				} else if (!unit)
					parse_error(pCheck, 0, fac, "Order given without a unit selected.");
				else {
					// faction is 0 if checking syntax only, not running turn.
					if (faction != 0) {
						AString *retval;
						if (!pCheck && unit->former && unit->former->form_repeated)
							unit->former->oldorders.push_back(saveorder.const_str());
						retval = ProcessTurnOrder(unit, f, pCheck, repeating);
						if (retval) {
							order = *retval;
							continue;
						}
					} else {
						unit->inTurnBlock = 1;
						unit->presentMonthOrders = unit->monthorders;
						unit->monthorders = nullptr;
						unit->presentTaxing = unit->taxing;
						unit->taxing = 0;
					}
				}
				break;
			case O_ENDTURN:
				if (unit && unit->inTurnBlock) {
					if (unit->monthorders) delete unit->monthorders;
					unit->monthorders = unit->presentMonthOrders;
					unit->presentMonthOrders = nullptr;
					unit->taxing = unit->presentTaxing;
					unit->presentTaxing = 0;
					unit->inTurnBlock = 0;
					if (!pCheck && unit->former && unit->former->form_repeated)
						unit->former->oldorders.push_back(saveorder.const_str());
				} else
					parse_error(pCheck, unit, fac, "ENDTURN: without TURN.");
				break;
			default:
				if (fac) {
					if (unit) {
						if (!pCheck && repeating)
							unit->oldorders.push_back(saveorder.const_str());
						if (!pCheck && unit->former && unit->former->form_repeated)
							unit->former->oldorders.push_back(saveorder.const_str());

						ProcessOrder(code, unit, &order, pCheck, repeating);
					} else {
						parse_error(pCheck, 0, fac,
								"Order given without a unit selected.");
					}
				}
			}
			SAFE_DELETE(token);
		} else {
			code = NORDERS;
			if (!pCheck) {
				if (repeating && fac && unit)
					unit->oldorders.push_back(saveorder.const_str());
			}
		}

		if (pCheck) {
			if (code == O_ENDTURN || code == O_ENDFORM)
				indent--;
			for (i = 0, prefix = ""; i < indent; i++)
				prefix += "  ";
			pCheck->pCheckFile << prefix << saveorder << "\n";
			if (code == O_TURN || code == O_FORM)
				indent++;
		}
		f >> ws >> order;
	}

	while (unit) {
		Unit *former = unit->former;
		if (unit->inTurnBlock)
			parse_error(pCheck, 0, fac, "TURN: without ENDTURN");
		if (unit->former)
			parse_error(pCheck, 0, fac, "FORM: without END.");
		if (unit && pCheck) unit->ClearOrders();
		if (pCheck && former) delete unit;
		unit = former;
	}

	if (unit && pCheck) {
		unit->ClearOrders();
		unit = 0;
	}

	if (pCheck) {
		pCheck->pCheckFile << "\n";
		if (!pCheck->numerrors) {
			pCheck->pCheckFile << "No errors found.\n";
		} else {
			pCheck->pCheckFile << pCheck->numerrors << " error" << (pCheck->numerrors == 1 ? "" : "s") << " found!\n";
		}
	}
}

void Game::ProcessOrder(int orderNum, Unit *unit, AString *o, OrdersCheck *pCheck, bool repeating)
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
		case O_IDLE:
			ProcessIdleOrder(unit, o, pCheck);
			break;
		case O_BEHIND:
			ProcessBehindOrder(unit, o, pCheck);
			break;
		case O_BUILD:
			ProcessBuildOrder(unit, o, pCheck);
			break;
		case O_BUY:
			ProcessBuyOrder(unit, o, pCheck, repeating);
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
			ProcessGiveOrder(orderNum, unit, o, pCheck);
			break;
		case O_GUARD:
			ProcessGuardOrder(unit, o, pCheck);
			break;
		case O_HOLD:
			ProcessHoldOrder(unit, o, pCheck);
			break;
		case O_JOIN:
			ProcessJoinOrder(unit, o, pCheck);
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
			ProcessSellOrder(unit, o, pCheck, repeating);
			break;
		case O_SHARE:
			ProcessShareOrder(unit, o, pCheck);
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
		case O_TAKE:
			ProcessGiveOrder(orderNum, unit, o, pCheck);
			break;
		case O_TAX:
			ProcessTaxOrder(unit, pCheck);
			break;
		case O_TEACH:
			ProcessTeachOrder(unit, o, pCheck);
			break;
		case O_WORK:
			ProcessWorkOrder(unit, 0, pCheck);
			break;
		case O_TRANSPORT:
		case O_DISTRIBUTE:
			ProcessTransportOrder(unit, o, pCheck);
			break;
		// NO7 victory condition orders
		case O_ANNIHILATE:
			ProcessAnnihilateOrder(unit, o, pCheck);
			break;
		case O_SACRIFICE:
			ProcessSacrificeOrder(unit, o, pCheck);
			break;
	}
}

void Game::ProcessPasswordOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (pCheck) return;

	AString *token = o->gettoken();
	if (u->faction->password) delete u->faction->password;
	if (token) {
		u->faction->password = token;
		string tmp = "Password is now: ";
		tmp += token->const_str();
		u->faction->event(tmp, "password");
	} else {
		u->faction->password = new AString("none");
		u->faction->event("Password cleared.", "password");
	}
}

void Game::ProcessOptionOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "OPTION: What option?");
		return;
	}

	if (*token == "times") {
		delete token;
		if (!pCheck) {
			u->faction->event("Times will be sent to your faction.", "option");
			u->faction->times = 1;
		}
		return;
	}

	if (*token == "notimes") {
		delete token;
		if (!pCheck) {
			u->faction->event("Times will not be sent to your faction.", "option");
			u->faction->times = 0;
		}
		return;
	}

	if (*token == "showattitudes") {
		delete token;
		if (!pCheck) {
			u->faction->event("Units will now have a leading sign to show your attitude to them.", "option");
			u->faction->showunitattitudes = 1;
		}
		return;
	}

	if (*token == "dontshowattitudes") {
		delete token;
		if (!pCheck) {
			u->faction->event("Units will now have a leading minus sign regardless of your attitude to them.", "option");
			u->faction->showunitattitudes = 0;
		}
		return;
	}

	if (*token == "template") {
		delete token;

		token = o->gettoken();
		if (!token) {
			parse_error(pCheck, u, 0, "OPTION: No template type specified.");
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
			parse_error(pCheck, u, 0, "OPTION: Invalid template type.");
			return;
		}

		if (!pCheck) {
			u->faction->temformat = newformat;
		}

		return;
	}

	delete token;

	parse_error(pCheck, u, 0, "OPTION: Invalid option.");
}

void Game::ProcessReshowOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	int sk, lvl, item, obj;
	AString *token;

	token = o->gettoken();
	if (!token) {
		// LLS
		parse_error(pCheck, u, 0, "SHOW: Show what?");
		return;
	}

	int count = (pCheck ? pCheck->numshows++ : u->faction->numshows++);
	if (count > 100) {
		if (count == 102) {
			parse_error(pCheck, u, 0, "Too many SHOW orders.");
		}
		return;
	}

	if (*token == "skill") {
		delete token;

		token = o->gettoken();
		if (!token) {
			parse_error(pCheck, u, 0, "SHOW: Show what skill?");
			return;
		}
		sk = ParseSkill(token);
		delete token;

		token = o->gettoken();
		if (!token) {
			parse_error(pCheck, u, 0, "SHOW: No skill level given.");
			return;
		}
		lvl = token->value();
		delete token;

		if (!pCheck) {
			if (sk == -1 ||
					SkillDefs[sk].flags & SkillType::DISABLED ||
					(SkillDefs[sk].flags & SkillType::APPRENTICE &&
					                                 !Globals->APPRENTICES_EXIST) ||
					lvl > u->faction->skills.GetDays(sk)) {
				u->error("SHOW: Faction doesn't have that skill.");
				return;
			}

			u->faction->shows.push_back({ .skill = sk, .level = lvl });
		}
		return;
	}

	if (*token == "object") {
		delete token;
		token = o->gettoken();

		if (!token) {
			parse_error(pCheck, u, 0, "SHOW: Show which object?");
			return;
		}

		obj = ParseObject(token, 1);
		delete token;

		if (!pCheck && obj >= -1) {
			if (obj == -1 ||
					obj == O_DUMMY ||
					(ObjectDefs[obj].flags & ObjectType::DISABLED)) {
				u->error("SHOW: No such object.");
				return;
			}
			u->faction->objectshows.push_back({.obj = obj});
		}
		if (obj >= -1)
			return;
		token = new AString("item");
		o = new AString(ItemDefs[-(obj + 1)].abr);
	}

	if (*token == "item") {
		delete token;
		token = o->gettoken();

		if (!token) {
			parse_error(pCheck, u, 0, "SHOW: Show which item?");
			return;
		}

		item = ParseEnabledItem(token);
		delete token;

		if (!pCheck) {
			if (item == -1 || (ItemDefs[item].flags & ItemType::DISABLED)) {
				u->error("SHOW: You don't know anything about that item.");
				return;
			}
			if (ItemDefs[item].pSkill) {
				token = new AString(ItemDefs[item].pSkill);
				sk = LookupSkill(token);
				delete token;
				if (ItemDefs[item].pLevel <= u->faction->skills.GetDays(sk)) {
					u->faction->DiscoverItem(item, 1, 1);
					return;
				}
			}
			if (ItemDefs[item].mSkill) {
				token = new AString(ItemDefs[item].mSkill);
				sk = LookupSkill(token);
				delete token;
				if (ItemDefs[item].mLevel <= u->faction->skills.GetDays(sk)) {
					u->faction->DiscoverItem(item, 1, 1);
					return;
				}
			}
			if (u->faction->items.GetNum(item)) {
				u->faction->DiscoverItem(item, 1, 0);
				return;
			}
			if (ItemDefs[item].type & (IT_MAN | IT_NORMAL | IT_TRADE | IT_MONSTER)) {
				u->faction->DiscoverItem(item, 1, 0);
				return;
			}

			u->error("SHOW: You don't know anything about that item.");
		}
		return;
	}

	parse_error(pCheck, u, 0, "SHOW: Show what?");
}

void Game::ProcessForgetOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "FORGET: No skill given.");
		return;
	}

	int sk = ParseSkill(token);
	delete token;

	if (sk==-1) {
		parse_error(pCheck, u, 0, "FORGET: Invalid skill.");
		return;
	}

	if (!pCheck) {
		ForgetOrder *ord = new ForgetOrder;
		ord->skill = sk;
		u->forgetorders.push_back(ord);
	}
}

void Game::ProcessEntertainOrder(Unit *unit, OrdersCheck *pCheck)
{
	if (unit->monthorders ||
		(Globals->TAX_PILLAGE_MONTH_LONG && ((unit->taxing == TAX_TAX) || (unit->taxing == TAX_PILLAGE)))
	) {
		string err = string("ENTERTAIN: Overwriting previous ") +
			(unit->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, unit, 0, err);
		if (unit->monthorders) delete unit->monthorders;
	}
	ProduceOrder *o = new ProduceOrder;
	o->item = I_SILVER;
	o->skill = S_ENTERTAINMENT;
	o->target = 0;
	unit->monthorders = o;
}

void Game::ProcessCombatOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!token) {
		if (!pCheck) {
			u->combat = -1;
			u->event("Combat spell set to none.", "combat_preparation");
		}
		return;
	}
	int sk = ParseSkill(token);
	delete token;

	if (!pCheck) {
		if (sk==-1) {
			parse_error(pCheck, u, 0, "COMBAT: Invalid skill.");
			return;
		}
		if (!(SkillDefs[sk].flags & SkillType::MAGIC)) {
			parse_error(pCheck, u, 0, "COMBAT: That is not a magic skill.");
			return;
		}
		if (!(SkillDefs[sk].flags & SkillType::COMBAT)) {
			parse_error(pCheck, u, 0,
					"COMBAT: That skill cannot be used in combat.");
			return;
		}

		if (u->type != U_MAGE) {
			u->error("COMBAT: That unit is not a mage.");
			return;
		}
		if (!u->GetSkill(sk)) {
			u->error("COMBAT: Unit does not possess that skill.");
			return;
		}

		u->combat = sk;
		string temp = "Combat spell set to " + SkillDefs[sk].name;
		if (Globals->USE_PREPARE_COMMAND) {
			u->readyItem = -1;
			temp += " and prepared item set to none";
		}
		temp += ".";

		u->event(temp, "combat_preparation");
	}
}

// Lacandon's prepare command
void Game::ProcessPrepareOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (!(Globals->USE_PREPARE_COMMAND)) {
		parse_error(pCheck, u, 0, "PREPARE is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	if (!token) {
		if (!pCheck) {
			u->readyItem = -1;
			u->event("Prepared battle item set to none.", "combat_preparation");
		}
		return;
	}
	int it = ParseEnabledItem(token);
	BattleItemType *bt = FindBattleItem(token->Str());
	delete token;
	if (!pCheck) {
		string temp;
		if (it == -1 || !u->items.GetNum(it)) {
			u->error("PREPARE: Unit does not possess that item.");
			return;
		}

		if (bt == NULL || !(bt->flags & BattleItemType::SPECIAL)) {
			u->error("PREPARE: That item cannot be prepared.");
			return;
		}

		if ((bt->flags & BattleItemType::MAGEONLY) &&
			!((u->type == U_MAGE) || (u->type == U_APPRENTICE) || (u->type == U_GUARDMAGE))) {
			temp = string("PREPARE: Only a mage ") +
				(Globals->APPRENTICES_EXIST ? "or " + string(Globals->APPRENTICE_NAME) : "") +
				"may use that item.";
			u->error(temp);
			return;
		}
		u->readyItem = it;
		temp = "Prepared item set to " + ItemDefs[it].name;
		if (u->combat != -1) {
			u->combat = -1;
			temp += " and combat spell set to none";
		}
		temp += ".";
		u->event(temp, "combat_preparation");
	}
}

void Game::ProcessWeaponOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (!(Globals->USE_WEAPON_ARMOR_COMMAND)) {
		parse_error(pCheck, u, 0, "WEAPON is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	int i;
	if (!token) {
		if (!pCheck) {
			for (i = 0; i < MAX_READY; ++i) u->readyWeapon[i] = -1;
			u->event("Preferred weapons set to none.", "combat_preparation");
		}
		return;
	}
	if (pCheck) {
		delete token;
		return;
	}
	int it;
	int items[MAX_READY];
	i = 0;
	while (token && (i < MAX_READY)) {
		it = ParseEnabledItem(token);
		delete token;
		if (it == -1 || u->faction->items.GetNum(it) < 1) {
			u->error("WEAPON: Unknown item.");
		} else if (!(ItemDefs[it].type & IT_WEAPON)) {
			u->error("WEAPON: Item is not a weapon.");
		} else {
			if (!pCheck) items[i++] = it;
		}
		token = o->gettoken();
	}
	if (token) delete token;

	while (i < MAX_READY) {
		items[i++] = -1;
	}
	if (items[0] == -1) return;
	string temp = "Preferred weapons set to: ";
	for (i=0; i<MAX_READY;++i) {
		u->readyWeapon[i] = items[i];
		if (items[i] != -1) {
			if (i > 0) temp += ", ";
			temp += ItemDefs[items[i]].name;
		}
	}
	temp += ".";
	u->event(temp, "combat_preparation");
}

void Game::ProcessArmorOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (!(Globals->USE_WEAPON_ARMOR_COMMAND)) {
		parse_error(pCheck, u, 0, "ARMOR is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	int i;
	if (!token) {
		if (!pCheck) {
			for (i = 0; i < MAX_READY; ++i) u->readyArmor[i] = -1;
			u->event("Preferred armor set to none.", "combat_preparation");
		}
		return;
	}
	if (pCheck) {
		delete token;
		return;
	}
	int it;
	int items[MAX_READY];
	i = 0;
	while (token && (i < MAX_READY)) {
		it = ParseEnabledItem(token);
		delete token;
		if (it == -1 || u->faction->items.GetNum(it) < 1) {
			u->error("ARMOR: Unknown item.");
		} else if (!(ItemDefs[it].type & IT_ARMOR)) {
			u->error("ARMOR: Item is not armor.");
		} else {
			if (!pCheck) items[i++] = it;
		}
		token = o->gettoken();
	}
	if (token) delete token;

	while (i < MAX_READY) {
		items[i++] = -1;
	}
	if (items[0] == -1) return;
	string temp = "Preferred armor set to: ";
	for (i=0; i<MAX_READY;++i) {
		u->readyArmor[i] = items[i];
		if (items[i] != -1) {
			if (i > 0) temp += ", ";
			temp += ItemDefs[items[i]].name;
		}
	}
	temp += ".";
	u->event(temp, "combat_preparation");
}

void Game::ProcessClaimOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "CLAIM: No amount given.");
		return;
	}

	int value = token->value();
	delete token;
	if (!value) {
		parse_error(pCheck, u, 0, "CLAIM: No amount given.");
		return;
	}

	if (!pCheck) {
		if (value > u->faction->unclaimed) {
			u->error("CLAIM: Don't have that much unclaimed silver.");
			value = u->faction->unclaimed;
		}
		u->faction->unclaimed -= value;
		u->SetMoney(u->GetMoney() + value);
		u->faction->DiscoverItem(I_SILVER, 0, 1);
		u->event("Claims $" + to_string(value) + ".", "claim");
	}
}

void Game::ProcessFactionOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_FACTION_TYPES) {
		parse_error(pCheck, u, 0, "FACTION: Invalid order, no faction types in this game.");
		return;
	}

	std::unordered_map<std::string, int> oldfactype;
	std::unordered_map<std::string, int> factype;

	if (!pCheck) {
		// copy current values into temp variable
		oldfactype = u->faction->type;
	}

	int retval = ParseFactionType(o, factype);
	if (retval == -1) {
		parse_error(pCheck, u, 0, "FACTION: Bad faction type.");
		return;
	}

	if (!pCheck) {
		int m = CountMages(u->faction);
		int a = CountApprentices(u->faction);

		u->faction->type = factype;

		if (m > AllowedMages(u->faction)) {
			u->error("FACTION: Too many mages to change to that faction type.");
			u->faction->type = oldfactype;
			return;
		}

		if (a > AllowedApprentices(u->faction)) {
			u->error(string("FACTION: Too many ") + Globals->APPRENTICE_NAME + "s to change to that faction type.");
			u->faction->type = oldfactype;
			return;
		}

		if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			int q = CountQuarterMasters(u->faction);
			if ((Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) && (q > AllowedQuarterMasters(u->faction))) {
				u->error("FACTION: Too many quartermasters to change to that faction type.");
				u->faction->type = oldfactype;
				return;
			}
		}

		u->faction->lastchange = TurnNumber();
		u->faction->DefaultOrders();
	}
}

void Game::ProcessAssassinateOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	UnitId *id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		parse_error(pCheck, u, 0, "ASSASSINATE: No target given.");
		return;
	}
	if (!pCheck) {
		if (u->stealthorders) delete u->stealthorders;
		AssassinateOrder *ord = new AssassinateOrder;
		ord->target = id;
		u->stealthorders = ord;
	}
}

void Game::ProcessStealOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	UnitId *id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		parse_error(pCheck, u, 0, "STEAL: No target given.");
		return;
	}
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "STEAL: No item given.");
		delete id;
		return;
	}
	int i = ParseEnabledItem(token);
	delete token;
	if (!pCheck) {
		if (i == -1) {
			u->error("STEAL: Bad item given.");
			delete id;
			return;
		}

		if (IsSoldier(i)) {
			u->error("STEAL: Can't steal that.");
			delete id;
			return;
		}
		StealOrder *ord = new StealOrder;
		ord->target = id;
		ord->item = i;
		if (u->stealthorders) delete u->stealthorders;
		u->stealthorders = ord;
	}
}

void Game::ProcessQuitOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (!pCheck) {
		if (u->faction->password && !(*(u->faction->password) == "none")) {
			AString *token = o->gettoken();
			if (!token) {
				u->faction->error("QUIT: Must give the correct password.");
				return;
			}

			if (!(*token == *(u->faction->password))) {
				delete token;
				u->faction->error("QUIT: Must give the correct password.");
				return;
			}

			delete token;
		}

		if (u->faction->quit != QUIT_AND_RESTART) {
			u->faction->quit = QUIT_BY_ORDER;
		}
	}
}

void Game::ProcessRestartOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (!pCheck) {
		if (u->faction->password && !(*(u->faction->password) == "none")) {
			AString *token = o->gettoken();
			if (!token) {
				u->faction->error("RESTART: Must give the correct password.");
				return;
			}

			if (!(*token == *(u->faction->password))) {
				delete token;
				u->faction->error("RESTART: Must give the correct password.");
				return;
			}

			delete token;
		}

		if (u->faction->quit != QUIT_AND_RESTART) {
			u->faction->quit = QUIT_AND_RESTART;
			Faction *pFac = AddFaction(0, NULL);
			pFac->SetAddress(*(u->faction->address));
			AString *pass = new AString(*(u->faction->password));
			pFac->password = pass;
			string facstr = string("Restarting ") + pFac->address->const_str() + ".";
		}
	}
}

void Game::ProcessDestroyOrder(Unit *u, OrdersCheck *pCheck)
{
	if (!pCheck) {
		u->destroy = 1;
	}
}

void Game::ProcessFindOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
		parse_error(pCheck, u, 0, "FIND: This command was disabled.");
		return;
	}
	if (!token) {
		parse_error(pCheck, u, 0, "FIND: No faction number given.");
		return;
	}
	int n = token->value();
	int is_all = (*token == "all");
	delete token;
	if (n==0 && !is_all) {
		parse_error(pCheck, u, 0, "FIND: No faction number given.");
		return;
	}
	if (!pCheck) {
		FindOrder *f = new FindOrder;
		f->find = n;
		u->findorders.push_back(f);
	}
}

void Game::ProcessConsumeOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (token) {
		if (*token == "unit") {
			if (!pCheck) {
				u->SetFlag(FLAG_CONSUMING_UNIT, 1);
				u->SetFlag(FLAG_CONSUMING_FACTION, 0);
			}
			delete token;
			return;
		}

		if (*token == "faction") {
			if (!pCheck) {
				u->SetFlag(FLAG_CONSUMING_UNIT, 0);
				u->SetFlag(FLAG_CONSUMING_FACTION, 1);
			}
			delete token;
			return;
		}

		if (*token == "none") {
			if (!pCheck) {
				u->SetFlag(FLAG_CONSUMING_UNIT, 0);
				u->SetFlag(FLAG_CONSUMING_FACTION, 0);
			}
			delete token;
			return;
		}

		delete token;
		parse_error(pCheck, u, 0, "CONSUME: Invalid value.");
	} else {
		if (!pCheck) {
			u->SetFlag(FLAG_CONSUMING_UNIT, 0);
			u->SetFlag(FLAG_CONSUMING_FACTION, 0);
		}
	}
}

void Game::ProcessRevealOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
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
		parse_error(pCheck, u, 0, "REVEAL: Invalid value.");
	} else {
		u->reveal = REVEAL_NONE;
	}
}

void Game::ProcessTaxOrder(Unit *u, OrdersCheck *pCheck)
{
	if (u->taxing == TAX_PILLAGE) {
		parse_error(pCheck, u, 0, "TAX: The unit is already pillaging.");
		return;
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG && u->monthorders) {
		delete u->monthorders;
		u->monthorders = nullptr;
		string err = string("TAX: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
	}

	u->taxing = TAX_TAX;
}

void Game::ProcessPillageOrder(Unit *u, OrdersCheck *pCheck)
{
	if (u->taxing == TAX_TAX) {
		parse_error(pCheck, u, 0, "PILLAGE: The unit is already taxing.");
		return;
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG && u->monthorders) {
		delete u->monthorders;
		u->monthorders = nullptr;
		string err = string("PILLAGE: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
	}

	u->taxing = TAX_PILLAGE;
}

void Game::ProcessPromoteOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	UnitId *id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		parse_error(pCheck, u, 0, "PROMOTE: No target given.");
		return;
	}
	if (!pCheck) {
		if (u->promote) {
			delete u->promote;
		}
		u->promote = id;
	}
}

void Game::ProcessLeaveOrder(Unit *u, OrdersCheck *pCheck)
{
	if (!pCheck) {
		// if the unit isn't already trying to enter a building,
		// then set it to leave.
		if (u->enter == 0) u->enter = -1;
	}
}

void Game::ProcessEnterOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token;
	int i = 0;

	token = o->gettoken();
	if (token) {
		i = token->value();
		delete token;
	}
	if (i) {
		if (!pCheck) {
			u->enter = i;
		}
	} else {
		parse_error(pCheck, u, 0, "ENTER: No object specified.");
	}
}

void Game::ProcessBuildOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	AString * token = o->gettoken();
	BuildOrder * order = new BuildOrder;
	int maxbuild;

	// 'incomplete' for ships:
	maxbuild = 0;
	unit->build = 0;

	if (token && !(*token == "complete")) {
		if (*token == "help") {
			// "build help unitnum"
			UnitId *targ = 0;
			delete token;
			if (!pCheck) {
				targ = ParseUnit(o);
				if (!targ) {
					unit->error("BUILD: Non-existent unit to help.");
					return;
				}
				if (targ->unitnum == -1) {
					unit->error("BUILD: Non-existent unit to help.");
					return;
				}
			}
			order->target = targ;	// set the order's target to the unit number helped
			// see if they added complete
			token = o->gettoken();
			if (token && *token == "complete") {
				delete token;
				if (!pCheck) {
					order->until_complete = true; // stay until complete
				}
			} else if (token) {
				parse_error(pCheck, unit, 0, "BUILD: Unknown keyword '" + string(token->Str()) + "'.");
				delete token;
				return;
			}
		} else {
			// token exists and != "help": must be something like 'build tower'
			int ot = ParseObject(token, 1);
			delete token;
			if (ot == -1) {
				parse_error(pCheck, unit, 0, "BUILD: Not a valid object name.");
				return;
			}

			if (!pCheck) {
				ARegion *reg = unit->object->region;
				if (TerrainDefs[reg->type].similar_type == R_OCEAN){
					unit->error("BUILD: Can't build in an ocean.");
					return;
				}

				if (ot < 0) {
					/* Build SHIP item */
					int st = abs(ot+1);
					if (ItemDefs[st].flags & ItemType::DISABLED) {
						parse_error(pCheck, unit, 0, "BUILD: Not a valid object name.");
						return;
					}
					int flying = ItemDefs[st].fly;
					if (!reg->IsCoastalOrLakeside() && (flying <= 0)) {
						unit->error("BUILD: Can't build ship in non-coastal or lakeside region.");
						return;
					}
					unit->build = -st;
					maxbuild = ItemDefs[st].pMonths;
					// if we already have an unfinished
					// ship, see how much work is left
					if (unit->items.GetNum(st) > 0)
						maxbuild = unit->items.GetNum(st);
					// Don't create a fleet yet
				} else {
					/* build standard OBJECT */
					if (ObjectDefs[ot].flags & ObjectType::DISABLED) {
						parse_error(pCheck, unit, 0, "BUILD: Not a valid object name.");
						return;
					}
					if (!(ObjectDefs[ot].flags & ObjectType::CANENTER)) {
						parse_error(pCheck, unit, 0, "BUILD: Can't build that.");
						return;
					}
					AString skname = ObjectDefs[ot].skill;
					int sk = LookupSkill(&skname);
					if (sk == -1) {
						parse_error(pCheck, unit, 0, "BUILD: Can't build that.");
						return;
					}
					order->new_building = ot;
				}
			}
			order->target = NULL; // Not helping anyone...
			// Check for 'COMPLETE' keyword to stay in current building until completion
			token = o->gettoken();
			if (token && *token == "complete") {
				delete token;
				if (!pCheck) {
					order->until_complete = true;
				}
			} else if (token) {
				parse_error(pCheck, unit, 0, "BUILD: Unknown keyword '" + string(token->Str()) + "'.");
				delete token;
				return;
			}
		}
	} else {
		// just a 'build' (or 'build complete') order
		order->target = NULL;
		if (!pCheck) {
			// look for an incomplete ship type in inventory
			int st = O_DUMMY;
			for(auto it : unit->items) {
				if ((ItemDefs[it->type].type & IT_SHIP) && (!(ItemDefs[it->type].flags & ItemType::DISABLED))) {
					st = -(it->type);
					break;
				}
			}

			if (st == O_DUMMY) {
				// Build whatever we happen to be in when
				// we get to the build phase
				unit->build = 0;
			} else {
				unit->build = st;
				maxbuild = unit->items.GetNum(-st);
			}
		}
		token = o->gettoken();
		if (token && *token == "complete") {
			// 'build complete' order
			delete token;
			if (!pCheck) {
				order->until_complete = true; // stay until complete
			}
		} else if (token) {
			parse_error(pCheck, unit, 0, "BUILD: Unknown keyword '" + string(token->Str()) + "'.");
			delete token;
			return;
		}
	}

	// set needtocomplete
	if (maxbuild != 0) order->needtocomplete = maxbuild;

	// Now do all of the generic bits...
	// Check that the unit isn't doing anything else important
	if (unit->monthorders ||
		(Globals->TAX_PILLAGE_MONTH_LONG && ((unit->taxing == TAX_TAX) || (unit->taxing == TAX_PILLAGE)))
	) {
		if (unit->monthorders) delete unit->monthorders;
		string err = string("BUILD: Overwriting previous ") + (unit->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, unit, 0, err);
	}

	// reset their taxation status if taxing is a month-long order
	if (Globals->TAX_PILLAGE_MONTH_LONG) unit->taxing = TAX_NONE;
	unit->monthorders = order;
}

void Game::ProcessAttackOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	UnitId *id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		if (!pCheck) {
			if (!u->attackorders) u->attackorders = new AttackOrder;
			u->attackorders->targets.push_back(id);
		}
		id = ParseUnit(o);
	}
}

void Game::ProcessSellOrder(Unit *u, AString *o, OrdersCheck *pCheck, bool repeating)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "SELL: Number to sell not given.");
		return;
	}
	int num = 0;
	if (*token == "ALL") {
		num = -1;
	} else {
		num = token->value();
	}
	delete token;
	if (!num) {
		parse_error(pCheck, u, 0, "SELL: Number to sell not given.");
		return;
	}
	token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "SELL: Item not given.");
		return;
	}
	int it = ParseGiveableItem(token);
	delete token;

	// Check if we already have an order for this type of item.  If we do, merge with it.
	// We will not merge repeating orders with non-repeating, but will merge all orders of
	// the same type for the same item.  The reason for this is due to how demand is
	// processed.
	// Currently, if 1 unit issues 100 orders of `SELL 1 <item>` in a hex and another unit
	// issues a single `SELL 100 <item>`, each of the first units 100 orders would have a
	// chance to succeed, while the second unit's orders would a) be capped at the max
	// available, and b) only be checked once. Because the number of orders aren't capped,
	// each one counts 1 toward the total demand. The other order being capped at the
	// max available only counts as that max amount toward demand, so the odds get skewed
	// in favor of the first unit winning more sells than their fair share.
	// By merging here, both units would have the same max cap, and each should get half of
	// available supply rather than unit 1 likely ending up with all of it.
	if (!pCheck) {
		for (const auto s : u->sellorders) {
			if (s->item == it && s->repeating == repeating) {
				// Found a matching order.  Merge it.
				if (s->num == -1 || num == -1) {
					s->num = -1;
				} else {
					s->num += num;
				}
				return;
			}
		}

		// we didn't find one above, so make a new one.
		SellOrder *s = new SellOrder;
		s->item = it;
		s->num = num;
		s->repeating = repeating;
		u->sellorders.push_back(s);
	}
}

void Game::ProcessBuyOrder(Unit *u, AString *o, OrdersCheck *pCheck, bool repeating)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "BUY: Number to buy not given.");
		return;
	}
	int num = 0;
	if (*token == "ALL") {
		num = -1;
	} else {
		num = token->value();
	}
	delete token;
	if (!num) {
		parse_error(pCheck, u, 0, "BUY: Number to buy not given.");
		return;
	}
	token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "BUY: Item not given.");
		return;
	}
	int it = ParseGiveableItem(token);
	if (it == -1) {
		if (*token == "peasant" || *token == "peasants" || *token == "peas") {
			if (pCheck) {
				it = -1;
				for (int i = 0; i < NITEMS; i++) {
					if (ItemDefs[i].flags & ItemType::DISABLED) continue;
					if (ItemDefs[i].type & IT_LEADER) continue;
					if (ItemDefs[i].type & IT_MAN) {
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

	// Check if we already have an order for this type of item.  If we do, merge with it.
	// We will not merge repeating orders with non-repeating, but will merge all orders of
	// the same type for the same item.  The reason for this is due to how demand is
	// processed.
	// Currently, if 1 unit issues 100 orders of `BUY 1 <item>` in a hex and another unit
	// issues a single `BUY 100 <item>`, each of the first units 100 orders would have a
	// chance to succeed, while the second unit's orders would a) be capped at the max
	// available, and b) only be checked once. Because the number of orders aren't capped,
	// each one counts 1 toward the total demand. The other order being capped at the
	// max available only counts as that max amount toward demand, so the odds get skewed
	// in favor of the first unit winning more buys than their fair share.
	// By merging here, both units would have the same max cap, and each should get half of
	// available supply rather than unit 1 likely ending up with all of it.
	if (!pCheck) {
		for (const auto b : u->buyorders) {
			if (b->item == it && b->repeating == repeating) {
				// Found a matching order.  Merge it.
				if (b->num == -1 || num == -1) {
					b->num = -1;
				} else {
					b->num += num;
				}
				return;
			}
		}

		// we didn't find one above, so make a new one.
		BuyOrder *b = new BuyOrder;
		b->item = it;
		b->num = num;
		b->repeating = repeating;
		u->buyorders.push_back(b);
	}
}

void Game::ProcessProduceOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	int target = 0;
	AString *token = o->gettoken();

	if (token && token->value() > 0)
	{
		target = token->value();
		token = o->gettoken();
	}
	if (!token) {
		parse_error(pCheck, u, 0, "PRODUCE: No item given.");
		return;
	}
	const int it = ParseEnabledItem(token);
	delete token;

	if (it == -1) {
		parse_error(pCheck, u, 0, "PRODUCE: Invalid item.");
		return;
	}
	const ItemType &item_def = ItemDefs[it];

	if (item_def.type == IT_SHIP ) {
		parse_error(pCheck, u, 0, "PRODUCE: Use BUILD for ships.");
		return;
	}

	ProduceOrder *p = new ProduceOrder;
	p->item = it;
	AString skname = item_def.pSkill;
	p->skill = LookupSkill(&skname);
	p->target = target;

	if (u->monthorders || (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE))) {
		if (u->monthorders) delete u->monthorders;
		string err = string("PRODUCE: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = p;
}

void Game::ProcessWorkOrder(Unit *u, int quiet, OrdersCheck *pCheck)
{
	ProduceOrder *order = new ProduceOrder;
	order->skill = -1;
	order->item = I_SILVER;
	order->target = 0;
	if (quiet) order->quiet = 1;
	if (u->monthorders || (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE))) {
		if (u->monthorders) delete u->monthorders;
		string err = string("WORK: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = order;
}

void Game::ProcessTeachOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	TeachOrder *order = 0;

	if (u->monthorders && u->monthorders->type == O_TEACH) {
		order = dynamic_cast<TeachOrder *>(u->monthorders);
	} else {
		order = new TeachOrder;
	}

	int students = 0;
	UnitId *id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		students++;
		if (order) {
			order->targets.push_back(id);
		}
		id = ParseUnit(o);
	}

	if (!students) {
		parse_error(pCheck, u, 0, "TEACH: No students given.");
		return;
	}

	if ((u->monthorders && u->monthorders->type != O_TEACH) ||
		(Globals->TAX_PILLAGE_MONTH_LONG && ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))
	) {
		if (u->monthorders) delete u->monthorders;
		string err = string("TEACH: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = order;
}

void Game::ProcessStudyOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "STUDY: No skill given.");
		return;
	}
	int sk = ParseSkill(token);
	delete token;

	StudyOrder *order = new StudyOrder;
	order->skill = sk;
	order->days = 0;
	// parse study level:
	token = o->gettoken();
	if (token) {
		order->level = token->value();
		delete token;
	} else
		order->level = -1;

	if (u->monthorders || (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE))) {
		if (u->monthorders) delete u->monthorders;
		string err = string("STUDY: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = order;
}

void Game::ProcessDeclareOrder(Faction *f, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, 0, f, "DECLARE: No faction given.");
		return;
	}
	int fac;
	if (*token == "default") {
		fac = -1;
	} else {
		fac = token->strict_value();
		if (fac == -1) {
			parse_error(pCheck, 0, f, "DECLARE: Non-existent faction.");
			return;
		}
	}
	delete token;

	if (!pCheck) {
		Faction *target;
		if (fac != -1) {
			target = GetFaction(factions, fac);
			if (!target) {
				f->error("DECLARE: Non-existent faction " + to_string(fac) + ".");
				return;
			}
			if (target == f) {
				f->error("DECLARE: Can't declare towards your own faction.");
				return;
			}
		}
	}

	token = o->gettoken();
	if (!token) {
		if (fac != -1) {
			if (!pCheck) {
				f->remove_attitude(fac);
			}
		}
		return;
	}

	int att = ParseAttitude(token);
	delete token;
	if (att == -1) {
		parse_error(pCheck, 0, f, "DECLARE: Invalid attitude.");
		return;
	}

	if (!pCheck) {
		if (fac == -1) {
			f->defaultattitude = att;
		} else {
			f->set_attitude(fac, att);
		}
	}
}

void Game::ProcessWithdrawOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	if (!(Globals->ALLOW_WITHDRAW)) {
		parse_error(pCheck, unit, 0, "WITHDRAW is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	if (!token) {
		parse_error (pCheck, unit, 0, "WITHDRAW: No amount given.");
		return;
	}
	int amt = token->value();
	if (amt < 1) {
		amt = 1;
	} else {
		delete token;
		token = o->gettoken();
		if (!token) {
			parse_error(pCheck, unit, 0, "WITHDRAW: No item given.");
			return;
		}
	}
	int item = ParseGiveableItem(token);
	delete token;

	if (item == -1) {
		parse_error(pCheck, unit, 0, "WITHDRAW: Invalid item.");
		return;
	}
	if (ItemDefs[item].flags & ItemType::DISABLED) {
		parse_error(pCheck, unit, 0, "WITHDRAW: Invalid item.");
		return;
	}
	if (!(ItemDefs[item].type & IT_NORMAL)) {
		parse_error(pCheck, unit, 0, "WITHDRAW: Invalid item.");
		return;
	}
	if (item == I_SILVER) {
		parse_error(pCheck, unit, 0, "WITHDRAW: Invalid item.");
		return;
	}

	if (!pCheck) {
		WithdrawOrder *order = new WithdrawOrder;
		order->item = item;
		order->amount = amt;
		unit->withdraworders.push_back(order);
	}
	return;
}

AString *Game::ProcessTurnOrder(Unit *unit, istream& f, OrdersCheck *pCheck, bool repeat)
{
	int turnDepth = 1;
	int turnLast = 1;
	int formDepth = 0;
	TurnOrder *tOrder = new TurnOrder;
	tOrder->repeating = repeat;

	AString order, *token;

	while (turnDepth) {
		// get the next line
		f >> ws >> order;
		if (f.eof()) {
			// Fake end of commands to invoke appropriate processing
			order = AString("#end");
		}

		AString	saveorder = order;
		// In order to allow @endturn to work the same as endturn we need to check for and eat the possible @
		std::ignore = order.getat(); // we don't care about whether it was set or not, so just ignore the return value
		token = order.gettoken();

		if (token) {
			int i = Parse1Order(token);
			switch (i) {
				case O_TURN:
					if (turnLast) {
						parse_error(pCheck, unit, 0, "TURN: cannot nest.");
						break;
					}
					turnDepth++;
					tOrder->turnOrders.push_back(saveorder.const_str());
					turnLast = 1;
					break;
				case O_FORM:
					if (!turnLast) {
						parse_error(pCheck, unit, 0, "FORM: cannot nest.");
						break;
					}
					turnLast = 0;
					formDepth++;
					tOrder->turnOrders.push_back(saveorder.const_str());
					break;
				case O_ENDFORM:
					if (turnLast) {
						if (!(formDepth + (unit->former != 0))) {
							parse_error(pCheck, unit, 0, "END: without FORM.");
							break;
						} else {
							parse_error(pCheck, unit, 0, "TURN: without ENDTURN.");
							if (!--turnDepth) {
								unit->turnorders.push_back(tOrder);
								return new AString(saveorder);
							}
						}
					}
					formDepth--;
					tOrder->turnOrders.push_back(saveorder.const_str());
					turnLast = 1;
					break;
				case O_UNIT:
				case O_END:
					if (!turnLast)
						parse_error(pCheck, unit, 0, "FORM: without END.");
					while (--turnDepth) {
						parse_error(pCheck, unit, 0, "TURN: without ENDTURN.");
						parse_error(pCheck, unit, 0, "FORM: without END.");
					}
					parse_error(pCheck, unit, 0, "TURN: without ENDTURN.");
					unit->turnorders.push_back(tOrder);
					return new AString(saveorder);
					break;
				case O_ENDTURN:
					if (!turnLast) {
						parse_error(pCheck, unit, 0, "ENDTURN: without TURN.");
					} else {
						if (--turnDepth)
							tOrder->turnOrders.push_back(saveorder.const_str());
						turnLast = 0;
					}
					break;
				default:
					tOrder->turnOrders.push_back(saveorder.const_str());
					break;
			}
			if (!pCheck && unit->former && unit->former->form_repeated)
				unit->former->oldorders.push_back(saveorder.const_str());
			delete token;
		}
	}

	unit->turnorders.push_back(tOrder);
	return NULL;
}

void Game::ProcessExchangeOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	UnitId *t = ParseUnit(o);
	if (!t) {
		parse_error(pCheck, unit, 0, "EXCHANGE: Invalid target.");
		return;
	}
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, unit, 0, "EXCHANGE: No amount given.");
		return;
	}
	int amtGive;
	amtGive = token->value();
	delete token;

	if (amtGive < 0) {
		parse_error(pCheck, unit, 0, "EXCHANGE: Illegal amount given.");
		return;
	}

	token = o->gettoken();
	if (!token) {
		parse_error(pCheck, unit, 0, "EXCHANGE: No item given.");
		return;
	}
	int itemGive;
	itemGive = ParseGiveableItem(token);
	delete token;

	if (itemGive == -1) {
		parse_error(pCheck, unit, 0, "EXCHANGE: Invalid item.");
		return;
	}
	if (ItemDefs[itemGive].type & IT_SHIP) {
		parse_error(pCheck, unit, 0, "EXCHANGE: Can't exchange ships.");
		return;
	}

	token = o->gettoken();
	if (!token) {
		parse_error(pCheck, unit, 0, "EXCHANGE: No amount expected.");
		return;
	}
	int amtExpected;
	amtExpected = token->value();
	delete token;

	if (amtExpected < 0) {
		parse_error(pCheck, unit, 0, "EXCHANGE: Illegal amount given.");
		return;
	}

	token = o->gettoken();
	if (!token) {
		parse_error(pCheck, unit, 0, "EXCHANGE: No item expected.");
		return;
	}
	int itemExpected;
	itemExpected = ParseGiveableItem(token);
	delete token;

	if (itemExpected == -1) {
		parse_error(pCheck, unit, 0, "EXCHANGE: Invalid item.");
		return;
	}
	if (ItemDefs[itemExpected].type & IT_SHIP) {
		parse_error(pCheck, unit, 0, "EXCHANGE: Can't exchange ships.");
		return;
	}

	if (!pCheck) {
		ExchangeOrder *order = new ExchangeOrder;
		order->giveItem = itemGive;
		order->giveAmount = amtGive;
		order->expectAmount = amtExpected;
		order->expectItem = itemExpected;
		order->target = t;
		unit->exchangeorders.push_back(order);
	}
}

void Game::ProcessGiveOrder(int order, Unit *unit, AString *o, OrdersCheck *pCheck)
{
	UnitId *t;
	AString *token;
	int unfinished, amt, item, excpt;

	string ord = (order == O_GIVE) ? "GIVE" : "TAKE";

	if (order == O_TAKE) {
		token = o->gettoken();
		if (!token || !(*token == "from")) {
			parse_error(pCheck, unit, 0, ord + ": Missing FROM.");
			return;
		}
	}

	t = ParseUnit(o);
	if (!t) {
		parse_error(pCheck, unit, 0, ord + ": Invalid target.");
		return;
	}
	if (t->unitnum == -1 && order == O_TAKE) {
		parse_error(pCheck, unit, 0, ord + ": Invalid target.");
		return;
	}
	token = o->gettoken();
	if (!token) {
		parse_error(pCheck, unit, 0, ord + ": No amount given.");
		return;
	}
	if (*token == "unit" && order == O_GIVE) {
		amt = -1;
	} else if (*token == "all") {
		amt = -2;
	} else {
		amt = token->value();
		if (amt < 1) {
			parse_error(pCheck, unit, 0, ord + ": Illegal amount given.");
			return;
		}
	}
	delete token;
	item = -1;
	unfinished = 0;
	if (amt != -1) {
		token = o->gettoken();
		if (token && *token == "unfinished") {
			unfinished = 1;
			token = o->gettoken();
		}
		if (token) {
			if (t->unitnum == -1)
				item = ParseEnabledItem(token);
			else
				item = ParseGiveableItem(token);
			if (amt == -2) {
				int found = 0;
				if (*token == "normal") {
					item = -IT_NORMAL;
					found = 1;
				} else if (*token == "advanced") {
					item = -IT_ADVANCED;
					found = 1;
				} else if (*token == "trade") {
					item = -IT_TRADE;
					found = 1;
				} else if ((*token == "man") || (*token == "men")) {
					item = -IT_MAN;
					found = 1;
				} else if ((*token == "monster") || (*token == "monsters")) {
					item = -IT_MONSTER;
					found = 1;
				} else if (*token == "magic") {
					item = -IT_MAGIC;
					found = 1;
				} else if ((*token == "weapon") || (*token == "weapons")) {
					item = -IT_WEAPON;
					found = 1;
				} else if (*token == "armor") {
					item = -IT_ARMOR;
					found = 1;
				} else if ((*token == "mount") || (*token == "mounts")) {
					item = -IT_MOUNT;
					found = 1;
				} else if (*token == "battle") {
					item = -IT_BATTLE;
					found = 1;
				} else if (*token == "special") {
					item = -IT_SPECIAL;
					found = 1;
				} else if (*token == "food") {
					item = -IT_FOOD;
					found = 1;
				} else if ((*token == "tool") || (*token == "tools")) {
					item = -IT_TOOL;
					found = 1;
				} else if ((*token == "item") || (*token == "items")) {
					item = -NITEMS;
					found = 1;
				} else if ((*token == "ship") || (*token == "ships")) {
					item = -IT_SHIP;
					found = 1;
				} else if (item != -1) {
					found = 1;
				}
				if (!found) {
					parse_error(pCheck, unit, 0,
							ord + ": Invalid item or item class.");
					return;
				}
			} else if (item == -1) {
				parse_error(pCheck, unit, 0, ord + ": Invalid item.");
				return;
			}
			if (unfinished &&
					item != -IT_SHIP &&
					item != -NITEMS &&
					!(item >= 0 &&
					ItemDefs[item].type & IT_SHIP)) {
				parse_error(pCheck, unit, 0, ord + ": That item does not have an unfinished version.");
				return;
			}
		} else {
			parse_error(pCheck, unit, 0, ord + ": No item given.");
			return;
		}
		delete token;
	}

	token = o->gettoken();
	excpt = 0;
	if (token && *token == "except") {
		if (amt == -2) {
			delete token;
			if (item < 0) {
				parse_error(pCheck, unit, 0,
						ord + ": EXCEPT only valid with specific items.");
				return;
			}
			token = o->gettoken();
			if (!token) {
				parse_error(pCheck, unit, 0, ord + ": EXCEPT requires a value.");
				return;
			}
			excpt = token->value();
			if (excpt <= 0) {
				parse_error(pCheck, unit, 0, ord + ": Invalid EXCEPT value.");
				return;
			}
		} else {
			parse_error(pCheck, unit, 0, ord + ": EXCEPT only valid with ALL");
			return;
		}
		delete token;
	}

	if (!pCheck) {
		GiveOrder *go = new GiveOrder;
		go->type = order;
		go->item = item;
		go->target = t;
		go->amount = amt;
		go->except = excpt;
		go->unfinished = unfinished;
		unit->giveorders.push_back(go);
	}
	return;
}

void Game::ProcessDescribeOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, unit, 0, "DESCRIBE: No argument.");
		return;
	}
	if (*token == "unit") {
		delete token;
		token = o->gettoken();
		if (!pCheck) {
			unit->SetDescribe(token);
		}
		return;
	}
	if (*token == "ship" || *token == "building" || *token == "object" ||
		*token == "structure") {
		delete token;
		token = o->gettoken();
		if (!pCheck) {
			// ALT, 25-Jul-2000
			// Fix to prevent non-owner units from describing objects
			if (unit != unit->object->GetOwner()) {
				unit->error("DESCRIBE: Unit is not owner.");
				return;
			}
			unit->object->SetDescribe(token);
		}
		return;
	}
	parse_error(pCheck, unit, 0, "DESCRIBE: Can't describe that.");
}

void Game::ProcessNameOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, unit, 0, "NAME: No argument.");
		return;
	}

	if (*token == "faction") {
		delete token;
		token = o->gettoken();
		if (!token) {
			parse_error(pCheck, unit, 0, "NAME: No name given.");
			return;
		}
		if (!pCheck) {
			unit->faction->SetName(token);
		}
		return;
	}

	if (*token == "unit") {
		delete token;
		token = o->gettoken();
		if (!token) {
			parse_error(pCheck, unit, 0, "NAME: No name given.");
			return;
		}
		if (!pCheck) {
			unit->SetName(token);
		}
		return;
	}

	if (*token == "building" || *token == "ship" || *token == "object" ||
		*token == "structure") {
		delete token;
		token = o->gettoken();
		if (!token) {
			parse_error(pCheck, unit, 0, "NAME: No name given.");
			return;
		}
		if (!pCheck) {
			// ALT, 25-Jul-2000
			// Fix to prevent non-owner units from renaming objects
			if (unit != unit->object->GetOwner()) {
				unit->error("NAME: Unit is not owner.");
				return;
			}
			if (!unit->object->CanModify()) {
				unit->error("NAME: Can't name this type of object.");
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
			parse_error(pCheck, unit, 0, "NAME: No name given.");
			return;
		}

		if (!pCheck) {
			if (!unit->object) {
				unit->error("NAME: Unit is not in a structure.");
				return;
			}
			if (!unit->object->region->town) {
				unit->error("NAME: Unit is not in a village, town or city.");
				return;
			}
			int cost = 0;
			int towntype = unit->object->region->town->TownType();
			string tstring = (towntype == TOWN_VILLAGE) ? "village" : ((towntype == TOWN_TOWN) ? "town" : "city");
			if (Globals->CITY_RENAME_COST) {
				cost = (towntype+1)* Globals->CITY_RENAME_COST;
			}
			int ok = 0;
			switch(towntype) {
				case TOWN_VILLAGE:
					if (unit->object->type == O_TOWER) ok = 1;
					if (unit->object->type == O_MTOWER) ok = 1;
				case TOWN_TOWN:
					if (unit->object->type == O_FORT) ok = 1;
					if (unit->object->type == O_MFORTRESS) ok = 1;
				case TOWN_CITY:
					if (unit->object->type == O_CASTLE) ok = 1;
					if (unit->object->type == O_CITADEL) ok = 1;
					if (unit->object->type == O_MCASTLE) ok = 1;
					if (unit->object->type == O_MCITADEL) ok = 1;
			}
			if (!ok) {
				unit->error("NAME: Unit is not in a large enough structure to rename a " + tstring + ".");
				return;
			}
			if (unit != unit->object->GetOwner()) {
				unit->error("NAME: Cannot name " + tstring + ".  Unit is not the owner of object.");
				return;
			}
			if (unit->object->incomplete > 0) {
				unit->error("NAME: Cannot name " + tstring + ".  Object is not finished.");
				return;
			}

			AString *newname = token->getlegal();
			if (!newname) {
				unit->error("NAME: Illegal name.");
				return;
			}
			if (cost) {
				int silver = unit->items.GetNum(I_SILVER);
				if (silver < cost) {
					unit->error("NAME: Unit doesn't have enough silver to rename a " + tstring + ".");
					return;
				}
				unit->items.SetNum(I_SILVER, silver-cost);
			}

			unit->event("Renames " + string(unit->object->region->town->name->const_str()) + " to " +
					newname->const_str() + ".", "rename");
			unit->object->region->NotifyCity(unit,
					*(unit->object->region->town->name), *newname);
			delete unit->object->region->town->name;
			unit->object->region->town->name = newname;
		}
		return;
	}

	delete token;
	parse_error(pCheck, unit, 0, "NAME: Can't name that.");
}

void Game::ProcessGuardOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "GUARD: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		parse_error(pCheck, u, 0, "GUARD: Invalid value.");
		return;
	}
	if (!pCheck) {
		if (val==0) {
			if (u->guard != GUARD_AVOID)
				u->guard = GUARD_NONE;
		} else {
			if (u->guard != GUARD_GUARD)
				u->guard = GUARD_SET;
		}
	}
}

void Game::ProcessBehindOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "BEHIND: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	if (val == -1) {
		parse_error(pCheck, u, 0, "BEHIND: Invalid value.");
		return;
	}
	if (!pCheck) {
		u->SetFlag(FLAG_BEHIND, val);
	}
}

void Game::ProcessNoaidOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "NOAID: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		parse_error(pCheck, u, 0, "NOAID: Invalid value.");
		return;
	}
	if (!pCheck) {
		u->SetFlag(FLAG_NOAID, val);
	}
}

void Game::ProcessSpoilsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	int flag = 0;
	int val = 1;
	if (token) {
		if (*token == "none") flag = FLAG_NOSPOILS;
		else if (*token == "walk") flag = FLAG_WALKSPOILS;
		else if (*token == "ride") flag = FLAG_RIDESPOILS;
		else if (*token == "fly") flag = FLAG_FLYSPOILS;
		else if (*token == "swim") flag = FLAG_SWIMSPOILS;
		else if (*token == "sail") flag = FLAG_SAILSPOILS;
		else if (*token == "all") val = 0;
		else parse_error(pCheck, u, 0, "SPOILS: Bad argument.");
		delete token;
	}

	if (!pCheck) {
		/* Clear all the flags */
		u->SetFlag(FLAG_NOSPOILS, 0);
		u->SetFlag(FLAG_WALKSPOILS, 0);
		u->SetFlag(FLAG_RIDESPOILS, 0);
		u->SetFlag(FLAG_FLYSPOILS, 0);
		u->SetFlag(FLAG_SWIMSPOILS, 0);
		u->SetFlag(FLAG_SAILSPOILS, 0);

		/* Set the flag we're trying to set */
		if (flag) {
			u->SetFlag(flag, val);
		}
	}
}

void Game::ProcessNospoilsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	parse_error(pCheck, u, 0, "NOSPOILS: This command is deprecated.  Use the 'SPOILS' command instead");

	/* Instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "NOSPOILS: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		parse_error(pCheck, u, 0, "NOSPOILS: Invalid value.");
		return;
	}
	if (!pCheck) {
		u->SetFlag(FLAG_FLYSPOILS, 0);
		u->SetFlag(FLAG_RIDESPOILS, 0);
		u->SetFlag(FLAG_WALKSPOILS, 0);
		u->SetFlag(FLAG_NOSPOILS, val);
	}
}

void Game::ProcessNocrossOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	int move_over_water = 0;

	if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE)
		move_over_water = 1;
	if (!move_over_water) {
		int i;
		for (i = 0; i < NITEMS; i++) {
			if (ItemDefs[i].flags & ItemType::DISABLED) continue;
			if (ItemDefs[i].swim > 0) move_over_water = 1;
		}
	}
	if (!move_over_water) {
		parse_error(pCheck, u, 0, "NOCROSS is not a valid order.");
		return;
	}

	/* Instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "NOCROSS: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		parse_error(pCheck, u, 0, "NOCROSS: Invalid value.");
		return;
	}
	if (!pCheck) {
		u->SetFlag(FLAG_NOCROSS_WATER, val);
	}
}

void Game::ProcessHoldOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "HOLD: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		parse_error(pCheck, u, 0, "HOLD: Invalid value.");
		return;
	}
	if (!pCheck) {
		u->SetFlag(FLAG_HOLDING, val);
	}
}

void Game::ProcessAutoTaxOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "AUTOTAX: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		parse_error(pCheck, u, 0, "AUTOTAX: Invalid value.");
		return;
	}
	if (!pCheck) {
		u->SetFlag(FLAG_AUTOTAX, val);
	}
}

void Game::ProcessAvoidOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "AVOID: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		parse_error(pCheck, u, 0, "AVOID: Invalid value.");
		return;
	}
	if (!pCheck) {
		if (val==1) {
			u->guard = GUARD_AVOID;
		} else {
			if (u->guard == GUARD_AVOID) {
				u->guard = GUARD_NONE;
			}
		}
	}
}

Unit *Game::ProcessFormOrder(Unit *former, AString *o, OrdersCheck *pCheck, bool repeating)
{
	AString *t = o->gettoken();
	if (!t) {
		parse_error(pCheck, former, 0, "Must give alias in FORM order.");
		return 0;
	}

	int an = t->strict_value();
	delete t;
	if (!an) {
		parse_error(pCheck, former, 0, "Must give alias in FORM order.");
		return 0;
	}
	if (an == -1) {
		parse_error(pCheck, former, 0, "Invalid alias in FORM order.");
		return 0;
	}
	if (pCheck) {
		Unit *retval = new Unit;
		retval->former = former;
		former->form_repeated = repeating;
		return retval;
	} else {
		if (former->object->region->GetUnitAlias(an, former->faction->num)) {
			former->error("Alias multiply defined.");
			return 0;
		}
		Unit *temp = GetNewUnit(former->faction, an);
		temp->CopyFlags(former);
		temp->DefaultOrders(former->object);
		temp->MoveUnit(former->object);
		temp->former = former;
		former->form_repeated = repeating;
		return temp;
	}
}

void Game::ProcessAddressOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString *token = o->gettoken();
	if (token) {
		if (!pCheck) {
			u->faction->address = token;
		}
	} else {
		parse_error(pCheck, u, 0, "ADDRESS: No address given.");
	}
}

void Game::ProcessAdvanceOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	MoveOrder *m = 0;

	if ((u->monthorders && u->monthorders->type != O_ADVANCE) ||
		(Globals->TAX_PILLAGE_MONTH_LONG && ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))
	) {
		string err = string("ADVANCE: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
		if (u->monthorders) delete u->monthorders;
		u->monthorders = nullptr;
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	if (!u->monthorders) {
		u->monthorders = new MoveOrder;
		u->monthorders->type = O_ADVANCE;
	}
	m = dynamic_cast<MoveOrder *>(u->monthorders);
	m->advancing = 1;

	for (;;) {
		AString *t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d!=-1) {
			if (!pCheck) {
				MoveDir *x = new MoveDir;
				x->dir = d;
				m->dirs.push_back(x);
			}
		} else {
			parse_error(pCheck, u, 0, "ADVANCE: Warning, bad direction.");
			return;
		}
	}
}

void Game::ProcessMoveOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	MoveOrder *m = 0;

	if ((u->monthorders && u->monthorders->type != O_MOVE) ||
		(Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE))
	) {
		string err = string("MOVE: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
		if (u->monthorders) delete u->monthorders;
		u->monthorders = nullptr;
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	if (!u->monthorders) {
		u->monthorders = new MoveOrder;
	}
	m = dynamic_cast<MoveOrder *>(u->monthorders);
	m->advancing = 0;

	for (;;) {
		AString *t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d!=-1) {
			if (!pCheck) {
				MoveDir *x = new MoveDir;
				x->dir = d;
				m->dirs.push_back(x);
			}
		} else {
			parse_error(pCheck, u, 0, "MOVE: Warning, bad direction.");
			return;
		}
	}
}

void Game::ProcessSailOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	SailOrder *m = 0;

	if ((u->monthorders && u->monthorders->type != O_SAIL) ||
		(Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE))
	) {
		string err = string("SAIL: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
		if (u->monthorders) delete u->monthorders;
		u->monthorders = nullptr;
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	if (!u->monthorders) {
		u->monthorders = new SailOrder;
	}
	m = dynamic_cast<SailOrder *>(u->monthorders);

	for (;;) {
		AString *t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d == -1) {
			parse_error(pCheck, u, 0, "SAIL: Warning, bad direction.");
			return;
		} else {
			if (d < NDIRS || d == MOVE_PAUSE) {
				if (!pCheck) {
					MoveDir *x = new MoveDir;
					x->dir = d;
					m->dirs.push_back(x);
				}
			} else {
				parse_error(pCheck, u, 0, "SAIL: Warning, bad direction.");
				return;
			}
		}
	}
}

void Game::ProcessEvictOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	UnitId *id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		if (!pCheck) {
			if (!u->evictorders) u->evictorders = new EvictOrder;
			u->evictorders->targets.push_back(id);
		}
		id = ParseUnit(o);
	}
}

void Game::ProcessIdleOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (u->monthorders || (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE))) {
		if (u->monthorders) delete u->monthorders;
		string err = string("IDLE: Overwriting previous ") + (u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
		parse_error(pCheck, u, 0, err);
	}
	if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	IdleOrder *i = new IdleOrder;
	u->monthorders = i;
}

void Game::ProcessTransportOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	UnitId *tar = ParseUnit(o);
	if (!tar) {
		parse_error(pCheck, u, 0, "TRANSPORT: Invalid target.");
		return;
	}
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "TRANSPORT: No amount given.");
		return;
	}

	int amt;
	if (*token == "all")
		amt = -1;
	else
		amt = token->value();
	delete token;

	token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "TRANSPORT: No item given.");
		return;
	}
	int item = ParseTransportableItem(token);

	if (item == -1) {
		parse_error(pCheck, u, 0, "TRANSPORT: Invalid item" + string(token->const_str()) + ".");
		delete token;
		return;
	}
	delete token;

	int except = 0;
	token = o->gettoken();
	if (token && *token == "except") {
		delete token;

		token = o->gettoken();
		if (!token) {
			parse_error(pCheck, u, 0, "TRANSPORT: EXCEPT requires a value.");
			return;
		}

		except = token->value();
		delete token;

		if (except <= 0) {
			parse_error(pCheck, u, 0, "TRANSPORT: Invalid except value.");
			return;
		}
	}

	if (!pCheck) {
		TransportOrder *order = new TransportOrder;
		// At this point we don't know that transport phase for the order but
		// we will set that later.
		order->item = item;
		order->target = tar;
		order->amount = amt;
		order->except = except;
		u->transportorders.push_back(order);
	}
	return;
}

void Game::ProcessShareOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	if (!token) {
		parse_error(pCheck, u, 0, "SHARE: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		parse_error(pCheck, u, 0, "SHARE: Invalid value.");
		return;
	}
	if (!pCheck) {
		u->SetFlag(FLAG_SHARING, val);
	}
}

void Game::ProcessJoinOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	int overload = 1;
	int merge = 0;

	UnitId *id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		parse_error(pCheck, u, 0, "JOIN: No target given.");
		return;
	}
	AString *token = o->gettoken();
	if (token) {
		if (*token == "nooverload")
			overload = 0;
		else if (*token == "merge")
			merge = 1;
		delete token;
	}
	if (!pCheck) {
		JoinOrder *ord = new JoinOrder;
		ord->target = id;
		ord->overload = overload;
		ord->merge = merge;
		if (u->joinorders) delete u->joinorders;
		u->joinorders = ord;
	}
}

void Game::ProcessSacrificeOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	// See if we have a sacrifice enabled object
	bool can_sacrifice = false;
	for (auto o = 0; o < NOBJECTS; o++) {
		ObjectType ob = ObjectDefs[o];
		if (!(ob.flags & ObjectType::DISABLED) && (ob.flags & ObjectType::SACRIFICE)) {
			can_sacrifice = true;
			break;
		}
	}
	if (!can_sacrifice) {
		parse_error(pCheck, unit, 0, "SACRIFICE is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	if (!token) {
		parse_error (pCheck, unit, 0, "SACRIFICE: No amount given.");
		return;
	}
	int amt = token->value();
	if (amt < 1) {
		parse_error (pCheck, unit, 0, "SACRIFICE: No amount given.");
		return;
	}
	delete token;
	token = o->gettoken();
	if (!token) {
		parse_error(pCheck, unit, 0, "SACRIFICE: No item given.");
		return;
	}

	int item = ParseEnabledItem(token);
	delete token;

	if (item == -1) {
		parse_error(pCheck, unit, 0, "SACRIFICE: Invalid item.");
		return;
	}
	if (ItemDefs[item].flags & ItemType::DISABLED) {
		parse_error(pCheck, unit, 0, "SACRIFICE: Invalid item.");
		return;
	}
	if (!pCheck) {
		SacrificeOrder *order = new SacrificeOrder;
		order->item = item;
		order->amount = amt;
		if (unit->sacrificeorders) delete unit->sacrificeorders;
		unit->sacrificeorders = order;
	}
	return;
}

void Game::ProcessAnnihilateOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	// Make sure the ANNIHILATE skill is enabled.
	if (SkillDefs[S_ANNIHILATION].flags & SkillType::DISABLED) {
		parse_error(pCheck, unit, 0, "ANNIHILATE is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	int x = -1;
	int y = -1;
	int z = unit->object->region->zloc;

	if (!token) {
		parse_error(pCheck, unit, 0, "ANNIHILATE: No region specified.");
		return;
	}
	if (!(*token == "region")) {
		delete token;
		parse_error(pCheck, unit, 0, "ANNIHILATE: No region specified.");
		return;
	}
	delete token;

	token = o->gettoken();
	if (!token) {
		unit->error("ANNIHILATE: Region X coordinate not specified.");
		return;
	}
	x = token->value();
	delete token;

	token = o->gettoken();
	if (!token) {
		unit->error("ANNIHILATE: Region Y coordinate not specified.");
		return;
	}
	y = token->value();
	delete token;


	RangeType *range = FindRange(SkillDefs[S_ANNIHILATION].range);
	if (range->flags & RangeType::RNG_SURFACE_ONLY) {
		z = (Globals->NEXUS_EXISTS ? 1 : 0);
	}

	if (range && (range->flags & RangeType::RNG_CROSS_LEVELS) && !(range->flags & RangeType::RNG_SURFACE_ONLY)) {
		token = o->gettoken();
		if (token) {
			z = token->value();
			delete token;
			if (z < 0 || z >= (Globals->UNDERWORLD_LEVELS + Globals->UNDERDEEP_LEVELS + Globals->ABYSS_LEVEL + 2)) {
				unit->error("ANNIHILATE: Invalid Z coordinate specified.");
				return;
			}
		}
	}

	if (!pCheck) {
		AnnihilateOrder *order = new AnnihilateOrder;
		order->xloc = x;
		order->yloc = y;
		order->zloc = z;
		unit->annihilateorders.push_back(order);
	}
}

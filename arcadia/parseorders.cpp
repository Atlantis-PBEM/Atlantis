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
#include "gameio.h"
#include "orders.h"
#include "skills.h"
#include "gamedata.h"

// replace with [[maybe_unused]] when c++17 is supported
#define MAYBE_UNUSED __attribute__ ((unused))

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

	AString *token = o->gettoken();
	if (!token) return -1;

	if (*token == "generic") {
		delete token;
		for (i=0; i<NFACTYPES; i++) type[i] = 1;
		return 0;
	}

	while(token) {
		int foundone = 0;
		for (i=0; i<NFACTYPES; i++) {
			if (*token == FactionStrs[i] || (i == 2 && *token == "MAGIC")) {
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

Faction * Game::ParseOrders(int faction, Aorders *f, OrdersCheck *pCheck)
{
	Faction *fac = 0;
	Unit *unit = 0;

	AString *order = f->GetLine();
//	FormTemplate * formtem = 0;
//	int formtemline = 0;

	while (order) {
		AString saveorder = *order;
		int getatsign = order->getat();
		int getquietsign = order->getexclamation();
		AString *token = order->gettoken();	

		if (token) {
			int i = Parse1Order(token);
			switch (i) {
			case -1:
				ParseError(pCheck, unit, fac, *token + " is not a valid order.");
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
					if(!faction) faction = token->value();  //"faction" is only used for label orders. This stuck in to allow parsing orders without knowing which faction it is ... could also otherwise cause trouble if unexpected faction orders are pulled in
					fac = GetFaction(&factions, faction);
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
						ParseError(pCheck, unit, fac, "TURN: without ENDTURN");
					if (unit->former)
						ParseError(pCheck, unit, fac, "FORM: without END.");
					if(pCheck) DoLabelOrders(pCheck, unit, fac, faction); //if not pCheck, this is done at the end of orders so that template/all orders included later in the report can be processed
					if(unit && pCheck) unit->ClearOrders();
					if (pCheck && former) delete unit;
					unit = former;
				}

				unit = 0;
//				fac = 0;
				break;

			case O_UNIT:
				if (fac) {
					while (unit) {
						Unit *former = unit->former;
						if (unit->inTurnBlock)
							ParseError(pCheck, unit, fac, "TURN: without ENDTURN");
						if (unit->former)
							ParseError(pCheck, unit, fac, "FORM: without END.");
					    if(pCheck) DoLabelOrders(pCheck, unit, fac, faction);
						if (unit && pCheck) unit->ClearOrders();
						if (pCheck && former) delete unit;
						unit = former;
					}
					unit = 0;
					delete token;

					token = order->gettoken();
					if (!token) {
						ParseError(pCheck, 0, fac, "UNIT without unit number.");
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
							unit = ProcessFormOrder(unit, order, pCheck, getquietsign);
							if(!pCheck) {
								if(unit) unit->ClearOrders();
							}
						}
					} else {
						ParseError(pCheck, 0, fac,
								"Order given without a unit selected:");
						ParseError(pCheck, 0, fac, saveorder);
					}
				}
				break;
			case O_ENDFORM:
				if (fac) {
					if (unit && unit->former) {
						Unit *former = unit->former;

						if (unit->inTurnBlock)
							ParseError(pCheck, unit, fac, "TURN: without ENDTURN");
					    if(pCheck) DoLabelOrders(pCheck, unit, fac, faction);
						if (pCheck && former) delete unit;
						unit = former;
					} else {
						ParseError(pCheck, unit, fac, "END: without FORM.");
					}
				}
				break;
			case O_ALL:
			    if(fac) {
                    //want to unselect unit here!
/*    				while (unit) {
    					Unit *former = unit->former;
    					if (unit->inTurnBlock)
    						ParseError(pCheck, unit, fac, "TURN: without ENDTURN");
    					if (unit->former)
    						ParseError(pCheck, unit, fac, "FORM: without END.");
				        if(pCheck) DoLabelOrders(pCheck, unit, fac, faction);
    					if (unit && pCheck) unit->ClearOrders();
    					if (pCheck && former) delete unit;
    					unit = former;
    				}
    				unit = 0;
        */				
    				// faction is 0 if checking syntax only, not running turn.
/*    				if (faction == 0) {
    				    unit = &(pCheck->dummyUnit);
						unit->monthorders = 0;
    				} else {
*/
    					if(pCheck) {
    						pCheck->pCheckFile->PutStr(saveorder);
			      		}
			      		
    					AString *retval;
    					retval = ProcessAllOrder(f, pCheck, order, fac); //no need to carry quiet sign - produce errors for incorrect TURN
    					if (retval) {
    						delete order;
    						order = retval;
    						continue;
//    					}
    				}
				}
				break;
			case O_TEMPLATE:
				if(fac) {
                    //want to unselect unit here!
/*    				while (unit) {
    					Unit *former = unit->former;
    					if (unit->inTurnBlock)
    						ParseError(pCheck, unit, fac, "TURN: without ENDTURN");
    					if (unit->former)
    						ParseError(pCheck, unit, fac, "FORM: without END.");
				        if(pCheck) DoLabelOrders(pCheck, unit, fac, faction);
    					if (unit && pCheck) unit->ClearOrders();
    					if (pCheck && former) delete unit;
    					unit = former;
    				}
    				unit = 0;
        				*/
				    // faction is 0 if checking syntax only, not running turn.
/*    				if (faction == 0) {
    				    unit = &(pCheck->dummyUnit);
						unit->monthorders = 0;
    				} else {
                        //want to unselect unit here!
        				while (unit) {
        					Unit *former = unit->former;
        					if (unit->inTurnBlock)
        						ParseError(pCheck, unit, fac, "TURN: without ENDTURN");
        					if (unit->former)
        						ParseError(pCheck, unit, fac, "FORM: without END.");
					        if(pCheck) DoLabelOrders(pCheck, unit, fac, faction);
        					if (unit && pCheck) unit->ClearOrders();
        					if (pCheck && former) delete unit;
        					unit = former;
        				}
        				unit = 0;
    				*/
    					if(pCheck) {
    						pCheck->pCheckFile->PutStr(saveorder);
			      		}
			      		
    					AString *retval;
    					retval = ProcessTemplateOrder(f, pCheck, order, fac); //no need to carry quiet sign - produce errors for incorrect TURN
    					if (retval) {
    						delete order;
    						order = retval;
    						continue;
//    					}
    				}
				}
				break;
			case O_TURN:
				if (unit && unit->inTurnBlock) {
					ParseError(pCheck, unit, fac, "TURN: cannot nest");
				} else if (!unit) {
						ParseError(pCheck, 0, fac, "Order given without a unit selected:");
						ParseError(pCheck, 0, fac, saveorder);
				} else {
					// faction is 0 if checking syntax only, not running turn.
					if (faction != 0) {
						AString *retval;
						int dummy = 0;
						retval = ProcessTurnOrder(unit, f, pCheck, getatsign, dummy); //no need to carry quiet sign - produce errors for incorrect TURN
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

						ProcessOrder(i, unit, order, pCheck, getquietsign);
					} else {
						ParseError(pCheck, 0, fac, "Order given without a unit selected:");
						ParseError(pCheck, 0, fac, saveorder);
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
		order = 0;
        order = f->GetLine();
		
		if(pCheck) {
			pCheck->pCheckFile->PutStr(saveorder);
		}
	}
	
	while (unit) {
		Unit *former = unit->former;
		if (unit->inTurnBlock)
			ParseError(pCheck, 0, fac, "TURN: without ENDTURN");
		if (unit->former)
			ParseError(pCheck, 0, fac, "FORM: without END.");
		if(unit && pCheck) unit->ClearOrders();
		if (pCheck && former) delete unit;
		unit = former;
	}

	if(unit && pCheck) {
		DoLabelOrders(pCheck, unit, fac, faction); //the ALL and LABEL might not have been put in, but at least this gives us some chance of processing ALL/LABEL orders
		unit->ClearOrders();
		unit = 0;
	}
	
	//process type/template and label/all orders
	if(!pCheck) {
	    forlist(&regions) {
    		ARegion *r = (ARegion *)elem;
    		forlist(&r->objects) {
    			Object *o = (Object *)elem;
    			forlist(&o->units) {
    				Unit *u = (Unit *)elem;
    				if(u->faction == fac) DoLabelOrders(0, u, fac, faction);
   				}
 			}
 		}	
	}
	
	return fac;
}

void Game::DoLabelOrders(OrdersCheck *pCheck, Unit *unit, Faction *fac, int faction)
{
    if(unit->label) DoLabelOrder(pCheck, unit, fac, faction, 0);
	if(unit->typeorders.Num()) {
	    forlist(&unit->typeorders) {
	        AString *name = (AString *) elem;
	        if(name) DoLabelOrder(pCheck, unit, fac, faction, name);
        }
		unit->typeorders.DeleteAll();
	}
}


void Game::DoLabelOrder(OrdersCheck *pCheck, Unit *unit, Faction *fac, int faction, AString *unittype) //label = 0 for label orders
{
    if(!fac) return;
    if(!unit) return;
    
    FormTemplate *formtem = 0;
    if(!unittype) {
        forlist(&fac->labeltemplates) {
            FormTemplate *ftem = (FormTemplate *) elem;
            if(*ftem->name == *unit->label) formtem = ftem;
        }
        if(!formtem) return;
    } else {
        forlist(&fac->formtemplates) {
            FormTemplate *ftem = (FormTemplate *) elem;
            if(*ftem->name == *unittype) formtem = ftem;
        }
    	if(!formtem) {
            if(!pCheck) ParseError(pCheck, unit, fac, AString("TYPE: Could not find template ") + *unittype); //no error for pCheck as it may refer to a saved template
            return;
        }
    }
    
    AString *order;
    int linenum = 0;
    int upto = 1;
    
    forlist(&formtem->orders) {
        order = new AString(*((AString *) elem));  //this gets the temp'th order
        linenum++; //1 for first line, etc. Used in turn orders, where passing a 1 will access the second line.
        if(linenum < upto) continue;
        else upto = linenum;

		AString saveorder = *order;
		int getatsign = order->getat();
		int getquietsign = order->getexclamation();
		AString *token = order->gettoken();

		if (token) {
			int i = Parse1Order(token);
			switch (i) {
			case -1:
				ParseError(pCheck, unit, fac, *token + " is not a valid order.");
				break;
			case O_TURN:
				if (unit && unit->inTurnBlock) {
					ParseError(pCheck, unit, fac, "TURN: cannot nest");
				} else {
					// faction is 0 if checking syntax only, not running turn.
					if (faction != 0) {
						AString *retval;
						retval = ProcessTurnOrder(unit, 0, pCheck, getatsign, upto, formtem); //no need to carry quiet sign - produce errors for incorrect TURN
						upto++; //upto gets returned at the ENDTURN line (assuming it exists)
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
				if (unit->inTurnBlock) {
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
				if(!pCheck && getatsign) unit->oldorders.Add(new AString(saveorder));
				ProcessOrder(i, unit, order, pCheck, getquietsign);
			}
			SAFE_DELETE(token);
        }
    }
}

void Game::ProcessOrder(int orderNum, Unit *unit, AString *o,
		OrdersCheck *pCheck, int isquiet)
{
	switch(orderNum) {
		case O_ADDRESS:
			ProcessAddressOrder(unit, o, pCheck);
			break;
		case O_ADVANCE:
			ProcessAdvanceOrder(unit, o, pCheck, isquiet);
			break;
		case O_ASSASSINATE:
			ProcessAssassinateOrder(unit, o, pCheck, isquiet);
			break;
		case O_ATTACK:
			ProcessAttackOrder(unit, o, pCheck, isquiet);
			break;
		case O_AUTOTAX:
			ProcessAutoTaxOrder(unit, o, pCheck);
			break;
		case O_AVOID:
			ProcessAvoidOrder(unit, o, pCheck);
			break;
		case O_BANK:
			ProcessBankOrder(unit, o, pCheck, isquiet);
			break;
		case O_IDLE:
			ProcessIdleOrder(unit, o, pCheck);
			break;
		case O_BEHIND:
			ProcessBehindOrder(unit, o, pCheck);
			break;
		case O_BUILD: //this includes BUILDHEXSIDE
		case O_BUILDHEXSIDE:
			ProcessBuildOrder(unit, o, pCheck, isquiet);
			break;
		case O_BUY:
			ProcessBuyOrder(unit, o, pCheck, isquiet);
			break;
		case O_CAST:
			ProcessCastOrder(unit, o, pCheck, isquiet);
			break;
		case O_CLAIM:
			ProcessClaimOrder(unit, o, pCheck, isquiet);
			break;
		case O_COMBAT:
			ProcessCombatOrder(unit, o, pCheck);
			break;
		case O_COMMAND:
			ProcessCommandOrder(unit, o, pCheck);
			break;
		case O_CONSUME:
			ProcessConsumeOrder(unit, o, pCheck);
			break;
		case O_DECLARE:
			ProcessDeclareOrder(unit->faction, o, pCheck, isquiet);
			break;
		case O_DESCRIBE:
			ProcessDescribeOrder(unit, o, pCheck, isquiet);
			break;
		case O_LABEL:
			ProcessLabelOrder(unit, o, pCheck, isquiet);
			break;
		case O_DESTROY:
			ProcessDestroyOrder(unit, pCheck, isquiet);
		    break;
		case O_DISABLE:
			ProcessDisableOrder(unit, o, pCheck, isquiet);
			break;
		case O_ENTER:
			ProcessEnterOrder(unit, o, pCheck, isquiet);
			break;
		case O_ENTERTAIN:
			ProcessEntertainOrder(unit, pCheck, isquiet);
			break;
		case O_EVICT:
			ProcessEvictOrder(unit, o, pCheck, isquiet);
			break;
		case O_EXCHANGE:
			ProcessExchangeOrder(unit, o, pCheck, isquiet);
			break;
		case O_FACTION:
			ProcessFactionOrder(unit, o, pCheck, isquiet);
			break;
		case O_FIGHTAS:
			ProcessFightAsOrder(unit, o, pCheck);
			break;			
		case O_TACTICS:
			ProcessTacticsOrder(unit, o, pCheck);
			break;
   		case O_FIND:
			ProcessFindOrder(unit, o, pCheck, isquiet);
			break;
   		case O_FOLLOW:
			ProcessFollowOrder(unit, o, pCheck, isquiet);
			break;
		case O_FORGET:
			ProcessForgetOrder(unit, o, pCheck, isquiet);
			break;
		case O_WISHDRAW:
			ProcessWishdrawOrder(unit, o, pCheck);
			break;
		case O_WISHSKILL:
			ProcessWishskillOrder(unit, o, pCheck);
			break;
		case O_WITHDRAW:
			ProcessWithdrawOrder(unit, o, pCheck, isquiet);
			break;
		case O_MASTER:
		    ProcessMasterOrder(unit, o, pCheck, isquiet);
		    break;
		case O_GIVE:
			ProcessGiveOrder(unit, o, pCheck, isquiet);
			break;
		case O_GUARD:
			ProcessGuardOrder(unit, o, pCheck, isquiet);
			break;
		case O_HOLD:
			ProcessHoldOrder(unit, o, pCheck);
			break;
		case O_LEAVE:
			ProcessLeaveOrder(unit, pCheck, isquiet);
			break;
		case O_MOVE:
			ProcessMoveOrder(unit, o, pCheck, isquiet);
			break;
		case O_NAME:
			ProcessNameOrder(unit, o, pCheck, isquiet);
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
			ProcessPillageOrder(unit, pCheck, isquiet);
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
			ProcessProduceOrder(unit, o, pCheck, isquiet);
			break;
		case O_PROMOTE:
			ProcessPromoteOrder(unit, o, pCheck, isquiet);
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
			ProcessSailOrder(unit, o, pCheck, isquiet);
			break;
		case O_SELL:
			ProcessSellOrder(unit, o, pCheck, isquiet);
			break;
		case O_SEND:
			ProcessSendOrder(unit, o, pCheck, isquiet);
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
			ProcessStealOrder(unit, o, pCheck, isquiet);
			break;
		case O_STUDY:
			ProcessStudyOrder(unit, o, pCheck, isquiet);
			break;
		case O_TAX:
			ProcessTaxOrder(unit, pCheck, isquiet);
			break;
		case O_TEACH:
			ProcessTeachOrder(unit, o, pCheck, isquiet);
			break;
		case O_TYPE:
			ProcessTypeOrder(unit, o, pCheck);
			break;
		case O_WORK:
			ProcessWorkOrder(unit, pCheck, isquiet);
			break;
		case O_TRANSPORT:
			ProcessTransportOrder(unit, o, pCheck, isquiet);
			break;
		case O_DISTRIBUTE:
			ProcessDistributeOrder(unit, o, pCheck, isquiet);
			break;
	}
}

void Game::ProcessPasswordOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if(pCheck) return;

	AString *token = o->gettoken();
	if (u->faction->password) delete u->faction->password;
	if (token) {
		u->faction->password = token;
		u->faction->Event(AString("Password is now: ") + *token);
	} else {
		u->faction->password = new AString("none");
		u->faction->Event("Password cleared.");
	}
}

void Game::ProcessOptionOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
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

	if (*token == "showattitudes") {
		delete token;
		u->faction->Event("Units will now have a leading sign to show your " 
									"attitude to them.");
		u->faction->showunitattitudes = 1;
		return;
	}

	if (*token == "dontshowattitudes") {
		delete token;
		u->faction->Event("Units will now have a leading minus sign regardless"
									" of your attitude to them.");
		u->faction->showunitattitudes = 0;
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

void Game::ProcessReshowOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
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
		int lvl;
		if (!token) {
			ParseError(pCheck, u, 0, "SHOW: No skill level given. Showing level 1");
			lvl = 1;
		}
		else {
            lvl = token->value();
		    delete token;
        }

		if(!pCheck) {
/*		//Disabled for Arcadia - you can see any skill you want to!
			if (lvl > u->faction->skills.GetDays(sk)) {
				u->Error("SHOW: Faction doesn't have that skill.");
				return;
			}*/

			u->faction->shows.Add(new ShowSkill(sk, lvl));
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

void Game::ProcessForgetOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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
		ord->quiet = isquiet;
		ord->skill = sk;
		u->forgetorders.Add(ord);
	}
}

void Game::ProcessEntertainOrder(Unit *unit, OrdersCheck *pCheck, int isquiet)
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
	ProduceOrder *ord = new ProduceOrder;
	ord->item = I_SILVER;
	ord->skill = S_ENTERTAINMENT;
	ord->quiet = isquiet;
	unit->monthorders = ord;
}

void Game::ProcessCombatOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	AString *token = o->gettoken();
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
			u->Error("COMBAT: That unit is not a hero.");
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

void Game::ProcessCommandOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if(!pCheck) {
		if (u->type != U_MAGE) {
			u->Error("COMMAND: That unit is not a hero.");
			return;
		}
		u->SetFlag(FLAG_COMMANDER, 1);
		//remove the old commander
		forlist(&regions) {
			ARegion *reg = (ARegion *)elem;
			forlist(&reg->objects) {
				Object *obj = (Object *)elem;
				forlist(&obj->units) {
					Unit *u2 = (Unit *)elem;
					if((u2->flags & FLAG_COMMANDER) && u2->faction == u->faction && u2 != u) {
					    u2->SetFlag(FLAG_COMMANDER, 0);
					    u2->Event("Is removed from command");
	                }
                }
            }
        }
		u->Event(AString(*u->name) + " takes command of your faction.");		
		
		if(u->GetEthnicity() != u->faction->ethnicity) {
		    u->faction->ethnicity = u->GetEthnicity();
		    WorldEvent *event = new WorldEvent;
            event->type = WorldEvent::CONVERSION;
            event->fact1 = u->faction->num;
            event->fact2 = u->faction->ethnicity;
            event->reportdelay = 0;
            worldevents.Add(event);
        }
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
	if (it == -1) {
		ParseError(pCheck, u, 0, "PREPARE: Invalid item.");
		return;
	}
	BattleItemType *bt = FindBattleItem(token->Str());
	delete token;

	if(bt == NULL) {
		ParseError(pCheck, u, 0, "PREPARE: Invalid item.");
		return;
	}

	if (!(bt->flags & BattleItemType::SPECIAL)) {
		ParseError(pCheck, u, 0, "PREPARE: That item cannot be prepared.");
		return;
	}

	if(!pCheck) {
		if ((bt->flags & BattleItemType::MAGEONLY) &&
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
	AString temp = "Preferred armour set to: ";
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

void Game::ProcessClaimOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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
			if(!isquiet) u->Error("CLAIM: Don't have that much unclaimed silver.");
			value = u->faction->unclaimed;
		}
		u->faction->unclaimed -= value;
		u->SetMoney(u->GetMoney() + value);
		u->Event(AString("Claims $") + value + ".");
	}
}

void Game::ProcessFactionOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	if(Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_FACTION_TYPES) {
		ParseError(pCheck, u, 0,
				"FACTION: Invalid order, no faction types in this game.");
		return;
	}

	int oldfactype[NFACTYPES];
	int factype[NFACTYPES];

	int i;
	if(!pCheck) {
		for(i = 0; i < NFACTYPES; i++) {
			oldfactype[i] = u->faction->type[i];
		}
	}

	int retval = ParseFactionType(o, factype);
	if (retval == -1) {
		ParseError(pCheck, u, 0, "FACTION: Bad faction type.");
		return;
	}

	if(!pCheck) {
		int m = CountMages(u->faction);
		int a = CountApprentices(u->faction);

		for(i = 0; i < NFACTYPES; i++) u->faction->type[i] = factype[i];

		if(m > AllowedMages(u->faction)) {
			if(!isquiet) u->Error(AString("FACTION: Too many mages to change to that "
							 "faction type."));

			for(i = 0; i < NFACTYPES; i++)
				u->faction->type[i] = oldfactype[i];

			return;
		}

		if (a > AllowedApprentices(u->faction)) {
			if(!isquiet) u->Error(AString("FACTION: Too many apprentices to change to that "
							 "faction type."));

			for(i = 0; i < NFACTYPES; i++)
				u->faction->type[i] = oldfactype[i];

			return;
		}

		if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
			int q = CountQuarterMasters(u->faction);
			if((Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) &&
					(q > AllowedQuarterMasters(u->faction))) {
				if(!isquiet) u->Error(AString("FACTION: Too many quartermasters to "
							"change to that faction type."));

				for(i = 0; i < NFACTYPES; i++)
					u->faction->type[i] = oldfactype[i];

				return;
			}
		}

		u->faction->lastchange = TurnNumber();
		u->faction->DefaultOrders();
	}
}

void Game::ProcessAssassinateOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	UnitId *id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		ParseError(pCheck, u, 0, "ASSASSINATE: No target given.");
		return;
	}
	if(!pCheck) {
		if (u->stealorders) delete u->stealorders;
		AssassinateOrder *ord = new AssassinateOrder;
		ord->target = id;
	    ord->quiet = isquiet;
		u->stealorders = ord;
	}
}

void Game::ProcessStealOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	UnitId *id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		ParseError(pCheck, u, 0, "STEAL: No target given.");
		return;
	}
	AString *token = o->gettoken();
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
		StealOrder *ord = new StealOrder;
		ord->target = id;
		ord->item = i;
	    ord->quiet = isquiet;
		if (u->stealorders) delete u->stealorders;
		u->stealorders = ord;
	}
}

void Game::ProcessQuitOrder(Unit *u, AString *o, OrdersCheck *pCheck)
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

void Game::ProcessRestartOrder(Unit *u, AString *o, OrdersCheck *pCheck)
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
			Faction *pFac = AddFaction(0, NULL);
			pFac->SetAddress(*(u->faction->address));
			AString *pass = new AString(*(u->faction->password));
			pFac->password = pass;
			AString *facstr = new AString(AString("Restarting ")
					+ *(pFac->address) + ".");
			newfactions.Add(facstr);
		}
	}
}

void Game::ProcessDestroyOrder(Unit *u, OrdersCheck *pCheck, int isquiet)
{
	if(!pCheck) {
		u->destroy = 1;             //No easy way of keeping this quiet
	}
}

void Game::ProcessFindOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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
		FindOrder *ord = new FindOrder;
		ord->find = n;
	    ord->quiet = isquiet;
		u->findorders.Add(ord);
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

void Game::ProcessBankOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	int amt;
	int what;
//	int inbank;
//	int lvl;
//	int max = Globals->BANK_MAXSKILLPERLEVEL *5; // value if banks & skills disabled

	if (!(Globals->ALLOW_BANK & GameDefs::BANK_ENABLED)) {
		ParseError(pCheck, u, 0, "There are no banks in this game.");
		return;
	}
	AString *token = o->gettoken();
	if (token) {
		if (*token == "deposit")
			what = 2;
		if (*token == "withdraw")
			what = 1;
		delete token;
		if (what == 2) {
			token = o->gettoken();
			if (!token) {
				ParseError(pCheck, u, 0, "BANK: No amount to deposit given.");
				return;
			}
			amt = token->value();
			delete token;
		} else if (what == 1) {	// withdrawal
			token = o->gettoken();
			if (!token) {
				ParseError(pCheck, u, 0, "BANK: No amount to withdraw given.");
				return;
			}
			amt = token->value();
			delete token;
		} else {
			ParseError(pCheck, u, 0, "BANK: No WITHDRAW or DEPOSIT given.");
			return;
		}
	} else {
		ParseError(pCheck, u, 0, "BANK: No action given.");
		return;
	}
	if(!pCheck) {
		BankOrder *ord = new BankOrder;
		ord->what = what;
		ord->amount = amt;
	    ord->quiet = isquiet;
		u->bankorders.Add(ord);
	}
	return;
}

void Game::ProcessRevealOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if(!pCheck) {
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
		} else {
			u->reveal = REVEAL_NONE;
		}
	}
}

void Game::ProcessTaxOrder(Unit *u, OrdersCheck *pCheck, int isquiet)
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

	u->taxing = TAX_TAX;   //no easy way to keep quiet
}

void Game::ProcessPillageOrder(Unit *u, OrdersCheck *pCheck, int isquiet)
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

void Game::ProcessPromoteOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	UnitId *id = ParseUnit(o);
	if (!id || id->unitnum == -1) {
		ParseError(pCheck, u, 0, "PROMOTE: No target given.");
		return;
	}
	if(!pCheck) {
		if (u->promote) {
			delete u->promote;
		}
		u->promote = id;
		u->promotequiet = isquiet;
	}
}

void Game::ProcessLeaveOrder(Unit *u, OrdersCheck *pCheck, int isquiet)
{
	if(!pCheck) {
//		if (u->monthorders && u->monthorders->type == O_BUILD) return;
		if (u->enter == 0) u->enter = -1;
	}
}

void Game::ProcessEnterOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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

void Game::ProcessBuildOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
	if (token) {
		if(*token == "help") {
			UnitId *targ = 0;
			delete token;
			if(!pCheck) {
				targ = ParseUnit(o);
				if(!targ) {
					unit->Error("BUILD: Non-existent unit to help.");
					return;
				}
				if(targ->unitnum == -1) {
					unit->Error("BUILD: Non-existent unit to help.");
					return;
				}
			}
			BuildOrder *ord = new BuildOrder;
			ord->target = targ;
	        ord->quiet = isquiet;
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
			unit->monthorders = ord;
//			if (unit->enter == -1) unit->enter = 0;
			return;
		}

		int ot = ParseObject(token);
		if (ot==-1 || ObjectDefs[ot].flags & ObjectType::DISABLED) {
		    int ht = ParseHexside(token);
		    delete token;
            if (ht==-1 || HexsideDefs[ht].flags & HexsideType::DISABLED || !Globals->HEXSIDE_TERRAIN) {
			    ParseError(pCheck, unit, 0, "BUILD: Not a valid object name.");
			    return;
		    } else {
		        token = o->gettoken();
		        int dir = ParseHexsideDir(token);
		        if(dir<0 || dir>5) {
		            ParseError(pCheck, unit, 0, "BUILD: Not a valid direction.");
		            return;
		        }
	            BuildHexsideOrder *ord = new BuildHexsideOrder;
	            ord->terrain = ht;
	            ord->direction = dir;
                ord->quiet = isquiet;
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
	            unit->monthorders = ord;
//	            if (unit->enter == -1) unit->enter = 0;
			    return;
		    }
		}
		delete token;
		if (!pCheck) {
			ARegion *reg = unit->object->region;
			if (TerrainDefs[reg->type].similar_type == R_OCEAN && !(ObjectDefs[ot].flags & ObjectType::OCEANBUILD)) {
				if(!isquiet) unit->Error("BUILD: Can't build that in an ocean.");
				return;
			}

			if (ObjectIsShip(ot) && !(ObjectDefs[ot].flags & ObjectType::SAILOVERLAND) && !Globals->HEXSIDE_TERRAIN) {
				if (!reg->IsCoastalOrLakeside()) {
					if(!isquiet) unit->Error("BUILD: Can't build ship in "
							"non-(coastal or lakeside) region.");
					return;
				}
			}
			if (reg->buildingseq > 99) {
				if(!isquiet) unit->Error("BUILD: The region is full.");
				return;
			}
/* Hexside Patch 030825 BS*/			
			int dir=-1;
			if (ObjectDefs[ot].hexside && Globals->HEXSIDE_TERRAIN) {
		   		AString * t = o->gettoken();
		   		if (!t) {
                     if(!isquiet) unit->Error("BUILD: No direction specified.");
                     return;
                }
                dir = ParseHexsideDir(t);
		   		if(dir < 0) {
					if(!isquiet) unit->Error("BUILD: Incorrect direction specified.");
					return;
				}
		   	    if (reg->neighbors[dir]->buildingseq > 99) {
	                if(!isquiet) unit->Error("BUILD: The neighbouring region is full.");
	                return;
                }                 	
			}			
			
			
			Object *obj = new Object(reg);
			obj->type = ot;
			obj->incomplete = ObjectDefs[obj->type].cost;
			obj->hexside = dir;
			obj->inner = -1;
			unit->build = obj;
			unit->object->region->objects.Add(obj);
		}
	}

	BuildOrder *ord = new BuildOrder;
	ord->target = NULL;
	ord->quiet = isquiet;
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
	unit->monthorders = ord;
//	if (unit->enter == -1) unit->enter = 0;
}

void Game::ProcessAttackOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	UnitId *id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		if(!pCheck) {
			if (!u->attackorders) u->attackorders = new AttackOrder;
			u->attackorders->targets.Add(id);
			if(!isquiet) u->attackorders->quiet = 0;
		}
		id = ParseUnit(o);
	}
}

void Game::ProcessSellOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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
		SellOrder *ord = new SellOrder;
		ord->item = it;
		ord->num = num;
	    ord->quiet = isquiet;
		u->sellorders.Add(ord);
	}
}

void Game::ProcessBuyOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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
					if(ItemDefs[i].type & IT_LEADER) continue;
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
		BuyOrder *ord = new BuyOrder;
		ord->item = it;
		ord->num = num;
	    ord->quiet = isquiet;
		u->buyorders.Add(ord);
	}
}

void Game::ProcessProduceOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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

	ProduceOrder *ord = new ProduceOrder;
	ord->item = it;
	ord->quiet = isquiet;
	AString skname = ItemDefs[it].pSkill;
	ord->skill = LookupSkill(&skname);
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
	u->monthorders = ord;
}

void Game::ProcessWorkOrder(Unit *u, OrdersCheck *pCheck, int isquiet)
{
	ProduceOrder *ord = new ProduceOrder;
	ord->skill = -1;
	ord->item = I_SILVER;
	ord->quiet = isquiet;
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
	u->monthorders = ord;
}

void Game::ProcessTeachOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	TeachOrder *ord = 0;

	if (u->monthorders && u->monthorders->type == O_TEACH) {
		ord = (TeachOrder *) u->monthorders;
	} else {
		ord = new TeachOrder;
		ord->quiet = 1;
	}
	
	if(!isquiet) ord->quiet = 0;

	int students = 0;
	UnitId *id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		students++;
		if(ord) {
			ord->targets.Add(id);
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
	u->monthorders = ord;
}

void Game::ProcessStudyOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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

	StudyOrder *ord = new StudyOrder;
	ord->skill = sk;
	ord->days = 0;
	ord->quiet = isquiet;
	
	token = o->gettoken(); //STUDY order mod start
	if(token) {
	    ord->level = token->value();
	    delete token;	
	} else ord->level = 0; //STUDY order mod end	
	
	if(pCheck || u->IsMage() && Globals->ARCADIA_MAGIC) {
	    if(u->herostudyorders) {
	        delete u->herostudyorders;
            AString err = "STUDY: Overwriting previous ";
    		err += "study order.";
    		if(!u->inTurnBlock) ParseError(pCheck, u, 0, err); //if in turn block, don't worry about the error for now. Saves making new variables for unit.
	    }
        u->herostudyorders = ord;
        return;
    }
	
	//not a pCheck or a mage
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
	u->monthorders = ord;
}

void Game::ProcessDeclareOrder(Faction *f, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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
		Faction *target;
		if (fac != -1) {
			target = GetFaction(&factions, fac);
			if (!target) {
				if(!isquiet) f->Error(AString("DECLARE: Non-existent faction ")+fac+".");
				return;
			}
			if (target == f) {
				if(!isquiet) f->Error(AString("DECLARE: Can't declare towards your own "
								 "faction."));
				return;
			}
		}
	}

	token = o->gettoken();
	if (!token) {
		if (fac != -1) {
			if(!pCheck) {
				f->SetAttitude(fac, -1);
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
			f->SetAttitude(fac, att);
		}
	}
}

void Game::ProcessWithdrawOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
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
		WithdrawOrder *ord = new WithdrawOrder;
		ord->item = item;
		ord->amount = amt;
	    ord->quiet = isquiet;
		unit->withdraworders.Add(ord);
	}
	return;
}

void Game::ProcessMasterOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
{
    //MASTER not enabled for Xanaxor
	ParseError(pCheck, unit, 0, "MASTER is not a valid order.");

/*
	if(!(Globals->ARCADIA_MAGIC)) {
		ParseError(pCheck, unit, 0, "MASTER is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	if (!token) {
		ParseError(pCheck, unit, 0, "MASTER: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, unit, 0, "MASTER: Invalid value.");
		return;
	}

	if(!pCheck) {
	    if( val == 0 ) unit->mastery = 0;
		else {
        	if ((unit->monthorders && unit->monthorders->type != O_MASTER) ||          //check other monthlong orders delete MASTER orders
        		(Globals->TAX_PILLAGE_MONTH_LONG &&
        		 ((unit->taxing == TAX_TAX) || (unit->taxing == TAX_PILLAGE)))) {
        		AString err = "MASTER: Overwriting previous ";
        		if (unit->inTurnBlock) err += "DELAYED ";
        		err += "month-long order.";
        		ParseError(pCheck, unit, 0, err);
        		if (unit->monthorders) delete unit->monthorders;
        		unit->monthorders = 0;
        	}
        	if(Globals->TAX_PILLAGE_MONTH_LONG) unit->taxing = TAX_NONE;
        	unit->monthorders = new MasterOrder;
        	unit->monthorders->quiet = isquiet;
 	    }
 	}
*/
	return;
}

void Game::ProcessWishdrawOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	if(!(Globals->TESTGAME_ENABLED) && (pCheck || !unit->faction->IsNPC() ) ) {
		ParseError(pCheck, unit, 0, "WISHDRAW is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	if(!token) {
		ParseError (pCheck, unit, 0, "WISHDRAW: No amount given.");
		return;
	}
	int amt = token->value();
	if(amt < 1) {
		amt = 1;
	} else {
		delete token;
		token = o->gettoken();
		if(!token) {
			ParseError(pCheck, unit, 0, "WISHDRAW: No item given.");
			return;
		}
	}
	int item = ParseGiveableItem(token);
	delete token;

	if (item == -1) {
		ParseError(pCheck, unit, 0, "WISHDRAW: Invalid item.");
		return;
	}
	if (ItemDefs[item].flags & ItemType::DISABLED) {
		ParseError(pCheck, unit, 0, "WISHDRAW: Invalid item.");
		return;
	}
	
	if(!pCheck) {
		WishdrawOrder *order = new WishdrawOrder;
		order->item = item;
		order->amount = amt;
		unit->wishdraworders.Add(order);
	}
	return;
}

void Game::ProcessWishskillOrder(Unit *unit, AString *o, OrdersCheck *pCheck)
{
	if(!(Globals->TESTGAME_ENABLED) && (pCheck || !unit->faction->IsNPC() ) ) {
		ParseError(pCheck, unit, 0, "WISHSKILL is not a valid order.");
		return;
	}

	AString *token = o->gettoken();
	if(!token) {
		ParseError (pCheck, unit, 0, "WISHSKILL: No skill given.");
		return;
	}

	int sk = ParseSkill(token);
	delete token;
	if (sk==-1) {
		ParseError(pCheck, unit, 0, "WISHSKILL: Invalid skill.");
		return;
	}

	if(SkillDefs[sk].flags & SkillType::DISABLED) {
		ParseError(pCheck, unit, 0, "WISHSKILL: Invalid skill.");
		return;
	}

	if((SkillDefs[sk].flags & SkillType::APPRENTICE) &&
			!Globals->APPRENTICES_EXIST) {
		ParseError(pCheck, unit, 0, "WISHSKILL: Invalid skill.");
		return;
	}

	token = o->gettoken();
	if(!token) {
		ParseError(pCheck, unit, 0, "WISHSKILL: No knowledge level given.");
		return;
	}
	int days = token->value();
	delete token;

	if(days < 0) days = 0;

	token = o->gettoken();
	if(!token) {
		ParseError(pCheck, unit, 0, "WISHSKILL: No experience level given.");
		return;
	}
	int exper = token->value();
	delete token;

	if(exper < 0) exper = 0;
	
	if(!pCheck) {
		WishskillOrder *order = new WishskillOrder;
		order->skill = sk;
		order->knowledge = days;
		order->experience = exper;
		unit->wishskillorders.Add(order);
	}
	return;
}

AString *Game::ProcessTurnOrder(Unit *unit, Aorders *f, OrdersCheck *pCheck,
		int repeat, int &formline, FormTemplate *formtem)
{
	int turnDepth = 1;
	int turnLast = 1;
	int formDepth = 0;
	TurnOrder *tOrder = new TurnOrder;
	tOrder->repeating = repeat;

	AString *order, *token;
	MAYBE_UNUSED int atsign;

	while (turnDepth) {
		// get the next line
		if(f) order = f->GetLine();
		else {
            order = formtem->GetLine(formline++); //todo!!!!
            if(!order) {
                ParseError(pCheck, unit, 0, "TURN: without ENDTURN.");
                return 0;
            }
        }
		
		if (!order) {
			// Fake end of commands to invoke appropriate processing
			order = new AString("#end");
		}
		AString	saveorder = *order;
		atsign = order->getat();
		token = order->gettoken();

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
							ParseError(pCheck, unit, 0, "TURN: without ENDTURN.");
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
				case O_TEMPLATE:
				case O_ALL:
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

AString *Game::ProcessTemplateOrder(Aorders *f, OrdersCheck *pCheck, AString *name, Faction *fac)
{
	FormTemplate *unittype = 0; 
	AString *temname = name->gettoken();
	if(!temname) {
        temname = new AString("default");
    }
    
    forlist(&fac->formtemplates) {
        FormTemplate *formtem = (FormTemplate *) elem;
        if(*formtem->name == *temname) {
            unittype = formtem;
            unittype->orders.DeleteAll();
        }
    }
    
    if(!unittype) {
        unittype = new FormTemplate;
        unittype->name = temname;
        fac->formtemplates.Add(unittype);
    } else delete temname;
    
	AString *order, *token;

	int exitafterone = 0;
	
	while (1) {
		// get the next line
		order = f->GetLine();
		if (!order) {
			// Fake end of commands to invoke appropriate processing
			order = new AString("#end");
		}
		
		if(exitafterone) return order;
		
		AString	saveorder = *order;
		token = order->gettoken();

		if (token) {
			int i = Parse1Order(token);
			switch (i) {
				case O_UNIT:
				case O_END:
				case O_TEMPLATE:
				case O_ALL:
					return new AString(saveorder);
					break;
				case O_ENDTEMPLATE:
				    exitafterone = 1;
				    break;
				default:
					unittype->orders.Add(new AString(saveorder));
					break;
			}
			delete token;
		}
		
		if(pCheck) {
			pCheck->pCheckFile->PutStr(saveorder);
		}
		
		delete order;
	}

	return NULL;
}

AString *Game::ProcessAllOrder(Aorders *f, OrdersCheck *pCheck, AString *name, Faction *fac)
{
	FormTemplate *labelorders = 0; 
	AString *temname = name->gettoken();
	if(!temname) {
        ParseError(pCheck, 0, 0, "ALL: No label specified.");
		return NULL;
    }
    
    forlist(&fac->labeltemplates) {
        FormTemplate *formtem = (FormTemplate *) elem;
        if(*formtem->name == *temname) {
            labelorders = formtem;        //add on to earlier all order.
        }
    }
    
    if(!labelorders) {
        labelorders = new FormTemplate;
        labelorders->name = temname;
        fac->labeltemplates.Add(labelorders);
    } else delete temname;
    
	AString *order, *token;

	int exitafterone = 0;

	while (1) {
	    
		// get the next line
		order = f->GetLine();
		if (!order) {
			// Fake end of commands to invoke appropriate processing
			order = new AString("#end");
		}
		
		if(exitafterone) return order;
		
		AString	saveorder = *order;
		
		token = order->gettoken();

		if (token) {
			int i = Parse1Order(token);
			switch (i) {
				case O_UNIT:
				case O_END:
				case O_TEMPLATE:
				case O_ALL:
					return new AString(saveorder);
					break;
				case O_ENDALL:
				    exitafterone = 1;
				    break;
				default:
					labelorders->orders.Add(new AString(saveorder));
					break;
			}
			delete token;
		}
		
		if(pCheck) {
			pCheck->pCheckFile->PutStr(saveorder);
		}
		
		delete order;
	}

	return NULL;
}

void Game::ProcessExchangeOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
{
	UnitId *t = ParseUnit(o);
	if (!t) {
		ParseError(pCheck, unit, 0, "EXCHANGE: Invalid target.");
		return;
	}
	AString *token = o->gettoken();
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
		ExchangeOrder *ord = new ExchangeOrder;
		ord->giveItem = itemGive;
		ord->giveAmount = amtGive;
		ord->expectAmount = amtExpected;
		ord->expectItem = itemExpected;
		ord->target = t;
	    ord->quiet = isquiet;
		unit->exchangeorders.Add(ord);
	}
}

void Game::ProcessGiveOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
{
	UnitId *t = ParseUnit(o);
	if (!t) {
		ParseError(pCheck, unit, 0, "GIVE: Invalid target.");
		return;
	}
	AString *token = o->gettoken();
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
	int item = -1;
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
				} else if(*token == "armour") {
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
		GiveOrder *ord = new GiveOrder;
		ord->item = item;
		ord->target = t;
		ord->amount = amt;
		ord->except = excpt;
	    ord->quiet = isquiet;
		unit->giveorders.Add(ord);
	}
	return;
}


void Game::ProcessSendOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
{
    if(Globals->SEND_COST < 0) {
		ParseError(pCheck, unit, 0, "SEND is not a valid order.");
		return;
	}
     
	AString *token = o->gettoken();
	int dir = -1;
	if(token && (*token == "direction" || *token == "dir")) {
	    delete token;
	    token = o->gettoken();
		if (!token) {
		    ParseError(pCheck, unit, 0, "SEND: No direction specified.");
            return;
        }
		dir = ParseDir(token);
		delete token;
		if(dir < 0) {
		    ParseError(pCheck, unit, 0, "SEND: Invalid direction specified.");
            return;		
		}
		token = o->gettoken();
	}
	
	UnitId *tar = NULL;
	UnitId *via = NULL;
	
	if(token && *token == "unit") {
	    delete token;
    	tar = ParseUnit(o);
    	if (!tar) {
    		ParseError(pCheck, unit, 0, "SEND: Invalid target.");
    		return;
    	}
	    token = o->gettoken();
	}
	
	if(dir == -1 && tar == NULL) {
	    if(token) delete token;
    	ParseError(pCheck, unit, 0, "SEND: Must specify direction and/or unit number.");
    	return;
	}

	if(token && *token == "via") {
	    delete token;
    	via = ParseUnit(o);
    	if (!tar) {
    		ParseError(pCheck, unit, 0, "SEND: Invalid quartermaster.");
    	}
	    token = o->gettoken();
	}

	if (!token) {
		ParseError(pCheck, unit, 0, "SEND: No amount given.");
		return;
	}
	
	int amt;
	if (*token == "all") {
		amt = -2;
	} else {
		amt = token->value();
	}
	delete token;
	int item = -1;

	token = o->gettoken();
	if(token) {
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
			//men and monsters cannot be SENT
			} else if(*token == "magic") {
				item = -IT_MAGIC;
				found = 1;
			} else if((*token == "weapon") || (*token == "weapons")) {
				item = -IT_WEAPON;
				found = 1;
			} else if(*token == "armour") {
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
						"SEND: Invalid item or item class.");
				return;
			}
		} else if(item == -1) {
			ParseError(pCheck, unit, 0, "SEND: Invalid item.");
			return;
		}
	} else {
		ParseError(pCheck, unit, 0, "SEND: No item given.");
		return;
	}
	delete token;
	
	if(item >= 0 && ((ItemDefs[item].type & IT_MAN) || 
        (ItemDefs[item].type & IT_MONSTER) || (ItemDefs[item].type & IT_ILLUSION))) {
	    ParseError(pCheck, unit, 0, "SEND: Invalid item.");
    }

	token = o->gettoken();
	int excpt = 0;
	if(token && *token == "except") {
		delete token;
		if(amt == -2) {
			if(item < 0) {
				ParseError(pCheck, unit, 0,
						"SEND: EXCEPT only valid with specific items.");
				return;
			}
			token = o->gettoken();
			if(!token) {
				ParseError(pCheck, unit, 0, "SEND: EXCEPT requires a value.");
				return;
			}
			excpt = token->value();
			if(excpt <= 0) {
				ParseError(pCheck, unit, 0, "SEND: Invalid EXCEPT value.");
				return;
			}
		} else {
			ParseError(pCheck, unit, 0, "SEND: EXCEPT only valid with ALL");
			return;
		}
	}

	if(!pCheck) {
		SendOrder *ord = new SendOrder;
		ord->item = item;
		ord->target = tar;
		ord->via = via;
		ord->amount = amt;
		ord->except = excpt;
		ord->direction = dir;
	    ord->quiet = isquiet;
		unit->sendorders.Add(ord);
	}
	return;
}


void Game::ProcessDescribeOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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
				if(!isquiet) unit->Error("DESCRIBE: Unit is not owner.");
				return;
			}
			unit->object->SetDescribe(token);
		}
		return;
	}
	ParseError(pCheck, unit, 0, "DESCRIBE: Can't describe that.");
}

void Game::ProcessLabelOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
	if(!pCheck) {
		unit->SetLabel(token);
	}
}

void Game::ProcessNameOrder(Unit *unit, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
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
				if(!isquiet) unit->Error("NAME: Unit is not owner.");
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
				if(!isquiet) unit->Error("NAME: Unit is not in a structure.");
				return;
			}
			if(!unit->object->region->town) {
				if(!isquiet) unit->Error("NAME: Unit is not in a village, town or city.");
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
				if(!isquiet) unit->Error(AString("NAME: Unit is not in a large ")+
							"enough structure to rename a "+tstring+".");
				return;
			}
			if (unit != unit->object->GetOwner()) {
				if(!isquiet) unit->Error(AString("NAME: Cannot name ")+tstring+
							".  Unit is not the owner of object.");
				return;
			}
			if (unit->object->incomplete > 0) {
				if(!isquiet) unit->Error(AString("NAME: Cannot name ")+tstring+
							".  Object is not finished.");
				return;
			}

			AString *newname = token->getlegal();
			if (!newname) {
				if(!isquiet) unit->Error("NAME: Illegal name.");
				return;
			}
			if(cost) {
				int silver = unit->items.GetNum(I_SILVER);
				if(silver < cost) {
					if(!isquiet) unit->Error(AString("NAME: Unit doesn't have enough ")+
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

void Game::ProcessGuardOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	/* This is an instant order */
	AString *token = o->gettoken();
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

void Game::ProcessBehindOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString *token = o->gettoken();
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
		u->SetFlag(FLAG_BEHIND, val);
	}
}

void Game::ProcessFightAsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	int flag = 0;
	int val = 1;
	if(token) {
		if(*token == "foot") flag = FLAG_FIGHTASFOOT;
		else if(*token == "ride") flag = FLAG_FIGHTASRIDE;
		else if(*token == "fly")  val = 0; //No need for this?
		else ParseError(pCheck, u, 0, "FIGHT: Bad argument.");
		delete token;
	}

	if(!pCheck) {
		/* Clear all the flags */
		u->SetFlag(FLAG_FIGHTASFOOT, 0);
		u->SetFlag(FLAG_FIGHTASRIDE, 0);

		/* Set the flag we're trying to set */
		if(flag) {
			u->SetFlag(flag, val);
		}
	}
}

void Game::ProcessTacticsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
//instant order
	if(!pCheck) {
		AString *token = o->gettoken();
		if (token) {
			if (*token == "aggressive") {
				u->tactics = TACTICS_AGGRESSIVE;
				delete token;
				return;
			}
			if (*token == "defensive") {
				delete token;
				u->tactics = TACTICS_DEFENSIVE;
				return;
			}
			if (*token == "none") {
				delete token;
				u->tactics = TACTICS_NONE;
				return;
			}
		} else {
			u->tactics = TACTICS_NONE;
		}
	}
}


void Game::ProcessNoaidOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	// Instant order 
	AString *token = o->gettoken();
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
		u->SetFlag(FLAG_NOAID, val);
	}
}

void Game::ProcessDisableOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
     
	AString *token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "DISABLE: No skill given.");
		return;
	}
	int sk = ParseSkill(token);
	delete token;
	if (sk==-1) {
		ParseError(pCheck, u, 0, "DISABLE: Invalid skill.");
		return;
	}

	if(SkillDefs[sk].flags & SkillType::DISABLED) {
		ParseError(pCheck, u, 0, "DISABLE: Invalid skill.");
		return;
	}

	// Instant order 
	token = o->gettoken();
	int val = 1;
	
	if (token) {
	    val = ParseTF(token);
	    delete token;
	}
	
	if (val==-1) {
		ParseError(pCheck, u, 0, "DISABLE: Invalid value.");
		return;
	}
	if(!pCheck) {
		if(!u->skills.SetDisabled(sk, val) && !isquiet) {  //val of 1 means skill is disabled.
            u->Error("DISABLE: Does not know that skill");
        }
        if(u->combat == sk) {
			u->combat = -1;
			u->Event("Combat spell set to none.");
		}
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
		else if(*token == "swim") flag = FLAG_SWIMSPOILS;
		else if(*token == "sail") flag = FLAG_SAILSPOILS;
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
		u->SetFlag(FLAG_SWIMSPOILS, 0);
		u->SetFlag(FLAG_SAILSPOILS, 0);

		/* Set the flag we're trying to set */
		if(flag) {
			u->SetFlag(flag, val);
		}
	}
}

void Game::ProcessTypeOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	if(!token) token = new AString("default");
	
	if(!pCheck) {
	    forlist(&u->typeorders) {
	        AString *name = (AString *) elem;
	        if(*name == *token) {
	            delete token;
                return;
            }
	    }
	    u->typeorders.Add(token);
	}
}

void Game::ProcessNospoilsOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	ParseError(pCheck, u, 0, "NOSPOILS: This command is deprecated.  "
			"Use the 'SPOILS' command instead");

	/* Instant order */
	AString *token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "NOSPOILS: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "NOSPOILS: Invalid value.");
		return;
	}
	if(!pCheck) {
		u->SetFlag(FLAG_FLYSPOILS, 0);
		u->SetFlag(FLAG_RIDESPOILS, 0);
		u->SetFlag(FLAG_WALKSPOILS, 0);
		u->SetFlag(FLAG_SWIMSPOILS, 0);
		u->SetFlag(FLAG_SAILSPOILS, 0);
		u->SetFlag(FLAG_NOSPOILS, val);
	}
}

void Game::ProcessNocrossOrder(Unit *u, AString *o, OrdersCheck *pCheck)
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
	AString *token = o->gettoken();
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
		u->SetFlag(FLAG_NOCROSS_WATER, val);
	}
}


void Game::ProcessHoldOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
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
		u->SetFlag(FLAG_HOLDING, val);
	}
}

void Game::ProcessAutoTaxOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
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
		u->SetFlag(FLAG_AUTOTAX, val);
	}
}

void Game::ProcessAvoidOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString *token = o->gettoken();
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

Unit *Game::ProcessFormOrder(Unit *former, AString *o, OrdersCheck *pCheck, int isquiet)
{
//modified by BS to allow mistaken form orders

	AString *t = o->gettoken();
	int an = 0;
//	int error = 0;
	if(t) {
        an = t->value();
	    delete t;
    }
	
/*	if (!an) {
		if(pCheck) {
            ParseError(pCheck, former, 0, "No alias in FORM order.");
		    return 0;
        }
	}*/
	if(pCheck) {
		Unit *retval = new Unit;
		retval->former = former;
		return retval;
	} else {
		if(an && former->object->region->GetUnitAlias(an, former->faction->num)) {
			if(!isquiet) former->Error("Alias multiply defined.");
			an = 0;
		}
		Unit *temp = GetNewUnit(former->faction, an);
		temp->CopyFlags(former);
		temp->DefaultOrders(former->object);
		temp->MoveUnit(former->object);
		temp->former = former;
		return temp;
	}
}

void Game::ProcessAddressOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* This is an instant order */
	AString *token = o->gettoken();
	if (token) {
		if(!pCheck) {
			u->faction->address = token;
		}
	} else {
		ParseError(pCheck, u, 0, "ADDRESS: No address given.");
	}
}

void Game::ProcessFollowOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	AString *token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "FOLLOW: Specify to follow a UNIT or SHIP");
		return;
	}
    FollowOrder *ord = 0;
	if(*token == "OBJECT" || *token == "SHIP") {
	    delete token;
	    token = o->gettoken();
	    int shipnum = token->value();
	    delete token;
	    if(shipnum < 1) {
    		ParseError(pCheck, u, 0, "FOLLOW: Invalid ship number.");
    		return;
	    }
	    ord = new FollowOrder;
    	ord->targetid = 0;
    	ord->ship = shipnum;
	    ord->quiet = isquiet;
	} else if(*token == "UNIT" || *token == "UNITS") {
	    delete token;
    	UnitId *t = ParseUnit(o);
    	if (!t) {
    		ParseError(pCheck, u, 0, "FOLLOW: Invalid target.");
    		return;
    	}
    	ord = new FollowOrder;
    	ord->targetid = t;
    	ord->ship = 0;
	    ord->quiet = isquiet;
	} else {
	    delete token;
	    ParseError(pCheck, u, 0, "FOLLOW: Specify to follow a UNIT or SHIP");
	    return;
	}   
    
	if (u->monthorders || (Globals->TAX_PILLAGE_MONTH_LONG &&
		 ((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		AString err = "FOLLOW: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long orders";
		ParseError(pCheck, u, 0, err);
		if (u->monthorders) delete u->monthorders;
		u->monthorders = 0;
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	u->monthorders = ord;
}

void Game::ProcessAdvanceOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
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
		u->monthorders->quiet = isquiet;
	}
	m = (MoveOrder *) u->monthorders;
	m->advancing = 1;

	for (;;) {
		AString *t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d!=-1) {
			if(!pCheck) {
				MoveDir *x = new MoveDir;
				x->dir = d;
				m->dirs.Add(x);
			}
		} else {
			ParseError(pCheck, u, 0, "ADVANCE: Warning, bad direction.");
			return;
		}
	}
}

void Game::ProcessMoveOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
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
		u->monthorders->quiet = isquiet;
	}
	m = (MoveOrder *) u->monthorders;
	m->advancing = 0;

	for (;;) {
		AString *t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d!=-1) {
			if(!pCheck) {
				MoveDir *x = new MoveDir;
				x->dir = d;
				m->dirs.Add(x);
			}
		} else {
			ParseError(pCheck, u, 0, "MOVE: Warning, bad direction.");
			return;
		}
	}
}

void Game::ProcessSailOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	SailOrder *ord = 0;

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
		u->monthorders->quiet = isquiet;
	}
	ord = (SailOrder *) u->monthorders;

	for (;;) {
		AString *t = o->gettoken();
		if (!t) return;
		int d = ParseDir(t);
		delete t;
		if (d == -1) {
			ParseError(pCheck, u, 0, "SAIL: Warning, bad direction.");
			return;
		} else {
			if (d < NDIRS) {
				if(!pCheck) {
					MoveDir *x = new MoveDir;
					x->dir = d;
					ord->dirs.Add(x);
				}
			} else {
				return;
			}
		}
	}
}

void Game::ProcessEvictOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
	UnitId *id = ParseUnit(o);
	while (id && id->unitnum != -1) {
		if(!pCheck) {
			if (!u->evictorders) u->evictorders = new EvictOrder;
			u->evictorders->targets.Add(id);
			u->evictorders->quiet = isquiet;
		}
		id = ParseUnit(o);
	}
}

void Game::ProcessIdleOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	if (u->monthorders || (Globals->TAX_PILLAGE_MONTH_LONG &&
		((u->taxing == TAX_TAX) || (u->taxing == TAX_PILLAGE)))) {
		if (u->monthorders) delete u->monthorders;
		AString err = "IDLE: Overwriting previous ";
		if (u->inTurnBlock) err += "DELAYED ";
		err += "month-long order.";
		ParseError(pCheck, u, 0, err);
	}
	if(Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
	IdleOrder *i = new IdleOrder;
	u->monthorders = i;
}

void Game::ProcessTransportOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
    if(!(Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT)) {
        ParseError(pCheck, u, 0, "TRANSPORT: Invalid order.");
        return;
    }
     
	UnitId *tar = ParseUnit(o);
	if (!tar) {
		ParseError(pCheck, u, 0, "TRANSPORT: Invalid target.");
		return;
	}
	AString *token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "TRANSPORT: No amount given.");
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
		ParseError(pCheck, u, 0, "TRANSPORT: No item given.");
		return;
	}
	int item = ParseTransportableItem(token);
	delete token;
	if (item == -1) {
		ParseError(pCheck, u, 0, "TRANSPORT: Invalid item.");
		return;
	}

	int except = 0;
	token = o->gettoken();
	if (token && *token == "except") {
		delete token;
		token = o->gettoken();
		if (!token) {
			ParseError(pCheck, u, 0, "TRANSPORT: EXCEPT requires a value.");
			return;
		}
		except = token->value();
		delete token;
		if (except <= 0) {
			ParseError(pCheck, u, 0, "TRANSPORT: Invalid except value.");
			return;
		}
	}

	if (!pCheck) {
		TransportOrder *ord = new TransportOrder;
		ord->item = item;
		ord->target = tar;
		ord->amount = amt;
		ord->except = except;
	    ord->quiet = isquiet;
		u->transportorders.Add(ord);
	}
	return;
}

void Game::ProcessDistributeOrder(Unit *u, AString *o, OrdersCheck *pCheck, int isquiet)
{
    if(!(Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT)) {
        ParseError(pCheck, u, 0, "DISTRIBUTE: Invalid order.");
        return;
    }
    
    	UnitId *tar = ParseUnit(o);
	if (!tar) {
		ParseError(pCheck, u, 0, "DISTRIBUTE: Invalid target.");
		return;
	}
	AString *token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "DISTRIBUTE: No amount given.");
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
		ParseError(pCheck, u, 0, "DISTRIBUTE: No item given.");
		return;
	}
	int item = ParseTransportableItem(token);
	delete token;
	if (item == -1) {
		ParseError(pCheck, u, 0, "DISTRIBUTE: Invalid item.");
		return;
	}

	int except = 0;
	token = o->gettoken();
	if (token && *token == "except") {
		delete token;
		token = o->gettoken();
		if (!token) {
			ParseError(pCheck, u, 0, "DISTRIBUTE: EXCEPT requires a value.");
			return;
		}
		except = token->value();
		delete token;
		if (except <= 0) {
			ParseError(pCheck, u, 0, "DISTRIBUTE: Invalid except value.");
			return;
		}
	}

	if (!pCheck) {
		TransportOrder *ord = new TransportOrder;
		ord->type = O_DISTRIBUTE;
		ord->item = item;
		ord->target = tar;
		ord->amount = amt;
		ord->except = except;
	    ord->quiet = isquiet;
		u->transportorders.Add(ord);
	}
	return;
}

void Game::ProcessShareOrder(Unit *u, AString *o, OrdersCheck *pCheck)
{
	/* Instant order */
	AString *token = o->gettoken();
	if (!token) {
		ParseError(pCheck, u, 0, "SHARE: Invalid value.");
		return;
	}
	int val = ParseTF(token);
	delete token;
	if (val==-1) {
		ParseError(pCheck, u, 0, "SHARE: Invalid value.");
		return;
	}
	if(!pCheck) {
		u->SetFlag(FLAG_SHARING, val);
	}
}

#include <stdlib.h>

#include "game.h"

#include "logger.hpp"
#include "orders.h"
#include "skills.h"
#include "gamedata.h"
#include "indenter.hpp"
#include "string_parser.hpp"
#include "string_filters.hpp"

#include <string>

using namespace std;

void orders_check::error(const std::string& err)
{
    check_file << indent::push_indent(0) <<  "\n\n*** Error: " << err << " ***\n" << indent::pop_indent();
    numerrors++;
}

int Game::parse_dir(const parser::token& token)
{
    for (int i=0; i<NDIRS; i++) {
        if (token == DirectionStrs[i]) return i;
        if (token == DirectionAbrs[i]) return i;
    }
    if (token == "in") return MOVE_IN;
    if (token == "out") return MOVE_OUT;
    if (token == "pause" || token == "p") return MOVE_PAUSE;
    int num = token.get_number().value_or(0);
    if (num) return MOVE_ENTER + num;
    return -1;
}


UnitId *Game::parse_unit(parser::string_parser& parser)
{
    parser::token token = parser.get_token();
    if (!token) return nullptr;

    if (token == "0") {
        UnitId *id = new UnitId;
        id->unitnum = -1;
        id->alias = 0;
        id->faction = 0;
        return id;
    }

    if (token == "faction") {
        int faction_id = parser.get_token().get_number().value_or(0);
        if (!faction_id) return nullptr;

        if (parser.get_token() != "new") return nullptr;

        int unit_alias = parser.get_token().get_number().value_or(0);
        if (!unit_alias) return nullptr;

        /* Return UnitId */
        UnitId *id = new UnitId;
        id->unitnum = 0;
        id->alias = unit_alias;
        id->faction = faction_id;
        return id;
    }

    if (token == "new") {
        int unit_alias = parser.get_token().get_number().value_or(0);
        if (!unit_alias) return nullptr;

        UnitId *id = new UnitId;
        id->unitnum = 0;
        id->alias = unit_alias;
        id->faction = 0;
        return id;
    }

    int unit_id = token.get_number().value_or(0);
    if (!unit_id) return nullptr;

    UnitId *id = new UnitId;
    id->unitnum = unit_id;
    id->alias = 0;
    id->faction = 0;
    return id;
}

int parse_faction_type(parser::string_parser& parser, std::unordered_map<std::string, int> &type)
{
    for (auto &ft : *FactionTypes) {
        type[ft] = 0;
    }

    parser::token token = parser.get_token();
    if (!token) return -1;

    if (token == "generic") {
        for (auto &ft : *FactionTypes) {
            type[ft] = 1;
        }

        return 0;
    }

    while(token) {
        bool foundone = false;

        for (auto &ft : *FactionTypes) {
            if (token == ft.c_str()) {
                token = parser.get_token();
                if (!token) return -1;
                type[ft] = token.get_number().value_or(0);
                foundone = true;
                break;
            }
        }
        if (!foundone) return -1;

        token = parser.get_token();
    }

    int tot = 0;
    for (auto &kv : type) {
        tot += kv.second;
    }
    if (tot > Globals->FACTION_POINTS) return -1;

    return 0;
}

void Game::parse_error(orders_check *checker, Unit *unit, Faction *faction, const std::string& error) {
    if (checker) checker->error(error);
    else if (unit) unit->error(error);
    else if (faction) faction->error(error);
}

void Game::overwrite_month_warning(std::string type, Unit *u, orders_check * checker) {
    string err = type + ": Overwriting previous " + string(u->inTurnBlock ? "DELAYED " : "") + "month-long order.";
    parse_error(checker, u, 0, err);
}

void Game::ParseOrders(int faction, std::istream& f, orders_check *checker)
{
    Faction *fac = nullptr;
    Faction *passFac = nullptr;
    Unit *unit = nullptr;
    int code;
    parser::string_parser order;
    AString prefix;

    f >> std::ws >> order;
    while (!f.eof()) {
        bool repeating = order.get_at();
        auto token = order.get_token();

        if (token) {
            code = Parse1Order(token);
            switch (code) {
            case -1:
                parse_error(checker, unit, fac, token.get_string() + " is not a valid order.");
                break;
            case O_ATLANTIS:
                if (fac) parse_error(checker, 0, fac, "No #END statement given.");
                token = order.get_token();
                if (!token) {
                    parse_error(checker, 0, 0, "No faction number given on #atlantis line.");
                    fac = 0;
                    break;
                }
                if (checker) {
                    fac = &(checker->dummyFaction);
                    checker->numshows = 0;
                    // Even though we don't use the real faction for other things, we do want it for the password check.
                    passFac = GetFaction(factions, token.get_number().value_or(0));
                } else {
                    fac = GetFaction(factions, token.get_number().value_or(0));
                }

                if (!fac) break;

                token = order.get_token();

                if (checker) {
                    if (!token) {
                        parse_error(checker, 0, fac, "Warning: No password on #atlantis line.");
                        parse_error(checker, 0, fac, "If this is your first turn, ignore this error.");
                    } else {
                        // If we found their real faction above (we should have but let's not assume), then we
                        // can check if they gave us the correct password.
                        if (passFac) {
                            std::string fac_pass = passFac->password;
                            bool has_password = (fac_pass != "none");
                            bool wrong_password = (token != fac_pass);
                            if (has_password && wrong_password) {
                                parse_error(checker, 0, fac, "Incorrect password on #atlantis line.");
                                fac = 0;
                                break;
                            }
                        }
                    }
                } else {
                    std::string fac_pass = fac->password;
                    if (fac_pass != "none") {
                        if (!token || (token != fac_pass)) {
                            parse_error(checker, 0, fac, "Incorrect password on #atlantis line.");
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
                if (checker) checker->check_file << indent::clear;
                while (unit) {
                    Unit *former = unit->former;
                    if (unit->inTurnBlock)
                        parse_error(checker, unit, fac, "TURN: without ENDTURN");
                    if (unit->former)
                        parse_error(checker, unit, fac, "FORM: without END.");
                    if (unit && checker) unit->ClearOrders();
                    if (checker && former) delete unit;
                    unit = former;
                }

                unit = 0;
                fac = 0;
                break;

            case O_UNIT:
                if (checker) checker->check_file << indent::clear;
                if (fac) {
                    while (unit) {
                        Unit *former = unit->former;
                        if (unit->inTurnBlock) parse_error(checker, unit, fac, "TURN: without ENDTURN");
                        if (unit->former) parse_error(checker, unit, fac, "FORM: without END.");
                        if (unit && checker) unit->ClearOrders();
                        if (checker && former) delete unit;
                        unit = former;
                    }
                    unit = nullptr;
                    token = order.get_token();
                    if (!token) {
                        parse_error(checker, 0, fac, "UNIT without unit number.");
                        unit = 0;
                        break;
                    }

                    if (checker) {
                        if (!token.get_number()) {
                            parse_error(checker, 0, fac, "Invalid unit number.");
                        } else {
                            unit = &(checker->dummyUnit);
                            unit->monthorders = nullptr;
                        }
                    } else {
                        unit = GetUnit(token.get_number().value_or(-1));
                        if (!unit || unit->faction != fac) {
                            fac->error(token.get_string() + " is not your unit.");
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
                            parse_error(checker, unit, fac, "FORM: cannot nest.");
                        }
                        else {
                            unit = ProcessFormOrder(unit, order, checker, repeating);
                            if (!checker && unit && unit->former && unit->former->form_repeated)
                                unit->former->oldorders.push_back(order.original());
                            if (!checker) {
                                if (unit) unit->ClearOrders();
                            }
                        }
                    } else {
                        parse_error(checker, 0, fac, "Order given without a unit selected.");
                    }
                }
                break;
            case O_ENDFORM:
                if (fac) {
                    if (unit && unit->former) {
                        Unit *former = unit->former;

                        if (unit->inTurnBlock)
                            parse_error(checker, unit, fac, "TURN: without ENDTURN");

                        if (!checker && unit->former && unit->former->form_repeated)
                            unit->former->oldorders.push_back(order.original());

                        if (checker && former) delete unit;
                        unit = former;
                    } else {
                        parse_error(checker, unit, fac, "END: without FORM.");
                    }
                }
                break;
            case O_TURN:
                if (unit && unit->inTurnBlock) {
                    parse_error(checker, unit, fac, "TURN: cannot nest");
                } else if (!unit)
                    parse_error(checker, 0, fac, "Order given without a unit selected.");
                else {
                    // faction is 0 if checking syntax only, not running turn.
                    if (faction != 0) {
                        if (!checker && unit->former && unit->former->form_repeated)
                            unit->former->oldorders.push_back(order.original());
                        order = ProcessTurnOrder(unit, f, checker, repeating);
                        if (!order.empty()) continue;
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
                    if (!checker && unit->former && unit->former->form_repeated)
                        unit->former->oldorders.push_back(order.original());
                } else
                    parse_error(checker, unit, fac, "ENDTURN: without TURN.");
                break;
            default:
                if (fac) {
                    if (unit) {
                        if (!checker && repeating)
                            unit->oldorders.push_back(order.original());
                        if (!checker && unit->former && unit->former->form_repeated)
                            unit->former->oldorders.push_back(order.original());

                        ProcessOrder(code, unit, order, checker, repeating);
                    } else {
                        parse_error(checker, 0, fac,
                                "Order given without a unit selected.");
                    }
                }
            }
        } else {
            code = NORDERS;
            if (!checker) {
                if (repeating && fac && unit)
                    unit->oldorders.push_back(order.original());
            }
        }

        if (checker) {
            if (code == O_ENDTURN || code == O_ENDFORM)
                checker->check_file << indent::incr;
            checker->check_file << prefix << order.original() << "\n";
            if (code == O_TURN || code == O_FORM)
                checker->check_file << indent::decr;
        }
        f >> std::ws >> order;
    }

    while (unit) {
        Unit *former = unit->former;
        if (unit->inTurnBlock)
            parse_error(checker, 0, fac, "TURN: without ENDTURN");
        if (unit->former)
            parse_error(checker, 0, fac, "FORM: without END.");
        if (unit && checker) unit->ClearOrders();
        if (checker && former) delete unit;
        unit = former;
    }

    if (unit && checker) {
        unit->ClearOrders();
        unit = 0;
    }

    if (checker) {
        checker->check_file << indent::clear << "\n";
        if (!checker->numerrors) {
            checker->check_file << "No errors found.\n";
        } else {
            checker->check_file << checker->numerrors << " error" << (checker->numerrors == 1 ? "" : "s") << " found!\n";
        }
    }
}

void Game::ProcessOrder(int order, Unit *unit, parser::string_parser& parser, orders_check *checker, bool repeating)
{
    switch(order) {
        case O_ADDRESS:
            ProcessAddressOrder(unit, parser, checker);
            break;
        case O_ADVANCE:
            ProcessAdvanceOrder(unit, parser, checker);
            break;
        case O_ASSASSINATE:
            ProcessAssassinateOrder(unit, parser, checker);
            break;
        case O_ATTACK:
            ProcessAttackOrder(unit, parser, checker);
            break;
        case O_AUTOTAX:
            ProcessAutoTaxOrder(unit, parser, checker);
            break;
        case O_AVOID:
            ProcessAvoidOrder(unit, parser, checker);
            break;
        case O_IDLE:
            ProcessIdleOrder(unit, checker);
            break;
        case O_BEHIND:
            ProcessBehindOrder(unit, parser, checker);
            break;
        case O_BUILD:
            ProcessBuildOrder(unit, parser, checker);
            break;
        case O_BUY:
            ProcessBuyOrder(unit, parser, checker, repeating);
            break;
        case O_CAST:
            ProcessCastOrder(unit, parser, checker);
            break;
        case O_CLAIM:
            ProcessClaimOrder(unit, parser, checker);
            break;
        case O_COMBAT:
            ProcessCombatOrder(unit, parser, checker);
            break;
        case O_CONSUME:
            ProcessConsumeOrder(unit, parser, checker);
            break;
        case O_DECLARE:
            ProcessDeclareOrder(unit->faction, parser, checker);
            break;
        case O_DESCRIBE:
            ProcessDescribeOrder(unit, parser, checker);
            break;
        case O_DESTROY:
            ProcessDestroyOrder(unit, checker);
            break;
        case O_ENTER:
            ProcessEnterOrder(unit, parser, checker);
            break;
        case O_ENTERTAIN:
            ProcessEntertainOrder(unit, checker);
            break;
        case O_EVICT:
            ProcessEvictOrder(unit, parser, checker);
            break;
        case O_EXCHANGE:
            ProcessExchangeOrder(unit, parser, checker);
            break;
        case O_FACTION:
            ProcessFactionOrder(unit, parser, checker);
            break;
        case O_FIND:
            ProcessFindOrder(unit, parser, checker);
            break;
        case O_FORGET:
            ProcessForgetOrder(unit, parser, checker);
            break;
        case O_WITHDRAW:
            ProcessWithdrawOrder(unit, parser, checker);
            break;
        case O_GIVE:
            ProcessGiveOrder(order, unit, parser, checker);
            break;
        case O_GUARD:
            ProcessGuardOrder(unit, parser, checker);
            break;
        case O_HOLD:
            ProcessHoldOrder(unit, parser, checker);
            break;
        case O_JOIN:
            ProcessJoinOrder(unit, parser, checker);
            break;
        case O_LEAVE:
            ProcessLeaveOrder(unit, checker);
            break;
        case O_MOVE:
            ProcessMoveOrder(unit, parser, checker);
            break;
        case O_NAME:
            ProcessNameOrder(unit, parser, checker);
            break;
        case O_NOAID:
            ProcessNoaidOrder(unit, parser, checker);
            break;
        case O_NOCROSS:
            ProcessNocrossOrder(unit, parser, checker);
            break;
        case O_NOSPOILS:
            ProcessNospoilsOrder(unit, parser, checker);
            break;
        case O_OPTION:
            ProcessOptionOrder(unit, parser, checker);
            break;
        case O_PASSWORD:
            ProcessPasswordOrder(unit, parser, checker);
            break;
        case O_PILLAGE:
            ProcessPillageOrder(unit, checker);
            break;
        case O_PREPARE:
            ProcessPrepareOrder(unit, parser, checker);
            break;
        case O_WEAPON:
            ProcessWeaponOrder(unit, parser, checker);
            break;
        case O_ARMOR:
            ProcessArmorOrder(unit, parser, checker);
            break;
        case O_PRODUCE:
            ProcessProduceOrder(unit, parser, checker);
            break;
        case O_PROMOTE:
            ProcessPromoteOrder(unit, parser, checker);
            break;
        case O_QUIT:
            ProcessQuitOrder(unit, parser, checker);
            break;
        case O_RESTART:
            ProcessRestartOrder(unit, parser, checker);
            break;
        case O_REVEAL:
            ProcessRevealOrder(unit, parser, checker);
            break;
        case O_SAIL:
            ProcessSailOrder(unit, parser, checker);
            break;
        case O_SELL:
            ProcessSellOrder(unit, parser, checker, repeating);
            break;
        case O_SHARE:
            ProcessShareOrder(unit, parser, checker);
            break;
        case O_SHOW:
            ProcessReshowOrder(unit, parser, checker);
            break;
        case O_SPOILS:
            ProcessSpoilsOrder(unit, parser, checker);
            break;
        case O_STEAL:
            ProcessStealOrder(unit, parser, checker);
            break;
        case O_STUDY:
            ProcessStudyOrder(unit, parser, checker);
            break;
        case O_TAKE:
            ProcessGiveOrder(order, unit, parser, checker);
            break;
        case O_TAX:
            ProcessTaxOrder(unit, checker);
            break;
        case O_TEACH:
            ProcessTeachOrder(unit, parser, checker);
            break;
        case O_WORK:
            ProcessWorkOrder(unit, 0, checker);
            break;
        case O_TRANSPORT:
        case O_DISTRIBUTE:
            ProcessTransportOrder(unit, parser, checker);
            break;
        // NO7 victory condition orders
        case O_ANNIHILATE:
            ProcessAnnihilateOrder(unit, parser, checker);
            break;
        case O_SACRIFICE:
            ProcessSacrificeOrder(unit, parser, checker);
            break;
    }
}

void Game::ProcessPasswordOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (checker) return;

    parser::token token = parser.get_token();
    if (token) {
        u->faction->password = token.get_string();
        std::string message = "Password is now: ";
        message += token.get_string();
        u->faction->event(message, "password");
    } else {
        u->faction->password = "none";
        u->faction->event("Password cleared.", "password");
    }
}

void Game::ProcessOptionOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "OPTION: What option?");
        return;
    }

    if (token == "times") {
        if (!checker) {
            u->faction->event("Times will be sent to your faction.", "option");
            u->faction->times = 1;
        }
        return;
    }

    if (token == "notimes") {
        if (!checker) {
            u->faction->event("Times will not be sent to your faction.", "option");
            u->faction->times = 0;
        }
        return;
    }

    if (token == "showattitudes") {
        if (!checker) {
            u->faction->event("Units will now have a leading sign to show your attitude to them.", "option");
            u->faction->showunitattitudes = 1;
        }
        return;
    }

    if (token == "dontshowattitudes") {
        if (!checker) {
            u->faction->event("Units will now have a leading minus sign regardless of your attitude to them.", "option");
            u->faction->showunitattitudes = 0;
        }
        return;
    }

    if (token == "template") {
        token = parser.get_token();
        if (!token) {
            parse_error(checker, u, 0, "OPTION: No template type specified.");
            return;
        }
        int newformat = parse_template_type(token);
        if (newformat == -1) {
            parse_error(checker, u, 0, "OPTION: Invalid template type.");
            return;
        }

        if (!checker) u->faction->temformat = newformat;
        return;
    }
    parse_error(checker, u, 0, "OPTION: Invalid option.");
}

void Game::ProcessReshowOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "SHOW: Show what?");
        return;
    }

    int count = (checker ? checker->numshows++ : u->faction->numshows++);
    if (count > 100) {
        if (count == 102) parse_error(checker, u, 0, "Too many SHOW orders.");
        return;
    }

    if (token == "skill") {
        token = parser.get_token();
        if (!token) {
            parse_error(checker, u, 0, "SHOW: Show what skill?");
            return;
        }
        int skill = parse_skill(token);

        token = parser.get_token();
        if (!token) {
            parse_error(checker, u, 0, "SHOW: No skill level given.");
            return;
        }
        int level = token.get_number().value_or(0);

        if (checker) return;

        if (skill == -1 ||
            SkillDefs[skill].flags & SkillType::DISABLED ||
            (SkillDefs[skill].flags & SkillType::APPRENTICE && !Globals->APPRENTICES_EXIST) ||
            level > u->faction->skills.GetDays(skill)) {
            u->error("SHOW: Faction doesn't have that skill.");
            return;
        }

        u->faction->shows.push_back({ .skill = skill, .level = level });
        return;
    }

    bool ship_object = false;
    int obj = -1;

    if (token == "object") {
        token = parser.get_token();
        if (!token) {
            parse_error(checker, u, 0, "SHOW: Show which object?");
            return;
        }

        if (checker) return;

        obj = parse_object(token, true);

        if (obj >= -1) {
            if (obj == -1 || obj == O_DUMMY ||
               (ObjectDefs[obj].flags & ObjectType::DISABLED)) {
                u->error("SHOW: No such object.");
                return;
            }
            u->faction->objectshows.push_back({.obj = obj});
            return;
        }

        ship_object = true;
    }

    if (token == "item" || ship_object) {
        int item_type;

        if (ship_object) {
            item_type = -(obj + 1);
        } else {
            token = parser.get_token();
            if (!token) {
                parse_error(checker, u, 0, "SHOW: Show which item?");
                return;
            }
            item_type = parse_enabled_item(token);
        }

        if (checker) return;

        if (item_type == -1 || (ItemDefs[item_type].flags & ItemType::DISABLED)) {
            u->error("SHOW: You don't know anything about that item.");
            return;
        }

        if (ItemDefs[item_type].pSkill) {
            int skill = lookup_skill(ItemDefs[item_type].pSkill);
            if (ItemDefs[item_type].pLevel <= u->faction->skills.GetDays(skill)) {
                u->faction->DiscoverItem(item_type, 1, 1);
                return;
            }
        }

        if (ItemDefs[item_type].mSkill) {
            int skill = lookup_skill(ItemDefs[item_type].mSkill);
            if (ItemDefs[item_type].mLevel <= u->faction->skills.GetDays(skill)) {
                u->faction->DiscoverItem(item_type, 1, 1);
                return;
            }
        }

        if (u->faction->items.GetNum(item_type)) {
            u->faction->DiscoverItem(item_type, 1, 0);
            return;
        }

        if (ItemDefs[item_type].type & (IT_MAN | IT_NORMAL | IT_TRADE | IT_MONSTER)) {
            u->faction->DiscoverItem(item_type, 1, 0);
            return;
        }

        u->error("SHOW: You don't know anything about that item.");
        return;
    }

    parse_error(checker, u, 0, "SHOW: Show what?");
}

void Game::ProcessForgetOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "FORGET: No skill given.");
        return;
    }

    int sk = parse_skill(token);
    if (sk == -1) {
        parse_error(checker, u, 0, "FORGET: Invalid skill.");
        return;
    }

    if (checker) return;

    ForgetOrder *ord = new ForgetOrder;
    ord->skill = sk;
    u->forgetorders.push_back(ord);
}

void Game::ProcessEntertainOrder(Unit *unit, orders_check *checker)
{
    if (
        unit->monthorders ||
        (Globals->TAX_PILLAGE_MONTH_LONG && (unit->taxing == TAX_TAX || unit->taxing == TAX_PILLAGE))
    ) {
        overwrite_month_warning("ENTERTAIN", unit, checker);
        if (unit->monthorders) delete unit->monthorders;
    }

    if (Globals->TAX_PILLAGE_MONTH_LONG) unit->taxing = TAX_NONE;

    ProduceOrder *order = new ProduceOrder;
    order->item = I_SILVER;
    order->skill = S_ENTERTAINMENT;
    order->target = 0;
    unit->monthorders = order;
}

/**
 * Process COMBAT order: Sets a unit's combat spell
 * Format: COMBAT [skill]
 * If no skill is given, clears the combat spell.
 */
void Game::ProcessCombatOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        if (!checker) {
            u->combat = -1;
            u->event("Combat spell set to none.", "combat_preparation");
        }
        return;
    }

    int sk = parse_skill(token);

    if (sk == -1) {
        parse_error(checker, u, 0, "COMBAT: Invalid skill.");
        return;
    }

    if (!(SkillDefs[sk].flags & SkillType::MAGIC)) {
        parse_error(checker, u, 0, "COMBAT: That is not a magic skill.");
        return;
    }

    if (!(SkillDefs[sk].flags & SkillType::COMBAT)) {
        parse_error(checker, u, 0, "COMBAT: That skill cannot be used in combat.");
        return;
    }

    if (checker) return;

    if (u->type != U_MAGE) {
        u->error("COMBAT: That unit is not a mage.");
        return;
    }

    if (!u->GetSkill(sk)) {
        u->error("COMBAT: Unit does not possess that skill.");
        return;
    }

    u->combat = sk;
    std::string message = "Combat spell set to " + SkillDefs[sk].name;
    if (Globals->USE_PREPARE_COMMAND) {
        u->readyItem = -1;
        message += " and prepared item set to none";
    }
    message += ".";
    u->event(message, "combat_preparation");
}

/**
 * Process PREPARE order (Lacandon's prepare command): Sets a unit's combat item
 * Format: PREPARE [item]
 * If no item is given, clears the prepared item.
 */
void Game::ProcessPrepareOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (!Globals->USE_PREPARE_COMMAND) {
        parse_error(checker, u, 0, "PREPARE is not a valid order.");
        return;
    }

    parser::token token = parser.get_token();
    if (!token) {
        if (!checker) {
            u->readyItem = -1;
            u->event("Prepared battle item set to none.", "combat_preparation");
        }
        return;
    }

    int it = parse_enabled_item(token);
    auto bt = FindBattleItem(ItemDefs[it].abr.c_str());

    if (checker) return;

    if (it == -1 || u->items.GetNum(it) < 1) {
        u->error("PREPARE: Unit does not possess that item.");
        return;
    }

    if (!bt || !(bt->get().flags & BattleItemType::SPECIAL)) {
        u->error("PREPARE: That item cannot be prepared.");
        return;
    }

    if ((bt->get().flags & BattleItemType::MAGEONLY) &&
        !(u->type == U_MAGE || u->type == U_APPRENTICE || u->type == U_GUARDMAGE)) {

        std::string err = "PREPARE: Only a mage ";
        if (Globals->APPRENTICES_EXIST)
            err += "or " + std::string(Globals->APPRENTICE_NAME);
        err += "may use that item.";

        u->error(err);
        return;
    }

    u->readyItem = it;
    std::string message = "Prepared item set to " + ItemDefs[it].name;

    if (u->combat != -1) {
        u->combat = -1;
        message += " and combat spell set to none";
    }

    message += ".";
    u->event(message, "combat_preparation");
}

void Game::ProcessWeaponOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (!(Globals->USE_WEAPON_ARMOR_COMMAND)) {
        parse_error(checker, u, 0, "WEAPON is not a valid order.");
        return;
    }

    parser::token token = parser.get_token();
    if (!token) {
        if (!checker) {
            for (int i = 0; i < MAX_READY; ++i) u->readyWeapon[i] = -1;
            u->event("Preferred weapons set to none.", "combat_preparation");
        }
        return;
    }
    if (checker) return;

    int items[MAX_READY];
    int i = 0;
    while (token && (i < MAX_READY)) {
        int it = parse_enabled_item(token);
        if (it == -1 || u->faction->items.GetNum(it) < 1) {
            u->error("WEAPON: Unknown item.");
        } else if (!(ItemDefs[it].type & IT_WEAPON)) {
            u->error("WEAPON: Item is not a weapon.");
        } else {
            items[i++] = it;
        }
        token = parser.get_token();
    }

    while (i < MAX_READY) {
        items[i++] = -1;
    }
    if (items[0] == -1) return;
    std::string message = "Preferred weapons set to: ";
    for (i=0; i<MAX_READY;++i) {
        u->readyWeapon[i] = items[i];
        if (items[i] != -1) {
            if (i > 0) message += ", ";
            message += ItemDefs[items[i]].name;
        }
    }
    message += ".";
    u->event(message, "combat_preparation");
}

void Game::ProcessArmorOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (!(Globals->USE_WEAPON_ARMOR_COMMAND)) {
        parse_error(checker, u, 0, "ARMOR is not a valid order.");
        return;
    }

    parser::token token = parser.get_token();
    if (!token) {
        if (!checker) {
            for (int i = 0; i < MAX_READY; ++i) u->readyArmor[i] = -1;
            u->event("Preferred armor set to none.", "combat_preparation");
        }
        return;
    }
    if (checker) return;

    int items[MAX_READY];
    int i = 0;
    while (token && (i < MAX_READY)) {
        int it = parse_enabled_item(token);
        if (it == -1 || u->faction->items.GetNum(it) < 1) {
            u->error("ARMOR: Unknown item.");
        } else if (!(ItemDefs[it].type & IT_ARMOR)) {
            u->error("ARMOR: Item is not armor.");
        } else {
            items[i++] = it;
        }
        token = parser.get_token();
    }

    while (i < MAX_READY) {
        items[i++] = -1;
    }
    if (items[0] == -1) return;
    std::string message = "Preferred armor set to: ";
    for (i=0; i<MAX_READY;++i) {
        u->readyArmor[i] = items[i];
        if (items[i] != -1) {
            if (i > 0) message += ", ";
            message += ItemDefs[items[i]].name;
        }
    }
    message += ".";
    u->event(message, "combat_preparation");
}

void Game::ProcessClaimOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "CLAIM: No amount given.");
        return;
    }

    int value = token.get_number().value_or(0);
    if (!value) {
        parse_error(checker, u, 0, "CLAIM: No amount given.");
        return;
    }

    if (checker) return;

    if (value > u->faction->unclaimed) {
        u->error("CLAIM: Don't have that much unclaimed silver.");
        value = u->faction->unclaimed;
    }
    u->faction->unclaimed -= value;
    u->SetMoney(u->GetMoney() + value);
    u->faction->DiscoverItem(I_SILVER, 0, 1);
    u->event("Claims $" + to_string(value) + ".", "claim");
}

void Game::ProcessFactionOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (Globals->FACTION_LIMIT_TYPE != GameDefs::FACLIM_FACTION_TYPES) {
        parse_error(checker, u, 0, "FACTION: Invalid order, no faction types in this game.");
        return;
    }

    std::unordered_map<std::string, int> oldfactype;
    std::unordered_map<std::string, int> factype;

    if (!checker) {
        // copy current values into temp variable
        oldfactype = u->faction->type;
    }

    int retval = parse_faction_type(parser, factype);
    if (retval == -1) {
        parse_error(checker, u, 0, "FACTION: Bad faction type.");
        return;
    }

    if (checker) return;

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

void Game::ProcessAssassinateOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    UnitId *id = parse_unit(parser);
    if (!id || id->unitnum == -1) {
        if (id) delete id;
        parse_error(checker, u, 0, "ASSASSINATE: No target given.");
        return;
    }

    if (checker) {
        delete id;
        return;
    }

    if (u->stealthorders) delete u->stealthorders;
    AssassinateOrder *ord = new AssassinateOrder;
    ord->target = id;
    u->stealthorders = ord;
}

void Game::ProcessStealOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    UnitId *id = parse_unit(parser);
    if (!id || id->unitnum == -1) {
        if (id) delete id;
        parse_error(checker, u, 0, "STEAL: No target given.");
        return;
    }

    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "STEAL: No item given.");
        delete id;
        return;
    }

    if (checker) {
        delete id;
        return;
    }

    int i = parse_enabled_item(token);

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

void Game::ProcessQuitOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (checker) return;

    if (u->faction->password != "none") {
        if (parser.get_token().get_string() != u->faction->password) {
            u->faction->error("QUIT: Must give the correct password.");
            return;
        }
    }

    if (u->faction->quit != QUIT_AND_RESTART) {
        u->faction->quit = QUIT_BY_ORDER;
    }
}

void Game::ProcessRestartOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (checker) return;


    if (u->faction->password != "none") {
        if (parser.get_token().get_string() != u->faction->password) {
            u->faction->error("RESTART: Must give the correct password.");
            return;
        }
    }

    if (u->faction->quit != QUIT_AND_RESTART) {
        u->faction->quit = QUIT_AND_RESTART;
        Faction *pFac = AddFaction(0, NULL);
        pFac->set_address(u->faction->address);
        pFac->password = u->faction->password;
        string facstr = "Restarting " + pFac->address + ".";
    }
}

void Game::ProcessDestroyOrder(Unit *u, orders_check *checker)
{
    if (!checker) u->destroy = 1;
}

void Game::ProcessFindOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (!Globals->HAVE_EMAIL_SPECIAL_COMMANDS) {
        parse_error(checker, u, 0, "FIND: This command was disabled.");
        return;
    }

    parser::token token = parser.get_token();
    int n = token.get_number().value_or(0);
    int is_all = (token == "all");

    if (n == 0 && !is_all) {
        parse_error(checker, u, 0, "FIND: No faction number given.");
        return;
    }

    if (!checker) {
        FindOrder *f = new FindOrder;
        f->find = n;
        u->findorders.push_back(f);
    }
}

void Game::ProcessConsumeOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();

    if (!token) {
        if (!checker) {
            u->SetFlag(FLAG_CONSUMING_UNIT, 0);
            u->SetFlag(FLAG_CONSUMING_FACTION, 0);
        }
        return;
    }

    if (token == "unit") {
        if (!checker) {
            u->SetFlag(FLAG_CONSUMING_UNIT, 1);
            u->SetFlag(FLAG_CONSUMING_FACTION, 0);
        }
        return;
    }

    if (token == "faction") {
        if (!checker) {
            u->SetFlag(FLAG_CONSUMING_UNIT, 0);
            u->SetFlag(FLAG_CONSUMING_FACTION, 1);
        }
        return;
    }

    if (token == "none") {
        if (!checker) {
            u->SetFlag(FLAG_CONSUMING_UNIT, 0);
            u->SetFlag(FLAG_CONSUMING_FACTION, 0);
        }
        return;
    }

    parse_error(checker, u, 0, "CONSUME: Invalid value.");
}

void Game::ProcessRevealOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();

    if (checker) {
        // Only validate the token during syntax checking
        if (token && token != "unit" && token != "faction" && token != "none") {
            parse_error(checker, u, 0, "REVEAL: Invalid value.");
        }
        return;
    }

    // Default is REVEAL_NONE
    int reveal_type = REVEAL_NONE;

    if (token == "unit") {
        reveal_type = REVEAL_UNIT;
    } else if (token == "faction") {
        reveal_type = REVEAL_FACTION;
    } else if (token && token != "none") {
        u->error("REVEAL: Invalid value.");
        return;
    }

    u->reveal = reveal_type;
}

void Game::ProcessTaxOrder(Unit *u, orders_check *checker)
{
    if (u->taxing == TAX_PILLAGE) {
        parse_error(checker, u, 0, "TAX: The unit is already pillaging.");
        return;
    }
    if (Globals->TAX_PILLAGE_MONTH_LONG && u->monthorders) {
        delete u->monthorders;
        u->monthorders = nullptr;
        overwrite_month_warning("TAX", u, checker);
    }

    u->taxing = TAX_TAX;
}

void Game::ProcessPillageOrder(Unit *u, orders_check *checker)
{
    if (u->taxing == TAX_TAX) {
        parse_error(checker, u, 0, "PILLAGE: The unit is already taxing.");
        return;
    }
    if (Globals->TAX_PILLAGE_MONTH_LONG && u->monthorders) {
        delete u->monthorders;
        u->monthorders = nullptr;
        overwrite_month_warning("PILLAGE", u, checker);
    }

    u->taxing = TAX_PILLAGE;
}

void Game::ProcessPromoteOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    UnitId *target_unit = parse_unit(parser);
    if (!target_unit || target_unit->unitnum == -1) {
        if (target_unit) delete target_unit;
        parse_error(checker, u, 0, "PROMOTE: No target given.");
        return;
    }

    if (checker) {
        delete target_unit;
        return;
    }

    if (u->promote) delete u->promote;

    u->promote = target_unit;
}

void Game::ProcessLeaveOrder(Unit *u, orders_check *checker)
{
    if (checker) return;

    if (u->enter == 0) u->enter = -1;
}

void Game::ProcessEnterOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    int object_num = parser.get_token().get_number().value_or(0);

    if (!object_num) {
        parse_error(checker, u, 0, "ENTER: No object specified.");
        return;
    }

    if (checker) return;

    u->enter = object_num;
}

BuildOrder* Game::ProcessBuildHelp(Unit *unit, parser::string_parser& parser, orders_check *checker)
{
    if (checker) return new BuildOrder;

    UnitId *target = parse_unit(parser);
    if (!target || target->unitnum == -1) {
        if(target) delete target;
        unit->error("BUILD: Non-existent unit to help.");
        return nullptr;
    }

    BuildOrder *order = new BuildOrder;
    order->target = target;
    return order;
}

BuildOrder* Game::ProcessBuildShip(Unit *unit, int object_type, orders_check *checker)
{
    int shipType = std::abs(object_type + 1);

    // the ship type was checked in the caller for being a valid type of item.
    int flying = ItemDefs[shipType].fly;
    ARegion *region = unit->object->region;
    if (!region->IsCoastalOrLakeside() && flying <= 0) {
        unit->error("BUILD: Can't build ship in non-coastal or lakeside region.");
        return nullptr;
    }

    unit->build = -shipType;
    int maxbuild = ItemDefs[shipType].pMonths;

    // If we already have an unfinished ship, see how much work is left
    if (unit->items.GetNum(shipType) > 0) maxbuild = unit->items.GetNum(shipType);

    BuildOrder* order = new BuildOrder;
    // Set needtocomplete if we have a maxbuild value
    if (maxbuild != 0) order->needtocomplete = maxbuild;
    return order;
}

BuildOrder* Game::ProcessBuildStructure(Unit *unit, int object_type, orders_check *checker)
{
    // Object type was checked in the caller for being a valid, enabled object.
    if (!(ObjectDefs[object_type].flags & ObjectType::CANENTER)) {
        unit->error("BUILD: Can't build that.");
        return nullptr;
    }

    int skillRequired = lookup_skill(ObjectDefs[object_type].skill);
    if (skillRequired == -1) {
        unit->error("BUILD: Can't build that.");
        return nullptr;
    }

    BuildOrder* order = new BuildOrder;
    order->new_building = object_type;
    return order;
}

BuildOrder *Game::ProcessBuildObject(Unit *unit, int object_type, orders_check *checker)
{
    if (object_type == -1) {
        parse_error(checker, unit, 0, "BUILD: Not a valid object name.");
        return nullptr;
    }

    // If we get here, we can just treat it as a valid order and bail early.
    if (checker) return new BuildOrder;

    ARegion *region = unit->object->region;
    if (TerrainDefs[region->type].similar_type == R_OCEAN) {
        unit->error("BUILD: Can't build in an ocean.");
        return nullptr;
    }

    // Handle ship building (negative object type)
    if (object_type < 0) return ProcessBuildShip(unit, object_type, checker);

    // Regular structure
    return ProcessBuildStructure(unit, object_type, checker);
}

BuildOrder *Game::ProcessContinuedBuild(Unit *unit, orders_check *checker)
{

    if(checker) return new BuildOrder;

    int maxbuild = 0;
    // Look for an incomplete ship in inventory
    int ship_type = O_DUMMY;
    for (auto it : unit->items) {
        if (ItemDefs[it->type].type & IT_SHIP && !(ItemDefs[it->type].flags & ItemType::DISABLED)) {
            ship_type = -(it->type);
            break;
        }
    }

    if (ship_type == O_DUMMY) {
        // Build whatever we happen to be in when we get to the build phase
        unit->build = 0;
    } else {
        unit->build = ship_type;
        maxbuild = unit->items.GetNum(-ship_type);
    }

    BuildOrder *order = new BuildOrder;
    // Set needtocomplete if we have a maxbuild value
    if (maxbuild != 0) order->needtocomplete = maxbuild;
    return order;
}

void Game::ProcessBuildOrder(Unit *unit, parser::string_parser& parser, orders_check *checker)
{
    unit->build = 0;
    BuildOrder *order = nullptr;

    parser::token token = parser.get_token();

    if (!token || token == "complete") {
        order = ProcessContinuedBuild(unit, checker);
    } else if (token == "help") {
        order = ProcessBuildHelp(unit, parser, checker);
    } else if (token != "complete") {
        int object_type = parse_object(token, true);
        order = ProcessBuildObject(unit, object_type, checker);
    }

    // the underlying functions shoulld handle any errors.
    if (!order) return;

    // If we processed everything else, now check for the complete keyword
    if (token != "complete") {
        token = parser.get_token();
    }

    if (token && token != "complete") {
        parse_error(checker, unit, 0, "BUILD: Unknown keyword '" + token.get_string() + "'.");
        delete order;
        return;
    }

    if (token == "complete" && !checker) {
        order->until_complete = true;
    }

    // Check that the unit isn't doing anything else important
    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (unit->taxing == TAX_TAX || unit->taxing == TAX_PILLAGE));
    if (unit->monthorders || monthtaxing) {
        if (unit->monthorders) delete unit->monthorders;
        overwrite_month_warning("BUILD", unit, checker);
    }

    // Reset taxation status if taxing is a month-long order
    if (Globals->TAX_PILLAGE_MONTH_LONG) unit->taxing = TAX_NONE;
    unit->monthorders = order;
}

void Game::ProcessAttackOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if(checker) return;

    UnitId *target = parse_unit(parser);

    // Process all target units listed in the order
    while (target && target->unitnum != -1) {
        // Create attack orders structure if this is the first target
        if (!u->attackorders) u->attackorders = new AttackOrder;
        u->attackorders->targets.push_back(target);
        target = parse_unit(parser);
    }

    // clean up if we had an invalid target as the last one.
    if (target && target->unitnum == -1) delete target;
}

void Game::ProcessSellOrder(Unit *u, parser::string_parser& parser, orders_check *checker, bool repeating)
{
    parser::token token = parser.get_token();
    int num = 0;
    if (token == "ALL") num = -1;
    else num = token.get_number().value_or(0);

    if (!num) {
        parse_error(checker, u, 0, "SELL: Number to sell not given.");
        return;
    }

    token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "SELL: Item not given.");
        return;
    }

    // Skip the rest in check-only mode
    if (checker) return;

    int it = parse_giveable_item(token);

    // Check if we already have an order for this type of item.  If we do, merge with it.
    // We will not merge repeating orders with non-repeating, but will merge all orders of
    // the same type for the same item.
    for (const auto s : u->sellorders) {
        if (s->item == it && s->repeating == repeating) {
            // Found a matching order.  Merge it.
            if (s->num == -1 || num == -1) s->num = -1;
            else s->num += num;
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

void Game::ProcessBuyOrder(Unit *u, parser::string_parser& parser, orders_check *checker, bool repeating)
{
    parser::token token = parser.get_token();
    int num = 0;
    if (token == "ALL") num = -1;
    else num = token.get_number().value_or(0);

    if (!num) {
        parse_error(checker, u, 0, "BUY: Number to buy not given.");
        return;
    }

    token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "BUY: Item not given.");
        return;
    }

    if (checker) return;

    // Check for peasant special keyword
    int it = -1;
    if (token == "peasant" || token == "peasants" || token == "peas") it = u->object->region->race;
    else it = parse_giveable_item(token);

    // Check if we already have an order for this type of item.  If we do, merge with it.
    // We will not merge repeating orders with non-repeating, but will merge all orders of
    // the same type for the same item.
    for (const auto b : u->buyorders) {
        if (b->item == it && b->repeating == repeating) {
            // Found a matching order.  Merge it.
            if (b->num == -1 || num == -1) b->num = -1;
            else b->num += num;
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

void Game::ProcessProduceOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();

    // First check if there's a target production amount and if so, store it and read next token
    int target = token.get_number().value_or(0);
    if (target > 0) token = parser.get_token();

    if (!token) {
        parse_error(checker, u, 0, "PRODUCE: No item given.");
        return;
    }

    const int it = parse_enabled_item(token);
    if (it == -1) {
        parse_error(checker, u, 0, "PRODUCE: Invalid item.");
        return;
    }

    const ItemType &item_def = ItemDefs[it];
    if (item_def.type == IT_SHIP) {
        parse_error(checker, u, 0, "PRODUCE: Use BUILD for ships.");
        return;
    }

    ProduceOrder *p = new ProduceOrder;
    p->item = it;
    p->skill = lookup_skill(item_def.pSkill);
    p->target = target;

    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE));
    if (u->monthorders || monthtaxing) {
        if (u->monthorders) delete u->monthorders;
        overwrite_month_warning("PRODUCE", u, checker);
    }

    if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
    u->monthorders = p;
}

void Game::ProcessWorkOrder(Unit *u, int quiet, orders_check *checker)
{
    ProduceOrder *order = new ProduceOrder;
    order->skill = -1;
    order->item = I_SILVER;
    order->target = 0;
    if (quiet) order->quiet = 1;

    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE));
    if (u->monthorders || monthtaxing) {
        if (u->monthorders) delete u->monthorders;
        overwrite_month_warning("WORK", u, checker);
    }

    if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
    u->monthorders = order;
}

void Game::ProcessTeachOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    TeachOrder *order = nullptr;

    // Reuse existing teach order if one exists
    if (u->monthorders && u->monthorders->type == O_TEACH) order = dynamic_cast<TeachOrder *>(u->monthorders);
    else order = new TeachOrder;

    // Parse all student units
    int students = 0;
    UnitId *id = parse_unit(parser);
    while (id && id->unitnum != -1) {
        students++;
        order->targets.push_back(id);
        id = parse_unit(parser);
    }

    // Cleanup if we had an invalid target as the last one
    if (id && id->unitnum == -1) delete id;

    if (!students) {
        parse_error(checker, u, 0, "TEACH: No students given.");
        // If we weren't adding to an existing order, delete the new order
        if (order && u->monthorders->type != O_TEACH) delete order;
        return;
    }

    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE));
    if ((u->monthorders && u->monthorders->type != O_TEACH) || monthtaxing) {
        if (u->monthorders) delete u->monthorders;
        overwrite_month_warning("TEACH", u, checker);
    }

    if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
    u->monthorders = order;
}

void Game::ProcessStudyOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "STUDY: No skill given.");
        return;
    }

    int sk = parse_skill(token);
    if (sk == -1) {
        parse_error(checker, u, 0, "STUDY: Unknown skill '" + token.get_string() + "'.");
        return;
    }

    StudyOrder *order = new StudyOrder;
    order->skill = sk;
    order->days = 0;

    // Parse study level if provided
    order->level = parser.get_token().get_number().value_or(-1);

    // Check for conflicts with existing monthlong orders
    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE));
    if (u->monthorders || monthtaxing) {
        if (u->monthorders) delete u->monthorders;
        overwrite_month_warning("STUDY", u, checker);
    }

    // Reset taxation status if taxing is a month-long order
    if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;

    // Always store the new order so conflicts can be detected later
    u->monthorders = order;
}

void Game::ProcessDeclareOrder(Faction *f, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, 0, f, "DECLARE: No faction given.");
        return;
    }

    int fac = -2; // special token so we can use -1 for 'default'
    if (token == "default") fac = -1;
    else fac = token.get_number().value_or(-2);

    if (fac == -2) {
        parse_error(checker, 0, f, "DECLARE: Non-existent faction.");
        return;
    }

    // Only valdiate the faction number in non-check mode.
    if (fac != -1 && !checker) {
        Faction *target = GetFaction(factions, fac);
        if (!target) {
            f->error("DECLARE: Non-existent faction " + std::to_string(fac) + ".");
            return;
        }
        if (target == f) {
            f->error("DECLARE: Can't declare towards your own faction.");
            return;
        }
    }

    token = parser.get_token();
    // if we don't give an attitude, remove the attitude for a specific faction and we aren't in check mode
    if (!token) {
        if (fac == -1) {
            parse_error(checker, 0, f, "DECLARE: No default attitude provided.");
            return;
        }
        if (fac != -1 && !checker) f->remove_attitude(fac);
        return;
    }

    std::optional<AttitudeType> att = parse_attitude(token);
    if (!att) {
        parse_error(checker, 0, f, "DECLARE: Invalid attitude.");
        return;
    }

    if (checker) return;

    if (fac == -1) f->defaultattitude = att.value();
    else f->set_attitude(fac, att.value());
}

void Game::ProcessWithdrawOrder(Unit *unit, parser::string_parser& parser, orders_check *checker)
{
    if (!(Globals->ALLOW_WITHDRAW)) {
        parse_error(checker, unit, 0, "WITHDRAW is not a valid order.");
        return;
    }

    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, unit, 0, "WITHDRAW: No amount given.");
        return;
    }

    int amt = token.get_number().value_or(0);

    // If we didn't have an amount, reuse that token as the item, otherwise we need to get the next token
    if (amt < 1) amt = 1;
    else token = parser.get_token();

    if (!token) {
        parse_error(checker, unit, 0, "WITHDRAW: No item given.");
        return;
    }

    int item = parse_giveable_item(token);

    if (item == -1 || item == I_SILVER || !(ItemDefs[item].type & IT_NORMAL)) {
        parse_error(checker, unit, 0, "WITHDRAW: Invalid item.");
        return;
    }

    if (checker) return;

    WithdrawOrder *order = new WithdrawOrder;
    order->item = item;
    order->amount = amt;
    unit->withdraworders.push_back(order);
}

std::string Game::ProcessTurnOrder(Unit *unit, istream& f, orders_check *checker, bool repeat)
{
    int turnDepth = 1;
    int turnLast = 1;
    int formDepth = 0;
    TurnOrder *tOrder = new TurnOrder;
    tOrder->repeating = repeat;

    parser::string_parser order;

    while (turnDepth) {
        // get the next line
        f >> order;
        if (f.eof()) {
            // Fake end of commands to invoke appropriate processing
            order = "#end";
        }

        // In order to allow @endturn to work the same as endturn we need to check for and eat the possible @
        std::ignore = order.get_at(); // we don't care about whether it was set or not, so just ignore the return value
        auto token = order.get_token();

        if (token) {
            int i = Parse1Order(token);
            switch (i) {
                case O_TURN:
                    if (turnLast) {
                        parse_error(checker, unit, 0, "TURN: cannot nest.");
                        break;
                    }
                    turnDepth++;
                    tOrder->turnOrders.push_back(order.original());
                    turnLast = 1;
                    break;
                case O_FORM:
                    if (!turnLast) {
                        parse_error(checker, unit, 0, "FORM: cannot nest.");
                        break;
                    }
                    turnLast = 0;
                    formDepth++;
                    tOrder->turnOrders.push_back(order.original());
                    break;
                case O_ENDFORM:
                    if (turnLast) {
                        if (!(formDepth + (unit->former != 0))) {
                            parse_error(checker, unit, 0, "END: without FORM.");
                            break;
                        } else {
                            parse_error(checker, unit, 0, "TURN: without ENDTURN.");
                            if (!--turnDepth) {
                                unit->turnorders.push_back(tOrder);
                                return order.original();
                            }
                        }
                    }
                    formDepth--;
                    tOrder->turnOrders.push_back(order.original());
                    turnLast = 1;
                    break;
                case O_UNIT:
                case O_END:
                    if (!turnLast)
                        parse_error(checker, unit, 0, "FORM: without END.");
                    while (--turnDepth) {
                        parse_error(checker, unit, 0, "TURN: without ENDTURN.");
                        parse_error(checker, unit, 0, "FORM: without END.");
                    }
                    parse_error(checker, unit, 0, "TURN: without ENDTURN.");
                    unit->turnorders.push_back(tOrder);
                    return order.original();
                    break;
                case O_ENDTURN:
                    if (!turnLast) {
                        parse_error(checker, unit, 0, "ENDTURN: without TURN.");
                    } else {
                        if (--turnDepth)
                            tOrder->turnOrders.push_back(order.original());
                        turnLast = 0;
                    }
                    break;
                default:
                    tOrder->turnOrders.push_back(order.original());
                    break;
            }
            if (!checker && unit->former && unit->former->form_repeated)
                unit->former->oldorders.push_back(order.original());
        }
    }

    unit->turnorders.push_back(tOrder);
    return "";
}

void Game::ProcessExchangeOrder(Unit *unit, parser::string_parser& parser, orders_check *checker)
{
    UnitId *t = parse_unit(parser);
    if (!t || t->unitnum == -1) {
        if (t) delete t;
        parse_error(checker, unit, 0, "EXCHANGE: Invalid target.");
        return;
    }
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, unit, 0, "EXCHANGE: No amount given.");
        return;
    }

    int amtGive = token.get_number().value_or(-1);
    if (amtGive < 0) {
        parse_error(checker, unit, 0, "EXCHANGE: Illegal amount given.");
        return;
    }

    token = parser.get_token();
    if (!token) {
        parse_error(checker, unit, 0, "EXCHANGE: No item given.");
        return;
    }

    int itemGive = parse_giveable_item(token);

    if (itemGive == -1) {
        parse_error(checker, unit, 0, "EXCHANGE: Invalid item.");
        return;
    }
    if (ItemDefs[itemGive].type & IT_SHIP) {
        parse_error(checker, unit, 0, "EXCHANGE: Can't exchange ships.");
        return;
    }

    token = parser.get_token();
    if (!token) {
        parse_error(checker, unit, 0, "EXCHANGE: No amount expected.");
        return;
    }
    int amtExpected = token.get_number().value_or(-1);

    if (amtExpected < 0) {
        parse_error(checker, unit, 0, "EXCHANGE: Illegal amount given.");
        return;
    }

    token = parser.get_token();
    if (!token) {
        parse_error(checker, unit, 0, "EXCHANGE: No item expected.");
        return;
    }
    int itemExpected = parse_giveable_item(token);

    if (itemExpected == -1) {
        parse_error(checker, unit, 0, "EXCHANGE: Invalid item.");
        return;
    }
    if (ItemDefs[itemExpected].type & IT_SHIP) {
        parse_error(checker, unit, 0, "EXCHANGE: Can't exchange ships.");
        return;
    }

    if (checker) return;

    ExchangeOrder *order = new ExchangeOrder;
    order->giveItem = itemGive;
    order->giveAmount = amtGive;
    order->expectAmount = amtExpected;
    order->expectItem = itemExpected;
    order->target = t;
    unit->exchangeorders.push_back(order);
}

void Game::ProcessGiveOrder(int order, Unit *unit, parser::string_parser& parser, orders_check *checker)
{
    // Create the order string ("GIVE" or "TAKE" ) based on the order type
    std::string ord = order == O_GIVE ? "GIVE" : "TAKE";

    // For TAKE orders, require "from" keyword
    if (order == O_TAKE) {
        parser::token token = parser.get_token();
        if (!token || token != "from") {
            parse_error(checker, unit, 0, ord + ": Missing FROM.");
            return;
        }
    }

    // Parse target unit
    UnitId *t = parse_unit(parser);
    if (!t || (t->unitnum == -1 && order == O_TAKE)) {
        if (t) delete t;
        parse_error(checker, unit, 0, ord + ": Invalid target.");
        return;
    }

    // Parse amount
    parser::token token = parser.get_token();
    if (!token) {
        if (t) delete t;
        parse_error(checker, unit, 0, ord + ": No amount given.");
        return;
    }

    // Parse the amount - special keywords or numeric value
    int amt;
    if (token == "unit" && order == O_GIVE) amt = -1;
    else if (token == "all") amt = -2;
    else {
        amt = token.get_number().value_or(0);
        if (amt < 1) {
            if (t) delete t;
            parse_error(checker, unit, 0, ord + ": Illegal amount given.");
            return;
        }
    }

    int item = -1;
    bool unfinished = false;

    // Parse item (only if not giving a unit)
    if (amt != -1) {
        // Check for "unfinished" keyword
        token = parser.get_token();
        if (token && token == "unfinished") {
            unfinished = true;
            token = parser.get_token();
        }

        if (!token) {
            if (t) delete t;
            parse_error(checker, unit, 0, ord + ": No item given.");
            return;
        }

        // Parse the item or item class
        item = t->unitnum == -1 ? parse_enabled_item(token) : parse_giveable_item(token);

        // Handle item classes when using "all"
        if (amt == -2) {
            // If we don't already have a specific item, check for item class
            auto category_code = parse_item_category(token);
            if (category_code) item = category_code.value();
            else if (item == -1) {
                if (t) delete t;
                parse_error(checker, unit, 0, ord + ": Invalid item or item class.");
                return;
            }
        } else if (item == -1) {
            if (t) delete t;
            parse_error(checker, unit, 0, ord + ": Invalid item.");
            return;
        }

        // Validate unfinished items
        if (unfinished && item != -IT_SHIP && item != -NITEMS && !(item >= 0 && ItemDefs[item].type & IT_SHIP)) {
            if (t) delete t;
            parse_error(checker, unit, 0, ord + ": That item does not have an unfinished version.");
            return;
        }
    }

    int excpt = 0;
    // Handle EXCEPT clause
    token = parser.get_token();
    if (token == "except") {
        // EXCEPT only works with ALL and specific items
        if (amt != -2) {
            if (t) delete t;
            parse_error(checker, unit, 0, ord + ": EXCEPT only valid with ALL");
            return;
        }

        if (item < 0) {
            if (t) delete t;
            parse_error(checker, unit, 0, ord + ": EXCEPT only valid with specific items.");
            return;
        }

        token = parser.get_token();
        if (!token) {
            if (t) delete t;
            parse_error(checker, unit, 0, ord + ": EXCEPT requires a value.");
            return;
        }

        excpt = token.get_number().value_or(0);
        if (excpt <= 0) {
            if (t) delete t;
            parse_error(checker, unit, 0, ord + ": Invalid EXCEPT value.");
            return;
        }
    }

    if (checker) {
        if (t) delete t;
        return;
    }

    // Create the order
    GiveOrder *go = new GiveOrder;
    go->type = order;
    go->item = item;
    go->target = t;
    go->amount = amt;
    go->except = excpt;
    go->unfinished = unfinished;
    unit->giveorders.push_back(go);
}

void Game::ProcessDescribeOrder(Unit *unit, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, unit, 0, "DESCRIBE: No argument.");
        return;
    }

    if (token != "unit" && token != "ship" && token != "building" && token != "object" && token != "structure") {
        parse_error(checker, unit, 0, "DESCRIBE: Can't describe that.");
        return;
    }

    if (checker) return;

    parser::token description = parser.get_token();
    if (token == "unit") {
        unit->set_description(description.get_string());
        return;
    }

    // Describing an object, so make sure it's the owner
    if (unit != unit->object->GetOwner()) {
        unit->error("DESCRIBE: Unit is not owner.");
        return;
    }
    unit->object->set_description(description.get_string());
}

void Game::ProcessNameOrder(Unit *unit, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, unit, 0, "NAME: No argument.");
        return;
    }

    if (
        token != "faction" && token != "unit" && token != "building" && token != "ship" &&
        token != "object" && token != "structure" && token != "village" && token != "town" &&
        token != "city"
    ) {
        parse_error(checker, unit, 0, "NAME: Can't name that.");
        return;
    }

    parser::token name = parser.get_token();
    if (!name) {
        parse_error(checker, unit, 0, "NAME: No name given.");
        return;
    }

    if (checker) return;

    if (token == "faction") {
        unit->faction->set_name(name.get_string());
        return;
    }

    if (token == "unit") {
        unit->set_name(name.get_string());
        return;
    }

    if (token == "building" || token == "ship" || token == "object" || token == "structure") {
        // Fix to prevent non-owner units from renaming objects
        if (unit != unit->object->GetOwner()) {
            unit->error("NAME: Unit is not owner.");
            return;
        }
        if (!unit->object->CanModify()) {
            unit->error("NAME: Can't name this type of object.");
            return;
        }
        unit->object->set_name(name.get_string(), unit);
        return;
    }

    // We are trying to rename a city

    // Allow some units to rename cities. Unit must be at least the owner of tower to rename
    // village, fort to rename town and castle to rename city.
    if (!unit->object) {
        unit->error("NAME: Unit is not in a structure.");
        return;
    }
    if (!unit->object->region->town) {
        unit->error("NAME: Unit is not in a village, town or city.");
        return;
    }

    int towntype = unit->object->region->town->TownType();
    string tstring = (towntype == TOWN_VILLAGE) ? "village" : (towntype == TOWN_TOWN) ? "town" : "city";

    // Calculate rename cost if applicable
    int cost = 0;
    if (Globals->CITY_RENAME_COST) cost = (towntype+1) * Globals->CITY_RENAME_COST;

    // Check if structure is large enough for city type
    bool ok = false;
    switch(towntype) {
        case TOWN_VILLAGE:
            ok = unit->object->type == O_TOWER || unit->object->type == O_MTOWER;
            // Fall through intentionally
        case TOWN_TOWN:
            ok = ok || unit->object->type == O_FORT || unit->object->type == O_MFORTRESS;
            // Fall through intentionally
        case TOWN_CITY:
            ok = ok || unit->object->type == O_CASTLE || unit->object->type == O_CITADEL ||
                unit->object->type == O_MCASTLE || unit->object->type == O_MCITADEL;
    }

    if (!ok) {
        unit->error("NAME: Unit is not in a large enough structure to rename a " + tstring + ".");
        return;
    }

    if (unit != unit->object->GetOwner()) {
        unit->error("NAME: Cannot name " + tstring + ". Unit is not the owner of object.");
        return;
    }

    if (unit->object->incomplete > 0) {
        unit->error("NAME: Cannot name " + tstring + ". Object is not finished.");
        return;
    }

    int silver = unit->items.GetNum(I_SILVER);
    if (cost) {
        if (silver < cost) {
            unit->error("NAME: Unit doesn't have enough silver to rename a " + tstring + ".");
            return;
        }
    }

    std::string newname = name.get_string() | filter::legal_characters;
    if (newname.empty()) {
        unit->error("NAME: Illegal name.");
        return;
    }

    unit->items.SetNum(I_SILVER, silver - cost);


    unit->event("Renames " + unit->object->region->town->name + " to " + newname + ".", "rename");

    unit->object->region->notify_city(unit, unit->object->region->town->name, newname);

    unit->object->region->town->name = newname;
    return;

}

void Game::ProcessGuardOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "GUARD: Invalid value.");
        return;
    }

    if (checker) return;

    if (!val.value()) {
        if (u->guard != GUARD_AVOID) u->guard = GUARD_NONE;
    } else {
        if (u->guard != GUARD_GUARD) u->guard = GUARD_SET;
    }
}

void Game::ProcessBehindOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    /* This is an instant order */
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "BEHIND: Invalid value.");
        return;
    }

    if (checker) return;

    u->SetFlag(FLAG_BEHIND, val.value());
}

void Game::ProcessNoaidOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    /* Instant order */
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "NOAID: Invalid value.");
        return;
    }

    if (checker) return;

    u->SetFlag(FLAG_NOAID, val.value());
}

void Game::ProcessSpoilsOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    /* Instant order */
    parser::token token = parser.get_token();
    int flag = 0;
    int val = 1;
    if (token == "none") flag = FLAG_NOSPOILS;
    else if (token == "walk") flag = FLAG_WALKSPOILS;
    else if (token == "ride") flag = FLAG_RIDESPOILS;
    else if (token == "fly") flag = FLAG_FLYSPOILS;
    else if (token == "swim") flag = FLAG_SWIMSPOILS;
    else if (token == "sail") flag = FLAG_SAILSPOILS;
    else if (token == "all") val = 0;
    else parse_error(checker, u, 0, "SPOILS: Bad argument.");

    if (checker) return;

    /* Clear all the flags */
    int allflags = FLAG_NOSPOILS | FLAG_WALKSPOILS | FLAG_RIDESPOILS | FLAG_FLYSPOILS |
        FLAG_SWIMSPOILS | FLAG_SAILSPOILS;
    u->SetFlag(allflags, 0);

    /* Set the flag we're trying to set */
    if (flag) u->SetFlag(flag, val);
}

void Game::ProcessNospoilsOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parse_error(checker, u, 0, "NOSPOILS: This command is deprecated.  Use the 'SPOILS' command instead");

    /* Instant order */
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "NOSPOILS: Invalid value.");
        return;
    }

    if (checker) return;
\
    int allflags = FLAG_NOSPOILS | FLAG_WALKSPOILS | FLAG_RIDESPOILS | FLAG_FLYSPOILS |
        FLAG_SWIMSPOILS | FLAG_SAILSPOILS;
    u->SetFlag(allflags, 0);
    u->SetFlag(FLAG_NOSPOILS, val.value());
}

void Game::ProcessNocrossOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    bool move_over_water = false;

    if (Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) move_over_water = true;
    if (!move_over_water) {
        int i;
        for (i = 0; i < NITEMS; i++) {
            if (ItemDefs[i].flags & ItemType::DISABLED) continue;
            if (ItemDefs[i].swim > 0) move_over_water = true;
        }
    }
    if (!move_over_water) {
        parse_error(checker, u, 0, "NOCROSS is not a valid order.");
        return;
    }

    /* Instant order */
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "NOCROSS: Invalid value.");
        return;
    }

    if (checker) return;

    u->SetFlag(FLAG_NOCROSS_WATER, val.value());
}

void Game::ProcessHoldOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    /* Instant order */
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "HOLD: Invalid value.");
        return;
    }

    if (checker) return;

    u->SetFlag(FLAG_HOLDING, val.value());
}

void Game::ProcessAutoTaxOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    /* Instant order */
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "AUTOTAX: Invalid value.");
        return;
    }

    if (checker) return;

    u->SetFlag(FLAG_AUTOTAX, val.value());
}

void Game::ProcessAvoidOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    /* This is an instant order */
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "AVOID: Invalid value.");
        return;
    }

    if (checker) return;

    if (val.value()) u->guard = GUARD_AVOID;
    else if (u->guard == GUARD_AVOID) u->guard = GUARD_NONE;
}

Unit *Game::ProcessFormOrder(Unit *former, parser::string_parser& parser, orders_check *checker, bool repeating)
{
    auto alias_num = parser.get_token().get_number();
    if (!alias_num) {
        parse_error(checker, former, 0, "Must give alias in FORM order.");
        return nullptr;
    }
    int alias = alias_num.value_or(-1);
    if (alias <= 0) {
        parse_error(checker, former, 0, "Invalid alias in FORM order.");
        return nullptr;
    }

    if (checker) {
        Unit *unit = new Unit;
        unit->former = former;
        former->form_repeated = repeating;
        return unit;
    }

    if (former->object->region->GetUnitAlias(alias, former->faction->num)) {
        former->error("Alias multiply defined.");
        return nullptr;
    }

    Unit *new_unit = GetNewUnit(former->faction, alias);
    new_unit->CopyFlags(former);
    new_unit->DefaultOrders(former->object);
    new_unit->MoveUnit(former->object);
    new_unit->former = former;
    former->form_repeated = repeating;
    return new_unit;
}

void Game::ProcessAddressOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, u, 0, "ADDRESS: No address given.");
        return;
    }

    if (checker) return;
    u->faction->set_address(token.get_string());
}

void Game::ProcessAdvanceOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE));
    if ((u->monthorders && u->monthorders->type != O_ADVANCE) || monthtaxing) {
        if (u->monthorders) delete u->monthorders;
        u->monthorders = nullptr;
        overwrite_month_warning("ADVANCE", u, checker);
    }

    if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;

    if (!u->monthorders) u->monthorders = new MoveOrder;
    u->monthorders->type = O_ADVANCE;

    MoveOrder *m = dynamic_cast<MoveOrder *>(u->monthorders);
    m->advancing = 1;

    while(parser::token token = parser.get_token()) {
        int d = parse_dir(token);
        if (d == -1) {
            parse_error(checker, u, 0, "ADVANCE: Warning, bad direction '" + token.get_string() + "'.");
            return;
        }
        if (checker) continue;
        MoveDir *x = new MoveDir;
        x->dir = d;
        m->dirs.push_back(x);
    }
}

void Game::ProcessMoveOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE));
    if ((u->monthorders && u->monthorders->type != O_MOVE) || monthtaxing) {
        if (u->monthorders) delete u->monthorders;
        u->monthorders = nullptr;
        overwrite_month_warning("MOVE", u, checker);
    }

    if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;

    if (!u->monthorders) u->monthorders = new MoveOrder;

    MoveOrder *m = dynamic_cast<MoveOrder *>(u->monthorders);
    m->advancing = 0;

    while (parser::token token = parser.get_token()) {
        int d = parse_dir(token);
        if (d == -1) {
            parse_error(checker, u, 0, "MOVE: Warning, bad direction '" + token.get_string() + "'.");
            return;
        }
        if (checker) continue;
        MoveDir *x = new MoveDir;
        x->dir = d;
        m->dirs.push_back(x);
    }
}

void Game::ProcessSailOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE));
    if ((u->monthorders && u->monthorders->type != O_SAIL) || monthtaxing) {
        if (u->monthorders) delete u->monthorders;
        u->monthorders = nullptr;
        overwrite_month_warning("SAIL", u, checker);
    }

    if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
    if (!u->monthorders) u->monthorders = new SailOrder;

    SailOrder* m = dynamic_cast<SailOrder *>(u->monthorders);

    while (parser::token token = parser.get_token()) {
        int d = parse_dir(token);
        if (d == -1 || !(d < NDIRS || d == MOVE_PAUSE)) {
            parse_error(checker, u, 0, "SAIL: Warning, bad direction '" + token.get_string() + "'.");
            return;
        }
        if (checker) continue;
        MoveDir *x = new MoveDir;
        x->dir = d;
        m->dirs.push_back(x);
    }
}

void Game::ProcessEvictOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    if (checker) return;

    UnitId *id = parse_unit(parser);
    while (id && id->unitnum != -1) {
        if (!u->evictorders) u->evictorders = new EvictOrder;
        u->evictorders->targets.push_back(id);
        id = parse_unit(parser);
    }
    if (id && id->unitnum == -1) delete id;
}

void Game::ProcessIdleOrder(Unit *u, orders_check *checker)
{
    bool monthtaxing = (Globals->TAX_PILLAGE_MONTH_LONG && (u->taxing == TAX_TAX || u->taxing == TAX_PILLAGE));
    if (u->monthorders || monthtaxing) {
        if (u->monthorders) delete u->monthorders;
        u->monthorders = nullptr;
        overwrite_month_warning("IDLE", u, checker);
    }
    if (Globals->TAX_PILLAGE_MONTH_LONG) u->taxing = TAX_NONE;
    IdleOrder *i = new IdleOrder;
    u->monthorders = i;
}

void Game::ProcessTransportOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    UnitId *tar = parse_unit(parser);
    if (!tar || tar->unitnum == -1) {
        if (tar) delete tar;
        parse_error(checker, u, 0, "TRANSPORT: Invalid target.");
        return;
    }
    parser::token token = parser.get_token();
    if (!token) {
        if (tar) delete tar;
        parse_error(checker, u, 0, "TRANSPORT: No amount given.");
        return;
    }

    int amt;
    if (token == "all") amt = -1;
    else {
        amt = token.get_number().value_or(-1);
        if (amt < 0) {
            if (tar) delete tar;
            parse_error(checker, u, 0, "TRANSPORT: Illegal amount given.");
            return;
        }
    }

    token = parser.get_token();
    if (!token) {
        if (tar) delete tar;
        parse_error(checker, u, 0, "TRANSPORT: No item given.");
        return;
    }

    int item = parse_transportable_item(token);
    if (item == -1) {
        if (tar) delete tar;
        parse_error(checker, u, 0, "TRANSPORT: Invalid item '" + token.get_string() + "'.");
        return;
    }

    int except = 0;
    token = parser.get_token();
    if (token == "except") {
        token = parser.get_token();
        if (!token) {
            if (tar) delete tar;
            parse_error(checker, u, 0, "TRANSPORT: EXCEPT requires a value.");
            return;
        }

        except = token.get_number().value_or(0);
        if (except <= 0) {
            if (tar) delete tar;
            parse_error(checker, u, 0, "TRANSPORT: Invalid except value.");
            return;
        }
    }

    if (checker) {
        if (tar) delete tar;
        return;
    }

    TransportOrder *order = new TransportOrder;
    // At this point we don't know that transport phase for the order but
    // we will set that later.
    order->item = item;
    order->target = tar;
    order->amount = amt;
    order->except = except;
    u->transportorders.push_back(order);
}

void Game::ProcessShareOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    /* Instant order */
    auto val = parser.get_token().get_bool();
    if (!val) {
        parse_error(checker, u, 0, "SHARE: Invalid value.");
        return;
    }

    if (checker) return;

    u->SetFlag(FLAG_SHARING, val.value());
}

void Game::ProcessJoinOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
{
    int overload = 1;
    int merge = 0;

    UnitId *id = parse_unit(parser);
    if (!id || id->unitnum == -1) {
        if (id) delete id;
        parse_error(checker, u, 0, "JOIN: No target given.");
        return;
    }

    if (checker) {
        if (id) delete id;
        return;
    }

    parser::token token = parser.get_token();
    if (token == "nooverload") overload = 0;
    if (token == "merge") merge = 1;

    JoinOrder *ord = new JoinOrder;
    ord->target = id;
    ord->overload = overload;
    ord->merge = merge;
    if (u->joinorders) delete u->joinorders;
    u->joinorders = ord;
}

void Game::ProcessSacrificeOrder(Unit *unit, parser::string_parser& parser, orders_check *checker)
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
        parse_error(checker, unit, 0, "SACRIFICE is not a valid order.");
        return;
    }

    int amt = parser.get_token().get_number().value_or(0);
    if (amt < 1) {
        parse_error(checker, unit, 0, "SACRIFICE: No amount given.");
        return;
    }

    parser::token token = parser.get_token();
    if (!token) {
        parse_error(checker, unit, 0, "SACRIFICE: No item given.");
        return;
    }

    int item = parse_enabled_item(token);
    if (item == -1) {
        parse_error(checker, unit, 0, "SACRIFICE: Invalid item.");
        return;
    }

    if (checker) return;

    SacrificeOrder *order = new SacrificeOrder;
    order->item = item;
    order->amount = amt;
    if (unit->sacrificeorders) delete unit->sacrificeorders;
    unit->sacrificeorders = order;
}

void Game::ProcessAnnihilateOrder(Unit *unit, parser::string_parser& parser, orders_check *checker)
{
    // Make sure the ANNIHILATE skill is enabled.
    if (SkillDefs[S_ANNIHILATION].flags & SkillType::DISABLED) {
        parse_error(checker, unit, 0, "ANNIHILATE is not a valid order.");
        return;
    }

    parser::token token = parser.get_token();
    if (token != "region") {
        parse_error(checker, unit, 0, "ANNIHILATE: No region specified.");
        return;
    }

    auto xval = parser.get_token().get_number();
    if (!xval) {
        parse_error(checker, unit, 0, "ANNIHILATE: Region X coordinate not specified.");
        return;
    }
    int x = xval.value();
    if (x < 0) {
        parse_error(checker, unit, 0, "ANNIHILATE: Invalid X coordinate specified.");
        return;
    }

    auto yval = parser.get_token().get_number();
    if (!yval) {
        parse_error(checker, unit, 0, "ANNIHILATE: Region Y coordinate not specified.");
        return;
    }
    int y = yval.value();
    if (y < 0) {
        parse_error(checker, unit, 0, "ANNIHILATE: Invalid Y coordinate specified.");
        return;
    }

    int z = -1;
    auto range = FindRange(SkillDefs[S_ANNIHILATION].range).value().get();
    if (range.flags & RangeType::RNG_SURFACE_ONLY) {
        z = (Globals->NEXUS_EXISTS ? 1 : 0);
    } else {
        // in the order check mode, just make sure we have a valid z-coordinate otherwise
        // use the unit's current z-coordinate
        if (checker) z = (Globals->NEXUS_EXISTS ? 1 : 0);
        else z = unit->object->region->zloc;
    }

    if ((range.flags & RangeType::RNG_CROSS_LEVELS) && !(range.flags & RangeType::RNG_SURFACE_ONLY)) {
        auto zval = parser.get_token().get_number();
        if (!zval) {
            parse_error(checker, unit, 0, "ANNIHILATE: Region Z coordinate not specified.");
            return;
        }
        z = zval.value();
        if (z < 0 || z >= (Globals->UNDERWORLD_LEVELS + Globals->UNDERDEEP_LEVELS + Globals->ABYSS_LEVEL + 2)) {
            parse_error(checker, unit, 0, "ANNIHILATE: Invalid Z coordinate specified.");
            return;
        }
    }

    if (checker) return;

    AnnihilateOrder *order = new AnnihilateOrder;
    order->xloc = x;
    order->yloc = y;
    order->zloc = z;
    unit->annihilateorders.push_back(order);
}

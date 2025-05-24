#include "unit.h"
#include "gamedata.h"
#include "rng.hpp"
#include <stack>
#include "string_filters.hpp"
#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

using namespace std;

UnitId::UnitId()
{
}

UnitId::~UnitId()
{
}

string UnitId::Print()
{
    if (unitnum) {
        return to_string(unitnum);
    } else {
        if (faction) {
            return string("faction ") + to_string(faction) + " new " + to_string(alias);
        } else {
            return string("new ") + to_string(alias);
        }
    }
}

Unit::Unit()
{
    num = 0;
    type = U_NORMAL;
    faction = 0;
    formfaction = 0;
    alias = 0;
    guard = GUARD_NONE;
    reveal = REVEAL_NONE;
    flags = FLAG_NOCROSS_WATER;
    movepoints = Globals->PHASED_MOVE_OFFSET % Globals->MAX_SPEED;
    combat = -1;
    for (int i = 0; i < MAX_READY; i++) {
        readyWeapon[i] = -1;
        readyArmor[i] = -1;
    }
    readyItem = -1;
    object = 0;
    attackorders = nullptr;
    evictorders = nullptr;
    stealthorders = nullptr;
    monthorders = nullptr;
    castorders = nullptr;
    sacrificeorders = nullptr;
    teleportorders = nullptr;
    joinorders = nullptr;
    inTurnBlock = 0;
    presentTaxing = 0;
    presentMonthOrders = nullptr;
    former = nullptr;
    form_repeated = false;
    free = 0;
    practiced = 0;
    moved = 0;
    phase = -1;
    savedmovement = 0;
    savedmovedir = -1;
    ClearOrders();
    raised = 0;
    initial_region = nullptr;
}

Unit::Unit(int seq, Faction *f, int a)
{
    num = seq;
    type = U_NORMAL;
    set_name("Unit");
    faction = f;
    formfaction = f;
    alias = a;
    guard = 0;
    reveal = REVEAL_NONE;
    flags = FLAG_NOCROSS_WATER;
    movepoints = Globals->PHASED_MOVE_OFFSET % Globals->MAX_SPEED;
    combat = -1;
    for (int i = 0; i < MAX_READY; i++) {
        readyWeapon[i] = -1;
        readyArmor[i] = -1;
    }
    readyItem = -1;
    object = 0;
    attackorders = nullptr;
    evictorders = nullptr;
    stealthorders = nullptr;
    monthorders = nullptr;
    castorders = nullptr;
    teleportorders = nullptr;
    joinorders = nullptr;
    sacrificeorders = nullptr;
    inTurnBlock = 0;
    presentTaxing = 0;
    presentMonthOrders = nullptr;
    former = nullptr;
    form_repeated = false;
    free = 0;
    practiced = 0;
    moved = 0;
    phase = -1;
    savedmovement = 0;
    savedmovedir = -1;
    ClearOrders();
    raised = 0;
    initial_region = nullptr;
}

Unit::~Unit()
{
    if (monthorders) delete monthorders;
    if (presentMonthOrders) delete presentMonthOrders;
    if (attackorders) delete attackorders;
    if (stealthorders) delete stealthorders;
}

void Unit::SetMonFlags()
{
    guard = GUARD_AVOID;
    SetFlag(FLAG_HOLDING, 1);
}

void Unit::MakeWMon(char const *monname, int mon, int num)
{
    std::string temp(monname);
    set_name(temp);

    type = U_WMON;
    items.SetNum(mon, num);
    SetMonFlags();
}

void Unit::Writeout(ostream& f)
{
    set<string>::iterator it;

    f << name << '\n';
    f << (describe.empty() ? "none" : describe) << '\n';
    f << num << '\n';
    f << type << '\n';
    f << faction->num << '\n';
    f << guard << '\n';
    f << reveal << '\n';
    f << free << '\n';
    f << (readyItem != -1 ? ItemDefs[readyItem].abr : "NO_ITEM") << '\n';

    for (int i = 0; i < MAX_READY; ++i) {
        f << (readyWeapon[i] != -1 ? ItemDefs[readyWeapon[i]].abr : "NO_ITEM") << '\n';
        f << (readyArmor[i] != -1 ? ItemDefs[readyArmor[i]].abr : "NO_ITEM") << '\n';
    }

    f << flags << '\n';
    items.Writeout(f);
    skills.Writeout(f);
    f << (combat != -1 ? SkillDefs[combat].abbr : "NO_SKILL") << '\n';
    f << savedmovement << '\n';
    f << savedmovedir << '\n';
    f << visited.size() << '\n';
    for (it = visited.begin(); it != visited.end(); it++) {
        f << it->c_str() << '\n';
    }
}

void Unit::Readin(istream& f, std::list<Faction *>& facs)
{
    std::string str;
    std::getline(f >> ws, str);
    name = str | filter::strip_number;

    std::getline(f >> ws, str);
    describe = str | filter::legal_characters;
    if (describe == "none") describe.clear();

    f >> num;
    set_name(name);
    f >> type;

    int i;
    f >> i;

    faction = GetFaction(facs, i);
    f >> guard;
    if (guard == GUARD_ADVANCE) guard = GUARD_NONE;
    if (guard == GUARD_SET) guard = GUARD_GUARD;

    f >> reveal;

    /* Handle the new 'ready item', ready weapons/armor, and free */
    free = 0;
    readyItem = -1;
    for (i = 0; i < MAX_READY; i++) {
        readyWeapon[i] = -1;
        readyArmor[i] = -1;
    }

    f >> free;

    f >> ws >> str;
    readyItem = lookup_item(str);
    for (i = 0; i < MAX_READY; i++) {
        f >> ws >> str;
        readyWeapon[i] = lookup_item(str);
        f >> ws >> str;
        readyArmor[i] = lookup_item(str);
    }

    f >> flags;

    items.Readin(f);
    skills.Readin(f);

    f >> ws >> str;
    combat = lookup_skill(str);

    f >> savedmovement;
    f >> savedmovedir;

    f >> i;
    while (i-- > 0) {
        std::getline(f >> ws, str);
        visited.insert(str);
    }
}

AString Unit::MageReport()
{
    AString temp;

    if (combat != -1) {
        temp = AString(". Combat spell: ") + SkillStrs(combat);
    }
    return temp;
}

AString Unit::ReadyItem()
{
    AString temp, weaponstr, armorstr, battlestr;
    int weapon, armor, item, i, ready;

    item = 0;
    for (i = 0; i < MAX_READY; ++i) {
        ready = readyWeapon[i];
        if (ready != -1) {
            if (item) weaponstr += ", ";
            weaponstr += item_string(ready, 1);
            ++item;
        }
    }
    if (item > 0)
        weaponstr = AString("Ready weapon") + (item == 1?"":"s") + ": " +
            weaponstr;
    weapon = item;

    item = 0;
    for (i = 0; i < MAX_READY; ++i) {
        ready = readyArmor[i];
        if (ready != -1) {
            if (item) armorstr += ", ";
            armorstr += item_string(ready, 1);
            ++item;
        }
    }
    if (item > 0)
        armorstr = AString("Ready armor: ") + armorstr;
    armor = item;

    if (readyItem != -1) {
        battlestr = AString("Ready item: ") + item_string(readyItem, 1);
        item = 1;
    } else
        item = 0;

    if (weapon || armor || item) {
        temp += AString(". ");
        if (weapon) temp += weaponstr;
        if (armor) {
            if (weapon) temp += ". ";
            temp += armorstr;
        }
        if (item) {
            if (armor || weapon) temp += ". ";
            temp += battlestr;
        }
    }
    return temp;
}

AString Unit::StudyableSkills()
{
    AString temp;
    int j=0;

    for (int i=0; i<NSKILLS; i++) {
        if (SkillDefs[i].depends[0].skill != NULL) {
            if (CanStudy(i)) {
                if (j) {
                    temp += ", ";
                } else {
                    temp += ". Can Study: ";
                    j=1;
                }
                temp += SkillStrs(i);
            }
        }
    }
    return temp;
}

std::string Unit::get_name(int observation)
{
    std::string ret = name;
    int stealth = GetAttribute("stealth");
    if (reveal == REVEAL_FACTION || observation > stealth) {
        ret += ", ";
        ret += faction->name;
    }
    return ret;
}

int Unit::CanGetSpoil(Item *i)
{
    int weight, load, capacity;

    if (!i) return 0;
    if (ItemDefs[i->type].type & IT_SHIP) {
        // Don't pick up an incomplete ship if we already have one
        if (items.GetNum(i->type) > 0) return 0;
    }
    weight = ItemDefs[i->type].weight;
    if (!weight) return 1; // any unit can carry 0 weight spoils

    if (flags & FLAG_NOSPOILS)
        return 0;

    load = items.Weight();

    if (flags & FLAG_FLYSPOILS) {
        capacity = ItemDefs[i->type].fly;
        if (FlyingCapacity() + capacity < load + weight)
            return 0;
    }

    if (flags & FLAG_RIDESPOILS) {
        capacity = ItemDefs[i->type].ride;
        if (RidingCapacity() + capacity < load + weight)
            return 0;
    }

    if (flags & FLAG_WALKSPOILS) {
        capacity = ItemDefs[i->type].walk;
        if (ItemDefs[i->type].hitchItem) {
            if (items.GetNum(ItemDefs[i->type].hitchItem) >
                    items.GetNum(i->type))
                capacity = ItemDefs[i->type].hitchwalk;
        }
        if (WalkingCapacity() + capacity < load + weight)
            return 0;
    }

    if (flags & FLAG_SWIMSPOILS) {
        capacity = ItemDefs[i->type].swim;
        if (ItemDefs[i->type].type & IT_SHIP)
            capacity = 0;
        if (SwimmingCapacity() + capacity < load + weight)
            return 0;
    }

    if ((flags & FLAG_SAILSPOILS) && object && object->IsFleet()) {
        load = object->FleetLoad();
        if (object->FleetCapacity() < load + weight)
            return 0;
    }

    return 1; // all spoils
}

AString Unit::SpoilsReport() {
    AString temp;
    if (GetFlag(FLAG_NOSPOILS)) temp = ", weightless battle spoils";
    else if (GetFlag(FLAG_FLYSPOILS)) temp = ", flying battle spoils";
    else if (GetFlag(FLAG_WALKSPOILS)) temp = ", walking battle spoils";
    else if (GetFlag(FLAG_RIDESPOILS)) temp = ", riding battle spoils";
    else if (GetFlag(FLAG_SAILSPOILS)) temp = ", sailing battle spoils";
    else if (GetFlag(FLAG_SWIMSPOILS)) temp = ", swimming battle spoils";
    return temp;
}

// TODO: Move this somewhere more global.  For now it's only used here.
static inline void trim(string& s) {
    s.erase(s.begin(), find_if_not(s.begin(), s.end(), [](char c){ return isspace(c); }));
    s.erase(find_if_not(s.rbegin(), s.rend(), [](char c){ return isspace(c); }).base(), s.end());
}

json Unit::write_json_orders()
{
    stack<json> parent_stack;
    json container = json::array();
    parser::string_parser temp;
    parser::token token(std::nullopt);

    bool has_continuing_month_order = false;

    for(auto order: oldorders) {
        temp = order;
        std::ignore = temp.get_at();
        token = temp.get_token();
        int order_val = token ? Parse1Order(token) : NORDERS;

        set<int> month_orders = { O_MOVE, O_SAIL, O_TEACH, O_STUDY, O_BUILD, O_PRODUCE, O_ENTERTAIN, O_WORK };
        if (Globals->TAX_PILLAGE_MONTH_LONG) {
            month_orders.emplace(O_TAX);
            month_orders.emplace(O_PILLAGE);
        }

        if (month_orders.contains(order_val)) has_continuing_month_order = true;

        // Okay, if we are at the end of a form/turn block, we need to move back up the stack.
        if (order_val == O_ENDTURN || order_val == O_ENDFORM) {
            json parent = parent_stack.top();
            parent_stack.pop();
            // Get the last element of the parent array which is what contains the nested orders
            parent.back()["nested"] = container;
            container = parent;
        }
        trim(order);
        container.push_back( { { "order", order } } );

        // Okay, if the order we currently have is the start of a form/turn block, we need to push it on the stack.
        if (order_val == O_TURN || order_val == O_FORM) {
            parent_stack.push(container);
            container = json::array();
        }
    }

    // now write out any turn orders that were part of previous turn blocks.
    // If we had a continuing month order, or a previous turn block, the turn block will be deferred (and so wrapped).
    // If we had no month long order, the first turn block will not be wrapped (and so become the upcoming turn orders)
    // and then re-added to the end of the list if it was a repeating turn order.  wrapping here uses the same stack as
    // above in the same way.
    bool wrap_turn_block = has_continuing_month_order;
    if (!turnorders.empty()) {
        for(const auto tOrder : turnorders) {
            if (wrap_turn_block) {
                container.push_back( { { "order", (tOrder->repeating ? "@TURN" : "TURN") } } );
                parent_stack.push(container);
                container = json::array();
            }
            for(auto order: tOrder->turnOrders) {
                temp = order;
                std::ignore = temp.get_at();
                token = temp.get_token();
                int order_val = token ? Parse1Order(token) : NORDERS;
                if (order_val == O_ENDTURN || order_val == O_ENDFORM) {
                    json parent = parent_stack.top();
                    parent_stack.pop();
                    // Get the last element of the parent array which is what contains the nested orders
                    parent.back()["nested"] = container;
                    container = parent;
                }
                trim(order);
                container.push_back( { { "order", order } } );
                if (order_val == O_TURN || order_val == O_FORM) {
                    parent_stack.push(container);
                    container = json::array();
                }
            }
            if (wrap_turn_block) {
                json parent = parent_stack.top();
                parent_stack.pop();
                // Get the last element of the parent array which is what contains the nested orders
                parent.back()["nested"] = container;
                container = parent;
                container.push_back( { { "order", "ENDTURN" } } );
            }
            wrap_turn_block = true; // All future turn blocks get wrapped.
        }
        // Now, move the container back to the end of the list if it was a repeating turn order and it got unwrapped
        auto tOrder = turnorders.front();
        if (tOrder->repeating && !has_continuing_month_order) {
            container.push_back( { { "order", "@TURN" } } );
            parent_stack.push(container);
            container = json::array();
            for(auto order: tOrder->turnOrders) {
                temp = order;
                std::ignore = temp.get_at();
                token = temp.get_token();
                int order_val = token ? Parse1Order(token) : NORDERS;
                if (order_val == O_ENDTURN || order_val == O_ENDFORM) {
                    json parent = parent_stack.top();
                    parent_stack.pop();
                    // Get the last element of the parent array which is what contains the nested orders
                    parent.back()["nested"] = container;
                    container = parent;
                }
                trim(order);
                container.push_back( { { "order", order } } );
                if (order_val == O_TURN || order_val == O_FORM) {
                    parent_stack.push(container);
                    container = json::array();
                }
            }
            json parent = parent_stack.top();
            parent_stack.pop();
            // Get the last element of the parent array which is what contains the nested orders
            parent.back()["nested"] = container;
            container = parent;
            container.push_back( { { "order", "ENDTURN" } } );
        }
    }
    return container;
}

json Unit::build_json_descriptor() {
    json j = json::object();
    j["name"] = name | filter::strip_number;
    j["number"] = num;
    return j;
}

void Unit::build_json_report(
    json& j, int obs, int truesight, int detfac, int autosee, AttitudeType attitude, bool showattitudes
)
{
    int stealth = GetAttribute("stealth");
    bool my_unit = (obs == -1);
    bool see_faction = (my_unit || (detfac != 0));
    bool see_illusion = (my_unit || (GetSkill(S_ILLUSION) <= truesight));
    bool cannot_stealth = false;

    for(auto it : items) {
        if (ItemDefs[it->type].flags & ItemType::NOSTEALTH) cannot_stealth = true;
    }

    if(!my_unit) {
        // exit early if we cannot see the unit
        if ((obs < stealth) && (reveal == REVEAL_NONE) && (guard != GUARD_GUARD) && !autosee && !cannot_stealth) return;
        // ensure we can see the faction if able.
        if (obs > stealth || reveal == REVEAL_FACTION) see_faction = true;
    }

    j = build_json_descriptor();

    if (!my_unit) {
        string att = AttitudeStrs[static_cast<int>(attitude)];
        transform(att.begin(), att.end(), att.begin(), ::tolower);
        j["attitude"] = att;
    } else {
        j["own_unit"] = true;
    }

    if (!describe.empty()) j["description"] = describe;

    j["flags"]["guard"] = (guard == GUARD_GUARD);

    if (see_faction) {
        j["faction"] = {
            { "name", faction->name | filter::strip_number },
            { "number", faction->num }
        };
        j["flags"]["avoid"] = (guard == GUARD_AVOID);
        if (GetFlag(FLAG_BEHIND)) j["flags"]["behind"] = true;
    }

    if (my_unit) {
        if (reveal == REVEAL_UNIT) j["flags"]["reveal"] = "unit";
        if (reveal == REVEAL_FACTION) j["flags"]["reveal"] = "faction";
        j["flags"]["holding"] = (GetFlag(FLAG_HOLDING) ? true : false);
        j["flags"]["taxing"] = (GetFlag(FLAG_AUTOTAX) ? true : false);
        j["flags"]["no_aid"] = (GetFlag(FLAG_NOAID) ? true : false);
        j["flags"]["sharing"] = (GetFlag(FLAG_SHARING) ? true : false);
        if (GetFlag(FLAG_CONSUMING_UNIT)) j["flags"]["consume"] = "unit";
        if (GetFlag(FLAG_CONSUMING_FACTION)) j["flags"]["consume"] = "faction";
        j["flags"]["no_cross_water"] = GetFlag(FLAG_NOCROSS_WATER) ? true : false;

        // Only one of these is allowed to be true at a time.
        if (GetFlag(FLAG_NOSPOILS)) j["flags"]["spoils"] = "weightless";
        else if (GetFlag(FLAG_FLYSPOILS)) j["flags"]["spoils"] = "flying";
        else if (GetFlag(FLAG_WALKSPOILS)) j["flags"]["spoils"] = "walking";
        else if (GetFlag(FLAG_RIDESPOILS)) j["flags"]["spoils"] = "riding";
        else if (GetFlag(FLAG_SAILSPOILS)) j["flags"]["spoils"] = "sailing";
        else if (GetFlag(FLAG_SWIMSPOILS)) j["flags"]["spoils"] = "swimming";
        else j["flags"]["spoils"] = "all";

        j["weight"] = items.Weight();
        j["capacity"]["flying"] = FlyingCapacity();
        j["capacity"]["riding"] = RidingCapacity();
        j["capacity"]["walking"] = WalkingCapacity();
        j["capacity"]["swimming"] = SwimmingCapacity();

        j["skills"]["known"] = json::array();
        int men = GetMen();
        for(const auto s: skills) {
            if (s->days == 0) continue;
            int man_days = s->days/men;
            json skill = json{
                { "name", SkillDefs[s->type].name },
                { "tag", SkillDefs[s->type].abbr },
                { "level", GetLevelByDays(s->days/men) },
                { "skill_days", s->days/men },
            };
            if (Globals->REQUIRED_EXPERIENCE) {
                int exp = s->exp/men;
                int rate = StudyRateAdjustment(man_days, exp);
                skill["study_rate"] = rate;
            }
            j["skills"]["known"].push_back(skill);
        }
        for (auto i = 0; i < NSKILLS; i++) {
            if (SkillDefs[i].depends[0].skill != NULL) {
                if (CanStudy(i)) {
                    j["skills"]["can_study"].push_back({
                        { "name", SkillDefs[i].name },
                        { "tag", SkillDefs[i].abbr }
                    });
                }
            }
        }

        if ((type == U_MAGE || type == U_GUARDMAGE) && combat != -1) {
            j["combat_spell"] = { { "name", SkillDefs[combat].name }, { "tag", SkillDefs[combat].abbr } };
        }

        for (auto i = 0; i < MAX_READY; ++i) {
            if (readyWeapon[i] != -1) {
                // TODO: go back and clean this to report items better.
                j["readied"]["weapons"].push_back({
                    { "name", ItemDefs[readyWeapon[i]].name },
                    { "tag", ItemDefs[readyWeapon[i]].abr },
                    { "plural", ItemDefs[readyWeapon[i]].names }
                });
            }
        }

        for (auto i = 0; i < MAX_READY; ++i) {
            if (readyArmor[i] != -1) {
                j["readied"]["armor"].push_back({
                    { "name", ItemDefs[readyArmor[i]].name },
                    { "tag", ItemDefs[readyArmor[i]].abr },
                    { "plural", ItemDefs[readyArmor[i]].names }
                });
            }
        }

        if (readyItem != -1) {
            j["readied"]["item"] = { { "name", ItemDefs[readyItem].name }, { "tag", ItemDefs[readyItem].abr } };
        }

        // this is just a list of strings, so we can just copy it over
        if (!visited.empty()) j["visited"] = visited;

        // For the JSON report, the best location for order information is on the unit itself.
        j["orders"] = write_json_orders();

    }

    j["items"] = json::array();

    // Clear any marks on the item list since we want to report items in a specific order.
    // Not sure this is necessary with json output, but we are going to hold it for now so that the array in json
    // is in the same order the items would be listed in the normal report.
    for(auto it : items) it->checked = 0;

    // now, report the items in the specific order (men, monsters, weapons, mounts, wagons, other, silver)
    // items will be marked when they are reported to make sure they don't get reported twice if they fit
    // multiple categories.
    for (auto output_phase = 0; output_phase < 7; output_phase++) {
        for(auto item : items) {
            if (item->checked) continue;

            ItemType def = ItemDefs[item->type];
            bool combat_item = (def.type & (IT_WEAPON | IT_BATTLE | IT_ARMOR | IT_MAGIC));
            bool item_reported_in_other_phase = (
                (def.type & (IT_WEAPON | IT_BATTLE | IT_ARMOR | IT_MAGIC | IT_MOUNT | IT_MAN | IT_MONSTER)) ||
                (item->type == I_WAGON) || (item->type == I_MWAGON) || (item->type == I_SILVER)
            );

            if (output_phase == 0 && !(def.type & IT_MAN)) continue;
            if (output_phase == 1 && !(def.type & IT_MONSTER)) continue;
            if (output_phase == 2 && !combat_item) continue;
            if (output_phase == 3 && !(def.type & IT_MOUNT)) continue;
            if (output_phase == 4 && !((item->type == I_WAGON) || (item->type == I_MWAGON))) continue;
            if (output_phase == 5 && item_reported_in_other_phase) continue;
            if (output_phase == 6 && !(item->type == I_SILVER)) continue;

            item->checked = 1;
            if (my_unit || (def.weight != 0)) {
                json item_obj = json{ { "name", def.name }, { "tag", def.abr }, { "plural", def.names } };
                if (def.type & IT_SHIP) {
                    item_obj["unfinished"] = true;
                    item_obj["needs"] = item->num;
                } else {
                    item_obj["amount"] = item->num;
                }
                if (see_illusion && (def.type & IT_ILLUSION)) item_obj["illusion"] = true;
                j["items"].push_back(item_obj);
            }
        }
    }
}

std::string Unit::battle_report(int observation)
{
    std::string temp = "";
    if (Globals->BATTLE_FACTION_INFO)
        temp += get_name(observation);
    else
        temp += name;

    if (GetFlag(FLAG_BEHIND)) temp += ", behind";

    temp += items.battle_report();

    for(const auto s: skills) {
        if (SkillDefs[s->type].flags & SkillType::BATTLEREP) {
            int lvl = GetAvailSkill(s->type);
            if (lvl) temp += ", " + SkillDefs[s->type].name + " " + std::to_string(lvl);
        }
    }

    if (!describe.empty()) {
        temp += "; " + describe;
    }

    temp += ".";
    return temp;
}

void Unit::ClearOrders()
{
    canattack = 1;
    nomove = 0;
    routed = 0;
    enter = 0;
    build = 0;
    destroy = 0;
    if (attackorders) delete attackorders;
    attackorders = nullptr;
    if (evictorders) delete evictorders;
    evictorders = nullptr;
    if (stealthorders) delete stealthorders;
    stealthorders = nullptr;
    promote = nullptr;
    taxing = TAX_NONE;
    advancefrom = 0;
    if (monthorders) delete monthorders;
    monthorders = nullptr;
    inTurnBlock = 0;
    presentTaxing = 0;
    if (presentMonthOrders) delete presentMonthOrders;
    presentMonthOrders = nullptr;
    if (castorders) delete castorders;
    castorders = nullptr;
    if (teleportorders) delete teleportorders;
    teleportorders = nullptr;
    if (sacrificeorders) delete sacrificeorders;
    sacrificeorders = nullptr;
}

void Unit::ClearCastOrders()
{
    if (castorders) delete castorders;
    castorders = nullptr;
    if (teleportorders) delete teleportorders;
    teleportorders = nullptr;
}

void Unit::DefaultOrders(Object *obj)
{
    int weight, i;
    ARegion *r, *n;

    ClearOrders();
    if (type == U_WMON) {
        if (ObjectDefs[obj->type].monster == -1) {
            // determine terrain perferences
            std::set<int> forbidden;
            std::set<int> perferred;

            int aggression = 0;
            for(auto it : items) {
                ItemType &itemType = ItemDefs[it->type];

                if (!(itemType.type & IT_MONSTER)) {
                    continue;
                }

                auto monster = FindMonster(itemType.abr.c_str(), (itemType.type & IT_ILLUSION))->get();
                aggression = std::max(aggression, monster.getAggression());

                // bad terrain is an union of all bad terrains
                for (auto& item : monster.forbiddenTerrain) {
                    forbidden.insert(item);
                }

                // for simplicity good terrains will be union too
                for (auto& item : monster.preferredTerrain) {
                    perferred.insert(item);
                }
            }

            weight = items.Weight();
            r = obj->region;

            // this vector will contain all directions where monster can move
            // normal move chance will contain two enteries
            // reduced move chacne will containe one entry
            std::vector<int> directions;

            // then chance of a wandering monster not moving will be 2 / (available dirs + 2)
            // it is why we add 4 entries (see comment above) to the directions list
            directions.push_back(-1);
            directions.push_back(-1);
            directions.push_back(-1);
            directions.push_back(-1);

            for (i = 0; i < NDIRS; i++) {
                n = r->neighbors[i];
                if (!n) {
                    continue;
                }

                const int terrainSimilarType = TerrainDefs[n->type].similar_type;

                if (terrainSimilarType == R_OCEAN && !CanReallySwim() && !(CanFly(weight) && Globals->FLIGHT_OVER_WATER == GameDefs::WFLIGHT_UNLIMITED)) {
                    continue;
                }

                if (terrainSimilarType != R_OCEAN && !CanWalk(weight) && !CanRide(weight) && !CanFly(weight)) {
                    continue;
                }

                if (!forbidden.empty() && forbidden.find(terrainSimilarType) != forbidden.end()) {
                    // the direction is in a bad terrain, monster will not move there
                    continue;
                }

                if (perferred.empty() || perferred.find(terrainSimilarType) != perferred.end()) {
                    // if there are no preferred terrains at all
                    //   or target terrain is in preferred terrains list
                    // add 2 move direction entries into the list for normal hit chance
                    directions.push_back(i);
                    directions.push_back(i);
                }
                else {
                    // this is direction to the neutral terrain, we must check can monster move there
                    // monster will be able to move there only if target region is connected with a good terrain

                    bool connectedToGoodTerrain = false;
                    for (int j = 0; j < NDIRS; j++) {
                        auto region = n->neighbors[j];
                        if (!region) {
                            continue;
                        }

                        if (perferred.find(TerrainDefs[region->type].similar_type) != perferred.end()) {
                            connectedToGoodTerrain = true;
                            break;
                        }
                    }

                    if (connectedToGoodTerrain) {
                        // 2x lower chance to enter neutral region
                        directions.push_back(i);
                    }
                }
            }

            // pick a direction where to move
            // it will be uniform selection of all possible directions, better than previos alogrithm
            int dirIndex = rng::get_random(directions.size());
            int dir = directions[dirIndex];

            if (dir >= 0) {
                MoveOrder *o = new MoveOrder;
                o->advancing = 0;

                if (rng::get_random(100) < aggression) {
                    o->advancing = 1;
                }

                MoveDir *d = new MoveDir;
                d->dir = dir;
                o->dirs.push_back(d);
                monthorders = o;
            }
        }
    }   //if (type == U_WMON) {

    else if (type == U_GUARD) {
        if (guard != GUARD_GUARD)
            guard = GUARD_SET;
    }

    else if (type == U_GUARDMAGE) {
        combat = S_FIRE;
    }

    else{
        /* Set up default orders for factions which submit none */
        if (obj->region->type != R_NEXUS) {
            if (GetFlag(FLAG_AUTOTAX) && Globals->TAX_PILLAGE_MONTH_LONG && Taxers(1)) {
                taxing = TAX_AUTO;
            } else {
                ProduceOrder *order = new ProduceOrder;
                order->skill = -1;
                order->item = I_SILVER;
                order->target = 0;
                order->quiet = 1;
                monthorders = order;
            }
        }
    }
}

void Unit::PostTurn(ARegion *r)
{
    if (type == U_WMON) {
        for(auto it = items.begin(); it != items.end(); ) {
            Item *item = *it;
            if (!(ItemDefs[item->type].type & IT_MONSTER)) {
                it = items.erase(it);
                delete item;
                continue;
            }
            ++it;
        }
        if (free > 0) --free;
    }
}

void Unit::set_name(const std::string& newname)
{
    if (newname.empty()) return;

    std::string temp = newname | filter::legal_characters;
    if (temp.empty()) return;

    name = temp + " (" + to_string(num) + ")";
}

void Unit::set_description(const std::string& newdescription)
{
    describe.clear();
    if (!newdescription.empty()) describe = newdescription | filter::legal_characters;
}

int Unit::IsAlive()
{
    if (type == U_MAGE || type == U_APPRENTICE) {
        return(GetMen());
    } else {
        for(auto i : items) {
            if (IsSoldier(i->type) && i->num > 0)
                return 1;
        }
    }
    return 0;
}

void Unit::SetMen(int t, int n)
{
    if (ItemDefs[t].type & IT_MAN) {
        int oldmen = GetMen();
        items.SetNum(t, n);
        int newmen = GetMen();
        if (newmen < oldmen) {
            delete skills.Split(oldmen, oldmen - newmen);
        }
    } else {
        /* This is probably a monster in this case */
        items.SetNum(t, n);
    }
}

int Unit::GetMen(int t)
{
    return items.GetNum(t);
}

int Unit::GetMons()
{
    int n=0;
    for(auto i : items) {
        if (ItemDefs[i->type].type & IT_MONSTER) {
            n += i->num;
        }
    }
    return n;
}

int Unit::GetMen()
{
    int n = 0;
    for(auto i : items) {
        if (ItemDefs[i->type].type & IT_MAN) {
            n += i->num;
        }
    }
    return n;
}

int Unit::GetLeaders()
{
    int n = 0;
    for(auto i : items) {
        if (ItemDefs[i->type].type & IT_LEADER) {
            n += i->num;
        }
    }
    return n;
}

int Unit::GetSoldiers()
{
    int n = 0;
    for(auto i : items) {
        if (IsSoldier(i->type)) n+=i->num;
    }

    return n;
}

void Unit::SetMoney(int n)
{
    items.SetNum(I_SILVER, n);
}

int Unit::GetMoney()
{
    return items.GetNum(I_SILVER);
}

int Unit::GetSharedNum(int item)
{
    int count = 0;

    if (ItemDefs[item].type & IT_MAN) return items.GetNum(item);

    for(const auto obj : object->region->objects) {
        for(const auto u : obj->units) {
            if ((u->num == num) || (u->faction == faction && u->GetFlag(FLAG_SHARING))) count += u->items.GetNum(item);
        }
    }
    return count;
}

void Unit::ConsumeShared(int item, int needed)
{
    // Use up items carried by the using unit first
    int amount = items.GetNum(item);
    int used = min(amount, needed);
    needed -= used;
    items.SetNum(item, amount - used);
    // If we had enough, we are done
    if (needed == 0) return;

    // We still need more, so look for whomever is able to share with us
    for(const auto obj : object->region->objects) {
        for(const auto u : obj->units) {
            if (u->faction == faction && u->GetFlag(FLAG_SHARING)) {
                amount = u->items.GetNum(item);
                if (amount < 1) continue;
                used = min(needed, amount);
                u->items.SetNum(item, amount - used);
                needed -= used;
                string temp = u->name + " shares " + item_string(item, used) + " with " + name + ".";
                u->event(temp, "share");
                // If the need has been filled, then we are done
                if (needed == 0) return;
            }
        }
    }
}

int Unit::GetSharedMoney()
{
    return GetSharedNum(I_SILVER);
}

void Unit::ConsumeSharedMoney(int n)
{
    return ConsumeShared(I_SILVER, n);
}

int Unit::GetAttackRiding()
{
    int riding = 0;
    if (type == U_WMON) {
        for(auto i : items) {
            if (ItemDefs[i->type].type & IT_MONSTER) {
                if (ItemDefs[i->type].fly) {
                    return 5;
                }
                if (ItemDefs[i->type].ride) riding = 3;
            }
        }
        return riding;
    } else {
        int attackriding = 0;
        int minweight = 10000;
        AString skname;
        for(auto i : items) {
            if (ItemDefs[i->type].type & IT_MAN)
                if (ItemDefs[i->type].weight < minweight)
                    minweight = ItemDefs[i->type].weight;
        }
        for(auto i : items) {
            int skill, maxBonus;
            if (!(ItemDefs[i->type].type & IT_MOUNT)) continue;
            auto mountType = FindMount(ItemDefs[i->type].abr.c_str());
            if (!mountType) continue;
            auto mount = mountType.value().get();
            maxBonus = mount.maxBonus;
            /*
             * This code applies terrain restrictions to the attack riding
             * calculations, but given that these have never been applied
             * historically we probably don't want to start now.
             * Thus: for reference only.
            int canRide = TerrainDefs[object->region->type].flags & TerrainType::RIDINGMOUNTS;
            int canFly = TerrainDefs[object->region->type].flags & TerrainType::FLYINGMOUNTS;
            if (canRide && !canFly)
                maxBonus = mount->maxHamperedBonus;
            if (!canRide && !canFly)
                maxBonus = 0;
            */
            skill = lookup_skill(mount.skill);
            if (skill == -1) {
                // This mount doesn't require skill to use.
                // I guess the rider gets the max bonus!
                if (attackriding < maxBonus) attackriding = maxBonus;
            } else {
                riding = GetSkill(skill);
                if (
                    (ItemDefs[i->type].type & IT_MAN) ||
                    (ItemDefs[i->type].fly - ItemDefs[i->type].weight >= minweight) ||
                    (ItemDefs[i->type].ride - ItemDefs[i->type].weight >= minweight)
                ) {
                    if (riding > maxBonus) riding = maxBonus;
                    if (attackriding < riding) attackriding = riding;
                }
            }
        }
        return attackriding;
    }
}

int Unit::GetDefenseRiding()
{
    if (guard == GUARD_GUARD) return 0;

    int riding = 0;
    int weight = Weight();

    if (CanFly(weight)) {
        riding = 5;
        // Limit riding to the slowest flying mount
        for(auto i : items) {
            if (ItemDefs[i->type].type & IT_MOUNT && ItemDefs[i->type].fly) {
                auto mountType = FindMount(ItemDefs[i->type].abr.c_str());
                if (mountType) {
                    auto mount = mountType.value().get();
                    // If we wanted to apply terrain restrictions,
                    // we'd do it here
                    if (mount.maxBonus < riding)
                        riding = mount.maxBonus;
                }
            }
        }
    } else if (CanRide(weight)) {
        riding = 3;
        // Limit riding to the slowest riding mount
        for(auto i : items) {
            if (ItemDefs[i->type].type & IT_MOUNT && ItemDefs[i->type].ride) {
                auto mountType = FindMount(ItemDefs[i->type].abr.c_str());
                if (mountType) {
                    auto mount = mountType.value().get();
                    // If we wanted to apply terrain restrictions, we'd also do it here
                    if (mount.maxBonus < riding) riding = mount.maxBonus;
                }
            }
        }
    }

    if (GetMen()) {
        int manriding = GetSkill(S_RIDING);
        if (manriding < riding) return manriding;
    }

    return riding;
}

int Unit::GetSkill(int sk)
{
    if (sk == S_TACTICS) return GetAttribute("tactics");
    if (sk == S_STEALTH) return GetAttribute("stealth");
    if (sk == S_OBSERVATION) return GetAttribute("observation");
    if (sk == S_ENTERTAINMENT) return GetAttribute("entertainment");
    int retval = GetAvailSkill(sk);
    return retval;
}

void Unit::SetSkill(int sk, int level)
{
    skills.SetDays(sk, GetDaysByLevel(level) * GetMen());
    skills.SetExp(sk, 0);
}

int Unit::GetAvailSkill(int sk)
{
    AString str;
    int retval = GetRealSkill(sk);

    for(auto i : items) {
        if (ItemDefs[i->type].flags & ItemType::DISABLED) continue;
        if (ItemDefs[i->type].type & IT_MAGEONLY && type != U_MAGE && type != U_APPRENTICE && type != U_GUARDMAGE)
            continue;
        if ((SkillDefs[sk].flags & SkillType::MAGIC) && type != U_MAGE && type != U_APPRENTICE && type != U_GUARDMAGE)
            continue;
        if (i->num < GetMen()) continue;
        if (ItemDefs[i->type].grantSkill && lookup_skill(ItemDefs[i->type].grantSkill) == sk) {
            int grant = 0;
            for (unsigned j = 0; j < sizeof(ItemDefs[0].fromSkills) / sizeof(ItemDefs[0].fromSkills[0]); j++) {
                if (ItemDefs[i->type].fromSkills[j]) {
                    int fromSkill;

                    fromSkill = lookup_skill(ItemDefs[i->type].fromSkills[j]);
                    if (fromSkill != -1) {
                        /*
                            Should this use GetRealSkill or GetAvailSkill?
                            GetAvailSkill could cause unbounded recursion,
                            but only if the GM sets up items stupidly...
                        */
                        if (grant < GetRealSkill(fromSkill)) grant = GetRealSkill(fromSkill);
                    }
                }
            }
            if (grant < ItemDefs[i->type].minGrant) grant = ItemDefs[i->type].minGrant;
            if (grant > ItemDefs[i->type].maxGrant) grant = ItemDefs[i->type].maxGrant;

            if (grant > retval) retval = grant;
        }
    }

    // Check for a skill granted by a structure
    if (object && object->type != O_DUMMY) {
        ObjectType ob = ObjectDefs[object->type];
        if (ob.flags & ObjectType::GRANTSKILL && object->GetOwner() == this) {
            if (ob.granted_skill == sk) {
                int grant = ob.granted_level;
                if (grant > retval) retval = grant;
            }
        }
    }

    return retval;
}

int Unit::GetRealSkill(int sk)
{
    if (GetMen()) {
        return GetLevelByDays(skills.GetDays(sk)/GetMen());
    } else {
        return 0;
    }
}

void Unit::ForgetSkill(int sk)
{
    skills.SetDays(sk, 0);
    if (type == U_MAGE) {
        for(const auto s: skills) {
            if (SkillDefs[s->type].flags & SkillType::MAGIC) {
                return;
            }
        }
        type = U_NORMAL;
    }
    if (type == U_APPRENTICE) {
        for(const auto s: skills) {
            if (SkillDefs[s->type].flags & SkillType::APPRENTICE) {
                return;
            }
        }
        type = U_NORMAL;
    }
}

int Unit::CheckDepend(int lev, SkillDepend &dep)
{
    int sk = lookup_skill(dep.skill);
    if (sk == -1) return 0;
    int temp = GetRealSkill(sk);
    if (temp < dep.level) return 0;
    if (lev >= temp) return 0;
    return 1;
}

int Unit::CanStudy(int sk)
{
    if (skills.GetStudyRate(sk, GetMen()) < 1) return 0;

    if (Globals->SKILL_LIMIT_NONLEADERS && IsNormal() && skills.GetDays(sk) < 1 && skills.size() > 0) {
        if (!Globals->MAGE_NONLEADERS || !(SkillDefs[sk].flags & SkillType::MAGIC))
        return 0;
    }

    int curlev = GetRealSkill(sk);

    if (SkillDefs[sk].flags & SkillType::DISABLED) return 0;

    unsigned int c;
    for (c = 0; c < sizeof(SkillDefs[sk].depends)/sizeof(SkillDefs[sk].depends[0]); c++) {
        if (SkillDefs[sk].depends[c].skill == NULL) return 1;
        auto pS = FindSkill(SkillDefs[sk].depends[c].skill);
        if (pS && (pS->get().flags & SkillType::DISABLED)) continue;
        if (!CheckDepend(curlev, SkillDefs[sk].depends[c])) return 0;
    }
    return 1;
}

int Unit::Study(int sk, int days)
{
    if (Globals->SKILL_LIMIT_NONLEADERS && !IsLeader()) {
        if (SkillDefs[sk].flags & SkillType::MAGIC) {
            for(const auto s: skills) {
                if (!(SkillDefs[s->type].flags & SkillType::MAGIC)) {
                    error("STUDY: Non-leader mages cannot possess non-magical skills.");
                    return 0;
                }
            }
        } else if (skills.size()) {
            Skill *s = skills.front();
            if ((s->type != sk) && (s->days > 0)) {
                error("STUDY: Can know only 1 skill.");
                return 0;
            }
        }
    }
    int max = GetSkillMax(sk);
    if (GetRealSkill(sk) >= max) {
        error("STUDY: Maximum level for skill reached.");
        return 0;
    }

    if (!CanStudy(sk)) {
        if (GetRealSkill(sk) > 0)
            error("STUDY: Doesn't have the pre-requisite skills to study that.");
        else
            error("STUDY: Can't study that.");
        return 0;
    }

    skills.SetDays(sk, skills.GetDays(sk) + days);
    AdjustSkills();

    /* Check to see if we need to show a skill report */
    int lvl = GetRealSkill(sk);
    int shown = faction->skills.GetDays(sk);
    while (lvl > shown) {
        shown++;
        faction->skills.SetDays(sk, shown);
        faction->shows.push_back({.skill = sk, .level = shown});
    }
    return 1;
}

int Unit::GetSkillMax(int sk)
{
    int max = 0;

    if (SkillDefs[sk].flags & SkillType::DISABLED) return 0;

    for(auto i : items) {
        if (ItemDefs[i->type].flags & ItemType::DISABLED) continue;
        if (!(ItemDefs[i->type].type & IT_MAN)) continue;
        int m = SkillMax(SkillDefs[sk].abbr.c_str(), i->type);
        if ((max == 0 && m > max) || (m < max)) max = m;
    }
    return max;
}

int Unit::Practice(int sk)
{
    int bonus, men, curlev, reqsk, reqlev, days;
    unsigned int i;

    bonus = Globals->SKILL_PRACTICE_AMOUNT;
    if (bonus == 0) bonus = Globals->REQUIRED_EXPERIENCE / 8;
    if (practiced || (bonus < 1)) return 1;
    days = skills.GetDays(sk);
    men = GetMen();

    if (GetAvailSkill(sk) > GetRealSkill(sk)) {
        // This is a skill granted by an item, so try to practice
        // the skills it depends on (if any)
        AString str;

        reqlev = 0;

        for(auto it : items) {
            if (ItemDefs[it->type].flags & ItemType::DISABLED) continue;
            if (ItemDefs[it->type].type & IT_MAGEONLY && type != U_MAGE && type != U_APPRENTICE && type != U_GUARDMAGE)
                continue;
            if ((SkillDefs[sk].flags & SkillType::MAGIC) && type != U_MAGE && type != U_APPRENTICE && type != U_GUARDMAGE)
                continue;
            if (it->num < GetMen()) continue;
            if (ItemDefs[it->type].grantSkill && lookup_skill(ItemDefs[it->type].grantSkill) == sk) {
                for (unsigned j = 0; j < sizeof(ItemDefs[0].fromSkills) / sizeof(ItemDefs[0].fromSkills[0]); j++) {
                    if (ItemDefs[it->type].fromSkills[j]) {
                        int fromSkill;

                        fromSkill = lookup_skill(ItemDefs[it->type].fromSkills[j]);
                        if (fromSkill != -1 && GetRealSkill(fromSkill) > reqlev) {
                            reqsk = fromSkill;
                            reqlev = GetRealSkill(fromSkill);
                        }
                    }
                }
            }
        }

        if (reqlev > 0) {
            // Since granting items use the highest contributing
            // skill, practice that skill.
            Practice(reqsk);
            return 1;
        }
    }

    if (men < 1 || ((days < 1) && (!Globals->REQUIRED_EXPERIENCE))) return 0;

    int max = GetSkillMax(sk);
    curlev = GetRealSkill(sk);
    if (curlev >= max) return 0;

    for (i = 0; i < sizeof(SkillDefs[sk].depends)/sizeof(SkillDefs[sk].depends[0]); i++) {
        reqsk = lookup_skill(SkillDefs[sk].depends[i].skill);
        if (reqsk == -1) break;
        if (SkillDefs[reqsk].flags & SkillType::DISABLED) continue;
        if (SkillDefs[reqsk].flags & SkillType::NOEXP) continue;
        reqlev = GetRealSkill(reqsk);
        if (reqlev <= curlev) {
            if (Practice(reqsk)) return 1;
            // We don't meet the reqs, and can't practice that
            // req, but we still need to check the other reqs.
            bonus = 0;
        }
    }

    if (bonus) {
        if (!Globals->REQUIRED_EXPERIENCE) {
            Study(sk, men * bonus);
        } else {
            // check if it's a nonleader and this is not it's
            // only skill
            if (Globals->SKILL_LIMIT_NONLEADERS && !IsLeader()) {
                for(const auto s: skills) {
                    if ((s->days > 0) && (s->type != sk)) {
                        return 0;
                    }
                }
            }
            // don't raise exp above the maximum days for
            // that unit
            int max = men * GetDaysByLevel(GetSkillMax(sk));
            int exp = skills.GetExp(sk);
            exp += men * bonus;
            if (exp > max) exp = max;
            skills.SetExp(sk, exp);
        }
        practiced = 1;
        event("Gets " + to_string(bonus) + " days of practice with " + SkillStrs(sk) + ".", "practice");
    }

    return bonus;
}

int Unit::IsLeader()
{
    if (GetLeaders()) return 1;
    return 0;
}

int Unit::IsNormal()
{
    if (GetMen() && !IsLeader()) return 1;
    return 0;
}

void Unit::AdjustSkills()
{
    if (!IsLeader() && Globals->SKILL_LIMIT_NONLEADERS) {
        //
        // Not a leader: can only know 1 skill
        //
        if (skills.size() > 1) {
            //
            // Find highest skill, eliminate others
            //
            unsigned int max = 0;
            Skill *maxskill = 0;
            for(const auto s: skills) {
                if (s->days > max) {
                    max = s->days;
                    maxskill = s;
                }
            }

            // In order to avoid modifying the container while iterating, we will collect the skills to remove and
            // then remove them after
            std::vector<Skill *> skillsToRemove;
            for(const auto s: skills) {
                if (s != maxskill) {
                    // Allow multiple skills if they're all
                    // magical ones
                    if ((SkillDefs[maxskill->type].flags & SkillType::MAGIC) &&
                            (SkillDefs[s->type].flags & SkillType::MAGIC) )
                        continue;
                    if ((Globals->REQUIRED_EXPERIENCE) && (s->exp > 0)) continue;
                    skillsToRemove.push_back(s);
                }
            }
            if (!skillsToRemove.empty()) {
                for(const auto s: skillsToRemove) {
                    skills.erase(s);
                    delete s;
                }
            }
        }
    }

    // Everyone: limit all skills to their maximum level
    for(const auto theskill: skills) {
        int max = GetSkillMax(theskill->type);
        if (GetRealSkill(theskill->type) >= max) {
            theskill->days = GetDaysByLevel(max) * GetMen();
        }
    }
}

int Unit::MaintCost(ARegionList& regions, ARegion *current_region)
{
    int retval = 0;
    int i;
    if (type == U_WMON || type == U_GUARD || type == U_GUARDMAGE) return 0;

    int leaders = GetLeaders();
    if (leaders < 0) leaders = 0;
    int nonleaders = GetMen() - leaders;
    if (nonleaders < 0) nonleaders = 0;

    // Handle leaders
    // Leaders are counted at maintenance_multiplier * skills in all except
    // the case where it's not being used (mages, leaders, all)
    if (Globals->MULTIPLIER_USE != GameDefs::MULT_NONE) {
        i = leaders * SkillLevels() * Globals->MAINTENANCE_MULTIPLIER;
        if (i < (leaders * Globals->LEADER_COST))
            i = leaders * Globals->LEADER_COST;
    } else
        i = leaders * Globals->LEADER_COST;
    retval += i;

    // Handle non-leaders
    // Non leaders are counted at maintenance_multiplier * skills only if
    // all characters pay that way.
    if (Globals->MULTIPLIER_USE == GameDefs::MULT_ALL) {
        i = nonleaders * SkillLevels() * Globals->MAINTENANCE_MULTIPLIER;
        if (i < (nonleaders * Globals->MAINTENANCE_COST))
            i = nonleaders * Globals->MAINTENANCE_COST;
    } else
        i = nonleaders * Globals->MAINTENANCE_COST;
    retval += i;

    // Check for any items which require item specific maintenance and handling
    for(auto it : items) {
        if (!(ItemDefs[it->type].flags & ItemType::MAINTENANCE)) continue;
        int cost = ItemDefs[it->type].baseprice * it->num;
        // Now, do a special check for NO7 victory work based on the flags.  Ideally the structure to seek
        // would be a field on the item, but that would require a ton of changes to all hundreds of item defs.
        if (ItemDefs[it->type].flags & ItemType::SEEK_ALTAR) {
            // if the unit moved, figure out if it moved toward or away from an altar
            if (initial_region) {
                // Find the nearest O_RITUAL_ALTAR from the start of turn location (initial_region)
                int start_distance = regions.FindDistanceToNearestObject(O_RITUAL_ALTAR, initial_region);
                // Find the nearest O_RITUAL_ALTAR from the current location
                int final_distance = regions.FindDistanceToNearestObject(O_RITUAL_ALTAR, current_region);
                // compute the difference.
                if (final_distance < start_distance) {
                    // If we've moved closer, halve the cost
                    cost /= 2;
                } else if (final_distance > start_distance) {
                    // We have moved away, charge 5 times as much.
                    cost *= 5;
                }
            }
        }
        retval += cost;
    }

    return retval;
}

void Unit::Short(int needed, int hunger)
{
    int i, n = 0, levels;

    if (faction->is_npc)
        return; // Don't starve monsters and the city guard!

    if (needed < 1 && hunger < 1) return;

    switch(Globals->SKILL_STARVATION) {
        case GameDefs::STARVE_MAGES:
            if (type == U_MAGE) SkillStarvation();
            return;
        case GameDefs::STARVE_LEADERS:
            if (GetLeaders()) SkillStarvation();
            return;
        case GameDefs::STARVE_ALL:
            SkillStarvation();
            return;
    }

    for (i = 0; i<= NITEMS; i++) {
        if (!(ItemDefs[ i ].type & IT_MAN)) {
            // Only men need sustenance.
            continue;
        }

        if (ItemDefs[i].type & IT_LEADER) {
            // Don't starve leaders just yet.
            continue;
        }

        while (GetMen(i)) {
            if (rng::get_random(100) < Globals->STARVE_PERCENT) {
                SetMen(i, GetMen(i) - 1);
                n++;
            }
            if (Globals->MULTIPLIER_USE == GameDefs::MULT_ALL) {
                levels = SkillLevels();
                i = levels * Globals->MAINTENANCE_MULTIPLIER;
                if (i < Globals->MAINTENANCE_COST)
                    i = Globals->MAINTENANCE_COST;
                needed -= i;
            } else
                needed -= Globals->MAINTENANCE_COST;
            hunger -= Globals->UPKEEP_MINIMUM_FOOD;
            if (needed < 1 && hunger < 1) {
                if (n) error(to_string(n) + " starve to death.");
                return;
            }
        }
    }

    // Now starve leaders
    for (int i = 0; i<= NITEMS; i++) {
        if (!(ItemDefs[ i ].type & IT_MAN)) {
            // Only men need sustenance.
            continue;
        }

        if (!(ItemDefs[i].type & IT_LEADER)) {
            // now we're doing leaders
            continue;
        }

        while (GetMen(i)) {
            if (rng::get_random(100) < Globals->STARVE_PERCENT) {
                SetMen(i, GetMen(i) - 1);
                n++;
            }
            if (Globals->MULTIPLIER_USE != GameDefs::MULT_NONE) {
                levels = SkillLevels();
                i = levels * Globals->MAINTENANCE_MULTIPLIER;
                if (i < Globals->LEADER_COST)
                    i = Globals->LEADER_COST;
                needed -= i;
            } else
                needed -= Globals->LEADER_COST;
            hunger -= Globals->UPKEEP_MINIMUM_FOOD;
            if (needed < 1 && hunger < 1) {
                if (n) error(to_string(n) + " starve to death.");
                return;
            }
        }
    }
}

int Unit::Weight()
{
    int retval = items.Weight();
    return retval;
}

int Unit::FlyingCapacity()
{
    int cap = 0;
    for(auto i : items) {
        // except ship items
        if (ItemDefs[i->type].type & IT_SHIP) continue;
        cap += ItemDefs[i->type].fly * i->num;
    }

    return cap;
}

int Unit::RidingCapacity()
{
    int cap = 0;
    for(auto i : items) {
        cap += ItemDefs[i->type].ride * i->num;
    }

    return cap;
}

int Unit::SwimmingCapacity()
{
    int cap = 0;
    for(auto i : items) {
        // except ship items
        if (ItemDefs[i->type].type & IT_SHIP) continue;
        cap += ItemDefs[i->type].swim * i->num;
    }

    return cap;
}

int Unit::WalkingCapacity()
{
    int cap = 0;
    for(auto i : items) {
        cap += ItemDefs[i->type].walk * i->num;
        if (ItemDefs[i->type].hitchItem != -1) {
            int hitch = ItemDefs[i->type].hitchItem;
            if (!(ItemDefs[hitch].flags & ItemType::DISABLED)) {
                int hitches = items.GetNum(hitch);
                int hitched = i->num;
                if (hitched > hitches) hitched = hitches;
                cap += hitched * ItemDefs[i->type].hitchwalk;
            }
        }
    }

    return cap;
}

int Unit::CanFly(int weight)
{
    if (FlyingCapacity() >= weight) return 1;
    return 0;
}

int Unit::CanReallySwim()
{
    if (IsAlive() && (SwimmingCapacity() >= items.Weight())) return 1;
    return 0;
}

int Unit::CanSwim()
{
    if (this->CanReallySwim())
        return 1;
    if ((Globals->FLIGHT_OVER_WATER != GameDefs::WFLIGHT_NONE) && this->CanFly())
        return 1;
    return 0;
}

int Unit::CanFly()
{
    int weight = items.Weight();
    return CanFly(weight);
}

int Unit::CanRide(int weight)
{
    if (RidingCapacity() >= weight) return 1;
    return 0;
}

int Unit::CanWalk(int weight)
{
    if (WalkingCapacity() >= weight) return 1;
    return 0;
}

int Unit::MoveType(ARegion *r)
{
    int weight;

    if (!r)
        r = object->region;
    weight = items.Weight();
    if (!weight)
        return M_NONE;
    if (CanFly(weight))
        return M_FLY;
    if (TerrainDefs[r->type].similar_type != R_OCEAN) {
        if (CanRide(weight))
            return M_RIDE;
        if (CanWalk(weight))
            return M_WALK;
    } else {
        /* Check if we should be able to 'swim' */
        /* This should become it's own M_TYPE sometime */
        if (CanSwim())
            return M_SWIM;
    }
    if (r->type == R_NEXUS)
        return M_WALK;
    return M_NONE;
}

static int ContributesToMovement(int movetype, int item)
{
    switch(movetype) {
        case M_WALK:
            if (ItemDefs[item].walk > 0)
                return ItemDefs[item].walk;
            break;
        case M_RIDE:
            if (ItemDefs[item].ride > 0)
                return ItemDefs[item].ride;
            break;
        case M_FLY:
            if (ItemDefs[item].fly > 0)
                return ItemDefs[item].fly;
            break;
        case M_SWIM:
            // incomplete ship items do have a "swimming"
            // capacity given, but don't help us to swim
            if (ItemDefs[item].type & IT_SHIP)
                return 0;
            if (ItemDefs[item].swim > 0)
                return ItemDefs[item].swim;
            break;
    }

    return 0;
}

int Unit::CalcMovePoints(ARegion *r)
{
    int movetype, speed, weight, cap, hitches;

    movetype = MoveType(r);
    speed = 0;
    if (movetype == M_NONE)
        return 0;

    for(auto i : items) {
        if (ContributesToMovement(movetype, i->type)) {
            if (ItemDefs[i->type].speed > speed)
                speed = ItemDefs[i->type].speed;
        }
    }
    weight = items.Weight();
    while (weight > 0 && speed > 0) {
        for(auto i : items) {
            cap = ContributesToMovement(movetype, i->type);
            if (ItemDefs[i->type].speed == speed) {
                if (cap > 0) weight -= cap * i->num;
                else if (ItemDefs[i->type].hitchItem != -1) {
                    hitches = items.GetNum(ItemDefs[i->type].hitchItem);
                    if (i->num < hitches) hitches = i->num;
                    weight -= hitches * ItemDefs[i->type].hitchwalk;
                }
            }
        }
        if (weight > 0) {
            // Hm, can't move at max speed.  There must be
            // items with different speeds, and we have to
            // use some of the slower ones...
            speed--;
        }
    }

    if (weight > 0) return 0; // not that this should be possible!

    if (movetype == M_FLY && GetAttribute("wind") > 0) speed += Globals->FLEET_WIND_BOOST;

    if (speed > Globals->MAX_SPEED) speed = Globals->MAX_SPEED;

    return speed;
}

int Unit::CanMoveTo(ARegion *r1, ARegion *r2)
{
    if (r1 == r2) return 1;

    int exit = 1;
    int i;
    int dir;

    for (i=0; i<NDIRS; i++) {
        if (r1->neighbors[i] == r2) {
            exit = 0;
            dir = i;
            break;
        }
    }
    if (exit) return 0;
    exit = 1;
    for (i=0; i<NDIRS; i++) {
        if (r2->neighbors[i] == r1) {
            exit = 0;
            break;
        }
    }
    if (exit) return 0;

    int mt = MoveType();
    if (((TerrainDefs[r1->type].similar_type == R_OCEAN) ||
                (TerrainDefs[r2->type].similar_type == R_OCEAN)) &&
            (!CanSwim() || GetFlag(FLAG_NOCROSS_WATER)))
        return 0;
    int mp = CalcMovePoints() - moved;
    if (mp < (r2->MoveCost(mt, r1, dir, 0))) return 0;
    return 1;
}

int Unit::CanCatch(ARegion *r, Unit *u)
{
    return faction->CanCatch(r, u);
}

int Unit::CanSee(ARegion *r, Unit *u, int practice)
{
    return faction->CanSee(r, u, practice);
}

int Unit::AmtsPreventCrime(Unit *u)
{
    if (!u) return 0;

    int amulets = items.GetNum(I_AMULETOFTS);
    if ((u->items.GetNum(I_RINGOFI) < 1) || (amulets < 1)) return 0;
    int men = GetMen();
    if (men <= amulets) return 1;
    if (!Globals->PROPORTIONAL_AMTS_USAGE) return 0;
    if (rng::get_random(men) < amulets) return 1;
    return 0;
}

AttitudeType Unit::GetAttitude(ARegion *r, Unit *u)
{
    if (faction == u->faction) return AttitudeType::ALLY;
    AttitudeType att = faction->get_attitude(u->faction->num);
    if (att >= AttitudeType::FRIENDLY && att >= faction->defaultattitude) return att;

    if (CanSee(r, u) == 2)
        return att;
    else
        return faction->defaultattitude;
}

int Unit::Hostile()
{
    if (type != U_WMON) return 0;
    int retval = 0;
    for(auto i : items) {
        if (ItemDefs[i->type].type & IT_MONSTER) {
            auto monster = FindMonster(ItemDefs[i->type].abr.c_str(), (ItemDefs[i->type].type & IT_ILLUSION))->get();
            int hos = monster.hostile;
            if (hos > retval) retval = hos;
        }
    }
    return retval;
}

int Unit::Forbids(ARegion *r, Unit *u)
{
    if (guard != GUARD_GUARD) return 0;
    if (!IsAlive()) return 0;
    if (!CanSee(r, u, Globals->SKILL_PRACTICE_AMOUNT > 0)) return 0;
    if (!CanCatch(r, u)) return 0;
    if (GetAttitude(r, u) < AttitudeType::NEUTRAL) return 1;
    return 0;
}

/* This function was modified to either return the amount of
   taxes this unit is eligible for (numtaxers == 0) or the
   number of taxing men (numtaxers > 0).
*/
int Unit::Taxers(int numtaxers)
{
    int totalMen = GetMen();
    int illusions = 0;
    int creatures = 0;
    int taxers = 0;
    int basetax = 0;
    int weapontax = 0;
    int armortax = 0;

    // check out items
    int numMelee= 0;
    int numUsableMelee = 0;
    int numBows = 0;
    int numUsableBows = 0;
    int numMounted= 0;
    int numUsableMounted = 0;
    int numMounts = 0;
    int numUsableMounts = 0;
    int numBattle = 0;
    int numUsableBattle = 0;
    int numArmor = 0;

    for(auto item : items) {
        auto pBat = FindBattleItem(ItemDefs[item->type].abr.c_str());

        if ((ItemDefs[item->type].type & IT_BATTLE) && pBat && (pBat->get().flags & BattleItemType::SPECIAL)) {
            // Only consider offensive items
            if (
                (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_BATTLE_ITEM) &&
                (!(pBat->get().flags & BattleItemType::MAGEONLY) || type == U_MAGE || type == U_APPRENTICE)
            ) {
                numUsableBattle += item->num;
                numBattle += item->num;
                continue; // Don't count this as a weapon as well!
            }
            if (Globals->WHO_CAN_TAX & GameDefs::TAX_BATTLE_ITEM) {
                numBattle += item->num;
                continue; // Don't count this as a weapon as well!
            }
        }

        if (ItemDefs[item->type].type & IT_WEAPON) {
            auto weapon = FindWeapon(ItemDefs[item->type].abr.c_str())->get();
            int num = item->num;
            int basesk = 0;
            int sk = lookup_skill(weapon.baseSkill);
            if (sk != -1) basesk = GetSkill(sk);
            if (basesk == 0) {
                sk = lookup_skill(weapon.orSkill);
                if (sk != -1) basesk = GetSkill(sk);
            }
            if (!(weapon.flags & WeaponType::NEEDSKILL)) {
                if (basesk) {
                    numUsableMelee += num;
                }
                numMelee += num;
            } else if (weapon.flags & WeaponType::NOFOOT) {
                if (basesk) {
                    numUsableMounted += num;
                }
                numMounted += num;
            } else {
                if (weapon.flags & WeaponType::RANGED) {
                    if (basesk) {
                        numUsableBows += num;
                    }
                    numBows += num;
                } else {
                    if (basesk) {
                        numUsableMelee += num;
                    }
                    numMelee += num;
                }
            }
        }

        if (ItemDefs[item->type].type & IT_MOUNT) {
            auto pm = FindMount(ItemDefs[item->type].abr.c_str()).value().get();
            if (pm.skill) {
                int sk = lookup_skill(pm.skill);
                if (pm.minBonus <= GetSkill(sk))
                    numUsableMounts += item->num;
            } else
                numUsableMounts += item->num;
            numMounts += item->num;
        }

        if (ItemDefs[item->type].type & IT_MONSTER) {
            if (ItemDefs[item->type].type & IT_ILLUSION) illusions += item->num;
            else creatures += item->num;
        }

        if (ItemDefs[item->type].type & IT_ARMOR) {
            numArmor += item->num;
        }
    }


    // Ok, now process the counts!
    if ((Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE) ||
        ((Globals->WHO_CAN_TAX & GameDefs::TAX_COMBAT_SKILL) &&
         GetSkill(S_COMBAT)) ||
        ((Globals->WHO_CAN_TAX & GameDefs::TAX_BOW_SKILL) &&
         (GetSkill(S_CROSSBOW) || GetSkill(S_LONGBOW))) ||
        ((Globals->WHO_CAN_TAX & GameDefs::TAX_RIDING_SKILL) &&
         GetSkill(S_RIDING)) ||
        ((Globals->WHO_CAN_TAX & GameDefs::TAX_STEALTH_SKILL) &&
         GetSkill(S_STEALTH))) {
        basetax = totalMen;
        taxers = totalMen;

        // Weapon tax bonus
        if ((Globals->WHO_CAN_TAX & GameDefs::TAX_ANYONE) ||
        ((Globals->WHO_CAN_TAX & GameDefs::TAX_COMBAT_SKILL) &&
         GetSkill(S_COMBAT)) ||
        ((Globals->WHO_CAN_TAX & GameDefs::TAX_STEALTH_SKILL) &&
         GetSkill(S_STEALTH))) {
            if (numUsableMounted > numUsableMounts) {
                weapontax = numUsableMounts;
            } else {
                weapontax = numUsableMounted;
            }
            weapontax += numMelee;
         }

        if (((Globals->WHO_CAN_TAX & GameDefs::TAX_BOW_SKILL) &&
         (GetSkill(S_CROSSBOW) || GetSkill(S_LONGBOW)))) {
            weapontax += numUsableBows;
         }
        if ((Globals->WHO_CAN_TAX & GameDefs::TAX_RIDING_SKILL) &&
         GetSkill(S_RIDING)) {
            if (weapontax < numUsableMounts) weapontax = numUsableMounts;
         }

    } else {

        if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_WEAPON) {
            if (numUsableMounted > numUsableMounts) {
                weapontax = numUsableMounts;
                taxers = numUsableMounts;
                numMounts -= numUsableMounts;
                numUsableMounts = 0;
            } else {
                weapontax = numUsableMounted;
                taxers = numUsableMounted;
                numMounts -= numUsableMounted;
                numUsableMounts -= numUsableMounted;
            }
            weapontax += numMelee + numUsableBows;
            taxers += numMelee + numUsableBows;
        } else if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANY_WEAPON) {
            weapontax = numMelee + numBows + numMounted;
            taxers = numMelee + numBows + numMounted;
        } else {
            if (Globals->WHO_CAN_TAX &
                    GameDefs::TAX_MELEE_WEAPON_AND_MATCHING_SKILL) {
                if (numUsableMounted > numUsableMounts) {
                    weapontax += numUsableMounts;
                    taxers += numUsableMounts;
                    numMounts -= numUsableMounts;
                    numUsableMounts = 0;
                } else {
                    weapontax += numUsableMounted;
                    taxers += numUsableMounted;
                    numMounts -= numUsableMounted;
                    numUsableMounts -= numUsableMounted;
                }
                weapontax += numUsableMelee;
                taxers += numUsableMelee;
            }
            if (Globals->WHO_CAN_TAX &
                    GameDefs::TAX_BOW_SKILL_AND_MATCHING_WEAPON) {
                weapontax += numUsableBows;
                taxers += numUsableBows;
            }
        }

        if (Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE) {
            weapontax += numMounts;
            taxers += numMounts;
        }
        else if (Globals->WHO_CAN_TAX & GameDefs::TAX_HORSE_AND_RIDING_SKILL) {
            weapontax += numUsableMounts;
            taxers += numUsableMounts;
        }

        if (Globals->WHO_CAN_TAX & GameDefs::TAX_BATTLE_ITEM) {
            weapontax += numBattle;
            taxers += numBattle;
        }
        else if (Globals->WHO_CAN_TAX & GameDefs::TAX_USABLE_BATTLE_ITEM) {
            weapontax += numUsableBattle;
            taxers += numUsableBattle;
        }

    }

    // Ok, all the items categories done - check for mages taxing
    if (type == U_MAGE) {
        if (Globals->WHO_CAN_TAX & GameDefs::TAX_ANY_MAGE) {
            basetax = totalMen;
            taxers = totalMen;
        }
        else {
            if (Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_COMBAT_SPELL) {
                if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_DAMAGE) &&
                        SkillDefs[combat].flags & SkillType::DAMAGE) {
                    basetax = totalMen;
                    taxers = totalMen;
                }

                if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_FEAR) &&
                        SkillDefs[combat].flags & SkillType::FEAR) {
                    basetax = totalMen;
                    taxers = totalMen;
                }

                if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_OTHER) &&
                        SkillDefs[combat].flags & SkillType::MAGEOTHER) {
                    basetax = totalMen;
                    taxers = totalMen;
                }
            } else {
                for(const auto s: skills) {
                    if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_DAMAGE) &&
                            SkillDefs[s->type].flags & SkillType::DAMAGE) {
                        basetax = totalMen;
                        taxers = totalMen;
                        break;
                    }
                    if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_FEAR) &&
                            SkillDefs[s->type].flags & SkillType::FEAR) {
                        basetax = totalMen;
                        taxers = totalMen;
                        break;
                    }
                    if ((Globals->WHO_CAN_TAX & GameDefs::TAX_MAGE_OTHER) &&
                            SkillDefs[s->type].flags & SkillType::MAGEOTHER) {
                        basetax = totalMen;
                        taxers = totalMen;
                        break;
                    }
                }
            }
        }
    }

    armortax = numArmor;

    // Check for overabundance
    if (weapontax > totalMen) weapontax = totalMen;
    if (armortax > weapontax) armortax = weapontax;

    // Adjust basetax in case of weapon taxation
    if (basetax < weapontax) basetax = weapontax;

    // Now check for an overabundance of tax enabling objects
    if (taxers > totalMen) taxers = totalMen;

    // And finally for creatures
    if (Globals->WHO_CAN_TAX & GameDefs::TAX_CREATURES) {
        basetax += creatures;
        taxers += creatures;
    }

    if (Globals->WHO_CAN_TAX & GameDefs::TAX_ILLUSIONS) {
        basetax += illusions;
        taxers += illusions;
    }

    if (numtaxers) return(taxers);

    int taxes = Globals->TAX_BASE_INCOME * basetax
        + Globals->TAX_BONUS_WEAPON * weapontax
        + Globals->TAX_BONUS_ARMOR * armortax;
    return(taxes);
}

int Unit::GetFlag(int x)
{
    return (flags & x);
}

void Unit::SetFlag(int x, int val)
{
    if (val) flags = flags | x; // Set all bits specified by x
    else flags = flags & ~x; // Unset all bits specified by x
}

void Unit::CopyFlags(Unit *x)
{
    flags = x->flags;
    guard = GUARD_NONE;
    if (Taxers(1)) {
        if (x->guard != GUARD_SET && x->guard != GUARD_ADVANCE)
            guard = x->guard;
    } else {
        if (x->guard == GUARD_AVOID)
            guard = GUARD_AVOID;
        SetFlag(FLAG_AUTOTAX, 0);
    }
    reveal = x->reveal;
}

int Unit::get_battle_item(const std::string &item)
{
    int item_num = lookup_item(item);
    if (item_num == -1) return -1;

    int num = items.GetNum(item_num);
    if (num < 1) return -1;

    if (!(ItemDefs[item_num].type & IT_BATTLE)) return -1;
    // Exclude weapons.  They will be handled later.
    if (ItemDefs[item_num].type & IT_WEAPON) return -1;
    items.SetNum(item_num, num - 1);
    return item_num;
}

int Unit::get_armor(const std::string &item, int ass)
{
    int item_num = lookup_item(item);
    auto armor_type = FindArmor(ItemDefs[item_num].abr.c_str());

    if (!armor_type) return -1;
    if (ass && !(armor_type->get().flags & ArmorType::USEINASSASSINATE)) return -1;

    int num = items.GetNum(item_num);
    if (num < 1) return -1;

    if (!(ItemDefs[item_num].type & IT_ARMOR)) return -1;
    items.SetNum(item_num, num - 1);
    return item_num;
}

int Unit::get_mount(const std::string &item, int canFly, int canRide, int &bonus)
{
    bonus = 0;

    // This region doesn't allow riding or flying, so no mounts, bail
    if (!canFly && !canRide) return -1;

    int item_num = lookup_item(item);
    auto pMnt = FindMount(ItemDefs[item_num].abr.c_str()).value().get();

    int num = items.GetNum(item_num);
    if (num < 1) return -1;

    if (canFly) {
        // If the mount cannot fly, and the region doesn't allow
        // riding mounts, bail
        if (!ItemDefs[item_num].fly && !canRide) return -1;
    } else {
        // This region allows riding mounts, so if the mount
        // can not carry at a riding level, bail
        if (!ItemDefs[item_num].ride) return -1;
    }

    if (pMnt.skill) {
        int sk = lookup_skill(pMnt.skill);
        bonus = GetSkill(sk);
        if (bonus < pMnt.minBonus) {
            // Unit isn't skilled enough for this mount
            bonus = 0;
            return -1;
        }
        // Limit to max mount bonus;
        if (bonus > pMnt.maxBonus) bonus = pMnt.maxBonus;
        // If the mount can fly and the terrain doesn't allow
        // flying mounts, limit the bonus to the maximum hampered
        // bonus allowed by the mount
        if (ItemDefs[item_num].fly && !canFly) {
            if (bonus > pMnt.maxHamperedBonus)
                bonus = pMnt.maxHamperedBonus;
        }

        // Practice the mount's skill
        Practice(sk);
    }

    // Remove the mount from the unit to attach it to the soldier
    // UNLESS it IS the soldier (looking at you, Centaurs)
    if (!(ItemDefs[item_num].type & IT_MAN))
        items.SetNum(item_num, num - 1);
    return item_num;
}

int Unit::get_weapon(
    const std::string &item, int riding, int ridingBonus, int &attackBonus, int &defenseBonus, int &attacks, int &hitDamage
)
{
    int item_num = lookup_item(item);
    auto weapontype = FindWeapon(ItemDefs[item_num].abr.c_str());

    if (!weapontype) return -1;
    auto weapon = weapontype->get();

    int num = items.GetNum(item_num);
    if (num < 1) return -1;

    if (!(ItemDefs[item_num].type & IT_WEAPON)) return -1;

    attackBonus = 0;
    defenseBonus = 0;
    attacks = 1;

    // Found a weapon, check flags and skills
    int baseSkillLevel = CanUseWeapon(weapon, riding);
    // returns -1 if weapon cannot be used, else the usable skill level
    if (baseSkillLevel == -1) return -1;

    // Attack and defense skill
    attackBonus = baseSkillLevel + weapon.attackBonus;
    if (weapon.flags & WeaponType::NOATTACKERSKILL)
        defenseBonus = weapon.defenseBonus;
    else
        defenseBonus = baseSkillLevel + weapon.defenseBonus;
    // Riding bonus
    if (weapon.flags & WeaponType::RIDINGBONUS) attackBonus += ridingBonus;
    if (weapon.flags & (WeaponType::RIDINGBONUSDEFENSE|WeaponType::RIDINGBONUS))
        defenseBonus += ridingBonus;
    // Number of attacks
    attacks = weapon.numAttacks;
    // Note: NUM_ATTACKS_SKILL must be > NUM_ATTACKS_HALF_SKILL
    if (attacks >= WeaponType::NUM_ATTACKS_SKILL)
        attacks += baseSkillLevel - WeaponType::NUM_ATTACKS_SKILL;
    else if (attacks >= WeaponType::NUM_ATTACKS_HALF_SKILL)
        attacks += (baseSkillLevel +1)/2 - WeaponType::NUM_ATTACKS_HALF_SKILL;
    // Sanity check
    if (attacks == 0) attacks = 1;

    hitDamage = weapon.hitDamage;
    // // Check if attackDamage is based on skill level
    // // >= used in case NUM_DAMAGE_SKILL+1
    if (hitDamage >= WeaponType::NUM_DAMAGE_SKILL) {
        hitDamage = hitDamage - WeaponType::NUM_DAMAGE_SKILL + baseSkillLevel;
    // >= used in case NUM_DAMAGE_HALF_SKILL+1
    } else if (hitDamage >= WeaponType::NUM_DAMAGE_HALF_SKILL) {
        hitDamage = hitDamage - WeaponType::NUM_DAMAGE_HALF_SKILL + (baseSkillLevel + 1)/2;
    }

    // get the weapon
    items.SetNum(item_num, num-1);
    return item_num;
}

void Unit::MoveUnit(Object *toobj)
{
    if (object) std::erase(object->units, this);
    object = toobj;
    if (object) {
        object->units.push_back(this);
        ObjectType& ob = ObjectDefs[object->type];
        if (object->GetOwner() == this  && ob.flags & ObjectType::GRANTSKILL) {
            if (faction->skills.GetDays(ob.granted_skill) < ob.granted_level) {
                faction->shows.push_back({ .skill = ob.granted_skill, .level = ob.granted_level });
                faction->skills.SetDays(ob.granted_skill, ob.granted_level);
            }
        }
    }
}

void Unit::DiscardUnfinishedShips() {
    int discard = 0;
    // remove all unfinished ship-type items
    for (int i=0; i<NITEMS; i++) {
        if (ItemDefs[i].type & IT_SHIP) {
            if (items.GetNum(i) > 0) discard = 1;
            items.SetNum(i,0);
        }
    }
    if (discard > 0) event("discards all unfinished ships.", "discard");
}

void Unit::event(const string& message, const string& category, ARegion *r)
{
    faction->event(message, category, r, this);
}

void Unit::error(const string& s) {
    faction->error(s, this);
}

int Unit::GetAttribute(char const *attrib)
{
    auto ap = FindAttrib(attrib);
    if (!ap) return 0;
    AString temp;
    int base = 0;
    int bonus = 0;
    int monbase = -1;
    int monbonus = 0;

    if (ap->get().flags & AttribModType::CHECK_MONSTERS) {
        for(auto i : items) {
            if (ItemDefs[i->type].type & IT_MONSTER) {
                auto monster = FindMonster(ItemDefs[i->type].abr.c_str(), (ItemDefs[i->type].type & IT_ILLUSION))->get();
                int val = 0;
                temp = attrib;
                if (temp == "observation") val = monster.obs;
                else if (temp == "stealth") val = monster.stealth;
                else if (temp == "tactics") val = monster.tactics;
                else continue;
                if (monbase == -1) monbase = val;
                else if (ap->get().flags & AttribModType::USE_WORST)
                    monbase = (val < monbase) ? val : monbase;
                else
                    monbase = (val > monbase) ? val : monbase;
            }
        }
    }

    for (int index = 0; index < 5; index++) {
        int val = 0;
        if (ap->get().mods[index].flags & AttribModItem::SKILL) {
            int sk = lookup_skill(ap->get().mods[index].ident);
            val = GetAvailSkill(sk);
            if (ap->get().mods[index].modtype == AttribModItem::UNIT_LEVEL_HALF) {
                val = ((val + 1)/2) * ap->get().mods[index].val;
            } else if (ap->get().mods[index].modtype == AttribModItem::CONSTANT) {
                val = ap->get().mods[index].val;
            } else {
                val *= ap->get().mods[index].val;
            }
        } else if (ap->get().mods[index].flags & AttribModItem::ITEM) {
            val = 0;
            int item = lookup_item(ap->get().mods[index].ident);
            if (item != -1) {
                if (ItemDefs[item].type & IT_MAGEONLY
                    && type != U_MAGE
                    && type != U_APPRENTICE
                    && type != U_GUARDMAGE) {
                    // Ignore mage only items for non-mages
                } else if (ap->get().mods[index].flags & AttribModItem::PERMAN) {
                    int men = GetMen();
                    if (men > 0 && men <= items.GetNum(item))
                        val = ap->get().mods[index].val;
                } else {
                    if (items.GetNum(item) > 0)
                        val = ap->get().mods[index].val;
                }
            }
        } else if (ap->get().mods[index].flags & AttribModItem::FLAGGED) {
            if (ap->get().mods[index].ident == "invis")
                val = (GetFlag(FLAG_INVIS) ? ap->get().mods[index].val : 0);
            if (ap->get().mods[index].ident == "guard")
                val = (guard == GUARD_GUARD ? ap->get().mods[index].val : 0);

        }
        if (ap->get().mods[index].flags & AttribModItem::NOT)
            val = ((val == 0) ? ap->get().mods[index].val : 0);
        if (val && ap->get().mods[index].modtype == AttribModItem::FORCECONSTANT)
            return val;
        // Only flags can add to monster bonuses
        if (ap->get().mods[index].flags & AttribModItem::FLAGGED) {
            if (ap->get().flags & AttribModType::CHECK_MONSTERS) monbonus += val;
        }
        if (ap->get().mods[index].flags & AttribModItem::CUMULATIVE)
            base += val;
        else if (val > bonus) bonus = val;
    }

    base += bonus;

    if (monbase != -1) {
        monbase += monbonus;
        if (GetMen() > 0) {
            if (ap->get().flags & AttribModType::USE_WORST)
                base = (monbase < base) ? monbase : base;
            else
                base = (monbase > base) ? monbase : base;
        }
        else
            base = monbase; // monster units have no men
    }
    return base;
}

int Unit::PracticeAttribute(char const *attrib)
{
    auto ap = FindAttrib(attrib);
    if (!ap) return 0;
    for (int index = 0; index < 5; index++) {
        if (ap->get().mods[index].flags & AttribModItem::SKILL) {
            int sk = lookup_skill(ap->get().mods[index].ident);
            if (sk != -1)
                if (Practice(sk)) return 1;
        }
    }
    return 0;
}

int Unit::GetProductionBonus(int item)
{
    int bonus = 0;
    if (ItemDefs[item].mult_item != -1)
        bonus = items.GetNum(ItemDefs[item].mult_item);
    else
        bonus = GetMen();
    if (bonus > GetMen()) bonus = GetMen();
    return bonus * ItemDefs[item].mult_val;
}

int Unit::SkillLevels()
{
    int levels = 0;
    for(const auto s: skills) {
        levels += GetLevelByDays(s->days/GetMen());
    }
    return levels;
}

Skill *Unit::GetSkillObject(int sk)
{
    for(const auto s: skills) {
        if (s->type == sk)
            return s;
    }
    return NULL;
}

void Unit::SkillStarvation()
{
    int can_forget[NSKILLS];
    int count = 0;
    int i;
    for (i = 0; i < NSKILLS; i++) {
        if (SkillDefs[i].flags & SkillType::DISABLED) {
            can_forget[i] = 0;
            continue;
        }
        if (GetSkillObject(i)) {
            can_forget[i] = 1;
            count++;
        } else {
            can_forget[i] = 0;
        }
    }
    for (i = 0; i < NSKILLS; i++) {
        if (!can_forget[i]) continue;
        Skill *si = GetSkillObject(i);
        for (int j=0; j < NSKILLS; j++) {
            if (SkillDefs[j].flags & SkillType::DISABLED) continue;
            Skill *sj = GetSkillObject(j);
            int dependancy_level = 0;
            unsigned int c;
            for (c=0;c < sizeof(SkillDefs[i].depends)/sizeof(SkillDefs[i].depends[0]);c++) {
                AString skname = SkillDefs[i].depends[c].skill;
                if (skname == SkillDefs[j].abbr) {
                    dependancy_level = SkillDefs[i].depends[c].level;
                    break;
                }
            }
            if (dependancy_level > 0) {
                if (GetLevelByDays(sj->days) == GetLevelByDays(si->days)) {
                    can_forget[j] = 0;
                    count--;
                }
            }
        }
    }
    if (!count) {
        for(auto i : items) {
            if (ItemDefs[i->type].type & IT_MAN) {
                count += items.GetNum(i->type);
                items.SetNum(i->type, 0);
            }
        }
        error(to_string(count) + " starve to death.");
        return;
    }
    count = rng::get_random(count)+1;
    for (i = 0; i < NSKILLS; i++) {
        if (can_forget[i]) {
            if (--count == 0) {
                Skill *s = GetSkillObject(i);
                error(string("Starves and forgets one level of ") + SkillDefs[i].name + ".");
                switch(GetLevelByDays(s->days)) {
                    case 1:
                        s->days -= 30;
                        if (s->days <= 0)
                            ForgetSkill(i);
                        break;
                    case 2:
                        s->days -= 60;
                        break;
                    case 3:
                        s->days -= 90;
                        break;
                    case 4:
                        s->days -= 120;
                        break;
                    case 5:
                        s->days -= 150;
                        break;
                }
            }
        }
    }
    return;
}

int Unit::CanUseWeapon(const WeaponType& weapon, int riding)
{
    if (riding == -1 && (weapon.flags & WeaponType::NOFOOT)) return -1;
    if (weapon.flags & WeaponType::NOMOUNT) return -1;

    int baseSkillLevel = 0;
    int tempSkillLevel = 0;

    int bsk, orsk;
    AString skname;
    if (weapon.baseSkill != NULL) {
        bsk = lookup_skill(weapon.baseSkill);
        if (bsk != -1) baseSkillLevel = GetSkill(bsk);
    }

    if (weapon.orSkill != NULL) {
        orsk = lookup_skill(weapon.orSkill);
        if (orsk != -1) tempSkillLevel = GetSkill(orsk);
    }

    if (tempSkillLevel > baseSkillLevel) {
        baseSkillLevel = tempSkillLevel;
        Practice(orsk);
    } else
        Practice(bsk);

    if (weapon.flags & WeaponType::NEEDSKILL && !baseSkillLevel) return -1;

    return baseSkillLevel;
}

#include "production.h"
#include "logger.hpp"
#include "items.h"
#include "skills.h"
#include "gamedata.h"
#include "rng.hpp"

Production::Production(int it, int maxamt)
    : itemtype(it), baseamount(amount), amount(maxamt), productivity(10)
{
    if (Globals->RANDOM_ECONOMY)
        amount += rng::get_random(maxamt);
    skill = lookup_skill(ItemDefs[it].pSkill);
}

void Production::write_out(std::ostream& f)
{
    f << (itemtype == -1 ? "NO_ITEM" : ItemDefs[itemtype].abr) << '\n';
    f << amount << '\n';
    f << baseamount << '\n';
    if (itemtype == I_SILVER) {
        f << (skill == -1 ? "NO_SKILL" : SkillDefs[skill].abbr) << '\n';
    }
    f << productivity << '\n';
}

void Production::read_in(std::istream& f)
{
    std::string temp;

    f >> std::ws >> temp;
    itemtype = lookup_item(temp);

    f >> amount;
    f >> baseamount;

    if (itemtype == I_SILVER)
        f >> std::ws >> temp;
    else
        temp = ItemDefs[itemtype].pSkill;

    skill = lookup_skill(temp);

    f >> productivity;
}

std::string Production::write_report() {
    return item_string(itemtype, amount);
}

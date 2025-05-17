#include "market.h"
#include "items.h"
#include "logger.hpp"
#include "gamedata.h"
#include <utility>

void Market::post_turn(int population, int wages)
{
    // Nothing to do to the markets.
    if (!(Globals->VARIABLE_ECONOMY)) return;

    //
    // Unlimited, unchanging market.
    //
    if (amount == -1) return;

    if (ItemDefs[item].type & IT_MAN) {
        float ratio = ItemDefs[item].baseprice / (10 * (float)Globals->BASE_MAN_COST);
        // hack: included new wage factor of ten in float assignment above
        price = (int)((float) wages * 4 * ratio);
        if (ItemDefs[item].type & IT_LEADER)
            amount = population / 125;
        else
            amount = population / 25;
        return;
    }

    int tarprice = price;
    if (amount) {
        int fluctuation = (baseprice * activity)/amount;
        if (type == MarketType::M_BUY)
            tarprice = (2 * baseprice + fluctuation) / 2;
        else
            tarprice = (3 * baseprice - fluctuation) / 2;
    }
    price = price + (tarprice - price) / 5;

    if (population <= minpop)
        amount = minamt;
    else if (population >= maxpop)
        amount = maxamt;
    else {
        amount = minamt + ((maxamt - minamt) * (population - minpop)) / (maxpop - minpop);
    }
}

void Market::write_out(std::ostream& f)
{
    f << static_cast<std::underlying_type_t<MarketType>>(type) << '\n';
    f << (item == -1 ? "NO_ITEM" : ItemDefs[item].abr) << '\n';
    f << price << '\n';
    f << amount << '\n';
    f << minpop << '\n';
    f << maxpop << '\n';
    f << minamt << '\n';
    f << maxamt << '\n';
    f << baseprice << '\n';
}

void Market::read_in(std::istream& f)
{
    int t;
    f >> t;

    type = MarketType{t};

    std::string temp;
    f >> std::ws >> temp;
    item = lookup_item(temp);

    f >> price;
    f >> amount;
    f >> minpop;
    f >> maxpop;
    f >> minamt;
    f >> maxamt;
    f >> baseprice;
}

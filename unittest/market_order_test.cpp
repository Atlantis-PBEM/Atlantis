#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"Market Orders"> market_order_suite = []
{
  using namespace ut;

  "Multiple buy orders of same type are merged"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name = "Test Faction";
    Faction *faction = helper.create_faction(name);
    Unit *unit = helper.get_first_unit(faction);
    unit->items.SetNum(I_SILVER, 8000);
    ARegion *region = unit->object->region;
    Unit *unit2 = helper.create_unit(faction, region);
    unit2->items.SetNum(I_LEADERS, 1);
    unit2->items.SetNum(I_SILVER, 8000);

    int item_id;
    int max_amount = 0;
    int price = 0;
    for(const auto market : region->markets) {
      // M_SELL markets are what the region wants people to sell (ie, what it wants to buy).
      if (market->type == Market::MarketType::M_SELL) continue;
      // Don't buy men.
      if (ItemDefs[market->item].type & IT_MAN) continue;
      // Just for simplicity of testing, make the amount evenly divisible by 4.
      market->amount = market->amount ? (market->amount / 4) * 4 : 0;
      // Skip over things that don't have a quantity or price.
      if (market->amount == 0 || market->price == 0) continue;
      item_id = market->item;
      max_amount = market->amount;
      price = market->price;
      break;
    }

    expect(max_amount > 0); // silly, just make sure we have a market item that we found.

    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    for (int i = 0; i < max_amount; i++) {
      ss << "buy 1 " << ItemDefs[item_id].abr << std::endl;
      ss << "@buy 1 " << ItemDefs[item_id].abr << std::endl;
    }
    ss << "unit 3\n";
    ss << "buy " << std::to_string(max_amount) << ' ' << ItemDefs[item_id].abr << std::endl;
    ss << "@buy " << std::to_string(max_amount) << ' ' << ItemDefs[item_id].abr << std::endl;

    // each unit should end up with about 25% for each buy order.
    helper.parse_orders(faction->num, ss);
    helper.run_buy_orders();

    expect(unit->items.GetNum(item_id) == (max_amount / 2));
    expect(unit2->items.GetNum(item_id) == (max_amount / 2));

    expect(faction->errors.size() == 0_ul);
    expect(faction->events.size() == 4_ul);
    std::string amt_string = item_string(item_id, max_amount / 4);
    expect(faction->events[0].message == "Buys " + amt_string + " at $" + std::to_string(price) + " each.");
    expect(faction->events[0].unit == unit);
    expect(faction->events[1].message == "Buys " + amt_string + " at $" + std::to_string(price) + " each.");
    expect(faction->events[1].unit == unit);
    expect(faction->events[2].message == "Buys " + amt_string + " at $" + std::to_string(price) + " each.");
    expect(faction->events[2].unit == unit2);
    expect(faction->events[3].message == "Buys " + amt_string + " at $" + std::to_string(price) + " each.");
    expect(faction->events[3].unit == unit2);

    // Check the unit recurring orders.  There should be max_amount/2 recurring buy orders for each.
    expect(unit->oldorders.size() == static_cast<size_t>(max_amount));
    expect(unit2->oldorders.size() == 1_ul);
    // And make sure the orders are buy orders.
    expect(unit->oldorders.front() == "@buy 1 " + std::string(ItemDefs[item_id].abr));
    expect(unit->oldorders.back() == "@buy 1 " + std::string(ItemDefs[item_id].abr));
    expect(unit2->oldorders.front() == "@buy " + std::to_string(max_amount) + ' ' + std::string(ItemDefs[item_id].abr));
  };

  "Multiple sell orders of same type are merged"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name = "Test Faction";
    Faction *faction = helper.create_faction(name);
    Unit *unit = helper.get_first_unit(faction);
    ARegion *region = unit->object->region;
    Unit *unit2 = helper.create_unit(faction, region);
    unit2->items.SetNum(I_LEADERS, 1);

    int item_id;
    int max_amount = 0;
    int price = 0;
    for(const auto market : region->markets) {
      // M_BUY markets are what the region wants people to buy (ie, what it wants to sell).
      if (market->type == Market::MarketType::M_BUY) continue;
      // Don't sell men, we don't do slavery.
      if (ItemDefs[market->item].type & IT_MAN) continue;
      // Just for simplicity of testing, make the amount evenly divisible by 4.
      market->amount = market->amount ? (market->amount / 4) * 4 : 0;
      // Skip over things that don't have a quantity or price.
      if (market->amount == 0 || market->price == 0) continue;
      item_id = market->item;
      max_amount = market->amount;
      price = market->price;
      break;
    }

    // Make sure they have enough to sell the items.
    unit->items.SetNum(item_id, max_amount * 4);
    unit2->items.SetNum(item_id, max_amount * 4);

    expect(max_amount > 0); // silly, just make sure we have a market item that we found.

    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    for (int i = 0; i < max_amount; i++) {
      ss << "sell 1 " << ItemDefs[item_id].abr << std::endl;
      ss << "@sell 1 " << ItemDefs[item_id].abr << std::endl;
    }
    ss << "unit 3\n";
    ss << "sell " << std::to_string(max_amount) << ' ' << ItemDefs[item_id].abr << std::endl;
    ss << "@sell " << std::to_string(max_amount) << ' ' << ItemDefs[item_id].abr << std::endl;

    // each unit should end up with about 25% for each buy order.
    helper.parse_orders(faction->num, ss);
    helper.run_sell_orders();

    expect(unit->items.GetNum(item_id) == (max_amount * 4 - (max_amount / 2)));
    expect(unit->items.GetNum(I_SILVER) == (max_amount / 2) * price);
    expect(unit2->items.GetNum(item_id) == (max_amount * 4 - (max_amount / 2)));
    expect(unit2->items.GetNum(I_SILVER) == (max_amount / 2) * price);

    expect(faction->errors.size() == 0_ul);
    expect(faction->events.size() == 4_ul);
    std::string amt_string = item_string(item_id, max_amount / 4);
    expect(faction->events[0].message == "Sells " + amt_string + " at $" + std::to_string(price) + " each.");
    expect(faction->events[0].unit == unit);
    expect(faction->events[1].message == "Sells " + amt_string + " at $" + std::to_string(price) + " each.");
    expect(faction->events[1].unit == unit);
    expect(faction->events[2].message == "Sells " + amt_string + " at $" + std::to_string(price) + " each.");
    expect(faction->events[2].unit == unit2);
    expect(faction->events[3].message == "Sells " + amt_string + " at $" + std::to_string(price) + " each.");
    expect(faction->events[3].unit == unit2);

    // Check the unit recurring orders.  There should be max_amount/2 recurring buy orders for each.
    expect(unit->oldorders.size() == static_cast<size_t>(max_amount));
    expect(unit2->oldorders.size() == 1_ul);
    // And make sure the orders are buy orders.
    expect(unit->oldorders.front() == "@sell 1 " + std::string(ItemDefs[item_id].abr));
    expect(unit->oldorders.back() == "@sell 1 " + std::string(ItemDefs[item_id].abr));
    expect(unit2->oldorders.front() == "@sell " + std::to_string(max_amount) + ' ' + std::string(ItemDefs[item_id].abr));
  };
};

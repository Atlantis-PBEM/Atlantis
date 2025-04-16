#include "external/boost/ut.hpp"
#include "external/nlohmann/json.hpp"

using json = nlohmann::json;

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"
#include "items.h"
#include <iostream>
#include "orders.h"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of order parsing in Atlantis
ut::suite<"order_parsing"> order_parsing_suite = []
{
  using namespace ut;

  "WEAPON order accepts multiple items"_test = [] {
    UnitTestHelper helper;

    // Enable the items needed for the test
    helper.enable(UnitTestHelper::Type::ITEM, I_SWORD, true);
    helper.enable(UnitTestHelper::Type::ITEM, I_LONGBOW, true);

    helper.initialize_game();
    helper.setup_turn();

    // Create test faction with a unit
    Faction *faction = helper.create_faction("Test Faction");
    Unit *unit = helper.get_first_unit(faction);

    // Add weapons to the unit and make the faction discover them
    unit->items.SetNum(I_SWORD, 1);
    unit->items.SetNum(I_LONGBOW, 1);
    faction->DiscoverItem(I_SWORD, 1, 1);
    faction->DiscoverItem(I_LONGBOW, 1, 1);

    // Parse the WEAPON order with multiple weapons
    std::stringstream ss;
    ss << "#atlantis " << faction->num << "\n";
    ss << "unit " << unit->num << "\n";
    ss << "WEAPON sword longbow\n";
    helper.parse_orders(faction->num, ss, nullptr);

    // Verify the weapons are set correctly
    expect(unit->readyWeapon[0] == I_SWORD);
    expect(unit->readyWeapon[1] == I_LONGBOW);
  };

  "ARMOR order accepts multiple items"_test = [] {
    UnitTestHelper helper;

    // Enable the items needed for the test
    helper.enable(UnitTestHelper::Type::ITEM, I_CHAINARMOR, true);
    helper.enable(UnitTestHelper::Type::ITEM, I_PLATEARMOR, true);

    helper.initialize_game();
    helper.setup_turn();

    // Create test faction with a unit
    Faction *faction = helper.create_faction("Test Faction");
    Unit *unit = helper.get_first_unit(faction);

    // Add armor to the unit and make the faction discover them
    unit->items.SetNum(I_CHAINARMOR, 1);
    unit->items.SetNum(I_PLATEARMOR, 1);
    faction->DiscoverItem(I_CHAINARMOR, 1, 1);
    faction->DiscoverItem(I_PLATEARMOR, 1, 1);

    // Parse the ARMOR order with multiple armor pieces using different name formats
    std::stringstream ss;
    ss << "#atlantis " << faction->num << "\n";
    ss << "unit " << unit->num << "\n";
    ss << "ARMOR \"chain armor\" plate_armor\n";
    helper.parse_orders(faction->num, ss, nullptr);

    // Verify the armor is set correctly
    expect(unit->readyArmor[0] == I_CHAINARMOR);
    expect(unit->readyArmor[1] == I_PLATEARMOR);
  };

  "DESCRIBE order handles different target types"_test = [] {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    // Create test faction with a unit
    Faction *faction = helper.create_faction("Test Faction");
    Unit *unit = helper.get_first_unit(faction);
    ARegion *region = unit->object->region;

    // Create a building in the same region
    helper.create_building(region, unit, O_TOWER);
    Object *building = unit->object;

    // Parse DESCRIBE orders for both unit and building
    std::stringstream ss;
    ss << "#atlantis " << faction->num << "\n";
    ss << "unit " << unit->num << "\n";
    ss << "DESCRIBE UNIT \"Unit Description\"\n";
    ss << "DESCRIBE BUILDING \"Building Description\"\n";
    helper.parse_orders(faction->num, ss, nullptr);

    // Verify descriptions
    expect(!unit->describe.empty());
    expect(!building->describe.empty());

    std::string unit_desc = unit->describe;
    std::string building_desc = building->describe;

    expect(unit_desc == "Unit Description");
    expect(building_desc == "Building Description");
  };
};

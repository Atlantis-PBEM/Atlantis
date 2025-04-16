#include "external/boost/ut.hpp"
#include "external/nlohmann/json.hpp"

using json = nlohmann::json;

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

namespace ut = boost::ut;

using namespace std;

// This suite will test various aspects of the Build Order functionality.
ut::suite<"Build Order"> build_order_suite = [] {
    using namespace ut;

    "Build order processes valid input correctly"_test = [] {
        // validate that all variants of the build order parse correctly.
        UnitTestHelper helper;
        helper.initialize_game();
        helper.setup_turn();
        string name("Test Faction");
        Faction *faction = helper.create_faction(name);
        Unit *leader = helper.get_first_unit(faction);
        Unit *unit = helper.create_unit(faction, leader->object->region);
        Unit *unit2 = helper.create_unit(faction, leader->object->region);
        Unit *unit3 = helper.create_unit(faction, leader->object->region);
        Unit *unit4 = helper.create_unit(faction, leader->object->region);
        leader->items.SetNum(I_STONE, 5);
        unit->items.SetNum(I_STONE, 5);
        unit2->items.SetNum(I_STONE, 5);
        unit3->items.SetNum(I_STONE, 5);
        unit4->items.SetNum(I_STONE, 5);

        helper.set_skill_level(leader, S_BUILDING, 1);
        helper.set_skill_level(unit, S_BUILDING, 1);
        helper.set_skill_level(unit2, S_BUILDING, 1);
        helper.set_skill_level(unit3, S_BUILDING, 1);
        helper.set_skill_level(unit4, S_BUILDING, 1);

        helper.create_building(leader->object->region, unit4, O_TOWER);
        unit4->object->incomplete = 10; // Set the tower to be incomplete for testin

        // Test a simple build order
        stringstream ss;
        ss << "#atlantis 3 \"mypassword\"\n";
        ss << "unit 2\n";
        ss << "build tower\n"; // this should start a new tower and not carry over to the next month orders
        ss << "unit 3\n";
        ss << "build help 2\n"; // build help with 1 unit and not carry over.
        ss << "unit 4\n";
        ss << "build tower complete\n"; // build a different tower until completion and carry over to next month.
        ss << "unit 5\n";
        ss << "build help 4 complete\n"; // this should carry over.
        ss << "unit 6\n";
        ss << "build tower complete\n"; // this should continue to build the tower they are in and carry over.

        helper.parse_orders(faction->num, ss, nullptr);

        expect(leader->object->region->objects.size() == 3_ul); // dummy + shaft + tower

        helper.run_month_orders();

        expect(leader->oldorders.empty() == "true"_b);
        expect(unit->oldorders.empty() == "true"_b);
        expect(leader->object->incomplete == 8_i);
        expect(unit2->oldorders.front() == "BUILD Tower COMPLETE");
        expect(unit3->oldorders.front() == "BUILD HELP 4 COMPLETE");
        expect(unit2->object->incomplete == 8_i);
        expect(unit4->oldorders.front() == "BUILD Tower COMPLETE");
        expect(unit4->object->incomplete == 9_i);

        expect(leader->object->region->objects.size() == 5_ul); // dummy + shaft + 3 towers.

        // Check the messages too
        expect(faction->errors.size() == 0_ul); // No errors should be reported
        expect(faction->events.size() == 5_ul); // 5 messages for the builds
    };
};

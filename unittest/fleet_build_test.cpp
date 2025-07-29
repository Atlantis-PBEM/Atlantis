#include "external/boost/ut.hpp"
#include "external/nlohmann/json.hpp"

using json = nlohmann::json;

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

namespace ut = boost::ut;

// This suite will test various aspects of the Build Order functionality.
ut::suite<"Fleet Builds"> fleet_build_suite = [] {
    using namespace ut;

    "No Cross Join behavior"_test = [] {
        UnitTestHelper helper;
        helper.initialize_game();
        helper.setup_turn();
        std::string name("Test Faction");
        Faction *faction = helper.create_faction(name);
        Unit *leader = helper.get_first_unit(faction);
        Unit *unit = helper.create_unit(faction, leader->object->region);
        // Make an adjacent region into water for this test.
        leader->object->region->neighbors[2]->type = R_OCEAN;
        leader->items.SetNum(I_LEADERS, 10);
        leader->items.SetNum(I_WOOD, 20);
        unit->items.SetNum(I_LEADERS, 10);
        unit->items.SetNum(I_FLOATER, 20);

        helper.set_skill_level(leader, S_SHIPBUILDING, 5);
        helper.set_skill_level(unit, S_SHIPBUILDING, 5);

        // Put the leader in a flying ship.
        helper.create_fleet(leader->object->region, leader, I_BALLOON, 1);
        leader->object->flying = true;
        // Put the unit in a non-flying ship
        helper.create_fleet(unit->object->region, unit, I_LONGBOAT, 1);
        unit->object->flying = false;

        // Now set up some build orders
        std::stringstream ss;
        ss << "#atlantis 3 \"mypassword\"\n";
        ss << "unit 2\n";
        ss << "build raft\n"; // this should create a new fleet with a raft and move the unit into it.
        ss << "unit 3\n";
        ss << "build balloon\n"; // this should create a new fleet with a balloon and move the unit into it.
        helper.parse_orders(faction->num, ss, nullptr);

        expect(leader->object->region->objects.size() == 4_ul); // dummy + shaft + 2 fleets

        helper.run_month_orders();

        expect(leader->object->region->objects.size() == 6_ul); // dummy + shaft + 4 fleets.

        // Check the messages too
        expect(faction->errors.size() == 0_ul); // No errors should be reported
        expect(faction->events.size() == 2_ul); // 2 messages for the builds
        expect(leader->object->num == 102_i); // leader should be in the new fleet
        expect(leader->object->ships.front()->type == I_RAFT); // leader should be in a raft
        expect(unit->object->num == 103_i); // unit should be in the new fleet
        expect(unit->object->ships.front()->type == I_BALLOON); // unit should be in a balloon
    };

    "Only Flying Cross Join behavior"_test = [] {
        UnitTestHelper helper;
        helper.initialize_game();
        helper.setup_turn();
        Globals->NEW_SHIP_JOINS_FLEET_BEHAVIOR  = GameDefs::NewShipJoinsFleetBehavior::ONLY_FLYING_CROSS_JOIN;
        std::string name("Test Faction");
        Faction *faction = helper.create_faction(name);
        Unit *leader = helper.get_first_unit(faction);
        Unit *unit = helper.create_unit(faction, leader->object->region);
        // Make an adjacent region into water for this test.
        leader->object->region->neighbors[2]->type = R_OCEAN;
        leader->items.SetNum(I_LEADERS, 10);
        leader->items.SetNum(I_WOOD, 20);
        unit->items.SetNum(I_LEADERS, 10);
        unit->items.SetNum(I_FLOATER, 20);

        helper.set_skill_level(leader, S_SHIPBUILDING, 5);
        helper.set_skill_level(unit, S_SHIPBUILDING, 5);

        // Put the leader in a flying ship.
        helper.create_fleet(leader->object->region, leader, I_BALLOON, 1);
        leader->object->flying = true;
        // Put the unit in a non-flying ship
        helper.create_fleet(unit->object->region, unit, I_LONGBOAT, 1);
        unit->object->flying = false;

        // Now set up some build orders
        std::stringstream ss;
        ss << "#atlantis 3 \"mypassword\"\n";
        ss << "unit 2\n";
        ss << "build raft\n"; // this should create a new fleet with a raft and move the unit into it.
        ss << "unit 3\n";
        ss << "build balloon\n"; // this should create join the existing fleet with a balloon.
        helper.parse_orders(faction->num, ss, nullptr);

        expect(leader->object->region->objects.size() == 4_ul); // dummy + shaft + 2 fleets

        helper.run_month_orders();

        expect(leader->object->region->objects.size() == 5_ul); // dummy + shaft + 3 fleets.

        // Check the messages too
        expect(faction->errors.size() == 0_ul); // No errors should be reported
        expect(faction->events.size() == 2_ul); // 2 messages for the builds
        expect(leader->object->num == 102_i); // leader should be in the new fleet
        expect(leader->object->ships.front()->type == I_RAFT); // leader should be in a raft
        expect(unit->object->num == 101_i); // unit should be in the same fleet.
        expect(unit->object->ships.front()->type == I_LONGBOAT); // fleet should still have a longboat.
        expect(unit->object->ships.size() == 2_ul); // fleet should have 2 ships.
        expect(unit->object->ships.GetNum(I_BALLOON) == 1_i); // fleet should have a balloon.
    };

    "All Cross Join behavior"_test = [] {
        UnitTestHelper helper;
        helper.initialize_game();
        helper.setup_turn();
        Globals->NEW_SHIP_JOINS_FLEET_BEHAVIOR  = GameDefs::NewShipJoinsFleetBehavior::ALL_CROSS_JOIN;
        std::string name("Test Faction");
        Faction *faction = helper.create_faction(name);
        Unit *leader = helper.get_first_unit(faction);
        Unit *unit = helper.create_unit(faction, leader->object->region);
        // Make an adjacent region into water for this test.
        leader->object->region->neighbors[2]->type = R_OCEAN;
        leader->items.SetNum(I_LEADERS, 10);
        leader->items.SetNum(I_WOOD, 20);
        unit->items.SetNum(I_LEADERS, 10);
        unit->items.SetNum(I_FLOATER, 20);

        helper.set_skill_level(leader, S_SHIPBUILDING, 5);
        helper.set_skill_level(unit, S_SHIPBUILDING, 5);

        // Put the leader in a flying ship.
        helper.create_fleet(leader->object->region, leader, I_BALLOON, 1);
        leader->object->flying = true;
        // Put the unit in a non-flying ship
        helper.create_fleet(unit->object->region, unit, I_LONGBOAT, 1);
        unit->object->flying = false;

        // Now set up some build orders
        std::stringstream ss;
        ss << "#atlantis 3 \"mypassword\"\n";
        ss << "unit 2\n";
        ss << "build raft\n"; // this should join the raft to the flying fleet and make it non-flying
        ss << "unit 3\n";
        ss << "build balloon\n"; // this should join a balloon to the non-flying fleet.
        helper.parse_orders(faction->num, ss, nullptr);

        expect(leader->object->region->objects.size() == 4_ul); // dummy + shaft + 2 fleets

        helper.run_month_orders();

        expect(leader->object->region->objects.size() == 4_ul); // dummy + shaft + 2 fleets.

        // Check the messages too
        expect(faction->errors.size() == 0_ul); // No errors should be reported
        expect(faction->events.size() == 2_ul); // 2 messages for the builds
        expect(leader->object->num == 100_i); // leader should be in the new fleet
        expect(leader->object->ships.front()->type == I_BALLOON); // leaders fleet should have a balloon.
        expect(leader->object->ships.size() == 2_ul); // leaders fleet should have 2 ships.
        expect(leader->object->ships.GetNum(I_RAFT) == 1_i); // leaders fleet should have a raft.
        expect(leader->object->flying == 0_i); // leaders fleet should be non-flying.

        expect(unit->object->num == 101_i); // unit should be in the same fleet.
        expect(unit->object->ships.front()->type == I_LONGBOAT); // fleet should still have a longboat.
        expect(unit->object->ships.size() == 2_ul); // fleet should have 2 ships.
        expect(unit->object->ships.GetNum(I_BALLOON) == 1_i); // fleet should also have a balloon.
    };
};

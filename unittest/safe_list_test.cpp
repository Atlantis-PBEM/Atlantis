#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"Safe List"> safe_list_suite = []
{
  using namespace ut;

  "safe::list basic functionality"_test = []
  {
    // Create a safe list and add some elements to it
    safe::list<int> sl;
    sl.push_back(1);
    sl.push_back(2);
    sl.push_back(3);

    // Check the size of the list
    expect(sl.size() == 3_ul);

    // Check the elements in the list
    expect(sl.front() == 1_i);
    expect(sl.back() == 3_i);

    // Remove an element and check the size again
    sl.pop_front();
    expect(sl.size() == 2_ul);
    expect(sl.front() == 2_i);
  };

  "safe::list is deletable while iterating"_test = []
  {
    // Create a safe list and add some elements to it
    safe::list<int> sl;
    sl.push_back(1);
    sl.push_back(2);
    sl.push_back(3);
    sl.push_back(4);
    sl.push_back(5);
    sl.push_back(6);

    // Create an iterator for the list
    auto it = sl.begin();

    // Remove an element while iterating
    it = sl.erase(it); // This should remove the first element (1)

    // Check the size and the first element again
    expect(sl.size() == 5_ul);
    expect(sl.front() == 2_i);

    // a normal std::list would invalidate the iterator when we remove elements from it, this list does not.
    for(auto i : sl) {
      if (i % 2 == 0) {
        std::erase(sl, i); // This should remove all even elements (2, 4, 6)
      }
    }

    // Check the size and the first element again
    expect(sl.size() == 2_ul);
    expect(sl.front() == 3_i);
    expect(sl.back() == 5_i);
  };

  "safe::list is correct even when the first few elements are deleted while iterating"_test = []
  {
    // Create a safe list and add some elements to it
    safe::list<int> sl;
    sl.push_back(1);
    sl.push_back(2);
    sl.push_back(3);
    sl.push_back(4);
    sl.push_back(5);
    sl.push_back(6);

    // prove list is safe even if multiple elements including the one after the current iterator are deleted.
    for(auto i : sl) {
      if (i == 2) {
        std::erase(sl, i); // This should remove the second element (2)
        std::erase(sl, i+1); // This should remove the third element (3)
        std::erase(sl, i+3); // This should remove the fourth element (5)
      }
    }

    // Check the size and the first element again
    expect(sl.size() == 3_ul);
    expect(sl.front() == 1_i);
    expect(std::find(sl.begin(), sl.end(), 4) != sl.end()); // 4 should still be there
    expect(sl.back() == 6_i);
  };


  "Practical example of safe::list for Objects with sailing"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    // Since this is such a tightly defined world, we know this is legal
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *second = helper.create_unit(faction, region);
    second->items.SetNum(I_LEADERS, 20);
    helper.set_skill_level(second, S_SAILING, 5);
    Unit *third = helper.create_unit(faction, region);
    third->items.SetNum(I_LEADERS, 20);
    helper.set_skill_level(third, S_SAILING, 5);
    Unit *fourth = helper.create_unit(faction, region);
    fourth->items.SetNum(I_LEADERS, 20);
    Unit *fifth = helper.create_unit(faction, region);
    fifth->items.SetNum(I_LEADERS, 20);
    helper.set_skill_level(fifth, S_SAILING, 5);

    // Create a fleet in the region for the faction.  **THIS MOVES THE UNIT INTO THE FLEET**
    helper.create_fleet(region, second, I_CLOUDSHIP, 1);
    helper.create_fleet(region, third, I_CLOUDSHIP, 1);
    helper.create_fleet(region, fourth, I_CLOUDSHIP, 1); // this fleet won't sail and should get an error
    helper.create_fleet(region, fifth, I_CLOUDSHIP, 1);

    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 3\n";
    ss << "SAIL S\n";
    ss << "unit 4\n";
    ss << "SAIL S\n";
    ss << "unit 5\n";
    ss << "SAIL S\n"; // This unit cannot sail so should get an error.
    ss << "unit 6\n";
    ss << "SAIL S\n";
    helper.parse_orders(faction->num, ss);
    helper.move_units();

    expect(faction->errors.size() == 1_ul);
    expect(faction->events.size() == 3_ul);

    json json_report;
    Game &game = helper.game_object();
    faction->build_json_report(json_report, &game, nullptr);

    // Validate we get the messages we expect for the faction
    json errors = json_report["errors"];
    expect(errors.size() == 1_ul);
    json error = errors[0];
    expect(error["message"] == "SAIL: Not enough sailors.");
    expect(error["unit"]["number"] == 5_i);

    json events = json_report["events"];
    expect(events.size() == 3_ul);
    // Validate we get the messages we expect for the faction
    json event = events[0];
    expect(event["message"] == "Ship [100] sails from plain (0,0) in Testing Wilds to forest (0,2) in Testing Wilds");
    event = events[1];
    expect(event["message"] == "Ship [101] sails from plain (0,0) in Testing Wilds to forest (0,2) in Testing Wilds");
    event = events[2];
    expect(event["message"] == "Ship [103] sails from plain (0,0) in Testing Wilds to forest (0,2) in Testing Wilds");
  };

  "Practical example of safe::list for Objects with destroy"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    // Since this is such a tightly defined world, we know this is legal
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *second = helper.create_unit(faction, region);
    second->items.SetNum(I_LEADERS, 20);
    helper.set_skill_level(second, S_BUILDING, 5);
    Unit *third = helper.create_unit(faction, region);
    third->items.SetNum(I_LEADERS, 20);
    helper.set_skill_level(third, S_BUILDING, 5);
    Unit *fourth = helper.create_unit(faction, region);
    fourth->items.SetNum(I_LEADERS, 20);
    Unit *fifth = helper.create_unit(faction, region);
    fifth->items.SetNum(I_LEADERS, 20);
    helper.set_skill_level(fifth, S_BUILDING, 5);

    helper.create_building(region, nullptr, O_FORT);
    helper.create_building(region, nullptr, O_FORT);
    helper.create_building(region, nullptr, O_FORT);
    helper.create_building(region, nullptr, O_FORT);

    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 3\n";
    ss << "ENTER 2\n";
    ss << "DESTROY\n";
    ss << "unit 4\n";
    ss << "ENTER 3\n";
    ss << "DESTROY\n";
    ss << "unit 5\n";
    ss << "ENTER 4\n";
    ss << "DESTROY\n"; // This unit cannot sail so should get an error.
    ss << "unit 6\n";
    ss << "ENTER 5\n";
    ss << "DESTROY\n";
    helper.parse_orders(faction->num, ss);
    helper.run_enter();
    helper.run_destroy();

    expect(faction->errors.size() == 0_ul);
    expect(faction->events.size() == 4_ul);

    json json_report;
    Game &game = helper.game_object();
    faction->build_json_report(json_report, &game, nullptr);

    // Validate we get the messages we expect for the faction
    json errors = json_report["errors"];
    expect(errors.size() == 0_ul);

    json events = json_report["events"];
    expect(events.size() == 4_ul);
    // Validate we get the messages we expect for the faction
    json event = events[0];
    expect(event["message"] == "Destroys Building [2].");
    expect(event["unit"]["number"] == 3_i);
    event = events[1];
    expect(event["message"] == "Destroys Building [3].");
    expect(event["unit"]["number"] == 4_i);
    event = events[2];
    expect(event["message"] == "Destroys 20 structure points from the Building [4].");
    expect(event["unit"]["number"] == 5_i);
    event = events[3];
    expect(event["message"] == "Destroys Building [5].");
    expect(event["unit"]["number"] == 6_i);
  };
};

#include "external/boost/ut.hpp"
#include "external/nlohmann/json.hpp"

using json = nlohmann::json;

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"
#include "skills.h"
#include "items.h"
#include "orders.h"
#include "spells.h"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"Spells"> spell_suite = []
{
  using namespace ut;

  "Unit with PHBE 3 cannot summon 4 dragons"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name = "Test Faction";
    Faction *faction = helper.create_faction(name);
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *leader = helper.get_first_unit(faction);
    leader->set_name("My Leader");

    helper.set_skill_level(leader, S_CREATE_PHANTASMAL_BEASTS, 3);

    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "cast create_phantasmal_beasts DRAGON 4\n";
    helper.parse_orders(faction->num, ss);
    helper.activate_spell(S_CREATE_PHANTASMAL_BEASTS, {
        .region = region, .unit = leader, .object = nullptr, .val1 = 0, .val2 = 0
    });

    // verify that the unit has no illusionary dragons in it's inventory
    expect(leader->items.GetNum(I_IDRAGON) == 0_i);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get an error message in the report.
    auto count = json_report["errors"].size();
    expect(count == 1_ul);
    json error = json_report["errors"][0];
    expect(error["message"] == "CAST: Can't create that many Phantasmal Beasts.");
    expect(error["unit"]["number"] == 2_i);
  };

  "Unit with PHBE 4 can summon 4 dragons"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name ="Test Faction";
    Faction *faction = helper.create_faction(name);
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *leader = helper.get_first_unit(faction);
    leader->set_name("My Leader");

    helper.set_skill_level(leader, S_CREATE_PHANTASMAL_BEASTS, 4);

    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "cast create_phantasmal_beasts DRAGON 4\n";
    helper.parse_orders(faction->num, ss);
    helper.activate_spell(S_CREATE_PHANTASMAL_BEASTS, {
        .region = region, .unit = leader, .object = nullptr, .val1 = 0, .val2 = 0
    });

    // verify that the unit has 4 illusionary dragons in it's inventory
    expect(leader->items.GetNum(I_IDRAGON) == 4_i);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get no errors in the report
    auto count = json_report["errors"].size();
    expect(count == 0_ul);
  };

  "Unit which cannot swim cannot teleport to ocean"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name = "Test Faction";
    Faction *faction = helper.create_faction(name);
    ARegion *region = helper.get_region(1, 1, 0);
    Unit *leader = helper.get_first_unit(faction);
    leader->set_name("My Leader");

    helper.set_skill_level(leader, S_TELEPORTATION, 1);

    region->type = R_LAKE; // Use something that is similar_type to ocean

    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "cast teleportation REGION 1 1\n";
    helper.parse_orders(faction->num, ss);
    helper.activate_spell(S_TELEPORTATION, {
        .region = region, .unit = leader, .object = nullptr, .val1 = 0, .val2 = 0
    });

    expect(leader->object->region != region); // The unit should not have teleported
    expect(faction->errors.size() == 1_ul); // Expect an error to be generated

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get an error message in the report.
    auto count = json_report["errors"].size();
    expect(count == 1_ul);
    json error = json_report["errors"][0];
    expect(error["message"] == "CAST: lake (1,1) in Testing Wilds is ocean.");
    expect(error["unit"]["number"] == 2_i);
  };

  "Unit which can swim can teleport to ocean"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name = "Test Faction";
    Faction *faction = helper.create_faction(name);
    ARegion *region = helper.get_region(1, 1, 0);
    Unit *leader = helper.get_first_unit(faction);
    leader->set_name("My Leader");

    helper.set_skill_level(leader, S_TELEPORTATION, 1);

    leader->items.SetNum(I_MCARPET, 1); // Give the unit an item that allows swimming

    region->type = R_LAKE; // Use something that is similar_type to ocean

    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "cast teleportation REGION 1 1\n";
    helper.parse_orders(faction->num, ss);
    helper.activate_spell(S_TELEPORTATION, {
        .region = region, .unit = leader, .object = nullptr, .val1 = 0, .val2 = 0
    });

    // verify that the unit did teleport
    expect(leader->object->region == region);
    expect(faction->errors.size() == 0_ul); // No errors should be generated
    expect(faction->events.size() == 1_ul); // Expect a teleportation event to be generated

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get no errors in the report.
    auto count = json_report["errors"].size();
    expect(count == 0_ul);

    // Check the events to ensure the teleportation was successful
    auto events = json_report["events"];
    expect(events.size() == 1_ul); // Expect a single event for the teleportation
    json event = events[0];
    expect(event["message"] == "Teleports to lake (1,1) in Testing Wilds."); // Check the message for correctness
    expect(event["unit"]["number"] == 2_i); // Check the unit number in the even
    expect(event["category"] == "spell"); // Check the category of the event
  };

  "CAST TRANSMUTATION parses and executes correctly"_test = [] {
    UnitTestHelper helper;

    // Enable the TRANSMUTATION spell and required items
    helper.enable(UnitTestHelper::Type::ITEM, I_IRON, true);
    helper.enable(UnitTestHelper::Type::ITEM, I_MITHRIL, true);
    helper.enable(UnitTestHelper::Type::SKILL, S_PATTERN, true);
    helper.enable(UnitTestHelper::Type::SKILL, S_TRANSMUTATION, true);

    helper.initialize_game();
    helper.setup_turn();

    // Create test faction with a mage
    Faction *faction = helper.create_faction("Test Faction");
    Unit *unit = helper.get_first_unit(faction);
    ARegion *region = unit->object->region;

    // Set up unit to be able to cast TRANSMUTATION
    helper.set_skill_level(unit, S_PATTERN, 3);
    helper.set_skill_level(unit, S_TRANSMUTATION, 3);
    unit->items.SetNum(I_IRON, 1);
    faction->DiscoverItem(I_IRON, 1, 1);
    faction->DiscoverItem(I_MITHRIL, 1, 1);

    // Parse the CAST order with TRANSMUTATION
    std::stringstream ss;
    ss << "#atlantis " << faction->num << "\n";
    ss << "unit " << unit->num << "\n";
    ss << "CAST TRANSMUTATION MITHRIL\n";
    helper.parse_orders(faction->num, ss, nullptr);

    // Execute the spell directly using the activate_spell method
    helper.activate_spell(S_TRANSMUTATION, {.region = region, .unit = unit, .object = nullptr, .val1 = 0, .val2 = 0});

    // Verify that the spell was parsed and executed correctly
    expect(unit->items.GetNum(I_IRON) == 0);
    expect(unit->items.GetNum(I_MITHRIL) == 1);
  };

  "CAST TRANSMUTATION without target item generates error"_test = [] {
    UnitTestHelper helper;

    // Enable the TRANSMUTATION spell and required items
    helper.enable(UnitTestHelper::Type::ITEM, I_IRON, true);
    helper.enable(UnitTestHelper::Type::ITEM, I_MITHRIL, true);
    helper.enable(UnitTestHelper::Type::SKILL, S_PATTERN, true);
    helper.enable(UnitTestHelper::Type::SKILL, S_TRANSMUTATION, true);

    helper.initialize_game();
    helper.setup_turn();

    // Create test faction with a mage
    Faction *faction = helper.create_faction("Test Faction");
    Unit *unit = helper.get_first_unit(faction);

    // Set up unit to be able to cast TRANSMUTATION
    helper.set_skill_level(unit, S_PATTERN, 3);
    helper.set_skill_level(unit, S_TRANSMUTATION, 3);
    unit->items.SetNum(I_IRON, 1);
    faction->DiscoverItem(I_IRON, 1, 1);
    faction->DiscoverItem(I_MITHRIL, 1, 1);

    // Parse the CAST order with TRANSMUTATION but no target item
    std::stringstream ss;
    ss << "#atlantis " << faction->num << "\n";
    ss << "unit " << unit->num << "\n";
    ss << "CAST TRANSMUTATION\n";
    helper.parse_orders(faction->num, ss, nullptr);

    // Verify that the specific error message was generated
    expect(faction->errors.size() == 1);
    expect(faction->errors[0].message == "CAST 'transmutation': You must specify what you wish to create.");

    // Verify no items were changed
    expect(unit->items.GetNum(I_IRON) == 1);
    expect(unit->items.GetNum(I_MITHRIL) == 0);
  };

};

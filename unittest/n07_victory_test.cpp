#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"NO7 Victory Conditions"> no7victory_suite = []
{
  using namespace ut;

  "Unit without entity cannot enter hex containing ritual"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::ITEM, I_IMPRISONED_ENTITY, true);
    helper.enable(UnitTestHelper::Type::OBJECT, O_RITUAL_ALTAR, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    ARegion *region = helper.get_region(1, 1, 0);
    Unit *leader = helper.get_first_unit(faction);
    AString *tmp_name = new AString("My Leader");
    leader->SetName(tmp_name);

    helper.create_building(region, nullptr, O_RITUAL_ALTAR);

    // Try to move the unit into the region with the ritual altar.
    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "move SE\n";
    helper.parse_orders(faction->num, ss);
    helper.move_units();

    expect(faction->errors.size() == 1_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get an error message in the report.
    auto count = json_report["errors"].size();
    expect(count == 1_ul);
    json error = json_report["errors"][0];
    expect(error["message"] == "MOVE: A mystical barrier prevents movement in that direction.");
    expect(error["unit"]["number"] == 2_i);
  };

  "Unit without entity cannot teleport into hex containing ritual"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::ITEM, I_IMPRISONED_ENTITY, true);
    helper.enable(UnitTestHelper::Type::OBJECT, O_RITUAL_ALTAR, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    ARegion *region = helper.get_region(1, 1, 0);
    Unit *leader = helper.get_first_unit(faction);
    leader->Study(S_PATTERN, 90);
    leader->Study(S_SPIRIT, 90);
    leader->Study(S_FARSIGHT, 90);
    leader->Study(S_GATE_LORE, 30);
    leader->Study(S_TELEPORTATION, 30);
    AString *tmp_name = new AString("My Leader");
    leader->SetName(tmp_name);

    helper.create_building(region, nullptr, O_RITUAL_ALTAR);

    // Try to teleport the unit into the region with the ritual altar.
    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "cast Teleportation REGION 1 1\n";
    helper.parse_orders(faction->num, ss);
    helper.activate_spell(S_TELEPORTATION, {
        .region = region, .unit = leader, .object = nullptr, .val1 = 0, .val2 = 0
    });

    expect(faction->errors.size() == 1_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get an error message in the report.
    auto count = json_report["errors"].size();
    expect(count == 1_ul);
    json error = json_report["errors"][0];
    expect(error["message"] == "CAST: A mystical barrier prevents teleporting to that location.");
    expect(error["unit"]["number"] == 2_i);
  };

  "Unit with entity can enter hex containing ritual"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::ITEM, I_IMPRISONED_ENTITY, true);
    helper.enable(UnitTestHelper::Type::OBJECT, O_RITUAL_ALTAR, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    // Since this is such a tightly defined world, we know this is legal
    ARegion *region = helper.get_region(1, 1, 0);
    Unit *leader = helper.get_first_unit(faction);
    AString *tmp_name = new AString("My Leader");
    leader->SetName(tmp_name);
    leader->items.SetNum(I_IMPRISONED_ENTITY, 1);

    helper.create_building(region, nullptr, O_RITUAL_ALTAR);

    // Try to move the unit into the region with the ritual altar.
    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "move SE\n";
    helper.parse_orders(faction->num, ss);
    helper.move_units();

    expect(faction->errors.size() == 0_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get no error message in the report.
    auto count = json_report["errors"].size();
    expect(count == 0_ul);
    json events = json_report["events"];
    expect(events.size() == 1_ul);
    json event = events[0];
    expect(event["message"] == "Walks from plain (0,0) in Testing Wilds to mountain (1,1) in Testing Wilds.");
    expect(event["unit"]["number"] == 2_i);
  };                                                                                                                                                                                                                                                                 
};

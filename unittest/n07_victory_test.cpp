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

  "Unit can sacrifice to anomaly to get an entity"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::ITEM, I_IMPRISONED_ENTITY, true);
    helper.enable(UnitTestHelper::Type::OBJECT, O_ENTITY_CAGE, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    Unit *leader = helper.get_first_unit(faction);
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *sac1 = helper.create_unit(faction, region);
    AString *tmp_name= new AString("My Leader");
    leader->SetName(tmp_name);
    tmp_name = new AString("My Sacrifice 1");
    sac1->SetName(tmp_name);
    sac1->items.SetNum(I_LEADERS, 10);

    helper.create_building(region, nullptr, O_ENTITY_CAGE);

    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 3\n";
    ss << "sacrifice 10 lead\n";
    helper.parse_orders(faction->num, ss);
    helper.run_sacrifice();

    helper.setup_reports();
    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // verify that we got two events and that they are the expected events.
    auto count = json_report["events"].size();
    expect(count == 2_ul);
    json event = json_report["events"][0];
    expect(event["message"] == "Sacrifices 10 leaders [LEAD].");
    expect(event["unit"]["number"] == 3_i);
    event = json_report["events"][1];
    expect(event["message"] == "Gains imprisoned entity [IENT] from sacrifice.");
    expect(event["unit"]["number"] == 2_i);

    // verify that there is only object in the region now.
    auto objects = json_report["regions"][0]["structures"];
    expect(objects.size() == 1_ul);
    expect(objects[0]["type"] == "Shaft");
  };

  "Unit can fulfill part of a sacrifice to anomaly and it will remain"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::ITEM, I_IMPRISONED_ENTITY, true);
    helper.enable(UnitTestHelper::Type::OBJECT, O_ENTITY_CAGE, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    Unit *leader = helper.get_first_unit(faction);
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *sac1 = helper.create_unit(faction, region);
    AString *tmp_name= new AString("My Leader");
    leader->SetName(tmp_name);
    tmp_name = new AString("My Sacrifice 1");
    sac1->SetName(tmp_name);
    sac1->items.SetNum(I_LEADERS, 10);

    helper.create_building(region, nullptr, O_ENTITY_CAGE);

    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 3\n";
    ss << "sacrifice 5 lead\n";
    helper.parse_orders(faction->num, ss);
    helper.run_sacrifice();

    helper.setup_reports();
    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // verify that we got two events and that they are the expected events.
    auto count = json_report["events"].size();
    expect(count == 1_ul);
    json event = json_report["events"][0];
    expect(event["message"] == "Sacrifices 5 leaders [LEAD].");
    expect(event["unit"]["number"] == 3_i);

    // verify that there is only object in the region now.
    auto objects = json_report["regions"][0]["structures"];
    expect(objects.size() == 2_ul);
    expect(objects[0]["type"] == "Shaft");
    expect(objects[1]["type"] == "Mystical Anomaly");
  };

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

"Unit with entity gets 'standard' entity maintainence with no movement"_test = []
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

    helper.maintain_units();

    expect(faction->errors.size() == 0_ul);
    expect(faction->events.size() == 1_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get a maintenance event in the report.
    json events = json_report["events"];
    expect(events.size() == 1_ul);
    json event = events[0];
    expect(event["message"] == "Claims 1020 silver for maintenance."); // 20 for leader, 1000 for entity
    expect(event["unit"]["number"] == 2_i);
  };

  "Unit with entity gets 'less' entity maintainence with moving toward altar"_test = []
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

    helper.maintain_units();

    expect(faction->errors.size() == 0_ul);
    expect(faction->events.size() == 2_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get a maintenance event in the report.
    json events = json_report["events"];
    expect(events.size() == 2_ul);
    json event = events[0];
    expect(event["message"] == "Walks from plain (0,0) in Testing Wilds to mountain (1,1) in Testing Wilds.");
    expect(event["unit"]["number"] == 2_i);

    event = events[1];
    expect(event["message"] == "Claims 520 silver for maintenance."); // 20 for leader, 500 for entity
    expect(event["unit"]["number"] == 2_i);
  };

  "Unit with entity gets 'more' entity maintainence with moving away from altar"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::ITEM, I_IMPRISONED_ENTITY, true);
    helper.enable(UnitTestHelper::Type::OBJECT, O_RITUAL_ALTAR, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    // Since this is such a tightly defined world, we know this is legal
    ARegion *region = helper.get_region(0, 0, 0);
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

    helper.maintain_units();

    expect(faction->errors.size() == 0_ul);
    expect(faction->events.size() == 2_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get a maintenance event in the report.
    json events = json_report["events"];
    expect(events.size() == 2_ul);
    json event = events[0];
    expect(event["message"] == "Walks from plain (0,0) in Testing Wilds to mountain (1,1) in Testing Wilds.");
    expect(event["unit"]["number"] == 2_i);

    event = events[1];
    expect(event["message"] == "Claims 5020 silver for maintenance."); // 20 for leader, 5000 for entity
    expect(event["unit"]["number"] == 2_i);
  };

  "Annhilation requires access to the annihilation skill"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::OBJECT, O_ACTIVE_MONOLITH, true);
    helper.enable(UnitTestHelper::Type::SKILL, S_ANNIHILATION, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    // Since this is such a tightly defined world, we know this is legal
    //ARegion *region = helper.get_region(0, 0, 0);
    Unit *leader = helper.get_first_unit(faction);
    AString *tmp_name = new AString("My Leader");
    leader->SetName(tmp_name);

    // Try to use annihilate
    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "annihilate region 1, 1, 0\n";
    helper.parse_orders(faction->num, ss);
    helper.run_annihilation();

    expect(faction->errors.size() == 1_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get a maintenance event in the report.
    json errors = json_report["errors"];
    expect(errors.size() == 1_ul);
    json error = errors[0];
    expect(error["message"] == "ANNIHILATE: Unit does not have access to the annihilate skill.");
    expect(error["unit"]["number"] == 2_i);
  };

  "Annihilation cannot destroy an already annihilated hex"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::OBJECT, O_ACTIVE_MONOLITH, true);
    helper.enable(UnitTestHelper::Type::SKILL, S_ANNIHILATION, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    // Since this is such a tightly defined world, we know this is legal
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *leader = helper.get_first_unit(faction);
    AString *tmp_name = new AString("My Leader");
    leader->SetName(tmp_name);
    region->type = R_BARREN;
    helper.create_building(region, leader, O_ACTIVE_MONOLITH);

    // Try to use annihilate
    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "annihilate region 0, 0, 0\n";
    helper.parse_orders(faction->num, ss);
    helper.run_annihilation();

    expect(faction->errors.size() == 1_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Expect that we get a maintenance event in the report.
    json errors = json_report["errors"];
    expect(errors.size() == 1_ul);
    json error = errors[0];
    expect(error["message"] == "ANNIHILATE: Target region is already annihilated.");
    expect(error["unit"]["number"] == 2_i);
  };

  "Annihilation will destroy everythign in a hex but leave shafts and anomalies"_test = []
  {
    UnitTestHelper helper;
    helper.enable(UnitTestHelper::Type::OBJECT, O_ACTIVE_MONOLITH, true);
    helper.enable(UnitTestHelper::Type::ITEM, I_IMPRISONED_ENTITY, true);
    helper.enable(UnitTestHelper::Type::OBJECT, O_ENTITY_CAGE, true);
    helper.enable(UnitTestHelper::Type::SKILL, S_ANNIHILATION, true);
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    // Since this is such a tightly defined world, we know this is legal
    ARegion *region = helper.get_region(0, 0, 0);
    ARegion *region2 = helper.get_region(1, 1, 0);
    Unit *leader = helper.get_first_unit(faction);
    AString *tmp_name = new AString("My Leader");
    leader->SetName(tmp_name);
    region2->type = R_BARREN;
    helper.create_building(region2, nullptr, O_ACTIVE_MONOLITH);
    helper.create_building(region, nullptr, O_ENTITY_CAGE);
    Unit *second = helper.create_unit(faction, region);
    second->items.SetNum(I_LEADERS, 10);
    helper.create_building(region, second, O_TOWER);

    // Try to use annihilate
    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "move se 1\n";
    ss << "annihilate region 0, 0, 0\n";
    helper.parse_orders(faction->num, ss);
    helper.move_units();
    helper.run_annihilation();

    expect(faction->errors.size() == 0_ul);
    expect(faction->events.size() == 5_ul);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // Validate we get the messages we expect for the faction
    json events = json_report["events"];
    expect(events.size() == 5_ul);
    json event = events[0];
    expect(event["message"] == "Walks from plain (0,0) in Testing Wilds to barren (1,1) in Testing Wilds.");
    expect(event["unit"]["number"] == 2_i);
    event = events[1];
    expect(event["message"] == "Enters Building [1].");
    expect(event["unit"]["number"] == 2_i);
    event = events[2];
    expect(event["message"] == "forest (0,2) in Testing Wilds has been utterly annihilated.");
    expect(event["category"] == "annihilate");
    event = events[3];
    expect(event["message"] == "plain (0,0) in Testing Wilds, contains Basictown [city] has been utterly annihilated.");
    expect(event["category"] == "annihilate");
    event = events[4];
    expect(event["message"] == "Is annihilated.");
    expect(event["category"] == "annihilate");
    expect(event["unit"]["number"] == 3_i);

    // load the gm faction report
    json gm_report;
    Faction *gm_faction = helper.get_faction(1);
    gm_faction->build_json_report(gm_report, &game, nullptr);

    // Verify that everything in region 0,0,0 is gone except the shaft and the anomaly.
    json region_rep = gm_report["regions"][0];
    expect(region_rep["terrain"] == "barren");
    expect(region_rep["structures"].size() == 2_ul);
    expect(region_rep["structures"][0]["type"] == "Shaft");
    expect(region_rep["structures"][1]["type"] == "Mystical Anomaly");
    // verify that markets, products and all units are gone.
    expect(region_rep["markets"]["for_sale"].size() == 0_ul);
    expect(region_rep["markets"]["wanted"].size() == 0_ul);
    expect(region_rep["products"].size() == 0_ul);
    expect(region_rep["units"].size() == 0_ul);
    expect(region_rep["structures"][0]["units"].size() == 0_ul);
    expect(region_rep["structures"][1]["units"].size() == 0_ul);
    expect(region_rep["wages"]["amount"] == 0_i);
    expect(region_rep["wages"].value("max", 0) == 0_i);
    expect(region_rep["population"].empty());
    expect(region_rep.value("tax", 0) == 0_i);
  };

};

#include "../external/boost/ut.hpp"
#include "../external/nlohmann/json.hpp"

using json = nlohmann::json;

#include "../game.h"
#include "../gamedata.h"
#include "testhelper.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"JSON Report"> json_report_suite = []
{
  using namespace ut;

  "Faction json contains basic faction information"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    std::string name("Test Faction");
    Faction *faction = helper.create_faction(name);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->build_json_report(json_report, &game, nullptr);

    // pick some of the data out of the report for checking
    std::string data_name = json_report["name"];
    auto data_num = json_report["number"];
    auto new_items = json_report["item_reports"].size();
    auto regions = json_report["regions"].size();
    auto skill_reports = json_report["skill_reports"].size();

    expect(data_name == name); // faction name should match our name.
    expect(data_num == 3_i); // faction num should be 3 since we have guards and wandering monsters.
    expect(new_items == 1_ul); // should just be the new leader
    expect(skill_reports == 3_ul); // should have exactly 3 skill reports

    // Verify that the game date is correct.
    std::string expected_month("January");
    std::string data_month = json_report["date"]["month"];
    expect(data_month == expected_month);

    // Verify the basic region info is correct.
    expect(regions == 1_ul); // and we should only have 1 region we know about (our start region)
  };

  "Errors are reported"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    Faction *faction = helper.create_faction("Test Faction");
    faction->error("This is an error");

    json json_report;
    faction->build_json_report(json_report, &helper.game_object(), nullptr);

    auto count = json_report["errors"].size();
    expect(count == 1_ul);
    std::string error = json_report["errors"][0]["message"];
    std::string expected = "This is an error";
    expect(error == expected);
  };

  "More than 1000 errors will log a too many errors and ignore the rest"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    Faction *faction = helper.create_faction("Test Faction");
    for (auto i = 0; i < 1003; i++) faction->error("This is error #" + std::to_string(i+1));

    json json_report;
    faction->build_json_report(json_report, &helper.game_object(), nullptr);

    auto count = json_report["errors"].size();
    expect(count == 1001_ul);
    std::string error = json_report["errors"][1000]["message"];
    std::string expected = "Too many errors!";
    expect(error == expected);

    error = json_report["errors"][999]["message"];
    expected = "This is error #1000";
    expect(error == expected);
  };

  "Events are reported"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    Faction *faction = helper.create_faction("Test Faction");
    faction->event("This is event 1", "type1");
    faction->event("This is event 2", "type2");

    json json_report;
    faction->build_json_report(json_report, &helper.game_object(), nullptr);

    auto count = json_report["events"].size();
    expect(count == 2_ul);

    // make sure they are in the right order
    std::string event = json_report["events"][0]["message"];
    std::string type = json_report["events"][0]["category"];
    std::string expected = "This is event 1";
    std::string expected_type = "type1";
    expect(event == expected);
    expect(type == expected_type);

    event = json_report["events"][1]["message"];
    type = json_report["events"][1]["category"];
    expected = "This is event 2";
    expected_type = "type2";
    expect(event == expected);
    expect(type == expected_type);
  };


  "Region json contains basic region information"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    helper.setup_reports();

    // Generate just this single regions json object.
    Faction *faction = helper.create_faction("Test Faction");
    ARegion *region = helper.get_region(0, 0, 0);
    ARegionList& regions = helper.get_regions();

    json json_report;
    region->build_json_report(json_report, faction, helper.get_month(), regions);

    std::string expected_provice("Testing Wilds"); // name given in the unit test setup
    std::string province = json_report["province"];
    expect(province == expected_provice);

    std::string expected_terrain("plain"); // should be the terrain set up in the unit test
    std::string terrain = json_report["terrain"];
    expect(terrain == expected_terrain);

    auto x = json_report["coordinates"]["x"];
    auto y = json_report["coordinates"]["y"];
    auto z = json_report["coordinates"]["z"];
    expect(x == 0_i);
    expect(y == 0_i);
    expect(z == 0_i);

    std::string expected_label("surface");
    std::string label = json_report["coordinates"]["label"];
    expect(label == expected_label);

    std::string settlement_name = json_report["settlement"]["name"];
    std::string expected_settlement_name("Basictown"); // name given in the unit test setup
    expect(settlement_name == expected_settlement_name);

    std::string settlement_size = json_report["settlement"]["size"];
    std::string expected_settlement_size("city");
    expect(settlement_size == expected_settlement_size);

    auto wages = json_report["wages"];
    auto expected_wages = json{ {"amount", 15.5}, {"max", 1795} };
    expect(wages == expected_wages);
    auto entertainment = json_report["entertainment"];
    expect(entertainment == 121_i);

    // verify the products and the markets
    auto products = json_report["products"].size();
    expect(products == 1_ul);
    auto expected_product = json{ {"tag", "HORS"}, {"name", "horse"}, {"plural", "horses"}, {"amount", 38 } };
    auto first_product = json_report["products"][0];
    expect(first_product == expected_product);

    auto for_sale = json_report["markets"]["for_sale"].size();
    expect(for_sale == 4_ul);

    auto expected_sale = json{
      {"tag", "PEAR"}, {"name", "pearls"}, {"plural", "pearls"}, {"amount", 8 }, { "price", 109 }
    };
    auto first_sale = json_report["markets"]["for_sale"][0];
    expect(first_sale == expected_sale);

    auto wanted = json_report["markets"]["wanted"].size();
    expect(wanted == 9_ul);
    auto expected_wanted = json{
      {"tag", "GRAI"}, {"name", "grain"}, {"plural", "grain"}, {"amount", 72 }, { "price", 20 }
    };
    auto first_wanted = json_report["markets"]["wanted"][0];
    expect(first_wanted == expected_wanted);

    auto exits = json_report["exits"].size();
    expect(exits == 3_ul);  // for our 4 hex world, from this hex, it'll be se, s, and sw.
    auto expected_exit_s = json{
      {"direction", "South" },
      { "region",
        {
          { "coordinates", { {"x", 0}, {"y", 2}, {"z", 0}, {"label", "surface"} } },
          { "province", "Testing Wilds" },
          { "terrain", "forest" }
        }
      }
    };
    auto exit_s = json_report["exits"][1];
    expect(exit_s == expected_exit_s);
    // The other 2 exits point to the same region due to wrapping.
    auto exit_se_region = json_report["exits"][0]["region"];
    auto exit_sw_region = json_report["exits"][2]["region"];
    expect(exit_se_region == exit_sw_region);
  };

  "Region contains object data which includes correct details"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    helper.setup_reports();

    // Generate just this single regions json object.
    Faction *faction = helper.create_faction("Test Faction");
    Faction *faction2 = helper.create_faction("Test Faction 2");
    ARegion *region = helper.get_region(0, 0, 0);
    ARegionList& regions = helper.get_regions();
    Unit *leader = helper.get_first_unit(faction);
    // Create a fleet in the region for the faction.  **THIS MOVES THE UNIT INTO THE FLEET**
    helper.create_fleet(region, leader, I_GALLEON, 3); // 3 galleons
    helper.create_fleet(region, leader, I_LONGSHIP, 2); // 2 longships

    // Get a report for each region so we can verify that fleet data is correct for owners and non-owners.
    json json_report_1;
    json json_report_2;
    region->build_json_report(json_report_1, faction, helper.get_month(), regions);
    region->build_json_report(json_report_2, faction2, helper.get_month(), regions);

    // Verify that owner sees additional data
    auto capacity = json_report_1["structures"][1]["capacity"];
    expect(capacity == 8400_ul);

    // Verify that non-owner does not see additional data
    auto capacity2 = json_report_2["structures"][1]["capacity"];
    expect(capacity2 == nullptr);

    // Verify they both see the same ships
    auto ships = json_report_1["structures"][1]["ships"];
    auto ships2 = json_report_2["structures"][1]["ships"];
    expect(ships == ships2);
  };

  "Unit json contains the right data"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    helper.setup_reports();

    // Generate just this single regions json object.
    Faction *faction = helper.create_faction("Test Faction");
    Unit *leader = helper.get_first_unit(faction);
    leader->set_name("My Leader");
    std::stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "@work\n";
    ss << "turn\n";
    ss << "  form 1\n";
    ss << "    buy 1 lead\n";
    ss << "  end\n";
    ss << "endturn\n";
    helper.parse_orders(faction->num, ss);

    // Get a report for each region so we can verify that fleet data is correct for owners and non-owners.
    json json_unit_report;
    leader->build_json_report(json_unit_report, -1, 1, 1, 1, AttitudeType::ALLY, 1);

    std::string name = json_unit_report["name"];
    std::string expected_name = "My Leader";
    expect(name == expected_name);

    auto id = json_unit_report["number"];
    expect(id == 2_i);

    std::string faction_name = json_unit_report["faction"]["name"];
    std::string expected_faction_name = "Test Faction";
    expect(faction_name == expected_faction_name);

    auto faction_id = json_unit_report["faction"]["number"];
    expect(faction_id == 3_i);

    auto guard = json_unit_report["flags"]["guard"];
    expect(guard == _b(false));

    auto items_count = json_unit_report["items"].size();
    expect(items_count == 1_ul);

    auto item = json_unit_report["items"][0];
    auto expected_item = json{ {"tag", "LEAD"}, {"name", "leader"}, {"plural", "leaders"}, {"amount", 1 } };
    expect(item == expected_item);

    auto orders_count = json_unit_report["orders"].size();
    // top level orders are the work order, the turn, and the endturn
    expect(orders_count == 3_ul);

    // verify the work order
    auto work_order = json_unit_report["orders"][0];
    auto expected_work_order = json{ {"order", "@work"} };
    expect(work_order == expected_work_order);

    // verify the turn order
    std::string turn_order = json_unit_report["orders"][1]["order"];
    std::string expected_turn_order = "TURN";
    expect(turn_order == expected_turn_order);
    auto turn_orders_count = json_unit_report["orders"][1]["nested"].size();
    expect(turn_orders_count == 2_ul);

    auto turn_orders = json_unit_report["orders"][1]["nested"];
    auto form_order = turn_orders[0]["order"];
    auto expected_form_order = "form 1";
    expect(form_order == expected_form_order);
    auto form_orders_count = turn_orders[0]["nested"].size();
    expect(form_orders_count == 1_ul);

    auto form_orders = turn_orders[0]["nested"];
    auto buy_order = json{ {"order", "buy 1 lead"} };
    expect(form_orders[0] == buy_order);

    // verify the endform
    auto endform_order = json{ {"order", "end"} };
    expect(turn_orders[1] == endform_order);

    // verify the endturn
    auto endturn_order = json{ {"order", "ENDTURN"} };
    expect(json_unit_report["orders"][2] == endturn_order);
  };
};

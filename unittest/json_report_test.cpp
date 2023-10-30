#include "external/boost/ut.hpp"
#include "external/nlohmann/json.hpp"

using json = nlohmann::json;

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

using namespace std;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"JSON Report"> json_report_suite = []
{
  using namespace ut;

  "Faction json contains basic faction information"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);

    helper.setup_reports();

    // Generate just this single factions json object.
    Game &game = helper.game_object();
    json json_report;
    faction->write_json_report(json_report, &game, nullptr);

    // pick some of the data out of the report for checking
    string data_name = json_report["name"];
    auto data_num = json_report["number"];
    auto new_items = json_report["item_reports"].size();
    auto new_skills = json_report["skill_reports"].size();
    auto regions = json_report["regions"].size();

    expect(data_name == name); // faction name should match our name.
    expect(data_num == 3_i); // faction num should be 3 since we have guards and wandering monsters.
    expect(new_items == 1_ul); // should just be the new leader
    expect(new_skills == 3_ul); // should be combat 1, 2, and 3 based on the test game setup.

    // Verify that the game date is correct.
    string expected_month("January");
    string data_month = json_report["date"]["month"];
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
    faction->write_json_report(json_report, &helper.game_object(), nullptr);

    auto count = json_report["errors"].size();
    expect(count == 1_ul);
    string error = json_report["errors"][0];
    string expected = "This is an error";
    expect(error == expected);
  };

  "More than 1000 errors will log a too many errors and ignore the rest"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    Faction *faction = helper.create_faction("Test Faction");
    for (auto i = 0; i < 1003; i++) faction->error("This is error #" + to_string(i+1));

    json json_report;
    faction->write_json_report(json_report, &helper.game_object(), nullptr);

    auto count = json_report["errors"].size();
    expect(count == 1001_ul);
    string error = json_report["errors"][1000];
    string expected = "Too many errors!";
    expect(error == expected);

    error = json_report["errors"][999];
    expected = "This is error #1000";
    expect(error == expected);
  };

  "Events are reported"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    Faction *faction = helper.create_faction("Test Faction");
    faction->event("This is event 1");
    faction->event("This is event 2");

    json json_report;
    faction->write_json_report(json_report, &helper.game_object(), nullptr);

    auto count = json_report["events"].size();
    expect(count == 2_ul);

    // make sure they are in the right order
    string event = json_report["events"][0];
    string expected = "This is event 1";
    expect(event == expected);

    event = json_report["events"][1];
    expected = "This is event 2";
    expect(event == expected);
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
    ARegionList *regions = helper.get_regions();

    json json_report;
    region->write_json_report(json_report, faction, helper.get_month(), regions);

    string expected_provice("Testing Wilds"); // name given in the unit test setup
    string province = json_report["province"];
    expect(province == expected_provice);

    string expected_terrain("plain"); // should be the terrain set up in the unit test
    string terrain = json_report["terrain"];
    expect(terrain == expected_terrain);

    auto x = json_report["coordinates"]["x"];
    auto y = json_report["coordinates"]["y"];
    auto z = json_report["coordinates"]["z"];
    expect(x == 0_i);
    expect(y == 0_i);
    expect(z == 0_i);

    string expected_label("surface");
    string label = json_report["coordinates"]["label"];
    expect(label == expected_label);

    string settlement_name = json_report["settlement"]["name"];
    string expected_settlement_name("Basictown"); // name given in the unit test setup
    expect(settlement_name == expected_settlement_name);

    string settlement_size = json_report["settlement"]["size"];
    string expected_settlement_size("city");
    expect(settlement_size == expected_settlement_size);

    auto wages = json_report["wages"];
    auto expected_wages = json{ {"amount", 15.2}, {"max", 1935} };
    expect(wages == expected_wages);
    auto entertainment = json_report["entertainment"];
    expect(entertainment == 141_i);

    // verify the products and the markets
    auto products = json_report["products"].size();
    expect(products == 1_ul);
    auto expected_product = json{ {"abbr", "HORS"}, {"name", "horse"}, {"plural", "horses"}, {"amount", 21 } };
    auto first_product = json_report["products"][0];
    expect(first_product == expected_product);

    auto for_sale = json_report["markets"]["for_sale"].size();
    expect(for_sale == 4_ul);
  
    auto expected_sale = json{
      {"abbr", "IVOR"}, {"name", "ivory"}, {"plural", "ivory"}, {"amount", 13 }, { "price", 81 }
    };
    auto first_sale = json_report["markets"]["for_sale"][0];
    expect(first_sale == expected_sale);

    auto wanted = json_report["markets"]["wanted"].size();
    expect(wanted == 9_ul);
    auto expected_wanted = json{
      {"abbr", "GRAI"}, {"name", "grain"}, {"plural", "grain"}, {"amount", 51 }, { "price", 16 }
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
    ARegionList *regions = helper.get_regions();
    Unit *leader = helper.get_first_unit(faction);
    // Create a fleet in the region for the faction.  **THIS MOVES THE UNIT INTO THE FLEET**
    helper.create_fleet(region, leader, I_GALLEON, 3); // 3 galleons
    helper.create_fleet(region, leader, I_LONGSHIP, 2); // 2 longships

    // Get a report for each region so we can verify that fleet data is correct for owners and non-owners.
    json json_report_1;
    json json_report_2;
    region->write_json_report(json_report_1, faction, helper.get_month(), regions);
    region->write_json_report(json_report_2, faction2, helper.get_month(), regions);

    // Verify that owner sees additional data
    auto capacity = json_report_1["structures"][0]["capacity"];
    expect(capacity == 8400_ul);

    // Verify that non-owner does not see additional data
    auto capacity2 = json_report_2["structures"][0]["capacity"];
    expect(capacity2 == nullptr);

    // Verify they both see the same ships
    auto ships = json_report_1["structures"][0]["ships"];
    auto ships2 = json_report_2["structures"][0]["ships"];
    expect(ships == ships2);
  };
};

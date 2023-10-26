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

  "Report contains basic faction information for a new faction"_test = []
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
    int data_num = json_report["number"];
    string data_month = json_report["date"]["month"];
    int new_items = json_report["item_reports"].size();
    int new_skills = json_report["skill_reports"].size();
    int regions = json_report["regions"].size();

    expect(data_name == name);
    expect(data_num == 3_i);
    expect(new_items == 1_i); // should just be the new leader
    expect(new_skills == 3_i); // should be combat 1, 2, and 3
    expect(regions == 1_i); // and we should only have 1 region we know about (our start region)

    string expected_month("January");
    expect(data_month == expected_month);
  };
};

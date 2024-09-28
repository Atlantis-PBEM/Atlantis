#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

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
    AString *tmp_name = new AString("My Leader");
    leader->SetName(tmp_name);
    leader->Study(S_FORCE, 450);
    leader->Study(S_PATTERN, 450);
    leader->Study(S_ILLUSION, 450);
    leader->Study(S_CREATE_PHANTASMAL_BEASTS, 180);

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
    AString *tmp_name = new AString("My Leader");
    leader->SetName(tmp_name);
    leader->Study(S_FORCE, 450);
    leader->Study(S_PATTERN, 450);
    leader->Study(S_ILLUSION, 450);
    leader->Study(S_CREATE_PHANTASMAL_BEASTS, 300);

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

};

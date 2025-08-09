#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

namespace ut = boost::ut;

ut::suite<"UnitTestHelper"> unit_test_helper_suite = []
{
  using namespace ut;

  "UnitTestHelper can access Game's private members"_test = []
  {
    // Create a game helper which will create a minimal game and provide us the ability to manipulate it for testing.
    UnitTestHelper helper;

    // at this point the game object has been created but not initialized, so it has no regions.
    expect(helper.get_region_count() == 0_i);
    // This will call Game::NewGame() which will set up the dummy game with 4 surface regions.
    helper.initialize_game();
    expect(helper.get_region_count() == 5_i);
  };

  "UnitTestHelper captures cout correctly"_test = []
  {
    // Create a game helper which will create a minimal game and provide us the ability to manipulate it for testing.
    // We do this per-test so that we have independance between tests.
    UnitTestHelper helper;

    // initialize the game which will generate a small bit of output
    helper.initialize_game();
    // The output will have the fact that it created the world and 1 '.' for each region.
    std::string current = helper.log_output();
    std::string expected = "Creating world\nMaking a level...\n....\nMaking a level...\n.\n";
    expect(current == expected);
  };

  "UnitTestHelper overrides random number generation"_test = []
  {
    // Create a game helper which will create a minimal game and provide us the ability to manipulate it for testing.
    // We do this per-test so that we have independance between tests.
    UnitTestHelper helper;

    // initialize the game which will generate a small bit of output
    helper.initialize_game();
    auto seed = helper.get_seed();
    expect(seed == 108_i);
  };

  "UnitTestHelper can set basic skills"_test = []
  {
    UnitTestHelper helper;

    helper.initialize_game();
    helper.setup_turn();

    ARegion *region = helper.get_region(1, 1, 0);
    expect(region != nullptr);

    Faction *faction = helper.create_faction("Test Faction");
    expect(faction != nullptr);

    // Basic skill test with a single unit
    Unit *unit = helper.create_unit(faction, region);
    expect(unit != nullptr);

    helper.set_skill_level(unit, S_COMBAT, 3);
    int combat_level = unit->GetRealSkill(S_COMBAT);
    expect(combat_level == 3_i);

    // Test with a different skill
    unit = helper.create_unit(faction, region);
    expect(unit != nullptr);

    helper.set_skill_level(unit, S_STEALTH, 2);
    int stealth_level = unit->GetRealSkill(S_STEALTH);
    expect(stealth_level == 2_i);

    // Test with multiple men in the unit
    unit = helper.create_unit(faction, region);
    expect(unit != nullptr);
    unit->SetMen(I_LEADERS, 3); // Set 3 leaders

    helper.set_skill_level(unit, S_COMBAT, 3);
    combat_level = unit->GetRealSkill(S_COMBAT);
    expect(combat_level == 3_i);
  };

  "UnitTestHelper handles skill prerequisites"_test = []
  {
    UnitTestHelper helper;

    helper.initialize_game();
    helper.setup_turn();

    // Use Dragon Lore which has multiple levels of prerequisites
    int dragon_lore_skill = S_DRAGON_LORE;

    // Make sure Dragon Lore is enabled
    helper.enable(UnitTestHelper::SKILL, dragon_lore_skill, true);

    // Also make sure its prerequisites are enabled
    helper.enable(UnitTestHelper::SKILL, S_BIRD_LORE, true);
    helper.enable(UnitTestHelper::SKILL, S_WOLF_LORE, true);
    helper.enable(UnitTestHelper::SKILL, S_EARTH_LORE, true);
    helper.enable(UnitTestHelper::SKILL, S_PATTERN, true);
    helper.enable(UnitTestHelper::SKILL, S_FORCE, true);

    // Set up test environment
    ARegion *region = helper.get_region(1, 1, 0);
    expect(region != nullptr);

    Faction *faction = helper.create_faction("Test Faction");
    expect(faction != nullptr);

    Unit *unit = helper.create_unit(faction, region);
    expect(unit != nullptr);

    // Set Dragon Lore to level 3
    int level = 3;
    helper.set_skill_level(unit, dragon_lore_skill, level);

    // Check that Dragon Lore was set correctly
    int dragon_level = unit->GetRealSkill(dragon_lore_skill);
    expect(dragon_level == level);

    // Check that all prerequisites in the entire chain are at least at the same level
    // Direct prerequisites: Bird Lore and Wolf Lore
    int bird_level = unit->GetRealSkill(S_BIRD_LORE);
    int wolf_level = unit->GetRealSkill(S_WOLF_LORE);
    expect(bird_level >= level);
    expect(wolf_level >= level);

    // Second level prerequisites: Earth Lore
    int earth_level = unit->GetRealSkill(S_EARTH_LORE);
    expect(earth_level >= level);

    // Third level prerequisites: Pattern and Force
    int pattern_level = unit->GetRealSkill(S_PATTERN);
    int force_level = unit->GetRealSkill(S_FORCE);
    expect(pattern_level >= level);
    expect(force_level >= level);
  };
};

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
    std::string current = helper.cout_data();
    std::string expected = "Creating world\n.....";
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
    expect(seed == 3901_i);
  };
};

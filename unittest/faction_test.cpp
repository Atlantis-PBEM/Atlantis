#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"
#include <iostream>

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"Faction"> faction_suite = []
{
  using namespace ut;

  // Insert some faction types into the global array.   This is normally done as part of Game::Game()
  FactionTypes->push_back(F_WAR);
  FactionTypes->push_back(F_TRADE);
  FactionTypes->push_back(F_MAGIC);

  "Faction id is set correctly"_test = []
  {
    Faction *faction = new Faction(1);

    // the _i suffix is a user defined literal that allows the framework to output a nicer error text.
    // (ie, if the faction number is not 1 the error will be something like
    // "file:line - test condition [1 == 2]"
    // rather than just
    // "file:line - test condition [false]"
    expect(faction->num == 1_i);

    delete faction;
  };

  "set_name appends faction id"_test = []
  {
    Faction *faction = new Faction(1);

    faction->set_name("Test Faction");
    // In order to have nice output, we need to give the test framework something it knows how to compare.
    // It groks strings, so we'll just compare the string values.  We could compare char * but then we would
    // get just a 'true' or 'false' in the compare, rather than the nice verbose output we get in this case
    // if the strings don't match.
    std::string current = faction->name;
    std::string expected = "Test Faction (1)";
    expect(eq(current, expected));

    delete faction;
  };

  "set_name filters illegal characters"_test = []
  {
    Faction *faction = new Faction(1);

    faction->set_name("Test Faction || bar");
    std::string current = faction->name;
    std::string expected = "Test Faction  bar (1)";
    expect(eq(current, expected));

    delete faction;
  };

  "FACTION order with multiple type settings"_test = [] {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    // Create test faction and unit
    Faction *faction = helper.create_faction("Test Faction");
    Unit *unit = helper.get_first_unit(faction);

    // Parse faction order with multiple type settings
    std::stringstream ss;
    ss << "#atlantis " << faction->num << "\n";
    ss << "unit " << unit->num << "\n";
    ss << "faction war 2 trade 2 magic 1\n";
    helper.parse_orders(faction->num, ss, nullptr);

    // Verify faction types are set correctly
    expect(faction->type[F_WAR] == 2);
    expect(faction->type[F_TRADE] == 2);
    expect(faction->type[F_MAGIC] == 1);
  };

  // Return the global array to it's original state.
  FactionTypes->clear();
};

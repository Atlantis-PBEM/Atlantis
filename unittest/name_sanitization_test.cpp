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
ut::suite<"Name Sanitization"> name_sanitization_suite = []
{
  using namespace ut;

  "Name is correctly read in/out on load"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);

    string expected_name("Test Faction (3)");
    string actual_name(faction->name->const_str());
    expect(actual_name == expected_name);

    stringstream ss;
    faction->Writeout(ss);

    Faction faction2;
    faction2.Readin(ss);

    string actual_name2(faction2.name->const_str());
    expect(actual_name2 == expected_name);
  };

  "Name is correctly sanitized on load"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction (3)");
    Faction *faction = helper.create_faction(name);
    // Create an unsanitized name that we want to make sure gets sanitized.
    delete faction->name;
    faction->name = new AString("Test Faction [];boy (3)");

    string expected_name("Test Faction [];boy (3)");
    string actual_name(faction->name->const_str());
    expect(actual_name == expected_name);

    stringstream ss;
    faction->Writeout(ss);

    Faction faction2;
    faction2.Readin(ss);

    string sanitized_name("Test Faction boy (3)");
    string actual_name2(faction2.name->const_str());
    expect(actual_name2 == sanitized_name);
  };
};

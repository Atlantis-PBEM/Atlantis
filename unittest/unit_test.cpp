#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Unit class in isolation.
ut::suite<"Unit"> unit_suite = []
{
  using namespace ut;

  "Unit id is set correctly"_test = []
  {
    // Create a faction and unit for this test
    Faction *faction = new Faction(1);
    Unit *unit = new Unit(500, faction, 12345);

    // the _i suffix is a user defined literal that allows the framework to output a nicer error text.
    // (ie, if the faction number is not 1 the error will be something like
    // "file:line - test condition [1 == 2]"
    // rather than just
    // "file:line - test condition [false]"
    expect(unit->num == 500_i);

    // Clean up
    delete unit;
    delete faction;
  };

  "set_name appends unit id"_test = []
  {
    // Create a faction and unit for this test
    Faction *faction = new Faction(1);
    Unit *unit = new Unit(500, faction, 12345);

    unit->set_name("Test Unit");
    // In order to have nice output, we need to give the test framework something it knows how to compare.
    // It groks strings, so we'll just compare the string values.  We could compare char * but then we would
    // get just a 'true' or 'false' in the compare, rather than the nice verbose output we get in this case
    // if the strings don't match.
    std::string current = unit->name;
    std::string expected = "Test Unit (500)";
    expect(eq(current, expected));

    // Clean up
    delete unit;
    delete faction;
  };

  "set_name filters illegal characters"_test = []
  {
    // Create a faction and unit for this test
    Faction *faction = new Faction(1);
    Unit *unit = new Unit(500, faction, 12345);

    unit->set_name("Test Unit || bar");
    std::string current = unit->name;
    std::string expected = "Test Unit  bar (500)";
    expect(eq(current, expected));

    // Clean up
    delete unit;
    delete faction;
  };

  "Units with no men have no attribute values"_test = []
  {
    // Create a faction and unit for this test
    Faction *faction = new Faction(1);
    Unit *unit = new Unit(500, faction, 12345);

    // This bug was actually found by the test below, so I am fixing it and adding a test for it as part of this
    // change.  The bug was in the evaluation of GetAttribute for objects usable by non-mages (such as rings) when
    // the unit had no men AND no objects.  In that case, it tested men (0) <= item count (0) and 0 <= 0 is true,
    // so it gave the unit a stealth level of 3.
    int stealth = unit->GetAttribute("stealth");
    expect(stealth == 0_i);
    int observation = unit->GetAttribute("observation");
    expect(observation == 0_i);

    // Clean up
    delete unit;
    delete faction;
  };

  "Units stealth obscures name correctly"_test = []
  {
    // Create a new faction and a new unit for this test
    Faction *faction = new Faction(1);
    Unit *unit = new Unit(500, faction, 12345);
    UnitTestHelper helper;

    unit->set_name("Test Unit");
    unit->SetMen(I_LEADERS, 1);

    helper.set_skill_level(unit, S_STEALTH, 2);

    // We have rank 2 of stealth.
    int stealth = unit->GetAttribute("stealth");
    expect(stealth == 2_i);

    // Someone with no observation should not see the unit's faction, since it's not revealing.
    // Technically they shouldn't even see the unit, but that's handled in the region code, NOT in the unit code.
    // This function is only called if something asserts the unit is visible before calling this.
    std::string current = unit->get_name(0);
    std::string expected = "Test Unit (500)";
    expect(eq(current, expected));

    // Someone with observation 2 should not see the unit's faction.
    current = unit->get_name(2);
    expected = "Test Unit (500)";
    expect(eq(current, expected));

    // Someone with observation 3 should see the unit's faction.
    current = unit->get_name(3);
    expected = "Test Unit (500), Faction (1)";
    expect(eq(current, expected));

    // Clean up
    delete unit;
    delete faction;
  };
};

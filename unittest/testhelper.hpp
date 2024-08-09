#pragma once
#ifndef __UNIT_TEST_HELPER_HPP__
#define __UNIT_TEST_HELPER_HPP__

#include "../game.h"

#include <iostream>
#include <sstream>

// In order to allow testing spells, this is a generic structure to capture the Run<Spell> possible arguments.
// Each spell takes a different number of arguments, though most of them take just the region and the unit.
// However, some take an object and one or two integer values as well.
struct SpellTestHelper {
    ARegion *region;
    Unit *unit;
    Object *object;
    int val1;
    int val2;
};

// This class is used to access private members of Game for testing.
// It also provides utilities to capture the output of cout into a string, as well as provide
// utility functions to manipulate the game object for testing.
//
// It does not currently provide a mechanism to to force input into cin, nor to capture files that the
// various game functions might open/write/read for more extensive end to end tests.
// The intent would be to create an integration/end-to-end test suite that used much of the same unit test
// framework, but a much more involved helper class to allow interception of those sorts of inputs/outputs
// in the future if needed.
class UnitTestHelper {
public:
    UnitTestHelper();
    ~UnitTestHelper();

    // initialize a game
    int initialize_game();
    // Set up the turn so that month gets incremented, etc.
    void setup_turn();
    // Initialize data needed for reports, such as which factions are in which regions.
    void setup_reports();

    // Some functions need to pass the game object in, so.
    Game& game_object();

    // Just get a total count of regions so we can make sure we got the world we exepected.
    int get_region_count();
    // Create a faction that we can use for testing.
    Faction *create_faction(string name);
    // Get a region by coordinates
    ARegion *get_region(int x, int y, int level);
    // Get all regions
    ARegionList *get_regions() { return &game.regions; }
    // get the game month 
    int get_month() { return game.month; }
    // Find the first unit for a given faction
    Unit *get_first_unit(Faction *faction);
    // Create a unit for the given faction in the given region.
    Unit *create_unit(Faction *faction, ARegion *region);
    // Create a fleet in the given region.
    void create_fleet(ARegion *region, Unit *owner, int ship_type, int ship_count);
    // Create a building in the given region.
    void create_building(ARegion *region, Unit *owner, int building_type);
    // Parse the orders contained in the input stream.
    void parse_orders(int faction_id, istream& orders, OrdersCheck *check = nullptr);
    // Run the transport order checks
    void check_transport_orders();
    // Execute movement orders
    void move_units();
    // Run one phase of the transport orders
    void transport_phase(TransportOrder::TransportPhase phase);
    // Collect the transported goods from the quartermasters
    void collect_transported_goods();
    // connected distance
    int connected_distance(ARegion *reg1, ARegion *reg2, int penalty, int max);

    // dummy
    int get_seed() { return getrandom(10000); };

    // Get the contents of cout as a string.
    string cout_data();

    // Activate a specific spell for testing.   Since the spells can have different arguments, we use a helper struct
    // to pass in the arguments and call the appropriate Run<Spell> function based on the skill associate with the spell.
    void activate_spell(int spell, SpellTestHelper helper);

    enum Type { SKILL, ITEM, OBJECT};
    // enable or disable items, objects etc
    void enable(Type type, int id, bool enable);

private:
    stringstream cout_buffer;
    streambuf *cout_streambuf;
    Game game;
};

#endif

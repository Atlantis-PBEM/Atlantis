#include "game.h"

#include <iostream>
#include <sstream>


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

    // Get the contents of cout as a string.
    string cout_data();

private:
    stringstream cout_buffer;
    streambuf *cout_streambuf;
    Game game;
};
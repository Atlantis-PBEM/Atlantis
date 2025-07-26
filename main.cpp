#include "gamedefs.h"
#include "game.h"
#include "items.h"
#include "skills.h"
#include "gamedata.h"
#include "strings_util.hpp"

void usage()
{
    logger::write("atlantis new");
    logger::write("atlantis run");
    logger::write("atlantis edit");
    logger::write("");
    logger::write("atlantis map <geo|wmon|lair|gate|hex> <mapfile>");
    logger::write("atlantis mapunits");
    logger::write("atlantis genrules <introfile> <cssfile> <rules-outputfile>");
    logger::write("");
    logger::write("atlantis check <orderfile> <checkfile>");
}

int main(int argc, char *argv[])
{
    Game game;
    int retval = 1;

    // Give the rng an initial see.
    rng::seed_random(1783); // this is the historical seed.. it gets reseeded in NewGame() to a random value.

    logger::write("Atlantis Engine Version: " + ATL_VER_STRING(CURRENT_ATL_VER));
    logger::write(Globals->RULESET_NAME + ", Version: " + ATL_VER_STRING(Globals->RULESET_VERSION));
    logger::write("");

    if (argc == 1) {
        usage();
        return 0;
    }

    // For simplicity, convert all arguments to strings
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }

    game.ModifyTablesPerRuleset();

    do {
        if (args[1] == "new") {
            if (!game.NewGame()) {
                logger::write("Couldn't make the new game!");
                break;
            }

            if ( !game.SaveGame() ) {
                logger::write("Couldn't save the game!");
                break;
            }

            if ( !game.WritePlayers() ) {
                logger::write("Couldn't write the players file!");
                break;
            }
        } else if (args[1] == "map") {
            if (argc != 4) {
                usage();
                break;
            }

            if (!game.OpenGame() ) {
                logger::write("Couldn't open the game file!");
                break;
            }

            if (!game.view_map(args[2], args[3])) {
                logger::write("Couldn't write the map file!");
                break;
            }
        } else if (args[1] == "run") {
            if (!game.OpenGame()) {
                logger::write("Couldn't open the game file!");
                break;
            }

            if (!game.RunGame()) {
                logger::write("Couldn't run the game!");
                break;
            }

            if (!game.SaveGame()) {
                logger::write("Couldn't save the game!");
                break;
            }
        } else if (args[1] == "edit") {
            if (!game.OpenGame()) {
                logger::write("Couldn't open the game file!");
                break;
            }

            int saveGame = 0;
            if (!game.EditGame(&saveGame)) {
                logger::write("Couldn't edit the game!");
                break;
            }

            if (saveGame) {
                if (!game.SaveGame()) {
                    logger::write("Couldn't save the game!");
                    break;
                }
            }
        } else if (args[1] == "check") {
            if (argc != 4) {
                usage();
                break;
            }

            game.DummyGame();
            if (!game.do_orders_check(args[2], args[3])) {
                logger::write("Couldn't check the orders!");
                break;
            }
        } else if (args[1] == "mapunits") {
            if (!game.OpenGame()) {
                logger::write("Couldn't open the game file!");
                break;
            }
            game.UnitFactionMap();
        } else if (args[1] == "genrules") {
            if (argc != 5) {
                usage();
                break;
            }
            if (!game.generate_rules(args[4], args[3], args[2])) {
                logger::write("Unable to generate rules!");
                break;
            }
        } else {
            logger::write("Unknown option: " + args[1]);
            break;
        }
        retval = 0;
    } while( 0 );

    return retval;
}

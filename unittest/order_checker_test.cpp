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

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"Order Checker"> order_checker_suite = [] {
    using namespace ut;

    "Order checker reports no errors with correct password"_test = [] {
        UnitTestHelper helper;
        helper.initialize_game();
        helper.setup_turn();

        std::string name("Test Faction");
        Faction *faction = helper.create_faction(name);
        faction->password = "mypassword";

        std::stringstream ss;
        ss << "#atlantis 3 \"mypassword\"\n";
        ss << "unit 2\n";
        ss << "work\n";

        std::stringstream ss2;
        orders_check checker(ss2);

        helper.parse_orders(faction->num, ss, &checker);
        expect(checker.numerrors == 0_i);
        expect(ss2.str().find("No errors found.\n") != std::string::npos);
    };

    "Order checker reports an error for incorrect password"_test = [] {
        UnitTestHelper helper;
        helper.initialize_game();
        helper.setup_turn();

        std::string name("Test Faction");
        Faction *faction = helper.create_faction(name);
        faction->password = "mypassword";

        std::stringstream ss;
        ss << "#atlantis 3 \"wrongpassword\"\n";
        ss << "unit 2\n";
        ss << "work\n";

        std::stringstream ss2;
        orders_check checker(ss2);

        helper.parse_orders(faction->num, ss, &checker);
        expect(checker.numerrors == 1_i);
        expect(ss2.str().find("No errors found.") == std::string::npos);
        expect(ss2.str().find("*** Error: Incorrect password on #atlantis line. ***\n") != std::string::npos);
    };
};

#include <iostream>

#include "astring.h"
#include "faction.h"
#include "gamedefs.h"
#include "indenter.hpp"
#include "text_report_generator.hpp"

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

using namespace std;

const int TextReportGenerator::line_width = 70;
const int TextReportGenerator::map_width = 23;
const size_t TextReportGenerator::template_fill_size = 6;
const int TextReportGenerator::template_max_lines = 13;


// Initialize the string constants
const string TextReportGenerator::template_map[] = {
//   01234567890123456789012
    "         ____          ",   // 1
    " nw     /    \\     ne  ",  // 2
    "   ____/      \\____    ",  // 3
    "  /    \\      /    \\   ", // 4
    " /      \\____/      \\  ", // 5
    " \\      /    \\      /  ", // 6
    "  \\____/      \\____/   ", // 7
    "  /    \\      /    \\   ", // 8
    " /      \\____/      \\  ", // 9
    " \\      /    \\      /  ", // 10
    "  \\____/      \\____/   ", // 11
    "       \\      /        ",  // 12
    " sw     \\____/     se  "   // 13
};

const map<const string, const array<int, 2>> TextReportGenerator::direction_offsets = {
    {"center",    {  8,   7-1 }},
    {"north",     {  8,   3-1 }},
    {"northeast", { 14,   5-1 }},
    {"southeast", { 14,   9-1 }},
    {"south",     {  8,  11-1 }},
    {"southwest", {  2,   9-1 }},
    {"northwest", {  2,   5-1 }}
};

const map<const string, const array<string, 2>> TextReportGenerator::terrain_fill = {
    { "block", {
        "####",
        "####" }
    },
    { "ocean", {
        "  ~ ~ ",
        " ~ ~  " }
    },
    { "plain", {
        "      ",
        "      " }
    },
    { "forest", {
        "  ^ ^ ",
        " ^ ^  " }
    },
    { "mountain", {
        " /\\/\\ ",
        "/  \\ \\" }
    },
    { "swamp", {
        "  v v ",
        " v v  " }
    },
    { "jungle", {
        "  @ @ ",
        " @ @  " }
    },
    { "desert", {
        "  . . ",
        " . .  " }
    },
    { "tundra", {
        "  ' ' ",
        " ' '  " }
    },
    { "cavern", {
        "  . . ",
        " . .  " }
    },
    { "underforest", {
        "  ^ ^ ",
        " ^ ^  " }
    },
    { "tunnels", {
        "      ",
        "      " }
    },
    { "nexus", {
        " !!!! ",
        " !!!! " }
    },
    { "mystforest", {
        "  ` ` ",
        " ` `  " }
    },
    { "hill", {
        "  * * ",
        " * *  " }
    },
    { "wasteland", {
        "  ; ; ",
        " ; ;  " }
    },
    { "lake", {
        "  ~ ~ ",
        " ~ ~  " }
    },
    { "grotto", {
        "  . . ",
        " . .  " }
    },
    { "deepforest", {
        "  ^ ^ ",
        " ^ ^  " }
    },
    { "chasm", {
        "      ",
        "      " }
    },
    { "volcano", {
        " /\\/\\ ",
        "/  \\ \\" }
    },
    { "barren", {
        "  * * ",
        " * *  " }
    }
};

string TextReportGenerator::to_s(const json& j) {
    string s;
    if (j.is_string()) return j.template get<string>();
    // If we somehow get something that isn't a string, just dump it as json.
    return j.dump();
}

void TextReportGenerator::output_region_header(ostream&f, const json& region, bool show_region_depth) {
    f << to_s(region["terrain"]) << " (" << region["coordinates"]["x"] << "," << region["coordinates"]["y"];
    if (region["coordinates"]["label"] != "surface") {
        int z = region["coordinates"]["z"];
        f << ",";
        if (show_region_depth) f << z << " <";
        if (region["coordinates"].contains("depth_prefix")) {
            f << to_s(region["coordinates"]["depth_prefix"]) << " ";
        }
        f << to_s(region["coordinates"]["label"]);
        if (show_region_depth) f << ">";
    }
    f << ")";
    f << " in " << to_s(region["province"]);
    if (region.contains("settlement")) {
        f << ", contains " << to_s(region["settlement"]["name"]) << " [" << to_s(region["settlement"]["size"]) << "]";
    }
}

void TextReportGenerator::output_item(ostream& f, const json& item, bool assume_singular, bool show_single_amt) {
    if (item.contains("unlimited")) {
        f << "unlimited " << to_s(item["plural"]);
    } else {
        int amount = item.value("amount", 0);
        if ((amount > 1) || (show_single_amt && (amount == 1))) f << amount << " ";
        if (item.contains("unfinished")) {
            f << "unfinished ";
            // If we are dealing with an unfinished item, we always display the singular name.
            assume_singular = true;
        }
        if (assume_singular) {
            f << to_s(item["name"]);
        } else {
            f << plural(amount, item["name"], item["plural"]);
        }
    }

    f << " [" << to_s(item["tag"]) << "]";
    if (item.contains("needs")) f << " (needs " << item["needs"] << ")";
    if (item.contains("illusion")) f << " (illusion)";
    if (item.contains("price")) f << " at $" << item["price"];
}

void TextReportGenerator::output_items(ostream& f, const json& item_list, bool assume_singular, bool show_single_amt) {
    if (item_list.empty()) return;
    bool comma = false;
    for(const auto& item : item_list) {
        if (comma) f << ", ";
        output_item(f, item, assume_singular, show_single_amt);
        comma = true;
    }
}

void TextReportGenerator::output_item_list(ostream& f, const json& item_list, string header) {
    f << header << ": ";
    if (item_list.empty()) {
        f << "none.\n";
        return;
    }
    output_items(f, item_list);
    f << ".\n";
}

void TextReportGenerator::output_ships(ostream& f, const json& ships) {
    if (ships.empty()) return;
    int count = ships.size();
    bool comma = false;
    for (const auto& ship : ships) {
        if (comma) f << ", ";
        int number = ship["number"];
        if (count > 1 || number > 1) f << number << " ";
        f << plural(number, ship["name"], ship["plural"]);
        comma = true;
    }
}

void TextReportGenerator::output_error(ostream& f, const json& error) {
    if (error.contains("unit")) {
        f << to_s(error["unit"]["name"]) << " (" << error["unit"]["number"] << "): ";
    }
    f << to_s(error["message"]) << '\n';
}

void TextReportGenerator::output_event(ostream& f, const json& event) {
    if (event.contains("unit")) {
        f << to_s(event["unit"]["name"]) << " (" << event["unit"]["number"] << "): ";
    }
    f << to_s(event["message"]) << '\n';
}

void TextReportGenerator::output_unit_summary(ostream& f, const json& unit, bool show_faction) {
    f << to_s(unit["name"]) << " (" << unit["number"] << ")";
    // display guard flag *before* the faction
    if (unit.contains("flags") && unit["flags"]["guard"]) f << ", on guard";
    if (unit.contains("faction") && show_faction) {
        f << ", " << to_s(unit["faction"]["name"]) << " (" << unit["faction"]["number"] << ")";
    }
    // and then the rest of the flags (which might or might not be present)
    if (unit["flags"].contains("avoid") && unit["flags"]["avoid"]) f << ", avoiding";
    if (unit["flags"].contains("behind") && unit["flags"]["behind"]) f << ", behind";
    if (unit["flags"].contains("reveal")) {
        if (unit["flags"]["reveal"] == "unit") f << ", revealing unit";
        if (unit["flags"]["reveal"] == "faction") f << ", revealing faction";
    }
    if (unit["flags"].contains("holding") && unit["flags"]["holding"]) f << ", holding";
    if (unit["flags"].contains("taxing") && unit["flags"]["taxing"]) f << ", taxing";
    if (unit["flags"].contains("no_aid") && unit["flags"]["no_aid"]) f << ", receiving no aid";
    if (unit["flags"].contains("sharing") && unit["flags"]["sharing"]) f << ", sharing";
    if (unit["flags"].contains("consume"))  {
        if (unit["flags"]["consume"] == "unit") f << ", consuming unit's food";
        if (unit["flags"]["consume"] == "faction") f << ", consuming faction's food";
    }
    if (unit["flags"].contains("no_cross_water") && unit["flags"]["no_cross_water"]) f << ", won't cross water";
    // All spoils is the default, so we don't need to output it.
    if (unit["flags"].contains("spoils") && unit["flags"]["spoils"] != "all") {
        f << ", " << to_s(unit["flags"]["spoils"]) << " battle spoils";
    }

    f << ", ";
    output_items(f, unit["items"]);
    if (unit.contains("weight")) f << ". Weight: " << unit["weight"];
    if (unit.contains("capacity")) {
        f << ". Capacity: " << unit["capacity"]["flying"] << "/" << unit["capacity"]["riding"] << "/"
          << unit["capacity"]["walking"] << "/" << unit["capacity"]["swimming"];
    }

    if (unit.contains("skills")) {
        f << ". Skills: ";
        if (unit["skills"].contains("known") && !unit["skills"]["known"].empty()) {
            bool comma = false;
            for(const auto& skill : unit["skills"]["known"]) {
                if (comma) f << ", ";
                f << to_s(skill["name"]) << " [" << to_s(skill["tag"]) << "]" << " " << skill["level"] << " ("
                  << skill["skill_days"];
                if (skill.contains("study_rate")) {
                    f << "+" << skill["study_rate"];
                }
                f << ")";
                comma = true;
            }
        } else {
            f << "none";
        }
    }

    if (unit.contains("combat_spell")) {
        f << ". Combat spell: " << to_s(unit["combat_spell"]["name"])
          << " [" << to_s(unit["combat_spell"]["tag"]) << "]";
    }

    // Readied items
    if (unit.contains("readied")) {
        if (unit["readied"].contains("weapons")) {
            int count = unit["readied"]["weapons"].size();
            f << ". Ready " << plural(count, "weapon", "weapons") << ": ";
            output_items(f, unit["readied"]["weapons"], true);
        }
        if (unit["readied"].contains("armor")) {
            f << ". Ready armor: ";
            output_items(f, unit["readied"]["armor"], true);
        }
        if (unit["readied"].contains("item")) {
            f << ". Ready item: ";
            output_item(f, unit["readied"]["item"], true);
        }
    }

    // Studyable skills
    if (unit.contains("skills") && unit["skills"].contains("can_study")) {
        f << ". Can Study: ";
        bool comma = false;
        for(const auto& skill : unit["skills"]["can_study"]) {
            if (comma) f << ", ";
            f << to_s(skill["name"]) << " [" << to_s(skill["tag"]) << "]";
            comma = true;
        }
    }

    if (unit.contains("visited") && !unit["visited"].empty()) {
        f << ". Has visited ";
        for (auto it = unit["visited"].begin(); it != unit["visited"].end(); it++) {
            if (it != unit["visited"].begin()) {
                if (it == unit["visited"].end() - 1)
                    f << " and ";
                else
                    f << ", ";
            }
            f << to_s(*it);
        }
    }

    if (unit.contains("description")) f << "; " << to_s(unit["description"]);
	f << ".\n";
}

void TextReportGenerator::output_unit(ostream& f, const json& unit, bool show_unit_attitudes) {
    if (unit.contains("own_unit")) {
        f << "* ";
    } else if (!show_unit_attitudes) {
        f << "- ";
    } else if (unit.contains("attitude")) {
        if(unit["attitude"] == "ally") f << "= ";
        if(unit["attitude"] == "friendly") f << ": ";
        if(unit["attitude"] == "neutral") f << "- ";
        if(unit["attitude"] == "unfriendly") f << "% ";
        if(unit["attitude"] == "hostile") f << "! ";
    } else {
        f << "- ";
    }
    output_unit_summary(f, unit);
}

void TextReportGenerator::output_structure(ostream& f, const json& structure, bool show_unit_attitudes) {
    f << "+ " << to_s(structure["name"]) << " [" << structure["number"] << "] : ";
    if (structure.contains("ships")) {
        // Fleets are wierd. If you have a fleet of a single ship, the structure type is just the ships item name.
        // If you have a fleet of ships, then the structure name if the name of the fleet type, and the ships are listed.
        if (structure["ships"].size() > 1) f << to_s(structure["type"]) << ", ";
        output_ships(f, structure["ships"]);

        if (structure.contains("damage_percent")) f << "; " << structure["damage_percent"] << "% damaged";
        if (structure.contains("load"))
            f << "; " << "Load: " << structure["load"] << "/" << structure["capacity"];
        if (structure.contains("sailors"))
            f << "; " << "Sailors: " << structure["sailors"] << "/" << structure["fleet_size"];
        if (structure.contains("max_speed")) f << "; " << "MaxSpeed: " << structure["max_speed"];
        if (structure.contains("sail_directions")) {
            f << "; " << "Sail directions: ";
            bool comma = false;
            for(const auto& direction : structure["sail_directions"]) {
                if (comma) f << ", ";
                f << to_s(direction);
                comma = true;
            }
        }
        if (structure.contains("description")) f << "; " << to_s(structure["description"]);
        f << ".\n";
    } else {
        f << to_s(structure["type"]);
        if (structure.contains("incomplete")) f << ", needs " << structure["incomplete"];
        if (structure.contains("decay")) f << ", about to decay";
        if (structure.contains("needs_maintenance")) f << ", needs maintenance";
        if (structure.contains("inner_location")) f << ", contains an inner location";
        if (structure.contains("runes")) f << ", engraved with Runes of Warding";
        if (structure.contains("description")) f << "; " << to_s(structure["description"]);
        if (structure.contains("closed")) f << ", closed to player units";
        if (structure.contains("sacrifice")) {
            int amount = structure["sacrifice"]["amount"];
            f << ", requires sacrifice of " << structure["sacrifice"]["amount"] << " "
              <<  plural(amount, structure["sacrifice"]["name"], structure["sacrifice"]["plural"]) << " ["
              << to_s(structure["sacrifice"]["tag"]) << "]";
        }
        if (structure.contains("grantskill")) {
            f << ", grants owner " << to_s(structure["grantskill"]["name"]) << " ["
              << to_s(structure["grantskill"]["tag"]) << "]" << " " << structure["grantskill"]["level"];
        }
        f << ".\n";
    }
	f << indent::incr;
    if (structure.contains("units") && !structure["units"].empty()) {
        for(const auto& unit : structure["units"]) {
            output_unit(f, unit, show_unit_attitudes);
        }
    }
    f << indent::decr;
	f << '\n';
}

void TextReportGenerator::output_region(
    ostream& f, const json& region, bool show_unit_attitudes, bool show_region_depth
) {
    output_region_header(f, region, show_region_depth);
    if (region.contains("population")) {
        f << ", " << region["population"]["amount"] << " peasants";
        if (region["population"]["race"] != "men") {
            f << " (" << to_s(region["population"]["race"]) << ")";
        }
        if (region.contains("tax")) {
            f << ", $" << region["tax"];
        }
    }
    f << ".\n";
    f << "------------------------------------------------------------\n";

    f << indent::incr;
    if (region.contains("weather")) {
        if (region["weather"]["current"] == "clear") {
            f << "The weather was clear last month; ";
        } else if (region["weather"]["current"] == "blizzard") {
            f << "There was an unnatural blizzard last month; ";
        } else {
            f << "It was " << to_s(region["weather"]["current"]) << " last month; ";
        }
        f << "it will be " << to_s(region["weather"]["next"]) << " next month.\n";
    }
    if (region.contains("description"))  f << '\n' << to_s(region["description"]) << "\n\n";

    if (region.contains("wages")) {
        f << "Wages: $" << region["wages"]["amount"];
        if (region["wages"].contains("max")) f << " (Max: $" << region["wages"]["max"] << ")";
        f << ".\n";
    }

    if (region.contains("markets")) {
        output_item_list(f, region["markets"]["wanted"], "Wanted");
        output_item_list(f, region["markets"]["for_sale"], "For Sale");
    }
    
    if (region.contains("entertainment")) f << "Entertainment available: $" << region["entertainment"] << ".\n";

    output_item_list(f, region["products"], "Products");
    f  << indent::decr << '\n';

    f << "Exits:\n" << indent::incr;
    if (region["exits"].empty()) {
        f << "none\n";
    } else {
        for (const auto& exit : region["exits"]) {
            f << to_s(exit["direction"]) << " : ";
            output_region_header(f, exit["region"], show_region_depth);
            f << ".\n";
        }
    }
    f << indent::decr << '\n';

    if (region.contains("gate")) {
        if (region["gate"].contains("open")) {
            f << "There is a Gate here (Gate " << region["gate"]["number"];
            if (region["gate"].contains("total")) {
                f << " of " << region["gate"]["total"];
            }
            f << ").\n\n";
        } else {
            f << "There is a closed Gate here.\n\n";
        }
    }

    if (region.contains("units") && !region["units"].empty()) {
        for(const auto& unit : region["units"]) {
            output_unit(f, unit, show_unit_attitudes);
        }
    }
    f << '\n';

    if (region.contains("structures") && !region["structures"].empty()) {
        for(const auto& structure : region["structures"]) {
            output_structure(f, structure, show_unit_attitudes);
        }
        f << '\n';
    } else {
        f << '\n';
    }
}

void TextReportGenerator::output(ostream& f, const json& report, bool show_region_depth) {
    f << indent::wrap;

    if (report.contains("statistics")) {
        f << ";Treasury:\n";
        f << ";\n";
        f << ";Item                                      Rank  Max        Total\n";
        f << ";=====================================================================\n";
        for (const auto& stat : report["statistics"]) {
            f << ';' << left << setw(42) << to_s(stat["item_name"]) << setw(6) << stat.value("rank", 0)
              << setw(11) << stat.value("max", 0) << stat.value("total", 0) << right << '\n';
        }
        f << '\n';
    }

    // Player reports have an extra newline.  GM reports don't.  This is silly, but for now we will emulate that
    // behavior.
    bool final_newline = false;

    if (report.contains("name")) {
        // Only player reports generate the report header
        final_newline = true;

        f << "Atlantis Report For:\n";
        f << to_s(report["name"]) << " (" << report["number"] << ")";
        if (report.contains("type")) {
            f << " (";
            bool comma = false;
            // right now there are 4 possible faction types.
            // martial, war, trade, magic.
            // while it's not legal to have martial along with war and trade, for now we will just check and output
            // all of them.
            string types[] = {"martial", "war", "trade", "magic"};
            for (const auto& type : types) {
                string key = type;
                key[0] = toupper(key[0]);
                if (report["type"].contains(type)) {
                    if (comma) f << ", ";
                    f << key << " " << report["type"][type];
                    comma = true;
                }
            }
            f << ")";
        }
        f << '\n';
        f << to_s(report["date"]["month"]) << ", Year " << report["date"]["year"] << "\n\n";
    }

    if (report.contains("engine")) {
        f << "Atlantis Engine Version: " << to_s(report["engine"]["version"]) << '\n';
        f << to_s(report["engine"]["ruleset"]) << ", Version: " << to_s(report["engine"]["ruleset_version"]) << "\n\n";
    }

    bool show_unit_attitudes = false;
    if (report.contains("administrative")) {
        if (!report["administrative"].contains("times_sent") || !report["administrative"]["times_sent"])
            f << "Note: The Times is not being sent to you.\n\n";
        if (report["administrative"].contains("password_unset") && report["administrative"]["password_unset"])
            f << "REMINDER: You have not set a password for your faction!\n\n";
        if (report["administrative"].contains("inactivity_deletion_turns")) {
            f << "WARNING: You have " << report["administrative"]["inactivity_deletion_turns"]
              << " turns until your faction is automatically removed due to inactivity!\n\n";
        }
        if (report["administrative"].contains("quit")) {
            string quit = to_s(report["administrative"]["quit"]);
            if (quit == "quit and restart") {
                f << "You restarted your faction this turn. This faction has been removed, and a new faction has "
                << "been started for you. (Your new faction report will come in a separate message.)\n";
            } else if (quit == "game over") {
                f << "I'm sorry, the game has ended. Better luck in the next game you play!\n";
            } else if (quit == "won game") {
                f << "Congratulations, you have won the game!\n";
            } else {
                f << "I'm sorry, your faction has been eliminated.\n" << "If you wish to restart, please let the "
                << "Gamemaster know, and you will be restarted for the next available turn.\n";
            }
            f << '\n';
        }
        if (report["administrative"].contains("show_unit_attitudes")) {
            show_unit_attitudes = report["administrative"]["show_unit_attitudes"];
        }
    }

    if (report.contains("status")) {
        auto status = report["status"];
        f << "Faction Status:\n";
        if (status.contains("regions")) {
            f << "Regions: " << status["regions"]["current"] << " (" << status["regions"]["allowed"] << ")\n";
        }
        if (status.contains("activity")) {
            f << "Activity: " << status["activity"]["current"] << " (" << status["activity"]["allowed"] << ")\n";
        }
        if (status.contains("tax_regions")) {
            f << "Tax Regions: " << status["tax_regions"]["current"] << " ("
              << status["tax_regions"]["allowed"] << ")\n";
        }
        if (status.contains("trade_regions")) {
            f << "Trade Regions: " << status["trade_regions"]["current"] << " ("
              << status["trade_regions"]["allowed"] << ")\n";
        }
        if (status.contains("quartermasters")) {
            f << "Quartermasters: " << status["quartermasters"]["current"] << " ("
              << status["quartermasters"]["allowed"] << ")\n";
        }
        if (status.contains("tacticians")) {
            f << "Tacticians: " << status["tacticians"]["current"] << " ("
              << status["tacticians"]["allowed"] << ")\n";
        }
        if (status.contains("mages")) {
            f << "Mages: " << status["mages"]["current"] << " (" << status["mages"]["allowed"] << ")\n";
        }
        if (status.contains("apprentices")) {
            f << to_s(status["apprentices"]["name"]) << "s: " << status["apprentices"]["current"] << " ("
              << status["apprentices"]["allowed"] << ")\n";
        }
        f << '\n';
    }

    if (report.contains("errors") && !report["errors"].empty()) {
        f << "Errors during turn:\n";
        for (const auto& error : report["errors"]) output_error(f, error);
        f << '\n';
    }

    if (report.contains("battles") && !report["battles"].empty()) {
        f << "Battles during turn:\n";
        for (const auto& battle: report["battles"]) {
            for (const auto& line: battle["report"]) {
                f << to_s(line) << '\n';
            }
        }
    }

    if (report.contains("events") && !report["events"].empty()) {
        f << "Events during turn:\n";
        for (const auto& event: report["events"]) output_event(f, event);
        f << '\n';
    }

    if(report.contains("skill_reports") && !report["skill_reports"].empty()) {
        f << "Skill reports:\n";
        for (const auto& skillshow : report["skill_reports"]) {
            f << '\n' << to_s(skillshow["name"]) << " [" << to_s(skillshow["tag"]) << "] "
              << skillshow["level"] << ": " << to_s(skillshow["description"]) << '\n';
        }
        f << '\n';
    }

    if (report.contains("item_reports") && !report["item_reports"].empty()) {
        f << "Item reports:\n";
        for (const auto& itemshow : report["item_reports"]) {
            f << '\n' << to_s(itemshow["description"]) << '\n';
        }
        f << '\n';
    }

    if (report.contains("object_reports") && !report["object_reports"].empty()) {
        f << "Object reports:\n";
        for (const auto& objectshow : report["object_reports"]) {
            f << '\n' << to_s(objectshow["name"]) << ": " << to_s(objectshow["description"]) << '\n';
        }
        f << '\n';
    }

    if (report.contains("attitudes") && !report["attitudes"].empty()) {
        string default_attitude = to_s(report["attitudes"]["default"]);
        default_attitude[0] = toupper(default_attitude[0]); 
        f << "Declared Attitudes (default " << default_attitude << "):\n";
        string available_attitudes[] = { "hostile", "unfriendly", "neutral", "friendly", "ally" };
        for (const auto& attitude : available_attitudes) {
            if (report["attitudes"].contains(attitude)) {
                string attitude_name = attitude;
                attitude_name[0] = toupper(attitude_name[0]);
                f << attitude_name << " : ";
                if (report["attitudes"][attitude].empty()) {
                    f << "none";
                } else {
                    bool comma = false;
                    for (const auto& faction : report["attitudes"][attitude]) {
                        if (comma) f << ", ";
                        f << to_s(faction["name"]) << " (" << faction["number"] << ")";
                        comma = true;
                    }
                }
                f << ".\n";
            }
        }
        f << '\n';
    }

    if(report.contains("unclaimed_silver")) {
        f << "Unclaimed silver: " << report["unclaimed_silver"] << ".\n\n";
    }

    if (report.contains("regions") && !report["regions"].empty()) {
        for (const auto& region : report["regions"])
            output_region(f, region, show_unit_attitudes, show_region_depth);
    }
    if (final_newline) f << '\n';
}

void TextReportGenerator::output_unit_orders(ostream& f, const json& orders) {
    for (const auto& order : orders) {
        f << to_s(order["order"]) << '\n';
        if (order.contains("nested") && !order["nested"].empty()) {
            f << indent::incr;
            output_unit_orders(f, order["nested"]);
            f << indent::decr;
        }
    }
}

void TextReportGenerator::output_unit_template(ostream& f, const json& unit, int template_type) {
    f << "\nunit " << unit["number"] << '\n';
    if (template_type == TEMPLATE_LONG || template_type == TEMPLATE_MAP) {
        f << indent::comment;
        output_unit_summary(f, unit, false);
    }
    if (unit.contains("orders") && !unit["orders"].empty()) {
        output_unit_orders(f, unit["orders"]);
    }
}

string TextReportGenerator::next_map_header_line(int line, const json& region) {
    if (line >= template_max_lines) return string(map_width, ' ');

    string result = template_map[line];

    // See if there is anything special to fill on for this line.
    for (const auto& offset: direction_offsets) {
        int x = offset.second[0];
        int y = offset.second[1];

        if (line == y || line == y + 1) {
            if (offset.first == "center") {
                string value = (
                    region.contains("settlement") && line == y
                    ? to_s(region["settlement"]["name"])
                    : terrain_fill.at(to_s(region["terrain"]))[line - y]
                );
                int len = min(value.length(), template_fill_size);
                result.replace(x, len, value.substr(0, len));
            } else {
                if (region.contains("exits")) {
                    for (const auto& exit : region["exits"]) {
                        string lower_dir = exit["direction"];
                        transform(lower_dir.begin(), lower_dir.end(), lower_dir.begin(), ::tolower);
                        if (lower_dir == offset.first) {
                            string value = (
                                exit["region"].contains("settlement") && line == y
                                ? to_s(exit["region"]["settlement"]["name"])
                                : terrain_fill.at(to_s(exit["region"]["terrain"]))[line - y]
                            );
                            int len = min(value.length(), template_fill_size);
                            result.replace(x, len, value.substr(0, len));
                        }
                    }
                }
            }
        }
    }
    return result;
}

void TextReportGenerator::output_region_map_header_line(ostream& f, string line) {
    string out = line.substr(0, line_width);
    out.erase(out.find_last_not_of(' ') + 1);
    f << indent::comment << out << '\n';
}

string TextReportGenerator::map_header_item(const string& header, const json& item) {
    stringstream ss;
    ss << header;
    if (item.value("unlimited", false)) ss << "unlim";
    else ss << setw(5) << right << item.value("amount", 0);
    ss << " " << setw(4) << to_s(item["tag"]);
    if (item.contains("price")) ss << " @ " << setw(3) << right << item.value("price", 0);
    return ss.str();
}

void TextReportGenerator::output_region_map_header(ostream& f, const json& region, bool show_region_depth) {
    f << '\n' << indent::comment << "-----------------------------------------------------------\n";
    f << indent::comment;
    output_region_header(f, region, show_region_depth);
    f << '\n';

    int line = 0;
    output_region_map_header_line(f, next_map_header_line(line++, region));

    if (region.contains("weather")) {
        string weather = "Next " + to_s(region["weather"]["next"]);
        // strip off ' season' if it's there (for monsoon)
        if (weather.ends_with(" season")) weather = weather.substr(0, weather.length() - 7);
        output_region_map_header_line(f, next_map_header_line(line++, region) + weather);
    }

    stringstream ss;
    ss << "Tax  " << setw(5) << right << region.value("tax", 0);
    output_region_map_header_line(f, next_map_header_line(line++, region) + ss.str());

    if (region.contains("entertainment")) {
        stringstream ss;
        ss << "Ente " << setw(5) << right << region.value("entertainment", 0);
        output_region_map_header_line(f, next_map_header_line(line++, region) + ss.str());
    }

    if (region.contains("wages")) {
        stringstream ss;
        ss << "Wage " << setprecision(1) << fixed << setw(7) << right << region["wages"].value("amount", 0.0)
           << " (max " << region["wages"].value("max", 0) << ")";
        output_region_map_header_line(f, next_map_header_line(line++, region) + ss.str());
    }

    bool first = true;
    if (region.contains("markets")) {
        for (const auto& item : region["markets"]["wanted"]) {
            if (first) output_region_map_header_line(f, next_map_header_line(line++, region));
            string s = map_header_item((first ? "Want " : "     "), item);
            output_region_map_header_line(f, next_map_header_line(line++, region) + s);
            first = false;
        }

        first = true;
        for (const auto& item : region["markets"]["for_sale"]) {
            if (first) output_region_map_header_line(f, next_map_header_line(line++, region));
            string s = map_header_item((first ? "Sell " : "     "), item);
            output_region_map_header_line(f, next_map_header_line(line++, region) + s);
            first = false;
        }
    }

    first = true;
    for (const auto& item : region["products"]) {
        if (first) output_region_map_header_line(f, next_map_header_line(line++, region));
        string s = map_header_item((first ? "Prod " : "     "), item);
        output_region_map_header_line(f, next_map_header_line(line++, region) + s);
        first = false;
    }

    if (region.contains("gate")) {
        if (region["gate"].contains("open")) {
            stringstream ss;
            ss << "Gate " << setw(4) << right << region["gate"].value("number", 0);
            output_region_map_header_line(f, next_map_header_line(line++, region) + ss.str());
        } else {
            output_region_map_header_line(f, next_map_header_line(line++, region) + "Gate closed");
        }
    }

    while (line < template_max_lines) {
        output_region_map_header_line(f, next_map_header_line(line++, region));
    }
}

void TextReportGenerator::output_region_template(
    ostream& f, const json& region, int template_type, bool show_region_depth
) {
    if (template_type == TEMPLATE_MAP) output_region_map_header(f, region, show_region_depth);
    else {
        f << "\n" << indent::comment << "*** ";
        output_region_header(f, region, show_region_depth);
        f << " ***\n";
    }

    if (region.contains("units")) {
        for(auto& unit: region["units"]) {
            if (unit.contains("own_unit")) output_unit_template(f, unit, template_type);
        }
    }

    if (region.contains("structures")) {
        for (auto& structure : region["structures"]) {
            for (auto &unit : structure["units"]) {
                if (unit.contains("own_unit")) output_unit_template(f, unit, template_type);
            }
        }
    }
}

void TextReportGenerator::output_template(ostream& f, const json& report, int template_type, bool show_region_depth) {
    f << indent::wrap;
    f << "\n";
    f << "Orders Template (";
    f << (template_type == TEMPLATE_SHORT ? "Short" : (template_type == TEMPLATE_LONG ? "Long" : "Map"));
    f << " Format):\n\n";
    f << "#atlantis " << report["number"];

    if (!report["administrative"].value("password_unset", true)) {
        // Since the json output will quote for strings, just use it as is
        f << " " << report["administrative"]["password"];
    }
    f << "\n";

    if (report.contains("regions") && !report["regions"].empty()) {
        for(const auto& region : report["regions"]) {
            // Only output the region template if you have actual units in the region.
            if (region.contains("present")) output_region_template(f, region, template_type, show_region_depth);
        }
    }
    f << "\n#end\n\n";
}

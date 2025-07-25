#ifdef WIN32
#include <memory.h> // Needed for memcpy on windows
#include "io.h"     // Needed for access() on windows
#define F_OK    0
#endif

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string.h>

#include "game.h"
#include "gamedata.h"
#include "indenter.hpp"
#include "text_report_generator.hpp"
#include "quests.h"
#include "unit.h"
#include "rng.hpp"
#include "string_parser.hpp"
#include "strings_util.hpp"

#include "external/nlohmann/json.hpp"
using json = nlohmann::json;

Game::Game()
{
    gameStatus = GAME_STATUS_UNINIT;
    ppUnits = 0;
    maxppunits = 0;
    events = new Events();
    rulesetSpecificData = json::object();

    if (Globals->FACTION_ACTIVITY == FactionActivityRules::DEFAULT)
    {
        FactionTypes->push_back(F_WAR);
        FactionTypes->push_back(F_TRADE);
        FactionTypes->push_back(F_MAGIC);
    }
    else
    {
        FactionTypes->push_back(F_MARTIAL);
        FactionTypes->push_back(F_MAGIC);
    }
}

Game::~Game()
{
    delete[] ppUnits;
    delete events;
    ppUnits = 0;
    maxppunits = 0;
    // Return the global array to it's original state. (needed for unit tests)
    FactionTypes->clear();
}

int Game::TurnNumber()
{
    return (year-1)*12 + month + 1;
}

// ALT, 25-Jul-2000
// Default work order procedure
void Game::DefaultWorkOrder()
{
    for(const auto r : regions) {
        if (r->type == R_NEXUS) continue;
        for(const auto o : r->objects) {
            for(const auto u : o->units) {
                if (u->monthorders || u->faction->is_npc || (Globals->TAX_PILLAGE_MONTH_LONG && u->taxing != TAX_NONE))
                    continue;
                if (u->GetFlag(FLAG_AUTOTAX) && (Globals->TAX_PILLAGE_MONTH_LONG && u->Taxers(1))) {
                    u->taxing = TAX_AUTO;
                } else if (Globals->DEFAULT_WORK_ORDER) {
                    ProcessWorkOrder(u, 1, 0);
                }
            }
        }
    }
}

std::string Game::GetXtraMap(ARegion *reg, int type)
{
    int i;

    if (!reg) return " ";

    switch (type) {
        case 0:
            return reg->IsStartingCity() ? "!" : (reg->HasShaft() ? "*" : " ");
        case 1:
            i = reg->CountWMons();
            return (i ? std::to_string(i) : " ");
        case 2:
            for(const auto o : reg->objects) {
                if (!(ObjectDefs[o->type].flags & ObjectType::CANENTER)) {
                    return (o->units.front() ? "*" : ".");
                }
            }
            return " ";
        case 3:
            if (reg->gate) return "*";
            return " ";
    }
    return " ";
}

void Game::WriteSurfaceMap(std::ostream& f, ARegionArray *pArr, int type)
{
    ARegion *reg;
    int yy = 0;
    int xx = 0;

    f << "Map (" << xx*32 << "," << yy*16 << ")\n";
    for (int y=0; y < pArr->y; y+=2) {
        std::string temp;
        int x;
        for (x=0; x< pArr->x; x+=2) {
            reg = pArr->GetRegion(x+xx*32,y+yy*16);
            temp += GetRChar(reg);
            temp += GetXtraMap(reg,type);
            temp += "  ";
        }
        f << temp << "\n";
        temp = "  ";
        for (x=1; x< pArr->x; x+=2) {
            reg = pArr->GetRegion(x+xx*32,y+yy*16+1);
            temp += GetRChar(reg);
            temp += GetXtraMap(reg,type);
            temp += "  ";
        }
        f << temp << "\n";
    }
    f << "\n";
}

void Game::WriteUnderworldMap(std::ostream& f, ARegionArray *pArr, int type)
{
    ARegion *reg, *reg2;
    int xx = 0;
    int yy = 0;
    f << "Map (" << xx*32 << "," << yy*16 << ")\n";
    for (int y=0; y< pArr->y; y+=2) {
        std::string temp = " ";
        std::string temp2;
        int x;
        for (x=0; x< pArr->x; x+=2) {
            reg = pArr->GetRegion(x+xx*32,y+yy*16);
            reg2 = pArr->GetRegion(x+xx*32+1,y+yy*16+1);
            temp += GetRChar(reg);
            temp += GetXtraMap(reg,type);
            if (reg2 && reg2->neighbors[D_NORTH]) temp += "|";
            else temp += " ";

            temp += " ";
            if (reg && reg->neighbors[D_SOUTHWEST]) temp2 += "/";
            else temp2 += " ";

            temp2 += " ";
            if (reg && reg->neighbors[D_SOUTHEAST]) temp2 += "\\";
            else temp2 += " ";

            temp2 += " ";
        }
        f << temp << "\n" << temp2 << "\n";

        temp = " ";
        temp2 = "  ";
        for (x=1; x< pArr->x; x+=2) {
            reg = pArr->GetRegion(x+xx*32,y+yy*16+1);
            reg2 = pArr->GetRegion(x+xx*32-1,y+yy*16);

            if (reg2 && reg2->neighbors[D_SOUTH]) temp += "|";
            else temp += " ";

            temp += " ";
            temp += GetRChar(reg);
            temp += GetXtraMap(reg,type);

            if (reg && reg->neighbors[D_SOUTHWEST]) temp2 += "/";
            else temp2 += " ";

            temp2 += " ";
            if (reg && reg->neighbors[D_SOUTHEAST]) temp2 += "\\";
            else temp2 += " ";

            temp2 += " ";
        }
        f << temp << "\n" << temp2 << "\n";
    }
    f << "\n";
}

int Game::view_map(const std::string& typestr,const std::string& mapfile)
{
    int type = 0;
    if (typestr == "wmon") type = 1;
    if (typestr == "lair") type = 2;
    if (typestr == "gate") type = 3;
    if (typestr == "cities") type = 4;
    if (typestr == "hex") type = 5;

    std::ofstream f(mapfile, std::ios::out|std::ios::ate);
    if (!f.is_open()) return(0);

    switch (type) {
        case 0:
            f << "Geographical Map\n";
            break;
        case 1:
            f << "Wandering Monster Map\n";
            break;
        case 2:
            f << "Lair Map\n";
            break;
        case 3:
            f << "Gate Map\n";
            break;
        case 4:
            f << "Cities Map\n";
            break;
    }

    // Cities map is a bit special since it is really just a list of all the cities in that region
    if (type == 4) {
        for(const auto reg : regions) {
            // Ignore anything that isn't the surface
            if (reg->level->levelType != ARegionArray::LEVEL_SURFACE) continue;
            // Ignore anything with no city
            if (!reg->town || (reg->town->TownType() != TOWN_CITY)) continue;

            f << "(" << reg->xloc << "," << reg->yloc << "): " << reg->town->name << "\n";
        }
        return(1);
    }

    if (type == 5) {
        json worldmap;

        std::vector<ARegion *> start_regions;
        for (auto i = 0; i < regions.numLevels; i++) {
            ARegionArray *pArr = regions.pRegionArrays[i];
            if (pArr->levelType == ARegionArray::LEVEL_NEXUS) {
                if (!Globals->START_CITIES_EXIST) {
                    ARegion *nexus = pArr->GetRegion(0, 0);
                    for(const auto o : nexus->objects) {
                        if (o->inner != -1) {
                            start_regions.push_back(regions.GetRegion(o->inner));
                        }
                    }
                }
                continue;
            };
            std::string label = (pArr->strName.empty() ? "surface" : (pArr->strName + std::to_string(i-1)));
            worldmap[label] = json::array();

            for (int y = 0; y < pArr->y; y++) {
                for (int x = 0; x < pArr->x; x++) {
                    ARegion *reg = pArr->GetRegion(x, y);
                    if (!reg) continue;
                    json data = reg->basic_region_data();
                    json hexout = {
                        { "x", x }, { "y", y }, { "z", i },
                        { "terrain", data["terrain"] },
                        { "yew", reg->produces_item(I_YEW) },
                        { "mithril", reg->produces_item(I_MITHRIL) },
                        { "admantium", reg->produces_item(I_ADMANTIUM) },
                        { "floater", reg->produces_item(I_FLOATER) },
                        { "wing", reg->produces_item(I_WHORSE) },
                        { "gate", reg->gate != 0 },
                        { "exits", json::array() },
                        { "shaft", reg->HasShaft() },
                        { "starting",
                          (start_regions.end() != std::find(start_regions.begin(), start_regions.end(), reg)) ||
                          reg->IsStartingCity()
                        }
                    };
                    // Fill in the exits
                    for (auto d = 0; d < NDIRS; d++) {
                        hexout["exits"].push_back(reg->neighbors[d] != nullptr);
                    }
                    if (reg->town) {
                        hexout["town"] = data["settlement"]["size"];
                    }
                    worldmap[label].push_back(hexout);
                }
            }
        }
        f << worldmap.dump(2);
        return(1);
    }

    int i;
    for (i = 0; i < regions.numLevels; i++) {
        f << '\n';
        ARegionArray *pArr = regions.pRegionArrays[i];
        switch(pArr->levelType) {
            case ARegionArray::LEVEL_NEXUS:
                f << "Level " << i << ": Nexus\n";
                break;
            case ARegionArray::LEVEL_SURFACE:
                f << "Level " << i << ": Surface\n";
                WriteSurfaceMap(f, pArr, type);
                break;
            case ARegionArray::LEVEL_UNDERWORLD:
                f << "Level " << i << ": Underworld\n";
                WriteUnderworldMap(f, pArr, type);
                break;
            case ARegionArray::LEVEL_UNDERDEEP:
                f << "Level " << i << ": Underdeep\n";
                WriteUnderworldMap(f, pArr, type);
                break;
        }
    }
    return(1);
}

int Game::NewGame()
{
    factionseq = 1;
    guardfaction = 0;
    monfaction = 0;
    unitseq = 1;
    SetupUnitNums();
    shipseq = 100;
    year = 1;
    month = -1;
    gameStatus = GAME_STATUS_NEW;

    //
    // Seed the random number generator with a different value each time.
    //
    // init_random_seed() is a function reference that can be overwritten by the test suites to control the random seed
    // so that tests are repeatable.
    init_random_seed();

    CreateWorld();
    CreateNPCFactions();

    if (Globals->CITY_MONSTERS_EXIST)
        CreateCityMons();
    if (Globals->WANDERING_MONSTERS_EXIST)
        CreateWMons();
    if (Globals->LAIR_MONSTERS_EXIST)
        CreateLMons();

    if (Globals->LAIR_MONSTERS_EXIST)
        CreateVMons();

    /*
    if (Globals->PLAYER_ECONOMY) {
        Equilibrate();
    }
    */



    return(1);
}

int Game::OpenGame()
{
    //
    // The order here must match the order in SaveGame
    //
    std::ifstream f("game.in", std::ios::in);
    if (!f.is_open()) return(0);
    //
    // Read in Globals
    std::string str;
    std::getline(f >> std::ws, str);
    if (f.eof()) return(0);

    if (str.empty()) return(0);

    if (str != "atlantis_game") return(0);

    ATL_VER eVersion;
    f >> eVersion;
    logger::write("Saved Game Engine Version: " + ATL_VER_STRING(eVersion));
    if (
        ATL_VER_MAJOR(eVersion) != ATL_VER_MAJOR(CURRENT_ATL_VER) ||
        ATL_VER_MINOR(eVersion) != ATL_VER_MINOR(CURRENT_ATL_VER)
    ) {
        logger::write("Incompatible Engine versions!");
        return(0);
    }
    if (ATL_VER_PATCH(eVersion) > ATL_VER_PATCH(CURRENT_ATL_VER)) {
        logger::write("This game was created with a more recent Atlantis Engine!");
        return(0);
    }

    std::string gameName;
    f >> std::ws;
    std::getline(f, gameName);
    if (f.eof()) return(0);

    if (!(gameName == Globals->RULESET_NAME)) {
        logger::write("Incompatible rule-set!");
        return(0);
    }

    ATL_VER gVersion;
    f >> gVersion;
    logger::write("Saved Rule-Set Version: " + ATL_VER_STRING(gVersion));

    if (ATL_VER_MAJOR(gVersion) < ATL_VER_MAJOR(Globals->RULESET_VERSION)) {
        logger::write("Upgrading to " + ATL_VER_STRING(MAKE_ATL_VER(ATL_VER_MAJOR(Globals->RULESET_VERSION), 0, 0)));
        if (!upgrade_major_version(gVersion)) {
            logger::write("Unable to upgrade!  Aborting!");
            return(0);
        }
        gVersion = MAKE_ATL_VER(ATL_VER_MAJOR(Globals->RULESET_VERSION), 0, 0);
    }
    if (ATL_VER_MINOR(gVersion) < ATL_VER_MINOR(Globals->RULESET_VERSION)) {
        logger::write("Upgrading to " + ATL_VER_STRING(
            MAKE_ATL_VER(ATL_VER_MAJOR(Globals->RULESET_VERSION), ATL_VER_MINOR(Globals->RULESET_VERSION), 0)
        ));
        if (!upgrade_minor_version(gVersion)) {
            logger::write("Unable to upgrade!  Aborting!");
            return(0);
        }
        gVersion = MAKE_ATL_VER(ATL_VER_MAJOR(gVersion), ATL_VER_MINOR(Globals->RULESET_VERSION), 0);
    }
    if (ATL_VER_PATCH(gVersion) < ATL_VER_PATCH(Globals->RULESET_VERSION)) {
        logger::write("Upgrading to " + ATL_VER_STRING(Globals->RULESET_VERSION));
        if (!upgrade_patch_level(gVersion)) {
            logger::write("Unable to upgrade!  Aborting!");
            return(0);
        }
        gVersion = MAKE_ATL_VER(ATL_VER_MAJOR(gVersion), ATL_VER_MINOR(gVersion), ATL_VER_PATCH(Globals->RULESET_VERSION));
    }

    f >> year;
    f >> month;

    int seed;
    f >> seed;
    rng::seed_random(seed);

    f >> factionseq;
    f >> unitseq;
    f >> shipseq;
    f >> guardfaction;
    f >> monfaction;

    //
    // Read in the Factions
    //
    int i;
    f >> i;

    for (int j = 0; j < i; j++) {
        Faction *temp = new Faction;
        temp->Readin(f);
        factions.push_back(temp);
    }

    //
    // Read in the ARegions
    //
    i = regions.ReadRegions(f, factions);
    if (!i) return 0;

    // read in quests
    if (!quests.read_quests(f))
        return 0;

    SetupUnitNums();

    return(1);
}

int Game::SaveGame()
{
    std::ofstream f("game.out", std::ios::out|std::ios::ate);
    if (!f.is_open()) return(0);

    //
    // Write out Globals
    //
    f << "atlantis_game\n";
    f << CURRENT_ATL_VER << "\n";
    f << Globals->RULESET_NAME << "\n";
    f << Globals->RULESET_VERSION << "\n";

    f << year << "\n";
    f << month << "\n";
    f << rng::get_random(10000) << "\n";
    f << factionseq << "\n";
    f << unitseq << "\n";
    f << shipseq << "\n";
    f << guardfaction << "\n";
    f << monfaction << "\n";
    //
    // Write out the Factions
    //
    f << factions.size() << "\n";
    for(const auto fac : factions) {
        fac->Writeout(f);
    }

    //
    // Write out the ARegions
    //
    regions.WriteRegions(f);

    // Write out quests
    quests.write_quests(f);

    return(1);
}

void Game::DummyGame()
{
    //
    // No need to set anything up; we're just syntax checking some orders.
    //
}

#define PLAYERS_FIRST_LINE "AtlantisPlayerStatus"

int Game::WritePlayers()
{
    std::ofstream f("players.out", std::ios::out|std::ios::ate);
    if (!f.is_open()) return(0);

    f << PLAYERS_FIRST_LINE << "\n";
    f << "Version: " << CURRENT_ATL_VER << "\n";
    f << "TurnNumber: " << TurnNumber() << "\n";
    if (gameStatus == GAME_STATUS_UNINIT)
        return(0);

    if (gameStatus == GAME_STATUS_NEW)
        f << "GameStatus: New\n\n";
    else if (gameStatus == GAME_STATUS_RUNNING)
        f << "GameStatus: Running\n\n";
    else if (gameStatus == GAME_STATUS_FINISHED)
        f << "GameStatus: Finished\n\n";

    for(const auto fac : factions) {
        fac->WriteFacInfo(f);
    }

    return(1);
}

bool Game::ReadPlayers()
{
    std::ifstream f("players.in", std::ios::in);
    if (!f.is_open()) return(0);

    parser::string_parser parser;

    //
    // Default: failure.
    //
    bool return_code = false;

    do {
        //
        // The first line of the file should match.
        //
        f >> parser;
        if (parser.get_token() != PLAYERS_FIRST_LINE) break;

        //
        // Get the file version number.
        //
        f >> parser;
        if (parser.get_token() != "Version:") break;

        auto token = parser.get_token();
        if (!token) break;

        int nVer = token.get_number().value_or(0);
        if (
            ATL_VER_MAJOR(nVer) != ATL_VER_MAJOR(CURRENT_ATL_VER) ||
            ATL_VER_MINOR(nVer) != ATL_VER_MINOR(CURRENT_ATL_VER) ||
            ATL_VER_PATCH(nVer) > ATL_VER_PATCH(CURRENT_ATL_VER)
        ) {
            logger::write("The players.in file is not compatible with this version of Atlantis.");
            break;
        }

        //
        // Ignore the turn number line.
        //
        f >> parser;

        //
        // Next, the game status.
        //
        f >> parser;
        if (parser.get_token() != "GameStatus:") break;

        token = parser.get_token();
        if (!token) break;

        if (token == "New") gameStatus = GAME_STATUS_NEW;
        else if (token == "Running") gameStatus = GAME_STATUS_RUNNING;
        else if (token == "Finished") gameStatus = GAME_STATUS_FINISHED;
        else break; // invalid game status

        //
        // Now, we should have a list of factions.
        //
        f >> parser;
        Faction *fac = nullptr;

        bool lastWasNew = false;

        //
        // OK, set our return code to success; we'll set it to fail below
        // if necessary.
        //
        return_code = true;

        while(!f.eof()) {
            auto token = parser.get_token();
            if (!token) {
                f >> parser;
                continue;
            }

            if (token == "Faction:") {
                //
                // Get the new faction
                //
                token = parser.get_token();
                if (!token) {
                    return_code = false;
                    break;
                }

                if (token == "new") {
                    std::string save = parser.str();
                    int noleader = 0;
                    int x, y, z;
                    ARegion *reg = nullptr;

                    /* Check for the noleader flag */
                    token = parser.get_token();
                    if (token == "noleader") {
                        noleader = 1;
                        token = parser.get_token();
                        /* Initialize reg to something useful */
                        reg = regions.GetRegion(0, 0, 0);
                    }

                    x = token.get_number().value_or(-1);
                    y = parser.get_token().get_number().value_or(-1);
                    z = parser.get_token().get_number().value_or(-1);
                    if (x != -1 && y != -1 && z != -1) {
                        reg = regions.GetRegion(x, y, z);
                        if (reg == nullptr) logger::write("Bad faction line: " + save);
                    }

                    fac = AddFaction(noleader, reg);
                    if (!fac) {
                        logger::write("Failed to add a new faction!");
                        return_code = false;
                        break;
                    }

                    lastWasNew = true;
                } else {
                    int nFacNum = token.get_number().value_or(0);
                    lastWasNew = false;
                    fac = GetFaction(factions, nFacNum);
                    if (!fac) continue;

                    fac->startturn = TurnNumber();
                }
            } else if (fac) {
                return_code = ReadPlayersLine(token, parser, fac, lastWasNew);
                if (!return_code) break;
            }

            f >> parser;
        }
    } while(false);

    return(return_code);
}

Unit *Game::parse_gm_unit(std::string tag, Faction *fac)
{
    if (tag.empty()) return nullptr;

    if (tag[0] == 'g' && tag[1] == 'm') {
        auto gma = parser::token(tag.substr(2)).get_number();
        for(const auto reg : regions) {
            for(const auto obj : reg->objects) {
                for(const auto u : obj->units) {
                    if (u->faction->num == fac->num && u->gm_alias == gma) {
                        return u;
                    }
                }
            }
        }
    } else {
        auto v = parser::token(tag).get_number();
        if (!v) return nullptr;
        int id = v.value();
        if (static_cast<unsigned int>(id) >= maxppunits) return nullptr;
        return GetUnit(id);
    }
    return nullptr;
}

bool Game::ReadPlayersLine(parser::token& token, parser::string_parser& parser, Faction *fac, bool new_player)
{
    if (token == "Name:") {
        std::string name = parser.str();
        if (!name.empty()) {
            if (new_player) name += " (" + std::to_string(fac->num) + ")";
            fac->set_name(name, false);
        }
        return true;
    }

    if (token == "RewardTimes") {
        fac->TimesReward();
        return true;
    }

    if (token == "Email:") {
        std::string str = parser.get_token().get_string();
        if (!str.empty()) {
            fac->address = str;
        }
        return true;
    }

    if (token == "Password:") {
        std::string newpass = parser.get_token().get_string();
        if (newpass.empty()) newpass = "none";
        fac->password = newpass;
        return true;
    }

    if (token == "Battle:") {
        fac->battleLogFormat = 0;
        return true;
    }

    if (token == "Template:") {
        token = parser.get_token();
        int temp = parse_template_type(token);
        fac->temformat = temp == -1 ? TEMPLATE_LONG : temp;
        return true;
    }

    if (token == "Reward:") {
        int amt = parser.get_token().get_number().value_or(0);
        fac->event("Reward of " + std::to_string(amt) + " silver.", "reward");
        fac->unclaimed += amt;
        return true;
    }

    if (token == "SendTimes:") {
        // get the token, but otherwise ignore it
        fac->times = parser.get_token().get_number().value_or(0);
        return true;
    }

    if (token == "LastOrders:") {
        // Read this line and correctly set the lastorders for this
        // faction if the game itself isn't maintaining them.
        if (Globals->LASTORDERS_MAINTAINED_BY_SCRIPTS)
            fac->lastorders = parser.get_token().get_number().value_or(0);
        return true;
    }

    if (token == "FirstTurn:") {
        fac->startturn = parser.get_token().get_number().value_or(0);
        return true;
    }

    if (token == "Loc:") {
        auto x = parser.get_token().get_number();
        auto y = parser.get_token().get_number();
        auto z = parser.get_token().get_number();

        ARegion *reg = nullptr;
        if (x && y && z) {
            reg = regions.GetRegion(x.value(), y.value(), z.value());
        }
        if (!reg) {
            std::string str = "Invalid Loc: ";
            str += x ? std::to_string(x.value()) + ", " : "missing x-coord, ";
            str += y ? std::to_string(y.value()) + ", " : "missing y-coord, ";
            str += z ? std::to_string(z.value()) : "missing z-coord";
            str += " in faction " + std::to_string(fac->num);
            logger::write(str);
        }
        fac->pReg = reg;
        return true;
    }

    if (token == "NewUnit:") {
        // Creates a new unit in the location specified by a Loc: line
        // with a gm_alias of whatever is after the NewUnit: tag.
        if (!fac->pReg) {
            logger::write("NewUnit is not valid without a Loc: for faction "+ std::to_string(fac->num));
            return true;
        }
        int val = parser.get_token().get_number().value_or(0);
        if (!val) {
            logger::write("NewUnit: must be followed by an alias in faction " + std::to_string(fac->num));
            return true;
        }
        Unit *u = GetNewUnit(fac);
        u->gm_alias = val;
        u->MoveUnit(fac->pReg->GetDummy());
        u->event("Is given to your faction.", "gm_gift");
        return true;
    }

    if (token == "Item:") {
        std::string alias = parser.get_token().get_string();
        if (alias.empty()) {
            logger::write("Item: needs to specify a unit in faction " + std::to_string(fac->num));
            return true;
        }
        Unit *u = parse_gm_unit(alias, fac);
        if (!u) {
            logger::write("Item: needs to specify a unit in faction " + std::to_string(fac->num));
            return true;
        }
        if (u->faction->num != fac->num) {
            logger::write("Item: unit "+ std::to_string(u->num) + " doesn't belong to faction " +
                std::to_string(fac->num));
            return true;
        }

        auto val = parser.get_token().get_number();
        if (!val) {
            logger::write("Must specify a number of items to give for Item: in faction " + std::to_string(fac->num));
            return true;
        }

        token = parser.get_token();
        int it = parse_all_items(token);
        if (it == -1) {
            logger::write("Must specify a valid item to give for Item: in faction " + std::to_string(fac->num));
            return true;
        }

        int has = u->items.GetNum(it);
        u->items.SetNum(it, has + val.value_or(0));
        if (!u->gm_alias) {
            u->event("Is given " + item_string(it, val.value_or(0)) + " by the gods.", "gm_gift");
        }
        u->faction->DiscoverItem(it, 0, 1);
        return true;
    }

    if (token == "Skill:") {
        std::string alias = parser.get_token().get_string();
        if (alias.empty()) {
            logger::write("Skill: needs to specify a unit in faction " + std::to_string(fac->num));
            return true;
        }
        Unit *u = parse_gm_unit(alias, fac);
        if (!u) {
            logger::write("Skill: needs to specify a unit in faction " + std::to_string(fac->num));
            return true;
        }
        if (u->faction->num != fac->num) {
            logger::write("Item: unit "+ std::to_string(u->num) + " doesn't belong to faction " +
                std::to_string(fac->num));
            return true;
        }

        token = parser.get_token();
        int sk = parse_skill(token);
        if (sk == -1) {
            logger::write("Must specify a valid skill for Skill: in faction " + std::to_string(fac->num));
            return true;
        }

        int days = parser.get_token().get_number().value_or(0) * u->GetMen();
        if (!days) {
            logger::write("Must specify a days for Skill: in faction " + std::to_string(fac->num));
            return true;
        }

        int odays = u->skills.GetDays(sk);
        u->skills.SetDays(sk, odays + days);
        u->AdjustSkills();
        int lvl = u->GetRealSkill(sk);
        if (lvl > fac->skills.GetDays(sk)) {
            fac->skills.SetDays(sk, lvl);
            fac->shows.push_back({ .skill = sk, .level = lvl });
        }
        if (!u->gm_alias) {
            u->event("Is taught " + std::to_string(days) + " days of " + SkillStrs(sk) + " by the gods.", "gm_gift");
        }
        /* This is NOT quite the same, but the gods are more powerful than mere mortals */
        int mage = (SkillDefs[sk].flags & SkillType::MAGIC);
        int app = (SkillDefs[sk].flags & SkillType::APPRENTICE);
        if (mage) u->type = U_MAGE;
        if (app && u->type == U_NORMAL) u->type = U_APPRENTICE;
        return true;
    }

    if (token == "Order:") {
        std::string alias = parser.get_token().get_string();
        if (alias == "quit") {
            fac->quit = QUIT_BY_GM;
            return true;
        }
        //Not quit so, handle it as a unit order
        Unit *u = parse_gm_unit(alias, fac);
        if (!u) {
            logger::write("Order: needs to specify a unit in faction " + std::to_string(fac->num));
            return true;
        }
        if (u->faction->num != fac->num) {
            logger::write("Order: unit "+ std::to_string(u->num) + " doesn't belong to faction " +
                std::to_string(fac->num));
            return true;
        }

        std::string saved = parser.str();
        bool repeating = parser.get_at();
        token = parser.get_token();
        if (!token) {
            logger::write("Order: must provide unit order for faction "+ std::to_string(fac->num));
            return true;
        }
        int o = Parse1Order(token);
        if (o == -1 || o == O_ATLANTIS || o == O_END || o == O_UNIT || o == O_FORM || o == O_ENDFORM) {
            logger::write("Order: invalid order given for faction "+ std::to_string(fac->num));
            return true;
        }
        if (repeating) {
            u->oldorders.push_back(saved);
        }
        ProcessOrder(o, u, parser, nullptr, repeating);
        return true;
    }

    std::string temp = parser.original();
    if (temp.empty()) return true;

    fac->extra_player_data.push_back(temp);
    return true;
}

int Game::do_orders_check(const std::string& strOrders, const std::string& strCheck)
{
    std::ifstream ordersFile(strOrders, std::ios::in);
    if (!ordersFile.is_open()) {
        logger::write("No such orders file!");
        return(0);
    }

    std::ofstream checkFile(strCheck, std::ios::out|std::ios::ate);
    if (!checkFile.is_open()) {
        logger::write("Couldn't open the orders check file!");
        return(0);
    }

    orders_check check(checkFile);
    ParseOrders(0, ordersFile, &check);

    return(1);
}

int Game::RunGame()
{
    logger::write("Setting Up Turn...");
    PreProcessTurn();

    logger::write("Reading the Gamemaster File...");
    if (!ReadPlayers()) return(0);

    if (gameStatus == GAME_STATUS_FINISHED) {
        logger::write("This game is finished!");
        return(0);
    }
    gameStatus = GAME_STATUS_RUNNING;

    logger::write("Reading the Orders File...");
    ReadOrders();

    if (Globals->MAX_INACTIVE_TURNS != -1) {
        logger::write("QUITting Inactive Factions...");
        RemoveInactiveFactions();
    }

    logger::write("Running the Turn...");
    RunOrders();

    if (Globals->WORLD_EVENTS) {
        logger::write("Writing world events...");
        WriteWorldEvents();
    }

    logger::write("Writing the Report File...");
    WriteReport();
    logger::write("");

    battles.clear();

    EmptyHell();

    logger::write("Writing Playerinfo File...");
    WritePlayers();

    logger::write("Removing Dead Factions...");
    DeleteDeadFactions();

    logger::write("done");

    return(1);
}

void Game::RecordFact(FactBase* fact) {
    this->events->AddFact(fact);
}

void Game::WriteWorldEvents() {
    std::string text = this->events->Write(Globals->RULESET_NAME, MonthNames[this->month], this->year);
    if (text.empty() || text.length() == 0) return;

    this->write_times_article(text);
}

void Game::PreProcessTurn()
{
    month++;
    if (month>11) {
        month = 0;
        year++;
    }
    SetupUnitNums();
    for(const auto f : factions) f->DefaultOrders();

    for(const auto reg : regions) {
        if (Globals->WEATHER_EXISTS)
            reg->SetWeather(regions.GetWeather(reg, month));
        if (Globals->GATES_NOT_PERENNIAL)
            reg->SetGateStatus(month);
        reg->DefaultOrders();
    }
}

void Game::ClearOrders(Faction *f)
{
    for(const auto r : regions) {
        for(const auto o : r->objects) {
            for(const auto u : o->units) {
                if (u->faction == f) {
                    u->ClearOrders();
                }
            }
        }
    }
}

void Game::ReadOrders()
{
    for(const auto fac : factions) {
        if (!fac->is_npc) {
            std::string str = "orders.";
            str += std::to_string(fac->num);

            std::ifstream file(str, std::ios::in);
            if(file.is_open()) {
                ParseOrders(fac->num, file, nullptr);
                file.close();
            }
            DefaultWorkOrder();
        }
    }
}

void Game::MakeFactionReportLists()
{
    std::vector<Faction *> facs(factionseq, nullptr);

    for(const auto reg : regions) {
        // clear the temporary
        std::fill(facs.begin(), facs.end(), nullptr);

        for(const auto far : reg->farsees) {
            facs[far->faction->num] = far->faction;
        }
        for(const auto pass : reg->passers) {
            facs[pass->faction->num] = pass->faction;
        }
        for(const auto obj : reg->objects) {
            for(const auto unit : obj->units) {
                facs[unit->faction->num] = unit->faction;
            }
        }

        for(const auto& fac: facs) {
            if (fac) fac->present_regions.push_back(reg);
        }
    }
}

void Game::WriteReport()
{
    MakeFactionReportLists();
    CountAllSpecialists();

    size_t ** citems = nullptr;

    if (Globals->FACTION_STATISTICS) {
        citems = new size_t * [factionseq];
        for (int i = 0; i < factionseq; i++)
        {
            citems [i] = new size_t [NITEMS];
            for (int j = 0; j < NITEMS; j++)
            {
                citems [i][j] = 0;
            }
        }
        CountItems(citems);
    }

    for(const auto fac : factions) {
        std::string report_file = "report." + std::to_string(fac->num);
        std::string template_file = "template." + std::to_string(fac->num);

        if (!fac->is_npc || fac->gets_gm_report(this)) {
            // Generate the report in JSON format and then write it to whatever formats we want
            json json_report;
            bool show_region_depth = Globals->EASIER_UNDERWORLD
                && (Globals->UNDERWORLD_LEVELS + Globals->UNDERDEEP_LEVELS > 1);
            fac->build_json_report(json_report, this, citems);

            if (Globals->REPORT_FORMAT & GameDefs::REPORT_FORMAT_JSON) {
                std::ofstream jsonf(report_file + ".json", std::ios::out | std::ios::ate);
                if (jsonf.is_open()) {
                    jsonf << json_report.dump(2);
                }
            }

            if (Globals->REPORT_FORMAT & GameDefs::REPORT_FORMAT_TEXT) {
                TextReportGenerator text_report;
                std::ofstream f(report_file, std::ios::out | std::ios::ate);
                if (f.is_open()) {
                    text_report.output(f, json_report, show_region_depth);
                }
                if (!fac->is_npc && fac->temformat != TEMPLATE_OFF) {
                    // even factions which get a gm report do not get a template.
                    std::ofstream f(template_file, std::ios::out | std::ios::ate);
                    if (f.is_open()) {
                        text_report.output_template(f, json_report, fac->temformat, show_region_depth);
                    }
                }
            }
        }
        logger::dot();
    }

    // stop leaking memory
    if (Globals->FACTION_STATISTICS) {
        for (auto i = 0; i < factionseq; i++) delete [] citems [i];
        delete [] citems;
    }
}

void Game::DeleteDeadFactions()
{
    for(auto it = factions.begin(); it != factions.end();) {
        auto fac = *it;
        if (!fac->is_npc && !fac->exists) {
            it = factions.erase(it);
            for(const auto fac2 : factions) fac2->remove_attitude(fac->num);
            delete fac;
            continue;
        }
        ++it;
    }
}

Faction *Game::AddFaction(int noleader, ARegion *pStart)
{
    //
    // set up faction
    //
    Faction *temp = new Faction(factionseq);
    temp->set_address("NoAddress");
    temp->lastorders = TurnNumber();
    temp->startturn = TurnNumber();
    temp->pStartLoc = pStart;
    temp->pReg = pStart;
    temp->noStartLeader = noleader;

    if (!SetupFaction(temp)) {
        delete temp;
        return 0;
    }
    factions.push_back(temp);
    factionseq++;
    return temp;
}

void Game::ViewFactions()
{
    for(const auto f : factions) f->View();
}

void Game::SetupUnitSeq()
{
    int max = 0;
    for(const auto r : regions) {
        for(const auto o : r->objects) {
            for(const auto u : o->units) {
                if (u && u->num > max) max = u->num;
            }
        }
    }
    unitseq = max+1;
}

void Game::SetupUnitNums()
{
    if (ppUnits) delete[] ppUnits;

    SetupUnitSeq();

    maxppunits = unitseq+10000;

    ppUnits = new Unit *[maxppunits];

    unsigned int i;
    for (i = 0; i < maxppunits ; i++) ppUnits[i] = 0;

    for(const auto r : regions) {
        for(const auto o : r->objects) {
            for(const auto u : o->units) {
                i = u->num;
                if ((i > 0) && (i < maxppunits)) {
                    if (!ppUnits[i])
                        ppUnits[u->num] = u;
                    else {
                        logger::write("Error: Unit number " + std::to_string(i) + " multiply defined.");
                        if ((unitseq > 0) && (unitseq < maxppunits)) {
                            u->num = unitseq;
                            ppUnits[unitseq++] = u;
                        }
                    }
                } else {
                    logger::write("Error: Unit number " + std::to_string(i) + " out of range.");
                    if ((unitseq > 0) && (unitseq < maxppunits)) {
                        u->num = unitseq;
                        ppUnits[unitseq++] = u;
                    }
                }
            }
        }
    }
}

Unit *Game::GetNewUnit(Faction *fac, int an)
{
    unsigned int i;
    for (i = 1; i < unitseq; i++) {
        if (!ppUnits[i]) {
            Unit *pUnit = new Unit(i, fac, an);
            ppUnits[i] = pUnit;
            return(pUnit);
        }
    }

    Unit *pUnit = new Unit(unitseq, fac, an);
    ppUnits[unitseq] = pUnit;
    unitseq++;
    if (unitseq >= maxppunits) {
        Unit **temp = new Unit*[maxppunits+10000];
        memcpy(temp, ppUnits, maxppunits*sizeof(Unit *));
        maxppunits += 10000;
        delete[] ppUnits;
        ppUnits = temp;
    }

    return(pUnit);
}

Unit *Game::GetUnit(int num)
{
    if (num < 0 || (unsigned int)num >= maxppunits) return NULL;
    return(ppUnits[num]);
}


void Game::CountAllSpecialists()
{
    for(const auto f : factions) {
        f->nummages = 0;
        f->numqms = 0;
        f->numtacts = 0;
        f->numapprentices = 0;
    }
    for(const auto r : regions) {
        for(const auto o : r->objects) {
            for(const auto u : o->units) {
                if (u->type == U_MAGE) u->faction->nummages++;
                if (u->GetSkill(S_QUARTERMASTER)) u->faction->numqms++;
                if (u->GetSkill(S_TACTICS) == 5) u->faction->numtacts++;
                if (u->type == U_APPRENTICE) u->faction->numapprentices++;
            }
        }
    }
}

// LLS
void Game::UnitFactionMap()
{
    unsigned int i;
    Unit *u;

    logger::write("Opening units.txt");
    std::ofstream f("units.txt", std::ios::out|std::ios::ate);
    if (!f.is_open()) {
        logger::write("Couldn't open file!");
        return;
    }
    logger::write("Writing " + std::to_string(unitseq) + " units");
    for (i = 1; i < unitseq; i++) {
        u = GetUnit(i);
        if (!u) {
            logger::write("doesn't exist");
        } else {
            logger::write(std::to_string(i) + ":" + std::to_string(u->faction->num));
            f << i << ":" << u->faction->num << std::endl;
        }
    }
}

//The following function added by Creative PBM February 2000
void Game::RemoveInactiveFactions()
{
    if (Globals->MAX_INACTIVE_TURNS == -1) return;

    int cturn;
    cturn = TurnNumber();
    for(const auto fac : factions) {
        if ((cturn - fac->lastorders) >= Globals->MAX_INACTIVE_TURNS && !fac->is_npc) {
            fac->quit = QUIT_BY_GM;
        }
    }
}

int Game::CountMages(Faction *pFac)
{
    int i = 0;
    for(const auto r : regions) {
        for(const auto o : r->objects) {
            for(const auto u :o->units) {
                if (u->faction == pFac && u->type == U_MAGE) i++;
            }
        }
    }
    return(i);
}

int Game::CountQuarterMasters(Faction *pFac)
{
    int i = 0;
    for(const auto r : regions) {
        for(const auto o : r->objects) {
            for(const auto u : o->units) {
                if (u->faction == pFac && u->GetSkill(S_QUARTERMASTER)) i++;
            }
        }
    }
    return i;
}

int Game::CountTacticians(Faction *pFac)
{
    int i = 0;
    for(const auto r : regions) {
        for(const auto o : r->objects) {
            for(const auto u : o->units) {
                if (u->faction == pFac && u->GetSkill(S_TACTICS) == 5) i++;
            }
        }
    }
    return i;
}

int Game::CountApprentices(Faction *pFac)
{
    int i = 0;
    for(const auto r : regions) {
        for(const auto o : r->objects) {
            for(const auto u : o->units) {
                if (u->faction == pFac && u->type == U_APPRENTICE) i++;
            }
        }
    }
    return i;
}

int Game::AllowedMages(Faction *pFac)
{
    int points = pFac->type[F_MAGIC];

    if (points < 0) points = 0;
    if (points > allowedMagesSize - 1) points = allowedMagesSize - 1;

    return allowedMages[points];
}

int Game::AllowedQuarterMasters(Faction *pFac)
{
    int points = std::max(pFac->type[F_TRADE], pFac->type[F_MARTIAL]);

    if (points < 0) points = 0;
    if (points > allowedQuartermastersSize - 1)
        points = allowedQuartermastersSize - 1;

    return allowedQuartermasters[points];
}

int Game::AllowedTacticians(Faction *pFac)
{
    int points = std::max(pFac->type[F_WAR], pFac->type[F_MARTIAL]);

    if (points < 0) points = 0;
    if (points > allowedTacticiansSize - 1)
        points = allowedTacticiansSize - 1;

    return allowedTacticians[points];
}

int Game::AllowedApprentices(Faction *pFac)
{
    int points = pFac->type[F_MAGIC];

    if (points < 0) points = 0;
    if (points > allowedApprenticesSize - 1)
        points = allowedApprenticesSize - 1;

    return allowedApprentices[points];
}

int Game::AllowedTaxes(Faction *pFac)
{
    int points = std::max(pFac->type[F_WAR], pFac->type[F_MARTIAL]);

    if (points < 0) points = 0;
    if (points > allowedTaxesSize - 1) points = allowedTaxesSize - 1;

    return allowedTaxes[points];
}

int Game::AllowedTrades(Faction *pFac)
{
    int points = std::max(pFac->type[F_TRADE], pFac->type[F_MARTIAL]);

    if (points < 0) points = 0;
    if (points > allowedTradesSize - 1) points = allowedTradesSize - 1;

    return allowedTrades[points];
}

int Game::AllowedMartial(Faction *pFac)
{
    int points = pFac->type[F_MARTIAL];

    if (points < 0) points = 0;
    if (points > allowedMartialSize - 1) points = allowedMartialSize - 1;

    return allowedMartial[points];
}

bool Game::upgrade_major_version(int current_version)
{
    return false;
}

bool Game::upgrade_minor_version(int current_version)
{
    return true;
}

bool Game::upgrade_patch_level(int current_version)
{
    return true;
}

void Game::MidProcessUnitExtra(ARegion *r, Unit *u)
{
    if (Globals->CHECK_MONSTER_CONTROL_MID_TURN) MonsterCheck(r, u);
}

void Game::PostProcessUnitExtra(ARegion *r, Unit *u)
{
    if (!Globals->CHECK_MONSTER_CONTROL_MID_TURN) MonsterCheck(r, u);
}

void Game::MonsterCheck(ARegion *r, Unit *u)
{
    std::string tmp;
    int skill;
    int linked = 0;
    std::map< int, int > chances;

    if (u->type != U_WMON) {

        for(auto it = u->items.begin(); it != u->items.end();) {
            Item *i = *it;
            // Since items can be removed below, we will increment the iterator here
            it++;

            if (!i->num) continue;
            if (!ItemDefs[i->type].escape) continue;

            // Okay, check flat loss.
            if (ItemDefs[i->type].escape & ItemType::LOSS_CHANCE) {
                int losses = rng::calculate_losses(i->num, ItemDefs[i->type].esc_val);
                // LOSS_CHANCE and HAS_SKILL together mean the
                // decay rate only applies if you don't have
                // the required skill (this might get used if
                // you made illusions GIVEable, for example).
                if (ItemDefs[i->type].escape & ItemType::HAS_SKILL) {
                    skill = lookup_skill(ItemDefs[i->type].esc_skill);
                    if (u->GetSkill(skill) >= ItemDefs[i->type].esc_val) losses = 0;
                }
                if (losses) {
                    std::string temp = item_string(i->type, losses) + strings::plural(losses, " decay", " decays") +
                        " into nothingness.";
                    u->event(temp, "decay");
                    u->items.SetNum(i->type,i->num - losses);
                }
            } else if (ItemDefs[i->type].escape & ItemType::HAS_SKILL) {
                skill = lookup_skill(ItemDefs[i->type].esc_skill);
                if (u->GetSkill(skill) < ItemDefs[i->type].esc_val) {
                    if (Globals->WANDERING_MONSTERS_EXIST) {
                        Faction *mfac = GetFaction(factions, monfaction);
                        Unit *mon = GetNewUnit(mfac, 0);
                        auto monster = find_monster(ItemDefs[i->type].abr, (ItemDefs[i->type].type & IT_ILLUSION))->get();
                        mon->MakeWMon(monster.name.c_str(), i->type, i->num);
                        mon->MoveUnit(r->GetDummy());
                        // This will be zero unless these are set. (0 means
                        // full spoils)
                        mon->free = Globals->MONSTER_NO_SPOILS + Globals->MONSTER_SPOILS_RECOVERY;
                    }
                    u->event("Loses control of " + item_string(i->type, i->num) + ".", "escape");
                    u->items.SetNum(i->type, 0);
                }
            } else {
                // ESC_LEV_*
                skill = lookup_skill(ItemDefs[i->type].esc_skill);
                int level = u->GetSkill(skill);
                int chance;

                if (!level) chance = 10000;
                else {
                    int top = (ItemDefs[i->type].escape & ItemType::ESC_NUM_SQUARE) ? i->num * i->num : i->num;
                    int bottom = 0;
                    if (ItemDefs[i->type].escape & ItemType::ESC_LEV_LINEAR) bottom = level;
                    else if (ItemDefs[i->type].escape & ItemType::ESC_LEV_SQUARE) bottom = level * level;
                    else if (ItemDefs[i->type].escape & ItemType::ESC_LEV_CUBE) bottom = level * level * level;
                    else if (ItemDefs[i->type].escape & ItemType::ESC_LEV_QUAD) bottom = level * level * level * level;
                    else bottom = 1;
                    bottom = bottom * ItemDefs[i->type].esc_val;
                    chance = (top * 10000)/bottom;
                }

                if (ItemDefs[i->type].escape & ItemType::LOSE_LINKED) {
                    if (chance > chances[ItemDefs[i->type].type]) chances[ItemDefs[i->type].type] = chance;
                    linked = 1;
                } else if (chance > rng::get_random(10000)) {
                    if (Globals->WANDERING_MONSTERS_EXIST) {
                        Faction *mfac = GetFaction(factions, monfaction);
                        Unit *mon = GetNewUnit(mfac, 0);
                        auto monster = find_monster(ItemDefs[i->type].abr, (ItemDefs[i->type].type & IT_ILLUSION))->get();
                        mon->MakeWMon(monster.name.c_str(), i->type, i->num);
                        mon->MoveUnit(r->GetDummy());
                        // This will be zero unless these are set. (0 means full spoils)
                        mon->free = Globals->MONSTER_NO_SPOILS + Globals->MONSTER_SPOILS_RECOVERY;
                    }
                    u->event("Loses control of " + item_string(i->type, i->num) + ".", "escape");
                    u->items.SetNum(i->type, 0);
                }
            }
        }

        if (linked) {
            for (auto i = chances.begin(); i != chances.end(); i++) {
                // walk the chances list and for each chance, see if
                // escape happens and if escape happens then walk all items
                // and everything that is that type, get rid of it.
                if (i->second < rng::get_random(10000)) continue;
                for(auto iter = u->items.begin(); iter != u->items.end();) {
                    Item *it = *iter;
                    // Since items can be removed below, we will increment the iterator here
                    iter++;
                    if (ItemDefs[it->type].type == i->first) {
                        if (Globals->WANDERING_MONSTERS_EXIST) {
                            Faction *mfac = GetFaction(factions, monfaction);
                            Unit *mon = GetNewUnit(mfac, 0);
                            auto monster = find_monster(ItemDefs[it->type].abr, (ItemDefs[it->type].type & IT_ILLUSION))->get();
                            mon->MakeWMon(monster.name.c_str(), it->type, it->num);
                            mon->MoveUnit(r->GetDummy());
                            // This will be zero unless these are set. (0 means full spoils)
                            mon->free = Globals->MONSTER_NO_SPOILS + Globals->MONSTER_SPOILS_RECOVERY;
                        }
                        u->event("Loses control of " + item_string(it->type, it->num) + ".", "escape");
                        u->items.SetNum(it->type, 0);
                    }
                }
            }
        }

    }
}

void Game::CheckUnitMaintenance(int consume)
{
    CheckUnitMaintenanceItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE, consume);
    CheckUnitMaintenanceItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE, consume);
    CheckUnitMaintenanceItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE, consume);
    CheckUnitMaintenanceItem(I_FISH, Globals->UPKEEP_FOOD_VALUE, consume);
}

void Game::CheckFactionMaintenance(int con)
{
    CheckFactionMaintenanceItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE, con);
    CheckFactionMaintenanceItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE, con);
    CheckFactionMaintenanceItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE, con);
    CheckFactionMaintenanceItem(I_FISH, Globals->UPKEEP_FOOD_VALUE, con);
}

void Game::CheckAllyMaintenance()
{
    CheckAllyMaintenanceItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE);
    CheckAllyMaintenanceItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE);
    CheckAllyMaintenanceItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE);
    CheckAllyMaintenanceItem(I_FISH, Globals->UPKEEP_FOOD_VALUE);
}

void Game::CheckUnitHunger()
{
    CheckUnitHungerItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE);
    CheckUnitHungerItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE);
    CheckUnitHungerItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE);
    CheckUnitHungerItem(I_FISH, Globals->UPKEEP_FOOD_VALUE);
}

void Game::CheckFactionHunger()
{
    CheckFactionHungerItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE);
    CheckFactionHungerItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE);
    CheckFactionHungerItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE);
    CheckFactionHungerItem(I_FISH, Globals->UPKEEP_FOOD_VALUE);
}

void Game::CheckAllyHunger()
{
    CheckAllyHungerItem(I_FOOD, Globals->UPKEEP_FOOD_VALUE);
    CheckAllyHungerItem(I_GRAIN, Globals->UPKEEP_FOOD_VALUE);
    CheckAllyHungerItem(I_LIVESTOCK, Globals->UPKEEP_FOOD_VALUE);
    CheckAllyHungerItem(I_FISH, Globals->UPKEEP_FOOD_VALUE);
}

char Game::GetRChar(ARegion *r)
{
    int t;

    if (!r)
        return ' ';
    t = r->type;
    if (t < 0 || t > R_NUM) return '?';
    char c = TerrainDefs[r->type].marker;
    if (r->town) {
        c = (c - 'a') + 'A';
    }
    return c;
}

void Game::CreateNPCFactions()
{
    Faction *f;
    if (Globals->CITY_MONSTERS_EXIST) {
        f = new Faction(factionseq++);
        guardfaction = f->num;
        f->set_name("The Guardsmen");
        f->is_npc = true;
        f->lastorders = 0;
        factions.push_back(f);
    } else guardfaction = 0;
    // Only create the monster faction if wandering monsters or lair
    // monsters exist.
    if (Globals->LAIR_MONSTERS_EXIST || Globals->WANDERING_MONSTERS_EXIST) {
        f = new Faction(factionseq++);
        monfaction = f->num;
        f->set_name("Creatures");
        f->is_npc = true;
        f->lastorders = 0;
        factions.push_back(f);
    } else monfaction = 0;
}

void Game::CreateCityMon(ARegion *region, int percent, int needmage)
{
    int skilllevel;
    int AC = 0;
    int IV = 0;
    int num;
    if (region->type == R_NEXUS || region->IsStartingCity()) {
        skilllevel = TOWN_CITY + 1;
        if (Globals->SAFE_START_CITIES || (region->type == R_NEXUS))
            IV = 1;
        AC = 1;
        num = Globals->AMT_START_CITY_GUARDS;
    } else {
        skilllevel = region->town->TownType() + 1;
        num = Globals->CITY_GUARD * skilllevel;
    }
    num = num * percent / 100;
    Faction *fac = GetFaction(factions, guardfaction);
    Unit *u = GetNewUnit(fac);
    Unit *u2;

    if ((Globals->LEADERS_EXIST) || (region->type == R_NEXUS)) {
        /* standard Leader-type guards */
        u->SetMen(I_LEADERS,num);
        u->items.SetNum(I_SWORD,num);
        if (IV) u->items.SetNum(I_AMULETOFI,num);
        u->SetMoney(num * Globals->GUARD_MONEY);
        u->SetSkill(S_COMBAT,skilllevel);
        u->set_name("City Guard");
        u->type = U_GUARD;
        u->guard = GUARD_GUARD;
        u->reveal = REVEAL_FACTION;
    } else {
        /* non-leader guards */
        int n = 3 * num / 4;
        int plate = 0;
        if ((AC) && (Globals->START_CITY_GUARDS_PLATE)) plate = 1;
        u = MakeManUnit(fac, region->race, n, skilllevel, 1, plate, 0);
        if (IV) u->items.SetNum(I_AMULETOFI,num);
        u->SetMoney(num * Globals->GUARD_MONEY / 2);
        u->set_name("City Guard");
        u->type = U_GUARD;
        u->guard = GUARD_GUARD;
        u->reveal = REVEAL_FACTION;
        u2 = MakeManUnit(fac, region->race, n, skilllevel, 1, plate, 1);
        if (IV) u2->items.SetNum(I_AMULETOFI,num);
        u2->SetMoney(num * Globals->GUARD_MONEY / 2);
        u2->set_name("City Guard");
        u2->type = U_GUARD;
        u2->guard = GUARD_GUARD;
        u2->reveal = REVEAL_FACTION;
    }

    if (AC) {
        if (Globals->START_CITY_GUARDS_PLATE) {
            if (Globals->LEADERS_EXIST) u->items.SetNum(I_PLATEARMOR, num);
        }
        u->SetSkill(S_OBSERVATION,10);
        if (Globals->START_CITY_TACTICS)
            u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
    } else {
        u->SetSkill(S_OBSERVATION, skilllevel);
    }
    u->SetFlag(FLAG_HOLDING,1);
    u->MoveUnit(region->GetDummy());
    if ((!Globals->LEADERS_EXIST) && (region->type != R_NEXUS)) {
        u2->SetFlag(FLAG_HOLDING,1);
        u2->MoveUnit(region->GetDummy());
    }

    if (AC && Globals->START_CITY_MAGES && needmage) {
        u = GetNewUnit(fac);
        u->set_name("City Mage");
        u->type = U_GUARDMAGE;
        u->reveal = REVEAL_FACTION;
        u->SetMen(I_LEADERS,1);
        if (IV) u->items.SetNum(I_AMULETOFI,1);
        u->SetMoney(Globals->GUARD_MONEY);
        u->SetSkill(S_FORCE,Globals->START_CITY_MAGES);
        u->SetSkill(S_FIRE,Globals->START_CITY_MAGES);
        if (Globals->START_CITY_TACTICS) u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
        u->combat = S_FIRE;
        u->SetFlag(FLAG_BEHIND, 1);
        u->SetFlag(FLAG_HOLDING, 1);
        u->MoveUnit(region->GetDummy());
    }
}

void Game::AdjustCityMons(ARegion *r)
{
    int needguard = 1;
    int needmage = 1;
    for(const auto o : r->objects) {
        for(const auto u : o->units) {
            if (u->type == U_GUARD || u->type == U_GUARDMAGE) {
                AdjustCityMon(r, u);
                /* Don't create new city guards if we have some */
                needguard = 0;
                if (u->type == U_GUARDMAGE)
                    needmage = 0;
            }
            if (u->guard == GUARD_GUARD) needguard = 0;
        }
    }

    if (needguard && (rng::get_random(100) < Globals->GUARD_REGEN)) {
        CreateCityMon(r, 10, needmage);
    }
}

void Game::AdjustCityMon(ARegion *r, Unit *u)
{
    int towntype;
    int AC = 0;
    int men;
    int IV = 0;
    int mantype;
    int maxmen;
    int weapon = -1;
    int maxweapon = 0;
    int armor = -1;
    int maxarmor = 0;
    for (int i=0; i<NITEMS; i++) {
        int num = u->items.GetNum(i);
        if (num == 0) continue;
        if (ItemDefs[i].type & IT_MAN) mantype = i;
        if ((ItemDefs[i].type & IT_WEAPON)
            && (num > maxweapon)) {
            weapon = i;
            maxweapon = num;
        }
        if ((ItemDefs[i].type & IT_ARMOR)
            && (num > maxarmor)) {
            armor = i;
            maxarmor = num;
        }
    }
    int skill = S_COMBAT;

    if (weapon != -1) {
        auto weapon_def = find_weapon(ItemDefs[weapon].abr)->get();
        auto pS = FindSkill(weapon_def.baseSkill)->get();
        if (pS == FindSkill("XBOW")->get()) skill = S_CROSSBOW;
        if (pS == FindSkill("LBOW")->get()) skill = S_LONGBOW;
    }

    int sl = u->GetRealSkill(skill);

    if (r->type == R_NEXUS || r->IsStartingCity()) {
        towntype = TOWN_CITY;
        AC = 1;
        if (Globals->SAFE_START_CITIES || (r->type == R_NEXUS))
            IV = 1;
        if (u->type == U_GUARDMAGE) {
            men = 1;
        } else {
            maxmen = Globals->AMT_START_CITY_GUARDS;
            if ((!Globals->LEADERS_EXIST) && (r->type != R_NEXUS))
                maxmen = 3 * maxmen / 4;
            men = u->GetMen() + (Globals->AMT_START_CITY_GUARDS/10);
            if (men > maxmen)
                men = maxmen;
        }
    } else {
        towntype = r->town->TownType();
        maxmen = Globals->CITY_GUARD * (towntype+1);
        if (!Globals->LEADERS_EXIST) maxmen = 3 * maxmen / 4;
        men = u->GetMen() + (maxmen/10);
        if (men > maxmen)
            men = maxmen;
    }

    u->SetMen(mantype,men);
    if (IV) u->items.SetNum(I_AMULETOFI,men);

    if (u->type == U_GUARDMAGE) {
        if (Globals->START_CITY_TACTICS)
            u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
        u->SetSkill(S_FORCE, Globals->START_CITY_MAGES);
        u->SetSkill(S_FIRE, Globals->START_CITY_MAGES);
        u->combat = S_FIRE;
        u->SetFlag(FLAG_BEHIND, 1);
        u->SetMoney(Globals->GUARD_MONEY);
    } else {
        int money = men * (Globals->GUARD_MONEY * men / maxmen);
        u->SetMoney(money);
        u->SetSkill(skill, sl);
        if (AC) {
            u->SetSkill(S_OBSERVATION,10);
            if (Globals->START_CITY_TACTICS)
                u->SetSkill(S_TACTICS, Globals->START_CITY_TACTICS);
            if (Globals->START_CITY_GUARDS_PLATE)
                u->items.SetNum(armor,men);
        } else {
            u->SetSkill(S_OBSERVATION,towntype + 1);
        }
        if (weapon!= -1) {
            u->items.SetNum(weapon,men);
        }
    }
}

void Game::Equilibrate()
{
    logger::write("Initialising the economy");
    for (int a=0; a<25; a++) {
        logger::dot();
        ProcessMigration();
        for(const auto r : regions) {
            r->PostTurn();
        }
    }
    logger::write("");
}

void Game::write_times_article(std::string article)
{
    std::string fname;

    do { fname = "times." + std::to_string(rng::get_random(10000)); } while (std::filesystem::exists(fname));
    std::ofstream f(fname, std::ios::out | std::ios::trunc);
    if (f.is_open()) {
        f << indent::wrap(78,70,0) << article << '\n';
    }
}


void Game::CountItems(size_t ** citems)
{
    for(const auto fac : factions) {
        if (!fac->is_npc) {
            int i = fac->num - 1; // faction numbers are 1-based
            for (int j = 0; j < NITEMS; j++){
                citems[i][j] = CountItem (fac, j);
            }
        }
    }
}

int Game::CountItem (Faction *fac, int item)
{
    if (ItemDefs[item].type & IT_SHIP) return 0;

    size_t all = 0;
    for (const auto& r : fac->present_regions) {
        for(const auto obj : r->objects) {
            for(const auto unit : obj->units) {
                if (unit->faction == fac) all += unit->items.GetNum(item);
            }
        }
    }
    return all;
}

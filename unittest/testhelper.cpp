#include "game.h"
#include "testhelper.hpp"

static void seed_random() { seedrandom(0xdeadbeef); }

UnitTestHelper::UnitTestHelper() {
    // install our own random seed function
    game.init_random_seed = seed_random;

    // Set up the output streams to capture the output.
    cout_streambuf = cout.rdbuf();
    cout.rdbuf(cout_buffer.rdbuf());
}

UnitTestHelper::~UnitTestHelper() {
    // Restore the output streams.
    cout.rdbuf(cout_streambuf);
}

int UnitTestHelper::initialize_game() {
    return game.NewGame();
}

void UnitTestHelper::setup_turn() {
    game.PreProcessTurn();
}

void UnitTestHelper::setup_reports() {
    game.MakeFactionReportLists();
}

int UnitTestHelper::get_region_count() {
    return game.regions.Num();
}

Game& UnitTestHelper::game_object() {
    return game;
}

Faction *UnitTestHelper::create_faction(string name) {
    auto fac = game.AddFaction(0, nullptr);
    AString *tmp = new AString(name); // because the guts of SetName frees the string passed in. :/
    fac->SetName(tmp);
    return fac;
}

Unit *UnitTestHelper::get_first_unit(Faction *faction) {
    // This is pretty gross.. a faction should have a link to it's own units, but it doesn't.
    for (size_t i = 0; i < game.maxppunits; i++) {
        if (game.ppUnits[i] && game.ppUnits[i]->faction == faction) return game.ppUnits[i];
    }
    return nullptr;
}

void UnitTestHelper::create_fleet(ARegion *region, Unit *owner, int ship_type, int ship_count) {
    for (auto i = 0; i < ship_count; i++)
        game.CreateShip(region, owner, ship_type);
 
}

ARegion *UnitTestHelper::get_region(int x, int y, int z) {
    ARegionArray *level = game.regions.pRegionArrays[z];
    return level->GetRegion(x, y);
}

string UnitTestHelper::cout_data() {
    return cout_buffer.str();
}

void UnitTestHelper::parse_orders(int faction_id, istream& orders) {
    game.ParseOrders(faction_id, orders, nullptr);
}

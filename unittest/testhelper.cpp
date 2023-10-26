#include "game.h"
#include "testhelper.hpp"

UnitTestHelper::UnitTestHelper() {
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

string UnitTestHelper::cout_data() {
    return cout_buffer.str();
}


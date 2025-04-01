#include "../game.h"
#include "../gamedata.h"
#include "../rng.h"
#include "testhelper.hpp"

UnitTestHelper::UnitTestHelper() {
    rng::seed_random(0xdeadbeef); // Seed the random number generator with a fixed value for reproducibility.

    game.init_random_seed = []() { rng::seed_random(0xdeadbeef); };

    // Set up the output streams to capture the output.
    cout_streambuf = std::cout.rdbuf();
    std::cout.rdbuf(cout_buffer.rdbuf());
}

UnitTestHelper::~UnitTestHelper() {
    // Restore the output streams.
    std::cout.rdbuf(cout_streambuf);
}

int UnitTestHelper::initialize_game() {
    return game.NewGame();
}

void UnitTestHelper::setup_turn() {
    game.ModifyTablesPerRuleset();
    game.PreProcessTurn();
}

void UnitTestHelper::setup_reports() {
    game.MakeFactionReportLists();
}

int UnitTestHelper::get_region_count() {
    return game.regions.size();
}

Game& UnitTestHelper::game_object() {
    return game;
}

Faction *UnitTestHelper::create_faction(std::string name) {
    auto fac = game.AddFaction(0, nullptr);
    AString *tmp = new AString(name); // because the guts of SetName frees the string passed in. :/
    fac->SetName(tmp);
    return fac;
}

Faction *UnitTestHelper::get_faction(int id) {
    return GetFaction(game.factions, id);
}

Unit *UnitTestHelper::get_first_unit(Faction *faction) {
    // This is pretty gross.. a faction should have a link to it's own units, but it doesn't.
    for (size_t i = 0; i < game.maxppunits; i++) {
        if (game.ppUnits[i] && game.ppUnits[i]->faction == faction) return game.ppUnits[i];
    }
    return nullptr;
}

Unit *UnitTestHelper::create_unit(Faction *faction, ARegion *region) {
    Unit *temp2 = game.GetNewUnit(faction);
	temp2->SetMen(I_LEADERS, 1);
    temp2->MoveUnit(region->GetDummy());
    return temp2;
}

void UnitTestHelper::create_fleet(ARegion *region, Unit *owner, int ship_type, int ship_count) {
    for (auto i = 0; i < ship_count; i++)
        game.CreateShip(region, owner, ship_type);

}

void UnitTestHelper::create_building(ARegion *region, Unit *owner, int building_type) {
    Object * obj = new Object(region);
    obj->type = building_type;
    obj->num = region->buildingseq++;
    std::string name = "Building [" + std::to_string(obj->num) + "]";
    obj->name = new AString(name);
    region->objects.push_back(obj);
    ObjectType ob = ObjectDefs[building_type];
    if (ob.flags & ObjectType::SACRIFICE) {
        // set up the items sacrifice needed
        obj->incomplete = -(ob.sacrifice_amount);
    } else {
        obj->incomplete = -(ob.maxMaintenance);
    }
    if (owner) owner->MoveUnit(obj);
}

ARegion *UnitTestHelper::get_region(int x, int y, int z) {
    ARegionArray *level = game.regions.pRegionArrays[z];
    return level->GetRegion(x, y);
}

int UnitTestHelper::connected_distance(ARegion *reg1, ARegion *reg2, int penalty, int max) {
    return game.regions.get_connected_distance(reg1, reg2, penalty, max);
}

std::string UnitTestHelper::cout_data() {
    return cout_buffer.str();
}

void UnitTestHelper::parse_orders(int faction_id, std::istream& orders, OrdersCheck *check) {
    game.ParseOrders(faction_id, orders, check);
}

void UnitTestHelper::check_transport_orders() {
    game.CheckTransportOrders();
}

void UnitTestHelper::move_units() {
    game.RunMovementOrders();
}

void UnitTestHelper::run_enter() {
    game.RunEnterOrders(0);
}

void UnitTestHelper::run_destroy() {
    game.RunDestroyOrders();
}

void UnitTestHelper::run_sacrifice() {
    game.RunSacrificeOrders();
}

void UnitTestHelper::run_annihilation() {
    game.RunAnnihilateOrders();
}

void UnitTestHelper::transport_phase(TransportOrder::TransportPhase phase) {
    game.RunTransportPhase(phase);
}

void UnitTestHelper::collect_transported_goods() {
    game.CollectInterQMTransportItems();
}

void UnitTestHelper::run_month_orders() {
    // This will run all month long orders, which includes study, production, build, etc.
    game.RunMonthOrders();
}

void UnitTestHelper::run_buy_orders() {
    game.RunBuyOrders();
}

void UnitTestHelper::run_sell_orders() {
    game.RunSellOrders();
}

void UnitTestHelper::run_productions() {
    // This will run the production phase for all regions in the game.
    for (auto &region : game.regions) {
        game.RunProduceOrders(region); // This will run the produce orders for each region.
    }
}

void UnitTestHelper::activate_spell(int spell, SpellTestHelper helper) {
    // This is a bit of a hack, but it's the easiest way to get the game object to run the spell, especially since
    // C++ doesn't have actual introspection/reflection and the spell executors take different arguments.   The intent
    // is that you add the spell you want to test here as you need it by adding a case statement to call the appropriate
    // Run<Spell> function based on the skill associated with the spell.  This assumes that orders for the unit have
    // already been set up so that the unit is ready to cast the spell.
    switch(spell) {
        case S_CREATE_PHANTASMAL_BEASTS:
            game.RunPhanBeasts(helper.region, helper.unit);
            break;
        case S_TELEPORTATION:
            game.RunTeleport(helper.region, helper.object, helper.unit);
            break;
    }
}

void UnitTestHelper::enable(Type type, int id, bool enable) {
    switch(type) {
        case SKILL:
            if (enable) game.EnableSkill(id);
            else game.DisableSkill(id);
            break;
        case ITEM:
            if (enable) game.EnableItem(id);
            else game.DisableItem(id);
            break;
        case OBJECT:
            if (enable) game.EnableObject(id);
            else game.DisableObject(id);
            break;
        default:
            break;
    }
}

void UnitTestHelper::maintain_units() {
    game.AssessMaintenance();
}

void UnitTestHelper::set_ruleset_specific_data(const json &data) {
    game.rulesetSpecificData = data;
}

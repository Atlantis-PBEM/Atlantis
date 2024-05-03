#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "testhelper.hpp"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"Quartermaster"> quartermaster_suite = []
{
  using namespace ut;

  "Normal quartermaster function works"_test = []
  {
    UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *unit1 = helper.get_first_unit(faction);
    Unit *qm1 = helper.create_unit(faction, region);
    qm1->SetName(new AString("QM 1"));
    qm1->Study(S_QUARTERMASTER, 30);
    helper.create_building(region, qm1, O_CARAVANSERAI);
    Unit *qm2 = helper.create_unit(faction, region);
    qm2->SetName(new AString("QM 2"));
    qm2->Study(S_QUARTERMASTER, 30);
    helper.create_building(region, qm2, O_CARAVANSERAI);
    Unit *unit2 = helper.create_unit(faction, region);
    unit1->items.SetNum(I_STONE, 100);

    // We have two quartermasters, each with a caravanserai, and two units, so we will use transport to move 50
    // stone from unit1 to unit2.
    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "transport 3 50 stone\n";  // unit 1 -> QM 1
    ss << "unit 3\n";
    ss << "transport 4 50 stone\n"; // QM 1 -> QM 2
    ss << "unit 4\n";
    ss << "transport 5 50 stone\n"; // QM 2 -> unit 2
    helper.parse_orders(faction->num, ss);
    helper.check_transport_orders();

    helper.transport_phase(TransportOrder::SHIP_TO_QM);
    expect(unit1->items.GetNum(I_STONE) == 50_i);
    expect(qm1->items.GetNum(I_STONE) == 50_i);
    expect(qm2->items.GetNum(I_STONE) == 0_i);
    expect(unit2->items.GetNum(I_STONE) == 0_i);

    helper.transport_phase(TransportOrder::INTER_QM_TRANSPORT);
    expect(unit1->items.GetNum(I_STONE) == 50_i);
    expect(qm1->items.GetNum(I_STONE) == 0_i);
    expect(qm2->transport_items.GetNum(I_STONE) == 50_i);
    expect(qm2->items.GetNum(I_STONE) == 0_i);
    expect(unit2->items.GetNum(I_STONE) == 0_i);

    helper.collect_transported_goods();
    expect(unit1->items.GetNum(I_STONE) == 50_i);
    expect(qm1->items.GetNum(I_STONE) == 0_i);
    expect(qm2->transport_items.GetNum(I_STONE) == 0_i);
    expect(qm2->items.GetNum(I_STONE) == 50_i);
    expect(unit2->items.GetNum(I_STONE) == 0_i);

    helper.transport_phase(TransportOrder::DISTRIBUTE_FROM_QM);
    expect(unit1->items.GetNum(I_STONE) == 50_i);
    expect(qm1->items.GetNum(I_STONE) == 0_i);
    expect(qm2->items.GetNum(I_STONE) == 0_i);
    expect(unit2->items.GetNum(I_STONE) == 50_i);
  };

  "Quartermasters cannot chain with other quartermasters"_test = []
  {
        UnitTestHelper helper;
    helper.initialize_game();
    helper.setup_turn();

    string name("Test Faction");
    Faction *faction = helper.create_faction(name);
    ARegion *region = helper.get_region(0, 0, 0);
    Unit *unit1 = helper.get_first_unit(faction);
    Unit *qm1 = helper.create_unit(faction, region);
    qm1->SetName(new AString("QM 1"));
    qm1->Study(S_QUARTERMASTER, 30);
    helper.create_building(region, qm1, O_CARAVANSERAI);
    Unit *qm2 = helper.create_unit(faction, region);
    qm2->SetName(new AString("QM 2"));
    qm2->Study(S_QUARTERMASTER, 30);
    helper.create_building(region, qm2, O_CARAVANSERAI);
    Unit *qm3 = helper.create_unit(faction, region);
    qm3->SetName(new AString("QM 3"));
    qm3->Study(S_QUARTERMASTER, 30);
    helper.create_building(region, qm3, O_CARAVANSERAI);
    Unit *unit2 = helper.create_unit(faction, region);
    unit1->items.SetNum(I_STONE, 100);

    // We have three quartermasters, each with a caravanserai, and two units, and we will try to chain transport
    // through multiple quartermasters.  This should fail, as quartermasters cannot chain.
    stringstream ss;
    ss << "#atlantis 3\n";
    ss << "unit 2\n";
    ss << "transport 3 50 stone\n";  // unit 1 -> QM 1
    ss << "unit 3\n";
    ss << "transport 4 50 stone\n"; // QM 1 -> QM 2
    ss << "unit 4\n";
    ss << "transport 5 50 stone\n"; // QM 2 -> QM 3 -- should fail
    ss << "unit 5\n";
    ss << "transport 6 50 stone\n"; // QM 3 -> unit 2 -- should fail

    helper.parse_orders(faction->num, ss);
    helper.check_transport_orders();

    helper.transport_phase(TransportOrder::SHIP_TO_QM);
    expect(unit1->items.GetNum(I_STONE) == 50_i);
    expect(qm1->items.GetNum(I_STONE) == 50_i);
    expect(qm2->items.GetNum(I_STONE) == 0_i);
    expect(qm3->items.GetNum(I_STONE) == 0_i);
    expect(unit2->items.GetNum(I_STONE) == 0_i);

    helper.transport_phase(TransportOrder::INTER_QM_TRANSPORT);
    expect(unit1->items.GetNum(I_STONE) == 50_i);
    expect(qm1->items.GetNum(I_STONE) == 0_i);
    expect(qm2->transport_items.GetNum(I_STONE) == 50_i);
    expect(qm2->items.GetNum(I_STONE) == 0_i);
    expect(qm3->items.GetNum(I_STONE) == 0_i);
    expect(unit2->items.GetNum(I_STONE) == 0_i);
    expect(faction->errors.size() == 1);
    expect(faction->errors[0].message == "TRANSPORT: Unable to transport. Have 0 stone [STON].");
    expect(faction->errors[0].unit == qm2);

    helper.collect_transported_goods();
    expect(unit1->items.GetNum(I_STONE) == 50_i);
    expect(qm1->items.GetNum(I_STONE) == 0_i);
    expect(qm2->transport_items.GetNum(I_STONE) == 0_i);
    expect(qm2->items.GetNum(I_STONE) == 50_i);
    expect(qm3->items.GetNum(I_STONE) == 0_i);
    expect(unit2->items.GetNum(I_STONE) == 0_i);

    helper.transport_phase(TransportOrder::DISTRIBUTE_FROM_QM);
    expect(unit1->items.GetNum(I_STONE) == 50_i);
    expect(qm1->items.GetNum(I_STONE) == 0_i);
    expect(qm2->items.GetNum(I_STONE) == 50_i);
    expect(qm3->items.GetNum(I_STONE) == 0_i);
    expect(unit2->items.GetNum(I_STONE) == 0_i);
    expect(faction->errors[1].message == "TRANSPORT: Unable to transport. Have 0 stone [STON].");
    expect(faction->errors[1].unit == qm3);
  };
};

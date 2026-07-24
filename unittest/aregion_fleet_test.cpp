#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "unit.h"
#include "faction.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	Object *addObject(ARegion *reg, int num, int type)
	{
		Object *o = new Object(reg);
		o->num = num;
		o->type = type;
		reg->objects.Add(o);
		return o;
	}
}

ut::suite<"ARegion fleets"> aregion_fleet_suite = []
{
	using namespace ut;

	// CheckFleets removes a fleet with no capacity (FleetCapacity() < 1 sets bail). An
	// empty O_FLEET has no ships and therefore zero capacity, so it is culled. NOTE: the
	// fixture must set a land terrain type -- CheckFleets indexes TerrainDefs[type], and on
	// land it also forces alive = 1 so removal is driven solely by the capacity check.
	"CheckFleets removes an empty (zero-capacity) fleet"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN;
		Object *dummy = addObject(reg, 1, O_DUMMY);
		addObject(reg, 2, O_FLEET); // empty fleet: no ships -> capacity 0

		expect(fatal(reg->objects.Num() == 2_i));
		reg->CheckFleets();

		expect(reg->objects.Num() == 1_i) << "the empty fleet is culled";
		expect(reg->GetObject(1) == dummy) << "the non-fleet object survives";
		expect(reg->GetObject(2) == nullptr) << "the fleet is gone";
	};

	// A non-fleet object is never touched by CheckFleets.
	"CheckFleets leaves non-fleet objects alone"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN;
		addObject(reg, 1, O_DUMMY);
		addObject(reg, 2, O_TOWER);

		reg->CheckFleets();
		expect(reg->objects.Num() == 2_i);
	};

	// When an empty fleet carries units, CheckFleets evacuates them to the region's dummy
	// object before removing the fleet, so no unit is lost.
	"CheckFleets evacuates units from a culled fleet to the dummy"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN;
		Faction *fac = new Faction(1);
		Object *dummy = addObject(reg, 1, O_DUMMY);
		Object *fleet = addObject(reg, 2, O_FLEET);

		Unit *passenger = new Unit(100, fac, 0);
		passenger->MoveUnit(fleet);
		expect(fatal(fleet->units.Num() == 1_i));

		reg->CheckFleets();

		expect(reg->GetObject(2) == nullptr) << "empty fleet removed";
		expect(dummy->units.Num() == 1_i) << "passenger relocated to the dummy";
		expect(passenger->object == dummy);
	};
};

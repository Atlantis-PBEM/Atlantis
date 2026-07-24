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
	Object *addObject(ARegion *reg, int num)
	{
		Object *o = new Object(reg);
		o->num = num;
		o->type = O_DUMMY;
		reg->objects.Add(o);
		return o;
	}

	Unit *addUnit(Object *obj, int num, Faction *fac)
	{
		Unit *u = new Unit(num, fac, 0);
		u->MoveUnit(obj);
		return u;
	}
}

ut::suite<"ARegion lifecycle"> aregion_lifecycle_suite = []
{
	using namespace ut;

	// Kill moves a unit out of its object and into the region's hell list. With no other
	// unit of the same faction to inherit, the items simply travel with the corpse.
	// NOTE: Kill indexes TerrainDefs[type], so the fixture must set a real terrain type;
	// the default ARegion constructor leaves `type` uninitialized.
	"Kill moves a lone unit into hell"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN; // avoid TerrainDefs[garbage] and the ocean-drown branch
		Faction *fac = new Faction(1);
		Object *o = addObject(reg, 1);
		Unit *u = addUnit(o, 100, fac);

		reg->Kill(u);

		expect(o->units.Num() == 0_i) << "victim leaves its object";
		expect(reg->hell.Num() == 1_i) << "victim goes to hell";
	};

	// With a surviving same-faction unit present, Kill hands the victim's non-soldier
	// items to that heir before sending the corpse to hell.
	"Kill bequeaths non-soldier items to a same-faction heir"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN;
		Faction *fac = new Faction(1);
		Object *o = addObject(reg, 1);
		Unit *heir = addUnit(o, 100, fac);
		Unit *victim = addUnit(o, 101, fac);
		victim->items.SetNum(I_SILVER, 50);

		reg->Kill(victim);

		expect(heir->items.GetNum(I_SILVER) == 50_i) << "heir inherits the silver";
		expect(victim->items.GetNum(I_SILVER) == 0_i) << "victim's items are cleared";
		expect(reg->hell.Num() == 1_i);
	};

	// ClearHell empties the hell list (and deletes its occupants).
	"ClearHell empties the hell list"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN;
		Faction *fac = new Faction(1);
		Object *o = addObject(reg, 1);
		Unit *u = addUnit(o, 100, fac);
		reg->Kill(u);
		expect(fatal(reg->hell.Num() == 1_i));

		reg->ClearHell();
		expect(reg->hell.Num() == 0_i);
	};

	// HasItem is true iff some unit of the given faction holds a positive quantity.
	"HasItem detects a faction's item in the region"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *mine = new Faction(1);
		Faction *other = new Faction(2);
		Object *o = addObject(reg, 1);
		Unit *u = addUnit(o, 100, mine);

		expect(reg->HasItem(mine, I_SILVER) == 0_i) << "no silver yet";
		u->items.SetNum(I_SILVER, 5);
		expect(reg->HasItem(mine, I_SILVER) == 1_i);
		expect(reg->HasItem(other, I_SILVER) == 0_i) << "another faction owns none";
	};

	// GetLocation resolves a UnitId to a Location tying together region, object and unit.
	"GetLocation returns a Location for a resolvable unit id"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		Object *o = addObject(reg, 1);
		Unit *u = addUnit(o, 555, fac);

		UnitId *id = new UnitId();
		id->unitnum = 555;
		id->alias = 0;
		id->faction = 0;

		Location *loc = reg->GetLocation(id, 1);
		expect(fatal(loc != nullptr));
		expect(loc->region == reg);
		expect(loc->obj == o);
		expect(loc->unit == u);

		UnitId *missing = new UnitId();
		missing->unitnum = 999;
		expect(reg->GetLocation(missing, 1) == nullptr);
	};

	"SetWeather stores the weather value"_test = []
	{
		ARegion *reg = new ARegion();
		reg->SetWeather(W_BLIZZARD);
		expect(reg->weather == W_BLIZZARD);
	};

	// SetGateStatus always opens a Nexus gate. For a normal region under the unittest
	// ruleset (GATES_NOT_PERENNIAL = 0, START_GATES_OPEN = 0) the gate stays closed
	// regardless of month. START_GATES_OPEN being 0 also short-circuits IsStartingCity(),
	// so no world is required.
	"SetGateStatus opens Nexus gates and closes ordinary ones"_test = []
	{
		ARegion *nexus = new ARegion();
		nexus->type = R_NEXUS;
		nexus->gateopen = 0;
		nexus->SetGateStatus(0);
		expect(nexus->gateopen == 1_i) << "Nexus gate is always open";

		ARegion *plain = new ARegion();
		plain->type = R_PLAIN;
		plain->gate = 1;
		plain->gatemonth = 3;
		plain->gateopen = 1;
		plain->SetGateStatus(5);
		expect(plain->gateopen == 0_i) << "non-perennial gates stay closed off-month";
	};
};

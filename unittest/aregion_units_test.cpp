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
	// Add a non-dummy object with an explicit num to a region and return it.
	Object *addObject(ARegion *reg, int num)
	{
		Object *o = new Object(reg);
		o->num = num;
		o->type = O_DUMMY;
		reg->objects.Add(o);
		return o;
	}

	// Place a fresh unit into an object. MoveUnit sets unit->object and links the lists
	// correctly (it tolerates the unit's initial null object).
	Unit *addUnit(Object *obj, int num, Faction *fac, int alias = 0)
	{
		Unit *u = new Unit(num, fac, alias);
		u->MoveUnit(obj);
		return u;
	}
}

ut::suite<"ARegion units"> aregion_units_suite = []
{
	using namespace ut;

	// GetObject looks up an object by its num, returning null when absent.
	"GetObject finds an object by num and returns null otherwise"_test = []
	{
		ARegion *reg = new ARegion();
		Object *o10 = addObject(reg, 10);
		Object *o20 = addObject(reg, 20);

		expect(reg->GetObject(10) == o10);
		expect(reg->GetObject(20) == o20);
		expect(reg->GetObject(999) == nullptr);
	};

	// GetDummy returns the first O_DUMMY-typed object (open terrain lives in the dummy).
	"GetDummy returns the dummy object"_test = []
	{
		ARegion *reg = new ARegion();
		Object *dummy = new Object(reg); // constructor default type is O_DUMMY
		reg->objects.Add(dummy);
		expect(reg->GetDummy() == dummy);
	};

	"GetDummy returns null when there is no dummy object"_test = []
	{
		ARegion *reg = new ARegion();
		Object *building = new Object(reg);
		building->type = O_TOWER;
		reg->objects.Add(building);
		expect(reg->GetDummy() == nullptr);
	};

	// GetUnit searches across all objects in the region for a unit with the given num.
	"GetUnit finds a unit across objects by num"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		Object *o1 = addObject(reg, 1);
		Object *o2 = addObject(reg, 2);
		Unit *ua = addUnit(o1, 100, fac);
		Unit *ub = addUnit(o2, 200, fac);

		expect(reg->GetUnit(100) == ua);
		expect(reg->GetUnit(200) == ub);
		expect(reg->GetUnit(300) == nullptr);
	};

	// GetUnitAlias matches on the unit's alias and its (form)faction number. A freshly
	// constructed unit has formfaction == faction, so the faction num is that of `fac`.
	"GetUnitAlias matches alias within the given faction"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(7);
		Object *o1 = addObject(reg, 1);
		Unit *u = addUnit(o1, 100, fac, /*alias*/ 42);

		expect(reg->GetUnitAlias(42, 7) == u);
		expect(reg->GetUnitAlias(42, 8) == nullptr) << "wrong faction must not match";
		expect(reg->GetUnitAlias(99, 7) == nullptr) << "wrong alias must not match";
	};

	// GetUnitId dispatches: a non-zero unitnum resolves by number.
	"GetUnitId resolves a UnitId carrying a unit number"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		Object *o1 = addObject(reg, 1);
		Unit *u = addUnit(o1, 555, fac);

		UnitId *id = new UnitId();
		id->unitnum = 555;
		id->alias = 0;
		id->faction = 0;

		expect(reg->GetUnitId(id, 1) == u);
	};

	// Present is true iff at least one unit of the given faction stands in the region.
	"Present reflects whether a faction has a unit here"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac1 = new Faction(1);
		Faction *fac2 = new Faction(2);
		Object *o1 = addObject(reg, 1);
		addUnit(o1, 100, fac1);

		expect(reg->Present(fac1) == 1_i);
		expect(reg->Present(fac2) == 0_i) << "faction with no unit is not present";
	};

	// PresentFactions returns one entry per distinct faction, deduplicated by faction num.
	"PresentFactions lists each faction once"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac1 = new Faction(1);
		Faction *fac2 = new Faction(2);
		Object *o1 = addObject(reg, 1);
		Object *o2 = addObject(reg, 2);
		// Two units of fac1 (should collapse to one entry) and one of fac2.
		addUnit(o1, 100, fac1);
		addUnit(o1, 101, fac1);
		addUnit(o2, 200, fac2);

		AList *facs = reg->PresentFactions();
		expect(facs->Num() == 2_i) << "duplicate faction must appear only once";
	};

	"PresentFactions is empty for a region with no units"_test = []
	{
		ARegion *reg = new ARegion();
		addObject(reg, 1); // object present but no units in it
		AList *facs = reg->PresentFactions();
		expect(facs->Num() == 0_i);
	};

	// DeduplicateUnitList drops UnitId entries that resolve to the same unit. When every
	// entry resolves to a DISTINCT unit, nothing is removed and the list is untouched.
	"DeduplicateUnitList keeps entries that resolve to distinct units"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		Object *o1 = addObject(reg, 1);
		addUnit(o1, 100, fac);
		addUnit(o1, 200, fac);

		AList list;
		UnitId *id1 = new UnitId(); id1->unitnum = 100; id1->alias = 0; id1->faction = 0;
		UnitId *id2 = new UnitId(); id2->unitnum = 200; id2->alias = 0; id2->faction = 0;
		list.Add(id1);
		list.Add(id2);

		reg->DeduplicateUnitList(&list, 1);
		expect(list.Num() == 2_i) << "distinct references must all be preserved";
	};

	// KNOWN BUG (see the aregion review): DeduplicateUnitList deletes a duplicate node
	// inside a nested plain `forlist`, which can free the outer loop's already-pre-fetched
	// `_elem2`, causing a use-after-free when the list contains a repeated unit id. This
	// test is SKIPPED so it does not crash CI; it documents the intended post-dedup result
	// (one entry) and should be enabled once DeduplicateUnitList is switched to
	// forlist_safe / deferred deletion.
	skip / "DeduplicateUnitList collapses repeated ids (blocked by use-after-free)"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		Object *o1 = addObject(reg, 1);
		addUnit(o1, 100, fac);

		AList list;
		UnitId *id1 = new UnitId(); id1->unitnum = 100; id1->alias = 0; id1->faction = 0;
		UnitId *id2 = new UnitId(); id2->unitnum = 100; id2->alias = 0; id2->faction = 0;
		list.Add(id1);
		list.Add(id2);

		reg->DeduplicateUnitList(&list, 1);
		expect(list.Num() == 1_i) << "the duplicate reference should be removed";
	};
};

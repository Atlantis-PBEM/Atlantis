#include "external/boost/ut.hpp"

#include <cstdio>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "unit.h"
#include "faction.h"
#include "fileio.h"
#include "gameio.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	Unit *placeUnit(ARegion *reg, int num, Faction *fac)
	{
		Object *o = new Object(reg);
		o->type = O_DUMMY;
		reg->objects.Add(o);
		Unit *u = new Unit(num, fac, 0);
		u->MoveUnit(o);
		return u;
	}

	// Populate every field ARegion::Writeout persists, so a save/load round-trip is
	// deterministic (the default ctor leaves the economy fields uninitialized).
	ARegion *initRegion(int num, int x, int y, int z, char const *name)
	{
		ARegion *r = new ARegion();
		r->SetName(name);
		r->num = num;
		r->type = R_PLAIN;
		r->buildingseq = 1;
		r->gate = 0;
		r->race = -1;
		r->population = 0;
		r->basepopulation = 0;
		r->wages = 0;
		r->maxwages = 0;
		r->wealth = 0;
		r->elevation = 0;
		r->humidity = 0;
		r->temperature = 0;
		r->vegetation = 0;
		r->culture = 0;
		r->habitat = 0;
		r->development = 0;
		r->maxdevelopment = 0;
		r->town = 0;
		r->SetLoc(x, y, z);
		r->visited = 0;
		return r;
	}
}

ut::suite<"ARegion region list"> aregion_regionlist_suite = []
{
	using namespace ut;

	// ARegionList is an AList of ARegion; GetRegion(n) is a linear search by region num.
	"ARegionList::GetRegion(n) finds a region by num"_test = []
	{
		ARegionList *regs = new ARegionList();
		ARegion *r1 = new ARegion(); r1->num = 5;
		ARegion *r2 = new ARegion(); r2->num = 9;
		regs->Add(r1);
		regs->Add(r2);

		expect(regs->GetRegion(5) == r1);
		expect(regs->GetRegion(9) == r2);
		expect(regs->GetRegion(123) == nullptr);
	};

	// CreateLevels allocates the level array; GetRegionArray returns the stored pointer.
	"CreateLevels + GetRegionArray round-trips a level array"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		expect(regs->numLevels == 2_i);

		ARegionArray *arr0 = new ARegionArray(4, 4);
		ARegionArray *arr1 = new ARegionArray(4, 4);
		regs->pRegionArrays[0] = arr0;
		regs->pRegionArrays[1] = arr1;

		expect(regs->GetRegionArray(0) == arr0);
		expect(regs->GetRegionArray(1) == arr1);
	};

	// GetRegion(x,y,z) returns NULL when the level index is out of range.
	"ARegionList::GetRegion(x,y,z) is null for an out-of-range level"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(1);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);

		expect(regs->GetRegion(0, 0, 1) == nullptr) << "z == numLevels is out of range";
		expect(regs->GetRegion(0, 0, 5) == nullptr);
	};

	// GetRegion(x,y,z) locates a region placed in the level's array, wrapping coordinates
	// modulo the array dimensions.
	"ARegionList::GetRegion(x,y,z) resolves via the level array with wrap"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(1);
		ARegionArray *arr = new ARegionArray(4, 4);
		regs->pRegionArrays[0] = arr;

		ARegion *reg = new ARegion();
		arr->SetRegion(2, 0, reg); // even-parity cell

		expect(regs->GetRegion(2, 0, 0) == reg);
		// x wraps modulo 4: -2 -> 2, so it maps to the same cell.
		expect(regs->GetRegion(-2, 0, 0) == reg);
	};

	// ARegionArray stores regions only on cells where (x + y) is even; odd-parity cells
	// are always empty by construction. This is the hex-offset packing the whole map
	// relies on, so it is worth pinning explicitly.
	"ARegionArray SetRegion/GetRegion round-trips on even-parity cells"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();

		arr->SetRegion(0, 0, a); // 0+0 even
		arr->SetRegion(2, 2, b); // 2+2 even

		expect(arr->GetRegion(0, 0) == a);
		expect(arr->GetRegion(2, 2) == b);
	};

	"ARegionArray GetRegion returns null on odd-parity cells"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		// (1 + 0) is odd: no region can live here regardless of what was stored.
		expect(arr->GetRegion(1, 0) == nullptr);
		expect(arr->GetRegion(0, 1) == nullptr);
	};

	// GetRegion wraps coordinates toroidally before indexing.
	"ARegionArray GetRegion wraps out-of-bounds coordinates"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		ARegion *r = new ARegion();
		arr->SetRegion(2, 0, r);

		expect(arr->GetRegion(-2, 0) == r) << "-2 wraps to 2";
		expect(arr->GetRegion(6, 0) == r)  << "6 wraps to 2";
	};

	// SetName stores the name; passing null clears it back to empty.
	"ARegionArray SetName sets and clears the name"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		arr->SetName("Surface");
		expect(fatal(arr->strName != nullptr));
		std::string current = arr->strName->Str();
		std::string expected = "Surface";
		expect(eq(current, expected));

		arr->SetName(nullptr);
		expect(arr->strName == nullptr);
	};

	// ARegionFlatArray is a plain indexed array with no parity/wrap rules.
	"ARegionFlatArray SetRegion/GetRegion round-trips by index"_test = []
	{
		ARegionFlatArray *arr = new ARegionFlatArray(3);
		ARegion *r0 = new ARegion();
		ARegion *r2 = new ARegion();
		arr->SetRegion(0, r0);
		arr->SetRegion(2, r2);

		expect(arr->GetRegion(0) == r0);
		expect(arr->GetRegion(2) == r2);
	};

	// FindUnit scans every region -> object -> unit for a unit number, returning a Location.
	"ARegionList::FindUnit locates a unit across regions"_test = []
	{
		ARegionList *regs = new ARegionList();
		Faction *fac = new Faction(1);
		ARegion *r1 = new ARegion(); r1->num = 1;
		ARegion *r2 = new ARegion(); r2->num = 2;
		regs->Add(r1);
		regs->Add(r2);
		Unit *target = placeUnit(r2, 777, fac);

		Location *loc = regs->FindUnit(777);
		expect(fatal(loc != nullptr));
		expect(loc->unit == target);
		expect(loc->region == r2);

		expect(regs->FindUnit(999) == nullptr) << "unknown unit -> null";
	};

	// ARegionList::GetUnitId checks the current region first, then falls back to a global
	// FindUnit when the id carries a unit number.
	"ARegionList::GetUnitId prefers the current region then falls back"_test = []
	{
		ARegionList *regs = new ARegionList();
		Faction *fac = new Faction(1);
		ARegion *here = new ARegion(); here->num = 1;
		ARegion *there = new ARegion(); there->num = 2;
		regs->Add(here);
		regs->Add(there);
		Unit *local = placeUnit(here, 100, fac);
		Unit *remote = placeUnit(there, 200, fac);

		UnitId hereId; hereId.unitnum = 100; hereId.alias = 0; hereId.faction = 0;
		Location *l1 = regs->GetUnitId(&hereId, 1, here);
		expect(fatal(l1 != nullptr));
		expect(l1->unit == local) << "resolved within the current region";

		UnitId remoteId; remoteId.unitnum = 200; remoteId.alias = 0; remoteId.faction = 0;
		Location *l2 = regs->GetUnitId(&remoteId, 1, here);
		expect(fatal(l2 != nullptr));
		expect(l2->unit == remote) << "fell back to a global lookup";
	};

	// FindGate(x != -1) is a linear lookup by gate number. (The x == -1 form picks a random
	// gate and is intentionally not covered here.)
	"ARegionList::FindGate finds a region by its gate number"_test = []
	{
		ARegionList *regs = new ARegionList();
		ARegion *r1 = new ARegion(); r1->num = 1; r1->gate = 5;
		ARegion *r2 = new ARegion(); r2->num = 2; r2->gate = 9;
		regs->Add(r1);
		regs->Add(r2);

		expect(regs->FindGate(5) == r1);
		expect(regs->FindGate(9) == r2);
		expect(regs->FindGate(3) == nullptr) << "no region carries gate 3";
	};

	// GetPlanarDistance (non-icosahedral world) is a wrap-aware grid distance on the surface
	// level. Level scaling is 1 in the unittest ruleset, so region coords map straight
	// through. A Nexus endpoint short-circuits to the "unreachable" sentinel.
	"ARegionList::GetPlanarDistance measures surface grid distance"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2); // index 0 = nexus, index 1 = surface (LEVEL_SURFACE)
		regs->pRegionArrays[0] = new ARegionArray(2, 2);
		regs->pRegionArrays[1] = new ARegionArray(10, 10);

		ARegion *a = new ARegion(); a->SetLoc(0, 0, ARegionArray::LEVEL_SURFACE);
		ARegion *b = new ARegion(); b->SetLoc(2, 0, ARegionArray::LEVEL_SURFACE);
		regs->Add(a);
		regs->Add(b);

		// dx = 2, dy = 0; wrap alternatives (10-away) are larger, so distance is 2.
		expect(regs->GetPlanarDistance(a, b, 0, -1) == 2_i);

		// A Nexus endpoint is unreachable by planar distance.
		ARegion *nexus = new ARegion();
		nexus->SetLoc(0, 0, ARegionArray::LEVEL_NEXUS);
		expect(regs->GetPlanarDistance(nexus, b, 0, -1) == 10000000_i);
	};

	// The x == -1 form of FindGate picks a random region that has a gate. (Seeded for
	// determinism; we assert the invariant rather than which one.)
	"ARegionList::FindGate(-1) returns a random gated region"_test = []
	{
		seedrandom(1);
		ARegionList *regs = new ARegionList();
		ARegion *r1 = new ARegion(); r1->num = 1; r1->gate = 5;
		ARegion *r2 = new ARegion(); r2->num = 2; r2->gate = 9;
		ARegion *nogate = new ARegion(); nogate->num = 3; nogate->gate = 0;
		regs->Add(r1); regs->Add(r2); regs->Add(nogate);

		ARegion *found = regs->FindGate(-1);
		expect(fatal(found != nullptr)) << "a gated region is chosen";
		expect(that % found->gate > 0) << "the chosen region actually has a gate";
	};

	// The icosahedral branch of GetPlanarDistance does a neighbor BFS instead of the flat
	// grid formula. (ICOSAHEDRAL_WORLD is off in the unittest ruleset; flip it.) Level scale
	// is 1 here, so region coords map straight through.
	"ARegionList::GetPlanarDistance BFS on an icosahedral world"_test = []
	{
		int saved = Globals->ICOSAHEDRAL_WORLD;
		Globals->ICOSAHEDRAL_WORLD = 1;

		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		ARegionArray *surf = new ARegionArray(4, 4);
		regs->pRegionArrays[1] = surf;

		ARegion *a = new ARegion(); a->SetLoc(0, 0, ARegionArray::LEVEL_SURFACE);
		ARegion *b = new ARegion(); b->SetLoc(2, 0, ARegionArray::LEVEL_SURFACE);
		surf->SetRegion(0, 0, a);
		surf->SetRegion(2, 0, b);
		a->neighbors[D_SOUTHEAST] = b;
		b->neighbors[D_NORTHWEST] = a;
		regs->Add(a);
		regs->Add(b);

		expect(regs->GetPlanarDistance(a, b, 0, -1) == 1_i) << "adjacent hexes are distance 1";

		Globals->ICOSAHEDRAL_WORLD = saved;
	};

	// With an abyss level configured, teleporting into or out of it is forbidden -- the
	// distance is the "unreachable" sentinel. The abyss level index is
	// UNDERWORLD_LEVELS + UNDERDEEP_LEVELS + 2 (== 2 in the unittest ruleset).
	"ARegionList::GetPlanarDistance forbids crossing the abyss"_test = []
	{
		int saved = Globals->ABYSS_LEVEL;
		Globals->ABYSS_LEVEL = 1;

		ARegionList *regs = new ARegionList();
		ARegion *deep = new ARegion(); deep->SetLoc(0, 0, 2); // the abyss level
		ARegion *surf = new ARegion(); surf->SetLoc(2, 0, ARegionArray::LEVEL_SURFACE);

		expect(regs->GetPlanarDistance(deep, surf, 0, -1) == 10000000_i)
			<< "cannot path into or out of the abyss";

		Globals->ABYSS_LEVEL = saved;
	};

	// FindConnectedRegions is the BFS frontier step: for the source region r it links each
	// undiscovered (distance == -1) neighbor onto the r->next chain, stamps distance+1, and
	// returns the new tail. Neighbors already discovered (distance != -1) and null slots are
	// skipped. NOTE: distance/next are not initialized by the ARegion ctor, so the test sets
	// them explicitly, exactly as the real BFS seed does before the first call.
	"FindConnectedRegions links undiscovered neighbors onto the frontier"_test = []
	{
		ARegionList *regs = new ARegionList();
		ARegion *r  = new ARegion(); r->num = 0;  r->distance = 0;  r->next = 0;
		ARegion *n1 = new ARegion(); n1->num = 1; n1->distance = -1;
		ARegion *n2 = new ARegion(); n2->num = 2; n2->distance = -1;
		ARegion *seen = new ARegion(); seen->num = 3; seen->distance = 5;

		// Direction order is N(0), NE(1), SE(2), S(3), SW(4), NW(5): n1 links before n2, and
		// the already-seen NE neighbor is skipped.
		r->neighbors[D_NORTH]     = n1;
		r->neighbors[D_NORTHEAST] = seen;
		r->neighbors[D_SOUTH]     = n2;

		ARegion *tail = regs->FindConnectedRegions(r, r, 0);

		expect(r->next == n1)  << "first undiscovered neighbor linked to the source";
		expect(n1->next == n2) << "frontier chains in direction order";
		expect(n1->distance == 1_i);
		expect(n2->distance == 1_i);
		expect(seen->distance == 5_i) << "already-discovered neighbor is left untouched";
		expect(tail == n2) << "returns the new tail of the frontier";
	};

	// With shaft = 1, FindConnectedRegions also follows objects with an inner link, resolving
	// the inner region by number through the list. With shaft = 0 that traversal is skipped.
	"FindConnectedRegions follows shafts only when asked"_test = []
	{
		ARegionList *regs = new ARegionList();
		ARegion *r = new ARegion(); r->num = 0; r->distance = 0; r->next = 0;
		ARegion *deep = new ARegion(); deep->num = 42; deep->distance = -1;
		regs->Add(r);
		regs->Add(deep); // must be in the list so GetRegion(42) resolves it

		Object *shaft = new Object(r);
		shaft->inner = 42; // links down to region 42
		r->objects.Add(shaft);

		// shaft = 0: the inner region is not touched.
		ARegion *tail0 = regs->FindConnectedRegions(r, r, 0);
		expect(deep->distance == -1_i) << "shaft not followed when shaft == 0";
		expect(tail0 == r) << "no neighbors, tail unchanged";

		// shaft = 1: the inner region joins the frontier.
		ARegion *tail1 = regs->FindConnectedRegions(r, r, 1);
		expect(r->next == deep);
		expect(deep->distance == 1_i);
		expect(tail1 == deep);
	};

	// WriteRegions / ReadRegions are a positional save/load pair for the whole list: counts,
	// per-level array metadata, every region (via ARegion::Writeout), then the neighbor
	// adjacency by region number. This round-trips a two-region world through a temp file.
	// NOTE: regions are indexed into a flat array by num during load, so nums must be the
	// dense range 0..N-1.
	"WriteRegions/ReadRegions round-trips the region list"_test = []
	{
		const char *scratch = "aregion_regionlist.tmp";
		std::remove(scratch);

		ARegionList *src = new ARegionList();
		src->CreateLevels(1);
		ARegionArray *lvl = new ARegionArray(4, 4);
		lvl->levelType = ARegionArray::LEVEL_SURFACE;
		lvl->SetName("Surface");
		src->pRegionArrays[0] = lvl;
		src->numberofgates = 2;

		ARegion *a = initRegion(0, 0, 0, 0, "Alpha");
		ARegion *b = initRegion(1, 2, 0, 0, "Beta");
		a->neighbors[D_NORTH] = b; // adjacency to be restored by num on load
		b->neighbors[D_SOUTH] = a;
		src->Add(a);
		src->Add(b);

		Aoutfile out;
		expect(fatal(out.OpenByName(scratch) == 0_i));
		src->WriteRegions(&out);
		out.Close();

		ARegionList *dst = new ARegionList();
		AList factions; // no objects reference factions, so an empty list suffices
		Ainfile in;
		expect(fatal(in.OpenByName(scratch) == 0_i));
		dst->ReadRegions(&in, &factions, CURRENT_ATL_VER);
		in.Close();

		expect(dst->Num() == 2_i);
		expect(dst->numLevels == 1_i);
		expect(dst->numberofgates == 2_i);

		ARegionArray *lvl2 = dst->GetRegionArray(0);
		expect(fatal(lvl2 != nullptr));
		expect(lvl2->x == 4_i);
		expect(lvl2->y == 4_i);
		expect(lvl2->levelType == ARegionArray::LEVEL_SURFACE);
		expect(fatal(lvl2->strName != nullptr));
		expect(eq(std::string(lvl2->strName->Str()), std::string("Surface")));

		ARegion *a2 = dst->GetRegion(0);
		ARegion *b2 = dst->GetRegion(1);
		expect(fatal(a2 != nullptr));
		expect(fatal(b2 != nullptr));
		expect(eq(std::string(a2->name->Str()), std::string("Alpha")));
		expect(eq(std::string(b2->name->Str()), std::string("Beta")));

		// Neighbor adjacency restored by region number.
		expect(a2->neighbors[D_NORTH] == b2) << "Alpha's north neighbor is Beta";
		expect(b2->neighbors[D_SOUTH] == a2) << "Beta's south neighbor is Alpha";
		expect(a2->neighbors[D_SOUTH] == nullptr) << "unset directions stay null";

		// Regions were also placed into the level array at their own coordinates.
		expect(lvl2->GetRegion(0, 0) == a2);
		expect(lvl2->GetRegion(2, 0) == b2);

		std::remove(scratch);
	};
};

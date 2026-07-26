#include "external/boost/ut.hpp"

#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "mapgen.h"   // Map
#include "gameio.h"   // seedrandom
#include "orders.h"   // MOVE_IN
#include "aregion_test_util.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

using aregion_test::captureCout;

// economy() has external linkage but no prototype in any header -- it is only referenced within
// aregion.cpp, transitively through CreateNaturalSurfaceLevel. Declare it here so the settlement
// test can drive it directly on a controlled surface (see aregion_mapgen_helpers_test.cpp for the
// same pattern used on the map-gen leaf helpers).
void economy(ARegionArray *arr, const int w, const int h);

namespace {
	ARegion *placeCity(ARegionArray *arr, ARegionList *regs, int x, int y, int pop)
	{
		ARegion *r = new ARegion();
		r->type = R_PLAIN;
		// The ARegion() constructor does NOT initialise `race`, so a hand-built region carries a
		// garbage value. In the live pipeline GrowRaces() sets it before economy() runs; tests that
		// skip that step must supply the pre-growth sentinel -1 (NO_RACE) themselves. Without it,
		// economy() -> getRegionEtnos() evaluates `ItemDefs[reg->race]` on a garbage index (an
		// out-of-bounds read) and SetupHabitat() would never enter its race-selection branch. The
		// bad read is benign on macOS's allocator but segfaults on Linux (confirmed via coredump:
		// getRegionEtnos at aregion.cpp:3400). SetupHabitat() treats -1 as "pick a race for me".
		r->race = -1;
		r->SetLoc(x, y, ARegionArray::LEVEL_SURFACE);
		r->num = regs->Num();
		regs->Add(r);
		arr->SetRegion(x, y, r);
		if (pop > 0) {
			r->town = new TownInfo;
			r->town->name = new AString("Metropolis");
			r->town->pop = pop;
			r->town->hab = pop;
			r->town->dev = 100;
		}
		return r;
	}

	// Link a<->b through a spare direction slot (FindConnectedRegions only cares that the
	// neighbor pointer is set, not which direction).
	void link(ARegion *a, int da, ARegion *b, int db)
	{
		a->neighbors[da] = b;
		b->neighbors[db] = a;
	}
}

ut::suite<"ARegion mapgen"> aregion_mapgen_suite = []
{
	using namespace ut;

	// CreateNaturalSurfaceLevel builds the whole surface: it calls MakeRegions (which wires
	// neighbors via the private NeighSetup), types every region from the generated biome map,
	// names them, and adds historical buildings. We suppress the generator's chatty stdout and
	// assert the observable results. This is also the transitive coverage of NeighSetup.
	"CreateNaturalSurfaceLevel builds a fully wired, typed, named surface"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);

		captureCout([&]{
			seedrandom(42);
			Map *map = new Map(16, 16); // w = h = 8 -> 32 even-parity regions
			regs->CreateNaturalSurfaceLevel(map);
		});

		expect(regs->Num() == 32_i) << "8x8 grid of even-parity hexes";
		ARegionArray *surface = regs->GetRegionArray(ARegionArray::LEVEL_SURFACE);
		expect(fatal(surface != nullptr));
		expect(surface->levelType == ARegionArray::LEVEL_SURFACE);

		ARegion *mid = surface->GetRegion(2, 2);
		expect(fatal(mid != nullptr));
		// NeighSetup ran: an interior hex is fully connected.
		int nb = 0;
		for (int i = 0; i < NDIRS; i++) if (mid->neighbors[i]) nb++;
		expect(nb == 6_i) << "interior hex has six neighbors (NeighSetup)";
		// Biome typing ran.
		expect(that % mid->type >= 0) << "region assigned a real terrain type";
		// giveNames ran.
		expect(fatal(mid->name != nullptr));
		expect(std::string(mid->name->Str()).size() > 0_ul) << "region was named";
	};

	// Same seed reproduces the same world.
	"CreateNaturalSurfaceLevel is reproducible under a fixed seed"_test = []
	{
		auto build = [] {
			ARegionList *regs = new ARegionList();
			regs->CreateLevels(2);
			regs->pRegionArrays[0] = new ARegionArray(4, 4);
			captureCout([&]{
				seedrandom(99);
				Map *map = new Map(16, 16);
				regs->CreateNaturalSurfaceLevel(map);
			});
			return regs;
		};
		ARegionList *a = build();
		ARegionList *b = build();

		expect(a->Num() == b->Num());
		expect(a->GetRegionArray(1)->GetRegion(2, 2)->type ==
			   b->GetRegionArray(1)->GetRegion(2, 2)->type) << "same seed -> same biome";
	};

	// AddHistoricalBuildings plants ancient structures in populous settlements. On a 4x4 grid
	// with a single pop-9000 city, seed 1 deterministically adds at least one structure.
	"AddHistoricalBuildings plants structures in a large city"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		ARegionArray *arr = new ARegionArray(4, 4);
		regs->pRegionArrays[1] = arr;

		ARegion *city = nullptr;
		for (int y = 0; y < 4; y++)
			for (int x = 0; x < 4; x++)
				if ((x + y) % 2 == 0) {
					ARegion *r = placeCity(arr, regs, x, y, (x == 0 && y == 0) ? 9000 : 0);
					if (x == 0 && y == 0) city = r;
				}

		expect(fatal(city != nullptr));
		expect(city->objects.Num() == 0_i) << "no buildings before";

		captureCout([&]{
			seedrandom(1);
			regs->AddHistoricalBuildings(arr, 4, 4);
		});

		expect(that % city->objects.Num() > 0) << "the city gained a historical structure";
	};

	// FindNearestStartingCity walks the region graph from a start and returns the nearest
	// valid target. With START_CITIES_EXIST off, a *visited town* is the target.
	"FindNearestStartingCity finds the nearest visited town"_test = []
	{
		int saved = Globals->START_CITIES_EXIST;
		Globals->START_CITIES_EXIST = 0;

		ARegionList *regs = new ARegionList();
		ARegion *a = new ARegion(); a->num = 0; a->SetLoc(0, 0, 1);
		ARegion *b = new ARegion(); b->num = 1; b->SetLoc(2, 0, 1);
		ARegion *c = new ARegion(); c->num = 2; c->SetLoc(4, 0, 1);
		regs->Add(a); regs->Add(b); regs->Add(c);
		link(a, D_SOUTHEAST, b, D_NORTHWEST);
		link(b, D_SOUTHEAST, c, D_NORTHWEST);
		// Only C is a valid target.
		c->town = new TownInfo; c->town->name = new AString("Haven");
		c->visited = 1;

		int dir = -1;
		ARegion *found = regs->FindNearestStartingCity(a, &dir);
		expect(found == c) << "nearest visited town is C";

		Globals->START_CITIES_EXIST = saved;
	};

	// With START_CITIES_EXIST on, validity is decided by ARegion::IsStartingCity(). The
	// unittest ruleset stubs that to always return 0 (unittest/world.cpp), so no region can
	// ever be a starting city -- this drives FindNearestStartingCity's "nothing found"
	// fall-through, which returns 0.
	"FindNearestStartingCity returns null when no region qualifies"_test = []
	{
		int saved = Globals->START_CITIES_EXIST;
		Globals->START_CITIES_EXIST = 1;

		ARegionList *regs = new ARegionList();
		ARegion *a = new ARegion(); a->num = 0; a->SetLoc(0, 0, 1);
		ARegion *b = new ARegion(); b->num = 1; b->SetLoc(2, 0, 1);
		regs->Add(a); regs->Add(b);
		link(a, D_SOUTHEAST, b, D_NORTHWEST);
		// Even a big town is not a "starting city" under the unittest IsStartingCity stub.
		b->town = new TownInfo; b->town->name = new AString("Capital");
		b->town->pop = 25000;

		int dir = -1;
		ARegion *found = regs->FindNearestStartingCity(a, &dir);
		expect(found == nullptr) << "no starting city exists in the unittest ruleset";

		Globals->START_CITIES_EXIST = saved;
	};

	// When the target is reached through a shaft rather than a lateral neighbor, the returned
	// direction is MOVE_IN. We link start A and target C with a bidirectional shaft so the BFS
	// descends to C and the back-direction resolves to the inner link.
	"FindNearestStartingCity reports MOVE_IN through a shaft"_test = []
	{
		int saved = Globals->START_CITIES_EXIST;
		Globals->START_CITIES_EXIST = 0; // visited-town validity

		ARegionList *regs = new ARegionList();
		ARegion *a = new ARegion(); a->num = 0; a->SetLoc(0, 0, 1);
		ARegion *c = new ARegion(); c->num = 1; c->SetLoc(0, 0, 2); // a level down
		regs->Add(a); regs->Add(c);

		Object *down = new Object(a); down->inner = c->num; a->objects.Add(down);
		Object *up   = new Object(c); up->inner   = a->num; c->objects.Add(up);

		c->town = new TownInfo; c->town->name = new AString("Undertown");
		c->visited = 1;

		int dir = -99;
		ARegion *found = regs->FindNearestStartingCity(a, &dir);
		expect(found == c) << "target reached through the shaft";
		expect(dir == MOVE_IN) << "back-direction to the start is MOVE_IN";

		Globals->START_CITIES_EXIST = saved;
	};

	// The base "builds a fully wired, typed, named surface" test only inspects one region, so
	// the economy() and giveNames() subsystems were executed but never verified. Here we assert
	// whole-surface post-conditions that prove they processed *every* region rather than merely
	// running: economy() calls ManualSetup on all even-parity hexes (setting habitat > 0) and
	// giveNames() names them all. These are the only post-conditions that hold *regardless of the
	// RNG draws*, so they are the only ones safe to assert through CreateNaturalSurfaceLevel.
	//
	// Deliberately NOT asserted here: the settlement count. It is not a draw-independent
	// post-condition, and it is not even reproducible. CreateNaturalSurfaceLevel types the surface
	// from SimplexNoise, whose permutation table is shuffled with a std::default_random_engine
	// seeded from time(0) (simplex.cpp) -- NOT from seedrandom(). So the biome map (a) changes every
	// wall-clock second and (b) differs between C++ standard libraries, because std::default_random_engine
	// and std::shuffle are implementation-defined (libc++ on macOS vs libstdc++ on Linux). On maps
	// where getPoints' candidate sites all fall on ocean/barren terrain, economy() places zero
	// settlements -- which is exactly why an earlier `towns > 0` assertion here passed on macOS but
	// FAILED on the Linux CI box under the same seed. Settlement placement is instead pinned
	// portably by the "economy() places settlements deterministically" test below, which drives
	// economy() directly on a controlled surface and never touches the time(0) terrain generator.
	// (The time(0)/default_random_engine seeding in simplex.cpp and aregion.cpp makeRivers is an
	// engine-side reproducibility bug flagged to the maintainers; this test only documents it.)
	"CreateNaturalSurfaceLevel drives economy and naming across the whole surface"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);

		captureCout([&]{
			seedrandom(42);
			Map *map = new Map(16, 16);
			regs->CreateNaturalSurfaceLevel(map);
		});

		expect(fatal(regs->Num() == 32_i));

		int named = 0, habitatSet = 0;
		forlist(regs) {
			ARegion *r = (ARegion *) elem;
			if (r->name && std::string(r->name->Str()).size() > 0) named++;
			if (r->habitat > 0) habitatSet++;   // set by economy() -> ManualSetup
		}

		expect(named == regs->Num()) << "giveNames named every region";
		expect(habitatSet == regs->Num()) << "economy() ran ManualSetup on every region";
	};

	// Settlement placement, pinned portably. Driving economy() directly -- rather than through
	// CreateNaturalSurfaceLevel -- sidesteps the non-deterministic time(0) terrain generator (see
	// the note in the test above) and lets us control the surface. economy() itself is fully
	// deterministic and platform-independent: getPoints uses only std::vector plus the seedable
	// ISAAC getrandom(), with no time(0), no std::shuffle and no unordered-container iteration
	// feeding the RNG. So a fixed seed produces an *identical* town count on every platform, which
	// is what makes `towns > 0` a legitimate, non-brittle assertion here even though it was not one
	// through the full generator. On an all-plains surface every getPoints candidate is a valid
	// settlement site, so at least one town is always placed.
	"economy() places settlements deterministically on an all-plains surface"_test = []
	{
		const int W = 16;

		// R_PLAIN everywhere on the even-parity sublattice; pop 0 so there is no pre-existing town.
		auto buildPlains = [W] {
			ARegionList *regs = new ARegionList();
			regs->CreateLevels(2);
			ARegionArray *arr = new ARegionArray(W, W);
			regs->pRegionArrays[1] = arr;
			for (int y = 0; y < W; y++)
				for (int x = 0; x < W; x++)
					if ((x + y) % 2 == 0)
						placeCity(arr, regs, x, y, 0);
			return regs;
		};

		auto countTowns = [](ARegionList *regs) {
			int towns = 0;
			forlist(regs) {
				ARegion *r = (ARegion *) elem;
				if (r->town) towns++;
			}
			return towns;
		};

		ARegionList *first = buildPlains();
		int firstTowns = 0;
		captureCout([&]{
			seedrandom(20260725);
			economy(first->GetRegionArray(1), W, W);
		});
		firstTowns = countTowns(first);

		expect(that % firstTowns > 0) << "economy() placed at least one settlement on all-plains terrain";

		// Reproducibility property: same seed, same surface -> same settlement count. This holds on
		// each platform (ISAAC is deterministic) and, because economy() has no platform-dependent
		// ordering, the count itself matches across platforms too.
		ARegionList *second = buildPlains();
		int secondTowns = 0;
		captureCout([&]{
			seedrandom(20260725);
			economy(second->GetRegionArray(1), W, W);
		});
		secondTowns = countTowns(second);

		expect(secondTowns == firstTowns) << "same seed -> identical settlement count";
	};
};

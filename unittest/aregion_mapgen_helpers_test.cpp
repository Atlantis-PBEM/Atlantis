#include "external/boost/ut.hpp"

#include <algorithm>
#include <cmath>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "graphs.h"
#include "mapgen.h" // B_* biome constants

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

// These map-generation leaf helpers have external linkage but no prototype in any header (they
// are only referenced within aregion.cpp, transitively through CreateNaturalSurfaceLevel). The
// mapgen suite *executes* them but asserts nothing about their results, so their behaviour was
// unpinned. They are pure / hand-constructible, so declare them here and assert directly. The
// remaining map-gen functions (makeRivers, cleanupIsolatedPlaces, placeVolcanoes, giveNames,
// economy, ...) take file-local types (WaterBody, NameArea) or need a full RNG Map, and stay
// snapshot-suite territory.
int mapBiome(int biome);
bool isInnerWater(ARegion *reg);
bool isNearWater(ARegion *reg);
int distance(graphs::Location2D a, graphs::Location2D b);
int cylDistance(graphs::Location2D a, graphs::Location2D b, int w);

namespace {
	// Independent oracle for the hex distance formula (re-derived from the spec, not by calling
	// distance() back on itself).
	int expectedDistance(int ax, int ay, int bx, int by)
	{
		int dX = std::abs(ax - bx);
		int dY = std::abs(ay - by);
		return dX + std::max(0, (dY - dX) / 2);
	}

	ARegion *regionOfType(int type)
	{
		ARegion *r = new ARegion();
		r->type = type;
		return r;
	}
}

ut::suite<"ARegion mapgen helpers"> aregion_mapgen_helpers_suite = []
{
	using namespace ut;

	// mapBiome maps each generator biome to a terrain type; anything unrecognised is -1.
	"mapBiome maps every biome, -1 for the unknown"_test = []
	{
		expect(mapBiome(B_TUNDRA)    == R_TUNDRA);
		expect(mapBiome(B_MOUNTAINS) == R_MOUNTAIN);
		expect(mapBiome(B_SWAMP)     == R_SWAMP);
		expect(mapBiome(B_FOREST)    == R_FOREST);
		expect(mapBiome(B_PLAINS)    == R_PLAIN);
		expect(mapBiome(B_JUNGLE)    == R_JUNGLE);
		expect(mapBiome(B_DESERT)    == R_DESERT);
		expect(mapBiome(B_WATER)     == R_OCEAN);
		// B_UNKNOWN and any out-of-range biome hit the default arm.
		expect(mapBiome(B_UNKNOWN) == -1_i);
		expect(mapBiome(999)       == -1_i);
	};

	// isInnerWater is true only when every *set* neighbor is ocean (a hex fully enclosed by
	// water). It inspects neighbors, not the region's own type.
	"isInnerWater is true only when all neighbors are ocean"_test = []
	{
		ARegion *r = regionOfType(R_OCEAN);
		// No neighbors set yet -> vacuously "inner" (no non-ocean neighbor exists).
		expect(isInnerWater(r)) << "no neighbors -> vacuously inner";

		r->neighbors[0] = regionOfType(R_OCEAN);
		r->neighbors[3] = regionOfType(R_OCEAN);
		expect(isInnerWater(r)) << "all ocean neighbors -> inner";

		r->neighbors[1] = regionOfType(R_PLAIN); // one land neighbor
		expect(!isInnerWater(r)) << "any non-ocean neighbor -> not inner";
	};

	// isNearWater is true when at least one neighbor is ocean.
	"isNearWater is true when any neighbor is ocean"_test = []
	{
		ARegion *r = regionOfType(R_PLAIN);
		r->neighbors[0] = regionOfType(R_PLAIN);
		r->neighbors[2] = regionOfType(R_FOREST);
		expect(!isNearWater(r)) << "no ocean neighbor -> not near water";

		r->neighbors[4] = regionOfType(R_OCEAN);
		expect(isNearWater(r)) << "an ocean neighbor -> near water";
	};

	// distance is a hex-grid distance: dX + max(0, (dY - dX)/2).
	"distance matches the hex-grid formula"_test = []
	{
		expect(distance({0, 0}, {0, 0}) == 0_i);
		expect(distance({0, 0}, {2, 0}) == expectedDistance(0, 0, 2, 0));
		expect(distance({0, 0}, {0, 4}) == expectedDistance(0, 0, 0, 4)); // 2
		expect(distance({0, 0}, {2, 4}) == expectedDistance(0, 0, 2, 4)); // 3
		expect(distance({1, 1}, {4, 6}) == expectedDistance(1, 1, 4, 6));
		// Symmetric.
		expect(distance({2, 4}, {0, 0}) == distance({0, 0}, {2, 4}));
	};

	// cylDistance wraps in x over a cylinder of width w, taking the shortest of the direct and
	// the two wrapped offsets.
	"cylDistance takes the shorter wrapped path"_test = []
	{
		// On a width-10 cylinder, (0,0) and (9,0) are 1 apart the short way, not 9.
		int direct = distance({0, 0}, {9, 0});
		int wrapped = cylDistance({0, 0}, {9, 0}, 10);
		expect(wrapped == 1_i) << "wrap makes the far edge adjacent";
		expect(that % wrapped < direct) << "cylinder distance never exceeds the direct distance";

		// When wrapping does not help, cylDistance equals the direct distance.
		expect(cylDistance({0, 0}, {2, 0}, 10) == distance({0, 0}, {2, 0}));
	};
};

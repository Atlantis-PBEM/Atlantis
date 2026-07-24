#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "orders.h" // M_WALK / M_SWIM / M_RIDE

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	Object *addRoad(ARegion *reg, int roadType)
	{
		Object *o = new Object(reg);
		o->type = roadType; // complete by default (incomplete == 0)
		reg->objects.Add(o);
		return o;
	}
}

// NOTE: the unittest ruleset sets WEATHER_EXISTS = 0, so MoveCost's weather multiplier is
// never applied (cost stays at the base of 1 before the terrain multiplier). The blizzard
// and clearskies branches are only reachable in a weather-enabled ruleset (snapshot suite).
ut::suite<"ARegion terrain"> aregion_terrain_suite = []
{
	using namespace ut;

	// IsNativeRace consults the terrain's race tables. The coastal_races table is only
	// checked when the region is coastal; the plain (non-coastal) races are always checked.
	// Plain's tables (gamedata.cpp): races {PLAINSMAN, NOMAD, HIGHELF}, coastal {VIKING, SEAELF}.
	"IsNativeRace checks non-coastal races for an inland region"_test = []
	{
		ARegion *plain = new ARegion();
		plain->type = R_PLAIN;
		plain->ZeroNeighbors(); // inland: not coastal

		expect(plain->IsNativeRace(I_PLAINSMAN) == 1_i) << "plainsman is a plains race";
		expect(plain->IsNativeRace(I_VIKING) == 0_i)
			<< "viking is coastal-only and this region is inland";
		expect(plain->IsNativeRace(I_LEADERS) == 0_i) << "leaders are not a native race";
	};

	// Give the plain an ocean neighbor so IsCoastal() is true; now the coastal race matches.
	"IsNativeRace admits coastal races for a coastal region"_test = []
	{
		ARegion *plain = new ARegion();
		plain->type = R_PLAIN;
		plain->ZeroNeighbors();
		ARegion *sea = new ARegion();
		sea->type = R_OCEAN;
		plain->neighbors[D_NORTH] = sea;

		expect(fatal(plain->IsCoastal() > 0));
		expect(plain->IsNativeRace(I_VIKING) == 1_i) << "coastal plain admits vikings";
		expect(plain->IsNativeRace(I_PLAINSMAN) == 1_i) << "non-coastal races still count";
	};

	// MoveCost for a walking move is the terrain's movepoints (weather multiplier == 1
	// here). Plain movepoints == 1, mountain == 2 (gamedata.cpp).
	"MoveCost is the terrain movepoints for a plain walk"_test = []
	{
		ARegion *dest = new ARegion();
		dest->type = R_PLAIN;
		dest->weather = W_NORMAL;
		ARegion *from = new ARegion();
		from->ZeroNeighbors();

		expect(dest->MoveCost(M_WALK, from, D_NORTH, nullptr) == 1_i);
	};

	// A road on the connecting edge halves the walking cost (cost -= cost/2). Mountain
	// costs 2 normally, 1 with a road; this also exercises the `road` out-parameter.
	"MoveCost halves the cost across a connecting road"_test = []
	{
		ARegion *dest = new ARegion();
		dest->type = R_MOUNTAIN;
		dest->weather = W_NORMAL;
		dest->ZeroNeighbors();

		ARegion *from = new ARegion();
		from->type = R_MOUNTAIN;
		from->weather = W_NORMAL;
		from->ZeroNeighbors();

		// No road yet: full mountain cost.
		expect(dest->MoveCost(M_WALK, from, D_NORTH, nullptr) == 2_i);

		// Build a mutual road link on the from->dest (north) edge.
		from->neighbors[D_NORTH] = dest;
		dest->neighbors[D_SOUTH] = from;
		addRoad(from, O_ROADN); // from's north exit road
		addRoad(dest, O_ROADS); // dest's south road faces back toward `from`

		AString road;
		expect(dest->MoveCost(M_WALK, from, D_NORTH, &road) == 1_i) << "road halves 2 -> 1";
		std::string roadText = road.Str();
		expect(eq(roadText, std::string("on a road ")));
	};

	// Swimming ignores roads even when they exist: cost is purely terrain movepoints.
	"MoveCost for swimming ignores roads"_test = []
	{
		ARegion *dest = new ARegion();
		dest->type = R_MOUNTAIN; // movepoints 2
		dest->weather = W_NORMAL;
		dest->ZeroNeighbors();
		ARegion *from = new ARegion();
		from->ZeroNeighbors();
		from->neighbors[D_NORTH] = dest;
		dest->neighbors[D_SOUTH] = from;
		addRoad(from, O_ROADN);
		addRoad(dest, O_ROADS);

		// M_SWIM never enters the road-discount branch, so the cost stays at 2.
		expect(dest->MoveCost(M_SWIM, from, D_NORTH, nullptr) == 2_i);
	};
};

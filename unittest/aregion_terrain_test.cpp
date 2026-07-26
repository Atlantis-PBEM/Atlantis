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

	// M_FLY takes neither MoveCost branch: not the swim arm, not the walk/ride arm. So the
	// terrain movepoints multiplier AND the road discount are both skipped -- a flying move
	// costs exactly the weather base (1 with weather off), independent of terrain and roads.
	// This is a real, reachable movetype: Unit::MoveType returns M_FLY for flying units
	// (unit.cpp) and it flows straight into MoveCost. Walking this mountain costs 2 (1 across a
	// road); flying it costs 1 either way. Without this test a broken flying-cost calculation --
	// e.g. one that started multiplying by movepoints -- would pass every other MoveCost test.
	"MoveCost for flying is the weather base, ignoring terrain and roads"_test = []
	{
		ARegion *dest = new ARegion();
		dest->type = R_MOUNTAIN; // movepoints 2 -- would make a walk cost 2
		dest->weather = W_NORMAL;
		dest->ZeroNeighbors();

		ARegion *from = new ARegion();
		from->type = R_MOUNTAIN;
		from->weather = W_NORMAL;
		from->ZeroNeighbors();

		// A mutual road on the traversed (north) edge -- which flying must ignore.
		from->neighbors[D_NORTH] = dest;
		dest->neighbors[D_SOUTH] = from;
		addRoad(from, O_ROADN);
		addRoad(dest, O_ROADS);

		AString road;
		expect(dest->MoveCost(M_FLY, from, D_NORTH, &road) == 1_i)
			<< "flying ignores the mountain movepoints and stays at the base cost";
		expect(eq(std::string(road.Str()), std::string("")))
			<< "flying never enters the road branch, so the road out-param is left untouched";
	};

	// With weather enabled the flying base tracks the weather multiplier (2 in bad weather, the
	// fixed 10 in a blizzard, 1 under clearskies) but still never picks up the terrain
	// movepoints: a walk over this mountain in bad weather would be 2*2 = 4, whereas flying is
	// just the doubled base of 2. This pins that M_FLY's cost is the *raw weather base*.
	"MoveCost for flying tracks the weather base but not the terrain"_test = []
	{
		int saved = Globals->WEATHER_EXISTS;
		Globals->WEATHER_EXISTS = 1;

		ARegion *from = new ARegion();
		from->ZeroNeighbors();

		ARegion *mtn = new ARegion();
		mtn->type = R_MOUNTAIN; // movepoints 2 -- irrelevant to flying
		mtn->weather = W_WINTER; // non-blizzard bad weather -> base doubles to 2
		mtn->clearskies = 0;
		mtn->ZeroNeighbors();
		expect(mtn->MoveCost(M_FLY, from, D_NORTH, nullptr) == 2_i)
			<< "bad-weather base 2, no terrain multiplier";

		// Blizzard short-circuits to the fixed 10 before the movetype is even considered.
		mtn->weather = W_BLIZZARD;
		expect(mtn->MoveCost(M_FLY, from, D_NORTH, nullptr) == 10_i) << "blizzard is a fixed 10";

		// clearskies resets the base back to 1.
		mtn->weather = W_WINTER;
		mtn->clearskies = 1;
		expect(mtn->MoveCost(M_FLY, from, D_NORTH, nullptr) == 1_i) << "clearskies resets the base";

		Globals->WEATHER_EXISTS = saved;
	};

	// The weather block of MoveCost is gated by WEATHER_EXISTS (off in the unittest ruleset).
	// Flip it to reach: the blizzard fast-return, the bad-weather doubled base cost, and the
	// clearskies/normal reset.
	"MoveCost applies weather modifiers when weather exists"_test = []
	{
		int saved = Globals->WEATHER_EXISTS;
		Globals->WEATHER_EXISTS = 1;

		ARegion *from = new ARegion();
		from->ZeroNeighbors();

		// Blizzard: fixed cost of 10 regardless of terrain/movement.
		ARegion *blizz = new ARegion();
		blizz->type = R_PLAIN;
		blizz->weather = W_BLIZZARD;
		blizz->clearskies = 0;
		blizz->ZeroNeighbors();
		expect(blizz->MoveCost(M_WALK, from, D_NORTH, nullptr) == 10_i) << "blizzard costs 10";

		// Non-blizzard bad weather doubles the base cost: plain movepoints 1 * 2 = 2.
		ARegion *winter = new ARegion();
		winter->type = R_PLAIN;
		winter->weather = W_WINTER;
		winter->clearskies = 0;
		winter->ZeroNeighbors();
		expect(winter->MoveCost(M_WALK, from, D_NORTH, nullptr) == 2_i) << "bad weather doubles";

		// clearskies resets the base back to 1: plain 1 * 1 = 1.
		winter->clearskies = 1;
		expect(winter->MoveCost(M_WALK, from, D_NORTH, nullptr) == 1_i) << "clearskies resets";

		Globals->WEATHER_EXISTS = saved;
	};
};

#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h" // terrain suite: roads
#include "orders.h" // terrain suite: M_WALK / M_SWIM / M_FLY / M_RIDE

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

ut::suite<"ARegion geometry"> aregion_geometry_suite = []
{
	using namespace ut;

	// The constructor zeroes all neighbor slots; a fresh region is fully disconnected.
	"a new region has no neighbors and origin location"_test = []
	{
		ARegion *reg = new ARegion();
		expect(fatal(reg != nullptr));
		for (int i = 0; i < NDIRS; i++)
			expect(reg->neighbors[i] == nullptr);
	};

	"ZeroNeighbors clears every direction slot"_test = []
	{
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();
		for (int i = 0; i < NDIRS; i++) a->neighbors[i] = b;

		a->ZeroNeighbors();

		for (int i = 0; i < NDIRS; i++)
			expect(a->neighbors[i] == nullptr);
	};

	"SetLoc assigns x, y and z coordinates"_test = []
	{
		ARegion *reg = new ARegion();
		reg->SetLoc(3, 7, 2);
		expect(reg->xloc == 3_i);
		expect(reg->yloc == 7_i);
		expect(reg->zloc == 2_i);
	};

	// SetName copies the string; the stored AString compares equal to the source text.
	"SetName stores a copy of the given name"_test = []
	{
		ARegion *reg = new ARegion();
		reg->SetName("Rivendell");
		std::string current = reg->name->Str();
		std::string expected = "Rivendell";
		expect(eq(current, expected));
	};

	// GetPoleDistance counts 1 for the region itself plus one per neighbor hop along a
	// fixed direction, following the neighbors[dir] chain until it runs out.
	"GetPoleDistance is 1 when there is no neighbor in that direction"_test = []
	{
		ARegion *reg = new ARegion();
		reg->ZeroNeighbors();
		expect(reg->GetPoleDistance(D_NORTH) == 1_i);
	};

	"GetPoleDistance counts the region plus each hop in the chain"_test = []
	{
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();
		ARegion *c = new ARegion();
		a->ZeroNeighbors();
		b->ZeroNeighbors();
		c->ZeroNeighbors();

		// Two hops north: a -> b -> c. Distance = 1 (self) + 2 (hops) = 3.
		a->neighbors[D_NORTH] = b;
		b->neighbors[D_NORTH] = c;

		expect(a->GetPoleDistance(D_NORTH) == 3_i);
		// The chain is directional: nothing was linked going south.
		expect(a->GetPoleDistance(D_SOUTH) == 1_i);
	};

	// An ocean region is coastal by virtue of its own terrain type (similar_type OCEAN).
	"IsCoastal is true for an ocean region itself"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_OCEAN;
		reg->ZeroNeighbors();
		expect(that % reg->IsCoastal() > 0);
	};

	// A land region's coastal status is the count of adjacent ocean neighbors.
	"IsCoastal counts adjacent ocean neighbors for a land region"_test = []
	{
		ARegion *land = new ARegion();
		land->type = R_PLAIN;
		land->ZeroNeighbors();
		expect(land->IsCoastal() == 0_i) << "inland plain has no ocean neighbors";

		ARegion *sea1 = new ARegion();
		sea1->type = R_OCEAN;
		ARegion *sea2 = new ARegion();
		sea2->type = R_OCEAN;
		land->neighbors[D_NORTH] = sea1;
		land->neighbors[D_SOUTH] = sea2;

		expect(land->IsCoastal() == 2_i) << "two ocean neighbors -> seacount 2";
	};

	// IsCoastalOrLakeside also returns 1 for an ocean region, and otherwise counts ocean
	// neighbors. It differs from IsCoastal only in how it handles lakes, which we keep out
	// of this land/ocean case to isolate the shared behavior.
	"IsCoastalOrLakeside is true for an ocean region"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_OCEAN;
		reg->ZeroNeighbors();
		expect(reg->IsCoastalOrLakeside() == 1_i);
	};

	"IsCoastalOrLakeside counts ocean neighbors for a land region"_test = []
	{
		ARegion *land = new ARegion();
		land->type = R_PLAIN;
		land->ZeroNeighbors();
		expect(land->IsCoastalOrLakeside() == 0_i);

		ARegion *sea = new ARegion();
		sea->type = R_OCEAN;
		land->neighbors[D_NORTHEAST] = sea;
		expect(land->IsCoastalOrLakeside() == 1_i);
	};

	// A lake region itself is coastal only when LAKESIDE_IS_COASTAL is set (off in the
	// unittest ruleset). We flip the flag for the test and restore it.
	"IsCoastal treats a lake region as coastal only under LAKESIDE_IS_COASTAL"_test = []
	{
		int saved = Globals->LAKESIDE_IS_COASTAL;

		ARegion *lake = new ARegion();
		lake->type = R_LAKE;
		lake->ZeroNeighbors();

		Globals->LAKESIDE_IS_COASTAL = 1;
		expect(lake->IsCoastal() == 1_i) << "lake is coastal when the rule is on";
		Globals->LAKESIDE_IS_COASTAL = 0;
		expect(lake->IsCoastal() == 0_i) << "isolated lake is not coastal when the rule is off";

		Globals->LAKESIDE_IS_COASTAL = saved;
	};

	// A lake NEIGHBOR is ocean-similar, but IsCoastal skips it in the seacount unless
	// LAKESIDE_IS_COASTAL is set. This exercises the neighbor-lake-skip branch.
	"IsCoastal skips a lake neighbor unless LAKESIDE_IS_COASTAL"_test = []
	{
		int saved = Globals->LAKESIDE_IS_COASTAL;

		ARegion *land = new ARegion();
		land->type = R_PLAIN;
		land->ZeroNeighbors();
		ARegion *lakeNbr = new ARegion();
		lakeNbr->type = R_LAKE;
		land->neighbors[D_NORTH] = lakeNbr;

		Globals->LAKESIDE_IS_COASTAL = 0;
		expect(land->IsCoastal() == 0_i) << "lake neighbor is skipped when the rule is off";
		Globals->LAKESIDE_IS_COASTAL = 1;
		expect(land->IsCoastal() == 1_i) << "lake neighbor counts when the rule is on";

		Globals->LAKESIDE_IS_COASTAL = saved;
	};
};

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

	// MoveCost's road discount and the bad-weather multiplier are each tested alone above; this
	// pins their combination. On a plain (movepoints 1) in bad weather the base doubles to 2, and
	// a connecting road halves it back to 1.
	"MoveCost halves a bad-weather cost across a road"_test = []
	{
		int saved = Globals->WEATHER_EXISTS;
		Globals->WEATHER_EXISTS = 1;

		ARegion *dest = new ARegion();
		dest->type = R_PLAIN;
		dest->weather = W_WINTER; // non-blizzard bad weather -> base cost 2
		dest->clearskies = 0;
		dest->ZeroNeighbors();

		ARegion *from = new ARegion();
		from->type = R_PLAIN;
		from->weather = W_WINTER;
		from->clearskies = 0;
		from->ZeroNeighbors();

		from->neighbors[D_NORTH] = dest;
		dest->neighbors[D_SOUTH] = from;
		addRoad(from, O_ROADN);
		addRoad(dest, O_ROADS);

		AString road;
		expect(dest->MoveCost(M_WALK, from, D_NORTH, &road) == 1_i)
			<< "bad-weather 2, road halves to 1";
		expect(eq(std::string(road.Str()), std::string("on a road ")));

		Globals->WEATHER_EXISTS = saved;
	};
};

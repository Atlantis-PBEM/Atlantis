#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	// Attach a *complete* road Object of the given O_ROAD* type to a region. HasRoad and
	// friends only count roads with incomplete < 1, which the Object constructor already
	// gives us (incomplete = 0), so we just set the type.
	Object *addRoad(ARegion *reg, int roadType)
	{
		Object *o = new Object(reg);
		o->type = roadType;
		reg->objects.Add(o);
		return o;
	}
}

ut::suite<"ARegion roads"> aregion_roads_suite = []
{
	using namespace ut;

	// GetRoadDirection is a pure switch: it maps a real hex direction to the road
	// Object type that would sit on that edge. Nothing here touches the world, so a
	// bare `new ARegion()` is a sufficient fixture.
	"GetRoadDirection maps each direction to its road object type"_test = []
	{
		ARegion *reg = new ARegion();
		expect(fatal(reg != nullptr));

		expect(reg->GetRoadDirection(D_NORTH)     == O_ROADN);
		expect(reg->GetRoadDirection(D_NORTHEAST) == O_ROADNE);
		expect(reg->GetRoadDirection(D_NORTHWEST) == O_ROADNW);
		expect(reg->GetRoadDirection(D_SOUTH)     == O_ROADS);
		expect(reg->GetRoadDirection(D_SOUTHEAST) == O_ROADSE);
		expect(reg->GetRoadDirection(D_SOUTHWEST) == O_ROADSW);
	};

	// With no neighbor set in that direction, GetRealDirComp falls through to its
	// hard-coded geometric complement table.
	"GetRealDirComp returns geometric complement when no neighbor is linked"_test = []
	{
		ARegion *reg = new ARegion();
		reg->ZeroNeighbors();

		expect(reg->GetRealDirComp(D_NORTH)     == D_SOUTH);
		expect(reg->GetRealDirComp(D_NORTHEAST) == D_SOUTHWEST);
		expect(reg->GetRealDirComp(D_NORTHWEST) == D_SOUTHEAST);
		expect(reg->GetRealDirComp(D_SOUTH)     == D_NORTH);
		expect(reg->GetRealDirComp(D_SOUTHEAST) == D_NORTHWEST);
		expect(reg->GetRealDirComp(D_SOUTHWEST) == D_NORTHEAST);
	};

	// When a neighbor IS linked, GetRealDirComp ignores the complement table and instead
	// returns the index under which the neighbor points back at us. This can differ from
	// the geometric complement, which is exactly the case worth locking in: an asymmetric
	// back-link wins over the switch.
	"GetRealDirComp returns the actual back-link index of the neighbor"_test = []
	{
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();
		a->ZeroNeighbors();
		b->ZeroNeighbors();

		// a's north neighbor is b, but b points back at a from the NORTHEAST slot, not
		// the geometric complement (SOUTH).
		a->neighbors[D_NORTH] = b;
		b->neighbors[D_NORTHEAST] = a;

		expect(a->GetRealDirComp(D_NORTH) == D_NORTHEAST)
			<< "should follow the real back-link, not the SOUTH complement";
	};

	// If a neighbor exists but does not point back at us at all, the back-link search
	// finds nothing and GetRealDirComp falls back to the geometric complement.
	"GetRealDirComp falls back to complement when neighbor has no back-link"_test = []
	{
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();
		a->ZeroNeighbors();
		b->ZeroNeighbors();

		a->neighbors[D_NORTH] = b; // b->neighbors all null: no path back to a

		expect(a->GetRealDirComp(D_NORTH) == D_SOUTH);
	};

	"HasRoad is false with no objects"_test = []
	{
		ARegion *reg = new ARegion();
		expect(reg->HasRoad() == 0_i);
	};

	// A road under construction (incomplete >= 1) does not count as a road yet.
	"HasRoad ignores incomplete roads and counts complete ones"_test = []
	{
		ARegion *reg = new ARegion();

		Object *building = addRoad(reg, O_ROADN);
		building->incomplete = 1;
		expect(reg->HasRoad() == 0_i) << "incomplete road must not count";

		building->incomplete = 0;
		expect(reg->HasRoad() == 1_i) << "completed road must count";
	};

	// A non-road object never satisfies HasRoad.
	"HasRoad is false for a non-road object"_test = []
	{
		ARegion *reg = new ARegion();
		Object *o = new Object(reg);
		o->type = O_DUMMY;
		reg->objects.Add(o);
		expect(reg->HasRoad() == 0_i);
	};

	// HasExitRoad matches only the road type that corresponds to the queried direction.
	"HasExitRoad matches the road for the queried direction only"_test = []
	{
		ARegion *reg = new ARegion();
		addRoad(reg, O_ROADN); // road on the NORTH edge

		expect(reg->HasExitRoad(D_NORTH) == 1_i);
		expect(reg->HasExitRoad(D_SOUTH) == 0_i);
		expect(reg->HasExitRoad(D_NORTHEAST) == 0_i);
	};

	// HasConnectingRoad is true only when the neighbor in that direction has an exit road
	// pointing back along the complementary edge.
	"HasConnectingRoad requires a matching road on the neighbor's opposite edge"_test = []
	{
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();
		a->ZeroNeighbors();
		b->ZeroNeighbors();
		a->neighbors[D_NORTH] = b;
		b->neighbors[D_SOUTH] = a; // symmetric geometric link

		// Our own exit road is not required for HasConnectingRoad; only the neighbor's is.
		expect(a->HasConnectingRoad(D_NORTH) == 0_i) << "neighbor has no road yet";

		addRoad(b, O_ROADS); // b's south edge road faces back toward a
		expect(a->HasConnectingRoad(D_NORTH) == 1_i);
	};

	"HasConnectingRoad is false with no neighbor"_test = []
	{
		ARegion *a = new ARegion();
		a->ZeroNeighbors();
		expect(a->HasConnectingRoad(D_NORTH) == 0_i);
	};

	// CountConnectingRoads counts directions where we have an exit road, a neighbor, and
	// that neighbor has a connecting road back.
	"CountConnectingRoads counts fully mutual road links"_test = []
	{
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();
		ARegion *c = new ARegion();
		a->ZeroNeighbors();
		b->ZeroNeighbors();
		c->ZeroNeighbors();

		// North link to b is fully mutual (both sides have the facing road).
		a->neighbors[D_NORTH] = b;
		b->neighbors[D_SOUTH] = a;
		addRoad(a, O_ROADN);
		addRoad(b, O_ROADS);

		// South link to c: a has an exit road, but c has none, so it must not count.
		a->neighbors[D_SOUTH] = c;
		c->neighbors[D_NORTH] = a;
		addRoad(a, O_ROADS);

		expect(a->CountConnectingRoads() == 1_i);
	};

	// RoadDevelopmentBonus with no roads at all yields zero. This exercises the top-level
	// traversal setup (allocates its own visited list) without needing a populated map.
	"RoadDevelopmentBonus is zero with no connecting roads"_test = []
	{
		ARegion *reg = new ARegion();
		reg->ZeroNeighbors();
		expect(reg->RoadDevelopmentBonus(50, 0) == 0_i);
	};

	// A single mutual road link to a developed, town-bearing neighbor produces a positive
	// bonus via TraceConnectedRoad. We assert only that the bonus is positive rather than
	// re-deriving the exact scoring, which is the traversal's own private policy.
	"RoadDevelopmentBonus rewards a connected developed neighbor"_test = []
	{
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();
		a->ZeroNeighbors();
		b->ZeroNeighbors();
		a->neighbors[D_NORTH] = b;
		b->neighbors[D_SOUTH] = a;
		addRoad(a, O_ROADN);
		addRoad(b, O_ROADS);

		// Make b clearly "more developed" so TraceConnectedRoad's development bonuses fire.
		b->development = 100;

		expect(a->RoadDevelopmentBonus(50, 0) > 0_i);
	};
};

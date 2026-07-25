#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"

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

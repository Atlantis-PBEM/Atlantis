#include "external/boost/ut.hpp"

#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "unit.h"
#include "faction.h"
#include "orders.h" // directions, M_WALK
#include "aregion_test_util.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

using aregion_test::capture;
using aregion_test::contains;

namespace {
	ARegionList *makeRegions(char const *surfaceName)
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		ARegionArray *surface = new ARegionArray(8, 8);
		surface->levelType = ARegionArray::LEVEL_SURFACE;
		surface->SetName(surfaceName);
		regs->pRegionArrays[1] = surface;
		return regs;
	}

	ARegion *surfaceRegion(int x, int y, char const *name)
	{
		ARegion *r = new ARegion();
		r->SetName(name);
		r->type = R_PLAIN;
		r->SetLoc(x, y, ARegionArray::LEVEL_SURFACE);
		// ARegion's ctor does not initialize the economy fields; in a real game
		// ARegion::Setup()/Readin() fill them in before any region is reported on.
		// A test that constructs a region directly skips that, so we must set them
		// here or WriteReport() reads indeterminate memory: it gates a line on
		// Population() (the raw `population` member) and, inside it, indexes
		// ItemDefs[race] — an unset `race` is an out-of-bounds read that crashes.
		// race == -1 is the engine's "no race" sentinel (see economy.cpp and
		// aregion.cpp's PutStr guard); population 0 keeps the peasant/race branch
		// out of WriteReport for reporting tests that don't care about peasants.
		r->race = -1;
		r->population = 0;
		r->basepopulation = 0;
		r->wealth = 0;
		r->wages = 0;
		r->maxwages = 0;
		return r;
	}

	Unit *addUnit(ARegion *reg, int num, Faction *fac)
	{
		Object *o = new Object(reg);
		o->type = O_DUMMY;
		reg->objects.Add(o);
		Unit *u = new Unit(num, fac, 0);
		u->MoveUnit(o);
		return u;
	}

	Object *addRoad(ARegion *reg, int roadType)
	{
		Object *o = new Object(reg);
		o->type = roadType; // complete by default (incomplete == 0)
		reg->objects.Add(o);
		return o;
	}
}

// These cover sub-branches inside methods that are otherwise tested, but only ever along one
// path: the teleport penalty and range limit of GetPlanarDistance, ShortPrint's underworld-depth
// text, WriteReport's dispersed-gate format, and a bad-weather + road MoveCost combo. Each depends
// on a Globals flag or a parameter value the existing suites never vary.
ut::suite<"ARegion branch gaps"> aregion_branch_gaps_suite = []
{
	using namespace ut;

	// GetPlanarDistance's `penalty` parameter is added per cross-level step -- but every existing
	// test passes penalty = 0 with both endpoints on the same level. Here we drive the flat-world
	// z-penalty arithmetic: with the endpoints stacked on the same x,y the horizontal distance is
	// 0, so the result is exactly penalty * |z difference|.
	"GetPlanarDistance adds the teleport penalty across levels (flat world)"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(3);
		regs->pRegionArrays[1] = new ARegionArray(10, 10); // LEVEL_SURFACE: supplies pArr->x

		ARegion *up   = new ARegion(); up->SetLoc(0, 0, ARegionArray::LEVEL_SURFACE); // zloc 1
		ARegion *down = new ARegion(); down->SetLoc(0, 0, 2);                          // one level deeper

		// Horizontal distance 0, |z| = 1 -> result is the penalty itself.
		expect(regs->GetPlanarDistance(up, down, 7, -1) == 7_i) << "penalty * 1 level";

		// A horizontal offset of 2 plus the one-level penalty of 3 -> 2 + 3 = 5.
		ARegion *down2 = new ARegion(); down2->SetLoc(2, 0, 2);
		expect(regs->GetPlanarDistance(up, down2, 3, -1) == 5_i) << "grid distance + penalty";
	};

	// The icosahedral branch seeds its BFS distance with zdist * penalty. With both endpoints
	// mapping to the same surface hex the target is found on the first iteration, so the returned
	// distance is exactly that seed.
	"GetPlanarDistance seeds the icosahedral BFS with the z-penalty"_test = []
	{
		int saved = Globals->ICOSAHEDRAL_WORLD;
		Globals->ICOSAHEDRAL_WORLD = 1;

		ARegionList *regs = new ARegionList();
		regs->CreateLevels(4);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		ARegionArray *surf = new ARegionArray(4, 4);
		regs->pRegionArrays[1] = surf;

		ARegion *r = new ARegion(); r->SetLoc(0, 0, ARegionArray::LEVEL_SURFACE);
		surf->SetRegion(0, 0, r);
		regs->Add(r);

		// Both endpoints project onto surface (0,0); |z| = 2, penalty 5 -> 10, found immediately.
		ARegion *a = new ARegion(); a->SetLoc(0, 0, 1);
		ARegion *b = new ARegion(); b->SetLoc(0, 0, 3);
		expect(regs->GetPlanarDistance(a, b, 5, -1) == 10_i) << "zdist(2) * penalty(5)";

		Globals->ICOSAHEDRAL_WORLD = saved;
	};

	// GetPlanarDistance's `maxdist` bounds the icosahedral BFS. Every existing test passes -1
	// (unbounded). With a real limit the loop stops early and returns the partial distance rather
	// than the true one.
	"GetPlanarDistance honors the icosahedral search range limit"_test = []
	{
		int saved = Globals->ICOSAHEDRAL_WORLD;
		Globals->ICOSAHEDRAL_WORLD = 1;

		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		ARegionArray *surf = new ARegionArray(8, 8);
		regs->pRegionArrays[1] = surf;

		// Chain a - b - c along one row, target two hops from the start.
		ARegion *a = new ARegion(); a->SetLoc(0, 0, ARegionArray::LEVEL_SURFACE);
		ARegion *b = new ARegion(); b->SetLoc(2, 0, ARegionArray::LEVEL_SURFACE);
		ARegion *c = new ARegion(); c->SetLoc(4, 0, ARegionArray::LEVEL_SURFACE);
		surf->SetRegion(0, 0, a);
		surf->SetRegion(2, 0, b);
		surf->SetRegion(4, 0, c);
		a->neighbors[D_SOUTHEAST] = b; b->neighbors[D_NORTHWEST] = a;
		b->neighbors[D_SOUTHEAST] = c; c->neighbors[D_NORTHWEST] = b;
		regs->Add(a); regs->Add(b); regs->Add(c);

		// Unbounded: the full two-hop distance.
		expect(regs->GetPlanarDistance(a, c, 0, -1) == 2_i) << "true distance is 2";
		// Bounded at 0: the search cannot reach distance-1 hexes, so it returns the partial reach.
		expect(regs->GetPlanarDistance(a, c, 0, 0) == 1_i) << "range limit truncates the search";

		Globals->ICOSAHEDRAL_WORLD = saved;
	};

	// GetPlanarDistance's icosahedral branch projects each endpoint onto the surface array and,
	// if the projected hex is empty, re-projects to the wedge corner (one_x += GetLevelXScale-1).
	// The unittest ruleset hardcodes GetLevelXScale/YScale to 1 (unittest/world.cpp), so that
	// offset is 0: the re-projection block still runs but re-checks the SAME empty cell, and
	// start/target stay null. That drives the "couldn't find ends" sentinel -- when either
	// endpoint fails to resolve, the distance is the unreachable marker 10000000.
	// NOTE: the re-projection SUCCESS sub-path (where the +scale-1 offset lands on a real hex)
	// needs GetLevelXScale > 1 and so is only reachable in a real ruleset / the snapshot suite.
	"GetPlanarDistance returns the sentinel when an icosahedral endpoint has no surface hex"_test = []
	{
		int saved = Globals->ICOSAHEDRAL_WORLD;
		Globals->ICOSAHEDRAL_WORLD = 1;

		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		regs->pRegionArrays[1] = new ARegionArray(8, 8); // empty surface grid: no cell populated

		// Both endpoints project onto valid (even-parity) but unpopulated surface cells, so both
		// GetRegion lookups -- and the zero-offset re-projections -- return null. Using both
		// exercises the start AND target re-projection blocks before the sentinel.
		ARegion *a = new ARegion(); a->SetLoc(0, 2, ARegionArray::LEVEL_SURFACE);
		ARegion *b = new ARegion(); b->SetLoc(2, 0, ARegionArray::LEVEL_SURFACE);
		expect(regs->GetPlanarDistance(a, b, 0, -1) == 10000000_i)
			<< "unresolved endpoints -> unreachable sentinel";

		Globals->ICOSAHEDRAL_WORLD = saved;
	};

	// When both endpoints resolve to real but DISCONNECTED surface hexes, the icosahedral BFS
	// drains its frontier without reaching the target and bails out through the "ran out of
	// hexes" arm, again returning the 10000000 sentinel. Two placed hexes with no neighbor link
	// between them reproduce exactly that: the frontier from the start empties after one step.
	"GetPlanarDistance returns the sentinel when the icosahedral BFS is exhausted"_test = []
	{
		int saved = Globals->ICOSAHEDRAL_WORLD;
		Globals->ICOSAHEDRAL_WORLD = 1;

		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		ARegionArray *surf = new ARegionArray(8, 8);
		regs->pRegionArrays[1] = surf;

		ARegion *a = new ARegion(); a->SetLoc(0, 0, ARegionArray::LEVEL_SURFACE);
		ARegion *b = new ARegion(); b->SetLoc(2, 0, ARegionArray::LEVEL_SURFACE);
		surf->SetRegion(0, 0, a); // start resolves
		surf->SetRegion(2, 0, b); // target resolves, but...
		regs->Add(a);
		regs->Add(b);
		// ...deliberately NO neighbor links: FindConnectedRegions adds nothing, so after the
		// first step the queue is empty (start->next == 0) and the loop returns the sentinel.
		// maxdist = -1 keeps the loop running until the frontier is genuinely exhausted.
		expect(regs->GetPlanarDistance(a, b, 0, -1) == 10000000_i)
			<< "disconnected target -> BFS exhausts -> sentinel";

		Globals->ICOSAHEDRAL_WORLD = saved;
	};

	// ShortPrint prefixes underworld levels with "deep"/"very deep" (or, under EASIER_UNDERWORLD,
	// the raw level number in angle brackets). Both need UNDERWORLD_LEVELS > 0, which the unittest
	// ruleset sets to 0 -- so flip it. pArr->strName must be set for the depth text to appear.
	"ShortPrint prefixes underworld depth text"_test = []
	{
		int savedU = Globals->UNDERWORLD_LEVELS;
		int savedD = Globals->UNDERDEEP_LEVELS;
		int savedE = Globals->EASIER_UNDERWORLD;
		Globals->UNDERWORLD_LEVELS = 3; // levels 3 and 4 count as underworld
		Globals->UNDERDEEP_LEVELS = 0;
		Globals->EASIER_UNDERWORLD = 0;

		ARegionList *regs = new ARegionList();
		regs->CreateLevels(6);
		ARegionArray *lvl3 = new ARegionArray(8, 8); lvl3->SetName("Underdark");
		ARegionArray *lvl4 = new ARegionArray(8, 8); lvl4->SetName("Underdark");
		regs->pRegionArrays[3] = lvl3;
		regs->pRegionArrays[4] = lvl4;

		ARegion *shallow = new ARegion(); shallow->SetName("Pit"); shallow->type = R_PLAIN;
		shallow->SetLoc(0, 0, 3);
		ARegion *deeper = new ARegion(); deeper->SetName("Abyssal"); deeper->type = R_PLAIN;
		deeper->SetLoc(0, 0, 4);

		std::string s3 = shallow->ShortPrint(regs).Str();
		std::string s4 = deeper->ShortPrint(regs).Str();
		expect(contains(s3, "deep Underdark")) << s3;
		expect(!contains(s3, "very deep")) << "level 3 is 'deep', not 'very deep'\n" << s3;
		expect(contains(s4, "very deep Underdark")) << s4;

		// EASIER_UNDERWORLD renders the raw level number in angle brackets instead.
		Globals->EASIER_UNDERWORLD = 1;
		std::string se = shallow->ShortPrint(regs).Str();
		expect(contains(se, "3 <Underdark>")) << se;

		Globals->UNDERWORLD_LEVELS = savedU;
		Globals->UNDERDEEP_LEVELS = savedD;
		Globals->EASIER_UNDERWORLD = savedE;
	};

	// WriteReport's open-gate line appends " of <numberofgates>" unless DISPERSE_GATE_NUMBERS is
	// set, in which case only the gate's own number is shown. The existing gate test never toggles
	// the flag, so the dispersed format was unverified.
	"WriteReport omits the gate count under DISPERSE_GATE_NUMBERS"_test = []
	{
		int savedG = Globals->GATES_EXIST;
		int savedD = Globals->DISPERSE_GATE_NUMBERS;
		Globals->GATES_EXIST = 1;
		Globals->DISPERSE_GATE_NUMBERS = 1;

		ARegionList *regs = makeRegions("Surface");
		regs->numberofgates = 12;
		ARegion *r = surfaceRegion(2, 2, "Portland");
		r->gate = 5;
		r->gateopen = 1;
		regs->Add(r);
		Faction *fac = new Faction(1);
		Unit *u = addUnit(r, 100, fac);
		u->items.SetNum(I_LEADERS, 1);
		u->SetSkill(S_GATE_LORE, 1);

		std::string out = capture([&](Areport *rep){ r->WriteReport(rep, fac, 0, regs); });
		expect(contains(out, "There is a Gate here (Gate 5).")) << out;
		expect(!contains(out, " of 12")) << "dispersed gates hide the total\n" << out;

		Globals->DISPERSE_GATE_NUMBERS = savedD;
		Globals->GATES_EXIST = savedG;
	};

	// MoveCost's road discount and the bad-weather multiplier are each tested alone; this pins
	// their combination. On a plain (movepoints 1) in bad weather the base doubles to 2, and a
	// connecting road halves it back to 1.
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

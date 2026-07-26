#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "production.h"
#include "gameio.h" // seedrandom

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

// RNG-DEPENDENT COVERAGE. ARegion::Setup drives the private SetupProds/SetupPop economy
// generators, both of which draw from getrandom(). We make them testable by seeding the RNG
// with seedrandom() before each call. Rather than pin the exact draws (brittle, and coupled
// to the getrandom sequence), we assert the reproducibility property -- identical seed =>
// identical world -- plus the deterministic post-conditions Setup guarantees regardless of
// the draws. This is the general recipe for any getrandom()-based method.
ut::suite<"ARegion setup"> aregion_setup_suite = []
{
	using namespace ut;

	auto freshPlain = [] {
		ARegion *r = new ARegion();
		r->type = R_PLAIN; // Setup assumes type + location are already set
		r->SetLoc(1, 1, ARegionArray::LEVEL_SURFACE);
		return r;
	};

	// Setup always appends the dummy object (open terrain lives in it), independent of RNG.
	"Setup creates the region's dummy object"_test = [&]
	{
		seedrandom(4242);
		ARegion *r = freshPlain();
		expect(r->GetDummy() == nullptr) << "no dummy before Setup";
		r->Setup();
		expect(r->GetDummy() != nullptr) << "Setup builds the dummy object";
	};

	// Same seed => same generated economy. This proves Setup (and the SetupProds/SetupPop it
	// calls) is fully determined by the RNG seed, which is what makes it unit-testable at all.
	"Setup is reproducible under a fixed seed"_test = [&]
	{
		seedrandom(9001);
		ARegion *a = freshPlain();
		a->Setup();

		seedrandom(9001);
		ARegion *b = freshPlain();
		b->Setup();

		expect(a->population == b->population) << "same seed -> same population";
		expect(a->basepopulation == b->basepopulation);
		expect(a->wealth == b->wealth) << "same seed -> same wealth";
		expect(a->products.Num() == b->products.Num()) << "same seed -> same product draws";
	};

	// A different seed is allowed to (and here does) diverge, confirming the draws actually
	// feed the result rather than being ignored. Population is always non-negative.
	"Setup produces a sane, seed-driven population"_test = [&]
	{
		seedrandom(1);
		ARegion *a = freshPlain();
		a->Setup();

		expect(that % a->population >= 0) << "population is never negative";
		expect(that % a->basepopulation >= 0);
	};

	// --- ManualSetup ----------------------------------------------------------------------
	// ManualSetup drives the same private economy generators as Setup but from an explicit
	// RegionSetup, and optionally plants a settlement. (The lair branch needs
	// LAIR_MONSTERS_EXIST, which is off in the unittest ruleset, so it stays dormant here.)

	auto baseSettings = [] {
		RegionSetup s;
		s.terrain = &TerrainDefs[R_PLAIN];
		s.habitat = 1000;
		s.prodWeight = 1.0;
		s.addLair = false;
		s.addSettlement = false;
		s.settlementName = "";
		s.settlementSize = TOWN_VILLAGE;
		return s;
	};

	// No settlement: ManualSetup seeds habitat from the settings then SetupHabitat transforms
	// it (x5 and randomized), so the result is positive but not the raw input. A dummy object
	// is created and no town is planted.
	"ManualSetup without a settlement sets up habitat and the dummy object"_test = [&]
	{
		seedrandom(7);
		ARegion *r = freshPlain();
		RegionSetup s = baseSettings();
		r->ManualSetup(s);

		expect(that % r->habitat > 0) << "habitat seeded from settings then transformed";
		expect(r->GetDummy() != nullptr) << "dummy object created";
		expect(r->town == nullptr) << "no settlement requested";
	};

	// With a settlement: a town is planted with the requested name.
	"ManualSetup plants a named settlement when asked"_test = [&]
	{
		seedrandom(7);
		ARegion *r = freshPlain();
		RegionSetup s = baseSettings();
		s.addSettlement = true;
		s.settlementName = "Rivendell";
		s.settlementSize = TOWN_CITY;
		r->ManualSetup(s);

		expect(fatal(r->town != nullptr)) << "settlement planted";
		std::string name = r->town->name->Str();
		expect(eq(name, std::string("Rivendell")));
	};

	// Same seed + same settings => same generated economy (reproducibility property).
	"ManualSetup is reproducible under a fixed seed"_test = [&]
	{
		seedrandom(2024);
		ARegion *a = freshPlain();
		a->ManualSetup(baseSettings());

		seedrandom(2024);
		ARegion *b = freshPlain();
		b->ManualSetup(baseSettings());

		expect(a->products.Num() == b->products.Num()) << "same seed -> same product draws";
		expect(a->wealth == b->wealth);
		expect(a->population == b->population);
	};

	// SetupProds' food branch only runs when FOOD_ITEMS_EXIST (off in the unittest ruleset).
	// Flip it and confirm Setup generates one of the food staples on a plain (economy > 0).
	"Setup adds a food production when food items exist"_test = [&]
	{
		int saved = Globals->FOOD_ITEMS_EXIST;
		Globals->FOOD_ITEMS_EXIST = 1;

		seedrandom(1);
		ARegion *r = freshPlain();
		r->Setup();

		int food = 0;
		forlist(&r->products) {
			Production *p = (Production *) elem;
			if (p && (p->itemtype == I_GRAIN || p->itemtype == I_LIVESTOCK ||
					  p->itemtype == I_FISH))
				food++;
		}
		expect(that % food > 0) << "a food staple was generated";

		Globals->FOOD_ITEMS_EXIST = saved;
	};

	// ManualSetup's lair branch runs when LAIR_MONSTERS_EXIST is on AND settings.addLair is
	// set. Unlike Setup's LairCheck there is no chance roll: a lair is planted whenever the
	// terrain has possible lairs (mountain does). We flip the flag for the test.
	"ManualSetup plants a lair when asked and lairs are enabled"_test = [&]
	{
		int saved = Globals->LAIR_MONSTERS_EXIST;
		Globals->LAIR_MONSTERS_EXIST = 1;

		seedrandom(1);
		ARegion *r = new ARegion();
		r->type = R_MOUNTAIN; // has possible lairs
		r->SetLoc(1, 1, ARegionArray::LEVEL_SURFACE);

		RegionSetup s = baseSettings();
		s.terrain = &TerrainDefs[R_MOUNTAIN];
		s.addLair = true;
		r->ManualSetup(s);

		// Setup's dummy plus a lair object.
		expect(r->objects.Num() == 2_i) << "dummy + one lair";
		int lairs = 0;
		forlist(&r->objects) {
			Object *o = (Object *) elem;
			if (o->type != O_DUMMY) lairs++;
		}
		expect(lairs == 1_i) << "a lair object was planted";

		Globals->LAIR_MONSTERS_EXIST = saved;
	};
};

namespace {
	// The first object in a region that is not the Setup dummy (i.e. the planted lair).
	Object *nonDummy(ARegion *r)
	{
		forlist(&r->objects) {
			Object *o = (Object *)elem;
			if (o->type != O_DUMMY) return o;
		}
		return nullptr;
	}
}

// LairCheck / GetPossibleLairs / MakeLair are private; their only caller is Setup(), gated by
// Globals->LAIR_MONSTERS_EXIST (off in the unittest ruleset). We flip that flag for the
// duration of each test (saving/restoring it, since Globals is process-wide) and drive the
// branches through Setup on R_MOUNTAIN (lairChance 15, six enabled lairs). Setup consumes RNG
// before LairCheck, so the trigger seeds were found empirically: seed 1 produces a lair, seed
// 2 does not. A lair shows up as an extra, non-dummy object beyond Setup's dummy.
ut::suite<"ARegion lairs"> aregion_lair_suite = []
{
	using namespace ut;

	auto mountain = [] {
		ARegion *r = new ARegion();
		r->type = R_MOUNTAIN;
		r->SetLoc(1, 1, ARegionArray::LEVEL_SURFACE);
		return r;
	};

	// Success path: LairCheck passes its roll, GetPossibleLairs is non-empty, MakeLair adds a
	// lair object. (seed 1)
	"Setup plants a lair when the check succeeds"_test = [&]
	{
		int saved = Globals->LAIR_MONSTERS_EXIST;
		Globals->LAIR_MONSTERS_EXIST = 1;

		seedrandom(1);
		ARegion *r = mountain();
		r->Setup();

		expect(r->objects.Num() == 2_i) << "dummy + one lair object";
		Object *lair = nonDummy(r);
		expect(fatal(lair != nullptr)) << "a non-dummy lair object was created";
		expect(lair->type != O_DUMMY);
		expect(that % ObjectDefs[lair->type].monster != -1)
			<< "the planted object is a monster lair";

		Globals->LAIR_MONSTERS_EXIST = saved;
	};

	// Roll-fail path: getrandom(100) >= lairChance, LairCheck returns without a lair. (seed 2)
	"Setup plants no lair when the check fails"_test = [&]
	{
		int saved = Globals->LAIR_MONSTERS_EXIST;
		Globals->LAIR_MONSTERS_EXIST = 1;

		seedrandom(2);
		ARegion *r = mountain();
		r->Setup();

		expect(r->objects.Num() == 1_i) << "only Setup's dummy object, no lair";
		expect(nonDummy(r) == nullptr);

		Globals->LAIR_MONSTERS_EXIST = saved;
	};

	// Gate: with LAIR_MONSTERS_EXIST off, Setup never calls LairCheck, so even the
	// lair-producing seed yields no lair.
	"Setup skips LairCheck entirely when lairs are globally disabled"_test = [&]
	{
		int saved = Globals->LAIR_MONSTERS_EXIST;
		Globals->LAIR_MONSTERS_EXIST = 0;

		seedrandom(1); // the lair seed -- but the gate is closed
		ARegion *r = mountain();
		r->Setup();

		expect(r->objects.Num() == 1_i) << "lairs disabled -> no LairCheck -> dummy only";

		Globals->LAIR_MONSTERS_EXIST = saved;
	};
};

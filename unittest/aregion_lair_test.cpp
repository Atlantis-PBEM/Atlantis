#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "gameio.h" // seedrandom

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

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

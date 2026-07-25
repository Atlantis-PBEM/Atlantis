#include "external/boost/ut.hpp"

#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "unit.h"
#include "faction.h"
#include "gameio.h" // seedrandom

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	// O_TOWER decays (maxMonthlyDecay == 1, no NEVERDECAY flag); O_DUMMY has NEVERDECAY.
	// A region on a surface level whose array pRegs knows about, so RunDecayEvent's
	// ShortPrint(pRegs) works.
	struct World {
		ARegionList *regs;
		ARegion *reg;
	};

	World makeWorld()
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		ARegionArray *surface = new ARegionArray(8, 8);
		surface->SetName("Surface");
		regs->pRegionArrays[1] = surface;

		ARegion *r = new ARegion();
		r->SetName("Testville");
		r->type = R_PLAIN; // GetMaxClicks / ShortPrint need a real terrain
		r->SetLoc(2, 2, ARegionArray::LEVEL_SURFACE);
		regs->Add(r);
		return { regs, r };
	}

	Object *addObj(ARegion *r, int type)
	{
		Object *o = new Object(r);
		o->type = type;
		r->objects.Add(o);
		return o;
	}
}

// DoDecayCheck -> DoDecayClicks -> RunDecayEvent. Branches covered: NEVERDECAY skip (both in
// DoDecayCheck and the DoDecayClicks guard), the clamp to maxMonthlyDecay, the incomplete>0
// event trigger, and the RunDecayEvent fan-out (present factions vs none).
ut::suite<"ARegion decay pipeline"> aregion_decay_pipeline_suite = []
{
	using namespace ut;

	// DoDecayCheck decays a normal building but leaves a NEVERDECAY object untouched. We make
	// PillageCheck large (maxwages - wages) so clicks are clamped to maxMonthlyDecay,
	// removing RNG from the assertion.
	"DoDecayCheck decays buildings but skips NEVERDECAY objects"_test = []
	{
		seedrandom(1);
		World w = makeWorld();
		w.reg->maxwages = 1000;
		w.reg->wages = 0; // PillageCheck == 1000 -> clicks clamp to maxMonthlyDecay

		Object *dummy = addObj(w.reg, O_DUMMY);  // NEVERDECAY
		Object *tower = addObj(w.reg, O_TOWER);  // maxMonthlyDecay == 1

		w.reg->DoDecayCheck(w.regs);

		expect(dummy->incomplete == 0_i) << "NEVERDECAY object is skipped";
		expect(tower->incomplete == 1_i) << "tower decays by its clamped maxMonthlyDecay";
	};

	// DoDecayClicks clamps clicks to maxMonthlyDecay when PillageCheck overflows it.
	"DoDecayClicks clamps to maxMonthlyDecay"_test = []
	{
		seedrandom(1);
		World w = makeWorld();
		w.reg->maxwages = 1000;
		w.reg->wages = 0;
		Object *tower = addObj(w.reg, O_TOWER);

		w.reg->DoDecayClicks(tower, w.regs);
		expect(tower->incomplete == 1_i) << "clamped to O_TOWER maxMonthlyDecay (1)";
	};

	// DoDecayClicks returns immediately for a NEVERDECAY object, even with a huge pillage add.
	"DoDecayClicks returns early for a NEVERDECAY object"_test = []
	{
		seedrandom(1);
		World w = makeWorld();
		w.reg->maxwages = 1000;
		w.reg->wages = 0;
		Object *dummy = addObj(w.reg, O_DUMMY);

		w.reg->DoDecayClicks(dummy, w.regs);
		expect(dummy->incomplete == 0_i) << "NEVERDECAY -> no decay";
	};

	// RunDecayEvent notifies every faction present in the region. The event text carries the
	// decay flavor, the object name/type and the region short-print.
	"RunDecayEvent notifies each present faction"_test = []
	{
		World w = makeWorld();
		Faction *fac = new Faction(1); // non-NPC by default -> Event records
		Object *tower = addObj(w.reg, O_TOWER);
		// Put a unit of the faction in the region so it counts as present.
		Unit *u = new Unit(100, fac, 0);
		u->MoveUnit(tower);

		w.reg->RunDecayEvent(tower, w.regs);

		expect(fatal(fac->events.Num() == 1_i)) << "present faction got one decay event";
		std::string ev = ((AString *)fac->events.First())->Str();
		expect(ev.find("Tower") != std::string::npos) << "names the object type\n" << ev;
		expect(ev.find("damaged") != std::string::npos) << "carries decay flavor\n" << ev;
	};

	// With no factions present, RunDecayEvent iterates an empty list -- no events, no crash.
	"RunDecayEvent is a no-op with nobody present"_test = []
	{
		World w = makeWorld();
		Object *tower = addObj(w.reg, O_TOWER);
		w.reg->RunDecayEvent(tower, w.regs); // must not crash
		expect(w.reg->PresentFactions()->Num() == 0_i) << "nobody present -> nothing to notify";
	};
};

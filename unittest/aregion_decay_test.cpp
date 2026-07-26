#include "external/boost/ut.hpp"

#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "unit.h"
#include "faction.h"
#include "gameio.h" // seedrandom (decay pipeline suite)

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

// NOTE: the unittest ruleset sets WEATHER_EXISTS = 0 (unittest/rules.cpp). GetMaxClicks and
// GetDecayFlavor both force badWeather = 0 in that case, so every assertion below exercises
// the "normal weather" branch regardless of the region's `weather` field. The bad-weather
// branches (avalanches, ground-freezing, lava flows, the +4/+5/+6 weather adds) are only
// reachable in a ruleset with weather enabled, i.e. the snapshot suite.

ut::suite<"ARegion decay"> aregion_decay_suite = []
{
	using namespace ut;

	// GetMaxClicks = terrainMult * (terrainAdd + 2) + (weatherAdd + 1), with weatherAdd = 0
	// here. The expected values are derived independently from the terrain rules in the
	// switch, not by calling the function back on itself.
	"GetMaxClicks computes per-terrain click budgets (normal weather)"_test = []
	{
		auto clicks = [](int terrain) {
			ARegion *r = new ARegion();
			r->type = terrain;
			r->weather = W_NORMAL;
			r->clearskies = 0;
			return r->GetMaxClicks();
		};

		// plain: terrainAdd -1, mult 1 -> 1*(1) + 1 = 2
		expect(clicks(R_PLAIN) == 2_i);
		// tundra: same coefficients as plain -> 2
		expect(clicks(R_TUNDRA) == 2_i);
		// mountain: mult 2, add 0 -> 2*2 + 1 = 5
		expect(clicks(R_MOUNTAIN) == 5_i);
		// forest: add -1, mult 2 -> 2*1 + 1 = 3
		expect(clicks(R_FOREST) == 3_i);
		// cavern: add 1, mult 2 -> 2*3 + 1 = 7
		expect(clicks(R_CAVERN) == 7_i);
		// ocean hits the default arm: add 0, mult 1 -> 1*2 + 1 = 3
		expect(clicks(R_OCEAN) == 3_i);
	};

	// REGRESSION GUARD: the R_DESERT case in GetMaxClicks (aregion.cpp ~line 575) once
	// lacked a `break` and fell through into the R_CAVERN block, so desert wrongly reported
	// the cavern click budget (7). With the break in place, desert uses its own coefficients
	// (terrainAdd = -1, terrainMult = 1 -> 2) and is therefore distinct from cavern (7).
	// If the break is ever dropped again, both expectations below fail.
	"GetMaxClicks: desert uses its own click budget, not cavern's"_test = []
	{
		ARegion *desert = new ARegion();
		desert->type = R_DESERT;
		desert->weather = W_NORMAL;
		desert->clearskies = 0;

		ARegion *cavern = new ARegion();
		cavern->type = R_CAVERN;
		cavern->weather = W_NORMAL;
		cavern->clearskies = 0;

		// desert: terrainAdd -1, mult 1 -> 1*(1) + 1 = 2
		expect(desert->GetMaxClicks() == 2_i)
			<< "desert must use its own coefficients, not fall through to cavern";
		expect(desert->GetMaxClicks() != cavern->GetMaxClicks())
			<< "desert (2) and cavern (7) must differ once the break is present";
	};

	// GetDecayFlavor picks a damage message from the terrain type. With badWeather = 0 the
	// tundra/mountain/cavern arms take their fair-weather text.
	"GetDecayFlavor returns terrain-appropriate text (normal weather)"_test = []
	{
		auto flavor = [](int terrain) {
			ARegion *r = new ARegion();
			r->type = terrain;
			r->weather = W_NORMAL;
			r->clearskies = 0;
			AString a = r->GetDecayFlavor();
			return std::string(a.Str());
		};

		expect(eq(flavor(R_PLAIN),    std::string("Floods have damaged ")));
		expect(eq(flavor(R_DESERT),   std::string("Flashfloods have damaged ")));
		expect(eq(flavor(R_TUNDRA),   std::string("Ground thaw has damaged ")));
		expect(eq(flavor(R_MOUNTAIN), std::string("Rockslides have damaged ")));
		expect(eq(flavor(R_FOREST),   std::string("Encroaching vegetation has damaged ")));
		expect(eq(flavor(R_CAVERN),   std::string("Quakes have damaged ")));
		// R_OCEAN is not enumerated in the switch, so it hits the default arm.
		expect(eq(flavor(R_OCEAN),    std::string("Unexplained phenomena have damaged ")));
	};

	// PillageCheck returns the positive gap between maxwages and wages, clamped at 0.
	"PillageCheck returns the clamped wage gap"_test = []
	{
		ARegion *r = new ARegion();

		r->maxwages = 100;
		r->wages = 60;
		expect(r->PillageCheck() == 40_i);

		r->maxwages = 50;
		r->wages = 50;
		expect(r->PillageCheck() == 0_i) << "no gap -> 0";

		r->maxwages = 30;
		r->wages = 50;
		expect(r->PillageCheck() == 0_i) << "negative gap is clamped to 0";
	};

	// Wasteland has its own distinct decay flavor, reachable without weather.
	"GetDecayFlavor names magical radiation for wasteland"_test = []
	{
		ARegion *r = new ARegion();
		r->type = R_CERAN_WASTELAND;
		r->weather = W_NORMAL;
		r->clearskies = 0;
		std::string a = r->GetDecayFlavor().Str();
		expect(eq(a, std::string("Magical radiation has damaged ")));
	};

	// The bad-weather flavor variants (tundra/mountain/cavern) require WEATHER_EXISTS, which
	// the unittest ruleset disables -- flip it for the test. badWeather is set when the
	// weather is not W_NORMAL and clearskies is off.
	"GetDecayFlavor uses bad-weather variants when weather is enabled"_test = []
	{
		int saved = Globals->WEATHER_EXISTS;
		Globals->WEATHER_EXISTS = 1;

		auto flavor = [](int terrain) {
			ARegion *r = new ARegion();
			r->type = terrain;
			r->weather = W_WINTER; // not W_NORMAL -> bad weather
			r->clearskies = 0;
			return std::string(r->GetDecayFlavor().Str());
		};

		expect(eq(flavor(R_TUNDRA),   std::string("Ground freezing has damaged ")));
		expect(eq(flavor(R_MOUNTAIN), std::string("Avalanches have damaged ")));
		expect(eq(flavor(R_CAVERN),   std::string("Lava flows have damaged ")));

		// clearskies overrides bad weather back to the fair-weather variant.
		ARegion *clear = new ARegion();
		clear->type = R_MOUNTAIN;
		clear->weather = W_WINTER;
		clear->clearskies = 1;
		expect(eq(std::string(clear->GetDecayFlavor().Str()),
				std::string("Rockslides have damaged ")))
			<< "clearskies negates bad weather";

		Globals->WEATHER_EXISTS = saved;
	};

	// GetMaxClicks adds a weather penalty in bad weather (WEATHER_EXISTS on).
	"GetMaxClicks adds a weather penalty in bad weather"_test = []
	{
		int saved = Globals->WEATHER_EXISTS;
		Globals->WEATHER_EXISTS = 1;

		auto clicks = [](int terrain) {
			ARegion *r = new ARegion();
			r->type = terrain;
			r->weather = W_WINTER;
			r->clearskies = 0;
			return r->GetMaxClicks();
		};

		// plain: mult 1, add -1, weatherAdd 4 -> 1*(1) + (4+1) = 6
		expect(clicks(R_PLAIN) == 6_i);
		// mountain: mult 2, add 0, weatherAdd 4 -> 2*2 + 5 = 9
		expect(clicks(R_MOUNTAIN) == 9_i);

		// Plain and mountain share weatherAdd = 4, so they don't distinguish the switch arms
		// that set a different bad-weather penalty. Cover those distinct weatherAdd values too:
		// forest (+1), desert (+5) and cavern (+6). Each expected value is derived independently
		// from the switch coefficients, not by calling GetMaxClicks back on itself.
		// forest: mult 2, add -1, weatherAdd 1 -> 2*(1) + (1+1) = 4
		expect(clicks(R_FOREST) == 4_i);
		// desert: mult 1, add -1, weatherAdd 5 -> 1*(1) + (5+1) = 7
		expect(clicks(R_DESERT) == 7_i);
		// cavern: mult 2, add 1, weatherAdd 6 -> 2*(3) + (6+1) = 13
		expect(clicks(R_CAVERN) == 13_i);

		Globals->WEATHER_EXISTS = saved;
	};
};

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

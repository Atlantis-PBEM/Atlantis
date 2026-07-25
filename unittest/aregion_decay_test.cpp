#include "external/boost/ut.hpp"

#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"

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

		Globals->WEATHER_EXISTS = saved;
	};
};

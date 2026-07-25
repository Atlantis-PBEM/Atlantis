#include "external/boost/ut.hpp"

#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "market.h"
#include "production.h"
#include "aregion_test_util.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

using aregion_test::captureCout;
using aregion_test::contains;

namespace {
	ARegion *typedRegion(ARegionList *regs, int type)
	{
		ARegion *r = new ARegion();
		r->type = type;
		regs->Add(r);
		return r;
	}

	// Attach a town of a chosen size. TownType() = pop*(dev+220)/270 compared against
	// CITY_POP (20000 here): pop 1000 -> village, 10000 -> town, 25000 -> city (dev 0).
	void addTown(ARegion *r, int pop)
	{
		r->town = new TownInfo;
		r->town->name = new AString("Town");
		r->town->pop = pop;
		r->town->hab = pop;
		r->town->dev = 0; // TownInfo ctor leaves dev uninitialized
	}
}

// The statistics methods report via Awrite (-> std::cout) or std::cout directly and return
// nothing, so we capture stdout and assert on the printed lines.
ut::suite<"ARegion statistics"> aregion_stats_suite = []
{
	using namespace ut;

	// CalcDensities tallies regions per terrain type and prints the non-zero counts.
	"CalcDensities reports per-terrain counts"_test = []
	{
		ARegionList *regs = new ARegionList();
		typedRegion(regs, R_PLAIN);
		typedRegion(regs, R_PLAIN);
		typedRegion(regs, R_MOUNTAIN);

		std::string out = captureCout([&]{ regs->CalcDensities(); });
		expect(contains(out, "Densities:")) << out;
		expect(contains(out, "plain 2")) << out;
		expect(contains(out, "mountain 1")) << out;
	};

	// TownStatistics counts each settlement size and prints totals + percentages. It divides by
	// the settlement total; the zero-town path is pinned separately below.
	"TownStatistics counts villages, towns and cities"_test = []
	{
		ARegionList *regs = new ARegionList();
		addTown(typedRegion(regs, R_PLAIN), 1000);   // village
		addTown(typedRegion(regs, R_PLAIN), 10000);  // town
		addTown(typedRegion(regs, R_PLAIN), 25000);  // city

		std::string out = captureCout([&]{ regs->TownStatistics(); });
		expect(contains(out, "Settlements: 3")) << out;
		expect(contains(out, "Villages: 1")) << out;
		expect(contains(out, "Towns   : 1")) << out;
		expect(contains(out, "Cities  : 1")) << out;
	};

	// REGRESSION GUARD: with no towns the settlement total is 0. TownStatistics guards the
	// percentage divisions with `if (tot > 0)` (aregion.cpp ~line 2410); without that guard the
	// three `x * 100 / tot` divisions are a divide-by-zero -- SIGFPE -- which would crash the
	// whole unittest binary and report nothing. This exercises a region list that has regions but
	// no settlements (e.g. an all-ocean or freshly created level): it must not crash, and every
	// count/percentage must print as 0.
	"TownStatistics survives a settlement-free region list"_test = []
	{
		ARegionList *regs = new ARegionList();
		typedRegion(regs, R_OCEAN); // regions present, but none carry a town
		typedRegion(regs, R_PLAIN);

		std::string out = captureCout([&]{ regs->TownStatistics(); });
		expect(contains(out, "Settlements: 0")) << out;
		expect(contains(out, "Villages: 0 (0%)")) << out;
		expect(contains(out, "Towns   : 0 (0%)")) << out;
		expect(contains(out, "Cities  : 0 (0%)")) << out;
	};

	// ResoucesStatistics aggregates products and markets across regions and prints three
	// sections. I_SILVER is deliberately excluded.
	"ResoucesStatistics reports products, wanted and for-sale"_test = []
	{
		ARegionList *regs = new ARegionList();
		ARegion *r = typedRegion(regs, R_PLAIN);

		Production *p = new Production(I_IRON, 20);
		p->amount = 20;
		r->products.Add(p);
		r->markets.Add(new Market(M_SELL, I_IRON, 10, 15, 0, 10000, 0, 100)); // -> "wanted"
		r->markets.Add(new Market(M_BUY,  I_WOOD, 8, 7, 0, 10000, 0, 100));   // -> "for sale"

		std::string out = captureCout([&]{ regs->ResoucesStatistics(); });
		expect(contains(out, "Products:")) << out;
		expect(contains(out, "Wanted:")) << out;
		expect(contains(out, "For Sale:")) << out;
		expect(contains(out, "[IRON]")) << "iron product/market listed\n" << out;
		expect(contains(out, "[WOOD]")) << "wood for-sale listed\n" << out;
	};
};

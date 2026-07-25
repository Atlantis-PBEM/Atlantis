#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

// Constructor field-init and destructor exercise. Destructors have little to assert beyond
// "runs and frees without corruption", so these tests build a populated object, destroy it,
// and confirm control returns. NOTE: ~ARegionList and ~ARegionFlatArray free their array with
// scalar `delete` on a `new[]` allocation (a known delete/delete[] mismatch); these tests
// exercise that path -- harmless on this toolchain, but a sanitizer build will flag it.
ut::suite<"ARegion destructors"> aregion_dtor_suite = []
{
	using namespace ut;

	// Farsight's constructor zero-initializes every field.
	"Farsight constructor zero-initializes its fields"_test = []
	{
		Farsight *fs = new Farsight();
		expect(fs->faction == nullptr);
		expect(fs->unit == nullptr);
		expect(fs->level == 0_i);
		expect(fs->observation == 0_i);
		for (int i = 0; i < NDIRS; i++)
			expect(fs->exits_used[i] == 0_i);
	};

	// ~TownInfo frees its name.
	"TownInfo destructor runs cleanly"_test = []
	{
		TownInfo *t = new TownInfo;
		t->name = new AString("Town");
		delete t;
		expect(true) << "TownInfo destroyed without crashing";
	};

	// ~ARegion frees its name and (transitively) its TownInfo.
	"ARegion destructor frees name and town"_test = []
	{
		ARegion *r = new ARegion();
		r->SetName("Doomed");
		r->town = new TownInfo;
		r->town->name = new AString("Township");
		delete r; // runs ~TownInfo via ~ARegion
		expect(true) << "ARegion (with town) destroyed without crashing";
	};

	// ~ARegionArray frees its region pointer array and name.
	"ARegionArray destructor runs cleanly"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		arr->SetName("Surface");
		arr->SetRegion(0, 0, new ARegion());
		delete arr;
		expect(true) << "ARegionArray destroyed without crashing";
	};

	// ~ARegionFlatArray frees its region pointer array.
	"ARegionFlatArray destructor runs cleanly"_test = []
	{
		ARegionFlatArray *arr = new ARegionFlatArray(3);
		arr->SetRegion(0, new ARegion());
		delete arr;
		expect(true) << "ARegionFlatArray destroyed without crashing";
	};

	// ~ARegionGraph runs when a stack graph leaves scope.
	"ARegionGraph destructor runs cleanly"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		{
			ARegionGraph graph(arr); // destroyed at the end of this block
			expect(graph.get({0, 0}) == nullptr); // touch it so it is not optimized away
		}
		expect(true) << "ARegionGraph destroyed without crashing";
	};

	// ~ARegionList destroys each level array (running ~ARegionArray transitively).
	"ARegionList destructor destroys its level arrays"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);
		regs->pRegionArrays[1] = new ARegionArray(4, 4);
		regs->pRegionArrays[1]->SetName("Surface");
		delete regs; // runs ~ARegionArray for both levels
		expect(true) << "ARegionList destroyed without crashing";
	};
};

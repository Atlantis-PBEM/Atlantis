#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "aregion.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

ut::suite<"ARegion region list"> aregion_regionlist_suite = []
{
	using namespace ut;

	// ARegionList is an AList of ARegion; GetRegion(n) is a linear search by region num.
	"ARegionList::GetRegion(n) finds a region by num"_test = []
	{
		ARegionList *regs = new ARegionList();
		ARegion *r1 = new ARegion(); r1->num = 5;
		ARegion *r2 = new ARegion(); r2->num = 9;
		regs->Add(r1);
		regs->Add(r2);

		expect(regs->GetRegion(5) == r1);
		expect(regs->GetRegion(9) == r2);
		expect(regs->GetRegion(123) == nullptr);
	};

	// CreateLevels allocates the level array; GetRegionArray returns the stored pointer.
	"CreateLevels + GetRegionArray round-trips a level array"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(2);
		expect(regs->numLevels == 2_i);

		ARegionArray *arr0 = new ARegionArray(4, 4);
		ARegionArray *arr1 = new ARegionArray(4, 4);
		regs->pRegionArrays[0] = arr0;
		regs->pRegionArrays[1] = arr1;

		expect(regs->GetRegionArray(0) == arr0);
		expect(regs->GetRegionArray(1) == arr1);
	};

	// GetRegion(x,y,z) returns NULL when the level index is out of range.
	"ARegionList::GetRegion(x,y,z) is null for an out-of-range level"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(1);
		regs->pRegionArrays[0] = new ARegionArray(4, 4);

		expect(regs->GetRegion(0, 0, 1) == nullptr) << "z == numLevels is out of range";
		expect(regs->GetRegion(0, 0, 5) == nullptr);
	};

	// GetRegion(x,y,z) locates a region placed in the level's array, wrapping coordinates
	// modulo the array dimensions.
	"ARegionList::GetRegion(x,y,z) resolves via the level array with wrap"_test = []
	{
		ARegionList *regs = new ARegionList();
		regs->CreateLevels(1);
		ARegionArray *arr = new ARegionArray(4, 4);
		regs->pRegionArrays[0] = arr;

		ARegion *reg = new ARegion();
		arr->SetRegion(2, 0, reg); // even-parity cell

		expect(regs->GetRegion(2, 0, 0) == reg);
		// x wraps modulo 4: -2 -> 2, so it maps to the same cell.
		expect(regs->GetRegion(-2, 0, 0) == reg);
	};

	// ARegionArray stores regions only on cells where (x + y) is even; odd-parity cells
	// are always empty by construction. This is the hex-offset packing the whole map
	// relies on, so it is worth pinning explicitly.
	"ARegionArray SetRegion/GetRegion round-trips on even-parity cells"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		ARegion *a = new ARegion();
		ARegion *b = new ARegion();

		arr->SetRegion(0, 0, a); // 0+0 even
		arr->SetRegion(2, 2, b); // 2+2 even

		expect(arr->GetRegion(0, 0) == a);
		expect(arr->GetRegion(2, 2) == b);
	};

	"ARegionArray GetRegion returns null on odd-parity cells"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		// (1 + 0) is odd: no region can live here regardless of what was stored.
		expect(arr->GetRegion(1, 0) == nullptr);
		expect(arr->GetRegion(0, 1) == nullptr);
	};

	// GetRegion wraps coordinates toroidally before indexing.
	"ARegionArray GetRegion wraps out-of-bounds coordinates"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		ARegion *r = new ARegion();
		arr->SetRegion(2, 0, r);

		expect(arr->GetRegion(-2, 0) == r) << "-2 wraps to 2";
		expect(arr->GetRegion(6, 0) == r)  << "6 wraps to 2";
	};

	// SetName stores the name; passing null clears it back to empty.
	"ARegionArray SetName sets and clears the name"_test = []
	{
		ARegionArray *arr = new ARegionArray(4, 4);
		arr->SetName("Surface");
		expect(fatal(arr->strName != nullptr));
		std::string current = arr->strName->Str();
		std::string expected = "Surface";
		expect(eq(current, expected));

		arr->SetName(nullptr);
		expect(arr->strName == nullptr);
	};

	// ARegionFlatArray is a plain indexed array with no parity/wrap rules.
	"ARegionFlatArray SetRegion/GetRegion round-trips by index"_test = []
	{
		ARegionFlatArray *arr = new ARegionFlatArray(3);
		ARegion *r0 = new ARegion();
		ARegion *r2 = new ARegion();
		arr->SetRegion(0, r0);
		arr->SetRegion(2, r2);

		expect(arr->GetRegion(0) == r0);
		expect(arr->GetRegion(2) == r2);
	};
};

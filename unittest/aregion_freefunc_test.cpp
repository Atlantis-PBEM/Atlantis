#include "external/boost/ut.hpp"

#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "object.h"
#include "unit.h"
#include "faction.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

// TownString has external linkage but no prototype in any header (it is only referenced inside
// aregion.cpp, via ARegion::Print). Declare it here so the test can call it directly; the
// signature matches the definition at aregion.cpp:80.
AString TownString(int i);

namespace {
	// Build a region list with a nexus level (0) and a named surface level (1), matching the
	// scaffolding the report suite uses. ShortPrint/Print reach through pRegionArrays[zloc].
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
}

// GetUnit / GetRegion are the two free list-lookup helpers at the top of aregion.cpp (public
// via aregion.h). They walk an AList and match by ->num, returning 0 on a miss. Both the found
// and not-found arms were previously uncovered.
ut::suite<"ARegion free helpers"> aregion_freefunc_suite = []
{
	using namespace ut;

	// GetUnit(AList*, int) scans a list of Location elements and returns the one whose unit
	// carries the given number.
	"GetUnit finds a Location by unit number, else null"_test = []
	{
		Faction *fac = new Faction(1);
		AList list;

		auto addLoc = [&](int unitnum) {
			Location *l = new Location;
			l->unit = new Unit(unitnum, fac, 0);
			l->obj = 0;
			l->region = 0;
			list.Add(l);
			return l;
		};
		addLoc(10);
		Location *want = addLoc(20);
		addLoc(30);

		Location *got = GetUnit(&list, 20);
		expect(fatal(got != nullptr)) << "unit 20 is present";
		expect(got == want) << "returns the matching Location";
		expect(GetUnit(&list, 99) == nullptr) << "absent unit number -> null";
	};

	// GetRegion(AList*, int) does the same over a list of ARegionPtr elements, matching ptr->num.
	"GetRegion finds an ARegionPtr by region number, else null"_test = []
	{
		AList list;

		auto addPtr = [&](int num) {
			ARegion *r = new ARegion();
			r->num = num;
			ARegionPtr *p = new ARegionPtr();
			p->ptr = r;
			list.Add(p);
			return p;
		};
		addPtr(1);
		ARegionPtr *want = addPtr(7);
		addPtr(3);

		ARegionPtr *got = GetRegion(&list, 7);
		expect(fatal(got != nullptr)) << "region 7 is present";
		expect(got == want) << "returns the matching ARegionPtr";
		expect(GetRegion(&list, 42) == nullptr) << "absent region number -> null";
	};

	// TownString maps a TownType constant to its label. All four arms (village/town/city and
	// the "huh?" default) were previously uncovered; the only in-engine caller is Print.
	"TownString labels each town size, huh? for the default"_test = []
	{
		expect(eq(std::string(TownString(TOWN_VILLAGE).Str()), std::string("village")));
		expect(eq(std::string(TownString(TOWN_TOWN).Str()),    std::string("town")));
		expect(eq(std::string(TownString(TOWN_CITY).Str()),    std::string("city")));
		// Any value outside the enum falls through to the default arm.
		expect(eq(std::string(TownString(-1).Str()), std::string("huh?")));
		expect(eq(std::string(TownString(999).Str()), std::string("huh?")));
	};

	// ParseTerrain looks a token up first against every TerrainDefs[].type (abbreviation), then
	// against every TerrainDefs[].name, returning -1 on a miss.
	//
	// NOTE: with the shipped terrain tables the second (name) loop can never *return*: every
	// terrain name ("plain", "mountain", ...) is also some terrain's type, so any token that
	// would match a name has already matched a type in the first loop. The no-match case below
	// still *executes* the name loop (it just falls through to -1). So we assert the two
	// reachable outcomes: a type hit, and a miss.
	"ParseTerrain resolves a terrain abbreviation, -1 on a miss"_test = []
	{
		AString plain("plain");
		AString mountain("mountain");
		AString bogus("definitely_not_a_terrain");

		expect(ParseTerrain(&plain) == R_PLAIN) << "type 'plain' -> R_PLAIN";
		expect(ParseTerrain(&mountain) == R_MOUNTAIN) << "type 'mountain' -> R_MOUNTAIN";
		expect(ParseTerrain(&bogus) == -1_i) << "unknown token -> -1";
	};

	// Print appends the town clause -- ", contains <name> [<size>]" -- when the region has a
	// town. Every other Print/ShortPrint test uses a townless region, so this branch (and the
	// TownString call inside it) was previously unexercised.
	"Print appends the town clause for a region with a town"_test = []
	{
		ARegionList *regs = makeRegions("Surface");
		ARegion *r = new ARegion();
		r->SetName("Testville");
		r->type = R_PLAIN;
		r->SetLoc(2, 2, ARegionArray::LEVEL_SURFACE);
		// pop 100, dev 0 -> prestige well under CITY_POP/4 -> TOWN_VILLAGE.
		r->town = new TownInfo;
		r->town->name = new AString("Townsville");
		r->town->pop = 100;
		r->town->dev = 0;
		regs->Add(r);

		std::string shortForm = r->ShortPrint(regs).Str();
		std::string longForm = r->Print(regs).Str();

		expect(neq(longForm, shortForm)) << "town makes Print differ from ShortPrint";
		expect(longForm.find("contains Townsville [village]") != std::string::npos)
			<< "town clause with the village label\n" << longForm;
	};
};

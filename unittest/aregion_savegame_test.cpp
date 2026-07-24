#include "external/boost/ut.hpp"

#include <cstdio>
#include <string>

#include "game.h"
#include "gamedata.h"
#include "aregion.h"
#include "fileio.h"

// boost::ut exposes an `events` namespace that collides with the game's Events class, so a
// file-scope `using namespace boost::ut;` will not compile. Alias it, and pull the literals
// in inside the suite body where the collision does not apply.
namespace ut = boost::ut;

namespace {
	// A scratch filename in the current working directory (the unittest run dir). Each test
	// removes it before writing, because Aoutfile::OpenByName refuses to open a file that
	// already exists and is non-empty.
	const char *SCRATCH = "aregion_savegame_test.tmp";

	void freshScratch()
	{
		std::remove(SCRATCH);
	}
}

ut::suite<"ARegion savegame"> aregion_savegame_suite = []
{
	using namespace ut;

	// TownInfo persists name, pop and hab positionally. Crucially, `activity` is NOT
	// written by Writeout, so it does not survive a round-trip — it is recomputed during
	// the turn, not saved. This is a positional-protocol nuance: adding a field to Writeout
	// without matching Readin (or vice versa) would silently corrupt every later field.
	"TownInfo round-trips name/pop/hab but not activity"_test = []
	{
		freshScratch();

		TownInfo *src = new TownInfo();
		src->name = new AString("Tharbad");
		src->pop = 1234;
		src->hab = 5000;
		src->activity = 999; // deliberately non-zero; must not survive the round-trip

		Aoutfile out;
		expect(fatal(out.OpenByName(SCRATCH) == 0_i));
		src->Writeout(&out);
		out.Close();

		ATL_VER v = CURRENT_ATL_VER;
		TownInfo *dst = new TownInfo();
		Ainfile in;
		expect(fatal(in.OpenByName(SCRATCH) == 0_i));
		dst->Readin(&in, v);
		in.Close();

		std::string name = dst->name->Str();
		std::string expectedName = "Tharbad";
		expect(eq(name, expectedName));
		expect(dst->pop == 1234_i);
		expect(dst->hab == 5000_i);
		expect(dst->activity == 0_i) << "activity is not persisted, stays at its default";

		freshScratch();
	};

	// A full ARegion (no town, no objects) round-trips its scalar fields and terrain type.
	// race == -1 is written as the sentinel "NO_RACE" and read back as -1 via LookupItem.
	"ARegion round-trips scalar fields with no town or objects"_test = []
	{
		freshScratch();

		ARegion *src = new ARegion();
		src->SetName("Eriador");
		src->num = 77;
		src->type = R_PLAIN;
		src->buildingseq = 3;
		src->gate = 0;            // no gate -> gatemonth is not written
		src->race = -1;           // -> "NO_RACE"
		src->population = 500;
		src->basepopulation = 450;
		src->wages = 130;
		src->maxwages = 145;
		src->wealth = 6000;
		src->elevation = 12;
		src->humidity = 34;
		src->temperature = 56;
		src->vegetation = 78;
		src->culture = 90;
		src->habitat = 800;
		src->development = 25;
		src->maxdevelopment = 40;
		src->town = 0;
		src->SetLoc(4, 6, 1);
		src->visited = 1;

		Aoutfile out;
		expect(fatal(out.OpenByName(SCRATCH) == 0_i));
		src->Writeout(&out);
		out.Close();

		ATL_VER v = CURRENT_ATL_VER;
		AList *facs = new AList(); // no objects, so the faction list is never consulted
		ARegion *dst = new ARegion();
		Ainfile in;
		expect(fatal(in.OpenByName(SCRATCH) == 0_i));
		dst->Readin(&in, facs, v);
		in.Close();

		std::string name = dst->name->Str();
		std::string expectedName = "Eriador";
		expect(eq(name, expectedName));
		expect(dst->num == 77_i);
		expect(dst->type == R_PLAIN);
		expect(dst->race == -1_i) << "NO_RACE sentinel resolves back to -1";
		expect(dst->population == 500_i);
		expect(dst->basepopulation == 450_i);
		expect(dst->wages == 130_i);
		expect(dst->maxwages == 145_i);
		expect(dst->wealth == 6000_i);
		expect(dst->elevation == 12_i);
		expect(dst->humidity == 34_i);
		expect(dst->temperature == 56_i);
		expect(dst->vegetation == 78_i);
		expect(dst->culture == 90_i);
		expect(dst->habitat == 800_i);
		expect(dst->development == 25_i);
		expect(dst->maxdevelopment == 40_i);
		expect(dst->xloc == 4_i);
		expect(dst->yloc == 6_i);
		expect(dst->zloc == 1_i);
		expect(dst->visited == 1_i);
		expect(dst->town == nullptr) << "no town was written";
		expect(dst->objects.Num() == 0_i);

		freshScratch();
	};

	// A region carrying a town writes a present-flag then the TownInfo block; the reader
	// reconstructs the town. This exercises the conditional branch in ARegion save/load.
	"ARegion round-trips a present town"_test = []
	{
		freshScratch();

		ARegion *src = new ARegion();
		src->SetName("Bree");
		src->num = 1;
		src->type = R_PLAIN;
		src->buildingseq = 1;
		src->gate = 0;
		src->race = -1;
		src->population = 100;
		src->basepopulation = 100;
		src->wages = 100;
		src->maxwages = 100;
		src->wealth = 100;
		src->elevation = 1;
		src->humidity = 1;
		src->temperature = 1;
		src->vegetation = 1;
		src->culture = 1;
		src->habitat = 100;
		src->development = 1;
		src->maxdevelopment = 1;
		src->SetLoc(0, 0, 0);
		src->visited = 0;

		src->town = new TownInfo();
		src->town->name = new AString("Bree-town");
		src->town->pop = 321;
		src->town->hab = 654;

		Aoutfile out;
		expect(fatal(out.OpenByName(SCRATCH) == 0_i));
		src->Writeout(&out);
		out.Close();

		ATL_VER v = CURRENT_ATL_VER;
		AList *facs = new AList();
		ARegion *dst = new ARegion();
		Ainfile in;
		expect(fatal(in.OpenByName(SCRATCH) == 0_i));
		dst->Readin(&in, facs, v);
		in.Close();

		expect(fatal(dst->town != nullptr)) << "town must be reconstructed";
		std::string townName = dst->town->name->Str();
		std::string expectedTown = "Bree-town";
		expect(eq(townName, expectedTown));
		expect(dst->town->pop == 321_i);
		expect(dst->town->hab == 654_i);

		freshScratch();
	};
};

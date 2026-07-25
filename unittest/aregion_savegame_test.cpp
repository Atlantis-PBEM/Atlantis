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

	// Covers the positive sides of the two conditional fields: gate > 0 makes gatemonth
	// part of the stream, and race != -1 writes a real item abbreviation (vs the NO_RACE
	// sentinel covered above).
	"ARegion round-trips a gate month and a real race"_test = []
	{
		freshScratch();

		ARegion *src = new ARegion();
		src->SetName("Moria");
		src->num = 9;
		src->type = R_PLAIN;
		src->buildingseq = 1;
		src->gate = 7;          // gate > 0 -> gatemonth IS written
		src->gatemonth = 4;
		src->race = I_LEADERS;  // race != -1 -> "LEAD" is written
		src->population = 10; src->basepopulation = 10;
		src->wages = 0; src->maxwages = 0; src->wealth = 0;
		src->elevation = 0; src->humidity = 0; src->temperature = 0;
		src->vegetation = 0; src->culture = 0;
		src->habitat = 0; src->development = 0; src->maxdevelopment = 0;
		src->town = 0;
		src->SetLoc(1, 1, 1);
		src->visited = 0;

		Aoutfile out;
		expect(fatal(out.OpenByName(SCRATCH) == 0_i));
		src->Writeout(&out);
		out.Close();

		AList *facs = new AList();
		ARegion *dst = new ARegion();
		Ainfile in;
		expect(fatal(in.OpenByName(SCRATCH) == 0_i));
		dst->Readin(&in, facs, CURRENT_ATL_VER);
		in.Close();

		expect(dst->gate == 7_i);
		expect(dst->gatemonth == 4_i) << "gatemonth persisted because gate > 0";
		expect(eq(dst->race, (int)I_LEADERS)) << "real race abbreviation round-trips";

		freshScratch();
	};

	// A region with no terrain (type == -1) writes the NO_TERRAIN sentinel, which
	// LookupRegionType maps back to -1.
	"ARegion round-trips the NO_TERRAIN sentinel"_test = []
	{
		freshScratch();

		ARegion *src = new ARegion();
		src->SetName("Unformed");
		src->num = 2;
		src->type = -1;         // -> "NO_TERRAIN"
		src->buildingseq = 1;
		src->gate = 0;
		src->race = -1;
		src->population = 0; src->basepopulation = 0;
		src->wages = 0; src->maxwages = 0; src->wealth = 0;
		src->elevation = 0; src->humidity = 0; src->temperature = 0;
		src->vegetation = 0; src->culture = 0;
		src->habitat = 0; src->development = 0; src->maxdevelopment = 0;
		src->town = 0;
		src->SetLoc(0, 0, 0);
		src->visited = 0;

		Aoutfile out;
		expect(fatal(out.OpenByName(SCRATCH) == 0_i));
		src->Writeout(&out);
		out.Close();

		AList *facs = new AList();
		ARegion *dst = new ARegion();
		Ainfile in;
		expect(fatal(in.OpenByName(SCRATCH) == 0_i));
		dst->Readin(&in, facs, CURRENT_ATL_VER);
		in.Close();

		expect(dst->type == -1_i) << "NO_TERRAIN resolves back to -1";

		freshScratch();
	};

	// A region carrying objects exercises the object read/write loop and the buildingseq
	// bookkeeping (buildingseq becomes max(object num) + 1 on load). Earlier tests all used
	// zero objects.
	"ARegion round-trips its objects and rebuilds buildingseq"_test = []
	{
		freshScratch();

		ARegion *src = new ARegion();
		src->SetName("Fortress");
		src->num = 4;
		src->type = R_PLAIN;
		src->buildingseq = 1;
		src->gate = 0;
		src->race = -1;
		src->population = 0; src->basepopulation = 0;
		src->wages = 0; src->maxwages = 0; src->wealth = 0;
		src->elevation = 0; src->humidity = 0; src->temperature = 0;
		src->vegetation = 0; src->culture = 0;
		src->habitat = 0; src->development = 0; src->maxdevelopment = 0;
		src->town = 0;
		src->SetLoc(0, 0, 0);
		src->visited = 0;

		Object *o = new Object(src);
		o->num = 5;
		o->type = O_TOWER;
		o->SetName(new AString("Watchtower"));
		src->objects.Add(o);

		Aoutfile out;
		expect(fatal(out.OpenByName(SCRATCH) == 0_i));
		src->Writeout(&out);
		out.Close();

		AList *facs = new AList(); // object has no units, so the faction list is unused
		ARegion *dst = new ARegion();
		Ainfile in;
		expect(fatal(in.OpenByName(SCRATCH) == 0_i));
		dst->Readin(&in, facs, CURRENT_ATL_VER);
		in.Close();

		expect(fatal(dst->objects.Num() == 1_i)) << "the object round-trips";
		Object *ro = dst->GetObject(5);
		expect(fatal(ro != nullptr));
		expect(ro->type == O_TOWER);
		expect(dst->buildingseq == 6_i) << "buildingseq is max object num + 1";

		freshScratch();
	};
};

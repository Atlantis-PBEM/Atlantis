#include "external/boost/ut.hpp"

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

namespace {
	Farsight *farsight(Faction *fac, Unit *u)
	{
		Farsight *fs = new Farsight();
		fs->faction = fac;
		fs->unit = u;
		return fs;
	}
}

// These cover branches left uncovered by aregion_observation_test.cpp / aregion_report_test.cpp:
//   * GetTrueSight's transit-passers arm (usepassers=1) -- only its farsees and present-unit
//     arms were tested (its twin GetObservation *does* test passers).
//   * CanMakeAdv's IMPROVED_FARSIGHT (farsees) and TRANSIT_REPORT (passers) arms -- only the
//     present-unit arm was reached, via WriteProducts.
// All three read Globals flags that are off in the unittest ruleset, so each flips the flag for
// its duration (Globals is process-wide -- save and restore).
ut::suite<"ARegion sight branch gaps"> aregion_sight_gaps_suite = []
{
	using namespace ut;

	// GetTrueSight consults the passers list only when usepassers is set AND the transit report
	// is configured to use unit skills and show units. The passer's unit carries the skill.
	"GetTrueSight reads passers when transit skills are enabled"_test = []
	{
		int savedTR = Globals->TRANSIT_REPORT;
		Globals->TRANSIT_REPORT =
			GameDefs::REPORT_USE_UNIT_SKILLS | GameDefs::REPORT_SHOW_UNITS;

		ARegion *reg = new ARegion();
		Faction *mine = new Faction(1);
		Unit *scout = new Unit(200, mine, 0);
		scout->items.SetNum(I_LEADERS, 1);
		scout->SetSkill(S_TRUE_SEEING, 6);
		reg->passers.Add(farsight(mine, scout));

		expect(reg->GetTrueSight(mine, 1) == 6_i) << "passer's true-seeing counted with usepassers";
		expect(reg->GetTrueSight(mine, 0) == 0_i) << "passers ignored without usepassers";

		Globals->TRANSIT_REPORT = savedTR;
	};

	// CanMakeAdv reports whether any unit the faction can see has the skill to reveal an advanced
	// product. I_MITHRIL needs mining (MINI) at pLevel 3. The present-unit arm is covered
	// elsewhere; here we drive the farsight and passer arms.
	"CanMakeAdv sees an advanced product through an improved-farsight witness"_test = []
	{
		int saved = Globals->IMPROVED_FARSIGHT;
		Globals->IMPROVED_FARSIGHT = 1;

		ARegion *reg = new ARegion();
		Faction *mine = new Faction(1);

		// A capable witness in the farsees list; its unit need not be present in the region.
		Unit *miner = new Unit(200, mine, 0);
		miner->items.SetNum(I_LEADERS, 1);
		miner->SetSkill(LookupSkill(new AString("MINI")), 3);
		reg->farsees.Add(farsight(mine, miner));

		expect(reg->CanMakeAdv(mine, I_MITHRIL) == 1_i) << "farsight witness reveals the product";

		// A different faction, with no witness, cannot.
		Faction *other = new Faction(2);
		expect(reg->CanMakeAdv(other, I_MITHRIL) == 0_i) << "no witness -> hidden";

		Globals->IMPROVED_FARSIGHT = saved;
	};

	"CanMakeAdv sees an advanced product through a transit passer"_test = []
	{
		int savedTR = Globals->TRANSIT_REPORT;
		Globals->TRANSIT_REPORT =
			GameDefs::REPORT_USE_UNIT_SKILLS | GameDefs::REPORT_SHOW_RESOURCES;

		ARegion *reg = new ARegion();
		Faction *mine = new Faction(1);

		Unit *miner = new Unit(200, mine, 0);
		miner->items.SetNum(I_LEADERS, 1);
		miner->SetSkill(LookupSkill(new AString("MINI")), 3);
		reg->passers.Add(farsight(mine, miner));

		expect(reg->CanMakeAdv(mine, I_MITHRIL) == 1_i) << "passer witness reveals the product";

		// Below the required level, the passer does not qualify.
		Unit *apprentice = new Unit(201, mine, 0);
		apprentice->items.SetNum(I_LEADERS, 1);
		apprentice->SetSkill(LookupSkill(new AString("MINI")), 2);
		ARegion *reg2 = new ARegion();
		reg2->passers.Add(farsight(mine, apprentice));
		expect(reg2->CanMakeAdv(mine, I_MITHRIL) == 0_i) << "below pLevel -> hidden";

		Globals->TRANSIT_REPORT = savedTR;
	};
};

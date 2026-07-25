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
	Unit *addUnit(ARegion *reg, int num, Faction *fac)
	{
		Object *o = new Object(reg);
		o->type = O_DUMMY;
		reg->objects.Add(o);
		Unit *u = new Unit(num, fac, 0);
		u->MoveUnit(o);
		return u;
	}

	Farsight *farsight(Faction *fac, Unit *u, int observation)
	{
		Farsight *fs = new Farsight();
		fs->faction = fac;
		fs->unit = u;
		fs->observation = observation;
		return fs;
	}
}

// GetObservation / GetTrueSight aggregate the best sight over three sources: the improved-
// farsight list (farsees), the transit passer list (passers), and the units physically
// present. The farsight lists are gated by Globals flags that are off in the unittest
// ruleset, so those tests flip the flag for their duration (Globals is process-wide, so we
// save and restore). NOTE: GetObservation reads a unit's "observation" *attribute*, which is
// 0 here because the unittest ruleset defines no attribute mods -- so the present-unit path
// can only be asserted as 0. GetTrueSight instead reads the S_TRUE_SEEING *skill*, which we
// can set directly, so its present-unit path is fully exercised.
ut::suite<"ARegion observation"> aregion_observation_suite = []
{
	using namespace ut;

	// Present units contribute only through the observation attribute, which is 0 in this
	// ruleset. With no farsight, the result is 0.
	"GetObservation from present units alone is zero in this ruleset"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		addUnit(reg, 100, fac);
		expect(reg->GetObservation(fac, 0) == 0_i);
	};

	// With IMPROVED_FARSIGHT on, the best matching farsees entry wins; entries for other
	// factions are ignored.
	"GetObservation takes the best matching farsees entry"_test = []
	{
		int saved = Globals->IMPROVED_FARSIGHT;
		Globals->IMPROVED_FARSIGHT = 1;

		ARegion *reg = new ARegion();
		Faction *mine = new Faction(1);
		Faction *other = new Faction(2);
		Unit *witness = addUnit(reg, 100, mine);

		reg->farsees.Add(farsight(mine, witness, 3));
		reg->farsees.Add(farsight(mine, witness, 7)); // best for my faction
		reg->farsees.Add(farsight(other, witness, 9)); // wrong faction, ignored

		expect(reg->GetObservation(mine, 0) == 7_i) << "max over my faction's farsights";

		Globals->IMPROVED_FARSIGHT = saved;
	};

	// The passers list is consulted only when usepassers is set AND the transit report is
	// configured to use unit skills and show units.
	"GetObservation reads passers when transit skills are enabled"_test = []
	{
		int savedTR = Globals->TRANSIT_REPORT;
		Globals->TRANSIT_REPORT =
			GameDefs::REPORT_USE_UNIT_SKILLS | GameDefs::REPORT_SHOW_UNITS;

		ARegion *reg = new ARegion();
		Faction *mine = new Faction(1);
		Unit *witness = addUnit(reg, 100, mine);
		reg->passers.Add(farsight(mine, witness, 5));

		expect(reg->GetObservation(mine, 1) == 5_i) << "passers counted with usepassers";
		expect(reg->GetObservation(mine, 0) == 0_i) << "passers ignored without usepassers";

		Globals->TRANSIT_REPORT = savedTR;
	};

	// GetTrueSight reads the S_TRUE_SEEING skill of present units directly.
	"GetTrueSight reads a present unit's true-seeing skill"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *mine = new Faction(1);
		Faction *other = new Faction(2);

		Unit *seer = addUnit(reg, 100, mine);
		seer->items.SetNum(I_LEADERS, 1);
		seer->SetSkill(S_TRUE_SEEING, 2);

		Unit *enemy = addUnit(reg, 101, other);
		enemy->items.SetNum(I_LEADERS, 1);
		enemy->SetSkill(S_TRUE_SEEING, 5); // belongs to another faction

		expect(reg->GetTrueSight(mine, 0) == 2_i) << "only my faction's skill counts";
		Faction *blind = new Faction(3);
		expect(reg->GetTrueSight(blind, 0) == 0_i) << "faction with no unit sees nothing";
	};

	// The improved-farsight list contributes true-seeing via the farsight unit's skill.
	"GetTrueSight reads the farsees list under IMPROVED_FARSIGHT"_test = []
	{
		int saved = Globals->IMPROVED_FARSIGHT;
		Globals->IMPROVED_FARSIGHT = 1;

		ARegion *reg = new ARegion();
		Faction *mine = new Faction(1);
		// The farsight's unit carries the skill; the unit itself need not be in this region.
		Unit *scout = new Unit(200, mine, 0);
		scout->items.SetNum(I_LEADERS, 1);
		scout->SetSkill(S_TRUE_SEEING, 4);
		reg->farsees.Add(farsight(mine, scout, 0));

		expect(reg->GetTrueSight(mine, 0) == 4_i);

		Globals->IMPROVED_FARSIGHT = saved;
	};
};

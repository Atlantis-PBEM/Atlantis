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
	Object *addObject(ARegion *reg, int num, int type = O_DUMMY)
	{
		Object *o = new Object(reg);
		o->num = num;
		o->type = type;
		reg->objects.Add(o);
		return o;
	}

	Unit *addUnit(Object *obj, int num, Faction *fac)
	{
		Unit *u = new Unit(num, fac, 0);
		u->MoveUnit(obj);
		return u;
	}

	// IsAlive() (and GetSoldiers()) count IsSoldier items; leaders carry IT_MAN, so one
	// leader is enough to make a unit count as a live soldier for guard purposes.
	void makeAlive(Unit *u)
	{
		u->items.SetNum(I_LEADERS, 1);
	}
}

ut::suite<"ARegion control"> aregion_control_suite = []
{
	using namespace ut;

	// HasShaft is true iff some object has an inner link (inner != -1).
	"HasShaft reflects an object with an inner link"_test = []
	{
		ARegion *reg = new ARegion();
		Object *o = addObject(reg, 1);
		expect(reg->HasShaft() == 0_i) << "default object has inner == -1";
		o->inner = 5;
		expect(reg->HasShaft() == 1_i);
	};

	// IsGuarded is true iff any unit is set to GUARD_GUARD.
	"IsGuarded reflects a guarding unit"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		Object *o = addObject(reg, 1);
		Unit *u = addUnit(o, 100, fac);
		expect(reg->IsGuarded() == 0_i);
		u->guard = GUARD_GUARD;
		expect(reg->IsGuarded() == 1_i);
	};

	// CountWMons tallies only wandering-monster (U_WMON) units.
	"CountWMons counts wandering monster units only"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		Object *o = addObject(reg, 1);
		addUnit(o, 100, fac);                 // U_NORMAL by default
		Unit *m1 = addUnit(o, 101, fac); m1->type = U_WMON;
		Unit *m2 = addUnit(o, 102, fac); m2->type = U_WMON;
		expect(reg->CountWMons() == 2_i);
	};

	// CanPillage is blocked only by a LIVE guard of a DIFFERENT faction; attitude is
	// irrelevant here.
	"CanPillage is blocked by a live enemy guard only"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *me = new Faction(1);
		Faction *them = new Faction(2);
		Object *o = addObject(reg, 1);
		Unit *taxer = addUnit(o, 100, me);

		expect(reg->CanPillage(taxer) == 1_i) << "no guards -> allowed";

		// A guard of our own faction does not block us.
		Unit *ownGuard = addUnit(o, 101, me);
		ownGuard->guard = GUARD_GUARD;
		makeAlive(ownGuard);
		expect(reg->CanPillage(taxer) == 1_i) << "own-faction guard does not block";

		// A DEAD enemy guard does not block (IsAlive() is false with no soldiers).
		Unit *deadEnemy = addUnit(o, 102, them);
		deadEnemy->guard = GUARD_GUARD;
		expect(reg->CanPillage(taxer) == 1_i) << "non-alive enemy guard does not block";

		// A LIVE enemy guard blocks.
		makeAlive(deadEnemy);
		expect(reg->CanPillage(taxer) == 0_i) << "live enemy guard blocks pillage";
	};

	// CanTax is blocked by a live guard whose attitude toward the taxer is <= A_NEUTRAL.
	// We avoid the CanSee code path by using attitudes that short-circuit before it: a
	// friendly/ally attitude (>= A_FRIENDLY) is returned directly, and same-faction is
	// A_ALLY.
	"CanTax is blocked by a hostile/neutral live guard"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *me = new Faction(1);
		Faction *them = new Faction(2);
		Object *o = addObject(reg, 1);
		Unit *taxer = addUnit(o, 100, me);

		expect(reg->CanTax(taxer) == 1_i) << "no guards -> allowed";

		Unit *enemyGuard = addUnit(o, 101, them);
		enemyGuard->guard = GUARD_GUARD;
		makeAlive(enemyGuard);
		// Default attitude is A_NEUTRAL, so GetAttitude() yields A_NEUTRAL <= A_NEUTRAL.
		expect(reg->CanTax(taxer) == 0_i) << "neutral enemy guard blocks taxing";

		// Make the guard's faction friendly toward us: attitude > A_NEUTRAL unblocks.
		them->SetAttitude(me->num, A_FRIENDLY);
		expect(reg->CanTax(taxer) == 1_i) << "friendly guard permits taxing";
	};

	// CanGuard is stricter than CanTax: a live guard blocks unless its attitude is A_ALLY.
	"CanGuard requires ally attitude to share a region with a guard"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *me = new Faction(1);
		Faction *them = new Faction(2);
		Object *o = addObject(reg, 1);
		Unit *newGuard = addUnit(o, 100, me);

		expect(reg->CanGuard(newGuard) == 1_i) << "no guards -> allowed";

		Unit *enemyGuard = addUnit(o, 101, them);
		enemyGuard->guard = GUARD_GUARD;
		makeAlive(enemyGuard);
		them->SetAttitude(me->num, A_FRIENDLY);
		expect(reg->CanGuard(newGuard) == 0_i) << "friendly (< ally) still blocks guarding";

		them->SetAttitude(me->num, A_ALLY);
		expect(reg->CanGuard(newGuard) == 1_i) << "ally attitude permits guarding";
	};

	// Forbidden returns a forbidding unit, or null. Here we cover the null paths: no
	// guard at all, and a present-but-not-guarding unit (Forbids() bails on guard != GUARD).
	"Forbidden returns null when nobody is guarding"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *me = new Faction(1);
		Faction *them = new Faction(2);
		Object *o = addObject(reg, 1);
		Unit *mover = addUnit(o, 100, me);

		expect(reg->Forbidden(mover) == nullptr) << "empty region forbids nobody";

		Unit *idle = addUnit(o, 101, them);
		makeAlive(idle);
		idle->guard = GUARD_NONE; // not guarding -> Forbids() returns 0
		expect(reg->Forbidden(mover) == nullptr) << "non-guarding unit does not forbid";
	};

	// ForbiddenByAlly only considers guards belonging to a faction the mover treats as an
	// ally. A non-ally guard, even a forbidding one, is ignored by this variant. Here we
	// cover the null paths (which need no CanSee/CanCatch setup): no guard, and a guard
	// whose faction the mover does not consider an ally.
	"ForbiddenByAlly ignores guards the mover does not consider allies"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *me = new Faction(1);
		Faction *them = new Faction(2);
		Object *o = addObject(reg, 1);
		Unit *mover = addUnit(o, 100, me);

		expect(reg->ForbiddenByAlly(mover) == nullptr) << "empty region forbids nobody";

		Unit *guard = addUnit(o, 101, them);
		guard->guard = GUARD_GUARD;
		makeAlive(guard);
		// mover's attitude toward `them` defaults to A_NEUTRAL (not A_ALLY), so the guard
		// is not considered even though it is guarding.
		expect(reg->ForbiddenByAlly(mover) == nullptr)
			<< "a non-ally guard is not consulted by ForbiddenByAlly";
	};

	// ForbiddenShip walks the ship's units and returns 1 if any is forbidden. With no
	// guards in the region, none are forbidden.
	"ForbiddenShip is false when the region has no guards"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *me = new Faction(1);
		Object *ship = addObject(reg, 1, O_DUMMY);
		addUnit(ship, 100, me);
		expect(reg->ForbiddenShip(ship) == 0_i);
	};

	// HasCityGuard needs a U_GUARD-typed unit with soldiers set to GUARD_GUARD.
	"HasCityGuard requires an armed U_GUARD on guard"_test = []
	{
		ARegion *reg = new ARegion();
		Faction *fac = new Faction(1);
		Object *o = addObject(reg, 1);

		// A normal guarding unit is not a city guard.
		Unit *normal = addUnit(o, 100, fac);
		normal->guard = GUARD_GUARD;
		makeAlive(normal);
		expect(reg->HasCityGuard() == 0_i) << "U_NORMAL is not a city guard";

		// A U_GUARD with no soldiers does not count.
		Unit *emptyGuard = addUnit(o, 101, fac);
		emptyGuard->type = U_GUARD;
		emptyGuard->guard = GUARD_GUARD;
		expect(reg->HasCityGuard() == 0_i) << "U_GUARD with no soldiers does not count";

		// A U_GUARD with soldiers on guard is a city guard.
		makeAlive(emptyGuard);
		expect(reg->HasCityGuard() == 1_i);
	};

	// AddFleet records an alias -> fleet-num mapping that ResolveFleetAlias reads back.
	// NOTE: the default ARegion constructor does NOT initialize fleetalias (only Readin
	// does). Tests must seed it, matching how a loaded region starts at 1.
	"AddFleet and ResolveFleetAlias round-trip fleet aliases"_test = []
	{
		ARegion *reg = new ARegion();
		reg->fleetalias = 1;      // seed as Readin would
		reg->newfleets.clear();

		Object *fleet1 = new Object(reg); fleet1->num = 500;
		Object *fleet2 = new Object(reg); fleet2->num = 501;
		reg->AddFleet(fleet1); // alias 1 -> 500
		reg->AddFleet(fleet2); // alias 2 -> 501

		expect(reg->ResolveFleetAlias(1) == 500_i);
		expect(reg->ResolveFleetAlias(2) == 501_i);
		expect(reg->ResolveFleetAlias(99) == -1_i) << "unknown alias -> -1";
	};
};

// CheckFleets culls empty/sunk fleets and evacuates their crews. (The addObject helper above,
// with its default type == O_DUMMY, serves these three-argument calls unchanged.)
ut::suite<"ARegion fleets"> aregion_fleet_suite = []
{
	using namespace ut;

	// CheckFleets removes a fleet with no capacity (FleetCapacity() < 1 sets bail). An
	// empty O_FLEET has no ships and therefore zero capacity, so it is culled. NOTE: the
	// fixture must set a land terrain type -- CheckFleets indexes TerrainDefs[type], and on
	// land it also forces alive = 1 so removal is driven solely by the capacity check.
	"CheckFleets removes an empty (zero-capacity) fleet"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN;
		Object *dummy = addObject(reg, 1, O_DUMMY);
		addObject(reg, 2, O_FLEET); // empty fleet: no ships -> capacity 0

		expect(fatal(reg->objects.Num() == 2_i));
		reg->CheckFleets();

		expect(reg->objects.Num() == 1_i) << "the empty fleet is culled";
		expect(reg->GetObject(1) == dummy) << "the non-fleet object survives";
		expect(reg->GetObject(2) == nullptr) << "the fleet is gone";
	};

	// A non-fleet object is never touched by CheckFleets.
	"CheckFleets leaves non-fleet objects alone"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN;
		addObject(reg, 1, O_DUMMY);
		addObject(reg, 2, O_TOWER);

		reg->CheckFleets();
		expect(reg->objects.Num() == 2_i);
	};

	// When an empty fleet carries units, CheckFleets evacuates them to the region's dummy
	// object before removing the fleet, so no unit is lost.
	"CheckFleets evacuates units from a culled fleet to the dummy"_test = []
	{
		ARegion *reg = new ARegion();
		reg->type = R_PLAIN;
		Faction *fac = new Faction(1);
		Object *dummy = addObject(reg, 1, O_DUMMY);
		Object *fleet = addObject(reg, 2, O_FLEET);

		Unit *passenger = new Unit(100, fac, 0);
		passenger->MoveUnit(fleet);
		expect(fatal(fleet->units.Num() == 1_i));

		reg->CheckFleets();

		expect(reg->GetObject(2) == nullptr) << "empty fleet removed";
		expect(dummy->units.Num() == 1_i) << "passenger relocated to the dummy";
		expect(passenger->object == dummy);
	};

	// A crewless fleet that still has capacity (a real ship aboard, so bail is NOT set) is
	// culled only at sea: on the ocean `alive` stays 0 and the fleet sinks, but on land
	// CheckFleets force-sets `alive` so the same fleet survives. This exercises the
	// terrain-dependent `similar_type != R_OCEAN` branch.
	"CheckFleets sinks a crewless fleet at sea but keeps it on land"_test = []
	{
		// At sea: removed.
		ARegion *sea = new ARegion();
		sea->type = R_OCEAN;
		Object *seaFleet = addObject(sea, 1, O_FLEET);
		seaFleet->SetNumShips(I_LONGBOAT, 1); // capacity > 0 -> bail is not set
		expect(fatal(seaFleet->FleetCapacity() >= 1_i));
		sea->CheckFleets();
		expect(sea->GetObject(1) == nullptr) << "crewless fleet sinks at sea";

		// On land: the identical fleet survives.
		ARegion *land = new ARegion();
		land->type = R_PLAIN;
		Object *landFleet = addObject(land, 1, O_FLEET);
		landFleet->SetNumShips(I_LONGBOAT, 1);
		land->CheckFleets();
		expect(land->GetObject(1) != nullptr) << "on land the fleet is not auto-removed";
	};
};
